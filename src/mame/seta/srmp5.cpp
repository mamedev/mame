// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*

Super Real Mahjong P5
(c)1994 SETA

preliminary driver by Tomasz Slanina


--

CPU   : R3560 (IDT MIPS R3000 derivative)
SOUND : TC6210AF (ST-0016)
OSC.  : 50.0000MHz 42.9545MHz

SX008-01.BIN ; CHR ROM
SX008-02.BIN ;  |
SX008-03.BIN ;  |
SX008-04.BIN ;  |
SX008-05.BIN ;  |
SX008-06.BIN ;  |
SX008-07.BIN ; /
SX008-08.BIN ; SOUND DATA
SX008-09.BIN ; /
SX008-11.BIN ; MAIN PRG
SX008-12.BIN ;  |
SX008-13.BIN ;  |
SX008-14.BIN ; /


Note:

attract sound ON/OFF of DIPSW doesn't work.
This is not a bug (real machine behaves the same).
*/


#include "emu.h"
#include "st0016.h"
#include "cpu/mips/mips1.h"
#include "emupal.h"
#include "speaker.h"


namespace {

#define DEBUG_CHAR

#define SPRITE_GLOBAL_X 0
#define SPRITE_GLOBAL_Y 1
#define SUBLIST_OFFSET  2
#define SUBLIST_LENGTH  3

#define SUBLIST_OFFSET_SHIFT 3
#define SPRITE_LIST_END_MARKER 0x8000

#define SPRITE_TILE    0
#define SPRITE_PALETTE 1
#define SPRITE_LOCAL_X 2
#define SPRITE_LOCAL_Y 3
#define SPRITE_SIZE    4

#define SPRITE_SUBLIST_ENTRY_LENGTH 8
#define SPRITE_LIST_ENTRY_LENGTH    4

#define SPRITE_DATA_GRANULARITY 0x80

class srmp5_state : public driver_device
{
public:
	srmp5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this,"soundcpu")
		, m_chrrom(*this, "chr")
		, m_soundbank(*this, "soundbank")
		, m_keys(*this, "KEY.%u", 0)
		, m_chrbank(0)
	{
	}

	void srmp5(machine_config &config);

	void init_srmp5();

private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<r3051_device> m_maincpu;
	required_device<st0016_cpu_device> m_soundcpu;

	required_region_ptr<uint16_t> m_chrrom;

	required_memory_bank m_soundbank;

	required_ioport_array<4> m_keys;

	uint32_t m_chrbank;
	std::unique_ptr<uint16_t[]> m_tileram;
	std::unique_ptr<uint16_t[]> m_sprram;

	uint8_t m_input_select;

	uint8_t m_cmd1;
	uint8_t m_cmd2;
	uint8_t m_cmd_stat;

	uint32_t m_vidregs[0x120 / 4];
#ifdef DEBUG_CHAR
	std::unique_ptr<uint8_t[]> m_tileduty;
#endif
	void bank_w(uint32_t data);
	uint32_t tileram_r(offs_t offset);
	void tileram_w(offs_t offset, uint32_t data);
	uint32_t spr_r(offs_t offset);
	void spr_w(offs_t offset, uint32_t data);
	uint32_t chrrom_r(offs_t offset);
	void input_select_w(uint32_t data);
	uint32_t srmp5_inputs_r();
	void cmd1_w(uint32_t data);
	void cmd2_w(uint32_t data);
	uint32_t cmd_stat32_r();
	uint32_t srmp5_vidregs_r(offs_t offset);
	void srmp5_vidregs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t irq_ack_clear();
	uint8_t cmd1_r();
	uint8_t cmd2_r();
	uint8_t cmd_stat8_r();
	virtual void machine_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void st0016_rom_bank_w(uint8_t data);
	void srmp5_mem(address_map &map) ATTR_COLD;
	void st0016_io(address_map &map) ATTR_COLD;
	void st0016_mem(address_map &map) ATTR_COLD;
};


uint32_t srmp5_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int address,height,width,sizex,sizey;
	uint16_t *sprite_list=m_sprram.get();
	uint16_t *sprite_list_end=&m_sprram[0x4000]; //guess
	uint8_t *pixels=(uint8_t *)m_tileram.get();
	const pen_t * const pens = m_palette->pens();

