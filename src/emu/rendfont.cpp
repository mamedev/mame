// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    rendfont.cpp

    Rendering system font management.

***************************************************************************/

#include "emu.h"
#include "rendfont.h"

#include "emuopts.h"
#include "fileio.h"

#include "corestr.h"
#include "coreutil.h"

#include "osdepend.h"
#include "uismall.fh"
#include "unicode.h"

#include "ui/uicmd14.fh"
#include "ui/cmddata.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>


#define VERBOSE 0

#define LOG(...) do { if (VERBOSE) osd_printf_verbose(__VA_ARGS__); } while (0)


namespace {

template <typename Iterator>
class bdf_helper
{
public:
	bdf_helper(Iterator const &begin, Iterator const &end)
		: m_keyword_begin(begin)
		, m_keyword_end(begin)
		, m_value_begin(begin)
		, m_value_end(begin)
		, m_line_end(begin)
		, m_end(end)
	{
		next_line();
	}

	bool at_end() const { return m_end == m_keyword_begin; }

	void next_line()
	{
		m_keyword_begin = m_line_end;
		while ((m_end != m_keyword_begin) && (('\r' == *m_keyword_begin) || ('\n' == *m_keyword_begin)))
			++m_keyword_begin;

		m_keyword_end = m_keyword_begin;
		while ((m_end != m_keyword_end) && (' ' != *m_keyword_end) && ('\t' != *m_keyword_end) && ('\r' != *m_keyword_end) && ('\n' != *m_keyword_end))
			++m_keyword_end;

		m_value_begin = m_keyword_end;
		while ((m_end != m_value_begin) && ((' ' == *m_value_begin) || ('\t' == *m_value_begin)) && ('\r' != *m_value_begin) && ('\n' != *m_value_begin))
			++m_value_begin;

		m_value_end = m_line_end = m_value_begin;
		while ((m_end != m_line_end) && ('\r' != *m_line_end) && ('\n' != *m_line_end))
		{
			if ((' ' != *m_line_end) && ('\t' != *m_line_end))
				m_value_end = ++m_line_end;
			else
				++m_line_end;
		}
	}

	bool is_keyword(char const *keyword) const
	{
		Iterator pos(m_keyword_begin);
		while (true)
		{
			if (m_keyword_end == pos)
			{
				return '\0' == *keyword;
			}
			else if (('\0' == *keyword) || (*pos != *keyword))
			{
				return false;
			}
			else
			{
				++pos;
				++keyword;
			}
		}
	}

	Iterator const &keyword_begin() const { return m_keyword_begin; }
	Iterator const &keyword_end() const { return m_keyword_end; }
	auto keyword_length() const { return std::distance(m_keyword_begin, m_keyword_end); }

	Iterator const &value_begin() const { return m_value_begin; }
	Iterator const &value_end() const { return m_value_end; }
	auto value_length() const { return std::distance(m_value_begin, m_value_end); }

private:
	Iterator        m_keyword_begin;
	Iterator        m_keyword_end;
	Iterator        m_value_begin;
	Iterator        m_value_end;
	Iterator        m_line_end;
	Iterator const  m_end;
};


class bdc_header
{
public:
	static constexpr unsigned MAJVERSION = 1;
	static constexpr unsigned MINVERSION = 0;

	bool read(util::read_stream &f)
	{
		std::size_t actual(0);
		return !f.read(m_data, sizeof(m_data), actual) && actual == sizeof(m_data);
	}
	bool write(util::write_stream &f)
	{
		std::size_t actual(0);
		return !f.write(m_data, sizeof(m_data), actual) && actual == sizeof(m_data);
	}

	bool check_magic() const
	{
		return !std::memcmp(MAGIC, m_data + OFFS_MAGIC, OFFS_MAJVERSION - OFFS_MAGIC);
	}
	unsigned get_major_version() const
	{
		return m_data[OFFS_MAJVERSION];
	}
	unsigned get_minor_version() const
	{
		return m_data[OFFS_MINVERSION];
	}
	u64 get_original_length() const
	{
		return
				(u64(m_data[OFFS_ORIGLENGTH + 0]) << (7 * 8)) |
				(u64(m_data[OFFS_ORIGLENGTH + 1]) << (6 * 8)) |
				(u64(m_data[OFFS_ORIGLENGTH + 2]) << (5 * 8)) |
				(u64(m_data[OFFS_ORIGLENGTH + 3]) << (4 * 8)) |
				(u64(m_data[OFFS_ORIGLENGTH + 4]) << (3 * 8)) |
				(u64(m_data[OFFS_ORIGLENGTH + 5]) << (2 * 8)) |
				(u64(m_data[OFFS_ORIGLENGTH + 6]) << (1 * 8)) |
				(u64(m_data[OFFS_ORIGLENGTH + 7]) << (0 * 8));
	}
	u32 get_original_hash() const
	{
		return
				(u32(m_data[OFFS_ORIGHASH + 0]) << (3 * 8)) |
				(u32(m_data[OFFS_ORIGHASH + 1]) << (2 * 8)) |
				(u32(m_data[OFFS_ORIGHASH + 2]) << (1 * 8)) |
				(u32(m_data[OFFS_ORIGHASH + 3]) << (0 * 8));
	}
	u32 get_glyph_count() const
	{
		return
				(u32(m_data[OFFS_GLYPHCOUNT + 0]) << (3 * 8)) |
				(u32(m_data[OFFS_GLYPHCOUNT + 1]) << (2 * 8)) |
				(u32(m_data[OFFS_GLYPHCOUNT + 2]) << (1 * 8)) |
				(u32(m_data[OFFS_GLYPHCOUNT + 3]) << (0 * 8));
	}
	u16 get_height() const
	{
		return
				(u16(m_data[OFFS_HEIGHT + 0]) << (1 * 8)) |
				(u16(m_data[OFFS_HEIGHT + 1]) << (0 * 8));
	}
	s16 get_y_offset() const
	{
		return
				(u16(m_data[OFFS_YOFFSET + 0]) << (1 * 8)) |
				(u16(m_data[OFFS_YOFFSET + 1]) << (0 * 8));
	}
	s32 get_default_character() const
	{
		return
				(u32(m_data[OFFS_DEFCHAR + 0]) << (3 * 8)) |
				(u32(m_data[OFFS_DEFCHAR + 1]) << (2 * 8)) |
				(u32(m_data[OFFS_DEFCHAR + 2]) << (1 * 8)) |
				(u32(m_data[OFFS_DEFCHAR + 3]) << (0 * 8));
	}

