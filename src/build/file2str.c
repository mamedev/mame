/***************************************************************************

    file2str.c

    Simple file to string converter.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include <stdio.h>
#include "osdcore.h"


/*-------------------------------------------------
    main - primary entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	const char *srcfile, *dstfile, *varname, *type;
	FILE *src, *dst;
	UINT8 *buffer;
	int bytes, offs;
	int terminate = 1;

	/* needs at least three arguments */
	if (argc < 4)
	{
		fprintf(stderr,
			"Usage:\n"
			"  laytostr <source.lay> <output.h> <varname> [<type>]\n"
			"\n"
			"The default <type> is char, with an assumed NULL terminator\n"
		);
		return 0;
	}

	/* extract arguments */
	srcfile = argv[1];
	dstfile = argv[2];
	varname = argv[3];
	type = (argc >= 5) ? argv[4] : "char";
	if (argc >= 5)
		terminate = 0;

	/* open source file */
	src = fopen(srcfile, "rb");
	if (src == NULL)
	{
		fprintf(stderr, "Unable to open source file '%s'\n", srcfile);
		return 1;
	}

	/* determine file size */
	fseek(src, 0, SEEK_END);
	bytes = ftell(src);
	fseek(src, 0, SEEK_SET);

	/* allocate memory */
	buffer = malloc(bytes + 1);
	if (buffer == NULL)
	{
		fprintf(stderr, "Out of memory allocating %d byte buffer\n", bytes);
		return 1;
	}

	/* read the source file */
	fread(buffer, 1, bytes, src);
	buffer[bytes] = 0;
	fclose(src);

	/* open dest file */
	dst = fopen(dstfile, "w");
	if (dst == NULL)
	{
		fprintf(stderr, "Unable to open output file '%s'\n", dstfile);
		return 1;
	}

	/* write the initial header */
	fprintf(dst, "const %s %s[] =\n{\n\t", type, varname);

	/* write out the data */
	for (offs = 0; offs < bytes + terminate; offs++)
	{
		fprintf(dst, "0x%02x%s", buffer[offs], (offs != bytes + terminate - 1) ? "," : "");
		if (offs % 16 == 15)
			fprintf(dst, "\n\t");
	}
	fprintf(dst, "\n};\n");

	/* close the files */
	free(buffer);
	fclose(dst);
	return 0;
}
