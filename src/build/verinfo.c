//============================================================
//
//  verinfo.c - Version resource emitter code
//
//  Copyright Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <stdlib.h>

typedef unsigned char UINT8;

#define ARRAY_LENGTH(x)		(sizeof(x) / sizeof(x[0]))
#define BUILD_WINDOWS		(0)
#define BUILD_WINUI			(1)
#define BUILD_MESS			(2)

//============================================================
//  TYPE DEFINITIONS
//============================================================

struct version_info
{
	int version_major;
	int version_minor;
	int version_build;
	int version_subbuild;
	const char *version_string;
	const char *author;
	const char *comments;
	const char *company_name;
	const char *file_description;
	const char *internal_name;
	const char *legal_copyright;
	const char *original_filename;
	const char *product_name;
};



//============================================================
// STATIC
//============================================================

static int build;


//============================================================
//  emit_version_info
//============================================================

static void emit_version_info(const version_info *v)
{
	printf("VS_VERSION_INFO VERSIONINFO\n");
	printf("\tFILEVERSION %d,%d,%d,%d\n", v->version_major, v->version_minor, v->version_build, v->version_subbuild);
	printf("\tPRODUCTVERSION %d,%d,%d,%d\n", v->version_major, v->version_minor, v->version_build, v->version_subbuild);
	printf("\tFILEFLAGSMASK 0x3fL\n");
#ifdef MAME_DEBUG
	if (v->version_build == 0)
		printf("\tFILEFLAGS VS_FF_DEBUG\n");
	else
		printf("\tFILEFLAGS VS_FF_PRERELEASE | VS_FF_DEBUG\n");
#else
	if (v->version_build == 0)
		printf("\tFILEFLAGS 0x0L\n");
	else
		printf("\tFILEFLAGS VS_FF_PRERELEASE\n");
#endif
	printf("\tFILEOS VOS_NT_WINDOWS32\n");
	printf("\tFILETYPE VFT_APP\n");
	printf("\tFILESUBTYPE VFT2_UNKNOWN\n");
	printf("BEGIN\n");
	printf("\tBLOCK \"StringFileInfo\"\n");
	printf("\tBEGIN\n");
	printf("#ifdef UNICODE\n");
	printf("\t\tBLOCK \"040904b0\"\n");
	printf("#else\n");
	printf("\t\tBLOCK \"040904E4\"\n");
	printf("#endif\n");
	printf("\t\tBEGIN\n");
	if (v->author != NULL)
		printf("\t\t\tVALUE \"Author\", \"%s\\0\"\n", v->author);
	if (v->comments != NULL)
		printf("\t\t\tVALUE \"Comments\", \"%s\\0\"\n", v->comments);
	if (v->company_name != NULL)
		printf("\t\t\tVALUE \"CompanyName\", \"%s\\0\"\n", v->company_name);
	if (v->file_description != NULL)
		printf("\t\t\tVALUE \"FileDescription\", \"%s\\0\"\n", v->file_description);
	printf("\t\t\tVALUE \"FileVersion\", \"%d, %d, %d, %d\\0\"\n", v->version_major, v->version_minor, v->version_build, v->version_subbuild);
	if (v->internal_name != NULL)
		printf("\t\t\tVALUE \"InternalName\", \"%s\\0\"\n", v->internal_name);
	if (v->legal_copyright != NULL)
		printf("\t\t\tVALUE \"LegalCopyright\", \"%s\\0\"\n", v->legal_copyright);
	if (v->original_filename != NULL)
		printf("\t\t\tVALUE \"OriginalFilename\", \"%s\\0\"\n", v->original_filename);
	if (v->product_name != NULL)
		printf("\t\t\tVALUE \"ProductName\", \"%s\\0\"\n", v->product_name);
	printf("\t\t\tVALUE \"ProductVersion\", \"%s\\0\"\n", v->version_string);
	printf("\t\tEND\n");
	printf("\tEND\n");
	printf("\tBLOCK \"VarFileInfo\"\n");
	printf("\tBEGIN\n");
	printf("#ifdef UNICODE\n");
	printf("\t\tVALUE \"Translation\", 0x409, 1200\n");
	printf("#else\n");
	printf("\t\tVALUE \"Translation\", 0x409, 1252\n");
	printf("#endif\n");
	printf("\tEND\n");
	printf("END\n");
}



//============================================================
//  parse_version_digit
//============================================================

