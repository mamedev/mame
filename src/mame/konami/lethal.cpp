// license:BSD-3-Clause
// copyright-holders:R. Belmont, Nicola Salmoria
/***************************************************************************

 Lethal Enforcers
 (c) 1992 Konami
 Driver by R. Belmont and Nicola Salmoria.

 This hardware is exceptionally weird - they have a bunch of chips intended
 for use with a 68000 hooked up to an 8-bit CPU.  So everything is bankswitched
 like crazy.

 LETHAL ENFORCERS
 KONAMI 1993

            84256         053245A
                191A03.A4     6116     191A04.A8 191A05.A10
                     054539             053244A   053244A
       Z80B  191A02.F4
                6116
                                          191A06.C9
                               7C185
                               7C185
                007644            5116
                054000            4464
                4464              4464
                191E01.U4         5116      054157  054157
                63C09EP           054156
       24MHZ                                 191A07.V8 191A08.V10
                                             191A09.V9 191A10.V10



---


Lethal Enforcers (c) Konami 1992
GX191 PWB353060A

Dump of USA program ROMs only.

Label   CRC32       Location    Code        Chip Type
  1   [72b843cc]      F4        Z80     TMS 27C512 (64k)
  6   [1b6b8f16]      U4        6309        ST 27C4001 (512k)

At offset 0x3FD03 in 6_usa.u4 is "08/17/92 21:38"

Run down of PCB:
Main CPU:  HD63C09EP
    OSC 24.00000MHz near 6309

Sound CPU:  Z80 (Zilog Z0840006PSC)
    OSC 18.43200MHz near Z80, 054968A & 054539

Konami Custom chips:

054986A (sound latch + Z80 memory mapper/banker + output DAC)
054539  (sound)
054000  (collision/protection)
053244A (x2) (sprites)
053245A (sprites)
054156 (tilemaps)
054157 (x2) (tilemaps)
007324 (???)

All other ROMs surface mounted (not included):

Label   Printed*    Position
191 A03 Mask16M-8bit - Near 054986A & 054539 - Sound - Also labeled as 056046

191A04  Mask8M-16bit \ Near 053244A (x2) & 05245A - Sprites
191A05  Mask8M-16bit /
191 A06 Mask8M-16bit - Also labeled as 056049

191A07  Mask8M-16bitx4 \
191A08  Mask8M-16bitx4  | Near 054157 (x2) & 054156 - Tiles
191A09  Mask8M-16bitx4  |
191A10  Mask8M-16bitx4 /

* This info is printed/silk-screened on to the PCB for assembly information?


4 way Dip Switch

---------------------------------------------------
 DipSwitch Title   |   Function   | 1 | 2 | 3 | 4 |
---------------------------------------------------
   Sound Output    |    Stereo    |off|           |
                   |   Monaural   |on |           |
---------------------------------------------------
  Coin Mechanism   |   Common     |   |off|       |
                   | Independent  |   |on |       |
---------------------------------------------------
    Game Type      |   Street     |       |off|   |
                   |   Arcade     |       |on |   |
---------------------------------------------------
    Language*      |   English    |           |off|
                   |   Spanish    |           |on |
---------------------------------------------------
     Default Settings             |off|off|on |off|
---------------------------------------------------
 NOTE: Listed as "NOT USED" in UK / World Manual  |
---------------------------------------------------

Push Button Test Switch


Memory map (from the schematics)

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
000xxxxxxxxxxxxx R   xxxxxxxx PROM      program ROM (banked)
001xxxxxxxxxxxxx R/W xxxxxxxx WRAM      work RAM
010000--00xxxxxx   W xxxxxxxx VREG      054156 control
010000--01--xxxx   W xxxxxxxx VSCG      054157 control
010000--1000---- R/W -------- AFR       watchdog reset
010000--1001----   W          SDON      sound enable?
010000--1010                  CCLR      ?
010000--1011----              n.c.
010000--11-000--   W ------xx COIN1/2   coin counters
010000--11-000--   W ----xx-- CKO1/2    coin enables?
010000--11-000--   W ---x---- VRD       \ related to reading graphics ROMs?
010000--11-000--   W --x----- CRDB      /
010000--11-001--   W -----xxx EEP       EEPROM DI, CS, CLK
010000--11-001--   W ----x--- MUT       sound mute?
010000--11-001--   W ---x---- CBNK      bank switch 4400-7FFF region between palette and 053245/054156
010000--11-001--   W --x----- n.c.
010000--11-001--   W xx------ SHD0/1    shadow control
010000--11-010--   W -----xxx PCU1/XBA  palette bank (tilemap A)
010000--11-010--   W -xxx---- PCU1/XBB  palette bank (tilemap B)
010000--11-011--   W -----xxx PCU2/XBC  palette bank (tilemap C)
010000--11-011--   W -xxx---- PCU2/XBD  palette bank (tilemap D)
010000--11-100--   W -----xxx PCU3/XBO  palette bank (sprites)
010000--11-100--   W -xxx---- PCU3/XBK  palette bank (background?)
010000--11-101xx R   xxxxxxxx POG       gun inputs
010000--11-11000 R   xxxxxxxx SW        dip switches, EEPROM DO, R/B
010000--11-11001 R   xxxxxxxx SW        inputs
010000--11-11010 R   xxxxxxxx SW        unused inputs (crossed out in schematics)
010000--11-11011 R   xx------ HHI1/2    gun input ready?
010000--11-11011 R   -------x NCPU      ?
010000--11-111--   W --xxxxxx BREG      ROM bank select
010010--00------              n.c.
010010--01---xxx R/W xxxxxxxx OREG      053244/053245 control
010010--10-xxxxx R/W xxxxxxxx HIP       054000
010010--11       R/W xxxxxxxx PAR       sound communication
010100xxxxxxxxxx R/W xxxxxxxx OBJ       053245 sprite RAM
011xxxxxxxxxxxxx R/W xxxxxxxx VRAM      054156 video RAM
1xxxxxxxxxxxxxxx R   xxxxxxxx PROM      program ROM


Lethal Enforcers
Konami 1992

PCB Layout
----------
GX191 PWB353060A
|--------------------------------------------------------|
|LA4705           191A03.A4  |------|191A04.A8 191A05.A10|
|CN6     84256     |------|  |053245A                    |
|         18.432MHz|054539|  |      |  |------| |------| |
| 054986A          |      |  |      |  |053244A |053244A |
|                  |      |  |------|  |      | |      | |
|                  |------|            |      | |      | |
|     Z80B        191A02.F4   LH5116   |------| |------| |
|                  LH5116                                |
|                                        191A06.G9       |
|                                                        |
|                                                        |
|J 051550                                                |
|A                            CY7C185                    |
|M 052535                     CY7C185                    |
|M 052535                                                |
|A 052535                                                |
|                                                        |
|                                                        |
|                                                        |
|                  054884     LH5116                     |
|                             MN4464                     |
|     005273                  MN4464   |------| |------| |
|     005273                  MN4464   |054157| |054157| |
| DSW(4)                      LH5116   |      | |      | |
|      ER5911.Q2   007644     |------| |      | |      | |
|TEST_SW           054000     |054156| |------| |------| |
|                  MN4464     |      |                   |
|                191UAD01.U4  |      |                   |
|                   007324    |------|                   |
|  CN8                              191A07.V8  191A08.V10|
|  CN7   24MHz   HD63C09EP          191A09.X8  191A10.X10|
|--------------------------------------------------------|
Notes:
      63C09EP - Clock 3.000MHz [24/8]
      Z80B    - Clock 6.000MHz [24/4]
      84256   - Fujitsu 84256 32kx8 SRAM (DIP28)
      LH5116  - Sharp LH5116 2kx8 SRAM (DIP24)
      CY7C185 - Cypress CY7C185 8kx8 SRAM (DIP28)
      MN4464  - Panasonic MN4464 8kx8 SRAM (DIP28)
      ER5911  - EEPROM (128 bytes)
      CN6     - 4 pin connector for stereo sound output
      CN7/CN8 - 4 pin connectors for standard light guns
                Pin numbering from left to right is 4 3 2 1
                Pin 1 - Opto
                Pin 2 - Ground
                Pin 3 - Trigger
                Pin 4 - +5v
      191*    - EPROM/mask ROM
      LA4705  - 15W 2-channel BTL audio power AMP

      Custom Chips
      ------------
      054000  - Collision/protection
      007324  - Resistor array package containing eight 150 ohm resistors. The IC looks like a DIP16 logic chip
                but with an epoxy top. The schematics show it connected to the 6309 data lines (D0-D7), main
                8k program RAM (D0-D7) and the 054000. It is a simple resistor array (x8)
      007644  - ? (DIP22)
      054157  \
      054156  / Tilemap generators
      053244A \
      053245A / Sprite generators
      054539  - 8-Channel ADPCM sound generator. Clock input 18.432MHz. Clock outputs 18.432/4 & 18.432/8
      052535  - Video DAC (one for each R,G,B video signal)
      051550  - EMI filter for credit/coin counter
      005273  - Resistor array for gun trigger and 1 player/2 player start
      054884  - MMI PAL16L8
      054986A - Audio DAC/filter + sound latch + Z80 memory mapper/banker (large ceramic SDIP64 module)
                This module contains several surface mounted capacitors and resistors, 4558 OP amp,
                Analog Devices AD1868 dual 18-bit audio DAC and a Konami 054321 QFP44 IC.

      Sync Measurements
      -----------------
      HSync - 15.2038kHz
      VSync - 59.6380Hz


note:

Lethal Enforcers has two sprite rendering chips working in parallel with their
output mixed to give 6bpp, and two tilemap rendering chips working in parallel
to give 8bpp. We currently cheat, using just one of each device but using
alternate gfx layouts. Emulating it accurately will require separating the
"front end" chips (053245, 054156) from the "back end" chips (053244, 054157)
as only the latter are doubled.

mirror not set up correctly

maybe some sprite placement issues

***************************************************************************/

