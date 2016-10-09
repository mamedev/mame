// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    main.cpp

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
#include "strformat.h"

/* ---------------------------------------------------------------------- */

static void writeusage(FILE *f, int write_word_usage, const struct command *c, char *argv[])
{
	std::string cmdname = core_filename_extract_base(argv[0]);
	fprintf(f, "%s %s %s %s\n",
		(write_word_usage ? "Usage:" : "      "),
		cmdname.c_str(),
		c->name,
		c->usage ? c->usage : "");
}



/* ----------------------------------------------------------------------- */

static int parse_options(int argc, char *argv[], int minunnamed, int maxunnamed,
	util::option_resolution *resolution, filter_getinfoproc *filter, const char **fork)
{
	int i;
	int lastunnamed = 0;
	char *s;
	char *name = nullptr;
	char *value = nullptr;
	static char buf[256];

	if (filter)
		*filter = nullptr;
	if (fork)
		*fork = nullptr;

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

				resolution->find(name)->set_value(value);
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

error:
	fprintf(stderr, "%s: Unrecognized option\n", argv[i]);
	return -1;
}



void reporterror(imgtoolerr_t err, const struct command *c, const char *format, const char *imagename,
	const char *filename, const char *newname, util::option_resolution *opts)
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
	imgtool::image::ptr image;
	imgtool::partition::ptr partition;
	imgtool::directory::ptr imgenum;
	imgtool_dirent ent;
	char buf[512];
	char last_modified[19];
	const char *path;
	int partition_index = 0;

	// attempt to open image
	err = imgtool::image::open(argv[0], argv[1], OSD_FOPEN_READ, image);
	if (err)
		goto done;

	/* attempt to open partition */
	err = imgtool::partition::open(*image, partition_index, partition);
	if (err)
		goto done;

	path = argc > 2 ? argv[2] : nullptr;

	err = imgtool::directory::open(*partition, path, imgenum);
	if (err)
		goto done;

	memset(&ent, 0, sizeof(ent));
	last_modified[0] = '\0';
	total_count = 0;
	total_size = 0;

	fprintf(stdout, "Contents of %s:%s\n", argv[1], path ? path : "");

	image->info(buf, sizeof(buf));
	if (buf[0])
		fprintf(stdout, "%s\n", buf);
	fprintf(stdout, "------------------------------  --------  ---------------  ------------------\n");

	while (((err = imgenum->get_next(ent)) == 0) && !ent.eof)
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

	freespace_err = partition->get_free_space(freespace);

	if (err)
		goto done;

	fprintf(stdout, "------------------------  ------ ---------------\n");
	fprintf(stdout, "%8i File(s)        %8i bytes\n", total_count, total_size);
	if (!freespace_err)
		fprintf(stdout, "                        %8u bytes free\n", (unsigned int) freespace);

done:
	if (err)
		reporterror(err, c, argv[0], argv[1], nullptr, nullptr, nullptr);
	return err ? -1 : 0;
}



static int cmd_get(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool::image::ptr image;
	imgtool::partition::ptr partition;
	const char *filename;
	char *new_filename;
	int unnamedargs = 0;
	filter_getinfoproc filter;
	const char *fork;
	int partition_index = 0;

	err = imgtool::image::open(argv[0], argv[1], OSD_FOPEN_READ, image);
	if (err)
		goto done;

	err = imgtool::partition::open(*image, partition_index, partition);
	if (err)
		goto done;

	filename = interpret_filename(argv[2]);

	unnamedargs = parse_options(argc, argv, 3, 4, nullptr, &filter, &fork);
	if (unnamedargs < 0)
		goto done;

	new_filename = (unnamedargs == 4) ? argv[3] : nullptr;

	err = partition->get_file(filename, fork, new_filename, filter);
	if (err)
		goto done;

	err = IMGTOOLERR_SUCCESS;

done:
	if (err)
		reporterror(err, c, argv[0], argv[1], argv[2], argv[3], nullptr);
	return (err || (unnamedargs < 0)) ? -1 : 0;
}



