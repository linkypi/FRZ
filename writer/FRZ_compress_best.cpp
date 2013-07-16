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
#include "FRZ_compress_best.h"

static const int _kBestForwardOffsert_zip_parameter_table_size=8+1;
static const int _kBestForwardOffsert_zip_parameter_table[_kBestForwardOffsert_zip_parameter_table_size]={
    960*1024,840*1024,720*1024,600*1024, 500*1024, //0..4
    420*1024,340*1024,300*1024,280*1024//5..8
};
static const int _kBestForwardOffsert_zip_parameter_table_minValue=150*1024;


TFRZCompressBest::TFRZCompressBest():m_sstring(0,0),m_bestForwardOffsert(-1){
}

const TFRZ_Byte* TFRZCompressBest::createCode_step(TFRZCode_base& out_FRZCode,const TFRZ_Byte* src_windows,const TFRZ_Byte* src_cur,const TFRZ_Byte* src_end,int kCanNotZipLength){
    m_sstring.resetString((const char*)src_windows,(const char*)src_end);
    m_sstring.R_create();
    m_sstring.LCPLite_create_withR();
    const TFRZ_Byte* result=TFRZCompressBase::createCode_step(out_FRZCode,src_windows,src_cur,src_end,kCanNotZipLength);
    m_sstring.LCPLite.clear();
    m_sstring.R.clear();
    return result;
}

void TFRZCompressBest::_getBestMatch(TFRZCode_base& out_FRZCode,TSuffixIndex curString,TFRZ_Int32& curBestZipBitLength,TFRZ_Int32& curBestMatchString,TFRZ_Int32& curBestMatchLength,int it_inc,int kBestForwardOffsert){
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
    const int kMaxForwardOffsert=out_FRZCode.getMaxForwardOffsert(curString);
    int min_lcp=kMaxValue_lcp;
    for (int deep=kMaxSearchDeepSize;(deep>0)&&(it!=it_end);it+=it_inc,LCP+=it_inc,--deep){
        int curLCP=*LCP;
        if (curLCP<min_lcp){
            min_lcp=curLCP;
            if (min_lcp*8<curBestZipBitLength+kMinZipLoseBitLength)//不可能压缩了.
                break;
        }
        
        TSuffixIndex matchString=m_sstring.SA[it];
        const int curForwardOffsert=(curString-matchString);
        if (curForwardOffsert>0){
            --deep;
            if (curForwardOffsert>kMaxForwardOffsert) continue;
            TFRZ_Int32 zipedBitLength=out_FRZCode.getZipBitLength(min_lcp,curString,matchString);
            if (curForwardOffsert>kBestForwardOffsert){//惩罚.
                zipedBitLength-=8+4;
                if (curForwardOffsert>kBestForwardOffsert*2){
                    zipedBitLength-=4*8+4;
                    if (curForwardOffsert>kBestForwardOffsert*4)
                        zipedBitLength-=8*8+4;
                }
            }
            if (zipedBitLength<curBestZipBitLength) continue;
            
            if((zipedBitLength>curBestZipBitLength) ||(matchString>curBestMatchString)){
                deep-=(kMaxSearchDeepSize/64);
                curBestZipBitLength=zipedBitLength;
                curBestMatchString=matchString;
                curBestMatchLength=min_lcp;
            }
        }
    }
}

bool TFRZCompressBest::getBestMatch(TFRZCode_base& out_FRZCode,TFRZ_Int32 curString,TFRZ_Int32* out_curBestMatchPos,TFRZ_Int32* out_curBestMatchLength,TFRZ_Int32 nozipBegin){
    if (m_bestForwardOffsert<0){
        //m_bestForwardOffsert 增大可以提高压缩率但可能会减慢解压速度(缓存命中降低).
        const int zip_parameter=out_FRZCode.zip_parameter();
        const int kS=_kBestForwardOffsert_zip_parameter_table_size; //todo:参数比例不同时,不准确.
        const int kFRZ_bestUncompressSpeed=out_FRZCode.getZipParameterForBestUncompressSpeed();
        assert(kFRZ_bestUncompressSpeed>kS);
        if (zip_parameter<kS){
            m_bestForwardOffsert=_kBestForwardOffsert_zip_parameter_table[zip_parameter];
        }else{
            const int kMax=_kBestForwardOffsert_zip_parameter_table[kS-1];
            const int kMin=_kBestForwardOffsert_zip_parameter_table_minValue;
            if (zip_parameter>=kFRZ_bestUncompressSpeed)
                m_bestForwardOffsert=kMin;
            else
                m_bestForwardOffsert=kMax-(kMax-kMin)*(zip_parameter-kS)/(kFRZ_bestUncompressSpeed-kS);
        }
    }
    
    const int noZipLength=curString-nozipBegin;
    //   pksize(noZipLength)+noZipLength
    // + pksize(zipLength)+pksize(ForwardOffsertInfo)
    // + pksize(allLength-noZipLength-zipLength)+ allLength-noZipLength-zipLength
    // < pksize(allLength)+allLength + zip_parameter
    //~=> zipLength - pksize(zipLength)-pksize(ForwardOffsertInfo) > zip_parameter + pksize(noZipLength)
    int minZipBitLength=out_FRZCode.getMinZipBitLength();//最少要压缩的bit数.
    if (noZipLength>0)
        minZipBitLength+=out_FRZCode.getNozipLengthOutBitLength(1);
    if (minZipBitLength<=0) minZipBitLength=1;
    
    TFRZ_Int32 curBestZipBitLength=minZipBitLength;
    *out_curBestMatchPos=-1;
    *out_curBestMatchLength=0;
    _getBestMatch(out_FRZCode,curString,curBestZipBitLength,*out_curBestMatchPos,*out_curBestMatchLength,1,m_bestForwardOffsert);
    _getBestMatch(out_FRZCode,curString,curBestZipBitLength,*out_curBestMatchPos,*out_curBestMatchLength,-1,m_bestForwardOffsert);
    
    return (((*out_curBestMatchPos)>=0)&&((*out_curBestMatchLength)>=out_FRZCode.getMinMatchLength()));
}
