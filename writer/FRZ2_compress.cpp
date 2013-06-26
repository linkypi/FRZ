//FRZ2_compress.cpp
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
#include "FRZ2_compress.h"
#include "../reader/FRZ2_decompress.h"
#include "FRZ_best_compress.h"

namespace {
    
    //变长10bit正整数编码方案,从高位开始输出0.5--1.5byte:
    // 0*  3     bit
    // 10  2+4   bit
    // 11  2+4+4 bit
    static int kPack32BitWithHalfByteMaxValue=(8+64+1024)-1;
    void pack32BitWithHalfByte(TFRZ_Buffer& out_code,TFRZ_UInt32 iValue,bool* isHaveHalfByte){
        const int kMaxPack32BitSize=6;
        TFRZ_Byte buf[kMaxPack32BitSize];
        int codeCount=0;
        if (iValue<8){
            buf[codeCount++]=iValue;
        }else{
            iValue-=8;
            if (iValue<64){
                buf[codeCount++]=(2<<2) | (iValue>>4);
                buf[codeCount++]=iValue&((1<<4)-1);
            }else{
                iValue-=64;
                assert(iValue<1024);
                buf[codeCount++]=(3<<2) | (iValue>>8);
                buf[codeCount++]=(iValue>>4)&((1<<4)-1);
                buf[codeCount++]=iValue&((1<<4)-1);
            }
        }
        for (int i=0; i<codeCount; ++i) {
            if (*isHaveHalfByte){
                assert(!out_code.empty());
                out_code.back()|=buf[i];
            }else{
                out_code.push_back(buf[i]<<4);
            }
            *isHaveHalfByte=!(*isHaveHalfByte);
        }
    }
    
    inline static int pack32BitWithHalfByteOutBitCount(TFRZ_UInt32 iValue){//返回pack后字节大小.
        if (iValue>(TFRZ_UInt32)kPack32BitWithHalfByteMaxValue)
            return 4*3+pack32BitWithHalfByteOutBitCount(iValue-kPack32BitWithHalfByteMaxValue);
        
        int codeCount=0;
        if (iValue<8){
            codeCount++;
        }else{
            iValue-=8;
            if (iValue<64){
                codeCount+=2;
            }else{
                codeCount+=3;
            }
        }
        return codeCount*4;
    }
   
    class TFRZ2Code:public TFRZCode_base{
    public:
        inline explicit TFRZ2Code(int zip_parameter)
          :TFRZCode_base(zip_parameter),m_ctrlHalfLength_isHaveHalfByte(false),m_ctrlCount(0){
        }
        
        virtual void pushNoZipData(TFRZ_Int32 nozipBegin,TFRZ_Int32 nozipEnd){
            const int kMinLength=1;
            TFRZ_Int32 length=nozipEnd-nozipBegin;
            assert(length>=kMinLength);
            if (length>kPack32BitWithHalfByteMaxValue+kMinLength){
                int cutLength=kPack32BitWithHalfByteMaxValue+kMinLength;
                length-=cutLength;
                while (length<kMinLength){
                    length++;
                    cutLength--;
                }
                pushNoZipData(nozipBegin,nozipBegin+cutLength);
                pushNoZipData(nozipBegin+cutLength,nozipEnd);
                return;
            }
            
            assert(nozipEnd<=src_end()-src_begin());
            const TFRZ_Byte* data=src_begin()+nozipBegin;
            const TFRZ_Byte* data_end=src_begin()+nozipEnd;
            ctrlPushBack(kFRZ2CodeType_nozip);
            pack32BitWithHalfByte(m_ctrlHalfLengthCode,length-kMinLength,&m_ctrlHalfLength_isHaveHalfByte);
            m_dataBuf.insert(m_dataBuf.end(),data,data_end);
        }
        
