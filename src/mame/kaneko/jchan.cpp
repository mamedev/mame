// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    Jackie Chan The Kung-Fu Master
    Jackie Chan in Fists of Fire
    (c) Kaneko 1995

    Driver by David Haywood
     based on work by Sebastien Volpe

 started: May 12 2004

Checks done by main code:
- as part of EEPROM data
jchan  : "95/05/24 Jackie ChanVer 1.20"  (both sets report the same info)
jchan2 : "95/11/28 Jackie ChanVer 2.31"
- as (one of) MCU protection cmd
jchan  : "1995/05/24 The kung-Fu Master Jackie Chan   "
jchan2 : "1995/10/24 Fists Of Fire"


 main2sub communication is done within $400000-$403fff (mainsub_shared_ram):
 - $403C02(W) : ]
 - $403C04(W) : ] main68k sets parameters before calling subcpu routine, when required
 - $403C06(W) : ]
 - $403FFE(W) : main68k writes cmd.w, which triggers IT4 on sub68k
 - $400002(W) : subcpu busy status (0=busy, 0xffff=cmd done) - main68k often clears it first and wait 0xffff
 - $400004(W) : subcpu result, read by maincpu, when required
 - $400000(W) : subcpu writes cmd.w, which triggers IT3 on main68k (*)

(*) this happens @ $5b4(cmd 3),$5f2(cmd 1) (diagnostic routine), IT4(cmd 7), and inside subcpu routines $18,$19

99.99% of main->sub communication is done the following:

        clr.w   $400002.l           ; clear (shared ram) sub-cpu busy status word
        move.w  #cmd, $403ffe.l     ; call subcpu
wait    tst.w   $400002.l           ; read (shared ram) sub-cpu busy status word
        beq     wait                ; active wait-loop

***********************************************************************************************

 probably IT 1,2 triggered at a time for maincpu and IT 1,2,3 for subcpu ?

main 68k interrupts (others are rte)
lev 1 : 0x64 : 0000 1ed8 - counter, dsw, controls, + more to find
lev 2 : 0x68 : 0000 1f26 -
lev 3 : 0x6c : 0000 1f68 - comm. with sub68k, triggered when sub writes cmd @ $400000

sub 68k interrupts (others are rte)
lev 1 : 0x64 : 0000 103a -
lev 2 : 0x68 : 0000 1066 -
lev 3 : 0x6c : 0000 10a4 -
lev 4 : 0x70 : 0000 10d0 - comm. with main68k, triggered when main writes cmd @ $403ffe

***********************************************************************************************

Jackie Chan Kung Fu Master
Jackie Chan Fist Of Fire
Kaneko, 1995

Both games run on the same PCB, only the EPROMs, maskROMs and EEPROM data are changed.

PCB Layout
----------

JC-BOARD
JC01X00047
|--------------------------------------------------------------------|
|TA8210 J2M1X1.U56   JC-300-00.U84  LC3564 LC3564      JC-106-00.U171|
|  VOL    YMZ280B    JC-301-00.U85  LC3564 LC3564                    |
|     C4741           J2P1X5.U86                       JC-107-00.U172|
|        58257        J2P1X6.U87          CG24143                    |
|        58257                                         42S4260       |
|        68000        PLSI2032            LH5497                     |
|                     28.6364MHz   42101  CG24173      42S4260       |
|   2003                           42101                             |
|                     BABY004              |-----|     JC-200-00.U177|
|J MC-1091                         LC3564  |VIEW2|                   |
|A     58257                       LC3564  |-CHIP|     JC-100-00.U179|
|M     58257                               |-----|                   |
|M                            LC3564       J2G1X1.U164 JC-101-00.U180|
|A                            LC3564       J2G1X2.U165               |
|     58257  J2P1X1.U67       42101                    JC-102-00.U181|
|SW1  58257  J2P1X2.U68       42101            42S4260               |
|58257       J2P1X3.U69  16MHz    33.3333MHz           JC-103-00.U182|
|58257       J2P1X4.U70  PLSI2032                                    |
|                                           CG24173    JC-104-00.U183|
|  J2D1X1.U13  68000                           42S4240               |
|        93C46                                         JC-105-00.U184|
|   TBS0P01                  LC3564 LC3564     LH5497                |
| SW2                        LC3564 LC3564  CG24143    JC-108-00.U185|
|--------------------------------------------------------------------|
Notes:
          68000 - Clock 16.0MHz (both)
        YMZ280B - Clock 16.0MHz
        TBS0P01 - Clock 16.0MHz (NEC uPD78324 with 32k internal ROM & 1k RAM)
         LC3564 - Sanyo LC3564SM-85 8k x8 SRAM
        42S4260 - NEC 42S4260-70 256k x16 DRAM
          42101 - NEC D42101C-3 910-word x8-bit Line Buffer for Flicker-Free NTSC picture
         LH5497 - SHARP LH5497D-20 CMOS 1kx9 FIFO
          58257 - SONY CXK58257ASP-10L 8kx8 SRAM
            SW1 - Test or reset switch, not populated
            SW2 - 8-position DIP Switch
          C4741 - NEC uPC4741 High Performance Quad Operational Amplifier
          93C46 - AMTEL AT93C46 EEPROM
       PLSI2032 - LATTICE pLSI 2032-80LJ (x2, PLCC44, labeled JCOP099 & JCOP100)
     VIEW2-CHIP - KANEKO VIEW2-CHIP 1633F1208
        BABY004 - KANEKO BABY004 9511EX009 VT-171
