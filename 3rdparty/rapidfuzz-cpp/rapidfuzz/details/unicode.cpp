#include "unicode.hpp"

namespace rapidfuzz {
namespace Unicode {

#define ALPHA_MASK 0x01
#define DECIMAL_MASK 0x02
#define DIGIT_MASK 0x04
#define LOWER_MASK 0x08
#define LINEBREAK_MASK 0x10
#define SPACE_MASK 0x20
#define TITLE_MASK 0x40
#define UPPER_MASK 0x80
#define XID_START_MASK 0x100
#define XID_CONTINUE_MASK 0x200
#define PRINTABLE_MASK 0x400
#define NUMERIC_MASK 0x800
#define CASE_IGNORABLE_MASK 0x1000
#define CASED_MASK 0x2000
#define EXTENDED_CASE_MASK 0x4000

constexpr static bool is_alnum(const unsigned short flags) {
    return ((flags & ALPHA_MASK)
      || (flags & DECIMAL_MASK)
      || (flags & DIGIT_MASK)
      || (flags & NUMERIC_MASK));
}

typedef struct {
    /*
       These are either deltas to the character or offsets in
       _PyUnicode_ExtendedCase.
    */
    const int upper;
    const int lower;
    const int title;
    /* Note if more flag space is needed, decimal and digit could be unified. */
    const unsigned char decimal;
    const unsigned char digit;
    const unsigned short flags;
} _PyUnicode_TypeRecord;

#include "unicodetype_db.h"

static inline const _PyUnicode_TypeRecord * gettyperecord(uint32_t code)
{
    unsigned int index;
    if (code >= 0x110000)
        index = 0;
    else
    {
        index = index1[(code>>SHIFT)];
        index = index2[(index<<SHIFT)+(code&((1<<SHIFT)-1))];
    }

    return &_PyUnicode_TypeRecords[index];
}

uint32_t UnicodeDefaultProcess(uint32_t ch)
{
    /* todo capital sigma not handled
     * see Python implementation
     */
    const _PyUnicode_TypeRecord *ctype = gettyperecord(ch);

    /* non alphanumeric characyers are replaces with whitespaces */
    if (!is_alnum(ctype->flags)) {
        return ' ';
    }

    if (ctype->flags & EXTENDED_CASE_MASK) {
        int index = ctype->lower & 0xFFFF;
        /*int n = ctype->lower >> 24;
        int i;
        for (i = 0; i < n; i++)
            res[i] = _PyUnicode_ExtendedCase[index + i];*/
        /* for now ignore extended cases. The only exisiting
         * on is U+0130 anyways */
        return _PyUnicode_ExtendedCase[index];
    }
    return ch + static_cast<uint32_t>(ctype->lower);
}

} // namespace Unicode
} // namespace rapidfuzz