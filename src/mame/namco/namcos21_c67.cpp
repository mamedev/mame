// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*
    NOTES:

    Air Combat:
         priority issues

TODO:   namcoic.c: in StarBlade, the sprite list is stored at a different location during startup tests.
        What register controls this?

TODO:   Map lamps/vibration outputs as used by StarBlade (and possibly other titles).
        These likely involve the MCU.

---------------------------------------------------------------------------

DSP RAM is shared with the 68000 CPUs and master DSP.
The memory map below reflects DSP RAM as seen by the 68000 CPUs.

    0x200000: ROM:
    0x200010: RAM:
    0x200020: PTR:
    0x200024: <checksum>
    0x200028: <checksum>
    0x200030: SMU: // "NO RESPONS" (DSP)
    0x200040: IDC: // "NO RESPONS" (DSP)
    0x200050: CPU: BOOTING..COMPLETE
    0x200060: DSP:
    0x200070: CRC: OK from cpu
    0x200080: CRC:    from dsp
    0x200090: ID:
    0x2000a0: B-M:
    0x2000b0: P-M:
    0x2000c0: S-M:
    0x200100    status: 2=upload needed, 4=error (abort)
    0x200102    status
    0x200104    0x0002
    0x200106    addr written by main cpu
    0x20010a    point rom checksum (starblade expects 0xed53)
    0x20010c    point rom checksum (starblade expects 0xd5df)
    0x20010e    1 : upload-code-to-dsp request trigger
    0x200110    status
    0x200112    status
    0x200114    master dsp code size
    0x200116    slave dsp code size
    0x200120    upload source1 addr hi
    0x200122    upload source1 addr lo
    0x200124    upload source2 addr hi
    0x200126    upload source2 addr lo
    0x200200    enable
    0x200202    status
    0x200206    work page select
    0x200208   0xa2c2 (air combat)
    0x208000..0x2080ff  camera attributes for page#0
    0x208200..0x208fff  3d object attribute display list for page#0
    0x20c000..0x20c0ff  camera attributes for page#1
    0x20c200..0x20cfff  3d object attribute display list for page#1

       Starblade Cybersled AirCombat22 Solvalou
[400]:= 00 0000   00 0000   00 0000    00 0000
[402]:= 00 0011   00 0011   00 0011    00 0011
[404]:= 00 0000   00 0000   00 0000    00 0000
[406]:= 00 0000   00 0000   00 0000    00 0000
[408]:= 10 1002   10 1000   10 1000    10 1000
[40a]:= 02 0000   00 0000   00 0000    00 ffff
[40c]:= 00 1040   00 023a   00 1000    ff ffff
[40e]:= 00 0000   00 0000   00 0000    ff ffff
[410]:= 10 1034   02 0258   10 1000    ff ffff
[412]:= 40 0000   3a 0000   00 0000    ff ffff
[414]:= 00 1030   00 027a   00 1000    ff ffff
[416]:= 00 0000   00 0000   00 0000    ff ffff
[418]:= 10 1030   02 02a2   10 1000    ff ffff
[41a]:= 34 0000   58 0000   00 0000    ff ffff
[41c]:= 00 1030   00 02c2   00 1000    ff ffff
[41e]:= 00 0000   00 0000   00 0000    ff ffff
[420]:= 10 1030   02 02e6   10 1000    ff ffff

---------------------------------------------------------------------------

Thanks to Aaron Giles for originally making sense of the Point ROM data.

Point data in ROMS (signed 24 bit words) encodes the 3d primitives.

The first part of the Point ROMs is an address table.

Given an object index, this table provides an address into the second part of
the ROM.

The second part of the ROM is a series of display lists.
This is a sequence of pointers to actual polygon data. There may be
more than one, and the list is terminated by $ffffff.

The remainder of the ROM is a series of polygon data. The first word of each
entry is the length of the entry (in words, not counting the length word).

The rest of the data in each entry is organized as follows:

length (1 word)
quad index (1 word) - this increments with each entry
vertex count (1 word) - the number of vertices encoded
unknown value (1 word) - almost always 0; depth bias
vertex list (n x 3 words)
quad count (1 word) - the number of quads to draw
quad primitives (n x 5 words) - color code and four vertex indices

-----------------------------------------------------------------------
Board 1 : DSP Board - 1st PCB. (Uppermost)
DSP : 1 x Master TMS320C25 (C67) 4 x Slave TMS320C25 (C67) each connected to a Namco Custom chip 342
OSC: 40.000MHz
RAM: HM62832 x 2, M5M5189 x 4, ISSI IS61C68 x 16
ROMS: TMS27C040
Custom Chips:
4 x Namco Custom 327 (24 pin NDIP), each one located next to a chip 67.
4 x Namco Custom chip 342 (160 pin PQFP), there are 3 leds (red/green/yellow) connected to each 342 chip. (12 leds total)
2 x Namco Custom 197 (28 pin NDIP)
Namco Custom chip 317 IDC (180 pin PQFP)
Namco Custom chip 195 (160 pin PQFP)
-----------------------------------------------------------------------
Board 2 : Unknown Board - 2nd PCB (no roms)
OSC: 20.000MHz
RAM: HM62256 x 10, 84256 x 4, CY7C128 x 5, M5M5178 x 4
OTHER Chips:
MB8422-90LP
L7A0565 316 (111) x 1 (100 PIN PQFP)
150 (64 PIN PQFP)
167 (128 PIN PQFP)
L7A0564 x 2 (100 PIN PQFP)
157 x 16 (24 PIN NDIP)
-----------------------------------------------------------------------
Board 3 : CPU Board - 3rd PCB (looks very similar to Namco System 2 CPU PCB)
CPU: MC68000P12 x 2 @ 12 MHz (16-bit)
Sound CPU: MC68B09EP (3 MHz)
Sound Chips: C140 24-channel PCM (Sound Effects), YM2151 (Music), YM3012 (?)
XTAL: 3.579545 MHz
OSC: 49.152 MHz
RAM: MB8464 x 2, MCM2018 x 2, HM65256 x 4, HM62256 x 2

Other Chips:
Sharp PC900 - Opto-isolator
Sharp PC910 - Opto-isolator
HN58C65P (EEPROM)
MB3771
MB87077-SK x 2 (24 pin NDIP, located in sound section)
LB1760 (16 pin DIP, located next to SYS87B-2B)
CY7C132 (48 PIN DIP)

Namco Custom:
148 x 2 (64 pin PQFP)
C68 (64 pin PQFP)
139 (64 pin PQFP)
137 (28 pin NDIP)
149 (28 pin NDIP, near C68)
-----------------------------------------------------------------------
Board 4 : 4th PCB (bottom-most)
OSC: 38.76922 MHz
There is a 6 wire plug joining this PCB with the CPU PCB. It appears to be video cable (RGB, Sync etc..)
Jumpers:
JP7 INTERLACE = SHORTED (Other setting is NON-INTERLACE)
JP8 68000 = SHORTED (Other setting is 68020)
Namco Custom Chips:
C355 (160 pin PQFP)
187 (120 pin PQFP)
138 (64 pin PQFP)
165 (28 pin NDIP)
-----------------------------------------------------------------------

-------------------
Air Combat by NAMCO
-------------------
malcor


Location        Device     File ID      Checksum
-------------------------------------------------
CPU68  1J       27C4001    MPR-L.AC1      9859   [ main program ]  [ rev AC1 ]
CPU68  3J       27C4001    MPR-U.AC1      97F1   [ main program ]  [ rev AC1 ]
CPU68  1J       27C4001    MPR-L.AC2      C778   [ main program ]  [ rev AC2 ]
CPU68  3J       27C4001    MPR-U.AC2      6DD9   [ main program ]  [ rev AC2 ]
CPU68  1C      MB834000    EDATA1-L.AC1   7F77   [    data      ]
CPU68  3C      MB834000    EDATA1-U.AC1   FA2F   [    data      ]
CPU68  3A      MB834000    EDATA-U.AC1    20F2   [    data      ]
CPU68  1A      MB834000    EDATA-L.AC1    9E8A   [    data      ]
CPU68  8J        27C010    SND0.AC1       71A8   [  sound prog  ]
CPU68  12B     MB834000    VOI0.AC1       08CF   [   voice 0    ]
CPU68  12C     MB834000    VOI1.AC1       925D   [   voice 1    ]
CPU68  12D     MB834000    VOI2.AC1       C498   [   voice 2    ]
CPU68  12E     MB834000    VOI3.AC1       DE9F   [   voice 3    ]
CPU68  4C        27C010    SPR-L.AC1      473B   [ slave prog L ]  [ rev AC1 ]
CPU68  6C        27C010    SPR-U.AC1      CA33   [ slave prog U ]  [ rev AC1 ]
CPU68  4C        27C010    SPR-L.AC2      08CE   [ slave prog L ]  [ rev AC2 ]
CPU68  6C        27C010    SPR-U.AC2      A3F1   [ slave prog U ]  [ rev AC2 ]
OBJ(B) 5S       HN62344    OBJ0.AC1       CB72   [ object data  ]
OBJ(B) 5X       HN62344    OBJ1.AC1       85E2   [ object data  ]
OBJ(B) 3S       HN62344    OBJ2.AC1       89DC   [ object data  ]
OBJ(B) 3X       HN62344    OBJ3.AC1       58FF   [ object data  ]
OBJ(B) 4S       HN62344    OBJ4.AC1       46D6   [ object data  ]
OBJ(B) 4X       HN62344    OBJ5.AC1       7B91   [ object data  ]
OBJ(B) 2S       HN62344    OBJ6.AC1       5736   [ object data  ]
OBJ(B) 2X       HN62344    OBJ7.AC1       6D45   [ object data  ]
OBJ(B) 17N     PLHS18P8    3P0BJ3         4342
OBJ(B) 17N     PLHS18P8    3POBJ4         1143
DSP    2N       HN62344    AC1-POIL.L     8AAF   [   DSP data   ]
DSP    2K       HN62344    AC1-POIL.L     CF90   [   DSP data   ]
DSP    2E       HN62344    AC1-POIH       4D02   [   DSP data   ]
DSP    17D     GAL16V8A    3PDSP5         6C00