#include "emu.h"

#include "k053244_k053245.h"
#include "k054156_k054157_k056832.h"
#include "k054000.h"
#include "konami_helper.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m6809/hd6309.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/bankdev.h"
#include "machine/k054321.h"
#include "sound/k054539.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class lethal_state : public driver_device
{
public:
	lethal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_bank4000(*this, "bank4000"),
		m_k056832(*this, "k056832"),
		m_k053244(*this, "k053244"),
		m_k054321(*this, "k054321"),
		m_palette(*this, "palette")
	{ }

	void lethalej(machine_config &config);
	void lethalen(machine_config &config);

private:
	/* video-related */
	int        m_layer_colorbase[4]{};
	int        m_sprite_colorbase = 0;
	int        m_back_colorbase = 0;

	/* misc */
	uint8_t      m_cur_control2 = 0U;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<address_map_bank_device> m_bank4000;
	required_device<k056832_device> m_k056832;
	required_device<k05324x_device> m_k053244;
	required_device<k054321_device> m_k054321;
	required_device<palette_device> m_palette;

	void control2_w(uint8_t data);
	uint8_t sound_irq_r();
	void sound_irq_w(uint8_t data);
	void le_bankswitch_w(uint8_t data);
	uint8_t guns_r(offs_t offset);
	uint8_t gunsaux_r();
	void lethalen_palette_control(offs_t offset, uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_lethalen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(lethalen_interrupt);
	K05324X_CB_MEMBER(sprite_callback);
	K056832_CB_MEMBER(tile_callback);
	void bank4000_map(address_map &map) ATTR_COLD;
	void le_main(address_map &map) ATTR_COLD;
	void le_sound(address_map &map) ATTR_COLD;
};


K05324X_CB_MEMBER(lethal_state::sprite_callback)
{
	int pri = (*color & 0xfff0);
	*color = *color & 0x000f;
	*color += m_sprite_colorbase;

	/* this isn't ideal.. shouldn't need to hardcode it? not 100% sure about it anyway*/
	if (pri == 0x10)
		*priority = 0xf0; // guys on first level
	else if (pri == 0x90)
		*priority = 0xf0; // car doors
	else if (pri == 0x20)
		*priority = 0xf0 | 0xcc; // people behind glass on 1st level
	else if (pri == 0xa0)
		*priority = 0xf0 | 0xcc; // glass on 1st/2nd level
	else if (pri == 0x40)
		*priority = 0; // blood splats?
	else if (pri == 0x00)
		*priority = 0; // gunshots etc
	else if (pri == 0x30)
		*priority = 0xf0 | 0xcc | 0xaa; // mask sprites (always in a bad colour, used to do special effects i think
	else
	{
		popmessage("unknown pri %04x\n", pri);
		*priority = 0;
	}

	*code = (*code & 0x3fff); // | spritebanks[(*code >> 12) & 3];
}

K056832_CB_MEMBER(lethal_state::tile_callback)
{
	*color = m_layer_colorbase[layer] + ((*color & 0x3c) << 2);
}

void lethal_state::video_start()
{
	// this game uses external linescroll RAM
	m_k056832->SetExtLinescroll();

	// the US and Japanese cabinets apparently use different mirror setups
	if (!strcmp(machine().system().name, "lethalenj"))
	{
		m_k056832->set_layer_offs(0, 29, 0);
		m_k056832->set_layer_offs(1, 31, 0);
		m_k056832->set_layer_offs(2, 33, 0);
		m_k056832->set_layer_offs(3, 35, 0);
	}
	else
	{
		m_k056832->set_layer_offs(0, 188, 0);
		m_k056832->set_layer_offs(1, 190, 0);
		m_k056832->set_layer_offs(2, 192, 0);
		m_k056832->set_layer_offs(3, 194, 0);
	}
}

void lethal_state::lethalen_palette_control(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // 40c8 - PCU1 from schematics
			m_layer_colorbase[0] = (data & 0x7) * 1024 / 16;
			m_layer_colorbase[1] = ((data >> 4) & 0x7) * 1024 / 16;
			m_k056832->mark_plane_dirty( 0);
			m_k056832->mark_plane_dirty( 1);
			break;

		case 4: // 40cc - PCU2 from schematics
			m_layer_colorbase[2] = (data & 0x7) * 1024 / 16;
			m_layer_colorbase[3] = ((data >> 4) & 0x7) * 1024 / 16;
			m_k056832->mark_plane_dirty( 2);
			m_k056832->mark_plane_dirty( 3);
			break;

		case 8: // 40d0 - PCU3 from schematics
			m_sprite_colorbase = (data & 0x7) * 1024 / 64;
			m_back_colorbase = ((data >> 4) & 0x7) * 1024 + 1023;
			break;
	}
}

