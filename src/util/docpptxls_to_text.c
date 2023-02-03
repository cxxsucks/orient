#include <orient/util/docpptxls_to_text.h>
#include <stdlib.h>
#include <memory.h>

int doctotext_setfile(struct doctotext* s, const char* path) {
    s->_readbuf_at = 101372;
    s->_readbuf_tot = 101372;
    s->_doc_handle = fopen(path, "rb");
    if (s->_doc_handle == NULL)
        return 1;
    return 0;
}

struct doctotext doctotext_alloc(void) {
    char* buf = malloc(131072 + 32768 + 65536);
    if (buf == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    struct doctotext res = {
        ._doc_handle = NULL,
        ._readbuf = buf,
        ._readbuf_at = 101372,
        ._readbuf_tot = 101372
    };

#ifndef _WIN32
    res.convd = iconv_open("UTF-8//IGNORE", "UTF-16LE");
    if (res.convd == (iconv_t)-1) {
        perror("iconv_open");
        exit(EXIT_FAILURE);
    }
    return res;
#else
    return;
}

int doctotext_setfilew(struct doctotext* s, const wchar_t* path) {
    s->_doc_handle = _wfopen(path, "rb");
    if (s->_doc_handle == NULL)
        return 1;
    return 0;
#endif
}

void doctotext_free(struct doctotext* s) {
    free(s->_readbuf);
    fclose(s->_doc_handle);
#ifndef _WIN32
    iconv_close(s->convd);
#endif
}

uint16_t* doctotext_next_u16le(struct doctotext* s, size_t* outsz) {
tryagain: ;
    size_t prev_left = s->_readbuf_tot - s->_readbuf_at;
    // Copy unscanned data in previous loop run to start of `readbuf`
    memcpy(s->_readbuf, s->_readbuf + s->_readbuf_at, prev_left);
    // Fill the remaining of buffer
    s->_readbuf_tot = fread(s->_readbuf + prev_left, 1, 131072 - prev_left,
                            s->_doc_handle) + prev_left;
    s->_readbuf_at = 0;
    // The final 32 bytes of document contains no text
    // Skip them for convenience, and text extraction is done!
    if (s->_readbuf_at + 32 > s->_readbuf_tot)
        return NULL;

    uint16_t* u16buf = (uint16_t*)(s->_readbuf + 131072);
    const char* srcend;
    // *2 converts to byte
    size_t u16sz = 2 * u16le_extract(s->_readbuf + s->_readbuf_at, 
        s->_readbuf_tot - s->_readbuf_at, u16buf, 16384, 6, &srcend);
    s->_readbuf_at = srcend - s->_readbuf;

    // No text in _readbuf. Try again with next 131072 bytes.
    if (u16sz == 0)
        goto tryagain; // 1 less indent than while true from beginning
    if (outsz != NULL)
        *outsz = u16sz;
    return u16buf;
}

#ifndef _WIN32
char* doctotext_next_u8(struct doctotext* s, size_t* outsz) {
    size_t u16sz;
tryagain:
    if (doctotext_next_u16le(s, &u16sz) == NULL)
        return NULL;

    char* u16buf = s->_readbuf + 131072;
    char* u8buf = u16buf + 32768;

    // Found text! Duplicate pointer for iconv(3)
    char* u8buf_icv = u8buf;
    size_t u8sz = 65536; // iconv(3) accepts bytes left in buffer
    if ((size_t)-1 == iconv(s->convd, &u16buf, &u16sz, &u8buf_icv, &u8sz)) {
        perror("iconv");
        goto tryagain;
    }
    u8sz = 65536 - u8sz; // Byte left to byte used
    if (outsz != NULL)
        *outsz = u8sz;
    return u8buf;
}
#endif

size_t u16le_extract(const char* src, size_t srcsz, uint16_t* dest,
                     size_t destsz, size_t cutoff, const char** srcend)
{
    srcsz = srcsz > 3 ? srcsz - 3 : 0; // Never go out of bound
    destsz = destsz > 2 ? destsz - 2 : 0;
    size_t combolen = 0, curat = 0, next_scan = 1;
    long validity = 0, ingroup = 7;
    uint_fast16_t last_judge = 0;
    while (curat < srcsz && combolen < destsz) {
        uint_fast16_t judging = *(uint16_t*)(src + curat);
        if (judging == last_judge)
            validity -= 100;
        last_judge = judging;

        if ((judging >= 0x20 && judging <= 0xff) || // ASCII
            (judging >= 0x2000 && judging <= 0x206f) ||
            (judging >= 0xff00 && judging <= 0xffee)) // Fullwidth Puncts
        {   // These chars are common across all languages so no ingroup reset.
            ++combolen;
            validity += 220;
            curat += 2;
            continue;
        } 

        if (judging >= 0xd83c && judging <= 0xd83f) {
            // UTF-16 surrogate. Only accepts emojis here, whose
            // next byte must within dc00~dfff
            judging = *(uint16_t*)(src + curat + 2);
            if (judging >= 0xdc00 && judging <= 0xdfff) {
                combolen += 2;
                validity += 512;
                curat += 4;
                continue;
            }
        }

        // Group 1: Latin 1 Supplement
        else if ((ingroup & 1) != 0 && judging >= 0xa0 && judging <= 0xfe) {
            ++combolen;
            validity += 215;
            curat += 2;
            ingroup &= 1; 
            continue;
        }

        // Group 2: CJK Family
        // First is CJK Unified Ideographs including Trad/Simp Chinese Chars
        else if ((ingroup & 2) != 0 && judging >= 0x4e00 && judging <= 0x9fff) {
            // Limit the text to CJK
            ++combolen;
            // The more common the char is used, the higher "score"
            validity += cjk_freqs[judging - 0x4e00];
            curat += 2;
            ingroup &= 2; 
            continue;
        }
        // Hiragana and Katakana are Japanese characters used along with some CJK
        else if ((ingroup & 2) != 0 && judging >= 0x3040 && judging <= 0x30ff) {
            // Place them in a same group as CJK
            ++combolen;
            validity += 220;
            curat += 2;
            ingroup &= 2; 
            continue;
        }

        // Group 3: Hangul Syllables
        else if ((ingroup & 4) != 0 && judging >= 0xac00 && judging <= 0xd7df) {
            // Limit the text to Hangul
            ++combolen;
            validity += hg_freqs[judging - 0xac00];
            curat += 2;
            ingroup &= 4; 
            continue;
        }

        // Only when the combo is longer than cutoff AND is mostly made up of
        // common CJK chars if any can the text be seen as valid
        if (combolen >= cutoff && validity >= (long)combolen * 200)
            break;
        // "Invalid" text. Cleanup.
        combolen = 0;
        size_t tmp = next_scan;
        next_scan = curat + 2;
        curat = tmp;
        ingroup = 7;
        validity = 0;
    }

    if (srcend != NULL)
        *srcend = src + curat;
    // Write text found. Never overflow due to first 2 lines "shrinking" buffer
    if (combolen >= cutoff && validity >= (long)combolen * 200) {
        memmove(dest, src + curat - combolen * 2, combolen * 2);
        return combolen;
    }
    else return 0;
}