NOTE:  CPU68  - CPU board        2252961002  (2252971002)
       OBJ(B) - Object board     8623961803  (8623963803)
       DSP    - DSP board        8623961703  (8623963703)
       PGN(C) - PGN board        2252961300  (8623963600)

       Namco System 21 Hardware

       ROMs that have the same locations are different revisions
       of the same ROMs (AC1 or AC2).

Jumper settings:

Location    Position set    alt. setting
----------------------------------------

CPU68 PCB:

  JP2          /D-ST           /VBL
  JP3

*****************************

Namco System 21 Video Hardware

- sprite hardware is identical to Namco System NB1
- there are no tilemaps
- 3d graphics are managed by DSP processors

  Palette:
    0x0000..0x1fff  sprite palettes (0x10 sets of 0x100 colors)

    0x2000..0x3fff  polygon palette bank0 (0x10 sets of 0x200 colors)
        (in starblade, some palette animation effects are performed here)

    0x4000..0x5fff  polygon palette bank1 (0x10 sets of 0x200 colors)

    0x6000..0x7fff  polygon palette bank2 (0x10 sets of 0x200 colors)

    The polygon-dedicated color sets within a bank typically increase in
    intensity from very dark to full intensity.

    Probably the selected palette is determined by most significant bits of z-code.
    This is not yet hooked up.

*/

#include "emu.h"
#include "screen.h"
#include "emupal.h"
#include "speaker.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6805/m6805.h"
#include "cpu/m6809/m6809.h"
#include "machine/nvram.h"
#include "namcoio_gearbox.h"
#include "machine/timer.h"
#include "namco_c139.h"
#include "namco_c148.h"
#include "namco68.h"
#include "namco_c67.h"
#include "namcos21_dsp_c67.h"
#include "namco_c355spr.h"
#include "namcos21_3d.h"
#include "sound/c140.h"
#include "sound/ymopm.h"
#include "emupal.h"

#define ENABLE_LOGGING      0


namespace {

class namcos21_c67_state : public driver_device
{
public:
	namcos21_c67_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_slave(*this, "slave"),
		m_c68(*this, "c68mcu"),
		m_sci(*this, "sci"),
		m_master_intc(*this, "master_intc"),
		m_slave_intc(*this, "slave_intc"),
		m_c140(*this, "c140"),
		m_c355spr(*this, "c355spr"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_audiobank(*this, "audiobank"),
		m_c140_region(*this, "c140"),
		m_dpram(*this, "dpram"),
		m_namcos21_3d(*this, "namcos21_3d"),
		m_namcos21_dsp_c67(*this, "namcos21dsp_c67")
	{ }

	void configure_c148_standard(machine_config &config);
	void namcos21(machine_config &config);
	void cybsled(machine_config &config);
	void solvalou(machine_config &config);
	void aircomb(machine_config &config);
	void starblad(machine_config &config);

	void init_solvalou();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_slave;
	required_device<namcoc68_device> m_c68;
	required_device<namco_c139_device> m_sci;
	required_device<namco_c148_device> m_master_intc;
	required_device<namco_c148_device> m_slave_intc;
	required_device<c140_device> m_c140;
	required_device<namco_c355spr_device> m_c355spr;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_memory_bank m_audiobank;
	required_region_ptr<u16> m_c140_region;
	required_shared_ptr<uint8_t> m_dpram;
	required_device<namcos21_3d_device> m_namcos21_3d;
	required_device<namcos21_dsp_c67_device> m_namcos21_dsp_c67;

	uint16_t m_video_enable;

	uint16_t video_enable_r();
	void video_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t dpram_word_r(offs_t offset);
	void dpram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t dpram_byte_r(offs_t offset);
	void dpram_byte_w(offs_t offset, uint8_t data);

	void eeprom_w(offs_t offset, uint8_t data);
	uint8_t eeprom_r(offs_t offset);

	void sound_bankselect_w(uint8_t data);

	void sound_reset_w(uint8_t data);
	void system_reset_w(uint8_t data);
	void reset_all_subcpus(int state);

	std::unique_ptr<uint8_t[]> m_eeprom;

	TIMER_DEVICE_CALLBACK_MEMBER(screen_scanline);

	void yield_hack(int state);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void configure_c68_namcos21(machine_config &config);

	void common_map(address_map &map) ATTR_COLD;
	void master_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;

	void sound_map(address_map &map) ATTR_COLD;
	void c140_map(address_map &map) ATTR_COLD;
};


uint32_t namcos21_c67_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//uint8_t *videoram = m_gpu_videoram.get();
	int pivot = 3;
	int pri;
	bitmap.fill(0xff, cliprect );
	screen.priority().fill(0, cliprect);
	m_c355spr->build_sprite_list_and_render_sprites(cliprect); // TODO : buffered?

	m_c355spr->draw(screen, bitmap, cliprect, 2 );

	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0x7fc0, 0x7ffe);

	m_c355spr->draw(screen, bitmap, cliprect, 0 );
	m_c355spr->draw(screen, bitmap, cliprect, 1 );

	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0, 0x7fbf);

	/* draw high priority 2d sprites */
	for( pri=pivot; pri<8; pri++ )
	{
		m_c355spr->draw(screen, bitmap, cliprect, pri );
	}
	return 0;
}

uint16_t namcos21_c67_state::video_enable_r()
{
	return m_video_enable;
}

void namcos21_c67_state::video_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA( &m_video_enable ); /* 0x40 = enable */
	if( m_video_enable!=0 && m_video_enable!=0x40 )
	{
		logerror( "unexpected video_enable_w=0x%x\n", m_video_enable );
	}
}

/***********************************************************/

/* dual port ram memory handlers */

uint16_t namcos21_c67_state::dpram_word_r(offs_t offset)
{
	return m_dpram[offset];
}

void namcos21_c67_state::dpram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if( ACCESSING_BITS_0_7 )
	{
		m_dpram[offset] = data&0xff;
	}
}

uint8_t namcos21_c67_state::dpram_byte_r(offs_t offset)
{
	return m_dpram[offset];
}

void namcos21_c67_state::dpram_byte_w(offs_t offset, uint8_t data)
{
	m_dpram[offset] = data;
}

/******************************************************************************/

/*************************************************************/
/* MASTER 68000 CPU Memory declarations                      */
/*************************************************************/

void namcos21_c67_state::common_map(address_map &map)
{
	map(0x280000, 0x280001).nopw(); /* written once on startup */
	map(0x400000, 0x400001).w(m_namcos21_dsp_c67, FUNC(namcos21_dsp_c67_device::pointram_control_w));
	map(0x440000, 0x440001).rw(m_namcos21_dsp_c67, FUNC(namcos21_dsp_c67_device::pointram_data_r), FUNC(namcos21_dsp_c67_device::pointram_data_w));
	map(0x440002, 0x47ffff).nopw(); /* (?) Air Combat */
	map(0x480000, 0x4807ff).rw(m_namcos21_dsp_c67, FUNC(namcos21_dsp_c67_device::namcos21_depthcue_r), FUNC(namcos21_dsp_c67_device::namcos21_depthcue_w)); /* Air Combat */
	map(0x700000, 0x71ffff).rw(m_c355spr, FUNC(namco_c355spr_device::spriteram_r), FUNC(namco_c355spr_device::spriteram_w));
	map(0x720000, 0x720007).rw(m_c355spr, FUNC(namco_c355spr_device::position_r), FUNC(namco_c355spr_device::position_w));
	map(0x740000, 0x74ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x750000, 0x75ffff).ram().w(m_palette, FUNC(palette_device::write16_ext)).share("palette_ext");
	map(0x760000, 0x760001).rw(FUNC(namcos21_c67_state::video_enable_r), FUNC(namcos21_c67_state::video_enable_w));
	map(0x800000, 0x8fffff).rom().region("data", 0);
	map(0x900000, 0x90ffff).ram().share("sharedram");
	map(0xa00000, 0xa00fff).rw(FUNC(namcos21_c67_state::dpram_word_r), FUNC(namcos21_c67_state::dpram_word_w));
	map(0xb00000, 0xb03fff).rw(m_sci, FUNC(namco_c139_device::ram_r), FUNC(namco_c139_device::ram_w));
	map(0xb80000, 0xb8000f).m(m_sci, FUNC(namco_c139_device::regs_map));
	map(0xc00000, 0xcfffff).rom().mirror(0x100000).region("edata", 0);
}