CG24143/CG24173 - Fujitsu custom graphics generators
        MC-1091 - Custom I/O ceramic module
         TA8210 - Toshiba TA8210AF 20W BTL 2 Channel Audio Power Amplifier
           2003 - NEC uPA2003 Darlington Transistor Array
          VSync - 59.6010Hz
          HSync - 15.55610kHz

EPROMs:
Location    Rom Type    PCB Label
---------------------------------
U164        27C2001     SPA-7A
U165        27C2001     SPA-7B
not populated (U158)    SPB-4A
not populated (U159)    SPB-4B
U13         27C1001     27C1001A
U56         27C2001     23C8001E
U67         27C040      27C4001
U68         27C040      27C4001
U69         27C040      27C4001
U70         27C040      27C4001
U86         27C040      27C4001
U87         27C040      27C4001

Mask ROMs:
Rom Label                   Label on PCB    Location
----------------------------------------------------
JC-100-00  9511 D           SPA-0           U179
JC-101-00  9511 D           SPA-1           U180
JC-102-00  9511 D           SPA-2           U181
JC-103-00  9511 D           SPA-3           U182
JC-104-00  T39 9510K7092    SPA-4           U183
JC-105-00  T40 9510K7094    SPA-5           U184
JC-108-00  T65 9517K7012    SPA-6           U185
not populated               SPA-7           U186
JC-106-00  T41 9510K7091    SPB-0           U171
JC-107-00  T42 9510K7096    SPB-1           U172
not populated               SPB-2           U173
not populated               SPB-3           U174
JC-200-00  W10 9510K7055    BG-0            U177
not populated               BG-1            U178
JC-300-00  T43 9510K7098    23C16000        U84
JC-301-00  W11 9510K7059    23C16000        U85

***********************************************************************************************

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ymz280b.h"
#include "sknsspr.h"
#include "kaneko_tmap.h"
#include "kaneko_toybox.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class jchan_state : public driver_device
{
public:
	jchan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this,"maincpu")
		, m_subcpu(*this,"sub")
		, m_palette(*this, "palette")
		, m_spritegen(*this, "spritegen%u", 1)
		, m_view2(*this, "view2")
		, m_spriteram(*this, "spriteram_%u", 1)
		, m_sprregs(*this, "sprregs_%u", 1)
		, m_mainsub_shared_ram(*this, "mainsub_shared")
		, m_ctrl(*this, "ctrl")
		, m_io_p1(*this, "P1")
		, m_io_p2(*this, "P2")
		, m_io_system(*this, "SYSTEM")
		, m_io_extra(*this, "EXTRA")
	{ }

	void jchan(machine_config &config);

	void init_jchan();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<palette_device> m_palette;
	required_device_array<sknsspr_device, 2> m_spritegen;
	required_device<kaneko_view2_tilemap_device> m_view2;

	required_shared_ptr_array<u16, 2> m_spriteram;
	required_shared_ptr_array<u16, 2> m_sprregs;
	required_shared_ptr<u16> m_mainsub_shared_ram;
	required_shared_ptr<u16> m_ctrl;

	required_ioport m_io_p1;
	required_ioport m_io_p2;
	required_ioport m_io_system;
	required_ioport m_io_extra;

	std::unique_ptr<bitmap_ind16> m_sprite_bitmap[2];
	std::unique_ptr<u32[]> m_sprite_ram32[2];
	std::unique_ptr<u32[]> m_sprite_regs32[2];
	int m_irq_sub_enable;

	void ctrl_w(u16 data);
	u16 ctrl_r(offs_t offset);
	void main2sub_cmd_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sub2main_cmd_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template<int Chip> void sknsspr_sprite32regs_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	virtual void video_start() override ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(vblank);
	void jchan_main(address_map &map) ATTR_COLD;
	void jchan_sub(address_map &map) ATTR_COLD;
};




/***************************************************************************

 video

***************************************************************************/



