/****************************************************************************
 Copyright (c) 2012 cocos2d-x.org
 Copyright (c) 2010 Sangwoo Im
 
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

#include "CCScrollView.h"

NS_CC_EXT_BEGIN

#define SCROLL_DEACCEL_RATE  0.95f
//#define SCROLL_DEACCEL_DIST  1.0f
//#define BOUNCE_DURATION      0.15f
// #HLP_BEGIN
#define SCROLL_DEACCEL_DIST  0.1f
#define BOUNCE_DURATION      0.5f
// #HLP_END
#define INSET_RATIO          0.2f
#define MOVE_INCH            7.0f/160.0f

static float convertDistanceFromPointToInch(float pointDis)
{
    float factor = ( CCEGLView::sharedOpenGLView()->getScaleX() + CCEGLView::sharedOpenGLView()->getScaleY() ) / 2;
    return pointDis * factor / CCDevice::getDPI();
}


CCScrollView::CCScrollView()
: m_fZoomScale(0.0f)
, m_fMinZoomScale(0.0f)
, m_fMaxZoomScale(0.0f)
, m_pDelegate(NULL)
, m_eDirection(kCCScrollViewDirectionBoth)
, m_bDragging(false)
, m_pContainer(NULL)
, m_bTouchMoved(false)
, m_bBounceable(false)
, m_bClippingToBounds(false)
, m_fTouchLength(0.0f)
, m_pTouches(NULL)
, m_fMinScale(0.0f)
, m_fMaxScale(0.0f)
// #HLP_BEGIN
, mIsPagingEnabled(false)
, m_bDisableVertical(false)
, m_bDisableHorizontal(false)
, m_bDidVertical(false)
, m_bDidHorizontal(false)
// #HLP_END
{

}

CCScrollView::~CCScrollView()
{
    m_pTouches->release();
}

CCScrollView* CCScrollView::create(CCSize size, CCNode* container/* = NULL*/)
{
    CCScrollView* pRet = new CCScrollView();
    if (pRet && pRet->initWithViewSize(size, container))
    {
        pRet->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(pRet);
    }
    return pRet;
}

CCScrollView* CCScrollView::create()
{
    CCScrollView* pRet = new CCScrollView();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(pRet);
    }
    return pRet;
}


bool CCScrollView::initWithViewSize(CCSize size, CCNode *container/* = NULL*/)
{
    if (CCLayer::init())
    {
        m_pContainer = container;
        
        if (!this->m_pContainer)
        {
            m_pContainer = CCLayer::create();
            this->m_pContainer->ignoreAnchorPointForPosition(false);
            this->m_pContainer->setAnchorPoint(ccp(0.0f, 0.0f));
        }

        this->setViewSize(size);

        setTouchEnabled(true);
        m_pTouches = new CCArray();
        m_pDelegate = NULL;
        m_bBounceable = true;
        m_bClippingToBounds = true;
        //m_pContainer->setContentSize(CCSizeZero);
        m_eDirection  = kCCScrollViewDirectionBoth;
        m_pContainer->setPosition(ccp(0.0f, 0.0f));
        m_fTouchLength = 0.0f;
        
        this->addChild(m_pContainer);
        m_fMinScale = m_fMaxScale = 1.0f;
        return true;
    }
    return false;
}

bool CCScrollView::init()
{
    return this->initWithViewSize(CCSizeMake(200, 200), NULL);
}

void CCScrollView::registerWithTouchDispatcher()
{
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, false);
}

bool CCScrollView::isNodeVisible(CCNode* node)
{
    const CCPoint offset = this->getContentOffset();
    const CCSize  size   = this->getViewSize();
    const float   scale  = this->getZoomScale();
    
    CCRect viewRect;
    
    viewRect = CCRectMake(-offset.x/scale, -offset.y/scale, size.width/scale, size.height/scale); 
    
    return viewRect.intersectsRect(node->boundingBox());
}

void CCScrollView::pause(CCObject* sender)
{
    m_pContainer->pauseSchedulerAndActions();

    CCObject* pObj = NULL;
    CCArray* pChildren = m_pContainer->getChildren();

    CCARRAY_FOREACH(pChildren, pObj)
    {
        CCNode* pChild = (CCNode*)pObj;
        pChild->pauseSchedulerAndActions();
    }
}

