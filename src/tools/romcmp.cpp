// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nicola Salmoria
/***************************************************************************

    romcmp.c

    ROM comparison utility program.

***************************************************************************/

#include "unzip.h"
#include "osdcore.h"
#include "osdcomm.h"

#include <stdarg.h>
#include <stdlib.h>


#define MAX_FILES 1000

#ifndef MAX_FILENAME_LEN
#define MAX_FILENAME_LEN 255
#endif

#ifndef PATH_DELIM
#define PATH_DELIM '/'
#endif



/* compare modes when one file is twice as long as the other */
/* A = All file */
/* 12 = 1st half */
/* 22 = 2nd half */
/* E = Even bytes */
/* O = Odd bytes */
/* E1 = Even bytes 1st half */
/* O1 = Odd bytes 1st half */
/* E2 = Even bytes 2nd half */
/* O2 = Odd bytes 2nd half */
enum
{
	MODE_A,
	MODE_NIB1,MODE_NIB2,
	MODE_12, MODE_22,
	MODE_14, MODE_24, MODE_34, MODE_44,
	MODE_E, MODE_O,
	MODE_E12, MODE_O12, MODE_E22, MODE_O22,
	TOTAL_MODES
};

static const char *const modenames[] =
{
	"          ",
	"[bits 0-3]",
	"[bits 4-7]",
	"[1/2]     ",
	"[2/2]     ",
	"[1/4]     ",
	"[2/4]     ",
	"[3/4]     ",
	"[4/4]     ",
	"[even]    ",
	"[odd]     ",
	"[even 1/2]",
	"[odd 1/2] ",
	"[even 2/2]",
	"[odd 2/2] ",
};

static void compatiblemodes(int mode,int *start,int *end)
{
	if (mode == MODE_A)
	{
		*start = MODE_A;
		*end = MODE_A;
	}
	if (mode >= MODE_NIB1 && mode <= MODE_NIB2)
	{
		*start = MODE_NIB1;
		*end = MODE_NIB2;
	}
	if (mode >= MODE_12 && mode <= MODE_22)
	{
		*start = MODE_12;
		*end = MODE_22;
	}
	if (mode >= MODE_14 && mode <= MODE_44)
	{
		*start = MODE_14;
		*end = MODE_44;
	}
	if (mode >= MODE_E && mode <= MODE_O)
	{
		*start = MODE_E;
		*end = MODE_O;
	}
	if (mode >= MODE_E12 && mode <= MODE_O22)
	{
		*start = MODE_E12;
		*end = MODE_O22;
	}
}

struct fileinfo
{
	char name[MAX_FILENAME_LEN+1];
	int size;
	unsigned char *buf; /* file is read in here */
	int listed;
};

static fileinfo files[2][MAX_FILES];
static float matchscore[MAX_FILES][MAX_FILES][TOTAL_MODES][TOTAL_MODES];