// interrupt generation is NOT understood
//  but the order must be something like this,
//  if it is incorrect jchan2 will crash when
//  certain characters win/lose but no finish
//  move was performed
TIMER_DEVICE_CALLBACK_MEMBER(jchan_state::vblank)
{
	int scanline = param;

	if (scanline == 240)
	{
		for (int chip = 0; chip < 2; chip++) // sprites are 1 frame delayed
		{
			for (int i = 0; i < m_spriteram[chip].bytes() / 4; i++)
				m_sprite_ram32[chip][i] = (m_spriteram[chip][i * 2 + 1] << 16) | (m_spriteram[chip][i * 2]);
		}
		m_maincpu->set_input_line(1, HOLD_LINE);
	}

	if (scanline == 11)
		m_maincpu->set_input_line(2, HOLD_LINE);

	if (m_irq_sub_enable)
	{
		if (scanline == 240)
			m_subcpu->set_input_line(1, HOLD_LINE);

		if (scanline == 249)
			m_subcpu->set_input_line(2, HOLD_LINE);

		if (scanline == 11)
			m_subcpu->set_input_line(3, HOLD_LINE);
	}
}


void jchan_state::video_start()
{
	/* so we can use sknsspr.cpp */
	for (int chip = 0; chip < 2; chip++)
	{
		const u32 size = m_spriteram[chip].bytes() / 4;
		m_sprite_ram32[chip] = std::make_unique<u32[]>(size);
		save_pointer(NAME(m_sprite_ram32[chip]), size, chip);
	}

	m_sprite_regs32[0] = std::make_unique<u32[]>(0x40/4);
	m_sprite_regs32[1] = std::make_unique<u32[]>(0x40/4);

	m_sprite_bitmap[0] = std::make_unique<bitmap_ind16>(1024,1024);
	m_sprite_bitmap[1] = std::make_unique<bitmap_ind16>(1024,1024);

	m_spritegen[0]->skns_sprite_kludge(0,0);
	m_spritegen[1]->skns_sprite_kludge(0,0);

	save_item(NAME(m_irq_sub_enable));
	save_pointer(NAME(m_sprite_regs32[0]), 0x40/4);
	save_pointer(NAME(m_sprite_regs32[1]), 0x40/4);
}


u32 jchan_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x7f00, cliprect); // verified

	screen.priority().fill(0, cliprect);

	m_view2->prepare(bitmap, cliprect);

	for (int i = 0; i < 8; i++)
	{
		m_view2->render_tilemap(screen,bitmap,cliprect,i);
	}

	for (int chip = 0; chip < 2; chip++)
	{
		m_spritegen[chip]->skns_draw_sprites(*m_sprite_bitmap[chip], cliprect, m_sprite_ram32[chip].get(), 0x4000, m_sprite_regs32[chip].get());
	}

	bitmap_ind8 *tile_primap = &screen.priority();

	// TODO : verify sprite-tile priorities from real hardware, Check what 15 bit of palette actually working
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 const *const src1 = &m_sprite_bitmap[0]->pix(y);
		u16 const *const src2 = &m_sprite_bitmap[1]->pix(y);
		u8 *const tilepri = &tile_primap->pix(y);
		u16 *const dst =  &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const u16 pixdata1 = src1[x];
			const u16 pixdata2 = src2[x];
			const u16 pridata1 = (pixdata1 >> 14) & 3;
			const u16 pridata2 = (pixdata2 >> 14) & 3;

			const u8 bgpridata = tilepri[x] >> 1;

			if (pridata1 >= bgpridata)
			{
				if (pridata2 >= bgpridata)
				{
					if (pridata2 >= pridata1)
					{
						if (pixdata2 & 0xff)
						{
							dst[x] = (pixdata2 & 0x3fff) | 0x4000;
							tilepri[x] = (pridata2 << 1);
						}
						else if (pixdata1 & 0xff)
						{
							dst[x] = (pixdata1 & 0x3fff) | 0x4000;
							tilepri[x] = (pridata1 << 1);
						}
					}
					else
					{
						if (pixdata1 & 0xff)
						{
							dst[x] = (pixdata1 & 0x3fff) | 0x4000;
							tilepri[x] = (pridata1 << 1);
						}
						else if (pixdata2 & 0xff)
						{
							dst[x] = (pixdata2 & 0x3fff) | 0x4000;
							tilepri[x] = (pridata2 << 1);
						}
					}
				}
				else
				{
					if (pixdata1 & 0xff)
					{
						dst[x] = (pixdata1 & 0x3fff) | 0x4000;
						tilepri[x] = (pridata1 << 1);
					}
				}
			}
			else if (pridata2 >= bgpridata)
			{
				if (pixdata2 & 0xff)
				{
					dst[x] = (pixdata2 & 0x3fff) | 0x4000;
					tilepri[x] = (pridata2 << 1);
				}
			}
		}
	}

	return 0;
}

