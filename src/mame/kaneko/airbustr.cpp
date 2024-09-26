// license:BSD-3-Clause
// copyright-holders: Luca Elia
/***************************************************************************

                                Air Buster
                            (C) 1990  Kaneko

                    driver by Luca Elia (l.elia@tin.it)

CPU   : Z-80 x 3
SOUND : YM2203C     M6295
OSC.  : 12.000MHz   16.000MHz

                    Interesting routines (main CPU)
                    -------------------------------

fd-fe   address of int: 0x38    (must alternate? see e600/1)
ff-100  address of int: 0x16
66      print "sub CPU caused nmi" and die!

7       after tests

1497    print string following call: (char)*,0. program continues after 0.
        base addr = c300 + HL & 08ff, DE=xy , BC=dxdy
        +0<-(e61b)  +100<-D +200<-E +300<-char  +400<-(e61c)

1642    A<- buttons status (bits 0&1)

                    Interesting locations (main CPU)
                    --------------------------------

2907    table of routines (f150 index = 1-3f, copied to e612)
        e60e<-address of routine, f150<-0. e60e is used until f150!=0

    1:  2bf4    service mode                                next

    2:  2d33    3:  16bd        4:  2dcb        5:  2fcf
    6:  3262    7:  32b8

    8:  335d>   print gfx/color test page                   next
    9:  33c0>   handle the above page

    a:  29c6        b:  2a24        c:  16ce

    d:  3e7e>   *
    e:  3ec5>   print "Sub CPU / Ram Error"; **
    f:  3e17>   print "Coin error"; **
    10: 3528>   print (c) notice, not shown                 next
    11: 3730>   show (c) notice, wait 0x100 calls           next

    12:     9658    13: 97c3        14: a9fa        15: aa6e
    16-19:  2985    1a: 9c2e        1b: 9ffa        1c: 29c6

    1d: 2988>   *

    1e: a2c4        1f: a31a        20: a32f        21: a344
    22: a359        23: a36e        24: a383        25: a398
    26: a3ad        27: a3c2        28: a3d7        29: a3f1
    2a: a40e        2b: a4e5        2c: a69d        2d: adb8
    2e: ade9        2f: 2b8b        30: a823

    31: 3d17>   print "warm up, wait few mins. secs left: 00"   next
    32: 3dc0>   pause (e624 counter).e626                       next

    33: 96b4        34: 97ad

    35-3f:  3e7e>   *

*   Print "Command Error [$xx]" where xx is last routine index (e612)
**  ld (0000h),A (??) ; loop

3cd7    hiscores table (0x40 bytes, copied to e160)
        Try entering TERU as your name :)

7fff    country code: 0 <-> Japan; else World

e615    rank:   0-easy  1-normal    2-hard  3-hardest
e624    sound code during sound test

-- Shared RAM --

f148<-  sound code (copied from e624)
f14a->  read on nmi routine. main CPU writes the value and writes to port 02
f150<-  index of table of routines at 2907

----------------

                    Interesting routines (sub CPU)
                    -------------------------------

491     copy palette data   d000<-f200(a0)  d200<-f300(a0)  d400<-f400(200)

61c     f150<-A     f151<-A (routine index of main CPU!)
        if dsw1-2 active, it does nothing (?!)

c8c     c000-c7ff<-0    c800-cfff<-0    f600<-f200(400)
1750    copies 10 lines of 20 bytes from 289e to f800

22cd    copy 0x100 bytes
22cf    copy 0x0FF bytes
238d    copy 0x0A0 bytes

                    Interesting locations (sub CPU)
                    --------------------------------

fd-fe   address of int: 0x36e   (same as 38)
ff-100  address of int: 0x4b0

-- Shared RAM --

f000    credits

f001/d<-IN 24 (Service)
f00e<-  bank
f002<-  dsw1 (cpl'd)
f003<-  dsw2 (cpl'd)
f004<-  IN 20 (cpl'd) (player 1)
f005<-  IN 22 (cpl'd) (player 2)
f006<-  start lives: dsw-2 & 0x30 index; values: 3,4,5,7        (5da table)
f007    current lives p1
f008    current lives p2

f009<-  coin/credit 1: dsw-1 & 0x30 index; values: 11,12,21,23  (5de table)
f00a<-  coin 1
f00b<-  coin/credit 2: dsw-1 & 0xc0 index; values: 11,12,21,23  (5e2 table)
f00c<-  coin 2

f00f    ?? outa (28h)
f010    written by sub CPU, bit 4 read by main CPU.
        bit 0   p1 playing
        bit 1   p2 playing

f014    index (1-f) of routine called during int 36e (table at c3f)
    1:  62b         2:  66a         3:  6ad         4:  79f
    5:  7e0         6:  81b         7:  8a7         8:  8e9
    9:  b02         a:  0           b:  0           c:  bfc
    d:  c0d         e:  a6f         f:  ac3

f015    index of the routine to call, usually the one selected by f014
        but sometimes written directly.

Scroll registers: ports 04/06/08/0a/0c, written in sequence (101f)
port 06 <- f100 + f140  x       port 04 <- f104 + f142  y
port 0a <- f120 + f140  x       port 08 <- f124 + f142  y
port 0c <- f14c = bit 0/1/2/3 = port 6/4/a/8 val < FF

f148->  sound code (from main CPU)
f14c    scroll regs high bits

----------------

                    Interesting routines (sound CPU)
                    -------------------------------

50a     6295
521     6295
a96     2203 reg<-B     val<-A

                    Interesting locations (sound CPU)
                    ---------------------------------

c715
c716    pending sound command
c760    ROM bank


                                To Do
                                -----

- Is the sub CPU / sound CPU communication status port (0x0e) correct ?
- Main CPU: port 0x01 ? boot sub/sound CPU?
- Sub  CPU: port 0x38 ? irq ack?
- incomplete DSWs
- Spriteram low 0x300 bytes (priority?)
*/

