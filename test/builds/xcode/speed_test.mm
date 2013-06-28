//
//  unit_test.cpp
//  for FRZ
//

#define _IOS
#ifdef _IOS
#import <UIKit/UIKit.h>
#else
#ifdef _MACOSX
#import <Cocoa/Cocoa.h>
#endif
#endif

//SYS_FILE_NEED_SOURCES_PATH
#define kPathOrFileNameMaxLength 1024
static const char* getSourcesPath(){
	static const char* _sourcePath=0;
	if (_sourcePath==0){
		static char sourcePath[kPathOrFileNameMaxLength];
		NSString* nsSourcePath=[[NSBundle mainBundle] resourcePath];//bundlePath;
        [nsSourcePath getCString:sourcePath maxLength:kPathOrFileNameMaxLength encoding:NSUTF8StringEncoding];
        _sourcePath=sourcePath;
	}
	return _sourcePath;
}

#include "../../speed_test.cpp"
