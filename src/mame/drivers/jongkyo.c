// license:BSD-3-Clause
// copyright-holders:David Haywood, Nicola Salmoria
/**********************************************************

    Jongkyo
    (c)1985 Kiwako

    834-5558 JONGKYO
    C2-00173

    CPU: SEGA Custom 315-5084 (Z80)
    Sound: AY-3-8910
    OSC: 18.432MHz

    ROMs:
    EPR-6258 (2764)
    EPR-6259 (2764)
    EPR-6260 (2764)
    EPR-6261 (2764)
    EPR-6262 (2732)

    PR-6263.6J (82S123N)
    PR-6264.0H (82S123N)
    PR-6265.0M (82S129N)
    PR-6266.0B (82S129N)

**********************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/segacrpt.h"

#define JONGKYO_CLOCK 18432000


class jongkyo_state : public driver_device
{
public:
	jongkyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu") { }

	/* misc */
	UINT8    m_rom_bank;
	UINT8    m_mux_data;
	UINT8    m_flip_screen;

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	UINT8    m_videoram2[0x4000];
	DECLARE_WRITE8_MEMBER(bank_select_w);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(jongkyo_coin_counter_w);
	DECLARE_WRITE8_MEMBER(videoram2_w);
	DECLARE_WRITE8_MEMBER(unknown_w);
	DECLARE_READ8_MEMBER(input_1p_r);
	DECLARE_READ8_MEMBER(input_2p_r);
	DECLARE_DRIVER_INIT(jongkyo);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(jongkyo);
	UINT32 screen_update_jongkyo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

void jongkyo_state::video_start()
{
}

UINT32 jongkyo_state::screen_update_jongkyo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y;

	for (y = 0; y < 256; ++y)
	{
		int x;

		for (x = 0; x < 256; x += 4)
		{
			int b;
			int res_x,res_y;
			UINT8 data1;
			UINT8 data2;
			UINT8 data3;

	//      data3 = m_videoram2[x/4 + y*64]; // wrong

	// good mahjong tiles
			data3 = 0x0f; // we're missing 2 bits.. there must be another piece of video ram somewhere or we can't use all the colours (6bpp).. banked somehow?
	// good girl tiles
	//  data3 = 0x00; // we're missing 2 bits.. there must be another piece of video ram somewhere or we can't use all the colours (6bpp).. banked somehow?



			data1 = m_videoram[0x4000 + x / 4 + y * 64];
			data2 = m_videoram[x / 4 + y * 64];

			for (b = 0; b < 4; ++b)
			{
				res_x = m_flip_screen ? 255 - (x + b) : (x + b);
				res_y = m_flip_screen ? 255 - y : y;
				bitmap.pix16(res_y, res_x) = ((data2 & 0x01)) + ((data2 & 0x10) >> 3) +
															((data1 & 0x01) << 2) + ((data1 & 0x10) >> 1) +
															((data3 & 0x01) << 4) + ((data3 & 0x10) << 1);
				data1 >>= 1;
				data2 >>= 1;
				data3 >>= 1;
			}
		}
	}

	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(jongkyo_state::bank_select_w)
{
	int mask = 1 << (offset >> 1);

	m_rom_bank &= ~mask;

	if (offset & 1)
		m_rom_bank |= mask;

	membank("bank1")->set_entry(m_rom_bank);
	membank("bank1d")->set_entry(m_rom_bank);
}

WRITE8_MEMBER(jongkyo_state::mux_w)
{
	m_mux_data = ~data;
	//  printf("%02x\n", m_mux_data);
}

WRITE8_MEMBER(jongkyo_state::jongkyo_coin_counter_w)
{
	/* bit 0 = hopper out? */

	/* bit 1 = coin counter */
	coin_counter_w(machine(), 0, data & 2);

	/* bit 2 always set? */
	m_flip_screen = (data & 4) >> 2;
}

READ8_MEMBER(jongkyo_state::input_1p_r)
{
	UINT8 cr_clear = ioport("CR_CLEAR")->read();

	switch (m_mux_data)
	{
		case 0x01: return ioport("PL1_1")->read() | cr_clear;
		case 0x02: return ioport("PL1_2")->read() | cr_clear;
		case 0x04: return ioport("PL1_3")->read() | cr_clear;
		case 0x08: return ioport("PL1_4")->read() | cr_clear;
		case 0x10: return ioport("PL1_5")->read() | cr_clear;
		case 0x20: return ioport("PL1_6")->read() | cr_clear;
	}
	//  printf("%04x\n", m_mux_data);

	return (ioport("PL1_1")->read() & ioport("PL1_2")->read() & ioport("PL1_3")->read() &
			ioport("PL1_4")->read() & ioport("PL1_5")->read() & ioport("PL1_6")->read()) | cr_clear;
}

