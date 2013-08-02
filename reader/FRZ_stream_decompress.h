//  FRZ_stream_decompress.h
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
#ifndef _FRZ_STREAM_DECOMPRESS_H_
#define _FRZ_STREAM_DECOMPRESS_H_
#include <assert.h>
#include "FRZ_decompress_base.h"

#ifdef __cplusplus
extern "C" {
#endif

static const int kSize_of_FRZ_stream_head_size=1;

//decompress head size
static inline int FRZ_stream_decompress_head_size(const unsigned char* streamCode0,const unsigned char* streamCode1){
    assert(streamCode1-streamCode0==kSize_of_FRZ_stream_head_size); return *streamCode0;     }

struct TFRZ_stream_head{
    int size_of_FRZ_stream_head; //==sizeof(int)*3 or sizeof(int)*5
    int data_size;
    int frz_code_size;
    int data_windows_size;
    int data_max_step_size;
};
static inline void FRZ_stream_head_init(struct TFRZ_stream_head* head){
    head->size_of_FRZ_stream_head=sizeof(struct TFRZ_stream_head);
    head->data_size=0;
    head->frz_code_size=0;
    head->data_windows_size=0;
    head->data_max_step_size=0;
};

//decompress head
const unsigned char* FRZ_stream_decompress_head(const unsigned char* streamHeadCode,const unsigned char* streamHeadCodeEnd,struct TFRZ_stream_head* out_head);

//

#ifdef __cplusplus
}
#endif

#endif //_FRZ_STREAM_DECOMPRESS_H_