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
        8*1024*1024, 6*1024*1024, 4*1024*1024, 2*1024*1024, 1*1024*1024, //0..4
        880*1024,780*1024,700*1024,600*1024//5..8
    };
    static const int _kMaxForwardOffsert_zip_parameter_table_minValue=200*1024;
}

TFRZBestZiper::TFRZBestZiper(TFRZCode_base& out_FRZCode,const TFRZ_Byte* src,const TFRZ_Byte* src_end)
:m_sstring((const char*)src,(const char*)src_end){
    m_sstring.R_create();
    m_sstring.LCP_create();
    createCode(out_FRZCode);
    m_sstring.R.clear();
    m_sstring.LCP.clear();
}
void TFRZBestZiper::createCode(TFRZCode_base& out_FRZCode){
    out_FRZCode.pushDataInit((const TFRZ_Byte*)m_sstring.ssbegin,(const TFRZ_Byte*)m_sstring.ssend);
    const int sstrSize=(int)m_sstring.size();
    
    TFRZ_Int32 nozipBegin=0;
    TFRZ_Int32 curIndex=1;
    while (curIndex<sstrSize) {
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
        out_FRZCode.pushNoZipData(nozipBegin,(TFRZ_Int32)sstrSize);
    }
}

void TFRZBestZiper::_getBestMatch(TFRZCode_base& out_FRZCode,TSuffixIndex curString,TFRZ_Int32& curBestZipBitLength,TFRZ_Int32& curBestMatchString,TFRZ_Int32& curBestMatchLength,int it_inc,int kMaxForwardOffsert){
    //const TFRZ_Int32 it_cur=m_sstring.lower_bound(m_sstring.ssbegin+curString,m_sstring.ssend);
    const TFRZ_Int32 it_cur=m_sstring.lower_bound_withR(curString);//查找curString自己的位置.
    int it=it_cur+it_inc;
    int it_end;
    const TFRZ_Int32* LCP;//当前的后缀字符串和下一个后缀字符串的相等长度.
    if (it_inc==1){
        it_end=(int)m_sstring.size();
        LCP=&m_sstring.LCP[it_cur];
    }else{
        assert(it_inc==-1);
        it_end=-1;
        LCP=&m_sstring.LCP[it_cur]-1;
    }
    
    const int kMaxValue_lcp=((TFRZ_UInt32)1<<31)-1;
    const int kMinZipLoseBitLength=1*8+out_FRZCode.getNozipLengthOutBitLength(1);
    const int kMaxSearchDeepSize=2048;//加大可以提高一点压缩率,但可能降低压缩速度.
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
            TFRZ_Int32 zipedBitLength=lcp*8-out_FRZCode.getZipLengthOutBitLength(lcp)-out_FRZCode.getForwardOffsertOutBitLength(curString, matchString);
            if (curForwardOffsert>kMaxForwardOffsert){//惩罚.
                zipedBitLength-=8*8+4;
                if (curForwardOffsert>kMaxForwardOffsert*2)
                    zipedBitLength-=16*8+4;
            }
            if (zipedBitLength<curBestZipBitLength) continue;
            
            if((zipedBitLength>curBestZipBitLength) ||(matchString>curBestMatchString)){
                   deep-=(kMaxSearchDeepSize/32);
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
    int minZipBitLength=zip_parameter*8+7;//最少要压缩的bit数.
    if (noZipLength>0)
        minZipBitLength+=out_FRZCode.getNozipLengthOutBitLength(1);
    *out_curBestZipBitLength=minZipBitLength;
    *out_curBestMatchPos=-1;
    *out_curBestMatchLength=0;
    _getBestMatch(out_FRZCode,curString,*out_curBestZipBitLength,*out_curBestMatchPos,*out_curBestMatchLength,1,kMaxForwardOffsert);
    _getBestMatch(out_FRZCode,curString,*out_curBestZipBitLength,*out_curBestMatchPos,*out_curBestMatchLength,-1,kMaxForwardOffsert);
    
    return (((*out_curBestMatchPos)>=0)&&((*out_curBestMatchLength)>=out_FRZCode.getMinMatchLength()));
}

