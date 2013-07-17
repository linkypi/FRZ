//  FRZ_stream_decompress_inc.c
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

frz_BOOL _PRIVATE_FRZ_stream_decompress_NAME(const struct TFRZ_stream_decompress* stream){
    TFRZ_Int32 codeSize;
    TFRZ_Int32 dataSize;
    TFRZ_Byte* out_data;
    TFRZ_Byte* windows_data;
    const TFRZ_Byte* zip_code;
    const TFRZ_Byte* zip_code_end;
    TFRZ_Int32 kMaxDecompressWindowsSize;
    TFRZ_Int32 kMaxStepMemorySize;
    TFRZ_Int32 curDecompressWindowsSize;
    curDecompressWindowsSize=0;
    
    while (1) {
        zip_code=stream->read(stream->read_callBackData,1);
        if (zip_code==0) return frz_TRUE; //finish
        codeSize=*zip_code; //head code size
        zip_code=stream->read(stream->read_callBackData,codeSize);
        zip_code_end=zip_code+codeSize;
        dataSize=_PRIVATE_FRZ_unpack32BitWithTag_NAME(&zip_code,zip_code_end,0);
        codeSize=_PRIVATE_FRZ_unpack32BitWithTag_NAME(&zip_code,zip_code_end,0);
        if (zip_code<zip_code_end){
            kMaxDecompressWindowsSize=_PRIVATE_FRZ_unpack32BitWithTag_NAME(&zip_code,zip_code_end,0);
            kMaxStepMemorySize=_PRIVATE_FRZ_unpack32BitWithTag_NAME(&zip_code,zip_code_end,0);
            stream->write_init(stream->write_callBackData,kMaxDecompressWindowsSize,kMaxStepMemorySize);
        }
        
        zip_code=stream->read(stream->read_callBackData,codeSize);
        windows_data=stream->write_begin(stream->write_callBackData,curDecompressWindowsSize,dataSize);
        out_data=windows_data+curDecompressWindowsSize;
#ifdef _PRIVATE_FRZ_DECOMPRESS_RUN_MEM_SAFE_CHECK
        if (!_PRIVATE_FRZ_decompress_safe_windows_NAME(out_data,out_data+dataSize,zip_code,zip_code+codeSize,windows_data)) return frz_FALSE;
#else
        if (!_PRIVATE_FRZ_decompress_NAME(out_data,out_data+dataSize,zip_code,zip_code+codeSize)) return frz_FALSE;
#endif
        stream->write_end(stream->write_callBackData);
        curDecompressWindowsSize+=dataSize;
        if (curDecompressWindowsSize>kMaxDecompressWindowsSize)
            curDecompressWindowsSize=kMaxDecompressWindowsSize;
    }
}


#endif //_PRIVATE_FRZ_DECOMPRESS_NEED_INCLUDE_CODE