void namcos21_c67_state::master_map(address_map &map)
{
	common_map(map);
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram(); /* private work RAM */
	map(0x180000, 0x183fff).rw(FUNC(namcos21_c67_state::eeprom_r), FUNC(namcos21_c67_state::eeprom_w)).umask16(0x00ff);
	map(0x1c0000, 0x1fffff).m(m_master_intc, FUNC(namco_c148_device::map));
	map(0x200000, 0x20ffff).rw(m_namcos21_dsp_c67, FUNC(namcos21_dsp_c67_device::dspram16_r), FUNC(namcos21_dsp_c67_device::dspram16_hack_w));
}

void namcos21_c67_state::slave_map(address_map &map)
{
	common_map(map);
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x13ffff).ram(); /* private work RAM */
	map(0x1c0000, 0x1fffff).m(m_slave_intc, FUNC(namco_c148_device::map));
	map(0x200000, 0x20ffff).rw(m_namcos21_dsp_c67, FUNC(namcos21_dsp_c67_device::dspram16_r), FUNC(namcos21_dsp_c67_device::dspram16_w));
}

/*************************************************************/
/* SOUND 6809 CPU Memory declarations                        */
/*************************************************************/

void namcos21_c67_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr("audiobank"); /* banked */
	map(0x3000, 0x3003).nopw(); /* ? */
	map(0x4000, 0x4001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x5000, 0x51ff).mirror(0x0e00).rw(m_c140, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w));
	map(0x6000, 0x61ff).mirror(0x0e00).rw(m_c140, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w)); // mirrored
	map(0x7000, 0x77ff).mirror(0x0800).rw(FUNC(namcos21_c67_state::dpram_byte_r), FUNC(namcos21_c67_state::dpram_byte_w)).share("dpram");
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).nopw(); /* amplifier enable on 1st write */
	map(0xc000, 0xffff).nopw(); /* avoid debug log noise; games write frequently to 0xe000 */
	map(0xc000, 0xc001).w(FUNC(namcos21_c67_state::sound_bankselect_w));
	map(0xd001, 0xd001).nopw(); /* watchdog */
	map(0xd000, 0xffff).rom().region("audiocpu", 0x01000);
}

void namcos21_c67_state::c140_map(address_map &map)
{
	map.global_mask(0x7fffff);
	// TODO: LSB not used? verify from schematics/real hardware
	map(0x000000, 0x7fffff).lr16([this](offs_t offset) { return m_c140_region[((offset & 0x300000) >> 1) | (offset & 0x7ffff)]; }, "c140_rom_r");
}

/*************************************************************/
/* I/O HD63705 MCU Memory declarations                       */
/*************************************************************/

void namcos21_c67_state::configure_c68_namcos21(machine_config &config)
{
	NAMCOC68(config, m_c68, 8000000);
	m_c68->in_pb_callback().set_ioport("MCUB");
	m_c68->in_pc_callback().set_ioport("MCUC");
	m_c68->in_ph_callback().set_ioport("MCUH");
	m_c68->in_pdsw_callback().set_ioport("DSW");
	m_c68->di0_in_cb().set_ioport("MCUDI0");
	m_c68->di1_in_cb().set_ioport("MCUDI1");
	m_c68->di2_in_cb().set_ioport("MCUDI2");
	m_c68->di3_in_cb().set_ioport("MCUDI3");
	m_c68->an0_in_cb().set_ioport("AN0");
	m_c68->an1_in_cb().set_ioport("AN1");
	m_c68->an2_in_cb().set_ioport("AN2");
	m_c68->an3_in_cb().set_ioport("AN3");
	m_c68->an4_in_cb().set_ioport("AN4");
	m_c68->an5_in_cb().set_ioport("AN5");
	m_c68->an6_in_cb().set_ioport("AN6");
	m_c68->an7_in_cb().set_ioport("AN7");
	m_c68->dp_in_callback().set(FUNC(namcos21_c67_state::dpram_byte_r));
	m_c68->dp_out_callback().set(FUNC(namcos21_c67_state::dpram_byte_w));
}

/*************************************************************/
/*                                                           */
/*  NAMCO SYSTEM 21 INPUT PORTS                              */
/*                                                           */
/*************************************************************/

static INPUT_PORTS_START( s21default )
	PORT_START("MCUB")     /* 63B05Z0 - PORT B */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("MCUC")     /* 63B05Z0 - PORT C & SCI */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_0) PORT_TOGGLE // alt test mode switch
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("AN0")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN1")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x60,0x9f) PORT_SENSITIVITY(15) PORT_KEYDELTA(10)
	PORT_START("AN2")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x60,0x9f) PORT_SENSITIVITY(20) PORT_KEYDELTA(10)
	PORT_START("AN3")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN4")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN5")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 5 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN6")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 6 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN7")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 7 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MCUH")     /* 63B05Z0 - PORT H */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")       /* 63B05Z0 - $2000 DIP SW */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "DSW2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "PCM ROM")
	PORT_DIPSETTING(    0x20, "2M" )
	PORT_DIPSETTING(    0x00, "4M" )
	PORT_DIPNAME( 0x40, 0x40, "DSW7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Screen Stop")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MCUDI0")     /* 63B05Z0 - $3000 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("MCUDI1")     /* 63B05Z0 - $3001 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("MCUDI2")     /* 63B05Z0 - $3002 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("MCUDI3")     /* 63B05Z0 - $3003 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// the default inc/dec analog keys have been chosen to map 'tank' style inputs found on Assault.
// this makes the game easier to use with the keyboard, providing a familiar left/right stick mapping
// ports are limited to 10/ef because otherwise, even when calibrated, the game will act as if the
// inputs wrap around when they hit the maximum, causing undesired movement
static INPUT_PORTS_START( cybsled )
	PORT_INCLUDE(s21default)

	PORT_MODIFY("AN0")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xef) /* using 0x00 / 0xff causes controls to malfunction */ PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) /* right joystick: vertical */
	PORT_MODIFY("AN1")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xef) /* using 0x00 / 0xff causes controls to malfunction */ PORT_CODE_DEC(KEYCODE_E) PORT_CODE_INC(KEYCODE_D) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) /* left joystick: vertical */
	PORT_MODIFY("AN2")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xef) /* using 0x00 / 0xff causes controls to malfunction */ PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) /* right joystick: horizontal */
	PORT_MODIFY("AN3")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xef) /* using 0x00 / 0xff causes controls to malfunction */ PORT_CODE_DEC(KEYCODE_S) PORT_CODE_INC(KEYCODE_F) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) /* left joystick: horizontal */
	PORT_MODIFY("AN4")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN5")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 5 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN6")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 6 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN7")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 7 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("MCUH")        /* 63B05Z0 - PORT H */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Viewport Change Button")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Missile Button")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Gun Button")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( starblad )
	PORT_INCLUDE(s21default)

	PORT_MODIFY("AN1")      /* IN#3: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(10)
	PORT_MODIFY("AN2")      /* IN#4: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(20) PORT_KEYDELTA(10)
INPUT_PORTS_END

static INPUT_PORTS_START( aircomb )
	PORT_INCLUDE(s21default)

	PORT_MODIFY("AN0")      /* IN#2: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN1")      /* IN#3: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)
	PORT_MODIFY("AN2")      /* IN#4: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)
	PORT_MODIFY("AN3")      /* IN#5: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE
	PORT_MODIFY("AN4")      /* IN#6: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN5")      /* IN#7: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 5 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN6")      /* IN#8: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 6 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN7")      /* IN#9: 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 7 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")      /* 63B05Z0 - $2000 DIP SW */
	PORT_DIPNAME( 0x01, 0x01, "DSW1") // not test mode on this game
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("MCUH")        /* IN#10: 63B05Z0 - PORT H */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) ///???
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) // prev color
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) // ???next color
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void namcos21_c67_state::sound_bankselect_w(uint8_t data)
{
	m_audiobank->set_entry(data>>4);
}


void namcos21_c67_state::sound_reset_w(uint8_t data)
{
	if (data & 0x01)
	{
		/* Resume execution */
		m_audiocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_maincpu->yield();
	}
	else
	{
		/* Suspend execution */
		m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}

	if (data & 0x04)
	{
		m_namcos21_dsp_c67->reset_kickstart();
	}
}

void namcos21_c67_state::system_reset_w(uint8_t data)
{
	reset_all_subcpus(data & 1 ? CLEAR_LINE : ASSERT_LINE);

	if (data & 0x01)
		m_maincpu->yield();
}

void namcos21_c67_state::reset_all_subcpus(int state)
{
	m_slave->set_input_line(INPUT_LINE_RESET, state);
	m_c68->ext_reset(state);
	m_namcos21_dsp_c67->reset_dsps(state);
}

void namcos21_c67_state::eeprom_w(offs_t offset, uint8_t data)
{
	m_eeprom[offset] = data;
}

uint8_t namcos21_c67_state::eeprom_r(offs_t offset)
{
	return m_eeprom[offset];
}

void namcos21_c67_state::machine_reset()
{
	/* Initialise the bank select in the sound CPU */
	m_audiobank->set_entry(0); /* Page in bank 0 */

	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE );

	/* Place CPU2 & CPU3 into the reset condition */
	reset_all_subcpus(ASSERT_LINE);
}



