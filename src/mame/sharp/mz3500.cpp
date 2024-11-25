// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    MZ-3500 (c) 198? Sharp

    preliminary driver by Angelo Salese

    TODO:
    - BUSREQ / BUSACK signals.
    - master/slave comms aren't perfect (especially noticeable if you change the video DIP)

    Notes:
    Sub-CPU test meanings:
    * RA (tests RAM, first is work RAM, other two are shared RAM banks)
    * VR (tests VRAM)
    * CRT interface test:
        - 40x20
        - 80x25
        - monochrome attribute test
        - 80x25 color test (text B-R-G-W, border: black, blue, red, green, black)
        - 40x20 color test (text B-R-G-W, border: black, blue, red, green, black)
    * Speaker test
    * PR (Printer interface test)
    * LP (Light pen test)
    * RS (RS-232C interface test)

***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "sound/beep.h"
#include "video/upd7220.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define MAIN_CLOCK XTAL(8'000'000)

class mz3500_state : public driver_device
{
public:
	mz3500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_master(*this, "master")
		, m_slave(*this, "slave")
		, m_hgdc1(*this, "upd7220_chr")
		, m_hgdc2(*this, "upd7220_gfx")
		, m_fdc(*this, "upd765a")
		, m_video_ram(*this, "video_ram")
		, m_beeper(*this, "beeper")
		, m_palette(*this, "palette")
		, m_system_dsw(*this, "SYSTEM_DSW")
		, m_fd_dsw(*this, "FD_DSW")
		, m_floppy_connector(*this, "upd765a:%u", 0U)
	{ }

	void mz3500(machine_config &config);

private:
	// devices
	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	required_device<upd7220_device> m_hgdc1;
	required_device<upd7220_device> m_hgdc2;
	required_device<upd765a_device> m_fdc;
	required_shared_ptr<uint16_t> m_video_ram;
	required_device<beep_device> m_beeper;
	required_device<palette_device> m_palette;

	uint8_t *m_ipl_rom;
	uint8_t *m_basic_rom;
	std::unique_ptr<uint8_t[]> m_work_ram;
	std::unique_ptr<uint8_t[]> m_shared_ram;
	uint8_t *m_char_rom;

	uint8_t m_ma,m_mo,m_ms,m_me2,m_me1;
	uint8_t m_crtc[0x10];

	uint8_t m_srdy;

	uint8_t m_fdd_sel;

	uint8_t mz3500_master_mem_r(offs_t offset);
	void mz3500_master_mem_w(offs_t offset, uint8_t data);
	uint8_t mz3500_ipl_r(offs_t offset);
	uint8_t mz3500_basic_r(offs_t offset);
	uint8_t mz3500_work_ram_r(offs_t offset);
	void mz3500_work_ram_w(offs_t offset, uint8_t data);
	uint8_t mz3500_shared_ram_r(offs_t offset);
	void mz3500_shared_ram_w(offs_t offset, uint8_t data);
	uint8_t mz3500_io_r(offs_t offset);
	void mz3500_io_w(offs_t offset, uint8_t data);
	void mz3500_crtc_w(offs_t offset, uint8_t data);
	uint8_t mz3500_fdc_r();
	void mz3500_fdc_w(uint8_t data);
	uint8_t mz3500_fdc_dma_r();
	void mz3500_pa_w(uint8_t data);
	void mz3500_pb_w(uint8_t data);
	void mz3500_pc_w(uint8_t data);

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );

	void mz3500_master_io(address_map &map) ATTR_COLD;
	void mz3500_master_map(address_map &map) ATTR_COLD;
	void mz3500_slave_io(address_map &map) ATTR_COLD;
	void mz3500_slave_map(address_map &map) ATTR_COLD;
	void upd7220_1_map(address_map &map) ATTR_COLD;
	void upd7220_2_map(address_map &map) ATTR_COLD;

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

