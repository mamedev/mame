// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************************************************

                                            -= Astro Corp. CGA Hardware =-

                                       driver by   Luca Elia (l.elia@tin.it)

CPU:    68000
GFX:    ASTRO V0x (x = 1,2,5,6 or 7)
SOUND:  OKI M6295 (AD-65)
OTHER:  EEPROM, Battery

 512 sprites, each made of N x M tiles. Tiles are 16x16x8

-------------------------------------------------------------------------------------------------------------------
Year + Game          PCB ID         CPU                Video        Chips                                   Notes
-------------------------------------------------------------------------------------------------------------------
00  Show Hand        CHE-B50-4002A  68000              ASTRO V01    pLSI1016-60LJ, ASTRO 0001B   (28 pins)
00  Wang Pai Dui J.  CHE-B50-4002A  68000              ASTRO V01    pLSI1016,      MDT2020AP MCU (28 pins)
02  Skill Drop GA    None           JX-1689F1028N      ASTRO V02    pLSI1016-60LJ
02? Keno 21          ?              ASTRO V102?        ASTRO V05    ASTRO F02?                              not dumped
03  Speed Drop       None           JX-1689HP          ASTRO V05    pLSI1016-60LJ
03? Dino Dino        T-3802A        ASTRO V102PX-010?  ASTRO V05    ASTRO F02 2003-03-12                    Encrypted
04? Stone Age        L1             ASTRO V102PX-012?  ASTRO V05x2  ASTRO F02 2004-09-04                    Encrypted
05? Zoo              M1.1           ASTRO V102PX-005?  ASTRO V06    ASTRO F02 2005-02-18                    Encrypted
05? Win Win Bingo    M1.2           ASTRO V102PX-006?  ASTRO V06    ASTRO F02 2005-09-17                    Encrypted
05? Hacher (hack)    M1.2           ?                  ?            ASTRO F02 2005-02-18                    Encrypted
07? Western Venture  CS350P032      ASTRO V102?        ASTRO V07    ASTRO F01 2007-06-03                    Encrypted
-------------------------------------------------------------------------------------------------------------------

To do:

- Find source of level 2 interrupt
- Decrypt newer games

*************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/ticket.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"

class astrocorp_state : public driver_device
{
public:
	astrocorp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<UINT16> m_spriteram;

	// video-related
	bitmap_ind16 m_bitmap;
	UINT16     m_screen_enable;
	UINT16     m_draw_sprites;
	DECLARE_WRITE16_MEMBER(astrocorp_draw_sprites_w);
	DECLARE_WRITE16_MEMBER(astrocorp_eeprom_w);
	DECLARE_WRITE16_MEMBER(showhand_outputs_w);
	DECLARE_WRITE16_MEMBER(skilldrp_outputs_w);
	DECLARE_WRITE16_MEMBER(astrocorp_screen_enable_w);
	DECLARE_READ16_MEMBER(astrocorp_unk_r);
	DECLARE_WRITE16_MEMBER(astrocorp_sound_bank_w);
	DECLARE_WRITE16_MEMBER(skilldrp_sound_bank_w);
	DECLARE_DRIVER_INIT(astoneag);
	DECLARE_DRIVER_INIT(showhanc);
	DECLARE_DRIVER_INIT(showhand);
	DECLARE_VIDEO_START(astrocorp);
	UINT32 screen_update_astrocorp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(skilldrp_scanline);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/***************************************************************************
                                Video
***************************************************************************/

VIDEO_START_MEMBER(astrocorp_state,astrocorp)
{
	m_screen->register_screen_bitmap(m_bitmap);

	save_item(NAME(m_bitmap));
	save_item(NAME(m_screen_enable));
	save_item(NAME(m_draw_sprites));
}

/***************************************************************************
                              Sprites Format

    Offset:    Bits:                  Value:

    0          f--- ---- ---- ----    Show This Sprite
               -e-- ---- ---- ----    ? set to 0
               --dc ba9- ---- ----    ignored?
               ---- ---8 7654 3210    X

    1                                 Code

    2          fedc ba98 ---- ----    ignored?
               ---- ---- 7654 3210    Y

    3          fedc ba98 ---- ----    X Size
               ---- ---- 7654 3210    Y Size

    If the first two words are zero, the sprite list ends

***************************************************************************/

void astrocorp_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *source = m_spriteram;
	UINT16 *finish = m_spriteram + m_spriteram.bytes() / 2;

	for ( ; source < finish; source += 8 / 2 )
	{
		int x, y;
		int xwrap, ywrap;

		int sx      = source[ 0x0/2 ];
		int code    = source[ 0x2/2 ];
		int sy      = source[ 0x4/2 ];
		int attr    = source[ 0x6/2 ];

		int dimx    = (attr >> 8) & 0xff;
		int dimy    = (attr >> 0) & 0xff;

		if (!sx && !code)
			return;

		if (!(sx & 0x8000))
			continue;

		sx &= 0x01ff;
		sy &= 0x00ff;

		for (y = 0 ; y < dimy ; y++)
		{
			for (x = 0 ; x < dimx ; x++)
			{
				for (ywrap = 0 ; ywrap <= 0x100 ; ywrap += 0x100)
				{
					for (xwrap = 0 ; xwrap <= 0x200 ; xwrap += 0x200)
					{
						m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
								code, 0,
								0, 0,
								sx + x * 16 - xwrap, sy + y * 16 - ywrap, 0xff);
					}
				}
				code++;
			}
		}
	}
}

UINT32 astrocorp_state::screen_update_astrocorp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_screen_enable & 1)
		copybitmap(bitmap, m_bitmap, 0,0,0,0, cliprect);
	else
		bitmap.fill(m_palette->black_pen(), cliprect);

	return 0;
}


/***************************************************************************
                                Memory Maps
***************************************************************************/

WRITE16_MEMBER(astrocorp_state::astrocorp_draw_sprites_w)
{
	UINT16 old = m_draw_sprites;
	UINT16 now = COMBINE_DATA(&m_draw_sprites);

	if (!old && now)
		draw_sprites(m_bitmap, m_screen->visible_area());
}

WRITE16_MEMBER(astrocorp_state::astrocorp_eeprom_w)
{
	if (ACCESSING_BITS_0_7)
	{
		ioport("EEPROMOUT")->write(data, 0xff);
	}
}

WRITE16_MEMBER(astrocorp_state::astrocorp_sound_bank_w)
{
	if (ACCESSING_BITS_8_15)
	{
		m_oki->set_bank_base(0x40000 * ((data >> 8) & 1));
//      logerror("CPU #0 PC %06X: OKI bank %08X\n", space.device().safe_pc(), data);
	}
}

WRITE16_MEMBER(astrocorp_state::skilldrp_sound_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_oki->set_bank_base(0x40000 * (data & 1));
//      logerror("CPU #0 PC %06X: OKI bank %08X\n", space.device().safe_pc(), data);
	}
}

WRITE16_MEMBER(astrocorp_state::showhand_outputs_w)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0,    (data & 0x0004));   // coin counter
		output().set_led_value(0,    (data & 0x0008));   // you win
		if ((data & 0x0010)) machine().bookkeeping().increment_dispensed_tickets(1); // coin out
		output().set_led_value(1,    (data & 0x0020));   // coin/hopper jam
	}
	if (ACCESSING_BITS_8_15)
	{
		output().set_led_value(2,    (data & 0x0100));   // bet
		output().set_led_value(3,    (data & 0x0800));   // start
		output().set_led_value(4,    (data & 0x1000));   // ? select/choose
		output().set_led_value(5,    (data & 0x2000));   // ? select/choose
		output().set_led_value(6,    (data & 0x4000));   // look
	}