static void checkintegrity(const fileinfo *file,int side)
{
	int i;
	int mask0,mask1;
	int addrbit;

	if (file->buf == nullptr) return;

	/* check for bad data lines */
	mask0 = 0x0000;
	mask1 = 0xffff;

	for (i = 0;i < file->size;i+=2)
	{
		mask0 |= ((file->buf[i] << 8) | file->buf[i+1]);
		mask1 &= ((file->buf[i] << 8) | file->buf[i+1]);
		if (mask0 == 0xffff && mask1 == 0x0000) break;
	}

	if (mask0 != 0xffff || mask1 != 0x0000)
	{
		int fixedmask;
		int bits;


		fixedmask = (~mask0 | mask1) & 0xffff;

		if (((mask0 >> 8) & 0xff) == (mask0 & 0xff) && ((mask1 >> 8) & 0xff) == (mask1 & 0xff))
			bits = 8;
		else bits = 16;

		printf("%-23s %-23s FIXED BITS (",side ? "" : file->name,side ? file->name : "");
		for (i = 0;i < bits;i++)
		{
			if (~mask0 & 0x8000) printf("0");
			else if (mask1 & 0x8000) printf("1");
			else printf("x");

			mask0 <<= 1;
			mask1 <<= 1;
		}
		printf(")\n");

		/* if the file contains a fixed value, we don't need to do the other */
		/* validity checks */
		if (fixedmask == 0xffff || fixedmask == 0x00ff || fixedmask == 0xff00)
			return;
	}


	addrbit = 1;
	mask0 = 0;
	while (addrbit <= file->size/2)
	{
		for (i = 0;i < file->size;i++)
		{
			if (file->buf[i] != file->buf[i ^ addrbit]) break;
		}

		if (i == file->size)
			mask0 |= addrbit;

		addrbit <<= 1;
	}

	if (mask0)
	{
		if (mask0 == file->size/2)
			printf("%-23s %-23s 1ST AND 2ND HALF IDENTICAL\n",side ? "" : file->name,side ? file->name : "");
		else
		{
			printf("%-23s %-23s BADADDR",side ? "" : file->name,side ? file->name : "");
			for (i = 0;i < 24;i++)
			{
				if (file->size <= (1<<(23-i))) printf(" ");
				else if (mask0 & 0x800000) printf("-");
				else printf("x");
				mask0 <<= 1;
			}
			printf("\n");
		}
		return;
	}

	mask0 = 0x000000;
	mask1 = file->size-1;
	for (i = 0;i < file->size;i++)
	{
		if (file->buf[i] != 0xff)
		{
			mask0 |= i;
			mask1 &= i;
			if (mask0 == file->size-1 && mask1 == 0x00) break;
		}
	}

	if (mask0 != file->size-1 || mask1 != 0x00)
	{
		printf("%-23s %-23s ",side ? "" : file->name,side ? file->name : "");
		for (i = 0;i < 24;i++)
		{
			if (file->size <= (1<<(23-i))) printf(" ");
			else if (~mask0 & 0x800000) printf("1");
			else if (mask1 & 0x800000) printf("0");
			else printf("x");
			mask0 <<= 1;
			mask1 <<= 1;
		}
		printf(" = 0xFF\n");

		return;
	}


	mask0 = 0x000000;
	mask1 = file->size-1;
	for (i = 0;i < file->size;i++)
	{
		if (file->buf[i] != 0x00)
		{
			mask0 |= i;
			mask1 &= i;
			if (mask0 == file->size-1 && mask1 == 0x00) break;
		}
	}

	if (mask0 != file->size-1 || mask1 != 0x00)
	{
		printf("%-23s %-23s ",side ? "" : file->name,side ? file->name : "");
		for (i = 0;i < 24;i++)
		{
			if (file->size <= (1<<(23-i))) printf(" ");
			else if ((mask0 & 0x800000) == 0) printf("1");
			else if (mask1 & 0x800000) printf("0");
			else printf("x");
			mask0 <<= 1;
			mask1 <<= 1;
		}
		printf(" = 0x00\n");

		return;
	}


	mask0 = 0xff;
	for (i = 0;i < file->size/4 && mask0;i++)
	{
		if (file->buf[               2*i  ] != 0x00) mask0 &= ~0x01;
		if (file->buf[               2*i  ] != 0xff) mask0 &= ~0x02;
		if (file->buf[               2*i+1] != 0x00) mask0 &= ~0x04;
		if (file->buf[               2*i+1] != 0xff) mask0 &= ~0x08;
		if (file->buf[file->size/2 + 2*i  ] != 0x00) mask0 &= ~0x10;
		if (file->buf[file->size/2 + 2*i  ] != 0xff) mask0 &= ~0x20;
		if (file->buf[file->size/2 + 2*i+1] != 0x00) mask0 &= ~0x40;
		if (file->buf[file->size/2 + 2*i+1] != 0xff) mask0 &= ~0x80;
	}

	if (mask0 & 0x01) printf("%-23s %-23s 1ST HALF = 00xx\n",side ? "" : file->name,side ? file->name : "");
	if (mask0 & 0x02) printf("%-23s %-23s 1ST HALF = FFxx\n",side ? "" : file->name,side ? file->name : "");
	if (mask0 & 0x04) printf("%-23s %-23s 1ST HALF = xx00\n",side ? "" : file->name,side ? file->name : "");
	if (mask0 & 0x08) printf("%-23s %-23s 1ST HALF = xxFF\n",side ? "" : file->name,side ? file->name : "");
	if (mask0 & 0x10) printf("%-23s %-23s 2ND HALF = 00xx\n",side ? "" : file->name,side ? file->name : "");
	if (mask0 & 0x20) printf("%-23s %-23s 2ND HALF = FFxx\n",side ? "" : file->name,side ? file->name : "");
	if (mask0 & 0x40) printf("%-23s %-23s 2ND HALF = xx00\n",side ? "" : file->name,side ? file->name : "");
	if (mask0 & 0x80) printf("%-23s %-23s 2ND HALF = xxFF\n",side ? "" : file->name,side ? file->name : "");
}