/*
**
**              Main CPU data
**
*/

/*  Runs in IM 2    fd-fe   address of int: 0x38
                    ff-100  address of int: 0x16    */

/*
**
**              Sub CPU data
**
**
*/

/*  Runs in IM 2    fd-fe   address of int: 0x36e   (same as 0x38)
                    ff-100  address of int: 0x4b0   (only writes to port 38h)   */
/*
   Sub CPU and Sound CPU communicate bidirectionally:

       Sub   CPU writes to soundlatch,  reads from soundlatch2
       Sound CPU writes to soundlatch2, reads from soundlatch

   Each latch raises a flag when it's been written.
   The flag is cleared when the latch is read.

Code at 505: waits for bit 1 to go low, writes command, waits for bit
0 to go low, reads back value. Code at 3b2 waits bit 2 to go high
(called during int fd)

*/

#include "emu.h"

#include "kaneko_hit.h"
#include "kan_pand.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_SCROLLREGS     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_SCROLLREGS)

#include "logmacro.h"

#define LOGSCROLLREGS(...)     LOGMASKED(LOG_SCROLLREGS,     __VA_ARGS__)


namespace {

class airbustr_state : public driver_device
{
public:
	airbustr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_devram(*this, "devram")
		, m_videoram(*this, "videoram%u", 1)
		, m_colorram(*this, "colorram%u", 1)
		, m_masterbank(*this, "masterbank")
		, m_slavebank(*this, "slavebank")
		, m_audiobank(*this, "audiobank")
		, m_master(*this, "master")
		, m_slave(*this, "slave")
		, m_audiocpu(*this, "audiocpu")
		, m_pandora(*this, "pandora")
		, m_calc1(*this, "calc1")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch%u", 1)
	{
	}

