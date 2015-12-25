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
#include "machine/upd765.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "sound/beep.h"
#include "video/upd7220.h"

#define MAIN_CLOCK XTAL_8MHz

class mz3500_state : public driver_device
{
public:
	mz3500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_master(*this, "master"),
			m_slave(*this, "slave"),
			m_hgdc1(*this, "upd7220_chr"),
			m_hgdc2(*this, "upd7220_gfx"),
			m_fdc(*this, "upd765a"),
			m_video_ram(*this, "video_ram"),
			m_beeper(*this, "beeper"),
			m_palette(*this, "palette"),
			m_system_dsw(*this, "SYSTEM_DSW"),
			m_fd_dsw(*this, "FD_DSW")
	{ }

	// devices
	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	required_device<upd7220_device> m_hgdc1;
	required_device<upd7220_device> m_hgdc2;
	required_device<upd765a_device> m_fdc;
	required_shared_ptr<UINT16> m_video_ram;
	required_device<beep_device> m_beeper;
	required_device<palette_device> m_palette;

	UINT8 *m_ipl_rom;
	UINT8 *m_basic_rom;
	std::unique_ptr<UINT8[]> m_work_ram;
	std::unique_ptr<UINT8[]> m_shared_ram;
	UINT8 *m_char_rom;

	UINT8 m_ma,m_mo,m_ms,m_me2,m_me1;
	UINT8 m_crtc[0x10];

	UINT8 m_srdy;

	UINT8 m_fdd_sel;

	DECLARE_READ8_MEMBER(mz3500_master_mem_r);
	DECLARE_WRITE8_MEMBER(mz3500_master_mem_w);
	DECLARE_READ8_MEMBER(mz3500_ipl_r);
	DECLARE_READ8_MEMBER(mz3500_basic_r);
	DECLARE_READ8_MEMBER(mz3500_work_ram_r);
	DECLARE_WRITE8_MEMBER(mz3500_work_ram_w);
	DECLARE_READ8_MEMBER(mz3500_shared_ram_r);
	DECLARE_WRITE8_MEMBER(mz3500_shared_ram_w);
	DECLARE_READ8_MEMBER(mz3500_io_r);
	DECLARE_WRITE8_MEMBER(mz3500_io_w);
	DECLARE_WRITE8_MEMBER(mz3500_crtc_w);
	DECLARE_READ8_MEMBER(mz3500_fdc_r);
	DECLARE_WRITE8_MEMBER(mz3500_fdc_w);
	DECLARE_READ8_MEMBER(mz3500_fdc_dma_r);
	DECLARE_WRITE8_MEMBER(mz3500_pa_w);
	DECLARE_WRITE8_MEMBER(mz3500_pb_w);
	DECLARE_WRITE8_MEMBER(mz3500_pc_w);

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	required_ioport m_system_dsw;
	required_ioport m_fd_dsw;
	floppy_connector *m_floppy_connector[4];
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
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int x;
	int xi,yi;
	int tile;
	int attr;
	UINT8 tile_data;
	UINT8 width80;
	UINT8 char_size;
	UINT8 hires;
	UINT8 color_mode;

//  popmessage("%02x",m_crtc[6]);

	color_mode = m_crtc[4] & 1;
	width80 = (m_crtc[5] & 2) >> 1;
	hires = (m_crtc[6] & 1);
	char_size = (hires) ? 16 : 8;

	for( x = 0; x < pitch; x++ )
	{
		tile = (m_video_ram[(((addr+x)*2) & 0x1fff) >> 1] & 0xff);
		attr = ((m_video_ram[(((addr+x)*2+1) & 0x3ffff) >> 1] >> 8) & 0x0f);

		//if(hires)
		//  tile <<= 1;

		for( yi = 0; yi < lr; yi++)
		{
			tile_data = m_char_rom[((tile*16+yi) & 0xfff) | (hires*0x1000)];

			for( xi = 0; xi < 8; xi++)
			{
				int res_x,res_y;
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

				res_x = x * 8 + xi;
				res_y = y + yi;

				if(pen != -1)
				{
					if(!width80)
					{
						bitmap.pix32(res_y, res_x*2+0) = palette[pen];
						bitmap.pix32(res_y, res_x*2+1) = palette[pen];
					}
					else
						bitmap.pix32(res_y, res_x) = palette[pen];
				}
			}
		}
	}

}

