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
//!!!FRZ2还处于测试阶段!!!
//FRZ2比FRZ1压缩率略大,但解码速度也变慢;建议一般只用在要求FRZ1-0无法达到的更大压缩率的场景.
//提示:FRZ2编码输出格式和FRZ1格式不兼容.

enum TFRZ2_zip_parameter{ kFRZ2_bestSize=0, kFRZ2_default=4, kFRZ2_bestUncompressSpeed=32 };
//zip_parameter: 增大该值,则压缩率变小,解压稍快  0时，压缩率最大.

//压缩; 压缩后的数据放到out_code后面; 需要压缩的数据大小(src_end-src)<2G,否则请自己几次压缩.
void _beta_FRZ2_compress(std::vector<unsigned char>& out_code,
                   const unsigned char* src,const unsigned char* src_end,int zip_parameter=kFRZ2_default);

//工具函数:按可用的最大内存计算比较合适的compress_step_count值.
static inline int FRZ2_compress_limitMemery_get_compress_step_count(int allCanUseMemrey_MB,int srcDataSize) {
    assert(allCanUseMemrey_MB*(1024*1024/14)>=3*srcDataSize/14);
    int result=srcDataSize/(allCanUseMemrey_MB*(1024*1024/14)-2*srcDataSize/14);
    if (result<1) result=1;
    return result;
}

//节约内存的压缩; 如果调用FRZ2_compress时内存不够,可以改掉用FRZ2_compress_limitMemery,解码器不变; 函数实现时分成COMPRESS_STEP_COUNT个块分别压缩.
void _beta_FRZ2_compress_limitMemery(int compress_step_count,std::vector<unsigned char>& out_code,
                               const unsigned char* src,const unsigned char* src_end,int zip_parameter=kFRZ2_default);

//如果要压缩(解压)的数据更大,一次装不到内存,那么你可以自己分多次加载数据,分别调用FRZ2_compress,解压时也需要自己分多次调用FRZ2_decompress.

#endif //_FRZ2_COMPRESS_H_