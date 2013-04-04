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
    FT_UInt thaiAdjust(FT_UInt current_index, FT_UInt prev_index, FT_UInt next_index);
    bool canBreakThai(const wchar_t* pText);
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
    oTempLine.iLineWidth =
    iCurXCursor -
    RSHIFT6( face->glyph->metrics.horiAdvance +
            face->glyph->metrics.horiBearingX
            - face->glyph->metrics.width)+m_fontSize/5;	//TODO interval m_fontSize/10 is hack not real fix
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
    /*
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
     return true;
     */
    
    
    const wchar_t* thaiDict[] = {
        L"ก็", L"กฎหมาย", L"กร", L"กรกฎาคม", L"กรม", L"กรรม", L"กรรมการ", L"กระจาย", L"กระดาษ", L"กระด้าง", L"กระดูก", L"กระทรวง", L"กระทำ", L"กระนั้น", L"กระมัง", L"กระไร", L"กรุง", L"กรุงเทพฯ", L"กลม", L"กลับ", L"กลาง", L"กล่าว", L"กล้า", L"กลัว", L"เกลา", L"เกล้า", L"กว่า", L"กว้าง", L"กษัตริย์", L"กสิกรรม", L"ก่อ", L"ก่อน", L"ก้อน", L"กอบ", L"กะ", L"กะดาน", L"กัน", L"กันยายน", L"กับ", L"กำไร", L"กำลัง", L"กำหนด", L"กา", L"กาง", L"กาชาด", L"กาย", L"การ", L"เกาะ", L"กิจ", L"กิน", L"กิริยา", L"กิเลส", L"กี่", L"กุมภาพันธ์", L"กุมาร", L"กู้", L"เก็บ", L"เกรง", L"เกณฑ์", L"เกวียน", L"เก่า", L"เก้า", L"เก้าอี้", L"เกิด", L"เกิน", L"เกียรติยศ", L"เกียจคร้าน", L"เกี่ยว", L"เกี่ยวข้อง", L"เกือบ", L"แก", L"แก่", L"แก้", L"แก้ว", L"แกะ", L"โกร่ง", L"โกรธ", L"ใกล้", L"ไก่", L"ไกล", L"ขด", L"ขน", L"ขนาด", L"ขณะ", L"ขยาย", L"ขวาง", L"ขว้าง", L"ขอ", L"ขอรับ", L"ของ", L"ขอบ", L"ข้อ", L"ขา", L"ขาด", L"ขาย", L"ขาว", L"ข่าว", L"ข้า", L"ข้าง", L"ข้าพเจ้า", L"ข้าม", L"ข้าราชการ", L"ข้าศึก", L"ขัง", L"ขัด", L"ขัน", L"ขับ", L"เขตต์", L"ขีด", L"ขี่", L"ขึ้น", L"เขา", L"เข้า", L"เขียน", L"เขียว", L"แขก", L"แข็ง", L"แข่ง", L"ไข", L"ไข่", L"ไข้", L"คอ", L"คอย", L"ค่อย", L"คับ", L"ค่า", L"ค้า", L"คำ", L"ค่ำ", L"คิด", L"คืน", L"คือ", L"คุณ", L"คู", L"คูณ", L"คดี", L"คน", L"ค้น", L"คณะ", L"ครึ่ง", L"ครั้ง", L"ครั้น", L"ครบ", L"ครอบครัว", L"ครอง", L"คราว", L"ครู", L"คลอง", L"คลาย", L"คลุม", L"ความ", L"ควร", L"ควาย", L"เคย", L"เคียง", L"เครื่อง", L"เคลื่อน", L"เคือง", L"โคม", L"ใคร", L"ใคร่", L"งอก", L"งาน", L"งาม", L"ง่าย", L"งู", L"เงิน", L"โง่", L"จง", L"จด", L"จดหมาย", L"จน", L"จบ", L"จม", L"จาก", L"จ่าย", L"จ้าง", L"จะ", L"จัก", L"จักร", L"จัง", L"จังหวัด", L"จัด", L"จันทร์", L"จับ", L"จำ", L"จำนวน", L"จำเป็น", L"จำพวก", L"จิตต์", L"จริง", L"จริต", L"จีน", L"จึ่ง", L"เจ็บ", L"เจริญ", L"เจ็ด", L"เจ้า", L"เจาะ", L"แจง", L"แจ้ง", L"แจ่ม", L"ใจ", L"โจทย์", L"ฉลาด", L"ฉัน", L"ฉะนั้น", L"ฉะนี้", L"ฉะบับ", L"ฉะเพาะ", L"ไฉน", L"ชน", L"ชม", L"ชอบ", L"ชวน", L"ช่วย", L"ช่อง", L"ชะนวน", L"ชะนะ", L"ชะนิด", L"ชัก", L"ชัด", L"ชั่ว", L"ชั่วโมง", L"ชั้น", L"ชาย", L"ชาว", L"ชา", L"ช้า", L"ช่าง", L"ช้าง", L"ชาติ", L"ชำนาญ", L"ชิง", L"ชีวิต", L"ชีพ", L"ชี้", L"ชื่อ", L"ชม", L"เช่น", L"เช้า", L"เชิญ", L"เชื่อ", L"เชื้อ", L"ใช่", L"ใช้", L"ซ่อน", L"ซ้าย", L"ซ้ำ", L"ซิ", L"ซีก", L"ซึ่ง", L"ซื่อ", L"ซื้อ", L"ญาติ", L"ญี่ปุ่น", L"ดง", L"ดวง", L"ด้วย", L"ดอก", L"ดัง", L"ดับ", L"ดาว", L"ด้าน", L"ดำ", L"ดำเนิน", L"ดำริ", L"ดิน", L"ดี", L"ดื่ม", L"ดุจ", L"ดู", L"เด็ก", L"เดิน", L"เดิม", L"เดียว", L"เดี่ยว", L"เดี๋ยว", L"เดี๋ยวนี้", L"เดือด", L"เดือน", L"แดง", L"แดด", L"แดน", L"โดย", L"ใด", L"ได้", L"ตก", L"ตน", L"ต้น", L"ตรง", L"ตรวจ", L"ตระกูล", L"ตรัส", L"ตรา", L"ตลอด", L"ตลาด", L"ตะวัน", L"ตอน", L"ต่อ", L"ต้อง", L"ต้อน", L"ตอบ", L"ตะเกียง", L"ตำ", L"ต่ำ", L"ตำบล", L"ตำแหน่ง", L"ตัว", L"ตัวอย่าง", L"ตัด", L"ตัดสิน", L"ตั้ง", L"ตา", L"ตาน", L"ตาม", L"ตาย", L"ตี", L"ติด", L"ตึก", L"เต็ม", L"เตรียม", L"เตือน", L"แต่", L"แตก", L"แต่ง", L"ตุลาคม", L"ตู้", L"โต", L"ใต้", L"ไต่", L"ถนน", L"ถวาย", L"ถอย", L"ถ้อย", L"ถั่ว", L"ถ้า", L"ถาน", L"ถาม", L"ถูก", L"เถิด", L"เถียง", L"แถว", L"ไถ", L"ทดลอง", L"ทน", L"ทรง", L"ทราบ", L"ทรัพย์", L"ทราบ", L"ทวี", L"ทวีป", L"ทหาร", L"ท่อ", L"ทอง", L"ท้อง", L"ทอด", L"ทะเล", L"ทัน", L"ทับ", L"ทัพ", L"ทั่ว", L"ทั้ง", L"ทั้งปวง", L"ทั้งหลาย", L"ทา", L"ทาง", L"ทาน", L"ท่าน", L"ทาบ", L"ทาย", L"ท้าย", L"ท้าว", L"ทำ", L"ทำไม", L"ทำลาย", L"ทิศ", L"ทิ้ง", L"ที", L"ทีเดียว", L"ที่", L"ทุก", L"ทุกข์", L"ทุง", L"ทูด", L"ทูน", L"เท", L"เท็จ", L"เทวดา", L"เท่า", L"เที่ยง", L"เทียบ", L"เทียม", L"เที่ยว", L"แทน", L"แทบ", L"แท้", L"โทษ", L"ไทย", L"ธง", L"ธันวาคม", L"ธรรม", L"ธรรมดา", L"ธรรมเนียม", L"ธาตุ", L"ธุระ", L"เธอ", L"นก", L"นอก", L"นอกจาก", L"นอน", L"น้อง", L"น้อย", L"นัก", L"นักปราชญ์", L"นักเรียน", L"นั่ง", L"นั่น", L"นั้น", L"นับ", L"นับถือ", L"นา", L"น่า", L"นาง", L"นาน", L"นาม", L"นาฬิกา", L"นำ", L"น้ำ", L"นิ่ง", L"นิด", L"นิทาน", L"นิยม", L"นิ้ว", L"นี่", L"นี้", L"นึก", L"นุ่ง", L"เนื่อง", L"เนื้อ", L"แนะนำ", L"แน่", L"แน่น", L"โน้น", L"ใน", L"บก", L"บท", L"บน", L"บรรณาธิการ", L"บรรดา", L"บริบูรณ์", L"บริษัท", L"บริสุทธิ์", L"บอก", L"บ่อย", L"บัง", L"บังเกิด", L"บังคับ", L"บัดนี้", L"บัญชา", L"บัญญัติ", L"บาง", L"บางที", L"บ้าง", L"บาท", L"บาน", L"บ้าน", L"บ่าย", L"บำรุง", L"บิดา", L"บุคคล", L"บุตร", L"บุตรี", L"บุญ", L"บุรุษ", L"บุหรี่", L"เบี้ย", L"เบื้อง", L"แบ่ง", L"แบน", L"โบราณ", L"ใบ", L"ปกครอง", L"ปฏิบัติ", L"ปกติ", L"ปรกติ", L"ประกอบ", L"ประการ", L"ประกาศ", L"ประจำ", L"ประชุม", L"ประตู", L"ประเทศ", L"ประพฤติ", L"ประมาณ", L"ประโยชน์", L"ประสงค์", L"ประเสริฐ", L"ประหลาด", L"ปรากฏ", L"ปรารถนา", L"ปราบปราม", L"ปราศจาก", L"ปรัดยุบัน", L"ปลา", L"ปลาบ", L"ปล่อย", L"ปลูก", L"ปวง", L"ป้องกัน", L"ปั้น", L"ปัญญา", L"ปัญหา", L"ป่า", L"ปาก", L"ปิด", L"ปี", L"เป็น", L"เป็ด", L"เปรียบ", L"เปล่า", L"เปลี่ยน", L"เปลือก", L"เปลือง", L"แปด", L"แปลก", L"แปลง", L"โปรด", L"ไป", L"ไปรษณีย์", L"ผล", L"ผะสม", L"ผ้า", L"ผ่าน", L"ผิด", L"ผิว", L"ผู้", L"ผูก", L"เผย", L"เผื่อ", L"แผ่", L"แผน", L"แผ่น", L"แผ่นดิน", L"ฝน", L"ฝรั่ง", L"ฝัก", L"ฝัง", L"ฝา", L"ฝาก", L"ฝ่าย", L"ฝึก", L"ฝึกหัด", L"พงศาวดาร", L"พ้น", L"พบ", L"พม่า", L"พยายาม", L"พรวน", L"พร้อม", L"พระ", L"พระยา", L"พราหมณ์", L"พฤศจิกายน", L"พฤศภาคม", L"พฤหัสบดี", L"พล", L"พลเมือง", L"พลอย", L"พลิก", L"พวก", L"พอ", L"พ่อ", L"พอก", L"พ้อง", L"พยาน", L"พัก", L"พัน", L"พันธ์", L"พา", L"พาณิชย์", L"พาย", L"พาล", L"พาหนะ", L"พิจารณา", L"พิธี", L"พิมพ์", L"พิษ", L"พิเศษ", L"พี่", L"พึง", L"พึ่ง", L"พืช", L"พื้น", L"พุทธ", L"พุธ", L"พูด", L"เพาะ", L"เพราะ", L"เพลิง", L"เพลิน", L"เพียง", L"เพียงไร", L"เพียน", L"เพิ่ม", L"เพื่อ", L"เพื่อน", L"แพง", L"แพ้", L"ฟ้อง", L"ฟัก", L"ฟัง", L"ฟัน", L"ฟ้า", L"ไฟ", L"ภรรยา", L"ภัย", L"ภาค", L"ภาพ", L"ภาย", L"ภายนอก", L"ภายใน", L"ภายหลัง", L"ภาษิต", L"ภาษี", L"ภูเขา", L"ภูม", L"มกราคม", L"มณฑล", L"มนตรี", L"มนุษย์", L"มอง", L"มอญ", L"มอบ", L"มัก", L"มั่งมี", L"มัธยม", L"มัน", L"มั่นคง", L"มา", L"มาก", L"มากมาย", L"มารดา", L"ม้า", L"มิตร", L"มิถุนายน", L"มีนาคม", L"มี", L"มีด", L"มุ่งหมาย", L"เมฆ", L"เมตตา", L"เมล็ด", L"เมษายน", L"เมา", L"เมีย", L"เมื่อ", L"เมือง", L"แมลง", L"แม่", L"แม่น้ำ", L"แม้", L"ไม่", L"ไมตรี", L"ยก", L"ยนต์", L"ยอด", L"ยอม", L"ย่อม", L"ยัง", L"ยา", L"ยาก", L"ยาม", L"ย้าย", L"ยาว", L"ยุง", L"ยุติธรรม", L"ยุโรป", L"ยิง", L"ยิ่ง", L"ยืน", L"เย็น", L"แยก", L"แย่ง", L"โยน", L"รถ", L"รบ", L"ร่ม", L"รวม", L"ร่วม", L"รอง", L"ร้อง", L"ร้อน", L"รอบ", L"รอย", L"ร้อย", L"ระงับ", L"ระเบียบ", L"ระลึก", L"ระยะ", L"ระหว่าง", L"รัก", L"รักษา", L"รัง", L"รัฐบาล", L"รับ", L"รับประทาน", L"ราก", L"ราคา", L"รางวัล", L"ราชการ", L"ราชา", L"ราง", L"ร่าง", L"ร้าน", L"ราษฎร", L"ราย", L"ร้าย", L"ราว", L"ริม", L"รีบ", L"รุ่ง", L"รู", L"รู้", L"รู้จัก", L"รู้สึก", L"รูป", L"เรา", L"เริ่ม", L"เรียก", L"เรียง", L"เรียน", L"เรียบร้อย", L"เรือ", L"เรื่อง", L"เรือน", L"แรก", L"แรง", L"โรค", L"โรง", L"โรงเรียน", L"ไร", L"ไร่", L"ไร้", L"ลง", L"ลด", L"ลบ", L"ลม", L"ล้ม", L"ลวง", L"ล่วง", L"ล้วน", L"ลอง", L"ลอย", L"ละ", L"ละคร", L"ละลาย", L"ละออง", L"ละเอียด", L"ลัก", L"ลักษณะ", L"ลา", L"ลาก", L"ล่าง", L"ล้าง", L"ล้าน", L"ลำดับ", L"ลำบาก", L"ลำพัง", L"ลิ้น", L"ลึก", L"ลืม", L"ลุก", L"ลูก", L"ลุง", L"เล็ก", L"เล่น", L"เล่ม", L"เลว", L"เล่า", L"เลิก", L"เลี้ยง", L"เลี้ยว", L"เลือก", L"เลื่อน", L"เลื่อม", L"แล", L"แล้ง", L"แล่น", L"แล้ว", L"และ", L"โลก", L"วงศ์", L"วัง", L"วัด", L"วัน", L"วัว", L"วาง", L"ว่า", L"ว่าง", L"วาจา", L"วาน", L"วิ่ง", L"วิชชา", L"วิทยา", L"วิธี", L"เวทนา", L"เว้น", L"เวลา", L"ศุกร์", L"ศาสนา", L"ส่ง", L"สงคราม", L"สงสัย", L"สงสาร", L"สด", L"สตรี", L"สตางค์", L"สติ", L"สถาน", L"สถานี", L"สนาม", L"สนุก", L"สภา", L"สม", L"สมควร", L"สมเด็จ", L"สมบัติ", L"สมมุติ", L"สมัย", L"สมาชิก", L"สมาคม", L"สรรเสริญ", L"สระ", L"สร้าง", L"สละ", L"สลึง", L"สว่าง", L"สวน", L"สวม", L"สวรรค์", L"สอง", L"ส่อง", L"สอน", L"สหาย", L"สะดวก", L"สะบาย", L"สะพาน", L"สะอาด", L"สัก", L"สั่ง", L"สังเกต", L"สัญญา", L"สัตว์", L"สั้น", L"สัมมา", L"สาม", L"สามารถ", L"สามี", L"สาย", L"สาว", L"สำนัก", L"สำราญ", L"สำเร็จ", L"สำหรับ", L"สิงหาคม", L"สิ่ง", L"สิน", L"สินค้า", L"สิบ", L"สี", L"สี่", L"สืบ", L"สุข", L"สู", L"สู้", L"สูง", L"เสด็จ", L"เสนาบดี", L"เส้น", L"เสมอ", L"เสมียน", L"เสร็จ", L"เสีย", L"เสียง", L"เสือ", L"เสื้อ", L"เสื่อม", L"เสาร์", L"แสง", L"แสดง", L"แสวง", L"ใส", L"ใส่", L"ไส", L"หก", L"หญ้า", L"หญิง", L"หนทาง", L"หน่อย", L"หนัก", L"หนัง", L"หนังสือ", L"หนา", L"หน้า", L"หน้าที่", L"หนาว", L"หนี", L"หนี้", L"หนึ่ง", L"หนุ่ม", L"ห่ม", L"หมด", L"หม้อ", L"หมอก", L"หมา", L"หมาย", L"หมุน", L"หมู", L"หมู่", L"หยด", L"หยาบ", L"หยิม", L"หยุด", L"หรือ", L"หลวง", L"หลัก", L"หลัง", L"หลับ", L"หลาน", L"หลาย", L"หลีก", L"หลุม", L"หวัง", L"หวาน", L"ห้อง", L"หัก", L"หัด", L"หัตถ์", L"หัน", L"หัว", L"หา", L"หาก", L"หาง", L"ห่าง", L"ห้าม", L"เหลน", L"แหละ", L"ให้", L"ใหญ่", L"ใหม่", L"ไหน", L"ไหม", L"ไหม้", L"ไหล", L"อก", L"อด", L"อธิบาย", L"อนุญาต", L"อยาก", L"อย่าง", L"อยู่", L"อริยะ", L"อวด", L"ออก", L"อ่อน", L"ออม", L"อะไร", L"อักษร", L"อังกฤษ", L"อังคาร", L"อัตรา", L"อัน", L"อันตราย", L"อากาศ", L"อ้าง", L"อาจ", L"อาณาเขต", L"อาณาจักร", L"อาทิตย์", L"อ่าน", L"อาบน้ำ", L"อาศัย", L"อาหาร", L"อำนาจ", L"อำเภอ", L"อีก", L"อื่น", L"อุ้ม", L"อุสสาหะ", L"เอก", L"เอง", L"เอา", L"เอื้อเฟื้อ", L"โอ", L"โอกาส", L"โอรส"
    };
    
    for(unsigned int i=0; i<sizeof(thaiDict)/sizeof(wchar_t*); i++){
        if(isThaiEqual(pText, thaiDict[i]))
            return true;
    }
    
    return false;
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
    while (*pText) {
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
		if ((iMaxWidth > 0 && iCurXCursor + RSHIFT6(face->glyph->metrics.width) > iMaxWidth)) {
//			buildLine(ss, face , iCurXCursor, cLastCh);
//
//			iCurXCursor = -RSHIFT6(face->glyph->metrics.horiBearingX);
            
            // #HLP_BEGIN
            // HLP: Word warp
            unsigned int backCount = 0;
            bool canWrap = true;
            int iCurXCursorBeforeTestWrap = iCurXCursor;
            const wchar_t* pTextBeforeTestWrap = pText;
            
            while(1){
                cLastCh = *pText;
                wchar_t pTextBefore = *(pText-1);
                
                if(cLastCh == ' ' ||
                   cLastCh == '\t'
                   ){
                    pText++;
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

	buildLine(ss,face, iCurXCursor, cLastCh);

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

	if (eAlignMask == CCImage::kAlignCenter) {
		iRet = (m_iMaxLineWidth - m_vLines[iLineIndex].iLineWidth) / 2 - RSHIFT6(face->glyph->metrics.horiBearingX );

	} else if (eAlignMask == CCImage::kAlignRight) {
		iRet = (m_iMaxLineWidth - m_vLines[iLineIndex].iLineWidth) - RSHIFT6(face->glyph->metrics.horiBearingX );
	} else {
		// left or other situation
		iRet = -RSHIFT6(face->glyph->metrics.horiBearingX );
	}
	return iRet;
}

int BitmapDC::openFont(const std::string& fontName, uint fontSize)
{
	FT_Face aFace ;

	int iError = 0 ;
	if( m_fontName != basename(fontName) || m_fontSize != fontSize ) {
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

FT_UInt BitmapDC::thaiAdjust(FT_UInt current_index, FT_UInt prev_index, FT_UInt next_index){
    // 3 space
    //  ั 77 normal 17 left
    //  ็ xx normal 18 left
    //  ิ 80 normal 19 left
    //  ี 81 normal 20 left
    //  ึ 82 normal 21 left
    //  ื 83 normal 22 left
    // ำ 79
    //  ่ 98 topright 23 topleft 5 lowleft 10 lowright
    //  ้ 99 topright 24 topleft 6 lowleft 11 lowright
    //  ๊ 100 topright 25 topleft 7 lowleft 12 lowright
    //  ๋ 101 topright 26 topleft 8 lowleft 13 lowright
    //  ์ 102 topright 27 topleft 9 lowleft 14 lowright
    // ป ฝ ฟ ฬ 55 57 59 72
    
    
    
    FT_UInt glyph_index = current_index;
    
    //CCLog("ddddd %i", glyph_index);
    
    // low
    if(prev_index != 77
       && prev_index != 80
       && prev_index != 81
       && prev_index != 82
       && prev_index != 83
       && prev_index != 19
       && prev_index != 20
       && prev_index != 21
       && prev_index != 22
       
       && next_index != 79){
        
        
        if(prev_index == 55 || prev_index == 57 || prev_index == 59 || prev_index == 72){
            // low left
            if(current_index == 98)
                glyph_index = 5;
            else if(current_index == 99)
                glyph_index = 6;
            else if(current_index == 100)
                glyph_index = 7;
            else if(current_index == 101)
                glyph_index = 8;
            else if(current_index == 102)
                glyph_index = 9;
            else if(current_index == 80)
                glyph_index = 19;
            else if(current_index == 81)
                glyph_index = 20;
            else if(current_index == 82)
                glyph_index = 21;
            else if(current_index == 83)
                glyph_index = 22;
        }else{
            // low
            if(current_index == 98)
                glyph_index = 10;
            else if(current_index == 99)
                glyph_index = 11;
            else if(current_index == 100)
                glyph_index = 12;
            else if(current_index == 101)
                glyph_index = 13;
            else if(current_index == 102)
                glyph_index = 14;
            
        }
        
    }else{
        
        if((prev_index != 77
            &&prev_index != 80
            && prev_index != 81
            && prev_index != 82
            && prev_index != 83)
           ||
           (prev_index == 19
            || prev_index == 20
            || prev_index == 21
            || prev_index == 22)) {
               
               // top left
               if(current_index == 98)
                   glyph_index = 23;
               else if(current_index == 99)
                   glyph_index = 24;
               else if(current_index == 100)
                   glyph_index = 25;
               else if(current_index == 101)
                   glyph_index = 26;
               else if(current_index == 102)
                   glyph_index = 27;
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
					//CCLog("No valid font, use default %s\n", fName.c_str());
					iError = openFont( fName, fontSize );
				}
			}
			CC_BREAK_IF(iError);

			//select utf8 charmap
			iError = FT_Select_Charmap(m_face,FT_ENCODING_UNICODE);
			CC_BREAK_IF(iError);

			iError = FT_Set_Pixel_Sizes(m_face, fontSize,fontSize);
			CC_BREAK_IF(iError);
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
        if(fixLineHeight && ((eAlignMask & CCImage::kAlignBottomRight)
           || (eAlignMask & CCImage::kAlignBottom)
           || (eAlignMask & CCImage::kAlignBottomLeft))){
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
                glyph_index = thaiAdjust(current_index, prev_index, next_index);
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

				FT_Pos horiBearingYPixels = RSHIFT6(m_face->glyph->metrics.horiBearingY) ;
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
            bRet = _initWithRawData(pData, nDataLen, nWidth, nHeight, nBitsPerComponent);
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
                             int             fixLineHeight/* = 0*/)
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
        CC_BREAK_IF(! dc.getBitmap(pText, nWidth, nHeight, eAlignMask, fullFontName.c_str(), nSize, fixLineHeight));
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

bool CCImage::_initWithRawData(void * pData, int nDatalen, int nWidth, int nHeight, int nBitsPerComponent)
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
	// todo
	return false;
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