UINT32 mz3500_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	bitmap.fill(m_palette->pen((m_crtc[4] & 2) ? m_crtc[3] & 7 : 0), cliprect);

	/* graphics */
	m_hgdc2->screen_update(screen, bitmap, cliprect);
	m_hgdc1->screen_update(screen, bitmap, cliprect);
	return 0;
}

READ8_MEMBER(mz3500_state::mz3500_ipl_r)
{
	return m_ipl_rom[offset];
}

READ8_MEMBER(mz3500_state::mz3500_basic_r)
{
	return m_basic_rom[offset];
}

READ8_MEMBER(mz3500_state::mz3500_work_ram_r)
{
	return m_work_ram[offset];
}

WRITE8_MEMBER(mz3500_state::mz3500_work_ram_w)
{
	m_work_ram[offset] = data;
}


READ8_MEMBER(mz3500_state::mz3500_master_mem_r)
{
	if(m_ms == 0)
	{
		if((offset & 0xe000) == 0x0000) { return mz3500_ipl_r(space,(offset & 0xfff) | 0x1000); }
		if((offset & 0xe000) == 0x2000) { return mz3500_basic_r(space,(offset & 0x1fff) | 0x2000); }
		if((offset & 0xc000) == 0x4000) { return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x4000); }
		if((offset & 0xc000) == 0x8000) { return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x8000); }
		if((offset & 0xc000) == 0xc000)
		{
			if(m_ma == 0x0) { return mz3500_work_ram_r(space,(offset & 0x3fff) | 0xc000); }
			if(m_ma == 0x1) { return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x0000); }
			if(m_ma == 0xf) { return mz3500_shared_ram_r(space,(offset & 0x7ff)); }
		}

		printf("Read with unmapped memory bank offset %04x MS %02x MA %02x\n",offset,m_ms,m_ma);
	}
	else if(m_ms == 1)
	{
		return ((offset & 0xf800) == 0xf800) ? mz3500_shared_ram_r(space,(offset & 0x7ff)) : mz3500_work_ram_r(space,offset);
	}
	else if(m_ms == 2) // ROM based BASIC
	{
		if((offset & 0xe000) == 0x0000) { return mz3500_basic_r(space,offset & 0x1fff); }
		if((offset & 0xe000) == 0x2000)
		{
			switch(m_mo)
			{
				case 0x0: return mz3500_basic_r(space,(offset & 0x1fff) | 0x2000);
				case 0x1: return mz3500_basic_r(space,(offset & 0x1fff) | 0x4000);
				case 0x2: return mz3500_basic_r(space,(offset & 0x1fff) | 0x6000);
			}
		}
		if((offset & 0xc000) == 0x4000) { return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x4000); }
		if((offset & 0xc000) == 0x8000) { return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x8000); }
		if((offset & 0xc000) == 0xc000)
		{
			switch(m_ma)
			{
				case 0x0: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x0c000);
				case 0x1: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x00000);
				case 0x2: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x10000);
				case 0x3: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x14000);
				case 0x4: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x18000);
				case 0x5: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x1c000);
				case 0x6: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x20000);
				case 0x7: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x24000);
				case 0x8: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x28000);
				case 0x9: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x2c000);
				case 0xa: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x30000);
				case 0xb: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x34000);
				case 0xc: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x38000);
				case 0xd: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x3c000);
				case 0xf: return mz3500_shared_ram_r(space,(offset & 0x7ff));
			}
		}

		printf("Read with unmapped memory bank offset %04x MS %02x MA %02x MO %02x\n",offset,m_ms,m_ma,m_mo);
	}
	else if (m_ms == 3) // RAM based BASIC
	{
		if((offset & 0xe000) == 0x0000) { return mz3500_work_ram_r(space,offset & 0x1fff); }
		if((offset & 0xe000) == 0x2000)
		{
			switch(m_mo)
			{
				case 0x0: return mz3500_work_ram_r(space,(offset & 0x1fff) | 0x2000);
				case 0x1: return mz3500_work_ram_r(space,(offset & 0x1fff) | 0xc000);
				case 0x2: return mz3500_work_ram_r(space,(offset & 0x1fff) | 0xe000);
			}

			printf("Read with unmapped memory bank offset %04x MS %02x MO %02x\n",offset,m_ms,m_mo);
		}
		if((offset & 0xc000) == 0x4000) { return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x4000); }
		if((offset & 0xc000) == 0x8000) { return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x8000); }
		if((offset & 0xc000) == 0xc000)
		{
			switch(m_ma)
			{
				case 0x0: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x10000);
				case 0x1: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x14000);
				case 0x2: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x18000);
				case 0x3: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x1c000);
				case 0x4: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x20000);
				case 0x5: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x24000);
				case 0x6: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x28000);
				case 0x7: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x2c000);
				case 0x8: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x30000);
				case 0x9: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x34000);
				case 0xa: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x38000);
				case 0xb: return mz3500_work_ram_r(space,(offset & 0x3fff) | 0x3c000);
				case 0xf: return mz3500_shared_ram_r(space,(offset & 0x7ff));
			}
		}
	}


	return 0xff; // shouldn't happen
}