private:
	required_ioport m_system_dsw;
	required_ioport m_fd_dsw;
	required_device_array<floppy_connector, 4> m_floppy_connector;
};

void mz3500_state::video_start()
{
}

/*
CRTC regs
[0]
---- -x-- "Choice of whether attribute or cursor be put on the frame  that displayed on CRT2" (whatever that means ...)
---- --x- CRT2 output
---- ---x CRT1 output
[1]
---- -GRB CRT1 color output
[2]
---- -GRB CRT2 color output
[3]
---- -GRB background color output
[4]
---- --x- border color mode in effect
---- ---x color mode
[5]
---- --x- width setting (0: 40 chars, 1: 80 chars)
---- ---x data size for graphics RAM (0: 8 bits, 1: 16 bits)
[6]
---- -x-- "Connection of graphic GDC"
---- --x- "Connection of the 96K bytes VRAM"
---- ---x "Connection of a 400 raster CRT"
[7]
---- ---x 0: 25 lines, 1: 20 lines display
[d]
(mirror of [5]?)
*/

UPD7220_DISPLAY_PIXELS_MEMBER( mz3500_state::hgdc_display_pixels )
{
	// ...
}

UPD7220_DRAW_TEXT_LINE_MEMBER( mz3500_state::hgdc_draw_text )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

//  popmessage("%02x",m_crtc[6]);

	uint8_t color_mode = m_crtc[4] & 1;
	uint8_t width80 = (m_crtc[5] & 2) >> 1;
	uint8_t hires = (m_crtc[6] & 1);
	uint8_t char_size = (hires) ? 16 : 8;

	for( int x = 0; x < pitch; x++ )
	{
		int tile = (m_video_ram[(((addr+x)*2) & 0x1fff) >> 1] & 0xff);
		int attr = ((m_video_ram[(((addr+x)*2+1) & 0x3ffff) >> 1] >> 8) & 0x0f);

		//if(hires)
		//  tile <<= 1;

		for( int yi = 0; yi < lr; yi++)
		{
			uint8_t tile_data = m_char_rom[((tile*16+yi) & 0xfff) | (hires*0x1000)];

			for( int xi = 0; xi < 8; xi++)
			{
				int pen;

				if(yi >= char_size)
					pen = -1;
				else
				{
					if(color_mode)
						pen = (tile_data >> (7-xi)) & 1 ? (attr ^ 7) : -1;
					else
					{
						/* TODO: "highlight"  */
						//if(attr & 4)
						//  tile_data ^= 0xff;

						if(attr & 1) // VL
							tile_data |= 8;

						/* TODO: correct position of HL */
						if(attr & 2 && yi == char_size/2) // HL
							tile_data |= 0xff;

						/* TODO: blink (bit 3) */

						pen = (tile_data >> (7-xi)) & 1 ? 7 : -1;
					}
				}

				int res_x = x * 8 + xi;
				int res_y = y + yi;

				if(pen != -1)
				{
					if(!width80)
					{
						bitmap.pix(res_y, res_x*2+0) = palette[pen];
						bitmap.pix(res_y, res_x*2+1) = palette[pen];
					}
					else
						bitmap.pix(res_y, res_x) = palette[pen];
				}
			}
		}
	}

}

uint32_t mz3500_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	bitmap.fill(m_palette->pen((m_crtc[4] & 2) ? m_crtc[3] & 7 : 0), cliprect);

	/* graphics */
	m_hgdc2->screen_update(screen, bitmap, cliprect);
	m_hgdc1->screen_update(screen, bitmap, cliprect);
	return 0;
}

uint8_t mz3500_state::mz3500_ipl_r(offs_t offset)
{
	return m_ipl_rom[offset];
}

uint8_t mz3500_state::mz3500_basic_r(offs_t offset)
{
	return m_basic_rom[offset];
}

uint8_t mz3500_state::mz3500_work_ram_r(offs_t offset)
{
	return m_work_ram[offset];
}

void mz3500_state::mz3500_work_ram_w(offs_t offset, uint8_t data)
{
	m_work_ram[offset] = data;
}