static int cmd_put(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	int i;
	imgtool::image::ptr image;
	imgtool::partition::ptr partition;
	const char *filename = nullptr;
	int unnamedargs;
	filter_getinfoproc filter;
	const imgtool_module *module;
	std::unique_ptr<util::option_resolution> resolution;
	const char *fork;
	const char *new_filename;
	char **filename_list;
	int filename_count;
	int partition_index = 0;
	const util::option_guide *writefile_optguide;
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
		err = imgtool::image::open(module, argv[1], OSD_FOPEN_RW, image);
		if (err)
			goto done;

		/* open up the partition */
		err = imgtool::partition::open(*image, partition_index, partition);
		if (err)
			goto done;

		writefile_optguide = (const util::option_guide *) partition->get_info_ptr(IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE);
		writefile_optspec = (const char *)partition->get_info_ptr(IMGTOOLINFO_STR_WRITEFILE_OPTSPEC);

		if (writefile_optguide && writefile_optspec)
		{
			try { resolution.reset(new util::option_resolution(*writefile_optguide)); }
			catch (...)
			{
				err = IMGTOOLERR_OUTOFMEMORY;
				goto done;
			}
			resolution->set_specification(writefile_optspec);
		}
	}

	unnamedargs = parse_options(argc, argv, 4, 0xffff, resolution.get(), &filter, &fork);
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
		err = partition->put_file(new_filename, fork, filename, resolution.get(), filter);
		if (err)
			goto done;
	}

done:
	if (err)
		reporterror(err, c, argv[0], argv[1], filename, nullptr, resolution.get());
	return err ? -1 : 0;
}



static int cmd_getall(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool::image::ptr image;
	imgtool::partition::ptr partition;
	imgtool::directory::ptr imgenum;
	imgtool_dirent ent;
	filter_getinfoproc filter;
	int unnamedargs;
	const char *path = nullptr;
	int arg;
	int partition_index = 0;

	err = imgtool::image::open(argv[0], argv[1], OSD_FOPEN_READ, image);
	if (err)
		goto done;

	err = imgtool::partition::open(*image, partition_index, partition);
	if (err)
		goto done;

	arg = 2;
	if ((argc > 2) && (argv[2][0] != '-'))
	{
		path = argv[arg++];
	}

	unnamedargs = parse_options(argc, argv, arg, arg, nullptr, &filter, nullptr);
	if (unnamedargs < 0)
		goto done;

	err = imgtool::directory::open(*partition, path, imgenum);
	if (err)
		goto done;

	memset(&ent, 0, sizeof(ent));

	while (((err = imgenum->get_next(ent)) == 0) && !ent.eof)
	{
		fprintf(stdout, "Retrieving %s (%u bytes)\n", ent.filename, (unsigned int) ent.filesize);

		err = partition->get_file(ent.filename, nullptr, nullptr, filter);
		if (err)
			goto done;
	}

done:
	if (err)
		reporterror(err, c, argv[0], argv[1], nullptr, nullptr, nullptr);
	return err ? -1 : 0;
}



static int cmd_del(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool::image::ptr image;
	imgtool::partition::ptr partition;
	int partition_index = 0;

	err = imgtool::image::open(argv[0], argv[1], OSD_FOPEN_RW, image);
	if (err)
		goto done;

	err = imgtool::partition::open(*image, partition_index, partition);
	if (err)
		goto done;

	err = partition->delete_file(argv[2]);
	if (err)
		goto done;

done:
	if (err)
		reporterror(err, c, argv[0], argv[1], argv[2], nullptr, nullptr);
	return err ? -1 : 0;
}



static int cmd_mkdir(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool::image::ptr image;
	imgtool::partition::ptr partition;
	int partition_index = 0;

	err = imgtool::image::open(argv[0], argv[1], OSD_FOPEN_RW, image);
	if (err)
		goto done;

	err = imgtool::partition::open(*image, partition_index, partition);
	if (err)
		goto done;

	err = partition->create_directory(argv[2]);
	if (err)
		goto done;

done:
	if (err)
		reporterror(err, c, argv[0], argv[1], argv[2], nullptr, nullptr);
	return err ? -1 : 0;
}



