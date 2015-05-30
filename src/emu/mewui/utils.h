/***************************************************************************

	mewui/utils.h

	Internal MEWUI user interface.

***************************************************************************/

#pragma once

#ifndef __MEWUI_UTILS_H__
#define __MEWUI_UTILS_H__

#include "osdepend.h"
#include "render.h"
#include "libjpeg/jpeglib.h"
#include <algorithm>

#define MENU_FLAG_MEWUI			 (1 << 6)
#define MENU_FLAG_MEWUI_HISTORY	 (1 << 7)
#define MENU_FLAG_MEWUI_SWLIST	 (1 << 8)
#define MENU_FLAG_MEWUI_FAVORITE (1 << 9)
#define MAX_CHAR_INFO			 256
#define CR						 0x0d	//  '\n' and '\r' meanings are swapped in some
#define LF						 0x0a	//  compilers (e.g., Mac compilers)
#define UI_MENU_PROCESS_ONLYCHAR 8
#define MAX_FILTER				 10
#define MAX_ICONS_RENDER		 40
#define MEWUI_DIR				 "mewui"

// GLOBAL ENUMERATORS
enum
{
	FILTER_FIRST = 0,
	FILTER_ALL = FILTER_FIRST,
	FILTER_AVAILABLE,
	FILTER_UNAVAILABLE,
	FILTER_WORKING,
	FILTER_NOT_MECHANICAL,
	FILTER_CATEGORY,
	FILTER_FAVORITE_GAME,
	FILTER_BIOS,
	FILTER_PARENT,
	FILTER_CLONES,
	FILTER_NOT_WORKING,
	FILTER_MECHANICAL,
	FILTER_MANUFACTURER,
	FILTER_YEAR,
	FILTER_SAVE,
	FILTER_CHD,
	FILTER_SAMPLES,
	FILTER_STEREO,
	FILTER_VERTICAL,
	FILTER_HORIZONTAL,
	FILTER_RASTER,
	FILTER_VECTOR,
	FILTER_CUSTOM,
	FILTER_LAST = FILTER_CUSTOM
};

enum
{
	FIRST_VIEW = 0,
	SNAPSHOT_VIEW = FIRST_VIEW,
	CABINETS_VIEW,
	CPANELS_VIEW,
	PCBS_VIEW,
	FLYERS_VIEW,
	TITLES_VIEW,
	ARTPREV_VIEW,
	BOSSES_VIEW,
	LOGOS_VIEW,
	VERSUS_VIEW,
	GAMEOVER_VIEW,
	HOWTO_VIEW,
	SCORES_VIEW,
	SELECT_VIEW,
	MARQUEES_VIEW,
	LAST_VIEW = MARQUEES_VIEW
};

enum
{
	RP_FIRST = 0,
	RP_IMAGES = RP_FIRST,
	RP_INFOS,
	RP_LAST = RP_INFOS
};

enum
{
	MEWUI_FIRST_LOAD = 0,
	MEWUI_GENERAL_LOAD = MEWUI_FIRST_LOAD,
	MEWUI_HISTORY_LOAD,
	MEWUI_MAMEINFO_LOAD,
	MEWUI_SYSINFO_LOAD,
	MEWUI_MESSINFO_LOAD,
	MEWUI_COMMAND_LOAD,
	MEWUI_STORY_LOAD,
	MEWUI_LAST_LOAD = MEWUI_STORY_LOAD
};

enum
{
	MEWUI_SW_FIRST = 0,
	MEWUI_SW_ALL = MEWUI_SW_FIRST,
	MEWUI_SW_AVAILABLE,
	MEWUI_SW_UNAVAILABLE,
	MEWUI_SW_PARENTS,
	MEWUI_SW_CLONES,
	MEWUI_SW_YEARS,
	MEWUI_SW_PUBLISHERS,
	MEWUI_SW_SUPPORTED,
	MEWUI_SW_PARTIAL_SUPPORTED,
	MEWUI_SW_UNSUPPORTED,
	MEWUI_SW_REGION,
	MEWUI_SW_LAST = MEWUI_SW_REGION
};

enum
{
	MEWUI_MAME_FIRST = 0,
	MEWUI_MAME = MEWUI_MAME_FIRST,
	MEWUI_ARCADES,
	MEWUI_SYSTEMS,
	MEWUI_MAME_LAST = MEWUI_SYSTEMS
};

// GLOBAL STRUCTURES
struct ui_software_info
{
	std::string shortname;
	std::string longname;
	std::string parentname;
	std::string year;
	std::string publisher;
	UINT8 supported;
	std::string part;
	const game_driver *driver;
	std::string listname;
	std::string interface;
	std::string instance;
	UINT8 startempty;
	std::string parentlongname;
	std::string usage;
	std::string devicetype;
	bool available;

