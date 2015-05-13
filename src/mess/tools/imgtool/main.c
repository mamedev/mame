// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    main.c

    Imgtool command line front end

***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "imgtool.h"
#include "main.h"
#include "modules.h"
/* ---------------------------------------------------------------------- */

static void writeusage(FILE *f, int write_word_usage, const struct command *c, char *argv[])
{
	fprintf(f, "%s %s %s %s\n",
		(write_word_usage ? "Usage:" : "      "),
		imgtool_basename(argv[0]),
		c->name,
		c->usage ? c->usage : "");
}



/* ----------------------------------------------------------------------- */

static int parse_options(int argc, char *argv[], int minunnamed, int maxunnamed,
	option_resolution *resolution, filter_getinfoproc *filter, const char **fork)
{
	int i;
	int lastunnamed = 0;
	char *s;
	char *name = NULL;
	char *value = NULL;
	optreserr_t oerr;
	static char buf[256];

	if (filter)
		*filter = NULL;
	if (fork)
		*fork = NULL;

	for (i = 0; i < argc; i++)
	{
		/* Named or unamed arg */
		if ((argv[i][0] != '-') || (argv[i][1] != '-'))
		{
			/* Unnamed */
			if (i >= maxunnamed)
				goto error; /* Too many unnamed */
			lastunnamed = i + 1;
		}
		else
		{
			/* Named */
			name = argv[i] + 2;
			s = strchr(name, '=');
			if (!s)
				goto error;
			*s = 0;
			value = s + 1;

			if (!strcmp(name, "filter"))
			{
				/* filter option */
				if (!filter)
					goto error; /* this command doesn't use filters */
				if (*filter)
					goto optionalreadyspecified;
				*filter = filter_lookup(value);
				if (!(*filter))
					goto filternotfound;

			}
			else if (!strcmp(name, "fork"))
			{
				/* fork option */
				if (!fork)
					goto error; /* this command doesn't use filters */
				if (*fork)
					goto optionalreadyspecified;

				snprintf(buf, ARRAY_LENGTH(buf), "%s", value);
				*fork = buf;
			}
			else
			{
				/* Other named option */
				if (i < minunnamed)
					goto error; /* Too few unnamed */

				oerr = option_resolution_add_param(resolution, name, value);
				if (oerr)
					goto opterror;
			}
		}
	}
	return lastunnamed;

filternotfound:
	fprintf(stderr, "%s: Unknown filter type\n", value);
	return -1;

optionalreadyspecified:
	fprintf(stderr, "Cannot specify multiple %ss\n", name);
	return -1;

opterror:
	fprintf(stderr, "%s: %s\n", name, option_resolution_error_string(oerr));
	return -1;

error:
	fprintf(stderr, "%s: Unrecognized option\n", argv[i]);
	return -1;
}



void reporterror(imgtoolerr_t err, const struct command *c, const char *format, const char *imagename,
	const char *filename, const char *newname, option_resolution *opts)
{
	const char *src = "imgtool";
	const char *err_name;

	err_name = imgtool_error(err);

	switch(ERRORSOURCE(err)) {
	case IMGTOOLERR_SRC_MODULE:
		src = format;
		break;
	case IMGTOOLERR_SRC_FUNCTIONALITY:
		src = c->name;
		break;
	case IMGTOOLERR_SRC_IMAGEFILE:
		src = imagename;
		break;
	case IMGTOOLERR_SRC_FILEONIMAGE:
		src = filename;
		break;
	case IMGTOOLERR_SRC_NATIVEFILE:
		src = newname ? newname : filename;
		break;
	}
	fflush(stdout);
	fflush(stderr);

	if (!src)
		src = c->name;
	fprintf(stderr, "%s: %s\n", src, err_name);
}



static const char *interpret_filename(const char *filename)
{
	if (!strcmp(filename, "??BOOT??")
			|| !strcmp(filename, "\'??BOOT??\'")
			|| !strcmp(filename, "\"??BOOT??\""))
		filename = FILENAME_BOOTBLOCK;
	return filename;
}



/* ----------------------------------------------------------------------- */

