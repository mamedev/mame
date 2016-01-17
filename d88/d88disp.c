/*
	.D88のトラック情報を表示する
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void invalid(char *s)
{
	printf("d88disp: %s\n", s); exit(1);
}


void main(int argc, char **argv)
{
	int i,j,n;
	long of;
	FILE *fp;
	unsigned char hd[16];
	long offs[80];

	if (argc == 1) { puts("usage: d88disp <D88-file>"); return; }

	fp = fopen(argv[1], "rb");
	if (!fp) invalid("File not found");

	fseek(fp, 0x20L, SEEK_SET);
	fread(offs, 4, 80, fp);

	for(i=0;i<80;i++)
	{
		printf("Track %02d\n", i);
		of = offs[i];
		fseek(fp, of, SEEK_SET);
		n = -1;
		for(j=1;;j++)
		{
			fread(hd, 1, 16, fp);
			if (n < 0) n = hd[4];
			printf("No.%2d %02X%02X%02X%02X", j, hd[0], hd[1], hd[2], hd[3]);
			if (hd[8] == 0xb0) puts("  CRC Error"); else putchar('\n');
			if (n == j) break;
			switch(hd[3])
			{
			  case 1: fseek(fp, 256L, SEEK_CUR); break;
			  case 2: fseek(fp, 512L, SEEK_CUR); break;
			  case 3: fseek(fp, 1024L, SEEK_CUR); break;
			  case 4: fseek(fp, 2048L, SEEK_CUR); break;
			  case 5: fseek(fp, 4096L, SEEK_CUR); break;
			  case 6: fseek(fp, 8192L, SEEK_CUR); break;
			  default: puts("Nの値が不正です");
			}
		}
		putchar('\n');
	}

	fclose(fp);
}