uint8_t mz3500_state::mz3500_master_mem_r(offs_t offset)
{
	if(m_ms == 0)
	{
		if((offset & 0xe000) == 0x0000) { return mz3500_ipl_r((offset & 0xfff) | 0x1000); }
		if((offset & 0xe000) == 0x2000) { return mz3500_basic_r((offset & 0x1fff) | 0x2000); }
		if((offset & 0xc000) == 0x4000) { return mz3500_work_ram_r((offset & 0x3fff) | 0x4000); }
		if((offset & 0xc000) == 0x8000) { return mz3500_work_ram_r((offset & 0x3fff) | 0x8000); }
		if((offset & 0xc000) == 0xc000)
		{
			if(m_ma == 0x0) { return mz3500_work_ram_r((offset & 0x3fff) | 0xc000); }
			if(m_ma == 0x1) { return mz3500_work_ram_r((offset & 0x3fff) | 0x0000); }
			if(m_ma == 0xf) { return mz3500_shared_ram_r((offset & 0x7ff)); }
		}

		printf("Read with unmapped memory bank offset %04x MS %02x MA %02x\n",offset,m_ms,m_ma);
	}
	else if(m_ms == 1)
	{
		return ((offset & 0xf800) == 0xf800) ? mz3500_shared_ram_r((offset & 0x7ff)) : mz3500_work_ram_r(offset);
	}
	else if(m_ms == 2) // ROM based BASIC
	{
		if((offset & 0xe000) == 0x0000) { return mz3500_basic_r(offset & 0x1fff); }
		if((offset & 0xe000) == 0x2000)
		{
			switch(m_mo)
			{
				case 0x0: return mz3500_basic_r((offset & 0x1fff) | 0x2000);
				case 0x1: return mz3500_basic_r((offset & 0x1fff) | 0x4000);
				case 0x2: return mz3500_basic_r((offset & 0x1fff) | 0x6000);
			}
		}
		if((offset & 0xc000) == 0x4000) { return mz3500_work_ram_r((offset & 0x3fff) | 0x4000); }
		if((offset & 0xc000) == 0x8000) { return mz3500_work_ram_r((offset & 0x3fff) | 0x8000); }
		if((offset & 0xc000) == 0xc000)
		{
			switch(m_ma)
			{
				case 0x0: return mz3500_work_ram_r((offset & 0x3fff) | 0x0c000);
				case 0x1: return mz3500_work_ram_r((offset & 0x3fff) | 0x00000);
				case 0x2: return mz3500_work_ram_r((offset & 0x3fff) | 0x10000);
				case 0x3: return mz3500_work_ram_r((offset & 0x3fff) | 0x14000);
				case 0x4: return mz3500_work_ram_r((offset & 0x3fff) | 0x18000);
				case 0x5: return mz3500_work_ram_r((offset & 0x3fff) | 0x1c000);
				case 0x6: return mz3500_work_ram_r((offset & 0x3fff) | 0x20000);
				case 0x7: return mz3500_work_ram_r((offset & 0x3fff) | 0x24000);
				case 0x8: return mz3500_work_ram_r((offset & 0x3fff) | 0x28000);
				case 0x9: return mz3500_work_ram_r((offset & 0x3fff) | 0x2c000);
				case 0xa: return mz3500_work_ram_r((offset & 0x3fff) | 0x30000);
				case 0xb: return mz3500_work_ram_r((offset & 0x3fff) | 0x34000);
				case 0xc: return mz3500_work_ram_r((offset & 0x3fff) | 0x38000);
				case 0xd: return mz3500_work_ram_r((offset & 0x3fff) | 0x3c000);
				case 0xf: return mz3500_shared_ram_r((offset & 0x7ff));
			}
		}

		printf("Read with unmapped memory bank offset %04x MS %02x MA %02x MO %02x\n",offset,m_ms,m_ma,m_mo);
	}
	else if (m_ms == 3) // RAM based BASIC
	{
		if((offset & 0xe000) == 0x0000) { return mz3500_work_ram_r(offset & 0x1fff); }
		if((offset & 0xe000) == 0x2000)
		{
			switch(m_mo)
			{
				case 0x0: return mz3500_work_ram_r((offset & 0x1fff) | 0x2000);
				case 0x1: return mz3500_work_ram_r((offset & 0x1fff) | 0xc000);
				case 0x2: return mz3500_work_ram_r((offset & 0x1fff) | 0xe000);
			}

			printf("Read with unmapped memory bank offset %04x MS %02x MO %02x\n",offset,m_ms,m_mo);
		}
		if((offset & 0xc000) == 0x4000) { return mz3500_work_ram_r((offset & 0x3fff) | 0x4000); }
		if((offset & 0xc000) == 0x8000) { return mz3500_work_ram_r((offset & 0x3fff) | 0x8000); }
		if((offset & 0xc000) == 0xc000)
		{
			switch(m_ma)
			{
				case 0x0: return mz3500_work_ram_r((offset & 0x3fff) | 0x10000);
				case 0x1: return mz3500_work_ram_r((offset & 0x3fff) | 0x14000);
				case 0x2: return mz3500_work_ram_r((offset & 0x3fff) | 0x18000);
				case 0x3: return mz3500_work_ram_r((offset & 0x3fff) | 0x1c000);
				case 0x4: return mz3500_work_ram_r((offset & 0x3fff) | 0x20000);
				case 0x5: return mz3500_work_ram_r((offset & 0x3fff) | 0x24000);
				case 0x6: return mz3500_work_ram_r((offset & 0x3fff) | 0x28000);
				case 0x7: return mz3500_work_ram_r((offset & 0x3fff) | 0x2c000);
				case 0x8: return mz3500_work_ram_r((offset & 0x3fff) | 0x30000);
				case 0x9: return mz3500_work_ram_r((offset & 0x3fff) | 0x34000);
				case 0xa: return mz3500_work_ram_r((offset & 0x3fff) | 0x38000);
				case 0xb: return mz3500_work_ram_r((offset & 0x3fff) | 0x3c000);
				case 0xf: return mz3500_shared_ram_r((offset & 0x7ff));
			}
		}
	}


	return 0xff; // shouldn't happen
}