//Table surface seems to be tiles, but display corrupts when switching the scene if always ON.
//Currently the tiles are OFF.
#ifdef BG_ENABLE
	uint8_t tile_width  = (m_vidregs[2] >> 0) & 0xFF;
	uint8_t tile_height = (m_vidregs[2] >> 8) & 0xFF;
	if(tile_width && tile_height)
	{
		// 16x16 tile
		uint16_t *map = &sprram[0x2000];
		for(int yw = 0; yw < tile_height; yw++)
		{
			for(int xw = 0; xw < tile_width; xw++)
			{
				uint16_t tile = map[yw * 128 + xw * 2];
				if(tile >= 0x2000) continue;

				address = tile * SPRITE_DATA_GRANULARITY;
				for(int y = 0; y < 16; y++)
				{
					for(int x = 0; x < 16; x++)
					{
						uint8_t pen = pixels[BYTE_XOR_LE(address)];
						if(pen)
						{
							bitmap.pix(yw * 16 + y, xw * 16 + x) = pens[pen];
						}
						address++;
					}
				}
			}
		}
	}
	else
#endif
		bitmap.fill(0, cliprect);

	while((sprite_list[SUBLIST_OFFSET]&SPRITE_LIST_END_MARKER)==0 && sprite_list<sprite_list_end)
	{
		uint16_t *sprite_sublist=&m_sprram[sprite_list[SUBLIST_OFFSET]<<SUBLIST_OFFSET_SHIFT];
		uint16_t sublist_length=sprite_list[SUBLIST_LENGTH];
		int16_t global_x,global_y;

		if(0!=sprite_list[SUBLIST_OFFSET])
		{
			global_x=(int16_t)sprite_list[SPRITE_GLOBAL_X];
			global_y=(int16_t)sprite_list[SPRITE_GLOBAL_Y];
			while(sublist_length)
			{
				int x=(int16_t)sprite_sublist[SPRITE_LOCAL_X]+global_x;
				int y=(int16_t)sprite_sublist[SPRITE_LOCAL_Y]+global_y;
				width =(sprite_sublist[SPRITE_SIZE]>> 4)&0xf;
				height=(sprite_sublist[SPRITE_SIZE]>>12)&0xf;

				sizex=(sprite_sublist[SPRITE_SIZE]>>0)&0xf;
				sizey=(sprite_sublist[SPRITE_SIZE]>>8)&0xf;

				address=(sprite_sublist[SPRITE_TILE] & ~(sprite_sublist[SPRITE_SIZE] >> 11 & 7))*SPRITE_DATA_GRANULARITY;
				y -= (height + 1) * (sizey + 1)-1;
				for(int xw=0;xw<=width;xw++)
				{
					int xb = (sprite_sublist[SPRITE_PALETTE] & 0x8000) ? (width-xw)*(sizex+1)+x: xw*(sizex+1)+x;
					for(int yw=0;yw<=height;yw++)
					{
						int yb = yw*(sizey+1)+y;
						for(int ys=0;ys<=sizey;ys++)
						{
							int ys2 = (sprite_sublist[SPRITE_PALETTE] & 0x4000) ? ys : (sizey - ys);
							for(int xs=0;xs<=sizex;xs++)
							{
								uint8_t pen=pixels[BYTE_XOR_LE(address)&(0x100000-1)];
								int xs2 = (sprite_sublist[SPRITE_PALETTE] & 0x8000) ? (sizex - xs) : xs;
								if(pen)
								{
									if(cliprect.contains(xb+xs2, yb+ys2))
									{
										bitmap.pix(yb+ys2, xb+xs2) = pens[pen+((sprite_sublist[SPRITE_PALETTE]&0xff)<<8)];
									}
								}
								++address;
							}
						}
					}
				}
				sprite_sublist+=SPRITE_SUBLIST_ENTRY_LENGTH;
				--sublist_length;
			}
		}
		sprite_list+=SPRITE_LIST_ENTRY_LENGTH;
	}

#ifdef DEBUG_CHAR
	{
		for(int i = 0; i < 0x2000; i++)
		{
			if (m_tileduty[i] == 1)
			{
				m_gfxdecode->gfx(0)->mark_dirty(i);
				m_gfxdecode->gfx(0)->get_data(i);
				m_tileduty[i] = 0;
			}
		}
	}
#endif
	return 0;
}

void srmp5_state::machine_start()
{
	m_soundbank->configure_entries(0, 256, memregion("soundcpu")->base(), 0x4000);

	save_item(NAME(m_input_select));
	save_item(NAME(m_cmd1));
	save_item(NAME(m_cmd2));
	save_item(NAME(m_cmd_stat));
	save_item(NAME(m_chrbank));
	save_pointer(NAME(m_tileram), 0x100000/2);
	save_pointer(NAME(m_sprram), 0x80000/2);
#ifdef DEBUG_CHAR
	save_pointer(NAME(m_tileduty), 0x2000);
#endif
	save_item(NAME(m_vidregs));
}

