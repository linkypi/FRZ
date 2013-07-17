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

//变长10bit正整数编码方案,从高位开始输出0.5--1.5byte:
// 0*  3     bit
// 10  2+4   bit
// 11  2+4+4 bit
static inline TFRZ_UInt32 _PRIVATE_FRZ_unpack32BitWithHalfByte_NAME(const TFRZ_Byte** src_code,const TFRZ_Byte* src_code_end,TFRZ_UInt32* _halfByte){//读出整数并前进指针.
    TFRZ_UInt32 value;
    TFRZ_UInt32 code;
    TFRZ_UInt32 codeNext;
    const TFRZ_Byte* pcode;
    code=*_halfByte;
    pcode=*src_code;
    if (code==0){
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
        if (pcode>=src_code_end) return 0;
#endif
        code=pcode[0]; ++pcode;
        switch (code>>6) {
            case 3:{
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
                if (pcode>=src_code_end) return 0;
#endif
                codeNext=*pcode;
                (*src_code)=pcode+1;
                (*_halfByte)=codeNext | (15<<4);
                value=((code&((1<<6)-1))<<4) | (codeNext>>4);
                return value+(8+64);
            }
            case 2:{
                (*src_code)=pcode;
                //(*_halfByte)=0;
                value=(code&((1<<6)-1));
                return value+(8);
            }
            case 1:
            case 0:{
                (*src_code)=pcode;
                (*_halfByte)=code | (15<<4);
                return (code>>4);
            }
        }
    }else{
        code&=15;
        switch (code>>2) {
            case 3:{
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
                if (pcode>=src_code_end) return 0;
#endif
                (*src_code)=pcode+1;
                (*_halfByte)=0;
                value=((code&3)<<8) | (*pcode);
                return value+(8+64);
            }
            case 2:{
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
                if (pcode>=src_code_end) return 0;
#endif
                codeNext=*pcode;
                (*src_code)=pcode+1;
                (*_halfByte)=codeNext | (15<<4);
                value=((code&3)<<4) | (codeNext>>4);
                return value+(8);
            }
            case 1:
            case 0:{
                //(*src_code)=pcode;
                (*_halfByte)=0;
                return code;
            }
        }
    }
    //assert(false);
    return 0;
}


#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
static frz_BOOL _FRZ2_decompress_safe_windows(TFRZ_Byte* out_data,TFRZ_Byte* out_data_end,const TFRZ_Byte* zip_code,const TFRZ_Byte* zip_code_end
                                       ,TFRZ_Byte*  _out_data_begin){
#else
    frz_BOOL FRZ2_decompress(TFRZ_Byte* out_data,TFRZ_Byte* out_data_end,const TFRZ_Byte* zip_code,const TFRZ_Byte* zip_code_end){
#endif

    const TFRZ_Byte* src_data;
    TFRZ_UInt32 ctrls24;
    TFRZ_UInt32 frontMatchPos;
    TFRZ_UInt32 length;
    TFRZ_UInt32 halfByte;
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
    if (zip_code>zip_code_end) return frz_FALSE;
    if (out_data>out_data_end) return frz_FALSE;
#endif
    halfByte=0;
    ctrls24=1; //empty
    for (;zip_code<zip_code_end;ctrls24>>=1) {
        if (ctrls24==1){
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
            if (3>(TFRZ_UInt32)(zip_code_end-zip_code)) return frz_FALSE;
#endif
            ctrls24=(zip_code[0]<<16) | (zip_code[1]<<8) | (zip_code[2]) | (1<<24); zip_code+=3;
        }
        length=_PRIVATE_FRZ_unpack32BitWithHalfByte_NAME(&zip_code,zip_code_end,&halfByte);
        switch ((enum TFRZ2CodeType)(ctrls24&0x01)){
            case kFRZ2CodeType_zip:{
                length+=kMinMatchLength;
                frontMatchPos= 1 + _PRIVATE_FRZ_unpack32BitWithTag_NAME(&zip_code,zip_code_end,0);
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
                if ((length==0)||(length>(TFRZ_UInt32)(out_data_end-out_data))) return frz_FALSE;
                if ((frontMatchPos==0)||(frontMatchPos>(TFRZ_UInt32)(out_data-_out_data_begin))) return frz_FALSE;
#endif
                out_data+=length;
                src_data=out_data-frontMatchPos;
                if (length>frontMatchPos){
                    memcpy_order_end(out_data,src_data,length);//warning!! can not use memmove
                    continue; //while
                }
            }break;
            case kFRZ2CodeType_nozip:{
                ++length;
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
                if ((length==0)||(length>(TFRZ_UInt32)(out_data_end-out_data))) return frz_FALSE;
                if (length>(TFRZ_UInt32)(zip_code_end-zip_code)) return frz_FALSE;
#endif                
                zip_code+=length;
                out_data+=length;
                src_data=zip_code;
            }break;
        }
        memcpy_tiny64_end(out_data,src_data,length);
    }
    return (zip_code==zip_code_end)&&(out_data==out_data_end);
}

#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
frz_BOOL FRZ2_decompress_safe(TFRZ_Byte* out_data,TFRZ_Byte* out_data_end,const TFRZ_Byte* zip_code,const TFRZ_Byte* zip_code_end){
    return _FRZ2_decompress_safe_windows(out_data,out_data_end,zip_code,zip_code_end,out_data);
}
#endif


#endif //_PRIVATE_FRZ_DECOMPRESS_NEED_INCLUDE_CODE