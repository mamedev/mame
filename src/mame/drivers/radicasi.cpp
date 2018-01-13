// license:BSD-3-Clause
// copyright-holders:David Haywood, R.Belmont

/*
	Radica Games 6502 based 'TV Game' hardware

	These use a 6502 derived CPU under a glob
	The CPU die is marked 'ELAN EU3A05'

	There is a second glob surrounded by TSOP48 pads
	this contains the ROM

	Space Invaders uses a 3rd glob marked
	AMIC (C) (M) 1998-1 AM3122A
	this is presumably for the bitmap layer on Qix

	--
	Known games on this hardare

	Tetris
	Space Invaders

	---
	Other games that might be on this hardware

	Golden Tee Home Edition
	Skateboarding
	+ some of the earlier PlayTV games (not Soccer, that's XaviX, see xavix.cpp)

	---
	The XaviX ones seem to have a XaviX logo on the external packaging while the
	ones for this driver don't seem to have any specific marking.
*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "screen.h"
#include "speaker.h"
#include "machine/bankdev.h"
//#include "cpu/m6502/r65c02.h"

class radica_6502_state : public driver_device
{
public:
	radica_6502_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_pixram(*this, "pixram"),
		m_bank(*this, "bank"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(radicasi_500c_w);
	DECLARE_WRITE8_MEMBER(radicasi_500d_w);
	
	// palette bases
	DECLARE_WRITE8_MEMBER(radicasi_palbase_lo_w);
	DECLARE_WRITE8_MEMBER(radicasi_palbase_hi_w);
	DECLARE_READ8_MEMBER(radicasi_palbase_lo_r);
	DECLARE_READ8_MEMBER(radicasi_palbase_hi_r);


	// tile bases
	DECLARE_WRITE8_MEMBER(radicasi_tile_gfxbase_lo_w);
	DECLARE_WRITE8_MEMBER(radicasi_tile_gfxbase_hi_w);
	DECLARE_READ8_MEMBER(radicasi_tile_gfxbase_lo_r);
	DECLARE_READ8_MEMBER(radicasi_tile_gfxbase_hi_r);

	// sprite tile bases
	DECLARE_WRITE8_MEMBER(radicasi_sprite_gfxbase_lo_w);
	DECLARE_WRITE8_MEMBER(radicasi_sprite_gfxbase_hi_w);
	DECLARE_READ8_MEMBER(radicasi_sprite_gfxbase_lo_r);
	DECLARE_READ8_MEMBER(radicasi_sprite_gfxbase_hi_r);

	// unknown rom bases
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_0_0_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_0_0_r);
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_0_1_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_0_1_r);
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_0_2_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_0_2_r);
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_0_3_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_0_3_r);
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_0_4_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_0_4_r);
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_0_5_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_0_5_r);

	DECLARE_WRITE8_MEMBER(radicasi_unkregs_1_0_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_1_0_r);
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_1_1_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_1_1_r);
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_1_2_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_1_2_r);
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_1_3_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_1_3_r);
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_1_4_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_1_4_r);
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_1_5_w);
	DECLARE_READ8_MEMBER(radicasi_unkregs_1_5_r);

	DECLARE_READ8_MEMBER(radicasi_unkregs_trigger_r);
	DECLARE_WRITE8_MEMBER(radicasi_unkregs_trigger_w);

	DECLARE_READ8_MEMBER(radicasi_500b_r);
	DECLARE_READ8_MEMBER(radicasi_500d_r);
	DECLARE_READ8_MEMBER(radicasi_50a8_r);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_ram;
	required_shared_ptr<uint8_t> m_pixram;
	required_device<address_map_bank_device> m_bank;
	required_device<gfxdecode_device> m_gfxdecode;

	uint8_t m_500d_data;

	uint8_t m_palbase_lo_data;
	uint8_t m_palbase_hi_data;

	uint8_t m_tile_gfxbase_lo_data;
	uint8_t m_tile_gfxbase_hi_data;

	uint8_t m_sprite_gfxbase_lo_data;
	uint8_t m_sprite_gfxbase_hi_data;

	uint16_t m_unkregs_0_address[6];
	uint8_t m_unkregs_0_unk[6];

	uint8_t m_unkregs_1_unk0[6];
	uint8_t m_unkregs_1_unk1[6];
	uint8_t m_unkregs_1_unk2[6];

	uint8_t m_unkregs_trigger;

	void handle_trigger(int which);

	void handle_unkregs_0_w(int which, int offset, uint8_t data);
	uint8_t handle_unkregs_0_r(int which, int offset);
	void handle_unkregs_1_w(int which, int offset, uint8_t data);
	uint8_t handle_unkregs_1_r(int which, int offset);

	int m_hackmode;
};

void radica_6502_state::video_start()
{
	m_hackmode = 0;
}

/* (m_tile_gfxbase_lo_data | (m_tile_gfxbase_hi_data << 8)) * 0x100
   gives you the actual rom address, everything references the 3MByte - 4MByte region, like the banking so
   the system can probalby have up to a 4MByte rom, all games we have so far just use the upper 1MByte of
   that space
*/