/***************************************************************************

 controls

***************************************************************************/
/*
    controls handling routine is $21e2a, part of IT 1
    player 1/2 controls are read from $f00000/$f00002 resp.
    $f00006 is read and impacts controls 'decoding'
    $f00000 is the only location also written
*/

void jchan_state::ctrl_w(u16 data)
{
	m_irq_sub_enable = data & 0x8000; // hack / guess!
}

u16 jchan_state::ctrl_r(offs_t offset)
{
	switch (offset)
	{
		case 0/2: return m_io_p1->read();
		case 2/2: return m_io_p2->read();
		case 4/2: return m_io_system->read();
		case 6/2: return m_io_extra->read();
		default: logerror("ctrl_r unknown!"); break;
	}
	return m_ctrl[offset];
}

/***************************************************************************

 memory maps

***************************************************************************/

/* communications - hacky! */
void jchan_state::main2sub_cmd_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_mainsub_shared_ram[0x03ffe/2]);
	m_subcpu->set_input_line(4, HOLD_LINE);
}

// is this called?
void jchan_state::sub2main_cmd_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_mainsub_shared_ram[0x0000/2]);
	m_maincpu->set_input_line(3, HOLD_LINE);
}

/* ram convert for suprnova (requires 32-bit stuff) */
template<int Chip>
void jchan_state::sknsspr_sprite32regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_sprregs[Chip][offset]);
	offset >>= 1;
	m_sprite_regs32[Chip][offset] = (m_sprregs[Chip][offset * 2 + 1] << 16) | (m_sprregs[Chip][offset * 2]);
}


void jchan_state::jchan_main(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x20ffff).ram(); // Work RAM - [A] grid tested, cleared ($9d6-$a54)

	map(0x300000, 0x30ffff).ram().share("mcuram"); //    [G] MCU share
	map(0x330000, 0x330001).w("toybox", FUNC(kaneko_toybox_device::mcu_com0_w));
	map(0x340000, 0x340001).w("toybox", FUNC(kaneko_toybox_device::mcu_com1_w));
	map(0x350000, 0x350001).w("toybox", FUNC(kaneko_toybox_device::mcu_com2_w));
	map(0x360000, 0x360001).w("toybox", FUNC(kaneko_toybox_device::mcu_com3_w));
	map(0x370000, 0x370001).r("toybox", FUNC(kaneko_toybox_device::mcu_status_r));

	map(0x400000, 0x403fff).ram().share("mainsub_shared");

	/* 1st sprite layer */
	map(0x500000, 0x503fff).ram().share("spriteram_1");
	map(0x600000, 0x60003f).ram().w(FUNC(jchan_state::sknsspr_sprite32regs_w<0>)).share("sprregs_1");

	map(0x700000, 0x70ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // palette

	map(0xf00000, 0xf00007).rw(FUNC(jchan_state::ctrl_r), FUNC(jchan_state::ctrl_w)).share("ctrl");

	map(0xf80000, 0xf80001).rw("watchdog", FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
}


void jchan_state::jchan_sub(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram(); // Work RAM - grid tested, cleared ($612-$6dc)

	map(0x400000, 0x403fff).ram().share("mainsub_shared");

	/* VIEW2 Tilemap - [D] grid tested, cleared ($1d84), also cleared at startup ($810-$826) */
	map(0x500000, 0x503fff).m(m_view2, FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x600000, 0x60001f).rw(m_view2, FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));

	/* background sprites */
	map(0x700000, 0x703fff).ram().share("spriteram_2");
	map(0x780000, 0x78003f).ram().w(FUNC(jchan_state::sknsspr_sprite32regs_w<1>)).share("sprregs_2");

	map(0x800000, 0x800003).w("ymz", FUNC(ymz280b_device::write)).umask16(0x00ff); // sound

	map(0xa00000, 0xa00001).rw("watchdog", FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
}


/* input ports */

/* BUTTON1 : weak punch - BUTTON2 : strong punch - BUTTON3 : weak kick - BUTTON 4 : strong kick */
static INPUT_PORTS_START( jchan )
	PORT_START("P1")        /* $f00000.w (-> $2000b1.b) */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CONDITION("DSW1",0x8000,EQUALS,0x8000)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("DSW1",0x8000,EQUALS,0x0000)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")        /* $f00002.w (-> $2000b5.b) */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CONDITION("DSW1",0x8000,EQUALS,0x8000)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("DSW1",0x8000,EQUALS,0x0000)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* $f00004.b */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( DEF_STR( Test ) ) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EXTRA")     /* $f00006.b */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CONDITION("DSW1",0x8000,EQUALS,0x8000)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("DSW1",0x8000,EQUALS,0x8000)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_CONDITION("DSW1",0x8000,EQUALS,0x8000)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("DSW1",0x8000,EQUALS,0x8000)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CONDITION("DSW1",0x8000,EQUALS,0x0000)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CONDITION("DSW1",0x8000,EQUALS,0x0000)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CONDITION("DSW1",0x8000,EQUALS,0x0000)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_CONDITION("DSW1",0x8000,EQUALS,0x0000)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )               /* duplicated Player 1 Button 4 (whatever the layout is) */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )               /* duplicated Player 2 Button 4 (whatever the layout is) */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")      /* provided by the MCU - $200098.b <- $300200 */
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Sound" )             PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Mono ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "Blood Mode" )            PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Special Prize Available" )   PORT_DIPLOCATION("SW1:7")   /* impacts $200008.l many times -> see high-score tables - WTF is it ? */
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Buttons Layout" )        PORT_DIPLOCATION("SW1:8")   /* impacts $200116.l once! -> impacts reading of controls at 0x00021e2a */
	PORT_DIPSETTING(      0x8000, "3+1" )
	PORT_DIPSETTING(      0x0000, "2+2" )
