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
#include <vector>
#include <string>
#include <sstream> 
#include <algorithm>
#include "CCImage.h"
#include "CCCommon.h"
#include "CCStdC.h"
#include "CCFileUtils.h"
#include "s3eFile.h"
#include "IwUtil.h"
#include "png.h"
#include "ft2build.h"
#include "tiffio.h"
#include FT_FREETYPE_H 
//#define FONT_KERNING 2
// #HLP_BEGIN
#define FONT_KERNING 0
// #HLP_END
#define RSHIFT6(num) ((num)>>6)

#include <strings.h>


// #HLP_BEGIN
#include "ccMacros.h"
#include "CCDirector.h"
using namespace std;
// #HLP_END


extern "C"
{
#include <jpeglib.h>
}

//#include <string>

typedef struct 
{
    unsigned char* data;
    int size;
    int offset;
} tImageSource;

struct TextLine {
//	std::string sLineStr;
// #HLP_BEGIN
    wstring sLineStr;
// #HLP_END    
	int iLineWidth;
};

NS_CC_BEGIN;

class CC_DLL CCImageHelper
{
public:
	CCImageHelper();
	~CCImageHelper();
	// dummy funcs to help libjpeg
	static void JPEGInitSource(j_decompress_ptr cinfo)
	{
	}

	static boolean JPEGFillInputBuffer(j_decompress_ptr cinfo)
	{
		return 0;
	}

	static void JPEGSkipInputData(j_decompress_ptr cinfo, long num_bytes)
	{
		cinfo->src->next_input_byte += num_bytes;
		cinfo->src->bytes_in_buffer -= num_bytes;
	}

	static void JPEGTermSource(j_decompress_ptr cinfo)
	{
	}
};


class BitmapDC
{
public:
	BitmapDC();
	~BitmapDC();

	void reset();
    
    // #HLP_BEGIN
    FT_UInt thaiAdjust(FT_UInt current_index, FT_UInt prev_index, FT_UInt next_index, const char *fontName);
    bool canBreakThai(const wchar_t* pText);
    int mCursorX;
    int mCursorXStart;
    float mCursorY;
    unsigned int mMaxLine;
    // #HLP_END

    
	//bool getBitmap(const char *text, int nWidth, int nHeight, CCImage::ETextAlign eAlignMask, const char * pFontName, uint fontSize);
    // #HLP_BEGIN
    bool getBitmap(const char *text, int nWidth, int nHeight, CCImage::ETextAlign eAlignMask, const char * pFontName, uint fontSize, int fixLineHeight = 0);
    // #HLP_END

public:
	unsigned char*		m_pData;
	int					m_iMaxLineWidth;
	int					m_iMaxLineHeight;

private:
	//void buildLine(std::stringstream& ss, FT_Face face, int iCurXCursor, char cLastChar);

	//bool divideString(FT_Face face, const char* sText, int iMaxWidth, int iMaxHeight);

    // #HLP_BEGIN
    void buildLine(wstringstream& ss, FT_Face face, int iCurXCursor, wchar_t cLastChar);
    bool divideString(FT_Face face, const wchar_t* sText, int iMaxWidth, int iMaxHeight);
    int getCharWidth(FT_Face face, FT_ULong charcode);
    // #HLP_END
    
	/**
	 * compute the start pos of every line
	 * return value>0 represents the start x pos of the line, while -1 means fail
	 */
	//int computeLineStart(FT_Face face, CCImage::ETextAlign eAlignMask, char cText, int iLineIndex);
    // #HLP_BEGIN
    int computeLineStart( FT_Face face, CCImage::ETextAlign eAlignMask, wchar_t cText, int iLineIndex );
    // #HLP_END

	bool startsWith(const std::string& str, const std::string& what);
	bool endsWith(const std::string& str, const std::string& what);
	std::string fileNameExtension(const std::string& pathName);
	std::string basename(const std::string& pathName);
	int openFont(const std::string& fontName, uint fontSize);

private:
	FT_Library		m_library;
	FT_Face			m_face ;
	std::string		m_fontName ;
	uint			m_fontSize ;

	int				m_libError;
	int				m_iInterval;
	std::vector<TextLine> m_vLines;
};

bool BitmapDC::startsWith(const std::string& str, const std::string& what)
{
	bool result = false ;
	if( what.size() <= str.size() ) {
		result = (str.substr( 0, what.size() ) == what) ;
	}
	return result ;
}


bool BitmapDC::endsWith(const std::string& str, const std::string& what)
{
	bool result = false ;
	if( what.size() <= str.size() ) {
		result = (str.substr( str.size() - what.size() ) == what) ;
	}
	return result ;
}

std::string BitmapDC::fileNameExtension(const std::string& pathName)
{
	std::string ext ;
	std::string::size_type pos = pathName.find_last_of(".") ;
	if( pos != std::string::npos && pos != pathName.size()-1 ) {
		ext = pathName.substr(pos+1) ;
	}

	return ext ;
}

std::string BitmapDC::basename(const std::string& pathName)
{
	int pos = pathName.rfind("/"); 
	std::string bn = pathName.substr(pos + 1, pathName.length() - pos + 1);
	return bn ;
}

BitmapDC::BitmapDC() :
	m_face(NULL)
	,m_fontName()
	,m_fontSize(0)
	,m_iInterval(FONT_KERNING)
	,m_pData(NULL)
// #HLP_BEGIN
    ,mMaxLine(0)
// #HLP_END
{
	m_libError = FT_Init_FreeType( &m_library );
	reset();
}

BitmapDC::~BitmapDC()
{
	//  free face
	if( m_face ) {
		FT_Done_Face(m_face);
		m_face = NULL;
	}

	FT_Done_FreeType(m_library);
	//data will be deleted by CCImage
	//	if (m_pData) {
	//		delete [] m_pData;
	//	}
}

void BitmapDC::reset()
{
	m_iMaxLineWidth = 0;
	m_iMaxLineHeight = 0;
	m_vLines.clear();
}

//void BitmapDC::buildLine(std::stringstream& ss, FT_Face face, int iCurXCursor, char cLastChar )
// #HLP_BEGIN
void BitmapDC::buildLine(wstringstream& ss, FT_Face face, int iCurXCursor, wchar_t cLastChar)
// #HLP_END
{
	TextLine oTempLine;
	//ss << '\0';
    // #HLP_BEGIN
    ss << L'\0';
    // #HLP_END    
	oTempLine.sLineStr = ss.str();
	//get last glyph
	FT_Load_Glyph(face, FT_Get_Char_Index(face, cLastChar), FT_LOAD_DEFAULT);

//	oTempLine.iLineWidth =
//		iCurXCursor - 
//		RSHIFT6( face->glyph->metrics.horiAdvance +
//		face->glyph->metrics.horiBearingX
//		- face->glyph->metrics.width)/*-iInterval*/;	//TODO interval
    
    // #HLP_BEGIN
//    if(cLastChar == ' '){
//        oTempLine.iLineWidth = iCurXCursor;
//    }else{
        oTempLine.iLineWidth =
        iCurXCursor -
        RSHIFT6( face->glyph->metrics.horiAdvance -
                face->glyph->metrics.horiBearingX
                - face->glyph->metrics.width);
//    }
    //http://www.freetype.org/freetype2/docs/tutorial/step2.html
    // it should be advance - bearingx - width
    // #HLP_END    

	m_iMaxLineWidth = MAX(m_iMaxLineWidth, oTempLine.iLineWidth);
	ss.clear();
	//ss.str("");
    // #HLP_BEGIN
    ss.str(L"");
    // #HLP_END
	m_vLines.push_back(oTempLine);
}