void mz3500_state::mz3500_master_mem_w(offs_t offset, uint8_t data)
{
	if(m_ms == 0) // Initialize State
	{
		if((offset & 0xc000) == 0x4000) { mz3500_work_ram_w((offset & 0x3fff) | 0x4000,data); return; }
		if((offset & 0xc000) == 0x8000) { mz3500_work_ram_w((offset & 0x3fff) | 0x8000,data); return; }
		if((offset & 0xc000) == 0xc000)
		{
			if(m_ma == 0x0) { mz3500_work_ram_w((offset & 0x3fff) | 0xc000,data); return; }
			if(m_ma == 0x1) { mz3500_work_ram_w((offset & 0x3fff) | 0x0000,data); return; }
			if(m_ma == 0xf) { mz3500_shared_ram_w((offset & 0x7ff),data); return; }
		}

		printf("Write with unmapped memory bank offset %04x data %02x MS %02x MA %02x\n",offset,data,m_ms,m_ma);
	}
	else if(m_ms == 1) // System Loading & CP/M
	{
		if((offset & 0xf800) == 0xf800)
			mz3500_shared_ram_w((offset & 0x7ff),data);
		else
			mz3500_work_ram_w(offset,data);

		return;
	}
	else if(m_ms == 2) // ROM based BASIC
	{
		if((offset & 0xc000) == 0x4000) { mz3500_work_ram_w((offset & 0x3fff) | 0x4000,data); return; }
		if((offset & 0xc000) == 0x8000) { mz3500_work_ram_w((offset & 0x3fff) | 0x8000,data); return; }
		if((offset & 0xc000) == 0xc000)
		{
			switch(m_ma)
			{
				case 0x0: mz3500_work_ram_w((offset & 0x3fff) | 0x0c000,data); return;
				case 0x1: mz3500_work_ram_w((offset & 0x3fff) | 0x00000,data); return;
				case 0x2: mz3500_work_ram_w((offset & 0x3fff) | 0x10000,data); return;
				case 0x3: mz3500_work_ram_w((offset & 0x3fff) | 0x14000,data); return;
				case 0x4: mz3500_work_ram_w((offset & 0x3fff) | 0x18000,data); return;
				case 0x5: mz3500_work_ram_w((offset & 0x3fff) | 0x1c000,data); return;
				case 0x6: mz3500_work_ram_w((offset & 0x3fff) | 0x20000,data); return;
				case 0x7: mz3500_work_ram_w((offset & 0x3fff) | 0x24000,data); return;
				case 0x8: mz3500_work_ram_w((offset & 0x3fff) | 0x28000,data); return;
				case 0x9: mz3500_work_ram_w((offset & 0x3fff) | 0x2c000,data); return;
				case 0xa: mz3500_work_ram_w((offset & 0x3fff) | 0x30000,data); return;
				case 0xb: mz3500_work_ram_w((offset & 0x3fff) | 0x34000,data); return;
				case 0xc: mz3500_work_ram_w((offset & 0x3fff) | 0x38000,data); return;
				case 0xd: mz3500_work_ram_w((offset & 0x3fff) | 0x3c000,data); return;
				case 0xf: mz3500_shared_ram_w((offset & 0x7ff),data); return;
			}
		}

		printf("Write with unmapped memory bank offset %04x data %02x MS %02x MA %02x\n",offset,data,m_ms,m_ma);
	}
	else if (m_ms == 3) // RAM based BASIC
	{
		if((offset & 0xe000) == 0x0000) { mz3500_work_ram_w(offset & 0x1fff,data); return; }
		if((offset & 0xe000) == 0x2000)
		{
			switch(m_mo)
			{
				case 0x0: mz3500_work_ram_w((offset & 0x1fff) | 0x2000,data); return;
				case 0x1: mz3500_work_ram_w((offset & 0x1fff) | 0xc000,data); return;
				case 0x2: mz3500_work_ram_w((offset & 0x1fff) | 0xe000,data); return;
			}

			printf("Read with unmapped memory bank offset %04x MS %02x MO %02x\n",offset,m_ms,m_mo);
		}
		if((offset & 0xc000) == 0x4000) { mz3500_work_ram_w((offset & 0x3fff) | 0x4000,data); return; }
		if((offset & 0xc000) == 0x8000) { mz3500_work_ram_w((offset & 0x3fff) | 0x8000,data); return; }
		if((offset & 0xc000) == 0xc000)
		{
			switch(m_ma)
			{
				case 0x0: mz3500_work_ram_w((offset & 0x3fff) | 0x10000,data); return;
				case 0x1: mz3500_work_ram_w((offset & 0x3fff) | 0x14000,data); return;
				case 0x2: mz3500_work_ram_w((offset & 0x3fff) | 0x18000,data); return;
				case 0x3: mz3500_work_ram_w((offset & 0x3fff) | 0x1c000,data); return;
				case 0x4: mz3500_work_ram_w((offset & 0x3fff) | 0x20000,data); return;
				case 0x5: mz3500_work_ram_w((offset & 0x3fff) | 0x24000,data); return;
				case 0x6: mz3500_work_ram_w((offset & 0x3fff) | 0x28000,data); return;
				case 0x7: mz3500_work_ram_w((offset & 0x3fff) | 0x2c000,data); return;
				case 0x8: mz3500_work_ram_w((offset & 0x3fff) | 0x30000,data); return;
				case 0x9: mz3500_work_ram_w((offset & 0x3fff) | 0x34000,data); return;
				case 0xa: mz3500_work_ram_w((offset & 0x3fff) | 0x38000,data); return;
				case 0xb: mz3500_work_ram_w((offset & 0x3fff) | 0x3c000,data); return;
				case 0xf: mz3500_shared_ram_w((offset & 0x7ff),data); return;
			}
		}
	}
}

