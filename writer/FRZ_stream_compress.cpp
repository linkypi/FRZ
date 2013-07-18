//FRZ_stream_compress.cpp
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
#include "FRZ_stream_compress.h"
#include "FRZ_private/FRZ_compress_best.h"

class TFRZ_stream_compress{
public:
    void append_data(const unsigned char* src,const unsigned char* src_end,bool isAppendDataFinish){
        m_dataBuf.insert(m_dataBuf.end(),src,src_end);
        compress(false);
    }
    inline void flush_code(){
        compress(true);
        write_code();
    }
    TFRZ_stream_compress(TFRZCodeBase* frzCode,TFRZCompressBase* frzCompress,int maxDecompressWindowsSize,
                          TFRZ_write_code_proc out_code_callBack,void* callBackData,int maxStepMemorySize)
        :m_frzCode(frzCode),m_frzCompress(frzCompress),m_maxDecompressWindowsSize(maxDecompressWindowsSize),m_maxStepMemorySize(maxStepMemorySize),
        m_isFirstOutHead(true),m_out_code_callBack(out_code_callBack),m_callBackData(callBackData), m_curWindowsSize(0){
            assert(out_code_callBack!=0); assert(maxDecompressWindowsSize>0); assert(maxStepMemorySize>0);
        }
    virtual ~TFRZ_stream_compress(){ flush_code(); delete m_frzCode; delete m_frzCompress; }
    
private:
    TFRZCodeBase*           m_frzCode;
    TFRZCompressBase*       m_frzCompress;
    int                     m_maxDecompressWindowsSize;
    int                     m_maxStepMemorySize;
    bool                    m_isFirstOutHead;
    TFRZ_write_code_proc    m_out_code_callBack;
    void*                   m_callBackData;
    int                     m_curWindowsSize;
    
    TFRZ_Buffer             m_dataBuf;
    
    void write_code(){
        assert(m_frzCode->getDataSize()<=m_maxStepMemorySize);
        const TFRZ_Byte* code_begin;
        const TFRZ_Byte* code_end;
        if (m_frzCode->outCodeBegin(&code_begin, &code_end)){
            TFRZ_Buffer  codeHeadBuf;
            pack32Bit(codeHeadBuf,m_frzCode->getDataSize());
            pack32Bit(codeHeadBuf,(int)(code_end-code_begin));
            if (m_isFirstOutHead){
                m_isFirstOutHead=false;
                pack32Bit(codeHeadBuf,m_maxDecompressWindowsSize);
                pack32Bit(codeHeadBuf,m_maxStepMemorySize);
            }
            codeHeadBuf.insert(codeHeadBuf.begin(), codeHeadBuf.size());
            m_out_code_callBack(m_callBackData,&codeHeadBuf[0],&codeHeadBuf[0]+codeHeadBuf.size());
            m_out_code_callBack(m_callBackData,code_begin,code_end);
            m_frzCode->outCodeEnd();
            m_frzCode->clear();
        }
    }
    inline int cacheSrcDataSize() const { return (int)m_dataBuf.size()-m_curWindowsSize; }
    void compress(bool isFlush){
        while ((cacheSrcDataSize()>=m_maxStepMemorySize)||(isFlush&&(cacheSrcDataSize()>0))) {
            compress_a_step();
        }
    }
    void compress_a_step(){
        assert(cacheSrcDataSize()!=0);

        const unsigned char* match_src=&m_dataBuf[0];
        const unsigned char* cur_src=match_src+m_curWindowsSize;
        const unsigned char* cur_src_end=match_src+m_dataBuf.size();
        if (cur_src_end-cur_src>m_maxStepMemorySize)
            cur_src_end=cur_src+m_maxStepMemorySize;
        
        m_frzCompress->createCode_step(*m_frzCode,match_src,cur_src,cur_src_end);
        cur_src=cur_src_end;
        write_code();
        
        m_curWindowsSize=(int)(cur_src-match_src);
        if (m_curWindowsSize>m_maxDecompressWindowsSize){
            m_dataBuf.erase(m_dataBuf.begin(),m_dataBuf.begin()+m_curWindowsSize-m_maxDecompressWindowsSize);
            m_curWindowsSize=m_maxDecompressWindowsSize;
        }
    }
};

extern TFRZCodeBase* _new_FRZ1Code(int zip_parameter);
extern TFRZCodeBase* _new_FRZ2Code(int zip_parameter);

TFRZ_stream_compress_handle FRZ2_stream_compress_best_create(int zip_parameter,int maxDecompressWindowsSize,
                                                             TFRZ_write_code_proc out_code_callBack,void* callBackData,int maxStepMemorySize){
    TFRZCodeBase* frzCode=_new_FRZ2Code(zip_parameter);
    TFRZCompressBase* frzCompress=new TFRZCompressBest();
    return new TFRZ_stream_compress(frzCode,frzCompress,maxDecompressWindowsSize,out_code_callBack,callBackData,maxStepMemorySize);
}

TFRZ_stream_compress_handle FRZ1_stream_compress_best_create(int zip_parameter,int maxDecompressWindowsSize,
                                                             TFRZ_write_code_proc out_code_callBack,void* callBackData,int maxStepMemorySize){
    TFRZCodeBase* frzCode=_new_FRZ1Code(zip_parameter);
    TFRZCompressBase* frzCompress=new TFRZCompressBest();
    return new TFRZ_stream_compress(frzCode,frzCompress,maxDecompressWindowsSize,out_code_callBack,callBackData,maxStepMemorySize);
}

void FRZ_stream_compress_append_data(TFRZ_stream_compress_handle handle,const unsigned char* src,const unsigned char* src_end,bool isAppendDataFinish){
    assert(handle!=0);
    ((TFRZ_stream_compress*)handle)->append_data(src,src_end,isAppendDataFinish);
}

void FRZ_stream_compress_append_data_finish(TFRZ_stream_compress_handle handle){
    assert(handle!=0);
    ((TFRZ_stream_compress*)handle)->flush_code();
}

void FRZ_stream_compress_delete(TFRZ_stream_compress_handle handle){
    if (handle==0) return;
    delete (TFRZ_stream_compress*)handle;
}