// #HLP_BEGIN

bool isThaiEqual(const wchar_t* pText, const wchar_t* s){
    unsigned int len = wcslen(s);
    return wcsncmp(pText-len, s, len) == 0;
}

bool BitmapDC::canBreakThai(const wchar_t* pText){
    const wchar_t* thaiPreSara[] = {
        L"ไ",
        L"ฯ",
        L"โ",
        L"เ",
        L"แ",
        L"ใ",
        L"ั"
    };
    for(unsigned int i=0; i<sizeof(thaiPreSara)/sizeof(wchar_t*); i++){
        if(isThaiEqual(pText, thaiPreSara[i]))
            return false;
    }
    
    
    const wchar_t *after = pText+1;
    
    const wchar_t* thaiAfterSara[] = {
        L"่",
        L"้",
        L"๊",
        L"๋",
        L"ะ",
        L"า",
        L"ู",
        L"ุ",
        L"ึ",
        L"ๆ",
        L"ำ",
        L"ั",
        L"ี",
        L"ฯ",
        L"็",
        L"ิ",
        L"ื",
        L"์"
    };
    for(unsigned int i=0; i<sizeof(thaiAfterSara)/sizeof(wchar_t*); i++){
        if(isThaiEqual(after, thaiAfterSara[i]))
            return false;
    }
     
    
    
    THAI_DICT
    
    for(unsigned int i=0; i<sizeof(thaiDict)/sizeof(wchar_t*); i++){
        if(isThaiEqual(pText, thaiDict[i]))
            return true;
    }
    
    return false;
}
// #HLP_END


// #HLP_BEGIN
int BitmapDC::getCharWidth(FT_Face face, FT_ULong charcode){
    FT_Load_Glyph(face, FT_Get_Char_Index(face, charcode), FT_LOAD_DEFAULT);
    return RSHIFT6(face->glyph->metrics.horiAdvance);
}
// #HLP_END



//bool BitmapDC::divideString( FT_Face face, const char* sText, int iMaxWidth, int iMaxHeight )
// #HLP_BEGIN
bool BitmapDC::divideString(FT_Face face, const wchar_t* sText, int iMaxWidth, int iMaxHeight)
// #HLP_END
{
	//const char* pText = sText;
    // #HLP_BEGIN
    const wchar_t* pText = sText;
    const wchar_t* pTextBegin = pText;
    // #HLP_END
    
	int iError = 0;
	int iCurXCursor;
	iError = FT_Load_Glyph(face, FT_Get_Char_Index(face, *pText), FT_LOAD_DEFAULT);
	if (iError) {
		return false;
	}
	iCurXCursor = -RSHIFT6(face->glyph->metrics.horiBearingX);
	//init stringstream
	//std::stringstream ss;
    // #HLP_BEGIN
    wstringstream ss;
    // #HLP_END

	//int cLastCh = 0;
    // #HLP_BEGIN
    wchar_t cLastCh = 0;
    // #HLP_END

//	while (*pText != '\0') {
//		if (*pText == '\n') {
    // #HLP_BEGIN
    bool isHitMaxLine = false;
    while (*pText) {
        
        if(mMaxLine && (m_vLines.size() == mMaxLine)){
            isHitMaxLine = true;
            break;
        }
        
        if (*pText == L'\n') {
    // #HLP_END
			buildLine(ss, face, iCurXCursor, cLastCh);
            // #HLP_BEGIN
            pTextBegin = pText + 1;
            // #HLP_END
            
            pText++;
			iError = FT_Load_Glyph(face, FT_Get_Char_Index(face, *pText), FT_LOAD_DEFAULT);
			if (iError) {
				return false;
			}
			iCurXCursor = -RSHIFT6(face->glyph->metrics.horiBearingX);
			continue;
		}

		iError = FT_Load_Glyph(face, FT_Get_Char_Index(face, *pText), FT_LOAD_DEFAULT);

		if (iError) {
			return false;
			//break;
		}
		//check its width
		//divide it when exceeding
//		if ((iMaxWidth > 0 && iCurXCursor + RSHIFT6(face->glyph->metrics.width) > iMaxWidth)) {
        // #HLP_BEGIN
        if ((iMaxWidth > 0 && iCurXCursor + RSHIFT6(face->glyph->metrics.width) + RSHIFT6(face->glyph->metrics.horiBearingX) > iMaxWidth)) {
        // #HLP_END
//			buildLine(ss, face , iCurXCursor, cLastCh);
//
//			iCurXCursor = -RSHIFT6(face->glyph->metrics.horiBearingX);
            
            // #HLP_BEGIN
            // HLP: Word wrap
            unsigned int backCount = 0;
            bool canWrap = true;
            bool needWrap = true;
            int iCurXCursorBeforeTestWrap = iCurXCursor;
            const wchar_t* pTextBeforeTestWrap = pText;
            
            unsigned int numLine = m_vLines.size();
            if(mMaxLine != 0 && numLine == mMaxLine-1){
                needWrap = false;
                canWrap = false;
            }
            
            while(needWrap){
                cLastCh = *pText;
                wchar_t pTextBefore = *(pText-1);
                
                if(cLastCh == ' ' ||
                   cLastCh == '\t'
                   ){
                    //pText++;
                    break;
                }
                
                if(pTextBefore == '.' ||
                   pTextBefore == ',' ||
                   pTextBefore == '!' ||
                   pTextBefore == ':' ||
                   pTextBefore == '?' )
                    break;
                
                if(canBreakThai(pText))
                    break;
                
                pText--;
                backCount++;
                
                iError = FT_Load_Glyph(face, FT_Get_Char_Index(face, *pText), FT_LOAD_DEFAULT);
                int char_width = RSHIFT6(face->glyph->metrics.width);
                iCurXCursor -= char_width;
                
                if(pText == pTextBegin){
                    canWrap = false;
                    iCurXCursor = iCurXCursorBeforeTestWrap;
                    pText = pTextBeforeTestWrap;
                    break;
                }
            }
            if(canWrap)
                ss.seekp(ss.str().length()-backCount);
            

            buildLine(ss, face , iCurXCursor, cLastCh);
            pTextBegin = pText + 1;
            
			iCurXCursor = -RSHIFT6(face->glyph->metrics.horiBearingX);
            // #HLP_END

		}

		cLastCh = *pText;
		ss << *pText;
		iCurXCursor += RSHIFT6(face->glyph->metrics.horiAdvance) + m_iInterval;
		pText++;

/*
		if (cLastCh == ' ' || cLastCh == ',' || cLastCh == '.' || cLastCh == '!' || cLastCh == '?')
		{
			char *pText_temp = (char *)pText;
			int	iCurXCursor_temp = 0;
			while((strlen(pText_temp) > 0) && (*pText_temp!=' ') && (*pText_temp !=',') && (*pText_temp != '.') && (*pText_temp != '!') && (*pText_temp != '?') && (*pText_temp != '/0') && (*pText_temp != '/n'))
			{
				iError = FT_Load_Glyph(face, FT_Get_Char_Index(face, *pText_temp), FT_LOAD_DEFAULT);

				if (iError) {
					return false;
					//break;
				}
				iCurXCursor_temp += SHIFT6(face->glyph->metrics.horiAdvance) + iInterval;
				if (iCurXCursor + iCurXCursor_temp > iMaxWidth && iMaxWidth > 0)
				{
					buildLine(ss, face , iCurXCursor, cLastCh);

					iCurXCursor = -SHIFT6(face->glyph->metrics.horiBearingX);
				}
				pText_temp++;
			}
		}
*/
	}

	if (iError) {
		return false;
	}

	//buildLine(ss,face, iCurXCursor, cLastCh);
    // #HLP_BEGIN
    unsigned int numLine = m_vLines.size();
    if(mMaxLine == 0 || numLine <= mMaxLine-1)
        buildLine(ss,face, iCurXCursor, cLastCh);
    
    if(numLine > mMaxLine-1)
        isHitMaxLine = true;
    
    if(isHitMaxLine){
        if(numLine > 0){
            wstring *s = &m_vLines[numLine-1].sLineStr;
            int len = wcslen(s->c_str());//s->size();

            if(len >= 3){
                
                //int dot_width = getCharWidth(face, L'.')*3;
                // for prevent exceed half dot
                int dot_width = getCharWidth(face, L'.')*4;
                int s1 = getCharWidth(face, (*s)[len-3]);
                int s2 = getCharWidth(face, (*s)[len-2]);
                int s3 = getCharWidth(face, (*s)[len-1]);
                
                int w = m_vLines[numLine-1].iLineWidth;
                if(w + dot_width <= iMaxWidth){
                    s->resize(len+4);
                    (*s)[len+3] = L'\0';
                    len += 3;
                    m_vLines[numLine-1].iLineWidth = w + dot_width;
                }else if(w - s3 + dot_width <= iMaxWidth){
                    s->resize(len+3);
                    (*s)[len+2] = L'\0';
                    len += 2;
                    m_vLines[numLine-1].iLineWidth = w - s3 + dot_width;
                }else if(w - s3 - s2 + dot_width <= iMaxWidth){
                    s->resize(len+2);
                    (*s)[len+1] = L'\0';
                    len += 1;
                    m_vLines[numLine-1].iLineWidth = w - s3 - s2 + dot_width;
                }else if(w - s3 - s2 - s1 + dot_width <= iMaxWidth){
                    m_vLines[numLine-1].iLineWidth = w - s3 - s2 - s1 + dot_width;
                }
            
                (*s)[len-1] = L'.';
                (*s)[len-2] = L'.';
                (*s)[len-3] = L'.';
            }

        }
    }
    // #HLP_END

	return true;
}

