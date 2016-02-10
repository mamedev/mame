// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    split.c

    Simple file splitter/joiner with hashes.

****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "corefile.h"
#include "corestr.h"
#include "sha1.h"

#define DEFAULT_SPLIT_SIZE      100
#define MAX_PARTS               1000



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/


/*-------------------------------------------------
    compute_hash_as_string - compute an SHA1
    hash over a buffer and return a string
-------------------------------------------------*/

static void compute_hash_as_string(std::string &buffer, void *data, UINT32 length)
{
	char expanded[SHA1_DIGEST_SIZE * 2];
	UINT8 sha1digest[SHA1_DIGEST_SIZE];
	struct sha1_ctx sha1;
	int ch;

	// compute the SHA1
	sha1_init(&sha1);
	sha1_update(&sha1, length, (const UINT8 *)data);
	sha1_final(&sha1);
	sha1_digest(&sha1, sizeof(sha1digest), sha1digest);

	// expand the digest to a string
	for (ch = 0; ch < SHA1_DIGEST_SIZE; ch++)
	{
		expanded[ch * 2 + 0] = "0123456789ABCDEF"[sha1digest[ch] >> 4];
		expanded[ch * 2 + 1] = "0123456789ABCDEF"[sha1digest[ch] & 15];
	}

	// copy it to the buffer
	buffer.assign(expanded, sizeof(expanded));
}


/*-------------------------------------------------
    split_file - split a file into multiple parts
-------------------------------------------------*/

