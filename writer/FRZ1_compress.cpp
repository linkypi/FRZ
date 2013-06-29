//FRZ1_compress.cpp
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
#include "FRZ1_compress.h"
#include "../reader/FRZ1_decompress.h"
#include "FRZ_best_compress.h"

namespace {
    
    class TFRZ1Code:public TFRZCode_base{
    public:
        inline explicit TFRZ1Code(int zip_parameter):TFRZCode_base(zip_parameter){
        }
        virtual void pushNoZipData(TFRZ_Int32 nozipBegin,TFRZ_Int32 nozipEnd){
            assert(nozipEnd>nozipBegin);
            assert(nozipEnd<=src_end()-src_begin());
            const TFRZ_Byte* data=src_begin()+nozipBegin;
            const TFRZ_Byte* data_end=src_begin()+nozipEnd;
            pack32BitWithTag(m_ctrlCode,(nozipEnd-nozipBegin)-1, kFRZ1CodeType_nozip,kFRZ1CodeType_bit);
            m_dataBuf.insert(m_dataBuf.end(),data,data_end);
        }
        
        virtual void pushZipData(TFRZ_Int32 curPos,TFRZ_Int32 matchPos,TFRZ_Int32 matchLength){
            const TFRZ_Int32 frontMatchPos=curPos-matchPos;
            assert(frontMatchPos>0);
            assert(matchLength>=getMinMatchLength());
            pack32BitWithTag(m_ctrlCode,matchLength-1, kFRZ1CodeType_zip,kFRZ1CodeType_bit);
            pack32Bit(m_ctrlCode,frontMatchPos-1);
        }
        
        virtual int getMinMatchLength()const { return 3+zip_parameter(); }
        virtual int getZipBitLength(int matchLength,TFRZ_Int32 curString=-1,TFRZ_Int32 matchString=-1)const{
            if (curString<0){ curString=1; matchString=0; }
            return 8*matchLength-8*pack32BitWithTagOutSize(matchLength,kFRZ1CodeType_bit)-8*pack32BitWithTagOutSize(curString-matchString-1,0);
        }
        virtual int getZipParameterForBestUncompressSpeed()const{ return kFRZ1_bestUncompressSpeed; }
        virtual int getNozipLengthOutBitLength(int nozipLength)const{ assert(nozipLength>=1); return 8*pack32BitWithTagOutSize(nozipLength-1,kFRZ1CodeType_bit); }
        
        void write_code(TFRZ_Buffer& out_code)const{
            pack32Bit(out_code,(TFRZ_Int32)m_ctrlCode.size());
            out_code.insert(out_code.end(),m_ctrlCode.begin(),m_ctrlCode.end());
            out_code.insert(out_code.end(),m_dataBuf.begin(),m_dataBuf.end());
        }
    private:
        TFRZ_Buffer m_ctrlCode;
        TFRZ_Buffer m_dataBuf;
    };

} //end namespace

int FRZ1_compress_limitMemery_get_compress_step_count(int allCanUseMemrey_MB,int srcDataSize){
    return TFRZBestZiper::compress_limitMemery_get_compress_step_count(allCanUseMemrey_MB, srcDataSize);
}

void FRZ1_compress_limitMemery(int compress_step_count,std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    assert(zip_parameter>=kFRZ1_bestSize);
    assert(zip_parameter<=kFRZ1_bestUncompressSpeed);
    TFRZ1Code FRZ1Code(zip_parameter);
    TFRZBestZiper::compress_by_step(FRZ1Code,compress_step_count,src,src_end);
    FRZ1Code.write_code(out_code);
}

void FRZ1_compress(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    FRZ1_compress_limitMemery(1,out_code,src,src_end,zip_parameter);
}

