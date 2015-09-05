// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/cmdrender.h

    MEWUI rendfont.

***************************************************************************/

#include "mewui/uicmd14.fh"
#include "mewui/cmddata.h"

void convert_command_glyph(std::string &str)
{
	int j;
	const char *s = str.c_str();
	int len = str.length();
	int buflen = (len + 2) * 2;
	char *d = global_alloc_array(char, buflen);

	for (int i = j = 0; i < len;)
	{
		fix_command_t *fixcmd = NULL;
		unicode_char uchar;

		int ucharcount = uchar_from_utf8(&uchar, s + i, len - i);
		if (ucharcount == -1)
			break;
		else if (ucharcount != 1)
			goto process_next;

		if (s[i] == '\n')
		{
			uchar = '\n';
			goto process_next;
		}

		if (s[i] == COMMAND_CONVERT_TEXT)
		{
			fix_strings_t *fixtext = convert_text;

			if (s[i] == s[i + 1])
			{
				i++;
				goto process_next;
			}

			for (; fixtext->glyph_code; fixtext++)
			{
				if (!fixtext->glyph_str_len)
					fixtext->glyph_str_len = strlen(fixtext->glyph_str);

				if (strncmp(fixtext->glyph_str, s + i + 1, fixtext->glyph_str_len) == 0)
				{
					uchar = fixtext->glyph_code + COMMAND_UNICODE;
					i += strlen(fixtext->glyph_str);
					goto process_next;
				}
			}
		}

		if (s[i] == COMMAND_DEFAULT_TEXT)
			fixcmd = default_text;

		else if (s[i] == COMMAND_EXPAND_TEXT)
			fixcmd = expand_text;

		if (fixcmd)
		{
			if (s[i] == s[i + 1])
			{
				i++;
				goto process_next;
			}

			for (; fixcmd->glyph_code; fixcmd++)
				if (s[i + 1] == fixcmd->glyph_char)
				{
					uchar = fixcmd->glyph_code + COMMAND_UNICODE;
					i++;
					goto process_next;
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
	str.assign(d);
	global_free_array(d);
}

void render_font::render_font_command_glyph()
{
	emu_file ramfile(OPEN_FLAG_READ);

	if (ramfile.open_ram(font_uicmd14, sizeof(font_uicmd14)) == FILERR_NONE)
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