void CCScrollView::resume(CCObject* sender)
{
    CCObject* pObj = NULL;
    CCArray* pChildren = m_pContainer->getChildren();

    CCARRAY_FOREACH(pChildren, pObj)
    {
        CCNode* pChild = (CCNode*)pObj;
        pChild->resumeSchedulerAndActions();
    }

    m_pContainer->resumeSchedulerAndActions();
}

void CCScrollView::setTouchEnabled(bool e)
{
    CCLayer::setTouchEnabled(e);
    if (!e)
    {
        m_bDragging = false;
        m_bTouchMoved = false;
        m_pTouches->removeAllObjects();
    }
}

void CCScrollView::setContentOffset(CCPoint offset, bool animated/* = false*/)
{
    if (animated)
    { //animate scrolling
        this->setContentOffsetInDuration(offset, BOUNCE_DURATION);
    } 
    else
    { //set the container position directly
        if (!m_bBounceable)
        {
            const CCPoint minOffset = this->minContainerOffset();
            const CCPoint maxOffset = this->maxContainerOffset();
            
            offset.x = MAX(minOffset.x, MIN(maxOffset.x, offset.x));
            offset.y = MAX(minOffset.y, MIN(maxOffset.y, offset.y));
        }

        m_pContainer->setPosition(offset);

        if (m_pDelegate != NULL)
        {
            m_pDelegate->scrollViewDidScroll(this);   
        }
    }
}

void CCScrollView::setContentOffsetInDuration(CCPoint offset, float dt)
{
    CCFiniteTimeAction *scroll, *expire;
    
    scroll = CCMoveTo::create(dt, offset);
    // #HLP_BEGIN
    //scroll = CCEaseOut::create((CCActionInterval*)scroll, 10);
    //scroll = CCEaseElasticOut::create((CCActionInterval*)scroll);
    //scroll = CCEaseInOut::create((CCActionInterval*)scroll,3);
    scroll = CCEaseSineOut::create((CCActionInterval*)scroll);
    // #HLP_END
    expire = CCCallFuncN::create(this, callfuncN_selector(CCScrollView::stoppedAnimatedScroll));
    m_pContainer->runAction(CCSequence::create(scroll, expire, NULL));
    this->schedule(schedule_selector(CCScrollView::performedAnimatedScroll));
}

CCPoint CCScrollView::getContentOffset()
{
    return m_pContainer->getPosition();
}

void CCScrollView::setZoomScale(float s)
{
    if (m_pContainer->getScale() != s)
    {
        CCPoint oldCenter, newCenter;
        CCPoint center;
        
        if (m_fTouchLength == 0.0f) 
        {
            center = ccp(m_tViewSize.width*0.5f, m_tViewSize.height*0.5f);
            center = this->convertToWorldSpace(center);
        }
        else
        {
            center = m_tTouchPoint;
        }
        
        oldCenter = m_pContainer->convertToNodeSpace(center);
        m_pContainer->setScale(MAX(m_fMinScale, MIN(m_fMaxScale, s)));
        newCenter = m_pContainer->convertToWorldSpace(oldCenter);
        
        const CCPoint offset = ccpSub(center, newCenter);
        if (m_pDelegate != NULL)
        {
            m_pDelegate->scrollViewDidZoom(this);
        }
        this->setContentOffset(ccpAdd(m_pContainer->getPosition(),offset));
    }
}

float CCScrollView::getZoomScale()
{
    return m_pContainer->getScale();
}

void CCScrollView::setZoomScale(float s, bool animated)
{
    if (animated)
    {
        this->setZoomScaleInDuration(s, BOUNCE_DURATION);
    }
    else
    {
        this->setZoomScale(s);
    }
}

void CCScrollView::setZoomScaleInDuration(float s, float dt)
{
    if (dt > 0)
    {
        if (m_pContainer->getScale() != s)
        {
            CCActionTween *scaleAction;
            scaleAction = CCActionTween::create(dt, "zoomScale", m_pContainer->getScale(), s);
            this->runAction(scaleAction);
        }
    }
    else
    {
        this->setZoomScale(s);
    }
}

