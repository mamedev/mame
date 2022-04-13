// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nicola Salmoria
/***************************************************************************

    romcmp.c

    ROM comparison utility program.

***************************************************************************/

#include "hash.h"
#include "path.h"
#include "unzip.h"

#include "osdfile.h"
#include "osdcomm.h"

#include <cstdarg>
#include <cstdlib>
#include <memory>


#define MAX_FILES 1000



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
	std::string name;
	int size;
	std::unique_ptr<unsigned char []> buf; // file is read in here
	int listed;


	static constexpr bool is_ascii_char(int ch)
	{
		return (ch >= 0x20 && ch < 0x7f) || (ch == '\n') || (ch == '\r') || (ch == '\t');
	}


	void checkintegrity(int side, bool all_hashes) const
	{
		if (!buf)
			return;

		if (all_hashes)
		{
			util::crc32_creator crc32;
			util::sha1_creator sha1;
			util::sum16_creator sum16;
			crc32.append(buf.get(), size);
			sha1.append(buf.get(), size);
			sum16.append(buf.get(), size);
			printf("%-23s %-23s [0x%x] CRC(%s) SHA1(%s) SUM(%s)\n",
					(side & 1) ? name.c_str() : "",
					(side & 2) ? name.c_str() : "",
					size,
					crc32.finish().as_string().c_str(),
					sha1.finish().as_string().c_str(),
					sum16.finish().as_string().c_str());
			side = 0;
		}

		// check for bad data lines
		unsigned mask0 = 0x0000;
		unsigned mask1 = 0xffff;

		bool is_ascii = true;
		for (unsigned i = 0; i < size; i += 2)
		{
			is_ascii = is_ascii && is_ascii_char(buf[i]);
			mask0 |= buf[i] << 8;
			mask1 &= (buf[i] << 8) | 0x00ff;
			if (i < size - 1)
			{
				is_ascii = is_ascii && is_ascii_char(buf[i+1]);
				mask0 |= buf[i+1];
				mask1 &= buf[i+1] | 0xff00;
			}
			if (mask0 == 0xffff && mask1 == 0x0000) break;
		}

		if (is_ascii && mask0 == 0x7f7f && mask1 == 0)
		{
			printf("%-23s %-23s ASCII TEXT FILE\n", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
			return;
		}

		if (mask0 != 0xffff || mask1 != 0x0000)
		{
			int fixedmask;
			int bits;

			fixedmask = (~mask0 | mask1) & 0xffff;

			if (((mask0 >> 8) & 0xff) == (mask0 & 0xff) && ((mask1 >> 8) & 0xff) == (mask1 & 0xff))
				bits = 8;
			else bits = 16;

			printf("%-23s %-23s FIXED BITS (", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
			for (int i = 0; i < bits; i++)
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

		unsigned addrbit = 1;
		unsigned addrmirror = 0;
		while (addrbit <= size/2)
		{
			unsigned i = 0;
			for (i = 0; i < size; i++)
			{
				if ((i ^ addrbit) < size && buf[i] != buf[i ^ addrbit]) break;
			}

			if (i == size)
				addrmirror |= addrbit;

			addrbit <<= 1;
		}

		if (addrmirror != 0)
		{
			if (addrmirror == size/2)
			{
				printf("%-23s %-23s 1ST AND 2ND HALF IDENTICAL\n", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
				util::hash_collection hash;
				hash.begin();
				hash.buffer(buf.get(), size / 2);
				hash.end();
				printf("%-23s %-23s                  %s\n", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "", hash.attribute_string().c_str());
			}
			else
			{
				printf("%-23s %-23s BADADDR", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
				for (int i = 0; i < 24; i++)
				{
					if (size <= (1<<(23-i))) printf(" ");
					else if (addrmirror & 0x800000) printf("-");
					else printf("x");
					addrmirror <<= 1;
				}
				printf("\n");
			}
			return;
		}

		unsigned sizemask = 1;
		while (sizemask < size - 1)
			sizemask = (sizemask << 1) | 1;

		mask0 = 0x000000;
		mask1 = sizemask;
		for (unsigned i = 0; i < size; i++)
		{
			if (buf[i] != 0xff)
			{
				mask0 |= i;
				mask1 &= i;
				if (mask0 == sizemask && mask1 == 0x00) break;
			}
		}

		if (mask0 != sizemask || mask1 != 0x00)
		{
			printf("%-23s %-23s ", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
			for (int i = 0; i < 24; i++)
			{
				if (size <= (1<<(23-i))) printf(" ");
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
		mask1 = sizemask;
		for (unsigned i = 0; i < size; i++)
		{
			if (buf[i] != 0x00)
			{
				mask0 |= i;
				mask1 &= i;
				if (mask0 == sizemask && mask1 == 0x00) break;
			}
		}

		if (mask0 != sizemask || mask1 != 0x00)
		{
			printf("%-23s %-23s ", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
			for (int i = 0; i < 24; i++)
			{
				if (size <= (1<<(23-i))) printf(" ");
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
		for (unsigned i = 0; i < size/4 && mask0 != 0x00; i++)
		{
			if (buf[         2*i  ] != 0x00) mask0 &= ~0x01;
			if (buf[         2*i  ] != 0xff) mask0 &= ~0x02;
			if (buf[         2*i+1] != 0x00) mask0 &= ~0x04;
			if (buf[         2*i+1] != 0xff) mask0 &= ~0x08;
			if (buf[size/2 + 2*i  ] != 0x00) mask0 &= ~0x10;
			if (buf[size/2 + 2*i  ] != 0xff) mask0 &= ~0x20;
			if (buf[size/2 + 2*i+1] != 0x00) mask0 &= ~0x40;
			if (buf[size/2 + 2*i+1] != 0xff) mask0 &= ~0x80;
		}

		if (mask0 & 0x01) printf("%-23s %-23s 1ST HALF = 00xx\n", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
		if (mask0 & 0x02) printf("%-23s %-23s 1ST HALF = FFxx\n", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
		if (mask0 & 0x04) printf("%-23s %-23s 1ST HALF = xx00\n", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
		if (mask0 & 0x08) printf("%-23s %-23s 1ST HALF = xxFF\n", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
		if (mask0 & 0x10) printf("%-23s %-23s 2ND HALF = 00xx\n", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
		if (mask0 & 0x20) printf("%-23s %-23s 2ND HALF = FFxx\n", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
		if (mask0 & 0x40) printf("%-23s %-23s 2ND HALF = xx00\n", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
		if (mask0 & 0x80) printf("%-23s %-23s 2ND HALF = xxFF\n", (side & 1) ? name.c_str() : "", (side & 2) ? name.c_str() : "");
	}

	int usedbytes(int mode) const
	{
		switch (mode)
		{
			case MODE_A:
			case MODE_NIB1:
			case MODE_NIB2:
				return size;
			case MODE_12:
			case MODE_22:
			case MODE_E:
			case MODE_O:
				return size / 2;
			case MODE_14:
			case MODE_24:
			case MODE_34:
			case MODE_44:
			case MODE_E12:
			case MODE_O12:
			case MODE_E22:
			case MODE_O22:
				return size / 4;
			default:
				return 0;
		}
	}

	void basemultmask(int mode, int &base, int &mult, int &mask) const
	{
		mult = 1;
		if (mode >= MODE_E)
			mult = 2;

		switch (mode)
		{
			case MODE_A:
			case MODE_12:
			case MODE_14:
			case MODE_E:
			case MODE_E12:
				base = 0; mask = 0xff; break;
			case MODE_NIB1:
				base = 0; mask = 0x0f; break;
			case MODE_NIB2:
				base = 0; mask = 0xf0; break;
			case MODE_O:
			case MODE_O12:
				base = 1; mask = 0xff; break;
			case MODE_22:
			case MODE_E22:
				base = size / 2; mask = 0xff; break;
			case MODE_O22:
				base = 1 + size / 2; mask = 0xff; break;
			case MODE_24:
				base = size / 4; mask = 0xff; break;
			case MODE_34:
				base = 2*size / 4; mask = 0xff; break;
			case MODE_44:
				base = 3*size / 4; mask = 0xff; break;
		}
	}

	float compare(const fileinfo &file2, int mode1, int mode2) const
	{
		if (!buf || !file2.buf)
			return 0.0;

		int const size1 = usedbytes(mode1);
		int const size2 = file2.usedbytes(mode2);

		if (size1 != size2)
			return 0.0;

		int base1=0, base2=0, mult1=0, mult2=0, mask1=0, mask2=0;
		basemultmask(mode1, base1, mult1, mask1);
		file2.basemultmask(mode2, base2, mult2, mask2);

		int match = 0;
		if (mask1 == mask2)
		{
			if (mask1 == 0xff)
			{
				// normal compare
				for (int i = 0; i < size1; i++)
					if (buf[base1 + mult1 * i] == file2.buf[base2 + mult2 * i]) match++;
			}
			else
			{
				// nibble compare, abort if other half is not empty
				for (int i = 0; i < size1; i++)
				{
					if (((buf[base1 + mult1 * i] & ~mask1) != (0x00 & ~mask1) &&
							(buf[base1 + mult1 * i] & ~mask1) != (0xff & ~mask1)) ||
						((file2.buf[base1 + mult1 * i] & ~mask2) != (0x00 & ~mask2) &&
							(file2.buf[base1 + mult1 * i] & ~mask2) != (0xff & ~mask2)))
					{
						match = 0;
						break;
					}
					if ((buf[base1 + mult1 * i] & mask1) == (file2.buf[base2 + mult2 * i] & mask2)) match++;
				}
			}
		}

		return float(match) / size1;
	}

	void readfile(const char *path)
	{
		std::string fullname(path ? path : "");
		util::path_append(fullname, name);

		buf.reset(new (std::nothrow) unsigned char [size]);
		if (!buf)
		{
			printf("%s: out of memory!\n", name.c_str());
			return;
		}

		std::error_condition filerr;
		osd_file::ptr f;
		uint64_t filesize;

		filerr = osd_file::open(fullname, OPEN_FLAG_READ, f, filesize);
		if (filerr)
		{
			printf("%s: error %s\n", fullname.c_str(), filerr.message().c_str());
			return;
		}

		uint32_t actual;
		filerr = f->read(buf.get(), 0, size, actual);
		if (filerr)
		{
			printf("%s: error %s\n", fullname.c_str(), filerr.message().c_str());
			return;
		}
	}

	void free()
	{
		buf.reset();
	}
};

static fileinfo files[2][MAX_FILES];
static float matchscore[MAX_FILES][MAX_FILES][TOTAL_MODES][TOTAL_MODES];


static void printname(const fileinfo *file1, const fileinfo *file2, float score, int mode1, int mode2)
{
	printf(
			"%-12s %s %-12s %s ",
			file1 ? file1->name.c_str() : "",
			modenames[mode1],
			file2 ? file2->name.c_str() : "",
			modenames[mode2]);
	if (score == 0.0f) printf("NO MATCH\n");
	else if (score == 1.0f) printf("IDENTICAL\n");
	else printf("%3.6f%%\n", double(score*100));
}


static int load_files(int i, int *found, const char *path)
{
	/* attempt to open as a directory first */
	auto dir = osd::directory::open(path);
	if (dir)
	{
		const osd::directory::entry *d;

		/* load all files in directory */
		while ((d = dir->read()) != nullptr)
		{
			const char *d_name = d->name;
			const std::string buf(util::path_concat(path, d_name)); // FIXME: is this even used for anything?

			if (d->type == osd::directory::entry::entry_type::FILE)
			{
				uint64_t size = d->size;
				while (size && (size & 1) == 0) size >>= 1;
				//if (size & ~1)
				//  printf("%-23s %-23s ignored (not a ROM)\n", i ? "" : d_name, i ? d_name : "");
				//else
				{
					files[i][found[i]].name = d_name;
					files[i][found[i]].size = d->size;
					files[i][found[i]].readfile(path);
					files[i][found[i]].listed = 0;
					if (found[i] >= MAX_FILES)
					{
						printf("%s: max of %d files exceeded\n", path, MAX_FILES);
						break;
					}
					found[i]++;
				}
			}
		}
		dir.reset();
	}
	else
	{
		/* if not, try to open as a ZIP file */
		util::archive_file::ptr zip;

		/* wasn't a directory, so try to open it as a zip file */
		if (util::archive_file::open_zip(path, zip) && util::archive_file::open_7z(path, zip))
		{
			printf("Error, cannot open zip file '%s' !\n", path);
			return 1;
		}

		/* load all files in zip file */
		for (int zipent = zip->first_file(); zipent >= 0; zipent = zip->next_file())
		{
			if (zip->current_is_directory()) continue;

			int size;

			size = zip->current_uncompressed_length();
			while (size && (size & 1) == 0) size >>= 1;
			if (zip->current_uncompressed_length() == 0) // || (size & ~1))
			{
				printf("%-23s %-23s ignored (not a ROM)\n",
					i ? "" : zip->current_name().c_str(), i ? zip->current_name().c_str() : "");
			}
			else
			{
				fileinfo &file = files[i][found[i]];
				const char *delim = strrchr(zip->current_name().c_str(), '/');

				if (delim)
					file.name = delim + 1;
				else
					file.name = zip->current_name();
				file.size = zip->current_uncompressed_length();
				file.buf.reset(new (std::nothrow) unsigned char [file.size]);
				if (!file.buf)
				{
					printf("%s: out of memory!\n", file.name.c_str());
				}
				else
				{
					if (zip->decompress(file.buf.get(), file.size))
						file.free();
				}

				file.listed = 0;
				if (found[i] >= MAX_FILES)
				{
					printf("%s: max of %d files exceeded\n",path,MAX_FILES);
					break;
				}
				found[i]++;
			}
		}
	}
	return 0;
}


int CLIB_DECL main(int argc,char *argv[])
{
	int err;
	int total_modes = MODE_NIB2;    /* by default, use only MODE_A, MODE_NIB1 and MODE_NIB2 */
	bool all_hashes = false;

	while (argc >= 2)
	{
		if (strcmp(argv[1], "-d") == 0)
			total_modes = TOTAL_MODES;
		else if (strcmp(argv[1], "-h") == 0)
			all_hashes = true;
		else
			break;
		argc--;
		argv++;
	}

	if (argc < 2)
	{
		printf("usage: romcmp [-d] [-h] [dir1 | zip1] [dir2 | zip2]\n");
		printf("-d enables a slower, more comprehensive comparison.\n");
		printf("-h prints hashes and sums for all files.\n");
		return 0;
	}

	{
		int found[2] = { 0, 0 };
		for (int i = 0; i < 2; i++)
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

		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < found[i]; j++)
			{
				files[i][j].checkintegrity(1 << i, all_hashes);
			}
		}

		if (argc < 3)
		{
			// find duplicates in one dir
			for (int i = 0;i < found[0];i++)
			{
				for (int j = i+1;j < found[0];j++)
				{
					for (int mode1 = 0;mode1 < total_modes;mode1++)
					{
						for (int mode2 = 0;mode2 < total_modes;mode2++)
						{
							if (files[0][i].compare(files[0][j],mode1,mode2) == 1.0f)
								printname(&files[0][i],&files[0][j],1.0,mode1,mode2);
						}
					}
				}
			}
		}
		else
		{
			// compare two dirs
			for (int i = 0;i < found[0];i++)
			{
				for (int j = 0;j < found[1];j++)
				{
					fprintf(stderr,"%2d%%\r",100*(i*found[1]+j)/(found[0]*found[1]));
					for (int mode1 = 0;mode1 < total_modes;mode1++)
					{
						for (int mode2 = 0;mode2 < total_modes;mode2++)
						{
							matchscore[i][j][mode1][mode2] = files[0][i].compare(files[1][j],mode1,mode2);
						}
					}
				}
			}
			fprintf(stderr,"   \r");

			int besti;
			do
			{
				besti = -1;
				int bestj = -1;
				float bestscore = 0.0;
				int bestmode1 = -1, bestmode2 = -1;

				for (int mode1 = 0;mode1 < total_modes;mode1++)
				{
					for (int mode2 = 0;mode2 < total_modes;mode2++)
					{
						for (int i = 0;i < found[0];i++)
						{
							for (int j = 0;j < found[1];j++)
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
					for (int j = 0;j < found[1];j++)
					{
						for (int mode2 = 0;mode2 < total_modes;mode2++)
						{
							if (matchscore[besti][j][bestmode1][mode2] < bestscore)
								matchscore[besti][j][bestmode1][mode2] = 0.0;
						}
					}
					for (int i = 0;i < found[0];i++)
					{
						for (int mode1 = 0;mode1 < total_modes;mode1++)
						{
							if (matchscore[i][bestj][mode1][bestmode2] < bestscore)
								matchscore[i][bestj][mode1][bestmode2] = 0.0;
						}
					}

					/* remove all matches using incompatible sections */
					compatiblemodes(bestmode1,&start,&end);
					for (int j = 0;j < found[1];j++)
					{
						for (int mode2 = 0;mode2 < total_modes;mode2++)
						{
							for (int mode1 = 0;mode1 < start;mode1++)
								matchscore[besti][j][mode1][mode2] = 0.0;
							for (int mode1 = end+1;mode1 < total_modes;mode1++)
								matchscore[besti][j][mode1][mode2] = 0.0;
						}
					}
					compatiblemodes(bestmode2,&start,&end);
					for (int i = 0;i < found[0];i++)
					{
						for (int mode1 = 0;mode1 < total_modes;mode1++)
						{
							for (int mode2 = 0;mode2 < start;mode2++)
								matchscore[i][bestj][mode1][mode2] = 0.0;
							for (int mode2 = end+1;mode2 < total_modes;mode2++)
								matchscore[i][bestj][mode1][mode2] = 0.0;
						}
					}
				}
			}
			while (besti != -1);


			for (int i = 0;i < found[0];i++)
			{
				if (files[0][i].listed == 0) printname(&files[0][i],nullptr,0.0,0,0);
			}
			for (int i = 0;i < found[1];i++)
			{
				if (files[1][i].listed == 0) printname(nullptr,&files[1][i],0.0,0,0);
			}
		}


		for (int i = 0;i < found[0];i++)
		{
			files[0][i].free();
		}
		for (int i = 0;i < found[1];i++)
		{
			files[1][i].free();
		}
	}

	util::archive_file::cache_clear();
	return 0;
}
