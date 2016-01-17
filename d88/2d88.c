/*
	.D88形式と.2D形式を相互変換する
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

/* ディスクごとのヘッダ */
struct diskhd {
	char title[16];        /* 余白は$00で埋める */
	char dmy[10];          /* $00 */
	long next_disk;        /* 次のdiskにSEEKしてEOFなら終わり */
	long next_track[80];   /* unformatは0Lが入る */
};

/* セクタごとのヘッダ */
struct sechd {
	char C;
	char H;
	char R;
	char N;
	char secn;      /* $10 */
	char dmy[10];   /* $00 */
	char mark;      /* $01 */
};


int modeF;          /* 動作モード(1=.2D->.D88 / 2=.D88->.2D) */
char d88title[16];	/* .D88ファイルのタイトル(最大１６文字) */


void usage()
{
	puts(
"usage: 2d88 <-2d/-d88> <image-file> [title]\n"
"\t-2d : .D88形式から.2D形式にコンバートする\n"
"\t-d88: .2D形式から.D88形式にコンバートする\n"
"\ttitle: .D88ファイルにタイトルをつける");
	exit(1);
}

void invalid(char *s)
{
	fputs("d88ut: ", stdout); puts(s); exit(1);
}


/*
	.2D -> .D88 変換
*/
void conv_D88(FILE *fpr, char *fname, char *mp)
{
	int i;
	long tridx;
	FILE *fpw;
	struct diskhd hd;
	struct sechd sd;
	char wfile[64];

	sprintf(wfile, "%s.d88", fname);
	fpw = fopen(wfile, "wb");
	if (!fpw) invalid("Write-File cannot create.");

	/* ディスクヘッダの設定 */
	memset(&hd, 0, sizeof(struct diskhd));
	memcpy(hd.title, d88title, 16);
	//hd.attr = 0x5;
	hd.next_disk = 0x552b0L;
	tridx = 0x2b0L;
	for(i=0;i<80;i++)
	{
		hd.next_track[i] = tridx;
		tridx += 0x1100L;
	}
	/* ディスクヘッダ書き込み */
	fwrite(&hd, 1, sizeof(struct diskhd), fpw);
	for(i=0;i<0x150;i++) putc(0, fpw);

	memset(&sd, 0, sizeof(struct sechd));
	sd.R = 1; sd.N = 1;
	sd.secn = 0x10; sd.mark = 1;
	for(;;)
	{
		/* 80trackになったらやめ */
		if (sd.C > 0x27) break;
		/* セクタごとに書き込み */
		i = fread(mp, 1, 0x100, fpr);
		if (sd.R == 1 && sd.N == 1)
		{
			if (mp[0xf] == 'y') mp[0xf] = 'Y';
			if (mp[0x10] == 's') mp[0x10] = 'S';
		}
		if (i != 0x100) { puts("イメージファイルが読めません"); break; }
		fwrite(&sd, 1, sizeof(struct sechd), fpw);
		fwrite(mp, 1, i, fpw);
		sd.R++;
		if (sd.R > 0x10) { sd.R = 1; sd.H++; }
		if (sd.H > 1) { sd.H = 0; sd.C++; }
	}
	fclose(fpw);
}


/*
	.D88 -> .2D 変換
*/
void conv_2D(FILE *fpr, char *fname, char *mp)
{
	int i, tr, sec, secn, ct;
	long baseoffs, offs, filelen;
	struct diskhd hd;
	struct sechd hs;
	FILE *fpw;
	char secF[16], wfile[64];

	filelen = filelength(fileno(fpr));
	rewind(fpr);

	for(i=0;;i++)
	{
		baseoffs = ftell(fpr);
		if (baseoffs >= filelen) break;

		/* 0,1,2,…9,A,B,C,… */
		if (i<10)
			sprintf(wfile, "%s.2D%d", fname, i);
		else
			sprintf(wfile, "%s.2D%c", fname, 'A'+i-10);

		fpw = fopen(wfile, "wb");
		if (!fpw) invalid("Write-File cannot create.");

		fread(&hd, 1, sizeof(struct diskhd), fpr);
		fputs("Converting ->", stdout); puts(hd.title);
		for(tr=0;tr<80;tr++)
		{
			fseek(fpr, hd.next_track[tr] + baseoffs, SEEK_SET);
			memset(secF, 0, 16); memset(mp, 0xff, 0x1000);
			/* このトラックのセクタ数を取得 */
			fread(&hs, 1, 16, fpr);
			fseek(fpr, -16L, SEEK_CUR);
			secn = hs.secn;
			if (secn != 16)
				printf("Tr=%d secn=%d セクタ数が不正\n", tr, secn);
			for(sec=0;sec<secn;sec++)
			{
				fread(&hs, 1, 16, fpr);
				if ((hs.C * 2 + hs.H) != tr)
				{
					printf("シリンダが不正: fpos=%08lx tr=%d sec=%d ",
						ftell(fpr)-16L, tr, sec);
					printf("C=%d H=%d R=%d N=%d\n", hs.C, hs.H, hs.R, hs.N);
					continue;
				}
				if (hs.R < 1 || hs.R > 16 || hs.N != 1)
				{
					printf("セクタ番号が不正: fpos=%08lx tr=%d sec=%d ",
						ftell(fpr)-16L, tr, sec);
					printf("C=%d H=%d R=%d N=%d\n", hs.C, hs.H, hs.R, hs.N);
					continue;
				}
				ct = fread(mp + (hs.R-1)*256, 1, 256, fpr);
				if (ct < 256) invalid("ファイルが途中で終わっています");
				secF[hs.R - 1] = 1;
			}
			for(sec=0;sec<16;sec++)
			{	/* 欠落セクタのチェック */
				if (!secF[sec])
					printf("Tr=%d sec=%d がありません\n", tr, sec);
			}
			fwrite(mp, 1, 0x1000, fpw);
		}
		fclose(fpw);
//		baseoffs += hd.next_disk;
	}
}


int main(int argc, char **argv)
{
	FILE *fpr, *fpw;
	char *p, *mp;
	char wfile[64];

	if ((argc != 3) && (argc != 4)) usage();

	/* 動作モードをチェック */
	modeF = 0;
	if (!strcmp(argv[1], "-d88")) modeF = 1;
	if (!strcmp(argv[1], "-2d")) modeF = 2;
	if (!modeF) usage();

	fpr = fopen(argv[2], "rb");
	if (!fpr) invalid("Read-file cannot open");

	memset(d88title, 0, 16);
	if (argv[3])
	{	/* タイトルがあれば取得する */
		if (strlen(argv[3]) > 16) invalid("タイトルは最大16文字です");
		strcpy(d88title, argv[3]);
	}
	else
		strcpy(d88title, "88Disk4SPC1500");

	/* 作業バッファ確保 */
	mp = malloc(0x1000);
	if (!mp) invalid("malloc error");

	/* 書き込みファイル名設定 */
	strcpy(wfile, argv[2]);
	p = strchr(wfile, '.');
	if (p) *p = '\0';

	if (modeF == 1) conv_D88(fpr, wfile, mp); else conv_2D(fpr, wfile, mp);

	free(mp);
	return 0;
}
/*
1998/06/02 v0.02
　D88->2Dコンバート時に複数ディスクをコンバートできるようにする
*/