void srmp5_state::bank_w(uint32_t data)
{
	m_chrbank = ((data & 0xf0) >> 4) * (0x100000 / sizeof(uint16_t));
}

uint32_t srmp5_state::tileram_r(offs_t offset)
{
	return m_tileram[offset];
}

void srmp5_state::tileram_w(offs_t offset, uint32_t data)
{
	m_tileram[offset] = data & 0xFFFF; //lower 16bit only
#ifdef DEBUG_CHAR
	m_tileduty[offset >> 6] = 1;
#endif
}

uint32_t srmp5_state::spr_r(offs_t offset)
{
	return m_sprram[offset];
}

void srmp5_state::spr_w(offs_t offset, uint32_t data)
{
	m_sprram[offset] = data & 0xFFFF; //lower 16bit only
}

uint32_t srmp5_state::chrrom_r(offs_t offset)
{
	return m_chrrom[m_chrbank + offset]; // lower 16bit only
}

void srmp5_state::input_select_w(uint32_t data)
{
	m_input_select = data & 0x0F;
}

uint32_t srmp5_state::srmp5_inputs_r()
{
	uint32_t ret = 0;

	switch (m_input_select)
	{
	case 0x01:
		ret = m_keys[0]->read();
		break;
	case 0x02:
		ret = m_keys[1]->read();
		break;
	case 0x04:
		ret = m_keys[2]->read();
		break;
	case 0x00:
	case 0x08:
		ret = m_keys[3]->read();
		break;
	}
	return ret;
}

//almost all cmds are sound related
void srmp5_state::cmd1_w(uint32_t data)
{
	m_cmd1 = data & 0xFF;
	logerror("cmd1_w %08X\n", data);
}

void srmp5_state::cmd2_w(uint32_t data)
{
	m_cmd2 = data & 0xFF;
	m_cmd_stat = 5;
	logerror("cmd2_w %08X\n", data);
}

uint32_t srmp5_state::cmd_stat32_r()
{
	return m_cmd_stat;
}

uint32_t srmp5_state::srmp5_vidregs_r(offs_t offset)
{
	logerror("vidregs read  %08X %08X\n", offset << 2, m_vidregs[offset]);
	return m_vidregs[offset];
}

void srmp5_state::srmp5_vidregs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vidregs[offset]);
	if(offset != 0x10C / 4)
		logerror("vidregs write %08X %08X\n", offset << 2, m_vidregs[offset]);
}

uint32_t srmp5_state::irq_ack_clear()
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ4, CLEAR_LINE);
	return 0;
}

void srmp5_state::srmp5_mem(address_map &map)
{
	map(0x00000000, 0x000fffff).ram(); //maybe 0 - 2fffff ?
	map(0x002f0000, 0x002f7fff).ram();
	map(0x01000000, 0x01000003).nopw();  // 0xaa .. watchdog ?
	map(0x01800000, 0x01800003).ram(); //?1
	map(0x01800004, 0x01800007).portr("DSW1");
	map(0x01800008, 0x0180000b).portr("DSW2");
	map(0x0180000c, 0x0180000f).w(FUNC(srmp5_state::bank_w));
	map(0x01800010, 0x01800013).r(FUNC(srmp5_state::srmp5_inputs_r)); //multiplexed controls (selected by writes to 1c)
	map(0x01800014, 0x01800017).portr("TEST");
	map(0x0180001c, 0x0180001f).w(FUNC(srmp5_state::input_select_w));//c1 c2 c4 c8 => mahjong inputs (at $10) - bits 0-3
	map(0x01800200, 0x01800203).ram();  //sound related ? only few writes after boot
	map(0x01802000, 0x01802003).w(FUNC(srmp5_state::cmd1_w));
	map(0x01802004, 0x01802007).w(FUNC(srmp5_state::cmd2_w));
	map(0x01802008, 0x0180200b).r(FUNC(srmp5_state::cmd_stat32_r));
	map(0x01a00000, 0x01bfffff).r(FUNC(srmp5_state::chrrom_r));
	map(0x01c00000, 0x01c00003).nopr(); // debug? 'Toru'

	map(0x0a000000, 0x0a0fffff).rw(FUNC(srmp5_state::spr_r), FUNC(srmp5_state::spr_w));
	map(0x0a100000, 0x0a17ffff).rw(m_palette, FUNC(palette_device::read16), FUNC(palette_device::write16)).umask32(0x0000ffff).share("palette");
	//0?N???A?????????i??????????
	map(0x0a180000, 0x0a18011f).rw(FUNC(srmp5_state::srmp5_vidregs_r), FUNC(srmp5_state::srmp5_vidregs_w));
	map(0x0a180000, 0x0a180003).nopr(); // write 0x00000400
	map(0x0a200000, 0x0a3fffff).rw(FUNC(srmp5_state::tileram_r), FUNC(srmp5_state::tileram_w));
	map(0x0fc00000, 0x0fdfffff).mirror(0x10000000).rom().region("maincpu", 0);

	map(0x1eff0000, 0x1eff001f).nopw();
	map(0x1eff003c, 0x1eff003f).r(FUNC(srmp5_state::irq_ack_clear));
}

