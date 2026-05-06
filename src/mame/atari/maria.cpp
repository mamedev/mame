// license:BSD-3-Clause
// copyright-holders:Dan Boris, Fabio Priuli, Mike Saarna, Robert Tuccitto
/***************************************************************************

  Atari MARIA video emulation


  - some history:
    2014-12-01 Mike Saarna, Robert Tuccitto Implemented "colorburst kill" bit
                of the MARIA CTRL register.
    2014-10-05 Mike Saarna, Robert Tuccitto Last Line DMA value corrected
                to 6. GCC and Atari docs both show a difference between
                Other Line and Last Line as +6 at the lowest part of the
                range.
                Blank scanlines are drawn when DMA is off, like real
                hardware.
                If MARIA hits the DMA limit, the CPU doesn't run until
                the next scanline.

    2014-09-03 Mike Saarna, Robert Tuccitto reorganized DMA penalties to
                support new rendering timeout code.

    2014-08-29 Mike Saarna Timeout rendering added.

    2014-08-26 Fabio Priuli Converted to device

    2014-05-06 Mike Saarna Added interrupts to DMA cycle eating. Updates to
                LL, OL, and spin accounting for HALT behavior.

    2014-03-24 Mike Saarna Fixed DMA regarding startup, shutdown and
                            cycle stealing.

    2013-05-08 huygens rewrite to emulate line ram buffers (mostly fixes Kung-Fu Master
                            Started DMA cycle stealing implementation

    2003-06-23 ericball Kangaroo mode & 320 mode & other stuff

    2002-05-14 kubecj vblank dma stop fix

    2002-05-13 kubecj   fixed 320C mode (displayed 2 pixels instead of one!)
                            noticed that Jinks uses 0x02-320D mode
                            implemented the mode - completely unsure if good!
                            implemented some Maria CTRL variables

    2002-05-12 kubecj added cases for 0x01-160A, 0x05-160B as stated by docs

***************************************************************************/

#include "emu.h"
#include "maria.h"
#include "screen.h"

#define LOG_UNK   (1 << 1)
#define LOG_VIDEO (1 << 2)
#define LOG_CTRL  (1 << 3)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGUNK(...)    LOGMASKED(LOG_UNK,   __VA_ARGS__)
#define LOGVIDEO(...)  LOGMASKED(LOG_VIDEO, __VA_ARGS__)
#define LOGCTRL(...)   LOGMASKED(LOG_CTRL,  __VA_ARGS__)


DEFINE_DEVICE_TYPE(ATARI_MARIA, atari_maria_device, "atari_maria", "Atari MARIA")

atari_maria_device::atari_maria_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ATARI_MARIA, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_maria_palette{0}
	, m_line_ram{0}
	, m_active_buffer(0)
	, m_write_mode(false)
	, m_dll(0)
	, m_dl(0)
	, m_holey(0)
	, m_offset(0)
	, m_vblank(0)
	, m_dmaon(false)
	, m_dpp(0)
	, m_wsync(false)
	, m_color_kill(false)
	, m_cwidth(false)
	, m_bcntl(false)
	, m_kangaroo(false)
	, m_rm(0)
	, m_dli(false)
	, m_charbase(0)
	, m_dmaspace(*this, finder_base::DUMMY_TAG, -1, 8)
	, m_dma_wait_cb(*this)
	, m_halt_cb(*this)
	, m_dli_cb(*this)
{
}


void atari_maria_device::device_start()
{
	screen().register_screen_bitmap(m_bitmap);

	for (int i = 0; i < 2; i++)
	{
		m_line_ram[i] = make_unique_clear<uint8_t []>(LINERAM_SIZE);

		save_pointer(NAME(m_line_ram[i]), LINERAM_SIZE, i);
	}

	save_item(NAME(m_maria_palette));
	save_item(NAME(m_active_buffer));
	save_item(NAME(m_write_mode));
	save_item(NAME(m_dll));
	save_item(NAME(m_dl));
	save_item(NAME(m_holey));
	save_item(NAME(m_offset));
	save_item(NAME(m_vblank));
	save_item(NAME(m_dmaon));
	save_item(NAME(m_dpp));
	save_item(NAME(m_wsync));
	save_item(NAME(m_color_kill));
	save_item(NAME(m_cwidth));
	save_item(NAME(m_bcntl));
	save_item(NAME(m_kangaroo));
	save_item(NAME(m_rm));
	save_item(NAME(m_dli));
	save_item(NAME(m_charbase));
}