uint32_t radica_6502_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	if (machine().input().code_pressed_once(KEYCODE_Q))
	{
		m_hackmode++;
		if (m_hackmode == 3) m_hackmode = 0;
	}

	// it is unclear if the tilemap is an internal structure or something actually used by the video rendering
	int offs = 0x600;

	// we draw the tiles as 8x1 strips as that's how they're stored in ROM
	// it might be they're format shifted at some point tho as I doubt it draws direct from ROM

	// is the data at 0x000 in ROM the palette? can't work out the format if so.

	if (m_hackmode == 0) // 16x16 tiles 4bpp (menu)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++)
			{
				gfx_element *gfx = m_gfxdecode->gfx(0);

				int tile = m_ram[offs] + (m_ram[offs + 1] << 8);
				int attr = (m_ram[offs + 3]); // set to 0x07 on the radica logo, 0x00 on the game select screen

				if (attr == 0)
				{
					/* this logic allows us to see the Taito logo and menu screen */
					gfx = m_gfxdecode->gfx(0); // 4bpp
					tile = (tile & 0xf) + ((tile & ~0xf) * 16);
					tile += ((m_tile_gfxbase_lo_data | m_tile_gfxbase_hi_data << 8) << 5);
					tile <<= 1; // due to 16 pixel wide
				}
				else
				{
					gfx = m_gfxdecode->gfx(2); // 8bpp
					tile = (tile & 0xf) + ((tile & ~0xf) * 16);
					tile <<= 1; // due to 16 pixel wide

					// why after the shift in this case?
					tile += ((m_tile_gfxbase_lo_data | m_tile_gfxbase_hi_data << 8) << 5);
				}

				for (int i = 0; i < 16; i++)
				{
					gfx->transpen(bitmap, cliprect, tile + i * 32, 0, 0, 0, x * 16, (y * 16) + i, 0);
					gfx->transpen(bitmap, cliprect, (tile + i * 32) + 1, 0, 0, 0, (x * 16) + 8, (y * 16) + i, 0);
				}

				offs += 4;
			}
		}
	}
	else if (m_hackmode == 1) // 8x8 tiles (games)
	{
		gfx_element *gfx = m_gfxdecode->gfx(2);

		for (int y = 0; y < 32; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				int tile = m_ram[offs] + (m_ram[offs + 1] << 8);

				tile = (tile & 0x1f) + ((tile & ~0x1f) * 8);
				tile += ((m_tile_gfxbase_lo_data | m_tile_gfxbase_hi_data << 8) << 5);

				for (int i = 0; i < 8; i++)
				{
					gfx->transpen(bitmap, cliprect, tile + i * 32, 0, 0, 0, x * 8, (y * 8) + i, 0);

				}
				offs += 4;
			}
		}
	}
	else if (m_hackmode == 2) // qix
	{
		for (int y = 0; y < 224; y++)
		{
			uint16_t* row = &bitmap.pix16(y);

			for (int x = 0; x < 256; x++)
			{
				int pixel = m_pixram[offs];

				if (pixel) row[x] = pixel;

				offs++;
			}
		}
	}

	return 0;
}