static int parse_version_digit(const char *str, int *position)
{
	int value = 0;

	while (str[*position] != 0 && !isspace((UINT8)str[*position]) && !isdigit((UINT8)str[*position]))
		(*position)++;

	if (str[*position] != 0 && isdigit((UINT8)str[*position]))
	{
		sscanf(&str[*position], "%d", &value);
		while (isdigit((UINT8)str[*position]))
			(*position)++;
	}
	return value;
}



//============================================================
//  parse_version
//============================================================

static int parse_version(char *str, int *version_major, int *version_minor, int *version_micro, const char **version_string)
{
	char *version;
	int position = 0;

	// find the version string
	version = strstr(str, "build_version");
	if (version != NULL)
		version = strchr(version, '"');
	if (version == NULL)
	{
		fprintf(stderr, "Unable to find build_version string\n");
		return 1;
	}
	version++;
	*strchr(version, ' ') = 0;

	*version_string = version;
	*version_major = parse_version_digit(version, &position);
	*version_minor = parse_version_digit(version, &position);
	*version_micro = parse_version_digit(version, &position);
	return 0;
}



//============================================================
//  main
//============================================================
static int usage(char *me)
{
	fprintf(stderr, "Usage: %s [-b windows|winui|mess] <filename>\n", me);
	return 1;
}


//============================================================
//  main
//============================================================

int main(int argc, char *argv[])
{
	version_info v;
	char legal_copyright[512];
	char *buffer;
	size_t size;
	int opt;
	FILE *f;

	memset(&v, 0, sizeof(v));
	build = BUILD_WINDOWS;

	// validate parameters
	opt = 1;
	while (opt < argc && *argv[opt] == '-')
	{
		if (!strcmp(argv[opt], "-b"))
		{
			char *p = argv[++opt];
			if (!strcmp(p,"windows"))
				build = BUILD_WINDOWS;
			else if (!strcmp(p,"winui"))
				build = BUILD_WINUI;
			else if (!strcmp(p,"mess"))
				build = BUILD_MESS;
			else
				return usage(argv[0]);
		}
		else
			return usage(argv[0]);
		opt++;
	}

	if (opt != argc-1 )
	{
		return usage(argv[0]);
	}

	// open the file
	f = fopen(argv[argc-1], "rb");
	if (f == NULL)
	{
		fprintf(stderr, "Error opening file %s\n", argv[argc-1]);
		return 1;
	}

	// get the file size
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	// allocate a buffer
	buffer = (char *)malloc(size + 1);
	if (buffer == NULL)
	{
		fclose(f);
		fprintf(stderr, "Error allocating %ld bytes\n", (long) size + 1);
		return 1;
	}

	// read the file contents and NULL-terminate
	fread(buffer, 1, size, f);
	fclose(f);
	buffer[size] = 0;

	// parse out version string
	if (parse_version(buffer, &v.version_major, &v.version_minor, &v.version_build, &v.version_string))
	{
		fprintf(stderr, "Error parsing version from '%s'\n", buffer);
		free(buffer);
		return 1;
	}

	if (build == BUILD_MESS)
	{
		// MESS
		v.author = "MESS Team";
		v.comments = "Multi Emulation Super System";
		v.company_name = "MESS Team";
		v.file_description = "Multi Emulation Super System";
		v.internal_name = "MESS";
		v.original_filename = "MESS";
		v.product_name = "MESS";
	}
	else if (build == BUILD_WINUI)
	{
		// MAMEUI
		v.author = "Christopher Kirmse and the MAMEUI team";
		v.comments = "Multiple Arcade Machine Emulator with GUI";
		v.company_name = "MAME Team";
		v.file_description = "Multiple Arcade Machine Emulator with GUI";
		v.internal_name = "MAMEUI";
		v.original_filename = "MAMEUI";
		v.product_name = "MAMEUI";
	}
	else
	{
		// MAME
		v.author = "Nicola Salmoria and the MAME Team";
		v.comments = "Multiple Arcade Machine Emulator";
		v.company_name = "MAME Team";
		v.file_description = "Multiple Arcade Machine Emulator";
		v.internal_name = "MAME";
		v.original_filename = "MAME";
		v.product_name = "MAME";
	}

	// build legal_copyright string
	v.legal_copyright = legal_copyright;
	snprintf(legal_copyright, ARRAY_LENGTH(legal_copyright), "Copyright Nicola Salmoria and the MAME team");

	// emit the info
	emit_version_info(&v);

	free(buffer);
	return 0;
}