void atari_maria_device::device_reset()
{
	for (auto & elem : m_maria_palette)
		elem = 0;

	for (auto & elem : m_line_ram)
		for (int j = 0; j < LINERAM_SIZE; j++)
			elem[j] = 0;

	m_active_buffer = 0;

	m_write_mode = false;
	m_dmaon = false;
	m_vblank = 0x80;
	m_dll = 0;
	m_wsync = false;

	m_color_kill = false;
	m_cwidth = false;
	m_bcntl = false;
	m_kangaroo = false;
	m_rm = 0;

	m_dl = 0;
	m_holey = 0;
	m_offset = 0;
	m_dpp = 0;
	m_dli = false;
	m_charbase = 0;
}

/***************************************************************************

  Stop the video hardware emulation.

***************************************************************************/

bool atari_maria_device::is_holey(offs_t addr)
{
	if ((BIT(m_holey, 1) && ((addr & 0x9000) == 0x9000)) || (BIT(m_holey, 0) && ((addr & 0x8800) == 0x8800)))
		return true;
	return false;
}

int atari_maria_device::write_line_ram(offs_t addr, uint8_t offset, uint8_t pal)
{
	uint8_t c;

	const uint8_t data = read_byte(addr);
	pal <<= 2;

	if (m_write_mode)
	{
		c = (pal & 0x10) | (data & 0x0c) | (data >> 6); // P2 D3 D2 D7 D6
		if (((c & 3) || m_kangaroo) && (offset < LINERAM_SIZE))
			m_line_ram[m_active_buffer][offset] = c;
		offset++;

		c = (pal & 0x10) | ((data & 0x03) << 2) | ((data & 0x30) >> 4); // P2 D1 D0 D5 D4
		if (((c & 3) || m_kangaroo) && (offset < LINERAM_SIZE))
			m_line_ram[m_active_buffer][offset] = c;
	}
	else
	{
		for (int i = 0; i < 4; i++, offset++)
		{
			c = pal | ((data >> (6 - 2 * i)) & 0x03);
			if (((c & 3) || m_kangaroo) && (offset < LINERAM_SIZE))
				m_line_ram[m_active_buffer][offset] = c;
		}
	}
	return m_write_mode ? 2 : 4;
}