//int BitmapDC::computeLineStart( FT_Face face, CCImage::ETextAlign eAlignMask, char cText, int iLineIndex )
// #HLP_BEGIN
int BitmapDC::computeLineStart( FT_Face face, CCImage::ETextAlign eAlignMask, wchar_t cText, int iLineIndex )
// #HLP_END
{
	int iRet;
	int iError = FT_Load_Glyph(face, FT_Get_Char_Index(face, cText), FT_LOAD_DEFAULT);
	if (iError) {
		return -1;
	}

//	if (eAlignMask == CCImage::kAlignCenter) {
//		iRet = (m_iMaxLineWidth - m_vLines[iLineIndex].iLineWidth) / 2 - RSHIFT6(face->glyph->metrics.horiBearingX );
//
//	} else if (eAlignMask == CCImage::kAlignRight) {
//		iRet = (m_iMaxLineWidth - m_vLines[iLineIndex].iLineWidth) - RSHIFT6(face->glyph->metrics.horiBearingX );
//	} else {
//		// left or other situation
//		iRet = -RSHIFT6(face->glyph->metrics.horiBearingX );
//	}

    // #HLP_BEGIN
    if (eAlignMask == CCImage::kAlignCenter || eAlignMask == CCImage::kAlignBottom || eAlignMask == CCImage::kAlignTop){
		iRet = (m_iMaxLineWidth - m_vLines[iLineIndex].iLineWidth) / 2 - RSHIFT6(face->glyph->metrics.horiBearingX );
	} else if (eAlignMask == CCImage::kAlignRight || eAlignMask == CCImage::kAlignBottomRight || eAlignMask == CCImage::kAlignTopRight) {
		iRet = (m_iMaxLineWidth - m_vLines[iLineIndex].iLineWidth) - RSHIFT6(face->glyph->metrics.horiBearingX );
	} else {
		// left or other situation
		iRet = -RSHIFT6(face->glyph->metrics.horiBearingX );
	}
    if(cText == ' ' || cText == '\t')
        iRet -= RSHIFT6(face->glyph->metrics.horiAdvance);
    // #HLP_END
    
    
    return iRet;
}

int BitmapDC::openFont(const std::string& fontName, uint fontSize)
{
	FT_Face aFace ;

	int iError = 0 ;
	//if( m_fontName != basename(fontName) || m_fontSize != fontSize ) {
    // #HLP_BEGIN
    if( m_fontName != basename(fontName) ) {
    // #HLP_END
		iError = FT_New_Face( m_library, fontName.c_str(), 0, &aFace );
		if( !iError ) {
			if(m_face) {
				FT_Done_Face(m_face);
			}

			m_face = aFace ;
			m_fontName = basename(fontName) ;
			m_fontSize = fontSize ;
		}
	}

	return iError ;
}


// #HLP_BEGIN
// GPOS not support so we have to hack like this
// until marmalade or cocos2d-x do ICU library support

