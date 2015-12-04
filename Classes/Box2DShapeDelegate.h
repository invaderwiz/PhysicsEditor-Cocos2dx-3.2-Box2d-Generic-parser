//
//  Box2DShapeDelegate.h
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

#pragma once

#include "cocos2d.h"
#include "Box2D\Box2D.h"

USING_NS_CC;

class BodyDef;
class b2Body;

class Box2DShapeDelegate 
{
public:
	bool init();
	static Box2DShapeDelegate* getInstance();
			
	void addShapesWithFile(const std::string &plist);
	void addFixturesToBody(b2Body *body, const std::string& shape);
	cocos2d::Point anchorPointForShape(const std::string& shape);
	void reset();

	bool areShapesLoaded();
		
private:
	Box2DShapeDelegate() {};
	~Box2DShapeDelegate();

	std::unordered_map<std::string, BodyDef*> shapeObjects;
	
	float ptmRatio;
	bool shapesAreLoaded = false;
};
