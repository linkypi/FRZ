//
//  unit_test.cpp
//  for FRZ
//
#include <iostream>
#include <string.h>
#include <math.h>
#include <time.h>
#include "../writer/FRZ1_compress.h"
#include "../writer/FRZ2_compress.h"
#include "../reader/FRZ1_decompress.h"
#include "../reader/FRZ2_decompress.h"
#include "lzo1x.h"
#include "zlib.h"


#ifdef _IOS
std::string TEST_FILE_DIR =std::string(getSourcesPath())+"/testFRZ/";
#else
std::string TEST_FILE_DIR ="/Users/Shared/test/testFRZ/";
#endif


void zip_compress(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter);
int zip_decompress(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* zip_code,const unsigned char* zip_code_end);
void lzo_compress(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter);
int lzo_decompress(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* lzo_code,const unsigned char* lzo_code_end);
int lzo_decompress_safe(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* lzo_code,const unsigned char* lzo_code_end);

void readFile(std::vector<unsigned char>& data,const char* fileName){
    FILE	* file=fopen(fileName, "rb");
    assert(file);
	fseek(file,0,SEEK_END);
	int file_length = (int)ftell(file);
	fseek(file,0,SEEK_SET);
    
    data.resize(file_length);
    if (file_length>0)
        fread(&data[0],1,file_length,file);
    
    fclose(file);
}

void writeFile(const std::vector<unsigned char>& data,const char* fileName){
    FILE	* file=fopen(fileName, "wb");
    
    int dataSize=(int)data.size();
    if (dataSize>0)
        fwrite(&data[0], 1,dataSize, file);
    
    fclose(file);
}


typedef void (*T_compress)(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter);
typedef frz_BOOL (*T_decompress)(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* zip_code,const unsigned char* zip_code_end);

struct TTestResult {
    std::string     procName;
    std::string     srcFileName;
    int             zip_parameter;
    double          compressTime_s;
    double          decompressTime_s;
    int             srcSize;
    int             zipSize;
};


double testDecodeProc(T_decompress proc_decompress,unsigned char* out_data,unsigned char* out_data_end,const unsigned char* zip_code,const unsigned char* zip_code_end){
    int testDecompressCount=0;
    clock_t time1=clock();
    for (;(clock()-time1)<CLOCKS_PER_SEC;) {
        for (int i=0; i<10; ++i){
            frz_BOOL ret=proc_decompress(out_data,out_data_end,zip_code,zip_code_end);
            ++testDecompressCount;
            if (!ret)
                throw "error result!";
        }
    }
    clock_t time2=clock();
    double decompressTime_s=(time2-time1)*1.0/CLOCKS_PER_SEC/testDecompressCount;
    return  decompressTime_s;
}


double testEncodeProc(T_compress proc_compress,std::vector<unsigned char>& compressedCode,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    int testCompressCount=0;
    clock_t time1=clock();
    for (;(clock()-time1)<CLOCKS_PER_SEC;) {
        compressedCode.clear();
        proc_compress(compressedCode,src,src_end,zip_parameter);
        ++testCompressCount;
    }
    clock_t time2=clock();
    double compressTime_s=(time2-time1)*1.0/CLOCKS_PER_SEC/testCompressCount;
    return compressTime_s;
}

TTestResult testProc(const char* srcFileName,T_compress proc_compress,const char* proc_compress_Name,
                 T_decompress proc_decompress,const char* proc_decompress_Name,int zip_parameter){
    
    std::vector<unsigned char> oldData; readFile(oldData,(TEST_FILE_DIR+srcFileName).c_str());
    const unsigned char* src=&oldData[0];
    const unsigned char* src_end=src+oldData.size();
    
    std::vector<unsigned char> compressedCode;
    double compressTime_s=testEncodeProc(proc_compress,compressedCode,src,src_end,zip_parameter);
    const unsigned char* unsrc=&compressedCode[0];
    
    std::vector<unsigned char> uncompressedCode(oldData.size(),0);
    unsigned char* undst=&uncompressedCode[0];
    
    double decompressTime_s=testDecodeProc(proc_decompress,undst,undst+uncompressedCode.size(),unsrc,unsrc+compressedCode.size());
    decompressTime_s=std::min(decompressTime_s,testDecodeProc(proc_decompress,undst,undst+uncompressedCode.size(),unsrc,unsrc+compressedCode.size()));
    decompressTime_s=std::min(decompressTime_s,testDecodeProc(proc_decompress,undst,undst+uncompressedCode.size(),unsrc,unsrc+compressedCode.size()));

    if (uncompressedCode!=oldData){
        throw "error data!";
    }
    
    TTestResult result;
    result.procName=proc_decompress_Name;
    result.srcFileName=srcFileName;
    result.compressTime_s=compressTime_s;
    result.decompressTime_s=decompressTime_s;
    result.srcSize=(int)(src_end-src);
    result.zipSize=(int)compressedCode.size();
    result.zip_parameter=zip_parameter;
    return result;
}


