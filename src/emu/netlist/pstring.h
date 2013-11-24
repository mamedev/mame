// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstring.h
 */

#ifndef _PSTRING_H_
#define _PSTRING_H_

#include "nl_config.h"

// ----------------------------------------------------------------------------------------
// nstring: immutable strings ...
//
// nstrings are just a pointer to a "pascal-style" string representation.
// It uses reference counts and only uses new memory when a string changes.
// ----------------------------------------------------------------------------------------

struct pstring
{
public:
    // simple construction/destruction
    pstring()
    {
        init();
    }
    ~pstring();

    // construction with copy
    pstring(const char *string) {init(); if (string != NULL) pcopy(string); }
    pstring(const pstring &string) {init(); pcopy(string); }

    // assignment operators
    pstring &operator=(const char *string) { pcopy(string); return *this; }
    pstring &operator=(const pstring &string) { pcopy(string); return *this; }

    // C string conversion operators and helpers
    operator const char *() const { return m_ptr->str(); }
    inline const char *cstr() const { return m_ptr->str(); }

    // concatenation operators
    pstring& operator+=(const pstring &string) { pcat(string.cstr()); return *this; }
    friend pstring operator+(const pstring &lhs, const pstring &rhs) { return pstring(lhs) += rhs; }
    friend pstring operator+(const pstring &lhs, const char *rhs) { return pstring(lhs) += rhs; }
    friend pstring operator+(const char *lhs, const pstring &rhs) { return pstring(lhs) += rhs; }

    // comparison operators
    bool operator==(const char *string) const { return (pcmp(string) == 0); }
    bool operator==(const pstring &string) const { return (pcmp(string.cstr()) == 0); }
    bool operator!=(const char *string) const { return (pcmp(string) != 0); }
    bool operator!=(const pstring &string) const { return (pcmp(string.cstr()) != 0); }
    bool operator<(const char *string) const { return (pcmp(string) < 0); }
    bool operator<(const pstring &string) const { return (pcmp(string.cstr()) < 0); }
    bool operator<=(const char *string) const { return (pcmp(string) <= 0); }
    bool operator<=(const pstring &string) const { return (pcmp(string.cstr()) <= 0); }
    bool operator>(const char *string) const { return (pcmp(string) > 0); }
    bool operator>(const pstring &string) const { return (pcmp(string.cstr()) > 0); }
    bool operator>=(const char *string) const { return (pcmp(string) >= 0); }
    bool operator>=(const pstring &string) const { return (pcmp(string.cstr()) >= 0); }

    //
    inline const int len() const { return m_ptr->len(); }

    inline bool equals(const pstring &string) { return (pcmp(string.cstr(), m_ptr->str()) == 0); }
    int cmp(pstring &string) { return pcmp(string.cstr()); }

    int find(const char *search, int start = 0) const
    {
        int alen = len();
        const char *result = strstr(cstr() + MIN(start, alen), search);
        return (result != NULL) ? (result - cstr()) : -1;
    }

    // various

    bool startsWith(pstring &arg) { return (pcmp(cstr(), arg.cstr(), arg.len()) == 0); }
    bool startsWith(const char *arg) { return (pcmp(cstr(), arg, strlen(arg)) == 0); }

    // these return nstring ...
    pstring cat(const pstring &s) const { return *this + s; }
    pstring cat(const char *s) const { return *this + s; }

    pstring substr(unsigned int start, int count = -1) const ;

    pstring left(unsigned int count) const { return substr(0, count); }
    pstring right(unsigned int count) const  { return substr(len() - count, count); }

    // printf using string as format ...

    pstring vprintf(va_list args) const;

    // static
    static pstring sprintf(const char *format, ...);
    static void resetmem();

protected:

    struct str_t
    {
        int reference_count;
        char *str() { return &m_str[0]; }
        int len() { return m_len; }
    //private:
        int m_len;
        char m_str[];
    };

    str_t *m_ptr;

private:
    void init();

    inline int pcmp(const char *right) const
    {
        return pcmp(m_ptr->str(), right);
    }

    inline int pcmp(const char *left, const char *right, int count = -1) const
    {
        if (count < 0)
            return strcmp(left, right);
        else
            return strncmp(left, right, count);
    }

    void pcopy(const char *from, int size);

    inline void pcopy(const char *from)
    {
        pcopy(from, strlen(from));
    }

    inline void pcopy(const pstring &from)
    {
        sfree(m_ptr);
        m_ptr = from.m_ptr;
        m_ptr->reference_count++;
    }

    void pcat(const char *s);

    static str_t *m_zero;

    static str_t *salloc(int n);
    static void sfree(str_t *s);

    struct memblock
    {
        memblock *next;
        int size;
        int allocated;
        int remaining;
        char *cur;
        char data[];
    };

    static memblock *m_first;
    static char *alloc_str(int n);
    static void dealloc_str(void *ptr);
};


#endif /* _PSTRING_H_ */

