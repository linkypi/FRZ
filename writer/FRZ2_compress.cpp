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
    
    //变长32bit正整数编码方案,从高位开始输出1-6byte:
    // 0*  3 bit
    // 1* 0*  3+3 bit
    // 1* 1* 0*  3+3+3 bit
    // 1* 1* 1* 0*  3+3+3+3 bit
    //...
    void pack32BitWithHalfByte(TFRZ_Buffer& out_code,TFRZ_UInt32 iValue,bool* isHaveHalfByte){
        const int kMaxPack32BitSize=6;
        TFRZ_Byte codeBuf[kMaxPack32BitSize*2];
        TFRZ_Byte* codeEnd=codeBuf;
        while (iValue>((1<<3)-1)) {
            *codeEnd=iValue&((1<<3)-1); ++codeEnd;
            iValue>>=3;
        }
        *codeEnd=iValue; ++codeEnd;
        while (codeBuf!=codeEnd){
            --codeEnd;
            int halfValue=(*codeEnd) | (((codeBuf!=codeEnd)?1:0)<<3);
            if (*isHaveHalfByte) {
                assert(!out_code.empty());
                out_code.back()|=halfValue;
                *isHaveHalfByte=false;
            }else{
                out_code.push_back(halfValue<<4);
                *isHaveHalfByte=true;
            }
        }
    }
    
    inline static int pack32BitWithHalfByteOutBitCount(TFRZ_UInt32 iValue){//返回pack后字节大小.
        int result=0;
        while (iValue>((1<<3)-1)) {
            ++result;
            iValue>>=3;
        }
        return (result+1)*4;
    }
   
    class TFRZ2Code:public TFRZCode_base{
    public:
        inline explicit TFRZ2Code(int zip_parameter)
          :TFRZCode_base(zip_parameter),m_ctrlHalfLength_isHaveHalfByte(false),m_ctrlCount(0){
        }
        
        virtual void pushNoZipData(TFRZ_Int32 nozipBegin,TFRZ_Int32 nozipEnd){
            assert(nozipEnd>nozipBegin);
            assert(nozipEnd<=src_end()-src_begin());
            const TFRZ_Byte* data=src_begin()+nozipBegin;
            const TFRZ_Byte* data_end=src_begin()+nozipEnd;
            ctrlPushBack(kFRZ2CodeType_nozip);
            pack32BitWithHalfByte(m_ctrlHalfLengthCode,(nozipEnd-nozipBegin)-1,&m_ctrlHalfLength_isHaveHalfByte);
            m_dataBuf.insert(m_dataBuf.end(),data,data_end);
        }
        
        virtual void pushZipData(TFRZ_Int32 curPos,TFRZ_Int32 matchPos,TFRZ_Int32 matchLength){
            const TFRZ_Int32 frontMatchPos=curPos-matchPos;
            assert(frontMatchPos>0);
            assert(matchLength>=getMinMatchLength());
            ctrlPushBack(kFRZ2CodeType_zip);
            pack32BitWithHalfByte(m_ctrlHalfLengthCode,matchLength-getMinMatchLength(),&m_ctrlHalfLength_isHaveHalfByte);
            pack32Bit(m_frontMatchPosCode,frontMatchPos-1);
        }
        
        virtual int getMinMatchLength()const { return 3+zip_parameter(); }
        virtual int getZipParameterForBestUncompressSpeed()const{ return kFRZ2_bestUncompressSpeed; }
        virtual int getNozipLengthOutBitLength(int nozipLength)const{ return pack32BitWithHalfByteOutBitCount(nozipLength-1); }
        virtual int getZipLengthOutBitLength(int zipLength)const{ return pack32BitWithHalfByteOutBitCount(zipLength-getMinMatchLength()); }
        virtual int getForwardOffsertOutBitLength(int curPos,int matchPos)const{ return 8*pack32BitWithTagOutSize(curPos-matchPos-1,0); }
        
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

void FRZ2_compress_limitMemery(int compress_step_count,std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    assert(zip_parameter>=kFRZ2_bestSize);
    assert(zip_parameter<=kFRZ2_bestUncompressSpeed);
    assert(src_end-src<=((1<<31)-1));
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

void FRZ2_compress(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    FRZ2_compress_limitMemery(1,out_code,src,src_end,zip_parameter);
}