static int cmd_dir(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	int total_count, total_size, freespace_err;
	UINT64 freespace;
	imgtool_image *image = NULL;
	imgtool_partition *partition = NULL;
	imgtool_directory *imgenum = NULL;
	imgtool_dirent ent;
	char buf[512];
	char last_modified[19];
	const char *path;
	int partition_index = 0;

	/* attempt to open image */
	err = imgtool_image_open_byname(argv[0], argv[1], OSD_FOPEN_READ, &image);
	if (err)
		goto done;

	/* attempt to open partition */
	err = imgtool_partition_open(image, partition_index, &partition);
	if (err)
		goto done;

	path = argc > 2 ? argv[2] : NULL;

	err = imgtool_directory_open(partition, path, &imgenum);
	if (err)
		goto done;

	memset(&ent, 0, sizeof(ent));
	last_modified[0] = '\0';
	total_count = 0;
	total_size = 0;

	fprintf(stdout, "Contents of %s:%s\n", argv[1], path ? path : "");

	imgtool_image_info(image, buf, sizeof(buf));
	if (buf[0])
		fprintf(stdout, "%s\n", buf);
	fprintf(stdout, "------------------------------  --------  ---------------  ------------------\n");

	while (((err = imgtool_directory_get_next(imgenum, &ent)) == 0) && !ent.eof)
	{
		if (ent.directory)
			snprintf(buf, sizeof(buf), "<DIR>");
		else
			snprintf(buf, sizeof(buf), "%u", (unsigned int) ent.filesize);

		if (ent.lastmodified_time != 0)
			strftime(last_modified, sizeof(last_modified), "%d-%b-%y %H:%M:%S",
				localtime(&ent.lastmodified_time));

		if (ent.hardlink)
			strcat(ent.filename, " <hl>");

		fprintf(stdout, "%-30s  %8s  %15s  %18s\n", ent.filename, buf, ent.attr, last_modified);

		if (ent.softlink && ent.softlink[0] != '\0')
			fprintf(stdout, "-> %s\n", ent.softlink);

		if (ent.comment && ent.comment[0] != '\0')
			fprintf(stdout, ": %s\n", ent.comment);

		total_count++;
		total_size += ent.filesize;

		memset(&ent, 0, sizeof(ent));
	}

	freespace_err = imgtool_partition_get_free_space(partition, &freespace);

	if (err)
		goto done;

	fprintf(stdout, "------------------------  ------ ---------------\n");
	fprintf(stdout, "%8i File(s)        %8i bytes\n", total_count, total_size);
	if (!freespace_err)
		fprintf(stdout, "                        %8u bytes free\n", (unsigned int) freespace);

done:
	if (imgenum)
		imgtool_directory_close(imgenum);
	if (partition)
		imgtool_partition_close(partition);
	if (image)
		imgtool_image_close(image);
	if (err)
		reporterror(err, c, argv[0], argv[1], NULL, NULL, NULL);
	return err ? -1 : 0;
}



static int cmd_get(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool_image *image = NULL;
	imgtool_partition *partition = NULL;
	const char *filename;
	char *new_filename;
	int unnamedargs = 0;
	filter_getinfoproc filter;
	const char *fork;
	int partition_index = 0;

	err = imgtool_image_open_byname(argv[0], argv[1], OSD_FOPEN_READ, &image);
	if (err)
		goto done;

	err = imgtool_partition_open(image, partition_index, &partition);
	if (err)
		goto done;

	filename = interpret_filename(argv[2]);

	unnamedargs = parse_options(argc, argv, 3, 4, NULL, &filter, &fork);
	if (unnamedargs < 0)
		goto done;

	new_filename = (unnamedargs == 4) ? argv[3] : NULL;

	err = imgtool_partition_get_file(partition, filename, fork, new_filename, filter);
	if (err)
		goto done;

	err = IMGTOOLERR_SUCCESS;

done:
	if (err)
		reporterror(err, c, argv[0], argv[1], argv[2], argv[3], NULL);
	if (partition)
		imgtool_partition_close(partition);
	if (image)
		imgtool_image_close(image);
	return (err || (unnamedargs < 0)) ? -1 : 0;
}