WRITE8_MEMBER(mz3500_state::mz3500_master_mem_w)
{
	if(m_ms == 0) // Initialize State
	{
		if((offset & 0xc000) == 0x4000) { mz3500_work_ram_w(space,(offset & 0x3fff) | 0x4000,data); return; }
		if((offset & 0xc000) == 0x8000) { mz3500_work_ram_w(space,(offset & 0x3fff) | 0x8000,data); return; }
		if((offset & 0xc000) == 0xc000)
		{
			if(m_ma == 0x0) { mz3500_work_ram_w(space,(offset & 0x3fff) | 0xc000,data); return; }
			if(m_ma == 0x1) { mz3500_work_ram_w(space,(offset & 0x3fff) | 0x0000,data); return; }
			if(m_ma == 0xf) { mz3500_shared_ram_w(space,(offset & 0x7ff),data); return; }
		}

		printf("Write with unmapped memory bank offset %04x data %02x MS %02x MA %02x\n",offset,data,m_ms,m_ma);
	}
	else if(m_ms == 1) // System Loading & CP/M
	{
		if((offset & 0xf800) == 0xf800)
			mz3500_shared_ram_w(space,(offset & 0x7ff),data);
		else
			mz3500_work_ram_w(space,offset,data);

		return;
	}
	else if(m_ms == 2) // ROM based BASIC
	{
		if((offset & 0xc000) == 0x4000) { mz3500_work_ram_w(space,(offset & 0x3fff) | 0x4000,data); return; }
		if((offset & 0xc000) == 0x8000) { mz3500_work_ram_w(space,(offset & 0x3fff) | 0x8000,data); return; }
		if((offset & 0xc000) == 0xc000)
		{
			switch(m_ma)
			{
				case 0x0: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x0c000,data); return;
				case 0x1: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x00000,data); return;
				case 0x2: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x10000,data); return;
				case 0x3: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x14000,data); return;
				case 0x4: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x18000,data); return;
				case 0x5: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x1c000,data); return;
				case 0x6: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x20000,data); return;
				case 0x7: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x24000,data); return;
				case 0x8: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x28000,data); return;
				case 0x9: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x2c000,data); return;
				case 0xa: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x30000,data); return;
				case 0xb: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x34000,data); return;
				case 0xc: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x38000,data); return;
				case 0xd: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x3c000,data); return;
				case 0xf: mz3500_shared_ram_w(space,(offset & 0x7ff),data); return;
			}
		}

		printf("Write with unmapped memory bank offset %04x data %02x MS %02x MA %02x\n",offset,data,m_ms,m_ma);
	}
	else if (m_ms == 3) // RAM based BASIC
	{
		if((offset & 0xe000) == 0x0000) { mz3500_work_ram_w(space,offset & 0x1fff,data); return; }
		if((offset & 0xe000) == 0x2000)
		{
			switch(m_mo)
			{
				case 0x0: mz3500_work_ram_w(space,(offset & 0x1fff) | 0x2000,data); return;
				case 0x1: mz3500_work_ram_w(space,(offset & 0x1fff) | 0xc000,data); return;
				case 0x2: mz3500_work_ram_w(space,(offset & 0x1fff) | 0xe000,data); return;
			}

			printf("Read with unmapped memory bank offset %04x MS %02x MO %02x\n",offset,m_ms,m_mo);
		}
		if((offset & 0xc000) == 0x4000) { mz3500_work_ram_w(space,(offset & 0x3fff) | 0x4000,data); return; }
		if((offset & 0xc000) == 0x8000) { mz3500_work_ram_w(space,(offset & 0x3fff) | 0x8000,data); return; }
		if((offset & 0xc000) == 0xc000)
		{
			switch(m_ma)
			{
				case 0x0: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x10000,data); return;
				case 0x1: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x14000,data); return;
				case 0x2: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x18000,data); return;
				case 0x3: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x1c000,data); return;
				case 0x4: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x20000,data); return;
				case 0x5: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x24000,data); return;
				case 0x6: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x28000,data); return;
				case 0x7: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x2c000,data); return;
				case 0x8: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x30000,data); return;
				case 0x9: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x34000,data); return;
				case 0xa: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x38000,data); return;
				case 0xb: mz3500_work_ram_w(space,(offset & 0x3fff) | 0x3c000,data); return;
				case 0xf: mz3500_shared_ram_w(space,(offset & 0x7ff),data); return;
			}
		}
	}
}