static int usedbytes(const fileinfo *file,int mode)
{
	switch (mode)
	{
		case MODE_A:
		case MODE_NIB1:
		case MODE_NIB2:
			return file->size;
		case MODE_12:
		case MODE_22:
		case MODE_E:
		case MODE_O:
			return file->size / 2;
		case MODE_14:
		case MODE_24:
		case MODE_34:
		case MODE_44:
		case MODE_E12:
		case MODE_O12:
		case MODE_E22:
		case MODE_O22:
			return file->size / 4;
		default:
			return 0;
	}
}

static void basemultmask(const fileinfo *file,int mode,int *base,int *mult,int *mask)
{
	*mult = 1;
	if (mode >= MODE_E) *mult = 2;

	switch (mode)
	{
		case MODE_A:
		case MODE_12:
		case MODE_14:
		case MODE_E:
		case MODE_E12:
			*base = 0; *mask = 0xff; break;
		case MODE_NIB1:
			*base = 0; *mask = 0x0f; break;
		case MODE_NIB2:
			*base = 0; *mask = 0xf0; break;
		case MODE_O:
		case MODE_O12:
			*base = 1; *mask = 0xff; break;
		case MODE_22:
		case MODE_E22:
			*base = file->size / 2; *mask = 0xff; break;
		case MODE_O22:
			*base = 1 + file->size / 2; *mask = 0xff; break;
		case MODE_24:
			*base = file->size / 4; *mask = 0xff; break;
		case MODE_34:
			*base = 2*file->size / 4; *mask = 0xff; break;
		case MODE_44:
			*base = 3*file->size / 4; *mask = 0xff; break;
	}
}

static float filecompare(const fileinfo *file1,const fileinfo *file2,int mode1,int mode2)
{
	int i;
	int match = 0;
	int size1,size2;
	int base1=0,base2=0,mult1=0,mult2=0,mask1=0,mask2=0;


	if (file1->buf == nullptr || file2->buf == nullptr) return 0.0;

	size1 = usedbytes(file1,mode1);
	size2 = usedbytes(file2,mode2);

	if (size1 != size2) return 0.0;

	basemultmask(file1,mode1,&base1,&mult1,&mask1);
	basemultmask(file2,mode2,&base2,&mult2,&mask2);

	if (mask1 == mask2)
	{
		if (mask1 == 0xff)
		{
			/* normal compare */
			for (i = 0;i < size1;i++)
				if (file1->buf[base1 + mult1 * i] == file2->buf[base2 + mult2 * i]) match++;
		}
		else
		{
			/* nibble compare, abort if other half is not empty */
			for (i = 0;i < size1;i++)
			{
				if (((file1->buf[base1 + mult1 * i] & ~mask1) != (0x00 & ~mask1) &&
						(file1->buf[base1 + mult1 * i] & ~mask1) != (0xff & ~mask1)) ||
					((file2->buf[base1 + mult1 * i] & ~mask2) != (0x00 & ~mask2) &&
						(file2->buf[base1 + mult1 * i] & ~mask2) != (0xff & ~mask2)))
				{
					match = 0;
					break;
				}
				if ((file1->buf[base1 + mult1 * i] & mask1) == (file2->buf[base2 + mult2 * i] & mask2)) match++;
			}
		}
	}

	return (float)match / size1;
}


static void readfile(const char *path,fileinfo *file)
{
	file_error filerr;
	UINT64 filesize;
	UINT32 actual;
	char fullname[256];
	osd_file *f = nullptr;

	if (path)
	{
		char delim[2] = { PATH_DELIM, '\0' };
		strcpy(fullname,path);
		strcat(fullname,delim);
	}
	else fullname[0] = 0;
	strcat(fullname,file->name);

	if ((file->buf = (unsigned char *)malloc(file->size)) == nullptr)
	{
		printf("%s: out of memory!\n",file->name);
		return;
	}

	filerr = osd_open(fullname, OPEN_FLAG_READ, &f, &filesize);
	if (filerr != FILERR_NONE)
	{
		printf("%s: error %d\n", fullname, filerr);
		return;
	}

	filerr = osd_read(f, file->buf, 0, file->size, &actual);
	if (filerr != FILERR_NONE)
	{
		printf("%s: error %d\n", fullname, filerr);
		osd_close(f);
		return;
	}

	osd_close(f);
}


static void freefile(fileinfo *file)
{
	free(file->buf);
	file->buf = nullptr;
}


