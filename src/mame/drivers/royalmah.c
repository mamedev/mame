// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Nicola Salmoria, Luca Elia
/****************************************************************************

Royal Mahjong (c) 1981 Nichibutsu
and many other Dyna/Dynax games running in similar bare-bones hardware

driver by Zsolt Vasvari, Nicola Salmoria, Luca Elia

CPU:    Z80 or TLCS-90
Video:  Framebuffer
Sound:  AY-3-8910 (or YM2149)
OSC:    18.432MHz and 8MHz

-----------------------------------------------------------------------------------------------------------------------
Year + Game               Board(s)               CPU      Company            Notes
-----------------------------------------------------------------------------------------------------------------------
81  Royal Mahjong                                Z80      Nichibutsu
81? Open Mahjong                                 Z80      Sapporo Mechanic
82  Royal Mahjong         ? + FRM-03             Z80      Falcon             bootleg
83  Janyou Part II                               Z80        Cosmo Denshi
84? Jan Oh                FRM-00?                Z80      Toaplan            Incomplete program roms
86  Ippatsu Gyakuten                             Z80      Public/Paradais
86  Don Den Mahjong       D039198L-0             Z80      Dyna Electronics
86  Watashiha Suzumechan  D8803288L1-0           Z80      Dyna Electronics
87  Mahjong Diplomat      D0706088L1-0           Z80      Dynax
87  Mahjong Studio 101    D1708228L1             Z80      Dynax
87  Tonton                D0908288L1-0           Z80      Dynax
88  Almond Pinky          D1401128L-0 + RM-1D    Z80      Dynax
89  Mahjong Shinkirou     D210301BL2 + FRM-00?   TLCS-90  Dynax
89  Mahjong Derringer     D2203018L              Z80      Dynax              Larger palette
90  Mahjong If..?         D2909278L              TLCS-90  Dynax              Larger palette
91  Mahjong Vegas         D5011308L1 + FRM-00    TLCS-90  Dynax              Undumped internal rom (mjvegas set)
92  Mahjong Cafe Time     D6310128L1-1           TLCS-90  Dynax              Larger palette, RTC
93  Mahjong Cafe Doll     D76052208L-2           TLCS-90  Dynax              Larger palette, RTC, Undumped internal rom
95  Mahjong Tensinhai     D10010318L1            TLCS-90  Dynax              Larger palette, RTC
96  Janputer '96          NS503X0727             Z80      Dynax              Larger palette, RTC
97  Janputer Special      CS166P008 + NS5110207  Z80      Dynax              Larger palette, RTC
99  Mahjong Cafe Break    NS528-9812             TLCS-90  Nakanihon / Dynax  Undumped internal rom
99  Mahjong Cafe Paradise ? + Techno Top Limited TLCS-90  Techno-Top?        Undumped internal rom
-----------------------------------------------------------------------------------------------------------------------

TODO:

- dip switches and inputs in dondenmj, suzume, mjderngr...

- there's something fishy with the bank switching in tontonb/mjdiplob

- majs101b: service mode doesn't work

- mjtensin: random crashes, interrupts related


Stephh's notes (based on the games Z80 code and some tests) :

1) 'royalmah'

  - COIN1 doesn't work correctly, the screen goes black instead of showing the
    credits, and you can start a game but the "phantom" credit is not subtracted;
    with NVRAM support, this means the game would always boot to a black screen.
  - The doesn't seem to be any possibility to play a 2 players game
    (but the inputs are mapped so you can test them in the "test mode").
    P1 IN4 doesn't seem to be needed outside the "test mode" either.

2) 'tontonb'

  - The doesn't seem to be any possibility to play a 2 players game
    (but the inputs are mapped so you can test them in the "test mode")
    P1 IN4 doesn't seem to be needed outside the "test mode" either.

  - I've DELIBERATELY mapped DSW3 before DSW2 to try to spot the common
    things with the other Dynax mahjong games ! Please don't change this !

  - When "Special Combinations" Dip Switch is ON, there is a marker in
    front of a random combination. It's value is *2 then.

3) 'mjdiplob'

  - The doesn't seem to be any possibility to play a 2 players game
    (but the inputs are mapped so you can test them in the "test mode")
    P1 IN4 doesn't seem to be needed outside the "test mode" either.

  - When "Special Combinations" Dip Switch is ON, there is a marker in
    front of a random combination. It's value remains *1 though.
    Could it be a leftover from another game ('tontonb' for exemple) ?

- janptr96, janptrsp: in service mode press in sequence N,Ron,Ron,N to access some
  hidden options. (thanks bnathan)

2009-03-25 FP: fixed verified DSW and default settings for mjclub (thanks to
    translation from manual by Yasu)

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/tlcs90/tlcs90.h"
#include "machine/msm6242.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "machine/nvram.h"


class royalmah_state : public driver_device
{
public:
	royalmah_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_videoram(*this, "videoram"),
		m_audiocpu(*this, "audiocpu"),
		m_rtc(*this, "rtc") { }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_videoram;
	optional_device<cpu_device> m_audiocpu;
	optional_device<msm6242_device> m_rtc;
	UINT8 m_input_port_select;
	UINT8 m_dsw_select;
	UINT8 m_rombank;
	int m_palette_base;
	UINT8 *m_janptr96_nvram;
	UINT8 m_suzume_bank;
	UINT8 m_gfx_adr_l;
	UINT8 m_gfx_adr_m;
	UINT8 m_gfx_adr_h;
	UINT32 m_gfx_adr;
	UINT8 m_gfxdata0;
	UINT8 m_gfxdata1;
	UINT8 m_jansou_colortable[16];
	UINT8 m_mjifb_rom_enable;
	UINT8 m_flip_screen;

	DECLARE_WRITE8_MEMBER(royalmah_palbank_w);
	DECLARE_WRITE8_MEMBER(royalmah_rom_w);
	DECLARE_WRITE8_MEMBER(tahjong_bank_w);
	DECLARE_WRITE8_MEMBER(mjderngr_coin_w);
	DECLARE_WRITE8_MEMBER(mjderngr_palbank_w);
	DECLARE_WRITE8_MEMBER(input_port_select_w);
	DECLARE_READ8_MEMBER(majs101b_dsw_r);
	DECLARE_READ8_MEMBER(suzume_dsw_r);
	DECLARE_WRITE8_MEMBER(suzume_bank_w);
	DECLARE_WRITE8_MEMBER(mjapinky_bank_w);
	DECLARE_WRITE8_MEMBER(mjapinky_palbank_w);
	DECLARE_READ8_MEMBER(mjapinky_dsw_r);
	DECLARE_WRITE8_MEMBER(tontonb_bank_w);
	DECLARE_WRITE8_MEMBER(dynax_bank_w);
	DECLARE_READ8_MEMBER(daisyari_dsw_r);
	DECLARE_WRITE8_MEMBER(daisyari_bank_w);
	DECLARE_READ8_MEMBER(mjclub_dsw_r);
	DECLARE_WRITE8_MEMBER(mjclub_bank_w);
	DECLARE_WRITE8_MEMBER(jansou_dsw_sel_w);
	DECLARE_READ8_MEMBER(jansou_dsw_r);
	DECLARE_WRITE8_MEMBER(jansou_colortable_w);
	DECLARE_WRITE8_MEMBER(jansou_6400_w);
	DECLARE_WRITE8_MEMBER(jansou_6401_w);
	DECLARE_WRITE8_MEMBER(jansou_6402_w);
	DECLARE_READ8_MEMBER(jansou_6403_r);
	DECLARE_READ8_MEMBER(jansou_6404_r);
	DECLARE_READ8_MEMBER(jansou_6405_r);
	DECLARE_WRITE8_MEMBER(jansou_sound_w);
	DECLARE_WRITE8_MEMBER(janptr96_dswsel_w);
	DECLARE_READ8_MEMBER(janptr96_dswsel_r);
	DECLARE_READ8_MEMBER(janptr96_dsw_r);
	DECLARE_WRITE8_MEMBER(janptr96_rombank_w);
	DECLARE_WRITE8_MEMBER(janptr96_rambank_w);
	DECLARE_READ8_MEMBER(janptr96_unknown_r);
	DECLARE_WRITE8_MEMBER(janptr96_coin_counter_w);
	DECLARE_WRITE8_MEMBER(mjifb_coin_counter_w);
	DECLARE_READ8_MEMBER(mjifb_rom_io_r);
	DECLARE_WRITE8_MEMBER(mjifb_rom_io_w);
	DECLARE_WRITE8_MEMBER(mjifb_videoram_w);
	DECLARE_READ8_MEMBER(mjifb_p3_r);
	DECLARE_READ8_MEMBER(mjifb_p5_r);
	DECLARE_READ8_MEMBER(mjifb_p6_r);
	DECLARE_READ8_MEMBER(mjifb_p7_r);
	DECLARE_READ8_MEMBER(mjifb_p8_r);
	DECLARE_WRITE8_MEMBER(mjifb_p3_w);
	DECLARE_WRITE8_MEMBER(mjifb_p4_w);
	DECLARE_WRITE8_MEMBER(mjifb_p8_w);
	DECLARE_READ8_MEMBER(mjdejavu_rom_io_r);
	DECLARE_WRITE8_MEMBER(mjdejavu_rom_io_w);
	DECLARE_READ8_MEMBER(mjtensin_p3_r);
	DECLARE_WRITE8_MEMBER(mjtensin_p4_w);
	DECLARE_WRITE8_MEMBER(mjtensin_6ff3_w);
	DECLARE_WRITE8_MEMBER(cafetime_p4_w);
	DECLARE_WRITE8_MEMBER(cafetime_p3_w);
	DECLARE_WRITE8_MEMBER(cafetime_dsw_w);
	DECLARE_READ8_MEMBER(cafetime_dsw_r);
	DECLARE_READ8_MEMBER(cafetime_7fe4_r);
	DECLARE_WRITE8_MEMBER(cafetime_7fe3_w);
	DECLARE_WRITE8_MEMBER(mjvegasa_p4_w);
	DECLARE_WRITE8_MEMBER(mjvegasa_p3_w);
	DECLARE_WRITE8_MEMBER(mjvegasa_rombank_w);
	DECLARE_READ8_MEMBER(mjvegasa_rom_io_r);
	DECLARE_WRITE8_MEMBER(mjvegasa_rom_io_w);
	DECLARE_WRITE8_MEMBER(mjvegasa_coin_counter_w);
	DECLARE_WRITE8_MEMBER(mjvegasa_12400_w);
	DECLARE_READ8_MEMBER(mjvegasa_12500_r);
	DECLARE_READ8_MEMBER(royalmah_player_1_port_r);
	DECLARE_READ8_MEMBER(royalmah_player_2_port_r);
	DECLARE_WRITE_LINE_MEMBER(janptr96_rtc_irq);
	DECLARE_WRITE_LINE_MEMBER(mjtensin_rtc_irq);
	DECLARE_DRIVER_INIT(janptr96);
	DECLARE_DRIVER_INIT(ippatsu);
	DECLARE_PALETTE_INIT(royalmah);
	DECLARE_PALETTE_INIT(mjderngr);
	UINT32 screen_update_royalmah(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(suzume_irq);
	INTERRUPT_GEN_MEMBER(mjtensin_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(janptr96_interrupt);
	void mjtensin_update_rombank();
	void cafetime_update_rombank();
};




PALETTE_INIT_MEMBER(royalmah_state, royalmah)
{
	offs_t i;
	const UINT8 *prom = memregion("proms")->base();
	int len = memregion("proms")->bytes();

	for (i = 0; i < len; i++)
	{
		UINT8 bit0, bit1, bit2, r, g, b;

		UINT8 data = prom[i];

		/* red component */
		bit0 = (data >> 0) & 0x01;
		bit1 = (data >> 1) & 0x01;
		bit2 = (data >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (data >> 3) & 0x01;
		bit1 = (data >> 4) & 0x01;
		bit2 = (data >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (data >> 6) & 0x01;
		bit2 = (data >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, r,g,b);
	}
}

PALETTE_INIT_MEMBER(royalmah_state,mjderngr)
{
	offs_t i;
	const UINT8 *prom = memregion("proms")->base();
	int len = memregion("proms")->bytes();

	for (i = 0; i < len / 2; i++)
	{
		UINT16 data = (prom[i] << 8) | prom[i + 0x200];

		/* the bits are in reverse order */
		UINT8 r = BITSWAP8((data >>  0) & 0x1f,7,6,5,0,1,2,3,4 );
		UINT8 g = BITSWAP8((data >>  5) & 0x1f,7,6,5,0,1,2,3,4 );
		UINT8 b = BITSWAP8((data >> 10) & 0x1f,7,6,5,0,1,2,3,4 );

		palette.set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}


WRITE8_MEMBER(royalmah_state::royalmah_palbank_w)
{
	/* bit 1 = coin counter */
	coin_counter_w(machine(), 0,data & 2);

	/* bit 2 = flip screen */
	m_flip_screen = (data & 4) >> 2;

	/* bit 3 = palette bank */
	m_palette_base = (data >> 3) & 0x01;
}


WRITE8_MEMBER(royalmah_state::mjderngr_coin_w)
{
	/* bit 1 = coin counter */
	coin_counter_w(machine(), 0,data & 2);

	/* bit 2 = flip screen */
	m_flip_screen = (data & 4) >> 2;
}


WRITE8_MEMBER(royalmah_state::mjderngr_palbank_w)
{
	m_palette_base = data;
}


UINT32 royalmah_state::screen_update_royalmah(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;

	offs_t offs;

	for (offs = 0; offs < 0x4000; offs++)
	{
		int i;

		UINT8 data1 = videoram[offs + 0x0000];
		UINT8 data2 = videoram[offs + 0x4000];

		UINT8 y = (m_flip_screen) ? 255 - (offs >> 6) : (offs >> 6);
		UINT8 x = (m_flip_screen) ? 255 - (offs << 2) : (offs << 2);

		for (i = 0; i < 4; i++)
		{
			UINT8 pen = ((data2 >> 1) & 0x08) | ((data2 << 2) & 0x04) | ((data1 >> 3) & 0x02) | ((data1 >> 0) & 0x01);

			bitmap.pix16(y, x) = (m_palette_base << 4) | pen;

			x = (m_flip_screen) ? x - 1 : x + 1;
			data1 = data1 >> 1;
			data2 = data2 >> 1;
		}
	}

	return 0;
}





WRITE8_MEMBER(royalmah_state::royalmah_rom_w)
{
	/* using this handler will avoid all the entries in the error log that are the result of
	   the RLD and RRD instructions this games uses to print text on the screen */
}


WRITE8_MEMBER(royalmah_state::input_port_select_w)
{
	m_input_port_select = data;
}

READ8_MEMBER(royalmah_state::royalmah_player_1_port_r)
{
	int ret = (ioport("KEY0")->read() & 0xc0) | 0x3f;

	if ((m_input_port_select & 0x01) == 0)  ret &= ioport("KEY0")->read();
	if ((m_input_port_select & 0x02) == 0)  ret &= ioport("KEY1")->read();
	if ((m_input_port_select & 0x04) == 0)  ret &= ioport("KEY2")->read();
	if ((m_input_port_select & 0x08) == 0)  ret &= ioport("KEY3")->read();
	if ((m_input_port_select & 0x10) == 0)  ret &= ioport("KEY4")->read();

	return ret;
}

READ8_MEMBER(royalmah_state::royalmah_player_2_port_r)
{
	int ret = (ioport("KEY5")->read() & 0xc0) | 0x3f;

	if ((m_input_port_select & 0x01) == 0)  ret &= ioport("KEY5")->read();
	if ((m_input_port_select & 0x02) == 0)  ret &= ioport("KEY6")->read();
	if ((m_input_port_select & 0x04) == 0)  ret &= ioport("KEY7")->read();
	if ((m_input_port_select & 0x08) == 0)  ret &= ioport("KEY8")->read();
	if ((m_input_port_select & 0x10) == 0)  ret &= ioport("KEY9")->read();

	return ret;
}



READ8_MEMBER(royalmah_state::majs101b_dsw_r)
{
	switch (m_dsw_select)
	{
		case 0x00: return ioport("DSW3")->read();   /* DSW3 */
		case 0x20: return ioport("DSW4")->read();   /* DSW4 */
		case 0x40: return ioport("DSW2")->read();   /* DSW2 */
	}
	return 0;
}



READ8_MEMBER(royalmah_state::suzume_dsw_r)
{
	if (m_suzume_bank & 0x40)
	{
		return m_suzume_bank;
	}
	else
	{
		switch (m_suzume_bank)
		{
			case 0x08: return ioport("DSW4")->read();   /* DSW4 */
			case 0x10: return ioport("DSW3")->read();   /* DSW3 */
			case 0x18: return ioport("DSW2")->read();   /* DSW2 */
		}
		return 0;
	}
}

WRITE8_MEMBER(royalmah_state::tahjong_bank_w)
{
	UINT8 *rom = memregion("maincpu")->base();
	int address;

logerror("%04x: bank %02x\n",space.device().safe_pc(),data);

	data &= 0x01;

	address = 0x10000 + data * 0x4000;

	membank("bank1")->set_base(&rom[address]);
}


WRITE8_MEMBER(royalmah_state::suzume_bank_w)
{
	UINT8 *rom = memregion("maincpu")->base();
	int address;

	m_suzume_bank = data;

logerror("%04x: bank %02x\n",space.device().safe_pc(),data);

	/* bits 6, 4 and 3 used for something input related? */

	address = 0x10000 + (data & 0x07) * 0x8000;
	membank("bank1")->set_base(&rom[address]);
}


WRITE8_MEMBER(royalmah_state::mjapinky_bank_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	m_rombank = data;
	membank("bank1")->set_base(ROM + 0x10000 + 0x8000 * data);
}

WRITE8_MEMBER(royalmah_state::mjapinky_palbank_w)
{
	m_flip_screen = (data & 4) >> 2;
	m_palette_base = (data >> 3) & 0x01;
	coin_counter_w(machine(), 0,data & 2);  // in
	coin_counter_w(machine(), 1,data & 1);  // out
}

READ8_MEMBER(royalmah_state::mjapinky_dsw_r)
{
	if (m_rombank == 0x0e)  return ioport("DSW3")->read();
	else                    return *(memregion("maincpu")->base() + 0x10000 + 0x8000 * m_rombank);
}

WRITE8_MEMBER(royalmah_state::tontonb_bank_w)
{
	UINT8 *rom = memregion("maincpu")->base();
	int address;

logerror("%04x: bank %02x\n",space.device().safe_pc(),data);

	if (data == 0) return;  // tontonb fix?

	data &= 0x0f;

	address = 0x10000 + data * 0x8000;

	membank("bank1")->set_base(&rom[address]);
}


/* bits 5 and 6 seem to affect which Dip Switch to read in 'majs101b' */
WRITE8_MEMBER(royalmah_state::dynax_bank_w)
{
	UINT8 *rom = memregion("maincpu")->base();
	int address;

//logerror("%04x: bank %02x\n",space.device().safe_pc(),data);

	m_dsw_select = data & 0x60;

	data &= 0x1f;

	address = 0x10000 + data * 0x8000;

	membank("bank1")->set_base(&rom[address]);
}

READ8_MEMBER(royalmah_state::daisyari_dsw_r)
{
	switch (m_dsw_select)
	{
		case 0x00: return ioport("DSW4")->read();
		case 0x04: return ioport("DSW1")->read();
		case 0x08: return ioport("DSW2")->read();
		case 0x0c: return ioport("DSW3")->read();
	}

	return 0;
}

WRITE8_MEMBER(royalmah_state::daisyari_bank_w)
{
	UINT8 *rom = memregion("maincpu")->base();
	int address;

	m_dsw_select = (data & 0xc);

	address = 0x10000 + ((data & 0x30)>>4) * 0x10000 + (data & 0x1) * 0x8000;
//  printf("%08x %02x\n",address,data);

	membank("bank1")->set_base(&rom[address]);

	/* bit 1 used too but unknown purpose. */
}

READ8_MEMBER(royalmah_state::mjclub_dsw_r)
{
	switch (m_dsw_select)
	{
//      case 0x00: return ioport("DSW4")->read();
		case 0x40: return ioport("DSW2")->read();
		case 0x80: return ioport("DSW3")->read();
		case 0xc0: return ioport("DSW4")->read();
	}

	return 0;
}

WRITE8_MEMBER(royalmah_state::mjclub_bank_w)
{
	UINT8 *rom = memregion("maincpu")->base();
	int address;

	m_dsw_select = data & 0xc0;

	data &= 0x0f;

	address = 0x10000 + data * 0x8000;
//  printf("%08x\n",address);

	membank("bank1")->set_base(&rom[address]);

	/* bit 5 used too but unknown purpose. */
}


static ADDRESS_MAP_START( royalmah_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0x6fff ) AM_ROM AM_WRITE(royalmah_rom_w )
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK( "bank1" )    // banked ROMs not present in royalmah
	AM_RANGE( 0x8000, 0xffff ) AM_WRITEONLY AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjapinky_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0x6fff ) AM_ROM AM_WRITE(royalmah_rom_w )
	AM_RANGE( 0x7000, 0x77ff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x7800, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0x8000 ) AM_READ(mjapinky_dsw_r )
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK( "bank1" )
	AM_RANGE( 0x8000, 0xffff ) AM_WRITEONLY AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( tahjong_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0x3fff ) AM_ROM AM_WRITE(royalmah_rom_w )
	AM_RANGE( 0x4000, 0x6fff ) AM_ROMBANK( "bank1" ) AM_WRITE(royalmah_rom_w )
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x8000, 0xffff ) AM_WRITEONLY AM_SHARE("videoram")
ADDRESS_MAP_END




