//FRZ_best_compress.cpp
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
#include "FRZ_best_compress.h"

namespace {
    static const int _kMaxForwardOffsert_zip_parameter_table_size=8+1;
    static const int _kMaxForwardOffsert_zip_parameter_table[_kMaxForwardOffsert_zip_parameter_table_size]={
        960*1024,840*1024,720*1024,600*1024, 500*1024, //0..4
        420*1024,340*1024,300*1024,280*1024//5..8
    };
    static const int _kMaxForwardOffsert_zip_parameter_table_minValue=150*1024;
}

TFRZBestZiper::TFRZBestZiper(const TFRZ_Byte* src,const TFRZ_Byte* src_end)
  :m_sstring((const char*)src,(const char*)src_end){
}

const TFRZ_Byte* TFRZBestZiper::getCode(TFRZCode_base& out_FRZCode,const TFRZ_Byte* src_cur,int kcanNotZipLength){
    m_sstring.R_create();
    m_sstring.LCPLite_create_withR();
    const TFRZ_Byte* result=createCode(out_FRZCode,src_cur,kcanNotZipLength);
    m_sstring.LCPLite.clear();
    m_sstring.R.clear();
    return result;
}

const TFRZ_Byte* TFRZBestZiper::createCode(TFRZCode_base& out_FRZCode,const TFRZ_Byte* src_cur,int kcanNotZipLength){
    out_FRZCode.pushDataInit((const TFRZ_Byte*)m_sstring.ssbegin,(const TFRZ_Byte*)m_sstring.ssend);
    const int sstrSize=(int)m_sstring.size();
    
    TFRZ_Int32 nozipBegin=(TFRZ_Int32)(src_cur-(const TFRZ_Byte*)m_sstring.ssbegin);
    TFRZ_Int32 curIndex=nozipBegin+1;
    while (curIndex<sstrSize-(kcanNotZipLength>>1)) {
        TFRZ_Int32 matchLength;
        TFRZ_Int32 matchPos;
        TFRZ_Int32 zipBitLength;
        if (getBestMatch(out_FRZCode,curIndex,&matchLength,&matchPos,&zipBitLength,nozipBegin,sstrSize)){
            if (curIndex!=nozipBegin){//out no zip data
                out_FRZCode.pushNoZipData(nozipBegin,curIndex);
            }
            out_FRZCode.pushZipData(curIndex,matchPos,matchLength);
            
            curIndex+=matchLength;
            assert(curIndex<=sstrSize);
            nozipBegin=curIndex;
        }else{
            ++curIndex;
        }
    }
    if (nozipBegin<sstrSize){
        //剩余的可以交给下一次处理.
        if (sstrSize-nozipBegin<=kcanNotZipLength){
            return (const TFRZ_Byte*)m_sstring.ssbegin+nozipBegin;
        }else{
            out_FRZCode.pushNoZipData(nozipBegin,(TFRZ_Int32)sstrSize-(kcanNotZipLength>>1));
            return (const TFRZ_Byte*)m_sstring.ssend-(kcanNotZipLength>>1);
        }
    }else{
        return (const TFRZ_Byte*)m_sstring.ssend; //finish
    }
}

void TFRZBestZiper::_getBestMatch(TFRZCode_base& out_FRZCode,TSuffixIndex curString,TFRZ_Int32& curBestZipBitLength,TFRZ_Int32& curBestMatchString,TFRZ_Int32& curBestMatchLength,int it_inc,int kMaxForwardOffsert){
    //const TFRZ_Int32 it_cur=m_sstring.lower_bound(m_sstring.ssbegin+curString,m_sstring.ssend);
    const TFRZ_Int32 it_cur=m_sstring.lower_bound_withR(curString);//查找curString自己的位置.
    int it=it_cur+it_inc;
    int it_end;
    const TSuffixString::TUShort* LCP;//当前的后缀字符串和下一个后缀字符串的相等长度.
    if (it_inc==1){
        it_end=(int)m_sstring.size();
        LCP=&m_sstring.LCPLite[it_cur];
    }else{
        assert(it_inc==-1);
        it_end=-1;
        LCP=&m_sstring.LCPLite[it_cur]-1;
    }
    
    const int kMaxValue_lcp=((TFRZ_UInt32)1<<31)-1;
    const int kMinZipLoseBitLength=8*out_FRZCode.getMinMatchLength()-out_FRZCode.getZipBitLength(out_FRZCode.getMinMatchLength());
    const int kMaxSearchDeepSize=1024*4;//加大可以提高一点压缩率,但可能降低压缩速度.
    int lcp=kMaxValue_lcp;
    for (int deep=kMaxSearchDeepSize;(deep>0)&&(it!=it_end);it+=it_inc,LCP+=it_inc,--deep){
        int curLCP=*LCP;
        if (curLCP<lcp){
            lcp=curLCP;
            if (lcp*8<curBestZipBitLength+kMinZipLoseBitLength)//不可能压缩了.
                break;
        }
        
        TSuffixIndex matchString=m_sstring.SA[it];
        const int curForwardOffsert=(curString-matchString);
        if (curForwardOffsert>0){
            --deep;
            TFRZ_Int32 zipedBitLength=out_FRZCode.getZipBitLength(lcp,curString,matchString);
            if (curForwardOffsert>kMaxForwardOffsert){//惩罚.
                zipedBitLength-=8+4;
                if (curForwardOffsert>kMaxForwardOffsert*2){
                    zipedBitLength-=4*8+4;
                    if (curForwardOffsert>kMaxForwardOffsert*4)
                        zipedBitLength-=8*8+4;
                }
            }
            if (zipedBitLength<curBestZipBitLength) continue;
            
            if((zipedBitLength>curBestZipBitLength) ||(matchString>curBestMatchString)){
                   deep-=(kMaxSearchDeepSize/64);
                   curBestZipBitLength=zipedBitLength;
                   curBestMatchString=matchString;
                   curBestMatchLength=lcp;
               }
        }
    }
}

