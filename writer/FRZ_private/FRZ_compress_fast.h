//FRZ_compress_fast.h
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
#ifndef _FRZ_COMPRESS_FAST_H_
#define _FRZ_COMPRESS_FAST_H_
#include "FRZ_compress_base.h"

class TFRZCompressFast:public TFRZCompressBase{
public:
    TFRZCompressFast();
    virtual void createCode_step(TFRZCodeBase& out_FRZCode,const TFRZ_Byte* src_windows,const TFRZ_Byte* src_cur,const TFRZ_Byte* src_end);
protected:
    virtual bool getBestMatch(TFRZCodeBase& out_FRZCode,TFRZ_Int32 curString,TFRZ_Int32* out_curBestMatchPos,TFRZ_Int32* out_curBestMatchLength,TFRZ_Int32 nozipBegin);
private:
    int m_bestForwardOffsert;
};

#endif //_FRZ_COMPRESS_FAST_H_