	void airbustr(machine_config &config);
	void airbustrb(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_devram;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_colorram;

	required_memory_bank m_masterbank;
	required_memory_bank m_slavebank;
	required_memory_bank m_audiobank;

	// video-related
	tilemap_t *m_tilemap[2]{};
	uint8_t m_scrollx[2]{};
	uint8_t m_scrolly[2]{};
	uint8_t m_highbits = 0;

	// devices
	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	required_device<cpu_device> m_audiocpu;
	required_device<kaneko_pandora_device> m_pandora;
	optional_device<kaneko_hit_device> m_calc1;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;

	void calc1_w(offs_t offset, uint8_t data);
	uint8_t calc1_r(offs_t offset);
	void master_nmi_trigger_w(uint8_t data);
	void master_bankswitch_w(uint8_t data);
	void slave_bankswitch_w(uint8_t data);
	void sound_bankswitch_w(uint8_t data);
	uint8_t soundcommand_status_r();
	void coin_counter_w(uint8_t data);
	template<int Layer> void videoram_w(offs_t offset, uint8_t data);
	template<int Layer> void colorram_w(offs_t offset, uint8_t data);
	void scrollregs_w(offs_t offset, uint8_t data);
	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	INTERRUPT_GEN_MEMBER(slave_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void master_map(address_map &map) ATTR_COLD;
	void master_prot_map(address_map &map) ATTR_COLD;
	void master_io_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
	void slave_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
};


// Video

/**************************************************************************

[Screen]
    Size:               256 x 256
    Colors:             256 x 3
    Color Space:        32R x 32G x 32B

[Scrolling layers]
    Number:             2
    Size:               512 x 512
    Scrolling:          X,Y
    Tiles Size:         16 x 16
    Tiles Number:       0x1000
    Colors:             256 x 2 (0-511)
    Format:
                Offset:     0x400    0x000
                Bit:        fedc---- --------   Color
                            ----ba98 76543210   Code

[Sprites]
    On Screen:          256 x 2
    In ROM:             0x2000
    Colors:             256     (512-767)
    Format:             See Below


**************************************************************************/

/*  Scroll Registers

    Port:
    4       Bg Y scroll, low 8 bits
    6       Bg X scroll, low 8 bits
    8       Fg Y scroll, low 8 bits
    A       Fg X scroll, low 8 bits

    C       3       2       1       0       <-Bit
            Bg Y    Bg X    Fg Y    Fg X    <-Scroll High Bits (complemented!)
*/

void airbustr_state::scrollregs_w(offs_t offset, uint8_t data)
{
	switch (offset)     // offset 0 <-> port 4
	{
		case 0x00:
		case 0x04:  m_scrolly[((offset & 4) >> 2) ^ 1] = data;    break;  // low 8 bits
		case 0x02:
		case 0x06:  m_scrollx[((offset & 4) >> 2) ^ 1] = data;    break;
		case 0x08:  m_highbits   = ~data;   break;  // complemented high bits

		default: LOGSCROLLREGS("CPU #2 - port %02X written with %02X - PC = %04X\n", offset, data, m_slave->pc());
	}

	for (int layer = 0; layer < 2; layer++)
	{
		m_tilemap[layer]->set_scrolly(0, ((m_highbits << (5 + (layer << 1))) & 0x100) + m_scrolly[layer]);
		m_tilemap[layer]->set_scrollx(0, ((m_highbits << (6 + (layer << 1))) & 0x100) + m_scrollx[layer]);
	}
}

template<int Layer>
TILE_GET_INFO_MEMBER(airbustr_state::get_tile_info)
{
	int const attr = m_colorram[Layer][tile_index];
	int const code = m_videoram[Layer][tile_index] + ((attr & 0x0f) << 8);
	int const color = (attr >> 4) + ((Layer ^ 1) << 4);

	tileinfo.set(0, code, color, 0);
}

void airbustr_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(airbustr_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(airbustr_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tilemap[1]->set_transparent_pen(0);

	for (int layer = 0; layer < 2; layer++)
	{
		m_tilemap[layer]->set_scrolldx(0x094, 0x06a);
		m_tilemap[layer]->set_scrolldy(0x100, 0x1ff);
	}
}


uint32_t airbustr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	// copy the sprite bitmap to the screen
	m_pandora->update(bitmap, cliprect);

	return 0;
}

void airbustr_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		// update the sprite bitmap
		m_pandora->eof();
	}
}


// Machine

// Read / Write Handlers

void airbustr_state::calc1_w(offs_t offset, uint8_t data)
{
	offset += 0xfe0;
	m_devram[offset] = data;

	// CALC1 chip is 16-bit
	uint16_t data16 = m_devram[offset | 1] << 8 | m_devram[offset & ~1];
	m_calc1->kaneko_hit_w((offset & 0x1f) / 2, data16);
}

uint8_t airbustr_state::calc1_r(offs_t offset)
{
	// CALC1 chip is 16-bit
	uint16_t ret = m_calc1->kaneko_hit_r(offset / 2);
	return (offset & 1) ? (ret >> 8) : (ret & 0xff);
}

void airbustr_state::master_nmi_trigger_w(uint8_t data)
{
	m_slave->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void airbustr_state::master_bankswitch_w(uint8_t data)
{
	m_masterbank->set_entry(data & 0x07);
}

void airbustr_state::slave_bankswitch_w(uint8_t data)
{
	m_slavebank->set_entry(data & 0x07);

	for (int layer = 0; layer < 2; layer++)
		m_tilemap[layer]->set_flip(BIT(data, 4) ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	m_pandora->flip_screen_set(BIT(data, 4));

	// used at the end of levels, after defeating the boss, to leave trails
	m_pandora->set_clear_bitmap(BIT(data, 5));
}

void airbustr_state::sound_bankswitch_w(uint8_t data)
{
	m_audiobank->set_entry(data & 0x07);
}

uint8_t airbustr_state::soundcommand_status_r()
{
	// bits: 2 <-> ?    1 <-> soundlatch full   0 <-> soundlatch2 empty
	return 4 | (m_soundlatch[0]->pending_r() << 1) | !m_soundlatch[1]->pending_r();
}


template<int Layer>
void airbustr_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[Layer][offset] = data;
	m_tilemap[Layer]->mark_tile_dirty(offset);
}

template<int Layer>
void airbustr_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[Layer][offset] = data;
	m_tilemap[Layer]->mark_tile_dirty(offset);
}

void airbustr_state::coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
	machine().bookkeeping().coin_lockout_w(0, ~data & 4);
	machine().bookkeeping().coin_lockout_w(1, ~data & 8);
}