void srmp5_state::st0016_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("soundbank");
	//map(0xe900, 0xe9ff) // sound - internal
	//map(0xec00, 0xec1f).rw(FUNC(srmp5_state::st0016_character_ram_r), FUNC(srmp5_state::st0016_character_ram_w));
	map(0xf000, 0xffff).ram();
}

uint8_t srmp5_state::cmd1_r()
{
	m_cmd_stat = 0;
	return m_cmd1;
}

uint8_t srmp5_state::cmd2_r()
{
	return m_cmd2;
}

uint8_t srmp5_state::cmd_stat8_r()
{
	return m_cmd_stat;
}

// common rombank? should go in machine/st0016 with larger address space exposed?
void srmp5_state::st0016_rom_bank_w(uint8_t data)
{
	m_soundbank->set_entry(data);
}


void srmp5_state::st0016_io(address_map &map)
{
	map.global_mask(0xff);
	//map(0x00, 0xbf).rw(FUNC(srmp5_state::st0016_vregs_r), FUNC(srmp5_state::st0016_vregs_w));
	map(0xc0, 0xc0).r(FUNC(srmp5_state::cmd1_r));
	map(0xc1, 0xc1).r(FUNC(srmp5_state::cmd2_r));
	map(0xc2, 0xc2).r(FUNC(srmp5_state::cmd_stat8_r));
	map(0xe1, 0xe1).w(FUNC(srmp5_state::st0016_rom_bank_w));
	map(0xe7, 0xe7).w(FUNC(srmp5_state::st0016_rom_bank_w));
	//map(0xf0, 0xf0).r(FUNC(srmp5_state::st0016_dma_r));
}