uint32_t lethal_state::screen_update_lethalen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_back_colorbase, cliprect);
	screen.priority().fill(0, cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 3, K056832_DRAW_FLAG_MIRROR, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 2, K056832_DRAW_FLAG_MIRROR, 2);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, K056832_DRAW_FLAG_MIRROR, 4);

	m_k053244->sprites_draw(bitmap, cliprect, screen.priority());

	// force "A" layer over top of everything
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, K056832_DRAW_FLAG_MIRROR, 0);

	return 0;
}


static const char *const gunnames[] = { "LIGHT0_X", "LIGHT0_Y", "LIGHT1_X", "LIGHT1_Y" };

/* a = 1, 2 = player # */
#define GUNX( a ) (( ( ioport(gunnames[2 * (a - 1)])->read() * 287 ) / 0xff ) + 16)
#define GUNY( a ) (( ( ioport(gunnames[2 * (a - 1) + 1])->read() * 223 ) / 0xff ) + 10)

void lethal_state::control2_w(uint8_t data)
{
	/* bit 0 is data */
	/* bit 1 is cs (active low) */
	/* bit 2 is clock (active high) */
	/* bit 3 is "MUT" on the schematics (audio mute?) */
	/* bit 4 bankswitches the 4400-7fff region: 0 = registers, 1 = palette RAM ("CBNK" on schematics) */
	/* bit 6 is "SHD0" (some kind of shadow control) */
	/* bit 7 is "SHD1" (ditto) */

	m_cur_control2 = data;

	m_bank4000->set_bank(BIT(m_cur_control2, 4));

	ioport("EEPROMOUT")->write(m_cur_control2, 0xff);
}