static void outResult(const TTestResult& rt){
    std::cout<<"\""<<rt.srcFileName<<"\"\t";
    std::cout<<rt.srcSize/1024.0/1024<<"M\t";
    std::cout<<rt.procName<<"_"<<rt.zip_parameter<<"\t";
    std::cout<<rt.zipSize*100.0/rt.srcSize<<"%\t";
    //std::cout<<rt.compressTime_s<<"S\t";
    std::cout<<rt.srcSize/rt.compressTime_s/1024/1024<<"M/S\t";
    //std::cout<<rt.decompressTime_s<<"S\t";
    std::cout<<rt.srcSize/rt.decompressTime_s/1024/1024<<"M/S\n";
}


static void testFile(const char* srcFileName){
    outResult(testProc(srcFileName,FRZ2_compress,"",FRZ2_decompress,"frz2",0));
    return;
    
    outResult(testProc(srcFileName,zip_compress,"",zip_decompress,"zlib",9));
    outResult(testProc(srcFileName,zip_compress,"",zip_decompress,"zlib",6));
    outResult(testProc(srcFileName,zip_compress,"",zip_decompress,"zlib",1));
    std::cout << "\n";
    
    outResult(testProc(srcFileName,lzo_compress,"",lzo_decompress,"lzo1x",999));
    outResult(testProc(srcFileName,lzo_compress,"",lzo_decompress_safe,"lzo1xSafe",999));
    outResult(testProc(srcFileName,lzo_compress,"",lzo_decompress,"lzo1x",1));
    outResult(testProc(srcFileName,lzo_compress,"",lzo_decompress_safe,"lzo1xSafe",1));
    outResult(testProc(srcFileName,lzo_compress,"",lzo_decompress,"lzo1x",15));
    outResult(testProc(srcFileName,lzo_compress,"",lzo_decompress,"lzo1x",12));
    outResult(testProc(srcFileName,lzo_compress,"",lzo_decompress,"lzo1x",11));
     std::cout << "\n";
    
    outResult(testProc(srcFileName,FRZ2_compress,"",FRZ2_decompress,"frz2",0));
    outResult(testProc(srcFileName,FRZ2_compress,"",FRZ2_decompress_safe,"frz2Safe",0));
    outResult(testProc(srcFileName,FRZ2_compress,"",FRZ2_decompress,"frz2",1));
    outResult(testProc(srcFileName,FRZ2_compress,"",FRZ2_decompress_safe,"frz2Safe",1));
    outResult(testProc(srcFileName,FRZ2_compress,"",FRZ2_decompress,"frz2",2));
    outResult(testProc(srcFileName,FRZ2_compress,"",FRZ2_decompress_safe,"frz2Safe",2));
    outResult(testProc(srcFileName,FRZ2_compress,"",FRZ2_decompress,"frz2",4));
    outResult(testProc(srcFileName,FRZ2_compress,"",FRZ2_decompress_safe,"frz2Safe",4));
    outResult(testProc(srcFileName,FRZ2_compress,"",FRZ2_decompress,"frz2",7));
    outResult(testProc(srcFileName,FRZ2_compress,"",FRZ2_decompress_safe,"frz2Safe",7));
    std::cout << "\n";
    
    outResult(testProc(srcFileName,FRZ1_compress,"",FRZ1_decompress,"frz1",0));
    outResult(testProc(srcFileName,FRZ1_compress,"",FRZ1_decompress_safe,"frz1Safe",0));
    outResult(testProc(srcFileName,FRZ1_compress,"",FRZ1_decompress,"frz1",1));
    outResult(testProc(srcFileName,FRZ1_compress,"",FRZ1_decompress_safe,"frz1Safe",1));
    outResult(testProc(srcFileName,FRZ1_compress,"",FRZ1_decompress,"frz1",2));
    outResult(testProc(srcFileName,FRZ1_compress,"",FRZ1_decompress_safe,"frz1Safe",2));
    outResult(testProc(srcFileName,FRZ1_compress,"",FRZ1_decompress,"frz1",4));
    outResult(testProc(srcFileName,FRZ1_compress,"",FRZ1_decompress_safe,"frz1Safe",4));
    outResult(testProc(srcFileName,FRZ1_compress,"",FRZ1_decompress,"frz1",7));
    outResult(testProc(srcFileName,FRZ1_compress,"",FRZ1_decompress_safe,"frz1Safe",7));
    std::cout << "\n";
    
    std::cout << "\n";
}