void atari_maria_device::draw_scanline()
{
	int cells = 0;

	if (m_dmaon)
	{
		// All lines in a zone have the same initial DMA startup time. We'll adjust
		// cycles for the special last zone line later, as those penalties happen after
		// MARIA is done rendering, or after its hit the maximum rendering time.
		uint64_t maria_cycles = 16;

		/* Process this DLL entry */
		uint16_t dl = m_dl;

		/* DMA */
		/* Step through DL's while we're within maximum rendering time. */
		/*  max render time = ( scanline length - DMA start ) */
		/*     426          = (     454         -     28    ) */

		uint16_t graph_adr, data_addr;
		int width;
		uint8_t hpos, pal;
		bool ind;
		while (((read_byte(dl + 1) & 0x5f) != 0) && (maria_cycles < 426))
		{
			/* Extended header */
			if (!(read_byte(dl + 1) & 0x1f)) // bit 4 also needs to set?
			{
				graph_adr = (read_byte(dl + 2) << 8) | read_byte(dl);
				width = ((read_byte(dl + 3) ^ 0xff) & 0x1f) + 1;
				hpos = read_byte(dl + 4);
				pal = read_byte(dl + 3) >> 5;
				m_write_mode = BIT(read_byte(dl + 1), 7);
				ind = BIT(read_byte(dl + 1), 5);
				dl += 5;
				maria_cycles += 10;
			}
			/* Normal header */
			else
			{
				graph_adr = (read_byte(dl + 2) << 8) | read_byte(dl);
				width = ((read_byte(dl + 1) ^ 0xff) & 0x1f) + 1;
				hpos = read_byte(dl + 3);
				pal = read_byte(dl + 1) >> 5;
				ind = false;
				dl += 4;
				maria_cycles += 8;
			}

			/*LOGVIDEO("%x DL: ADR=%x  width=%x  hpos=%x  pal=%x  mode=%x  ind=%x\n", screen().vpos(), graph_adr, width, hpos, pal, m_write_mode, ind);*/

			for (int x = 0; x < width; x++)
			{
				if (maria_cycles >= 426) // ensure we haven't overrun the maximum render time
					break;

				/* Do indirect mode */
				if (ind)
				{
					const uint8_t c = read_byte(graph_adr + x) & 0xff;
					data_addr = (m_charbase | c) + (m_offset << 8);
					if (is_holey(data_addr))
						continue;

					maria_cycles += 3;
					if (m_cwidth) // two data bytes per map byte
					{
						cells = write_line_ram(data_addr, hpos, pal);
						hpos += cells;
						cells = write_line_ram(data_addr + 1, hpos, pal);
						hpos += cells;
						maria_cycles += 6;
					}
					else
					{
						cells = write_line_ram(data_addr, hpos, pal);
						hpos += cells;
						maria_cycles += 3;
					}
				}
				else // direct mode
				{
					data_addr = graph_adr + x + (m_offset << 8);
					if (is_holey(data_addr))
						continue;
					cells = write_line_ram(data_addr, hpos, pal);
					hpos += cells;
					maria_cycles += 3;
				}
			}
		}

		// Last Line post-render DMA cycle penalties...
		if (m_offset == 0)
		{
			maria_cycles += 6; // extra shutdown time
			if (BIT(read_byte(m_dll + 3), 7))
				maria_cycles += 17; // interrupt overhead
		}

		// If MARIA used up all of the DMA time then the CPU can't run until next line...
		if (maria_cycles >= 426)
		{
			m_halt_cb(ASSERT_LINE);
			m_wsync = true;
		}

		// Spin the CPU for Maria DMA, if it's not already spinning for WSYNC.
		// MARIA generates the 6502 clock by dividing its own clock by 4. It needs to HALT and unHALT
		// the 6502 on ths same clock phase, so MARIA will wait until its clock divides evenly by 4.
		// To spin until an even divisor, we just round-up any would-be truncations by adding 3.
		if (!m_wsync)
			m_dma_wait_cb(maria_cycles);
	}

	// draw line buffer to screen
	m_active_buffer ^= 1; // switch buffers
	uint16_t *const scanline = &m_bitmap.pix(screen().vpos());

	for (int i = 0; i < LINERAM_SIZE; i++)
	{
		uint8_t d, pixel_cell;
		switch (m_rm)
		{
			case 0x00:  /* 160A, 160B */
			case 0x01:  /* 160A, 160B */
				pixel_cell =  m_line_ram[m_active_buffer][i];
				scanline[2 * i] = m_maria_palette[pixel_cell];
				scanline[2 * i + 1] = m_maria_palette[pixel_cell];
				break;

			case 0x02: /* 320B, 320D */
				pixel_cell = m_line_ram[m_active_buffer][i];
				d = (pixel_cell & 0x10) | (pixel_cell & 0x02) | ((pixel_cell >> 3) & 1); // b4 0 0 b1 b3
				scanline[2 * i] = m_maria_palette[d];
				d = (pixel_cell & 0x10) | ((pixel_cell << 1) & 0x02) | ((pixel_cell >> 2) & 1); // b4 0 0 b0 b2
				scanline[2 * i + 1] = m_maria_palette[d];
				break;

			case 0x03:  /* 320A, 320C */
				pixel_cell = m_line_ram[m_active_buffer][i];
				d = (pixel_cell & 0x1c) | (pixel_cell & 0x02); // b4 b3 b2 b1 0
				scanline[2 * i] = m_maria_palette[d];
				d = (pixel_cell & 0x1c) | ((pixel_cell << 1) & 0x02); // b4 b3 b2 b0 0
				scanline[2 * i + 1] = m_maria_palette[d];
				break;
		}

		if (m_color_kill) //remove color if there's no colorburst signal
		{
			scanline[2 * i] &= 0x0f;
			scanline[2 * i + 1] &= 0x0f;
		}
	}

	for (int i = 0; i < LINERAM_SIZE; i++) // buffer automaticaly cleared once displayed
		m_line_ram[m_active_buffer][i] = 0;
}


