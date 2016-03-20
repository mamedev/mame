// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/cmdrender.h

    UI command render fonts.

***************************************************************************/

#include "ui/uicmd14.fh"
#include "ui/cmddata.h"

void convert_command_glyph(std::string &str)
{
	int j;
	int len = str.length();
	int buflen = (len + 2) * 2;
	char *d = global_alloc_array(char, buflen);

	for (int i = j = 0; i < len;)
	{
		fix_command_t *fixcmd = nullptr;
		unicode_char uchar;
		int ucharcount = uchar_from_utf8(&uchar, str.substr(i).c_str(), len - i);
		if (ucharcount == -1)
			break;
		else if (ucharcount != 1)
			goto process_next;
		else if (str[i] == '\n')
			uchar = '\n';
		else if (str[i] == COMMAND_CONVERT_TEXT)
		{
			if (str[i] == str[i + 1])
				++i;
			else
			{
				fix_strings_t *fixtext = convert_text;
				for (; fixtext->glyph_code; ++fixtext)
				{
					if (!fixtext->glyph_str_len)
						fixtext->glyph_str_len = strlen(fixtext->glyph_str);

					if (strncmp(fixtext->glyph_str, str.substr(i + 1).c_str(), fixtext->glyph_str_len) == 0)
					{
						uchar = fixtext->glyph_code + COMMAND_UNICODE;
						i += strlen(fixtext->glyph_str);
						break;
					}
				}
			}
		}
		else if (str[i] == COMMAND_DEFAULT_TEXT)
			fixcmd = default_text;
		else if (str[i] == COMMAND_EXPAND_TEXT)
			fixcmd = expand_text;

		if (fixcmd)
		{
			if (str[i] == str[i + 1])
				i++;
			else
			{
				for (; fixcmd->glyph_code; ++fixcmd)
					if (str[i + 1] == fixcmd->glyph_char)
					{
						uchar = fixcmd->glyph_code + COMMAND_UNICODE;
						++i;
						break;
					}
			}
		}
process_next:
		i += ucharcount;
		ucharcount = utf8_from_uchar(d + j, buflen - j - 1, uchar);
		if (ucharcount == -1)
			break;
		j += ucharcount;
	}
	d[j] = '\0';
	str = d;
	global_free_array(d);
}

void render_font::render_font_command_glyph()
{
	emu_file ramfile(OPEN_FLAG_READ);

	if (ramfile.open_ram(font_uicmd14, sizeof(font_uicmd14)) == osd_file::error::NONE)
		load_cached_cmd(ramfile, 0);
}

bool render_font::load_cached_cmd(emu_file &file, UINT32 hash)
{
	UINT64 filesize = file.size();
	UINT8 header[CACHED_HEADER_SIZE];
	UINT32 bytes_read = file.read(header, CACHED_HEADER_SIZE);

	if (bytes_read != CACHED_HEADER_SIZE)
		return false;

	if (header[0] != 'f' || header[1] != 'o' || header[2] != 'n' || header[3] != 't')
		return false;
	if (header[4] != (UINT8)(hash >> 24) || header[5] != (UINT8)(hash >> 16) || header[6] != (UINT8)(hash >> 8) || header[7] != (UINT8)hash)
		return false;
	m_height_cmd = (header[8] << 8) | header[9];
	m_yoffs_cmd = (INT16)((header[10] << 8) | header[11]);
	UINT32 numchars = (header[12] << 24) | (header[13] << 16) | (header[14] << 8) | header[15];
	if (filesize - CACHED_HEADER_SIZE < numchars * CACHED_CHAR_SIZE)
		return false;

	m_rawdata_cmd.resize(filesize - CACHED_HEADER_SIZE);
	bytes_read = file.read(&m_rawdata_cmd[0], filesize - CACHED_HEADER_SIZE);
	if (bytes_read != filesize - CACHED_HEADER_SIZE)
	{
		m_rawdata_cmd.clear();
		return false;
	}

	UINT64 offset = numchars * CACHED_CHAR_SIZE;
	for (int chindex = 0; chindex < numchars; chindex++)
	{
		const UINT8 *info = reinterpret_cast<UINT8 *>(&m_rawdata_cmd[chindex * CACHED_CHAR_SIZE]);
		int chnum = (info[0] << 8) | info[1];

		if (!m_glyphs_cmd[chnum / 256])
			m_glyphs_cmd[chnum / 256] = new glyph[256];

		glyph &gl = m_glyphs_cmd[chnum / 256][chnum % 256];

		if (chnum >= COMMAND_UNICODE && chnum < COMMAND_UNICODE + COLOR_BUTTONS)
			gl.color = color_table[chnum - COMMAND_UNICODE];

		gl.width = (info[2] << 8) | info[3];
		gl.xoffs = (INT16)((info[4] << 8) | info[5]);
		gl.yoffs = (INT16)((info[6] << 8) | info[7]);
		gl.bmwidth = (info[8] << 8) | info[9];
		gl.bmheight = (info[10] << 8) | info[11];
		gl.rawdata = &m_rawdata_cmd[offset];

		offset += (gl.bmwidth * gl.bmheight + 7) / 8;
		if (offset > filesize - CACHED_HEADER_SIZE)
		{
			m_rawdata_cmd.clear();
			return false;
		}
	}

	return true;
}