static ADDRESS_MAP_START( royalmah_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1") AM_WRITE(royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( ippatsu_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1") AM_WRITE(royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0x12, 0x12 ) AM_READ_PORT("DSW2")
	AM_RANGE( 0x13, 0x13 ) AM_READ_PORT("DSW3")
ADDRESS_MAP_END

static ADDRESS_MAP_START( tahjong_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1") AM_WRITE(royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0x12, 0x12 ) AM_WRITE(tahjong_bank_w)
	AM_RANGE( 0x13, 0x13 ) AM_READ_PORT("DSW2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( suzume_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1") AM_WRITE(royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0x80, 0x80 ) AM_READ(suzume_dsw_r )
	AM_RANGE( 0x81, 0x81 ) AM_WRITE(suzume_bank_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( dondenmj_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1") AM_WRITE(royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0x85, 0x85 ) AM_READ_PORT("DSW2") // DSW2
	AM_RANGE( 0x86, 0x86 ) AM_READ_PORT("DSW3") // DSW3
	AM_RANGE( 0x87, 0x87 ) AM_WRITE(dynax_bank_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( makaijan_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1") AM_WRITE(royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0x84, 0x84 ) AM_READ_PORT("DSW2") // DSW2
	AM_RANGE( 0x85, 0x85 ) AM_READ_PORT("DSW3") // DSW3
	AM_RANGE( 0x86, 0x86 ) AM_WRITE(dynax_bank_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( daisyari_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10, 0x10 ) AM_WRITE(royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0xc0, 0xc0 ) AM_READWRITE(daisyari_dsw_r, daisyari_bank_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjclub_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_READWRITE(mjclub_dsw_r, mjclub_bank_w )
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1") AM_WRITE(royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjdiplob_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1") AM_WRITE(royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0x61, 0x61 ) AM_WRITE(tontonb_bank_w)
	AM_RANGE( 0x62, 0x62 ) AM_READ_PORT("DSW2") // DSW2
	AM_RANGE( 0x63, 0x63 ) AM_READ_PORT("DSW3") // DSW3
ADDRESS_MAP_END

static ADDRESS_MAP_START( tontonb_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1") AM_WRITE(royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0x44, 0x44 ) AM_WRITE(tontonb_bank_w )
	AM_RANGE( 0x46, 0x46 ) AM_READ_PORT("DSW2") // DSW2
	AM_RANGE( 0x47, 0x47 ) AM_READ_PORT("DSW3") // DSW3
ADDRESS_MAP_END

static ADDRESS_MAP_START( majs101b_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1") AM_WRITE(royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0x00, 0x00 ) AM_READWRITE(majs101b_dsw_r, dynax_bank_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjderngr_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
//  AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1")
	AM_RANGE( 0x10, 0x10 ) AM_WRITE(mjderngr_coin_w )   // palette bank is set separately
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0x20, 0x20 ) AM_WRITE(dynax_bank_w )
	AM_RANGE( 0x40, 0x40 ) AM_READ_PORT("DSW2")
	AM_RANGE( 0x4c, 0x4c ) AM_READ_PORT("DSW1")
	AM_RANGE( 0x60, 0x60 ) AM_WRITE(mjderngr_palbank_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjapinky_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_WRITE(mjapinky_bank_w )
	AM_RANGE( 0x01, 0x01 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x02, 0x03 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x04, 0x04 ) AM_READ_PORT("DSW2")
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("DSW1") AM_WRITE(mjapinky_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( janoh_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0x6fff ) AM_ROM AM_WRITE(royalmah_rom_w )
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x8000, 0xffff ) AM_WRITEONLY AM_SHARE("videoram")
ADDRESS_MAP_END


/* this CPU makes little sense - what is it for? why so many addresses accessed?
  -- it puts a value in shared ram to allow the main CPU to boot, then.. ?
*/
static ADDRESS_MAP_START( janoh_sub_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0x3fff ) AM_ROM
	AM_RANGE( 0x4100, 0x413f ) AM_RAM
	AM_RANGE( 0x6000, 0x607f ) AM_RAM
	AM_RANGE( 0x7000, 0x7000 ) AM_READNOP
	AM_RANGE( 0x7200, 0x7200 ) AM_WRITENOP
	AM_RANGE( 0xf000, 0xffff ) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( janoh_sub_iomap, AS_IO, 8, royalmah_state )
ADDRESS_MAP_END

/****************************************************************************
                                Jansou
****************************************************************************/



WRITE8_MEMBER(royalmah_state::jansou_dsw_sel_w)
{
	m_dsw_select = data;
}

READ8_MEMBER(royalmah_state::jansou_dsw_r)
{
	switch (m_dsw_select & 7)
	{
		case 1: return ioport("DSW1")->read();
		case 2: return ioport("DSW2")->read();
		case 4: return ioport("DSW3")->read();
	}

	return 0xff;
}

WRITE8_MEMBER(royalmah_state::jansou_colortable_w)
{
	m_jansou_colortable[offset] = data;
}

WRITE8_MEMBER(royalmah_state::jansou_6400_w)
{
	m_gfx_adr_l = data;
	m_gfx_adr = m_gfx_adr_h*0x10000 + m_gfx_adr_m*0x100 + m_gfx_adr_l;
}

WRITE8_MEMBER(royalmah_state::jansou_6401_w)
{
	m_gfx_adr_m = data;
	m_gfx_adr = m_gfx_adr_h*0x10000 + m_gfx_adr_m*0x100 + m_gfx_adr_l;
}

WRITE8_MEMBER(royalmah_state::jansou_6402_w)
{
	m_gfx_adr_h = data & 1;
	m_gfx_adr = m_gfx_adr_h*0x10000 + m_gfx_adr_m*0x100 + m_gfx_adr_l;
}

READ8_MEMBER(royalmah_state::jansou_6403_r)
{
	UINT8 *GFXROM = memregion("gfx1")->base();
	int d0 = GFXROM[m_gfx_adr];
	int d1 = GFXROM[m_gfx_adr+1];
	int c0 = m_jansou_colortable[d1 & 0x0f] & 0x0f;
	int c1 = m_jansou_colortable[(d1 & 0xf0) >> 4] >> 4;
	int c2 = m_jansou_colortable[d0 & 0x0f] & 0x0f;
	int c3 = m_jansou_colortable[(d0 & 0xf0) >> 4] >> 4;

	m_gfx_adr += 2;

	m_gfxdata0 = (c3 & 1) << 0 | ((c2 & 1) << 1) | ((c1 & 1) << 2) | ((c0 & 1) << 3)
				| ((c3 & 2) << 3) | ((c2 & 2) << 4) | ((c1 & 2) << 5) | ((c0 & 2) << 6);
	m_gfxdata1 = (c3 & 4) >> 2 | ((c2 & 4) >> 1) | (c1 & 4) | ((c0 & 4) << 1)
				| ((c3 & 8) << 1) | ((c2 & 8) << 2) | ((c1 & 8) << 3) | ((c0 & 8) << 4);

	return 0xff;
}

READ8_MEMBER(royalmah_state::jansou_6404_r)
{
	return m_gfxdata0;
}

READ8_MEMBER(royalmah_state::jansou_6405_r)
{
	return m_gfxdata1;
}

WRITE8_MEMBER(royalmah_state::jansou_sound_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( jansou_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0x3fff ) AM_ROM

	AM_RANGE( 0x6000, 0x600f ) AM_WRITE(jansou_colortable_w)
	AM_RANGE( 0x6400, 0x6400 ) AM_WRITE(jansou_6400_w)
	AM_RANGE( 0x6401, 0x6401 ) AM_WRITE(jansou_6401_w)
	AM_RANGE( 0x6402, 0x6402 ) AM_WRITE(jansou_6402_w)
	AM_RANGE( 0x6403, 0x6403 ) AM_READ(jansou_6403_r)
	AM_RANGE( 0x6404, 0x6404 ) AM_READ(jansou_6404_r)
	AM_RANGE( 0x6405, 0x6405 ) AM_READ(jansou_6405_r)
	AM_RANGE( 0x6406, 0x6406 ) AM_WRITE(jansou_dsw_sel_w)
	AM_RANGE( 0x6407, 0x6407 ) AM_READ(jansou_dsw_r)
	AM_RANGE( 0x6800, 0x6800 ) AM_WRITE(jansou_sound_w)

	AM_RANGE( 0x7000, 0x77ff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x8000, 0xffff ) AM_WRITEONLY AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( jansou_sub_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0xffff ) AM_ROM AM_WRITENOP // tries to write to the stack at irq generation
ADDRESS_MAP_END


static ADDRESS_MAP_START( jansou_sub_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_byte_r) AM_DEVWRITE("dac", dac_device, write_unsigned8 )
ADDRESS_MAP_END


/****************************************************************************
                                Janputer '96
****************************************************************************/

static ADDRESS_MAP_START( janptr96_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x6000, 0x6fff ) AM_RAMBANK("bank3") AM_SHARE("nvram")    // nvram
	AM_RANGE( 0x7000, 0x7fff ) AM_RAMBANK("bank2")  // banked nvram
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK("bank1")
	AM_RANGE( 0x8000, 0xffff ) AM_WRITEONLY AM_SHARE("videoram")
ADDRESS_MAP_END

WRITE8_MEMBER(royalmah_state::janptr96_dswsel_w)
{
	// 0x20 = 0 -> hopper on
	// 0x40 ?
	m_dsw_select = data;
}

READ8_MEMBER(royalmah_state::janptr96_dswsel_r)
{
	return m_dsw_select;
}

READ8_MEMBER(royalmah_state::janptr96_dsw_r)
{
	if (~m_dsw_select & 0x01) return ioport("DSW4")->read();
	if (~m_dsw_select & 0x02) return ioport("DSW3")->read();
	if (~m_dsw_select & 0x04) return ioport("DSW2")->read();
	if (~m_dsw_select & 0x08) return ioport("DSW1")->read();
	if (~m_dsw_select & 0x10) return ioport("DSWTOP")->read();
	return 0xff;
}

WRITE8_MEMBER(royalmah_state::janptr96_rombank_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->set_base(ROM + 0x10000 + 0x8000 * data);
}

WRITE8_MEMBER(royalmah_state::janptr96_rambank_w)
{
	membank("bank2")->set_base(m_janptr96_nvram + 0x1000 + 0x1000 * data);
}

READ8_MEMBER(royalmah_state::janptr96_unknown_r)
{
	// 0x08 = 0 makes the game crash (e.g. in the m-ram test: nested interrupts?)
	return 0xff;
}

WRITE8_MEMBER(royalmah_state::janptr96_coin_counter_w)
{
	m_flip_screen = (data & 4) >> 2;
	coin_counter_w(machine(), 0,data & 2);  // in
	coin_counter_w(machine(), 1,data & 1);  // out
}

static ADDRESS_MAP_START( janptr96_iomap, AS_IO, 8, royalmah_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_WRITE(janptr96_rombank_w )    // BANK ROM Select
	AM_RANGE( 0x1e, 0x1e ) AM_READWRITE(janptr96_dswsel_r, janptr96_dswsel_w )
	AM_RANGE( 0x1c, 0x1c ) AM_READ(janptr96_dsw_r )
	AM_RANGE( 0x20, 0x20 ) AM_READWRITE(janptr96_unknown_r, janptr96_rambank_w )
	AM_RANGE( 0x50, 0x50 ) AM_WRITE(mjderngr_palbank_w )
	AM_RANGE( 0x60, 0x6f ) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE( 0x81, 0x81 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x82, 0x83 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x93, 0x93 ) AM_WRITE(input_port_select_w )
	AM_RANGE( 0xd8, 0xd8 ) AM_WRITE(janptr96_coin_counter_w )
	AM_RANGE( 0xd9, 0xd9 ) AM_READ_PORT("SYSTEM")
ADDRESS_MAP_END

/****************************************************************************
                                Mahjong If
****************************************************************************/


WRITE8_MEMBER(royalmah_state::mjifb_coin_counter_w)
{
	m_flip_screen = ((data & 4) >> 2) ^ 1;
	coin_counter_w(machine(), 0,data & 2);  // in
	coin_counter_w(machine(), 1,data & 1);  // out
}

READ8_MEMBER(royalmah_state::mjifb_rom_io_r)
{
	if (m_mjifb_rom_enable)
		return ((UINT8*)(memregion("maincpu")->base() + 0x10000 + m_rombank * 0x4000))[offset];

	offset += 0x8000;

	switch(offset)
	{
		case 0x8000:    return ioport("DSW4")->read();      // dsw 4
		case 0x8200:    return ioport("DSW3")->read();      // dsw 3
		case 0x9001:    return machine().device<ay8910_device>("aysnd")->data_r(space, 0);   // inputs
		case 0x9011:    return ioport("SYSTEM")->read();
	}

	logerror("%04X: unmapped input read at %04X\n", space.device().safe_pc(), offset);
	return 0xff;
}

WRITE8_MEMBER(royalmah_state::mjifb_rom_io_w)
{
	UINT8 *videoram = m_videoram;
	if (m_mjifb_rom_enable)
	{
		videoram[offset] = data;
		return;
	}

	offset += 0x8000;

	switch(offset)
	{
		case 0x8e00:    m_palette_base = data & 0x1f;   return;
		case 0x9002:    machine().device<ay8910_device>("aysnd")->data_w(space,0,data);          return;
		case 0x9003:    machine().device<ay8910_device>("aysnd")->address_w(space,0,data);       return;
		case 0x9010:
			mjifb_coin_counter_w(space,0,data);
			return;
		case 0x9011:    input_port_select_w(space,0,data);  return;
		case 0x9013:
//          if (data)   popmessage("%02x",data);
			return;
	}

	logerror("%04X: unmapped input write at %04X = %02X\n", space.device().safe_pc(), offset,data);
}

WRITE8_MEMBER(royalmah_state::mjifb_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset + 0x4000] = data;
}

static ADDRESS_MAP_START( mjifb_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0x6fff ) AM_ROM
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x8000, 0xbfff ) AM_READWRITE(mjifb_rom_io_r, mjifb_rom_io_w) AM_SHARE("videoram")
	AM_RANGE( 0xc000, 0xffff ) AM_ROM AM_WRITE(mjifb_videoram_w)
//  AM_RANGE( 0xc000, 0xffff ) AM_ROM AM_WRITEONLY  This should, but doesn't work
ADDRESS_MAP_END

READ8_MEMBER(royalmah_state::mjifb_p3_r)
{
	return ioport("PORT3_5")->read() >> 6;
}
READ8_MEMBER(royalmah_state::mjifb_p5_r)
{
	return ioport("PORT3_5")->read();
}
READ8_MEMBER(royalmah_state::mjifb_p6_r)
{
	return ioport("PORT6_7")->read();
}
READ8_MEMBER(royalmah_state::mjifb_p7_r)
{
	return ioport("PORT6_7")->read() >> 4;
}
READ8_MEMBER(royalmah_state::mjifb_p8_r)
{
	return 0xff;
}

WRITE8_MEMBER(royalmah_state::mjifb_p3_w)
{
	m_rombank = (m_rombank & 0x0f) | ((data & 0x0c) << 2);
}
WRITE8_MEMBER(royalmah_state::mjifb_p4_w)
{
	m_rombank = (m_rombank & 0xf0) | (data & 0x0f);
}
WRITE8_MEMBER(royalmah_state::mjifb_p8_w)
{
	m_mjifb_rom_enable = (data & 0x08);
}

static ADDRESS_MAP_START( mjifb_iomap, AS_IO, 8, royalmah_state )
	AM_RANGE( T90_P3, T90_P3 ) AM_READWRITE(mjifb_p3_r, mjifb_p3_w )
	AM_RANGE( T90_P4, T90_P4 ) AM_WRITE(mjifb_p4_w )
	AM_RANGE( T90_P5, T90_P5 ) AM_READ(mjifb_p5_r )
	AM_RANGE( T90_P6, T90_P6 ) AM_READ(mjifb_p6_r )
	AM_RANGE( T90_P7, T90_P7 ) AM_READ(mjifb_p7_r )
	AM_RANGE( T90_P8, T90_P8 ) AM_READWRITE(mjifb_p8_r, mjifb_p8_w )
ADDRESS_MAP_END


/****************************************************************************
                           Mahjong Shinkirou Deja Vu
****************************************************************************/

READ8_MEMBER(royalmah_state::mjdejavu_rom_io_r)
{
	if (m_mjifb_rom_enable)
		return ((UINT8*)(memregion("maincpu")->base() + 0x10000 + m_rombank * 0x4000))[offset];

	offset += 0x8000;

	switch(offset)
	{
		case 0x8000:    return ioport("DSW2")->read();      // dsw 2
		case 0x8001:    return ioport("DSW1")->read();      // dsw 1
		case 0x9001:    return machine().device<ay8910_device>("aysnd")->data_r(space, 0);   // inputs
		case 0x9011:    return ioport("SYSTEM")->read();
	}

	logerror("%04X: unmapped input read at %04X\n", space.device().safe_pc(), offset);
	return 0xff;
}

WRITE8_MEMBER(royalmah_state::mjdejavu_rom_io_w)
{
	UINT8 *videoram = m_videoram;
	if (m_mjifb_rom_enable)
	{
		videoram[offset] = data;
		return;
	}

	offset += 0x8000;
	switch(offset)
	{
		case 0x8802:    m_palette_base = data & 0x1f;                   return;
		case 0x9002:    machine().device<ay8910_device>("aysnd")->data_w(space,0,data);      return;
		case 0x9003:    machine().device<ay8910_device>("aysnd")->address_w(space,0,data);   return;
		case 0x9010:    janptr96_coin_counter_w(space,0,data);     return;
		case 0x9011:    input_port_select_w(space,0,data);      return;
		case 0x9013:
//          if (data)   popmessage("%02x",data);
			return;
	}

	logerror("%04X: unmapped input write at %04X = %02X\n", space.device().safe_pc(), offset,data);
}

static ADDRESS_MAP_START( mjdejavu_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0x6fff ) AM_ROM
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x8000, 0xbfff ) AM_READWRITE(mjdejavu_rom_io_r, mjdejavu_rom_io_w) AM_SHARE("videoram")
	AM_RANGE( 0xc000, 0xffff ) AM_ROM AM_WRITE(mjifb_videoram_w)
ADDRESS_MAP_END


/****************************************************************************
                                Mahjong Tensinhai
****************************************************************************/

READ8_MEMBER(royalmah_state::mjtensin_p3_r)
{
	return 0xff;
}

void royalmah_state::mjtensin_update_rombank()
{
	membank("bank1")->set_base(memregion("maincpu")->base() + 0x10000 + m_rombank * 0x8000 );
}
WRITE8_MEMBER(royalmah_state::mjtensin_p4_w)
{
	m_rombank = (m_rombank & 0xf0) | (data & 0x0f);
	mjtensin_update_rombank();
}
WRITE8_MEMBER(royalmah_state::mjtensin_6ff3_w)
{
	m_rombank = (data << 4) | (m_rombank & 0x0f);
	mjtensin_update_rombank();
}

static ADDRESS_MAP_START( mjtensin_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x6000, 0x6fbf ) AM_RAM
	AM_RANGE( 0x6fc1, 0x6fc1 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x6fc2, 0x6fc3 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x6fd0, 0x6fd0 ) AM_WRITE(janptr96_coin_counter_w )
	AM_RANGE( 0x6fd1, 0x6fd1 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0x6fe0, 0x6fef ) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE( 0x6ff0, 0x6ff0 ) AM_READWRITE(janptr96_dsw_r, janptr96_dswsel_w )
	AM_RANGE( 0x6ff1, 0x6ff1 ) AM_WRITE(mjderngr_palbank_w )
	AM_RANGE( 0x6ff3, 0x6ff3 ) AM_WRITE(mjtensin_6ff3_w )
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK( "bank1" )
	AM_RANGE( 0x8000, 0xffff ) AM_WRITEONLY AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjtensin_iomap, AS_IO, 8, royalmah_state )
	AM_RANGE( T90_P3, T90_P3 ) AM_READ(mjtensin_p3_r )
	AM_RANGE( T90_P4, T90_P4 ) AM_WRITE(mjtensin_p4_w )
ADDRESS_MAP_END


/****************************************************************************
                                Mahjong Cafe Time
****************************************************************************/

void royalmah_state::cafetime_update_rombank()
{
	membank("bank1")->set_base(memregion("maincpu")->base() + 0x10000 + m_rombank * 0x8000 );
}
WRITE8_MEMBER(royalmah_state::cafetime_p4_w)
{
	m_rombank = (m_rombank & 0xf0) | (data & 0x0f);
	cafetime_update_rombank();
}
WRITE8_MEMBER(royalmah_state::cafetime_p3_w)
{
	m_rombank = (m_rombank & 0x0f) | ((data & 0x0c) << 2);
	cafetime_update_rombank();
}

WRITE8_MEMBER(royalmah_state::cafetime_dsw_w)
{
	m_dsw_select = data;
}
READ8_MEMBER(royalmah_state::cafetime_dsw_r)
{
	switch( m_dsw_select )
	{
		case 0x00: return ioport("DSW1")->read();
		case 0x01: return ioport("DSW2")->read();
		case 0x02: return ioport("DSW3")->read();
		case 0x03: return ioport("DSW4")->read();
		case 0x04: return ioport("DSWTOP")->read();
	}
	logerror("%04X: unmapped dsw read %02X\n", space.device().safe_pc(), m_dsw_select);
	return 0xff;
}

READ8_MEMBER(royalmah_state::cafetime_7fe4_r)
{
	return 0xff;
}
WRITE8_MEMBER(royalmah_state::cafetime_7fe3_w)
{
//  popmessage("%02x",data);
}

static ADDRESS_MAP_START( cafetime_map, AS_PROGRAM, 8, royalmah_state )
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x6000, 0x7eff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x7fc1, 0x7fc1 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x7fc2, 0x7fc3 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x7fd0, 0x7fd0 ) AM_WRITE(janptr96_coin_counter_w )
	AM_RANGE( 0x7fd1, 0x7fd1 ) AM_READ_PORT("SYSTEM") AM_WRITENOP
	AM_RANGE( 0x7fd3, 0x7fd3 ) AM_WRITE(input_port_select_w )
	AM_RANGE( 0x7fe0, 0x7fe0 ) AM_READ(cafetime_dsw_r )
	AM_RANGE( 0x7fe1, 0x7fe1 ) AM_WRITE(cafetime_dsw_w )
	AM_RANGE( 0x7fe2, 0x7fe2 ) AM_WRITE(mjderngr_palbank_w )
	AM_RANGE( 0x7fe3, 0x7fe3 ) AM_WRITE(cafetime_7fe3_w )
	AM_RANGE( 0x7fe4, 0x7fe4 ) AM_READ(cafetime_7fe4_r )
	AM_RANGE( 0x7ff0, 0x7fff ) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK( "bank1" )
	AM_RANGE( 0x8000, 0xffff ) AM_WRITEONLY AM_SHARE("videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( cafetime_iomap, AS_IO, 8, royalmah_state )
	AM_RANGE( T90_P3, T90_P3 ) AM_WRITE(cafetime_p3_w )
	AM_RANGE( T90_P4, T90_P4 ) AM_WRITE(cafetime_p4_w )
ADDRESS_MAP_END


/****************************************************************************
                               Mahjong Vegas
****************************************************************************/

WRITE8_MEMBER(royalmah_state::mjvegasa_p4_w)
{
	m_rombank = (m_rombank & 0xf8) | ((data & 0x0e) >> 1);
}
WRITE8_MEMBER(royalmah_state::mjvegasa_p3_w)
{
	m_rombank = (m_rombank & 0xf7) | ((data & 0x04) << 1);
}
WRITE8_MEMBER(royalmah_state::mjvegasa_rombank_w)
{
	m_rombank = (m_rombank & 0x0f) | ((data & 0x0f) << 4);
}

READ8_MEMBER(royalmah_state::mjvegasa_rom_io_r)
{
	if ((m_rombank & 0x70) != 0x70)
		return memregion("maincpu")->base()[0x10000 + m_rombank * 0x8000 + offset];

	offset += 0x8000;

	if((offset & 0xfff0) == 0x8000)
	{
		return m_rtc->read(space, offset & 0xf);
	}

	logerror("%04X: unmapped IO read at %04X\n", space.device().safe_pc(), offset);
	return 0xff;
}

WRITE8_MEMBER(royalmah_state::mjvegasa_rom_io_w)
{
	UINT8 *videoram = m_videoram;
	if ((m_rombank & 0x70) != 0x70)
	{
		videoram[offset] = data;
		return;
	}

	offset += 0x8000;

	if((offset & 0xfff0) == 0x8000)
	{
		m_rtc->write(space, offset & 0xf,data);
		return;
	}

	logerror("%04X: unmapped IO write at %04X = %02X\n", space.device().safe_pc(), offset,data);
}

WRITE8_MEMBER(royalmah_state::mjvegasa_coin_counter_w)
{
	m_flip_screen = (data & 4) >> 2;
	coin_counter_w(machine(), 0,data & 2);  // in
	coin_counter_w(machine(), 1,data & 1);  // out
}

// hopper?
WRITE8_MEMBER(royalmah_state::mjvegasa_12400_w)
{
	// bits 0 & 1
//  popmessage("UNK: %02x",data);
}
READ8_MEMBER(royalmah_state::mjvegasa_12500_r)
{
	// bits 0 & 2
	return 0xff;
}

static ADDRESS_MAP_START( mjvegasa_map, AS_PROGRAM, 8, royalmah_state )

	AM_RANGE( 0x00000, 0x05fff ) AM_ROM
	AM_RANGE( 0x06000, 0x07fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x08000, 0x0ffff ) AM_READWRITE(mjvegasa_rom_io_r, mjvegasa_rom_io_w) AM_SHARE("videoram")

	AM_RANGE( 0x10001, 0x10001 ) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE( 0x10002, 0x10003 ) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE( 0x10010, 0x10010 ) AM_WRITE(mjvegasa_coin_counter_w )
	AM_RANGE( 0x10011, 0x10011 ) AM_READ_PORT("SYSTEM") AM_WRITE(input_port_select_w )
	AM_RANGE( 0x10013, 0x10013 ) AM_WRITE(input_port_select_w )

	AM_RANGE( 0x12000, 0x12000 ) AM_WRITE(mjvegasa_rombank_w )
	AM_RANGE( 0x12100, 0x12100 ) AM_READ(cafetime_dsw_r )
	AM_RANGE( 0x12200, 0x12200 ) AM_WRITE(cafetime_dsw_w )
	AM_RANGE( 0x12300, 0x12300 ) AM_WRITE(mjderngr_palbank_w )
	AM_RANGE( 0x12400, 0x12400 ) AM_WRITE(mjvegasa_12400_w )
	AM_RANGE( 0x12500, 0x12500 ) AM_READ(mjvegasa_12500_r )

ADDRESS_MAP_END

static ADDRESS_MAP_START( mjvegasa_iomap, AS_IO, 8, royalmah_state )
	AM_RANGE( T90_P3, T90_P3 ) AM_READWRITE(mjtensin_p3_r, mjvegasa_p3_w )
	AM_RANGE( T90_P4, T90_P4 ) AM_WRITE(mjvegasa_p4_w )
ADDRESS_MAP_END



static INPUT_PORTS_START( mjctrl1 )
	PORT_START("KEY0")  /* P1 IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Credit Clear") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Credit Clear") PORT_CODE(KEYCODE_8)

	PORT_START("KEY1")  /* P1 IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")  /* P1 IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")  /* P1 IN3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")  /* P1 IN4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")  /* P2 IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY6")  /* P2 IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY7")  /* P2 IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY8")  /* P2 IN3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY9")  /* P2 IN4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")    /* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 ) /* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE2 )  /* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )  /* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( mjctrl2 )
	PORT_INCLUDE( mjctrl1 )

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( royalmah )
	PORT_INCLUDE( mjctrl1 )

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )  // "COIN2"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )  // "COIN1", but not working

	PORT_START("DSW1")  /* DSW  (inport $10) */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tahjong )
	PORT_INCLUDE( mjctrl1 )

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )  // "COIN2"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )  // "COIN1", but not working

	PORT_START("DSW1")  // port $10
	PORT_DIPNAME( 0x07, 0x07, "SWB:1,2,3" ) PORT_DIPLOCATION("SWB:1,2,3")   // only with bets on
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "5 (6)" )
	PORT_DIPSETTING(    0x07, "5 (7)" )
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SWB:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SWB:5")
	PORT_DIPNAME( 0x60, 0x60, "Winnings" ) PORT_DIPLOCATION("SWB:6,7")
	PORT_DIPSETTING(    0x40, "30 30 10 10 5 5 1 1" )
	PORT_DIPSETTING(    0x20, "32 24 16 12 8 4 2 1" )
	PORT_DIPSETTING(    0x60, "100 50 30 10 5 4 3 2" )
	PORT_DIPSETTING(    0x00, "200 50 30 10 5 4 3 2" )
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SWB:8")

	PORT_START("DSW2")  // port $13
	PORT_DIPNAME( 0x07, 0x07, "Pay Out Rate" ) PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x02, "68%" )
	PORT_DIPSETTING(    0x06, "71%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x05, "78%" )
	PORT_DIPSETTING(    0x03, "81%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPNAME( 0x18, 0x18, "Maximum Bet" ) PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x18, "20" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x40, "Allow Bets" ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Note Rate" )     PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x80, "10" )
INPUT_PORTS_END

static INPUT_PORTS_START( janyoup2 )
	PORT_INCLUDE( royalmah )

	PORT_START("DSW2")  /* DSW  (inport $12) */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 1-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* DSW  (inport $13) */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( suzume )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
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
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* DSW2 */
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )
	PORT_DIPSETTING(    0x00, "50 30 20 15 8 6 3 2" )
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* DSW3 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Background" )
	PORT_DIPSETTING(    0x08, "Black" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Girls" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")  /* DSW4 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tontonb )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  /* DSW1 (inport $10 -> 0x73b0) */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      // affects videoram - flip screen ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode ?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* DSW3 (inport $47 -> 0x73b1) */
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )          // check code at 0x0e6d
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )   // table at 0x4e7d
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )    // table at 0x4e4d
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )  // table at 0x4e5d
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" ) // table at 0x4e6d
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      // check code at 0x5184
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      // stores something at 0x76ff
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )      // check code at 0x1482, 0x18c2, 0x1a1d, 0x1a83, 0x2d2f and 0x2d85
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Maximum Payout ?" )      // check code at 0x1ab7
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "300" )
	PORT_DIPSETTING(    0x60, "500" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )      // check code at 0x18c2, 0x1a1d, 0x2d2f and 0x2d85
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")  /* DSW2 (inport $46 -> 0x73b2) */
	PORT_DIPNAME( 0x01, 0x00, "Special Combinations" )  // see notes
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )      // check code at 0x07c5
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      // check code at 0x5375
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      // check code at 0x5241
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )      // untested ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )      // check code at 0x13aa
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Full Tests" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