	void set_magic()
	{
		std::memcpy(m_data + OFFS_MAGIC, MAGIC, OFFS_MAJVERSION - OFFS_MAGIC);
	}
	void set_version()
	{
		m_data[OFFS_MAJVERSION] = MAJVERSION;
		m_data[OFFS_MINVERSION] = MINVERSION;
	}
	void set_original_length(u64 value)
	{
		m_data[OFFS_ORIGLENGTH + 0] = u8((value >> (7 * 8)) & 0x00ff);
		m_data[OFFS_ORIGLENGTH + 1] = u8((value >> (6 * 8)) & 0x00ff);
		m_data[OFFS_ORIGLENGTH + 2] = u8((value >> (5 * 8)) & 0x00ff);
		m_data[OFFS_ORIGLENGTH + 3] = u8((value >> (4 * 8)) & 0x00ff);
		m_data[OFFS_ORIGLENGTH + 4] = u8((value >> (3 * 8)) & 0x00ff);
		m_data[OFFS_ORIGLENGTH + 5] = u8((value >> (2 * 8)) & 0x00ff);
		m_data[OFFS_ORIGLENGTH + 6] = u8((value >> (1 * 8)) & 0x00ff);
		m_data[OFFS_ORIGLENGTH + 7] = u8((value >> (0 * 8)) & 0x00ff);
	}
	void set_original_hash(u32 value)
	{
		m_data[OFFS_ORIGHASH + 0] = u8((value >> (3 * 8)) & 0x00ff);
		m_data[OFFS_ORIGHASH + 1] = u8((value >> (2 * 8)) & 0x00ff);
		m_data[OFFS_ORIGHASH + 2] = u8((value >> (1 * 8)) & 0x00ff);
		m_data[OFFS_ORIGHASH + 3] = u8((value >> (0 * 8)) & 0x00ff);
	}
	void set_glyph_count(u32 value)
	{
		m_data[OFFS_GLYPHCOUNT + 0] = u8((value >> (3 * 8)) & 0x00ff);
		m_data[OFFS_GLYPHCOUNT + 1] = u8((value >> (2 * 8)) & 0x00ff);
		m_data[OFFS_GLYPHCOUNT + 2] = u8((value >> (1 * 8)) & 0x00ff);
		m_data[OFFS_GLYPHCOUNT + 3] = u8((value >> (0 * 8)) & 0x00ff);
	}
	void set_height(u16 value)
	{
		m_data[OFFS_HEIGHT + 0] = u8((value >> (1 * 8)) & 0x00ff);
		m_data[OFFS_HEIGHT + 1] = u8((value >> (0 * 8)) & 0x00ff);
	}
	void set_y_offset(s16 value)
	{
		m_data[OFFS_YOFFSET + 0] = u8((value >> (1 * 8)) & 0x00ff);
		m_data[OFFS_YOFFSET + 1] = u8((value >> (0 * 8)) & 0x00ff);
	}
	void set_default_character(s32 value)
	{
		m_data[OFFS_DEFCHAR + 0] = u8((value >> (3 * 8)) & 0x00ff);
		m_data[OFFS_DEFCHAR + 1] = u8((value >> (2 * 8)) & 0x00ff);
		m_data[OFFS_DEFCHAR + 2] = u8((value >> (1 * 8)) & 0x00ff);
		m_data[OFFS_DEFCHAR + 3] = u8((value >> (0 * 8)) & 0x00ff);
	}

private:
	static constexpr std::size_t    OFFS_MAGIC      = 0x00; // 0x06 bytes
	static constexpr std::size_t    OFFS_MAJVERSION = 0x06; // 0x01 bytes (binary integer)
	static constexpr std::size_t    OFFS_MINVERSION = 0x07; // 0x01 bytes (binary integer)
	static constexpr std::size_t    OFFS_ORIGLENGTH = 0x08; // 0x08 bytes (big-endian binary integer)
	static constexpr std::size_t    OFFS_ORIGHASH   = 0x10; // 0x04 bytes
	static constexpr std::size_t    OFFS_GLYPHCOUNT = 0x14; // 0x04 bytes (big-endian binary integer)
	static constexpr std::size_t    OFFS_HEIGHT     = 0x18; // 0x02 bytes (big-endian binary integer)
	static constexpr std::size_t    OFFS_YOFFSET    = 0x1a; // 0x02 bytes (big-endian binary integer)
	static constexpr std::size_t    OFFS_DEFCHAR    = 0x1c; // 0x04 bytes (big-endian binary integer)
	static constexpr std::size_t    OFFS_END        = 0x20;

	static u8 const                 MAGIC[OFFS_MAJVERSION - OFFS_MAGIC];

	u8                              m_data[OFFS_END];
};

u8 const bdc_header::MAGIC[OFFS_MAJVERSION - OFFS_MAGIC] = { 'b', 'd', 'c', 'f', 'n', 't' };


class bdc_table_entry
{
public:
	bdc_table_entry(void *bytes)
		: m_ptr(reinterpret_cast<u8 *>(bytes))
	{
	}
	bdc_table_entry(bdc_table_entry const &that) = default;
	bdc_table_entry(bdc_table_entry &&that) = default;

	bdc_table_entry get_next() const
	{
		return bdc_table_entry(m_ptr + OFFS_END);
	}

	u32 get_encoding() const
	{
		return
				(u32(m_ptr[OFFS_ENCODING + 0]) << (3 * 8)) |
				(u32(m_ptr[OFFS_ENCODING + 1]) << (2 * 8)) |
				(u32(m_ptr[OFFS_ENCODING + 2]) << (1 * 8)) |
				(u32(m_ptr[OFFS_ENCODING + 3]) << (0 * 8));
	}
	u16 get_x_advance() const
	{
		return
				(u16(m_ptr[OFFS_XADVANCE + 0]) << (1 * 8)) |
				(u16(m_ptr[OFFS_XADVANCE + 1]) << (0 * 8));
	}
	s16 get_bb_x_offset() const
	{
		return
				(u16(m_ptr[OFFS_BBXOFFSET + 0]) << (1 * 8)) |
				(u16(m_ptr[OFFS_BBXOFFSET + 1]) << (0 * 8));
	}
	s16 get_bb_y_offset() const
	{
		return
				(u16(m_ptr[OFFS_BBYOFFSET + 0]) << (1 * 8)) |
				(u16(m_ptr[OFFS_BBYOFFSET + 1]) << (0 * 8));
	}
	u16 get_bb_width() const
	{
		return
				(u16(m_ptr[OFFS_BBWIDTH + 0]) << (1 * 8)) |
				(u16(m_ptr[OFFS_BBWIDTH + 1]) << (0 * 8));
	}
	u16 get_bb_height() const
	{
		return
				(u16(m_ptr[OFFS_BBHEIGHT + 0]) << (1 * 8)) |
				(u16(m_ptr[OFFS_BBHEIGHT + 1]) << (0 * 8));
	}

	void set_encoding(u32 value)
	{
		m_ptr[OFFS_ENCODING + 0] = u8((value >> (3 * 8)) & 0x00ff);
		m_ptr[OFFS_ENCODING + 1] = u8((value >> (2 * 8)) & 0x00ff);
		m_ptr[OFFS_ENCODING + 2] = u8((value >> (1 * 8)) & 0x00ff);
		m_ptr[OFFS_ENCODING + 3] = u8((value >> (0 * 8)) & 0x00ff);
	}
	void set_x_advance(u16 value)
	{
		m_ptr[OFFS_XADVANCE + 0] = u8((value >> (1 * 8)) & 0x00ff);
		m_ptr[OFFS_XADVANCE + 1] = u8((value >> (0 * 8)) & 0x00ff);
	}
	void set_bb_x_offset(s16 value)
	{
		m_ptr[OFFS_BBXOFFSET + 0] = u8((value >> (1 * 8)) & 0x00ff);
		m_ptr[OFFS_BBXOFFSET + 1] = u8((value >> (0 * 8)) & 0x00ff);
	}
	void set_bb_y_offset(s16 value)
	{
		m_ptr[OFFS_BBYOFFSET + 0] = u8((value >> (1 * 8)) & 0x00ff);
		m_ptr[OFFS_BBYOFFSET + 1] = u8((value >> (0 * 8)) & 0x00ff);
	}
	void set_bb_width(u16 value)
	{
		m_ptr[OFFS_BBWIDTH + 0] = u8((value >> (1 * 8)) & 0x00ff);
		m_ptr[OFFS_BBWIDTH + 1] = u8((value >> (0 * 8)) & 0x00ff);
	}
	void set_bb_height(u16 value)
	{
		m_ptr[OFFS_BBHEIGHT + 0] = u8((value >> (1 * 8)) & 0x00ff);
		m_ptr[OFFS_BBHEIGHT + 1] = u8((value >> (0 * 8)) & 0x00ff);
	}