void namcos21_c67_state::machine_start()
{
	m_eeprom = std::make_unique<uint8_t[]>(0x2000);
	subdevice<nvram_device>("nvram")->set_base(m_eeprom.get(), 0x2000);

	uint32_t max = memregion("audiocpu")->bytes() / 0x4000;
	for (int i = 0; i < 0x10; i++)
		m_audiobank->configure_entry(i, memregion("audiocpu")->base() + (i % max) * 0x4000);

	save_item(NAME(m_video_enable));
}

TIMER_DEVICE_CALLBACK_MEMBER(namcos21_c67_state::screen_scanline)
{
	int scanline = param;
//  int cur_posirq = get_posirq_scanline()*2;

	if(scanline == 240*2)
	{
		m_master_intc->vblank_irq_trigger();
		m_slave_intc->vblank_irq_trigger();
		m_c68->ext_interrupt(ASSERT_LINE);
	}
}

void namcos21_c67_state::configure_c148_standard(machine_config &config)
{
	NAMCO_C148(config, m_master_intc, 0, m_maincpu, true);
	m_master_intc->link_c148_device(m_slave_intc);
	m_master_intc->out_ext1_callback().set(FUNC(namcos21_c67_state::sound_reset_w));
	m_master_intc->out_ext2_callback().set(FUNC(namcos21_c67_state::system_reset_w));

	NAMCO_C148(config, m_slave_intc, 0, m_slave, false);
	m_slave_intc->link_c148_device(m_master_intc);
}

// starblad, solvalou, aircomb, cybsled base state
void namcos21_c67_state::namcos21(machine_config &config)
{
	M68000(config, m_maincpu, 49.152_MHz_XTAL / 4); /* Master */
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos21_c67_state::master_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(namcos21_c67_state::screen_scanline), "screen", 0, 1);

	M68000(config, m_slave, 49.152_MHz_XTAL / 4); /* Slave */
	m_slave->set_addrmap(AS_PROGRAM, &namcos21_c67_state::slave_map);

	MC6809E(config, m_audiocpu, 49.152_MHz_XTAL / 24); /* Sound */
	m_audiocpu->set_addrmap(AS_PROGRAM, &namcos21_c67_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(namcos21_c67_state::irq0_line_hold), attotime::from_hz(2*60));

	configure_c68_namcos21(config);

	NAMCOS21_DSP_C67(config, m_namcos21_dsp_c67, 0);
	m_namcos21_dsp_c67->set_renderer_tag("namcos21_3d");

	config.set_maximum_quantum(attotime::from_hz(12000));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: basic parameters to get 60.606060 Hz, x2 is for interlace
	m_screen->set_raw(49.152_MHz_XTAL / 4 * 2, 768, 0, 496, 264*2, 0, 480);
	m_screen->set_screen_update(FUNC(namcos21_c67_state::screen_update));
	m_screen->set_palette(m_palette);

	NAMCOS21_3D(config, m_namcos21_3d, 0);
	m_namcos21_3d->set_zz_shift_mult(11, 0x200);
	m_namcos21_3d->set_depth_reverse(false);
	m_namcos21_3d->set_framebuffer_size(496,480);

	configure_c148_standard(config);
	NAMCO_C139(config, m_sci, 0);

	PALETTE(config, m_palette).set_format(palette_device::xBRG_888, 0x10000/2);

	NAMCO_C355SPR(config, m_c355spr, 0);
	m_c355spr->set_screen(m_screen);
	m_c355spr->set_palette(m_palette);
	m_c355spr->set_scroll_offsets(0x26, 0x19);
	m_c355spr->set_tile_callback(namco_c355spr_device::c355_obj_code2tile_delegate());
	m_c355spr->set_palxor(0xf); // reverse mapping
	m_c355spr->set_color_base(0x1000);
	m_c355spr->set_external_prifill(true);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	C140(config, m_c140, 49.152_MHz_XTAL / 2304);
	m_c140->set_addrmap(0, &namcos21_c67_state::c140_map);
	m_c140->int1_callback().set_inputline(m_audiocpu, M6809_FIRQ_LINE);
	m_c140->add_route(0, "lspeaker", 0.50);
	m_c140->add_route(1, "rspeaker", 0.50);

	YM2151(config, "ymsnd", 3.579545_MHz_XTAL).add_route(0, "lspeaker", 0.30).add_route(1, "rspeaker", 0.30);
}

void namcos21_c67_state::aircomb(machine_config &config)
{
	namcos21(config);
	m_namcos21_dsp_c67->set_gametype(namcos21_dsp_c67_device::NAMCOS21_AIRCOMBAT);
}

void namcos21_c67_state::starblad(machine_config &config)
{
	namcos21(config);
	m_namcos21_dsp_c67->set_gametype(namcos21_dsp_c67_device::NAMCOS21_STARBLADE);
}

void namcos21_c67_state::cybsled(machine_config &config)
{
	namcos21(config);
	m_namcos21_dsp_c67->set_gametype(namcos21_dsp_c67_device::NAMCOS21_CYBERSLED);
}

void namcos21_c67_state::yield_hack(int state)
{
	m_maincpu->yield();
}

void namcos21_c67_state::solvalou(machine_config &config)
{
	namcos21(config);
	m_namcos21_dsp_c67->set_gametype(namcos21_dsp_c67_device::NAMCOS21_SOLVALOU);
	m_namcos21_dsp_c67->yield_hack_callback().set(FUNC(namcos21_c67_state::yield_hack)); // VCK function
}


ROM_START( starblad )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "st2_mpu.mpru", 0x000000, 0x80000, CRC(35bc9e4a) SHA1(03401fb846c1b2aee775071a554654e49fe5c47c) )
	ROM_LOAD16_BYTE( "st2_mpl.mprl", 0x000001, 0x80000, CRC(193e641b) SHA1(fed803167c5b0bba5b8381c26c909b7380d57efd) )

	ROM_REGION( 0x080000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "st1-sp-u.bin", 0x000000, 0x40000, CRC(9f9a55db) SHA1(72bf5d6908cc57cc490fa2292b4993d796b2974d) )
	ROM_LOAD16_BYTE( "st1-sp-l.bin", 0x000001, 0x40000, CRC(acbe39c7) SHA1(ca48b7ea619b1caaf590eed33001826ce7ef36d8) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "st1-snd0.bin", 0x000000, 0x020000, CRC(c0e934a3) SHA1(678ed6705c6f494d7ecb801a4ef1b123b80979a5) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x200000, "c355spr", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "st1-obj0.bin", 0x000000, 0x80000, CRC(5d42c71e) SHA1(f1aa2bb31bbbcdcac8e94334b1c78238cac1a0e7) )
	ROM_LOAD32_BYTE( "st1-obj1.bin", 0x000001, 0x80000, CRC(c98011ad) SHA1(bc34c21428e0ef5887051c0eb0fdef5397823a82) )
	ROM_LOAD32_BYTE( "st1-obj2.bin", 0x000002, 0x80000, CRC(6cf5b608) SHA1(c8537fbe97677c4c8a365b1cf86c4645db7a7d6b) )
	ROM_LOAD32_BYTE( "st1-obj3.bin", 0x000003, 0x80000, CRC(cdc195bb) SHA1(91443917a6982c286b6f15381d441d061aefb138) )

	ROM_REGION16_BE( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "st1-data-u.bin", 0x000000, 0x20000, CRC(2433e911) SHA1(95f5f00d3bacda4996e055a443311fb9f9a5fe2f) )
	ROM_LOAD16_BYTE( "st1-data-l.bin", 0x000001, 0x20000, CRC(4a2cc252) SHA1(d9da9992bac878f8a1f5e84cc3c6d457b4705e8f) )

	ROM_REGION16_BE( 0x100000, "edata", ROMREGION_ERASEFF )

	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67:point24", ROMREGION_ERASE00) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "st1-pt0-h.bin", 0x000001, 0x80000, CRC(84eb355f) SHA1(89a248b8be2e0afcee29ba4c4c9cca65d5fb246a) )
	ROM_LOAD32_BYTE( "st1-pt0-u.bin", 0x000002, 0x80000, CRC(1956cd0a) SHA1(7d21b3a59f742694de472c545a1f30c3d92e3390) )
	ROM_LOAD32_BYTE( "st1-pt0-l.bin", 0x000003, 0x80000, CRC(ff577049) SHA1(1e1595174094e88d5788753d05ce296c1f7eca75) )
	ROM_LOAD32_BYTE( "st1-pt1-h.bin", 0x200001, 0x80000, CRC(96b1bd7d) SHA1(55da7896dda2aa4c35501a55c8605a065b02aa17) )
	ROM_LOAD32_BYTE( "st1-pt1-u.bin", 0x200002, 0x80000, CRC(ecf21047) SHA1(ddb13f5a2e7d192f0662fa420b49f89e1e991e66) )
	ROM_LOAD32_BYTE( "st1-pt1-l.bin", 0x200003, 0x80000, CRC(01cb0407) SHA1(4b58860bbc353de8b4b8e83d12b919d9386846e8) )

	ROM_REGION16_BE( 0x400000, "c140", ROMREGION_ERASE00 ) /* sound samples */
	ROM_LOAD16_BYTE("st1-voi0.bin", 0x000000, 0x80000,CRC(5b3d43a9) SHA1(cdc04f19dc91dca9fa88ba0c2fca72aa195a3694) )
	ROM_LOAD16_BYTE("st1-voi1.bin", 0x100000, 0x80000,CRC(413e6181) SHA1(e827ec11f5755606affd2635718512aeac9354da) )
	ROM_LOAD16_BYTE("st1-voi2.bin", 0x200000, 0x80000,CRC(067d0720) SHA1(a853b2d43027a46c5e707fc677afdaae00f450c7) )
	ROM_LOAD16_BYTE("st1-voi3.bin", 0x300000, 0x80000,CRC(8b5aa45f) SHA1(e1214e639200758ad2045bde0368a2d500c1b84a) )

	ROM_REGION( 0x2000, "nvram", ROMREGION_ERASE00)
	// starblad needs default NVRAM to be all 0