/* TODO: check dip-switches */
static INPUT_PORTS_START( makaijan )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, "Background Color" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPSETTING(    0x40, "Black" )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode ?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "Special Combinations" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Full Tests" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* TODO: check dip-switches */
static INPUT_PORTS_START( daisyari )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2" )
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
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, "DSW4" )
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
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjclub )
	PORT_INCLUDE( mjctrl2 )

	/* On the main board */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x08, "Pay Out Rate" )  PORT_DIPLOCATION("SW4:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x20, "Maximum Bet" )   PORT_DIPLOCATION("SW4:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x40, "Note Rate" )     PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPNAME( 0x80, 0x80, "Data Display" )  PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	/* On the subboard */
	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Game Type" )                     PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x02, "B" )
	PORT_DIPSETTING(    0x01, "C" )
	PORT_DIPSETTING(    0x03, "D" )
	PORT_DIPUNUSED_DIPLOC( 0x0c, 0x08, "SW2:3,4" ) PORT_CONDITION("DSW3", 0x03, EQUALS, 0x00)
	PORT_DIPUNUSED_DIPLOC( 0x0c, 0x08, "SW2:3,4" ) PORT_CONDITION("DSW3", 0x03, EQUALS, 0x02)
	PORT_DIPNAME( 0x0c, 0x08, "Bonus Rate (3renchan bonus)" )   PORT_DIPLOCATION("SW2:3,4") PORT_CONDITION("DSW3", 0x03, EQUALS, 0x01)
	PORT_DIPSETTING(    0x00, "A (1 2 2 3 pts.)" )
	PORT_DIPSETTING(    0x04, "B (1 2 2 5 pts.)" )
	PORT_DIPSETTING(    0x08, "C (1 2 3 6 pts.)" )
	PORT_DIPSETTING(    0x0c, "D (1 2 6 10 pts.)" )
	PORT_DIPNAME( 0x0c, 0x08, "Bonus Rate (5renchan bonus)" )   PORT_DIPLOCATION("SW2:3,4") PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "A (5 pts.)" )
	PORT_DIPSETTING(    0x04, "B (10 pts.)" )
	PORT_DIPSETTING(    0x08, "C (15 pts.)" )
	PORT_DIPSETTING(    0x0c, "D (20 pts.)" )
	PORT_DIPNAME( 0x30, 0x00, "CPU Houjuu Pattern" )            PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "100% Free" )
	PORT_DIPSETTING(    0x10, "75% Free" )
	PORT_DIPSETTING(    0x20, "50% Free" )
	PORT_DIPSETTING(    0x30, "25% Free" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Rate Autochange" )        PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )                     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* On the subboard */
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Double Odds Bonus" )             PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Extra Bet" )                     PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Color Hai Bonus" )               PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Sangenhai Bonus" )               PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x70, 0x00, "SW3:5,6,7" )
	PORT_DIPNAME( 0x80, 0x00, "Coin Needed for Last Chance" )   PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	/* On the subboard */
	PORT_START("DSW4")
	PORT_DIPNAME( 0x03, 0x00, "Odds Rate" )             PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "A (50 30 15 8 5 4 3 2)" )
	PORT_DIPSETTING(    0x02, "B (100 40 20 10 5 4 3 2)" )
	PORT_DIPSETTING(    0x01, "C (150 70 30 10 5 4 3 2)" )
	PORT_DIPSETTING(    0x00, "D (32 24 16 12 8 4 2 1)" )
	PORT_DIPNAME( 0x3c, 0x3c, "Bonus Awarded at:" )     PORT_DIPLOCATION("SW1:3,4,5,6")
	PORT_DIPSETTING(    0x00, "1st Time Only" )
	PORT_DIPSETTING(    0x20, "200 Coins" )
	PORT_DIPSETTING(    0x10, "300 Coins" )
	PORT_DIPSETTING(    0x30, "400 Coins" )
	PORT_DIPSETTING(    0x08, "500 Coins" )
	PORT_DIPSETTING(    0x28, "600 Coins" )
	PORT_DIPSETTING(    0x18, "700 Coins" )
	PORT_DIPSETTING(    0x38, "1000 Coins" )
	PORT_DIPSETTING(    0x3c, "Never" )
	PORT_DIPNAME( 0x40, 0x40, "Bonus Occurrence" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "Once" )
	PORT_DIPSETTING(    0x40, "Twice" )
	PORT_DIPNAME( 0x80, 0x80, "Background Color" )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "White" )     // Black according to manual
	PORT_DIPSETTING(    0x80, "Green" )
