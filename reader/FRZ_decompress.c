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
#if defined(_MSC_VER) && (!defined(__cplusplus))
#define inline __inline
#endif

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
inline static void memcpy_order_end(TFRZ_Byte* dstEnd,const TFRZ_Byte* srcEnd,TFRZ_Int32 length){
    memcpy_order(dstEnd-length,srcEnd-length,length);
}

#ifdef __cplusplus
extern "C" {
#endif
    
    typedef struct TFRZ_data_16Bit {
    #ifndef FRZ_DECOMPRESS_MEM_NOTMUST_ALIGN
        TFRZ_UInt16     _data;
    #else
        unsigned char   _data[2];
    #endif
    } TFRZ_data_16Bit;
    
    typedef struct TFRZ_data_32Bit {
    #ifndef FRZ_DECOMPRESS_MEM_NOTMUST_ALIGN
        TFRZ_UInt32     _data;
    #else
        unsigned char   _data[4];
    #endif
    } TFRZ_data_32Bit;
    
    typedef struct TFRZ_data_64Bit {
    #ifndef FRZ_DECOMPRESS_MEM_NOTMUST_ALIGN
        void*           _data[8/sizeof(void*)];
    #else
        unsigned char   _data[8];
    #endif
    } TFRZ_data_64Bit;
    
    //static char _private_assert_TFRZ_data_64Bit_size8[1-2*(sizeof(struct TFRZ_data_64Bit)!=8)];
    //static char _private_assert_TFRZ_data_32Bit_size8[1-2*(sizeof(struct TFRZ_data_32Bit)!=4)];
    //static char _private_assert_TFRZ_data_16Bit_size8[1-2*(sizeof(struct TFRZ_data_16Bit)!=2)];

#define _memcpy_tiny_COPYBYTE(i,d,s,IType) *(IType*)(d-i) = *(const IType*)(s-i)
#define _memcpy_tiny_COPY_BYTE_8(i,d,s)  _memcpy_tiny_COPYBYTE(i,d,s,struct TFRZ_data_64Bit)
#define _memcpy_tiny_COPY_BYTE_4(i,d,s)  _memcpy_tiny_COPYBYTE(i,d,s,struct TFRZ_data_32Bit)
#define _memcpy_tiny_COPY_BYTE_2(i,d,s)  _memcpy_tiny_COPYBYTE(i,d,s,struct TFRZ_data_16Bit)
#define _memcpy_tiny_COPY_BYTE_1(i,d,s)  _memcpy_tiny_COPYBYTE(i,d,s,TFRZ_Byte)

#define _memcpy_tiny_case_COPY_BYTE_8(i,d,s)  case i: _memcpy_tiny_COPY_BYTE_8(i,d,s)
#define _memcpy_tiny_case_COPY_BYTE_4(i,d,s)  case i: _memcpy_tiny_COPY_BYTE_4(i,d,s)
#define _memcpy_tiny_case_COPY_BYTE_2(i,d,s)  case i: _memcpy_tiny_COPY_BYTE_2(i,d,s)
#define _memcpy_tiny_case_COPY_BYTE_1(i,d,s)  case i: _memcpy_tiny_COPY_BYTE_1(i,d,s)

#ifdef __cplusplus
}
#endif


