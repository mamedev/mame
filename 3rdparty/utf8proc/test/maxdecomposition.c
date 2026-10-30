#include "tests.h"

/* Check the maximum decomposed size returned by utf8proc_decompose_char with UTF8PROC_DECOMPOSE,
   in order to give a hint in the documentation.  The hint will need to be updated if this changes. */

int main(void)
{
    utf8proc_int32_t dst[128];
    utf8proc_ssize_t maxsize = 0, expected_maxsize = 4;
    int success;

    for (utf8proc_int32_t c = 0; c <= 0x110000; ++c) {
        utf8proc_ssize_t sz = utf8proc_decompose_char(c, dst, 128, UTF8PROC_DECOMPOSE, NULL);
        maxsize = sz > maxsize ? sz : maxsize;
    }

    success = expected_maxsize == maxsize;
    fprintf(success ? stdout : stderr,
            "%s: maximum decomposed size = %d chars\n",
            success ? "SUCCEEDED" : "FAILED", (int) maxsize);
    return !success;
}