ROM_END

ROM_START( starbladj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "st1_mpu.mpru", 0x000000, 0x80000, CRC(483a311c) SHA1(dd9416b8d4b0f8b361630e312eac71c113064eae) )
	ROM_LOAD16_BYTE( "st1_mpl.mprl", 0x000001, 0x80000, CRC(0a4dd661) SHA1(fc2b71a255a8613693c4d1c79ddd57a6d396165a) )

	ROM_REGION( 0x080000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "st1-sp-u.bin", 0x000000, 0x40000, CRC(9f9a55db) SHA1(72bf5d6908cc57cc490fa2292b4993d796b2974d) )
	ROM_LOAD16_BYTE( "st1-sp-l.bin", 0x000001, 0x40000, CRC(acbe39c7) SHA1(ca48b7ea619b1caaf590eed33001826ce7ef36d8) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "st1-snd0.bin", 0x000000, 0x020000, CRC(c0e934a3) SHA1(678ed6705c6f494d7ecb801a4ef1b123b80979a5) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x200000, "c355spr", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "st1-obj0.bin", 0x000000, 0x80000, CRC(5d42c71e) SHA1(f1aa2bb31bbbcdcac8e94334b1c78238cac1a0e7) )
	ROM_LOAD32_BYTE( "st1-obj1.bin", 0x000001, 0x80000, CRC(c98011ad) SHA1(bc34c21428e0ef5887051c0eb0fdef5397823a82) )
	ROM_LOAD32_BYTE( "st1-obj2.bin", 0x000002, 0x80000, CRC(6cf5b608) SHA1(c8537fbe97677c4c8a365b1cf86c4645db7a7d6b) )
	ROM_LOAD32_BYTE( "st1-obj3.bin", 0x000003, 0x80000, CRC(cdc195bb) SHA1(91443917a6982c286b6f15381d441d061aefb138) )

	ROM_REGION16_BE( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "st1-data-u.bin", 0x000000, 0x20000, CRC(2433e911) SHA1(95f5f00d3bacda4996e055a443311fb9f9a5fe2f) )
	ROM_LOAD16_BYTE( "st1-data-l.bin", 0x000001, 0x20000, CRC(4a2cc252) SHA1(d9da9992bac878f8a1f5e84cc3c6d457b4705e8f) )

	ROM_REGION16_BE( 0x100000, "edata", ROMREGION_ERASEFF )

	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67:point24", ROMREGION_ERASE00) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "st1-pt0-h.bin", 0x000001, 0x80000, CRC(84eb355f) SHA1(89a248b8be2e0afcee29ba4c4c9cca65d5fb246a) )
	ROM_LOAD32_BYTE( "st1-pt0-u.bin", 0x000002, 0x80000, CRC(1956cd0a) SHA1(7d21b3a59f742694de472c545a1f30c3d92e3390) )
	ROM_LOAD32_BYTE( "st1-pt0-l.bin", 0x000003, 0x80000, CRC(ff577049) SHA1(1e1595174094e88d5788753d05ce296c1f7eca75) )
	ROM_LOAD32_BYTE( "st1-pt1-h.bin", 0x200001, 0x80000, CRC(96b1bd7d) SHA1(55da7896dda2aa4c35501a55c8605a065b02aa17) )
	ROM_LOAD32_BYTE( "st1-pt1-u.bin", 0x200002, 0x80000, CRC(ecf21047) SHA1(ddb13f5a2e7d192f0662fa420b49f89e1e991e66) )
	ROM_LOAD32_BYTE( "st1-pt1-l.bin", 0x200003, 0x80000, CRC(01cb0407) SHA1(4b58860bbc353de8b4b8e83d12b919d9386846e8) )

	ROM_REGION16_BE( 0x400000, "c140", ROMREGION_ERASE00 ) /* sound samples */
	ROM_LOAD16_BYTE("st1-voi0.bin", 0x000000, 0x80000,CRC(5b3d43a9) SHA1(cdc04f19dc91dca9fa88ba0c2fca72aa195a3694) )
	ROM_LOAD16_BYTE("st1-voi1.bin", 0x100000, 0x80000,CRC(413e6181) SHA1(e827ec11f5755606affd2635718512aeac9354da) )
	ROM_LOAD16_BYTE("st1-voi2.bin", 0x200000, 0x80000,CRC(067d0720) SHA1(a853b2d43027a46c5e707fc677afdaae00f450c7) )
	ROM_LOAD16_BYTE("st1-voi3.bin", 0x300000, 0x80000,CRC(8b5aa45f) SHA1(e1214e639200758ad2045bde0368a2d500c1b84a) )

	ROM_REGION( 0x2000, "nvram", ROMREGION_ERASE00)
	// starblad needs default NVRAM to be all 0
ROM_END

ROM_START( solvalou )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "sv1-mp-u.bin", 0x000000, 0x20000, CRC(b6f92762) SHA1(d177328b3da2ab0580e101478142bc8c373d6140) )
	ROM_LOAD16_BYTE( "sv1-mp-l.bin", 0x000001, 0x20000, CRC(28c54c42) SHA1(32fcca2eb4bb8ba8c2587b03d3cf59f072f7fac5) )

	ROM_REGION( 0x80000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "sv1-sp-u.bin", 0x000000, 0x20000, CRC(ebd4bf82) SHA1(67946360d680a675abcb3c131bac0502b2455573) )
	ROM_LOAD16_BYTE( "sv1-sp-l.bin", 0x000001, 0x20000, CRC(7acab679) SHA1(764297c9601be99dbbffb75bbc6fe4a40ea38529) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "sv1-snd0.bin", 0x000000, 0x020000, CRC(5e007864) SHA1(94da2d51544c6127056beaa251353038646da15f) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 )
	ROM_LOAD32_BYTE( "sv1-obj0.bin", 0x000000, 0x80000, CRC(773798bb) SHA1(51ab76c95030bab834f1a74ae677b2f0afc18c52) )
	ROM_LOAD32_BYTE( "sv1-obj1.bin", 0x000001, 0x80000, CRC(a36d9e79) SHA1(928d9995e97ee7509e23e6cc64f5e7bfb5c02d42) )
	ROM_LOAD32_BYTE( "sv1-obj2.bin", 0x000002, 0x80000, CRC(c8672b8a) SHA1(8da037b27d2c2b178aab202781f162371458f788) )
	ROM_LOAD32_BYTE( "sv1-obj3.bin", 0x000003, 0x80000, CRC(293ef1c5) SHA1(f677883bfec16bbaeb0a01ac565d0e6cac679174) )
	ROM_LOAD32_BYTE( "sv1-obj4.bin", 0x200000, 0x80000, CRC(33a008a7) SHA1(4959a0ac24ad64f1367e2d8d63d39a0273c60f3e) )
	ROM_LOAD32_BYTE( "sv1-obj5.bin", 0x200001, 0x80000, CRC(31551245) SHA1(385452ea4830c466263ad5241313ac850dfef756) )
	ROM_LOAD32_BYTE( "sv1-obj6.bin", 0x200002, 0x80000, CRC(fe319530) SHA1(8f7e46c8f0b86c7515f6d763b795ce07d11c77bc) )
	ROM_LOAD32_BYTE( "sv1-obj7.bin", 0x200003, 0x80000, CRC(95ed6dcb) SHA1(931706ce3fea630823ce0c79febec5eec0cc623d) )

	ROM_REGION16_BE( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "sv1-data-u.bin", 0x000000, 0x80000, CRC(2e561996) SHA1(982158481e5649f21d5c2816fdc80cb725ed1419) )
	ROM_LOAD16_BYTE( "sv1-data-l.bin", 0x000001, 0x80000, CRC(495fb8dd) SHA1(813d1da4109652008d72b3bdb03032efc5c0c2d5) )

	ROM_REGION16_BE( 0x100000, "edata", ROMREGION_ERASEFF )

	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67:point24", ROMREGION_ERASE00)       /* 24bit signed point data */
	ROM_LOAD32_BYTE( "sv1-pt0-h.bin", 0x000001, 0x80000, CRC(3be21115) SHA1(c9f30353c1216f64199f87cd34e787efd728e739) ) /* most significant */
	ROM_LOAD32_BYTE( "sv1-pt0-u.bin", 0x000002, 0x80000, CRC(4aacfc42) SHA1(f0e179e057183b41744ca429764f44306f0ce9bf) )
	ROM_LOAD32_BYTE( "sv1-pt0-l.bin", 0x000003, 0x80000, CRC(6a4dddff) SHA1(9ed182d21d328c6a684ee6658a9dfcf3f3dd8646) ) /* least significant */

	ROM_REGION16_BE( 0x400000, "c140", ROMREGION_ERASE00 ) /* sound samples */
	ROM_LOAD16_BYTE("sv1-voi0.bin", 0x000000, 0x80000,CRC(7f61bbcf) SHA1(b3b7e66e24d9cb16ebd139237c1e51f5d60c1585) )
	ROM_LOAD16_BYTE("sv1-voi1.bin", 0x100000, 0x80000,CRC(c732e66c) SHA1(14e75dd9bea4055f85eb2bcbf69cf6695a3f7ec4) )
	ROM_LOAD16_BYTE("sv1-voi2.bin", 0x200000, 0x80000,CRC(51076298) SHA1(ec52c9ae3029118f3ea3732948d6de28f5fba561) )
	ROM_LOAD16_BYTE("sv1-voi3.bin", 0x300000, 0x80000,CRC(33085ff3) SHA1(0a30b91618c250a5e7bd896a8ceeb3d16da178a9) )