INTERRUPT_GEN_MEMBER(lethal_state::lethalen_interrupt)
{
	if (m_k056832->is_irq_enabled(0))
		device.execute().set_input_line(HD6309_IRQ_LINE, HOLD_LINE);
}

uint8_t lethal_state::sound_irq_r()
{
	m_soundcpu->set_input_line(0, HOLD_LINE);
	return 0x00;
}

void lethal_state::sound_irq_w(uint8_t data)
{
	m_soundcpu->set_input_line(0, HOLD_LINE);
}

void lethal_state::le_bankswitch_w(uint8_t data)
{
	membank("bank1")->set_entry(data);
}

uint8_t lethal_state::guns_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return GUNX(1) >> 1;
		case 1:
			if ((GUNY(1)<=0x0b) || (GUNY(1)>=0xe8))
				return 0;
			else
				return (232 - GUNY(1));
		case 2:
			return GUNX(2) >> 1;
		case 3:
			if ((GUNY(2)<=0x0b) || (GUNY(2)>=0xe8))
				return 0;
			else
				return (232 - GUNY(2));
	}

	return 0;
}

uint8_t lethal_state::gunsaux_r()
{
	int res = 0;

	if (GUNX(1) & 1) res |= 0x80;
	if (GUNX(2) & 1) res |= 0x40;

	return res;
}