void CCScrollView::setViewSize(CCSize size)
{
    m_tViewSize = size;
    CCLayer::setContentSize(size);
}

CCNode * CCScrollView::getContainer()
{
    return this->m_pContainer;
}

void CCScrollView::setContainer(CCNode * pContainer)
{
    this->removeAllChildrenWithCleanup(true);

    if (!pContainer) return;

    this->m_pContainer = pContainer;

    this->m_pContainer->ignoreAnchorPointForPosition(false);
    this->m_pContainer->setAnchorPoint(ccp(0.0f, 0.0f));

    this->addChild(this->m_pContainer);

    this->setViewSize(this->m_tViewSize);
}

void CCScrollView::relocateContainer(bool animated)
{
    CCPoint oldPoint, min, max;
    float newX, newY;
    
    min = this->minContainerOffset();
    max = this->maxContainerOffset();
    
    oldPoint = m_pContainer->getPosition();

    newX     = oldPoint.x;
    newY     = oldPoint.y;
    if (m_eDirection == kCCScrollViewDirectionBoth || m_eDirection == kCCScrollViewDirectionHorizontal)
    {
        newX     = MAX(newX, min.x);
        newX     = MIN(newX, max.x);
    }

    if (m_eDirection == kCCScrollViewDirectionBoth || m_eDirection == kCCScrollViewDirectionVertical)
    {
        newY     = MIN(newY, max.y);
        newY     = MAX(newY, min.y);
    }

    if (newY != oldPoint.y || newX != oldPoint.x)
    {
        this->setContentOffset(ccp(newX, newY), animated);
    }
}

CCPoint CCScrollView::maxContainerOffset()
{
    return ccp(0.0f, 0.0f);
}

CCPoint CCScrollView::minContainerOffset()
{
    return ccp(m_tViewSize.width - m_pContainer->getContentSize().width*m_pContainer->getScaleX(), 
               m_tViewSize.height - m_pContainer->getContentSize().height*m_pContainer->getScaleY());
}

void CCScrollView::deaccelerateScrolling(float dt)
{
    // #HLP_BEGIN
    if (mIsPagingEnabled)
        return;
    // #HLP_END
    
    
    
    if (m_bDragging)
    {
        this->unschedule(schedule_selector(CCScrollView::deaccelerateScrolling));
        return;
    }
    
    float newX, newY;
    CCPoint maxInset, minInset;
    
    m_pContainer->setPosition(ccpAdd(m_pContainer->getPosition(), m_tScrollDistance));
    
    if (m_bBounceable)
    {
        maxInset = m_fMaxInset;
        minInset = m_fMinInset;
    }
    else
    {
        maxInset = this->maxContainerOffset();
        minInset = this->minContainerOffset();
    }
    
    //check to see if offset lies within the inset bounds
    newX     = MIN(m_pContainer->getPosition().x, maxInset.x);
    newX     = MAX(newX, minInset.x);
    newY     = MIN(m_pContainer->getPosition().y, maxInset.y);
    newY     = MAX(newY, minInset.y);
    
    newX = m_pContainer->getPosition().x;
    newY = m_pContainer->getPosition().y;
    
    m_tScrollDistance     = ccpSub(m_tScrollDistance, ccp(newX - m_pContainer->getPosition().x, newY - m_pContainer->getPosition().y));
    m_tScrollDistance     = ccpMult(m_tScrollDistance, SCROLL_DEACCEL_RATE);
    this->setContentOffset(ccp(newX,newY));
    
    if ((fabsf(m_tScrollDistance.x) <= SCROLL_DEACCEL_DIST &&
         fabsf(m_tScrollDistance.y) <= SCROLL_DEACCEL_DIST) ||
        newY > maxInset.y || newY < minInset.y ||
        newX > maxInset.x || newX < minInset.x ||
        newX == maxInset.x || newX == minInset.x ||
        newY == maxInset.y || newY == minInset.y)
    {
        this->unschedule(schedule_selector(CCScrollView::deaccelerateScrolling));
        this->relocateContainer(true);
    }
}