ROM_END


ROM_START( aircomb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "ac2-mpr-u.3j", 0x000000, 0x80000, CRC(a7133f85) SHA1(9f1c99dd503f1fc81096170fd272e33ae8a7de2f) )
	ROM_LOAD16_BYTE( "ac2-mpr-l.1j", 0x000001, 0x80000, CRC(520a52e6) SHA1(74306e02abfe08aa1afbf325b74dbc0840c3ad3a) )

	ROM_REGION( 0x80000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "ac2-spr-u.6c", 0x000000, 0x20000, CRC(42aca956) SHA1(10ea2400bb4d5b2d805e2de43ca0e0f54597f660) )
	ROM_LOAD16_BYTE( "ac2-spr-l.4c", 0x000001, 0x20000, CRC(3e15fa19) SHA1(65dbb33ab6b3c06c793613348ebb7b110b8bba0d) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "ac1-snd0.8j", 0x000000, 0x020000, CRC(5c1fb84b) SHA1(20e4d81289dbe58ffcfc947251a6ff1cc1e36436) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 )
	ROM_LOAD32_BYTE( "ac2-obj0.5s", 0x000000, 0x80000, CRC(8327ff22) SHA1(16f6022dedb7a74590898bc8ed3e8a97993c4635) )
	ROM_LOAD32_BYTE( "ac2-obj1.5x", 0x000001, 0x80000, CRC(43af566d) SHA1(99f0d9f005e28040f5cc10de2198893946a31d09) )
	ROM_LOAD32_BYTE( "ac2-obj2.3s", 0x000002, 0x80000, CRC(dafbf489) SHA1(c53ccb3e1b4a6a660bd28c8abe52ccc3f85d111f) )
	ROM_LOAD32_BYTE( "ac2-obj3.3x", 0x000003, 0x80000, CRC(bd555a1d) SHA1(96e432b30da6f5f7ccb768c516b1f7186bc0d4c9) )
	ROM_LOAD32_BYTE( "ac2-obj4.4s", 0x200000, 0x80000, CRC(e433e344) SHA1(98ade550cf066fcb5c09fa905f441a1464d4d625) )
	ROM_LOAD32_BYTE( "ac2-obj5.4x", 0x200001, 0x80000, CRC(ecb19199) SHA1(8e0aa1bc1141c4b09576ab08970d0c7629560643) )
	ROM_LOAD32_BYTE( "ac2-obj6.2s", 0x200002, 0x80000, CRC(24cc3f36) SHA1(e50af176eb3034c9cab7613ca614f5cc2c62f95e) )
	ROM_LOAD32_BYTE( "ac2-obj7.2x", 0x200003, 0x80000, CRC(d561fbe3) SHA1(a23976e10bddf74d4a6b292f044dfd0affbab101) )

	ROM_REGION16_BE( 0x100000, "data", 0 ) /* collision */
	ROM_LOAD16_BYTE( "ac1-data-u.3a",   0x000000, 0x80000, CRC(82320c71) SHA1(2be98d46853febb46e1cc728af2735c0e00ce303) )
	ROM_LOAD16_BYTE( "ac1-data-l.1a",   0x000001, 0x80000, CRC(fd7947d3) SHA1(2696eeae37de6d256e626cc3f3cea7b0f6eff60e) )

	ROM_REGION16_BE( 0x100000, "edata", 0 )
	ROM_LOAD16_BYTE( "ac1-edata1-u.3c", 0x000000, 0x80000, CRC(a9547509) SHA1(1bc663cec03b60ad968896bbc2546f02efda135e) )
	ROM_LOAD16_BYTE( "ac1-edata1-l.1c", 0x000001, 0x80000, CRC(a87087dd) SHA1(cd9b83a8f07886ab44e4ded68002b44338777e8c) )

	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67:point24", ROMREGION_ERASE00)       /* 24bit signed point data */
	ROM_LOAD32_BYTE( "ac1-poi-h.2f",  0x000001, 0x80000, CRC(573bbc3b) SHA1(371be12b915db6872049f18980c1b55544cfc445) ) /* most significant */
	ROM_LOAD32_BYTE( "ac1-poi-lu.2k", 0x000002, 0x80000, CRC(d99084b9) SHA1(c604d60a2162af7610e5ff7c1aa4195f7df82efe) )
	ROM_LOAD32_BYTE( "ac1-poi-ll.2n", 0x000003, 0x80000, CRC(abb32307) SHA1(8e936ba99479215dd33a951d81ec2b04020dfd62) ) /* least significant */

	ROM_REGION16_BE( 0x400000, "c140", ROMREGION_ERASE00 ) /* sound samples */
	ROM_LOAD16_BYTE("ac1-voi0.12b", 0x000000, 0x80000,CRC(f427b119) SHA1(bd45bbe41c8be26d6c997fcdc226d080b416a2cf) )
	ROM_LOAD16_BYTE("ac1-voi1.12c", 0x100000, 0x80000,CRC(c9490667) SHA1(4b6fbe635c32469870a8e6f82742be6a9d4918c9) )
	ROM_LOAD16_BYTE("ac1-voi2.12d", 0x200000, 0x80000,CRC(1fcb51ba) SHA1(80fc815e5fad76d20c3795ab1d89b57d9abc3efd) )
	ROM_LOAD16_BYTE("ac1-voi3.12e", 0x300000, 0x80000,CRC(cd202e06) SHA1(72a18f5ba402caefef14b8d1304f337eaaa3eb1d) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "gal16v8a-3pdsp5.17d", 0x0000, 0x0117, CRC(799c1f26) SHA1(d28ed1b9fa78180c5a0b01a7198a2870137c7349) )
	ROM_LOAD( "plhs18p8-3pobj3.17n", 0x0200, 0x0149, CRC(9625f469) SHA1(29158a3d37485fb0714d0a60bcd07abd26a3f56e) )
	ROM_LOAD( "plhs18p8-3pobj4.17n", 0x0400, 0x0149, CRC(1b7c90c1) SHA1(ae65aab7a191cdf1af488e144af22b9d8669c903) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "aircomb.nv", 0x0000, 0x2000, CRC(a97ea3e0) SHA1(95684bb7369c1cb1e2fa53c743d4f94b0080c6f5) )
ROM_END