	bdc_table_entry &operator=(bdc_table_entry const &that) = default;
	bdc_table_entry &operator=(bdc_table_entry &&that) = default;

	static std::size_t size()
	{
		return OFFS_END;
	}

private:
	static constexpr std::size_t    OFFS_ENCODING   = 0x00; // 0x04 bytes (big-endian binary integer)
	static constexpr std::size_t    OFFS_XADVANCE   = 0x04; // 0x02 bytes (big-endian binary integer)
	// two bytes reserved
	static constexpr std::size_t    OFFS_BBXOFFSET  = 0x08; // 0x02 bytes (big-endian binary integer)
	static constexpr std::size_t    OFFS_BBYOFFSET  = 0x0a; // 0x02 bytes (big-endian binary integer)
	static constexpr std::size_t    OFFS_BBWIDTH    = 0x0c; // 0x02 bytes (big-endian binary integer)
	static constexpr std::size_t    OFFS_BBHEIGHT   = 0x0e; // 0x02 bytes (big-endian binary integer)
	static constexpr std::size_t    OFFS_END        = 0x10;

	u8                              *m_ptr;
};

} // anonymous namespace


std::string convert_command_glyph(std::string_view str)
{
	std::vector<char> buf(2 * (str.length() + 1));
	std::size_t j(0);
	while (!str.empty())
	{
		// decode UTF-8
		char32_t uchar;
		int const codelen(uchar_from_utf8(&uchar, str));
		if (0 >= codelen)
			break;
		str.remove_prefix(codelen);

		// check for three metacharacters
		fix_command_t const *fixcmd(nullptr);
		switch (uchar)
		{
		case COMMAND_CONVERT_TEXT:
			for (fix_strings_t const *fixtext = convert_text; fixtext->glyph_code; ++fixtext)
			{
				if (str.substr(0, fixtext->glyph_str.length()) == fixtext->glyph_str)
				{
					uchar = fixtext->glyph_code + COMMAND_UNICODE;
					str.remove_prefix(fixtext->glyph_str.length());
					break;
				}
			}
			break;

		case COMMAND_DEFAULT_TEXT:
			fixcmd = default_text;
			break;

		case COMMAND_EXPAND_TEXT:
			fixcmd = expand_text;
			break;
		}

		// this substitutes a single character
		if (fixcmd && !str.empty())
		{
			if (str[0] == uchar)
			{
				str.remove_prefix(1);
			}
			else
			{
				while (fixcmd->glyph_code && !str.empty() && fixcmd->glyph_char != str[0])
					++fixcmd;
				if (fixcmd->glyph_code && !str.empty())
				{
					uchar = COMMAND_UNICODE + fixcmd->glyph_code;
					str.remove_prefix(1);
				}
			}
		}

		// copy character to output
		int const outlen(utf8_from_uchar(&buf[j], buf.size() - j, uchar));
		if (0 >= outlen)
			break;
		j += outlen;
	}
	return std::string(&buf[0], j);
}


const u64 render_font::CACHED_BDF_HASH_SIZE;

//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  next_line - return a pointer to the start of
//  the next line
//-------------------------------------------------

inline const char *next_line(const char *ptr)
{
	// scan forward until we hit the end or a carriage return
	while (*ptr != 13 && *ptr != 10 && *ptr != 0) ptr++;

	// if we hit the end, return nullptr
	if (*ptr == 0)
		return nullptr;

	// eat the trailing linefeed if present
	if (*++ptr == 10)
		ptr++;
	return ptr;
}


//-------------------------------------------------
//  get_char - return a pointer to a character
//  in a font, expanding if necessary
//-------------------------------------------------

inline render_font::glyph &render_font::get_char(char32_t chnum)
{
	static glyph dummy_glyph;

	unsigned const page(chnum / 256);
	if (page >= std::size(m_glyphs))
	{
		if ((0 <= m_defchar) && (chnum != m_defchar))
			return get_char(m_defchar);
		else
			return dummy_glyph;
	}
	else if (!m_glyphs[page])
	{
		//mamep: make table for command glyph
		if ((m_format == format::OSD) || ((chnum >= COMMAND_UNICODE) && (chnum < COMMAND_UNICODE + MAX_GLYPH_FONT)))
			m_glyphs[page] = new glyph[256];
		else if ((0 <= m_defchar) && (chnum != m_defchar))
			return get_char(m_defchar);
		else
			return dummy_glyph;
	}

	// if the character isn't generated yet, do it now
	glyph &gl = m_glyphs[page][chnum % 256];
	if (!gl.bitmap.valid())
	{
		//mamep: command glyph support
		if (m_height_cmd && chnum >= COMMAND_UNICODE && chnum < COMMAND_UNICODE + MAX_GLYPH_FONT)
		{
			glyph &glyph_ch = m_glyphs_cmd[page][chnum % 256];
			float scale = float(m_height) / float(m_height_cmd);
			if (m_format == format::OSD)
				scale *= 0.90f;

			if (!glyph_ch.bitmap.valid())
				char_expand(chnum, glyph_ch);

			//mamep: for color glyph
			gl.color = glyph_ch.color;

			gl.width = int(glyph_ch.width * scale + 0.5f);
			gl.xoffs = int(glyph_ch.xoffs * scale + 0.5f);
			gl.yoffs = int(glyph_ch.yoffs * scale + 0.5f);
			gl.bmwidth = int(glyph_ch.bmwidth * scale + 0.5f);
			gl.bmheight = int(glyph_ch.bmheight * scale + 0.5f);

			gl.bitmap.allocate(gl.bmwidth, gl.bmheight);
			rectangle clip(
					0, glyph_ch.bitmap.width() - 1,
					0, glyph_ch.bitmap.height() - 1);
			render_texture::hq_scale(gl.bitmap, glyph_ch.bitmap, clip, nullptr);

			/* wrap a texture around the bitmap */
			gl.texture = m_manager.texture_alloc(render_texture::hq_scale);
			gl.texture->set_bitmap(gl.bitmap, gl.bitmap.cliprect(), TEXFORMAT_ARGB32);
		}
		else
		{
			char_expand(chnum, gl);
		}
	}

	return gl;
}



//**************************************************************************
//  RENDER FONT
//**************************************************************************

//-------------------------------------------------
//  render_font - constructor
//-------------------------------------------------

