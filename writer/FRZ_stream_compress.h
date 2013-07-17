//FRZ_compress.h
//快速解压的一个通用字节流压缩算法.
/*
 Copyright (c) 2012-2013 HouSisong All Rights Reserved.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef _FRZ_STREAM_COMPRESS_H_
#define _FRZ_STREAM_COMPRESS_H_

typedef void (*TFRZ_write_code_proc)(void* callBackData,const unsigned char* code,const unsigned char* code_end);
typedef void* TFRZ_stream_compress_handle;

TFRZ_stream_compress_handle FRZ1_stream_compress_best_create(int zip_parameter,int maxDecompressWindowsSize,
                                                             TFRZ_write_code_proc out_code_callBack,void* callBackData,int maxStepMemorySize);
TFRZ_stream_compress_handle FRZ2_stream_compress_best_create(int zip_parameter,int maxDecompressWindowsSize,
                                                             TFRZ_write_code_proc out_code_callBack,void* callBackData,int maxStepMemorySize);

void FRZ_stream_compress_append_data(TFRZ_stream_compress_handle handle,
                                      const unsigned char* src,const unsigned char* src_end,bool isAppendDataFinish=false);
void FRZ_stream_compress_append_data_finish(TFRZ_stream_compress_handle handle);
void FRZ_stream_compress_delete(TFRZ_stream_compress_handle handle);


#endif //_FRZ_STREAM_COMPRESS_H_