INPUT_PORTS_END

static INPUT_PORTS_START( jchan2 )
	PORT_INCLUDE( jchan )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED( 0x4000, IP_ACTIVE_LOW )                      /* only read in the "test mode" ("Input Test" screen) */
//  PORT_DIPNAME( 0x8000, 0x8000, "Buttons Layout" )             /* impacts $20011e.l once! -> impacts reading of controls at 0x0002a9b2 */
INPUT_PORTS_END


/* machine driver */

void jchan_state::jchan(machine_config &config)
{
	M68000(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &jchan_state::jchan_main);
	TIMER(config, "scantimer").configure_scanline(FUNC(jchan_state::vblank), "screen", 0, 1);

	M68000(config, m_subcpu, 16000000);
	m_subcpu->set_addrmap(AS_PROGRAM, &jchan_state::jchan_sub);

	WATCHDOG_TIMER(config, "watchdog");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(jchan_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 0x10000);

	KANEKO_TMAP(config, m_view2);
	m_view2->set_colbase(0);
	m_view2->set_offset(33, 11, 320, 240);
	m_view2->set_palette(m_palette);

	for (auto &spritegen : m_spritegen)
		SKNS_SPRITE(config, spritegen, 0);

	KANEKO_TOYBOX(config, "toybox", "eeprom", "DSW1", "mcuram", "mcudata");

	EEPROM_93C46_16BIT(config, "eeprom");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16000000));
	ymz.add_route(0, "lspeaker", 1.0);
	ymz.add_route(1, "rspeaker", 1.0);
}

/* ROM loading */

