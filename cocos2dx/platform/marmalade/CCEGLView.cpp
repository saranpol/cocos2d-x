/****************************************************************************
 Copyright (c) 2011      cocos2d-x.org   http://cocos2d-x.org
 Copyright (c) 2011      Максим Аксенов
 Copyright (c) 2011      Giovanni Zito, Francis Styck

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

#include "CCEGLView.h"
#include "IwGL.h"
#include "CCApplication.h"
#include "CCDirector.h"
#include "CCSet.h"
#include "CCTouch.h"
#include "CCTouchDispatcher.h"
#include "CCKeypadDispatcher.h"
#include "CCIMEDispatcher.h"
#include "ccMacros.h"
#include <stdlib.h>
#include <s3eOSReadString.h>
// #HLP_BEGIN
//#include "sStream"
#include "AndroidKeyboard.h"
// #HLP_END

NS_CC_BEGIN

CCEGLView* CCEGLView::m_pInstance = 0;










// #HLP_BEGIN

static int _calcCharCount(const char * pszText, int &lastCharIndex)
{
    int n = 0;
    char ch = 0;
    int currentByte = 0;
    while ((ch = *pszText))
    {
        CC_BREAK_IF(! ch);
        
        // last byte of char
        if (0x80 != (0xC0 & ch))
        {
            ++n;
            lastCharIndex = currentByte;
        }
        ++pszText;
        currentByte++;
    }    
    return n;
}

#include "s3eIME.h"
#define KEYBOARD_BUFFER_LENTH 16384
static char g_KeyboardBuffer[KEYBOARD_BUFFER_LENTH];
//static char g_KeyboardBufferOld[KEYBOARD_BUFFER_LENTH];

void revertOldBuffer(const char *s) {
    /*
    if(s3eIMEAvailable() && s){
        strncpy(g_KeyboardBufferOld, s, KEYBOARD_BUFFER_LENTH);
        s3eIMESetBuffer(g_KeyboardBufferOld);
    }
    */
    if(s3eIMEAvailable() && s){
        strncpy(g_KeyboardBuffer, s, KEYBOARD_BUFFER_LENTH);
        s3eIMESetBuffer(g_KeyboardBuffer);
    }
    
    
}


static int32 BufferChanged(void*, void*) {
    //const int pos = s3eIMEGetInt(S3E_IME_CURSOR_POS);
    const int len = s3eIMEGetInt(S3E_IME_BUFFER_LEN);
    //const int sel = s3eIMEGetInt(S3E_IME_SELECTION_END);
    s3eIMEGetBuffer(g_KeyboardBuffer, sizeof(g_KeyboardBuffer));
    
//    int newLastCharIndex = 0;
//    int oldLastCharIndex = 0;
//    int newLen = _calcCharCount(g_KeyboardBuffer, newLastCharIndex);
//    int oldLen = _calcCharCount(g_KeyboardBufferOld, oldLastCharIndex);
    
//    if(newLen < oldLen){
//        g_KeyboardBufferOld[oldLastCharIndex] = 0;
//        s3eIMESetBuffer(g_KeyboardBufferOld);
//    }else{
//        strncpy(g_KeyboardBufferOld, g_KeyboardBuffer, KEYBOARD_BUFFER_LENTH);
//    }
    
    
    
    
    
    CCIMEDispatcher::sharedDispatcher()->dispatchUpdateText(g_KeyboardBuffer, len);
//    CCIMEDispatcher::sharedDispatcher()->dispatchUpdateText(g_KeyboardBufferOld, len);
    
    return 0;
}

static int32 handler_s3eIME(void* sys, void*) {
    s3eKeyboardEvent* event = (s3eKeyboardEvent*)sys;

    int key = event->m_Key;
    int keyPress = event->m_Pressed;
    CCIMEDispatcher::sharedDispatcher()->dispatchClickKeyboardButton(key, keyPress);
    
    /*
    std::stringstream sstr;
    sstr << event->m_Key;
    std::string str1 = sstr.str();
    s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, str1.c_str());
    */
//    if (event->m_Key == 4) {
//        //s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "Search !!!!!");
//        CCIMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
//    }
    
//    
//    if (event->m_Pressed) { // Down
//
//    }else{ // Up
//        if(event->m_Key == s3eKeyBackspace){
//
//            //CCIMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
//        }
//        
//        
//        
//    }
    return 0;
}
// #HLP_END










