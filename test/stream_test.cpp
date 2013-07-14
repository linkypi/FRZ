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

#ifdef _IOS
std::string TEST_FILE_DIR =std::string(getSourcesPath())+"/testFRZ/";
#else
std::string TEST_FILE_DIR ="/Users/Shared/test/testFRZ/";
#endif


static void FRZ_write_file(void* file,const unsigned char* code,const unsigned char* code_end){
    fwrite(code,1,code_end-code,(FILE*)file);
}

static void testCompressFile(const char* srcFileName,const char* frz2StreamFileName){
    FILE* in_file=fopen(srcFileName, "rb");
    FILE* out_file=fopen(frz2StreamFileName, "wb");
    assert(in_file);
    assert(out_file);
    
    const int kMStep=1024*150; //for test
    TFRZ2_stream_compress_handle  compress_handle=FRZ2_stream_compress_create(0,2*kMStep,FRZ_write_file,out_file,9*kMStep);
    std::vector<unsigned char> buf(2*kMStep);
    while (true) {
        long bufSize=fread(&buf[0],1,buf.size(),in_file);
        if (bufSize>0) {
            FRZ2_stream_compress_append_data(compress_handle,&buf[0], &buf[0]+bufSize);
        }else
            break;
    }
    FRZ2_stream_compress_delete(compress_handle);
    
    fclose(in_file);
    fclose(out_file);
}

struct T_testDecompressFile{
    FILE*                       in_file;
    FILE*                       out_file;
    std::vector<unsigned char>  in_buf;
    std::vector<unsigned char>  out_buf;
    int                         saved_curDecompressWindowsSize;
    static unsigned char* read(void* read_callBackData,int readSize){
        T_testDecompressFile& data=*(T_testDecompressFile*)read_callBackData;
        if (data.in_buf.size()<readSize) data.in_buf.resize(readSize);
        long readedSize=fread(&data.in_buf[0],1,readSize,data.in_file);
        if (readedSize==readSize)
            return &data.in_buf[0];
        else
            return 0;
    }
    static void write_init (void* write_callBackData,int kMaxDecompressWindowsSize,int kMaxStepMemorySize){
        T_testDecompressFile& data=*(T_testDecompressFile*)write_callBackData;
        data.out_buf.reserve(kMaxDecompressWindowsSize+kMaxStepMemorySize);
    }
    static unsigned char* write_begin (void* write_callBackData,int curDecompressWindowsSize,int dataSize){
        T_testDecompressFile& data=*(T_testDecompressFile*)write_callBackData;
        assert(curDecompressWindowsSize<=data.out_buf.size());
        data.out_buf.erase(data.out_buf.begin(), data.out_buf.end()-curDecompressWindowsSize);
        data.out_buf.resize(curDecompressWindowsSize+dataSize);
        data.saved_curDecompressWindowsSize=curDecompressWindowsSize;
        return &data.out_buf[0];
    }
    static void write_end (void* write_callBackData){
        T_testDecompressFile& data=*(T_testDecompressFile*)write_callBackData;
        fwrite(&data.out_buf[data.saved_curDecompressWindowsSize],1,data.out_buf.size()-data.saved_curDecompressWindowsSize,data.out_file);
    }
};

static bool testIsEqFile(const char* fileName0,const char* fileName1){
    FILE* file0=fopen(fileName0, "rb");
    FILE* file1=fopen(fileName1, "rb");
    assert(file0);
    assert(file1);
    
    bool result=true;
    std::vector<unsigned char> buf0(1*1024*1024);
    std::vector<unsigned char> buf1(buf0.size());
    while (true) {
        long bufSize0=fread(&buf0[0],1,buf0.size(),file0);
        long bufSize1=fread(&buf1[0],1,buf1.size(),file1);
        if (bufSize0!=bufSize1) { result=false; break; }
        if (buf0!=buf1) { result=false; break; }
        if (bufSize0==0) { break; }
    }
    fclose(file0);
    fclose(file1);
    return result;
}

static void testDecompressFile(const char* dstFileName,const char* frz2StreamFileName){
    FILE* frz_file=fopen(frz2StreamFileName, "rb");
    FILE* out_file=fopen(dstFileName, "wb");
    assert(frz_file);
    assert(out_file);
    
    T_testDecompressFile testDecompressFileData;
    testDecompressFileData.in_file=frz_file;
    testDecompressFileData.out_file=out_file;
    TFRZ2_decompress_stream stream;
    stream.read_callBackData=&testDecompressFileData;
    stream.write_callBackData=&testDecompressFileData;
    stream.read=T_testDecompressFile::read;
    stream.write_init=T_testDecompressFile::write_init;
    stream.write_begin=T_testDecompressFile::write_begin;
    stream.write_end=T_testDecompressFile::write_end;
    
    int ret=FRZ2_decompress_stream_safe(&stream);
    assert(ret!=frz_FALSE);
    
    fclose(out_file);
    fclose(frz_file);
}


static void testFile(const char* _srcFileName){
    std::string srcFileName(TEST_FILE_DIR); srcFileName+=_srcFileName;
    std::string frz2FileName(srcFileName+".frz2");
    testCompressFile(srcFileName.c_str(),frz2FileName.c_str());
    
    std::string dstUnFrz2FileName(srcFileName+".unfrz2");
    testDecompressFile(dstUnFrz2FileName.c_str(),frz2FileName.c_str());
    
    assert(testIsEqFile(srcFileName.c_str(),dstUnFrz2FileName.c_str()));
}

int main(){
    std::cout << "start> \n";
    
    testFile("empty.txt");
    testFile("test1.txt");
    testFile("test.txt");
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