static int cmd_put(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	int i;
	imgtool_image *image = NULL;
	imgtool_partition *partition = NULL;
	const char *filename = NULL;
	int unnamedargs;
	filter_getinfoproc filter;
	const imgtool_module *module;
	option_resolution *resolution = NULL;
	const char *fork;
	const char *new_filename;
	char **filename_list;
	int filename_count;
	int partition_index = 0;
	const option_guide *writefile_optguide;
	const char *writefile_optspec;

	module = imgtool_find_module(argv[0]);
	if (!module)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_MODULENOTFOUND | IMGTOOLERR_SRC_MODULE);
		goto done;
	}

	/* ugh I hate the way this function is set up, this is because the
	 * arguments depend on the partition; something that requires some
	 * rudimentary parsing */
	if (argc >= 2)
	{
		/* open up the image */
		err = imgtool_image_open(module, argv[1], OSD_FOPEN_RW, &image);
		if (err)
			goto done;

		/* open up the partition */
		err = imgtool_partition_open(image, partition_index, &partition);
		if (err)
			goto done;

		writefile_optguide = (const option_guide *) imgtool_partition_get_info_ptr(partition, IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE);
		writefile_optspec = (const char *)imgtool_partition_get_info_ptr(partition, IMGTOOLINFO_STR_WRITEFILE_OPTSPEC);

		if (writefile_optguide && writefile_optspec)
		{
			resolution = option_resolution_create(writefile_optguide, writefile_optspec);
			if (!resolution)
			{
				err = IMGTOOLERR_OUTOFMEMORY;
				goto done;
			}
		}
	}

	unnamedargs = parse_options(argc, argv, 4, 0xffff, resolution, &filter, &fork);
	if (unnamedargs < 0)
		return -1;

	/* pick out which args are filenames, and which one is the destination */
	new_filename = interpret_filename(argv[unnamedargs - 1]);
	filename_list = &argv[2];
	filename_count = unnamedargs - 3;

	/* loop through the filenames, and put them */
	for (i = 0; i < filename_count; i++)
	{
		filename = filename_list[i];
		printf("Putting file '%s'...\n", filename);
		err = imgtool_partition_put_file(partition, new_filename, fork, filename, resolution, filter);
		if (err)
			goto done;
	}

done:
	if (partition)
		imgtool_partition_close(partition);
	if (image)
		imgtool_image_close(image);
	if (resolution)
		option_resolution_close(resolution);
	if (err)
		reporterror(err, c, argv[0], argv[1], filename, NULL, resolution);
	return err ? -1 : 0;
}



static int cmd_getall(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool_image *image = NULL;
	imgtool_partition *partition = NULL;
	imgtool_directory *imgenum = NULL;
	imgtool_dirent ent;
	filter_getinfoproc filter;
	int unnamedargs;
	const char *path = NULL;
	int arg;
	int partition_index = 0;

	err = imgtool_image_open_byname(argv[0], argv[1], OSD_FOPEN_READ, &image);
	if (err)
		goto done;

	err = imgtool_partition_open(image, partition_index, &partition);
	if (err)
		goto done;

	arg = 2;
	if ((argc > 2) && (argv[2][0] != '-'))
	{
		path = argv[arg++];
	}

	unnamedargs = parse_options(argc, argv, arg, arg, NULL, &filter, NULL);
	if (unnamedargs < 0)
		goto done;

	err = imgtool_directory_open(partition, path, &imgenum);
	if (err)
		goto done;

	memset(&ent, 0, sizeof(ent));

	while (((err = imgtool_directory_get_next(imgenum, &ent)) == 0) && !ent.eof)
	{
		fprintf(stdout, "Retrieving %s (%u bytes)\n", ent.filename, (unsigned int) ent.filesize);

		err = imgtool_partition_get_file(partition, ent.filename, NULL, NULL, filter);
		if (err)
			goto done;
	}

done:
	if (imgenum)
		imgtool_directory_close(imgenum);
	if (partition)
		imgtool_partition_close(partition);
	if (image)
		imgtool_image_close(image);
	if (err)
		reporterror(err, c, argv[0], argv[1], NULL, NULL, NULL);
	return err ? -1 : 0;
}



static int cmd_del(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool_image *image = NULL;
	imgtool_partition *partition = NULL;
	int partition_index = 0;

	err = imgtool_image_open_byname(argv[0], argv[1], OSD_FOPEN_RW, &image);
	if (err)
		goto done;

	err = imgtool_partition_open(image, partition_index, &partition);
	if (err)
		goto done;

	err = imgtool_partition_delete_file(partition, argv[2]);
	if (err)
		goto done;

done:
	if (partition)
		imgtool_partition_close(partition);
	if (image)
		imgtool_image_close(image);
	if (err)
		reporterror(err, c, argv[0], argv[1], argv[2], NULL, NULL);
	return err ? -1 : 0;
}



static int cmd_mkdir(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool_image *image = NULL;
	imgtool_partition *partition = NULL;
	int partition_index = 0;

	err = imgtool_image_open_byname(argv[0], argv[1], OSD_FOPEN_RW, &image);
	if (err)
		goto done;

	err = imgtool_partition_open(image, partition_index, &partition);
	if (err)
		goto done;

	err = imgtool_partition_create_directory(partition, argv[2]);
	if (err)
		goto done;

done:
	if (partition)
		imgtool_partition_close(partition);
	if (image)
		imgtool_image_close(image);
	if (err)
		reporterror(err, c, argv[0], argv[1], argv[2], NULL, NULL);
	return err ? -1 : 0;
}



