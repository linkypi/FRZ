//FRZ2_compress.h
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
#ifndef _FRZ2_COMPRESS_H_
#define _FRZ2_COMPRESS_H_

#include <vector>
#include <assert.h>
//FRZ2比FRZ1压缩率略大,但解码速度也变慢;建议一般只用在要求FRZ1-0无法达到的更大压缩率的场景.
//提示:FRZ2编码输出格式和FRZ1格式不兼容;FRZ2编码输出格式和FRZ2_stream格式不完全兼容;.

enum TFRZ2_zip_parameter{ kFRZ2_bestSize=0, kFRZ2_default=4, kFRZ2_bestUncompressSpeed=16 };
//zip_parameter: 增大该值,则压缩率变小,解压稍快  0时，压缩率最大.

//压缩; 压缩后的数据放到out_code后面; 需要压缩的数据大小(src_end-src)<2G.
void FRZ2_compress(std::vector<unsigned char>& out_code,
                   const unsigned char* src,const unsigned char* src_end,int zip_parameter);

//工具函数:按可用的最大内存计算比较合适的compress_step_count值.
int FRZ2_compress_limitMemery_get_compress_step_count(int allCanUseMemrey_MB,int srcDataSize);

//节约内存的压缩; 如果调用FRZ2_compress时内存不够,可以改掉用FRZ2_compress_limitMemery,解码器不变; 函数实现时分成compress_step_count个块分别压缩.
void FRZ2_compress_limitMemery(int compress_step_count,std::vector<unsigned char>& out_code,
                               const unsigned char* src,const unsigned char* src_end,int zip_parameter);

//如果要压缩(解压)的数据超大!装不到内存,那么你可以选择自己分多次处理数据(分别调用FRZ2_compress,解压时也需要自己分多次调用FRZ2_decompress),后面的FRZ2_stream_compress*函数就是一个类似的包装.


typedef void (*TFRZ_write_code_proc)(void* callBackData,const unsigned char* code,const unsigned char* code_end);
typedef void* TFRZ2_stream_compress_handle;

TFRZ2_stream_compress_handle FRZ2_stream_compress_create(int zip_parameter,int maxDecompressWindowsSize,
                                                         TFRZ_write_code_proc out_code_callBack,void* callBackData,int maxStepMemorySize);
void FRZ2_stream_compress_append_data(TFRZ2_stream_compress_handle handle,
                                      const unsigned char* src,const unsigned char* src_end,bool isAppendDataFinish=false);
void FRZ2_stream_compress_append_data_finish(TFRZ2_stream_compress_handle handle);
void FRZ2_stream_compress_delete(TFRZ2_stream_compress_handle handle);


#endif //_FRZ2_COMPRESS_H_