INPUT_PORTS_END

static INPUT_PORTS_START( mjdiplob )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  /* DSW1 (inport $10 -> 0x76fa) */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      // affects videoram - flip screen ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode ?" )      // check code at 0x0b94 and 0x0de2
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* DSW2 (inport $62 -> 0x76fb) */
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )          // check code at 0x09cd
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )   // table at 0x4b82
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )    // table at 0x4b52
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )  // table at 0x4b62
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" ) // table at 0x4b72
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Payout ?" )      // check code at 0x166c
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x10, "200" )
	PORT_DIPSETTING(    0x20, "300" )
	PORT_DIPSETTING(    0x30, "500" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      // check code at 0x2c64
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )      // check code at 0x2c64
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")  /* DSW3 (inport $63 -> 0x76fc) */
	PORT_DIPNAME( 0x01, 0x00, "Special Combinations" )  // see notes
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      // check code at 0x531f and 0x5375
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      // check code at 0x5240
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )      // check code at 0x2411
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )      // check code at 0x2411 and 0x4beb
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      // check code at 0x24ff, 0x25f2, 0x3fcf and 0x45d7
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )            // seems to hang after the last animation
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjderngr )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "ROM & Animation Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( majs101b )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  /* DSW1 (inport $10 -> 0x76fd) */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode ?" )      // check code at 0x1635
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* DSW2 (inport $00 (after out 0,$40) -> 0x76fa) */
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )          // check code at 0x14e4
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )   // table at 0x1539
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )    // table at 0x1509
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )  // table at 0x1519
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" ) // table at 0x1529
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      // check code at 0x1220, 0x128d, 0x13b1, 0x13cb and 0x2692
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x00, "Maximum Payout ?" )      // check code at 0x12c1
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x10, "300" )
	PORT_DIPSETTING(    0x30, "400" )
	PORT_DIPSETTING(    0x08, "500" )
	PORT_DIPSETTING(    0x28, "600" )
	PORT_DIPSETTING(    0x18, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
//  PORT_DIPSETTING(    0x38, "1000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      // check code at 0x1333
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Background" )
	PORT_DIPSETTING(    0x00, "Black" )
	PORT_DIPSETTING(    0x80, "Gray" )

	PORT_START("DSW3")  /* DSW3 (inport $00 (after out 0,$00) -> 0x76fc) */
	PORT_DIPNAME( 0x01, 0x00, "Special Combinations" )  // see notes
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )      // check code at 0x1cf9
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      // check code at 0x21a9, 0x21dc and 0x2244
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      // check code at 0x2b7f
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )      // check code at 0x50ba
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )      // check code at 0x1f65
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      // check code at 0x6412
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      // check code at 0x2cb2 and 0x2d02
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")  /* DSW4 (inport $00 (after out 0,$20) -> 0x76fb) */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Unknown ) )      // stored at 0x702f - check code at 0x1713,
	PORT_DIPSETTING(    0x00, "0" )             // 0x33d1, 0x3408, 0x3415, 0x347c, 0x3492, 0x350d,
	PORT_DIPSETTING(    0x01, "1" )             // 0x4af9, 0x4b1f and 0x61f6
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPNAME( 0x0c, 0x00, "Difficulty ?" )      // check code at 0x4b5c and 0x6d72
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )             // 0x05 - 0x03, 0x02, 0x02, 0x01
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )           // 0x0a - 0x05, 0x02, 0x02, 0x01
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )             // 0x0f - 0x06, 0x03, 0x02, 0x01
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )          // 0x14 - 0x0a, 0x06, 0x02, 0x01
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Unknown ) )      // check code at 0x228e
	PORT_DIPSETTING(    0x00, "0x00" )
	PORT_DIPSETTING(    0x10, "0x10" )
	PORT_DIPSETTING(    0x20, "0x20" )
	PORT_DIPSETTING(    0x30, "0x30" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      // check code at 0x11e4
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )            // check code at 0x006d
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjapinky )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  /* IN11 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x40, 0x00, "Background" )
	PORT_DIPSETTING(    0x40, "Black" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* IN12 */
	PORT_DIPNAME( 0x03, 0x03, "Unknown 2-0&1" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Unknown 2-4&5" )
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 2-6&7" )
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x00, "3" )

	PORT_START("DSW3")  /* IN13 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( janptr96 )
	PORT_INCLUDE( mjctrl1 )

	PORT_START("DSW4")  /* IN11 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Girls (Demo)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Don Den Key" )
	PORT_DIPSETTING(    0x80, "Start" )
	PORT_DIPSETTING(    0x00, "Flip/Flop" )

	PORT_START("DSW3")  /* IN12 */
	PORT_DIPNAME( 0x07, 0x07, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x07, "Cut" )
	PORT_DIPSETTING(    0x06, "1 T" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x03, "700" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1000?" )
	PORT_DIPSETTING(    0x00, "1000?" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* IN13 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Credits To Start" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Payout" )
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x10, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x40, 0x40, "W-BET" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")  /* IN14 */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Maximum Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSWTOP")    /* IN15 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Debug Mode" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Credits Per Note" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjifb )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("PORT3_5")   // IN10 - DSW1 (P3 & P5)
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x00, "Maximum Bet" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("PORT6_7")   // IN11 - DSW2 (P6 & P7)
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )
	PORT_DIPSETTING(    0x03, "32 24 16 12 8 4 2 1" )
	PORT_DIPSETTING(    0x00, "50 30 15 8 5 3 2 1" )
	PORT_DIPSETTING(    0x01, "100 50 25 10 5 3 2 1" )
	PORT_DIPSETTING(    0x02, "200 100 50 10 5 3 2 1" )
	PORT_DIPNAME( 0x04, 0x04, "Credits Per Note" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x38, 0x38, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x38, "Cut" )
	PORT_DIPSETTING(    0x30, "1 T" )
	PORT_DIPSETTING(    0x28, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x18, "700" )
	PORT_DIPSETTING(    0x10, "1000" )
//  PORT_DIPSETTING(    0x08, "1000?" )
//  PORT_DIPSETTING(    0x00, "1000?" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1/4" )
	PORT_DIPSETTING(    0x80, "2/4" )

	PORT_START("DSW3")  // IN13 - DSW3 ($8200)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "F-Rate" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Bye-Byte Bonus" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Auto-Mode After Reached" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Background Color" )
	PORT_DIPSETTING(    0x80, "Black" )
	PORT_DIPSETTING(    0x00, "Blue" )

	PORT_START("DSW4")  // IN14 - DSW4 ($8000)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 4-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 4-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Flip-Flop" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Animation" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjtensin )
	PORT_INCLUDE( mjctrl1 )

	PORT_START("DSW4")  /* IN11 */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Maximum Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW3")  /* IN12 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Credits To Start" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Payout" )
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x10, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x40, 0x40, "W-BET" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* IN13 */
	PORT_DIPNAME( 0x03, 0x03, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x03, "Cut" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPSETTING(    0x01, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x18, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x60, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")  /* IN14 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Girls (Demo)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Show Clock" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSWTOP")    /* IN15 */
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cafetime )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  /* IN11 */
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x00, "Maximum Bet" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 1-6&7" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("DSW2")  /* IN12 */
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )
	PORT_DIPSETTING(    0x03, "32 24 16 12 8 4 2 1" )
	PORT_DIPSETTING(    0x00, "50 30 15 8 5 3 2 1" )
	PORT_DIPSETTING(    0x01, "100 50 25 10 5 3 2 1" )
	PORT_DIPSETTING(    0x02, "200 100 50 10 5 3 2 1" )
	PORT_DIPNAME( 0x0c, 0x0c, "Unknown 2-2&3" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "Unknown 2-4&5&6" )
	PORT_DIPSETTING(    0x70, "0" )
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x50, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* IN13 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")  /* IN14 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 4-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 4-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Full Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Background" )
	PORT_DIPSETTING(    0x20, "Black" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWTOP")    /* IN15 */
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-8" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-9" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Unknown 3-8&9" )
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-9" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ippatsu )
	PORT_INCLUDE( mjctrl1 )

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )  // "COIN2"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )  // "COIN1", but not working


	PORT_START("DSW1") /* DSW  (inport $10) */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x40, "7" )
	PORT_DIPSETTING(    0x60, "10" )
	PORT_DIPNAME( 0x80, 0x80, "First Chance" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW2")  /* DSW  (inport $12) */
	PORT_DIPNAME( 0x03, 0x03, "Cut" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, "2 Bai" )
	PORT_DIPSETTING(    0x01, "3 Bai" )
	PORT_DIPSETTING(    0x03, "Yakuman" )
	PORT_DIPNAME( 0x0c, 0x0c, "Yakuman Bonus" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPSETTING(    0x04, "100" )
	PORT_DIPSETTING(    0x08, "200" )
	PORT_DIPSETTING(    0x0c, "300" )
	PORT_DIPNAME( 0x30, 0x30, "Unknown 1-4&5*" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6*" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* DSW  (inport $13) */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0*" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Second Bonus" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Allow Bets" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x18, "Unknown 2-3&4*" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjdejavu )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("PORT3_5")   // IN11 - DSW3 (P3 & P5)
	PORT_DIPNAME( 0x03, 0x03, "Unknown 3-0&1*" )
	PORT_DIPSETTING(    0x00, "1 1" )
	PORT_DIPSETTING(    0x02, "3 4" )
	PORT_DIPSETTING(    0x01, "1 2" )
	PORT_DIPSETTING(    0x03, "1 4" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PORT6_7")   // IN12 - DSW4 (P6 & P7)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 4-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 4-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 4-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")  // IN13 - DSW1 ($8001)
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x40, "Credits Per Note" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPNAME( 0x80, 0x80, "Background" )
	PORT_DIPSETTING(    0x80, "Gray" )
	PORT_DIPSETTING(    0x00, "Black" )

	PORT_START("DSW2")  // IN14 - DSW2 ($8000)
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" )
	PORT_DIPNAME( 0x3c, 0x3c, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x3c, "Cut" )
	PORT_DIPSETTING(    0x20, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
//  PORT_DIPSETTING(    0x04, "1000?" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1/4" )
	PORT_DIPSETTING(    0x80, "2/4" )
INPUT_PORTS_END

static INPUT_PORTS_START( jansou )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, "Pay Out Rate" )  PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "50%" )
	PORT_DIPSETTING(    0x0e, "53%" )
	PORT_DIPSETTING(    0x0d, "56%" )
	PORT_DIPSETTING(    0x0c, "59%" )
	PORT_DIPSETTING(    0x0b, "62%" )
	PORT_DIPSETTING(    0x0a, "65%" )
	PORT_DIPSETTING(    0x09, "68%" )
	PORT_DIPSETTING(    0x08, "71%" )
	PORT_DIPSETTING(    0x07, "74%" )
	PORT_DIPSETTING(    0x06, "77%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x04, "83%" )
	PORT_DIPSETTING(    0x03, "86%" )
	PORT_DIPSETTING(    0x02, "89%" )
	PORT_DIPSETTING(    0x01, "92%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x30, 0x00, "Maximum Bet" )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-7" )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-8" )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "First Chance" )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( None ) )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x00, "Last Chance" )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-5" )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-6" )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-7" )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Middle Chance" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-1" )   PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-2" )   PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-3" )   PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-4" )   PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-5" )   PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Girl" )          PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-7" )   PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-8" )   PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjvegasa )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  // 6810
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPSETTING(    0x20, "1 2 3 5 10 50 100 200" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")  // 6811
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "YAKUMAN Times" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x70, "Cut" )
	PORT_DIPSETTING(    0x60, "100?" )
	PORT_DIPSETTING(    0x50, "300" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x30, "700" )
	PORT_DIPSETTING(    0x20, "1000" )
//  PORT_DIPSETTING(    0x10, "1000" )
//  PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("DSW3")  // 6812
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "3 BAI In YAKUMAN Bonus Chance" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")  // 6813
	PORT_DIPNAME( 0x01, 0x01, "Unknown 4-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Show Clock" )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Girls" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Background" )
	PORT_DIPSETTING(    0x08, "Black" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWTOP")    // 6814
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-8" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-9" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Flip-Flop Key" )
	PORT_DIPSETTING(    0x00, "Flip-Flop" )
	PORT_DIPSETTING(    0x10, "Start" )
	PORT_DIPNAME( 0x20, 0x20, "Don Den Times" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x20, "8" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode" )    // e.g. press start in bet screen
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static MACHINE_CONFIG_START( royalmah, royalmah_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 18432000/6)        /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(royalmah_map)
	MCFG_CPU_IO_MAP(royalmah_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", royalmah_state,  irq0_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_PALETTE_ADD("palette", 16*2)
	MCFG_PALETTE_INIT_OWNER(royalmah_state,royalmah)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 8, 247)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_DRIVER(royalmah_state, screen_update_royalmah)
	MCFG_SCREEN_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 18432000/12)
	MCFG_AY8910_PORT_A_READ_CB(READ8(royalmah_state, royalmah_player_1_port_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(royalmah_state, royalmah_player_2_port_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( janoh, royalmah )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(8000000/2)   /* 4 MHz ? */
	MCFG_CPU_PROGRAM_MAP(janoh_map)

	MCFG_CPU_ADD("sub", Z80, 4000000)        /* 4 MHz ? */
	MCFG_CPU_PROGRAM_MAP(janoh_sub_map)
	MCFG_CPU_IO_MAP(janoh_sub_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", royalmah_state,  irq0_line_hold)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( jansou, royalmah )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(jansou_map)
	MCFG_CPU_IO_MAP(royalmah_iomap)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000) /* 4.000 MHz */
	MCFG_CPU_PROGRAM_MAP(jansou_sub_map)
	MCFG_CPU_IO_MAP(jansou_sub_iomap)
	MCFG_CPU_PERIODIC_INT_DRIVER(royalmah_state, irq0_line_hold, 4000000/512)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dondenmj, royalmah )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(8000000/2)   /* 4 MHz ? */
	MCFG_CPU_IO_MAP(dondenmj_iomap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tahjong, royalmah )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(8000000/2)   /* 4 MHz ? */
	MCFG_CPU_PROGRAM_MAP(tahjong_map)
	MCFG_CPU_IO_MAP(tahjong_iomap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( makaijan, royalmah )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(8000000/2)   /* 4 MHz ? */
	MCFG_CPU_IO_MAP(makaijan_iomap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( daisyari, royalmah )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(8000000/2)   /* 4 MHz ? */
	MCFG_CPU_IO_MAP(daisyari_iomap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mjclub, royalmah )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(8000000/2)   /* 4 MHz ? */
	MCFG_CPU_IO_MAP(mjclub_iomap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ippatsu, dondenmj )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(ippatsu_iomap)
MACHINE_CONFIG_END

INTERRUPT_GEN_MEMBER(royalmah_state::suzume_irq)
{
	if ( m_suzume_bank & 0x40 )
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_DERIVED( suzume, dondenmj )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(suzume_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", royalmah_state,  suzume_irq)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tontonb, dondenmj )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(tontonb_iomap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mjdiplob, dondenmj )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(mjdiplob_iomap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( majs101b, dondenmj )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(majs101b_iomap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mjapinky, dondenmj )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mjapinky_map)
	MCFG_CPU_IO_MAP(mjapinky_iomap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mjderngr, dondenmj )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(mjderngr_iomap)

	/* video hardware */
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(16*32)
	MCFG_PALETTE_INIT_OWNER(royalmah_state,mjderngr)
MACHINE_CONFIG_END

/* It runs in IM 2, thus needs a vector on the data bus */
TIMER_DEVICE_CALLBACK_MEMBER(royalmah_state::janptr96_interrupt)
{
	int scanline = param;

	if(scanline == 248)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x80);   // vblank

	if(scanline == 0)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x84);   // demo
}

WRITE_LINE_MEMBER(royalmah_state::janptr96_rtc_irq)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x82);   // rtc
}

static MACHINE_CONFIG_DERIVED( janptr96, mjderngr )
	MCFG_DEVICE_REMOVE("maincpu")

	MCFG_CPU_ADD("maincpu",Z80,XTAL_16MHz/2)    /* 8 MHz? */
	MCFG_CPU_PROGRAM_MAP(janptr96_map)
	MCFG_CPU_IO_MAP(janptr96_iomap)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", royalmah_state, janptr96_interrupt, "screen", 0, 1)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 8, 255-8)

	/* devices */
	MCFG_DEVICE_ADD("rtc", MSM6242, XTAL_32_768kHz)
	MCFG_MSM6242_OUT_INT_HANDLER(WRITELINE(royalmah_state, janptr96_rtc_irq))
	MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mjifb, mjderngr )
	MCFG_CPU_REPLACE("maincpu",TMP90841, 8000000)   /* ? */
	MCFG_CPU_PROGRAM_MAP(mjifb_map)
	MCFG_CPU_IO_MAP(mjifb_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", royalmah_state,  irq0_line_hold)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 8, 255-8)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mjdejavu, mjderngr )
	MCFG_CPU_REPLACE("maincpu",TMP90841, 8000000)   /* ? */
	MCFG_CPU_PROGRAM_MAP(mjdejavu_map)
	MCFG_CPU_IO_MAP(mjifb_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", royalmah_state,  irq0_line_hold)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 8, 255-8)
MACHINE_CONFIG_END


INTERRUPT_GEN_MEMBER(royalmah_state::mjtensin_interrupt)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);  // vblank
}

WRITE_LINE_MEMBER(royalmah_state::mjtensin_rtc_irq)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, HOLD_LINE);  // rtc
}


