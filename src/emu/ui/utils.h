// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    ui/utils.h

    Internal UI user interface.

***************************************************************************/

#pragma once

#ifndef __UI_UTILS_H__
#define __UI_UTILS_H__

#include "osdepend.h"
#include "render.h"
#include "libjpeg/jpeglib.h"

#define MAX_CHAR_INFO            256
#define MAX_CUST_FILTER          8

// GLOBAL ENUMERATORS
enum
{
	FILTER_FIRST = 0,
	FILTER_ALL = FILTER_FIRST,
	FILTER_AVAILABLE,
	FILTER_UNAVAILABLE,
	FILTER_WORKING,
	FILTER_NOT_WORKING,
	FILTER_MECHANICAL,
	FILTER_NOT_MECHANICAL,
	FILTER_CATEGORY,
	FILTER_FAVORITE,
	FILTER_BIOS,
	FILTER_PARENT,
	FILTER_CLONES,
	FILTER_MANUFACTURER,
	FILTER_YEAR,
	FILTER_SAVE,
	FILTER_NOSAVE,
	FILTER_CHD,
	FILTER_NOCHD,
	FILTER_VERTICAL,
	FILTER_HORIZONTAL,
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
	ENDS_VIEW,
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
	SHOW_PANELS = 0,
	HIDE_LEFT_PANEL,
	HIDE_RIGHT_PANEL,
	HIDE_BOTH
};

enum
{
	UI_FIRST_LOAD = 0,
	UI_GENERAL_LOAD = UI_FIRST_LOAD,
	UI_HISTORY_LOAD,
	UI_MAMEINFO_LOAD,
	UI_SYSINFO_LOAD,
	UI_MESSINFO_LOAD,
	UI_COMMAND_LOAD,
	UI_STORY_LOAD,
	UI_LAST_LOAD = UI_STORY_LOAD
};

enum
{
	UI_SW_FIRST = 0,
	UI_SW_ALL = UI_SW_FIRST,
	UI_SW_AVAILABLE,
	UI_SW_UNAVAILABLE,
	UI_SW_PARENTS,
	UI_SW_CLONES,
	UI_SW_YEARS,
	UI_SW_PUBLISHERS,
	UI_SW_SUPPORTED,
	UI_SW_PARTIAL_SUPPORTED,
	UI_SW_UNSUPPORTED,
	UI_SW_REGION,
	UI_SW_TYPE,
	UI_SW_LIST,
	UI_SW_CUSTOM,
	UI_SW_LAST = UI_SW_CUSTOM
};

enum
{
	HOVER_DAT_UP = -1000,
	HOVER_DAT_DOWN,
	HOVER_UI_LEFT,
	HOVER_UI_RIGHT,
	HOVER_ARROW_UP,
	HOVER_ARROW_DOWN,
	HOVER_B_FAV,
	HOVER_B_EXPORT,
	HOVER_B_DATS,
	HOVER_RPANEL_ARROW,
	HOVER_LPANEL_ARROW,
	HOVER_FILTER_FIRST,
	HOVER_FILTER_LAST = (HOVER_FILTER_FIRST) + 1 + FILTER_LAST,
	HOVER_SW_FILTER_FIRST,
	HOVER_SW_FILTER_LAST = (HOVER_SW_FILTER_FIRST) + 1 + UI_SW_LAST,
	HOVER_RP_FIRST,
	HOVER_RP_LAST = (HOVER_RP_FIRST) + 1 + RP_LAST
};

