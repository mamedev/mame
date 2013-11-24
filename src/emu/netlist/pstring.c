/*
 * nl_string.c
 *
 */

#include "pstring.h"
#include <cstdio>

//nstring::str_t *nstring::m_zero = NULL;
pstring::str_t *pstring::m_zero = pstring::salloc(0);
pstring::memblock *pstring::m_first = NULL;

#define IMMEDIATE_MODE  (1)
#define DEBUG_MODE      (0)

pstring::~pstring()
{
   sfree(m_ptr);
}

void pstring::init()
{
    if (m_zero == NULL)
    {
        m_zero = (str_t *) alloc_str(sizeof(str_t) + 1);
        m_zero->reference_count = 1;
        m_zero->m_len = 0;
        m_zero->m_str[0] = 0;
    }
    m_ptr = m_zero;
    m_ptr->reference_count++;
}

void pstring::pcat(const char *s)
{
    int slen = strlen(s);
    str_t *n = salloc(m_ptr->len() + slen);
    if (m_ptr->len() > 0)
        memcpy(n->str(), m_ptr->str(), m_ptr->len());
    if (slen > 0)
        memcpy(n->str() + m_ptr->len(), s, slen);
    *(n->str() + n->len()) = 0;
    sfree(m_ptr);
    m_ptr = n;
}

void pstring::pcopy(const char *from, int size)
{
    str_t *n = salloc(size);
    if (size > 0)
        memcpy(n->str(), from, size);
    *(n->str() + size) = 0;
    sfree(m_ptr);
    m_ptr = n;
}

pstring pstring::substr(unsigned int start, int count) const
{
    int alen = len();
    if (start >= alen)
        return pstring();
    if (count <0 || start + count > alen)
        count = alen - start;
    pstring ret;
    ret.pcopy(cstr() + start, count);
    return ret;
}

pstring pstring::vprintf(va_list args) const
{
    // sprintf into the temporary buffer
    char tempbuf[4096];
    vsprintf(tempbuf, cstr(), args);

    return pstring(tempbuf);
}

// ----------------------------------------------------------------------------------------
// static stuff ...
// ----------------------------------------------------------------------------------------

void pstring::sfree(str_t *s)
{
    s->reference_count--;
    if (s->reference_count == 0)
        dealloc_str(s);
}

pstring::str_t *pstring::salloc(int n)
{
    str_t *ret = (str_t *) alloc_str(sizeof(str_t) + n + 1);
    ret->reference_count = 1;
    ret->m_len = n;
    ret->m_str[0] = 0;
    return ret;
    //std::printf("old string %d <%s> %p %p\n", n, old, old, m_ptr);
}

pstring pstring::sprintf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    pstring ret = pstring(format).vprintf(ap);
    va_end(ap);
    return ret;
}

char *pstring::alloc_str(int n)
{
    if (IMMEDIATE_MODE)
        return (char *) malloc(n);
    else
    {
        int min_alloc = MAX((DEBUG_MODE) ? 0 : 8192, n+sizeof(memblock));
        char *ret = NULL;

        //std::printf("m_first %p\n", m_first);
        for (memblock *p = m_first; p != NULL && ret == NULL; p = p->next)
        {
            if (p->remaining > n)
            {
                ret = p->cur;
                p->cur += n;
                p->allocated += 1;
                p->remaining -= n;
            }
        }

        if (ret == NULL)
        {
            // need to allocate a new block
            memblock *p = (memblock *) malloc(min_alloc); //new char[min_alloc];
            p->allocated = 0;
            p->cur = &p->data[0];
            p->size = p->remaining = min_alloc - sizeof(memblock);
            p->next = m_first;
            //std::printf("allocated block size %d\n", p->size);

            ret = p->cur;
            p->cur += n;
            p->allocated += 1;
            p->remaining -= n;

            m_first = p;
        }

        return ret;
    }
}

void pstring::dealloc_str(void *ptr)
{
    if (IMMEDIATE_MODE)
        free(ptr);
    else
    {
        for (memblock *p = m_first; p != NULL; p = p->next)
        {
            if (ptr >= &p->data[0] && ptr < &p->data[p->size])
            {
                p->allocated -= 1;
                if (p->allocated < 0)
                    fatalerror("nstring: memory corruption\n");
                if (p->allocated == 0)
                {
                    //std::printf("Block entirely freed\n");
                    p->remaining = p->size;
                    p->cur = &p->data[0];
                }
                // shutting down ?
                if (m_zero == NULL)
                    resetmem(); // try to free blocks
                return;
            }
        }
        fatalerror("nstring: string <%p> not found\n", ptr);
    }
}

void pstring::resetmem()
{
    if (!IMMEDIATE_MODE)
    {
        memblock **p = &m_first;
        int totalblocks = 0;
        int freedblocks = 0;

        // Release the 0 string
        if (m_zero != NULL) sfree(m_zero);
        m_zero = NULL;

        while (*p != NULL)
        {
            totalblocks++;
            memblock **next = &((*p)->next);
            if ((*p)->allocated == 0)
            {
                //std::printf("freeing block %p\n", *p);
                memblock *freeme = *p;
                *p = *next;
                free(freeme); //delete[] *p;
                freedblocks++;
            }
            else
            {
                if (DEBUG_MODE)
                    std::printf("Allocated: <%s>\n", ((str_t *)(&(*p)->data[0]))->str());

                p = next;
            }
        }
        if (DEBUG_MODE)
            std::printf("Freed %d out of total %d blocks\n", freedblocks, totalblocks);
    }
}
