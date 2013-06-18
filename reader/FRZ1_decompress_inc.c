//  FRZ1_decompress_inc.c
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

//变长32bit正整数编码方案(x bit额外类型标志位,x<=3),从高位开始输出1-5byte:
// x0*  7-x bit
// x1* 0*  7+7-x bit
// x1* 1* 0*  7+7+7-x bit
// x1* 1* 1* 0*  7+7+7+7-x bit
// x1* 1* 1* 1* 0*  7+7+7+7+7-x bit
static inline TFRZ_UInt32 _PRIVATE_FRZ_unpack32BitWithTag_NAME(const TFRZ_Byte** src_code,const TFRZ_Byte* src_code_end,const int kTagBit){//读出整数并前进指针.
    const TFRZ_Byte* pcode;
    TFRZ_UInt32 value;
    TFRZ_UInt32 code;
    pcode=*src_code;
    
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
    if (src_code_end-pcode<=0) return 0;
#endif
    code=*pcode; ++pcode;
    value=code&((1<<(7-kTagBit))-1);
    if ((code&(1<<(7-kTagBit)))){
        do {
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
            //assert((value>>(sizeof(value)*8-7))==0);
            if (src_code_end==pcode) break;
#endif
            code=*pcode; ++pcode;
            value=(value<<7) | (code&((1<<7)-1));
        } while ((code&(1<<7)));
    }
    (*src_code)=pcode;
    return value;
}


frz_BOOL _PRIVATE_FRZ_DECOMPRESS_NAME(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* zip_code,const unsigned char* zip_code_end){
    TFRZ_UInt32 ctrlSize;
    const TFRZ_Byte* ctrlBuf;
    const TFRZ_Byte* ctrlBuf_end;
    enum TFRZCodeType type;
    TFRZ_UInt32 length;
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
    TFRZ_Byte* _out_data_begin;
#endif
    
    ctrlSize= _PRIVATE_FRZ_unpack32BitWithTag_NAME(&zip_code,zip_code_end,0);
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
    _out_data_begin=out_data;
    if (ctrlSize>(TFRZ_UInt32)(zip_code_end-zip_code)) return frz_FALSE;
#endif
    ctrlBuf=zip_code;
    zip_code+=ctrlSize;
    ctrlBuf_end=zip_code;
    while (ctrlBuf<ctrlBuf_end){
        type=(enum TFRZCodeType)((*ctrlBuf)>>(8-kFRZCodeType_bit));
        length= 1 + _PRIVATE_FRZ_unpack32BitWithTag_NAME(&ctrlBuf,ctrlBuf_end,kFRZCodeType_bit);
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
        if (length>(TFRZ_UInt32)(out_data_end-out_data)) return frz_FALSE;
#endif
        switch (type){
            case kFRZCodeType_zip:{
                const TFRZ_UInt32 frontMatchPos= 1 + _PRIVATE_FRZ_unpack32BitWithTag_NAME(&ctrlBuf,ctrlBuf_end,0);
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
                if (frontMatchPos>(TFRZ_UInt32)(out_data-_out_data_begin)) return frz_FALSE;
#endif
                if (length<=frontMatchPos)
                    memcpy_tiny(out_data,out_data-frontMatchPos,length);
                else
                    memcpy_order(out_data,out_data-frontMatchPos,length);//warning!! can not use memmove
                out_data+=length;
                continue; //while
            }break;
            case kFRZCodeType_nozip:{
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
                if (length>(TFRZ_UInt32)(zip_code_end-zip_code)) return frz_FALSE;
#endif
                memcpy_tiny(out_data,zip_code,length);
                zip_code+=length;
                out_data+=length;
                continue; //while
            }break;
        }
    }
    return (ctrlBuf==ctrlBuf_end)&&(zip_code==zip_code_end)&&(out_data==out_data_end);
}

#endif //_PRIVATE_FRZ_DECOMPRESS_NEED_INCLUDE_CODE