static int split_file(const char *filename, const char *basename, UINT32 splitsize)
{
	std::string outfilename, basefilename, splitfilename;
	core_file *outfile = nullptr, *infile = nullptr, *splitfile = nullptr;
	std::string computedhash;
	void *splitbuffer = nullptr;
	int index, partnum;
	UINT64 totallength;
	file_error filerr;
	int error = 1;

	// convert split size to MB
	if (splitsize > 500)
	{
		fprintf(stderr, "Fatal error: maximum split size is 500MB (even that is way huge!)\n");
		goto cleanup;
	}
	splitsize *= 1024 * 1024;

	// open the file for read
	filerr = core_fopen(filename, OPEN_FLAG_READ, &infile);
	if (filerr != FILERR_NONE)
	{
		fprintf(stderr, "Fatal error: unable to open file '%s'\n", filename);
		goto cleanup;
	}

	// get the total length
	totallength = core_fsize(infile);
	if (totallength < splitsize)
	{
		fprintf(stderr, "Fatal error: file is smaller than the split size\n");
		goto cleanup;
	}
	if ((UINT64)splitsize * MAX_PARTS < totallength)
	{
		fprintf(stderr, "Fatal error: too many splits (maximum is %d)\n", MAX_PARTS);
		goto cleanup;
	}

	// allocate a buffer for reading
	splitbuffer = malloc(splitsize);
	if (splitbuffer == nullptr)
	{
		fprintf(stderr, "Fatal error: unable to allocate memory for the split\n");
		goto cleanup;
	}

	// find the base name of the file
	basefilename.assign(basename);
	index = basefilename.find_last_of(PATH_SEPARATOR[0]);
	if (index != -1)
		basefilename.erase(0, index + 1);

	// compute the split filename
	splitfilename.assign(basename).append(".split");

	// create the split file
	filerr = core_fopen(splitfilename.c_str(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_NO_BOM, &splitfile);
	if (filerr != FILERR_NONE)
	{
		fprintf(stderr, "Fatal error: unable to create split file '%s'\n", splitfilename.c_str());
		goto cleanup;
	}

	// write the basics out
	core_fprintf(splitfile, "splitfile=%s\n", basefilename.c_str());
	core_fprintf(splitfile, "splitsize=%d\n", splitsize);

	printf("Split file is '%s'\n", splitfilename.c_str());
	printf("Splitting file %s into chunks of %dMB...\n", basefilename.c_str(), splitsize / (1024 * 1024));

	// now iterate until done
	for (partnum = 0; partnum < 1000; partnum++)
	{
		UINT32 actual, length;

		printf("Reading part %d...", partnum);

		// read as much as we can from the file
		length = core_fread(infile, splitbuffer, splitsize);
		if (length == 0)
			break;

		// hash what we have
		compute_hash_as_string(computedhash, splitbuffer, length);

		// write that info to the split file
		core_fprintf(splitfile, "hash=%s file=%s.%03d\n", computedhash.c_str(), basefilename.c_str(), partnum);

		// compute the full filename for this guy
		strprintf(outfilename,"%s.%03d", basename, partnum);

		// create it
		filerr = core_fopen(outfilename.c_str(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &outfile);
		if (filerr != FILERR_NONE)
		{
			printf("\n");
			fprintf(stderr, "Fatal error: unable to create output file '%s'\n", outfilename.c_str());
			goto cleanup;
		}

		printf(" writing %s.%03d...", basefilename.c_str(), partnum);

		// write the data
		actual = core_fwrite(outfile, splitbuffer, length);
		if (actual != length)
		{
			printf("\n");
			fprintf(stderr, "Fatal error: Error writing output file (out of space?)\n");
			goto cleanup;
		}
		core_fclose(outfile);
		outfile = nullptr;

		printf(" done\n");

		// stop if this is the end
		if (length < splitsize)
			break;
	}
	printf("File split successfully\n");

	// if we get here, clear the errors
	error = 0;

cleanup:
	if (splitfile != nullptr)
	{
		core_fclose(splitfile);
		if (error != 0)
			remove(splitfilename.c_str());
	}
	if (infile != nullptr)
		core_fclose(infile);
	if (outfile != nullptr)
	{
		core_fclose(outfile);
		if (error != 0)
			remove(outfilename.c_str());
	}
	if (splitbuffer != nullptr)
		free(splitbuffer);
	return error;
}


/*-------------------------------------------------
    join_file - rejoin a file from its split
    parts
-------------------------------------------------*/

static int join_file(const char *filename, const char *outname, int write_output)
{
	std::string expectedhash, computedhash;
	std::string outfilename, infilename;
	std::string basepath;
	core_file *outfile = nullptr, *infile = nullptr, *splitfile = nullptr;
	void *splitbuffer = nullptr;
	file_error filerr;
	UINT32 splitsize;
	char buffer[256];
	int error = 1;
	int index;

	// open the file for read
	filerr = core_fopen(filename, OPEN_FLAG_READ, &splitfile);
	if (filerr != FILERR_NONE)
	{
		fprintf(stderr, "Fatal error: unable to open file '%s'\n", filename);
		goto cleanup;
	}

	// read the first line and verify this is a split file
	if (!core_fgets(buffer, sizeof(buffer), splitfile) || strncmp(buffer, "splitfile=", 10) != 0)
	{
		fprintf(stderr, "Fatal error: corrupt or incomplete split file at line:\n%s\n", buffer);
		goto cleanup;
	}
	outfilename.assign(buffer + 10);
	strtrimspace(outfilename);

	// compute the base path
	basepath.assign(filename);
	index = basepath.find_last_of(PATH_SEPARATOR[0]);
	if (index != -1)
		basepath.erase(index + 1);
	else
		basepath.clear();

	// override the output filename if specified, otherwise prepend the path
	if (outname != nullptr)
		outfilename.assign(outname);
	else
		outfilename.insert(0, basepath);

	// read the split size
	if (!core_fgets(buffer, sizeof(buffer), splitfile) || sscanf(buffer, "splitsize=%d", &splitsize) != 1)
	{
		fprintf(stderr, "Fatal error: corrupt or incomplete split file at line:\n%s\n", buffer);
		goto cleanup;
	}

	// attempt to open the new file
	if (write_output)
	{
		// don't overwrite the original!
		filerr = core_fopen(outfilename.c_str(), OPEN_FLAG_READ, &outfile);
		if (filerr == FILERR_NONE)
		{
			core_fclose(outfile);
			outfile = nullptr;
			fprintf(stderr, "Fatal error: output file '%s' already exists\n", outfilename.c_str());
			goto cleanup;
		}

		// open the output for write
		filerr = core_fopen(outfilename.c_str(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &outfile);
		if (filerr != FILERR_NONE)
		{
			fprintf(stderr, "Fatal error: unable to create file '%s'\n", outfilename.c_str());
			goto cleanup;
		}
	}

	printf("%s file '%s'...\n", write_output ? "Joining" : "Verifying", outfilename.c_str());

	// now iterate through each file
	while (core_fgets(buffer, sizeof(buffer), splitfile))
	{
		UINT32 length, actual;

		// make sure the hash and filename are in the right place
		if (strncmp(buffer, "hash=", 5) != 0 || strncmp(buffer + 5 + SHA1_DIGEST_SIZE * 2, " file=", 6) != 0)
		{
			fprintf(stderr, "Fatal error: corrupt or incomplete split file at line:\n%s\n", buffer);
			goto cleanup;
		}
		expectedhash.assign(buffer + 5, SHA1_DIGEST_SIZE * 2);
		infilename.assign(buffer + 5 + SHA1_DIGEST_SIZE * 2 + 6);
		strtrimspace(infilename);

		printf("  Reading file '%s'...", infilename.c_str());

		// read the file's contents
		infilename.insert(0, basepath);
		filerr = core_fload(infilename.c_str(), &splitbuffer, &length);
		if (filerr != FILERR_NONE)
		{
			printf("\n");
			fprintf(stderr, "Fatal error: unable to load file '%s'\n", infilename.c_str());
			goto cleanup;
		}

		// hash the contents
		compute_hash_as_string(computedhash, splitbuffer, length);

		// compare
		if (computedhash.compare(expectedhash)!=0)
		{
			printf("\n");
			fprintf(stderr, "Fatal error: file '%s' has incorrect hash\n  Expected: %s\n  Computed: %s\n", infilename.c_str(), expectedhash.c_str(), computedhash.c_str());
			goto cleanup;
		}

		// append to the file
		if (write_output)
		{
			printf(" writing...");

			actual = core_fwrite(outfile, splitbuffer, length);
			if (actual != length)
			{
				printf("\n");
				fprintf(stderr, "Fatal error: Error writing output file (out of space?)\n");
				goto cleanup;
			}

			printf(" done\n");
		}
		else
			printf(" verified\n");

		// release allocated memory
		osd_free(splitbuffer);
		splitbuffer = nullptr;
	}
	if (write_output)
		printf("File re-created successfully\n");
	else
		printf("File verified successfully\n");

	// if we get here, clear the errors
	error = 0;

cleanup:
	if (splitfile != nullptr)
		core_fclose(splitfile);
	if (infile != nullptr)
		core_fclose(infile);
	if (outfile != nullptr)
	{
		core_fclose(outfile);
		if (error != 0)
			remove(outfilename.c_str());
	}
	if (splitbuffer != nullptr)
		osd_free(splitbuffer);
	return error;
}


/*-------------------------------------------------
    main - primary entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	int result;

	/* skip if no command */
	if (argc < 2)
		goto usage;

	/* split command */
	if (core_stricmp(argv[1], "-split") == 0)
	{
		if (argc != 4 && argc != 5)
			goto usage;
		result = split_file(argv[2], argv[3], (argc < 5) ? DEFAULT_SPLIT_SIZE : atoi(argv[4]));
	}

	/* join command */
	else if (core_stricmp(argv[1], "-join") == 0)
	{
		if (argc != 3 && argc != 4)
			goto usage;
		result = join_file(argv[2], (argc >= 4) ? argv[3] : nullptr, TRUE);
	}

	/* verify command */
	else if (core_stricmp(argv[1], "-verify") == 0)
	{
		if (argc != 3)
			goto usage;
		result = join_file(argv[2], nullptr, FALSE);
	}
	else
		goto usage;

	return result;

usage:
	fprintf(stderr,
		"Usage:\n"
		"  split -split <bigfile> <basename> [<size>] -- split file into parts\n"
		"  split -join <splitfile> [<outputfile>] -- join file parts into original file\n"
		"  split -verify <splitfile> -- verify a split file\n"
		"\n"
		"Where:\n"
		"  <bigfile> is the large file you wish to split\n"
		"  <basename> is the base path and name to assign to the split files\n"
		"  <size> is the optional split size, in MB (100MB is the default)\n"
		"  <splitfile> is the name of the <basename>.split generated with -split\n"
		"  <outputfile> is the name of the file to output (defaults to original name)\n"
	);
	return 0;
}