WRITE8_MEMBER(radica_6502_state::radicasi_500c_w)
{
	// written with the banking?
	logerror("%s: radicasi_500c_w (set ROM bank) %02x\n", machine().describe_context().c_str(), data);

	m_bank->set_bank(m_500d_data);
}

READ8_MEMBER(radica_6502_state::radicasi_500d_r)
{
	return m_500d_data;
}

READ8_MEMBER(radica_6502_state::radicasi_500b_r)
{
	// how best to handle this, we probably need to run the PAL machine at 50hz
	// the text under the radica logo differs between regions
	logerror("%s: radicasi_500b_r (region + more?)\n", machine().describe_context().c_str());
	return 0xff; // NTSC
	//return 0x00; // PAL
}

WRITE8_MEMBER(radica_6502_state::radicasi_500d_w)
{
	logerror("%s: radicasi_500d_w (select ROM bank) %02x\n", machine().describe_context().c_str(), data);
	m_500d_data = data;
}

// Tile bases

WRITE8_MEMBER(radica_6502_state::radicasi_tile_gfxbase_lo_w)
{
	logerror("%s: radicasi_tile_gfxbase_lo_w (select GFX base lower) %02x\n", machine().describe_context().c_str(), data);
	m_tile_gfxbase_lo_data = data;
}

WRITE8_MEMBER(radica_6502_state::radicasi_tile_gfxbase_hi_w)
{
	logerror("%s: radicasi_tile_gfxbase_hi_w (select GFX base upper) %02x\n", machine().describe_context().c_str(), data);
	m_tile_gfxbase_hi_data = data;
}

READ8_MEMBER(radica_6502_state::radicasi_tile_gfxbase_lo_r)
{
	logerror("%s: radicasi_tile_gfxbase_lo_r (GFX base lower)\n", machine().describe_context().c_str());
	return m_tile_gfxbase_lo_data;
}

READ8_MEMBER(radica_6502_state::radicasi_tile_gfxbase_hi_r)
{
	logerror("%s: radicasi_tile_gfxbase_hi_r (GFX base upper)\n", machine().describe_context().c_str());
	return m_tile_gfxbase_hi_data;
}

// Sprite Tile bases

WRITE8_MEMBER(radica_6502_state::radicasi_sprite_gfxbase_lo_w)
{
	logerror("%s: radicasi_sprite_gfxbase_lo_w (select Sprite GFX base lower) %02x\n", machine().describe_context().c_str(), data);
	m_sprite_gfxbase_lo_data = data;
}

WRITE8_MEMBER(radica_6502_state::radicasi_sprite_gfxbase_hi_w)
{
	logerror("%s: radicasi_sprite_gfxbase_hi_w (select Sprite GFX base upper) %02x\n", machine().describe_context().c_str(), data);
	m_sprite_gfxbase_hi_data = data;
}

READ8_MEMBER(radica_6502_state::radicasi_sprite_gfxbase_lo_r)
{
	logerror("%s: radicasi_sprite_gfxbase_lo_r (Sprite GFX base lower)\n", machine().describe_context().c_str());
	return m_sprite_gfxbase_lo_data;
}

READ8_MEMBER(radica_6502_state::radicasi_sprite_gfxbase_hi_r)
{
	logerror("%s: radicasi_sprite_gfxbase_hi_r (Sprite GFX base upper)\n", machine().describe_context().c_str());
	return m_sprite_gfxbase_hi_data;
}

// Palette bases

WRITE8_MEMBER(radica_6502_state::radicasi_palbase_lo_w)
{
	logerror("%s: radicasi_palbase_lo_w (select Palette base lower) %02x\n", machine().describe_context().c_str(), data);
	m_palbase_lo_data = data;
}