	bool operator==(const ui_software_info& r)
	{
		if (shortname == r.shortname && longname == r.longname && parentname == r.parentname
			&& year == r.year && publisher == r.publisher && supported == r.supported
			&& part == r.part && driver == r.driver && listname == r.listname
			&& interface == r.interface && instance == r.instance && startempty == r.startempty
			&& parentlongname == r.parentlongname && usage == r.usage && devicetype == r.devicetype)
			return true;

		return false;
	}
};

struct cache_info
{
	UINT8 b_vector, b_stereo, b_samples, b_chd;
};

struct reselect_last
{
	static std::string driver, software, part;
};

// Manufacturers
class c_mnfct
{
public:
	c_mnfct();

	static void set(const char *str);
	static std::string getname(const char *str);
	static std::vector<std::string> ui;
	static UINT16 actual;
};

// Years
class c_year
{
public:
	c_year();

	static void set(const char *str);
	static std::vector<std::string> ui;
	static UINT16 actual;
};

// GLOBAL CLASS
class mewui_globals
{
public:
	mewui_globals();

	static UINT16	   actual_filter, actual_sw_filter;
	static const char   *filter_text[], *sw_filter_text[], *ume_text[];
	static size_t	   s_filter_text, sw_filter_len, s_ume_text;
	static UINT8		curimage_view, curdats_view, ume_system, cur_sw_dats_view, rpanel_infos;
	static bool		 switch_image, redraw_icon, default_image;
	static bool		 force_reselect_software, force_reset_main;
	static int		  visible_main_lines, visible_sw_lines;
	static std::vector<cache_info> driver_cache;
};

// Custom filter class
class custfltr
{
public:
	custfltr();

	static UINT16  main_filter;
	static UINT16  numother;
	static UINT16  other[MAX_FILTER];
	static UINT16  mnfct[MAX_FILTER];
	static UINT16  year[MAX_FILTER];
};

// GLOBAL FUNCTIONS

// save options to file
void save_game_options(running_machine &machine);

// General info
void general_info(running_machine &machine, const game_driver *driver, std::string &buffer);

// advanced search function
int fuzzy_substring(const char *needle, const char *haystack);

// drivers cache
void save_cache_info(running_machine &machine);
void load_cache_info(running_machine &machine);

void fskip(char *s, int id = 0);

// custom filter load and save
void load_custom_filters();
void save_custom_filters();

// TEMPLATES

// jpeg loader
template <typename _T>
void render_load_jpeg(_T &bitmap, emu_file &file, const char *dirname, const char *filename)
{
	// deallocate previous bitmap
	bitmap.reset();

	bitmap_format format = bitmap.format();

	UINT64 jpg_size = 0;
	unsigned char *jpg_buffer = NULL;

	// define file's full name
	std::string fname;

	if (!dirname)
		fname.assign(filename);
	else
		fname.assign(dirname).append(PATH_SEPARATOR).append(filename);

	file_error filerr = file.open(fname.c_str());

	if (filerr != FILERR_NONE)
		return;

	// define standard JPEG structures
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// allocates a buffer for the image
	jpg_size = file.size();
	jpg_buffer = global_alloc_array(unsigned char, jpg_size + 100);

	// read data from the file and set them in the buffer
	file.read(jpg_buffer, jpg_size);
	jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);

	// read JPEG header and start decompression
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	// allocates the destination bitmap
	int w = cinfo.output_width;
	int h = cinfo.output_height;
	int s = cinfo.output_components;
	bitmap.allocate(w, h);

	// allocates a buffer to receive the information and copy them into the bitmap
	int row_stride = cinfo.output_width * cinfo.output_components;
	JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW));
	buffer[0] = (JSAMPROW)malloc(sizeof(JSAMPLE) * row_stride);

	while ( cinfo.output_scanline < cinfo.output_height )
	{
		int j = cinfo.output_scanline;
		jpeg_read_scanlines(&cinfo, buffer, 1);

		if (s == 1)
			for (int i = 0; i < w; i++)
				if (format == BITMAP_FORMAT_ARGB32)
					bitmap.pix32(j, i) = rgb_t(0xFF, buffer[0][i], buffer[0][i], buffer[0][i]);
				else
					bitmap.pix32(j, i) = rgb_t(buffer[0][i], buffer[0][i], buffer[0][i]);

		else if (s == 3)
			for (int i = 0; i < w; i++)
				if (format == BITMAP_FORMAT_ARGB32)
					bitmap.pix32(j, i) = rgb_t(0xFF, buffer[0][i * s], buffer[0][i * s + 1], buffer[0][i * s + 2]);
				else
					bitmap.pix32(j, i) = rgb_t(buffer[0][i * s], buffer[0][i * s + 1], buffer[0][i * s + 2]);

		else
		{
			osd_printf_verbose("Error! Cannot read JPEG data from %s file.\n", fname.c_str());
			break;
		}
	}

	// finish decompression and frees the memory
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	file.close();
	free(buffer[0]);
	free(buffer);
	global_free_array(jpg_buffer);
}

#endif /* __MEWUI_UTILS_H__ */