// GLOBAL STRUCTURES
struct ui_software_info
{
	ui_software_info() {}
	ui_software_info(std::string sname, std::string lname, std::string pname, std::string y, std::string pub,
		UINT8 s, std::string pa, const game_driver *d, std::string li, std::string i, std::string is, UINT8 em,
		std::string plong, std::string u, std::string de, bool av)
	{
		shortname = sname; longname = lname; parentname = pname; year = y; publisher = pub;
		supported = s; part = pa; driver = d; listname = li; interface = i; instance = is; startempty = em;
		parentlongname = plong; usage = u; devicetype = de; available = av;
	}
	std::string shortname;
	std::string longname;
	std::string parentname;
	std::string year;
	std::string publisher;
	UINT8 supported = 0;
	std::string part;
	const game_driver *driver;
	std::string listname;
	std::string interface;
	std::string instance;
	UINT8 startempty = 0;
	std::string parentlongname;
	std::string usage;
	std::string devicetype;
	bool available = false;

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

// Manufacturers
struct c_mnfct
{
	static void set(const char *str);
	static std::string getname(const char *str);
	static std::vector<std::string> ui;
	static UINT16 actual;
};

// Years
struct c_year
{
	static void set(const char *str);
	static std::vector<std::string> ui;
	static UINT16 actual;
};

// GLOBAL CLASS
struct ui_globals
{
	static UINT8        curimage_view, curdats_view, cur_sw_dats_view, rpanel;
	static bool         switch_image, redraw_icon, default_image, reset;
	static int          visible_main_lines, visible_sw_lines;
	static UINT16       panels_status;
	static bool         has_icons;
};

#define main_struct(name) \
struct name##_filters \
{ \
	static UINT16 actual; \
	static const char *text[]; \
	static size_t length; \
};

main_struct(main);
main_struct(sw);

// Custom filter
struct custfltr
{
	static UINT16  main;
	static UINT16  numother;
	static UINT16  other[MAX_CUST_FILTER];
	static UINT16  mnfct[MAX_CUST_FILTER];
	static UINT16  screen[MAX_CUST_FILTER];
	static UINT16  year[MAX_CUST_FILTER];
};

// Software custom filter
struct sw_custfltr
{
	static UINT16  main;
	static UINT16  numother;
	static UINT16  other[MAX_CUST_FILTER];
	static UINT16  mnfct[MAX_CUST_FILTER];
	static UINT16  year[MAX_CUST_FILTER];
	static UINT16  region[MAX_CUST_FILTER];
	static UINT16  type[MAX_CUST_FILTER];
	static UINT16  list[MAX_CUST_FILTER];
};

// GLOBAL FUNCTIONS

// advanced search function
int fuzzy_substring(std::string needle, std::string haystack);

// trim carriage return
char* chartrimcarriage(char str[]);

const char* strensure(const char* s);

// jpeg loader
template <typename _T>
void render_load_jpeg(_T &bitmap, emu_file &file, const char *dirname, const char *filename)
{
	// deallocate previous bitmap
	bitmap.reset();

	bitmap_format format = bitmap.format();

	// define file's full name
	std::string fname;

	if (dirname == nullptr)
		fname = filename;
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
	UINT64 jpg_size = file.size();
	unsigned char *jpg_buffer = global_alloc_array(unsigned char, jpg_size + 100);

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
			for (int i = 0; i < w; ++i)
				if (format == BITMAP_FORMAT_ARGB32)
					bitmap.pix32(j, i) = rgb_t(0xFF, buffer[0][i], buffer[0][i], buffer[0][i]);
				else
					bitmap.pix32(j, i) = rgb_t(buffer[0][i], buffer[0][i], buffer[0][i]);

		else if (s == 3)
			for (int i = 0; i < w; ++i)
				if (format == BITMAP_FORMAT_ARGB32)
					bitmap.pix32(j, i) = rgb_t(0xFF, buffer[0][i * s], buffer[0][i * s + 1], buffer[0][i * s + 2]);
				else
					bitmap.pix32(j, i) = rgb_t(buffer[0][i * s], buffer[0][i * s + 1], buffer[0][i * s + 2]);

		else
		{
			osd_printf_info("Error! Cannot read JPEG data from %s file.\n", fname.c_str());
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

#endif /* __UI_UTILS_H__ */