uint8_t mz3500_state::mz3500_shared_ram_r(offs_t offset)
{
	return m_shared_ram[offset];
}

void mz3500_state::mz3500_shared_ram_w(offs_t offset, uint8_t data)
{
	m_shared_ram[offset] = data;
}

uint8_t mz3500_state::mz3500_io_r(offs_t offset)
{
	/*
	[2]
	---x xxx- system assign switch
	---- ---x "SEC" FD assign
	[3]
	xxx- ---- FD assign
	---x ---- slave CPU Ready signal
	---- x--- slave CPU ack signal
	---- -xxx interrupt status
	*/

	switch(offset)
	{
		case 2:
			return ((m_system_dsw->read() & 0x0f) << 1) | ((m_fd_dsw->read() & 0x8) >> 3);
		case 3:
			return ((m_fd_dsw->read() & 0x7)<<5) | (m_srdy << 4);
	}

	return 0;
}

void mz3500_state::mz3500_io_w(offs_t offset, uint8_t data)
{
	/*
	[0]
	---- --x- SRQ bus request from master to slave
	---- ---x E1
	[1]
	x--- ---- slave reset signal
	---- --xx memory system define
	[2]
	xxxx ---- ma bank (memory 0xc000-0xffff)
	---- -xxx mo bank (memory 0x2000-0x3fff)
	[3]
	x--- ---- me2 bank (memory 0x8000-0xbfff)
	-x-- ---- me1 bank (memory 0x4000-0x7fff)
	*/

	switch(offset)
	{
		case 0:
			/* HACK: actually busreq */
			m_slave->set_input_line(INPUT_LINE_HALT, data & 2 ? ASSERT_LINE : CLEAR_LINE);

			break;
		case 1:
			m_ms = data & 3;
			break;
		case 2:
			m_ma = (data & 0xf0) >> 4;
			m_mo = (data & 0x07);
			break;
		case 3:
			m_me2 = (data & 0x80) >> 7;
			m_me1 = (data & 0x40) >> 6;
			break;
	}
}

