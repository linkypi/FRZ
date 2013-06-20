//  FRZ_decompress.c
/*
 Copyright (c) 2012-2013 HouSisong All Rights Reserved.
 (The MIT License)
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 */
#include "FRZ1_decompress.h"
#include "FRZ2_decompress.h"
#include "string.h" //memcpy
#include "assert.h" //assert

//按顺序拷贝内存数据.
inline static void memcpy_order(TFRZ_Byte* dst,const TFRZ_Byte* src,TFRZ_UInt32 length){
    TFRZ_UInt32 length_fast,i;
    
    length_fast=length&(~3);
    for (i=0;i<length_fast;i+=4){
        dst[i  ]=src[i  ];
        dst[i+1]=src[i+1];
        dst[i+2]=src[i+2];
        dst[i+3]=src[i+3];
    }
    for (;i<length;++i)
        dst[i]=src[i];
}
#ifdef __cplusplus
extern "C" {
#endif

typedef struct TFRZ_data_64Bit {
#ifndef FRZ_DECOMPRESS_MEM_NOTMUST_ALIGN
   void* _data[8/sizeof(void*)];
#else
    unsigned char _data[8/sizeof(unsigned char)];
#endif
} TFRZ_data_64Bit;

//static char _private_assert_TFRZ_data_64Bit_size8[1-2*(sizeof(struct TFRZ_data_64Bit)!=8)];
    
#ifdef __cplusplus
}
#endif

#define _memcpy_tiny_COPYBYTE(i,IType) *(IType*)(d-i) = *(const IType*)(s-i)
//#define _memcpy_tiny_COPYBYTE(i,IType) *(IType*)(dst+(i-sizeof(IType))) = *(const IType*)(src+(i-sizeof(IType)))
#define _memcpy_tiny_COPY_BYTE_8(i) _memcpy_tiny_COPYBYTE(i,struct TFRZ_data_64Bit)
#define _memcpy_tiny_COPY_BYTE_4(i) _memcpy_tiny_COPYBYTE(i,TFRZ_UInt32)
#define _memcpy_tiny_COPY_BYTE_2(i) _memcpy_tiny_COPYBYTE(i,TFRZ_UInt16)
#define _memcpy_tiny_COPY_BYTE_1(i) _memcpy_tiny_COPYBYTE(i,TFRZ_Byte)

#define _memcpy_tiny_case_COPY_BYTE_8(i) case i: _memcpy_tiny_COPY_BYTE_8(i)
#define _memcpy_tiny_case_COPY_BYTE_4(i) case i: _memcpy_tiny_COPY_BYTE_4(i)
#define _memcpy_tiny_case_COPY_BYTE_2(i) case i: _memcpy_tiny_COPY_BYTE_2(i)
#define _memcpy_tiny_case_COPY_BYTE_1(i) case i: _memcpy_tiny_COPY_BYTE_1(i)

