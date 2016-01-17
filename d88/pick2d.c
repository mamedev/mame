#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BYTE unsigned char

BYTE clbuf[0x800];
BYTE dirbuf[0xc00];
BYTE fatbuf[256];
char sfile[0x80][11];
BYTE stcl[0x80];
FILE *fpr,*fpw;

void usage()
{
	puts(
".2D ???? ????? ????\n"
"usage: pick2d <filename>[.2D]");
	exit(1);
}

void invalid(char *s)
{
	fputs("pick2d.c: ",stderr); fputs(s,stderr); putc('\n',stderr);
	exit(1);
}


int GetOffs(int cl)
{
	long offs;

	offs = (cl >> 1) * 0x1000L;
	if (cl & 1) offs += 0x800L;
	fseek(fpr, offs, SEEK_SET);
	printf("cl=%02x offs=%08lx\n", cl, offs);

	return fatbuf[cl];
}


/*
   Image->File Convert
     cl:先頭クラスタ
*/
void Conv(int cl)
{
	int ncl, ct, eof;

	eof = 0;
	ncl = cl;
	while(!eof)
	{
		ncl = GetOffs(ncl);
		if (ncl >= 0xc0)
		{
			ct = (ncl & 0x0f) * 0x100;
			fread(clbuf, 1, ct, fpr);
			fwrite(clbuf, 1, ct, fpw);
			eof = 1;
		}
		else
		{
			fread(clbuf, 1, 0x800, fpr);
			fwrite(clbuf, 1, 0x800, fpw);
		}
		if (kbhit())
			if (getch() == 0x1b) exit(1);
	}
}


void GetFilename()
{
	BYTE *p;
	int i,j,idx;

	p = dirbuf;
	idx = 0;
	for(i=0;i<0xc0;i++,p+=0x10)
	{
		if (*p == 0 || *p == 0xff) continue;
		for(j=0;j<6;j++)
			if (p[j] == 0x20) sfile[idx][j] = '_'; else sfile[idx][j] = p[j];
		sfile[idx][6] = '.';
		sfile[idx][7] = p[6];
		sfile[idx][8] = p[7];
		sfile[idx][9] = p[8];
		sfile[idx][10] = '\0';
		stcl[idx] = p[10];
		printf("File:%s StartAddr:%04x GoAddr:%04x\n", sfile[idx],
			p[11]+p[12]*256, p[13]+p[14]*256);
		idx++;
	}
	sfile[idx][0] = '\0';
	putchar('\n');
}


int main(int argc,char **argv)
{
	int i;
	char *p;
	char rfile[32];

	if (argc == 1) usage();

	/* 2Dファイルをオープンする */
	strcpy(rfile, argv[1]);
	p = strchr(rfile, '.');
	if (!p) strcat(rfile, ".2D");
	fpr = fopen(rfile, "rb");
	if (!fpr) invalid("File not found");

	fseek(fpr, 0x25000L, 0);
	/* DirEntを読む */
	fread(dirbuf, 1, 0xc00, fpr);
	/* ＦＡＴを読む */
	fread(fatbuf, 1, 256, fpr);
	fread(fatbuf, 1, 256, fpr);

	GetFilename();
	for(i=0;i<0xc0;i++)
	{
		if (!sfile[i][0]) break;
		printf("Creating %s\n", sfile[i]);
		fpw = fopen(sfile[i], "wb");
		Conv(stcl[i]);
		fclose(fpw);
	}

	fclose(fpr);

	return 0;
}