CCEGLView::CCEGLView()
: m_bCaptured(false)
, m_bAccelState(false)
, m_Key(s3eKeyFirst)
// #HLP_BEGIN
, mIsKeyboardShow(false)
, mIsSearchKeyboard(false)
// #HLP_END

{
	IW_CALLSTACK("CCEGLView::CCEGLView");
	
	IwGLInit();

	setFrameSize((float)IwGLGetInt(IW_GL_WIDTH), (float)IwGLGetInt(IW_GL_HEIGHT));

    // Determine if the device supports multi-touch
    m_isMultiTouch = s3ePointerGetInt(S3E_POINTER_MULTI_TOUCH_AVAILABLE) ? true : false;
    
	// For multi-touch devices we handle touch and motion events using different callbacks
    if (m_isMultiTouch)
    {
        s3ePointerRegister(S3E_POINTER_TOUCH_EVENT, &MultiTouchEventHandler, this);
        s3ePointerRegister(S3E_POINTER_TOUCH_MOTION_EVENT, &MultiMotionEventHandler, this);
    }
    else
    {        
        // Register pointer touch button event handler
        s3ePointerRegister(S3E_POINTER_BUTTON_EVENT, &TouchEventHandler, this);
        
        // Register pointer motion button event handler
        s3ePointerRegister(S3E_POINTER_MOTION_EVENT, &MotionEventHandler, this);
    }
    
    // #HLP_BEGIN
    // Register keyboard event handler
    if(s3eIMEAvailable()){ // Android s3eIME 
        s3eIMERegister(S3E_IME_BUFFER_CHANGED, BufferChanged, NULL);
        s3eIMERegister(S3E_IME_KEY_EVENT, handler_s3eIME, NULL);
    }else{ // Normal and iOS
        s3eKeyboardRegister(S3E_KEYBOARD_KEY_EVENT, &KeyEventHandler, this);
        s3eKeyboardRegister(S3E_KEYBOARD_CHAR_EVENT, &CharEventHandler, this);
    }
    // #HLP_END
}

CCEGLView::~CCEGLView()
{
	IW_CALLSTACK("CCEGLView::~CCEGLView");
}
	
void CCEGLView::setTouch(void* systemData)
{
	s3ePointerEvent* event =(s3ePointerEvent*)systemData;
    int id = 0;
    float x = (float)event->m_x;
    float y = (float)event->m_y;
	switch (event->m_Pressed)
	{
	case S3E_POINTER_STATE_DOWN :
		m_bCaptured = true;
        handleTouchesBegin(1, &id, &x, &y);
		break;
	case S3E_POINTER_STATE_UP :
		if (m_bCaptured)
		{
			handleTouchesEnd(1, &id, &x, &y);
			m_bCaptured = false;
		}
		break;
	}
}

void CCEGLView::setMotionTouch(void* systemData)
{
		s3ePointerMotionEvent* event =(s3ePointerMotionEvent*)systemData;
		if (m_bCaptured)
		{
            int id = 0;
            float x = (float)event->m_x;
            float y = (float)event->m_y;
            handleTouchesMove(1, &id, &x, &y);
		}
}

void CCEGLView::setMultiTouch(void* systemData)
{
	s3ePointerTouchEvent* event =(s3ePointerTouchEvent*)systemData;
    int id = (int)event->m_TouchID;
    float x = (float)event->m_x;
    float y = (float)event->m_y;

	switch (event->m_Pressed)
	{
        case S3E_POINTER_STATE_DOWN:
            {
                handleTouchesBegin(1, &id, &x, &y);
            }
            break;
            
        case S3E_POINTER_STATE_UP:
            {
                handleTouchesEnd(1, &id, &x, &y);
            }
            break;
	}
}

void CCEGLView::setMultiMotionTouch(void* systemData)
{
    s3ePointerTouchMotionEvent* event =(s3ePointerTouchMotionEvent*)systemData;
    int id = (int)event->m_TouchID;
    float x = (float)event->m_x;
    float y = (float)event->m_y;
    handleTouchesMove(1, &id, &x, &y);
}


