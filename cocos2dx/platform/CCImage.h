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

#ifndef __CC_IMAGE_H__
#define __CC_IMAGE_H__

#include "cocoa/CCObject.h"

NS_CC_BEGIN


// #HLP_BEGIN

#define THAI_DICT const wchar_t* thaiDict[] = {\
    L"ก็", L"กฎหมาย", L"กร", L"กรกฎาคม", L"กรม", L"กรรม", L"กรรมการ", L"กระจาย", L"กระดาษ", L"กระด้าง", L"กระดูก", L"กระทรวง", L"กระทำ", L"กระนั้น", L"กระมัง", L"กระไร", L"กรุง", L"กรุงเทพฯ", L"กลม", L"กลับ", L"กลาง", L"กล่าว", L"กล้า", L"กลัว", L"เกลา", L"เกล้า", L"กว่า", L"กว้าง", L"กษัตริย์", L"กสิกรรม", L"ก่อ", L"ก่อน", L"ก้อน", L"กอบ", L"กะ", L"กะดาน", L"กัน", L"กันยายน", L"กับ", L"กำไร", L"กำลัง", L"กำหนด", L"กา", L"กาง", L"กาชาด", L"กาย", L"การ", L"เกาะ", L"กิจ", L"กิน", L"กิริยา", L"กิเลส", L"กี่", L"กุมภาพันธ์", L"กุมาร", L"กู้", L"เก็บ", L"เกรง", L"เกณฑ์", L"เกวียน", L"เก่า", L"เก้า", L"เก้าอี้", L"เกิด", L"เกิน", L"เกียรติยศ", L"เกียจคร้าน", L"เกี่ยว", L"เกี่ยวข้อง", L"เกือบ", L"แก", L"แก่", L"แก้", L"แก้ว", L"แกะ", L"โกร่ง", L"โกรธ", L"ใกล้", L"ไก่", L"ไกล", L"ขด", L"ขน", L"ขนาด", L"ขณะ", L"ขยาย", L"ขวาง", L"ขว้าง", L"ขอ", L"ขอรับ", L"ของ", L"ขอบ", L"ข้อ", L"ขา", L"ขาด", L"ขาย", L"ขาว", L"ข่าว", L"ข้า", L"ข้าง", L"ข้าพเจ้า", L"ข้าม", L"ข้าราชการ", L"ข้าศึก", L"ขัง", L"ขัด", L"ขัน", L"ขับ", L"เขตต์", L"ขีด", L"ขี่", L"ขึ้น", L"เขา", L"เข้า", L"เขียน", L"เขียว", L"แขก", L"แข็ง", L"แข่ง", L"ไข", L"ไข่", L"ไข้", L"คอ", L"คอย", L"ค่อย", L"คับ", L"ค่า", L"ค้า", L"คำ", L"ค่ำ", L"คิด", L"คืน", L"คือ", L"คุณ", L"คู", L"คูณ", L"คดี", L"คน", L"ค้น", L"คณะ", L"ครึ่ง", L"ครั้ง", L"ครั้น", L"ครบ", L"ครอบครัว", L"ครอง", L"คราว", L"ครู", L"คลอง", L"คลาย", L"คลุม", L"ความ", L"ควร", L"ควาย", L"เคย", L"เคียง", L"เครื่อง", L"เคลื่อน", L"เคือง", L"โคม", L"ใคร", L"ใคร่", L"งอก", L"งาน", L"งาม", L"ง่าย", L"งู", L"เงิน", L"โง่", L"จง", L"จด", L"จดหมาย", L"จน", L"จบ", L"จม", L"จาก", L"จ่าย", L"จ้าง", L"จะ", L"จัก", L"จักร", L"จัง", L"จังหวัด", L"จัด", L"จันทร์", L"จับ", L"จำ", L"จำนวน", L"จำเป็น", L"จำพวก", L"จิตต์", L"จริง", L"จริต", L"จีน", L"จึ่ง", L"เจ็บ", L"เจริญ", L"เจ็ด", L"เจ้า", L"เจาะ", L"แจง", L"แจ้ง", L"แจ่ม", L"ใจ", L"โจทย์", L"ฉลาด", L"ฉัน", L"ฉะนั้น", L"ฉะนี้", L"ฉะบับ", L"ฉะเพาะ", L"ไฉน", L"ชน", L"ชม", L"ชอบ", L"ชวน", L"ช่วย", L"ช่อง", L"ชะนวน", L"ชะนะ", L"ชะนิด", L"ชัก", L"ชัด", L"ชั่ว", L"ชั่วโมง", L"ชั้น", L"ชาย", L"ชาว", L"ชา", L"ช้า", L"ช่าง", L"ช้าง", L"ชาติ", L"ชำนาญ", L"ชิง", L"ชีวิต", L"ชีพ", L"ชี้", L"ชื่อ", L"ชม", L"เช่น", L"เช้า", L"เชิญ", L"เชื่อ", L"เชื้อ", L"ใช่", L"ใช้", L"ซ่อน", L"ซ้าย", L"ซ้ำ", L"ซิ", L"ซีก", L"ซึ่ง", L"ซื่อ", L"ซื้อ", L"ญาติ", L"ญี่ปุ่น", L"ดง", L"ดวง", L"ด้วย", L"ดอก", L"ดัง", L"ดับ", L"ดาว", L"ด้าน", L"ดำ", L"ดำเนิน", L"ดำริ", L"ดิน", L"ดี", L"ดื่ม", L"ดุจ", L"ดู", L"เด็ก", L"เดิน", L"เดิม", L"เดียว", L"เดี่ยว", L"เดี๋ยว", L"เดี๋ยวนี้", L"เดือด", L"เดือน", L"แดง", L"แดด", L"แดน", L"โดย", L"ใด", L"ได้", L"ตก", L"ตน", L"ต้น", L"ตรง", L"ตรวจ", L"ตระกูล", L"ตรัส", L"ตรา", L"ตลอด", L"ตลาด", L"ตะวัน", L"ตอน", L"ต่อ", L"ต้อง", L"ต้อน", L"ตอบ", L"ตะเกียง", L"ตำ", L"ต่ำ", L"ตำบล", L"ตำแหน่ง", L"ตัว", L"ตัวอย่าง", L"ตัด", L"ตัดสิน", L"ตั้ง", L"ตา", L"ตาน", L"ตาม", L"ตาย", L"ตี", L"ติด", L"ตึก", L"เต็ม", L"เตรียม", L"เตือน", L"แต่", L"แตก", L"แต่ง", L"ตุลาคม", L"ตู้", L"โต", L"ใต้", L"ไต่", L"ถนน", L"ถวาย", L"ถอย", L"ถ้อย", L"ถั่ว", L"ถ้า", L"ถาน", L"ถาม", L"ถูก", L"เถิด", L"เถียง", L"แถว", L"ไถ", L"ทดลอง", L"ทน", L"ทรง", L"ทราบ", L"ทรัพย์", L"ทราบ", L"ทวี", L"ทวีป", L"ทหาร", L"ท่อ", L"ทอง", L"ท้อง", L"ทอด", L"ทะเล", L"ทัน", L"ทับ", L"ทัพ", L"ทั่ว", L"ทั้ง", L"ทั้งปวง", L"ทั้งหลาย", L"ทา", L"ทาง", L"ทาน", L"ท่าน", L"ทาบ", L"ทาย", L"ท้าย", L"ท้าว", L"ทำ", L"ทำไม", L"ทำลาย", L"ทิศ", L"ทิ้ง", L"ที", L"ทีเดียว", L"ที่", L"ทุก", L"ทุกข์", L"ทุง", L"ทูด", L"ทูน", L"เท", L"เท็จ", L"เทวดา", L"เท่า", L"เที่ยง", L"เทียบ", L"เทียม", L"เที่ยว", L"แทน", L"แทบ", L"แท้", L"โทษ", L"ไทย", L"ธง", L"ธันวาคม", L"ธรรม", L"ธรรมดา", L"ธรรมเนียม", L"ธาตุ", L"ธุระ", L"เธอ", L"นก", L"นอก", L"นอกจาก", L"นอน", L"น้อง", L"น้อย", L"นัก", L"นักปราชญ์", L"นักเรียน", L"นั่ง", L"นั่น", L"นั้น", L"นับ", L"นับถือ", L"นา", L"น่า", L"นาง", L"นาน", L"นาม", L"นาฬิกา", L"นำ", L"น้ำ", L"นิ่ง", L"นิด", L"นิทาน", L"นิยม", L"นิ้ว", L"นี่", L"นี้", L"นึก", L"นุ่ง", L"เนื่อง", L"เนื้อ", L"แนะนำ", L"แน่", L"แน่น", L"โน้น", L"ใน", L"บก", L"บท", L"บน", L"บรรณาธิการ", L"บรรดา", L"บริบูรณ์", L"บริษัท", L"บริสุทธิ์", L"บอก", L"บ่อย", L"บัง", L"บังเกิด", L"บังคับ", L"บัดนี้", L"บัญชา", L"บัญญัติ", L"บาง", L"บางที", L"บ้าง", L"บาท", L"บาน", L"บ้าน", L"บ่าย", L"บำรุง", L"บิดา", L"บุคคล", L"บุตร", L"บุตรี", L"บุญ", L"บุรุษ", L"บุหรี่", L"เบี้ย", L"เบื้อง", L"แบ่ง", L"แบน", L"โบราณ", L"ใบ", L"ปกครอง", L"ปฏิบัติ", L"ปกติ", L"ปรกติ", L"ประกอบ", L"ประการ", L"ประกาศ", L"ประจำ", L"ประชุม", L"ประตู", L"ประเทศ", L"ประพฤติ", L"ประมาณ", L"ประโยชน์", L"ประสงค์", L"ประเสริฐ", L"ประหลาด", L"ปรากฏ", L"ปรารถนา", L"ปราบปราม", L"ปราศจาก", L"ปรัดยุบัน", L"ปลา", L"ปลาบ", L"ปล่อย", L"ปลูก", L"ปวง", L"ป้องกัน", L"ปั้น", L"ปัญญา", L"ปัญหา", L"ป่า", L"ปาก", L"ปิด", L"ปี", L"เป็น", L"เป็ด", L"เปรียบ", L"เปล่า", L"เปลี่ยน", L"เปลือก", L"เปลือง", L"แปด", L"แปลก", L"แปลง", L"โปรด", L"ไป", L"ไปรษณีย์", L"ผล", L"ผะสม", L"ผ้า", L"ผ่าน", L"ผิด", L"ผิว", L"ผู้", L"ผูก", L"เผย", L"เผื่อ", L"แผ่", L"แผน", L"แผ่น", L"แผ่นดิน", L"ฝน", L"ฝรั่ง", L"ฝัก", L"ฝัง", L"ฝา", L"ฝาก", L"ฝ่าย", L"ฝึก", L"ฝึกหัด", L"พงศาวดาร", L"พ้น", L"พบ", L"พม่า", L"พยายาม", L"พรวน", L"พร้อม", L"พระ", L"พระยา", L"พราหมณ์", L"พฤศจิกายน", L"พฤศภาคม", L"พฤหัสบดี", L"พล", L"พลเมือง", L"พลอย", L"พลิก", L"พวก", L"พอ", L"พ่อ", L"พอก", L"พ้อง", L"พยาน", L"พัก", L"พัน", L"พันธ์", L"พา", L"พาณิชย์", L"พาย", L"พาล", L"พาหนะ", L"พิจารณา", L"พิธี", L"พิมพ์", L"พิษ", L"พิเศษ", L"พี่", L"พึง", L"พึ่ง", L"พืช", L"พื้น", L"พุทธ", L"พุธ", L"พูด", L"เพาะ", L"เพราะ", L"เพลิง", L"เพลิน", L"เพียง", L"เพียงไร", L"เพียน", L"เพิ่ม", L"เพื่อ", L"เพื่อน", L"แพง", L"แพ้", L"ฟ้อง", L"ฟัก", L"ฟัง", L"ฟัน", L"ฟ้า", L"ไฟ", L"ภรรยา", L"ภัย", L"ภาค", L"ภาพ", L"ภาย", L"ภายนอก", L"ภายใน", L"ภายหลัง", L"ภาษิต", L"ภาษี", L"ภูเขา", L"ภูม", L"มกราคม", L"มณฑล", L"มนตรี", L"มนุษย์", L"มอง", L"มอญ", L"มอบ", L"มัก", L"มั่งมี", L"มัธยม", L"มัน", L"มั่นคง", L"มา", L"มาก", L"มากมาย", L"มารดา", L"ม้า", L"มิตร", L"มิถุนายน", L"มีนาคม", L"มี", L"มีด", L"มุ่งหมาย", L"เมฆ", L"เมตตา", L"เมล็ด", L"เมษายน", L"เมา", L"เมีย", L"เมื่อ", L"เมือง", L"แมลง", L"แม่", L"แม่น้ำ", L"แม้", L"ไม่", L"ไมตรี", L"ยก", L"ยนต์", L"ยอด", L"ยอม", L"ย่อม", L"ยัง", L"ยา", L"ยาก", L"ยาม", L"ย้าย", L"ยาว", L"ยุง", L"ยุติธรรม", L"ยุโรป", L"ยิง", L"ยิ่ง", L"ยืน", L"เย็น", L"แยก", L"แย่ง", L"โยน", L"รถ", L"รบ", L"ร่ม", L"รวม", L"ร่วม", L"รอง", L"ร้อง", L"ร้อน", L"รอบ", L"รอย", L"ร้อย", L"ระงับ", L"ระเบียบ", L"ระลึก", L"ระยะ", L"ระหว่าง", L"รัก", L"รักษา", L"รัง", L"รัฐบาล", L"รับ", L"รับประทาน", L"ราก", L"ราคา", L"รางวัล", L"ราชการ", L"ราชา", L"ราง", L"ร่าง", L"ร้าน", L"ราษฎร", L"ราย", L"ร้าย", L"ราว", L"ริม", L"รีบ", L"รุ่ง", L"รู", L"รู้", L"รู้จัก", L"รู้สึก", L"รูป", L"เรา", L"เริ่ม", L"เรียก", L"เรียง", L"เรียน", L"เรียบร้อย", L"เรือ", L"เรื่อง", L"เรือน", L"แรก", L"แรง", L"โรค", L"โรง", L"โรงเรียน", L"ไร", L"ไร่", L"ไร้", L"ลง", L"ลด", L"ลบ", L"ลม", L"ล้ม", L"ลวง", L"ล่วง", L"ล้วน", L"ลอง", L"ลอย", L"ละ", L"ละคร", L"ละลาย", L"ละออง", L"ละเอียด", L"ลัก", L"ลักษณะ", L"ลา", L"ลาก", L"ล่าง", L"ล้าง", L"ล้าน", L"ลำดับ", L"ลำบาก", L"ลำพัง", L"ลิ้น", L"ลึก", L"ลืม", L"ลุก", L"ลูก", L"ลุง", L"เล็ก", L"เล่น", L"เล่ม", L"เลว", L"เล่า", L"เลิก", L"เลี้ยง", L"เลี้ยว", L"เลือก", L"เลื่อน", L"เลื่อม", L"แล", L"แล้ง", L"แล่น", L"แล้ว", L"และ", L"โลก", L"วงศ์", L"วัง", L"วัด", L"วัน", L"วัว", L"วาง", L"ว่า", L"ว่าง", L"วาจา", L"วาน", L"วิ่ง", L"วิชชา", L"วิทยา", L"วิธี", L"เวทนา", L"เว้น", L"เวลา", L"ศุกร์", L"ศาสนา", L"ส่ง", L"สงคราม", L"สงสัย", L"สงสาร", L"สด", L"สตรี", L"สตางค์", L"สติ", L"สถาน", L"สถานี", L"สนาม", L"สนุก", L"สภา", L"สม", L"สมควร", L"สมเด็จ", L"สมบัติ", L"สมมุติ", L"สมัย", L"สมาชิก", L"สมาคม", L"สรรเสริญ", L"สระ", L"สร้าง", L"สละ", L"สลึง", L"สว่าง", L"สวน", L"สวม", L"สวรรค์", L"สอง", L"ส่อง", L"สอน", L"สหาย", L"สะดวก", L"สะบาย", L"สะพาน", L"สะอาด", L"สัก", L"สั่ง", L"สังเกต", L"สัญญา", L"สัตว์", L"สั้น", L"สัมมา", L"สาม", L"สามารถ", L"สามี", L"สาย", L"สาว", L"สำนัก", L"สำราญ", L"สำเร็จ", L"สำหรับ", L"สิงหาคม", L"สิ่ง", L"สิน", L"สินค้า", L"สิบ", L"สี", L"สี่", L"สืบ", L"สุข", L"สู", L"สู้", L"สูง", L"เสด็จ", L"เสนาบดี", L"เส้น", L"เสมอ", L"เสมียน", L"เสร็จ", L"เสีย", L"เสียง", L"เสือ", L"เสื้อ", L"เสื่อม", L"เสาร์", L"แสง", L"แสดง", L"แสวง", L"ใส", L"ใส่", L"ไส", L"หก", L"หญ้า", L"หญิง", L"หนทาง", L"หน่อย", L"หนัก", L"หนัง", L"หนังสือ", L"หนา", L"หน้า", L"หน้าที่", L"หนาว", L"หนี", L"หนี้", L"หนึ่ง", L"หนุ่ม", L"ห่ม", L"หมด", L"หม้อ", L"หมอก", L"หมา", L"หมาย", L"หมุน", L"หมู", L"หมู่", L"หยด", L"หยาบ", L"หยิม", L"หยุด", L"หรือ", L"หลวง", L"หลัก", L"หลัง", L"หลับ", L"หลาน", L"หลาย", L"หลีก", L"หลุม", L"หวัง", L"หวาน", L"ห้อง", L"หัก", L"หัด", L"หัตถ์", L"หัน", L"หัว", L"หา", L"หาก", L"หาง", L"ห่าง", L"ห้าง", L"ห้าม", L"เหลน", L"แหละ", L"ให้", L"ใหญ่", L"ใหม่", L"ไหน", L"ไหม", L"ไหม้", L"ไหล", L"อก", L"อด", L"อธิบาย", L"อนุญาต", L"อยาก", L"อย่าง", L"อยู่", L"อริยะ", L"อวด", L"ออก", L"อ่อน", L"ออม", L"อะไร", L"อักษร", L"อังกฤษ", L"อังคาร", L"อัตรา", L"อัน", L"อันตราย", L"อากาศ", L"อ้าง", L"อาจ", L"อาณาเขต", L"อาณาจักร", L"อาทิตย์", L"อ่าน", L"อาบน้ำ", L"อาศัย", L"อาหาร", L"อำนาจ", L"อำเภอ", L"อีก", L"อื่น", L"อุ้ม", L"อุสสาหะ", L"เอก", L"เอง", L"เอา", L"เอื้อเฟื้อ", L"โอ", L"โอกาส", L"โอรส"\
};
// #HLP_END