void CCScrollView::stoppedAnimatedScroll(CCNode * node)
{
    this->unschedule(schedule_selector(CCScrollView::performedAnimatedScroll));
    // After the animation stopped, "scrollViewDidScroll" should be invoked, this could fix the bug of lack of tableview cells.
    if (m_pDelegate != NULL)
    {
        m_pDelegate->scrollViewDidScroll(this);
        // #HLP_BEGIN
        m_pDelegate->scrollViewDidStoppedAnimatedScroll(this);
        // #HLP_END
    }
}

void CCScrollView::performedAnimatedScroll(float dt)
{
    if (m_bDragging)
    {
        this->unschedule(schedule_selector(CCScrollView::performedAnimatedScroll));
        return;
    }

    if (m_pDelegate != NULL)
    {
        m_pDelegate->scrollViewDidScroll(this);
    }
}


const CCSize& CCScrollView::getContentSize()
{
	return m_pContainer->getContentSize();
}

void CCScrollView::setContentSize(const CCSize & size)
{
    if (this->getContainer() != NULL)
    {
        this->getContainer()->setContentSize(size);
		this->updateInset();
        
        // #HLP_BEGIN
        if (mIsPagingEnabled) {
            setupPagingData();
        }
        // #HLP_END
        
        
    }
}

void CCScrollView::updateInset()
{
	if (this->getContainer() != NULL)
	{
		m_fMaxInset = this->maxContainerOffset();
		m_fMaxInset = ccp(m_fMaxInset.x + m_tViewSize.width * INSET_RATIO,
			m_fMaxInset.y + m_tViewSize.height * INSET_RATIO);
		m_fMinInset = this->minContainerOffset();
		m_fMinInset = ccp(m_fMinInset.x - m_tViewSize.width * INSET_RATIO,
			m_fMinInset.y - m_tViewSize.height * INSET_RATIO);
	}
}

/**
 * make sure all children go to the container
 */
void CCScrollView::addChild(CCNode * child, int zOrder, int tag)
{
    child->ignoreAnchorPointForPosition(false);
    child->setAnchorPoint(ccp(0.0f, 0.0f));
    if (m_pContainer != child) {
        m_pContainer->addChild(child, zOrder, tag);
    } else {
        CCLayer::addChild(child, zOrder, tag);
    }
}

void CCScrollView::addChild(CCNode * child, int zOrder)
{
    this->addChild(child, zOrder, child->getTag());
}

void CCScrollView::addChild(CCNode * child)
{
    this->addChild(child, child->getZOrder(), child->getTag());
}

/**
 * clip this view so that outside of the visible bounds can be hidden.
 */
void CCScrollView::beforeDraw()
{
    if (m_bClippingToBounds)
    {
		CCRect frame = getViewRect();
        
        glEnable(GL_SCISSOR_TEST);
        
        CCEGLView::sharedOpenGLView()->setScissorInPoints(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
    }
}

/**
 * retract what's done in beforeDraw so that there's no side effect to
 * other nodes.
 */
void CCScrollView::afterDraw()
{
    if (m_bClippingToBounds)
    {
        glDisable(GL_SCISSOR_TEST);
    }
}

void CCScrollView::visit()
{
	// quick return if not visible
	if (!isVisible())
    {
		return;
    }

	kmGLPushMatrix();
	
    if (m_pGrid && m_pGrid->isActive())
    {
        m_pGrid->beforeDraw();
        this->transformAncestors();
    }

	this->transform();
    this->beforeDraw();

	if(m_pChildren)
    {
		ccArray *arrayData = m_pChildren->data;
		unsigned int i=0;
		
		// draw children zOrder < 0
		for( ; i < arrayData->num; i++ )
        {
			CCNode *child =  (CCNode*)arrayData->arr[i];
			if ( child->getZOrder() < 0 )
            {
				child->visit();
			}
            else
            {
				break;
            }
		}
		
		// this draw
		this->draw();
		
		// draw children zOrder >= 0
		for( ; i < arrayData->num; i++ )
        {
			CCNode* child = (CCNode*)arrayData->arr[i];
			child->visit();
		}
        
	}
    else
    {
		this->draw();
    }

    this->afterDraw();
	if ( m_pGrid && m_pGrid->isActive())
    {
		m_pGrid->afterDraw(this);
    }

	kmGLPopMatrix();
}

bool CCScrollView::ccTouchBegan(CCTouch* touch, CCEvent* event)
{
    if (!this->isVisible())
    {
        return false;
    }
    
    CCRect frame = getViewRect();

    //dispatcher does not know about clipping. reject touches outside visible bounds.
    if (m_pTouches->count() > 2 ||
        m_bTouchMoved          ||
        !frame.containsPoint(m_pContainer->convertToWorldSpace(m_pContainer->convertTouchToNodeSpace(touch))))
    {
        return false;
    }

    if (!m_pTouches->containsObject(touch))
    {
        m_pTouches->addObject(touch);
    }

    if (m_pTouches->count() == 1)
    { // scrolling
        m_tTouchPoint     = this->convertTouchToNodeSpace(touch);
        m_bTouchMoved     = false;
        m_bDragging     = true; //dragging started
        m_tScrollDistance = ccp(0.0f, 0.0f);
        m_fTouchLength    = 0.0f;
    }
    else if (m_pTouches->count() == 2)
    {
        m_tTouchPoint  = ccpMidpoint(this->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(0)),
                                   this->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(1)));
        m_fTouchLength = ccpDistance(m_pContainer->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(0)),
                                   m_pContainer->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(1)));
        m_bDragging  = false;
    } 
    return true;
}

