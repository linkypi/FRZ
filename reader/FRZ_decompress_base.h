//  FRZ_decompress_base.h
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
#ifndef _FRZ_DECOMPRESS_BASE_H_ 
#define _FRZ_DECOMPRESS_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif
#   if defined(_MSC_VER) && (!defined(__cplusplus))
#       define inline __inline
#   endif

typedef unsigned char   TFRZ_Byte;
typedef unsigned short  TFRZ_UInt16;
typedef signed int      TFRZ_Int32;
typedef unsigned int    TFRZ_UInt32;

//#define FRZ_DECOMPRESS_MEM_NOTMUST_ALIGN
//当编译不足够聪明时,用来强制启用未对齐内存读写的优化:要求运行环境允许CPU读写未对齐内存(CPU平台是否可用和未对齐内存读写速度需要测试).
#ifndef FRZ_DECOMPRESS_MEM_NOTMUST_ALIGN
//x86 x64 powerpc 默认打开.
#  ifndef FRZ_DECOMPRESS_USE_MEMCPY_TINY__MEM_NOTMUST_ALIGN
#    if defined(__amd64__) || defined(__x86_64__) || defined(_M_AMD64)\
        || defined(__386__) || defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(_M_I386) || defined(__I86__) || defined(_I386)\
        || defined(__powerpc__) || defined(__powerpc) || defined(__ppc__) || defined(__PPC__) || defined(_M_PPC) || defined(_ARCH_PPC) || defined(_ARCH_PWR)
#       define FRZ_DECOMPRESS_MEM_NOTMUST_ALIGN
#    endif
#  endif
#endif
    
    
#define frz_BOOL    int
#define frz_FALSE   0
#define frz_TRUE    (!frz_FALSE)


#ifdef __cplusplus
}
#endif

#endif //_FRZ_DECOMPRESS_BASE_H_
