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
#include "FRZ_private/FRZ_compress_best.h"
#include "FRZ_private/FRZ_compress_fast.h"

namespace {
    
    //变长10bit正整数编码方案,从高位开始输出0.5--1.5byte:
    // 0*  3     bit
    // 10  2+4   bit
    // 11  2+4+4 bit
    static const int kPack32BitWithHalfByteMaxValue=(8+64+1024)-1;
    void pack32BitWithHalfByte(TFRZ_Buffer& out_code,TFRZ_UInt32 iValue,int* _isHaveHalfByteIndex){
        assert(iValue<=kPack32BitWithHalfByteMaxValue);
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
        
        int& isHaveHalfByteIndex=*_isHaveHalfByteIndex;
        for (int i=0; i<codeCount; ++i) {
            if (isHaveHalfByteIndex>=0){
                assert(isHaveHalfByteIndex<out_code.size());
                out_code[isHaveHalfByteIndex]|=buf[i];
                isHaveHalfByteIndex=-1;
            }else{
                out_code.push_back(buf[i]<<4);
                isHaveHalfByteIndex=(int)out_code.size()-1;
            }
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
   
    class TFRZ2Code:public TFRZCodeBase{
    public:
        inline explicit TFRZ2Code(int zip_parameter)
          :TFRZCodeBase(zip_parameter){
              clear();
        }
        
        virtual int getMinMatchLength()const { return kMinMatchLength+zip_parameter(); }
        virtual int getZipBitLength(int matchLength,TFRZ_Int32 curString=-1,TFRZ_Int32 matchString=-1)const{
            assert(matchLength>=kMinMatchLength);
            if (curString<0){ curString=1; matchString=0; }
            return 8*matchLength-(kFRZ2CodeType_bit+8*pack32BitWithTagOutSize(curString-matchString-1,0)+pack32BitWithHalfByteOutBitCount(matchLength-kMinMatchLength));
        }
        virtual int getZipParameterForBestUncompressSpeed()const{ return kFRZ2_bestUncompressSpeed; }
        virtual int getNozipLengthOutBitLength(int nozipLength)const{ assert(nozipLength>=1); return kFRZ2CodeType_bit+pack32BitWithHalfByteOutBitCount(nozipLength-1); }
    public:
        virtual bool  outCodeBegin(const TFRZ_Byte** out_codeBegin,const TFRZ_Byte** out_codeEnd){
            if (m_codeBuf.empty()) return false;
            *out_codeBegin=&m_codeBuf[0];
            *out_codeEnd=&m_codeBuf[0]+m_codeBuf.size();
            return true;
        }
        virtual void  outCodeEnd(){ }
        virtual void clear(){ TFRZCodeBase::clear(); m_ctrlCodeIndex=-1; m_ctrlCount=0; m_ctrlHalfLength_isHaveHalfByteIndex=-1; m_codeBuf.clear();  }
    protected:
        virtual void doPushNoZipData(TFRZ_Int32 nozipBegin,TFRZ_Int32 nozipEnd){
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
                doPushNoZipData(nozipBegin,nozipBegin+cutLength);
                doPushNoZipData(nozipBegin+cutLength,nozipEnd);
                return;
            }
            
            assert(nozipEnd<=src_end()-src_windows());
            
            const TFRZ_Byte* data=src_windows()+nozipBegin;
            const TFRZ_Byte* data_end=src_windows()+nozipEnd;
            ctrlPushBack(kFRZ2CodeType_nozip);
            pack32BitWithHalfByte(m_codeBuf,length-kMinLength,&m_ctrlHalfLength_isHaveHalfByteIndex);
            m_codeBuf.insert(m_codeBuf.end(),data,data_end);
        }
        
        virtual void doPushZipData(TFRZ_Int32 curPos,TFRZ_Int32 matchPos,TFRZ_Int32 matchLength){
            const int kMinLength=kMinMatchLength;
            assert(matchLength>=kMinLength);
            if (matchLength-kMinLength>kPack32BitWithHalfByteMaxValue){
                int cutLength=kPack32BitWithHalfByteMaxValue+kMinLength;
                matchLength-=cutLength;
                while (matchLength<kMinLength){
                    matchLength++;
                    cutLength--;
                }
                doPushZipData(curPos,matchPos,cutLength);
                doPushZipData(curPos+cutLength,matchPos+cutLength,matchLength);
                return;
            }
            
            const TFRZ_Int32 frontMatchPos=curPos-matchPos;
            assert(frontMatchPos>0);
            assert(matchLength>=kMinMatchLength);
            ctrlPushBack(kFRZ2CodeType_zip);
            pack32BitWithHalfByte(m_codeBuf,matchLength-kMinLength,&m_ctrlHalfLength_isHaveHalfByteIndex);
            pack32Bit(m_codeBuf,frontMatchPos-1);
        }
    
    private:
        TFRZ_Int32  m_ctrlCodeIndex;
        TFRZ_Int32  m_ctrlCount;
        TFRZ_Int32  m_ctrlHalfLength_isHaveHalfByteIndex;
        TFRZ_Buffer m_codeBuf;
        
        void ctrlPushBack(TFRZ2CodeType type){
            //字节内从低bit开始放置类型.
            assert(kFRZ2CodeType_bit==1);
            if (m_ctrlCount==0){
                m_ctrlCodeIndex=(int)m_codeBuf.size();
                m_codeBuf.push_back(0);
                m_codeBuf.push_back(0);
                m_codeBuf.push_back(0);
            }
            m_codeBuf[m_ctrlCodeIndex+2-(m_ctrlCount>>3)]|=(type<<(m_ctrlCount&0x07));
            ++m_ctrlCount;
            if (m_ctrlCount==24) m_ctrlCount=0;
        }
    };

} //end namespace

int FRZ2_compress_limitMemery_get_compress_step_count(int allCanUseMemrey_MB,int srcDataSize){
    return TFRZCompressBest::compress_limitMemery_get_compress_step_count(allCanUseMemrey_MB, srcDataSize);
}
void FRZ2_compress_limitMemery(int compress_step_count,std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    assert(zip_parameter>=kFRZ2_bestSize);
    assert(zip_parameter<=kFRZ2_bestUncompressSpeed);
    TFRZ2Code FRZ2Code(zip_parameter);
    //TFRZCompressFast  FRZCompress;
    TFRZCompressBest  FRZCompress;
    TFRZCompressBase::compress_by_step(FRZ2Code,FRZCompress,compress_step_count,src,src_end);
    const TFRZ_Byte* code_begin;
    const TFRZ_Byte* code_end;
    if (FRZ2Code.outCodeBegin(&code_begin, &code_end)){
        out_code.insert(out_code.end(), code_begin,code_end);
        FRZ2Code.outCodeEnd();
    }}

void FRZ2_compress(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    FRZ2_compress_limitMemery(1,out_code,src,src_end,zip_parameter);
}

TFRZCodeBase* _new_FRZ2Code(int zip_parameter){
    return  new TFRZ2Code(zip_parameter);
}