        virtual void pushZipData(TFRZ_Int32 curPos,TFRZ_Int32 matchPos,TFRZ_Int32 matchLength){
            const int kMinLength=getMinMatchLength();
            assert(matchLength>=kMinLength);
            if (matchLength-kMinLength>kPack32BitWithHalfByteMaxValue){
                int cutLength=kPack32BitWithHalfByteMaxValue+kMinLength;
                matchLength-=cutLength;
                while (matchLength<kMinLength){
                    matchLength++;
                    cutLength--;
                }
                pushZipData(curPos,matchPos,cutLength);
                pushZipData(curPos+cutLength,matchPos+cutLength,matchLength);
                return;
            }
            
            const TFRZ_Int32 frontMatchPos=curPos-matchPos;
            assert(frontMatchPos>0);
            assert(matchLength>=getMinMatchLength());
            ctrlPushBack(kFRZ2CodeType_zip);
            pack32BitWithHalfByte(m_ctrlHalfLengthCode,matchLength-kMinLength,&m_ctrlHalfLength_isHaveHalfByte);
            pack32Bit(m_frontMatchPosCode,frontMatchPos-1);
        }
        
        virtual int getMinMatchLength()const { return 3+zip_parameter(); } //2的话很多情况下可以压缩的更小,但可能影响解压速度.
        virtual int getZipBitLength(int matchLength,TFRZ_Int32 curString=-1,TFRZ_Int32 matchString=-1)const{
            assert(matchLength>=getMinMatchLength());
            if (curString<0){ curString=1; matchString=0; }
            return 8*matchLength-(kFRZ2CodeType_bit+8*pack32BitWithTagOutSize(curString-matchString-1,0)+pack32BitWithHalfByteOutBitCount(matchLength-getMinMatchLength()));
        }
        virtual int getZipParameterForBestUncompressSpeed()const{ return kFRZ2_bestUncompressSpeed; }
        virtual int getNozipLengthOutBitLength(int nozipLength)const{ assert(nozipLength>=1); return kFRZ2CodeType_bit+pack32BitWithHalfByteOutBitCount(nozipLength-1); }
        
        void write_code(TFRZ_Buffer& out_code)const{
            pack32Bit(out_code,m_ctrlCount);
            pack32Bit(out_code,(TFRZ_Int32)m_ctrlHalfLengthCode.size());
            pack32Bit(out_code,(TFRZ_Int32)m_frontMatchPosCode.size());
            out_code.push_back(getMinMatchLength());
            out_code.insert(out_code.end(),m_ctrlCode.begin(),m_ctrlCode.end());
            out_code.insert(out_code.end(),m_ctrlHalfLengthCode.begin(),m_ctrlHalfLengthCode.end());
            out_code.insert(out_code.end(),m_frontMatchPosCode.begin(),m_frontMatchPosCode.end());
            out_code.insert(out_code.end(),m_dataBuf.begin(),m_dataBuf.end());
        }
    private:
        TFRZ_Buffer m_ctrlCode;
        TFRZ_Int32  m_ctrlCount;
        TFRZ_Buffer m_ctrlHalfLengthCode;
        bool        m_ctrlHalfLength_isHaveHalfByte;
        TFRZ_Buffer m_frontMatchPosCode;
        TFRZ_Buffer m_dataBuf;
        
        void ctrlPushBack(TFRZ2CodeType type){
            assert(kFRZ2CodeType_bit==1);
            assert(((m_ctrlCount+7)>>3)==m_ctrlCode.size());
            const int index=m_ctrlCount&7;
            //从低bit开始.
            if (index==0)
                m_ctrlCode.push_back(type);
            else
                m_ctrlCode.back()|=(type<<index);
            ++m_ctrlCount;
        }
    };

} //end namespace

void _beta_FRZ2_compress_limitMemery(int compress_step_count,std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    assert(zip_parameter>=kFRZ2_bestSize);
    assert(zip_parameter<=kFRZ2_bestUncompressSpeed);
    assert(src_end-src<=(((unsigned int)1<<31)-1));
    assert(compress_step_count>=1);
    const int stepMemSize=(int)((src_end-src+compress_step_count-1)/compress_step_count);
    assert((stepMemSize>0)||(src_end==src));
    
    TFRZ2Code FRZ2Code(zip_parameter);
    for (const unsigned char* step_src=src; step_src<src_end; step_src+=stepMemSize) {
        const unsigned char* step_src_end=step_src+stepMemSize;
        if (step_src_end>src_end)
            step_src_end=src_end;
        TFRZBestZiper FRZBestZiper(FRZ2Code,step_src,step_src_end);
    }
    FRZ2Code.write_code(out_code);
}

void _beta_FRZ2_compress(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    _beta_FRZ2_compress_limitMemery(1,out_code,src,src_end,zip_parameter);
}

