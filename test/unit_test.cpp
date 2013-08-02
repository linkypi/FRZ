//
//  unit_test.cpp
//  for FRZ
//

#include <iostream>
#include <string.h>
#include <math.h>
#include "../writer/FRZ1_compress.h"
#include "../writer/FRZ2_compress.h"
#include "../reader/FRZ1_decompress.h"
#include "../reader/FRZ2_decompress.h"


int     error_count=0;
double  sum_src_size=0;
double  sum_frz1_size=0;
double  sum_frz2_size=0;

typedef void (*TFRZ_compress)(std::vector<unsigned char>& out_code,const unsigned char* src,const unsigned char* src_end,int zip_parameter);
typedef frz_BOOL (*TFRZ_decompress)(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* frz_code,const unsigned char* frz_code_end);

static int test(const unsigned char* src,const unsigned char* src_end,const char* tag,
                 TFRZ_compress FRZ_compress,const char* FRZ_compress_Name,
                 TFRZ_decompress FRZ_decompress,const char* FRZ_decompress_Name){
    std::vector<unsigned char> compressedCode;
    FRZ_compress(compressedCode,src,src_end,0);
    
    std::vector<unsigned char> uncompressedCode(src_end-src,0);
    frz_BOOL ret=FRZ_decompress(&uncompressedCode[0],&uncompressedCode[0]+uncompressedCode.size(), &compressedCode[0], &compressedCode[0]+compressedCode.size());
    if (!ret){
        ++error_count;
        std::cout << "\nerror_count=="<<error_count<<", "<<FRZ_decompress_Name<<" result error, tag==\""<<tag<<"\"\n";
    }else if (uncompressedCode!=std::vector<unsigned char>(src,src_end)){
        ++error_count;
        std::cout << "\nerror_count=="<<error_count<<", "<<FRZ_decompress_Name<<" data error, tag==\""<<tag<<"\"\n";
    }else{
        std::cout << "error_count=="<<error_count<<", test ok "<<FRZ_compress_Name<<" frzSize/srcSize:"<<compressedCode.size()<<"/"<<src_end-src<<", tag==\""<<tag<<"\"\n";
    }
    return (int)compressedCode.size();
}
static void testFRZ(const unsigned char* src,const unsigned char* src_end,const char* tag){
    sum_src_size+=src_end-src;
    sum_frz1_size+=test(src,src_end,tag,FRZ1_compress,"FRZ1_compress",FRZ1_decompress_safe,"FRZ1_decompress_safe");
    sum_frz2_size+=test(src,src_end,tag,FRZ2_compress,"FRZ2_compress",FRZ2_decompress_safe,"FRZ2_decompress_safe");
}

static void testFRZ(const char* src,const char* tag){
    if (src!=0)
        testFRZ((const unsigned char*)src,(const unsigned char*)src+strlen(src),tag);
    else
        testFRZ(0, 0,tag);
}

int main(int argc, const char * argv[]){    
    testFRZ(0,"null");
    testFRZ("","tag0");
    testFRZ("1","tag1");
    testFRZ("11","tag3");
    testFRZ("111","tag4");
    testFRZ("1111","tag5");
    testFRZ("11111","tag6");
    testFRZ("1111111111","tag7");
    testFRZ("11111111111111111111111111111111111111111111111111111111111111111111111111111111","tag8");
    testFRZ("1111111111111111111111111234111111111111111111111111111111111111111111111111","tag9");
    testFRZ("121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212","tag10");
    testFRZ("12121212121212121212g12121212121212121212121g21212121212121212121212125121212121231212122121212121212121212","tag11");
    testFRZ("34tg5h45y6hdfknw23u8ey23eewbd8djny45n54n89dfhuvvbe78fh43ufjhbvdsuy673673fb4ggbhhjerfuy34gfbehjfberuiyg734gfbhj34fjh34bf","tag12");
    testFRZ("23454645756879234135464575686778233425346457657685723534645765876876876978987234235435465476587698797436547658763254364575647568","tag13");
    
    
    const int kRandTestCount=10000;
    const int kMaxDataSize=1024*65;
    const int kMaxCopyCount=5000;
    std::vector<int> seeds(kRandTestCount);
    //srand( (unsigned int)time(0) );
    for (int i=0; i<kRandTestCount; ++i)
        seeds[i]=rand();
    
    
    for (int i=0; i<kRandTestCount; ++i) {
        char tag[50];
        sprintf(tag, "testSeed=%d",seeds[i]);
        srand(seeds[i]);
        
        const int srcSize=(int)(pow(rand()*(1.0/RAND_MAX),3)*kMaxDataSize);
        std::vector<unsigned char> _srcData(srcSize);
        unsigned char* srcData=0; if (!_srcData.empty()) srcData=&_srcData[0];
        for (int i=0; i<srcSize; ++i)
            srcData[i]=rand();
        const int copyCount=(int)(rand()*(1.0/RAND_MAX)*kMaxCopyCount);
        const int kMaxCopyLength=1+(int)(pow(rand()*(1.0/RAND_MAX),3)*srcSize*0.3);
        for (int i=0; i<copyCount; ++i) {
            const int length=2+(int)(pow(rand()*(1.0/RAND_MAX),6)*kMaxCopyLength);
            if (length>=srcSize) {
                continue;
            }
            const int oldPos=rand()%(srcSize-length);
            const int newPos=rand()%(srcSize-length);
            memmove(&srcData[0]+newPos, &srcData[0]+oldPos, length);
        }
        testFRZ(&srcData[0],&srcData[0]+srcSize,tag);
    }
    
    std::cout << "\n  error_count=="<<error_count<<"\n";
    std::cout <<"  FRZ1_compress "<<" sum frz1Size/srcSize:"<<sum_frz1_size/sum_src_size<<"\n";
    std::cout <<"  FRZ2_compress "<<" sum frz2Size/srcSize:"<<sum_frz2_size/sum_src_size<<"\n";
    std::cout << "\ndone!\n";
    return error_count;
}

