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
#define BOUNCE_DURATION      0.8f
#define PAGE_DURATION        0.25f
#define REFRESH_LABEL_Y 30
#define REFRESH_LABEL_MAX_Y 350
#define REFRESH_SHOW_Y 70
#define REFRESH_START_Y 100
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
, mPagingEnabled(false)
, mIsRefreshing(false)
, mIsDone(true)
, mRefreshEnabled(false)
, m_bDisableVertical(false)
, m_bDisableHorizontal(false)
, m_bDidVertical(false)
, m_bDidHorizontal(false)
, mForcePageWidth(0)
, mForcePageHeight(0)
, mPageMarginX(0)
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
        //this->setContentOffsetInDuration(offset, BOUNCE_DURATION);
        // #HLP_BEGIN
        if(mPagingEnabled)
            this->setContentOffsetInDuration(offset, PAGE_DURATION);
        else
            this->setContentOffsetInDuration(offset, BOUNCE_DURATION);
        // #HLP_END
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
        
        // #HLP_BEGIN
        if(mRefreshEnabled){
            updateRefreshUI();
        }
        // #HLP_END
    }
}

void CCScrollView::setContentOffsetInDuration(CCPoint offset, float dt)
{
    CCFiniteTimeAction *scroll, *expire;
    
    scroll = CCMoveTo::create(dt, offset);
    // #HLP_BEGIN
    m_pContainer->stopAllActions();
    //scroll = CCEaseOut::create((CCActionInterval*)scroll, 10);
    //scroll = CCEaseElasticOut::create((CCActionInterval*)scroll);
    //scroll = CCEaseInOut::create((CCActionInterval*)scroll,3);
    if(mPagingEnabled)
        scroll = CCEaseSineOut::create((CCActionInterval*)scroll);
    else
        scroll = CCEaseExponentialOut::create((CCActionInterval*)scroll);
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
//    return ccp(m_tViewSize.width - m_pContainer->getContentSize().width*m_pContainer->getScaleX(), 
//               m_tViewSize.height - m_pContainer->getContentSize().height*m_pContainer->getScaleY());
    // #HLP_BEGIN
    if(mIsRefreshing)
        return ccp(m_tViewSize.width - m_pContainer->getContentSize().width*m_pContainer->getScaleX(),
                   m_tViewSize.height - m_pContainer->getContentSize().height*m_pContainer->getScaleY() - REFRESH_SHOW_Y);

    return ccp(m_tViewSize.width - m_pContainer->getContentSize().width*m_pContainer->getScaleX(),
               m_tViewSize.height - m_pContainer->getContentSize().height*m_pContainer->getScaleY());
    // #HLP_END
}

// #HLP_BEGIN
bool CCScrollView::getSpringConstant(float &kX, float &kY){
    float x = m_pContainer->getPosition().x;
    float y = m_pContainer->getPosition().y;
    
    const CCPoint minOffset = this->minContainerOffset();
    const CCPoint maxOffset = this->maxContainerOffset();
    
    CCPoint offset;
    offset.x = MAX(minOffset.x, MIN(maxOffset.x, x));
    offset.y = MAX(minOffset.y, MIN(maxOffset.y, y));
    
    bool hitSpring = false;
    
    if(y != offset.y){
        hitSpring = true;
        float diff = fabsf(y - offset.y);
        if(diff > 1.0f)
            kY = 1.0f / diff * 30.0f;
        if(kY > 1.0f)
            kY = 1.0f;
    }
    
    if(x != offset.x){
        hitSpring = true;
        float diff = fabsf(x - offset.x);
        if(diff > 1.0f)
            kX = 1.0f / diff * 30.0f;
        if(kX > 1.0f)
            kX = 1.0f;
    }
    
    return hitSpring;
}

// #HLP_END