bool TFRZBestZiper::getBestMatch(TFRZCode_base& out_FRZCode,TSuffixIndex curString,TFRZ_Int32* out_curBestMatchLength,TFRZ_Int32* out_curBestMatchPos,TFRZ_Int32* out_curBestZipBitLength,int nozipBegin,int endString){
    int kMaxForwardOffsert;//增大可以提高压缩率但可能会减慢解压速度(缓存命中降低).
    const int zip_parameter=out_FRZCode.zip_parameter();
    const int kS=_kMaxForwardOffsert_zip_parameter_table_size; //todo:参数比例不同时,不准确.
    const int kFRZ_bestUncompressSpeed=out_FRZCode.getZipParameterForBestUncompressSpeed();
    assert(kFRZ_bestUncompressSpeed>kS);
    if (zip_parameter<kS){
        kMaxForwardOffsert=_kMaxForwardOffsert_zip_parameter_table[zip_parameter];
    }else{
        const int kMax=_kMaxForwardOffsert_zip_parameter_table[kS-1];
        const int kMin=_kMaxForwardOffsert_zip_parameter_table_minValue;
        if (zip_parameter>=kFRZ_bestUncompressSpeed)
            kMaxForwardOffsert=kMin;
        else
            kMaxForwardOffsert=kMax-(kMax-kMin)*(zip_parameter-kS)/(kFRZ_bestUncompressSpeed-kS);
    }
    
    const int noZipLength=curString-nozipBegin;
    //   pksize(noZipLength)+noZipLength
    // + pksize(zipLength)+pksize(ForwardOffsertInfo)
    // + pksize(allLength-noZipLength-zipLength)+ allLength-noZipLength-zipLength
    // < pksize(allLength)+allLength + zip_parameter
    //~=> zipLength - pksize(zipLength)-pksize(ForwardOffsertInfo) > zip_parameter + pksize(noZipLength)
    int minZipBitLength=out_FRZCode.getZipBitLength(out_FRZCode.getMinMatchLength())-1;//最少要压缩的bit数.
    if (noZipLength>0)
        minZipBitLength+=out_FRZCode.getNozipLengthOutBitLength(1);
    if (minZipBitLength<=0) minZipBitLength=1;
    
    *out_curBestZipBitLength=minZipBitLength;
    *out_curBestMatchPos=-1;
    *out_curBestMatchLength=0;
    _getBestMatch(out_FRZCode,curString,*out_curBestZipBitLength,*out_curBestMatchPos,*out_curBestMatchLength,1,kMaxForwardOffsert);
    _getBestMatch(out_FRZCode,curString,*out_curBestZipBitLength,*out_curBestMatchPos,*out_curBestMatchLength,-1,kMaxForwardOffsert);
    
    return (((*out_curBestMatchPos)>=0)&&((*out_curBestMatchLength)>=out_FRZCode.getMinMatchLength()));
}



void TFRZBestZiper::compress_by_step(TFRZCode_base& out_FRZCode,int compress_step_count,const unsigned char* src,const unsigned char* src_end){
    assert(src_end-src<=(((unsigned int)1<<31)-1));
    assert(compress_step_count>=1);
    const int stepMemSize=(int)((src_end-src+compress_step_count-1)/compress_step_count);
    assert((stepMemSize>0)||(src_end==src));
    
    const int kLookupFrontLength=2*1024*1024;
    const int kLookupBackLength=2*1024;
    int lookupFrontLength=kLookupFrontLength;
    if (lookupFrontLength*4>stepMemSize)
        lookupFrontLength=(stepMemSize>>4);
    
    const unsigned char* cur_src=src;
    const unsigned char* cur_src_end;
    for (int i=0;i<compress_step_count;++i) {
        cur_src_end=src+(i+1)*stepMemSize+kLookupBackLength;
        int lookupBackLength=kLookupBackLength;
        if (cur_src_end>src_end){
            cur_src_end=src_end;
            lookupBackLength=0;
        }
        const unsigned char* match_src=cur_src;
        if (match_src-src>lookupFrontLength)
            match_src-=lookupFrontLength;
        TFRZBestZiper FRZBestZiper(match_src,cur_src_end);
        cur_src=FRZBestZiper.getCode(out_FRZCode,cur_src,lookupBackLength);
    }
    assert(cur_src==src_end);
}