static int cmd_rmdir(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	imgtool::image::ptr image;
	imgtool::partition::ptr partition;
	int partition_index = 0;

	err = imgtool::image::open(argv[0], argv[1], OSD_FOPEN_RW, image);
	if (err)
		goto done;

	err = imgtool::partition::open(*image, partition_index, partition);
	if (err)
		goto done;

	err = partition->delete_directory(argv[2]);
	if (err)
		goto done;

done:
	if (err)
		reporterror(err, c, argv[0], argv[1], argv[2], nullptr, nullptr);
	return err ? -1 : 0;
}



static int cmd_identify(const struct command *c, int argc, char *argv[])
{
	imgtool_module *modules[128];
	imgtoolerr_t err;
	int i;

	err = imgtool::image::identify_file(argv[0], modules, ARRAY_LENGTH(modules));
	if (err)
	{
		reporterror(err, c, nullptr, argv[0], nullptr, nullptr, nullptr);
		return -1;
	}
	else
	{
		for (i = 0; modules[i]; i++)
		{
			printf("%.16s %s\n", modules[i]->name, modules[i]->description);
		}

		return 0;
	}
}



static int cmd_create(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	int unnamedargs;
	const imgtool_module *module;
	std::unique_ptr<util::option_resolution> resolution;
	
	module = imgtool_find_module(argv[0]);
	if (!module)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_MODULENOTFOUND | IMGTOOLERR_SRC_MODULE);
		goto error;
	}

	if (module->createimage_optguide && module->createimage_optspec)
	{
		try { resolution.reset(new util::option_resolution(*module->createimage_optguide)); }
		catch (...)
		{
			err = IMGTOOLERR_OUTOFMEMORY;
			goto error;
		}
		resolution->set_specification(module->createimage_optspec);
	}

	unnamedargs = parse_options(argc, argv, 2, 3, resolution.get(), nullptr, nullptr);
	if (unnamedargs < 0)
		return -1;

	err = imgtool::image::create(module, argv[1], resolution.get());
	if (err)
		goto error;

	return 0;

error:
	reporterror(err, c, argv[0], argv[1], nullptr, nullptr, nullptr);
	return -1;
}



static int cmd_readsector(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	std::unique_ptr<imgtool::image> img;
	imgtool_stream *stream = nullptr;
	std::vector<UINT8> buffer;
	UINT32 track, head, sector;

	/* attempt to open image */
	err = imgtool::image::open(argv[0], argv[1], OSD_FOPEN_READ, img);
	if (err)
		goto done;

	track = atoi(argv[2]);
	head = atoi(argv[3]);
	sector = atoi(argv[4]);

	err = img->read_sector(track, head, sector, buffer);
	if (err)
		goto done;

	stream = stream_open(argv[5], OSD_FOPEN_WRITE);
	if (!stream)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_FILENOTFOUND | IMGTOOLERR_SRC_NATIVEFILE);
		goto done;
	}

	stream_write(stream, &buffer[0], buffer.size());

done:
	if (stream)
		stream_close(stream);
	if (err)
		reporterror(err, c, argv[0], argv[1], nullptr, nullptr, nullptr);
	return err ? -1 : 0;
}



static int cmd_writesector(const struct command *c, int argc, char *argv[])
{
	imgtoolerr_t err;
	std::unique_ptr<imgtool::image> img;
	imgtool_stream *stream = nullptr;
	dynamic_buffer buffer;
	UINT32 size, track, head, sector;

	// attempt to open image
	err = imgtool::image::open(argv[0], argv[1], OSD_FOPEN_RW, img);
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

	err = img->write_sector(track, head, sector, &buffer[0], size);
	if (err)
		goto done;

done:
	if (stream)
		stream_close(stream);
	if (err)
		reporterror(err, c, argv[0], argv[1], nullptr, nullptr, nullptr);
	return err ? -1 : 0;
}



