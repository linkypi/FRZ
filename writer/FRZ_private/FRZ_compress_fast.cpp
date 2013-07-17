//FRZ_compress_fast.cpp
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
#include "FRZ_compress_fast.h"

static const int _kBestForwardOffsert_zip_parameter_table_size=8+1;
static const int _kBestForwardOffsert_zip_parameter_table[_kBestForwardOffsert_zip_parameter_table_size]={
    1900*1024,1700*1024,1500*1024,1200*1024, 900*1024, //0..4
    800*1024,700*1024,600*1024,500*1024//5..8
};
static const int _kBestForwardOffsert_zip_parameter_table_minValue=200*1024;


TFRZCompressFast::TFRZCompressFast():m_bestForwardOffsert(-1){
}

const TFRZ_Byte* TFRZCompressFast::createCode_step(TFRZCodeBase& out_FRZCode,const TFRZ_Byte* src_windows,const TFRZ_Byte* src_cur,const TFRZ_Byte* src_end,int kCanNotZipLength){
   /* m_sstring.resetString((const char*)src_windows,(const char*)src_end);
    m_sstring.R_create();
    m_sstring.LCPLite_create_withR();*/
    const TFRZ_Byte* result=TFRZCompressBase::createCode_step(out_FRZCode,src_windows,src_cur,src_end,kCanNotZipLength);
    //todo:
    return result;
}

bool TFRZCompressFast::getBestMatch(TFRZCodeBase& out_FRZCode,TFRZ_Int32 curString,TFRZ_Int32* out_curBestMatchPos,TFRZ_Int32* out_curBestMatchLength,TFRZ_Int32 nozipBegin){
    return false;
    
    if (m_bestForwardOffsert<0){
        //m_bestForwardOffsert 增大可以提高压缩率但可能会减慢解压速度(缓存命中降低).
        const int zip_parameter=out_FRZCode.zip_parameter();
        const int kS=_kBestForwardOffsert_zip_parameter_table_size; //note:参数比例不同时,不准确.
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

    //todo:
    
    return (((*out_curBestMatchPos)>=0)&&((*out_curBestMatchLength)>=out_FRZCode.getMinMatchLength()));
}