FT_UInt BitmapDC::thaiAdjust(FT_UInt current_index, FT_UInt prev_index, FT_UInt next_index, const char *fontName){
    
    FT_UInt space;
    FT_UInt maihanakat_normal, maihanakat_left;
    FT_UInt maitaikhu_normal, maitaikhu_left;
    FT_UInt sarai_normal, sarai_left;
    FT_UInt saraii_normal, saraii_left;
    FT_UInt saraue_normal, saraue_left;
    FT_UInt sarauee_normal, sarauee_left;
    FT_UInt saraam;
    FT_UInt maiek_upperright, maiek_upperleft, maiek_lowleft, maiek_lowright;
    FT_UInt maitho_upperright, maitho_upperleft, maitho_lowleft, maitho_lowright;
    FT_UInt maitri_upperright, maitri_upperleft, maitri_lowleft, maitri_lowright;
    FT_UInt maichattawa_upperright, maichattawa_upperleft, maichattawa_lowleft, maichattawa_lowright;
    FT_UInt thanthakhat_upperright, thanthakhat_upperleft, thanthakhat_lowleft, thanthakhat_lowright;
    FT_UInt popla, fofa, fofan, lochula;
    
    if(!strcmp(fontName, "can_Rukdeaw.ttf")) {
        space = 3;
        maihanakat_normal = 179; maihanakat_left = 122;
        maitaikhu_normal = 201; maitaikhu_left = 124;
        sarai_normal = 182; sarai_left = 99;
        saraii_normal = 183; saraii_left = 100;
        saraue_normal = 184; saraue_left = 101;
        sarauee_normal = 185; sarauee_left = 102;
        saraam = 181;
        maiek_upperright = 202; maiek_upperleft = 125; maiek_lowleft = 104; maiek_lowright = 109;
        maitho_upperright = 203; maitho_upperleft = 126; maitho_lowleft = 105; maitho_lowright = 110;
        maitri_upperright = 204; maitri_upperleft = 127; maitri_lowleft = 106; maitri_lowright = 111;
        maichattawa_upperright = 205; maichattawa_upperleft = 128; maichattawa_lowleft = 107; maichattawa_lowright = 112;
        thanthakhat_upperright = 206; thanthakhat_upperleft = 129; thanthakhat_lowleft = 108; thanthakhat_lowright = 113;
        popla = 157; fofa = 159; fofan = 161; lochula = 228;
        
    }else if(!strcmp(fontName, "SukhumvitReg.ttf")) {
        space = 3;
        maihanakat_normal = 201; maihanakat_left = 271;
        maitaikhu_normal = 219; maitaikhu_left = 272;
        sarai_normal = 204; sarai_left = 256;
        saraii_normal = 205; saraii_left = 257;
        saraue_normal = 206; saraue_left = 258;
        sarauee_normal = 207; sarauee_left = 259;
        saraam = 203;
        maiek_upperright = 220; maiek_upperleft = 273; maiek_lowleft = 260; maiek_lowright = 265;
        maitho_upperright = 221; maitho_upperleft = 274; maitho_lowleft = 261; maitho_lowright = 266;
        maitri_upperright = 222; maitri_upperleft = 275; maitri_lowleft = 262; maitri_lowright = 267;
        maichattawa_upperright = 223; maichattawa_upperleft = 276; maichattawa_lowleft = 263; maichattawa_lowright = 268;
        thanthakhat_upperright = 224; thanthakhat_upperleft = 277; thanthakhat_lowleft = 264; thanthakhat_lowright = 269;
        popla = 180; fofa = 182; fofan = 184; lochula = 196;
    }else{
        //if(!strcmp(fontName, "Thonburi.ttf") or Bold
        // Thonburi.ttf
        // 3 space
        //  ั 77 normal 17 left
        //  ็ 97 normal 18 left
        //  ิ 80 normal 19 left
        //  ี 81 normal 20 left
        //  ึ 82 normal 21 left
        //  ื 83 normal 22 left
        // ำ 79
        //  ่ 98 upperright 23 upperleft 5 lowleft 10 lowright
        //  ้ 99 upperright 24 upperleft 6 lowleft 11 lowright
        //  ๊ 100 upperright 25 upperleft 7 lowleft 12 lowright
        //  ๋ 101 upperright 26 upperleft 8 lowleft 13 lowright
        //  ์ 102 upperright 27 upperleft 9 lowleft 14 lowright
        // ป ฝ ฟ ฬ 55 57 59 72
        space = 3;
        maihanakat_normal = 77; maihanakat_left = 17;
        maitaikhu_normal = 97; maitaikhu_left = 18;
        sarai_normal = 80; sarai_left = 19;
        saraii_normal = 81; saraii_left = 20;
        saraue_normal = 82; saraue_left = 21;
        sarauee_normal = 83; sarauee_left = 22;
        saraam = 79;
        maiek_upperright = 98; maiek_upperleft = 23; maiek_lowleft = 5; maiek_lowright = 10;
        maitho_upperright = 99; maitho_upperleft = 24; maitho_lowleft = 6; maitho_lowright = 11;
        maitri_upperright = 100; maitri_upperleft = 25; maitri_lowleft = 7; maitri_lowright = 12;
        maichattawa_upperright = 101; maichattawa_upperleft = 26; maichattawa_lowleft = 8; maichattawa_lowright = 13;
        thanthakhat_upperright = 102; thanthakhat_upperleft = 27; thanthakhat_lowleft = 9; thanthakhat_lowright = 14;
        popla = 55; fofa = 57; fofan = 59; lochula = 72;
    }
    
    
    
    
    FT_UInt glyph_index = current_index;
    
    //CCLog("ddddd %i", glyph_index);
    
    // low
    if(prev_index != maihanakat_normal
       && prev_index != sarai_normal
       && prev_index != saraii_normal
       && prev_index != saraue_normal
       && prev_index != sarauee_normal
       && prev_index != sarai_left
       && prev_index != saraii_left
       && prev_index != saraue_left
       && prev_index != sarauee_left
       
       && next_index != saraam){
        
        
        if(prev_index == popla || prev_index == fofa || prev_index == fofan || prev_index == lochula){
            // low left
            if(current_index == maiek_upperright)
                glyph_index = maiek_lowleft;
            else if(current_index == maitho_upperright)
                glyph_index = maitho_lowleft;
            else if(current_index == maitri_upperright)
                glyph_index = maitri_lowleft;
            else if(current_index == maichattawa_upperright)
                glyph_index = maichattawa_lowleft;
            else if(current_index == thanthakhat_upperright)
                glyph_index = thanthakhat_lowleft;
            else if(current_index == sarai_normal)
                glyph_index = sarai_left;
            else if(current_index == saraii_normal)
                glyph_index = saraii_left;
            else if(current_index == saraue_normal)
                glyph_index = saraue_left;
            else if(current_index == sarauee_normal)
                glyph_index = sarauee_left;
        }else{
            // low
            if(current_index == maiek_upperright)
                glyph_index = maiek_lowright;
            else if(current_index == maitho_upperright)
                glyph_index = maitho_lowright;
            else if(current_index == maitri_upperright)
                glyph_index = maitri_lowright;
            else if(current_index == maichattawa_upperright)
                glyph_index = maichattawa_lowright;
            else if(current_index == thanthakhat_upperright)
                glyph_index = thanthakhat_lowright;
            
        }
        
    }else{
        
        if((prev_index != maihanakat_normal
            &&prev_index != sarai_normal
            && prev_index != saraii_normal
            && prev_index != saraue_normal
            && prev_index != sarauee_normal)
           ||
           (prev_index == sarai_left
            || prev_index == saraii_left
            || prev_index == saraue_left
            || prev_index == sarauee_left)) {
               
               // top left
               if(current_index == maiek_upperright)
                   glyph_index = maiek_upperleft;
               else if(current_index == maitho_upperright)
                   glyph_index = maitho_upperleft;
               else if(current_index == maitri_upperright)
                   glyph_index = maitri_upperleft;
               else if(current_index == maichattawa_upperright)
                   glyph_index = maichattawa_upperleft;
               else if(current_index == thanthakhat_upperright)
                   glyph_index = thanthakhat_upperleft;
           }
        
        
    }
    return glyph_index;
}
// #HLP_END