/**
 * @addtogroup platform
 * @{
 */

class CC_DLL CCImage : public CCObject
{
public:
    CCImage();
    ~CCImage();

    typedef enum
    {
        kFmtJpg = 0,
        kFmtPng,
        kFmtTiff,
        kFmtWebp,
        kFmtRawData,
        kFmtUnKnown
    }EImageFormat;

    typedef enum
    {
        kAlignCenter        = 0x33, ///< Horizontal center and vertical center.
        kAlignTop           = 0x13, ///< Horizontal center and vertical top.
        kAlignTopRight      = 0x12, ///< Horizontal right and vertical top.
        kAlignRight         = 0x32, ///< Horizontal right and vertical center.
        kAlignBottomRight   = 0x22, ///< Horizontal right and vertical bottom.
        kAlignBottom        = 0x23, ///< Horizontal center and vertical bottom.
        kAlignBottomLeft    = 0x21, ///< Horizontal left and vertical bottom.
        kAlignLeft          = 0x31, ///< Horizontal left and vertical center.
        kAlignTopLeft       = 0x11, ///< Horizontal left and vertical top.
    }ETextAlign;
    
    /**
    @brief  Load the image from the specified path. 
    @param strPath   the absolute file path.
    @param imageType the type of image, currently only supporting two types.
    @return  true if loaded correctly.
    */
    bool initWithImageFile(const char * strPath, EImageFormat imageType = kFmtPng);

