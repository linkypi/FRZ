//FRZ_compress_best.cpp
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
#include "FRZ_compress_base.h"

void TFRZCompressBase::createCode_step(TFRZCodeBase& out_FRZCode,const TFRZ_Byte* src_windows,const TFRZ_Byte* src_cur,const TFRZ_Byte* src_end){
    out_FRZCode.pushDataInit(src_windows,src_cur,src_end);
    const int allDataSize=(int)(src_end-src_windows);
    
    TFRZ_Int32 nozipBegin=(TFRZ_Int32)(src_cur-src_windows);
    TFRZ_Int32 curIndex=nozipBegin+1;
    while (curIndex<allDataSize) {
        TFRZ_Int32 matchPos;
        TFRZ_Int32 matchLength;
        if (getBestMatch(out_FRZCode,curIndex,&matchPos,&matchLength,nozipBegin)){
            if (curIndex!=nozipBegin){//out no zip data
                out_FRZCode.pushNoZipData(nozipBegin,curIndex);
            }
            out_FRZCode.pushZipData(curIndex,matchPos,matchLength);
            
            curIndex+=matchLength;
            assert(curIndex<=allDataSize);
            nozipBegin=curIndex;
        }else{
            ++curIndex;
        }
    }
    if (nozipBegin<allDataSize)
        out_FRZCode.pushNoZipData(nozipBegin,(TFRZ_Int32)allDataSize);
}


void TFRZCompressBase::compress_by_step(TFRZCodeBase& out_FRZCode,TFRZCompressBase& FRZCompress,int compress_step_count,const unsigned char* src,const unsigned char* src_end){
    assert(src_end-src<=(((unsigned int)1<<31)-1));
    assert(compress_step_count>=1);
    const int stepMemSize=(int)((src_end-src+compress_step_count-1)/compress_step_count);
    assert((stepMemSize>0)||(src_end==src));
    
    const int kLookupFrontLength=2*1024*1024;
    int lookupFrontLength=kLookupFrontLength;
    if (lookupFrontLength*4>stepMemSize)
        lookupFrontLength=(stepMemSize>>4);
    
    const unsigned char* cur_src=src;
    const unsigned char* cur_src_end;
    for (int i=0;i<compress_step_count;++i) {
        cur_src_end=src+(i+1)*stepMemSize;
        if (cur_src_end>src_end)
            cur_src_end=src_end;
        const unsigned char* match_src=cur_src;
        if (match_src-src>lookupFrontLength)
            match_src-=lookupFrontLength;
        FRZCompress.createCode_step(out_FRZCode,match_src,cur_src,cur_src_end);
        cur_src=cur_src_end;
    }
    assert(cur_src==src_end);
}