static MACHINE_CONFIG_DERIVED( mjtensin, mjderngr )
	MCFG_CPU_REPLACE("maincpu",TMP90841, 12000000)  /* ? */
	MCFG_CPU_PROGRAM_MAP(mjtensin_map)
	MCFG_CPU_IO_MAP(mjtensin_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", royalmah_state,  mjtensin_interrupt)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 8, 255-8)

	/* devices */
	MCFG_DEVICE_ADD("rtc", MSM6242, XTAL_32_768kHz)
	MCFG_MSM6242_OUT_INT_HANDLER(WRITELINE(royalmah_state, mjtensin_rtc_irq))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cafetime, mjderngr )
	MCFG_CPU_REPLACE("maincpu",TMP90841, 12000000)  /* ? */
	MCFG_CPU_PROGRAM_MAP(cafetime_map)
	MCFG_CPU_IO_MAP(cafetime_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", royalmah_state,  mjtensin_interrupt)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 8, 255-8)

	/* devices */
	MCFG_DEVICE_ADD("rtc", MSM6242, XTAL_32_768kHz)
	MCFG_MSM6242_OUT_INT_HANDLER(WRITELINE(royalmah_state, mjtensin_rtc_irq))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mjvegasa, mjderngr )
	MCFG_CPU_REPLACE("maincpu",TMP90841, XTAL_8MHz) /* ? */
	MCFG_CPU_PROGRAM_MAP(mjvegasa_map)
	MCFG_CPU_IO_MAP(mjvegasa_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", royalmah_state,  mjtensin_interrupt)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 8, 255-8)

	/* devices */
	MCFG_DEVICE_ADD("rtc", MSM6242, XTAL_32_768kHz)
	MCFG_MSM6242_OUT_INT_HANDLER(WRITELINE(royalmah_state, mjtensin_rtc_irq))
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/***************************************************************************

Royal Mahjong
(c)1981 Nichibutsu

CPU: Z80
Sound: AY-3-8910
OSC: 18.432MHz

***************************************************************************/

ROM_START( royalmj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.p1", 0x0000, 0x1000, CRC(549544bb) SHA1(dfb221572c7bfd267a22c0a944830d5f127f9942) )
	ROM_LOAD( "2.p2", 0x1000, 0x1000, CRC(afc8a61e) SHA1(4134f6404f955838fc48fd0f87b83ebc75c1a021) )
	ROM_LOAD( "3.p3", 0x2000, 0x1000, CRC(5d33e54d) SHA1(bf5e0ad5177c086f1cea5c90d7273a841db941bc) )
	ROM_LOAD( "4.p4", 0x3000, 0x1000, CRC(91339560) SHA1(0fb4141e236ab57b3e915dadb982b28ca11d269f) )
	ROM_LOAD( "5.p5", 0x4000, 0x1000, CRC(cc9123a3) SHA1(75276045247a0c9ac5810be01f3b58ad63101f9b) )
	ROM_LOAD( "6.p6", 0x5000, 0x1000, CRC(92150a0f) SHA1(5c97ba5014abdba4afc78e02e7d90e6ca4d777ac) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "18s030n.6k", 0x0000, 0x0020, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

ROM_START( royalmah )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1",       0x0000, 0x1000, CRC(69b37a62) SHA1(7792528754b0df4e11f4ebe33380b713ac7351a3) )
	ROM_LOAD( "rom2",       0x1000, 0x1000, CRC(0c8351b6) SHA1(9e6b48fd39dd98478d1e3557df839b09652c4349) )
	ROM_LOAD( "rom3",       0x2000, 0x1000, CRC(b7736596) SHA1(4b8bc175d945e695b767b9fb2227ffc1cd4b0547) )
	ROM_LOAD( "rom4",       0x3000, 0x1000, CRC(e3c7c15c) SHA1(a335374cc0f5b1d8e689cc304d006dd97f3e35e7) )
	ROM_LOAD( "rom5",       0x4000, 0x1000, CRC(16c09c73) SHA1(ea712f9ca3200ca27434e4200187b488e24f4c65) )
	ROM_LOAD( "rom6",       0x5000, 0x1000, CRC(92687327) SHA1(4fafba5881dca2a147616d94dd055eba6aa3c653) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f-rom.bpr",  0x0000, 0x0020, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

ROM_START( openmj )
	ROM_REGION( 0x7000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "10", 0x0000, 0x2000, CRC(4042920e) SHA1(19753bcb27ebf391ab824a45c6e41d956826a263) )
	ROM_LOAD( "20", 0x2000, 0x2000, CRC(8fa0f735) SHA1(645154d51c0679b953b9ffc2f1d3b8f2752a0796) )
	ROM_LOAD( "30", 0x4000, 0x2000, CRC(00045cd7) SHA1(0c32995753c1da14dacc8bc6c12dbcbdcae4e1b0) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s123.prm", 0x00, 0x20, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

ROM_START( tahjong )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "p1.bin", 0x00000, 0x1000, CRC(49c922b3) SHA1(6672a4f739d428b722ab22e7ca93064211557abc) )
	ROM_LOAD( "p2.bin", 0x01000, 0x1000, CRC(c33e3cc3) SHA1(707c437f180ddd2916b5806f208e0a478207d528) )
	ROM_LOAD( "p3.bin", 0x02000, 0x1000, CRC(9e741a74) SHA1(6c8e8eb04331d48b72e2be270c13dbf8deb76005) )
	ROM_LOAD( "p4.bin", 0x03000, 0x1000, CRC(dc6ae62b) SHA1(d0c51047f734c885b7f19972c1bf0408199fde51) )
	ROM_LOAD( "p5.bin", 0x04000, 0x1000, CRC(cc9123a3) SHA1(75276045247a0c9ac5810be01f3b58ad63101f9b) ) // same as royalmj (unused)
	ROM_LOAD( "p6.bin", 0x05000, 0x1000, CRC(92150a0f) SHA1(5c97ba5014abdba4afc78e02e7d90e6ca4d777ac) ) // ""

	ROM_LOAD( "s1.bin", 0x10000, 0x4000, CRC(beff21af) SHA1(4dc40a1ac4e36401b1ad8ae3954af6e5001e0a67) )
	ROM_LOAD( "s2.bin", 0x14000, 0x4000, CRC(fed42e7c) SHA1(31136dff07bd1883dc2d107823ba83a34abf003d) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "18s030n.6k", 0x0000, 0x0020, CRC(c074c0f0) SHA1(b62519d1496ea366b0ea8ed657bd758ce93875ec) )
ROM_END

ROM_START( janputer )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "1.bin",   0x0000, 0x2000, CRC(f36f4222) SHA1(ce18c273a59f86cb17ea6ba8a3daefc3d750df1e) )
	ROM_LOAD( "2.bin",   0x2000, 0x2000, CRC(7d57cb48) SHA1(38b97c5d02e3ab6187e5e0f86c06b8a3747b51a8) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(fb481d9a) SHA1(122e2b0d11fe1fe8cf219da9c8f96fe5a1016bb6) )
	ROM_LOAD( "7.bin",   0x6000, 0x1000, CRC(bb00fb9e) SHA1(4d2965a0339328d1700b39c166a5a92a96b05e67) )

	ROM_REGION( 0x20, "proms", 0 )
	/* taken from Royal Mahjong, might or might not be the same. */
	ROM_LOAD( "82s123.prm", 0x00, 0x20, BAD_DUMP CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

/***************************************************************************
Janyou Part II
(c)1984 Cosmo Denshi

CPU: Z80
Sound: AY-3-8910
Video: HD46505SP(HD6845SP)
OSC: 18.432MHz

ROMs:
1.C110       [36ebb3d0]
2.C109       [324426d4]
3.C108       [e98b6d34]
4.C107       [377b8ce9]

N82S123N.C98 [d3007282]


Others:
Battery
empty socket for MC68705
***************************************************************************/

ROM_START( janyoup2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.c110",       0x0000, 0x2000, CRC(36ebb3d0) SHA1(39c0cdd1dc5878539768074dad3c39aac4ace8bf) )
	ROM_LOAD( "2.c109",       0x2000, 0x2000, CRC(324426d4) SHA1(409244c8458d9bafa325746c37de9e7b955b3787) )
	ROM_LOAD( "3.c108",       0x4000, 0x2000, CRC(e98b6d34) SHA1(e27ab9a03aff750df78c5db52a112247bdd31328) )
	ROM_LOAD( "4.c107",       0x6000, 0x1000, CRC(377b8ce9) SHA1(a5efc517ae975e54af5325b8b3f4867e9f449d4c) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "n82s123n.c98", 0x0000, 0x0020, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

/****************************************************************************

Ippatsu Gyakuten
(c)1986 Public Software / Paradais

modified Royal Mahjong hardware

CPU: Z80
Sound: AY-3-8910

ROMs:
11(27256)
12(27128)
82S123AN

dumped by sayu

****************************************************************************/

ROM_START( ippatsu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11", 0x0000, 0x8000, CRC(5f563be7) SHA1(2ce486777bd61a2de789683cd0c8abeefe31775b) )
	ROM_LOAD( "12", 0x8000, 0x4000, CRC(a09a43b0) SHA1(da12e669ccd036da817a69bd549e8668e6a45730) )
	ROM_RELOAD(     0xc000, 0x4000 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123an", 0x00, 0x20, CRC(3bde1bbd) SHA1(729498483943f960e38c4ada992b099b698b497a) )
ROM_END

ROM_START( suzume )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "p1.bin",     0x00000, 0x1000, CRC(e9706967) SHA1(2e3d78178623de6552c9036da90e02f240d94055) )
	ROM_LOAD( "p2.bin",     0x01000, 0x1000, CRC(dd48cd62) SHA1(1ce7b515fabae5054f0ac284a9ed5760f59d18fa) )
	ROM_LOAD( "p3.bin",     0x02000, 0x1000, CRC(10a05c23) SHA1(f13ba660bc5eff9057b1ab46f564f586c76e945d) )
	ROM_LOAD( "p4.bin",     0x03000, 0x1000, CRC(267eaf52) SHA1(56e2f5d7080463dc0f11a2751590ac2b79eb02c5) )
	ROM_LOAD( "p5.bin",     0x04000, 0x1000, CRC(2fde346b) SHA1(7f45aa4427b4cb6bf6cc5919d397b25d53e133f3) )
	ROM_LOAD( "p6.bin",     0x05000, 0x1000, CRC(57f42ac7) SHA1(209b2f62a64ddf544578f144d9ec83478603c8b2) )
	/* bank switched ROMs follow */
	ROM_LOAD( "1.1a",       0x10000, 0x08000, CRC(f670dd47) SHA1(d0236021ae4dd5a10603dde038eb777feeff016f) )    // 0
	ROM_LOAD( "2.1c",       0x18000, 0x08000, CRC(140b11aa) SHA1(6f6a96135434324dcb486596920cb785fe2bf1a2) )    // 1
	ROM_LOAD( "3.1d",       0x20000, 0x08000, CRC(3d437b61) SHA1(175308086e1d7ab566c82dcaeef9f50690edf92a) )    // 2
	ROM_LOAD( "4.1e",       0x28000, 0x08000, CRC(9da8952e) SHA1(956d16b82ff8fe733a7b3135d082e18ea5167dfe) )    // 3
	ROM_LOAD( "5.1h",       0x30000, 0x08000, CRC(04a6f41a) SHA1(37117faf6bc823770413faa7618387ca6f16fa34) )    // 4

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(97e1defe) SHA1(b5002218b2292f7623dd9a205ce183dedeec03f1) )
ROM_END

ROM_START( dondenmj )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "dn5.1h",     0x00000, 0x08000, CRC(3080252e) SHA1(e039087afc36a0c594da093ea599b81a1d757139) )
	/* bank switched ROMs follow */
	ROM_LOAD( "dn1.1e",     0x18000, 0x08000, CRC(1cd9c48a) SHA1(12bc519889dacea59ae49672ad5313fff3a99f12) )    // 1
	ROM_LOAD( "dn2.1d",     0x20000, 0x04000, CRC(7a72929d) SHA1(7955f41883fa53876172bac417955ed0b5eb43f4) )    // 2
	ROM_LOAD( "dn3.2h",     0x30000, 0x08000, CRC(b09d2897) SHA1(0cde3e16ca333be01a5ab3a232f2ea602faec7a2) )    // 4
	ROM_LOAD( "dn4.2e",     0x50000, 0x08000, CRC(67d7dcd6) SHA1(6b708a29de1f4738eb2d4e667327d9433ff7216c) )    // 8

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(97e1defe) SHA1(b5002218b2292f7623dd9a205ce183dedeec03f1) )
ROM_END

ROM_START( mjdiplob )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "071.4l",     0x00000, 0x10000, CRC(81a6d6b0) SHA1(c6169e6d5f35304a0c3efcc2175c3213650f179c) )
	/* bank switched ROMs follow */
	ROM_RELOAD(             0x10000, 0x10000 )              // 0,1
	ROM_LOAD( "072.4k",     0x20000, 0x10000, CRC(a992bb85) SHA1(e60231e04831dac122d1d49a68641ee47b57faaf) )    // 2,3
	ROM_LOAD( "073.4j",     0x30000, 0x10000, CRC(562ed64f) SHA1(42b4a7e5a8de4dde83c12d7b9facf561bc872978) )    // 4,5
	ROM_LOAD( "074.4h",     0x40000, 0x10000, CRC(1eba0140) SHA1(0d0b95be338d7450ad3b24cc47e24e94f86dcefe) )    // 6,7

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(c1e427df) SHA1(9a9980d93dff4b87a940398b18277acaf946eeab) )
ROM_END

ROM_START( tontonb )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "091.5e",     0x00000, 0x10000, CRC(d8d67b59) SHA1(7e7a85df738f80fc031cda8a104ac9c7b3e24785) )
	/* bank switched ROMs follow */
	ROM_RELOAD(             0x10000, 0x10000 )              // 0,1
	/**/                                                    // 2,3 unused
	ROM_LOAD( "093.5b",     0x30000, 0x10000, CRC(24b6be55) SHA1(11390d6ed55d7d0b7b84c6d36d4ac5330a06abba) )    // 4,5
	/**/                                                    // 6,7 unused
	ROM_LOAD( "092.5c",     0x50000, 0x10000, CRC(7ff2738b) SHA1(89a49f89705f499439dc024fc70c87141a84780b) )    // 8,9

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(97e1defe) SHA1(b5002218b2292f7623dd9a205ce183dedeec03f1) )
ROM_END

/***************************************************************************

Makaijan
(c)1987 Dynax

***************************************************************************/

ROM_START( makaijan )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "ic1h.bin",     0x00000, 0x10000, CRC(7448c220) SHA1(ebb6564b83ce4f40a6e50a1be734e2086d97f592) )
	/* bank switched ROMs follow */
	ROM_COPY( "maincpu",    0x08000, 0x10000, 0x8000 )
	ROM_COPY( "maincpu",    0x08000, 0x18000, 0x8000 )
	ROM_LOAD( "052.1e",     0x50000, 0x10000, CRC(a881ca93) SHA1(499e17d2f57caa49c391d57dd737399fe4672f78) )
	ROM_LOAD( "053.1d",     0x30000, 0x10000, CRC(5f1d3e88) SHA1(152fde9f8e506f7f4ca1b2ecf8a828ece0501f78) )
	ROM_LOAD( "054.2h",     0x70000, 0x10000, CRC(ebc387c7) SHA1(7dfc892a5cccde7494ed06bbab88b4ea320dffbc) )
	ROM_LOAD( "055.2e",     0x20000, 0x10000, CRC(e26852ae) SHA1(8f8edefe851fd3641a5b4b227fb4dd976cdfa3e9) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(97e1defe) SHA1(b5002218b2292f7623dd9a205ce183dedeec03f1) )
ROM_END

/***************************************************************************

Daisyarin
(c)1989 Best System

***************************************************************************/

ROM_START( daisyari )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "1a.bin",     0x00000, 0x10000, CRC(7d14f90c) SHA1(742684d0785a93a45de0467e004db00531d016e2) )
	/* bank switched ROMs follow */
	ROM_COPY( "maincpu",    0x00000, 0x10000, 0x10000 )
	ROM_LOAD( "1c.bin",     0x20000, 0x10000, CRC(edfe52b9) SHA1(704c107fc8b89f561d2031d10468c124ab3d007a) ) /*2*/
	ROM_LOAD( "1d.bin",     0x30000, 0x10000, CRC(38f54a98) SHA1(d06eb851c75bfb2d8dd99bf5072c7f359f1f17e2) ) /*3*/
	ROM_LOAD( "1f.bin",     0x40000, 0x10000, CRC(b635f295) SHA1(dba3a59133c33c915dba678c510f00fb476f24da) ) /*4*/

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6k.bin",   0x0000, 0x0020, CRC(c1e427df) SHA1(9a9980d93dff4b87a940398b18277acaf946eeab) )
ROM_END

/***************************************************************************

Mahjong Club
(c)XEX

Royal mahjong subboard

1
3
4
5
63s081n

6116 RAM
surface scrached 40pin DIP (Z80?)
4.000MHz

***************************************************************************/

ROM_START( mjclub )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "5",     0x00000, 0x10000, CRC(cd148465) SHA1(42d1848656e461cfbf3fc0ba88ef8f4e67425f8c) )
	/* bank switched ROMs follow */
	ROM_COPY( "maincpu", 0x00000, 0x10000, 0x10000 )
	ROM_LOAD( "1",       0x50000, 0x10000, CRC(d0131f4b) SHA1(aac40b47b48f0ebfb07aaf17cb2a080fdcaa4697) )
	ROM_LOAD( "3",       0x60000, 0x10000, CRC(25628c38) SHA1(5166934c488c2f91bd6026c7896ad3536727d950) )
	ROM_LOAD( "4",       0x70000, 0x10000, CRC(a6ada333) SHA1(5fd44bf298a6f327118b98641af1aa0910519ded) )
	ROM_COPY( "maincpu", 0x50000, 0x40000, 0x10000 ) /*guess*/
	ROM_COPY( "maincpu", 0x60000, 0x30000, 0x10000 )
	ROM_COPY( "maincpu", 0x70000, 0x20000, 0x10000 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "63s081n", 0x0000, 0x0020, CRC(4add90c5) SHA1(de14abcba6eee53e73801ff12c45a75e875e6ca3) )
ROM_END

ROM_START( majs101b )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "171.3e",     0x00000, 0x10000, CRC(fa3c553b) SHA1(fda212559c4d55610a12ad2927afe21f9069c7b6) )
	/* bank switched ROMs follow */
	/**/                                                    // 0,1 unused
	ROM_RELOAD(             0x20000, 0x10000 )              // 2,3
	ROM_LOAD( "172.3f",     0x30000, 0x20000, CRC(7da39a63) SHA1(34d07978a326c83e5b51ce19619d52a75a501795) )    // 4,5,6,7
	ROM_LOAD( "173.3h",     0x50000, 0x20000, CRC(7a9e71ae) SHA1(ce1bde6e05f81b7dbb14015514397ed72f8dd92a) )    // 8,9,a,b
	ROM_LOAD( "174.3j",     0x70000, 0x10000, CRC(972c2cc9) SHA1(ba78d29d1723783dbd0e8c754d2422caad5ab367) )    // c,d

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(c1e427df) SHA1(9a9980d93dff4b87a940398b18277acaf946eeab) )
ROM_END