READ8_MEMBER(mz3500_state::mz3500_shared_ram_r)
{
	return m_shared_ram[offset];
}

WRITE8_MEMBER(mz3500_state::mz3500_shared_ram_w)
{
	m_shared_ram[offset] = data;
}

READ8_MEMBER(mz3500_state::mz3500_io_r)
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

WRITE8_MEMBER(mz3500_state::mz3500_io_w)
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

WRITE8_MEMBER(mz3500_state::mz3500_crtc_w)
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

READ8_MEMBER(mz3500_state::mz3500_fdc_r)
{
	/*
	---- -x-- Motor
	---- --x- Index
	---- ---x Drq
	*/

	floppy_image_device *floppy = m_floppy_connector[m_fdd_sel]->get_device();

	return (floppy->mon_r() << 2) | (floppy->idx_r() << 1) | (m_fdc->get_drq() & 1);
}

WRITE8_MEMBER(mz3500_state::mz3500_fdc_w)
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

READ8_MEMBER(mz3500_state::mz3500_fdc_dma_r)
{
	return m_fdc->dma_r();
}

static ADDRESS_MAP_START( mz3500_master_map, AS_PROGRAM, 8, mz3500_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(mz3500_master_mem_r,mz3500_master_mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mz3500_master_io, AS_IO, 8, mz3500_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE(0xe4, 0xe7) SFD upd765
//  AM_RANGE(0xe8, 0xeb) SFD I/O port and DMAC chip select
//  AM_RANGE(0xec, 0xef) irq signal from slave to master CPU
	AM_RANGE(0xf4, 0xf5) AM_DEVICE("upd765a", upd765a_device, map) // MFD upd765
//  AM_RANGE(0xf8, 0xfb) MFD I/O port
	AM_RANGE(0xf8, 0xf8) AM_READWRITE(mz3500_fdc_r,mz3500_fdc_w)
	AM_RANGE(0xf9, 0xf9) AM_READ(mz3500_fdc_dma_r)
	AM_RANGE(0xfc, 0xff) AM_READWRITE(mz3500_io_r,mz3500_io_w) // memory mapper
ADDRESS_MAP_END

static ADDRESS_MAP_START( mz3500_slave_map, AS_PROGRAM, 8, mz3500_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("ipl", 0)
	AM_RANGE(0x2000, 0x27ff) AM_READWRITE(mz3500_shared_ram_r, mz3500_shared_ram_w)
	AM_RANGE(0x4000, 0x5fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mz3500_slave_io, AS_IO, 8, mz3500_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE(0x00, 0x0f) f/f and irq to master CPU
//  AM_RANGE(0x10, 0x1f) i8251
//  AM_RANGE(0x20, 0x2f) pit8253
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE("i8255", i8255_device, read, write)
	AM_RANGE(0x40, 0x40) AM_READ_PORT("DSW")
	AM_RANGE(0x50, 0x5f) AM_RAM_WRITE(mz3500_crtc_w)
	AM_RANGE(0x60, 0x61) AM_DEVREADWRITE("upd7220_gfx", upd7220_device, read, write)
	AM_RANGE(0x70, 0x71) AM_DEVREADWRITE("upd7220_chr", upd7220_device, read, write)
ADDRESS_MAP_END

WRITE8_MEMBER(mz3500_state::mz3500_pa_w)
{
	// printer data
}

WRITE8_MEMBER(mz3500_state::mz3500_pb_w)
{
	/*
	x--- ---- CG select (ROM and/or upd7220 clock?)
	-x-- ---- SRDY signal (to master)
	--xx xxxx upd1990 RTC (CLK, Din, C2, C1, C0, STRB)

	*/
	//printf("%02x PB\n",data);

	m_srdy = (data & 0x40) >> 6;
}

WRITE8_MEMBER(mz3500_state::mz3500_pc_w)
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

static GFXDECODE_START( mz3500 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout_8x8,     0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x0008, charlayout_8x8,     0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, charlayout_8x16,     0, 1 )
GFXDECODE_END

void mz3500_state::machine_start()
{
	m_ipl_rom = memregion("ipl")->base();
	m_basic_rom = memregion("basic")->base();
	m_char_rom = memregion("gfx1")->base();
	m_work_ram = make_unique_clear<UINT8[]>(0x40000);
	m_shared_ram = make_unique_clear<UINT8[]>(0x800);

	static const char *const m_fddnames[4] = { "upd765a:0", "upd765a:1", "upd765a:2", "upd765a:3"};

	for (int i = 0; i < 4; i++)
	{
		m_floppy_connector[i] = machine().device<floppy_connector>(m_fddnames[i]);
	}
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

	upd765a_device *fdc;
	fdc = machine().device<upd765a_device>(":upd765a");

	if (fdc)
	{
		m_fdd_sel = 0;
		{
			for(auto & elem : m_floppy_connector)
			{
				elem->get_device()->mon_w(ASSERT_LINE);
				elem->get_device()->set_rpm(300);
			}

			machine().device<upd765a_device>("upd765a")->set_rate(250000);
		}
	}

	m_beeper->set_frequency(2400);
	m_beeper->set_state(0);
}


static ADDRESS_MAP_START( upd7220_1_map, AS_0, 16, mz3500_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x00000, 0x00fff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( upd7220_2_map, AS_0, 16, mz3500_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM // AM_SHARE("video_ram_2")
ADDRESS_MAP_END

static SLOT_INTERFACE_START( mz3500_floppies )
	SLOT_INTERFACE( "525ssdd", FLOPPY_525_SSDD )
SLOT_INTERFACE_END

/* TODO: clocks */
static MACHINE_CONFIG_START( mz3500, mz3500_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("master",Z80,MAIN_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(mz3500_master_map)
	MCFG_CPU_IO_MAP(mz3500_master_io)

	MCFG_CPU_ADD("slave",Z80,MAIN_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(mz3500_slave_map)
	MCFG_CPU_IO_MAP(mz3500_slave_io)

	MCFG_QUANTUM_PERFECT_CPU("master")

	MCFG_DEVICE_ADD("i8255", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(mz3500_state, mz3500_pa_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(mz3500_state, mz3500_pb_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mz3500_state, mz3500_pc_w))

	MCFG_UPD765A_ADD("upd765a", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE("master", INPUT_LINE_IRQ0))
	MCFG_FLOPPY_DRIVE_ADD("upd765a:0", mz3500_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765a:1", mz3500_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765a:2", mz3500_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765a:3", mz3500_floppies, "525ssdd", floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD("upd7220_chr", UPD7220, MAIN_CLOCK/5)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, upd7220_1_map)
	MCFG_UPD7220_DRAW_TEXT_CALLBACK_OWNER(mz3500_state, hgdc_draw_text)
	MCFG_UPD7220_VSYNC_CALLBACK(DEVWRITELINE("upd7220_gfx", upd7220_device, ext_sync_w))

	MCFG_DEVICE_ADD("upd7220_gfx", UPD7220, MAIN_CLOCK/5)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, upd7220_2_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(mz3500_state, hgdc_display_pixels)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(mz3500_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mz3500)

	MCFG_PALETTE_ADD_3BIT_BRG("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.15)
MACHINE_CONFIG_END


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

COMP( 198?, mz3500,  0,   0,   mz3500,  mz3500, driver_device,  0,  "Sharp",      "MZ-3500", MACHINE_IS_SKELETON )
