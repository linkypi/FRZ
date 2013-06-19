//  FRZ2_decompress_inc.c
/*
 Copyright (c) 2012-2013 HouSisong All Rights Reserved.
 (The MIT License)
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 */
#ifdef _PRIVATE_FRZ_DECOMPRESS_NEED_INCLUDE_CODE

//变长32bit正整数编码方案,从高位开始输出1-6byte:
// 0*  3 bit
// 1* 0*  3+3 bit
// 1* 1* 0*  3+3+3 bit
// 1* 1* 1* 0*  3+3+3+3 bit
// 1* 1* 1* 1* 0*  3+3+3+3+3 bit
// 1* 1* 1* 1* 1* 0*  3+3+3+3+3+3 bit
// 1* 1* 1* 1* 1* 1* 0*  3+3+3+3+3+3+3 bit
// 1* 1* 1* 1* 1* 1* 1* 0*  3+3+3+3+3+3+3+3 bit
// 1* 1* 1* 1* 1* 1* 1* 1* 0*  3+3+3+3+3+3+3+3+3 bit
// 1* 1* 1* 1* 1* 1* 1* 1* 1* 0*  3+3+3+3+3+3+3+3+3+3 bit
// 1* 1* 1* 1* 1* 1* 1* 1* 1* 1* 0*  3+3+3+3+3+3+3+3+3+3+3 bit
static inline TFRZ_UInt32 _PRIVATE_FRZ_unpack32BitWithHalfByte_NAME(const TFRZ_Byte** src_code,const TFRZ_Byte* src_code_end,int* _isHaveHalfByte){//读出整数并前进指针.
    const TFRZ_Byte* pcode;
    TFRZ_UInt32 value;
    TFRZ_UInt32 code;
    int isHaveHalfByte;
    int isHaveNext;
    isHaveHalfByte=*_isHaveHalfByte;
    pcode=*src_code;
    if (isHaveHalfByte){
        isHaveHalfByte=0;
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
        if (src_code_end==pcode){ (*_isHaveHalfByte)=isHaveHalfByte; return 0; }
#endif
        code=*pcode;
        ++pcode;
        value=(code&((1<<3)-1));
        if (!(code&(1<<3))){
            (*src_code)=pcode;
            (*_isHaveHalfByte)=isHaveHalfByte;
            return value;
        }
    }else{
        value=0;
    }
    
    //assert(isHaveHalfByte==0);
    do {
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
        //assert((value>>(sizeof(value)*8-3))==0);
        if (src_code_end==pcode) break;
#endif
        code=*pcode;
        value=(value<<3) | ((code>>4)&((1<<3)-1));
        isHaveNext=code&(1<<(3+4));
        if (isHaveNext){
            ++pcode;
            value=(value<<3) | (code&((1<<3)-1));
            isHaveNext=code&(1<<3);
        }else{
            isHaveHalfByte=1;
            break;
        }
    } while (isHaveNext);
    (*src_code)=pcode;
    (*_isHaveHalfByte)=isHaveHalfByte;
    return value;
}


frz_BOOL _PRIVATE_FRZ2_DECOMPRESS_NAME(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* zip_code,const unsigned char* zip_code_end){
    TFRZ_UInt32 ctrlCount;
    TFRZ_UInt32 ctrlIndex;
    TFRZ_UInt32 ctrlHalfLengthBufSize;
    TFRZ_UInt32 frontMatchPosBufSize;
    TFRZ_UInt32 kMinMatchLength;
    const TFRZ_Byte* ctrlBuf;
    //const TFRZ_Byte* ctrlBuf_end;
    const TFRZ_Byte* ctrlHalfLengthBuf;
    const TFRZ_Byte* ctrlHalfLengthBuf_end;
    const TFRZ_Byte* frontMatchPosBuf;
    const TFRZ_Byte* frontMatchPosBuf_end;
    TFRZ_UInt32 length;
    enum TFRZ2CodeType type;
    int  isHaveHalfByte;
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
    TFRZ_Byte* _out_data_begin;
    _out_data_begin=out_data;
#endif
    
    ctrlCount= _PRIVATE_FRZ_unpack32BitWithTag_NAME(&zip_code,zip_code_end,0);
    ctrlHalfLengthBufSize= _PRIVATE_FRZ_unpack32BitWithTag_NAME(&zip_code,zip_code_end,0);
    frontMatchPosBufSize= _PRIVATE_FRZ_unpack32BitWithTag_NAME(&zip_code,zip_code_end,0);
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
    if (zip_code>=zip_code_end) return frz_FALSE;
#endif
    kMinMatchLength=*zip_code; ++zip_code;
    
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
    if (((ctrlCount+7)>>3)>(TFRZ_UInt32)(zip_code_end-zip_code)) return frz_FALSE;
#endif
    ctrlBuf=zip_code;
    zip_code+=((ctrlCount+7)>>3);
    //ctrlBuf_end=zip_code;
    
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
    if (ctrlHalfLengthBufSize>(TFRZ_UInt32)(zip_code_end-zip_code)) return frz_FALSE;
#endif
    ctrlHalfLengthBuf=zip_code;
    zip_code+=ctrlHalfLengthBufSize;
    ctrlHalfLengthBuf_end=zip_code;
    
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
    if (frontMatchPosBufSize>(TFRZ_UInt32)(zip_code_end-zip_code)) return frz_FALSE;
#endif
    frontMatchPosBuf=zip_code;
    zip_code+=frontMatchPosBufSize;
    frontMatchPosBuf_end=zip_code;
    
    isHaveHalfByte=0;
    for (ctrlIndex=0;ctrlCount>0; ++ctrlIndex,--ctrlCount) {
        ctrlBuf+=(ctrlIndex>>3);
        ctrlIndex&=7;
        type=(enum TFRZ2CodeType)(((*ctrlBuf)>>ctrlIndex)&0x1);
        length=_PRIVATE_FRZ_unpack32BitWithHalfByte_NAME(&ctrlHalfLengthBuf,ctrlHalfLengthBuf_end,&isHaveHalfByte);
        switch (type){
            case kFRZ2CodeType_zip:{
                length+=kMinMatchLength;
                const TFRZ_UInt32 frontMatchPos= 1 + _PRIVATE_FRZ_unpack32BitWithTag_NAME(&frontMatchPosBuf,frontMatchPosBuf_end,0);
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
                if (length>(TFRZ_UInt32)(out_data_end-out_data)) return frz_FALSE;
                if (frontMatchPos>(TFRZ_UInt32)(out_data-_out_data_begin)) return frz_FALSE;
#endif
                if (length<=frontMatchPos)
                    memcpy_tiny(out_data,out_data-frontMatchPos,length);
                else
                    memcpy_order(out_data,out_data-frontMatchPos,length);//warning!! can not use memmove
                out_data+=length;
                continue; //for
            }break;
            case kFRZ2CodeType_nozip:{
                ++length;
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
                if (length>(TFRZ_UInt32)(out_data_end-out_data)) return frz_FALSE;
                if (length>(TFRZ_UInt32)(zip_code_end-zip_code)) return frz_FALSE;
#endif
                memcpy_tiny(out_data,zip_code,length);
                zip_code+=length;
                out_data+=length;
                continue; //for
            }break;
        }
    }
    return (zip_code==zip_code_end)&&(out_data==out_data_end)&&(frontMatchPosBuf==frontMatchPosBuf_end)
                &&((ctrlHalfLengthBuf_end-ctrlHalfLengthBuf)/2==0);
}

#endif //_PRIVATE_FRZ_DECOMPRESS_NEED_INCLUDE_CODE