WRITE8_MEMBER(radica_6502_state::radicasi_palbase_hi_w)
{
	logerror("%s: radicasi_palbase_hi_w (select Palette base upper) %02x\n", machine().describe_context().c_str(), data);
	m_palbase_hi_data = data;
}

READ8_MEMBER(radica_6502_state::radicasi_palbase_lo_r)
{
	logerror("%s: radicasi_palbase_lo_r (Palette base lower)\n", machine().describe_context().c_str());
	return m_palbase_lo_data;
}

READ8_MEMBER(radica_6502_state::radicasi_palbase_hi_r)
{
	logerror("%s: radicasi_palbase_hi_r (Palette base upper)\n", machine().describe_context().c_str());
	return m_palbase_hi_data;
}

// unknown regs that seem to also be pointers
// seem to get set to sound data?

void radica_6502_state::handle_unkregs_0_w(int which, int offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		m_unkregs_0_unk[which] = data;
		logerror("%s: unkregs_0 (%d) write to unknown param %02x\n", machine().describe_context().c_str(), which, data);
		break;

	case 0x01:
		m_unkregs_0_address[which] = (m_unkregs_0_address[which] & 0xff00) | data;
		logerror("%s: unkregs_0 (%d) write lo address %02x (real address is now %08x)\n", machine().describe_context().c_str(), which, data, m_unkregs_0_address[which]*0x100);
		break;

	case 0x02:
		m_unkregs_0_address[which] = (m_unkregs_0_address[which] & 0x00ff) | (data<<8);
		logerror("%s: unkregs_0 (%d) write hi address %02x (real address is now %08x)\n", machine().describe_context().c_str(), which, data, m_unkregs_0_address[which]*0x100);
		break;
	}
}