READ8_MEMBER(jongkyo_state::input_2p_r)
{
	UINT8 coin_port = ioport("COINS")->read();

	switch (m_mux_data)
	{
		case 0x01: return ioport("PL2_1")->read() | coin_port;
		case 0x02: return ioport("PL2_2")->read() | coin_port;
		case 0x04: return ioport("PL2_3")->read() | coin_port;
		case 0x08: return ioport("PL2_4")->read() | coin_port;
		case 0x10: return ioport("PL2_5")->read() | coin_port;
		case 0x20: return ioport("PL2_6")->read() | coin_port;
	}
	//  printf("%04x\n", m_mux_data);

	return (ioport("PL2_1")->read() & ioport("PL2_2")->read() & ioport("PL2_3")->read() &
			ioport("PL2_4")->read() & ioport("PL2_5")->read() & ioport("PL2_6")->read()) | coin_port;
}

WRITE8_MEMBER(jongkyo_state::videoram2_w)
{
	m_videoram2[offset] = data;
}

WRITE8_MEMBER(jongkyo_state::unknown_w)
{
	switch (offset)
	{
		case 0: // different values
			break;
		case 1: // set to 0 at the boot and set to 1 continuesly
			break;
		case 2: // only set to 0 at the boot
			break;
		case 3: // not used
			break;
		case 4: // set to 1 before the girl drawing (probably is the palette selector, not sure how to restore the old palette)
			break;
		case 5: // only set to 0 at the boot
			break;
		case 6: // different values
			break;
		case 7: // 07 and 08 are like a counter: every write in 08 is a incremented value (from 00 to ff)
			break;
		case 8: // when this value is 0xff the next value is 00 and port 07 is incremented (from 00 to ff)
			break;
		case 9: // different values
			break;
	}
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( jongkyo_memmap, AS_PROGRAM, 8, jongkyo_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_WRITE(videoram2_w) // wrong, this doesn't seem to be video ram on write..
	AM_RANGE(0x4000, 0x6bff) AM_ROM // fixed rom
	AM_RANGE(0x6c00, 0x6fff) AM_ROMBANK("bank1")    // banked (8 banks)
	AM_RANGE(0x7000, 0x77ff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, jongkyo_state )
	AM_RANGE(0x0000, 0x6bff) AM_ROMBANK("bank0d")
	AM_RANGE(0x6c00, 0x6fff) AM_ROMBANK("bank1d")
ADDRESS_MAP_END


static ADDRESS_MAP_START( jongkyo_portmap, AS_IO, 8, jongkyo_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// R 01 keyboard
	AM_RANGE(0x01, 0x01) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)

	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW") AM_WRITE(jongkyo_coin_counter_w)
	AM_RANGE(0x11, 0x11) AM_READ_PORT("IN0") AM_WRITE(mux_w)
	// W 11 select keyboard row (fe fd fb f7)
	AM_RANGE(0x40, 0x40) AM_READNOP // unknown, if (A & 0xf) == 0x0a then a bit 0 write to 0x7520 doesn't occur
	AM_RANGE(0x40, 0x45) AM_WRITE(bank_select_w)
	AM_RANGE(0x46, 0x4f) AM_WRITE(unknown_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
-------------------------------------------------------------
Jongkyo ?1985 Kiwako
DIPSW         |      |1    2    3    4   |5   |6   |7   |8
-------------------------------------------------------------
Payout rate   |50%   |on   on   on   on  |    |    |    |
              |53%   |off  on   on   on  |    |    |    |
              |56%   |on   off  on   on  |    |    |    |
              |59%   |off  off  on   on  |    |    |    |
              |62%   |on   on   off  on  |    |    |    |
              |65%   |off  on   off  on  |    |    |    |
              |68%   |on   off  off  on  |    |    |    |
              |71%   |off  off  off  on  |    |    |    |
              |75%   |on   on   on   off |    |    |    |
              |78%   |off  on   on   off |    |    |    |
              |81%   |on   off  on   off |    |    |    |
              |84%   |off  off  on   off |    |    |    |
              |87%   |on   on   off  off |    |    |    |
              |90%   |off  on   off  off |    |    |    |
              |93%   |on   off  off  off |    |    |    |
              |96%   |off  off  off  off |    |    |    |
-------------------------------------------------------------
Start chance  |Yes   |                   |on  |    |    |
              |No    |                   |off |    |    |
-------------------------------------------------------------
Bet up        |Yes   |                   |    |on  |    |
              |No    |                   |    |off |    |
-------------------------------------------------------------
Last chance   |5     |                   |    |    |on  |
              |1     |                   |    |    |off |
-------------------------------------------------------------
Bonus credit  |50    |                   |    |    |    |on
              |10    |                   |    |    |    |off
-------------------------------------------------------------

 *************************************/


static INPUT_PORTS_START( jongkyo )
	PORT_START("CR_CLEAR")
	PORT_DIPNAME( 0x40, 0x40, "Credit Clear-1" )//button
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Credit Clear-2" )//button
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) //player-1 side
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) //player-2 side

	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_CODE(KEYCODE_3)//rate button
	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //another D button
	PORT_START("PL1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED ) //another opt 1 button
	PORT_START("PL1_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 4")
	PORT_START("PL1_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Pass") //???
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_CODE(KEYCODE_4) PORT_PLAYER(2)//rate button
	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //another D button
	PORT_START("PL2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED ) //another opt 1 button
	PORT_START("PL2_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 1") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 2") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 3") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 4") PORT_PLAYER(2)
	PORT_START("PL2_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Pass") PORT_PLAYER(2) //???
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "Note" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Memory Reset" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Analyzer" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0f, 0x0f, "Payout Rate" ) PORT_DIPLOCATION("SW:1,2,3,4")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x0b, "84$" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPNAME( 0x10, 0x10, "Start Chance" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Bet Up" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Last Chance" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Bonus Credit" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x00, "50" )
INPUT_PORTS_END


/*************************************
 *
 *  Palette initialization and
 *    graphics definitions
 *
 *************************************/

PALETTE_INIT_MEMBER(jongkyo_state, jongkyo)
{
	int i;
	UINT8* proms = memregion("proms")->base();
	for (i = 0; i < 0x40; i++)
	{
		int data = proms[i];

		int r = (data  >> 0) & 0x07;
		int g = (data  >> 3) & 0x07;
		int b = (data  >> 6) & 0x03;

			palette.set_pen_color(i, r << 5, g << 5, b << 6 );

	}
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void jongkyo_state::machine_start()
{
	save_item(NAME(m_videoram2));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_mux_data));
	save_item(NAME(m_flip_screen));
}

void jongkyo_state::machine_reset()
{
	m_rom_bank = 0;
	m_mux_data = 0;
	m_flip_screen = 1;
}


static MACHINE_CONFIG_START( jongkyo, jongkyo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,JONGKYO_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(jongkyo_memmap)
	MCFG_CPU_IO_MAP(jongkyo_portmap)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", jongkyo_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 8, 256-8-1)
	MCFG_SCREEN_UPDATE_DRIVER(jongkyo_state, screen_update_jongkyo)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(jongkyo_state, jongkyo)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, JONGKYO_CLOCK/8)
	MCFG_AY8910_PORT_A_READ_CB(READ8(jongkyo_state, input_1p_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(jongkyo_state, input_2p_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( jongkyo )
	ROM_REGION( 0x8c00, "maincpu", 0 )
	ROM_LOAD( "epr-6258", 0x00000, 0x02000, CRC(fb8b7bcc) SHA1(8ece7c2c82c237b4b51829d412b2109b96ccd0e7) )
	ROM_LOAD( "epr-6259", 0x02000, 0x02000, CRC(e46cde5d) SHA1(1cbe1677cfb3fa9f76ad90d5b1446ce9cefee6b7) )
	ROM_LOAD( "epr-6260", 0x04000, 0x02000, CRC(369a5365) SHA1(037a2971a59ab339595b333cbdfd4cbb104de2be) )
	ROM_LOAD( "epr-6262", 0x06000, 0x00c00, CRC(ecf50f34) SHA1(ecfa1a9360d8fbcbed457d46e53bae77f6d78c1d) )
	ROM_IGNORE(0x400)
	ROM_LOAD( "epr-6261", 0x06c00, 0x02000, CRC(9c475ae1) SHA1(b993c2636dafed9f80fa87e71921c3c85c039e45) )  // banked at 6c00-6fff

	ROM_REGION( 0x300, "proms", 0 )
	/* colours */
	ROM_LOAD( "pr-6263.6j", 0x00000, 0x00020, CRC(468134d9) SHA1(bb633929df17e448882ee80613fc1dfac3c35d7a) )
	ROM_LOAD( "pr-6264.0h", 0x00020, 0x00020, CRC(46014727) SHA1(eec451f292ee319fa6bfbbf223aaa12b231692c1) )

	/* unknown purpose */
	ROM_LOAD( "pr-6265.0m", 0x00100, 0x00100, CRC(f09d3c4c) SHA1(a9e752d75e7f3ebd05add4ccf2f9f15d8f9a8d15) )
	ROM_LOAD( "pr-6266.0b", 0x00200, 0x00100, CRC(86aeafd1) SHA1(c4e5c56ce5baf2be3962675ae333e28bd8108a00) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(jongkyo_state,jongkyo)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x28,0x08,0xa8,0x88 }, { 0xa0,0xa8,0x20,0x28 },   /* ...0...0...0...0 */
		{ 0x80,0x88,0xa0,0xa8 }, { 0xa0,0xa8,0x20,0x28 },   /* ...0...0...0...1 */
		{ 0xa0,0xa8,0x20,0x28 }, { 0x20,0xa0,0x00,0x80 },   /* ...0...0...1...0 */
		{ 0xa0,0xa8,0x20,0x28 }, { 0x80,0x88,0xa0,0xa8 },   /* ...0...0...1...1 */
		{ 0x08,0x88,0x00,0x80 }, { 0x08,0x88,0x00,0x80 },   /* ...0...1...0...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x08,0x88,0x00,0x80 },   /* ...0...1...0...1 */
		{ 0x20,0xa0,0x00,0x80 }, { 0x20,0xa0,0x00,0x80 },   /* ...0...1...1...0 */
		{ 0x08,0x88,0x00,0x80 }, { 0x08,0x88,0x00,0x80 },   /* ...0...1...1...1 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0xa0,0xa8,0x20,0x28 },   /* ...1...0...0...0 */
		{ 0x80,0x88,0xa0,0xa8 }, { 0x80,0x88,0xa0,0xa8 },   /* ...1...0...0...1 */
		{ 0xa0,0xa8,0x20,0x28 }, { 0x20,0xa0,0x00,0x80 },   /* ...1...0...1...0 */
		{ 0xa0,0xa8,0x20,0x28 }, { 0x80,0x88,0xa0,0xa8 },   /* ...1...0...1...1 */
		{ 0x08,0x88,0x00,0x80 }, { 0x28,0x08,0xa8,0x88 },   /* ...1...1...0...0 */
		{ 0x08,0x88,0x00,0x80 }, { 0x80,0x88,0xa0,0xa8 },   /* ...1...1...0...1 */
		{ 0x28,0x08,0xa8,0x88 }, { 0x20,0xa0,0x00,0x80 },   /* ...1...1...1...0 */
		{ 0x80,0x88,0xa0,0xa8 }, { 0x08,0x88,0x00,0x80 }    /* ...1...1...1...1 */
	};

	UINT8 *rom = memregion("maincpu")->base();

	/* first of all, do a simple bitswap */
	for (int i = 0x6000; i < 0x8c00; ++i)
	{
		rom[i] = BITSWAP8(rom[i], 7,6,5,3,4,2,1,0);
	}

	UINT8 *opcodes = auto_alloc_array(machine(), UINT8, 0x6c00+0x400*8);

	/* then do the standard Sega decryption */
	sega_decode(rom, opcodes, 0x6c00, convtable, 8, 0x400);

	membank("bank1")->configure_entries(0, 8, rom+0x6c00, 0x400);
	membank("bank1d")->configure_entries(0, 8, opcodes+0x6c00, 0x400);
	membank("bank0d")->set_base(opcodes);
}


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1985, jongkyo,  0,    jongkyo, jongkyo, jongkyo_state,  jongkyo, ROT0, "Kiwako", "Jongkyo", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