ROM_START( mjderngr )
	ROM_REGION( 0xb0000, "maincpu", 0 )
	ROM_LOAD( "2201.1a",    0x00000, 0x08000, CRC(54ec531d) SHA1(c5d9c575f6bdc499bae35123d7ad5bd4869b6ed9) )
	/* bank switched ROMs follow */
	ROM_CONTINUE(           0x10000, 0x08000 )              // 0
	ROM_LOAD( "2202.1b",    0x30000, 0x10000, CRC(edcf97f2) SHA1(8143f41d511fa01bd86faf829eb2c139292d705f) )    // 4,5
	ROM_LOAD( "2203.1d",    0x50000, 0x10000, CRC(a33368c0) SHA1(e216b65d7ed59d7cbf2b5d078799915d707b5291) )    // 8,9
	ROM_LOAD( "2204.1e",    0x70000, 0x20000, CRC(ed5fde4b) SHA1(d55487ae1007d43b71f06ae5c407c75db7054515) )    // c,d,e,f
	ROM_LOAD( "2205.1f",    0x90000, 0x20000, CRC(cfb8075d) SHA1(31f613a1a9b5f4295b552aeeddb760605ce2ac70) )    // 0x10,0x11,0x12,0x13

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "ic3g.bin",   0x000, 0x200, CRC(d43f4c7c) SHA1(117d2e4e8d5bea3e5dc903a4b87bd71786ae009c) )
	ROM_LOAD( "ic4g.bin",   0x200, 0x200, CRC(30cf7831) SHA1(b4593d51c6ceb301279a01a98665e4be8a3c403d) )
ROM_END

/***************************************************************************

Mahjong If (BET type)
(c)1990 Dynax

CPU:    Unknown 64P(Toshiba TLCS-90 series?)
Sound:  AY-3-8910
OSC:    8.000MHz
        18.432MHz


2911.1B   prg.
2902.1C
2903.1D
2904.1E
2905.1F
2906.1G

D29-1.4C  color
D29-2.4D

***************************************************************************/

ROM_START( mjifb )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "2911.1b",    0x00000, 0x10000, CRC(138a31a1) SHA1(7e77c63a968206b8e61aaa423e19a766e4142554) )
	/* bank switched ROMs follow */
	ROM_RELOAD(             0x10000, 0x08000 )  // bank 0 = 8000-bfff
	ROM_CONTINUE(           0x10000, 0x08000 )
	ROM_LOAD( "2903.1d",    0x30000, 0x20000, CRC(90c44965) SHA1(6904bfa7475f9de921bc2abcfc337b3daf7e0fad) )
	ROM_LOAD( "2906.1g",    0x50000, 0x20000, CRC(ad469345) SHA1(914ea4c77a540467da779ea78c52e66b05c30475) )
	ROM_LOAD( "2904.1e",    0x70000, 0x20000, CRC(2791abfa) SHA1(a8fd1a7e1cf4441b447a4605ad2f1c13775f92da) )
	ROM_LOAD( "2905.1f",    0x90000, 0x20000, CRC(b7a73cf7) SHA1(d93111e6d5f84e331f8198d8c595e3500abed133) )
	ROM_LOAD( "2902.1c",    0xb0000, 0x10000, CRC(0ce02a98) SHA1(69f6bca9af8548038401839047a304a4aa97cfe6) )
	ROM_RELOAD(             0xc0000, 0x10000 )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d29-2.4d",   0x000, 0x200, CRC(78252f6a) SHA1(1869147bc6b7573c2543bdf6b17d6c3c1debdddb) )
	ROM_LOAD( "d29-1.4c",   0x200, 0x200, CRC(4aaec8cf) SHA1(fbe1c3729d078a422ffe68dfde495fcb9f329cdd) )
ROM_END

ROM_START( mjifb2 )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "2921.bin",    0x00000, 0x10000, CRC(9f2bfa4e) SHA1(7d6ca22bf0a91d65fde34ae321054638df705eef) )
	/* bank switched ROMs follow */
	ROM_RELOAD(             0x10000, 0x08000 )  // bank 0 = 8000-bfff
	ROM_CONTINUE(           0x10000, 0x08000 )
	ROM_LOAD( "2903.1d",    0x30000, 0x20000, CRC(90c44965) SHA1(6904bfa7475f9de921bc2abcfc337b3daf7e0fad) )
	ROM_LOAD( "2906.1g",    0x50000, 0x20000, CRC(ad469345) SHA1(914ea4c77a540467da779ea78c52e66b05c30475) )
	ROM_LOAD( "2904.1e",    0x70000, 0x20000, CRC(2791abfa) SHA1(a8fd1a7e1cf4441b447a4605ad2f1c13775f92da) )
	ROM_LOAD( "2905.1f",    0x90000, 0x20000, CRC(b7a73cf7) SHA1(d93111e6d5f84e331f8198d8c595e3500abed133) )
	ROM_LOAD( "2902.1c",    0xb0000, 0x10000, CRC(0ce02a98) SHA1(69f6bca9af8548038401839047a304a4aa97cfe6) )
	ROM_RELOAD(             0xc0000, 0x10000 )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d29-2.4d",   0x000, 0x200, CRC(78252f6a) SHA1(1869147bc6b7573c2543bdf6b17d6c3c1debdddb) )
	ROM_LOAD( "d29-1.4c",   0x200, 0x200, CRC(4aaec8cf) SHA1(fbe1c3729d078a422ffe68dfde495fcb9f329cdd) )
ROM_END

ROM_START( mjifb3 )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "2931.bin",    0x00000, 0x10000, CRC(2a3133de) SHA1(9fdc8c145d3da17ec5f86810716f1b1a2abd8023) )
	/* bank switched ROMs follow */
	ROM_RELOAD(             0x10000, 0x08000 )  // bank 0 = 8000-bfff
	ROM_CONTINUE(           0x10000, 0x08000 )
	ROM_LOAD( "2903.1d",    0x30000, 0x20000, CRC(90c44965) SHA1(6904bfa7475f9de921bc2abcfc337b3daf7e0fad) )
	ROM_LOAD( "2906.1g",    0x50000, 0x20000, CRC(ad469345) SHA1(914ea4c77a540467da779ea78c52e66b05c30475) )
	ROM_LOAD( "2904.1e",    0x70000, 0x20000, CRC(2791abfa) SHA1(a8fd1a7e1cf4441b447a4605ad2f1c13775f92da) )
	ROM_LOAD( "2905.1f",    0x90000, 0x20000, CRC(b7a73cf7) SHA1(d93111e6d5f84e331f8198d8c595e3500abed133) )
	ROM_LOAD( "2902.1c",    0xb0000, 0x10000, CRC(0ce02a98) SHA1(69f6bca9af8548038401839047a304a4aa97cfe6) )
	ROM_RELOAD(             0xc0000, 0x10000 )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d29-2.4d",   0x000, 0x200, CRC(78252f6a) SHA1(1869147bc6b7573c2543bdf6b17d6c3c1debdddb) )
	ROM_LOAD( "d29-1.4c",   0x200, 0x200, CRC(4aaec8cf) SHA1(fbe1c3729d078a422ffe68dfde495fcb9f329cdd) )
ROM_END

/***************************************************************************

Janputer '96
(c)1996 Dynax

Colour proms are TBP28S42's

***************************************************************************/

ROM_START( janptr96 )
	ROM_REGION( 0x210000, "maincpu", 0 )
	ROM_LOAD( "503x-1.1h", 0x000000, 0x40000, CRC(39914ecd) SHA1(e5796a95a7e3e7b61da63d50fa089be2946ba611) )
	/* bank switched ROMs follow */
	ROM_RELOAD(            0x010000, 0x40000 )
	ROM_RELOAD(            0x050000, 0x40000 )
	ROM_LOAD( "503x-2.1g", 0x090000, 0x80000, CRC(d4b1ed79) SHA1(e1e266339d1d05c0405bfd32b67f215807696c82) )
	ROM_LOAD( "503x-3.1f", 0x110000, 0x80000, CRC(9ba4deb0) SHA1(e9d44a6ed849ff90c0b1f9321cdd62e18c3fd35c) )
	ROM_LOAD( "503x-4.1e", 0x190000, 0x80000, CRC(e266ca0b) SHA1(d84608e7b474061a680510a266842e667bf2eab5) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "ns503b.3h", 0x000, 0x200, CRC(3b2a6b12) SHA1(ebd2929e6acbde989964bfef602b81f2f2fe04eb) )
	ROM_LOAD( "ns503a.3j", 0x200, 0x200, CRC(fe49b2f0) SHA1(a36ca005380cc92dfe473254c26be2cef2ced9b4) )
ROM_END

/***************************************************************************

Janputer Special
(c)1997 Dynax

Colour proms are TBP28642's

***************************************************************************/

ROM_START( janptrsp )
	ROM_REGION( 0x210000, "maincpu", 0 )
	ROM_LOAD( "ns51101.1h", 0x000000, 0x80000, CRC(44492ca1) SHA1(49e3dc9872a26e446599deb47161b8f52e4968c4) )
	/* bank switched ROMs follow */
	ROM_RELOAD(             0x010000, 0x80000 )
	ROM_LOAD( "ns51102.1g", 0x090000, 0x80000, CRC(01e6aa19) SHA1(a761fe69fb69c0bf101033e71813742c9fc2d747) )
	ROM_LOAD( "ns51103.1f", 0x110000, 0x80000, CRC(0fc94805) SHA1(035002e8354673a063faacd3cb91d0512cab677a) )
	ROM_LOAD( "ns51104.1e", 0x190000, 0x80000, CRC(00442508) SHA1(268cc0c76bb9c21213c941952dbc891778ad397e) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "ns511b.3h", 0x000, 0x200, CRC(1286434f) SHA1(6818549d0e8b231d7071e67923f47b96fc6e1bb6) )
	ROM_LOAD( "ns511a.3j", 0x200, 0x200, CRC(26b6714c) SHA1(0d110c1e3f7050a3c4ecbed630a0a8b522f1b0b0) )
ROM_END

/***************************************************************************

Mahjong Tensinhai
Dynax, 1995

PCB Layout
----------

Top board

D10010318L1
|----------------------------------------|
|DSW2(1)  DSW4(10)                  DIP16|
|                 |---|                  |
|DSW1(10) DSW3(10)| * |                  |
|                 |---|     PROM2        |
|                                        |
|                           PROM1        |
|                                        |
|                                        |
|                                        |
|                                        |
|                        1001.5E         |
| |-------------|                        |
| |     &       |        1002.4E    |---||
| |-------------|                   | D ||
|12MHz                   1003.3E    | I ||
|                                   | P ||
|BATTERY        32.768kHz           |40 ||
|         CLOCK          6264       |---||
|----------------------------------------|
Notes:
      Most of the chips have their surface scratched off.
      *     - Unknown PLCC44 IC. Possibly Mach110 or similar CPLD
      &     - Unknown SDIP64 IC. Possibly a Toshiba TMP91P640? Clock input 12.000MHz
      CLOCK - Some kind of clock IC, like Oki M6242 or similar
      PROM1 - 82S147 PROM labelled 'D100-1'
      PROM2 - 82S147 PROM labelled 'D100-2'
      DIP16 - Socket for cable that joins to lower board
      DIP40 - Socket for connector that joins to lower board


Bottom board

|--------------------------------------------------------|
|    BATTERY 6116                                        |
|  VOL                                                   |
|                                                        |
|                                              DIP40     |
|                                                        |
|           DSW(8)                              18.432MHz|
|                                                        |
|                                                        |
|M      DIP16                                            |
|A              4164    4164                             |
|H                                                       |
|J              4164    4164                             |
|O                                                       |
|N              4164    4164                             |
|G                                                       |
|2              4164    4164                             |
|8  AY3-8910                                             |
|               4164    4164                             |
|                                                        |
|               4164    4164                             |
|                                                        |
|               4164    4164                             |
|                                                        |
|               4164    4164                             |
|--------------------------------------------------------|
Notes:
      DIP16 - Socket for cable that joins to upper board
      DIP40 - Socket for connector that joins to upper board
      AY3-8910 clock - 1.536 [18.432/12]
      HSync - 15.5kHz
      VSync - 60Hz

***************************************************************************/

