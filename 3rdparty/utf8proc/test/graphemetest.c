#include "tests.h"

int main(int argc, char **argv)
{
    char *buf = NULL;
    size_t bufsize = 0;
    FILE *f = argc > 1 ? fopen(argv[1], "r") : NULL;
    utf8proc_uint8_t src[1024];
    int len;
    
    check(f != NULL, "error opening GraphemeBreakTest.txt");
    while (getline(&buf, &bufsize, f) > 0) {
        size_t bi = 0, si = 0;
        lineno += 1;
        
        if (lineno % 100 == 0)
            printf("checking line %zd...\n", lineno);
        
        if (buf[0] == '#') continue;
        
        while (buf[bi]) {
            bi = skipspaces(buf, bi);
            if (buf[bi] == '/') { /* grapheme break */
                src[si++] = '/';
                bi++;
            }
            else if (buf[bi] == '+') { /* no break */
                bi++;
            }
            else if (buf[bi] == '#') { /* start of comments */
                break;
            }
	    else { /* hex-encoded codepoint */
                len = encode((char*) (src + si), buf + bi) - 1;
                while (src[si]) ++si; /* advance to NUL termination */
                bi += len;
            }
        }
        if (si && src[si-1] == '/')
            --si; /* no break after final grapheme */
        src[si] = 0; /* NUL-terminate */
        
        if (si) {
            utf8proc_uint8_t utf8[1024]; /* copy src without 0xff grapheme separators */
            size_t i = 0, j = 0;
            utf8proc_ssize_t glen;
            utf8proc_uint8_t *g; /* utf8proc_map grapheme results */
            while (i < si) {
                if (src[i] != '/')
                    utf8[j++] = src[i++];
                else
                    i++;
            }
            glen = utf8proc_map(utf8, j, &g, UTF8PROC_CHARBOUND);
            if (glen == UTF8PROC_ERROR_INVALIDUTF8) {
                 /* the test file contains surrogate codepoints, which are only for UTF-16 */
                 printf("line %zd: ignoring invalid UTF-8 codepoints\n", lineno);
            }
            else {
                 check(glen >= 0, "utf8proc_map error = %s",
                       utf8proc_errmsg(glen));
                 for (i = 0; i <= glen; ++i)
                      if (g[i] == 0xff)
                          g[i] = '/'; /* easier-to-read output (/ is not in test strings) */
                 check(!strcmp((char*)g, (char*)src),
                       "grapheme mismatch: \"%s\" instead of \"%s\"", (char*)g, (char*)src);
            }
            free(g);
        }
    }
    fclose(f);
    printf("Passed tests after %zd lines!\n", lineno);
    return 0;
}