//  popmessage("%04X",data);
}

WRITE16_MEMBER(astrocorp_state::skilldrp_outputs_w)
{
	// key in          (0001)
	// coin in         (0002)
	// key out         (0004)
	// coin out        (0008)
	// hopper?         (0010)
	// error  lamp     (0020)
	// ticket motor?   (0080)
	// select lamp     (0100)
	// take   lamp     (0400)
	// bet    lamp     (0800)
	// start  lamp     (1000)
	// win / test lamp (4000)
	// ticket?         (8000)

	// account    (5d20 4d20 4520 4420 4020 4000)

	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0,    (data & 0x0001));   // key in  |
		machine().bookkeeping().coin_counter_w(0,    (data & 0x0002));   // coin in |- manual shows 1 in- and 1 out- counter
		machine().bookkeeping().coin_counter_w(1,    (data & 0x0004));   // key out |
		machine().device<ticket_dispenser_device>("hopper")->write(space, 0, (data & 0x0008)<<4);   // hopper motor?
		//                                  (data & 0x0010)     // hopper?
		output().set_led_value(0,    (data & 0x0020));   // error lamp (coin/hopper jam: "call attendant")
		machine().device<ticket_dispenser_device>("ticket")->write(space, 0, data & 0x0080);    // ticket motor?
	}
	if (ACCESSING_BITS_8_15)
	{
		// lamps:
		output().set_led_value(1,    (data & 0x0100));   // select
		output().set_led_value(2,    (data & 0x0400));   // take
		output().set_led_value(3,    (data & 0x0800));   // bet
		output().set_led_value(4,    (data & 0x1000));   // start
		output().set_led_value(5,    (data & 0x4000));   // win / test
		output().set_led_value(6,    (data & 0x8000));   // ticket?
	}

//  popmessage("%04X",data);
}

WRITE16_MEMBER(astrocorp_state::astrocorp_screen_enable_w)
{
	COMBINE_DATA(&m_screen_enable);
//  popmessage("%04X",data);
	if (m_screen_enable & (~1))
		logerror("CPU #0 PC %06X: screen enable = %04X\n", space.device().safe_pc(), m_screen_enable);
}

READ16_MEMBER(astrocorp_state::astrocorp_unk_r)
{
	return 0xffff;  // bit 3?
}


static ADDRESS_MAP_START( showhand_map, AS_PROGRAM, 16, astrocorp_state )
	AM_RANGE( 0x000000, 0x01ffff ) AM_ROM
	AM_RANGE( 0x050000, 0x050fff ) AM_RAM AM_SHARE("spriteram")
	AM_RANGE( 0x052000, 0x052001 ) AM_WRITE(astrocorp_draw_sprites_w)
	AM_RANGE( 0x054000, 0x054001 ) AM_READ_PORT("INPUTS")
	AM_RANGE( 0x058000, 0x058001 ) AM_WRITE(astrocorp_eeprom_w)
	AM_RANGE( 0x05a000, 0x05a001 ) AM_WRITE(showhand_outputs_w)
	AM_RANGE( 0x05e000, 0x05e001 ) AM_READ_PORT("EEPROMIN")
	AM_RANGE( 0x060000, 0x0601ff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE( 0x070000, 0x073fff ) AM_RAM AM_SHARE("nvram") // battery
	AM_RANGE( 0x080000, 0x080001 ) AM_WRITE(astrocorp_sound_bank_w)
	AM_RANGE( 0x0a0000, 0x0a0001 ) AM_WRITE(astrocorp_screen_enable_w)
	AM_RANGE( 0x0d0000, 0x0d0001 ) AM_READ(astrocorp_unk_r) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00)
ADDRESS_MAP_END

static ADDRESS_MAP_START( showhanc_map, AS_PROGRAM, 16, astrocorp_state )
	AM_RANGE( 0x000000, 0x01ffff ) AM_ROM
	AM_RANGE( 0x060000, 0x0601ff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE( 0x070000, 0x070001 ) AM_WRITE(astrocorp_sound_bank_w)
	AM_RANGE( 0x080000, 0x080fff ) AM_RAM AM_SHARE("spriteram")
	AM_RANGE( 0x082000, 0x082001 ) AM_WRITE(astrocorp_draw_sprites_w)
	AM_RANGE( 0x084000, 0x084001 ) AM_READ_PORT("INPUTS")
	AM_RANGE( 0x088000, 0x088001 ) AM_WRITE(astrocorp_eeprom_w)
	AM_RANGE( 0x08a000, 0x08a001 ) AM_WRITE(showhand_outputs_w)
	AM_RANGE( 0x08e000, 0x08e001 ) AM_READ_PORT("EEPROMIN")
	AM_RANGE( 0x090000, 0x093fff ) AM_RAM AM_SHARE("nvram") // battery
	AM_RANGE( 0x0a0000, 0x0a0001 ) AM_WRITE(astrocorp_screen_enable_w)
	AM_RANGE( 0x0e0000, 0x0e0001 ) AM_READ(astrocorp_unk_r) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00)
ADDRESS_MAP_END

static ADDRESS_MAP_START( skilldrp_map, AS_PROGRAM, 16, astrocorp_state )
	AM_RANGE( 0x000000, 0x03ffff ) AM_ROM
	AM_RANGE( 0x200000, 0x200fff ) AM_RAM AM_SHARE("spriteram")
	AM_RANGE( 0x202000, 0x202001 ) AM_WRITE(astrocorp_draw_sprites_w)
	AM_RANGE( 0x204000, 0x204001 ) AM_READ_PORT("INPUTS")
	AM_RANGE( 0x208000, 0x208001 ) AM_WRITE(astrocorp_eeprom_w)
	AM_RANGE( 0x20a000, 0x20a001 ) AM_WRITE(skilldrp_outputs_w)
	AM_RANGE( 0x20e000, 0x20e001 ) AM_READ_PORT("EEPROMIN")
	AM_RANGE( 0x380000, 0x3801ff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE( 0x400000, 0x400001 ) AM_WRITE(astrocorp_screen_enable_w)
	AM_RANGE( 0x500000, 0x507fff ) AM_RAM AM_SHARE("nvram") // battery
	AM_RANGE( 0x580000, 0x580001 ) AM_WRITE(skilldrp_sound_bank_w)
	AM_RANGE( 0x600000, 0x600001 ) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( speeddrp_map, AS_PROGRAM, 16, astrocorp_state )
	AM_RANGE( 0x000000, 0x01ffff ) AM_ROM
	AM_RANGE( 0x280000, 0x283fff ) AM_RAM AM_SHARE("nvram") // battery
	AM_RANGE( 0x380000, 0x380fff ) AM_RAM AM_SHARE("spriteram")
	AM_RANGE( 0x382000, 0x382001 ) AM_WRITE(astrocorp_draw_sprites_w)
	AM_RANGE( 0x384000, 0x384001 ) AM_READ_PORT("INPUTS")
	AM_RANGE( 0x388000, 0x388001 ) AM_WRITE(astrocorp_eeprom_w)
	AM_RANGE( 0x38a000, 0x38a001 ) AM_WRITE(skilldrp_outputs_w)
	AM_RANGE( 0x38e000, 0x38e001 ) AM_READ_PORT("EEPROMIN")
	AM_RANGE( 0x480000, 0x4801ff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE( 0x500000, 0x500001 ) AM_WRITE(astrocorp_screen_enable_w)
	AM_RANGE( 0x580000, 0x580001 ) AM_WRITE(skilldrp_sound_bank_w)
	AM_RANGE( 0x600000, 0x600001 ) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
ADDRESS_MAP_END

/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( showhand )
	PORT_START("INPUTS")    // 54000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1     )   PORT_IMPULSE(1) // coin
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON5   )   PORT_NAME("Payout") PORT_CODE(KEYCODE_F1) // payout (must be 0 on startup)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN   )   // ?
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_START2    )   PORT_NAME("Bet / Double")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON3   )   PORT_NAME("Look / Small")
	PORT_SERVICE_NO_TOGGLE( 0x0020,   IP_ACTIVE_LOW )   // settings
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN   )   // ?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_SPECIAL   )   // coin sensor
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_BUTTON2   )   PORT_NAME("Yes / Big")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_BUTTON4   )   PORT_NAME("Hold1")  // HOLD1 in test mode
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_BUTTON1   )   PORT_NAME("Select")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_START1    )   PORT_NAME("Start / Take")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SERVICE1  )   PORT_NAME("Reset Settings") // when 1 in test mode: reset settings (must be 0 on startup)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN   )   // ?
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_COIN2     )   // key in
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_SPECIAL   )   // coin sensor

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0xfff7, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