ROM_START( aircombj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "ac1-mpr-u.3j", 0x000000, 0x80000, CRC(a4dec813) SHA1(2ee8b3492d30db4c841f695151880925a5e205e0) )
	ROM_LOAD16_BYTE( "ac1-mpr-l.1j", 0x000001, 0x80000, CRC(8577b6a2) SHA1(32194e392fbd051754be88eb8c90688c65c65d85) )

	ROM_REGION( 0x080000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "ac1-spr-u.6c", 0x000000, 0x20000, CRC(5810e219) SHA1(c312ffd8324670897871b12d521779570dc0f580) )
	ROM_LOAD16_BYTE( "ac1-spr-l.4c", 0x000001, 0x20000, CRC(175a7d6c) SHA1(9e31dde6646cd9b6dcdbdb3f2326177508559e56) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "ac1-snd0.8j", 0x000000, 0x020000, CRC(5c1fb84b) SHA1(20e4d81289dbe58ffcfc947251a6ff1cc1e36436) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 )
	ROM_LOAD32_BYTE( "ac1-obj0.5s", 0x000000, 0x80000, CRC(d2310c6a) SHA1(9bb8fdfc2c232574777248f4959975f9a20e3105) )
	ROM_LOAD32_BYTE( "ac1-obj1.5x", 0x000001, 0x80000, CRC(f5783a77) SHA1(0be1815ceb4ce4fa7ab75ba588e090f20ee0cac9) )
	ROM_LOAD32_BYTE( "ac1-obj2.3s", 0x000002, 0x80000, CRC(01343d5c) SHA1(64171fed1d1f8682b3d70d3233ea017719f4cc63) )
	ROM_LOAD32_BYTE( "ac1-obj3.3x", 0x000003, 0x80000, CRC(7717f52e) SHA1(be1df3f4d0fdcaa5d3c81a724e5eb9d14136c6f5) )
	ROM_LOAD32_BYTE( "ac1-obj4.4s", 0x200000, 0x80000, CRC(0c93b478) SHA1(a92ffbcf04b64e0eee5bcf37008e247700641b25) )
	ROM_LOAD32_BYTE( "ac1-obj5.4x", 0x200001, 0x80000, CRC(476aed15) SHA1(0e53fdf02e8ffe7852a1fa8bd2f64d0e58f3dc09) )
	ROM_LOAD32_BYTE( "ac1-obj6.2s", 0x200002, 0x80000, CRC(c67607b1) SHA1(df64ea7920cf64271fe742d3d0a57f842ee61e8d) )
	ROM_LOAD32_BYTE( "ac1-obj7.2x", 0x200003, 0x80000, CRC(cfa9fe5f) SHA1(0da25663b89d653c87ed32d15f7c82f3035702ab) )

	ROM_REGION16_BE( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "ac1-data-u.3a",   0x000000, 0x80000, CRC(82320c71) SHA1(2be98d46853febb46e1cc728af2735c0e00ce303) )
	ROM_LOAD16_BYTE( "ac1-data-l.1a",   0x000001, 0x80000, CRC(fd7947d3) SHA1(2696eeae37de6d256e626cc3f3cea7b0f6eff60e) )

	ROM_REGION16_BE( 0x100000, "edata", 0 )
	ROM_LOAD16_BYTE( "ac1-edata1-u.3c", 0x000000, 0x80000, CRC(a9547509) SHA1(1bc663cec03b60ad968896bbc2546f02efda135e) )
	ROM_LOAD16_BYTE( "ac1-edata1-l.1c", 0x000001, 0x80000, CRC(a87087dd) SHA1(cd9b83a8f07886ab44e4ded68002b44338777e8c) )

	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67:point24", ROMREGION_ERASE00)       /* 24bit signed point data */
	ROM_LOAD32_BYTE( "ac1-poi-h.2f",  0x000001, 0x80000, CRC(573bbc3b) SHA1(371be12b915db6872049f18980c1b55544cfc445) ) /* most significant */
	ROM_LOAD32_BYTE( "ac1-poi-lu.2k", 0x000002, 0x80000, CRC(d99084b9) SHA1(c604d60a2162af7610e5ff7c1aa4195f7df82efe) )
	ROM_LOAD32_BYTE( "ac1-poi-ll.2n", 0x000003, 0x80000, CRC(abb32307) SHA1(8e936ba99479215dd33a951d81ec2b04020dfd62) ) /* least significant */

	ROM_REGION16_BE( 0x400000, "c140", ROMREGION_ERASE00 ) /* sound samples */
	ROM_LOAD16_BYTE("ac1-voi0.12b", 0x000000, 0x80000,CRC(f427b119) SHA1(bd45bbe41c8be26d6c997fcdc226d080b416a2cf) )
	ROM_LOAD16_BYTE("ac1-voi1.12c", 0x100000, 0x80000,CRC(c9490667) SHA1(4b6fbe635c32469870a8e6f82742be6a9d4918c9) )
	ROM_LOAD16_BYTE("ac1-voi2.12d", 0x200000, 0x80000,CRC(1fcb51ba) SHA1(80fc815e5fad76d20c3795ab1d89b57d9abc3efd) )
	ROM_LOAD16_BYTE("ac1-voi3.12e", 0x300000, 0x80000,CRC(cd202e06) SHA1(72a18f5ba402caefef14b8d1304f337eaaa3eb1d) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "gal16v8a-3pdsp5.17d", 0x0000, 0x0117, CRC(799c1f26) SHA1(d28ed1b9fa78180c5a0b01a7198a2870137c7349) )
	ROM_LOAD( "plhs18p8-3pobj3.17n", 0x0200, 0x0149, CRC(9625f469) SHA1(29158a3d37485fb0714d0a60bcd07abd26a3f56e) )
	ROM_LOAD( "plhs18p8-3pobj4.17n", 0x0400, 0x0149, CRC(1b7c90c1) SHA1(ae65aab7a191cdf1af488e144af22b9d8669c903) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "aircombj.nv", 0x0000, 0x2000, CRC(56c71c83) SHA1(83dfcf4e3232f78e3807e9d3e862aa5446444165) )
ROM_END

ROM_START( cybsled )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "cy2-mpr-u.3j", 0x000000, 0x80000, CRC(b35a72bc) SHA1(d9bc5b8f0bc30510fca8fc57eeb67e5ca0e4c67f) )
	ROM_LOAD16_BYTE( "cy2-mpr-l.1j", 0x000001, 0x80000, CRC(c4a25919) SHA1(52f6947102001376e37730ace16283141b13fee7) )

	ROM_REGION( 0x100000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "cy2-spr-u.6c", 0x000000, 0x80000, CRC(575a422d) SHA1(cad97742da1e2baf47ac110fadef5544b3a30cc7) )
	ROM_LOAD16_BYTE( "cy2-spr-l.4c", 0x000001, 0x80000, CRC(4066291a) SHA1(6ebbc11a68f66ec1e6d2e6ee857e8c599691e289) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "cy1-snd0.8j", 0x000000, 0x020000, CRC(3dddf83b) SHA1(e16119cbef176b6f8f8ace773fcbc201e987823f) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 )
	ROM_LOAD32_BYTE( "cy1-obj0.5s", 0x000000, 0x80000, CRC(5ae542d5) SHA1(99b1a3ed476da4a97cb864538909d7b831f0fd3b) )
	ROM_LOAD32_BYTE( "cy1-obj1.5x", 0x000001, 0x80000, CRC(4aae3eff) SHA1(c80240bd2f4228a0261a14adb6b10560b31b5aa0) )
	ROM_LOAD32_BYTE( "cy1-obj2.3s", 0x000002, 0x80000, CRC(d64ec4c3) SHA1(0bed1cafc21ed8cef3850fb81e30076977086eb0) )
	ROM_LOAD32_BYTE( "cy1-obj3.3x", 0x000003, 0x80000, CRC(3d1f7168) SHA1(392dddcc79fe61dcc6514a91ac27b5e36825d8b7) )
	ROM_LOAD32_BYTE( "cy1-obj4.4s", 0x200000, 0x80000, CRC(57904076) SHA1(b1dc0d99543bc4b9584b37ffc12c6ebc59e30e3b) )
	ROM_LOAD32_BYTE( "cy1-obj5.4x", 0x200001, 0x80000, CRC(0e11ca47) SHA1(076a9a4cfddbee2d8aaa06110333090d8fdbefeb) )
	ROM_LOAD32_BYTE( "cy1-obj6.2s", 0x200002, 0x80000, CRC(7748b485) SHA1(adb4da419a6cdbefd0fef182d866a3479be379af) )
	ROM_LOAD32_BYTE( "cy1-obj7.2x", 0x200003, 0x80000, CRC(b6eb6ad2) SHA1(85a660c5e44012491be7d4e783cce6ba12c135cb) )

	ROM_REGION16_BE( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "cy1-data-u.3a",   0x000000, 0x80000, CRC(570da15d) SHA1(9ebe756f10756c079a92fb522332e9e52ff715c3) )
	ROM_LOAD16_BYTE( "cy1-data-l.1a",   0x000001, 0x80000, CRC(9cf96f9e) SHA1(91783f48b93e03c778c6641ca8fb419c13b0d3c5) )

	ROM_REGION16_BE( 0x100000, "edata", 0 )
	ROM_LOAD16_BYTE( "cy1-edata0-u.3b", 0x000000, 0x80000, CRC(77452533) SHA1(48fc199bcc1beb23c714eebd9b09b153c980170b) )
	ROM_LOAD16_BYTE( "cy1-edata0-l.1b", 0x000001, 0x80000, CRC(e812e290) SHA1(719e0a026ae8ef63d0d0269b67669ea9b4d950dd) )

	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67:point24", ROMREGION_ERASE00)       /* 24bit signed point data */
	ROM_LOAD32_BYTE( "cy1-poi-h1.2f",  0x000001, 0x80000, CRC(eaf8bac3) SHA1(7a2caf6672af158b4a23ce4626342d1f17d1a4e4) )    /* most significant */
	ROM_LOAD32_BYTE( "cy1-poi-lu1.2k", 0x000002, 0x80000, CRC(c544a8dc) SHA1(4cce5f2ab3519b4aa7edbdd15b2d79a7fdcade3c) )
	ROM_LOAD32_BYTE( "cy1-poi-ll1.2n", 0x000003, 0x80000, CRC(30acb99b) SHA1(a28dcb3e5405f166644f6353a903c1143ee268f1) )    /* least significant */
	ROM_LOAD32_BYTE( "cy1-poi-h2.2j",  0x200001, 0x80000, CRC(4079f342) SHA1(fa36aed1abbda54a42f29b183007474580870319) )
	ROM_LOAD32_BYTE( "cy1-poi-lu2.2l", 0x200002, 0x80000, CRC(61d816d4) SHA1(7991957b910d32530151abc7f469fcf1de62d8f3) )
	ROM_LOAD32_BYTE( "cy1-poi-ll2.2p", 0x200003, 0x80000, CRC(faf09158) SHA1(b56ebed6012362b1d599c396a43e90a1e4d9dc38) )

	ROM_REGION16_BE( 0x400000, "c140", ROMREGION_ERASE00 ) /* sound samples */
	ROM_LOAD16_BYTE("cy1-voi0.12b", 0x000000, 0x80000,CRC(99d7ce46) SHA1(b75f4055c3ce847daabfacda22df14e3f80c4fb9) )
	ROM_LOAD16_BYTE("cy1-voi1.12c", 0x100000, 0x80000,CRC(2b335f06) SHA1(2b2cd407c34388b56496f84a414daa153780b098) )
	ROM_LOAD16_BYTE("cy1-voi2.12d", 0x200000, 0x80000,CRC(10cd15f0) SHA1(9b721654ed97a13287373c1b2854ac9aeddc271f) )
	ROM_LOAD16_BYTE("cy1-voi3.12e", 0x300000, 0x80000,CRC(c902b4a4) SHA1(816357ec1a02a7ebf817ac1182e9c50ce5ca71f6) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "cybsled.nv", 0x0000, 0x2000, CRC(aa18bf9e) SHA1(3712d4d20e5f5f1c920e3f1f6a00101e874662d0) )
