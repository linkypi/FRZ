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
#include "../writer/FRZ_stream_compress.h"
#include "../reader/FRZ1_decompress.h"
#include "../reader/FRZ2_decompress.h"
#include "../reader/FRZ_stream_decompress.h"

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
    
    const int kMStep=1024*256; //for test
    TFRZ_stream_compress_handle compress_handle=FRZ2_stream_compress_best_create(0,2*kMStep,FRZ_write_file,out_file,21*kMStep);
    std::vector<unsigned char> buf(4*kMStep);
    while (true) {
        long bufSize=fread(&buf[0],1,buf.size(),in_file);
        if (bufSize>0) {
            FRZ_stream_compress_append_data(compress_handle,&buf[0], &buf[0]+bufSize);
        }else
            break;
    }
    FRZ_stream_compress_delete(compress_handle);
    
    fclose(in_file);
    fclose(out_file);
}


static bool testIsEqFile(const char* fileName0,const char* fileName1){
    FILE* file0=fopen(fileName0, "rb");
    FILE* file1=fopen(fileName1, "rb");
    assert(file0);
    assert(file1);
    
    bool result=true;
    std::vector<unsigned char> buf0(1*1024*1024);
    std::vector<unsigned char> buf1=buf0;
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
    
    const int kMaxHeadCodeSize=5*4;
    TFRZ_stream_head head;
    FRZ_stream_head_init(&head);
    unsigned char* in_buf=0;
    int in_buf_size=0;
    unsigned char* out_buf=0;
    int out_buf_size=0;
    int out_buf_used_size=0;
    while (true) {
        unsigned char headCode[kMaxHeadCodeSize];
        long readed=fread(&headCode[0],1,kSize_of_FRZ_stream_head_size,frz_file);
        if (readed!=kSize_of_FRZ_stream_head_size) break; //ok
        int headSize=FRZ_stream_decompress_head_size(&headCode[0], &headCode[0]+kSize_of_FRZ_stream_head_size);
        assert(headSize<=kMaxHeadCodeSize);
        
        readed=fread(&headCode[0],1,headSize,frz_file);
        assert(readed==headSize);
        FRZ_stream_decompress_head(&headCode[0], &headCode[0]+headSize,&head);
        if (in_buf==0){
            in_buf_size=head.data_max_step_size+(head.data_max_step_size>>4)+32;
            in_buf=new unsigned char[in_buf_size];
            out_buf_size=head.data_windows_size+head.data_max_step_size;
            out_buf=new unsigned char[out_buf_size];
        }
        assert(in_buf_size>=head.frz_code_size);
        assert(out_buf_size>=head.data_size+head.data_windows_size);
        
        readed=fread(&in_buf[0],1,head.frz_code_size,frz_file);
        assert(readed==head.frz_code_size);
        int curWindowsSize=out_buf_used_size;
        if (curWindowsSize>head.data_windows_size) {
            memcpy(&out_buf[0], &out_buf[0]+curWindowsSize-head.data_windows_size,head.data_windows_size);
            curWindowsSize=head.data_windows_size;
        }
        out_buf_used_size=curWindowsSize+head.data_size;
        
        int ret=FRZ2_decompress_windows(&out_buf[0], &out_buf[0]+curWindowsSize, &out_buf[0]+out_buf_used_size, &in_buf[0], &in_buf[0]+head.frz_code_size);
        assert(ret!=frz_FALSE);
        fwrite(&out_buf[curWindowsSize],1,out_buf_used_size-curWindowsSize,out_file);
    }
    delete []in_buf;
    delete []out_buf;
    
    fclose(out_file);
    fclose(frz_file);
}


static void testFile(const char* _srcFileName){
    std::cout<<_srcFileName<<"\n";
    std::string srcFileName(TEST_FILE_DIR); srcFileName+=_srcFileName;
    std::string frz2FileName(srcFileName+".frz2");
    testCompressFile(srcFileName.c_str(),frz2FileName.c_str());
    
    std::string dstUnFrz2FileName(srcFileName+".unfrz2");
    testDecompressFile(dstUnFrz2FileName.c_str(),frz2FileName.c_str());
    
    assert(testIsEqFile(srcFileName.c_str(),dstUnFrz2FileName.c_str()));
}

int main(){
    std::cout << "start> \n";
    clock_t time1=clock();
    const int testDecompressCount=1;
    for (int i=0; i<testDecompressCount; ++i) {
        testFile("empty.txt");
        testFile("test1.txt");
        testFile("test.txt");
        testFile("endict.txt");
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
    }

    
    clock_t time2=clock();
    double decompressTime_s=(time2-time1)*1.0/CLOCKS_PER_SEC/testDecompressCount;
    std::cout << decompressTime_s*1000<<" ms  ";
    
    std::cout << "done!\n";
    return 0;
}