static void memcpy_tiny(unsigned char* dst,const unsigned char* src,TFRZ_UInt32 len){
    if (len <= 64){
        register unsigned char *d =dst + len;
        register const unsigned char *s =src + len;
        switch(len){
            _memcpy_tiny_case_COPY_BYTE_8(64);
            _memcpy_tiny_case_COPY_BYTE_8(56);
            _memcpy_tiny_case_COPY_BYTE_8(48);
            _memcpy_tiny_case_COPY_BYTE_8(40);
            _memcpy_tiny_case_COPY_BYTE_8(32);
            _memcpy_tiny_case_COPY_BYTE_8(24);
            _memcpy_tiny_case_COPY_BYTE_8(16);
            _memcpy_tiny_case_COPY_BYTE_8(8);
            return;

            _memcpy_tiny_case_COPY_BYTE_8(63);
            _memcpy_tiny_case_COPY_BYTE_8(55);
            _memcpy_tiny_case_COPY_BYTE_8(47);
            _memcpy_tiny_case_COPY_BYTE_8(39);
            _memcpy_tiny_case_COPY_BYTE_8(31);
            _memcpy_tiny_case_COPY_BYTE_8(23);
            _memcpy_tiny_case_COPY_BYTE_8(15);
              _memcpy_tiny_COPY_BYTE_8(8);
              return;
              case 7:
                _memcpy_tiny_COPY_BYTE_4(7);
                _memcpy_tiny_COPY_BYTE_4(4);
            return;
            
            _memcpy_tiny_case_COPY_BYTE_8(62);
            _memcpy_tiny_case_COPY_BYTE_8(54);
            _memcpy_tiny_case_COPY_BYTE_8(46);
            _memcpy_tiny_case_COPY_BYTE_8(38);
            _memcpy_tiny_case_COPY_BYTE_8(30);
            _memcpy_tiny_case_COPY_BYTE_8(22);
            _memcpy_tiny_case_COPY_BYTE_8(14);
              _memcpy_tiny_COPY_BYTE_8(8);
              return;
              case 6:
                _memcpy_tiny_COPY_BYTE_4(6);
                _memcpy_tiny_COPY_BYTE_2(2);
            return;
        
            _memcpy_tiny_case_COPY_BYTE_8(61);
            _memcpy_tiny_case_COPY_BYTE_8(53);
            _memcpy_tiny_case_COPY_BYTE_8(45);
            _memcpy_tiny_case_COPY_BYTE_8(37);
            _memcpy_tiny_case_COPY_BYTE_8(29);
            _memcpy_tiny_case_COPY_BYTE_8(21);
            _memcpy_tiny_case_COPY_BYTE_8(13);
              _memcpy_tiny_COPY_BYTE_8(8);
              return;
              case 5:
                _memcpy_tiny_COPY_BYTE_4(5);
                _memcpy_tiny_COPY_BYTE_1(1);
            return;
            
            _memcpy_tiny_case_COPY_BYTE_8(60);
            _memcpy_tiny_case_COPY_BYTE_8(52);
            _memcpy_tiny_case_COPY_BYTE_8(44);
            _memcpy_tiny_case_COPY_BYTE_8(36);
            _memcpy_tiny_case_COPY_BYTE_8(28);
            _memcpy_tiny_case_COPY_BYTE_8(20);
            _memcpy_tiny_case_COPY_BYTE_8(12);
            _memcpy_tiny_case_COPY_BYTE_4(4);
            return;
            
            _memcpy_tiny_case_COPY_BYTE_8(59);
            _memcpy_tiny_case_COPY_BYTE_8(51);
            _memcpy_tiny_case_COPY_BYTE_8(43);
            _memcpy_tiny_case_COPY_BYTE_8(35);
            _memcpy_tiny_case_COPY_BYTE_8(27);
            _memcpy_tiny_case_COPY_BYTE_8(19);
            _memcpy_tiny_case_COPY_BYTE_8(11);
              _memcpy_tiny_COPY_BYTE_4(4);
              return;
              case  3:
                _memcpy_tiny_COPY_BYTE_2(3);
                _memcpy_tiny_COPY_BYTE_1(1);
            return;
            
            _memcpy_tiny_case_COPY_BYTE_8(58);
            _memcpy_tiny_case_COPY_BYTE_8(50);
            _memcpy_tiny_case_COPY_BYTE_8(42);
            _memcpy_tiny_case_COPY_BYTE_8(34);
            _memcpy_tiny_case_COPY_BYTE_8(26);
            _memcpy_tiny_case_COPY_BYTE_8(18);
            _memcpy_tiny_case_COPY_BYTE_8(10);
            _memcpy_tiny_case_COPY_BYTE_2(2);
            return;
            
            _memcpy_tiny_case_COPY_BYTE_8(57);
            _memcpy_tiny_case_COPY_BYTE_8(49);
            _memcpy_tiny_case_COPY_BYTE_8(41);
            _memcpy_tiny_case_COPY_BYTE_8(33);
            _memcpy_tiny_case_COPY_BYTE_8(25);
            _memcpy_tiny_case_COPY_BYTE_8(17);
            _memcpy_tiny_case_COPY_BYTE_8(9);
            _memcpy_tiny_case_COPY_BYTE_1(1);
            return;
            
            //case 0:
            //    return;
        }
    }else{
        memcpy(dst, src, len);
    }
}

#define _PRIVATE_FRZ_DECOMPRESS_NEED_INCLUDE_CODE

//for FRZ*_decompress_safe
#   define _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
#   define _PRIVATE_FRZ1_DECOMPRESS_NAME                FRZ1_decompress_safe
#   define _PRIVATE_FRZ2_DECOMPRESS_NAME                FRZ2_decompress_safe
#   define _PRIVATE_FRZ_unpack32BitWithTag_NAME         unpack32BitWithTag_safe
#   define _PRIVATE_FRZ_unpack32BitWithHalfByte_NAME    unpack32BitWithHalfByte_safe
#       include "FRZ1_decompress_inc.c"
#       include "FRZ2_decompress_inc.c"
#   undef  _PRIVATE_FRZ_unpack32BitWithHalfByte_NAME
#   undef  _PRIVATE_FRZ_unpack32BitWithTag_NAME
#   undef  _PRIVATE_FRZ2_DECOMPRESS_NAME
#   undef  _PRIVATE_FRZ1_DECOMPRESS_NAME
#   undef  _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK

//for FRZ*_decompress
#   define _PRIVATE_FRZ1_DECOMPRESS_NAME                FRZ1_decompress
#   define _PRIVATE_FRZ2_DECOMPRESS_NAME                FRZ2_decompress
#   define _PRIVATE_FRZ_unpack32BitWithTag_NAME         unpack32BitWithTag
#   define _PRIVATE_FRZ_unpack32BitWithHalfByte_NAME    unpack32BitWithHalfByte
#       include "FRZ1_decompress_inc.c"
#       include "FRZ2_decompress_inc.c"
#   undef  _PRIVATE_FRZ_unpack32BitWithTag_NAME
#   undef  _PRIVATE_FRZ_unpack32BitWithHalfByte_NAME
#   undef  _PRIVATE_FRZ2_DECOMPRESS_NAME
#   undef  _PRIVATE_FRZ1_DECOMPRESS_NAME

#undef  _PRIVATE_FRZ_DECOMPRESS_NEED_INCLUDE_CODE