render_font::render_font(render_manager &manager, const char *filename)
	: m_manager(manager)
	, m_format(format::UNKNOWN)
	, m_height(0)
	, m_yoffs(0)
	, m_defchar(-1)
	, m_scale(1.0f)
	, m_rawsize(0)
	, m_osdfont()
	, m_height_cmd(0)
	, m_yoffs_cmd(0)
{
	memset(m_glyphs, 0, sizeof(m_glyphs));
	memset(m_glyphs_cmd, 0, sizeof(m_glyphs_cmd));

	// if this is an OSD font, we're done
	if (filename)
	{
		m_osdfont = manager.machine().osd().font_alloc();
		if (m_osdfont && m_osdfont->open(manager.machine().options().font_path(), filename, m_height))
		{
			m_scale = 1.0f / float(m_height);
			m_format = format::OSD;

			//mamep: allocate command glyph font
			render_font_command_glyph();
			return;
		}
		m_osdfont.reset();
	}

	// if the filename is 'default' default to 'ui.bdf' for backwards compatibility
	if (filename && !core_stricmp(filename, "default"))
		filename = "ui.bdf";

	// attempt to load an external BDF font first
	if (filename && load_cached_bdf(filename))
	{
		//mamep: allocate command glyph font
		render_font_command_glyph();
		return;
	}

	// load the compiled in data instead
	util::random_read::ptr ramfile = util::ram_read(font_uismall, sizeof(font_uismall));
	if (ramfile)
		load_cached(*ramfile, 0, 0);
	render_font_command_glyph();
}


//-------------------------------------------------
//  ~render_font - destructor
//-------------------------------------------------

render_font::~render_font()
{
	// free all the subtables
	for (auto & elem : m_glyphs)
		if (elem)
		{
			for (unsigned int charnum = 0; charnum < 256; charnum++)
			{
				glyph &gl = elem[charnum];
				m_manager.texture_free(gl.texture);
			}
			delete[] elem;
		}

	for (auto & elem : m_glyphs_cmd)
		if (elem)
		{
			for (unsigned int charnum = 0; charnum < 256; charnum++)
			{
				glyph &gl = elem[charnum];
				m_manager.texture_free(gl.texture);
			}
			delete[] elem;
		}
}


//-------------------------------------------------
//  char_expand - expand the raw data for a
//  character into a bitmap
//-------------------------------------------------

void render_font::char_expand(char32_t chnum, glyph &gl)
{
	LOG("render_font::char_expand: expanding character %u\n", unsigned(chnum));

	rgb_t const fgcol(gl.color ? gl.color : rgb_t(0xff, 0xff, 0xff, 0xff));
	rgb_t const bgcol(0x00, 0xff, 0xff, 0xff);
	bool const is_cmd((chnum >= COMMAND_UNICODE) && (chnum < COMMAND_UNICODE + MAX_GLYPH_FONT));

	if (is_cmd)
	{
		// abort if nothing there
		if (gl.bmwidth == 0 || gl.bmheight == 0 || gl.rawdata == nullptr)
			return;

		// allocate a new bitmap of the size we need
		gl.bitmap.allocate(gl.bmwidth, m_height_cmd);
		gl.bitmap.fill(0);

		// extract the data
		const char *ptr = gl.rawdata;
		u8 accum = 0, accumbit = 7;
		for (int y = 0; y < gl.bmheight; y++)
		{
			int desty = y + m_height_cmd + m_yoffs_cmd - gl.yoffs - gl.bmheight;
			u32 *dest = (desty >= 0 && desty < m_height_cmd) ? &gl.bitmap.pix(desty, 0) : nullptr;
			{
				for (int x = 0; x < gl.bmwidth; x++)
				{
					if (accumbit == 7)
						accum = *ptr++;
					if (dest != nullptr)
						*dest++ = (accum & (1 << accumbit)) ? fgcol : bgcol;
					accumbit = (accumbit - 1) & 7;
				}
			}
		}
	}
	else if (m_format == format::OSD)
	{
		// if we're an OSD font, query the info
		if (0 > gl.bmwidth)
		{
			// we set bmwidth to -1 if we've previously queried and failed
			LOG("render_font::char_expand: previously failed to get bitmap from OSD font\n");
			return;
		}
		if (!m_osdfont->get_bitmap(chnum, gl.bitmap, gl.width, gl.xoffs, gl.yoffs))
		{
			// attempt to get the font bitmap failed - set bmwidth to -1
			LOG("render_font::char_expand: get bitmap from OSD font failed\n");
			gl.bitmap.reset();
			gl.bmwidth = -1;
			return;
		}
		else
		{
			// populate the bmwidth/bmheight fields
			LOG("render_font::char_expand: got %dx%d bitmap from OSD font\n", gl.bitmap.width(), gl.bitmap.height());
			gl.bmwidth = gl.bitmap.width();
			gl.bmheight = gl.bitmap.height();
		}
	}
	else if (!gl.bmwidth || !gl.bmheight || !gl.rawdata)
	{
		// abort if nothing there
		LOG("render_font::char_expand: empty bitmap bounds or no raw data\n");
		return;
	}
	else
	{
		// other formats need to parse their data
		LOG("render_font::char_expand: building bitmap from raw data\n");

		// allocate a new bitmap of the size we need
		gl.bitmap.allocate(gl.bmwidth, m_height);
		gl.bitmap.fill(0);

		// extract the data
		const char *ptr = gl.rawdata;
		u8 accum(0), accumbit(7);
		for (int y = 0; y < gl.bmheight; ++y)
		{
			int const desty(y + m_height + m_yoffs - gl.yoffs - gl.bmheight);
			u32 *dest(((0 <= desty) && (m_height > desty)) ? &gl.bitmap.pix(desty) : nullptr);

			if (m_format == format::TEXT)
			{
				if (dest)
				{
					for (int x = 0; gl.bmwidth > x; )
					{
						// scan for the next hex digit
						int bits = -1;
						while (('\r' != *ptr) && ('\n' != *ptr) && (0 > bits))
						{
							if (*ptr >= '0' && *ptr <= '9')
								bits = *ptr++ - '0';
							else if (*ptr >= 'A' && *ptr <= 'F')
								bits = *ptr++ - 'A' + 10;
							else if (*ptr >= 'a' && *ptr <= 'f')
								bits = *ptr++ - 'a' + 10;
							else
								ptr++;
						}

						// expand the four bits
						*dest++ = (bits & 8) ? fgcol : bgcol;
						if (gl.bmwidth > ++x)
							*dest++ = (bits & 4) ? fgcol : bgcol;
						if (gl.bmwidth > ++x)
							*dest++ = (bits & 2) ? fgcol : bgcol;
						if (gl.bmwidth > ++x)
							*dest++ = (bits & 1) ? fgcol : bgcol;
						++x;
					}
				}

				// advance to the next line
				ptr = next_line(ptr);
			}
			else if (m_format == format::CACHED)
			{
				for (int x = 0; x < gl.bmwidth; x++)
				{
					if (accumbit == 7)
						accum = *ptr++;
					if (dest != nullptr)
						*dest++ = (accum & (1 << accumbit)) ? fgcol : bgcol;
					accumbit = (accumbit - 1) & 7;
				}
			}
		}
	}

	// wrap a texture around the bitmap
	gl.texture = m_manager.texture_alloc(render_texture::hq_scale);
	gl.texture->set_bitmap(gl.bitmap, gl.bitmap.cliprect(), TEXFORMAT_ARGB32);
}