void lethal_state::le_main(address_map &map)
{
	map(0x0000, 0x1fff).bankr("bank1");
	map(0x2000, 0x3fff).ram();             // work RAM
	map(0x4000, 0x7fff).m(m_bank4000, FUNC(address_map_bank_device::amap8));
	map(0x4000, 0x43ff).unmaprw(); // first 0x400 bytes of palette RAM are inaccessible
	map(0x4000, 0x403f).w(m_k056832, FUNC(k056832_device::write));
	map(0x4040, 0x404f).w(m_k056832, FUNC(k056832_device::b_w));
	map(0x4080, 0x4080).nopr();     // watchdog
	map(0x4090, 0x4090).rw(FUNC(lethal_state::sound_irq_r), FUNC(lethal_state::sound_irq_w));
	map(0x40a0, 0x40a0).nopr();
	map(0x40c4, 0x40c4).w(FUNC(lethal_state::control2_w));
	map(0x40c8, 0x40d0).w(FUNC(lethal_state::lethalen_palette_control)); // PCU1-PCU3 on the schematics
	map(0x40d4, 0x40d7).r(FUNC(lethal_state::guns_r));
	map(0x40d8, 0x40d8).portr("DSW");
	map(0x40d9, 0x40d9).portr("INPUTS");
	map(0x40db, 0x40db).r(FUNC(lethal_state::gunsaux_r));     // top X bit of guns
	map(0x40dc, 0x40dc).w(FUNC(lethal_state::le_bankswitch_w));
	map(0x8000, 0xffff).rom().region("maincpu", 0x38000);
}