static void printname(const fileinfo *file1,const fileinfo *file2,float score,int mode1,int mode2)
{
	printf("%-12s %s %-12s %s ",file1 ? file1->name : "",modenames[mode1],file2 ? file2->name : "",modenames[mode2]);
	if (score == 0.0f) printf("NO MATCH\n");
	else if (score == 1.0f) printf("IDENTICAL\n");
	else printf("%3.6f%%\n",(double) (score*100));
}


static int load_files(int i, int *found, const char *path)
{
	osd_directory *dir;

	/* attempt to open as a directory first */
	dir = osd_opendir(path);
	if (dir != nullptr)
	{
		const osd_directory_entry *d;

		/* load all files in directory */
		while ((d = osd_readdir(dir)) != nullptr)
		{
			const char *d_name = d->name;
			char buf[255+1];

			sprintf(buf, "%s%c%s", path, PATH_DELIM, d_name);
			if (d->type == ENTTYPE_FILE)
			{
				UINT64 size = d->size;
				while (size && (size & 1) == 0) size >>= 1;
				//if (size & ~1)
				//  printf("%-23s %-23s ignored (not a ROM)\n",i ? "" : d_name,i ? d_name : "");
				//else
				{
					strcpy(files[i][found[i]].name,d_name);
					files[i][found[i]].size = d->size;
					readfile(path,&files[i][found[i]]);
					files[i][found[i]].listed = 0;
					if (found[i] >= MAX_FILES)
					{
						printf("%s: max of %d files exceeded\n",path,MAX_FILES);
						break;
					}
					found[i]++;
				}
			}
		}
		osd_closedir(dir);
	}

	/* if not, try to open as a ZIP file */
	else
	{
		zip_file *zip;
		const zip_file_header* zipent;
		zip_error ziperr;

		/* wasn't a directory, so try to open it as a zip file */
		ziperr = zip_file_open(path, &zip);
		if (ziperr != ZIPERR_NONE)
		{
			printf("Error, cannot open zip file '%s' !\n", path);
			return 1;
		}

		/* load all files in zip file */
		for (zipent = zip_file_first_file(zip); zipent != nullptr; zipent = zip_file_next_file(zip))
		{
			int size;

			size = zipent->uncompressed_length;
			while (size && (size & 1) == 0) size >>= 1;
			if (zipent->uncompressed_length == 0) // || (size & ~1))
				printf("%-23s %-23s ignored (not a ROM)\n",
					i ? "" : zipent->filename, i ? zipent->filename : "");
			else
			{
				fileinfo *file = &files[i][found[i]];
				const char *delim = strrchr(zipent->filename,'/');

				if (delim)
					strcpy (file->name,delim+1);
				else
					strcpy(file->name,zipent->filename);
				file->size = zipent->uncompressed_length;
				if ((file->buf = (unsigned char *)malloc(file->size)) == nullptr)
					printf("%s: out of memory!\n",file->name);
				else
				{
					if (zip_file_decompress(zip, (char *)file->buf, file->size) != ZIPERR_NONE)
					{
						free(file->buf);
						file->buf = nullptr;
					}
				}

				file->listed = 0;
				if (found[i] >= MAX_FILES)
				{
					printf("%s: max of %d files exceeded\n",path,MAX_FILES);
					break;
				}
				found[i]++;
			}
		}
		zip_file_close(zip);
	}
	return 0;
}