ROM_END

ROM_START( cybsleda )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Master */
	ROM_LOAD16_BYTE( "cy1-mpr-u.3j", 0x000000, 0x80000, CRC(cc5a2e83) SHA1(b794051b2c351e9ca43351603845e4e563f6740f) )
	ROM_LOAD16_BYTE( "cy1-mpr-l.1j", 0x000001, 0x80000, CRC(f7ee8b48) SHA1(6d36eb3dba9cf7f5f5e1a26c156e77a2dad3f257) )

	ROM_REGION( 0x100000, "slave", 0 ) /* Slave */
	ROM_LOAD16_BYTE( "cy1-spr-u.6c", 0x000000, 0x80000, CRC(28dd707b) SHA1(11297ceae4fe78d170785a5cf9ad77833bbe7fff) )
	ROM_LOAD16_BYTE( "cy1-spr-l.4c", 0x000001, 0x80000, CRC(437029de) SHA1(3d275a2b0ce6909e77e657c371bd22597ea9d398) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "cy1-snd0.8j", 0x000000, 0x020000, CRC(3dddf83b) SHA1(e16119cbef176b6f8f8ace773fcbc201e987823f) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 )
	ROM_LOAD32_BYTE( "cy1-obj0.5s", 0x000000, 0x80000, CRC(5ae542d5) SHA1(99b1a3ed476da4a97cb864538909d7b831f0fd3b) )
	ROM_LOAD32_BYTE( "cy1-obj1.5x", 0x000001, 0x80000, CRC(4aae3eff) SHA1(c80240bd2f4228a0261a14adb6b10560b31b5aa0) )
	ROM_LOAD32_BYTE( "cy1-obj2.3s", 0x000002, 0x80000, CRC(d64ec4c3) SHA1(0bed1cafc21ed8cef3850fb81e30076977086eb0) )
	ROM_LOAD32_BYTE( "cy1-obj3.3x", 0x000003, 0x80000, CRC(3d1f7168) SHA1(392dddcc79fe61dcc6514a91ac27b5e36825d8b7) )
	ROM_LOAD32_BYTE( "cy1-obj4.4s", 0x200000, 0x80000, CRC(57904076) SHA1(b1dc0d99543bc4b9584b37ffc12c6ebc59e30e3b) )
	ROM_LOAD32_BYTE( "cy1-obj5.4x", 0x200001, 0x80000, CRC(0e11ca47) SHA1(076a9a4cfddbee2d8aaa06110333090d8fdbefeb) )
	ROM_LOAD32_BYTE( "cy1-obj6.2s", 0x200002, 0x80000, CRC(7748b485) SHA1(adb4da419a6cdbefd0fef182d866a3479be379af) )
	ROM_LOAD32_BYTE( "cy1-obj7.2x", 0x200003, 0x80000, CRC(b6eb6ad2) SHA1(85a660c5e44012491be7d4e783cce6ba12c135cb) )

	ROM_REGION16_BE( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "cy1-data-u.3a",   0x000000, 0x80000, CRC(570da15d) SHA1(9ebe756f10756c079a92fb522332e9e52ff715c3) )
	ROM_LOAD16_BYTE( "cy1-data-l.1a",   0x000001, 0x80000, CRC(9cf96f9e) SHA1(91783f48b93e03c778c6641ca8fb419c13b0d3c5) )

	ROM_REGION16_BE( 0x100000, "edata", 0 )
	ROM_LOAD16_BYTE( "cy1-edata0-u.3b", 0x000000, 0x80000, CRC(77452533) SHA1(48fc199bcc1beb23c714eebd9b09b153c980170b) )
	ROM_LOAD16_BYTE( "cy1-edata0-l.1b", 0x000001, 0x80000, CRC(e812e290) SHA1(719e0a026ae8ef63d0d0269b67669ea9b4d950dd) )

	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67:point24", ROMREGION_ERASE00)       /* 24bit signed point data */
	ROM_LOAD32_BYTE( "cy1-poi-h1.2f",  0x000001, 0x80000, CRC(eaf8bac3) SHA1(7a2caf6672af158b4a23ce4626342d1f17d1a4e4) )    /* most significant */
	ROM_LOAD32_BYTE( "cy1-poi-lu1.2k", 0x000002, 0x80000, CRC(c544a8dc) SHA1(4cce5f2ab3519b4aa7edbdd15b2d79a7fdcade3c) )
	ROM_LOAD32_BYTE( "cy1-poi-ll1.2n", 0x000003, 0x80000, CRC(30acb99b) SHA1(a28dcb3e5405f166644f6353a903c1143ee268f1) )    /* least significant */
	ROM_LOAD32_BYTE( "cy1-poi-h2.2j",  0x200001, 0x80000, CRC(4079f342) SHA1(fa36aed1abbda54a42f29b183007474580870319) )
	ROM_LOAD32_BYTE( "cy1-poi-lu2.2l", 0x200002, 0x80000, CRC(61d816d4) SHA1(7991957b910d32530151abc7f469fcf1de62d8f3) )
	ROM_LOAD32_BYTE( "cy1-poi-ll2.2p", 0x200003, 0x80000, CRC(faf09158) SHA1(b56ebed6012362b1d599c396a43e90a1e4d9dc38) )

	ROM_REGION16_BE( 0x400000, "c140", ROMREGION_ERASE00 ) /* sound samples */
	ROM_LOAD16_BYTE("cy1-voi0.12b", 0x000000, 0x80000,CRC(99d7ce46) SHA1(b75f4055c3ce847daabfacda22df14e3f80c4fb9) )
	ROM_LOAD16_BYTE("cy1-voi1.12c", 0x100000, 0x80000,CRC(2b335f06) SHA1(2b2cd407c34388b56496f84a414daa153780b098) )
	ROM_LOAD16_BYTE("cy1-voi2.12d", 0x200000, 0x80000,CRC(10cd15f0) SHA1(9b721654ed97a13287373c1b2854ac9aeddc271f) )
	ROM_LOAD16_BYTE("cy1-voi3.12e", 0x300000, 0x80000,CRC(c902b4a4) SHA1(816357ec1a02a7ebf817ac1182e9c50ce5ca71f6) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "cybsleda.nv", 0x0000, 0x2000, CRC(a73bb03e) SHA1(e074bfeae14178c867070e06f6690ed13115f5fa) )
ROM_END

void namcos21_c67_state::init_solvalou()
{
	uint16_t *mem = (uint16_t *)memregion("maincpu")->base();
	mem[0x20ce4/2+1] = 0x0000; // $200128
	mem[0x20cf4/2+0] = 0x4e71; // 2nd ptr_booting
	mem[0x20cf4/2+1] = 0x4e71;
	mem[0x20cf4/2+2] = 0x4e71;
}

} // anonymous namespace


/*    YEAR  NAME       PARENT    MACHINE   INPUT       CLASS           INIT           MONITOR  COMPANY  FULLNAME                                 FLAGS */

// uses 5x TMS320C25 (C67, has internal ROM - dumped)
GAME( 1991, starblad,  0,        starblad, starblad,   namcos21_c67_state, empty_init,    ROT0,    "Namco", "Starblade (ST2, World)",                     MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, starbladj, starblad, starblad, starblad,   namcos21_c67_state, empty_init,    ROT0,    "Namco", "Starblade (ST1, Japan)",                     MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, solvalou,  0,        solvalou, s21default, namcos21_c67_state, init_solvalou, ROT0,    "Namco", "Solvalou (SV1, Japan)",                      MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1992, aircomb,   0,        aircomb,  aircomb,    namcos21_c67_state, empty_init,    ROT0,    "Namco", "Air Combat (AC2, US)",                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // There's code for a SCI, is it even possible to play multiplayer?
GAME( 1992, aircombj,  aircomb,  aircomb,  aircomb,    namcos21_c67_state, empty_init,    ROT0,    "Namco", "Air Combat (AC1, Japan)",                    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, cybsled,   0,        cybsled,  cybsled,    namcos21_c67_state, empty_init,    ROT0,    "Namco", "Cyber Sled (CY2, World)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_NOT_WORKING )
GAME( 1993, cybsleda,  cybsled,  cybsled,  cybsled,    namcos21_c67_state, empty_init,    ROT0,    "Namco", "Cyber Sled (CY1, World?)",                   MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_NOT_WORKING ) // usually an 'xx1' set would be Japan, but this shows neither a warning nor Japanese text, verify on hardware