void lethal_state::bank4000_map(address_map &map)
{
	// VRD = 0 or 1, CBNK = 0
	map(0x0840, 0x084f).mirror(0x8000).rw(m_k053244, FUNC(k05324x_device::k053244_r), FUNC(k05324x_device::k053244_w));
	map(0x0880, 0x089f).mirror(0x8000).m("k054000", FUNC(k054000_device::map));
	map(0x08c0, 0x08cf).m(m_k054321, FUNC(k054321_device::main_map));
	map(0x1000, 0x17ff).mirror(0x8000).rw(m_k053244, FUNC(k05324x_device::k053245_r), FUNC(k05324x_device::k053245_w));

	// VRD = 0, CBNK = 0
	map(0x2000, 0x27ff).rw(m_k056832, FUNC(k056832_device::ram_code_lo_r), FUNC(k056832_device::ram_code_lo_w));
	map(0x2800, 0x2fff).rw(m_k056832, FUNC(k056832_device::ram_code_hi_r), FUNC(k056832_device::ram_code_hi_w));
	map(0x3000, 0x37ff).rw(m_k056832, FUNC(k056832_device::ram_attr_lo_r), FUNC(k056832_device::ram_attr_lo_w));
	map(0x3800, 0x3fff).rw(m_k056832, FUNC(k056832_device::ram_attr_hi_r), FUNC(k056832_device::ram_attr_hi_w));

	// VRD = 1, CBNK = 0 or 1
	map(0xa000, 0xbfff).mirror(0x4000).unmaprw(); // .r(m_k056832, FUNC(k056832_device::rom_byte_r));

	// CBNK = 1; partially overlaid when VRD = 1
	map(0x4000, 0x7fff).mirror(0x8000).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
}

void lethal_state::le_sound(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xfa2f).rw("k054539", FUNC(k054539_device::read), FUNC(k054539_device::write));
	map(0xfc00, 0xfc03).m(m_k054321, FUNC(k054321_device::sound_map));
}

