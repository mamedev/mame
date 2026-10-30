/* Miscellaneous tests, e.g. regression tests */

#include "tests.h"

static void issue128(void) /* #128 */
{
    utf8proc_uint8_t input[] = {0x72, 0xcc, 0x87, 0xcc, 0xa3, 0x00}; /* "r\u0307\u0323" */
    utf8proc_uint8_t nfc[] = {0xe1, 0xb9, 0x9b, 0xcc, 0x87, 0x00}; /* "\u1E5B\u0307" */
    utf8proc_uint8_t nfd[] = {0x72, 0xcc, 0xa3, 0xcc, 0x87, 0x00}; /* "r\u0323\u0307" */

    check_compare("NFC", input, nfc, utf8proc_NFC(input), 1);
    check_compare("NFD", input, nfd, utf8proc_NFD(input), 1);
}

static void issue102(void) /* #102 */
{
    utf8proc_uint8_t input[] = {0x58, 0xe2, 0x81, 0xa5, 0x45, 0xcc, 0x80, 0xc2, 0xad, 0xe1, 0xb4, 0xac, 0x00}; /* "X\u2065E\u0300\u00ad\u1d2c" */
    utf8proc_uint8_t stripna[] = {0x78, 0xc3, 0xa8, 0x61, 0x00}; /* "x\u00e8a" */
    utf8proc_uint8_t correct[] = {0x78, 0xe2, 0x81, 0xa5, 0xc3, 0xa8, 0x61, 0x00}; /* "x\u2065\u00e8a" */
    utf8proc_uint8_t *output;

    utf8proc_map(input, 0, &output, UTF8PROC_NULLTERM | UTF8PROC_STABLE |
        UTF8PROC_COMPOSE | UTF8PROC_COMPAT | UTF8PROC_CASEFOLD | UTF8PROC_IGNORE | UTF8PROC_STRIPNA);
    check_compare("NFKC_Casefold+stripna", input, stripna, output, 1);
    check_compare("NFKC_Casefold", input, correct, utf8proc_NFKC_Casefold(input), 1);
}

static void issue317(void) /* #317 */
{
    utf8proc_uint8_t input[] = {0xec, 0xa3, 0xa0, 0xe1, 0x86, 0xa7, 0x00}; /* "\uc8e0\u11a7" */
    utf8proc_uint8_t combined[] = {0xec, 0xa3, 0xa, 0x00}; /* "\uc8e1" */
    utf8proc_int32_t codepoint;

    /* inputs that should *not* be combined* */
    check_compare("NFC", input, input, utf8proc_NFC(input), 1);
    utf8proc_encode_char(0x11c3, input+3);
    check_compare("NFC", input, input, utf8proc_NFC(input), 1);

    /* inputs that *should* be combined (TCOUNT-1 chars starting at TBASE+1) */
    for (codepoint = 0x11a8; codepoint < 0x11c3; ++codepoint) {
        utf8proc_encode_char(codepoint, input+3);
        utf8proc_encode_char(0xc8e0 + (codepoint - 0x11a7), combined);
        check_compare("NFC", input, combined, utf8proc_NFC(input), 1);
    }
}

int main(void)
{
    issue128();
    issue102();
    issue317();
#ifdef UNICODE_VERSION
    printf("Unicode version: Makefile has %s, has API %s\n", UNICODE_VERSION, utf8proc_unicode_version());
    check(!strcmp(UNICODE_VERSION, utf8proc_unicode_version()), "utf8proc_unicode_version mismatch");
#endif
    printf("Misc tests SUCCEEDED.\n");
    return 0;
}