//bool BitmapDC::getBitmap( const char *text, int nWidth, int nHeight, CCImage::ETextAlign eAlignMask, const char * pFontName, uint fontSize )
// #HLP_BEGIN
bool BitmapDC::getBitmap( const char *text, int nWidth, int nHeight, CCImage::ETextAlign eAlignMask, const char * pFontName, uint fontSize, int fixLineHeight)
// #HLP_END
{
	FT_Error iError;

	unsigned char cTemp ;
	int iY, iX, iTemp ;
	uint32 offset, rowOffset ;

	//data will be deleted by CCImage
	//		if (m_pData) {
	//			delete m_pData;
	//		}

	int iCurXCursor, iCurYCursor;
    // #HLP_BEGIN
    int iCurXCursorStart;
    // #HLP_END
    
	bool bRet = false;
	if (m_libError) {
		return false;
	}

	do {
		std::string fName = pFontName ;
		
		std::string ext = fileNameExtension(fName) ;
		if( ext.empty() || (ext != "ttf" && ext != "TTF") ) {
			fName += ".ttf" ;
		}

		if( !m_face || (m_fontName != basename(fName) || m_fontSize != fontSize) ) {

			iError = openFont( fName, fontSize );
			if (iError) {		// try loading from "fonts" folder
				if( !startsWith(fName,"fonts/") ) {
				 	fName = std::string("fonts/") + fName ;
				}

				iError = openFont( fName, fontSize );
				if (iError) { //no valid font found, try to use default
					
					fName = "fonts/Marker Felt.ttf" ;
					//CCLog("No valid font, use default %s", fName.c_str());
					iError = openFont( fName, fontSize );
				}
			}
			CC_BREAK_IF(iError);

			//select utf8 charmap
			iError = FT_Select_Charmap(m_face,FT_ENCODING_UNICODE);
			CC_BREAK_IF(iError);

			iError = FT_Set_Pixel_Sizes(m_face, fontSize,fontSize);
			CC_BREAK_IF(iError);
            
            // #HLP_BEGIN
            m_fontSize = fontSize;
            // #HLP_END
		}

		//iError = divideString(m_face, text, nWidth, nHeight) ? 0 : 1 ;
        // #HLP_BEGIN
        CCAssert(sizeof(wchar_t) == sizeof(ucs2char), "");
        int text_len = strlen(text);
        wstring wtext;
        wtext.reserve(text_len+1);
        UTF8ToUCS2(text, text_len+1, (ucs2char*)wtext.c_str(), sizeof(wchar_t)*(text_len+1));
        const wchar_t * pText = wtext.c_str();
        iError = divideString(m_face, pText, nWidth, nHeight) ? 0 : 1 ;

        FT_Pos adjustBearingY = 0;
        if(!strcmp(m_fontName.c_str(), "SukhumvitReg.ttf"))
            adjustBearingY = -3;
        // #HLP_END
        

		//compute the final line width
		m_iMaxLineWidth = MAX(m_iMaxLineWidth, nWidth);

		FT_Pos ascenderPixels = RSHIFT6(m_face->size->metrics.ascender) ;
		FT_Pos descenderPixels = RSHIFT6(m_face->size->metrics.descender) ;

		m_iMaxLineHeight = ascenderPixels - descenderPixels;
		m_iMaxLineHeight *= m_vLines.size();

		//compute the final line height
		m_iMaxLineHeight = MAX(m_iMaxLineHeight, nHeight);

        // #HLP_BEGIN
//        descenderPixels += 5;
//        ascenderPixels +=5;
        // Hack for prevent too big image cause crash memory
        if(m_iMaxLineHeight > 1024)
            m_iMaxLineHeight = 1024;
        // #HLP_END
        
		uint bitmapSize = m_iMaxLineWidth * m_iMaxLineHeight*4 ;

		m_pData = new unsigned char[bitmapSize];
		memset(m_pData,0, bitmapSize);

		//const char* pText = text;
		iCurYCursor = ascenderPixels;
        // #HLP_BEGIN
        if(fixLineHeight && (eAlignMask == CCImage::kAlignBottom || eAlignMask == CCImage::kAlignBottomLeft || eAlignMask == CCImage::kAlignBottomRight)){
            int numLine = m_vLines.size();
            int diff = ascenderPixels - descenderPixels - fixLineHeight;
            iCurYCursor += (numLine - 1)*diff;
        }
        // #HLP_END

		for (size_t i = 0; i < m_vLines.size(); i++) {
			pText = m_vLines[i].sLineStr.c_str();
			//initialize the origin cursor
			iCurXCursor = computeLineStart(m_face, eAlignMask, *pText, i);

            // #HLP_BEGIN
            iCurXCursorStart = iCurXCursor;
            
            // #hack for Thonburi.ttf only
            FT_UInt current_index;
            FT_UInt glyph_index;
            FT_UInt prev_index = 0;
            FT_UInt next_index;
            // #HLP_END
            
            
            
			while (*pText != 0) {
                
                // #HLP_BEGIN
                // #hack for Thonburi.ttf only
                current_index = FT_Get_Char_Index(m_face, *pText);
                if(*(pText+1) != 0)
                    next_index = FT_Get_Char_Index(m_face, *(pText+1));
                else
                    next_index = 0;
                glyph_index = thaiAdjust(current_index, prev_index, next_index, m_fontName.c_str());
                prev_index = glyph_index;
                
                
				//int iError = FT_Load_Glyph(m_face, FT_Get_Char_Index(m_face, *pText), FT_LOAD_RENDER);
                int iError = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER);
                // #HLP_END

				if (iError) {
					break;
				}

				//  convert glyph to bitmap with 256 gray
				//  and get the bitmap
				FT_Bitmap & bitmap = m_face->glyph->bitmap;

				//FT_Pos horiBearingYPixels = RSHIFT6(m_face->glyph->metrics.horiBearingY) ;
                // #HLP_BEGIN
                FT_Pos horiBearingYPixels = RSHIFT6(m_face->glyph->metrics.horiBearingY) + adjustBearingY ;
                // #HLP_END
				FT_Pos horiBearingXPixels = RSHIFT6(m_face->glyph->metrics.horiBearingX) ;
				FT_Pos horiAdvancePixels = RSHIFT6(m_face->glyph->metrics.horiAdvance) ;

				for (int i = 0; i < bitmap.rows; ++i) {

					iY = iCurYCursor + i - horiBearingYPixels;
					if (iY < 0 || iY>=m_iMaxLineHeight) {
						//exceed the height truncate
						continue;
					}

					rowOffset = iY * m_iMaxLineWidth ;

					// if it has gray>0 we set show it as 1, otherwise 0 
					for (int j = 0; j < bitmap.width; ++j) {
						cTemp = bitmap.buffer[i	* bitmap.width + j];
						if( cTemp )
						{
							iX = iCurXCursor + j + horiBearingXPixels;

							offset = (rowOffset + iX) * 4 ;

							//IwAssert( GAME, ((offset + 3) < bitmapSize) ) ;
                            // #HLP_BEGIN
                            if((offset + 3) >= bitmapSize)
                                break;
                            // #HLP_END

							iTemp = cTemp << 24 | cTemp << 16 | cTemp << 8 | cTemp;
 							*(int*) &m_pData[ offset ] = iTemp ;	// ARGB
						}
					}
				}

				//step to next glyph
				iCurXCursor += horiAdvancePixels + m_iInterval;
				pText++;
			}
			//iCurYCursor += ascenderPixels - descenderPixels ;
            // #HLP_BEGIN
            if(fixLineHeight)
                iCurYCursor += fixLineHeight;
            else
                iCurYCursor += ascenderPixels - descenderPixels ;
            // #HLP_END
		}

		//clear all lines
		m_vLines.clear();

		//success;
		if (iError) {
			bRet = false;
		} else
			bRet = true;
	}while(0);

    // #HLP_BEGIN
    mCursorX = iCurXCursor;
    mCursorXStart = iCurXCursorStart;
    float adjustCursorY = 0;
    if(!strcmp(m_fontName.c_str(), "Thonburi.ttf"))
        adjustCursorY = 1.48*CC_CONTENT_SCALE_FACTOR()*CC_CONTENT_SCALE_FACTOR();
    mCursorY = iCurYCursor + adjustCursorY;
    // #HLP_END
    
	return bRet;
}








