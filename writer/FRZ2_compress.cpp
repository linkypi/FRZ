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
#include "FRZ_compress_best.h"

namespace {
    
    //变长10bit正整数编码方案,从高位开始输出0.5--1.5byte:
    // 0*  3     bit
    // 10  2+4   bit
    // 11  2+4+4 bit
    static const int kPack32BitWithHalfByteMaxValue=(8+64+1024)-1;
    void pack32BitWithHalfByte(TFRZ_Buffer& out_code,TFRZ_UInt32 iValue,int* _isHaveHalfByteIndex){
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
   
    class TFRZ2Code:public TFRZCode_base{
    public:
        inline explicit TFRZ2Code(int zip_parameter)
          :TFRZCode_base(zip_parameter){
              clearCode();
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
            pack32BitWithHalfByte(m_codeBuf,length-kMinLength,&m_ctrlHalfLength_isHaveHalfByteIndex);
            m_codeBuf.insert(m_codeBuf.end(),data,data_end);
            m_dataSize+=length;
        }
        
        virtual void pushZipData(TFRZ_Int32 curPos,TFRZ_Int32 matchPos,TFRZ_Int32 matchLength){
            const int kMinLength=kMinMatchLength;
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
            assert(matchLength>=kMinMatchLength);
            ctrlPushBack(kFRZ2CodeType_zip);
            pack32BitWithHalfByte(m_codeBuf,matchLength-kMinLength,&m_ctrlHalfLength_isHaveHalfByteIndex);
            pack32Bit(m_codeBuf,frontMatchPos-1);
            m_dataSize+=matchLength;
        }
        
        virtual int getMaxForwardOffsert(TFRZ_Int32 curPos)const { return 32*1024*1024;  }
        virtual int getMinMatchLength()const { return kMinMatchLength+zip_parameter(); }
        virtual int getZipBitLength(int matchLength,TFRZ_Int32 curString=-1,TFRZ_Int32 matchString=-1)const{
            assert(matchLength>=kMinMatchLength);
            if (curString<0){ curString=1; matchString=0; }
            return 8*matchLength-(kFRZ2CodeType_bit+8*pack32BitWithTagOutSize(curString-matchString-1,0)+pack32BitWithHalfByteOutBitCount(matchLength-kMinMatchLength));
        }
        virtual int getZipParameterForBestUncompressSpeed()const{ return kFRZ2_bestUncompressSpeed; }
        virtual int getNozipLengthOutBitLength(int nozipLength)const{ assert(nozipLength>=1); return kFRZ2CodeType_bit+pack32BitWithHalfByteOutBitCount(nozipLength-1); }
        
        void write_code(TFRZ_Buffer& out_code){
            out_code.insert(out_code.end(),m_codeBuf.begin(),m_codeBuf.end());
        }
    protected:
        inline TFRZ_Buffer&  getCodeBuf(){ return m_codeBuf; }
        inline TFRZ_Int32    getDataSize()const { return m_dataSize; }
        void clearCode(){ m_dataSize=0; m_ctrlCodeIndex=-1; m_ctrlCount=0; m_ctrlHalfLength_isHaveHalfByteIndex=-1; m_codeBuf.clear(); }
    private:
        TFRZ_Int32  m_ctrlCodeIndex;
        TFRZ_Int32  m_ctrlCount;
        TFRZ_Int32  m_ctrlHalfLength_isHaveHalfByteIndex;
        TFRZ_Int32  m_dataSize;
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
    TFRZCompressBest  FRZCompress;
    TFRZCompressBase::compress_by_step(FRZ2Code,FRZCompress,compress_step_count,src,src_end);
    FRZ2Code.write_code(out_code);
}

void FRZ2_compress(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    FRZ2_compress_limitMemery(1,out_code,src,src_end,zip_parameter);
}


class TFRZ2_stream_compress:public TFRZ2Code{
public:
    void append_data(const unsigned char* src,const unsigned char* src_end,bool isAppendDataFinish){
        m_dataBuf.insert(m_dataBuf.end(),src,src_end);
        const int kLookupBackLength=2*1024;
        compress(isAppendDataFinish?0:kLookupBackLength);
    }
    inline void flush_code(){
        compress(0);
        write_code();
    }
    TFRZ2_stream_compress(int zip_parameter,int maxDecompressWindowsSize,
                          TFRZ_write_code_proc out_code_callBack,void* callBackData,int maxStepMemorySize)
        :TFRZ2Code(zip_parameter),m_maxDecompressWindowsSize(maxDecompressWindowsSize),m_maxStepMemorySize(maxStepMemorySize),
        m_isNeedOutHead(true),m_out_code_callBack(out_code_callBack),m_callBackData(callBackData), m_curWindowsSize(0){
            assert(out_code_callBack!=0); assert(maxDecompressWindowsSize>0); assert(maxStepMemorySize>0);
        }
    virtual ~TFRZ2_stream_compress(){ flush_code(); }
    
    virtual int getMaxForwardOffsert(TFRZ_Int32 curPos)const { return m_curWindowsSize+curPos;  }
private:
    int                     m_maxDecompressWindowsSize;
    int                     m_maxStepMemorySize;
    bool                    m_isNeedOutHead;
    TFRZ_write_code_proc    m_out_code_callBack;
    void*                   m_callBackData;
    int                     m_curWindowsSize;
    
    TFRZ_Buffer             m_dataBuf;
    
    void write_code(){
        assert(getDataSize()<=m_maxStepMemorySize);
        
        TFRZ_Buffer&  codeBuf=getCodeBuf();
        if (!codeBuf.empty()){
            TFRZ_Buffer  codeHeadBuf;
            if (m_isNeedOutHead){
                m_isNeedOutHead=false;
                pack32Bit(codeHeadBuf,m_maxDecompressWindowsSize);
                pack32Bit(codeHeadBuf,m_maxStepMemorySize);
            }
            pack32Bit(codeHeadBuf,getDataSize());
            pack32Bit(codeHeadBuf,(int)codeBuf.size());
            codeBuf.insert(codeBuf.begin(),codeHeadBuf.begin(),codeHeadBuf.end());
            m_out_code_callBack(m_callBackData,&codeBuf[0],&codeBuf[0]+codeBuf.size());
            clearCode();
        }
    }
    inline int cacheSrcDataSize() const { return (int)m_dataBuf.size()-m_curWindowsSize; }
    void compress(const int kLookupBackLength){
        while ((cacheSrcDataSize()>=m_maxStepMemorySize)||((kLookupBackLength==0)&&(cacheSrcDataSize()>0))) {
            compress_a_step(kLookupBackLength);
        }
    }
    void compress_a_step(const int kLookupBackLength){
        assert(cacheSrcDataSize()!=0);

        const unsigned char* match_src=&m_dataBuf[0];
        const unsigned char* cur_src=match_src+m_curWindowsSize;
        const unsigned char* cur_src_end=match_src+m_dataBuf.size();
        if (cur_src_end-cur_src>m_maxStepMemorySize)
            cur_src_end=cur_src+m_maxStepMemorySize;
        int lookupBackLength=kLookupBackLength;
        if (cur_src_end-cur_src<lookupBackLength){
            lookupBackLength=(int)(cur_src_end-cur_src)>>1;
            cur_src_end-=lookupBackLength;
        }
        
        TFRZCompressBest FRZBestZiper;
        cur_src=FRZBestZiper.createCode_step(*this,match_src,cur_src,cur_src_end,lookupBackLength);
        write_code();
        
        m_curWindowsSize=(int)(cur_src-match_src);
        if (m_curWindowsSize>m_maxDecompressWindowsSize){
            m_dataBuf.erase(m_dataBuf.begin(),m_dataBuf.begin()+m_curWindowsSize-m_maxDecompressWindowsSize);
            m_curWindowsSize=m_maxDecompressWindowsSize;
        }
    }
};


TFRZ2_stream_compress_handle FRZ2_stream_compress_create(int zip_parameter,int maxDecompressWindowsSize,
                                                         TFRZ_write_code_proc out_code_callBack,void* callBackData,int maxStepMemorySize){
    return new TFRZ2_stream_compress(zip_parameter,maxDecompressWindowsSize,out_code_callBack,callBackData,maxStepMemorySize);
}

void FRZ2_stream_compress_append_data(TFRZ2_stream_compress_handle handle,const unsigned char* src,const unsigned char* src_end,bool isAppendDataFinish){
    assert(handle!=0);
    ((TFRZ2_stream_compress*)handle)->append_data(src,src_end,isAppendDataFinish);
}

void FRZ2_stream_compress_append_data_finish(TFRZ2_stream_compress_handle handle){
    assert(handle!=0);
    ((TFRZ2_stream_compress*)handle)->flush_code();
}

void FRZ2_stream_compress_delete(TFRZ2_stream_compress_handle handle){
    if (handle==0) return;
    delete (TFRZ2_stream_compress*)handle;
}