ROM_START( mjtensin )
	ROM_REGION( 0x290000, "maincpu", 0 )
	ROM_LOAD( "1001.5e", 0x000000, 0x80000, CRC(960e1fe9) SHA1(11f5164b2c75c0e684e910ee8e09de978bdaff2f) )
	/* bank switched ROMs follow */
	ROM_RELOAD(          0x010000, 0x80000 )
	ROM_RELOAD(          0x090000, 0x80000 )

	ROM_LOAD( "1002.4e", 0x110000, 0x80000, CRC(240eb7af) SHA1(2309e1c251fe55f6e6b97b5db94fa2fe914b88f4) )

	ROM_LOAD( "1003.3e", 0x210000, 0x80000, CRC(876081bf) SHA1(fe962cfa9318a9444123bcaf3406e22fb08e8c4e) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d100-2.7e",  0x000, 0x200, CRC(6edeed23) SHA1(f4420c473ebbe3df92b0f5b1f0e4d5495fcb9fda) )
	ROM_LOAD( "d100-1.6e",  0x200, 0x200, CRC(88befd59) SHA1(cbcb437f9f6b5e542dc69f5c9e85ccbae47080af) )
ROM_END

/***************************************************************************

Almond Pinky
Dynax, 1988

This game runs on Royal Mahjong hardware.
It appears Royal Mahjong was originally manufactured by Nichibutsu
This PCB says "(C) 1983 Nichibutsu" on it.

Top PCB
-------
D1401128L-0
|------------------------------------------|
|         |---------|                      |
|DIP40    |         |                      |
| Z80A    |   &     |                      |
|         |---------|                      |
|     8MHz                           DSW1  |
|                                          |
|                                          |
|                                          |
|                                          |
|                                    DSW2  |
|                                          |
|                                          |
|                                          |
|DYNAX DYNAX DYNAX DYNAX DYNAX DYNAX       |
| 146   145   144   143   142   141  6116 *|
|------------------------------------------|
Notes:
      Every chip has it's surface scratched
      *     - 4 pin power connector joined to main PCB
      DSWx  - have 8 switches each
      DIP40 - Socket joins to main PCB
      &     - Large Dynax ceramic SIP module (DAC or similar)
      Z80   - clock 4MHz [8/2]
      All ROMs type 27512

Note! On Royal Mahjong-based PCBs where there is a 3 pin RGB connector (3 wires) tied to the main
board and joining to the top board, the color PROM is located on the top daughterboard. Usually that
chip is an 82S123 (32 bytes), 82S129 (256 bytes) or 82S147 (512 bytes). In any case, you can be sure
there is a PROM(s) on the PCB somewhere if the RGB connector cable is present.


Main PCB
--------
RM-1D (C) Nichibutsu 1983
|------------------------------------------------|
|  6116 DIP24  P6    P5    P4    P3     P2    P1 |
|MB3712                                          |
|     BATTERY                                    |
|              DSW(8)                    DIP40   |
|                                                |
|1                                      18.432MHz|
|8    TBP18S030.6K                               |
|W                                               |
|A                                               |
|Y                                               |
|   AY3-8910                                     |
|            4116    4116                        |
|1           4116    4116                        |
|0           4116    4116                        |
|W           4116    4116                        |
|A           4116    4116                        |
|Y           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Socket joins to top PCB
      DIP24 - Unpopulated DIP24 position (no socket)
      TBP18S030.6K - Color PROM (32 bytes)
      P1-P6 - Program ROM sockets (DIP24)

***************************************************************************/

ROM_START( mjapinky )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "141.4d",     0x00000, 0x10000, CRC(0c4fb83a) SHA1(5d467e8fae715ca4acf88f8e9437c7cdf9f876bd) )
	/* bank switched ROMs follow */
	ROM_RELOAD(             0x10000, 0x10000 )
	ROM_LOAD( "142.4e",     0x20000, 0x10000, CRC(129806f0) SHA1(d12d2c5bb0c653f2e4974c47004ada128ac30bea) )
	ROM_LOAD( "143.4f",     0x30000, 0x10000, CRC(3d0bc452) SHA1(ad61eaa892121f90f31a6baf83158a11e6051430) )
	ROM_LOAD( "144.4h",     0x40000, 0x10000, CRC(24509a18) SHA1(ab9daed2cbc72d02c2168a4c93f70ebfe3916ea2) )
	ROM_LOAD( "145.4j",     0x50000, 0x10000, CRC(fea3375a) SHA1(cbb89b72cfba9c0448d152dfdbedb20b9896516e) )
	ROM_LOAD( "146.4k",     0x60000, 0x10000, CRC(be27a9b9) SHA1(f12402182f598391e445245b345f49084a69620a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "18s030n.clr",   0x0000, 0x0020, CRC(5736d0aa) SHA1(298b51340d2697347842cfaa5921f31c7b7f9748) )
ROM_END

/***************************************************************************

Mahjong Cafe Time
(c)1992 Dynax
Modified Royal Mahjong PCB
D6310128L1-1 (Sub PCB)

CPU: Z80
Sound: AY-3-8910

ROMs:
6301.2E      [1fc10e7c]
6302.3E      [02bbdf78]
6303.5E      [0e71eea8]
6304.6E      [53c581d6]

D63-1.7F     [e7410136] MB7124H
D63-2.8F     [af735b42] /

***************************************************************************/

ROM_START( cafetime )
	ROM_REGION( 0x210000, "maincpu", 0 )
	ROM_LOAD( "6301.2e", 0x000000, 0x40000, CRC(1fc10e7c) SHA1(0ed6bfd4cc6fc64bbf55bd3c6bde2d8ba9da2afb) )
	/* bank switched ROMs follow */
	ROM_RELOAD(          0x010000, 0x40000 )
	ROM_RELOAD(          0x050000, 0x40000 )
	ROM_LOAD( "6302.3e", 0x090000, 0x80000, CRC(02bbdf78) SHA1(e1e107541236ed92854fac4e12c9b300dbac9822) )
	ROM_LOAD( "6303.5e", 0x110000, 0x80000, CRC(0e71eea8) SHA1(f95c3b7acee6deabff4aca83b490e255648e2f19) )
	ROM_LOAD( "6304.6e", 0x190000, 0x80000, CRC(53c581d6) SHA1(d9cfda63a8f2e92873f69c673d3efe5c22cfa0de) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d63-2.8f", 0x000, 0x200, CRC(af735b42) SHA1(deddde3e276d5b9de72e267f65399d80783c6244) )
	ROM_LOAD( "d63-1.7f", 0x200, 0x200, CRC(e7410136) SHA1(54d3aec0d11485d4f419e76f9c4071ab9b817937) )
ROM_END

/***************************************************************************

Mahjong Cafe Doll
Dynax, 1993

This game runs on Royal Mahjong hardware.

Top PCB
-------
D76052208L-2
|-----------------------------------|
|   7601  6264        RTC   BATTERY |
|DIP40                              |
|   7602                  8MHz      |
|                PAL           PAL  |
|   7603           |-----------|    |
|                  |    CPU    |    |
|   DIP32          |-----------|    |
|                                   |
|                                   |
|                                   |
|                                   |
|                                   |
| 82s147.7F                        &|
|*                      DSW3   DSW1 |
| 82S147.9F  %          DSW4   DSW2 |
|-----------------------------------|
Notes:
      Every chip has it's surface scratched, except the PROMs
      *     - Connector joined to main PCB
      &     - Power input connector
      %     - RGB Video output
      DIP32 - Empty DIP32 socket
      DSWx  - have 10 switches each
      DIP40 - Socket joins to main PCB
      CPU   - unknown SDIP64 chip. Possibly TMP90P640 or similar TLCS-90 type CPU


Main PCB
--------
no pcb number
|------------------------------------------------|
|   6116 DIP24 DIP24 DIP24 DIP28 DIP28 DIP28     |
|        DIP24                                   |
|HA1368                                          |
|                                        DIP40   |
|                                                |
|             DSW(8)                    18.432MHz|
|M                                               |
|A                                               |
|H                                               |
|J                                               |
|O  AY3-8910                                     |
|N           4116    4116                        |
|G           4116    4116                        |
|2           4116    4116                        |
|8           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Sockets joins to top PCB
      DIP24/28 - Unpopulated sockets

***************************************************************************/

ROM_START( cafedoll )
	ROM_REGION( 0x190000, "maincpu", 0 )
	ROM_LOAD( "76xx.tmp90841", 0x00000, 0x02000, NO_DUMP )
	ROM_LOAD( "7601", 0x000000, 0x80000, CRC(20c80ad9) SHA1(e45edd101c6e26c0fa3c3f15f4a4152a853e41bd) )
	/* bank switched ROMs follow */
	ROM_RELOAD(       0x010000, 0x80000 )
	ROM_LOAD( "7602", 0x090000, 0x80000, CRC(f472960c) SHA1(cc2feb4374ba94035101114c73e1690cfeac9b91) )
	ROM_LOAD( "7603", 0x110000, 0x80000, CRC(c4293019) SHA1(afd717844e9e681ada14e80cd10dce0ed60d4259) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d76-2_82s147.9f", 0x000, 0x200, CRC(9c1d0512) SHA1(3ca82d4271badc890701ecc76b97e80b16509b50) )
	ROM_LOAD( "d76-1_82s147.7f", 0x200, 0x200, CRC(9a75349c) SHA1(2071132267aafd8facf1d7841093d9a45c30a8d3) )
ROM_END

/***************************************************************************

Mahjong Cafe Paradise
1999 Techno-Top

Royal Mahjong board. No roms on the base board.

Top board looks like typical Dynax with scratched SDIP64.
It is marked 'Techno Top Limited' and has just 2 eproms and 2 PROMs.
Everything else is scratched but there's a 32.768kHz OSC, RTC and connected battery.
Also, 4 DIP sw each with 10 switches and an 8MHz OSC next to the SDIP64 chip,
and a PLCC68 chip (likely FPGA)

***************************************************************************/

ROM_START( cafepara )
	ROM_REGION( 0x290000, "maincpu", 0 )
	ROM_LOAD( "cafepara.tmp91640", 0x000000, 0x004000, NO_DUMP )
	// VIDEO & AM MICRO COMPUTER SYSTEMS 1999 TECHNO-TOP,LIMITED NAGOYA JAPAN MAHJONG CAFE PARADISE TSS001 VER. 1.00
	ROM_LOAD( "00101.1h", 0x000000, 0x080000, CRC(f5917280) SHA1(e6180e36643075ab9fa5bc27baef2a464a23f581) )
	/* bank switched ROMs follow */
	ROM_RELOAD(           0x010000, 0x080000 )
	ROM_LOAD( "00102.1d", 0x090000, 0x200000, CRC(ed3b5447) SHA1(ac24e9c00c94c35d2b2ec35f0c4262ceeda5408f) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "ts001b.4h", 0x000, 0x200, CRC(b0019654) SHA1(78ba9b35744849c430f99137ea0da3d5564cc72a) )
	ROM_LOAD( "ts001a.4j", 0x200, 0x200, CRC(e89d4db0) SHA1(ff191a76fe1144e72a1cf3769f0156adf2d0507f) )
ROM_END

/***************************************************************************

Mahjong Vegas
Dynax, 199?

This game runs on Royal Mahjong hardware.

Top PCB
-------
D5011308L1
|-----------------------------------|
| DIP32 DIP32 5003  5002 DIP32 5001A|
|DIP40                          62XX|
|                         32.768kHz |
|                             RTC   |
|                   |-----------|   |
|                   |    CPU    |   |
|                   |-----------|   |
|DSW4   DSW2                        |
|    DSW3   DSW1           PAL      |
|                                   |
|                                   |
|                                   |
|    D50-2                          |
|*      D50-1            8MHz       |
|  %         &             BATTERY  |
|-----------------------------------|
Notes:
      Every chip has it's surface scratched
      *     - Cable connector joined to main PCB (to original PROM socket on main board)
      %     - RGB Video output
      &     - +12V input to top PCB
      DIP32 - Empty DIP32 socket
      DSWx  - have 10 switches each
      DIP40 - Socket joins to main PCB
      CPU   - unknown SDIP64 chip. Possibly TMP90P640 or similar TLCS-90 type CPU
              Pins 9-14 have been broken off and removed!
      62XX  - 6264 or 62256 SRAM
      D50-* - 82S147 color PROMs


Main PCB
--------
FRM-00 (with Falcon Logo.... PCB is made by Falcon)
|------------------------------------------------|
|  6116 DIP24 ROM6  ROM5  ROM4  ROM3  ROM2  ROM1 |
|HA1368                                          |
|                                                |
|     VOL                                DIP40   |
|                                                |
|             DSW(8)                    18.432MHz|
|M                                               |
|A                                               |
|H                                               |
|J                                               |
|O  AY3-8910                                     |
|N           4116    4116                        |
|G           4116    4116                        |
|2           4116    4116                        |
|8           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Socket joins to top PCB
      DIP24 - Unpopulated socket
      AY-3-8910 - clock 1.536MHz (18.432/12]
      ROM* - Unpopulated DIP24 sockets

***************************************************************************/

ROM_START( mjvegas )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "50xx.tmp90841", 0x00000, 0x02000, NO_DUMP )
	ROM_LOAD( "5001a.1b", 0x00000, 0x20000, CRC(91859a47) SHA1(3c452405bf28f5e7302eaccdf472e91b64629a67) )
	/* bank switched ROMs follow */
	ROM_RELOAD(           0x10000, 0x20000 )
	ROM_LOAD( "5002.1d",  0x30000, 0x80000, CRC(016c0a32) SHA1(5c5fdd631eacb36a0ee7dba9e070c2d3d3d8fd5b) )
	ROM_LOAD( "5003.1e",  0xb0000, 0x20000, CRC(5323cc85) SHA1(58b75ba560f05a0568024f52ee89f54713219452) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d50-2_82s147.4h", 0x000, 0x200, CRC(3c960ea2) SHA1(65e05e3f129e9e6fcb14b7d44a75a76919c54d52) )
	ROM_LOAD( "d50-1_82s147.4g", 0x200, 0x200, CRC(50c0d0ec) SHA1(222899456cd2e15391d8d0f771bbd5e5333d6ba3) )
ROM_END

ROM_START( mjvegasa )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )    // 100 banks
	ROM_LOAD( "5040.1b", 0x00000, 0x20000, CRC(c4f03128) SHA1(758567f74de333207dfe6c1cb72b2afffb0c8f4b) )
	/* bank switched ROMs follow */
	ROM_RELOAD(           0x070000, 0x20000 )   // 0c-0f
	ROM_LOAD( "5002.1d",  0x210000, 0x80000, CRC(016c0a32) SHA1(5c5fdd631eacb36a0ee7dba9e070c2d3d3d8fd5b) ) // 40-4f
	ROM_LOAD( "5003.1e",  0x2f0000, 0x20000, CRC(5323cc85) SHA1(58b75ba560f05a0568024f52ee89f54713219452) ) // 5c-5f

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d50-2_82s147.4h", 0x000, 0x200, CRC(3c960ea2) SHA1(65e05e3f129e9e6fcb14b7d44a75a76919c54d52) )
	ROM_LOAD( "d50-1_82s147.4g", 0x200, 0x200, CRC(50c0d0ec) SHA1(222899456cd2e15391d8d0f771bbd5e5333d6ba3) )
ROM_END

/***************************************************************************

Mahjong Shinkirou Deja Vu (+ some roms from Jan Oh (Toapan) !?)

This game runs on Royal Mahjong hardware.

Top PCB
-------
D210301BL2
|-----------------------------------|
| DIP32 DIP32 2104  2103  2102 2101 |
|DIP40                          6116|
|                                   |
|DSW3                               |
|  |-----------|                    |
|  |    CPU    |                    |
|  |-----------|                    |
|DSW4                               |
|             8MHz                  |
|                         DSW2  DSW1|
|                                   |
|                                   |
|                                   |
|*                                  |
|  %                                |
|-----------------------------------|
Notes:
      Every chip has it's surface scratched
      *     - Connector joined to main PCB
      %     - RGB Video output
      DIP32 - Empty DIP32 socket
      DSWx  - have 8 switches each
      DIP40 - Socket joins to main PCB
      CPU   - unknown SDIP64 chip. Possibly TMP90P640 or similar TLCS-90 type CPU


Main PCB
--------
FRM-00
|------------------------------------------------|
|  6116 DIP24 ROM6  ROM5  ROM4  ROM3  ROM2  ROM1 |
|HA1368                                          |
|                                                |
|                                        DIP40   |
|                                                |
|             DSW(8)                    18.432MHz|
|M                                               |
|A                                               |
|H                                               |
|J                                               |
|O  AY3-8910                                     |
|N           4116    4116                        |
|G           4116    4116                        |
|2           4116    4116                        |
|8           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Sockets joins to top PCB
      DIP24 - Unpopulated socket

***************************************************************************/

ROM_START( mjdejavu )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "2101.1b", 0x00000, 0x10000, CRC(b0426ea7) SHA1(ac39cbf5d78acdaa4b01d948917965c3aa2761b8) )
	/* bank switched ROMs follow */
	ROM_RELOAD(          0x10000, 0x08000 )
	ROM_CONTINUE(        0x10000, 0x08000 ) // 0
	// unused
	ROM_LOAD( "2103.1d", 0x30000, 0x20000, CRC(ed5fde4b) SHA1(d55487ae1007d43b71f06ae5c407c75db7054515) )   // 8
	// unused
	ROM_LOAD( "2104.1e", 0x70000, 0x20000, CRC(cfb8075d) SHA1(31f613a1a9b5f4295b552aeeddb760605ce2ac70) )   // 18
	// unused
	ROM_LOAD( "2102.1c", 0xb0000, 0x20000, CRC(f461e422) SHA1(c3505feb32650fdd5c0d7f30faed69b65d94937a) )   // 28

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "82s147.4d", 0x000, 0x200, CRC(d43f4c7c) SHA1(117d2e4e8d5bea3e5dc903a4b87bd71786ae009c) )
	ROM_LOAD( "82s147.4c", 0x200, 0x200, CRC(30cf7831) SHA1(b4593d51c6ceb301279a01a98665e4be8a3c403d) )
ROM_END

ROM_START( mjdejav2 )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "210a.1b", 0x00000, 0x10000, CRC(caa5c267) SHA1(c779f9217f56d9d3b1ee9fadca07f7917d203e8e) )
	/* bank switched ROMs follow */
	ROM_RELOAD(          0x10000, 0x08000 )
	ROM_CONTINUE(        0x10000, 0x08000 ) // 0
	// unused
	ROM_LOAD( "2103.1d", 0x30000, 0x20000, CRC(ed5fde4b) SHA1(d55487ae1007d43b71f06ae5c407c75db7054515) )   // 8
	// unused
	ROM_LOAD( "2104.1e", 0x70000, 0x20000, CRC(cfb8075d) SHA1(31f613a1a9b5f4295b552aeeddb760605ce2ac70) )   // 18
	// unused
	ROM_LOAD( "210b.1c", 0xb0000, 0x20000, CRC(d4383830) SHA1(491333277e5e2341d1c1cc20f8cc32aa6b020b6c) )   // 28

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "82s147.4d", 0x000, 0x200, CRC(d43f4c7c) SHA1(117d2e4e8d5bea3e5dc903a4b87bd71786ae009c) )
	ROM_LOAD( "82s147.4c", 0x200, 0x200, CRC(30cf7831) SHA1(b4593d51c6ceb301279a01a98665e4be8a3c403d) )
ROM_END

// Incomplete romset (missing rom7 at $6000): "Jan Oh" by Toaplan, on royalmah hardware (try pc=64f).
ROM_START( janoh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1.p1",  0x0000, 0x1000, CRC(8fc19963) SHA1(309e941c059a97b117090fd9dd69a00031aa6109) )    // "1984 JAN OH"
	ROM_LOAD( "rom2.p12", 0x1000, 0x1000, CRC(e1141ae1) SHA1(38f7a71b367a607bb20a5cbe62e7c87c96c6997c) )
	ROM_LOAD( "rom3.p2",  0x2000, 0x1000, CRC(66e6d2f4) SHA1(d7e00e5bfee60daf844c46d36b1f4860fba70759) )    // "JANOH TOAPLAN 84"
	ROM_LOAD( "rom4.p3",  0x3000, 0x1000, CRC(9186f02c) SHA1(b7dc2d6c19e67dd3f841cbb56df9589e3e6941f7) )
	ROM_LOAD( "rom5.p4",  0x4000, 0x1000, CRC(f3c478a8) SHA1(02a8504457cbcdd3e67e7f5ba60fb789f198a51d) )
	ROM_LOAD( "rom6.p5",  0x5000, 0x1000, CRC(92687327) SHA1(4fafba5881dca2a147616d94dd055eba6aa3c653) )
	ROM_LOAD( "rom7.p6",  0x6000, 0x1000, NO_DUMP )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "janho.color", 0x00, 0x20, NO_DUMP )
ROM_END

/***************************************************************************

Mahjong Cafe Break
Dynax/Nakanihon, 199?

This game runs on Royal Mahjong hardware with a Nakanihon top board

Top PCB Layout
--------------

NS528-9812
|-------------------------|
|X  NS528A2               |
|   NS528B2  PLCC84 528011|
|RGB                 DIP32|
|          8MHz      DIP32|
|DSW4(10)                 |
|DSW3(10)   SDIP64  6264  |
|DSW2(10)                 |
|DSW1(10)           52802 |
|                 A3      |
|BATTERY     A1   A2  Y A4|
|-------------------------|
Notes:
      RGB 3  - wire cable tied to mainboard
      X      - DIP16 socket with flat cable plugged in coming from main board PROM socket
      PLCC84 - unknown PLCC84 in a socket
      DIP32  - unpopulated DIP32 socket
      SDIP64 - unknown CPU, probably TLCS-90 (TMP91640)
      A1     - unknown DIP8 IC, possibly MB3771 reset/watchdog chip
      A2/A3  - unknown DIP14 ICs, probably logic
      A4     - unknown DIP18 IC, RTC IC
      Y      - 32.768kHz OSC for RTC

***************************************************************************/

ROM_START( cafebrk )
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD( "528.tmp91640", 0x000000, 0x004000, NO_DUMP )
	ROM_LOAD( "528011.1f",    0x000000, 0x080000, CRC(440ae60b) SHA1(c24efd76ba73adcb614b1974e8f92592800ba53c) )
	/* bank switched ROMs follow */
	ROM_LOAD( "52802.1d",     0x080000, 0x200000, CRC(bf4760fc) SHA1(d54ab9e298800a31d95a5f8b98ab9ba5b2866acf) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "ns528b2.4h", 0x000, 0x200, CRC(5699e69a) SHA1(fe13b93dd2c4a16865b4edcb0fee1390fdade725) )
	ROM_LOAD( "ns528a2.4j", 0x200, 0x200, CRC(b5a3a569) SHA1(8e31c600ae24b672b614908ee920a333ed600941) )
ROM_END

/*

Janou
(c)1985 Toaplan (distributed by SNK)

RM-1C (modified Royal Mahjong hardware)

CPU: Z80x2 (on subboard)
Sound: AY-3-8910
OSC: 18.432MHz

ROMs:
JO1
JO2
JO3
JO4
JO5
JO6
JO7


Subboard GX002A
C8 (2764)
18S030.44
18S030.45

HM6116
MSM2128x4


dumped by sayu
--- Team Japump!!! ---

*/

