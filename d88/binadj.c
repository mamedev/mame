/*
	アドレス情報付きのバイナリファイルをプレーンなバイナリに変換する。
	アドレス情報は 0000.log にセーブされる
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

void invalid(char *s)
{
	fprintf(stderr, "binadj: %s\n", s); exit(1);
}


void binadj(char *fname)
{
	FILE *fp;
	unsigned c1,c2,fs;
	unsigned char *mp, *p;

	fp = fopen(fname, "rb");
	if (!fp) invalid("cannot open file");

	/* ファイルサイズ分mallocしてそこに読む */
	fs = (unsigned)filelength(fileno(fp));
	mp = malloc(fs);
	if (!mp) invalid("cannot malloc");
	fread(mp, 1, fs, fp);
	fclose(fp);

	/* ログファイル書き込み */
	fp = fopen("0000.log", "at");
	if (!fp) invalid("cannot open logfile");
	c1 = mp[0] + mp[1]*256;
	c2 = mp[2] + mp[3]*256;
	fprintf(fp, "%04x - %04x :%s\n", c1, c2 - 1, fname);
	printf(     "%04x - %04x :%s\n", c1, c2 - 1, fname);
	fclose(fp);

	/* 4byteずらして上書き */
	fp = fopen(fname, "wb");
	if (!fp) invalid("cannot create file");
	fwrite(mp+4, 1, c2 - c1, fp);
	fclose(fp);

	free(mp);
}


int main(int argc, char **argv)
{
	int i;

	if (argc == 1) { puts("usage: binadj <filename> [files...]"); return 0; }

	for(i=1;argv[i];i++) binadj(argv[i]);

	return 0;
}