static INPUT_PORTS_START( showhanc )
	PORT_START("INPUTS")    // 84000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1     )   PORT_IMPULSE(1) // coin
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON5   )   PORT_NAME("Payout") PORT_CODE(KEYCODE_F1) // payout (must be 0 on startup)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_START2    )   PORT_NAME("Bet / Double")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1   )   PORT_NAME("Select")
	PORT_SERVICE_NO_TOGGLE( 0x0020,   IP_ACTIVE_LOW )   // settings
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_START1    )   PORT_NAME("Start / Take")   // HOLD1 in test mode
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_BUTTON3   )   PORT_NAME("Look / Small / Exit")    // HOLD5 in test mode
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON4   )   PORT_NAME("Hold2")  // HOLD2 in test mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_BUTTON2   )   PORT_NAME("Yes / Big")  // HOLD4 in test mode
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SERVICE1  )   PORT_NAME("Reset Settings") // when 1 in test mode: reset settings (must be 0 on startup)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SPECIAL   )   // must be 0 for inputs to work
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_COIN2     )   PORT_IMPULSE(1) // key in (shows an error)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN   )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0xfff7, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

static INPUT_PORTS_START( skilldrp )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1         )   PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_GAMBLE_TAKE   )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_START3        )   PORT_NAME("Select / Double")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_START1        )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_OTHER         )   PORT_CODE(KEYCODE_T) PORT_NAME("Ticket Out")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_START2        )   PORT_NAME("Bet")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r) // ticket sw
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK   )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sw
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYIN  )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0xfff7, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_16x16x8 =
{
	16, 16,
	RGN_FRAC(1, 1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16*16*8
};

static GFXDECODE_START( astrocorp )
	GFXDECODE_ENTRY("sprites", 0, layout_16x16x8, 0, 1)
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static const UINT16 showhand_default_eeprom[15] =   {0x0001,0x0007,0x000a,0x0003,0x0000,0x0009,0x0003,0x0000,0x0002,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000};


/*
TODO: understand if later hardware uses different parameters (XTAL is almost surely NOT 20 MHz so ...). Also, weirdly enough, there's an unused
6x PAL XTAL according to notes, but VSync = 58,85 Hz?
*/
#define ASTROCORP_PIXEL_CLOCK XTAL_20MHz/2
#define ASTROCORP_HTOTAL 651
#define ASTROCORP_HBEND 0
//#define ASTROCORP_HBSTART 320
#define ASTROCORP_VTOTAL 261
#define ASTROCORP_VBEND 0
#define ASTROCORP_VBSTART 240

static MACHINE_CONFIG_START( showhand, astrocorp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz / 2)
	MCFG_CPU_PROGRAM_MAP(showhand_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", astrocorp_state,  irq4_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DATA(showhand_default_eeprom, sizeof(showhand_default_eeprom))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(58.846)    // measured on pcb
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
//  MCFG_SCREEN_SIZE(320, 240)
//  MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_RAW_PARAMS(ASTROCORP_PIXEL_CLOCK,ASTROCORP_HTOTAL,ASTROCORP_HBEND,320,ASTROCORP_VTOTAL,ASTROCORP_VBEND,ASTROCORP_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(astrocorp_state, screen_update_astrocorp)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", astrocorp)
	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_FORMAT(BBBBBGGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(astrocorp_state,astrocorp)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_20MHz/20, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( showhanc, showhand )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(showhanc_map)
MACHINE_CONFIG_END


TIMER_DEVICE_CALLBACK_MEMBER(astrocorp_state::skilldrp_scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq. controls sprites, sound, i/o
		m_maincpu->set_input_line(4, HOLD_LINE);

	if(scanline == 0) // vblank-in? controls palette
		m_maincpu->set_input_line(2, HOLD_LINE);
}

static MACHINE_CONFIG_START( skilldrp, astrocorp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz / 2) // JX-1689F1028N GRX586.V5
	MCFG_CPU_PROGRAM_MAP(skilldrp_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", astrocorp_state, skilldrp_scanline, "screen", 0, 1)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW )
	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(58.846)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
//  MCFG_SCREEN_SIZE(0x200, 0x100)
//  MCFG_SCREEN_VISIBLE_AREA(0, 0x200-1, 0, 0xf0-1)
	MCFG_SCREEN_RAW_PARAMS(ASTROCORP_PIXEL_CLOCK,ASTROCORP_HTOTAL,ASTROCORP_HBEND,512,ASTROCORP_VTOTAL,ASTROCORP_VBEND,ASTROCORP_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(astrocorp_state, screen_update_astrocorp)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", astrocorp)
	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_FORMAT(BBBBBGGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(astrocorp_state,astrocorp)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_24MHz/24, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( speeddrp, skilldrp )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(speeddrp_map)
MACHINE_CONFIG_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

/***************************************************************************

Show Hand
(C) 2000? Astro Corp.

PCB CHE-B50-4002A 88 94V-0 0002

CPU     1x MC68HC000FN12 (main)
        1x pLSI1016-60LJ (main)
        1x ASTRO V01 0005 (custom)
        1x AD-65 (equivalent to OKI6295)(sound)
        1x ASTRO 0001B (custom)
        1x oscillator 20.000MHz
        1x oscillator 25.601712MHz (Note Dec/3/2012: should this be 26.601712Mhz, 6x PAL subcarrier?)

ROMs    2x 27C512 (1,2)
        2x M27C801 (3,4)
        1x M27C4001 (5)
        1x 93C46 (not dumped)

Note    1x 28x2 JAMMA edge connector
        1x 18x2 edge connector
        1x 10x2 edge connector
        1x pushbutton
        1x trimmer (volume)
        1x 2x2 switches dip

Hardware info by f205v

***************************************************************************/

ROM_START( showhand )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1-8.even.u16", 0x00000, 0x10000, CRC(cf34bf0d) SHA1(72ad7ca63ef89451b2572d64cccfa764b9d9b353) )
	ROM_LOAD16_BYTE( "2-8.odd.u17",  0x00001, 0x10000, CRC(dd031c36) SHA1(198d0e685dd2d824a04c787f8a17c173efa272d9) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "4.even.u26", 0x000000, 0x100000, CRC(8a706e42) SHA1(989688ee3a5e4fc11fb502e43c9d6012488982ee) )
	ROM_LOAD16_BYTE( "3.odd.u26",  0x000001, 0x100000, CRC(a624b750) SHA1(fc5b09f8a10cba5fb2474e1edd62a0400177a5ad) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5", 0x00000, 0x80000, CRC(e6987122) SHA1(fb3e7c2399057c64b5c496a393f6f22a1e54c844) )
ROM_END

/***************************************************************************

Show Hand
Astro Corp, 199?

PCB Layout
----------
CHE-B50-4002A
|----------------------------------------|
|     LATTICE    JAMMA   SW   VOL UPC1242|
|     PLSI1016           U26             |
|                6264                 U43|
|                        U25    M6295    |
|     68000                              |
|                U17    |-----|          |
|                       |ASTRO|          |
|                U16    |V01  |          |
|                       |-----|MDT2020AP/3V
|                6264           20MHz    |
|DSW(2)                      26.601712MHz|
|93C46           6116  6116    KM681000  |
|BATTERY         6116  6116    KM681000  |
|----------------------------------------|
Notes:
      68000 clock - 10.000MHz [20/2]
      M6295 clock - 1.000MHz [20/20], pin 7 HIGH
      VSync - 58.846Hz
      HSync - 15.354kHz

Hardware info by Guru

***************************************************************************/

ROM_START( showhanc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1even.u16", 0x00000, 0x10000, CRC(d1295bdb) SHA1(bb035ee89b21368fb11c3b9cd23164b68feb84bd) )
	ROM_LOAD16_BYTE( "2odd.u17",  0x00001, 0x10000, CRC(bbca78e7) SHA1(a163569acad8d6b8821602ce24013fc46887aba9) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "4even.u26", 0x00000, 0x100000, CRC(285375e0) SHA1(63b47105f0751c65e528139074f5b342450495ba) )
	ROM_LOAD16_BYTE( "3odd.u25",  0x00001, 0x100000, CRC(b93e3a91) SHA1(5192375c32518532e08bddfe000efdee587e1ecc) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5.u43", 0x00000, 0x80000, CRC(d6b70f02) SHA1(5a94680594c1f06196fe3bcf7faf56e2ed576f01) )
ROM_END

/***************************************************************************

Skill Drop Georgia

"Sep 13 2002 09:17:54" in code rom and AAI276832 on sticker.

No specific PCB model or numer....

 Astro V02 0022 160pin PQFP ("ASTRO02" silkscreened under chip)
 JX-1689F1028N GRC586.V5 (68K core, has direct connection to program roms)
 Lattice IspLSI 1016 60LJ socketted FPGA
 OKI 6295 clone chip (AD-65 or U6295)

EEPROM Atmel 93C46
Battery 3.6V
OSC    24.000MHz

PC1 is a push button for test mode
VR1 is for sound volume

      +---------+   +----------------------------+ +----+
  +---+Connector+---+   28 Pin Edge  Connector   +-+    |
  |                                                  VR1|
  |                                                     |
+-+             +------+                                |
|      ULN2003A |IspLSI| UT6264CPC     ROM#4*           |
|  ULN2003A     | 1016 |             +---------+        |
|8              +------++----------+ |ROM#6 U26|        |
|                       |ROM#2  U20| +---------+  +----+|
|L        +-------+     +----------+   ROM#3*     |6295||
|i        | JX-   |                               +----+|
|n        | 1689F | +--------------+                    |
|e        | 1028N | |  ROM#7 U100  |                    |
|r        +-------+ +--------------+                    |
|                                    +----------+   +---+
|C                      +----------+ |          |   |   |
|o                      |ROM#1  U21| |  Astro   |   | R |
|n                      +----------+ |  V02     |   | O |
|n                                   |  0022    |   | M |
|e                                   +----------+   | # |
|c                       UT6264CPC                  | 5 |
|t                                                  |   |
|o                                                  +---+
|r     93C46              6116      6116      RAM1      |
|                                                  24MHz|
+-+ BAT1 PC1              6116      6116      RAM1      |
  +-----------------------------------------------------+

ROM#1 & ROM#2 are 32pin sockets
ROM#7 is a 40pin socket
ROM#3 & ROM#4 at U27 & U25 are optional unpopulated 32pin sockets and overlap with ROM#6
ROM#6 is 29F1610MC flash rom

RAM1 are SEC KM681000BLG-7L RAM chips

***************************************************************************/

ROM_START( skilldrp )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "7-skill_drop_g1.0s.u100", 0x00000, 0x40000, CRC(f968b783) SHA1(1d693b1d460e659ca94aae8625ea26e120053f84) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mx29f1610amc.u26", 0x000000, 0x200000, CRC(4fdac800) SHA1(bcafceb6c34866c474714347e23f9e819b5fcfa6) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5-skill_drop", 0x00000, 0x80000, CRC(a479e06d) SHA1(ee690d39188b8a43652c4aa5bf8267c1f6632d2f) ) // No chip location just "ROM#5" silkscreened under socket

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "skilldrp.u6", 0x00, 0x80, CRC(57886a3d) SHA1(bad8fa2ec2262ccb5ef8ec50959aec3f3bf8b90b) )
ROM_END

/***************************************************************************

Speed Drop

"16:01:26 Sep 3 2003" in code

No specific PCB model or numer, same as used for Skill Drop but with newer video chip

 Astro V05 0206 160pin PQFP ("ASTRO02" silkscreened under chip)
 JX-1689HP TA5265188 (68K core, has direct connection to program roms)
 Lattice IspLSI 1016 60LJ socketted FPGA
 OKI 6295 clone chip (AD-65 or U6295)

EEPROM  Atmel 93C46
Battery 3.6V
OSC     24.000MHz

PC1 is a push button for test mode
VR1 is for sound volume

      +---------+   +----------------------------+ +----+
  +---+Connector+---+   28 Pin Edge  Connector   +-+    |
  |                                                  VR1|
  |                                                     |
+-+             +------+                                |
|      ULN2003A |IspLSI| UT6264CPC     ROM#4*           |
|  ULN2003A     | 1016 |             +---------+        |
|8              +------++----------+ |ROM#6 U26|        |
|                       |ROM#2  U20| +---------+  +----+|
|L        +-------+     +----------+   ROM#3*     |6295||
|i        | JX-   |                               +----+|
|n        | 1689HP| +--------------+                    |
|e        |       | |  ROM#7 U100  |                    |
|r        +-------+ +--------------+                    |
|                                    +----------+   +---+
|C                      +----------+ |          |   |   |
|o                      |ROM#1  U21| |  Astro   |   | R |
|n                      +----------+ |  V05     |   | O |
|n                                   |  0206    |   | M |
|e                                   +----------+   | # |
|c                       UT6264CPC                  | 5 |
|t                                                  |   |
|o                                                  +---+
|r     93C46              6116      6116      RAM1      |
|                                                  24MHz|
+-+ BAT1 PC1              6116      6116      RAM1      |
  +-----------------------------------------------------+

ROM#1 & ROM#2 are 32pin sockets
ROM#7 is a 40pin socket
ROM#3 & ROM#4 at U27 & U25 are optional unpopulated 32pin sockets and overlap with ROM#6
ROM#6 is 29F1610MC flash rom

RAM1 are SEC KM681000BLG-7L RAM chips

1 SPEED DROP 1.06 is a WinBond W27C512
2 SPEED DROP 1.06 is a WinBond W27C512
5 SPEED DROP is ST 27C4001

***************************************************************************/

ROM_START( speeddrp )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_speed_drop_1.06.u21", 0x00000, 0x10000, CRC(ff4d0859) SHA1(abdb90d3498f64a9ac779f5fd66d313c1df3425b) )
	ROM_LOAD16_BYTE( "2_speed_drop_1.06.u20", 0x00001, 0x10000, CRC(a00cc120) SHA1(eb1e9a084aca18e71901ed599f4621c301bab43e) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mx29f1610amc.u26", 0x000000, 0x200000, CRC(baa0f728) SHA1(12f0e7689eb6555f86ac9a7272e8e119faa968e0) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_speed_drop", 0x00000, 0x80000, CRC(684bb8b5) SHA1(65276ce03da7be7275646f5a0d9d163eecb78190) ) // No chip location just "ROM#5" silkscreened under socket

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u6", 0x00, 0x80, CRC(6890534e) SHA1(a62893015e53c02551d57d0e1cce436b6df8d289) )
ROM_END

/***************************************************************************

Western Venture
(c) ASTRO

ASTRO V07 0709
ASTRO F01 2007-06-03
ASTRO ??? (V102?)
AD-65?
93C46 EEPROM

XTAL 24 MHz (near F01)
XTAL 18.432 MHz (near empty 44 pin PLCC socket and RS232?)

Push Button

ROM types:

EV29001TSC-70R
MX29F1610MC-10

***************************************************************************/

ROM_START( westvent )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_w.v.aa.02.d.bin", 0x00000, 0x20000, CRC(5e208192) SHA1(5a35a419fe95513b68423d4eb6c77fdd375667f3) )  // good?
	ROM_LOAD16_BYTE( "2_w.v.aa.02.d.bin", 0x00001, 0x20000, BAD_DUMP CRC(f56d3ead) SHA1(2cf8960eab221cfce1c7ac6a20d002c4b05d8cc6) ) // FIXED BITS (xxxxxx0xxxxxxxxx)

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD( "top.bin",    0x000000, 0x200000, CRC(75bbaae0) SHA1(ef35775dd481ff343df1ee071ccd52b024d084b7) )
	ROM_LOAD( "bottom.bin", 0x200000, 0x200000, CRC(e2dd58d5) SHA1(9ab881cfb2ee6cbc48aa28ba28529adb00803e44) )
	ROM_LOAD( "middle.bin", 0x400000, 0x200000, CRC(7348fd37) SHA1(b5ec0994afb5bceae5627c37f1b35c7abcfd2f0a) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5.bin", 0x00000, 0x80000, CRC(92dc09d1) SHA1(6b448b3372e78047d054c5e42fcfcff7f75ff9b9) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "at93c46.bin", 0x00, 0x80, CRC(fd961d46) SHA1(e238da65e8769575f17b4464fb00f5c4813bafab) )
ROM_END

/***************************************************************************

Win Win Bingo

ASTRO M1.2 PCB:
 Astro V06 0430 160pin PQFP ("ASTRO02" silkscreened under chip)
   Boards are known to have either Astro V06 0430 or Astro V07 0610
 Astro V102PX-006 at U10 (68K core, has direct connection to program roms)
 Astro F02 2005-09-17 socketted FPGA type chip (used for encryption?)
 OKI 6295 clone chip (AD-65 or U6295)

RAM:
 2 x HMI HM62C64P-70
 4 x HMI HM6116P-70
 2 x BSI BS62LV1023SC-70

EEPROM 93C46
Battery 3.6V
OSC    24MHz

PC1 is a push button for test mode
VR1 is for sound volume

      +---------+   +----------------------------+ +----+
  +---+Connector+---+   28 Pin Edge  Connector   +-+    |
  |                                                  VR1|
  |                    24MHz        +---------+         |
+-+          +-------+              |ROM#4 U30|         |
|            | Astro |   HM62C64P   +---------+         |
|            |  F2   | +----------+ +---------+         |
|8           |       | |ROM#2  U25| |ROM#3 U26|         |
|   ULN2003A +-------+ +----------+ +---------+         |
|L                                                      |
|i  ULN2003A +------+                                   |
|n           |Astro |      ROM#7*                 +----+|
|e           |V102PX|                             |6295||
|r           +------+                             +----+|
|                                    +----------+       |
|C                     +----------+  |          |   +---+
|o                     |ROM#1  U26|  |  Astro   |   |   |
|n                     +----------+  |  V06     |   | R |
|n                                   |  0430    |   | O |
|e                                   +----------+   | M |
|c                       HM62C64P                   | # |
|t                                               J1 | 5 |
|o            93C46                                 |   |
|r                        6116      6116      RAM1  +---+
|                                                       |
+-+ BAT1 PC1              6116      6116      RAM1      |
  +-----------------------------------------------------+

ROM#7 at U16 is an unpopulated 40pin socket
ROM#1 & ROM#2 are SYNCMOS F29C51001T
ROM#3 is a 29F1610MC flash rom
ROM#4 is a 29F1610MC flash rom (optionally populated based on game)
ROM#5 is a MX 27C4000PC-12

RAM1 are BSI BS62LV1023SC-70 RAM chips

J1 is an 2 pin connector, unknown purpose

***************************************************************************/

ROM_START( winbingo )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f29c51001t.u31", 0x00000, 0x20000, CRC(964cfdb5) SHA1(01109466e07f5e906be300bc69310171d34f2e6c) )
	ROM_LOAD16_BYTE( "f29c51001t.u25", 0x00001, 0x20000, CRC(4ebeec72) SHA1(c603265e6319cff94a0c75017a12c6d86787f906) )

	ROM_REGION( 0x400000, "sprites", ROMREGION_ERASE )
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, CRC(ad1f61e7) SHA1(845aa01d49c50bcadaed16d76c0dd9131a425b46) )
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, CRC(31613d99) SHA1(1c720f8d981c3e9cb9d9b3b27eb95e7f72ccfc93) )
//  ROM_LOAD( "mx29f1610mc.u30.bad.dump", 0x200000, 0x0a0000, BAD_DUMP CRC(6da439c5) SHA1(6afc0c800fe57b9b34ca317f4d1c040b11d3d988) )
//  U30 is a bad dump in this set, so use U30 from winbingoa (since U26 is the same too)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mx27c4000pc.u35", 0x00000, 0x80000, CRC(445d81c0) SHA1(cacb9c262740c31ea42f406e9f960a1edd1b3ead) )
ROM_END

ROM_START( winbingoa )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f29c51001t.u31", 0x00000, 0x20000, CRC(c33676c3) SHA1(9f5b7d05d187cf59948a572f80c55cb8fa1f656f) ) // sldh
	ROM_LOAD16_BYTE( "f29c51001t.u25", 0x00001, 0x20000, CRC(43c7b2d8) SHA1(16ee79c34b7c485dfccecdf3e0ae9f18f8a20150) ) // sldh

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, CRC(ad1f61e7) SHA1(845aa01d49c50bcadaed16d76c0dd9131a425b46) )
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, CRC(31613d99) SHA1(1c720f8d981c3e9cb9d9b3b27eb95e7f72ccfc93) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mx27c4000pc.u35", 0x00000, 0x80000, CRC(e48ed57d) SHA1(11995b90e70e010b292ba9db2da0af4ebf795c1a) ) // sldh
ROM_END

/***************************************************************************

Hacher (graphics hack of Win Win Bingo Ver. EN.01.6?)

ASTRO M1.2 PCB with Astro F02 2005-02-18

***************************************************************************/

ROM_START( hacher )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "w-w-en-01-6.rom1", 0x00000, 0x20000, CRC(994acd32) SHA1(ee137ca96f4e2d22f2bae32051bbf2bd487e8c5a) )
	ROM_LOAD16_BYTE( "w-w-en-01-6.rom2", 0x00001, 0x20000, CRC(b45c3f64) SHA1(c8f26fc3f9e2c46d8083d249f79ff8a3d47b67d0) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "wb3.bin", 0x000000, 0x200000, CRC(d97e5056) SHA1(662fefc2dcac31023fa063fbf891b05a139e48d8) )
	ROM_LOAD( "wb4.bin", 0x200000, 0x200000, BAD_DUMP CRC(5cd7dcd9) SHA1(69e5fd0c8c5c14938c02f4f50e5b16fc0fbff7e4) ) // FIXED BITS (xxxxxxxxxxxxx1xx)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "win-win-bingo.ic5", 0x00000, 0x80000, CRC(445d81c0) SHA1(cacb9c262740c31ea42f406e9f960a1edd1b3ead) ) // = mx27c4000pc.u35 winbingo

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u13", 0x00, 0x80, CRC(e097ae26) SHA1(90a670b5f1931e892d0a134aa8bf7d36d6222dcb) )
ROM_END


/***************************************************************************

Zoo by Astro

ASTRO M1.1 PCB:
 Astro V06 0430 160pin PQFP ("ASTRO02" silkscreened under chip)
 Astro V102PX-005 T042652846 at U10 (68K core, has direct connection to program roms)
 Astro F02 2005-02-18 socketted FPGA type chip (used for encryption?)
 OKI 6295 clone chip (AD-65 or U6295)

RAM:
 2 x KTC KT76C88-70LL
 4 x HM6116L-70
 2 x BSI BS62LV1025SC-70

EEPROM 93C46
Battery 3.6V
OSC    26.824MHz

PC1 is a push button for test mode
VR1 is for sound volume


ZOO Z0.02.D at both U25 & U26 are Winbond W27C512 roms and are program code
5 ZOO is a MX 27C4000 rom and is the sample rom
29F1610mc at U26 (yes "U26" is present twice on the PCB) are the graphics

      +---------+   +----------------------------+ +----+
  +---+Connector+---+   28 Pin Edge  Connector   +-+    |
  |                                                  VR1|
  |                   26.824MHz     +---------+         |
+-+          +-------+              |ROM#4 U30|         |
|            | Astro |   KT76C88    +---------+         |
|            |  F2   | +----------+ +---------+         |
|8           |       | |ROM#2  U25| |ROM#3 U26|         |
|   ULN2003A +-------+ +----------+ +---------+         |
|L                                                      |
|i  ULN2003A +------+                                   |
|n           |Astro |      ROM#7*                 +----+|
|e           |V102PX|                             |6295||
|r           +------+                             +----+|
|                                    +----------+       |
|C                     +----------+  |          |   +---+
|o                     |ROM#1  U26|  |  Astro   |   |   |
|n                     +----------+  |  V06     |   | R |
|n                                   |  0430    |   | O |
|e                                   +----------+   | M |
|c                       KT76C88                    | # |
|t                                               J1 | 5 |
|o            93C46                                 |   |
|r                        6116      6116      RAM1  +---+
|                                                       |
+-+ BAT1 PC1              6116      6116      RAM1      |
  +-----------------------------------------------------+

ROM#7 at U16 is an unpopulated 40pin socket
ROM#3 is a 29F1610MC flash rom
ROM#4 is a 29F1610MC flash rom (optionally populated based on game)

RAM1 are BSI BS62LV1025SC-70 RAM chips

J1 is an 2 pin connector, unknown purpose

***************************************************************************/

ROM_START( zoo )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "zoo_zo.02.d.u25", 0x00000, 0x10000, CRC(8566aa21) SHA1(319192e2074f3bdda6001d8e9a4b97e98826d7ce) )
	ROM_LOAD16_BYTE( "zoo_zo.02.d.u26", 0x00001, 0x10000, CRC(1a3be45a) SHA1(877be4c9e8d5e7c4644e7bcb9a6729443ed772a4) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "29f1610mc.u26", 0x000000, 0x200000, CRC(f5cfd915) SHA1(ec869b47d0762102509dcfc1349d94340037fad5) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_zoo", 0x00000, 0x80000, CRC(b0c9f7aa) SHA1(99345ba0f8da3907f26c9bd29d70135f3ab7cd60) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "zoo_93c46", 0x00, 0x80, CRC(0053fcc4) SHA1(e67a495f9586dd3946f79d50506fba1ae913f6ec) )
ROM_END

/***************************************************************************

Stone Age
(c) ASTRO

PCB ID  CPU                Video        Chips
L1      ASTRO V102PX-012?  ASTRO V05x2  ASTRO F02 2004-09-04

      +---------+   +----------------------------+ +-------------+
  +---+Connector+---+   28 Pin Edge  Connector   +-+             |
  |                                                   VR1        |
  |   93c46                                                      |
+-+          +-------+                          +----+ +-------+ |
|            | Astro |   HM62C64P               |6295| |ROM #7 | |
|            |  F2   | +----------+             +----+ +-------+ |
|8           |       | |ROM#2  U19|                              |
|   ULN2003A +-------+ +----------+      +--+ +--+ +--+ +--+     |
|L                                       | R| | R| | R| | R|     |
|i  ULN2003A +------+                    | o| | o| | o| | o|     |
|n           |Astro |      ROM#8*        | m| | m| | m| | m|     |
|e           |V102PX|                    | #| | #| | #| | #|     |
|r           +------+                    | 3| | 4| | 5| | 6|     |
|                                        +--+ +--+ +--+ +--+     |
|                                                                |
|                                      +----------+  +----------+|
|C                     +----------+    |          |  |          ||
|o                     |ROM#1  U21|    |  Astro   |  |  Astro   ||
|n                     +----------+    |  V05     |  |  V05     ||
|n                                     |  0424    |  |  0424    ||
|e                                     +----------+  +----------+|
|c                       HM62C64P                                |
|t                                                               |
|o        ASTRO0312     ASTRO  120Mhz                            |
|r                                    6116                       |
|                  6116      6116     6116   RAM1       RAM1     |
|                                                                |
+-+ BAT1 PC1  VGA  6116      6116              RAM1     RAM1     |
  +--------------------------------------------------------------+

ROM#7 at U16 is an unpopulated 40pin socket
ROM#1 & ROM#2 are MX 26c10000VPC-10
ROM#3,4,5,6 are a 29F1610ML flash rom
ROM#7 is a MX 27C4000PC-12

RAM1 are SEC KM681000BLG-7 RAM chips
PC1 is reset
Video output only on VGA connector, video signals ARE VGA

***************************************************************************/

ROM_START( astoneag )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1-s-a-eng-03-a.rom1", 0x00000, 0x20000, CRC(5e600713) SHA1(48ac0a52f90b972b77064e9e59711082aa95c654) )
	ROM_LOAD16_BYTE( "2-s-a-eng-03-a.rom2", 0x00001, 0x20000, CRC(488e355e) SHA1(6550292cae7eda95a24e1982e869540464b1fcdd) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD( "29f1610.rom3", 0x000000, 0x200000, CRC(8d4e66f0) SHA1(744f83b35684aa6653b0d93b303f2914cd0250ba) )
	ROM_LOAD( "29f1610.rom4", 0x200000, 0x200000, CRC(1affd8db) SHA1(2523f156933c61d36b6646944b5da874f8424864) )
	ROM_LOAD( "29f1610.rom5", 0x400000, 0x200000, CRC(2b77d827) SHA1(b082254e1c8a7945e2a406b1b937a763b30cb496) )
	ROM_LOAD( "29f1610.rom6", 0x600000, 0x200000, CRC(eb8ee0e7) SHA1(c6c973460ca96b54151f7523f6afc0184b8fbd40) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5-s-a-eng-03-a.rom7", 0x00000, 0x80000, CRC(1b13b0c2) SHA1(d6d8c8070ba146b444958fa0b896cebc12b32f5c) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.ic99", 0x0000, 0x0080, CRC(2fd85a9b) SHA1(3240e40debf5af15f08072b76d6910808d3d282f) )
ROM_END

/***************************************************************************

Dino Dino
Astro Corp.

PCB Layout
----------

ASTRO T-3802A PCB with ASTRO F02 2003-03-12
|--------------------------------------------|
|        |------|                    TDA2003 |
|ULN2003 |ASTRO |   V62C51864            VOL |
|        |F02   |            ROM4            |
|        |------|   ROM2                     |
|ULN2003                     ROM3      M6295 |
|        |-------|                           |
|8       |ASTRO  |                           |
|L       |V102PX |                       ROM5|
|I       |-010   |                           |
|N       |-------|            |------|       |
|E                            |ASTRO |       |
|R                  ROM1      |V05   |       |
|                             |------|       |
|                   V62C51864                |
|                                       24MHz|
|        93C46      6116 6116    HM628128    |
|        SW1        6116 6116    HM628128    |
|--------------------------------------------|

***************************************************************************/

ROM_START( dinodino )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dd_rom1.u20", 0x00000, 0x20000, CRC(056613dd) SHA1(e8481177b1dacda222fe4fae2b50841ddb0c87ba) )
	ROM_LOAD16_BYTE( "dd_rom2.u19", 0x00001, 0x20000, CRC(575519c5) SHA1(249fe33b6ea0bc154125a988315f571a30b9375c) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "dd_rom3.u26", 0x000000, 0x200000, CRC(47c95b43) SHA1(43e9a13c38f2f7d13dd4dcb105c65e43b18ccdbf) )
	ROM_LOAD( "dd_rom4.u24", 0x200000, 0x200000, CRC(2cf4be21) SHA1(831d7d125c4161b42b017a69fc05e30a51172620) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "dd_rom5.u33", 0x00000, 0x80000, CRC(482e456a) SHA1(c7111522383c4e1fd98b0f759153be98dcbe06c1) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u10", 0x0000, 0x0080, CRC(6769bfb8) SHA1(bf6b905805c2c61a89fbc4c046b23069431e4709) )
ROM_END


DRIVER_INIT_MEMBER(astrocorp_state,showhand)
{
#if 0
	UINT16 *rom = (UINT16*)memregion("maincpu")->base();

	rom[0x0a1a/2] = 0x6000; // hopper jam

	rom[0x1494/2] = 0x4e71; // enable full test mode
	rom[0x1496/2] = 0x4e71; // ""
	rom[0x1498/2] = 0x4e71; // ""

	rom[0x12f6/2] = 0x6000; // rom error
	rom[0x4916/2] = 0x6000; // rom error
#endif
}

DRIVER_INIT_MEMBER(astrocorp_state,showhanc)
{
#if 0
	UINT16 *rom = (UINT16*)memregion("maincpu")->base();

	rom[0x14d4/2] = 0x4e71; // enable full test mode
	rom[0x14d6/2] = 0x4e71; // ""
	rom[0x14d8/2] = 0x4e71; // ""

	rom[0x139c/2] = 0x6000; // rom error
#endif
}

DRIVER_INIT_MEMBER(astrocorp_state,astoneag)
{
#if 0
	UINT16 *rom = (UINT16*)memregion("maincpu")->base();
	UINT16 x;
	int i;

	for (i = 0x25100/2; i < 0x25200/2; i++)
	{
		x = 0x0000;
		if (  (i & 0x0001) )                    x |= 0x0200;
		if (  (i & 0x0004) && !(i & 0x0001) )   x |= 0x0080;
		if (  (i & 0x0040) ||  (i & 0x0001) )   x |= 0x0040;
		if (  (i & 0x0010) && !(i & 0x0001) )   x |= 0x0020;
		if ( !(i & 0x0020) ||  (i & 0x0001) )   x |= 0x0010;
		if (  (i & 0x0002) ||  (i & 0x0001) )   x |= 0x0008;
		if (  (i & 0x0008) && !(i & 0x0001) )   x |= 0x0004;
		if ( !(i & 0x0040) && !(i & 0x0001) )   x |= 0x0002;
		if (  (i & 0x0040) && !(i & 0x0001) )   x |= 0x0001;
		rom[i] ^= x;
	}

/*
    for (i = 0x25300/2; i < 0x25400/2; i++)
    {
        x = 0x1300;
        rom[i] ^= x;
    }
*/

	for (i = 0x25400/2; i < 0x25500/2; i++)
	{
		x = 0x4200;
		if (  (i & 0x0001) )                    x |= 0x0400;
		if (  (i & 0x0020) && !(i & 0x0001) )   x |= 0x0080;
		if ( !(i & 0x0010) ||  (i & 0x0001) )   x |= 0x0040;
		if (  (i & 0x0040) && !(i & 0x0001) )   x |= 0x0020;
		if ( !(i & 0x0004) ||  (i & 0x0001) )   x |= 0x0010;
		if ( !(i & 0x0040) && !(i & 0x0001) )   x |= 0x0004;
		if (  (i & 0x0008) && !(i & 0x0001) )   x |= 0x0002;
		if (  (i & 0x0002) ||  (i & 0x0001) )   x |= 0x0001;
		rom[i] ^= x;
	}

	for (i = 0x25500/2; i < 0x25600/2; i++)
	{
		x = 0x4200;
		if (  (i & 0x0001) )                    x |= 0x0400;
		if (  (i & 0x0010) && !(i & 0x0001) )   x |= 0x0080;
		if (  (i & 0x0040) && !(i & 0x0001) )   x |= 0x0040;
		if ( !(i & 0x0002) && !(i & 0x0001) )   x |= 0x0020;
		if ( !(i & 0x0040) && !(i & 0x0001) )   x |= 0x0010;
		if (  (i & 0x0008) && !(i & 0x0001) )   x |= 0x0008;
		if (  (i & 0x0020) && !(i & 0x0001) )   x |= 0x0004;
		if (  (i & 0x0004) && !(i & 0x0001) )   x |= 0x0002;
		if (  (i & 0x0001) )                    x |= 0x0001;
		rom[i] ^= x;
	}

/*
    for (i = 0x25700/2; i < 0x25800/2; i++)
    {
        x = 0x6800;
        if ( !(i & 0x0001) )                    x |= 0x8000;

        if ( !(i & 0x0040) || ((i & 0x0001) || !(i & 0x0001)) )                 x |= 0x0100;

        rom[i] ^= x;
    }
*/

	for (i = 0x25800/2; i < 0x25900/2; i++)
	{
		x = 0x8300;
		if (  (i & 0x0040) ||  (i & 0x0001) )   x |= 0x2000;
		if (  (i & 0x0002) ||  (i & 0x0001) )   x |= 0x0080;
		if ( !(i & 0x0040) && !(i & 0x0001) )   x |= 0x0040;
		if (  (i & 0x0020) && !(i & 0x0001) )   x |= 0x0020;
		if ( !(i & 0x0004) ||  (i & 0x0001) )   x |= 0x0010;
		if ( !(i & 0x0040) && !(i & 0x0001) )   x |= 0x0008;
		if (  (i & 0x0010) && !(i & 0x0001) )   x |= 0x0004;
		if (  (i & 0x0008) && !(i & 0x0001) )   x |= 0x0002;
		if ( !(i & 0x0040) && !(i & 0x0001) )   x |= 0x0001;
		rom[i] ^= x;
	}

//  for (i = 0x25900/2; i < 0x25a00/2; i++)

	for (i = 0x25c00/2; i < 0x25d00/2; i++)
	{
		// changed from 25400
//      x = 0x4200;
		x = 0x4000;
//      if (  (i & 0x0001) )                    x |= 0x0400;
		if (  (i & 0x0020) && !(i & 0x0001) )   x |= 0x0080;
		if ( !(i & 0x0010) ||  (i & 0x0001) )   x |= 0x0040;
		if (  (i & 0x0040) && !(i & 0x0001) )   x |= 0x0020;
		if ( !(i & 0x0004) ||  (i & 0x0001) )   x |= 0x0010;
		if ( !(i & 0x0040) && !(i & 0x0001) )   x |= 0x0004;
		if (  (i & 0x0008) && !(i & 0x0001) )   x |= 0x0002;
		if (  (i & 0x0002) ||  (i & 0x0001) )   x |= 0x0001;
		rom[i] ^= x;
	}

/*
    for (i = 0x25d00/2; i < 0x25e00/2; i++)
    {
        x = 0x4000;
        if ( !(i & 0x0040) )                                    x |= 0x0800;

        if ( !(i & 0x0040) && !(i & 0x0001) )   x |= 0x0100;    // almost!!

        if ( ((i & 0x0040)&&((i & 0x0020)||(i & 0x0010))) || !(i & 0x0001) )    x |= 0x0200;    // almost!!
        if ( (!(i & 0x0040) || !(i & 0x0008)) && !(i & 0x0001) )    x |= 0x0008;
        if (  (i & 0x0040) || !(i & 0x0020) ||  (i & 0x0001) )  x |= 0x0001;    // almost!!
        rom[i] ^= x;
    }
*/

/*
    for (i = 0x25e00/2; i < 0x25f00/2; i++)
    {
        x = 0xa600;

        if (  (i & 0x0040) &&  (i & 0x0001) )   x |= 0x4000;
        if (  (i & 0x0040) &&  (i & 0x0001) )   x |= 0x0800;
        if ( !(i & 0x0001) )                    x |= 0x0100;

        if ( (  (i & 0x0040) &&  (i & 0x0008) && !(i & 0x0001)) ||
             ( !(i & 0x0040) && ((i & 0x0004) ^ (i & 0x0002)) && !(i & 0x0001) ) )  x |= 0x0002;    // almost!!

        if ( !(i & 0x0040) || !(i & 0x0002) ||  (i & 0x0001) )  x |= 0x0001;
        rom[i] ^= x;
    }
*/

	for (i = 0x26f00/2; i < 0x27000/2; i++)
	{
		x = 0xb94c;
		rom[i] ^= x;
	}

	for (i = 0x27000/2; i < 0x27100/2; i++)
	{
		x = 0x5f10;
		rom[i] ^= x;
	}

#endif
}

GAME( 2000,  showhand,  0,        showhand, showhand, astrocorp_state, showhand, ROT0, "Astro Corp.",        "Show Hand (Italy)",                MACHINE_SUPPORTS_SAVE )
GAME( 2000,  showhanc,  showhand, showhanc, showhanc, astrocorp_state, showhanc, ROT0, "Astro Corp.",        "Wang Pai Dui Jue (China)",         MACHINE_SUPPORTS_SAVE )
GAME( 2002,  skilldrp,  0,        skilldrp, skilldrp, driver_device,   0,        ROT0, "Astro Corp.",        "Skill Drop Georgia (Ver. G1.0S)",  MACHINE_SUPPORTS_SAVE )
GAME( 2003,  speeddrp,  0,        speeddrp, skilldrp, driver_device,   0,        ROT0, "Astro Corp.",        "Speed Drop (Ver. 1.06)",           MACHINE_SUPPORTS_SAVE )

// Encrypted games (not working):
GAME( 2003?, dinodino,  0,        skilldrp, skilldrp, driver_device,   0,        ROT0, "Astro Corp.",        "Dino Dino",                        MACHINE_NOT_WORKING )
GAME( 2004?, astoneag,  0,        skilldrp, skilldrp, astrocorp_state, astoneag, ROT0, "Astro Corp.",        "Stone Age (Astro, Ver. ENG.03.A)", MACHINE_NOT_WORKING )
GAME( 2005?, winbingo,  0,        skilldrp, skilldrp, driver_device,   0,        ROT0, "Astro Corp.",        "Win Win Bingo (set 1)",            MACHINE_NOT_WORKING )
GAME( 2005?, winbingoa, winbingo, skilldrp, skilldrp, driver_device,   0,        ROT0, "Astro Corp.",        "Win Win Bingo (set 2)",            MACHINE_NOT_WORKING )
GAME( 2005?, hacher,    winbingo, skilldrp, skilldrp, driver_device,   0,        ROT0, "bootleg (Gametron)", "Hacher (hack of Win Win Bingo)",   MACHINE_NOT_WORKING )
GAME( 2005?, zoo,       0,        showhand, showhand, driver_device,   0,        ROT0, "Astro Corp.",        "Zoo (Ver. ZO.02.D)",               MACHINE_NOT_WORKING )
GAME( 2007?, westvent,  0,        skilldrp, skilldrp, driver_device,   0,        ROT0, "Astro Corp.",        "Western Venture (Ver. AA.02.D)",   MACHINE_NOT_WORKING )