//-------------------------------------------------
//  get_char_texture_and_bounds - return the
//  texture for a character and compute the
//  bounds of the final bitmap
//-------------------------------------------------

render_texture *render_font::get_char_texture_and_bounds(float height, float aspect, char32_t chnum, render_bounds &bounds)
{
	glyph &gl = get_char(chnum);

	// on entry, assume x0,y0 are the top,left coordinate of the cell and add
	// the character bounding box to that position
	float scale = m_scale * height;
	bounds.x0 += float(gl.xoffs) * scale * aspect;

	// compute x1,y1 from there based on the bitmap size
	bounds.x1 = bounds.x0 + float(gl.bmwidth) * scale * aspect;
	bounds.y1 = bounds.y0 + float(m_height) * scale;

	// return the texture
	return gl.texture;
}


//-------------------------------------------------
//  get_scaled_bitmap_and_bounds - return a
//  scaled bitmap and bounding rect for a char
//-------------------------------------------------

void render_font::get_scaled_bitmap_and_bounds(bitmap_argb32 &dest, float height, float aspect, char32_t chnum, rectangle &bounds)
{
	glyph &gl = get_char(chnum);

	// on entry, assume x0,y0 are the top,left coordinate of the cell and add
	// the character bounding box to that position
	float scale = m_scale * height;
	bounds.min_x = float(gl.xoffs) * scale * aspect;
	bounds.min_y = 0;

	// compute x1,y1 from there based on the bitmap size
	bounds.set_width(float(gl.bmwidth) * scale * aspect);
	bounds.set_height(float(m_height) * scale);

	// if the bitmap isn't big enough, bail
	if (dest.width() < bounds.width() || dest.height() < bounds.height())
		return;

	// if no texture, fill the target
	if (gl.texture == nullptr)
	{
		dest.fill(0);
		return;
	}

	// scale the font
	bitmap_argb32 tempbitmap(&dest.pix(0), bounds.width(), bounds.height(), dest.rowpixels());
	render_texture::hq_scale(tempbitmap, gl.bitmap, gl.bitmap.cliprect(), nullptr);
}


//-------------------------------------------------
//  char_width - return the width of a character
//  at the given height
//-------------------------------------------------

float render_font::char_width(float height, float aspect, char32_t ch)
{
	return float(get_char(ch).width) * m_scale * height * aspect;
}


//-------------------------------------------------
//  string_width - return the width of a string
//  at the given height
//-------------------------------------------------

float render_font::string_width(float height, float aspect, std::string_view string)
{
	// loop over the string and accumulate widths
	int totwidth = 0;

	char32_t schar;

	// loop over characters
	while (!string.empty())
	{
		int scharcount = uchar_from_utf8(&schar, string);
		totwidth += get_char(schar).width;
		string.remove_prefix(scharcount);
	}


	// scale the final result based on height
	return float(totwidth) * m_scale * height * aspect;
}


//-------------------------------------------------
//  utf8string_width - return the width of a
//  UTF8-encoded string at the given height
//-------------------------------------------------

float render_font::utf8string_width(float height, float aspect, std::string_view utf8string)
{
	// loop over the string and accumulate widths
	s32 totwidth = 0;
	while (!utf8string.empty())
	{
		char32_t uchar;
		int count = uchar_from_utf8(&uchar, utf8string);
		if (count < 0)
			break;

		totwidth += get_char(uchar).width;
		utf8string.remove_prefix(count);
	}

	// scale the final result based on height
	return float(totwidth) * m_scale * height * aspect;
}


//-------------------------------------------------
//  load_cached_bdf - attempt to load a cached
//  version of the BDF font 'filename'; if that
//  fails, fall back on the regular BDF loader
//  and create a new cached version
//-------------------------------------------------

bool render_font::load_cached_bdf(std::string_view filename)
{
	std::error_condition filerr;
	u32 chunk;
	std::size_t bytes;

	// first try to open the BDF itself
	emu_file file(m_manager.machine().options().font_path(), OPEN_FLAG_READ);
	filerr = file.open(filename);
	if (filerr)
		return false;

	// determine the file size and allocate memory
	try
	{
		m_rawsize = file.size();
		std::vector<char>::size_type const sz(m_rawsize + 1);
		if (u64(sz) != (m_rawsize + 1))
			return false;
		m_rawdata.resize(sz);
	}
	catch (...)
	{
		return false;
	}

	// read the first chunk and hash it
	chunk = u32((std::min<u64>)(CACHED_BDF_HASH_SIZE, m_rawsize));
	bytes = file.read(&m_rawdata[0], chunk);
	if (bytes != chunk)
	{
		m_rawdata.clear();
		return false;
	}
	u32 const hash(core_crc32(0, reinterpret_cast<u8 const *>(&m_rawdata[0]), bytes));

	// create the cached filename, changing the 'F' to a 'C' on the extension
	std::string cachedname(filename, 0, filename.length() - ((4U < filename.length()) && core_filename_ends_with(filename, ".bdf") ? 4 : 0));
	cachedname.append(".bdc");

	// attempt to open the cached version of the font
	{
		emu_file cachefile(m_manager.machine().options().font_path(), OPEN_FLAG_READ);
		filerr = cachefile.open(cachedname);
		if (!filerr)
		{
			// if we have a cached version, load it
			bool const result = load_cached(cachefile, m_rawsize, hash);

			// if that worked, we're done
			if (result)
				return true;
		}
	}

	// read in the rest of the font and NUL-terminate it
	while (bytes < m_rawsize)
	{
		chunk = u32((std::min<u64>)(std::numeric_limits<u32>::max(), m_rawsize - bytes));
		u32 const read(file.read(&m_rawdata[bytes], chunk));
		bytes += read;
		if (read != chunk)
		{
			m_rawdata.clear();
			return false;
		}
	}
	m_rawdata[m_rawsize] = '\0';

	// load the BDF
	bool result = load_bdf();

	// if we loaded okay, create a cached one
	if (result)
	{
		osd_printf_info("Generating cached BDF font...\n");

		// attempt to open the file
		emu_file cachefile(m_manager.machine().options().font_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);
		filerr = cachefile.open(cachedname);
		if (filerr)
			result = false;
		else
		{
			result = save_cached(cachefile, m_rawsize, hash);
			if (!result)
				cachefile.remove_on_close();
		}
	}
	else
		m_rawdata.clear();

	// close the file
	return result;
}


//-------------------------------------------------
//  load_bdf - parse and load a BDF font
//-------------------------------------------------