    /*
     @brief The same result as with initWithImageFile, but thread safe. It is caused by
            loadImage() in CCTextureCache.cpp.
     @param fullpath  full path of the file.
     @param imageType the type of image, currently only supporting two types.
     @return  true if loaded correctly.
     */
    bool initWithImageFileThreadSafe(const char *fullpath, EImageFormat imageType = kFmtPng);

    /**
    @brief  Load image from stream buffer.

    @warning kFmtRawData only supports RGBA8888.
    @param pBuffer  stream buffer which holds the image data.
    @param nLength  data length expressed in (number of) bytes.
    @param nWidth, nHeight, nBitsPerComponent are used for kFmtRawData.
    @return true if loaded correctly.
    */
    bool initWithImageData(void * pData, 
                           int nDataLen, 
                           EImageFormat eFmt = kFmtUnKnown,
                           int nWidth = 0,
                           int nHeight = 0,
                           int nBitsPerComponent = 8);

    /**
    @brief    Create image with specified string.
    @param  pText       the text the image will show (cannot be nil).
    @param  nWidth      the image width, if 0, the width will match the text's width.
    @param  nHeight     the image height, if 0, the height will match the text's height.
    @param  eAlignMask  the test Alignment
    @param  pFontName   the name of the font used to draw the text. If nil, use the default system font.
    @param  nSize       the font size, if 0, use the system default size.
    */
    bool initWithString(
        const char *    pText, 
        int             nWidth = 0, 
        int             nHeight = 0,
        ETextAlign      eAlignMask = kAlignCenter,
        const char *    pFontName = 0,
//        int             nSize = 0);
// #HLP_BEGIN
        int             nSize = 0,
        int             fixLineHeight = 0,
        unsigned int    maxLine = 0
                        );
    