static INPUT_PORTS_START( lethalen )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, do_read)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, ready_read)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR(Language) )       PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x10, DEF_STR(English) )
	PORT_DIPSETTING(    0x00, DEF_STR(Spanish) )
	PORT_DIPNAME( 0x20, 0x00, "Game Type" )         PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x20, "Street" )
	PORT_DIPSETTING(    0x00, "Arcade" )
	PORT_DIPNAME( 0x40, 0x40, "Coin Mechanism" )        PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x40, "Common" )
	PORT_DIPSETTING(    0x00, "Independent" )
	PORT_DIPNAME( 0x80, 0x80, "Sound Output" )      PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Stereo ) )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, cs_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, clk_write)

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( lethalenj )
	PORT_INCLUDE( lethalen )

		PORT_MODIFY("DSW")  /* Normal DIPs appear to do nothing for Japan region - wrong location?  Set to unknown */
		PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW:4")
		PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW:3")
		PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW:2")
		PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW:1")

		PORT_MODIFY("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_REVERSE

	PORT_MODIFY("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_REVERSE

	PORT_MODIFY("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_REVERSE

	PORT_MODIFY("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_REVERSE
INPUT_PORTS_END

static INPUT_PORTS_START( lethalene ) /* European region does not have non-english Language option */
	PORT_INCLUDE( lethalen )

	PORT_MODIFY("DSW")
		PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "DSW:4")
INPUT_PORTS_END


void lethal_state::machine_start()
{
	membank("bank1")->configure_entries(0, 0x20, memregion("maincpu")->base(), 0x2000);
	membank("bank1")->set_entry(0);

	save_item(NAME(m_cur_control2));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_back_colorbase));
}

void lethal_state::machine_reset()
{
	for (auto & elem : m_layer_colorbase)
		elem = 0;

	m_sprite_colorbase = 0;
	m_back_colorbase = 0;
	m_cur_control2 = 0;
	m_bank4000->set_bank(0);
}

void lethal_state::lethalen(machine_config &config)
{
	constexpr XTAL MAIN_CLOCK = 24_MHz_XTAL;
	constexpr XTAL SOUND_CLOCK = 18.432_MHz_XTAL;

	/* basic machine hardware */
	HD6309E(config, m_maincpu, MAIN_CLOCK/8);    /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &lethal_state::le_main);
	m_maincpu->set_vblank_int("screen", FUNC(lethal_state::lethalen_interrupt));

	Z80(config, m_soundcpu, MAIN_CLOCK/4);  /* verified on pcb */
	m_soundcpu->set_addrmap(AS_PROGRAM, &lethal_state::le_sound);

	ADDRESS_MAP_BANK(config, m_bank4000).set_map(&lethal_state::bank4000_map).set_options(ENDIANNESS_BIG, 8, 16, 0x4000);

	EEPROM_ER5911_8BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.62);  /* verified on pcb */
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(216, 504-1, 16, 240-1);
	screen.set_screen_update(FUNC(lethal_state::screen_update_lethalen));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 8192);
	m_palette->enable_shadows();

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(lethal_state::tile_callback));
	m_k056832->set_config(K056832_BPP_8LE, 1, 0);
	m_k056832->set_palette(m_palette);

	K053244(config, m_k053244, 0);
	m_k053244->set_palette(m_palette);
	m_k053244->set_bpp(6);
	m_k053244->set_offsets(95, 0);
	m_k053244->set_sprite_callback(FUNC(lethal_state::sprite_callback));

	K054000(config, "k054000", 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	K054321(config, m_k054321, "lspeaker", "rspeaker");

	k054539_device &k054539(K054539(config, "k054539", SOUND_CLOCK));
	k054539.timer_handler().set_inputline("soundcpu", INPUT_LINE_NMI);
	k054539.add_route(0, "rspeaker", 1.0);
	k054539.add_route(1, "lspeaker", 1.0);
}

void lethal_state::lethalej(machine_config &config)
{
	lethalen(config);

	subdevice<screen_device>("screen")->set_visarea(224, 512-1, 16, 240-1);

	m_k053244->set_offsets(-105, 0);
}

ROM_START( lethalen )   // US version UAE
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191uae01.u4", 0x00000, 0x40000, CRC(dca340e3) SHA1(8efbba0e3a459bcfe23c75c584bf3a4ce25148bb) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenue.nv", 0x0000, 0x0080, CRC(6e7224e6) SHA1(86dea9262d55e58b573d397d0fea437c58728707) )
ROM_END

ROM_START( lethalenub ) // US version UAB
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191uab01.u4", 0x00000, 0x40000, CRC(2afd7528) SHA1(65ce4a54fe96ad38d39d335b5d3a644a495c7e31) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenub.nv", 0x0000, 0x0080, CRC(14c6c6e5) SHA1(8a498b5322266df25fb24d1b7bd7937de459d207) )
ROM_END

ROM_START( lethalenua ) // US version UAA
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191uaa01.u4", 0x00000, 0x40000, CRC(ab6b8f16) SHA1(8de6c429a6e71144270e79d18ad47b5aad13fe04) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenua.nv", 0x0000, 0x0080, CRC(f71ad1c3) SHA1(04c7052d0895797af8a06183b8a877795bf2dbb3) )
ROM_END

ROM_START( lethalenux ) // US version ?, proto / hack?, very different to other sets
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191xxx01.u4", 0x00000, 0x40000, CRC(a3b9e790) SHA1(868b422850be129952c8b11c3c4aa730d8ea1544) ) // hacked? fails rom test, verified on multiple boards

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenux.nv", 0x0000, 0x0080, CRC(5d69c39d) SHA1(e468df829ee5094792289f9166d7e39b638ab70d) )
ROM_END

