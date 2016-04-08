// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    rendfont.c

    Rendering system font management.

***************************************************************************/

#include "emu.h"
#include "rendfont.h"
#include "emuopts.h"
#include "coreutil.h"

#include "osdepend.h"
#include "uismall.fh"

#include "ui/cmdrender.h"

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

	// if we hit the end, return NULL
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

inline render_font::glyph &render_font::get_char(unicode_char chnum)
{
	static glyph dummy_glyph;

	// grab the table; if none, return the dummy character
	if (!m_glyphs[chnum / 256] && m_format == FF_OSD)
		m_glyphs[chnum / 256] = new glyph[256];
	if (!m_glyphs[chnum / 256])
	{
		//mamep: make table for command glyph
		if (chnum >= COMMAND_UNICODE && chnum < COMMAND_UNICODE + MAX_GLYPH_FONT)
			m_glyphs[chnum / 256] = new glyph[256];
		else
			return dummy_glyph;
	}

	// if the character isn't generated yet, do it now
	glyph &gl = m_glyphs[chnum / 256][chnum % 256];
	if (!gl.bitmap.valid())
	{
		//mamep: command glyph support
		if (m_height_cmd && chnum >= COMMAND_UNICODE && chnum < COMMAND_UNICODE + MAX_GLYPH_FONT)
		{
			glyph &glyph_ch = m_glyphs_cmd[chnum / 256][chnum % 256];
			float scale = (float)m_height / (float)m_height_cmd;
			if (m_format == FF_OSD) scale *= 0.90f;

			if (!glyph_ch.bitmap.valid())
				char_expand(chnum, glyph_ch);

			//mamep: for color glyph
			gl.color = glyph_ch.color;

			gl.width = (int)(glyph_ch.width * scale + 0.5f);
			gl.xoffs = (int)(glyph_ch.xoffs * scale + 0.5f);
			gl.yoffs = (int)(glyph_ch.yoffs * scale + 0.5f);
			gl.bmwidth = (int)(glyph_ch.bmwidth * scale + 0.5f);
			gl.bmheight = (int)(glyph_ch.bmheight * scale + 0.5f);

			gl.bitmap.allocate(gl.bmwidth, gl.bmheight);
			rectangle clip;
			clip.min_x = clip.min_y = 0;
			clip.max_x = glyph_ch.bitmap.width() - 1;
			clip.max_y = glyph_ch.bitmap.height() - 1;
			render_texture::hq_scale(gl.bitmap, glyph_ch.bitmap, clip, nullptr);

			/* wrap a texture around the bitmap */
			gl.texture = m_manager.texture_alloc(render_texture::hq_scale);
			gl.texture->set_bitmap(gl.bitmap, gl.bitmap.cliprect(), TEXFORMAT_ARGB32);
		}
		else
			char_expand(chnum, gl);
	}

	// return the resulting character
	return gl;
}



//**************************************************************************
//  RENDER FONT
//**************************************************************************

//-------------------------------------------------
//  render_font - constructor
//-------------------------------------------------