#define memcpy_tiny64_end(d,s,len){             \
    switch(len){                                \
        _memcpy_tiny_case_COPY_BYTE_8(64,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(56,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(48,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(40,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(32,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(24,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(16,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(8,d,s);   \
        break;                                  \
        _memcpy_tiny_case_COPY_BYTE_8(63,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(55,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(47,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(39,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(31,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(23,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(15,d,s);  \
          _memcpy_tiny_COPY_BYTE_8(8,d,s);      \
        break;                                  \
        _memcpy_tiny_case_COPY_BYTE_4(7,d,s);   \
          _memcpy_tiny_COPY_BYTE_4(4,d,s);      \
        break;                                  \
        _memcpy_tiny_case_COPY_BYTE_8(62,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(54,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(46,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(38,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(30,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(22,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(14,d,s);  \
          _memcpy_tiny_COPY_BYTE_8(8,d,s);      \
        break;                                  \
        _memcpy_tiny_case_COPY_BYTE_4(6,d,s);   \
          _memcpy_tiny_COPY_BYTE_2(2,d,s);      \
        break;                                  \
        _memcpy_tiny_case_COPY_BYTE_8(61,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(53,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(45,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(37,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(29,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(21,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(13,d,s);  \
          _memcpy_tiny_COPY_BYTE_8(8,d,s);      \
        break;                                  \
        _memcpy_tiny_case_COPY_BYTE_4(5,d,s);   \
          _memcpy_tiny_COPY_BYTE_1(1,d,s);      \
        break;                                  \
        _memcpy_tiny_case_COPY_BYTE_8(60,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(52,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(44,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(36,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(28,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(20,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(12,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_4(4,d,s);   \
        break;                                  \
        _memcpy_tiny_case_COPY_BYTE_8(59,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(51,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(43,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(35,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(27,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(19,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(11,d,s);  \
          _memcpy_tiny_COPY_BYTE_4(4,d,s);      \
        break;                                  \
        _memcpy_tiny_case_COPY_BYTE_2(3,d,s);   \
          _memcpy_tiny_COPY_BYTE_1(1,d,s);      \
        break;                                  \
        _memcpy_tiny_case_COPY_BYTE_8(58,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(50,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(42,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(34,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(26,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(18,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(10,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_2(2,d,s);   \
        break;                                  \
        _memcpy_tiny_case_COPY_BYTE_8(57,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(49,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(41,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(33,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(25,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(17,d,s);  \
        _memcpy_tiny_case_COPY_BYTE_8(9,d,s);   \
        _memcpy_tiny_case_COPY_BYTE_1(1,d,s);   \
        break;                                  \
        default:{                               \
            memcpy(d-len,s-len,len);            \
        } break;                                \
    }                                           \
}


#define _PRIVATE_FRZ_DECOMPRESS_NEED_INCLUDE_CODE

//for FRZ*_decompress
#   define _PRIVATE_FRZ1_DECOMPRESS_NAME                FRZ1_decompress
#   define _PRIVATE_FRZ2_DECOMPRESS_NAME                _beta_FRZ2_decompress
#   define _PRIVATE_FRZ_unpack32BitWithTag_NAME         unpack32BitWithTag
#   define _PRIVATE_FRZ_unpack32BitWithHalfByte_NAME    unpack32BitWithHalfByte
#       include "FRZ1_decompress_inc.c"
#       include "FRZ2_decompress_inc.c"
#   undef  _PRIVATE_FRZ_unpack32BitWithTag_NAME
#   undef  _PRIVATE_FRZ_unpack32BitWithHalfByte_NAME
#   undef  _PRIVATE_FRZ2_DECOMPRESS_NAME
#   undef  _PRIVATE_FRZ1_DECOMPRESS_NAME

//for FRZ*_decompress_safe
#   define _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
#   define _PRIVATE_FRZ1_DECOMPRESS_NAME                FRZ1_decompress_safe
#   define _PRIVATE_FRZ2_DECOMPRESS_NAME                _beta_FRZ2_decompress_safe
#   define _PRIVATE_FRZ_unpack32BitWithTag_NAME         unpack32BitWithTag_safe
#   define _PRIVATE_FRZ_unpack32BitWithHalfByte_NAME    unpack32BitWithHalfByte_safe
#       include "FRZ1_decompress_inc.c"
#       include "FRZ2_decompress_inc.c"
#   undef  _PRIVATE_FRZ_unpack32BitWithHalfByte_NAME
#   undef  _PRIVATE_FRZ_unpack32BitWithTag_NAME
#   undef  _PRIVATE_FRZ2_DECOMPRESS_NAME
#   undef  _PRIVATE_FRZ1_DECOMPRESS_NAME
#   undef  _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK

#undef  _PRIVATE_FRZ_DECOMPRESS_NEED_INCLUDE_CODE
