//  FRZ2_decompress.h
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
#ifndef _FRZ2_DECOMPRESS_H_ 
#define _FRZ2_DECOMPRESS_H_
#include "FRZ_decompress_base.h"

#ifdef __cplusplus
extern "C" {
#endif
    
enum TFRZ2CodeType{
    kFRZ2CodeType_nozip = 0,    //0表示后面存的未压缩数据 (包中连续储存多个字节数据)
    kFRZ2CodeType_zip   = 1     //1表示后面存的压缩(替代)数据.
};
static const int kFRZ2CodeType_bit=1;
    
static const int kMinMatchLength=3;


frz_BOOL FRZ2_decompress(unsigned char* out_data,unsigned char* out_data_end,
                         const unsigned char* frz2_code,const unsigned char* frz2_code_end);

static inline frz_BOOL FRZ2_decompress_windows(const unsigned char* data_windows,unsigned char* out_data,unsigned char* out_data_end,
                                                const unsigned char* frz2_code,const unsigned char* frz2_code_end){
    return FRZ2_decompress(out_data,out_data_end,frz2_code,frz2_code_end); }

frz_BOOL FRZ2_decompress_windows_safe(const unsigned char* data_windows,unsigned char* out_data,unsigned char* out_data_end,
                                      const unsigned char* frz2_code,const unsigned char* frz2_code_end);
    
static inline frz_BOOL FRZ2_decompress_safe(unsigned char* out_data,unsigned char* out_data_end,
                                            const unsigned char* frz2_code,const unsigned char* frz2_code_end){
    return FRZ2_decompress_windows_safe(out_data,out_data,out_data_end,frz2_code,frz2_code_end); }

#ifdef __cplusplus
}
#endif

#endif //_FRZ2_DECOMPRESS_H_
