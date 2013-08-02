//suffix_string.cpp
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
#include "suffix_string.h"
#include "assert.h"
#include <stdio.h>
#include <algorithm>
#include <stdexcept>
#include "sais.hxx"

namespace {
    
    typedef TSuffixString::TInt TInt;
    
    static bool getStringIsEqual(const char* str0,const char* str0End,const char* str1,const char* str1End){
        const TInt strSize=(TInt)(str0End-str0);
        if (strSize!=(str1End-str1))
            return false;
        for (TInt i=0;i<strSize;++i){
            if (str0[i]!=str1[i])
                return false;
        }
        return true;
    }
    
    static bool getStringIsLess(const char* str0,const char* str0End,const char* str1,const char* str1End){
        while ((str0!=str0End)&&(str1!=str1End)) {
            unsigned char a=*str0; ++str0;
            unsigned char b=*str1; ++str1;
            if(a!=b){
                return a<b;
            }
        }
        return (str1!=str1End);
    }
    
    struct StringToken{
    private:
        const char* m_begin;
        const char* m_end;
    public:
    public:
        inline StringToken():m_begin(0),m_end(0){}
        inline StringToken(const StringToken& value):m_begin(value.m_begin),m_end(value.m_end){}
        inline explicit StringToken(const char* _begin,const char* _end){ reset(_begin,_end); }
        inline void reset(const char* _begin,const char* _end){ m_begin=_begin; m_end=_end; }
        inline const char* begin()const { return m_begin; }
        inline const char* end()const { return m_end; }
        inline void swap(StringToken& value){ std::swap(m_begin,value.m_begin); std::swap(m_end,value.m_end); }
        
        inline bool operator==(const StringToken& value)const{
            return getStringIsEqual(m_begin,m_end,value.m_begin,value.m_end);
        }
        inline bool operator !=(const StringToken& value)const{ return !(*this==value); }
        inline TInt getStringLength()const{ return (TInt)(m_end-m_begin); }
        inline bool operator <(const StringToken& value)const{
            return getStringIsLess(m_begin,m_end,value.m_begin,value.m_end);
        }
    };
    
    class TSuffixString_compare{
    public:
        inline TSuffixString_compare(const char* begin,const char* end):m_begin(begin),m_end(end){}
        inline bool operator()(const TSuffixIndex s0,const StringToken& s1)const{
            return StringToken(m_begin+s0,m_end)<s1;
        }
        inline bool operator()(const StringToken& s0,const TSuffixIndex s1)const{
            return s0<StringToken(m_begin+s1,m_end);
        }
        inline bool operator()(const StringToken& s0,const StringToken& s1)const{
            return s0<s1;
        }
        inline bool operator()(const TSuffixIndex s0,const TSuffixIndex s1)const{
            return StringToken(m_begin+s0,m_end)<StringToken(m_begin+s1,m_end);
        }
    private:
        const char* m_begin;
        const char* m_end;
    };
    
    /* uses std::sort
     static void suffixString_create(const char* src,const char* src_end,std::vector<TSuffixIndex>* out_sstring){
         std::vector<TInt>& sstring=*out_sstring;
        //init sstring
        TInt size=(TInt)(src_end-src);
        sstring.resize(size);
        if (size<=0) return;
        for (TInt i=0;i<size;++i)
        sstring[i]=i;
     
        //sort sstring
        std::sort<TInt*,const TSuffixString_compare&>(&sstring[0],&sstring[0]+size,TSuffixString_compare(src,src_end));
    }
    //*/
    
    //* uses saisxx
    static void suffixString_create(const char* src,const char* src_end,std::vector<TSuffixIndex>* out_sstring){
        std::vector<TSuffixIndex>& sstring=*out_sstring;
        //init sstring
        TInt size=(TInt)(src_end-src);
        sstring.resize(size);
        if (size<=0) return;
        if (saisxx((const unsigned char*)src, &sstring[0], size) !=0){
            throw std::runtime_error("suffixString_create() error.");
        }
    }
    //*/
    
