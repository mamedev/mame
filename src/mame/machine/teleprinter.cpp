// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#include "teleprinter.h"
#include "asr33_glyphs_11px.xpm"
#define KEYBOARD_TAG "keyboard"
#define PAPER_COLOR 0xFEF0CC

enum {
	KEY_PRESS_SAMPLE,
	LINE_FEED_SAMPLE,
	CARRIAGE_RETURN_SAMPLE
};

teleprinter_device::teleprinter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: generic_terminal_device(mconfig, TELEPRINTER, "Teleprinter", tag, owner, clock, "teleprinter", __FILE__)
	, m_samples(*this, "samples")
{
}

void teleprinter_device::scroll_line()
{
	m_samples->start(LINE_FEED_SAMPLE, LINE_FEED_SAMPLE);
	memmove(m_buffer,m_buffer+TELEPRINTER_WIDTH,(TELEPRINTER_HEIGHT-1)*TELEPRINTER_WIDTH);
	memset(m_buffer + TELEPRINTER_WIDTH*(TELEPRINTER_HEIGHT-1),0x20,TELEPRINTER_WIDTH);
}

void teleprinter_device::write_char(UINT8 data) {
	m_samples->start(KEY_PRESS_SAMPLE, KEY_PRESS_SAMPLE);
	m_buffer[(TELEPRINTER_HEIGHT-1)*TELEPRINTER_WIDTH+m_x_pos] = data;
	m_x_pos++;
	if (m_x_pos >= TELEPRINTER_WIDTH)
	{
		m_x_pos = 0;
		m_samples->start(CARRIAGE_RETURN_SAMPLE, CARRIAGE_RETURN_SAMPLE);
		scroll_line();
	}
}

void teleprinter_device::clear() {
	memset(m_buffer,0,TELEPRINTER_WIDTH*TELEPRINTER_HEIGHT);
	m_x_pos = 0;
}

void teleprinter_device::term_write(UINT8 data)
{
	switch(data) {
		case 10: m_x_pos = 0;
				scroll_line();
				break;
		case 13:
			m_x_pos = 0;
			m_samples->start(CARRIAGE_RETURN_SAMPLE, CARRIAGE_RETURN_SAMPLE);
			break;
		case  9: m_x_pos = (m_x_pos & 0xf8) + 8;
				if (m_x_pos >= TELEPRINTER_WIDTH)
					m_x_pos = TELEPRINTER_WIDTH-1;

				break;
		case 16: break;
		default: write_char(data); break;
	}
}

/***************************************************************************
    VIDEO HARDWARE
***************************************************************************/

static int hex2int(char v){
	if (v >= '0' && v <= '9') return v - '0';
	if (v >= 'A' && v <= 'F') return v - 'A' + 10;
	if (v >= 'a' && v <= 'f') return v - 'a' + 10;
	return 0;
}

static UINT32 get_color_if_code_matches(UINT16 code, const char* cstring){
	UINT16 value;
	UINT32 abgr = 0;
	value = cstring[0] + (cstring[1] << 8);
	if (value == code && cstring[5]=='#'){
		abgr = hex2int(cstring[11]) + 16*hex2int(cstring[10]);
		abgr |= (hex2int(cstring[9]) + 16*hex2int(cstring[8])) << 8;
		abgr |= (hex2int(cstring[7]) + 16*hex2int(cstring[6])) << 16;
		return abgr;
	}
	/* else */ return 0;
}

static char teleprinter_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTuvwxyz!@#$%*()<>-=_+'[]\".,;:\\/?^";
UINT32 teleprinter_device::tp_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 glyph_id, textchar;
	UINT16 code;
	UINT32 abgr;
	int y, c, x, b;
	int num_colors = 249; //TODO: read this from the data structures

	for (y = 0; y < TELEPRINTER_HEIGHT; y++)
	{
		for (c = 0; c < 12; c++)
		{
			int horpos = 0;
			for (x = 0; x < TELEPRINTER_WIDTH; x++)
			{
				textchar = m_buffer[y*TELEPRINTER_WIDTH + x];
				for (glyph_id=0; glyph_id<62; glyph_id++){
					if (teleprinter_chars[glyph_id] == textchar) break;
				}
				if (glyph_id == 62){
					for (b = 0; b < 11; b++)
						bitmap.pix32(y*12 + c, horpos++) = PAPER_COLOR;
				} else {

					for (b = 0; b < 11; b++)
					{
						code = asr33_glyphs_11px_xpm[num_colors + 1 + c][2*(glyph_id*11 + b)];
						code += asr33_glyphs_11px_xpm[num_colors + 1 + c][2*(glyph_id*11 + b) + 1] << 8;
						for (int idx=0; idx<num_colors;idx++){
							abgr = get_color_if_code_matches(code, asr33_glyphs_11px_xpm[idx+1]);
							if (abgr) break;
						}
						bitmap.pix32(y*12 + c, horpos++) =  abgr;
					}
				}
			}
		}
	}
	return 0;
}

static const char *const teleprinter_sample_names[] =
{
        "*teleprinter",
        "keypress",       /* 0 */
        "linefeed",       /* 1 */
        "carriagereturn", /* 2 */
        nullptr
};

/***************************************************************************
    VIDEO HARDWARE
***************************************************************************/
MACHINE_CONFIG_FRAGMENT( generic_teleprinter )
	MCFG_SCREEN_ADD(TELEPRINTER_SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(TELEPRINTER_WIDTH*11, TELEPRINTER_HEIGHT*12)
	MCFG_SCREEN_VISIBLE_AREA(0, TELEPRINTER_WIDTH*11-1, 0, TELEPRINTER_HEIGHT*12-1)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, teleprinter_device, tp_update)
	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(generic_terminal_device, kbd_put))

        /* Sound Effects */
        MCFG_SPEAKER_STANDARD_MONO("mono")
        MCFG_SOUND_ADD("samples", SAMPLES, 0)
        MCFG_SAMPLES_CHANNELS(3)
        MCFG_SAMPLES_NAMES(teleprinter_sample_names)
        MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

machine_config_constructor teleprinter_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(generic_teleprinter);
}

/*-------------------------------------------------
    device_reset - device-specific reset
-------------------------------------------------*/

void teleprinter_device::device_reset()
{
	clear();
	generic_terminal_device::device_reset();
}

const device_type TELEPRINTER = &device_creator<teleprinter_device>;