static int cmd_rmdir(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool_image *image = NULL;
	imgtool_partition *partition = NULL;
	int partition_index = 0;

	err = imgtool_image_open_byname(argv[0], argv[1], OSD_FOPEN_RW, &image);
	if (err)
		goto done;

	err = imgtool_partition_open(image, partition_index, &partition);
	if (err)
		goto done;

	err = imgtool_partition_delete_directory(partition, argv[2]);
	if (err)
		goto done;

done:
	if (partition)
		imgtool_partition_close(partition);
	if (image)
		imgtool_image_close(image);
	if (err)
		reporterror(err, c, argv[0], argv[1], argv[2], NULL, NULL);
	return err ? -1 : 0;
}



static int cmd_identify(const struct command *c, int argc, char *argv[])
{
	imgtool_module *modules[128];
	imgtoolerr_t err;
	int i;

	err = imgtool_identify_file(argv[0], modules, ARRAY_LENGTH(modules));
	if (err)
		goto error;

	for (i = 0; modules[i]; i++)
	{
		printf("%.16s %s\n", modules[i]->name, modules[i]->description);
	}

	return 0;

error:
	reporterror(err, c, NULL, argv[0], NULL, NULL, 0);
	return -1;
}



static int cmd_create(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	int unnamedargs;
	const imgtool_module *module;
	option_resolution *resolution = NULL;

	module = imgtool_find_module(argv[0]);
	if (!module)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_MODULENOTFOUND | IMGTOOLERR_SRC_MODULE);
		goto error;
	}

	if (module->createimage_optguide && module->createimage_optspec)
	{
		resolution = option_resolution_create(module->createimage_optguide, module->createimage_optspec);
		if (!resolution)
		{
			err = IMGTOOLERR_OUTOFMEMORY;
			goto error;
		}
	}

	unnamedargs = parse_options(argc, argv, 2, 3, resolution, NULL, NULL);
	if (unnamedargs < 0)
		return -1;

	err = imgtool_image_create(module, argv[1], resolution, NULL);
	if (err)
		goto error;

	if (resolution)
		option_resolution_close(resolution);
	return 0;

error:
	if (resolution)
		option_resolution_close(resolution);
	reporterror(err, c, argv[0], argv[1], NULL, NULL, 0);
	return -1;
}



static int cmd_readsector(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool_image *img;
	imgtool_stream *stream = NULL;
	dynamic_buffer buffer;
	UINT32 size, track, head, sector;

	/* attempt to open image */
	err = imgtool_image_open_byname(argv[0], argv[1], OSD_FOPEN_READ, &img);
	if (err)
		goto done;

	track = atoi(argv[2]);
	head = atoi(argv[3]);
	sector = atoi(argv[4]);

	err = imgtool_image_get_sector_size(img, track, head, sector, &size);
	if (err)
		goto done;

	buffer.resize(size);

	err = imgtool_image_read_sector(img, track, head, sector, &buffer[0], size);
	if (err)
		goto done;


	stream = stream_open(argv[5], OSD_FOPEN_WRITE);
	if (!stream)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_FILENOTFOUND | IMGTOOLERR_SRC_NATIVEFILE);
		goto done;
	}

	stream_write(stream, &buffer[0], size);

done:
	if (stream)
		stream_close(stream);
	if (err)
		reporterror(err, c, argv[0], argv[1], NULL, NULL, 0);
	return err ? -1 : 0;
}



static int cmd_writesector(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool_image *img;
	imgtool_stream *stream = NULL;
	dynamic_buffer buffer;
	UINT32 size, track, head, sector;

	/* attempt to open image */
	err = imgtool_image_open_byname(argv[0], argv[1], OSD_FOPEN_RW, &img);
	if (err)
		goto done;

	track = atoi(argv[2]);
	head = atoi(argv[3]);
	sector = atoi(argv[4]);

	stream = stream_open(argv[5], OSD_FOPEN_READ);
	if (!stream)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_FILENOTFOUND | IMGTOOLERR_SRC_NATIVEFILE);
		goto done;
	}

	size = (UINT32) stream_size(stream);

	buffer.resize(size);

	stream_read(stream, &buffer[0], size);

	err = imgtool_image_write_sector(img, track, head, sector, &buffer[0], size);
	if (err)
		goto done;