void CCScrollView::ccTouchMoved(CCTouch* touch, CCEvent* event)
{
    if (!this->isVisible())
    {
        return;
    }

    if (m_pTouches->containsObject(touch))
    {
        if (m_pTouches->count() == 1 && m_bDragging)
        { // scrolling
            CCPoint moveDistance, newPoint, maxInset, minInset;
            CCRect  frame;
            float newX, newY;
            
            frame = getViewRect();

            newPoint     = this->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(0));
            moveDistance = ccpSub(newPoint, m_tTouchPoint);
            
            float dis = 0.0f;
            if (m_eDirection == kCCScrollViewDirectionVertical)
            {
                // #HLP_BEGIN
                if(!m_bDidVertical && (m_bDisableVertical || fabsf(moveDistance.x) > fabsf(moveDistance.y))){
                    m_bDisableVertical = true;
                    return;
                }
                m_bDidVertical = true;
                // #HLP_END
                dis = moveDistance.y;
            }
            else if (m_eDirection == kCCScrollViewDirectionHorizontal)
            {
                // #HLP_BEGIN
                if(!m_bDidHorizontal && (m_bDisableHorizontal || fabsf(moveDistance.y) > fabsf(moveDistance.x))){
                    m_bDisableHorizontal = true;
                    return;
                }
                m_bDidHorizontal = true;
                // #HLP_END
                dis = moveDistance.x;
            }
            else
            {
                dis = sqrtf(moveDistance.x*moveDistance.x + moveDistance.y*moveDistance.y);
            }

            if (!m_bTouchMoved && fabs(convertDistanceFromPointToInch(dis)) < MOVE_INCH )
            {
                //CCLOG("Invalid movement, distance = [%f, %f], disInch = %f", moveDistance.x, moveDistance.y);
                return;
            }
            
            if (!m_bTouchMoved)
            {
                moveDistance = CCPointZero;
            }
            
            m_tTouchPoint = newPoint;
            m_bTouchMoved = true;
            
            if (frame.containsPoint(this->convertToWorldSpace(newPoint)))
            {
                switch (m_eDirection)
                {
                    case kCCScrollViewDirectionVertical:
                        moveDistance = ccp(0.0f, moveDistance.y);
                        break;
                    case kCCScrollViewDirectionHorizontal:
                        moveDistance = ccp(moveDistance.x, 0.0f);
                        break;
                    default:
                        break;
                }
                
                maxInset = m_fMaxInset;
                minInset = m_fMinInset;

                newX     = m_pContainer->getPosition().x + moveDistance.x;
                newY     = m_pContainer->getPosition().y + moveDistance.y;

                m_tScrollDistance = moveDistance;
                this->setContentOffset(ccp(newX, newY));
            }
        }
        else if (m_pTouches->count() == 2 && !m_bDragging)
        {
            const float len = ccpDistance(m_pContainer->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(0)),
                                            m_pContainer->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(1)));
            this->setZoomScale(this->getZoomScale()*len/m_fTouchLength);
        }
    }
}