void mz3500_state::mz3500_crtc_w(offs_t offset, uint8_t data)
{
	if(offset & 8)
	{
		if(offset == 0xd)
			m_crtc[offset & 7] = data;
		else
			printf("CRTC register access %02x\n",offset); // probably just a mirror, but who knows ...
	}
	else
		m_crtc[offset] = data;
}

uint8_t mz3500_state::mz3500_fdc_r()
{
	/*
	---- -x-- Motor
	---- --x- Index
	---- ---x Drq
	*/

	floppy_image_device *floppy = m_floppy_connector[m_fdd_sel]->get_device();

	return (floppy->mon_r() << 2) | (floppy->idx_r() << 1) | (m_fdc->get_drq() & 1);
}

void mz3500_state::mz3500_fdc_w(uint8_t data)
{
	/*
	x--- ---- FDC int enable
	-x-- ---- FDD select signal
	--x- ---- FDC TC
	---x ---- motor on signal
	---- xxxx Select FDD 0-3 (bit-wise)
	*/

	if(data & 0x40)
	{
		for(int i=0;i<4;i++)
		{
			if(data & 1 << i)
			{
				m_fdd_sel = i;
				break;
			}
		}
	}

	m_floppy_connector[m_fdd_sel]->get_device()->mon_w(data & 0x10 ? CLEAR_LINE : ASSERT_LINE);

}

uint8_t mz3500_state::mz3500_fdc_dma_r()
{
	return m_fdc->dma_r();
}

void mz3500_state::mz3500_master_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(mz3500_state::mz3500_master_mem_r), FUNC(mz3500_state::mz3500_master_mem_w));
}

