// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    TI-99/2 main board custom circuits

    This component implements the custom video controller and interface chip
    from the TI-99/2 console.

***************************************************************************/

#include "emu.h"
#include "992board.h"

/**
    Emulation of the CRT Gate Array of the TI-99/2

    Video display controller

    RF-modulated, composite output
    for standard black/white television
    Selectable channel 3 or 4 VHF

    625 lines for US markets
    525 lines for European markets

    Display: 24 rows, 32 columns

    The controller accesses ROM and RAM space of the 9995 CPU. It makes use
    of the HOLD line to gain access. Thus, the controller has DMA control
    while producing each scan line. The CPU has a chance to execute instructions
    in border time, horizontal retrace, vertical retrace.

    In order to get more computing time, a special character (BEOL - bank end
    of line) is used to indicate the last drawable character on the line. After
    this character, the buses are released.

    24K version: BEOL = any character from 0x70 to 0xff
    32K version: BEOL = any character from 0x80 to 0xff

    CRU Bit VIDENA: disables the scan line generation; blank white screen

    Clock: 10.7 MHz
    Divided by 2 for 9995 CLKIN

    Scanline refresh:
    - Pull down HOLD
    - Wait for a short time (some dots)
    - Use row, column, dot_line counters
      - Get the value c at 0xEC00 + row*32+col
      - Get the byte b from 0x1C00 + c*8 + (dot_line%8)
      - Push the byte to the shift register
      - Get the bits for the scanline from the register

   EF00: Control byte
         +--+--+--+--+--+--+--+--+
         |- |- |- |- |- |T |B |S |
         +--+--+--+--+--+--+--+--+

         Fabrice's 99/2:
         T: Text color (1=white)
         B: border color (1=white)
         S: Background color (1=text color, 0=inverted)

                 text border back
         0 0 0    b     b     w
         0 0 1    b     b     b
         0 1 0    b     w     w
         0 1 1    b     w     b
         1 0 0    w     b     b
         1 0 1    w     b     w
         1 1 0    w     w     b
         1 1 1    w     w     w

   Counters:
     dotline 9 bit
        after reaching 261, resets to 0

        224..236: Top blanking
        237..261: Top border
        000..191: Display
        192..217: Bottom border
        218..220: Bottom blanking
        221..223: Vert sync

     dotcolumn 9 bit
        increments every clock tick until reaching 341, resets to 0, and incr dotline

        305..328: Left blanking
        329..341: Left border
        000..255: Display
        256..270: Right border
        271..278: Right blanking
        279..304: Hor sync
   ------------------------------

   Later versions define a "bitmap mode" [2]

   EF00: Control byte
         +--+--+--+--+--+--+--+--+
         |- |- |- |- |- |C |B |M |
         +--+--+--+--+--+--+--+--+

         C: character color (1=white)
         B: border color (1=white)
         M: bitmap mode (1=bitmap)

   [1] Ground Squirrel Personal Computer Product Specification
   [2] VDC Controller CF40052
*/

#include "emu.h"
#include "992board.h"

DEFINE_DEVICE_TYPE_NS(VIDEO99224, bus::ti99::internal, video992_24_device, "video992_24", "TI-99/2 CRT Controller 24K version")
DEFINE_DEVICE_TYPE_NS(VIDEO99232, bus::ti99::internal, video992_32_device, "video992_32", "TI-99/2 CRT Controller 32K version")

namespace bus { namespace ti99 { namespace internal {

video992_device::video992_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	  device_video_interface(mconfig, *this),
	  m_mem_read_cb(*this),
	  m_hold_cb(*this),
	  m_int_cb(*this),
	  m_videna(true)
{
}

video992_24_device::video992_24_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: video992_device(mconfig, VIDEO99224, tag, owner, clock)
{
	m_beol = 0x70;
}

video992_32_device::video992_32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: video992_device(mconfig, VIDEO99232, tag, owner, clock)
{
	m_beol = 0x7f;
}

std::string video992_device::tts(attotime t)
{
	char buf[256];
	const char *sign = "";
	if(t.seconds() < 0) {
		t = attotime::zero-t;
		sign = "-";
	}
	int nsec = t.attoseconds() / ATTOSECONDS_PER_NANOSECOND;
	sprintf(buf, "%s%04d.%03d,%03d,%03d", sign, int(t.seconds()), nsec/1000000, (nsec/1000)%1000, nsec % 1000);
	return buf;
}


void video992_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int raw_vpos = screen().vpos();

