/*
install these packages via apt-get

libbeecrypt7
libbeecrypt-dev
libblitz0-dev
libblitz0ldbl
libblitz-doc
libbotan-1.10-0
libbotan1.10-dev
libdb5.1++:amd64
libdb5.1-dev
libdb5.1++-dev
libhangul1:amd64
libhangul-data
libhangul-dev
libjs-jquery
libmhash-dev
libmozjs185-1.0
libmozjs185-dev
libnspr4:amd64
libnspr4-dev
libsnappy1
libsnappy-dev
libv8-3.8.9.20
libv8-dbg
libv8-dev

install this https://github.com/Hello71/zopfli
git clone https://github.com/Hello71/zopfli.git
cd zopfli
make all
sudo make install

you might need to copy libzopfli.so.1 to /usr/lib/


compile this program using 

g++ original.cpp -lmozjs185 -lbotan-1.10 -lzopfli -lsnappy -lbeecrypt -lhangul -lmhash  -I/usr/include/botan-1.10/ -I/usr/include/nspr -I/usr/local/lib/ -o original

*/

#include "js/jsapi.h"
#include "js/jscntxt.h"
#include "js/jspubtd.h"
#include "js/js-config.h"
#include <stdio.h>
#include <v8.h>
#include <random/uniform.h>
#include <random/normal.h>
#include <random/exponential.h>
#include <random/discrete-uniform.h>
#include <random/beta.h>
#include <random/gamma.h>
#include <random/chisquare.h>
#include <random/F.h>
#include <beecrypt/mtprng.h>
#include <hangul-1.0/hangul.h>
#include <zopfli/zopfli.h>
#include <mhash.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <snappy.h>
#include <nspr/pratom.h>
#include <botan/botan.h>
// no include for you

unsigned char key[] = "aaaa";
int keylen = 4;

char together[10000] = {0};
int pos = 0;

int checkhash(unsigned char *data, size_t datalen, char *checkhash) {
    char keyword[100];

    memcpy(together + pos, data, datalen);
    pos += datalen;

    mhash_keygen((keygenid)0, (hashid)10, 10, keyword, 100, (void *)"aaaa", 3,  key, keylen);

    MHASH hash = mhash_hmac_init((hashid)10, keyword, 100, mhash_get_hash_pblock((hashid)10));

    mhash(hash, data, datalen);
    void *mac = mhash_hmac_end(hash);

    char hash_printed[1000] = "0x";
    for (int j = 0; j < mhash_get_block_size((hashid)10); j++) {
        unsigned char *m = (unsigned char *)mac;
        char r[3];
        snprintf(r, 3, "%.2x", m[j]);
        strcat(hash_printed, r);
    }

    printf("%s\n", hash_printed);

    return strcmp(hash_printed, checkhash) == 0;
}

void reportError(JSContext *cx, const char *message, JSErrorReport *report) {
     fprintf(stderr, "%s:%u:%s\n",
             report->filename ? report->filename : "[no filename]",
             (unsigned int) report->lineno,
             message);
}

int main() {
    std::string a = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbccccccccccccaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    std::string output;

    ZopfliOptions options;
    ZopfliInitOptions(&options);

    snappy::Compress(a.data(), a.size(), &output);

    if(!checkhash((unsigned char *)output.data(), output.size(), (char *)"0x5a8f06cf6817a74bc75c3c8290196928acb04c189ecdd192a93eb3c3")) {
        std::cout<<"Something wrong\n";
    }

    unsigned char b[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbccccccccccccaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    unsigned char *out = NULL;
    size_t outsize = 0;

    ZopfliCompress(&options, ZOPFLI_FORMAT_GZIP, b, a.size(), &out, &outsize);

    if(!checkhash(out, outsize, (char *)"0x0f66ce213d2d27067ff9ffa8bbde2dd06d7f3e687549fad846169e16")) {
        std::cout<<"Something wrong\n";
    }

    HangulInputContext* ic;
    const char* p = "dekde";
    const ucschar* commit;

    ic = hangul_ic_new("2y");

    while (*p != '\0') {
        hangul_ic_process(ic, *p);
        p++;
    }

    commit = hangul_ic_get_commit_string(ic);

    int len = wcslen((const wchar_t*)commit);

    if(!checkhash((unsigned char*)commit, len * sizeof(const wchar_t), (char *)"0xc9bf9374fbc9f4989afd0af7ac9824a4dcc768b33bfa3bb38e42617b")) {
        std::cout<<"Something wrong\n";
    }
    hangul_ic_delete(ic);

    static JSClass global_class = { "global",
                                    JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS,
                                    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
                                    JS_StrictPropertyStub,
                                    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,
                                    NULL, JSCLASS_NO_OPTIONAL_MEMBERS
    };
    JSRuntime *rt = JS_NewRuntime(8 * 1024 * 1024);
    JSContext *cx = JS_NewContext(rt, 8192);
    JS_SetOptions(cx, JSOPTION_VAROBJFIX);
    JS_SetErrorReporter(cx, reportError);

    
        JSObject *global = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);

        JS_SetGlobalObject(cx, global);

        if (!JS_InitStandardClasses(cx, global)) {
            return 1;
        }
        char source[] = "Math.random()";

        jsval rval;

        cx->rngSeed = 31337;

        JS_EvaluateScript(cx, global, source, strlen(source),
                       "none", 10, &rval);

        double nums[2];
        nums[0] = JSVAL_TO_DOUBLE(rval);
        JS_EvaluateScript(cx, global, source, strlen(source),
                       "none", 10, &rval);
        nums[1] = JSVAL_TO_DOUBLE(rval);

        if(!checkhash((unsigned char*)nums, 2 * sizeof(double), (char *)"0x61b35e8d466f24ee1ea502350ec6f5d1134fe6ec543c17845fa62f8a")) {
            std::cout<<"Something wrong\n";
        }
    
    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);

    JS_ShutDown();


    mtprngParam mt;

    byte bay[] = "qqqqqqqqqqqqqqqq";
    byte result[100];

    for(int i=0; i< 100; i++) {
        result[i] = 0;
    }

    mtprngSetup(&mt);
    mtprngSeed(&mt, bay, 16);

    mtprngNext(&mt, result, 100);
    if(!checkhash((unsigned char*)&result, 100, (char *)"0x7754dfd27fe6fa00551861ff41e4f48315bd89bef6da652f182ce2d6")) {
        std::cout<<"Something wrong\n";
    }



    ranlib::ChiSquare<double> gen(4);

    gen.seed(31337);
    double f[16];

    for(int i = 0; i<16; i++) {
        f[i] = gen.random();
    }

    if(!checkhash((unsigned char*)&f, 16 * sizeof(double), (char *)"0xd19d0c167fe93b11004c0167c226d2e92c17dfa36ffb243f39824098")) {
        std::cout<<"Something wrong\n";
    }


    Botan::byte pass[] = "aaaabbbb";

    Botan::PBKDF* pbkdf = Botan::get_pbkdf("PBKDF2(SHA-256)");
    Botan::OctetString aes_key = pbkdf->derive_key(32, "pass1337", pass, 8, 31337);


    std::string aa = aes_key.as_string();

    if(!checkhash((unsigned char*)aa.c_str(), aa.size(), (char *)"0x0c33d122ed50848a676539ae48eb84db0dcbf69e9ee857094755f2d7")) {
        std::cout<<"Something wrong\n";
    }

    std::cout<<"Answer is RUCTF_";
    checkhash((unsigned char*)together, pos, (char *)"");

    return 0;
}
