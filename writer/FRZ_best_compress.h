//FRZ_best_compress.h
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
#ifndef _FRZ_BEST_COMPRESS_H_
#define _FRZ_BEST_COMPRESS_H_
#include <map>
#include <assert.h>
#include "../reader/FRZ_decompress_base.h"
#include "FRZ_private/suffix_string.h"

class TFRZCode_base{
public:
    inline explicit TFRZCode_base(int zip_parameter)
    :m_src(0),m_src_end(0),m_zip_parameter(zip_parameter){ }
    inline int zip_parameter()const{ return m_zip_parameter; }
    inline const TFRZ_Byte* src_begin()const { return m_src; }
    inline const TFRZ_Byte* src_end()const { return m_src_end; }
    
    virtual void pushDataInit(const TFRZ_Byte* src,const TFRZ_Byte* src_end){
        m_src=src;
        m_src_end=src_end;
    }
    
    virtual void pushNoZipData(TFRZ_Int32 nozipBegin,TFRZ_Int32 nozipEnd)=0;
    virtual void pushZipData(TFRZ_Int32 curPos,TFRZ_Int32 matchPos,TFRZ_Int32 matchLength)=0;
    
    virtual int getMinMatchLength()const=0;
    virtual int getMinZipBitLength()const{ return getZipBitLength(getMinMatchLength())-1; }//最少要压缩的bit数.
    virtual int getZipBitLength(int matchLength,TFRZ_Int32 curString=-1,TFRZ_Int32 matchString=-1)const=0;
    virtual int getZipParameterForBestUncompressSpeed()const=0;
    virtual int getNozipLengthOutBitLength(int nozipLength)const=0;
private:
    const TFRZ_Byte* m_src;
    const TFRZ_Byte* m_src_end;
    int m_zip_parameter;
};

class TFRZBestZiper{
public:
    TFRZBestZiper(const TFRZ_Byte* src,const TFRZ_Byte* src_end);
    const TFRZ_Byte* getCode(TFRZCode_base& out_FRZCode,const TFRZ_Byte* src_cur,int kCanNotZipLength);
    
    static int compress_limitMemery_get_compress_step_count(int allCanUseMemrey_MB,int srcDataSize) {
        const int kSpace_O=10;
        const double allCanUseMemrey=allCanUseMemrey_MB*(1024.0*1024);
        if (allCanUseMemrey>(srcDataSize*(kSpace_O+2)))
            return 1;
        assert(allCanUseMemrey>=srcDataSize*2.5);
        int result=(int)(0.9+kSpace_O/(allCanUseMemrey/(srcDataSize+1)-2));
        if (result<1) result=1;
        return result;
    }
    
    static void compress_by_step(TFRZCode_base& out_FRZCode,int compress_step_count,const unsigned char* src,const unsigned char* src_end);
private:
    TSuffixString m_sstring;
    //std::map<int,int> m_forwardOffsert_memcache;
    const TFRZ_Byte* createCode(TFRZCode_base& out_FRZCode,const TFRZ_Byte* src_cur,int kCanNotZipLength);
    void _getBestMatch(TFRZCode_base& out_FRZCode,TSuffixIndex curString,TFRZ_Int32& curBestZipBitLength,TFRZ_Int32& curBestMatchString,TFRZ_Int32& curBestMatchLength,int it_inc,int kMaxForwardOffsert);
    
    bool getBestMatch(TFRZCode_base& out_FRZCode,TSuffixIndex curString,TFRZ_Int32* out_curBestMatchLength,TFRZ_Int32* out_curBestMatchPos,TFRZ_Int32* out_curBestZipBitLength,int nozipBegin,int endString);
    
};


typedef std::vector<TFRZ_Byte> TFRZ_Buffer;

//变长32bit正整数编码方案(x bit额外类型标志位,x<=3),从高位开始输出1-5byte:
// x0*  7-x bit
// x1* 0*  7+7-x bit
// x1* 1* 0*  7+7+7-x bit
// x1* 1* 1* 0*  7+7+7+7-x bit
// x1* 1* 1* 1* 0*  7+7+7+7+7-x bit
static void pack32BitWithTag(TFRZ_Buffer& out_code,TFRZ_UInt32 iValue,int highBit,const int kTagBit){//写入并前进指针.
    const int kMaxPack32BitTagBit=3;
    assert((0<=kTagBit)&&(kTagBit<=kMaxPack32BitTagBit));
    assert((highBit>>kTagBit)==0);
    const int kMaxPack32BitSize=5;
    const unsigned int kMaxValueWithTag=(1<<(7-kTagBit))-1;
    
    TFRZ_Byte codeBuf[kMaxPack32BitSize];
    TFRZ_Byte* codeEnd=codeBuf;
    while (iValue>kMaxValueWithTag) {
        *codeEnd=iValue&((1<<7)-1); ++codeEnd;
        iValue>>=7;
    }
    out_code.push_back( (highBit<<(8-kTagBit)) | iValue | (((codeBuf!=codeEnd)?1:0)<<(7-kTagBit)));
    while (codeBuf!=codeEnd) {
        --codeEnd;
        out_code.push_back((*codeEnd) | (((codeBuf!=codeEnd)?1:0)<<7));
    }
}

inline static int pack32BitWithTagOutSize(TFRZ_UInt32 iValue,int kTagBit){//返回pack后字节大小.
    const unsigned int kMaxValueWithTag=(1<<(7-kTagBit))-1;
    int result=0;
    while (iValue>kMaxValueWithTag) {
        ++result;
        iValue>>=7;
    }
    return (result+1);
}

static inline void pack32Bit(TFRZ_Buffer& out_code,TFRZ_UInt32 iValue){
    pack32BitWithTag(out_code, iValue, 0, 0);
}
static inline int pack32BitOutSize(TFRZ_UInt32 iValue){
    return pack32BitWithTagOutSize(iValue, 0);
}

#endif //_FRZ_BEST_COMPRESS_H_