void mz3500_state::mz3500_master_io(address_map &map)
{
	map.global_mask(0xff);
//  map.unmap_value_high();
//  map(0xe4, 0xe7) SFD upd765
//  map(0xe8, 0xeb) SFD I/O port and DMAC chip select
//  map(0xec, 0xef) irq signal from slave to master CPU
	map(0xf4, 0xf5).m(m_fdc, FUNC(upd765a_device::map)); // MFD upd765
//  map(0xf8, 0xfb) MFD I/O port
	map(0xf8, 0xf8).rw(FUNC(mz3500_state::mz3500_fdc_r), FUNC(mz3500_state::mz3500_fdc_w));
	map(0xf9, 0xf9).r(FUNC(mz3500_state::mz3500_fdc_dma_r));
	map(0xfc, 0xff).rw(FUNC(mz3500_state::mz3500_io_r), FUNC(mz3500_state::mz3500_io_w)); // memory mapper
}

void mz3500_state::mz3500_slave_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("ipl", 0);
	map(0x2000, 0x27ff).rw(FUNC(mz3500_state::mz3500_shared_ram_r), FUNC(mz3500_state::mz3500_shared_ram_w));
	map(0x4000, 0x5fff).ram();
}

void mz3500_state::mz3500_slave_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
//  map(0x00, 0x0f) f/f and irq to master CPU
//  map(0x10, 0x1f) i8251
//  map(0x20, 0x2f) pit8253
	map(0x30, 0x33).rw("i8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x40).portr("DSW");
	map(0x50, 0x5f).ram().w(FUNC(mz3500_state::mz3500_crtc_w));
	map(0x60, 0x61).rw(m_hgdc2, FUNC(upd7220_device::read), FUNC(upd7220_device::write));
	map(0x70, 0x71).rw(m_hgdc1, FUNC(upd7220_device::read), FUNC(upd7220_device::write));
}

void mz3500_state::mz3500_pa_w(uint8_t data)
{
	// printer data
}

void mz3500_state::mz3500_pb_w(uint8_t data)
{
	/*
	x--- ---- CG select (ROM and/or upd7220 clock?)
	-x-- ---- SRDY signal (to master)
	--xx xxxx upd1990 RTC (CLK, Din, C2, C1, C0, STRB)

	*/
	//printf("%02x PB\n",data);

	m_srdy = (data & 0x40) >> 6;
}

void mz3500_state::mz3500_pc_w(uint8_t data)
{
	/*
	x--- ---- printer OBF output
	-x-- ---- printer ACK input
	--x- ---- printer STROBE
	---x ---- buzzer
	---- -xxx Keyboard (ACKC, STC, DC)
	*/
	//printf("%02x PC\n",data);

	m_beeper->set_state(data & 0x10);

}

static INPUT_PORTS_START( mz3500 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Sub-CPU Test" ) /* hack */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FD_DSW")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM_DSW")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "CRT Select" )
	PORT_DIPSETTING(    0x04, "Normal Display (MZ1D01, MZ1D06)" )
	PORT_DIPSETTING(    0x00, "Hi-Res Display (MZ1D02, MZ1D03)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout_8x8 =
{
	8,8,
	0x100,
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*16
};


static const gfx_layout charlayout_8x16 =
{
	8,16,
	0x100,
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ STEP16(0,8) },
	8*16
};

static GFXDECODE_START( gfx_mz3500 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout_8x8,     0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x0008, charlayout_8x8,     0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, charlayout_8x16,     0, 1 )
GFXDECODE_END

void mz3500_state::machine_start()
{
	m_ipl_rom = memregion("ipl")->base();
	m_basic_rom = memregion("basic")->base();
	m_char_rom = memregion("gfx1")->base();
	m_work_ram = make_unique_clear<uint8_t[]>(0x40000);
	m_shared_ram = make_unique_clear<uint8_t[]>(0x800);
}