    template<class TLCPInt,TInt kMaxLCP=0>
    static void _LCP_create_withOutR(const char* T,TInt n,const TInt* SA,TLCPInt* LCP){
        //not need R
        if (n>0)
            LCP[n-1]=0;
        for (int i = 1; i < n; ++i) {
            TInt h = 0;
            while (((SA[i]+h)!=n)&&((SA[i-1]+h)!=n)&&(T[SA[i]+h]==T[SA[i-1]+h]))
                ++h;
            LCP[i-1] = h;
            TInt lcp=h;
            if ((kMaxLCP>0)&&(lcp>kMaxLCP))
                lcp=kMaxLCP;
            LCP[i-1] = (TLCPInt)lcp;
        }
        return;
    }
    
    template<class TLCPInt,TInt kMaxLCP=0>
    static void _LCP_create_withR(const char* T,TInt n,const TInt* SA,const TInt* R,TLCPInt* LCP){
        if (n>0)
            LCP[n-1]=0;
        for (TInt h = 0, i = 0; i < n; i++){
            if (R[i] == 0) continue;
            TInt j = SA[R[i]-1];
            while ((i+h!=n)&&(j+h!=n)&&(T[i+h] == T[j+h])) ++h;
            TInt lcp=h;
            if ((kMaxLCP>0)&&(lcp>kMaxLCP))
                lcp=kMaxLCP;
            LCP[R[i]-1] = (TLCPInt)lcp;
            if (h > 0) --h;
        }
    }
    
    static void _Rank_create(const char* T,TInt n,const TInt* SA,TInt* R){
        for (TInt i=0;i<n;++i){
            R[SA[i]]=i;
        }
    }
    
}//end namespace

TSuffixString::TSuffixString(const char* src_begin,const char* src_end){
    resetString(src_begin,src_end);
}
void TSuffixString::clear(){
    ssbegin=0;
    ssend=0;
    SA.clear();
    R.clear();
    LCP.clear();
    LCPLite.clear();
}
void TSuffixString::resetString(const char* src_begin,const char* src_end){
    assert(src_end>=src_begin);
    clear();
    ssbegin=src_begin;
    ssend=src_end;
    if(src_end>src_begin)
        suffixString_create(src_begin,src_end,&SA);
    /* test out
     printf("\nsstring:%s",src_begin);
     for (TInt i=0; i<SA.size(); ++i) {
     printf("\n%d|%d:%s  ",i,SA[i],src_begin+SA[i]);
     }
     //*/
}

void TSuffixString::R_create(){
    R.resize(SA.size());
    _Rank_create(ssbegin, (TInt)SA.size(), &SA[0],&R[0]);
}

void TSuffixString::LCP_create_withR(){
    assert(SA.empty()||(!R.empty()));
    LCP.resize(SA.size());
    _LCP_create_withR<TInt,0>(ssbegin,(TInt)SA.size(),&SA[0],&R[0],&LCP[0]);
}

void TSuffixString::LCP_create_withOutR(){
    LCP.resize(SA.size());
    _LCP_create_withOutR<TInt,0>(ssbegin,(TInt)SA.size(),&SA[0],&LCP[0]);
}
void TSuffixString::LCPLite_create_withR(){
    assert(SA.empty()||(!R.empty()));
    LCPLite.resize(SA.size());
    _LCP_create_withR<TUShort,(1<<16)-1>(ssbegin,(TInt)SA.size(),&SA[0],&R[0],&LCPLite[0]);
}

void TSuffixString::LCPLite_create_withOutR(){
    LCPLite.resize(SA.size());
    _LCP_create_withOutR<TUShort,(1<<16)-1>(ssbegin,(TInt)SA.size(),&SA[0],&LCPLite[0]);
}

TInt TSuffixString::lower_bound(const char* str,const char* str_end)const{
    if (SA.empty()) return 0;
    const TSuffixIndex* pos=std::lower_bound<const TSuffixIndex*,StringToken,const TSuffixString_compare&>
            (&SA[0],&SA[0]+SA.size(),StringToken(str,str_end),TSuffixString_compare(ssbegin,ssend));
    return (TInt)(pos-&SA[0]);
}