static BitmapDC& sharedBitmapDC()
{
	static BitmapDC s_BmpDC;
	return s_BmpDC;
}
 
//////////////////////////////////////////////////////////////////////////
// Implement CCImage
//////////////////////////////////////////////////////////////////////////

CCImage::CCImage()
: m_nWidth(0)
, m_nHeight(0)
, m_nBitsPerComponent(0)
, m_pData(0)
, m_bHasAlpha(false)
, m_bPreMulti(false)
{
	
}

CCImage::~CCImage()
{
    CC_SAFE_DELETE_ARRAY(m_pData);
}
bool CCImage::initWithImageFile(const char * strPath, EImageFormat eImgFmt/* = eFmtPng*/)
{
	IW_CALLSTACK("UIImage::initWithImageFile");
    bool bRet = false;
    unsigned long nSize = 0;
    unsigned char* pBuffer = CCFileUtils::sharedFileUtils()->getFileData(strPath, "rb", &nSize);
    if (pBuffer != NULL && nSize > 0)
    {
        bRet = initWithImageData(pBuffer, nSize, eImgFmt);
    }
    CC_SAFE_DELETE_ARRAY(pBuffer);
    return bRet;
}

bool CCImage::initWithImageFileThreadSafe( const char *fullpath, EImageFormat imageType /*= kFmtPng*/ )
{
	CC_UNUSED_PARAM(imageType);
    bool bRet = false;
    unsigned long nSize = 0;
    unsigned char *pBuffer = CCFileUtils::sharedFileUtils()->getFileData(fullpath, "rb", &nSize);
    if (pBuffer != NULL && nSize > 0)
    {
        bRet = initWithImageData(pBuffer, nSize, imageType);
    }
    CC_SAFE_DELETE_ARRAY(pBuffer);
    return bRet;
}

bool CCImage::initWithImageData(void * pData, 
								int nDataLen, 
								EImageFormat eFmt,
								int nWidth,
								int nHeight,
								int nBitsPerComponent)
{
    bool bRet = false;
    do 
    {
    	CC_BREAK_IF(! pData || nDataLen <= 0);
		
        if (kFmtPng == eFmt)
        {
            bRet = _initWithPngData(pData, nDataLen);
            break;
        }
        else if (kFmtJpg == eFmt)
        {
            bRet = _initWithJpgData(pData, nDataLen);
            break;
        }
        else if (kFmtTiff == eFmt)
        {
            bRet = _initWithTiffData(pData, nDataLen);
            break;
        }
        else if (kFmtWebp == eFmt)
        {
            bRet = _initWithWebpData(pData, nDataLen);
            break;
        }
        else if (kFmtRawData == eFmt)
        {
            bRet = _initWithRawData(pData, nDataLen, nWidth, nHeight, nBitsPerComponent, false);
            break;
        }
        else
        {
            // if it is a png file buffer.
            if (nDataLen > 8)
            {
                unsigned char* pHead = (unsigned char*)pData;
                if (   pHead[0] == 0x89
                    && pHead[1] == 0x50
                    && pHead[2] == 0x4E
                    && pHead[3] == 0x47
                    && pHead[4] == 0x0D
                    && pHead[5] == 0x0A
                    && pHead[6] == 0x1A
                    && pHead[7] == 0x0A)
                {
                    bRet = _initWithPngData(pData, nDataLen);
                    break;
                }
            }

            // if it is a tiff file buffer.
            if (nDataLen > 2)
            {
                unsigned char* pHead = (unsigned char*)pData;
                if (  (pHead[0] == 0x49 && pHead[1] == 0x49)
                    || (pHead[0] == 0x4d && pHead[1] == 0x4d)
                    )
                {
                    bRet = _initWithTiffData(pData, nDataLen);
                    break;
                }
            }

            // if it is a jpeg file buffer.
            if (nDataLen > 2)
            {
                unsigned char* pHead = (unsigned char*)pData;
                if (   pHead[0] == 0xff
                    && pHead[1] == 0xd8)
                {
                    bRet = _initWithJpgData(pData, nDataLen);
                    break;
                }
            }
        }
	} while (0);
    return bRet;
}

bool CCImage::_initWithJpgData(void * data, int nSize)
{	
	IW_CALLSTACK("CCImage::_initWithJpgData");

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1] = {0};
    unsigned long location = 0;
    unsigned int i = 0;

    bool bRet = false;
    do 
    {
        /* here we set up the standard libjpeg error handler */
        cinfo.err = jpeg_std_error( &jerr );

        /* setup decompression process and source, then read JPEG header */
        jpeg_create_decompress( &cinfo );

	    jpeg_source_mgr srcmgr;

		srcmgr.bytes_in_buffer = nSize;
		srcmgr.next_input_byte = (JOCTET*) data;
		srcmgr.init_source = CCImageHelper::JPEGInitSource;
		srcmgr.fill_input_buffer = CCImageHelper::JPEGFillInputBuffer;
		srcmgr.skip_input_data = CCImageHelper::JPEGSkipInputData;
		srcmgr.resync_to_restart = jpeg_resync_to_restart;
		srcmgr.term_source = CCImageHelper::JPEGTermSource;
		cinfo.src = &srcmgr;
//      jpeg_mem_src( &cinfo, (unsigned char *) data, nSize );

        /* reading the image header which contains image information */
        jpeg_read_header( &cinfo, true );

        // we only support RGB or grayscale
        if (cinfo.jpeg_color_space != JCS_RGB)
		{
            if (cinfo.jpeg_color_space == JCS_GRAYSCALE || cinfo.jpeg_color_space == JCS_YCbCr)
            {
                cinfo.out_color_space = JCS_RGB;
		    }
        }
        else
        {
            break;
        }

        /* Start decompression jpeg here */
        jpeg_start_decompress( &cinfo );

        /* init image info */
        m_nWidth  = (short)(cinfo.image_width);
        m_nHeight = (short)(cinfo.image_height);
		m_bHasAlpha = false;
        m_bPreMulti = false;
        m_nBitsPerComponent = 8;
        row_pointer[0] = new unsigned char[cinfo.output_width*cinfo.output_components];
        CC_BREAK_IF(! row_pointer[0]);

        m_pData = new unsigned char[cinfo.output_width*cinfo.output_height*cinfo.output_components];
        CC_BREAK_IF(! m_pData);

        /* now actually read the jpeg into the raw buffer */
        /* read one scan line at a time */
        while( cinfo.output_scanline < cinfo.image_height )
		{
            jpeg_read_scanlines( &cinfo, row_pointer, 1 );
            for( i=0; i<cinfo.image_width*cinfo.output_components;i++) 
            {
                m_pData[location++] = row_pointer[0][i];
			}
		}

        jpeg_finish_decompress( &cinfo );
		jpeg_destroy_decompress(&cinfo);
        /* wrap up decompression, destroy objects, free pointers and close open files */        
		bRet = true;
    } while (0);

    CC_SAFE_DELETE_ARRAY(row_pointer[0]);
	return bRet;
}