void atari_maria_device::interrupt(int lines)
{
	if (m_wsync)
	{
		m_halt_cb(CLEAR_LINE);
		m_wsync = false;
	}

	const int frame_scanline = screen().vpos() % (lines + 1);
	if (frame_scanline == 16)
		m_vblank = 0x00;

	if (frame_scanline == (lines - 5))
		m_vblank = 0x80;
}


void atari_maria_device::startdma(int lines)
{
	const int maria_scanline = screen().vpos();
	const int frame_scanline = maria_scanline % (lines + 1);

	if ((frame_scanline == 16) && m_dmaon)
	{
		/* end of vblank */
		m_dll = m_dpp; // currently only handle changes to dll during vblank
		m_dl = (read_byte(m_dll + 1) << 8) | read_byte(m_dll + 2);
		const uint8_t header = read_byte(m_dll);
		m_offset = header & 0x0f;
		m_holey = (header & 0x60) >> 5;
		m_dli = BIT(header, 7);
		/*  LOGVIDEO("DLL=%x\n", m_dll); */
	}

	if ((frame_scanline > 15) && (frame_scanline < (lines - 5)))
		draw_scanline();

	if ((frame_scanline > 16) && (frame_scanline < (lines - 5)) && m_dmaon)
	{
		if (m_offset == 0)
		{
			m_dll += 3;
			m_dl = (read_byte(m_dll + 1) << 8) | read_byte(m_dll + 2);
			const uint8_t header = read_byte(m_dll);
			m_offset = header & 0x0f;
			m_holey = (header & 0x60) >> 5;
			if (BIT(header, 4))
				LOGVIDEO("dll bit 4 set!\n");
			m_dli = BIT(header, 7);
		}
		else
		{
			m_offset--;
		}
	}

	if (m_dli)
	{
		m_dli_cb(ASSERT_LINE);
		m_dli = false;
	}
}

/***************************************************************************

  Refresh the video screen

***************************************************************************/

/* This routine is called at the start of vblank to refresh the screen */
uint32_t atari_maria_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


uint8_t atari_maria_device::read(offs_t offset)
{
	switch (offset)
	{
		case 0x08:
			return m_vblank;

		default:
			if (!machine().side_effects_disabled())
				LOGUNK("%s: undefined MARIA read %x\n", machine().describe_context(), offset);
			return 0x00; // don't know if this should be 0x00 or 0xff
	}
}

void atari_maria_device::write(offs_t offset, uint8_t data)
{
	if ((offset & 3) != 0)
		m_maria_palette[offset] = data;

	switch (offset)
	{
		case 0x00:
			// all color ram addresses with 00 for their least significant bits point to the background color
			for (int i = 0; i < 8; i++)
				m_maria_palette[4 * i] = data;
			break;
		case 0x04:
			m_halt_cb(ASSERT_LINE);
			m_wsync = true;
			break;
		case 0x0c: // DPPH
			m_dpp = (m_dpp & 0x00ff) | (data << 8);
			break;
		case 0x10: // DPPL
			m_dpp = (m_dpp & 0xff00) | data;
			break;
		case 0x14:
			m_charbase = (data << 8);
			break;
		case 0x1c:
			/*LOGCTRL("MARIA CTRL=%x\n",data);*/
			m_color_kill = BIT(data, 7);
			switch ((data >> 5) & 3)
			{
				case 0x00:
				case 0x01:
					LOGUNK("%s: dma test mode, do not use.\n", machine().describe_context());
					break;

				case 0x02:
					m_dmaon = true;
					break;

				case 0x03:
					m_dmaon = false;
					break;
			}
			m_cwidth = BIT(data, 4);
			m_bcntl = BIT(data, 3); // Currently unimplemented as we don't display the border
			m_kangaroo = BIT(data, 2);
			m_rm = data & 0x03;

			/*LOGCTRL( "MARIA CTRL: CK:%d DMA:%d CW:%d BC:%d KM:%d RM:%d\n",
			        m_color_kill ? 1 : 0,
			        (data & 0x60) >> 5,
			        m_cwidth ? 1 : 0,
			        m_bcntl ? 1 : 0,
			        m_kangaroo ? 1 : 0,
			        m_rm);*/

			break;
	}
}