done:
	if (stream)
		stream_close(stream);
	if (err)
		reporterror(err, c, argv[0], argv[1], NULL, NULL, 0);
	return err ? -1 : 0;
}



static int cmd_listformats(const struct command *c, int argc, char *argv[])
{
	const imgtool_module *mod;

	fprintf(stdout, "Image formats supported by imgtool:\n\n");

	mod = imgtool_find_module(NULL);
	while(mod)
	{
		fprintf(stdout, "  %-25s%s\n", mod->name, mod->description);
		mod = mod->next;
	}

	return 0;
}



static int cmd_listfilters(const struct command *c, int argc, char *argv[])
{
	int i;

	fprintf(stdout, "Filters supported by imgtool:\n\n");

	for (i = 0; filters[i]; i++)
	{
		fprintf(stdout, "  %-11s%s\n",
			filter_get_info_string(filters[i], FILTINFO_STR_NAME),
			filter_get_info_string(filters[i], FILTINFO_STR_HUMANNAME));
	}

	return 0;
}

static void listoptions(const option_guide *opt_guide, const char *opt_spec)
{
	char opt_name[32];
	const char *opt_desc;
	struct OptionRange range[32];
	char range_buffer[512];
	char buf[32];
	int i;

	assert(opt_guide);

	fprintf(stdout, "Option           Allowed values                 Description\n");
	fprintf(stdout, "---------------- ------------------------------ -----------\n");

	while(opt_guide->option_type != OPTIONTYPE_END)
	{
		range_buffer[0] = 0;
		snprintf(opt_name, ARRAY_LENGTH(opt_name), "--%s", opt_guide->identifier);
		opt_desc = opt_guide->display_name;

		/* is this option relevant? */
		if (!strchr(opt_spec, opt_guide->parameter))
		{
			opt_guide++;
			continue;
		}

		switch(opt_guide->option_type) {
		case OPTIONTYPE_INT:
			option_resolution_listranges(opt_spec, opt_guide->parameter,
				range, ARRAY_LENGTH(range));

			for (i = 0; range[i].max >= 0; i++)
			{
				if (range[i].min == range[i].max)
					snprintf(buf, ARRAY_LENGTH(buf), "%i", range[i].min);
				else
					snprintf(buf, ARRAY_LENGTH(buf), "%i-%i", range[i].min, range[i].max);

				if (i > 0)
					strncatz(range_buffer, "/", sizeof(range_buffer));
				strncatz(range_buffer, buf, sizeof(range_buffer));
			}
			break;

		case OPTIONTYPE_ENUM_BEGIN:
			i = 0;
			while(opt_guide[1].option_type == OPTIONTYPE_ENUM_VALUE)
			{
				if (i > 0)
					strncatz(range_buffer, "/", sizeof(range_buffer));
				strncatz(range_buffer, opt_guide[1].identifier, sizeof(range_buffer));

				opt_guide++;
				i++;
			}
			break;

		case OPTIONTYPE_STRING:
			snprintf(range_buffer, sizeof(range_buffer), "(string)");
			break;
		default:
			assert(0);
			break;
		}

		fprintf(stdout, "%16s %-30s %s\n",
			opt_name,
			range_buffer,
			opt_desc);
		opt_guide++;
	}
}



static int cmd_listdriveroptions(const struct command *c, int argc, char *argv[])
{
	const imgtool_module *mod;
	const option_guide *opt_guide;
	const char *opt_spec;

	mod = imgtool_find_module(argv[0]);
	if (!mod)
		goto error;

	fprintf(stdout, "Driver specific options for module '%s':\n\n", argv[0]);

	/* list write options */
	opt_guide = (const option_guide *) imgtool_get_info_ptr(&mod->imgclass, IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE);
	opt_spec = imgtool_get_info_string(&mod->imgclass, IMGTOOLINFO_STR_WRITEFILE_OPTSPEC);
	if (opt_guide)
	{
		fprintf(stdout, "Image specific file options (usable on the 'put' command):\n\n");
		listoptions(opt_guide, opt_spec);
		puts("\n");
	}
	else
	{
		fprintf(stdout, "No image specific file options\n\n");
	}

	/* list create options */
	opt_guide = mod->createimage_optguide;
	if (opt_guide)
	{
		fprintf(stdout, "Image specific creation options (usable on the 'create' command):\n\n");
		listoptions(opt_guide, mod->createimage_optspec);
		puts("\n");
	}
	else
	{
		fprintf(stdout, "No image specific creation options\n\n");
	}

	return 0;

error:
	reporterror((imgtoolerr_t)(IMGTOOLERR_MODULENOTFOUND|IMGTOOLERR_SRC_MODULE), c, argv[0], NULL, NULL, NULL, NULL);
	return -1;
}



