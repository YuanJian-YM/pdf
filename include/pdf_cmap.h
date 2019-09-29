#ifndef PDF_CMAP_H
#define PDF_CMAP_H
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>
#define WORD_ERROR -1
#define WORD_NAME 0
#define WORD_OTHER 1
#define WORD_STRING 2
#define WORD_HEX_STRING 3
#define WORD_ARRAY 4
#define WORD_DICT 5
#define EMPTY (unsigned int)0x40000000
using namespace std;
#ifdef __cplusplus
extern "C"
{
#endif
    namespace pdf
    {
    class PDFBuffer;
    typedef std::shared_ptr<PDFBuffer> PDFBufferPtr;

    struct cmap_range
    {
        unsigned short low;
        unsigned short high;
        unsigned short out;
    };

    struct cmap_xrange
    {
        unsigned int low;
        unsigned int high;
        unsigned int out;
    };

    struct cmap_mrange
    {
        unsigned int low;
        unsigned int out;
    };

    struct cmap_splay
    {
        unsigned int low;
        unsigned int high;
        unsigned int out;
        unsigned int left;
        unsigned int right;
        unsigned int parent : 31;
        unsigned int many : 1;
    };

    struct cmap_codespace
    {
        int n;
        unsigned int low;
        unsigned int high;
    };

    class PDFCmap
    {
    public:
        PDFCmap();
        virtual ~PDFCmap();

    public:
        std::string cmap_name;
        std::string usecmap_name;
        PDFCmap *usecmap;
        int wmode;
        int codespace_len;
        int rlen, rcap;
        int xlen, xcap;
        int mlen, mcap;
        int dlen, dcap;
        int *dict;
        int tlen, tcap, ttop;
        cmap_codespace codespace[40];
        cmap_range *ranges;
        cmap_xrange *xranges;
        cmap_mrange *mranges;
        cmap_splay *tree;
    };
    typedef std::shared_ptr<PDFCmap> PDFCmapPtr;

    PDFCmapPtr loadCmap(char *cUnicodeData);
    int lookupCmap(PDFCmapPtr cmap, unsigned int cpt, int *out);
    } //namespace pdf
#ifdef __cplusplus
}
#endif
#endif // PDF_CMAP_H