static INPUT_PORTS_START( srmp5 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "PUT" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPSETTING(      0x0001, "7" )
	PORT_DIPSETTING(      0x0002, "6" )
	PORT_DIPSETTING(      0x0003, "5" )
	PORT_DIPSETTING(      0x0007, "4" )
	PORT_DIPSETTING(      0x0006, "1" )
	PORT_DIPSETTING(      0x0005, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPNAME( 0x0008, 0x0008, "Kuitan" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Test ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT ( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.0")
	PORT_BIT ( 0xfffffff0, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )

	PORT_START("KEY.1")
	PORT_BIT ( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT ( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT ( 0x00000020, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("KEY.2")
	PORT_BIT ( 0xffffffe0, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT ( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )

	PORT_START("KEY.3")
	PORT_BIT ( 0xffffff60, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT ( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT ( 0x00000080, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("TEST")
	PORT_BIT ( 0x00000080, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F2)
	PORT_BIT ( 0xffffff7f, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded

INPUT_PORTS_END


static const gfx_layout tile_16x8x8_layout =
{
	16,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	{ STEP8(0, 8*16) },
	16*8*8
};

#if 0
static const gfx_layout tile_16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	{ STEP16(0, 8*16) },
	16*16*8
};
#endif

static GFXDECODE_START( gfx_srmp5 )
	GFXDECODE_ENTRY( "gfx1", 0, tile_16x8x8_layout,  0x0, 0x800  )
	//GFXDECODE_ENTRY( "gfx1", 0, tile_16x16x8_layout, 0x0, 0x800  )
GFXDECODE_END

void srmp5_state::srmp5(machine_config &config)
{
	/* basic machine hardware */
	R3051(config, m_maincpu, XTAL(50'000'000) / 2); // 25 MHz (50 MHz / 2)
	m_maincpu->set_endianness(ENDIANNESS_LITTLE);
	m_maincpu->set_addrmap(AS_PROGRAM, &srmp5_state::srmp5_mem);
	m_maincpu->set_vblank_int("screen", FUNC(srmp5_state::irq4_line_assert));

	ST0016_CPU(config, m_soundcpu, XTAL(42'954'545) / 6); // 7.159 MHz (42.9545 MHz / 6)
	m_soundcpu->set_addrmap(AS_PROGRAM, &srmp5_state::st0016_mem);
	m_soundcpu->set_addrmap(AS_IO, &srmp5_state::st0016_io);
	m_soundcpu->set_vblank_int("screen", FUNC(srmp5_state::irq0_line_hold));

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(96*8, 64*8);
	screen.set_visarea(0*8, 42*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(srmp5_state::screen_update));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x10000); // 0x20000? only first 0x1800 entries seem to be used outside memory test
	m_palette->set_membits(16);

#ifdef DEBUG_CHAR
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_srmp5);
#endif

	// TODO: Mono?
	SPEAKER(config, "speaker", 2).front();

	m_soundcpu->add_route(0, "speaker", 1.0, 0);
	m_soundcpu->add_route(1, "speaker", 1.0, 1);
}


ROM_START( srmp5 )
	ROM_REGION( 0x200000, "maincpu", 0 ) // "PRG00" - "PRG03"
	ROM_LOAD32_BYTE( "sx008-11.bin",   0x00000, 0x80000,   CRC(ca15ff45) SHA1(5ee610e0bb835568c36898210a6f8394902d5b54) )
	ROM_LOAD32_BYTE( "sx008-12.bin",   0x00001, 0x80000,   CRC(43e9bb98) SHA1(e46dd98d2e1babfa12ddf2fa9b31377e8691d3a1) )
	ROM_LOAD32_BYTE( "sx008-13.bin",   0x00002, 0x80000,   CRC(0af475e8) SHA1(24cddffa0f8c81832ae8870823d772e3b7493194) )
	ROM_LOAD32_BYTE( "sx008-14.bin",   0x00003, 0x80000,   CRC(b5c55120) SHA1(0a41351c9563b2c6a00709189a917757bd6e0a24) )

	ROM_REGION( 0x400000, "soundcpu", 0 ) // SoundDriverV1.26
	ROM_LOAD( "sx008-08.bin",   0x000000, 0x200000,   CRC(d4ac54f4) SHA1(c3dc76cd71485796a0b6a960294ea96eae8c946e) )
	ROM_LOAD( "sx008-09.bin",   0x200000, 0x200000,   CRC(5a3e6560) SHA1(92ea398f3c5e3035869f0ca5dfe7b05c90095318) )

	ROM_REGION16_LE( 0x1000000, "chr",0) // "CHR00" - "CHR06"
	ROM_LOAD( "sx008-01.bin",   0x000000, 0x200000,   CRC(82dabf48) SHA1(c53e9ed0056c431eab13ab362936c25d3cc5abba) )
	ROM_LOAD( "sx008-02.bin",   0x200000, 0x200000,   CRC(cfd2be0f) SHA1(a21f2928e08047c97443123aceba7ff4e95c6d3d) )
	ROM_LOAD( "sx008-03.bin",   0x400000, 0x200000,   CRC(d7323b10) SHA1(94ecc17b6b8b071cf2c61bbef4aec2c6c7693c62) )
	ROM_LOAD( "sx008-04.bin",   0x600000, 0x200000,   CRC(b10d3067) SHA1(21c36307780d4f38ec54d87cd222d65e4f8c00a5) )
	ROM_LOAD( "sx008-05.bin",   0x800000, 0x200000,   CRC(0ff5e6f5) SHA1(ab7d021757f341d28db6d7d009c20ec9d7bd83c1) )
	ROM_LOAD( "sx008-06.bin",   0xa00000, 0x200000,   CRC(ba6fd7c4) SHA1(f086195c5c647e07e77ce2a23e94d28e6ad9ff4f) )
	ROM_LOAD( "sx008-07.bin",   0xc00000, 0x200000,   CRC(3564485d) SHA1(12464de4e2b6c4df1595183996d1987f0ecffb01) )
#ifdef DEBUG_CHAR
	ROM_REGION( 0x100000, "gfx1", 0)
	ROM_FILL( 0, 0x100000, 0x00)
#endif
ROM_END

void srmp5_state::init_srmp5()
{
	m_tileram = std::make_unique<uint16_t[]>(0x100000/2);
	m_sprram  = std::make_unique<uint16_t[]>(0x080000/2);
#ifdef DEBUG_CHAR
	m_tileduty= make_unique_clear<uint8_t[]>(0x2000);
#endif
}

} // anonymous namespace


GAME( 1994, srmp5, 0, srmp5, srmp5, srmp5_state, init_srmp5, ROT0, "Seta", "Super Real Mahjong P5", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