// #HLP_BEGIN
#define SCROOL_TO_TIMER 0.15f
#define SECTION_PER_SCREEN_WIDTH_TO_SCROLL 5
// #HLP_END


void CCScrollView::ccTouchEnded(CCTouch* touch, CCEvent* event)
{
    if (!this->isVisible())
    {
        return;
    }
    if (m_pTouches->containsObject(touch))
    {
        if (m_pTouches->count() == 1 && m_bTouchMoved)
        {
            this->schedule(schedule_selector(CCScrollView::deaccelerateScrolling));
        }
        m_pTouches->removeObject(touch);
    } 

    if (m_pTouches->count() == 0)
    {
        m_bDragging = false;
        m_bTouchMoved = false;
        
        // #HLP_BEGIN
        m_bDisableVertical = false;
        m_bDisableHorizontal = false;
        m_bDidVertical = false;
        m_bDidHorizontal = false;
        // #HLP_END
    }
    
    
    // #HLP_BEGIN
    pageScroll();
    // #HLP_END
    
    
    
    
}

void CCScrollView::ccTouchCancelled(CCTouch* touch, CCEvent* event)
{
    if (!this->isVisible())
    {
        return;
    }
    m_pTouches->removeObject(touch); 
    if (m_pTouches->count() == 0)
    {
        m_bDragging = false;    
        m_bTouchMoved = false;
    }
}

CCRect CCScrollView::getViewRect()
{
    CCPoint screenPos = this->convertToWorldSpace(CCPointZero);
    
    float scaleX = this->getScaleX();
    float scaleY = this->getScaleY();
    
    for (CCNode *p = m_pParent; p != NULL; p = p->getParent()) {
        scaleX *= p->getScaleX();
        scaleY *= p->getScaleY();
    }
    
    return CCRectMake(screenPos.x, screenPos.y, m_tViewSize.width*scaleX, m_tViewSize.height*scaleY);
}



// #HLP_BEGIN
void CCScrollView::moveToPage(int page)
{
    this->setContentOffset(ccp(-((page-1)*scrollWidth),0), true);
    currentScreen = page;
}

void CCScrollView::moveToNextPage()
{
    this->setContentOffset(ccp(-(((currentScreen+1)-1)*scrollWidth),0), true);
	currentScreen = currentScreen+1;
}

void CCScrollView::moveToPreviousPage()
{
    this->setContentOffset(ccp(-(((currentScreen-1)-1)*scrollWidth),0), true);
	currentScreen = currentScreen-1;
}

void CCScrollView::setPagingEnabled(bool pagingEnabled) {
    
    mIsPagingEnabled = pagingEnabled;
    
    // Set up the starting variables
    currentScreen = 1;
}

void CCScrollView::setupPagingData() {
    // offset added to show preview of next/previous screens
    CCSize s = CCDirector::sharedDirector()->getWinSize();
    
    scrollWidth  = s.width;
    scrollHeight = s.height;
    
    int tableWidthContent = this->getContentSize().width;
    totalScreens = ceil((float)tableWidthContent / scrollWidth);
    
    //CCLOG(" table's cell content width %d, all screen = %d", tableWidthContent, totalScreens);
}

void CCScrollView::pageScroll() {
    if (mIsPagingEnabled) {
        float sectionPerScreenWidthToScroll = scrollWidth / SECTION_PER_SCREEN_WIDTH_TO_SCROLL;
        float contentOffset = this->getContentOffset().x;
        //        int index = this->__indexFromOffset(this->getContentOffset());
        
        float currentOffSet  = contentOffset - (float)(scrollWidth * - (currentScreen - 1));
        
        //CCLog("offset = %f", currentOffSet);
        
        if ( currentOffSet < -sectionPerScreenWidthToScroll && (currentScreen+1) <= totalScreens ) {
            this->moveToNextPage();
        } else if (currentOffSet > sectionPerScreenWidthToScroll && (currentScreen-1) > 0 ) {
            this->moveToPreviousPage();
        } else {
            this->moveToPage(currentScreen);
        }
    }
}

// #HLP_END




NS_CC_EXT_END