    int mCursorX;
    int mCursorY;
// #HLP_END
    
    #if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    
        bool initWithStringShadowStroke(
                                            const char *    pText,
                                            int             nWidth      = 0,
                                            int             nHeight     = 0,
                                            ETextAlign      eAlignMask  = kAlignCenter,
                                            const char *    pFontName   = 0,
                                            int             nSize       = 0,
                                            float           textTintR   = 1,
                                            float           textTintG   = 1,
                                            float           textTintB   = 1,
                                            bool shadow                 = false,
                                            float shadowOffsetX         = 0.0,
                                            float shadowOffsetY         = 0.0,
                                            float shadowOpacity         = 0.0,
                                            float shadowBlur            = 0.0,
                                            bool  stroke                =  false,
                                            float strokeR               = 1,
                                            float strokeG               = 1,
                                            float strokeB               = 1,
                                            float strokeSize            = 1
                                        
                                        );
    
    #endif
    

    unsigned char *   getData()               { return m_pData; }
    int               getDataLen()            { return m_nWidth * m_nHeight; }


    bool hasAlpha()                     { return m_bHasAlpha;   }
    bool isPremultipliedAlpha()         { return m_bPreMulti;   }


    /**
    @brief    Save CCImage data to the specified file, with specified format.
    @param    pszFilePath        the file's absolute path, including file suffix.
    @param    bIsToRGB        whether the image is saved as RGB format.
    */
    bool saveToFile(const char *pszFilePath, bool bIsToRGB = true);

    CC_SYNTHESIZE_READONLY(unsigned short,   m_nWidth,       Width);
    CC_SYNTHESIZE_READONLY(unsigned short,   m_nHeight,      Height);
    CC_SYNTHESIZE_READONLY(int,     m_nBitsPerComponent,   BitsPerComponent);

protected:
    bool _initWithJpgData(void *pData, int nDatalen);
    bool _initWithPngData(void *pData, int nDatalen);
    bool _initWithTiffData(void *pData, int nDataLen);
    bool _initWithWebpData(void *pData, int nDataLen);
    // @warning kFmtRawData only support RGBA8888
    bool _initWithRawData(void *pData, int nDatalen, int nWidth, int nHeight, int nBitsPerComponent, bool bPreMulti);

    bool _saveImageToPNG(const char *pszFilePath, bool bIsToRGB = true);
    bool _saveImageToJPG(const char *pszFilePath);

    unsigned char *m_pData;
    bool m_bHasAlpha;
    bool m_bPreMulti;


private:
    // noncopyable
    CCImage(const CCImage&    rImg);
    CCImage & operator=(const CCImage&);
};

// end of platform group
/// @}

NS_CC_END

#endif    // __CC_IMAGE_H__