void userReadData(png_structp pngPtr, png_bytep data, png_size_t length) {
	png_voidp png_pointer = png_get_io_ptr(pngPtr);
	s3eFileRead((char*)data, length, 1, (s3eFile*)png_pointer);
}

#define PNGSIGSIZE 8
bool CCImage::_initWithPngData(void * pData, int nDatalen)
{
	IW_CALLSTACK("CCImage::_initWithPngData");
	
    bool bRet = false;
	
	s3eFile* pFile = s3eFileOpenFromMemory(pData, nDatalen);
	
	IwAssert(GAME, pFile);
	
	png_byte pngsig[PNGSIGSIZE];
	
	bool is_png = false;
	
	s3eFileRead((char*)pngsig, PNGSIGSIZE, 1, pFile);
	
	is_png = png_sig_cmp(pngsig, 0, PNGSIGSIZE) == 0 ? true : false;
	
	if (!is_png)
		return false;
	
	png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (!pngPtr)
		return false;
	
	png_infop infoPtr = png_create_info_struct(pngPtr);
	
	if (!infoPtr)
		return false;
	
	png_bytep* rowPtrs = NULL;
	m_pData = NULL;
	
	if (setjmp(png_jmpbuf(pngPtr))) {
		png_destroy_read_struct(&pngPtr, &infoPtr,(png_infopp)0);
		if (rowPtrs != NULL) delete [] rowPtrs;
		if (m_pData != NULL) delete [] m_pData;
		
		CCLog("ERROR: An error occured while reading the PNG file");
		
		return false;
	}
	
	png_set_read_fn(pngPtr, pFile, userReadData);
	png_set_sig_bytes(pngPtr, PNGSIGSIZE);
	png_read_info(pngPtr, infoPtr);
	
	
	png_uint_32 bitdepth   = png_get_bit_depth(pngPtr, infoPtr);
	png_uint_32 channels   = png_get_channels(pngPtr, infoPtr);
	png_uint_32 color_type = png_get_color_type(pngPtr, infoPtr);
	
	// Convert palette color to true color
	if (color_type ==PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(pngPtr);
	
	// Convert low bit colors to 8 bit colors
	if (png_get_bit_depth(pngPtr, infoPtr) < 8)
	{
		if (color_type==PNG_COLOR_TYPE_GRAY || color_type==PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_1_2_4_to_8(pngPtr);
		else
			png_set_packing(pngPtr);
	}

	if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(pngPtr);
	
	// Convert high bit colors to 8 bit colors
	if (bitdepth == 16)
		png_set_strip_16(pngPtr);
	
	// Convert gray color to true color
	if (color_type==PNG_COLOR_TYPE_GRAY || color_type==PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(pngPtr);
	
	// Update the changes
	png_read_update_info(pngPtr, infoPtr);
	
	// init image info
	m_bPreMulti	= true;
	
	unsigned int bytesPerComponent = png_get_channels(pngPtr, infoPtr);
	
	m_bHasAlpha = (bytesPerComponent == 4 ? true : false);
	
	m_nHeight = (unsigned int)png_get_image_height(pngPtr, infoPtr);
	m_nWidth = (unsigned int) png_get_image_width(pngPtr, infoPtr);
	
	m_nBitsPerComponent = (unsigned int)png_get_bit_depth(pngPtr, infoPtr);
	
	m_pData = new unsigned char[m_nHeight * m_nWidth * bytesPerComponent];
	
	unsigned int bytesPerRow = m_nWidth * bytesPerComponent;
	
	{
		unsigned char *ptr = m_pData;
		rowPtrs = new png_bytep[m_nHeight];
				
		for (int i = 0; i < m_nHeight; i++) {
			
			int q = (i) * bytesPerRow;
			
			rowPtrs[i] = (png_bytep)m_pData + q;
		}
		
		png_read_image(pngPtr, rowPtrs);
		
		delete[] (png_bytep)rowPtrs;
		png_destroy_read_struct(&pngPtr, &infoPtr,(png_infopp)0);
		
		s3eFileClose(pFile);
		pFile = 0;
	}
	
	// premultiplay if alpha
	if(m_bHasAlpha)
		for(unsigned int i = 0; i < m_nHeight*bytesPerRow; i += bytesPerComponent){
			*(m_pData + i + 0)	=  (*(m_pData + i + 0) * *(m_pData + i + 3) + 1) >> 8;
			*(m_pData + i + 1)	=  (*(m_pData + i + 1) * *(m_pData + i + 3) + 1) >> 8;					
			*(m_pData + i + 2)	=  (*(m_pData + i + 2) * *(m_pData + i + 3) + 1) >> 8;
			*(m_pData + i + 3)	=   *(m_pData + i + 3);
	}
	

	
	bRet = true;
    return bRet;
}

bool CCImage::initWithString(
							 const char *    pText, 
							 int             nWidth/* = 0*/, 
							 int             nHeight/* = 0*/,
							 ETextAlign      eAlignMask/* = kAlignCenter*/,
							 const char *    pFontName/* = nil*/,
//							 int             nSize/* = 0*/)
                             // #HLP_BEGIN
                             int             nSize/* = 0*/,
                             int             fixLineHeight/* = 0*/,
                             unsigned int    maxLine/* = 0*/)
                             // #HLP_END
{
	bool bRet = false;
	do
	{
		CC_BREAK_IF(! pText);

		BitmapDC &dc = sharedBitmapDC();

        std::string fullFontName = pFontName;
    	std::string lowerCasePath = fullFontName;
    	std::transform(lowerCasePath.begin(), lowerCasePath.end(), lowerCasePath.begin(), ::tolower);
        
    	if ( lowerCasePath.find(".ttf") != std::string::npos ) {
    		fullFontName = CCFileUtils::sharedFileUtils()->fullPathForFilename(pFontName);
    	}

		//CC_BREAK_IF(! dc.getBitmap(pText, nWidth, nHeight, eAlignMask, fullFontName.c_str(), nSize));
        // #HLP_BEGIN
        dc.mMaxLine = maxLine;
        CC_BREAK_IF(! dc.getBitmap(pText, nWidth, nHeight, eAlignMask, fullFontName.c_str(), nSize, fixLineHeight));
        mCursorX = dc.mCursorX;
        mCursorXStart = dc.mCursorXStart;
        mCursorY = dc.mCursorY;
        // #HLP_END

		// assign the dc.m_pData to m_pData in order to save time
		m_pData = dc.m_pData;
		CC_BREAK_IF(! m_pData);

		m_nWidth = (short)dc.m_iMaxLineWidth;
		m_nHeight = (short)dc.m_iMaxLineHeight;
		m_bHasAlpha = true;
		m_bPreMulti = true;
		m_nBitsPerComponent = 8;

		bRet = true;

		dc.reset();
	}while (0);

	//do nothing
	return bRet; 
}

static tmsize_t _tiffReadProc(thandle_t fd, void* buf, tmsize_t size)
{
    tImageSource* isource = (tImageSource*)fd;
    uint8* ma;
    uint64 mb;
    unsigned long n;
    unsigned long o;
    tmsize_t p;
    ma=(uint8*)buf;
    mb=size;
    p=0;
    while (mb>0)
    {
        n=0x80000000UL;
        if ((uint64)n>mb)
            n=(unsigned long)mb;


        if((int)(isource->offset + n) <= isource->size)
        {
            memcpy(ma, isource->data+isource->offset, n);
            isource->offset += n;
            o = n;
        }
        else
        {
            return 0;
        }

        ma+=o;
        mb-=o;
        p+=o;
        if (o!=n)
        {
            break;
        }
    }
    return p;
}

static tmsize_t _tiffWriteProc(thandle_t fd, void* buf, tmsize_t size)
{
    CC_UNUSED_PARAM(fd);
    CC_UNUSED_PARAM(buf);
    CC_UNUSED_PARAM(size);
    return 0;
}


static uint64 _tiffSeekProc(thandle_t fd, uint64 off, int whence)
{
    tImageSource* isource = (tImageSource*)fd;
    uint64 ret = -1;
    do 
    {
        if (whence == SEEK_SET)
        {
            CC_BREAK_IF(off > isource->size-1);
            ret = isource->offset = (uint32)off;
        }
        else if (whence == SEEK_CUR)
        {
            CC_BREAK_IF(isource->offset + off > isource->size-1);
            ret = isource->offset += (uint32)off;
        }
        else if (whence == SEEK_END)
        {
            CC_BREAK_IF(off > isource->size-1);
            ret = isource->offset = (uint32)(isource->size-1 - off);
        }
        else
        {
            CC_BREAK_IF(off > isource->size-1);
            ret = isource->offset = (uint32)off;
        }
    } while (0);

    return ret;
}

static uint64 _tiffSizeProc(thandle_t fd)
{
    tImageSource* pImageSrc = (tImageSource*)fd;
    return pImageSrc->size;
}

static int _tiffCloseProc(thandle_t fd)
{
    CC_UNUSED_PARAM(fd);
    return 0;
}

static int _tiffMapProc(thandle_t fd, void** pbase, toff_t* psize)
{
    CC_UNUSED_PARAM(fd);
    CC_UNUSED_PARAM(pbase);
    CC_UNUSED_PARAM(psize);
    return 0;
}

static void _tiffUnmapProc(thandle_t fd, void* base, toff_t size)
{
    CC_UNUSED_PARAM(fd);
    CC_UNUSED_PARAM(base);
    CC_UNUSED_PARAM(size);
}

bool CCImage::_initWithTiffData(void* pData, int nDataLen)
{
    bool bRet = false;
    do 
    {
        // set the read call back function
        tImageSource imageSource;
        imageSource.data    = (unsigned char*)pData;
        imageSource.size    = nDataLen;
        imageSource.offset  = 0;

        TIFF* tif = TIFFClientOpen("file.tif", "r", (thandle_t)&imageSource, 
            _tiffReadProc, _tiffWriteProc,
            _tiffSeekProc, _tiffCloseProc, _tiffSizeProc,
            _tiffMapProc,
            _tiffUnmapProc);

        CC_BREAK_IF(NULL == tif);

        uint32 w = 0, h = 0;
        uint16 bitsPerSample = 0, samplePerPixel = 0, planarConfig = 0;
        size_t npixels = 0;
        
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplePerPixel);
        TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarConfig);

        npixels = w * h;
        
        m_bHasAlpha = true;
        m_nWidth = w;
        m_nHeight = h;
        m_nBitsPerComponent = 8;

        m_pData = new unsigned char[npixels * sizeof (uint32)];

        uint32* raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
        if (raster != NULL) 
        {
           if (TIFFReadRGBAImageOriented(tif, w, h, raster, ORIENTATION_TOPLEFT, 0))
           {
                unsigned char* src = (unsigned char*)raster;
                unsigned int* tmp = (unsigned int*)m_pData;

                /* the raster data is pre-multiplied by the alpha component 
                   after invoking TIFFReadRGBAImageOriented
                for(int j = 0; j < m_nWidth * m_nHeight * 4; j += 4)
                {
                    *tmp++ = CC_RGB_PREMULTIPLY_ALPHA( src[j], src[j + 1], 
                        src[j + 2], src[j + 3] );
                }
                */
                m_bPreMulti = true;

               memcpy(m_pData, raster, npixels*sizeof (uint32));
           }

          _TIFFfree(raster);
        }
        

        TIFFClose(tif);

        bRet = true;
    } while (0);
    return bRet;
}

bool CCImage::_initWithRawData(void * pData, int nDatalen, int nWidth, int nHeight, int nBitsPerComponent, bool bPreMulti)
{
    bool bRet = false;
    do 
    {
        CC_BREAK_IF(0 == nWidth || 0 == nHeight);

        m_nBitsPerComponent = nBitsPerComponent;
        m_nHeight   = (short)nHeight;
        m_nWidth    = (short)nWidth;
        m_bHasAlpha = true;

        // only RGBA8888 supported
        int nBytesPerComponent = 4;
        int nSize = nHeight * nWidth * nBytesPerComponent;
        m_pData = new unsigned char[nSize];
        CC_BREAK_IF(! m_pData);
        memcpy(m_pData, pData, nSize);

        bRet = true;
    } while (0);
    return bRet;
}

bool CCImage::saveToFile(const char *pszFilePath, bool bIsToRGB)
{
    // #HLP_BEGIN
    CIwImage *x = new CIwImage();
    x->SetFormat(CIwImage::ABGR_8888);
    x->SetWidth(m_nWidth);
    x->SetHeight(m_nHeight);
    x->SetPitch(m_nWidth * 4);
    x->SetBuffers(0, 0, 0, 0);
    x->ReadTexels(m_pData);
    x->SaveJpg(pszFilePath, 90);
    delete x;
    return true;
    // #HLP_END
    
	// todo
	//return false;
}

bool CCImage::_saveImageToPNG(const char * pszFilePath, bool bIsToRGB)
{
	// todo
	return false;
}

bool CCImage::_saveImageToJPG(const char * pszFilePath)
{
	// todo
	return false;
}

NS_CC_END;
