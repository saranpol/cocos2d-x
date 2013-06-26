/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2011      Zynga Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "CCScene.h"
#include "support/CCPointExtension.h"
#include "CCDirector.h"

NS_CC_BEGIN

CCScene::CCScene()
// #HLP_BEGIN
: mEnableSlideBack(false)
// #HLP_END
{
    m_bIgnoreAnchorPointForPosition = true;
    setAnchorPoint(ccp(0.5f, 0.5f));
}

CCScene::~CCScene()
{
}

bool CCScene::init()
{
    bool bRet = false;
     do 
     {
         CCDirector * pDirector;
         CC_BREAK_IF( ! (pDirector = CCDirector::sharedDirector()) );
         this->setContentSize(pDirector->getWinSize());
         // success
         //bRet = true;
         // #HLP_BEGIN
         bRet = CCLayer::init();
         setTouchEnabled(true);
         // #HLP_END
     } while (0);
     return bRet;
}

CCScene *CCScene::create()
{
    CCScene *pRet = new CCScene();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        CC_SAFE_DELETE(pRet);
        return NULL;
    }
}

// #HLP_BEGIN
void CCScene::registerWithTouchDispatcher()
{
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, CCLayer::getTouchPriority(), false);
}

bool CCScene::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
    if(!mEnableSlideBack)
        return false;
    return true;
}

void CCScene::delayBack() {
    clickBack(NULL, NULL);
}

void CCScene::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) {
    if(!mEnableSlideBack)
        return;
    CCPoint start = pTouch->getStartLocation();
    CCPoint end = pTouch->getLocation();
    if (start.x - end.x < -100.0f){
        scheduleOnce(schedule_selector(CCScene::delayBack), 0.0f);
    }
}

void CCScene::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) {
    
}


// #HLP_END



NS_CC_END