void CCEGLView::setKeyTouch(void* systemData)
{
    // #HLP_BEGIN
 	s3eKeyboardEvent* event = (s3eKeyboardEvent*)systemData;
 	if (event->m_Pressed)
 	{
 		if (event->m_Key!=m_Key)
 		{
// 			CCKeypadDispatcher::sharedDispatcher()->dispatchKeypadMSG(kTypeMenuClicked);
 		}
 		else
 		{
// 			CCKeypadDispatcher::sharedDispatcher()->dispatchKeypadMSG(kTypeBackClicked);
 		}
 		m_Key =event->m_Key;
 	}
    // #HLP_END
}

void CCEGLView::setCharTouch( void* systemData )
{
    // #HLP_BEGIN
    s3eKeyboardCharEvent* event = (s3eKeyboardCharEvent*)systemData;
 	s3eWChar c = event->m_Char ;
    //CCLog("s3eWChar %d", c);
    
    if(c == 127 || c == 8){
        CCIMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
    }else{
        CCIMEDispatcher::sharedDispatcher()->dispatchInsertText((const char *)&c, 1);
    }
    // #HLP_END
}

bool CCEGLView::isOpenGLReady()
{
    return (IwGLIsInitialised());
}

void CCEGLView::end()
{
	IW_CALLSTACK("CCEGLView::end");

    if (m_isMultiTouch)
    {
        s3ePointerUnRegister(S3E_POINTER_TOUCH_EVENT, &MultiTouchEventHandler);
        s3ePointerUnRegister(S3E_POINTER_TOUCH_MOTION_EVENT, &MultiMotionEventHandler);
    }
    else
    {
        s3ePointerUnRegister(S3E_POINTER_BUTTON_EVENT, &TouchEventHandler);
        s3ePointerUnRegister(S3E_POINTER_MOTION_EVENT, &MotionEventHandler);
    }
    
    // #HLP_BEGIN
    if(s3eIMEAvailable()){ // Android s3eIME
        if(s3eIMEAvailable())
            s3eIMEUnRegister(S3E_IME_BUFFER_CHANGED, BufferChanged);
            s3eIMEUnRegister(S3E_IME_KEY_EVENT, handler_s3eIME);
    }else{ // Normal and iOS
        s3eKeyboardUnRegister(S3E_KEYBOARD_KEY_EVENT, &KeyEventHandler);
        s3eKeyboardUnRegister(S3E_KEYBOARD_KEY_EVENT, &CharEventHandler);
    }
    // #HLP_END

	if (IwGLIsInitialised())
  		IwGLTerminate();

	 s3eDeviceRequestQuit() ;

	 delete this;
}

void CCEGLView::swapBuffers()
{
	IW_CALLSTACK("CCEGLView::swapBuffers(");
	IwGLSwapBuffers();
}


void CCEGLView::setIMEKeyboardState(bool bOpen)
{
//	if(bOpen && s3eOSReadStringAvailable() == S3E_TRUE) {
//		const char* inputText = s3eOSReadStringUTF8("") ;
//        //const char* inputText = s3eOSReadStringUTF8WithDefault("","") ;
//        //const char* inputText = s3eExtOSReadUserStringUTF8("") ;
//
//		if( inputText!=0 ) {
//			CCIMEDispatcher::sharedDispatcher()->dispatchInsertText(inputText, strlen(inputText));
//		}
//	}
    
    // #HLP_BEGIN
    if(s3eIMEAvailable()){
        if(bOpen) {
            mIsKeyboardShow = true;
            s3eIMESetInt(S3E_IME_BYPASS, 0);
            if (mIsSearchKeyboard)
                s3eIMEStartSearchSession();
            else
                s3eIMEStartSession();
//            g_KeyboardBufferOld[0] = 0;
        } else {
            if (mIsKeyboardShow) {
                s3eIMESetInt( S3E_IME_BYPASS, 1);
                s3eIMEEndSession();
                mIsKeyboardShow = false;
                hideAndroidKeyboard();
            }
        }
//        else
//            s3eIMEEndSession();
    }else{
        s3eKeyboardSetInt(S3E_KEYBOARD_GET_CHAR, bOpen);
    }
//        int newCharState = s3eKeyboardGetInt(S3E_KEYBOARD_GET_CHAR);
//        newCharState = !newCharState;
    
    // #HLP_END
    
}

CCEGLView* CCEGLView::sharedOpenGLView()		// MH: Cocos2D now uses pointer instead of ref
{
	if( !m_pInstance ) {
		m_pInstance = new CCEGLView() ;
	}
	return m_pInstance;							// MH: Cocos2D now uses pointer instead of ref
}

NS_CC_END