void mz3500_state::machine_reset()
{
	/* init memory bank states */
	m_ms = 0;
	m_ma = 0;
	m_mo = 0;
	m_me1 = 0;
	m_me2 = 0;
	//m_slave->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_srdy = 0;

	m_fdd_sel = 0;
	for(auto & elem : m_floppy_connector)
	{
		elem->get_device()->mon_w(ASSERT_LINE);
		elem->get_device()->set_rpm(300);
	}

	m_fdc->set_rate(250000);

	m_beeper->set_state(0);
}


void mz3500_state::upd7220_1_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x00000, 0x00fff).ram().share("video_ram");
}

void mz3500_state::upd7220_2_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram(); // .share("video_ram_2");
}

static void mz3500_floppies(device_slot_interface &device)
{
	device.option_add("525ssdd", FLOPPY_525_SSDD);
}

/* TODO: clocks */
void mz3500_state::mz3500(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_master, MAIN_CLOCK/2);
	m_master->set_addrmap(AS_PROGRAM, &mz3500_state::mz3500_master_map);
	m_master->set_addrmap(AS_IO, &mz3500_state::mz3500_master_io);

	Z80(config, m_slave, MAIN_CLOCK/2);
	m_slave->set_addrmap(AS_PROGRAM, &mz3500_state::mz3500_slave_map);
	m_slave->set_addrmap(AS_IO, &mz3500_state::mz3500_slave_io);

	config.set_perfect_quantum(m_master);

	i8255_device &ppi(I8255A(config, "i8255"));
	ppi.out_pa_callback().set(FUNC(mz3500_state::mz3500_pa_w));
	ppi.out_pb_callback().set(FUNC(mz3500_state::mz3500_pb_w));
	ppi.out_pc_callback().set(FUNC(mz3500_state::mz3500_pc_w));

	UPD765A(config, m_fdc, 8'000'000, true, true);
	m_fdc->intrq_wr_callback().set_inputline(m_master, INPUT_LINE_IRQ0);
	FLOPPY_CONNECTOR(config, "upd765a:0", mz3500_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765a:1", mz3500_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765a:2", mz3500_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765a:3", mz3500_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);

	UPD7220(config, m_hgdc1, MAIN_CLOCK/5);
	m_hgdc1->set_addrmap(0, &mz3500_state::upd7220_1_map);
	m_hgdc1->set_draw_text(FUNC(mz3500_state::hgdc_draw_text));
	m_hgdc1->vsync_wr_callback().set(m_hgdc2, FUNC(upd7220_device::ext_sync_w));

	UPD7220(config, m_hgdc2, MAIN_CLOCK/5);
	m_hgdc2->set_addrmap(0, &mz3500_state::upd7220_2_map);
	m_hgdc2->set_display_pixels(FUNC(mz3500_state::hgdc_display_pixels));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(mz3500_state::screen_update));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_mz3500);

	PALETTE(config, m_palette, palette_device::BRG_3BIT);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	BEEP(config, m_beeper, 2400).add_route(ALL_OUTPUTS, "mono", 0.15);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( mz3500 )
	ROM_REGION( 0x2000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD( "mz-3500_ipl-rom_2-0a_m5l2764k.bin", 0x000000, 0x002000, CRC(119708b9) SHA1(de81979608ba6ab76f09088a92bfd1a5bc42530e) )

	ROM_REGION( 0x8000, "basic", ROMREGION_ERASE00 )
	ROM_LOAD( "basic.rom", 0x00000, 0x8000, NO_DUMP )

	ROM_REGION( 0x2000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "mz-3500_cg-rom_2-b_m5l2764k.bin", 0x000000, 0x002000, CRC(29f2f80a) SHA1(64b307cd9de5a3327e3ec9f3d0d6b3485706f436) )
ROM_END

} // anonymous namespace


COMP( 198?, mz3500, 0, 0, mz3500, mz3500, mz3500_state, empty_init, "Sharp", "MZ-3500", MACHINE_IS_SKELETON )