bool render_font::load_bdf()
{
	// set the format to text
	m_format = format::TEXT;

	bdf_helper<std::vector<char>::const_iterator> helper(std::cbegin(m_rawdata), std::cend(m_rawdata));

	// the first thing we want to see is the STARTFONT declaration, failing that we can't do much
	for ( ; !helper.is_keyword("STARTFONT"); helper.next_line())
	{
		if (helper.at_end())
		{
			osd_printf_error("render_font::load_bdf: expected STARTFONT\n");
			return false;
		}
	}

	// parse out the global information we need
	bool have_bbox(false);
	bool have_properties(false);
	bool have_defchar(false);
	for (helper.next_line(); !helper.is_keyword("CHARS"); helper.next_line())
	{
		if (helper.at_end())
		{
			// font with no characters is useless
			osd_printf_error("render_font::load_bdf: no glyph section found\n");
			return false;
		}
		else if (helper.is_keyword("FONTBOUNDINGBOX"))
		{
			// check for duplicate bounding box
			if (have_bbox)
			{
				osd_printf_error("render_font::load_bdf: found additional bounding box \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
				return false;
			}
			have_bbox = true;

			// parse bounding box and check that it's at least half sane
			int width, xoffs;
			if (4 == sscanf(&*helper.value_begin(), "%d %d %d %d", &width, &m_height, &xoffs, &m_yoffs))
			{
				LOG("render_font::load_bdf: got bounding box %dx%d %d,%d\n", width, m_height, xoffs, m_yoffs);
				if ((0 >= m_height) || (0 >= width))
				{
					osd_printf_error("render_font::load_bdf: bounding box is invalid\n");
					return false;
				}
			}
			else
			{
				osd_printf_error("render_font::load_bdf: failed to parse bounding box \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
				return false;
			}
		}
		else if (helper.is_keyword("STARTPROPERTIES"))
		{
			// check for duplicated properties section
			if (have_properties)
			{
				osd_printf_error("render_font::load_bdf: found additional properties\n");
				return false;
			}
			have_properties = true;

			// get property count for sanity check
			int propcount;
			if (1 != sscanf(&*helper.value_begin(), "%d", &propcount))
			{
				osd_printf_error("render_font::load_bdf: failed to parse property count \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
				return false;
			}

			int actual(0);
			for (helper.next_line(); !helper.is_keyword("ENDPROPERTIES"); helper.next_line())
			{
				++actual;
				if (helper.at_end())
				{
					// unterminated properties section
					osd_printf_error("render_font::load_bdf: end of properties not found\n");
					return false;
				}
				else if (helper.is_keyword("DEFAULT_CHAR"))
				{
					// check for duplicate default character
					if (have_defchar)
					{
						osd_printf_error("render_font::load_bdf: found additional default character \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
						return false;
					}
					have_defchar = true;

					// parse default character
					if (1 == sscanf(&*helper.value_begin(), "%d", &m_defchar))
					{
						LOG("render_font::load_bdf: got default character 0x%x\n", m_defchar);
					}
					else
					{
						osd_printf_error("render_font::load_bdf: failed to parse default character \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
						return false;
					}
				}
			}

			// sanity check on number of properties
			if (actual != propcount)
			{
				osd_printf_error("render_font::load_bdf: incorrect number of properties %d\n", actual);
				return false;
			}
		}
	}

	// compute the scale factor
	if (!have_bbox)
	{
		osd_printf_error("render_font::load_bdf: no bounding box found\n");
		return false;
	}
	m_scale = 1.0f / float(m_height);

	// get expected character count
	int expected;
	if (1 == sscanf(&*helper.value_begin(), "%d", &expected))
	{
		LOG("render_font::load_bdf: got character count %d\n", expected);
	}
	else
	{
		osd_printf_error("render_font::load_bdf: failed to parse character count \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
		return false;
	}

	// now scan for characters
	auto const nothex([] (char ch) { return (('0' > ch) || ('9' < ch)) && (('A' > ch) || ('Z' < ch)) && (('a' > ch) || ('z' < ch)); });
	int charcount = 0;
	for (helper.next_line(); !helper.is_keyword("ENDFONT"); helper.next_line())
	{
		if (helper.at_end())
		{
			// unterminated font
			osd_printf_error("render_font::load_bdf: end of font not found\n");
			return false;
		}
		else if (helper.is_keyword("STARTCHAR"))
		{
			// required glyph properties
			bool have_encoding(false);
			bool have_advance(false);
			bool have_bbounds(false);
			int encoding(-1);
			int xadvance(-1);
			int bbw(-1), bbh(-1), bbxoff(-1), bbyoff(-1);

			// stuff for the bitmap data
			bool in_bitmap(false);
			int bitmap_rows(0);
			char const *bitmap_data(nullptr);

			// parse a glyph
			for (helper.next_line(); !helper.is_keyword("ENDCHAR"); helper.next_line())
			{
				if (helper.at_end())
				{
					// unterminated glyph
					osd_printf_error("render_font::load_bdf: end of glyph not found\n");
					return false;
				}
				else if (in_bitmap)
				{
					// quick sanity check
					if ((2 * ((bbw + 7) / 8)) != helper.keyword_length())
					{
						osd_printf_error("render_font::load_bdf: incorrect length for bitmap line \"%.*s\"\n", int(helper.keyword_length()), &*helper.keyword_begin());
						return false;
					}
					else if (std::find_if(helper.keyword_begin(), helper.keyword_end(), nothex) != helper.keyword_end())
					{
						osd_printf_error("render_font::load_bdf: found invalid character in bitmap line \"%.*s\"\n", int(helper.keyword_length()), &*helper.keyword_begin());
						return false;
					}

					// track number of rows
					if (1 == ++bitmap_rows)
						bitmap_data = &*helper.keyword_begin();

				}
				else if (helper.is_keyword("ENCODING"))
				{
					// check for duplicate glyph encoding
					if (have_encoding)
					{
						osd_printf_error("render_font::load_bdf: found additional glyph encoding \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
						return false;
					}
					have_encoding = true;

					// need to support Adobe Standard Encoding "123" and non-standard glyph index "-1 123"
					std::string const value(helper.value_begin(), helper.value_end());
					int aux;
					int const cnt(sscanf(value.c_str(), "%d %d", &encoding, &aux));
					if ((2 == cnt) && (-1 == encoding) && (0 <= aux))
					{
						encoding = aux;
					}
					else if ((1 != cnt) || (0 > encoding))
					{
						osd_printf_error("render_font::load_bdf: failed to parse glyph encoding \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
						return false;
					}
					LOG("render_font::load_bdf: got glyph encoding %d\n", encoding);
				}
				else if (helper.is_keyword("DWIDTH"))
				{
					// check for duplicate advance
					if (have_advance)
					{
						osd_printf_error("render_font::load_bdf: found additional pixel width \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
						return false;
					}
					have_advance = true;

					// completely ignore vertical advance
					int yadvance;
					if (2 == sscanf(&*helper.value_begin(), "%d %d", &xadvance, &yadvance))
					{
						LOG("render_font::load_bdf: got pixel width %d,%d\n", xadvance, yadvance);
					}
					else
					{
						osd_printf_error("render_font::load_bdf: failed to parse pixel width \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
						return false;
					}
				}
				else if (helper.is_keyword("BBX"))
				{
					// check for duplicate black pixel box
					if (have_bbounds)
					{
						osd_printf_error("render_font::load_bdf: found additional pixel width \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
						return false;
					}
					have_bbounds = true;

					// extract position/size of black pixel area
					if (4 == sscanf(&*helper.value_begin(), "%d %d %d %d", &bbw, &bbh, &bbxoff, &bbyoff))
					{
						LOG("render_font::load_bdf: got black pixel box %dx%d %d,%d\n", bbw, bbh, bbxoff, bbyoff);
						if ((0 > bbw) || (0 > bbh))
						{
							osd_printf_error("render_font::load_bdf: black pixel box is invalid\n");
							return false;
						}
					}
					else
					{
						osd_printf_error("render_font::load_bdf: failed to parse black pixel box \"%.*s\"\n", int(helper.value_length()), &*helper.value_begin());
						return false;
					}
				}
				else if (helper.is_keyword("BITMAP"))
				{
					// this is the bitmap - we need to already have properties before we get here
					if (!have_advance)
					{
						osd_printf_error("render_font::load_bdf: glyph has no pixel width\n");
						return false;
					}
					else if (!have_bbounds)
					{
						osd_printf_error("render_font::load_bdf: glyph has no black pixel box\n");
						return false;
					}
					in_bitmap = true;
				}
			}

			// now check that we have what we need
			if (!in_bitmap)
			{
				osd_printf_error("render_font::load_bdf: glyph has no bitmap\n");
				return false;
			}
			else if (bitmap_rows != bbh)
			{
				osd_printf_error("render_font::load_bdf: incorrect number of bitmap lines %d\n", bitmap_rows);
				return false;
			}

			// some kinds of characters will screw us up
			if (0 > xadvance)
			{
				LOG("render_font::load_bdf: ignoring character with negative x advance\n");
			}
			else if ((256 * std::size(m_glyphs)) <= encoding)
			{
				LOG("render_font::load_bdf: ignoring character with encoding outside range\n");
			}
			else
			{
				// if we don't have a subtable yet, make one
				if (!m_glyphs[encoding / 256])
				{
					try
					{
						m_glyphs[encoding / 256] = new glyph[256];
					}
					catch (...)
					{
						osd_printf_error("render_font::load_bdf: allocation failed\n");
						return false;
					}
				}

				// fill in the entry
				glyph &gl = m_glyphs[encoding / 256][encoding % 256];
				gl.width = xadvance;
				gl.bmwidth = bbw;
				gl.bmheight = bbh;
				gl.xoffs = bbxoff;
				gl.yoffs = bbyoff;
				gl.rawdata = bitmap_data;
			}

			// some progress for big fonts
			if (0 == (++charcount % 256))
				osd_printf_info("Loading BDF font... (%d characters loaded)\n", charcount);
		}
	}

	// check number of characters
	if (expected != charcount)
	{
		osd_printf_error("render_font::load_bdf: incorrect character count %d\n", charcount);
		return false;
	}

	// should have bailed by now if something went wrong
	return true;
}


//-------------------------------------------------
//  load_cached - load a font in cached format
//-------------------------------------------------

bool render_font::load_cached(util::random_read &file, u64 length, u32 hash)
{
	// get the file size, read the header, and check that it looks good
	u64 filesize;
	bdc_header header;
	if (file.length(filesize))
	{
		LOG("render_font::load_cached: error determining size of BDC file\n");
		return false;
	}
	else if (!header.read(file))
	{
		osd_printf_warning("render_font::load_cached: error reading BDC header\n");
		return false;
	}
	else if (!header.check_magic() || (bdc_header::MAJVERSION != header.get_major_version()) || (bdc_header::MINVERSION != header.get_minor_version()))
	{
		LOG("render_font::load_cached: incompatible BDC file\n");
		return false;
	}
	else if (length && ((header.get_original_length() != length) || (header.get_original_hash() != hash)))
	{
		LOG("render_font::load_cached: BDC file does not match original BDF file\n");
		return false;
	}

	// get global properties from the header
	m_height = header.get_height();
	m_scale = 1.0f / float(m_height);
	m_yoffs = header.get_y_offset();
	m_defchar = header.get_default_character();
	u32 const numchars(header.get_glyph_count());
	u64 filepos;
	if (file.tell(filepos))
	{
		LOG("render_font::load_cached: failed to determine position in BDC file\n");
		return false;
	}
	else if ((filepos + (u64(numchars) * bdc_table_entry::size())) > filesize)
	{
		LOG("render_font::load_cached: BDC file is too small to hold glyph table\n");
		return false;
	}

	// now read the rest of the data
	u64 const remaining(filesize - filepos);
	try
	{
		m_rawdata.resize(std::size_t(remaining));
	}
	catch (...)
	{
		osd_printf_error("render_font::load_cached: allocation error\n");
	}
	for (u64 bytes_read = 0; remaining > bytes_read; )
	{
		u32 const chunk((std::min)(u64(std::numeric_limits<u32>::max()), remaining));
		std::size_t bytes(0);
		if (file.read(&m_rawdata[bytes_read], chunk, bytes) || bytes != chunk)
		{
			osd_printf_error("render_font::load_cached: error reading BDC data\n");
			m_rawdata.clear();
			return false;
		}
		bytes_read += chunk;
	}

	// extract the data from the data
	std::size_t offset(std::size_t(numchars) * bdc_table_entry::size());
	bdc_table_entry entry(m_rawdata.empty() ? nullptr : &m_rawdata[0]);
	for (unsigned chindex = 0; chindex < numchars; chindex++, entry = entry.get_next())
	{
		// if we don't have a subtable yet, make one
		int const chnum(entry.get_encoding());
		LOG("render_font::load_cached: loading character %d\n", chnum);
		if (!m_glyphs[chnum / 256])
		{
			try
			{
				m_glyphs[chnum / 256] = new glyph[256];
			}
			catch (...)
			{
				osd_printf_error("render_font::load_cached: allocation error\n");
				m_rawdata.clear();
				return false;
			}
		}

		// fill in the entry
		glyph &gl = m_glyphs[chnum / 256][chnum % 256];
		gl.width = entry.get_x_advance();
		gl.xoffs = entry.get_bb_x_offset();
		gl.yoffs = entry.get_bb_y_offset();
		gl.bmwidth = entry.get_bb_width();
		gl.bmheight = entry.get_bb_height();
		gl.rawdata = &m_rawdata[offset];

		// advance the offset past the character
		offset += (gl.bmwidth * gl.bmheight + 7) / 8;
		if (m_rawdata.size() < offset)
		{
			osd_printf_verbose("render_font::load_cached: BDC file too small to hold all glyphs\n");
			m_rawdata.clear();
			return false;
		}
	}

	// got everything
	m_format = format::CACHED;
	return true;
}


//-------------------------------------------------
//  save_cached - save a font in cached format
//-------------------------------------------------

bool render_font::save_cached(util::random_write &file, u64 length, u32 hash)
{
	// count glyphs
	unsigned numchars = 0;
	for (glyph const *const page : m_glyphs)
	{
		for (unsigned chnum = 0; page && (256 > chnum); ++chnum)
		{
			if (0 < page[chnum].width)
				++numchars;
		}
	}
	LOG("render_font::save_cached: %u glyphs with positive advance to save\n", numchars);

	try
	{
		{
			LOG("render_font::save_cached: writing header\n");
			bdc_header hdr;
			hdr.set_magic();
			hdr.set_version();
			hdr.set_original_length(length);
			hdr.set_original_hash(hash);
			hdr.set_glyph_count(numchars);
			hdr.set_height(m_height);
			hdr.set_y_offset(m_yoffs);
			hdr.set_default_character(m_defchar);
			if (!hdr.write(file))
				throw emu_fatalerror("Error writing cached file");
		}
		u64 table_offs;
		if (file.tell(table_offs))
			throw emu_fatalerror("Error writing cached file");

		// allocate an array to hold the character data
		std::vector<u8> chartable(std::size_t(numchars) * bdc_table_entry::size(), 0);

		// allocate a temp buffer to compress into
		std::vector<u8> tempbuffer(65536);

		// write the empty table to the beginning of the file
		std::size_t bytes_written(0);
		if (file.write(&chartable[0], chartable.size(), bytes_written) || bytes_written != chartable.size())
			throw emu_fatalerror("Error writing cached file");

		// loop over all characters
		bdc_table_entry table_entry(chartable.empty() ? nullptr : &chartable[0]);
		for (unsigned chnum = 0; chnum < (256 * std::size(m_glyphs)); chnum++)
		{
			if (m_glyphs[chnum / 256] && (0 < m_glyphs[chnum / 256][chnum % 256].width))
			{
				LOG("render_font::save_cached: writing glyph %u\n", chnum);
				glyph &gl(get_char(chnum));

				// write out a bit-compressed bitmap if we have one
				if (gl.bitmap.valid())
				{
					// write the data to the tempbuffer
					u8 *dest = &tempbuffer[0];
					u8 accum = 0;
					u8 accbit = 7;

					// bit-encode the character data
					for (int y = 0; y < gl.bmheight; y++)
					{
						int desty = y + m_height + m_yoffs - gl.yoffs - gl.bmheight;
						u32 const *const src = (desty >= 0 && desty < m_height) ? &gl.bitmap.pix(desty) : nullptr;
						for (int x = 0; x < gl.bmwidth; x++)
						{
							if (src != nullptr && rgb_t(src[x]).a() != 0)
								accum |= 1 << accbit;
							if (accbit-- == 0)
							{
								*dest++ = accum;
								accum = 0;
								accbit = 7;
							}
						}
					}

					// flush any extra
					if (accbit != 7)
						*dest++ = accum;

					// write the data
					if (file.write(&tempbuffer[0], dest - &tempbuffer[0], bytes_written) || bytes_written != dest - &tempbuffer[0])
						throw emu_fatalerror("Error writing cached file");

					// free the bitmap and texture
					m_manager.texture_free(gl.texture);
					gl.bitmap.reset();
					gl.texture = nullptr;
				}

				// compute the table entry
				table_entry.set_encoding(chnum);
				table_entry.set_x_advance(gl.width);
				table_entry.set_bb_x_offset(gl.xoffs);
				table_entry.set_bb_y_offset(gl.yoffs);
				table_entry.set_bb_width(gl.bmwidth);
				table_entry.set_bb_height(gl.bmheight);
				table_entry = table_entry.get_next();
			}
		}

		// seek back to the beginning and rewrite the table
		if (!chartable.empty())
		{
			LOG("render_font::save_cached: writing character table\n");
			if (file.seek(table_offs, SEEK_SET))
				return false;
			u8 const *bytes(&chartable[0]);
			for (u64 remaining = chartable.size(); remaining; )
			{
				u32 const chunk((std::min<u64>)(std::numeric_limits<u32>::max(), remaining));
				if (file.write(bytes, chunk, bytes_written) || chunk != bytes_written)
					throw emu_fatalerror("Error writing cached file");
				bytes += chunk;
				remaining -= chunk;
			}
		}

		// no trouble?
		return true;
	}
	catch (...)
	{
		return false;
	}
}


void render_font::render_font_command_glyph()
{
	// FIXME: this is copy/pasta from the BDC loading, and it shouldn't be injected into every font
	util::random_read::ptr file = util::ram_read(font_uicmd14, sizeof(font_uicmd14));
	if (file)
	{
		// get the file size, read the header, and check that it looks good
		u64 const filesize = sizeof(font_uicmd14);
		bdc_header header;
		if (!header.read(*file))
		{
			osd_printf_warning("render_font::render_font_command_glyph: error reading BDC header\n");
			return;
		}
		else if (!header.check_magic() || (bdc_header::MAJVERSION != header.get_major_version()) || (bdc_header::MINVERSION != header.get_minor_version()))
		{
			LOG("render_font::render_font_command_glyph: incompatible BDC file\n");
			return;
		}

		// get global properties from the header
		m_height_cmd = header.get_height();
		m_yoffs_cmd = header.get_y_offset();
		u32 const numchars(header.get_glyph_count());
		u64 filepos;
		if (file->tell(filepos))
		{
			LOG("render_font::render_font_command_glyph: failed to determine position in BDC file\n");
			return;
		}
		else if ((filepos + (u64(numchars) * bdc_table_entry::size())) > filesize)
		{
			LOG("render_font::render_font_command_glyph: BDC file is too small to hold glyph table\n");
			return;
		}

		// now read the rest of the data
		u64 const remaining(filesize - filepos);
		try
		{
			m_rawdata_cmd.resize(std::size_t(remaining));
		}
		catch (...)
		{
			osd_printf_error("render_font::render_font_command_glyph: allocation error\n");
		}
		for (u64 bytes_read = 0; remaining > bytes_read; )
		{
			u32 const chunk((std::min)(u64(std::numeric_limits<u32>::max()), remaining));
			std::size_t bytes(0);
			if (file->read(&m_rawdata_cmd[bytes_read], chunk, bytes) || bytes != chunk)
			{
				osd_printf_error("render_font::render_font_command_glyph: error reading BDC data\n");
				m_rawdata_cmd.clear();
				return;
			}
			bytes_read += chunk;
		}

		// extract the data from the data
		std::size_t offset(std::size_t(numchars) * bdc_table_entry::size());
		bdc_table_entry entry(m_rawdata_cmd.empty() ? nullptr : &m_rawdata_cmd[0]);
		for (unsigned chindex = 0; chindex < numchars; chindex++, entry = entry.get_next())
		{
			// if we don't have a subtable yet, make one
			int const chnum(entry.get_encoding());
			LOG("render_font::render_font_command_glyph: loading character %d\n", chnum);
			if (!m_glyphs_cmd[chnum / 256])
			{
				try
				{
					m_glyphs_cmd[chnum / 256] = new glyph[256];
				}
				catch (...)
				{
					osd_printf_error("render_font::render_font_command_glyph: allocation error\n");
					m_rawdata_cmd.clear();
					return;
				}
			}

			// fill in the entry
			glyph &gl = m_glyphs_cmd[chnum / 256][chnum % 256];
			gl.width = entry.get_x_advance();
			gl.xoffs = entry.get_bb_x_offset();
			gl.yoffs = entry.get_bb_y_offset();
			gl.bmwidth = entry.get_bb_width();
			gl.bmheight = entry.get_bb_height();
			gl.rawdata = &m_rawdata_cmd[offset];

			// advance the offset past the character
			offset += (gl.bmwidth * gl.bmheight + 7) / 8;
			if (m_rawdata_cmd.size() < offset)
			{
				osd_printf_verbose("render_font::render_font_command_glyph: BDC file too small to hold all glyphs\n");
				m_rawdata_cmd.clear();
				return;
			}
		}
	}
}