int CLIB_DECL main(int argc,char *argv[])
{
	int err;
	int total_modes = MODE_NIB2;    /* by default, use only MODE_A, MODE_NIB1 and MODE_NIB2 */

	if (argc >= 2 && strcmp(argv[1],"-d") == 0)
	{
		argc--;
		argv++;
		total_modes = TOTAL_MODES;
	}

	if (argc < 2)
	{
		printf("usage: romcmp [-d] [dir1 | zip1] [dir2 | zip2]\n");
		printf("-d enables a slower, more comprehensive comparison.\n");
		return 0;
	}

	{
		int found[2];
		int i,j,mode1,mode2;
		int besti,bestj;


		found[0] = found[1] = 0;
		for (i = 0;i < 2;i++)
		{
			if (argc > i+1)
			{
				err = load_files (i, found, argv[i+1]);
				if (err != 0)
					return err;
			}
		}

		if (argc >= 3)
			printf("%d and %d files\n",found[0],found[1]);
		else
			printf("%d files\n",found[0]);

		for (i = 0;i < found[0];i++)
		{
			checkintegrity(&files[0][i],0);
		}

		for (j = 0;j < found[1];j++)
		{
			checkintegrity(&files[1][j],1);
		}

		if (argc < 3)
		{
			/* find duplicates in one dir */
			for (i = 0;i < found[0];i++)
			{
				for (j = i+1;j < found[0];j++)
				{
					for (mode1 = 0;mode1 < total_modes;mode1++)
					{
						for (mode2 = 0;mode2 < total_modes;mode2++)
						{
							if (filecompare(&files[0][i],&files[0][j],mode1,mode2) == 1.0f)
								printname(&files[0][i],&files[0][j],1.0,mode1,mode2);
						}
					}
				}
			}
		}
		else
		{
			/* compare two dirs */
			for (i = 0;i < found[0];i++)
			{
				for (j = 0;j < found[1];j++)
				{
					fprintf(stderr,"%2d%%\r",100*(i*found[1]+j)/(found[0]*found[1]));
					for (mode1 = 0;mode1 < total_modes;mode1++)
					{
						for (mode2 = 0;mode2 < total_modes;mode2++)
						{
							matchscore[i][j][mode1][mode2] = filecompare(&files[0][i],&files[1][j],mode1,mode2);
						}
					}
				}
			}
			fprintf(stderr,"   \r");

			do
			{
				float bestscore;
				int bestmode1,bestmode2;

				besti = -1;
				bestj = -1;
				bestscore = 0.0;
				bestmode1 = bestmode2 = -1;

				for (mode1 = 0;mode1 < total_modes;mode1++)
				{
					for (mode2 = 0;mode2 < total_modes;mode2++)
					{
						for (i = 0;i < found[0];i++)
						{
							for (j = 0;j < found[1];j++)
							{
								if (matchscore[i][j][mode1][mode2] > bestscore
									|| (matchscore[i][j][mode1][mode2] == 1.0f && mode2 == 0 && bestmode2 > 0))
								{
									bestscore = matchscore[i][j][mode1][mode2];
									besti = i;
									bestj = j;
									bestmode1 = mode1;
									bestmode2 = mode2;
								}
							}
						}
					}
				}

				if (besti != -1)
				{
					int start=0,end=0;

					printname(&files[0][besti],&files[1][bestj],bestscore,bestmode1,bestmode2);
					files[0][besti].listed = 1;
					files[1][bestj].listed = 1;

					matchscore[besti][bestj][bestmode1][bestmode2] = 0.0;

					/* remove all matches using the same sections with a worse score */
					for (j = 0;j < found[1];j++)
					{
						for (mode2 = 0;mode2 < total_modes;mode2++)
						{
							if (matchscore[besti][j][bestmode1][mode2] < bestscore)
								matchscore[besti][j][bestmode1][mode2] = 0.0;
						}
					}
					for (i = 0;i < found[0];i++)
					{
						for (mode1 = 0;mode1 < total_modes;mode1++)
						{
							if (matchscore[i][bestj][mode1][bestmode2] < bestscore)
								matchscore[i][bestj][mode1][bestmode2] = 0.0;
						}
					}

					/* remove all matches using incompatible sections */
					compatiblemodes(bestmode1,&start,&end);
					for (j = 0;j < found[1];j++)
					{
						for (mode2 = 0;mode2 < total_modes;mode2++)
						{
							for (mode1 = 0;mode1 < start;mode1++)
								matchscore[besti][j][mode1][mode2] = 0.0;
							for (mode1 = end+1;mode1 < total_modes;mode1++)
								matchscore[besti][j][mode1][mode2] = 0.0;
						}
					}
					compatiblemodes(bestmode2,&start,&end);
					for (i = 0;i < found[0];i++)
					{
						for (mode1 = 0;mode1 < total_modes;mode1++)
						{
							for (mode2 = 0;mode2 < start;mode2++)
								matchscore[i][bestj][mode1][mode2] = 0.0;
							for (mode2 = end+1;mode2 < total_modes;mode2++)
								matchscore[i][bestj][mode1][mode2] = 0.0;
						}
					}
				}
			} while (besti != -1);


			for (i = 0;i < found[0];i++)
			{
				if (files[0][i].listed == 0) printname(&files[0][i],nullptr,0.0,0,0);
			}
			for (i = 0;i < found[1];i++)
			{
				if (files[1][i].listed == 0) printname(nullptr,&files[1][i],0.0,0,0);
			}
		}


		for (i = 0;i < found[0];i++)
		{
			freefile(&files[0][i]);
		}
		for (i = 0;i < found[1];i++)
		{
			freefile(&files[1][i]);
		}
	}

	zip_file_cache_clear();
	return 0;
}
