//  FRZ1_decompress.h
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
#ifndef _FRZ1_DECOMPRESS_H_ 
#define _FRZ1_DECOMPRESS_H_
#include "FRZ_decompress_base.h"

#ifdef __cplusplus
extern "C" {
#endif
    
enum TFRZ1CodeType{
    kFRZ1CodeType_nozip = 0,    //0表示后面存的未压缩数据 (包中连续储存多个字节数据)
    kFRZ1CodeType_zip   = 1     //1表示后面存的压缩(替代)数据.
};
static const int kFRZ1CodeType_bit=1;


frz_BOOL FRZ1_decompress(unsigned char* out_data,unsigned char* out_data_end,
                         const unsigned char* frz1_code,const unsigned char* frz1_code_end);
    
static inline frz_BOOL FRZ1_decompress_windows(const unsigned char* data_windows,unsigned char* out_data,unsigned char* out_data_end,
                                               const unsigned char* frz1_code,const unsigned char* frz1_code_end){
    return FRZ1_decompress(out_data,out_data_end,frz1_code,frz1_code_end); }

frz_BOOL FRZ1_decompress_windows_safe(const unsigned char* data_windows,unsigned char* out_data,unsigned char* out_data_end,
                                      const unsigned char* frz1_code,const unsigned char* frz1_code_end);

static inline frz_BOOL FRZ1_decompress_safe(unsigned char* out_data,unsigned char* out_data_end,
                                            const unsigned char* frz1_code,const unsigned char* frz1_code_end){
        return FRZ1_decompress_windows_safe(out_data,out_data,out_data_end,frz1_code,frz1_code_end); }

#ifdef __cplusplus
}
#endif

#endif //_FRZ1_DECOMPRESS_H_