// Memory Maps

void airbustr_state::master_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_masterbank);
	map(0xc000, 0xcfff).rw(m_pandora, FUNC(kaneko_pandora_device::spriteram_r), FUNC(kaneko_pandora_device::spriteram_w));
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xefff).ram().share(m_devram); // shared with protection device (see below)
	map(0xf000, 0xffff).ram().share("master_slave");
}

void airbustr_state::master_prot_map(address_map &map)
{
	master_map(map);
	map(0xefe0, 0xeff5).rw(FUNC(airbustr_state::calc1_r), FUNC(airbustr_state::calc1_w));
}

void airbustr_state::master_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(airbustr_state::master_bankswitch_w));
	map(0x01, 0x01).nopw(); // ???
	map(0x02, 0x02).w(FUNC(airbustr_state::master_nmi_trigger_w));
}

void airbustr_state::slave_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_slavebank);
	map(0xc000, 0xc3ff).ram().w(FUNC(airbustr_state::videoram_w<1>)).share(m_videoram[1]);
	map(0xc400, 0xc7ff).ram().w(FUNC(airbustr_state::colorram_w<1>)).share(m_colorram[1]);
	map(0xc800, 0xcbff).ram().w(FUNC(airbustr_state::videoram_w<0>)).share(m_videoram[0]);
	map(0xcc00, 0xcfff).ram().w(FUNC(airbustr_state::colorram_w<0>)).share(m_colorram[0]);
	map(0xd000, 0xd5ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xd600, 0xdfff).ram();
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xffff).ram().share("master_slave");
}