uint8_t radica_6502_state::handle_unkregs_0_r(int which, int offset)
{
	switch (offset)
	{
	case 0x00:
		logerror("%s: unkregs_0 (%d) read from unknown param\n", machine().describe_context().c_str(), which);
		return m_unkregs_0_unk[which];

	case 0x01:
		logerror("%s: unkregs_0 (%d) read lo address\n", machine().describe_context().c_str(), which);
		return m_unkregs_0_address[which] & 0x00ff;

	case 0x02:
		logerror("%s: unkregs_0 (%d) read hi address\n", machine().describe_context().c_str(), which);
		return (m_unkregs_0_address[which]>>8) & 0x00ff;
	}

	return 0x00;
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_0_0_w)
{
	handle_unkregs_0_w(0,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_0_0_r)
{
	return handle_unkregs_0_r(0,offset);
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_0_1_w)
{
	handle_unkregs_0_w(1,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_0_1_r)
{
	return handle_unkregs_0_r(1,offset);
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_0_2_w)
{
	handle_unkregs_0_w(2,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_0_2_r)
{
	return handle_unkregs_0_r(2,offset);
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_0_3_w)
{
	handle_unkregs_0_w(3,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_0_3_r)
{
	return handle_unkregs_0_r(3,offset);
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_0_4_w)
{
	handle_unkregs_0_w(4,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_0_4_r)
{
	return handle_unkregs_0_r(4,offset);
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_0_5_w)
{
	handle_unkregs_0_w(5,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_0_5_r)
{
	return handle_unkregs_0_r(5,offset);
}

void radica_6502_state::handle_unkregs_1_w(int which, int offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		m_unkregs_1_unk0[which] = data;
		logerror("%s: unkregs_1 (%d) write to unknown param 0 %02x\n", machine().describe_context().c_str(), which, data);
		break;

	case 0x01:
		m_unkregs_1_unk1[which] = data;
		logerror("%s: unkregs_1 (%d) write to unknown param 1 %02x\n", machine().describe_context().c_str(), which, data);
		break;

	case 0x02:
		m_unkregs_1_unk2[which] = data;
		logerror("%s: unkregs_1 (%d) write to unknown param 2 %02x\n", machine().describe_context().c_str(), which, data);
		break;
	}
}

uint8_t radica_6502_state::handle_unkregs_1_r(int which, int offset)
{
	switch (offset)
	{
	case 0x00:
		logerror("%s: unkregs_1 (%d) read from unknown param 0\n", machine().describe_context().c_str(), which);
		return m_unkregs_1_unk0[which];

	case 0x01:
		logerror("%s: unkregs_1 (%d) read from unknown param 1\n", machine().describe_context().c_str(), which);
		return m_unkregs_1_unk1[which];

	case 0x02:
		logerror("%s: unkregs_1 (%d) read from unknown param 2\n", machine().describe_context().c_str(), which);
		return m_unkregs_1_unk2[which];
	}

	return 0x00;
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_1_0_w)
{
	handle_unkregs_1_w(0,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_1_0_r)
{
	return handle_unkregs_1_r(0,offset);
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_1_1_w)
{
	handle_unkregs_1_w(1,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_1_1_r)
{
	return handle_unkregs_1_r(1,offset);
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_1_2_w)
{
	handle_unkregs_1_w(2,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_1_2_r)
{
	return handle_unkregs_1_r(2,offset);
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_1_3_w)
{
	handle_unkregs_1_w(3,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_1_3_r)
{
	return handle_unkregs_1_r(3,offset);
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_1_4_w)
{
	handle_unkregs_1_w(4,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_1_4_r)
{
	return handle_unkregs_1_r(4,offset);
}

WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_1_5_w)
{
	handle_unkregs_1_w(5,offset,data);
}

READ8_MEMBER(radica_6502_state::radicasi_unkregs_1_5_r)
{
	return handle_unkregs_1_r(5,offset);
}

// do something with the above..
READ8_MEMBER(radica_6502_state::radicasi_unkregs_trigger_r)
{
	logerror("%s: unkregs read from trigger?\n", machine().describe_context().c_str());
	return m_unkregs_trigger;
}


WRITE8_MEMBER(radica_6502_state::radicasi_unkregs_trigger_w)
{
	logerror("%s: unkregs write to trigger? %02x\n", machine().describe_context().c_str(), data);
	m_unkregs_trigger= data;

	for (int i = 0; i < 6; i++)
	{
		int bit = (data >> i)&1;

		if (bit)
			handle_trigger(i);
	}

	if (data & 0xc0)
		logerror("  UNEXPECTED BITS SET");
}

void radica_6502_state::handle_trigger(int which)
{
	logerror("Triggering operation on channel (%d) with params %02x %06x %02x %02x %02x\n", which, m_unkregs_0_unk[which], m_unkregs_0_address[which] * 0x100, m_unkregs_1_unk0[which], m_unkregs_1_unk1[which], m_unkregs_1_unk2[which]);
}


READ8_MEMBER(radica_6502_state::radicasi_50a8_r)
{
	logerror("%s: radicasi_50a8_r\n", machine().describe_context().c_str());
	return 0x3f;
}

static ADDRESS_MAP_START( radicasi_map, AS_PROGRAM, 8, radica_6502_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("ram") // ends up copying code to ram, but could be due to banking issues
	AM_RANGE(0x4800, 0x49ff) AM_RAM

	AM_RANGE(0x500b, 0x500b) AM_READ(radicasi_500b_r) // PAL / NTSC flag at least
	AM_RANGE(0x500c, 0x500c) AM_WRITE(radicasi_500c_w)
	AM_RANGE(0x500d, 0x500d) AM_READWRITE(radicasi_500d_r, radicasi_500d_w)

	AM_RANGE(0x5010, 0x5010) AM_READWRITE(radicasi_palbase_lo_r, radicasi_palbase_lo_w) // palettebase
	AM_RANGE(0x5011, 0x5011) AM_READWRITE(radicasi_palbase_hi_r, radicasi_palbase_hi_w) // palettebase

	AM_RANGE(0x5029, 0x5029) AM_READWRITE(radicasi_tile_gfxbase_lo_r, radicasi_tile_gfxbase_lo_w) // tilebase
	AM_RANGE(0x502a, 0x502a) AM_READWRITE(radicasi_tile_gfxbase_hi_r, radicasi_tile_gfxbase_hi_w) // tilebase

	AM_RANGE(0x502b, 0x502b) AM_READWRITE(radicasi_sprite_gfxbase_lo_r, radicasi_sprite_gfxbase_lo_w) // tilebase (spr?)
	AM_RANGE(0x502c, 0x502c) AM_READWRITE(radicasi_sprite_gfxbase_hi_r, radicasi_sprite_gfxbase_hi_w) // tilebase (spr?)

	AM_RANGE(0x5041, 0x5041) AM_READ_PORT("IN0") // AM_READ(radicasi_5041_r)

	// These might be sound / DMA channels?

	AM_RANGE(0x5080, 0x5082) AM_READWRITE(radicasi_unkregs_0_0_r, radicasi_unkregs_0_0_w) // 5082 set to 0x33, so probably another 'high' address bits reg
	AM_RANGE(0x5083, 0x5085) AM_READWRITE(radicasi_unkregs_0_1_r, radicasi_unkregs_0_1_w) // 5085 set to 0x33, so probably another 'high' address bits reg
	AM_RANGE(0x5086, 0x5088) AM_READWRITE(radicasi_unkregs_0_2_r, radicasi_unkregs_0_2_w) // 5088 set to 0x33, so probably another 'high' address bits reg
	AM_RANGE(0x5089, 0x508b) AM_READWRITE(radicasi_unkregs_0_3_r, radicasi_unkregs_0_3_w) // 508b set to 0x33, so probably another 'high' address bits reg
	AM_RANGE(0x508c, 0x508e) AM_READWRITE(radicasi_unkregs_0_4_r, radicasi_unkregs_0_4_w) // 508e set to 0x33, so probably another 'high' address bits reg
	AM_RANGE(0x508f, 0x5091) AM_READWRITE(radicasi_unkregs_0_5_r, radicasi_unkregs_0_5_w) // 5091 set to 0x33, so probably another 'high' address bits reg
	// these are set at the same time as the above, so probably additional params  0x5092 is used with 0x5080 etc.
	AM_RANGE(0x5092, 0x5094) AM_READWRITE(radicasi_unkregs_1_0_r, radicasi_unkregs_1_0_w)
	AM_RANGE(0x5095, 0x5097) AM_READWRITE(radicasi_unkregs_1_1_r, radicasi_unkregs_1_1_w)
	AM_RANGE(0x5098, 0x509a) AM_READWRITE(radicasi_unkregs_1_2_r, radicasi_unkregs_1_2_w)
	AM_RANGE(0x509b, 0x509d) AM_READWRITE(radicasi_unkregs_1_3_r, radicasi_unkregs_1_3_w)
	AM_RANGE(0x509e, 0x50a0) AM_READWRITE(radicasi_unkregs_1_4_r, radicasi_unkregs_1_4_w)
	AM_RANGE(0x50a1, 0x50a3) AM_READWRITE(radicasi_unkregs_1_5_r, radicasi_unkregs_1_5_w)

	AM_RANGE(0x50a5, 0x50a5) AM_READWRITE(radicasi_unkregs_trigger_r, radicasi_unkregs_trigger_w)

	AM_RANGE(0x50a8, 0x50a8) AM_READ(radicasi_50a8_r)

	//AM_RANGE(0x5000, 0x50ff) AM_RAM

	AM_RANGE(0x6000, 0xdfff) AM_DEVICE("bank", address_map_bank_device, amap8)

	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0xf8000)
ADDRESS_MAP_END


static ADDRESS_MAP_START( radicasi_bank_map, AS_PROGRAM, 8, radica_6502_state )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_SHARE("pixram") // qix accesses here when 500c is 01 (which could be an additional bank bit)

	AM_RANGE(0x300000, 0x3fffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x400000, 0x40ffff) AM_RAM // could be tileram? or spriteram? only cleared tho, so would be populated with DMA?
ADDRESS_MAP_END

static INPUT_PORTS_START( radicasi )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )
INPUT_PORTS_END

void radica_6502_state::machine_start()
{
	uint8_t *rom = memregion("maincpu")->base();
	/* both NMI and IRQ vectors just point to RTI
	   there is a table of jumps just before that, those appear to be the real interrupt functions?

	   patch the main IRQ to be the one that decreases an address the code is waiting for
	   the others look like they might be timer service routines
	*/
	rom[0xf9ffe] = 0xd4;
	rom[0xf9fff] = 0xff;

	/*
		d8000-dffff maps to 6000-dfff
		e0000-e7fff maps to 6000-dfff
		e8000-effff maps to 6000-dfff
		f0000-f7fff maps to 6000-dfff
		f8000-fffff maps to 6000-dfff (but f8000-f9fff mapping to 6000-7fff isn't used, because it's the fixed area below - make sure nothing else gets mapped there instead)

		-- fixed
		f8000-f9fff maps to e000-ffff
	*/

	m_bank->set_bank(0x7f);
}

void radica_6502_state::machine_reset()
{
}

static const gfx_layout helper_4bpp_8_layout =
{
	8,1,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ STEP8(0,4) },
	{ 0 },
	8 * 4
};

static const gfx_layout helper_8bpp_8_layout =
{
	8,1,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,8) },
	{ 0 },
	8 * 8
};


// these are fake just to make looking at the texture pages easier
static const uint32_t texlayout_xoffset_8bpp[256] = { STEP256(0,8) };
static const uint32_t texlayout_yoffset_8bpp[256] = { STEP256(0,256*8) };
static const gfx_layout texture_helper_8bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*8,
	texlayout_xoffset_8bpp,
	texlayout_yoffset_8bpp
};

static const uint32_t texlayout_xoffset_4bpp[256] = { STEP256(0,4) };
static const uint32_t texlayout_yoffset_4bpp[256] = { STEP256(0,256*4) };
static const gfx_layout texture_helper_4bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*4,
	texlayout_xoffset_4bpp,
	texlayout_yoffset_4bpp
};



static GFXDECODE_START( radicasi_fake )
	GFXDECODE_ENTRY( "maincpu", 0, helper_4bpp_8_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, texture_helper_4bpp_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, helper_8bpp_8_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, texture_helper_8bpp_layout,  0x0, 1  )
GFXDECODE_END


// Tetris has a XTAL_21_28137MHz, not confirmed on Space Invaders, actual CPU clock unknown.

static MACHINE_CONFIG_START( radicasi )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502,XTAL_21_28137MHz/2)
	MCFG_CPU_PROGRAM_MAP(radicasi_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", radica_6502_state,  irq0_line_hold)

	MCFG_DEVICE_ADD("bank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(radicasi_bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(32)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x8000)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(radica_6502_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", radicasi_fake)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


ROM_START( rad_tetr )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "tetrisrom.bin", 0x000000, 0x100000, CRC(40538e08) SHA1(1aef9a2c678e39243eab8d910bb7f9f47bae0aee) )
ROM_END

ROM_START( rad_sinv )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "spaceinvadersrom.bin", 0x000000, 0x100000, CRC(5ffb2c8f) SHA1(9bde42ec5c65d9584a802de7d7c8b842ebf8cbd8) )
ROM_END

CONS( 2004, rad_tetr,  0,   0,  radicasi,  radicasi, radica_6502_state, 0, "Radica (licensed from Taito)", "Space Invaders (Radica, Arcade Legends TV Game)", MACHINE_NOT_WORKING )
CONS( 2004, rad_sinv,  0,   0,  radicasi,  radicasi, radica_6502_state, 0, "Radica",                       "Tetris (Radica, Arcade Legends TV Game)", MACHINE_NOT_WORKING ) // "5 Tetris games in 1"