void CCScrollView::deaccelerateScrolling(float dt)
{
    // #HLP_BEGIN
    if (mPagingEnabled)
        return;
    // #HLP_END
    
    
    
    if (m_bDragging)
    {
        this->unschedule(schedule_selector(CCScrollView::deaccelerateScrolling));
        return;
    }
    
    float newX, newY;
    CCPoint maxInset, minInset;
    
    
    
    
    // #HLP_BEGIN
    // make scrollview scroll less when at the edge of deaccelerate
    float kX = 1.0f;
    float kY = 1.0f;
    float deaccel_dist = SCROLL_DEACCEL_DIST;
    if(getSpringConstant(kX, kY))
        deaccel_dist = 1.0f;
    
    m_tScrollDistance.x *= kX;
    m_tScrollDistance.y *= kY;
    // #HLP_END
    
    
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
    
//    if ((fabsf(m_tScrollDistance.x) <= SCROLL_DEACCEL_DIST &&
//         fabsf(m_tScrollDistance.y) <= SCROLL_DEACCEL_DIST) ||
    // #HLP_BEGIN
    if ((fabsf(m_tScrollDistance.x) <= deaccel_dist &&
         fabsf(m_tScrollDistance.y) <= deaccel_dist) ||    
    // #HLP_END
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
        if(mPagingEnabled){
            setupPagingData();
        }
        
        if(mRefreshEnabled){
            updateRefreshUI();
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
// #HLP_BEGIN
#define MAX_SCROLL_VIEW_IN_SCROLL_VIEW 8
static int sScissorTestCount = 0;
static CCRect sScissorRect[MAX_SCROLL_VIEW_IN_SCROLL_VIEW];
// #HLP_END
void CCScrollView::beforeDraw()
{
    if (m_bClippingToBounds)
    {
		CCRect frame = getViewRect();

        glEnable(GL_SCISSOR_TEST);

        // #HLP_BEGIN
        sScissorRect[sScissorTestCount] = frame;
        sScissorTestCount++;
        // Clip by parent rect
        for(int i=0; i<sScissorTestCount-1; i++){
            CCRect parent = sScissorRect[i];
            if(parent.origin.x > frame.origin.x){
                float diff = parent.origin.x - frame.origin.x;
                frame.origin.x = parent.origin.x;
                frame.size.width -= diff;
            }
            if(parent.origin.y > frame.origin.y){
                float diff = parent.origin.y - frame.origin.y;
                frame.origin.y = parent.origin.y;
                frame.size.height -= diff;
            }
            float frame_max_x = frame.origin.x+frame.size.width;
            float parent_max_x = parent.origin.x+parent.size.width;
            if(frame_max_x > parent_max_x){
                float diff = frame_max_x - parent_max_x;
                frame.size.width -= diff;
            }

            float frame_max_y = frame.origin.y+frame.size.height;
            float parent_max_y = parent.origin.y+parent.size.height;
            if(frame_max_y > parent_max_y){
                float diff = frame_max_y - parent_max_y;
                frame.size.height -= diff;
            }
        }
        // #HLP_END
        
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
        //glDisable(GL_SCISSOR_TEST);
        
        // #HLP_BEGIN
        sScissorTestCount--;
        if(sScissorTestCount == 0)
            glDisable(GL_SCISSOR_TEST);
        else{
            CCRect frame = sScissorRect[sScissorTestCount-1];
            CCEGLView::sharedOpenGLView()->setScissorInPoints(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
        }
        // #HLP_END
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

    // #HLP_BEGIN
    CCPoint start = touch->getStartLocation();
    CCPoint end = touch->getLocation();
    if(ccpDistance(start, end) > 10.0f){
        if (m_pDelegate != NULL)
            m_pDelegate->scrollViewDidScrollMove(this);
    }
    // #HLP_END
    
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

//                newX     = m_pContainer->getPosition().x + moveDistance.x;
//                newY     = m_pContainer->getPosition().y + moveDistance.y;

                // #HLP_BEGIN
                // make scrollview scroll less when at the edge when drag
                float kX = 1.0f;
                float kY = 1.0f;
                getSpringConstant(kX, kY);
                newX     = m_pContainer->getPosition().x + moveDistance.x*kX;
                newY     = m_pContainer->getPosition().y + moveDistance.y*kY;
                // #HLP_END
                
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
        
        // #HLP_BEGIN
        m_bDisableVertical = false;
        m_bDisableHorizontal = false;
        m_bDidVertical = false;
        m_bDidHorizontal = false;
        // #HLP_END
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
void CCScrollView::moveToPage(int page, bool animated)
{
    setContentOffset(ccp(-(page*mScrollWidth)+mPageMarginX,0), animated);
    mCurrentPage = page;
    //removeAllTouch();
}

void CCScrollView::moveToNextPage(bool animated)
{
    moveToPage(mCurrentPage+1, animated);
}

void CCScrollView::moveToPreviousPage(bool animated)
{
    moveToPage(mCurrentPage-1, animated);
}

void CCScrollView::setPagingEnabled(bool pagingEnabled) {
    
    mPagingEnabled = pagingEnabled;
    
    // Set up the starting variables
    mCurrentPage = 0;
}

void CCScrollView::updateRefreshUI() {
    CCSize scrollSize = getContentSize();
    if(scrollSize.height < REFRESH_LABEL_Y)
        return;
    float py = scrollSize.height+REFRESH_LABEL_Y;

    float y = m_pContainer->getPosition().y;
    const CCPoint minOffset = this->minContainerOffset();
    const CCPoint maxOffset = this->maxContainerOffset();
    CCPoint offset;
    offset.y = MAX(minOffset.y, MIN(maxOffset.y, y));
    float diff = offset.y - y;
    //CCLog("diff %f", diff);
    
    if(diff > REFRESH_START_Y){
        if(m_pDelegate && !mIsRefreshing && mIsDone){
            m_pDelegate->scrollViewDidRefresh(this);
        }
    }
    mLabelRefresh->setPosition(CCPointMake(scrollSize.width/2.0, py));
    mMenu->setPosition(CCPointMake(scrollSize.width - 30.0f, py));
}

void CCScrollView::setRefreshStart(){
    mIsRefreshing = true;
    mIsDone = false;
    mLabelRefresh->setString("Refreshing...");
}

void CCScrollView::setDoneFlag() {
    mIsDone = true;
}

void CCScrollView::setRefreshDone(){
    mIsRefreshing = false;
    mLabelRefresh->setString("Pull to refresh");
    scheduleOnce(schedule_selector(CCScrollView::setDoneFlag), 2.0f);
    relocateContainer(true);
}

void CCScrollView::setRefreshText(const char *text){
    mLabelRefresh->setString(text);
}

void CCScrollView::clickCancelRefresh(CCObject *sender) {
    if(m_pDelegate)
        m_pDelegate->scrollViewDidCancelRefresh(this);
}

void CCScrollView::setRefreshEnabled(bool refresh) {
    mRefreshEnabled = refresh;
    
    if(mRefreshEnabled){
        mLabelRefresh = CCLabelTTF::create("Pull to refresh", "ccbResources/Thonburi.ttf", 20);
        mLabelRefresh->setColor(ccc3(76, 76, 76));
        CCSize scrollSize = getContentSize();
        mLabelRefresh->setPosition(CCPointMake(scrollSize.width/2.0, scrollSize.height+REFRESH_LABEL_Y));
        addChild(mLabelRefresh);
        mLabelRefresh->setAnchorPoint(ccp(0.5f, 0.5f));
        
        CCMenuItem *item = CCMenuItemImage::create("button_close_refresh.png", "button_close_refresh_press.png", this, menu_selector(CCScrollView::clickCancelRefresh));
        mMenu = CCMenu::createWithItem(item);
        addChild(mMenu);
        mMenu->setPosition(CCPointMake(scrollSize.width - 30.0f, scrollSize.height+REFRESH_LABEL_Y));
    }
}

void CCScrollView::setupPagingData() {
    CCSize s = getViewSize();
    
    if(mForcePageWidth)
        mScrollWidth = mForcePageWidth;
    else
        mScrollWidth  = s.width;
    
    if(mForcePageHeight)
        mScrollHeight = mForcePageHeight;
    else
        mScrollHeight = s.height;
    
    int tableWidthContent = getContentSize().width;
    mTotalPage = ceil((float)tableWidthContent / mScrollWidth);
}

void CCScrollView::pageScroll() {
    if (mPagingEnabled) {
        float sectionPerScreenWidthToScroll = mScrollWidth / SECTION_PER_SCREEN_WIDTH_TO_SCROLL;
        float contentOffset = getContentOffset().x;
        //        int index = this->__indexFromOffset(this->getContentOffset());
        
        float currentOffSet  = contentOffset - (float)(mScrollWidth * - mCurrentPage) - mPageMarginX;
        
        //CCLog("offset = %f", currentOffSet);
        
        if ( currentOffSet < -sectionPerScreenWidthToScroll && (mCurrentPage+1) < mTotalPage ) {
            moveToNextPage(true);
        } else if (currentOffSet > sectionPerScreenWidthToScroll && (mCurrentPage-1) >= 0 ) {
            moveToPreviousPage(true);
        } else {
            moveToPage(mCurrentPage, true);
        }
    }
}

void CCScrollView::removeAllTouch() {
    m_pTouches->removeAllObjects();
    m_bDragging = false;
    m_bTouchMoved = false;
    m_bDisableVertical = false;
    m_bDisableHorizontal = false;
    m_bDidVertical = false;
    m_bDidHorizontal = false;
    mIsDone = true;
}

// #HLP_END




NS_CC_EXT_END