int main(){
    std::cout << "start> \n";
    
    //testFile("endict.txt");
    testFile("world95.txt");
    testFile("ohs.doc");
    testFile("FP.LOG");
    testFile("A10.jpg");
    testFile("rafale.bmp");
    testFile("FlashMX.pdf");
    testFile("vcfiu.hlp");
    testFile("AcroRd32.exe");
    testFile("MSO97.DLL");
    testFile("english.dic");
    
    std::cout << "done!\n";
    return 0;
}


////


#define HEAP_ALLOC(var,size) \
lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem, LZO1X_999_MEM_COMPRESS);

void lzo_compress(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    lzo_uint out_len = (src_end-src)+((src_end-src) / 16 + 64 + 3);
    out_code.resize(out_len,0);
    unsigned char* dst=&out_code[0];
    int r;
    switch (zip_parameter) {
        case 1:
            r = lzo1x_1_compress((lzo_bytep)src,src_end-src,(lzo_bytep)dst,&out_len,wrkmem);
            break;
        case 11:
            r = lzo1x_1_11_compress((lzo_bytep)src,src_end-src,(lzo_bytep)dst,&out_len,wrkmem);
            break;
        case 12:
            r = lzo1x_1_12_compress((lzo_bytep)src,src_end-src,(lzo_bytep)dst,&out_len,wrkmem);
            break;
        case 15:
            r = lzo1x_1_15_compress((lzo_bytep)src,src_end-src,(lzo_bytep)dst,&out_len,wrkmem);
            break;
        case 999:
            r = lzo1x_999_compress((lzo_bytep)src,src_end-src,(lzo_bytep)dst,&out_len,wrkmem);
            break;
        default:
            assert(false);
            break;
    }
    assert(r == LZO_E_OK);
    out_code.resize(out_len);
}

int lzo_decompress(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* lzo_code,const unsigned char* lzo_code_end){
    lzo_uint new_len=out_data_end-out_data;
    int  r = lzo1x_decompress(&lzo_code[0],lzo_code_end-lzo_code,&out_data[0],&new_len,NULL);
    assert(r == LZO_E_OK);
    return r==LZO_E_OK;
}

int lzo_decompress_safe(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* lzo_code,const unsigned char* lzo_code_end){
    lzo_uint new_len=out_data_end-out_data;
    int  r = lzo1x_decompress_safe(&lzo_code[0],lzo_code_end-lzo_code,&out_data[0],&new_len,NULL);
    assert(r == LZO_E_OK);
    return r==LZO_E_OK;
}


/**
 * 对内容进行压缩和编码工作
 */
void zip_compress(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter){
    const unsigned char* _zipSrc=&src[0];
    out_code.resize(src_end-src+1024); //only for test,unsafe !!!
    unsigned char* _zipDst=&out_code[0];
    
    //先对原始内容进行压缩工作
    z_stream c_stream;
    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;
    c_stream.next_in = (Bytef*)_zipSrc;
    c_stream.avail_in = (int)(src_end-src);
    c_stream.next_out = (Bytef*)_zipDst;
    c_stream.avail_out = (unsigned int)out_code.size();
    int ret = deflateInit2(&c_stream, zip_parameter,Z_DEFLATED, 31,8, Z_DEFAULT_STRATEGY);
    if(ret != Z_OK)
    {
        std::cout <<"|"<<"deflateInit2 error "<<std::endl;
        return;
    }
    ret = deflate(&c_stream, Z_FINISH);
    if (ret != Z_STREAM_END)
    {
        deflateEnd(&c_stream);
        std::cout <<"|"<<"ret != Z_STREAM_END err="<< ret <<std::endl;
        return;
    }
    
    int zipLen = (int)c_stream.total_out;
    ret = deflateEnd(&c_stream);
    if (ret != Z_OK)
    {
        std::cout <<"|"<<"deflateEnd error "<<std::endl;
        return;
    }
    //压缩完毕进行返回包组织
    out_code.resize(zipLen);
    return;
}


int zip_decompress(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* zip_code,const unsigned char* zip_code_end){

#define CHUNK 100000

    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];
    int totalsize = 0;
    
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    
    ret = inflateInit2(&strm, 31);
    
    if (ret != Z_OK)
        return ret;
    
    strm.avail_in = (int)(zip_code_end-zip_code);
    strm.next_in = (unsigned char*)zip_code;
    
    /* run inflate() on input until output buffer not full */
    do {
        strm.avail_out = CHUNK;
        strm.next_out = out;
        ret = inflate(&strm, Z_NO_FLUSH);
        switch (ret)
        {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR; /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                return ret;
        }
        
        have = CHUNK - strm.avail_out;
        memcpy(out_data + totalsize,out,have);
        totalsize += have;
        assert(out_data+totalsize<=out_data_end);
    } while (strm.avail_out == 0);
    
    /* clean up and return */
    inflateEnd(&strm);
    assert( ret == Z_STREAM_END );
    return true;
}

