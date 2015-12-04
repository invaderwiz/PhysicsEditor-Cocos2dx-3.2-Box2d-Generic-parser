//
//  Box2DShapeDelegate.cpp
//  
//  Loads physics sprites created with http://www.PhysicsEditor.de
//  To be used with cocos2d-x
//
//  Created by Thomas Broquist
//	Modified by InvaderWiz
//
//	CodeAdnWeb urls:
//      http://www.PhysicsEditor.de
//      http://texturepacker.com
//      http://www.code-and-web.de
//  
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#include "Box2DShapeDelegate.h"

USING_NS_CC;

class FixtureDef
{
public:
	FixtureDef()
		: next(NULL) {}

	~FixtureDef()
	{
		delete next;
		delete fixture.shape;
	}

	FixtureDef *next;
	b2FixtureDef fixture;
	int callbackData;
};

class BodyDef
{
public:
	BodyDef()
		: fixtures(NULL) {}

	~BodyDef()
	{
		if (fixtures)
			delete fixtures;
	}

	FixtureDef *fixtures;
	Point anchorPoint;
};

Box2DShapeDelegate::~Box2DShapeDelegate()
{
	reset();
}

bool Box2DShapeDelegate::init() 
{
	return true;
}

Box2DShapeDelegate* Box2DShapeDelegate::getInstance()
{
	static Box2DShapeDelegate instance;
	return &instance;
}

void Box2DShapeDelegate::addShapesWithFile(const std::string& plist) 
{
	std::string path = FileUtils::getInstance()->fullPathForFilename(plist);
	ValueMap data = FileUtils::getInstance()->getValueMapFromFile(path);
	CCASSERT(data != ValueMapNull, "plist File was not found");
	CCASSERT((!data.empty()), "plist File is empty or non-existent");

	ValueMap& metaData = data["metadata"].asValueMap();
	int format = metaData.at("format").asInt();
	ptmRatio = metaData.at("ptm_ratio").asFloat();
	CCASSERT(format == 1, "plist Format is not supported");

	ValueMap& bodies = data.at("bodies").asValueMap();

	b2Vec2 vertices[b2_maxPolygonVertices];

	std::string bodyName;

	for (auto iter = bodies.begin(); iter != bodies.end(); ++iter)
	{
		bodyName = iter->first;
		ValueMap& bodyData = iter->second.asValueMap();

		BodyDef *bodyDef = new BodyDef();
		std::string pszContent = bodyData.at("anchorpoint").asString();
		bodyDef->anchorPoint = PointFromString(pszContent);

		ValueVector& fixtureList = bodyData.at("fixtures").asValueVector();
		FixtureDef **nextFixtureDef = &(bodyDef->fixtures);

		for (auto iter = fixtureList.begin(); iter != fixtureList.end(); ++iter)
		{
			ValueMap& fixtureData = iter->asValueMap();
			b2FixtureDef basicData;

			basicData.filter.categoryBits = fixtureData.at("filter_categoryBits").asInt();
			basicData.filter.maskBits = fixtureData.at("filter_maskBits").asInt();
			basicData.filter.groupIndex = fixtureData.at("filter_groupIndex").asInt();
			basicData.friction = fixtureData.at("friction").asFloat();
			basicData.density = fixtureData.at("density").asFloat();
			basicData.restitution = fixtureData.at("restitution").asFloat();
			basicData.isSensor = fixtureData.at("isSensor").asBool();

			std::string fixtureType = fixtureData.at("fixture_type").asString();

			int callbackData = 0;

			if (fixtureType == "POLYGON")
			{
				ValueVector& polygonsMap = fixtureData.at("polygons").asValueVector();

				for (auto iter = polygonsMap.begin(); iter != polygonsMap.end(); ++iter)
				{
					FixtureDef *fix = new FixtureDef();
					fix->fixture = basicData;
					fix->callbackData = callbackData;

					b2PolygonShape *polyShape = new b2PolygonShape();
					int vindex = 0;

					ValueVector& polygonMap = iter->asValueVector();
					//assert(polygonMap.size <= b2_maxPolygonVertices);

					for (auto iter = polygonMap.begin(); iter != polygonMap.end(); ++iter)
					{
						std::string polygonValue = iter->asString();
						Point offset = PointFromString(polygonValue);
						vertices[vindex].x = (offset.x / ptmRatio);
						vertices[vindex].y = (offset.y / ptmRatio);
						vindex++;
					}

					polyShape->Set(vertices, vindex);
					fix->fixture.shape = polyShape;

					*nextFixtureDef = fix;
					nextFixtureDef = &(fix->next);
				}
			}
			else if (fixtureType == "CIRCLE")
			{
				FixtureDef *fix = new FixtureDef();
				fix->fixture = basicData;
				fix->callbackData = callbackData;

				ValueMap& circleData = fixtureData.at("circle").asValueMap();

				b2CircleShape *circleShape = new b2CircleShape();

				circleShape->m_radius = circleData.at("radius").asFloat() / ptmRatio;
				Point p = PointFromString(circleData.at("position").asString());
				circleShape->m_p = b2Vec2(p.x / ptmRatio, p.y / ptmRatio);
				fix->fixture.shape = circleShape;

				*nextFixtureDef = fix;
				nextFixtureDef = &(fix->next);
			}

			else
			{
				CCLOG("Unknown fixtureType");
			}

			shapeObjects.insert(std::pair<std::string, BodyDef *>(bodyName, bodyDef));
		}
	}
	shapesAreLoaded = true;
}

void Box2DShapeDelegate::addFixturesToBody(b2Body *body, const std::string &shape) 
{
	auto pos = shapeObjects.find(shape);
	assert(pos != shapeObjects.end());

	BodyDef *so = (*pos).second;

	FixtureDef *fix = so->fixtures;
    while (fix) {
        body->CreateFixture(&fix->fixture);
        fix = fix->next;
    }
}

Point Box2DShapeDelegate::anchorPointForShape(const std::string &shape) 
{
	auto pos = shapeObjects.find(shape);
	//assert(pos != shapeObjects.end());
	
	BodyDef *bd = (*pos).second;
	return bd->anchorPoint;
}

void Box2DShapeDelegate::reset()
{
	for (auto iter = shapeObjects.begin(); iter != shapeObjects.end(); ++iter)
	{
		delete iter->second;
	}
	shapeObjects.clear();

	shapesAreLoaded = false;
}

bool Box2DShapeDelegate::areShapesLoaded()
{
	return shapesAreLoaded;
}