ROM_START( janoha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jo1",       0x0000, 0x1000, CRC(1a7dd28d) SHA1(347085c2b305861e4a4a602c3b3b0c57889f7f45) )
	ROM_LOAD( "jo2",       0x1000, 0x1000, CRC(e92ca79f) SHA1(9714ebee954dd98cf98b340e1dc424a4b2a78c36) )
	ROM_LOAD( "jo3",       0x2000, 0x1000, CRC(8e349cac) SHA1(27442fc97750ceb6e928682ee545a9ebff4511ac) )
	ROM_LOAD( "jo4",       0x3000, 0x1000, CRC(f2bcac9a) SHA1(46eea014edf9f260b35b5f9bd0fd0a0236da16ef) )
	ROM_LOAD( "jo5",       0x4000, 0x1000, CRC(16c09c73) SHA1(ea712f9ca3200ca27434e4200187b488e24f4c65) )
	ROM_LOAD( "jo6",       0x5000, 0x1000, CRC(92687327) SHA1(4fafba5881dca2a147616d94dd055eba6aa3c653) )
	ROM_LOAD( "jo7",       0x6000, 0x1000, CRC(f9a3fea6) SHA1(898c030b34f7432568e080e2814619d836d98a2f) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "c8",       0x0000, 0x2000, CRC(a37ed493) SHA1(a3246c635ee77f96afd96285ef7091f6fc0d7636) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "18s030.44",  0x0000, 0x0020, CRC(d4eabf78) SHA1(f14778b552ff483e36e7c30ee67e8e2075790ea2) )
	ROM_LOAD( "18s030.45",  0x0020, 0x0020, CRC(c6a24ae9) SHA1(ec7a4dee2fec2f7151ddc39e40a3eee6a1c4992d) ) // another color prom?
ROM_END

/*

Mahjong Shiyou (BET type)
(c)1986 Visco

Board:  S-0086-001-00
CPU:    Z80-A x2
Sound:  AY-3-8910
        M5205
OSC:    18.432MHz
        400KHz


1.1K       Z80#2 prg.
2.1G

3.3G       Z80#1 prg.
4.3F

COLOR.BPR  color

*/

ROM_START( mjsiyoub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.3g", 0x00000, 0x8000, CRC(47d0f16e) SHA1(a125be052668ba93756bf940af31a10e91a3d307) )
	ROM_LOAD( "4.3f", 0x08000, 0x8000, CRC(6cd6a200) SHA1(1c53e5caacdb9c660bd98f5331bf5354581f74c9) )

	/*encrypted z80*/
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "1.1k", 0x00000, 0x8000, CRC(a1083321) SHA1(b36772e90be60270234df16cf92d87f8d950190d) )
	ROM_LOAD( "2.1g", 0x08000, 0x4000, CRC(cfe5de1d) SHA1(4acf9a752aa3c02b0889b0b49d3744359fa24460) )

	ROM_REGION( 0x40000, "proms", 0 )
	ROM_LOAD( "color.bpr", 0x00, 0x20,  CRC(d21367e5) SHA1(b28321ac8f99abfebe2ef4da0c751cefe9f3f3b6) )
ROM_END

/*

Mahjong Senka
(c)1986 Visco

Modified Royal Mahjong Hardware

CPU: Z80 <- wrong,they are 2 z80 CPUs -AS
Sound: AY-3-8910
OSC: 18.432MHz
Others: Battery

ROMs:
1
2
3
4
1.2L (N82S129N)
2.2K (N82S123N)
3.1D (N82S129N)
4.8K (N82S123N) - color PROM


dumped by sayu

--- Team Japump!!! ---
http://japump.i.am/

*/

ROM_START( mjsenka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3",       0x0000, 0x4000, CRC(b2d8be1f) SHA1(da75e1072d271de2dbd897a551f6c32593f6421b) )
	ROM_LOAD( "4",       0x4000, 0x2000, CRC(e9e84999) SHA1(7b5f0edd92cf3a45e85055460e6cb00b154fd152) )
	ROM_LOAD( "2",       0x6000, 0x2000, CRC(cdb02fc5) SHA1(5de6b15b79ea7c4246a294b17f166e53be6a4abc) )

	/*encrypted z80*/
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "1",       0x0000, 0x2000, CRC(83e943d1) SHA1(c4f9b5036627ccb369e7db03a743e496b149af85) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "4.8k",  0x0000, 0x0020, CRC(41bd4d69) SHA1(4d2da761b338b62b2ea151c201063a24d6e4cc97) )
	ROM_LOAD( "2.2k",  0x0020, 0x0020, CRC(46014727) SHA1(eec451f292ee319fa6bfbbf223aaa12b231692c1) )

	ROM_REGION( 0x0200, "user1", 0 ) //?
	ROM_LOAD( "1.2l",  0x0000, 0x0100, CRC(24599429) SHA1(6c93bb2e7bc9902cace0c9d482fc1584c4c1a114) )
	ROM_LOAD( "3.1d",  0x0100, 0x0100, CRC(86aeafd1) SHA1(c4e5c56ce5baf2be3962675ae333e28bd8108a00) )
ROM_END

/*

Mahjong Yarou
(c)1986 Visco/Video System

FRM-00 (modified royal mahjong hardware)

CPU: Z80 (on subboard) <- wrong,they are 2 z80 CPUs -AS
Sound: AY-3-8910
OSC: 18.432MHz

ROMs:
1(2732)
2(2732)
3(2732)
4(2732)
5(2732)
6(2732)
4.6K (18S030) - pin14 is connected to subboard's WS
                pin9 is not inserted to the socket

Subboard:
7(2732)
8(2764)
N82S129N.IC4
N82S123N.IC7
N82S129N.IC15

Connetor between mainboard and subboard
sub - main
 CK - LS368 (1K) pin12
 HD - LS08  (2E) pin1
 VD - LS08  (2E) pin2
 WS - 18S030(6K) pin14
 () - LS138 (3K) pin13


Mainboard
----------------------------------------------------------
    1         2       3       4       5        6       7
A 74LS04    74LS86  74LS153  MB8116  MB8116  74LS157
B 74LS161   74LS86  74LS153  MB8116  MB8116  74LS95
C 74LS161   74LS86  74LS153  MB8116  MB8116  74LS157
D 74LS74    74LS86  74LS153  MB8116  MB8116  74LS95    8
E 74LS161   74LS08  74LS153  MB8116  MB8116  74LS157   9
F 74LS161   74LS74  74LS00   MB8116  MB8116  74LS95    1
H 74LS74    74LS00  74LS175  MB8116  MB8116  74LS157   0
J 74LS107   74LS32  74LS10   MB8116  MB8116  74LS95
K 74LS368   74LS241 74LS138  74LS08  74LS174 4.6K
L 18.432MHz 74LS241 74LS138  74LS04  74LS244 74LS174
M (socket to subbd) 74LS367  74LS08  DIPSW   74LS368
N                   (74LS245)74LS138 74LS04  TC40H000P

  1     2     3     4     5     6                      6 B
                                                       1 A
                                                       1 T
                                                       6 T
----------------------------------------------------------

Subboard
-----------------------------------------------------------
74LS42(IC21)   ?(IC22)        ?(IC23)        74LS85(IC24)
74LS125(IC16)  74LS08(IC17)   74LS393(IC9)   82S129N(IC15)
74LS161(IC6)   82S123N(IC7)   74LS161(IC8)   74LS157(IC14)
82S129N(IC4)   74LS259(IC5)   74LS32(IC12)   74LS74(IC13)
7(IC2)                        PAL20X10(IC19) 74LS00(IC20)
8(IC3)                        74LS245(IC18)  DIPSW
                                             74LS32(IC11)
Z80A                                         74LS04(IC10)
                                             5pin connector
-----------------------------------------------------------


dumped by sayu

--- Team Japump!!! ---
http://japump.i.am/

*/

ROM_START( mjyarou )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",       0x0000, 0x1000, CRC(312c3b29) SHA1(ec2e14b392cf761f0a7079376994418fd463a06c) )
	ROM_LOAD( "2",       0x1000, 0x1000, CRC(98f14097) SHA1(cd1f72d6effa50f95386dfc5fa9b5056d83e554f) )
	ROM_LOAD( "3",       0x2000, 0x1000, CRC(295dbf40) SHA1(d6ac7bd88da849e418e750e2c91a594f65bdff39) )
	ROM_LOAD( "4",       0x3000, 0x1000, CRC(a6a078c8) SHA1(936be36c7c938c705e7054a42c1908bb5a5ee1bb) )
	ROM_LOAD( "5",       0x4000, 0x1000, CRC(3179657e) SHA1(703fc57ae71554345754267c31809cf7af7f1639) )
	ROM_LOAD( "6",       0x5000, 0x1000, CRC(6ccc05b4) SHA1(6eefba6023673edd86e82a0ad861a4d8f7f6652b) )
	ROM_LOAD( "8",       0x6000, 0x2000, CRC(1adef246) SHA1(b5f5598daf71694effffbfb486b03fcda5a593ee) ) //might be a rom for the sub cpu.

	/*encrypted z80*/
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "7",       0x0000, 0x1000, CRC(dd144b90) SHA1(56b2c4472aaec49d9fddc99d8aa718b17655812c) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "4.6k",         0x0000, 0x0020, CRC(41bd4d69) SHA1(4d2da761b338b62b2ea151c201063a24d6e4cc97) )
	ROM_LOAD( "82s123n.ic7",  0x0020, 0x0020, CRC(46014727) SHA1(eec451f292ee319fa6bfbbf223aaa12b231692c1) )

	ROM_REGION( 0x0200, "user1", 0 ) //?
	ROM_LOAD( "82s129n.ic15",  0x0000, 0x0100, CRC(86aeafd1) SHA1(c4e5c56ce5baf2be3962675ae333e28bd8108a00) )
	ROM_LOAD( "82s129n.ic4",   0x0100, 0x0100, CRC(f09d3c4c) SHA1(a9e752d75e7f3ebd05add4ccf2f9f15d8f9a8d15) )
ROM_END

/*

Jansou
(c)1985 Dyna Industry

upgrade kit for Royal Mahjong
G85-12-05RL

CPU: Z80A
OSC: 4.000MHz
Other: surface scratched 40pin DIP device

ROMs:
1
2
3
4
5

Color PROM:
N82S123AN


dumped by sayu
--- Team Japump!!! ---
http://japump.i.am/

*/
/*
Nothing can be done on this one due of missing main program rom(s) (surface scratched 40pin DIP device).
A string of the current z80 rom at offset 0x90 says "THE Janso Voice Version 1.0 (c) Copy Right 1985 Dyna",
so it's just a voice player.
*/
ROM_START( jansou )
	/*Missing main cpu program rom*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk", 0x0000, 0x8000, NO_DUMP )

	/*These probably hooks up with the main cpu program,they are standard 4bpp bitmaps.*/
	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "3", 0x00000, 0x8000, CRC(75a0bef0) SHA1(c2f5b3ddc55b58d3ea784d8b3d0a0f577d313341) )
	ROM_LOAD( "4", 0x08000, 0x8000, CRC(7304899a) SHA1(636b7673563f75ff2ef95eef3b99f80ef0c45fee) )
	ROM_LOAD( "5", 0x10000, 0x8000, CRC(57a4d300) SHA1(35d211d50052cd76721dbd6ad02ec7cb56c475d1) )

	/*this is just a z80 Voice Player (and latches port I/O $00 with the main CPU)*/
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1", 0x00000, 0x8000, CRC(0321ac7e) SHA1(1a0372a25f979461db09cd153c15daaa556c3d1f) )
	ROM_LOAD( "2", 0x08000, 0x8000, CRC(fea7f3c6) SHA1(c196be0030b00cfb747b9dbfa387048d20c70b74) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "n82s123an", 0x0000, 0x0020, CRC(e9598146) SHA1(619e7eb76cc3e882b5b3e55cdd23fe00b0a1fe45) )
ROM_END

ROM_START( jansoua )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x1000, CRC(aa66a9fc) SHA1(e2a956f17d294e160e16297866cd9be117254ea4) )
	ROM_LOAD( "2.bin",  0x1000, 0x1000, CRC(3b6ef098) SHA1(eda181971153888e63aa14e10b0b199383f2d627) )
	ROM_LOAD( "3.bin",  0x2000, 0x1000, CRC(63070d44) SHA1(c9c08f774a94cfb4e291f3d7ef81b0f0f9f74460) )
	ROM_LOAD( "4.bin",  0x3000, 0x1000, CRC(2b14d3c1) SHA1(210d6f212bda7fb7225e5606b34f674cc5f85150) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "3s.bin", 0x00000, 0x8000, CRC(64df20f6) SHA1(6cbe4718d47b52c229863219dba3e1f964ba667a) )
	ROM_LOAD( "4s.bin", 0x08000, 0x8000, CRC(8ddc8258) SHA1(a97a5efd06965a70e34684986dd8538a35e43d31) )
	ROM_LOAD( "5s.bin", 0x10000, 0x8000, CRC(1745c996) SHA1(6905774b4bdd0bfcc34b847efb037f9d92884a6b) )

	/*this is just a z80 Voice Player (and latches port I/O $00 with the main CPU)*/
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1",  0x00000, 0x8000, CRC(0321ac7e) SHA1(1a0372a25f979461db09cd153c15daaa556c3d1f) )
	ROM_LOAD( "2",  0x08000, 0x8000, CRC(fea7f3c6) SHA1(c196be0030b00cfb747b9dbfa387048d20c70b74) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "n82s123an", 0x0000, 0x0020, CRC(e9598146) SHA1(619e7eb76cc3e882b5b3e55cdd23fe00b0a1fe45) )
ROM_END


DRIVER_INIT_MEMBER(royalmah_state,ippatsu)
{
	membank("bank1")->set_base(memregion("maincpu")->base() + 0x8000 );
}

DRIVER_INIT_MEMBER(royalmah_state,janptr96)
{
	m_janptr96_nvram = auto_alloc_array(machine(), UINT8, 0x1000 * 9);
	membank("bank3")->set_base(m_janptr96_nvram);
	machine().device<nvram_device>("nvram")->set_base(m_janptr96_nvram, 0x1000 * 9);
}

GAME( 1981,  royalmj,  0,        royalmah, royalmah, driver_device,  0,        ROT0,   "Nichibutsu",                 "Royal Mahjong (Japan, v1.13)",          0 )
GAME( 1981?, openmj,   royalmj,  royalmah, royalmah, driver_device,  0,        ROT0,   "Sapporo Mechanic",           "Open Mahjong [BET] (Japan)",            0 )
GAME( 1982,  royalmah, royalmj,  royalmah, royalmah, driver_device,  0,        ROT0,   "bootleg",                    "Royal Mahjong (Falcon bootleg, v1.01)", 0 )
GAME( 1983,  janyoup2, royalmj,  ippatsu,  janyoup2, driver_device,  0,        ROT0,   "Cosmo Denshi",               "Janyou Part II (ver 7.03, July 1 1983)",0 )
GAME( 1985,  tahjong,  royalmj,  tahjong,  tahjong,  driver_device,  0,        ROT0,   "Bally Pond / Nasco",         "Tahjong Yakitori (ver. 2-1)",           0 ) // 1985 Jun. 17
GAME( 1981,  janputer, 0,        royalmah, royalmah, driver_device,  0,        ROT0,   "bootleg (Public Software Ltd. / Mes)", "New Double Bet Mahjong (bootleg of Janputer)", 0 ) // the original Janputer (Sanritsu) is not yet dumped
GAME( 1984,  janoh,    0,        royalmah, royalmah, driver_device,  0,        ROT0,   "Toaplan",                    "Jan Oh (set 1)",                        MACHINE_NOT_WORKING )
GAME( 1984,  janoha,   janoh,    janoh,    royalmah, driver_device,  0,        ROT0,   "Toaplan",                    "Jan Oh (set 2)",                        MACHINE_NOT_WORKING ) // this one is complete?
GAME( 1985,  jansou,   0,        jansou,   jansou,   driver_device,  0,        ROT0,   "Dyna",                       "Jansou (set 1)",                        MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
GAME( 1985,  jansoua,  jansou,   jansou,   jansou,   driver_device,  0,        ROT0,   "Dyna",                       "Jansou (set 2)",                        0 )
GAME( 1986,  dondenmj, 0,        dondenmj, majs101b, driver_device,  0,        ROT0,   "Dyna Electronics",           "Don Den Mahjong [BET] (Japan)",         0 )
GAME( 1986,  ippatsu,  0,        ippatsu,  ippatsu,  royalmah_state, ippatsu,  ROT0,   "Public Software / Paradais", "Ippatsu Gyakuten [BET] (Japan)",        0 )
GAME( 1986,  suzume,   0,        suzume,   suzume,   driver_device,  0,        ROT0,   "Dyna Electronics",           "Watashiha Suzumechan (Japan)",          0 )
GAME( 1986,  mjsiyoub, 0,        royalmah, royalmah, driver_device,  0,        ROT0,   "Visco",                      "Mahjong Shiyou (Japan)",                MACHINE_NOT_WORKING )
GAME( 1986,  mjsenka,  0,        royalmah, royalmah, driver_device,  0,        ROT0,   "Visco",                      "Mahjong Senka (Japan)",                 MACHINE_NOT_WORKING )
GAME( 1986,  mjyarou,  0,        royalmah, royalmah, driver_device,  0,        ROT0,   "Visco / Video System",       "Mahjong Yarou [BET] (Japan)",           MACHINE_NOT_WORKING )
GAME( 1986?, mjclub,   0,        mjclub,   mjclub,   driver_device,  0,        ROT0,   "Xex",                        "Mahjong Club [BET] (Japan)",            0 )
GAME( 1987,  mjdiplob, 0,        mjdiplob, mjdiplob, driver_device,  0,        ROT0,   "Dynax",                      "Mahjong Diplomat [BET] (Japan)",        0 )
GAME( 1987,  tontonb,  0,        tontonb,  tontonb,  driver_device,  0,        ROT0,   "Dynax",                      "Tonton [BET] (Japan set 1)",            0 )
GAME( 1987,  makaijan, 0,        makaijan, makaijan, driver_device,  0,        ROT0,   "Dynax",                      "Makaijan [BET] (Japan)",                0 )
GAME( 1988,  majs101b, 0,        majs101b, majs101b, driver_device,  0,        ROT0,   "Dynax",                      "Mahjong Studio 101 [BET] (Japan)",      0 )
GAME( 1988,  mjapinky, 0,        mjapinky, mjapinky, driver_device,  0,        ROT0,   "Dynax",                      "Almond Pinky [BET] (Japan)",            0 )
GAME( 1989,  mjdejavu, 0,        mjdejavu, mjdejavu, driver_device,  0,        ROT0,   "Dynax",                      "Mahjong Shinkirou Deja Vu (Japan)",     0 )
GAME( 1989,  mjdejav2, mjdejavu, mjdejavu, mjdejavu, driver_device,  0,        ROT0,   "Dynax",                      "Mahjong Shinkirou Deja Vu 2 (Japan)",   0 )
GAME( 1989,  mjderngr, 0,        mjderngr, mjderngr, driver_device,  0,        ROT0,   "Dynax",                      "Mahjong Derringer (Japan)",             0 )
GAME( 1989,  daisyari, 0,        daisyari, daisyari, driver_device,  0,        ROT0,   "Best System",                "Daisyarin [BET] (Japan)",               0 )
GAME( 1990,  mjifb,    0,        mjifb,    mjifb,    driver_device,  0,        ROT0,   "Dynax",                      "Mahjong If...? [BET]",                  0 )
GAME( 1990,  mjifb2,   mjifb,    mjifb,    mjifb,    driver_device,  0,        ROT0,   "Dynax",                      "Mahjong If...? [BET](2921)",            0 )
GAME( 1990,  mjifb3,   mjifb,    mjifb,    mjifb,    driver_device,  0,        ROT0,   "Dynax",                      "Mahjong If...? [BET](2931)",            0 )
GAME( 1991,  mjvegasa, 0,        mjvegasa, mjvegasa, driver_device,  0,        ROT0,   "Dynax",                      "Mahjong Vegas (Japan, unprotected)",    0 )
GAME( 1991,  mjvegas,  mjvegasa, mjvegasa, mjvegasa, driver_device,  0,        ROT0,   "Dynax",                      "Mahjong Vegas (Japan)",                 MACHINE_NOT_WORKING )
GAME( 1992,  cafetime, 0,        cafetime, cafetime, driver_device,  0,        ROT0,   "Dynax",                      "Mahjong Cafe Time",                     0 )
GAME( 1993,  cafedoll, 0,        mjifb,    mjifb,    driver_device,  0,        ROT0,   "Dynax",                      "Mahjong Cafe Doll (Japan)",             MACHINE_NOT_WORKING )
GAME( 1995,  mjtensin, 0,        mjtensin, mjtensin, driver_device,  0,        ROT0,   "Dynax",                      "Mahjong Tensinhai (Japan)",             MACHINE_NOT_WORKING )
GAME( 1996,  janptr96, 0,        janptr96, janptr96, royalmah_state, janptr96, ROT0,   "Dynax",                      "Janputer '96 (Japan)",                  0 )
GAME( 1997,  janptrsp, 0,        janptr96, janptr96, royalmah_state, janptr96, ROT0,   "Dynax",                      "Janputer Special (Japan)",              0 )
GAME( 1999,  cafebrk,  0,        mjifb,    mjifb,    driver_device,  0,        ROT0,   "Nakanihon / Dynax",          "Mahjong Cafe Break",                    MACHINE_NOT_WORKING )
GAME( 1999,  cafepara, 0,        mjifb,    mjifb,    driver_device,  0,        ROT0,   "Techno-Top",                 "Mahjong Cafe Paradise",                 MACHINE_NOT_WORKING )
