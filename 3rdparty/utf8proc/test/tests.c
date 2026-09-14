/* Common functions for our test programs. */

#include "tests.h"

size_t lineno = 0;

void check(int cond, const char *format, ...)
{
     if (!cond) {
          va_list args;
          if (lineno)
               fprintf(stderr, "FAILED at line %zd: ", lineno);
          else
               fprintf(stderr, "FAILED: ");
          va_start(args, format);
          vfprintf(stderr, format, args);
          va_end(args);
          fprintf(stderr, "\n");
          exit(1);
     }
}

size_t skipspaces(const unsigned char *buf, size_t i)
{
    while (isspace(buf[i])) ++i;
    return i;
}

/* if buf points to a sequence of codepoints encoded as hexadecimal strings,
   separated by whitespace, and terminated by any character not in
   [0-9a-fA-F] or whitespace, then stores the corresponding utf8 string
   in dest, returning the number of bytes read from buf */
size_t encode(unsigned char *dest, size_t *dest_len, const unsigned char *buf)
{
     size_t i = 0, j;
     utf8proc_ssize_t d = 0;
     for (;;) {
          int c;
          i = skipspaces(buf, i);
          for (j=i; buf[j] && strchr("0123456789abcdef", tolower(buf[j])); ++j)
               ; /* find end of hex input */
          if (j == i) { /* no codepoint found */
               dest[d] = 0; /* NUL-terminate destination string */
               *dest_len = (size_t)d;
               return i + 1;
          }
          check(sscanf((char *) (buf + i), "%x", (unsigned int *)&c) == 1, "invalid hex input %s", buf+i);
          i = j; /* skip to char after hex input */
          d += utf8proc_encode_char(c, (utf8proc_uint8_t *) (dest + d));
     }
}

/* simplistic, portable replacement for getline, sufficient for our tests */
size_t simple_getline(unsigned char buf[8192], FILE *f) {
    size_t i = 0;
    while (i < 8191) {
        int c = getc(f);
        if (c == EOF || c == '\n') break;
        buf[i++] = (unsigned char) c;
    }
    buf[i] = 0;
    return i;
}

void print_escaped(FILE* f, const utf8proc_uint8_t *utf8) {
     fprintf(f, "\"");
     while (*utf8) {
          utf8proc_int32_t codepoint;
          utf8 += utf8proc_iterate(utf8, -1, &codepoint);
          if (codepoint < 0x10000)
               fprintf(f, "\\u%04x", codepoint);
          else
               fprintf(f, "\\U%06x", codepoint);
     }
     fprintf(f, "\"");
}

void print_string_and_escaped(FILE* f, const utf8proc_uint8_t *utf8) {
     fprintf(f, "\"%s\" (", (const char *) utf8);
     print_escaped(f, utf8);
     fprintf(f, ")");
}

void check_compare(const char *transformation,
                   const utf8proc_uint8_t *input, const utf8proc_uint8_t *expected,
                   utf8proc_uint8_t *received, int free_received) {
     int passed = !strcmp((const char *) received, (const char *) expected);
     FILE *f = passed ? stdout : stderr;
     fprintf(f, "%s: %s ", passed ? "PASSED" : "FAILED", transformation);
     print_string_and_escaped(f, input);
     fprintf(f, " -> ");
     print_string_and_escaped(f, received);
     if (!passed) {
          fprintf(f, " != expected ");
          print_string_and_escaped(f, expected);
     }
     fprintf(f, "\n");
     if (free_received) free(received);
     if (!passed) exit(1);
}