ROM_START( jchan )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "jm01x4.u67", 0x000001, 0x080000, CRC(ace80563) SHA1(1d2774935aa046f258682c953f788cf95348bb0c) ) // rev 4?
	ROM_LOAD16_BYTE( "jm00x4.u68", 0x000000, 0x080000, CRC(08172186) SHA1(7d6ade633bec18c4b2bbd54648c2e141fe925fbe) ) // rev 4?
	ROM_LOAD16_BYTE( "jm11x3.u69", 0x100001, 0x080000, CRC(d2e3f913) SHA1(db2d790fba5351660a9525f545ab1b23dfe319b0) )
	ROM_LOAD16_BYTE( "jm10x3.u70", 0x100000, 0x080000, CRC(ee08fee1) SHA1(5514bd8c625bc7cf8dd5da2f76b760716609b925) )

	ROM_REGION( 0x100000, "sub", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "jsp1x4.u86", 0x000001, 0x080000, CRC(787939d0) SHA1(9d896834bc5f14cd759dd8fcfff3f15e97139238) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF - rev 4?
	ROM_LOAD16_BYTE( "jsp0x4.u87", 0x000000, 0x080000, CRC(1b27383e) SHA1(eaea9722195d0356e5996d7648f31cffc07d3d9a) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF - rev 4?

	ROM_REGION( 0x2000000, "spritegen1", 0 ) /* SPA GFX */
	ROM_LOAD( "jc-100-00.179", 0x0000000, 0x0400000, CRC(578d928c) SHA1(1cfe04f9b02c04f95a85d6fe7c4306a535ff969f) ) // SPA0 kaneko logo
	ROM_LOAD( "jc-101-00.180", 0x0400000, 0x0400000, CRC(7f5e1aca) SHA1(66ed3deedfd55d88e7dcd017b9c2ce523ccb421a) ) // SPA1
	ROM_LOAD( "jc-102-00.181", 0x0800000, 0x0400000, CRC(72caaa68) SHA1(f6b98aa949768a306ac9bc5f9c05a1c1a3fb6c3f) ) // SPA2
	ROM_LOAD( "jc-103-00.182", 0x0c00000, 0x0400000, CRC(4e9e9fc9) SHA1(bf799cdee930b7f71aea4d55c3dd6a760f7478bb) ) // SPA3 title logo? + char select
	ROM_LOAD( "jc-104-00.183", 0x1000000, 0x0200000, CRC(6b2a2e93) SHA1(e34010e39043b67493bcb23a04828ab7cda8ba4d) ) // SPA4
	ROM_LOAD( "jc-105-00.184", 0x1200000, 0x0200000, CRC(73cad1f0) SHA1(5dbe4e318948e4f74bfc2d0d59455d43ba030c0d) ) // SPA5 11xxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "jc-108-00.185", 0x1400000, 0x0200000, CRC(67dd1131) SHA1(96f334378ae0267bdb3dc528635d8d03564bd859) ) // SPA6 text
	ROM_LOAD16_BYTE( "jcs0x3.164", 0x1600000, 0x040000, CRC(9a012cbc) SHA1(b3e7390220c90d55dccfb96397f0af73925e36f9) ) // SPA-7A female portraits
	ROM_LOAD16_BYTE( "jcs1x3.165", 0x1600001, 0x040000, CRC(57ae7c8d) SHA1(4086f638c2aabcee84e838243f0fd15cec5c040d) ) // SPA-7B female portraits

	ROM_REGION( 0x1000000, "spritegen2", 0 ) /* SPB GFX (background sprites) */
	ROM_LOAD( "jc-106-00.171", 0x000000, 0x200000, CRC(bc65661b) SHA1(da28b8fcd7c7a0de427a54be2cf41a1d6a295164) ) // SPB0
	ROM_LOAD( "jc-107-00.172", 0x200000, 0x200000, CRC(92a86e8b) SHA1(c37eddbc9d84239deb543504e27b5bdaf2528f79) ) // SPB1

	ROM_REGION( 0x100000, "view2", 0 ) /* BG GFX */
	ROM_LOAD( "jc-200.00", 0x000000, 0x100000, CRC(1f30c24e) SHA1(0c413fc67c3ec020e6786e7157d82aa242c8d2ad) )

	ROM_REGION( 0x1000000, "ymz", 0 ) /* Audio */
	ROM_LOAD( "jc-301-00.85", 0x000000, 0x100000, CRC(9c5b3077) SHA1(db9a31e1c65d9f12d0f2fb316ced48a02aae089d) )
	ROM_RELOAD(0x100000,0x100000)
	ROM_LOAD( "jc-300-00.84", 0x200000, 0x200000, CRC(13d5b1eb) SHA1(b047594d0f1a71d89b8f072879ccba480f54a483) )
	ROM_LOAD( "jcw0x0.u56",   0x400000, 0x040000, CRC(bcf25c2a) SHA1(b57a563ab5c05b05d133eed3d099c4de997f37e4) )

	ROM_REGION( 0x020000, "mcudata", 0 ) /* MCU Data */
	ROM_LOAD16_WORD_SWAP( "jcd0x2.u13", 0x000000, 0x020000, CRC(011dae3e) SHA1(348c5d4465d92be437f097e89e924d36cc681fc2) ) // rev 2?
ROM_END


ROM_START( jchana )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "jm01x3.u67", 0x000001, 0x080000, CRC(c0adb141) SHA1(de265e1da06e723492e0c2465cd04e25ce1c237f) ) // rev 3?
	ROM_LOAD16_BYTE( "jm00x3.u68", 0x000000, 0x080000, CRC(b1aadc5a) SHA1(0a93693088c0a4b8a79159fb0ebac47d5556d800) ) // rev 3?
	ROM_LOAD16_BYTE( "jm11x3.u69", 0x100001, 0x080000, CRC(d2e3f913) SHA1(db2d790fba5351660a9525f545ab1b23dfe319b0) )
	ROM_LOAD16_BYTE( "jm10x3.u70", 0x100000, 0x080000, CRC(ee08fee1) SHA1(5514bd8c625bc7cf8dd5da2f76b760716609b925) )

	ROM_REGION( 0x100000, "sub", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "jsp1x3.u86", 0x000001, 0x080000, CRC(d15d2b8e) SHA1(e253f2d64fee6627f68833b441f41ea6bbb3ab07) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF - rev 3?
	ROM_LOAD16_BYTE( "jsp0x3.u87", 0x000000, 0x080000, CRC(ebec50b1) SHA1(57d7bd728349c2b9d662bcf20a3be92902cb3ffb) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF - rev 3?

	ROM_REGION( 0x2000000, "spritegen1", 0 ) /* SPA GFX */
	ROM_LOAD( "jc-100-00.179", 0x0000000, 0x0400000, CRC(578d928c) SHA1(1cfe04f9b02c04f95a85d6fe7c4306a535ff969f) ) // SPA0 kaneko logo
	ROM_LOAD( "jc-101-00.180", 0x0400000, 0x0400000, CRC(7f5e1aca) SHA1(66ed3deedfd55d88e7dcd017b9c2ce523ccb421a) ) // SPA1
	ROM_LOAD( "jc-102-00.181", 0x0800000, 0x0400000, CRC(72caaa68) SHA1(f6b98aa949768a306ac9bc5f9c05a1c1a3fb6c3f) ) // SPA2
	ROM_LOAD( "jc-103-00.182", 0x0c00000, 0x0400000, CRC(4e9e9fc9) SHA1(bf799cdee930b7f71aea4d55c3dd6a760f7478bb) ) // SPA3 title logo? + char select
	ROM_LOAD( "jc-104-00.183", 0x1000000, 0x0200000, CRC(6b2a2e93) SHA1(e34010e39043b67493bcb23a04828ab7cda8ba4d) ) // SPA4
	ROM_LOAD( "jc-105-00.184", 0x1200000, 0x0200000, CRC(73cad1f0) SHA1(5dbe4e318948e4f74bfc2d0d59455d43ba030c0d) ) // SPA5 11xxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "jc-108-00.185", 0x1400000, 0x0200000, CRC(67dd1131) SHA1(96f334378ae0267bdb3dc528635d8d03564bd859) ) // SPA6 text
	ROM_LOAD16_BYTE( "jcs0x3.164", 0x1600000, 0x040000, CRC(9a012cbc) SHA1(b3e7390220c90d55dccfb96397f0af73925e36f9) ) // SPA-7A female portraits
	ROM_LOAD16_BYTE( "jcs1x3.165", 0x1600001, 0x040000, CRC(57ae7c8d) SHA1(4086f638c2aabcee84e838243f0fd15cec5c040d) ) // SPA-7B female portraits

	ROM_REGION( 0x1000000, "spritegen2", 0 ) /* SPB GFX (background sprites) */
	ROM_LOAD( "jc-106-00.171", 0x000000, 0x200000, CRC(bc65661b) SHA1(da28b8fcd7c7a0de427a54be2cf41a1d6a295164) ) // SPB0
	ROM_LOAD( "jc-107-00.172", 0x200000, 0x200000, CRC(92a86e8b) SHA1(c37eddbc9d84239deb543504e27b5bdaf2528f79) ) // SPB1

	ROM_REGION( 0x100000, "view2", 0 ) /* BG GFX */
	ROM_LOAD( "jc-200.00", 0x000000, 0x100000, CRC(1f30c24e) SHA1(0c413fc67c3ec020e6786e7157d82aa242c8d2ad) )

	ROM_REGION( 0x1000000, "ymz", 0 ) /* Audio */
	ROM_LOAD( "jc-301-00.85", 0x000000, 0x100000, CRC(9c5b3077) SHA1(db9a31e1c65d9f12d0f2fb316ced48a02aae089d) )
	ROM_RELOAD(0x100000,0x100000)
	ROM_LOAD( "jc-300-00.84", 0x200000, 0x200000, CRC(13d5b1eb) SHA1(b047594d0f1a71d89b8f072879ccba480f54a483) )
	ROM_LOAD( "jcw0x0.u56",   0x400000, 0x040000, CRC(bcf25c2a) SHA1(b57a563ab5c05b05d133eed3d099c4de997f37e4) )

	ROM_REGION( 0x020000, "mcudata", 0 ) /* MCU Data */
	ROM_LOAD16_WORD_SWAP( "jcd0x1.u13", 0x000000, 0x020000, CRC(2a41da9c) SHA1(7b1ba0efc0544e276196b9605df1881fde871708) ) // rev 1?
ROM_END


ROM_START( jchan2 ) /* Some kind of semi-sequel? Mask ROMs dumped and confirmed to be the same */
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "j2p1x1.u67", 0x000001, 0x080000, CRC(5448c4bc) SHA1(447835275d5454f86a51879490a6b22b06a23e81) )
	ROM_LOAD16_BYTE( "j2p1x2.u68", 0x000000, 0x080000, CRC(52104ab9) SHA1(d6647e628662bdb832270540ece18b265b7ce62d) )
	ROM_LOAD16_BYTE( "j2p1x3.u69", 0x100001, 0x080000, CRC(8763ebca) SHA1(daf6af42a34802ef9aa996e340e218779bad695f) )
	ROM_LOAD16_BYTE( "j2p1x4.u70", 0x100000, 0x080000, CRC(0f8e5e69) SHA1(1f71042458f76b7d99382db6412fb6c362cd3ded) )

	ROM_REGION( 0x100000, "sub", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "j2p1x5.u86", 0x000001, 0x080000, CRC(dc897725) SHA1(d3e94bac96497deb2f79996c2d4a349f6da5b1d6) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "j2p1x6.u87", 0x000000, 0x080000, CRC(594224f9) SHA1(bc546a98c5f3c5b08f521c54a4b0e9e2cdf83ced) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x2000000, "spritegen1", 0 ) /* SPA GFX */
	ROM_LOAD( "jc-100-00.179", 0x0000000, 0x0400000, CRC(578d928c) SHA1(1cfe04f9b02c04f95a85d6fe7c4306a535ff969f) ) // SPA0 kaneko logo
	ROM_LOAD( "jc-101-00.180", 0x0400000, 0x0400000, CRC(7f5e1aca) SHA1(66ed3deedfd55d88e7dcd017b9c2ce523ccb421a) ) // SPA1
	ROM_LOAD( "jc-102-00.181", 0x0800000, 0x0400000, CRC(72caaa68) SHA1(f6b98aa949768a306ac9bc5f9c05a1c1a3fb6c3f) ) // SPA2
	ROM_LOAD( "jc-103-00.182", 0x0c00000, 0x0400000, CRC(4e9e9fc9) SHA1(bf799cdee930b7f71aea4d55c3dd6a760f7478bb) ) // SPA3 title logo? + char select
	ROM_LOAD( "jc-104-00.183", 0x1000000, 0x0200000, CRC(6b2a2e93) SHA1(e34010e39043b67493bcb23a04828ab7cda8ba4d) ) // SPA4
	ROM_LOAD( "jc-105-00.184", 0x1200000, 0x0200000, CRC(73cad1f0) SHA1(5dbe4e318948e4f74bfc2d0d59455d43ba030c0d) ) // SPA5 11xxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "jc-108-00.185", 0x1400000, 0x0200000, CRC(67dd1131) SHA1(96f334378ae0267bdb3dc528635d8d03564bd859) ) // SPA6 text
	ROM_LOAD16_BYTE( "j2g1x1.164", 0x1600000, 0x080000, CRC(66a7ea6a) SHA1(605cbc1eb50fb0decbea790f2a11e999d5fde762) ) // SPA-7A female portraits
	ROM_LOAD16_BYTE( "j2g1x2.165", 0x1600001, 0x080000, CRC(660e770c) SHA1(1e385a6ee83559b269d2179e6c247238c0f3c850) ) // SPA-7B female portraits

	ROM_REGION( 0x1000000, "spritegen2", 0 ) /* SPB GFX (background sprites) */
	ROM_LOAD( "jc-106-00.171", 0x000000, 0x200000, CRC(bc65661b) SHA1(da28b8fcd7c7a0de427a54be2cf41a1d6a295164) ) // SPB0
	ROM_LOAD( "jc-107-00.172", 0x200000, 0x200000, CRC(92a86e8b) SHA1(c37eddbc9d84239deb543504e27b5bdaf2528f79) ) // SPB1

	ROM_REGION( 0x100000, "view2", 0 ) /* BG GFX */
	ROM_LOAD( "jc-200.00", 0x000000, 0x100000, CRC(1f30c24e) SHA1(0c413fc67c3ec020e6786e7157d82aa242c8d2ad) )

	ROM_REGION( 0x1000000, "ymz", 0 ) /* Audio */
	ROM_LOAD( "jc-301-00.85", 0x000000, 0x100000, CRC(9c5b3077) SHA1(db9a31e1c65d9f12d0f2fb316ced48a02aae089d) )
	ROM_RELOAD(0x100000,0x100000)
	ROM_LOAD( "jc-300-00.84", 0x200000, 0x200000, CRC(13d5b1eb) SHA1(b047594d0f1a71d89b8f072879ccba480f54a483) )
	ROM_LOAD( "j2m1x1.u56",   0x400000, 0x040000, CRC(baf6e25e) SHA1(6b02f3eb1eafcd43022a9f60f98573d02277adfe) )

	ROM_REGION( 0x020000, "mcudata", 0 ) /* MCU data */
	ROM_LOAD16_WORD_SWAP( "j2d1x1.u13", 0x000000, 0x020000, CRC(b2b7fc90) SHA1(1b90c13bb41a313c4ed791a15d56073a7c29928b) )
ROM_END

void jchan_state::init_jchan()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x403ffe, 0x403fff, write16s_delegate(*this, FUNC(jchan_state::main2sub_cmd_w)));
	m_subcpu->space(AS_PROGRAM).install_write_handler(0x400000, 0x400001, write16s_delegate(*this, FUNC(jchan_state::sub2main_cmd_w)));
}

} // anonymous namespace


/* game drivers */
GAME( 1995, jchan,      0, jchan, jchan,  jchan_state, init_jchan, ROT0, "Kaneko", "Jackie Chan - The Kung-Fu Master (rev 4?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, jchana, jchan, jchan, jchan,  jchan_state, init_jchan, ROT0, "Kaneko", "Jackie Chan - The Kung-Fu Master (rev 3?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, jchan2,     0, jchan, jchan2, jchan_state, init_jchan, ROT0, "Kaneko", "Jackie Chan in Fists of Fire",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
