#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#ifndef _WIN32
#include <iconv.h>
#else
#include <wchar.h>
#endif

extern uint8_t hg_freqs[11231];
extern uint8_t cjk_freqs[20991];

size_t u16le_extract(const char* src, size_t srcsz, uint16_t* dest,
                     size_t destsz, size_t cutoff, const char** srcend);
// size_t eng_extract(const char* src, size_t srcsz, char* dest,
//                    size_t destsz, size_t cutoff, const char** srcend);

struct doctotext {
    FILE* _doc_handle;
    // Internal buffer, 262144B heap-alloced
    char* _readbuf;

    // Internal state variables
    size_t _readbuf_at, _readbuf_tot;

#ifndef _WIN32
    // Encoding converter
    iconv_t convd;
#endif
};

// All doctotext objects must be returned by doctotext_alloc,
// then call doctotext_setfile
struct doctotext doctotext_alloc(void);
// Set success 0, otherwise 1
int doctotext_setfile(struct doctotext* s, const char* path);
void doctotext_free(struct doctotext* s);
uint16_t* doctotext_next_u16le(struct doctotext* s, size_t* outsz);
#ifndef _WIN32
char* doctotext_next_u8(struct doctotext* s, size_t* outsz);
#else
void doctotext_setfilew(struct doctotext* s, const wchar_t* path);
#endif