render_font::render_font(render_manager &manager, const char *filename)
	: m_manager(manager),
		m_format(FF_UNKNOWN),
		m_height(0),
		m_yoffs(0),
		m_scale(1.0f),
		m_rawsize(0),
		m_osdfont(),
		m_height_cmd(0),
		m_yoffs_cmd(0)
{
	memset(m_glyphs, 0, sizeof(m_glyphs));
	memset(m_glyphs_cmd, 0, sizeof(m_glyphs_cmd));

	// if this is an OSD font, we're done
	if (filename != nullptr)
	{
		m_osdfont = manager.machine().osd().font_alloc();
		if (m_osdfont)
		{
			if (m_osdfont->open(manager.machine().options().font_path(), filename, m_height))
			{
				m_scale = 1.0f / (float)m_height;
				m_format = FF_OSD;

				//mamep: allocate command glyph font
				render_font_command_glyph();
				return;
			}
			m_osdfont.reset();
		}
	}

	// if the filename is 'default' default to 'ui.bdf' for backwards compatibility
	if (filename != nullptr && core_stricmp(filename, "default") == 0)
		filename = "ui.bdf";

	// attempt to load the cached version of the font first
	if (filename != nullptr && load_cached_bdf(filename))
	{
		//mamep: allocate command glyph font
		render_font_command_glyph();
		return;
	}

	// load the raw data instead
	emu_file ramfile(OPEN_FLAG_READ);
	osd_file::error filerr = ramfile.open_ram(font_uismall, sizeof(font_uismall));
	if (filerr == osd_file::error::NONE)
		load_cached(ramfile, 0);
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

void render_font::char_expand(unicode_char chnum, glyph &gl)
{
	rgb_t color = rgb_t(0xff,0xff,0xff,0xff);
	bool is_cmd = (chnum >= COMMAND_UNICODE && chnum < COMMAND_UNICODE + MAX_GLYPH_FONT);

	if (gl.color)
		color = gl.color;

	if (is_cmd)
	{
		// punt if nothing there
		if (gl.bmwidth == 0 || gl.bmheight == 0 || gl.rawdata == nullptr)
			return;

		// allocate a new bitmap of the size we need
		gl.bitmap.allocate(gl.bmwidth, m_height_cmd);
		gl.bitmap.fill(0);

		// extract the data
		const char *ptr = gl.rawdata;
		UINT8 accum = 0, accumbit = 7;
		for (int y = 0; y < gl.bmheight; y++)
		{
			int desty = y + m_height_cmd + m_yoffs_cmd - gl.yoffs - gl.bmheight;
			UINT32 *dest = (desty >= 0 && desty < m_height_cmd) ? &gl.bitmap.pix32(desty, 0) : nullptr;
			{
				for (int x = 0; x < gl.bmwidth; x++)
				{
					if (accumbit == 7)
						accum = *ptr++;
					if (dest != nullptr)
						*dest++ = (accum & (1 << accumbit)) ? color : rgb_t(0x00,0xff,0xff,0xff);
					accumbit = (accumbit - 1) & 7;
				}
			}
		}
	}
	// if we're an OSD font, query the info
	else if (m_format == FF_OSD)
	{
		// we set bmwidth to -1 if we've previously queried and failed
		if (gl.bmwidth == -1)
			return;

		// attempt to get the font bitmap; if we fail, set bmwidth to -1
		if (!m_osdfont->get_bitmap(chnum, gl.bitmap, gl.width, gl.xoffs, gl.yoffs))
		{
			gl.bitmap.reset();
			gl.bmwidth = -1;
			return;
		}

		// populate the bmwidth/bmheight fields
		gl.bmwidth = gl.bitmap.width();
		gl.bmheight = gl.bitmap.height();
	}
	// other formats need to parse their data
	else
	{
		// punt if nothing there
		if (gl.bmwidth == 0 || gl.bmheight == 0 || gl.rawdata == nullptr)
			return;

		// allocate a new bitmap of the size we need
		gl.bitmap.allocate(gl.bmwidth, m_height);
		gl.bitmap.fill(0);

		// extract the data
		const char *ptr = gl.rawdata;
		UINT8 accum = 0, accumbit = 7;
		for (int y = 0; y < gl.bmheight; y++)
		{
			int desty = y + m_height + m_yoffs - gl.yoffs - gl.bmheight;
			UINT32 *dest = (desty >= 0 && desty < m_height) ? &gl.bitmap.pix32(desty) : nullptr;

			// text format
			if (m_format == FF_TEXT)
			{
				// loop over bytes
				for (int x = 0; x < gl.bmwidth; x += 4)
				{
					// scan for the next hex digit
					int bits = -1;
					while (*ptr != 13 && bits == -1)
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
					if (dest != nullptr)
					{
						*dest++ = (bits & 8) ? color : rgb_t(0x00,0xff,0xff,0xff);
						*dest++ = (bits & 4) ? color : rgb_t(0x00,0xff,0xff,0xff);
						*dest++ = (bits & 2) ? color : rgb_t(0x00,0xff,0xff,0xff);
						*dest++ = (bits & 1) ? color : rgb_t(0x00,0xff,0xff,0xff);
					}
				}

				// advance to the next line
				ptr = next_line(ptr);
			}

			// cached format
			else if (m_format == FF_CACHED)
			{
				for (int x = 0; x < gl.bmwidth; x++)
				{
					if (accumbit == 7)
						accum = *ptr++;
					if (dest != nullptr)
						*dest++ = (accum & (1 << accumbit)) ? color : rgb_t(0x00,0xff,0xff,0xff);
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

render_texture *render_font::get_char_texture_and_bounds(float height, float aspect, unicode_char chnum, render_bounds &bounds)
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

void render_font::get_scaled_bitmap_and_bounds(bitmap_argb32 &dest, float height, float aspect, unicode_char chnum, rectangle &bounds)
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

float render_font::char_width(float height, float aspect, unicode_char ch)
{
	return float(get_char(ch).width) * m_scale * height * aspect;
}


//-------------------------------------------------
//  string_width - return the width of a string
//  at the given height
//-------------------------------------------------

float render_font::string_width(float height, float aspect, const char *string)
{
	// loop over the string and accumulate widths
	int totwidth = 0;

	const char *ends = string + strlen(string);
	const char *s = string;
	unicode_char schar;
	
	// loop over characters
	while (*s != 0)
	{
		int	scharcount = uchar_from_utf8(&schar, s, ends - s);
		totwidth += get_char(schar).width;
		s += scharcount;
	}


	// scale the final result based on height
	return float(totwidth) * m_scale * height * aspect;
}


//-------------------------------------------------
//  utf8string_width - return the width of a
//  UTF8-encoded string at the given height
//-------------------------------------------------

float render_font::utf8string_width(float height, float aspect, const char *utf8string)
{
	int length = strlen(utf8string);

	// loop over the string and accumulate widths
	int count;
	int totwidth = 0;
	for (int offset = 0; offset < length; offset += count)
	{
		unicode_char uchar;
		count = uchar_from_utf8(&uchar, utf8string + offset, length - offset);
		if (count == -1)
			break;
		if (uchar < 0x10000)
			totwidth += get_char(uchar).width;
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

bool render_font::load_cached_bdf(const char *filename)
{
	// first try to open the BDF itself
	emu_file file(manager().machine().options().font_path(), OPEN_FLAG_READ);
	osd_file::error filerr = file.open(filename);
	if (filerr != osd_file::error::NONE)
		return false;

	// determine the file size and allocate memory
	m_rawsize = file.size();
	m_rawdata.resize(m_rawsize + 1);

	// read the first chunk
	UINT32 bytes = file.read(&m_rawdata[0], MIN(CACHED_BDF_HASH_SIZE, m_rawsize));
	if (bytes != MIN(CACHED_BDF_HASH_SIZE, m_rawsize))
		return false;

	// has the chunk
	UINT32 hash = core_crc32(0, (const UINT8 *)&m_rawdata[0], bytes) ^ (UINT32)m_rawsize;

	// create the cached filename, changing the 'F' to a 'C' on the extension
	std::string cachedname(filename);
	cachedname.erase(cachedname.length() - 3, 3).append("bdc");

	// attempt to open the cached version of the font
	{
		emu_file cachefile(manager().machine().options().font_path(), OPEN_FLAG_READ);
		filerr = cachefile.open(cachedname.c_str());
		if (filerr == osd_file::error::NONE)
		{
			// if we have a cached version, load it
			bool result = load_cached(cachefile, hash);

			// if that worked, we're done
			if (result)
			{
				// don't do that - glyphs data point into this array ...
				// m_rawdata.reset();
				return true;
			}
		}
	}

	// read in the rest of the font
	if (bytes < m_rawsize)
	{
		UINT32 read = file.read(&m_rawdata[bytes], m_rawsize - bytes);
		if (read != m_rawsize - bytes)
		{
			m_rawdata.clear();
			return false;
		}
	}

	// NULL-terminate the data and attach it to the font
	m_rawdata[m_rawsize] = 0;

	// load the BDF
	bool result = load_bdf();

	// if we loaded okay, create a cached one
	if (result)
		save_cached(cachedname.c_str(), hash);

	// close the file
	return result;
}


//-------------------------------------------------
//  load_bdf - parse and load a BDF font
//-------------------------------------------------

bool render_font::load_bdf()
{
	// set the format to text
	m_format = FF_TEXT;

	// first find the FONTBOUNDINGBOX tag
	const char *ptr;
	for (ptr = &m_rawdata[0]; ptr != nullptr; ptr = next_line(ptr))
	{
		// we only care about a tiny few fields
		if (strncmp(ptr, "FONTBOUNDINGBOX ", 16) == 0)
		{
			int dummy1, dummy2;
			if (sscanf(ptr + 16, "%d %d %d %d", &dummy1, &m_height, &dummy2, &m_yoffs) != 4)
				return false;
			break;
		}
	}

	// compute the scale factor
	m_scale = 1.0f / (float)m_height;

	// now scan for characters
	int charcount = 0;
	for ( ; ptr != nullptr; ptr = next_line(ptr))
	{
		// stop at ENDFONT
		if (strncmp(ptr, "ENDFONT", 7) == 0)
			break;

		// once we hit a STARTCHAR, parse until the end
		if (strncmp(ptr, "STARTCHAR ", 10) == 0)
		{
			int bmwidth = -1, bmheight = -1, xoffs = -1, yoffs = -1;
			const char *rawdata = nullptr;
			int charnum = -1;
			int width = -1;

			// scan for interesting per-character tags
			for ( ; ptr != nullptr; ptr = next_line(ptr))
			{
				// ENCODING tells us which character
				if (strncmp(ptr, "ENCODING ", 9) == 0)
				{
					if (sscanf(ptr + 9, "%d", &charnum) != 1)
						return 1;
				}

				// DWIDTH tells us the width to the next character
				else if (strncmp(ptr, "DWIDTH ", 7) == 0)
				{
					int dummy1;
					if (sscanf(ptr + 7, "%d %d", &width, &dummy1) != 2)
						return 1;
				}

				// BBX tells us the height/width of the bitmap and the offsets
				else if (strncmp(ptr, "BBX ", 4) == 0)
				{
					if (sscanf(ptr + 4, "%d %d %d %d", &bmwidth, &bmheight, &xoffs, &yoffs) != 4)
						return 1;
				}

				// BITMAP is the start of the data
				else if (strncmp(ptr, "BITMAP", 6) == 0)
				{
					// stash the raw pointer and scan for the end of the character
					for (rawdata = ptr = next_line(ptr); ptr != nullptr && strncmp(ptr, "ENDCHAR", 7) != 0; ptr = next_line(ptr)) { }
					break;
				}
			}

			// if we have everything, allocate a new character
			if (charnum >= 0 && charnum < 65536 && rawdata != nullptr && bmwidth >= 0 && bmheight >= 0)
			{
				// if we don't have a subtable yet, make one
				if (!m_glyphs[charnum / 256])
					m_glyphs[charnum / 256] = new glyph[256];

				// fill in the entry
				glyph &gl = m_glyphs[charnum / 256][charnum % 256];
				gl.width = width;
				gl.bmwidth = bmwidth;
				gl.bmheight = bmheight;
				gl.xoffs = xoffs;
				gl.yoffs = yoffs;
				gl.rawdata = rawdata;
			}

			// some progress for big fonts
			if (++charcount % 256 == 0)
				osd_printf_warning("Loading BDF font... (%d characters loaded)\n", charcount);
		}
	}

	// make sure all the numbers are the same width
	if (m_glyphs[0])
	{
		int maxwidth = 0;
		for (int ch = '0'; ch <= '9'; ch++)
			if (m_glyphs[0][ch].bmwidth > maxwidth)
				maxwidth = m_glyphs[0][ch].width;
		for (int ch = '0'; ch <= '9'; ch++)
			m_glyphs[0][ch].width = maxwidth;
	}

	return true;
}


//-------------------------------------------------
//  load_cached - load a font in cached format
//-------------------------------------------------

bool render_font::load_cached(emu_file &file, UINT32 hash)
{
	// get the file size
	UINT64 filesize = file.size();

	// first read the header
	UINT8 header[CACHED_HEADER_SIZE];
	UINT32 bytes_read = file.read(header, CACHED_HEADER_SIZE);
	if (bytes_read != CACHED_HEADER_SIZE)
		return false;

	// validate the header
	if (header[0] != 'f' || header[1] != 'o' || header[2] != 'n' || header[3] != 't')
		return false;
	if (hash && (header[4] != (UINT8)(hash >> 24) || header[5] != (UINT8)(hash >> 16) || header[6] != (UINT8)(hash >> 8) || header[7] != (UINT8)hash))
		return false;
	m_height = (header[8] << 8) | header[9];
	m_scale = 1.0f / (float)m_height;
	m_yoffs = (INT16)((header[10] << 8) | header[11]);
	UINT32 numchars = (header[12] << 24) | (header[13] << 16) | (header[14] << 8) | header[15];
	if (filesize - CACHED_HEADER_SIZE < numchars * CACHED_CHAR_SIZE)
		return false;

	// now read the rest of the data
	m_rawdata.resize(filesize - CACHED_HEADER_SIZE);
	bytes_read = file.read(&m_rawdata[0], filesize - CACHED_HEADER_SIZE);
	if (bytes_read != filesize - CACHED_HEADER_SIZE)
	{
		m_rawdata.clear();
		return false;
	}

	// extract the data from the data
	UINT64 offset = numchars * CACHED_CHAR_SIZE;
	for (int chindex = 0; chindex < numchars; chindex++)
	{
		const UINT8 *info = reinterpret_cast<UINT8 *>(&m_rawdata[chindex * CACHED_CHAR_SIZE]);
		int chnum = (info[0] << 8) | info[1];

		// if we don't have a subtable yet, make one
		if (!m_glyphs[chnum / 256])
			m_glyphs[chnum / 256] = new glyph[256];

		// fill in the entry
		glyph &gl = m_glyphs[chnum / 256][chnum % 256];
		gl.width = (info[2] << 8) | info[3];
		gl.xoffs = (INT16)((info[4] << 8) | info[5]);
		gl.yoffs = (INT16)((info[6] << 8) | info[7]);
		gl.bmwidth = (info[8] << 8) | info[9];
		gl.bmheight = (info[10] << 8) | info[11];
		gl.rawdata = &m_rawdata[offset];

		// advance the offset past the character
		offset += (gl.bmwidth * gl.bmheight + 7) / 8;
		if (offset > filesize - CACHED_HEADER_SIZE)
		{
			m_rawdata.clear();
			return false;
		}
	}

	// reuse the chartable as a temporary buffer
	m_format = FF_CACHED;
	return true;
}


//-------------------------------------------------
//  save_cached - save a font in cached format
//-------------------------------------------------

bool render_font::save_cached(const char *filename, UINT32 hash)
{
	osd_printf_warning("Generating cached BDF font...\n");

	// attempt to open the file
	emu_file file(manager().machine().options().font_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);
	osd_file::error filerr = file.open(filename);
	if (filerr != osd_file::error::NONE)
		return false;

	// determine the number of characters
	int numchars = 0;
	for (int chnum = 0; chnum < 65536; chnum++)
	{
		if (m_glyphs[chnum / 256])
		{
			glyph &gl = m_glyphs[chnum / 256][chnum % 256];
			if (gl.width > 0)
				numchars++;
		}
	}

	try
	{
		// allocate an array to hold the character data
		dynamic_buffer chartable(numchars * CACHED_CHAR_SIZE, 0);

		// allocate a temp buffer to compress into
		dynamic_buffer tempbuffer(65536);

		// write the header
		UINT8 *dest = &tempbuffer[0];
		*dest++ = 'f';
		*dest++ = 'o';
		*dest++ = 'n';
		*dest++ = 't';
		*dest++ = hash >> 24;
		*dest++ = hash >> 16;
		*dest++ = hash >> 8;
		*dest++ = hash & 0xff;
		*dest++ = m_height >> 8;
		*dest++ = m_height & 0xff;
		*dest++ = m_yoffs >> 8;
		*dest++ = m_yoffs & 0xff;
		*dest++ = numchars >> 24;
		*dest++ = numchars >> 16;
		*dest++ = numchars >> 8;
		*dest++ = numchars & 0xff;
		assert(dest == &tempbuffer[CACHED_HEADER_SIZE]);
		UINT32 bytes_written = file.write(&tempbuffer[0], CACHED_HEADER_SIZE);
		if (bytes_written != dest - &tempbuffer[0])
			throw emu_fatalerror("Error writing cached file");

		// write the empty table to the beginning of the file
		bytes_written = file.write(&chartable[0], numchars * CACHED_CHAR_SIZE);
		if (bytes_written != numchars * CACHED_CHAR_SIZE)
			throw emu_fatalerror("Error writing cached file");

		// loop over all characters
		int tableindex = 0;
		for (int chnum = 0; chnum < 65536; chnum++)
		{
			glyph &gl = get_char(chnum);
			if (gl.width > 0)
			{
				// write out a bit-compressed bitmap if we have one
				if (gl.bitmap.valid())
				{
					// write the data to the tempbuffer
					dest = &tempbuffer[0];
					UINT8 accum = 0;
					UINT8 accbit = 7;

					// bit-encode the character data
					for (int y = 0; y < gl.bmheight; y++)
					{
						int desty = y + m_height + m_yoffs - gl.yoffs - gl.bmheight;
						const UINT32 *src = (desty >= 0 && desty < m_height) ? &gl.bitmap.pix32(desty) : nullptr;
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
					bytes_written = file.write(&tempbuffer[0], dest - &tempbuffer[0]);
					if (bytes_written != dest - &tempbuffer[0])
						throw emu_fatalerror("Error writing cached file");

					// free the bitmap and texture
					m_manager.texture_free(gl.texture);
					gl.bitmap.reset();
					gl.texture = nullptr;
				}

				// compute the table entry
				dest = &chartable[tableindex++ * CACHED_CHAR_SIZE];
				*dest++ = chnum >> 8;
				*dest++ = chnum & 0xff;
				*dest++ = gl.width >> 8;
				*dest++ = gl.width & 0xff;
				*dest++ = gl.xoffs >> 8;
				*dest++ = gl.xoffs & 0xff;
				*dest++ = gl.yoffs >> 8;
				*dest++ = gl.yoffs & 0xff;
				*dest++ = gl.bmwidth >> 8;
				*dest++ = gl.bmwidth & 0xff;
				*dest++ = gl.bmheight >> 8;
				*dest++ = gl.bmheight & 0xff;
			}
		}

		// seek back to the beginning and rewrite the table
		file.seek(CACHED_HEADER_SIZE, SEEK_SET);
		bytes_written = file.write(&chartable[0], numchars * CACHED_CHAR_SIZE);
		if (bytes_written != numchars * CACHED_CHAR_SIZE)
			throw emu_fatalerror("Error writing cached file");
		return true;
	}
	catch (...)
	{
		file.remove_on_close();
		return false;
	}
}