void airbustr_state::slave_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(airbustr_state::slave_bankswitch_w));
	map(0x02, 0x02).r(m_soundlatch[1], FUNC(generic_latch_8_device::read)).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
	map(0x04, 0x0c).w(FUNC(airbustr_state::scrollregs_w));
	map(0x0e, 0x0e).r(FUNC(airbustr_state::soundcommand_status_r));
	map(0x20, 0x20).portr("P1");
	map(0x22, 0x22).portr("P2");
	map(0x24, 0x24).portr("SYSTEM");
	map(0x28, 0x28).w(FUNC(airbustr_state::coin_counter_w));
	map(0x38, 0x38).nopw(); // irq ack / irq mask
}

void airbustr_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_audiobank);
	map(0xc000, 0xdfff).ram();
}

void airbustr_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(airbustr_state::sound_bankswitch_w));
	map(0x02, 0x03).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x04, 0x04).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x06, 0x06).r(m_soundlatch[0], FUNC(generic_latch_8_device::read)).w(m_soundlatch[1], FUNC(generic_latch_8_device::write));
}


// Input Ports

static INPUT_PORTS_START( airbustr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // used

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Mode" )             PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Mode 1" )            // routine at 0x056d: 11 21 12 16 (bit 3 active)
	PORT_DIPSETTING(    0x00, "Mode 2" )            // 11 21 13 14 (bit 3 not active)
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( airbustrj )
	PORT_INCLUDE(airbustr)

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:5,6") // routine at 0x0546 : 11 12 21 23
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


// Graphics Decode Information

static GFXDECODE_START( gfx_airbustr )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x4_row_2x2_group_packed_lsb,   0, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_airbustr_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_row_2x2_group_packed_msb, 512, 16 )
GFXDECODE_END


// Interrupt Generators

// Main Z80 uses IM2
TIMER_DEVICE_CALLBACK_MEMBER(airbustr_state::scanline)
{
	int scanline = param;

	if (scanline == 240) // vblank-out irq
		m_master->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80

	// Pandora "sprite end dma" irq? TODO: timing is likely off
	if (scanline == 64)
		m_master->set_input_line_and_vector(0, HOLD_LINE, 0xfd); // Z80
}

// Sub Z80 uses IM2 too, but 0xff irq routine just contains an irq ack in it
INTERRUPT_GEN_MEMBER(airbustr_state::slave_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xfd); // Z80
}


// Machine Initialization

void airbustr_state::machine_start()
{
	m_masterbank->configure_entries(0, 8, memregion("master")->base(), 0x4000);
	m_slavebank->configure_entries(0, 8, memregion("slave")->base(), 0x4000);
	m_audiobank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_highbits));
}

void airbustr_state::machine_reset()
{
	m_scrollx[0] = 0;
	m_scrolly[0] = 0;
	m_scrollx[1] = 0;
	m_scrolly[1] = 0;
	m_highbits = 0;
}


// Machine Driver

void airbustr_state::airbustrb(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_master, XTAL(12'000'000) / 2); // verified on PCB
	m_master->set_addrmap(AS_PROGRAM, &airbustr_state::master_map);
	m_master->set_addrmap(AS_IO, &airbustr_state::master_io_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(airbustr_state::scanline), "screen", 0, 1);

	Z80(config, m_slave, XTAL(12'000'000) / 2); // verified on PCB
	m_slave->set_addrmap(AS_PROGRAM, &airbustr_state::slave_map);
	m_slave->set_addrmap(AS_IO, &airbustr_state::slave_io_map);
	m_slave->set_vblank_int("screen", FUNC(airbustr_state::slave_interrupt)); // nmi signal from master CPU

	Z80(config, m_audiocpu, XTAL(12'000'000) / 2); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &airbustr_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &airbustr_state::sound_io_map);

	// palette RAM is filled by sub CPU with data supplied by main CPU, maybe a high value is safer in order to avoid glitches
	config.set_maximum_quantum(attotime::from_hz(6000));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(57.4);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(airbustr_state::screen_update));
	m_screen->screen_vblank().set(FUNC(airbustr_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_airbustr);
	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 768);

	KANEKO_PANDORA(config, m_pandora, 0, m_palette, gfx_airbustr_spr);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	m_soundlatch[0]->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	GENERIC_LATCH_8(config, m_soundlatch[1]);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(12'000'000) / 4)); // verified on PCB
	ymsnd.port_a_read_callback().set_ioport("DSW1");
	ymsnd.port_b_read_callback().set_ioport("DSW2");
	ymsnd.irq_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.25);
	ymsnd.add_route(2, "mono", 0.25);
	ymsnd.add_route(3, "mono", 0.50);

	OKIM6295(config, "oki", XTAL(12'000'000)/4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.80); // verified on PCB
}