/* ----------------------------------------------------------------------- */

static const struct command cmds[] =
{
	{ "create",             cmd_create,             "<format> <imagename> [--(createoption)=value]", 2, 8, 0},
	{ "dir",                cmd_dir,                "<format> <imagename> [path]", 2, 3, 0 },
	{ "get",                cmd_get,                "<format> <imagename> <filename> [newname] [--filter=filter] [--fork=fork]", 3, 4, 0 },
	{ "put",                cmd_put,                "<format> <imagename> <filename>... <destname> [--(fileoption)==value] [--filter=filter] [--fork=fork]", 3, 0xffff, 0 },
	{ "getall",             cmd_getall,             "<format> <imagename> [path] [--filter=filter]", 2, 3, 0 },
	{ "del",                cmd_del,                "<format> <imagename> <filename>...", 3, 3, 1 },
	{ "mkdir",              cmd_mkdir,              "<format> <imagename> <dirname>", 3, 3, 0 },
	{ "rmdir",              cmd_rmdir,              "<format> <imagename> <dirname>...", 3, 3, 1 },
	{ "readsector",         cmd_readsector,         "<format> <imagename> <track> <head> <sector> <filename>", 6, 6, 0 },
	{ "writesector",        cmd_writesector,        "<format> <imagename> <track> <head> <sector> <filename>", 6, 6, 0 },
	{ "identify",           cmd_identify,           "<imagename>", 1, 1 },
	{ "listformats",        cmd_listformats,        NULL, 0, 0, 0 },
	{ "listfilters",        cmd_listfilters,        NULL, 0, 0, 0 },
	{ "listdriveroptions",  cmd_listdriveroptions, "<format>", 1, 1, 0 }
};

int CLIB_DECL main(int argc, char *argv[])
{
	int i;
	int result;
	const struct command *c;
	const char *sample_format = "coco_jvc_rsdos";

#ifdef MAME_DEBUG
	if (imgtool_validitychecks())
		return -1;
#endif /* MAME_DEBUG */

	putchar('\n');

	if (argc > 1)
	{
		/* figure out what command they are running, and run it */
		for (i = 0; i < ARRAY_LENGTH(cmds); i++)
		{
			c = &cmds[i];
			if (!core_stricmp(c->name, argv[1]))
			{
				/* check argument count */
				if (c->minargs > (argc - 2))
					goto cmderror;

				/* initialize the imgtool core */
				imgtool_init(TRUE, NULL);

				if (c->lastargrepeats && (argc > c->maxargs))
				{
					for (i = c->maxargs+1; i < argc; i++)
					{
						argv[c->maxargs+1] = argv[i];

						result = c->cmdproc(c, c->maxargs, argv + 2);
						if (result)
							goto done;
					}
					result = 0;
					goto done;
				}
				else
				{
					if ((c->maxargs > 0) && (c->maxargs < (argc - 2)))
						goto cmderror;

					result = c->cmdproc(c, argc - 2, argv + 2);
					goto done;
				}
			}
		}
	}

	/* Usage */
	fprintf(stderr, "imgtool - Generic image manipulation tool for use with MESS\n\n");
	for (i = 0; i < ARRAY_LENGTH(cmds); i++)
	{
		writeusage(stdout, (i == 0), &cmds[i], argv);
	}

	fprintf(stderr, "\n<format> is the image format, e.g. %s\n", sample_format);
	fprintf(stderr, "<imagename> is the image filename; can specify a ZIP file for image name\n");

	fprintf(stderr, "\nExample usage:\n");
	fprintf(stderr, "\t%s dir %s myimageinazip.zip\n", imgtool_basename(argv[0]), sample_format);
	fprintf(stderr, "\t%s get %s myimage.dsk myfile.bin mynewfile.txt\n", imgtool_basename(argv[0]), sample_format);
	fprintf(stderr, "\t%s getall %s myimage.dsk\n", imgtool_basename(argv[0]), sample_format);
	result = 0;
	goto done;

cmderror:
	writeusage(stdout, 1, &cmds[i], argv);
	result = -1;

done:
	imgtool_exit();
	return result;
}
