/****************************************************************************
Copyright (c) 2010 cocos2d-x.org

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

#include "CCTextFieldTTF.h"

#include "CCDirector.h"
#include "CCEGLView.h"
// #HLP_BEGIN
#include "touch_dispatcher/CCTouchDispatcher.h"
#include "support/CCPointExtension.h"
#include "CCAction.h"
#include "CCActionInterval.h"
#include "CCActionEase.h"
#include "CCActionInstant.h"
#include "CCEGLView.h"
// #HLP_END

NS_CC_BEGIN

static int _calcCharCount(const char * pszText)
{
    int n = 0;
    char ch = 0;
    while ((ch = *pszText))
    {
        CC_BREAK_IF(! ch);

        if (0x80 != (0xC0 & ch))
        {
            ++n;
        }
        ++pszText;
    }
    return n;
}

//////////////////////////////////////////////////////////////////////////
// constructor and destructor
//////////////////////////////////////////////////////////////////////////

CCTextFieldTTF::CCTextFieldTTF()
: m_pDelegate(0)
, m_nCharCount(0)
, m_pInputText(new std::string)
, m_pPlaceHolder(new std::string)   // prevent CCLabelTTF initWithString assertion
// #HLP_BEGIN
, mIsTouchBegan(false)
, mIsPassword(false)
, mLayerCursor(NULL)
, mIsTextView(false)
, mEnableEnter(false)
, mIsSearchKeyboard(false)
// #HLP_END
, m_bSecureTextEntry(false)
{
    m_ColorSpaceHolder.r = m_ColorSpaceHolder.g = m_ColorSpaceHolder.b = 127;    
}

CCTextFieldTTF::~CCTextFieldTTF()
{
    CC_SAFE_DELETE(m_pInputText);
    CC_SAFE_DELETE(m_pPlaceHolder);
}

// #HLP_BEGIN

CCTextFieldTTF * CCTextFieldTTF::create()
{
    CCTextFieldTTF * pRet = new CCTextFieldTTF();
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


void CCTextFieldTTF::onEnter()
{
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, false);
    
    CCNode::onEnter();
    
//    // add this layer to concern the Accelerometer Sensor
//    if (m_bAccelerometerEnabled)
//    {
//        pDirector->getAccelerometer()->setDelegate(this);
//    }
//    
//    // add this layer to concern the keypad msg
//    if (m_bKeypadEnabled)
//    {
//        pDirector->getKeypadDispatcher()->addDelegate(this);
//    }
}

void CCTextFieldTTF::onExit()
{
    CCDirector* pDirector = CCDirector::sharedDirector();
    pDirector->getTouchDispatcher()->removeDelegate(this);
    
//    // remove this layer from the delegates who concern Accelerometer Sensor
//    if (m_bAccelerometerEnabled)
//    {
//        pDirector->getAccelerometer()->setDelegate(NULL);
//    }
//    
//    // remove this layer from the delegates who concern the keypad msg
//    if (m_bKeypadEnabled)
//    {
//        pDirector->getKeypadDispatcher()->removeDelegate(this);
//    }
    
    CCNode::onExit();
}

bool CCTextFieldTTF::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
    
    CCPoint touchLocation = pTouch->getLocation();

    CCNode *parent = (CCNode*)this;
    while(parent){
        if(!parent->isVisible()){
            mIsTouchBegan = false;
            return false;
        }
        
        CCPoint local = parent->convertToNodeSpace(touchLocation);
        CCRect r = parent->boundingBox();
        // special case get parent = CCTableViewCell size 0 we ignore it
        if(r.size.width == 0 || r.size.height == 0){
            parent = parent->getParent();
            continue;
        }
        r.origin = CCPointZero;
        if(!r.containsPoint(local)){
            mIsTouchBegan = false;
            return false;
        }
        parent = parent->getParent();
    }
    
    mIsTouchBegan = true;
    return true;
}

void CCTextFieldTTF::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) {
    CCPoint start = pTouch->getStartLocation();
    CCPoint end = pTouch->getLocation();
    if(ccpDistance(start, end) > 10.0f)
        mIsTouchBegan = false;
}

void CCTextFieldTTF::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) {
    if(mIsTouchBegan){
        attachWithIME();
    }
    mIsTouchBegan = false;
}


// #HLP_END

//////////////////////////////////////////////////////////////////////////
// static constructor
//////////////////////////////////////////////////////////////////////////

CCTextFieldTTF * CCTextFieldTTF::textFieldWithPlaceHolder(const char *placeholder, const CCSize& dimensions, CCTextAlignment alignment, const char *fontName, float fontSize)
{
    CCTextFieldTTF *pRet = new CCTextFieldTTF();
    if(pRet && pRet->initWithPlaceHolder("", dimensions, alignment, fontName, fontSize))
    {
        pRet->autorelease();
        if (placeholder)
        {
            pRet->setPlaceHolder(placeholder);
        }
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

CCTextFieldTTF * CCTextFieldTTF::textFieldWithPlaceHolder(const char *placeholder, const char *fontName, float fontSize)
{
    CCTextFieldTTF *pRet = new CCTextFieldTTF();
    if(pRet && pRet->initWithString("", fontName, fontSize))
    {
        pRet->autorelease();
        if (placeholder)
        {
            pRet->setPlaceHolder(placeholder);
        }
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

//////////////////////////////////////////////////////////////////////////
// initialize
//////////////////////////////////////////////////////////////////////////

bool CCTextFieldTTF::initWithPlaceHolder(const char *placeholder, const CCSize& dimensions, CCTextAlignment alignment, const char *fontName, float fontSize)
{
    if (placeholder)
    {
        CC_SAFE_DELETE(m_pPlaceHolder);
        m_pPlaceHolder = new std::string(placeholder);
    }
    return CCLabelTTF::initWithString(m_pPlaceHolder->c_str(), fontName, fontSize, dimensions, alignment);
}
bool CCTextFieldTTF::initWithPlaceHolder(const char *placeholder, const char *fontName, float fontSize)
{
    if (placeholder)
    {
        CC_SAFE_DELETE(m_pPlaceHolder);
        m_pPlaceHolder = new std::string(placeholder);
    }
    return CCLabelTTF::initWithString(m_pPlaceHolder->c_str(), fontName, fontSize);
}

//////////////////////////////////////////////////////////////////////////
// CCIMEDelegate
//////////////////////////////////////////////////////////////////////////

// #HLP_BEGIN

float CCTextFieldTTF::getCursorHeight() {
    return getFontSize()*1.10f;
}

void CCTextFieldTTF::updateCursor() {
    if(mLayerCursor){
        float x = getTexture()->mCursorX/CC_CONTENT_SCALE_FACTOR();
        if (!m_pInputText->length() && m_pPlaceHolder->length())
            x = getTexture()->mCursorXStart/CC_CONTENT_SCALE_FACTOR();
        float y = getTexture()->mCursorY/CC_CONTENT_SCALE_FACTOR();
        mLayerCursor->setPosition(CCPointMake(x, getContentSize().height - y + getCursorHeight()));
    }
}

void CCTextFieldTTF::animateCursor() {
    if(mLayerCursor){
        CCFiniteTimeAction *fade;
        if(mLayerCursor->getOpacity() == 255)
            fade = CCFadeTo::create(0.5f, 0);
        else
            fade = CCFadeTo::create(0.5f, 255);
        fade = CCEaseSineInOut::create((CCActionInterval*)fade);
        CCFiniteTimeAction *expire = CCCallFuncN::create(this, callfuncN_selector(CCTextFieldTTF::animateCursor));
        mLayerCursor->runAction(CCSequence::create(fade, expire, NULL));
    }
}

void CCTextFieldTTF::addCursor() {
    if(!mLayerCursor){
        mLayerCursor = CCLayerColor::create(ccc4(0, 0, 0, 255), 2.0f, getCursorHeight());
        addChild(mLayerCursor);
        updateCursor();
        animateCursor();
    }
}

void CCTextFieldTTF::removeCursor() {
    if(mLayerCursor){
        CCNode::removeChild(mLayerCursor);
        mLayerCursor = NULL;
    }
}

#include "s3eIME.h"

// #HLP_END

bool CCTextFieldTTF::attachWithIME()
{
    bool bRet = CCIMEDelegate::attachWithIME();
    if (bRet)
    {
        // open keyboard
        CCEGLView * pGlView = CCDirector::sharedDirector()->getOpenGLView();
        if (pGlView)
        {
            // #HLP_BEGIN
            if(s3eIMEAvailable()){
                //Initial Text
                s3eIMESetBuffer(m_pInputText->c_str());
            }
            pGlView->mIsSearchKeyboard = mIsSearchKeyboard;
            // #HLP_END
            pGlView->setIMEKeyboardState(true);
        }
        
        // #HLP_BEGIN
        addCursor();
        // #HLP_END
    }
    return bRet;
}

bool CCTextFieldTTF::detachWithIME()
{
    bool bRet = CCIMEDelegate::detachWithIME();
    if (bRet)
    {
        // close keyboard
        CCEGLView * pGlView = CCDirector::sharedDirector()->getOpenGLView();
        if (pGlView)
        {
            pGlView->setIMEKeyboardState(false);
            
            
//            // #HLP_BEGIN
//            if (!mIsTextView) 
//                pGlView->setIMEKeyboardState(false);
//            else
//                addCursor();
//            
//            // #HLP_END

        }
    }
    return bRet;
}

// #HLP_BEGIN
void CCTextFieldTTF::didDetachWithIME() {
    removeCursor();
}
// #HLP_END


bool CCTextFieldTTF::canAttachWithIME()
{
    return (m_pDelegate) ? (! m_pDelegate->onTextFieldAttachWithIME(this)) : true;
}

bool CCTextFieldTTF::canDetachWithIME()
{
    return (m_pDelegate) ? (! m_pDelegate->onTextFieldDetachWithIME(this)) : true;
}

#include <stdio.h>
// #HLP_BEGIN
// From s3eIME
void CCTextFieldTTF::updateText(const char * text){
    //bool mEnableEnter = true; // xxx
    
    bool hasEnter = false;
    
    // check last key is enter ?
//    CCString *xxx = CCString::createWithFormat("#####%s#####", text);
    //s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, text);
    if(!strcmp(&text[strlen(text)-1], "\n"))
        hasEnter = true;
    
    if(mEnableEnter && hasEnter){
        if(m_pDelegate){
            m_pDelegate->didEnterKey();
            //s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "did enter");
        }
    }else{
        

        if (m_pDelegate && m_pDelegate->onTextFieldInsertText(this, text, strlen(text)))
        {
            // delegate doesn't want to insert text
            revertOldBuffer(getString());
            return;
        }
        
        setString(text);
  
        // TODO ???????
//        if (m_pDelegate && m_pDelegate->onTextFieldInsertText(this, "\n", 1))
//        {
//            return;
//        }
        
    }
}
// #HLP_END



void CCTextFieldTTF::insertText(const char * text, int len)
{
    //std::string sInsert(text, len);
    // #HLP_BEGIN
    char buffer[4] = {0, 0, 0, 0};
    UCS2ToUTF8((ucs2char*)text, 1, buffer, 4);
    text = buffer;

    
    
//    s3eWChar *c = (s3eWChar*)text;
//    sprintf(buffer, "%d", *c);
//    text = buffer;
    

    
    std::string sInsert(text);
    // #HLP_END

    
    // #HLP_BEGIN
    // insert \n means input end
    int nPos = sInsert.find('\n');
    if ((int)sInsert.npos != nPos)
    {
        if (!mIsTextView) {
            len = nPos;
            sInsert.erase(nPos);
        }
    }
    // #HLP_END
    
//    // insert \n means input end
//    int nPos = sInsert.find('\n');
//    if ((int)sInsert.npos != nPos)
//    {
//        len = nPos;
//        sInsert.erase(nPos);
//    }
    
    if (len > 0)
    {
        int old_m_nCharCount = m_nCharCount;
        m_nCharCount += _calcCharCount(sInsert.c_str());
        std::string sText(*m_pInputText);
        sText.append(sInsert);
        
        if (m_pDelegate && m_pDelegate->onTextFieldInsertText(this, sText.c_str(), len))
        {
            // delegate doesn't want to insert text
            m_nCharCount = old_m_nCharCount;
            return;
        }

        setString(sText.c_str());
    }

    if ((int)sInsert.npos == nPos) {
        return;
    }

    // '\n' inserted, let delegate process first
    if (m_pDelegate && m_pDelegate->onTextFieldInsertText(this, "\n", 1))
    {
        return;
    }

    // if delegate hasn't processed, detach from IME by default
    if (!mIsTextView)
        detachWithIME();
}

void CCTextFieldTTF::deleteBackward()
{
    int nStrLen = m_pInputText->length();
    if (! nStrLen)
    {
        // there is no string
        return;
    }

    // get the delete byte number
    int nDeleteLen = 1;    // default, erase 1 byte

    while(0x80 == (0xC0 & m_pInputText->at(nStrLen - nDeleteLen)))
    {
        ++nDeleteLen;
    }

    if (m_pDelegate && m_pDelegate->onTextFieldDeleteBackward(this, m_pInputText->c_str() + nStrLen - nDeleteLen, nDeleteLen))
    {
        // delegate doesn't wan't to delete backwards
        return;
    }

    // if all text deleted, show placeholder string
    if (nStrLen <= nDeleteLen)
    {
        CC_SAFE_DELETE(m_pInputText);
        m_pInputText = new std::string;
        m_nCharCount = 0;
        CCLabelTTF::setString(m_pPlaceHolder->c_str());
        // #HLP_BEGIN
        if(m_pDelegate){
            m_pDelegate->textChanged(this);
        }
        updateCursor();
        // #HLP_END
        return;
    }

    // set new input text
    std::string sText(m_pInputText->c_str(), nStrLen - nDeleteLen);
    setString(sText.c_str());
}

const char * CCTextFieldTTF::getContentText()
{
    return m_pInputText->c_str();
}

void CCTextFieldTTF::draw()
{
    if (m_pDelegate && m_pDelegate->onDraw(this))
    {
        return;
    }
    if (m_pInputText->length())
    {
        CCLabelTTF::draw();
        return;
    }

    // draw placeholder
    ccColor3B color = getColor();
    setColor(m_ColorSpaceHolder);
    CCLabelTTF::draw();
    setColor(color);
}

const ccColor3B& CCTextFieldTTF::getColorSpaceHolder()
{
    return m_ColorSpaceHolder;
}

void CCTextFieldTTF::setColorSpaceHolder(const ccColor3B& color)
{
    m_ColorSpaceHolder = color;
}

//////////////////////////////////////////////////////////////////////////
// properties
//////////////////////////////////////////////////////////////////////////

// input text property
void CCTextFieldTTF::setString(const char *text)
{
    static char bulletString[] = {(char)0xe2, (char)0x80, (char)0xa2, (char)0x00};
    std::string displayText;
    int length;

    CC_SAFE_DELETE(m_pInputText);
    // #HLP_BEGIN
    //bool isMax = false;
    // #HLP_END
    
    if (text)
    {
        m_pInputText = new std::string(text);
        displayText = *m_pInputText;
        if (m_bSecureTextEntry)
        {
            displayText = "";
            length = m_pInputText->length();
            while (length)
            {
                displayText.append(bulletString);
                --length;
            }
        }
    }
    else
    {
        m_pInputText = new std::string;
    }

    // if there is no input text, display placeholder instead
    if (! m_pInputText->length())
    {
        CCLabelTTF::setString(m_pPlaceHolder->c_str());
    }
    else
    {
        //CCLabelTTF::setString(displayText.c_str());

        // #HLP_BEGIN
        if (mIsPassword)
        {
            int length = _calcCharCount(m_pInputText->c_str());
            std::string passwordText;
            char str[length*3];  //3 bytes for •
            for (int i = 0; i<length; i++) {
                if (i == 0) 
                    strcpy (str, "•");
                else
                    strcat (str, "•");
            }
            puts (str);
            passwordText = str;
            CCLabelTTF::setString(passwordText.c_str()); 
        } else
        {
            CCLabelTTF::setString(m_pInputText->c_str());
        }
        // #HLP_END
    }
    m_nCharCount = _calcCharCount(m_pInputText->c_str());
    
    
    // #HLP_BEGIN
    if(m_pDelegate){
        m_pDelegate->textChanged(this);
    }
    
    updateCursor();
    // #HLP_END
}

const char* CCTextFieldTTF::getString(void)
{
    return m_pInputText->c_str();
}

// place holder text property
void CCTextFieldTTF::setPlaceHolder(const char * text)
{
    CC_SAFE_DELETE(m_pPlaceHolder);
    m_pPlaceHolder = (text) ? new std::string(text) : new std::string;
    if (! m_pInputText->length())
    {
        CCLabelTTF::setString(m_pPlaceHolder->c_str());
    }
    
    
}

const char * CCTextFieldTTF::getPlaceHolder(void)
{
    return m_pPlaceHolder->c_str();
}

// secureTextEntry
void CCTextFieldTTF::setSecureTextEntry(bool value)
{
    if (m_bSecureTextEntry != value)
    {
        m_bSecureTextEntry = value;
        setString(getString());
    }
}

bool CCTextFieldTTF::isSecureTextEntry()
{
    return m_bSecureTextEntry;
}


// #HLP_BEGIN
bool isNumber(const char *s){
    for(unsigned int i=0; i<strlen(s); i++){
        if(!atoi(&s[i]) && s[i]!='0')
            return false;
    }
    return true;
}
// #HLP_END



NS_CC_END
