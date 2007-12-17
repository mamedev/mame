#include <stdio.h>

#define CF	0x100
#define HF	0x200
#define DF	0x400

int dab[0x800];

int main(int ac, char **av)
{
	int i, result;

	for (i = 0; i < DF; i++) {
		if (i & CF) {
			if (i & 0x0f < 0x0a)
				dab[i] = CF | ((i + 0x60) & 0xff);
			else
				dab[i] = CF | ((i + 0x66) & 0xff);
		} else {
			if (i & HF) {
				if (i & 0xf0 < 0xa0)
					dab[i] = ((i + 0x06) & 0xff);
				else
					dab[i] = CF | ((i + 0x66) & 0xff);
			} else {
				if ((i & 0xf0) < 0xa0 && (i & 0x0f) < 0x0a)
					dab[i] = i & 0xff;
				else if ((i & 0xf0) < 0x90 && (i & 0x0f) >= 0x0a)
					dab[i] = ((i + 0x06) & 0xff);
				else if ((i & 0xf0) >= 0xa0 && (i & 0x0f) < 0x0a)
					dab[i] = CF | ((i + 0x60) & 0xff);
				else if ((i & 0xf0) >= 0x90 && (i & 0x0f) >= 0x0a)
					dab[i] = CF | ((i + 0x66) & 0xff);
				else {
					fprintf(stderr, "unhandled $%04x\n", i);
					return 1;
				}

            }
        }

        if (i & CF) {
			if (i & HF) {
				dab[DF+i] = CF | ((i + 0x9a) & 0xff);
			} else {
				dab[DF+i] = CF | ((i + 0xa0) & 0xff);
            }
		} else {
			if (i & HF) {
				dab[DF+i] = CF | ((i + 0xfa) & 0xff);
            } else {
				dab[DF+i] = (i & 0xff);
            }
        }
    }

	printf("/************************************************ \n");
    printf(" * Result table for Z8000 DAB instruction         \n");
    printf(" *                                                \n");
    printf(" * bits    description                            \n");
    printf(" * ---------------------------------------------- \n");
    printf(" * 0..7    destination value                      \n");
    printf(" * 8       carry flag before                      \n");
    printf(" * 9       half carry flag before                 \n");
    printf(" * 10      D flag (0 add/adc, 1 sub/sbc)          \n");
    printf(" *                                                \n");
    printf(" * result  description                            \n");
    printf(" * ---------------------------------------------- \n");
    printf(" * 0..7    result value                           \n");
    printf(" * 8       carry flag after                       \n");
    printf(" ************************************************/\n");
    printf("static UINT16 Z8000_dab[0x800] = {\n");
    for (i = 0; i < 0x800; i++) {
		if ((i & 0x3ff) == 0) {
			if (i & 0x400)
				printf("\t/* sub/sbc results */\n");
			else
				printf("\t/* add/adc results */\n");
		}
        if ((i & 7) == 0) printf("\t");
		printf("0x%03x,",dab[i]);
		if ((i & 7) == 7) printf("\n");
	}
	printf("};\n");

    return 0;
}