ROM_START( lethaleneab )    // Euro ver. EAB
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191eab01.u4", 0x00000, 0x40000, CRC(d7ce111e) SHA1(e56137a0ba7664f09b5d05bb39ec6eb4d1e412c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethaleneab.nv", 0x0000, 0x0080, CRC(4e9bb34d) SHA1(9502583bc9f5f6fc5bba333869398b24bf154b73) )
ROM_END

ROM_START( lethalenead )    // Euro ver. EAD
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191ead01.u4",    0x00000,  0x40000, CRC(ce2e13ce) SHA1(52df8fbcca0de02bdb83fc804ee27a90135993e7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenead.nv", 0x0000, 0x0080, CRC(e1dc2bc0) SHA1(adfb8dfaec0b0faf6784eab97a5b31c0c7813756) )
ROM_END

ROM_START( lethaleneae )    // Euro ver. EAE
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191eae01.u4",    0x00000,  0x40000, CRC(c6a3c6ac) SHA1(96a209a3a5b4af40af36bd7090c59a74f8c8df59) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethaleneae.nv", 0x0000, 0x0080, CRC(eb369a67) SHA1(6c67294669614e96de5efb38372dbed435ee04d3) )
ROM_END

ROM_START( lethalenj )  // Japan version JAD
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191jad01.u4",    0x00000,  0x40000, CRC(160a25c0) SHA1(1d3ed5a158e461a73c079fe24a8e9d5e2a87e126) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenj.nv", 0x0000, 0x0080, CRC(20b28f2f) SHA1(53d212f2c006729a01dfdb49cb36b67b9425172e) )
ROM_END

ROM_START( lethaleneaa )    // Euro ver. EAA
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191_a01.u4", 0x00000, 0x40000, CRC(c6f4d712) SHA1(92938b823f057b5185a2ada7878efa4bf7e6c682) ) // handwritten label

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethaleneaa.nv", 0x0000, 0x0080, CRC(a85d64ee) SHA1(68ab906c46c6a7dee2a7673d1d516e34d56c9ae3) )
ROM_END

// date strings are at 0x3fd00 in the main program rom

} // anonymous namespace

GAME( 1992, lethalen,   0,        lethalen, lethalen,  lethal_state, empty_init, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver UAE, 11/19/92 15:04)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes UE to eeprom
GAME( 1992, lethalenub, lethalen, lethalen, lethalen,  lethal_state, empty_init, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver UAB, 09/01/92 11:12)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes UB to eeprom
GAME( 1992, lethalenua, lethalen, lethalen, lethalen,  lethal_state, empty_init, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver UAA, 08/17/92 21:38)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes UA to eeprom
GAME( 1992, lethalenux, lethalen, lethalen, lethalen,  lethal_state, empty_init, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver unknown, US, 08/06/92 15:11, hacked/proto?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes UA to eeprom but earlier than suspected UAA set, might be a proto, might be hacked, fails rom test, definitely a good dump, another identical set was found in Italy

GAME( 1992, lethaleneae,lethalen, lethalen, lethalene, lethal_state, empty_init, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver EAE, 11/19/92 16:24)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes EE to eeprom
GAME( 1992, lethalenead,lethalen, lethalen, lethalene, lethal_state, empty_init, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver EAD, 11/11/92 10:52)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes ED to eeprom
GAME( 1992, lethaleneab,lethalen, lethalen, lethalene, lethal_state, empty_init, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver EAB, 10/14/92 19:53)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes EC to eeprom, so might actually be EC
GAME( 1992, lethaleneaa,lethalen, lethalen, lethalene, lethal_state, empty_init, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver EAA, 09/09/92 09:44)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes EA to eeprom

// different mirror / display setup
GAME( 1992, lethalenj,  lethalen, lethalej, lethalenj, lethal_state, empty_init, ORIENTATION_FLIP_X, "Konami", "Lethal Enforcers (ver JAD, 12/04/92 17:16)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes JC to eeprom?!