	if (id == HOLD_TIME)
	{
		// logerror("release time: %s, diff: %s\n", tts(machine().time()), tts(machine().time()-m_hold_time));
		// We're holding the CPU; release it until the next start
		m_hold_cb(CLEAR_LINE);
		m_free_timer->adjust(screen().time_until_pos((raw_vpos+1) % screen().height(), HORZ_DISPLAY_START));
		return;
	}

	// logerror("hold time: %s\n", tts(machine().time()));
	if (m_videna)
	{
		// Hold the CPU
		m_hold_time = machine().time();
		m_hold_cb(ASSERT_LINE);
	}

	int vpos = raw_vpos * m_vertical_size / screen().height();
	uint32_t *p = &m_tmpbmp.pix32(vpos);
	bool endofline = false;

	int linelength = 0;

	// logerror("draw line %d\n", vpos);
	// Get control byte
	uint8_t control = m_mem_read_cb(0xef00);
	bool text_white = ((control & 0x04)!=0);
	bool border_white = ((control & 0x02)!=0);
	bool background_white = ((control & 0x01)!=0)? text_white : !text_white;

	int y = vpos - m_top_border;
	if (y < 0 || y >= 192)
	{
		// Draw border colour
		for (int i = 0; i < TOTAL_HORZ; i++)
			p[i] = border_white? rgb_t::white() : rgb_t::black();

		// vblank is set at the last cycle of the first inactive line
		// not confirmed by the specs, just doing like 9928A.
		if ( y == 193 )
		{
			m_int_cb( ASSERT_LINE );
			m_int_cb( CLEAR_LINE );
		}
	}
	else
	{
		// Draw regular line
		// Left border
		for (int i = 0; i < HORZ_DISPLAY_START; i++)
			p[i] = border_white? rgb_t::white() : rgb_t::black();

		int addr = ((y << 2) & 0x3e0) | 0xec00;

		// Active display
		for (int x = HORZ_DISPLAY_START; x<HORZ_DISPLAY_START+256; x+=8)
		{
			uint8_t charcode = 0;
			uint8_t pattern = 0;
			if (!endofline && m_videna)
			{
				// Get character code at the location
				charcode = m_mem_read_cb(addr) & 0x7f;

				// Is it the BEOL (blank end-of-line)?
				if (charcode >= m_beol)
					endofline = true;
			}
			if (!endofline && m_videna)
			{
				// Get the pattern
				int addrp = 0x1c00 | (charcode << 3) | (y%8);
				pattern = m_mem_read_cb(addrp);
				linelength++;
			}
			for (int i = 0; i < 8; i++)
			{
				if ((pattern & 0x80)!=0)
					p[x+i] = text_white? rgb_t::white() : rgb_t::black();
				else
					p[x+i] = background_white? rgb_t::white() : rgb_t::black();

				pattern <<= 1;
			}
			addr++;
		}

		// Right border
		for (int i = HORZ_DISPLAY_START + 256; i < TOTAL_HORZ; i++)
			p[i] = border_white? rgb_t::white() : rgb_t::black();
	}

	// +1 for the minimum hold time
	// logerror("line length: %d\n", linelength);
	m_hold_timer->adjust(screen().time_until_pos(raw_vpos, HORZ_DISPLAY_START + linelength*8 + 1));
}


uint32_t video992_device::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	copybitmap( bitmap, m_tmpbmp, 0, 0, 0, 0, cliprect );
	return 0;
}

/*
    VIDENA pin, negative logic
*/
WRITE_LINE_MEMBER( video992_device::videna )
{
	m_videna = (state==ASSERT_LINE);
}

void video992_device::device_start()
{
	m_top_border = VERT_DISPLAY_START_NTSC;
	m_vertical_size = TOTAL_VERT_NTSC;
	m_tmpbmp.allocate(TOTAL_HORZ, TOTAL_VERT_NTSC);

	m_hold_timer = timer_alloc(HOLD_TIME);
	m_free_timer = timer_alloc(FREE_TIME);

	m_border_color = rgb_t::black();
	m_background_color = rgb_t::white();
	m_text_color = rgb_t::black();

	m_mem_read_cb.resolve();
	m_hold_cb.resolve();
	m_int_cb.resolve();
}

void video992_device::device_reset()
{
	m_free_timer->adjust(screen().time_until_pos(0, HORZ_DISPLAY_START));
}

}   }   }


