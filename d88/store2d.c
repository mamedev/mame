/*
	.2Dファイルに登録する
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>

#define uint unsigned
#define uchar unsigned char

uchar attr;

void usage()
{
	puts(
"usage: store2d <.2D-file> <Store-file> <\x22Internal-name\x22> <Start-Addr>\n"
"       store2d <.2D-file> <Store-file> <\x22Internal-name\x22> <-b>\n"
"       store2d <.2D-file> <Store-file> <\x22Internal-name\x22> <-a>\n"
"          <Start-Addr> を指定すると機械語ファイルとみなします\n"
"          <-a> を指定するとASCIIファイルとみなします\n"
"          <-b> を指定するとBASICファイルとみなします");
	exit(1);
}


void invalid(char *s)
{
	printf("store2d: %s\n", s); exit(1);
}


/*
	cl:開始クラスタ
	mode:$fffe=BASIC / else=Binary
*/
void store_dirent(FILE *fpw, char *intfile, int cl)
{
	int i;
	uchar *p;
	uchar dirbuf[0xc00];

	fseek(fpw, 0x25000L, SEEK_SET);
	fread(dirbuf, 1, 0xc00, fpw);
	p = dirbuf;
	for(i=0;i<0xc0;i++)
	{
		if (p[0] == 0xff) break;
		p += 0x10;
	}
	if (i == 0xc0) invalid("DirEntに空きがありません");

	memset(p, ' ', 9);
	for(i=0;intfile[i];i++) p[i] = intfile[i];
	p[9] = attr;
	p[10] = cl;

	fseek(fpw, 0x25000L, SEEK_SET);
	fwrite(dirbuf, 1, 0xc00, fpw);
}


/*
	２Ｄファイルに登録する
*/
void store2d(FILE *fpr, FILE *fpw, char *intfile, uint sa)
{
	int i, ct, cl, ncl, fre;
	int adrF;
	uint sz, remain;
	char *mp;
	uchar fatbuf[256];

	/* 読み込みバッファ確保 */
	mp = malloc(0x800);
	if (!mp) invalid("メモリ確保できません");

	/* ＦＡＴ読込＆空きクラスタ数を数える */
	fseek(fpw, 0x25d00L, SEEK_SET);
	fread(fatbuf, 1, 256, fpw);
	fre = 0; cl = 0;
	for(i=0;i<0xa0;i++)
		if (fatbuf[i] == 0xff)
		{
			fre++;
			if (!cl) cl = i;
		}

	/* 空き容量が足りているか？ */
	sz = (uint)filelength(fileno(fpr));
	if (attr == 0x01) sz += 4;
	remain = sz;
	sz = (sz + 0x7ff) / 0x800;
	if (fre < sz) invalid("空き容量が足りません");

	/* エントリ部を登録 */
	store_dirent(fpw, intfile, cl);

	adrF = 0;
	for(;;)
	{
		memset(mp, 0, 0x800);
		if (attr == 0x01 && !adrF)
		{
			adrF = 1;
			mp[0] = sa % 256; mp[1] = sa / 256;
			sa += remain - 4;
			mp[2] = sa % 256; mp[3] = sa / 256;
			ct = fread(mp+4, 1, 0x800-4, fpr);
			ct += 4;
		}
		else
			ct = fread(mp, 1, 0x800, fpr);

		if (!ct) break;
		fseek(fpw, (long)cl * 0x800L, SEEK_SET);
		fwrite(mp, 1, ct, fpw);

		remain -= ct;
		if (remain == 0)
		{	/* このクラスタで終わりの場合 */
			fatbuf[cl] = 0xc0 + ((ct + 0xff) / 0x100);
			break;
		}
		else
		{	/* 次のクラスタを探す */
			fatbuf[cl] = 0;
			for(i=0;i<0xa0;i++)
				if (fatbuf[i] == 0xff)
				{
					fatbuf[cl] = i; cl = i;
					break;
				}
		}
	}

	/* ＦＡＴ書き込み */
	memset(fatbuf+0xa0, 0, 0x60);
	fseek(fpw, 0x25d00L, SEEK_SET);
	fwrite(fatbuf, 1, 0x100, fpw);
	fwrite(fatbuf, 1, 0x100, fpw);
	fwrite(fatbuf, 1, 0x100, fpw);

	free(mp);
}


/* 文字列を16進数に変換 */
uint get_addr(char *av)
{
	int i,c;
	uint adr;

	if (!isxdigit(av[0])) return 0xffff;

	if (av[0] <= '9')
		adr = av[0] - '0';
	else
		adr = toupper(av[0]) - 'A' + 10;

	for(i=1;av[i];i++)
	{
		adr <<= 4;
		if (!isxdigit(av[i])) return 0xffff;
		if (av[i] <= '9')
			adr += av[i] - '0';
		else
			adr += toupper(av[i]) - 'A' + 10;
	}
	return adr;
}


int main(int argc, char **argv)
{
	int i;
	uint adr = 0;
	FILE *fpr,*fpw;

	if (argc != 5) usage();

	if (!strcmp("-b", argv[4]))
		attr = 0x80;	/* BASICファイルのとき */
	else if (!strcmp("-a", argv[4]))
	{
		attr = 0x00;	/* ASCIIファイルのとき */
	}
	else
	{
		attr = 0x01;	/* 機械語のとき */
		adr = get_addr(argv[4]);
		if (adr == 0xffff) invalid("アドレスの指定が不正です");
	}
	fpr = fopen(argv[2], "rb");
	if (!fpr) invalid("Store-Fileがオープンできません");
	fpw = fopen(argv[1], "r+b");
	if (!fpw) invalid("2Dファイルがオープンできません");

	/* 2Dファイルに登録 */
	store2d(fpr, fpw, argv[3], adr);

	fclose(fpr); fclose(fpw);
	return 0;
}