static int cmd_listformats(const struct command *c, int argc, char *argv[])
{
	fprintf(stdout, "Image formats supported by imgtool:\n\n");

	for (const auto &module : imgtool_get_modules())
	{
		fprintf(stdout, "  %-25s%s\n", module->name, module->description);
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

static void listoptions(const util::option_guide &opt_guide, const char *opt_spec)
{
	util::option_resolution resolution(opt_guide);
	resolution.set_specification(opt_spec);

	fprintf(stdout, "Option           Allowed values                 Description\n");
	fprintf(stdout, "---------------- ------------------------------ -----------\n");

	for (auto iter = resolution.entries_begin(); iter != resolution.entries_end(); iter++)
	{
		const util::option_resolution::entry &entry = *iter;
				std::stringstream description_buffer;

		std::string opt_name = string_format("--%s", entry.identifier());
		const char *opt_desc = entry.display_name();

		// is this option relevant?
		if (!strchr(opt_spec, entry.parameter()))
			continue;

		switch (entry.option_type())
		{
		case util::option_guide::entry::option_type::INT:
			for (const auto &range : entry.ranges())
			{
				if (!description_buffer.str().empty())
					description_buffer << "/";

				if (range.min == range.max)
					util::stream_format(description_buffer, "%d", range.min);
				else
					util::stream_format(description_buffer, "%d-%d", range.min, range.max);
			}
			break;

		case util::option_guide::entry::option_type::ENUM_BEGIN:
			for (auto enum_value = entry.enum_value_begin(); enum_value != entry.enum_value_end(); enum_value++)
			{
				if (!description_buffer.str().empty())
					description_buffer << '/';
				description_buffer << enum_value->identifier();
			}
			break;

		case util::option_guide::entry::option_type::STRING:
			description_buffer << "(string)";
			break;

		default:
			assert(0);
			break;
		}

		fprintf(stdout, "%16s %-30s %s\n",
			opt_name.c_str(),
			description_buffer.str().c_str(),
			opt_desc);
	}
}



static int cmd_listdriveroptions(const struct command *c, int argc, char *argv[])
{
	const imgtool_module *mod;
	const util::option_guide *opt_guide;
	const char *opt_spec;

	mod = imgtool_find_module(argv[0]);
	if (!mod)
	{
		reporterror((imgtoolerr_t)(IMGTOOLERR_MODULENOTFOUND|IMGTOOLERR_SRC_MODULE), c, argv[0], nullptr, nullptr, nullptr, nullptr);
		return -1;
	}

	fprintf(stdout, "Driver specific options for module '%s':\n\n", argv[0]);

	/* list write options */
	opt_guide = (const util::option_guide *) imgtool_get_info_ptr(&mod->imgclass, IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE);
	opt_spec = imgtool_get_info_string(&mod->imgclass, IMGTOOLINFO_STR_WRITEFILE_OPTSPEC);
	if (opt_guide)
	{
		fprintf(stdout, "Image specific file options (usable on the 'put' command):\n\n");
		listoptions(*opt_guide, opt_spec);
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
		listoptions(*opt_guide, mod->createimage_optspec);
		puts("\n");
	}
	else
	{
		fprintf(stdout, "No image specific creation options\n\n");
	}

	return 0;
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
	{ "listformats",        cmd_listformats,        nullptr, 0, 0, 0 },
	{ "listfilters",        cmd_listfilters,        nullptr, 0, 0, 0 },
	{ "listdriveroptions",  cmd_listdriveroptions, "<format>", 1, 1, 0 }
};

int CLIB_DECL main(int argc, char *argv[])
{
	int i;
	int result;
	const struct command *c;
	const char *sample_format = "coco_jvc_rsdos";
	std::string cmdname = core_filename_extract_base(argv[0]);

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
				imgtool_init(TRUE, nullptr);

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
	fprintf(stderr, "imgtool - Generic image manipulation tool for use with MAME\n\n");
	for (i = 0; i < ARRAY_LENGTH(cmds); i++)
	{
		writeusage(stdout, (i == 0), &cmds[i], argv);
	}

	fprintf(stderr, "\n<format> is the image format, e.g. %s\n", sample_format);
	fprintf(stderr, "<imagename> is the image filename; can specify a ZIP file for image name\n");

	fprintf(stderr, "\nExample usage:\n");
	fprintf(stderr, "\t%s dir %s myimageinazip.zip\n", cmdname.c_str(), sample_format);
	fprintf(stderr, "\t%s get %s myimage.dsk myfile.bin mynewfile.txt\n", cmdname.c_str(), sample_format);
	fprintf(stderr, "\t%s getall %s myimage.dsk\n", cmdname.c_str(), sample_format);
	result = 0;
	goto done;

cmderror:
	writeusage(stdout, 1, &cmds[i], argv);
	result = -1;

done:
	imgtool_exit();
	return result;
}