void airbustr_state::airbustr(machine_config &config)
{
	airbustrb(config);

	m_master->set_addrmap(AS_PROGRAM, &airbustr_state::master_prot_map);

	KANEKO_HIT(config, m_calc1).set_type(0);
	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_seconds(3)); // a guess, and certainly wrong
}


// ROMs

ROM_START( airbustr )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "pr12.h19",   0x00000, 0x20000, CRC(91362eb2) SHA1(cd85acfa6542af68dd1cad46f9426a95cfc9432e) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD( "pr13.l15",   0x00000, 0x20000, CRC(13b2257b) SHA1(325efa54e757a1f08caf81801930d61ea4e7b6d4) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "pr-21.bin",  0x00000, 0x20000, CRC(6e0a5df0) SHA1(616b7c7aaf52a9a55b63c60717c1866940635cd4) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "pr-000.bin", 0x00000, 0x80000, CRC(8ca68f0d) SHA1(d60389e7e63e9850bcddecb486558de1414f1276) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "pr-001.bin", 0x00000, 0x80000, CRC(7e6cb377) SHA1(005290f9f53a0c3a6a9d04486b16b7fd52cc94b6) )
	ROM_LOAD( "pr-02.bin",  0x80000, 0x10000, CRC(6bbd5e46) SHA1(26563737f3f91ee0a056d35ce42217bb57d8a081) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pr-200.bin", 0x00000, 0x40000, CRC(a4dd3390) SHA1(2d72b46b4979857f6b66489bebda9f48799f59cf) )
ROM_END

ROM_START( airbustrj )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "pr-14j.bin", 0x00000, 0x20000, CRC(6b9805bd) SHA1(db6df33cf17316a4b81d7731dca9fe8bbf81f014) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD( "pr-11j.bin", 0x00000, 0x20000, CRC(85464124) SHA1(8cce8dfdede48032c40d5f155fd58061866668de) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "pr-21.bin",  0x00000, 0x20000, CRC(6e0a5df0) SHA1(616b7c7aaf52a9a55b63c60717c1866940635cd4) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "pr-000.bin", 0x00000, 0x80000, CRC(8ca68f0d) SHA1(d60389e7e63e9850bcddecb486558de1414f1276) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "pr-001.bin", 0x000000, 0x80000, CRC(7e6cb377) SHA1(005290f9f53a0c3a6a9d04486b16b7fd52cc94b6) )
	ROM_LOAD( "pr-02.bin",  0x080000, 0x10000, CRC(6bbd5e46) SHA1(26563737f3f91ee0a056d35ce42217bb57d8a081) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pr-200.bin", 0x00000, 0x40000, CRC(a4dd3390) SHA1(2d72b46b4979857f6b66489bebda9f48799f59cf) )
ROM_END

/*

Differences with the original (when running on the bootleg hardware):

no title screen
long attract modes of every level
slow downs with corrupted screen (you can see the screen being redrawn!) when there are many sprites

the board has 2 oscillators (12 and 16 MHz).  ROM 1 and 2 are program ROMs. 3 and 4 for sound.
ROM 5 is on a piggyback daughterboard with a z80 and a PAL

*/

ROM_START( airbustrb )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "5.bin",   0x00000, 0x20000, CRC(9e4216a2) SHA1(46572da4df5a67b10cc3ee21bdc0ec4bcecaaf93) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD( "1.bin",   0x00000, 0x20000, CRC(85464124) SHA1(8cce8dfdede48032c40d5f155fd58061866668de) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "2.bin",  0x00000, 0x20000, CRC(6e0a5df0) SHA1(616b7c7aaf52a9a55b63c60717c1866940635cd4) )

	ROM_REGION( 0x80000, "tiles", 0 )
	// Same content as airbusj, pr-001.bin, different sized ROMs / interleave
	ROM_LOAD16_BYTE( "7.bin", 0x00000, 0x20000, CRC(2e3bf0a2) SHA1(84cabc753e5fd1164f0a8a9a9dee7d339a5607c5) )
	ROM_LOAD16_BYTE( "9.bin", 0x00001, 0x20000, CRC(2c23c646) SHA1(41c0f8788c9715918b4138f076415f8640adc483) )
	ROM_LOAD16_BYTE( "6.bin", 0x40000, 0x20000, CRC(0d6cd470) SHA1(329286bc6c9d1eccc74735d1c155a0f5f98f1444) )
	ROM_LOAD16_BYTE( "8.bin", 0x40001, 0x20000, CRC(b3372e51) SHA1(aa8dcbb84c829994ae04ceecbef795ac53e72493) )

	ROM_REGION( 0x100000, "sprites", 0 )
	// Same content as airbusj, pr-001.bin, different sized ROMs
	ROM_LOAD( "13.bin", 0x00000, 0x20000, CRC(75dee86d) SHA1(fe342fed5bb84ee6f35d3f91987141c559e94d5a) )
	ROM_LOAD( "12.bin", 0x20000, 0x20000, CRC(c98a8333) SHA1(3a990460e232ee07a9297fcffdb02451406f5bf1) )
	ROM_LOAD( "11.bin", 0x40000, 0x20000, CRC(4e9baebd) SHA1(6cf878a3fb3d344e3f5f4d031fbde6f14b653636) )
	ROM_LOAD( "10.bin", 0x60000, 0x20000, CRC(63dc8cd8) SHA1(4b466a8ede4211fa3f51572b223eba8766990d7a) )

	ROM_LOAD( "14.bin", 0x80000, 0x10000, CRC(6bbd5e46) SHA1(26563737f3f91ee0a056d35ce42217bb57d8a081) )

	ROM_REGION( 0x40000, "oki", 0 )
	// Same content as airbusj, pr-200.bin, different sized ROMs
	ROM_LOAD( "4.bin", 0x00000, 0x20000, CRC(21d9bfe3) SHA1(4a69458cd2a6309e389c9e7593ae29d3ef0f8daf) )
	ROM_LOAD( "3.bin", 0x20000, 0x20000, CRC(58cd19e2) SHA1(479f22241bf29f7af67d9679fc6c20f10004fdd8) )
ROM_END

} // anonymous namespace


// Game Drivers

GAME( 1990, airbustr,  0,        airbustr,  airbustr,  airbustr_state, empty_init, ROT0, "Kaneko (Namco license)", "Air Buster: Trouble Specialty Raid Unit (World)",   MACHINE_SUPPORTS_SAVE ) // 891220
GAME( 1990, airbustrj, airbustr, airbustr,  airbustrj, airbustr_state, empty_init, ROT0, "Kaneko (Namco license)", "Air Buster: Trouble Specialty Raid Unit (Japan)",   MACHINE_SUPPORTS_SAVE ) // 891229
GAME( 1990, airbustrb, airbustr, airbustrb, airbustrj, airbustr_state, empty_init, ROT0, "bootleg",                "Air Buster: Trouble Specialty Raid Unit (bootleg)", MACHINE_SUPPORTS_SAVE ) // based on Japan set (891229)
