// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/**
Namco System 21

                          | Winning Run(1)|Driver's Eyes(2) | Cyber Sled, Star Blade etc.(3)
--------------------------+---------------+-----------------+--------------------------
GPU+bitmap layer          | yes           | no              | no
Namco System NB1 Sprites  | no            | yes             | yes
Number of DSPs            | 1x TMS320C25  | 1x TMS320C25    | 1x Master C67 (TMS320C25)
                          |               |                 | 4x Slave C67

(1) namcos21.cpp (this driver)
(2) namcos21_de.cpp
(3) namcos21_c67.cpp

Galaxian 3 (gal3.cpp) uses the same DSP PCB as Cyber Sled, Star Blade etc. (2x, 1 for each screen)


The main 68k CPUs populate a chunk of shared RAM with an display list describing a scene to be rendered.
The main CPUs also specify attributes for a master camera which provides additional global transformations.
The display list contains references to specific 3d objects and their position/orientation in 3d space.

The master DSP parses the display list and applies high level geometry, emitting matrices and object
references.  Object references are expanded into meshes (decoded from point ROMs) and passed to the slave
DSP, interleaved with local transforms.

A collection of slave DSPs transforms, projects, and clips each primitive's vertices.  Each slave DSP outputs
a stream of quad descriptors.

Each quad has a reference color (shared across vertices), and for each vertex the tuple: (screenx,screeny,z-code).
The z-code scalar accounts for depth bias.  A zbuffer is used while rendering quads, and depth cueing is used to
shade pixels according to their depth.

-------------------



Winning Run
    polygon glitches/flicker
    posirq effects for bitmap layer not working
    priority, mixing incorrection (specifically title screen and background color)


    NOTES:

    Winning Run
    Winning Run 91
        working
          - some minor polygon glitches
          - posirq handling broken
          - priority, mixing incorrection

    reference videos
    - https://youtu.be/ZNNveBLWevg
    - https://youtu.be/KazxHW9wQ60

*****************************

Winning Run / Winning Run Suzuka GP/ Winning Run 91
Namco 1988-91

These games run on Namco System 21 hardware. Note each set of PCBs for System 21 games are slightly different,
with some common PCBs and some unique-to-that-game PCBs. This covers the Winning Run series.
The PCBs are housed in a metal box. The front has a small filter board containing a 60-pin flat cable connector.
Underneath is a connector that plugs into PCB #3.
Inside at the bottom is a small MOTHER PCB containing 8 connectors where the PCBs plug in and the power input connector.
On Winning Run there is an additional PCB for the controls and cabinet motion.
The PAL labels suggest it was originally used with Metal Hawk. It may be used with other games too.


PCB Layouts
-----------

In the layouts below, the jumpers set the ROM type.
If horizontally shorted the ROM type is 27C101
If vertically shorted the ROM type is 27C301/27C1001
ROM labels/locations shown are printed on the PCB. Not all ROM positions are used.
See the ROM loading below (per game) for ROM usage and actual label names.

PCB#1 (top):

2252960601
(2252970601)
|--------------------------------------------------------------|
|            POINT3L.7P  POINT3U.5P   DIP28_SOCKET(2P)         |
|            POINT2L.7N  POINT2U.5N       |-------|            |
|                 JP5 O=O   JP6 O=O       |  TMS  |    40MHz   |
|                     O=O       0=0       |320C25 |            |
|  POINT1U.8L  MB8422                     |-------|            |
|                          PAL16L8         MB81C69   MB81C69   |
|  POINT0U.8J  MB8422      (WR-D2.4J)                          |
|                                          MB81C69   MB81C69   |
|                                                              |
|                                                              |
|JP3 O=O                                                       |
|    O=O                                                       |
|  POINT1L.2E  MB8422   62256     62256                        |
|                                                              |
|  POINT0L.8D  MB8422                                          |
|JP4 O=O                                                       |
|    O=O                                                       |
|                                                              |
|                                                              |
|                                    PAL16L8                   |
|                                    (WR-D1.3B)                |
|                                                              |
|                                                              |
|----|----------------------|----|------------------------|----|
     |----------------------|    |------------------------|


PCB#2 (2nd down):

2252960701
(2252970701)
|--------------------------------------------------------------|
|                                                              |
|  62256  62256  C157  C157  M5M5178                           |
|                                                              |
|  62256  62256  C157  C157  M5M5178                           |
|                                                              |
|  62256  62256  C157  C157                      62256         |
|                             C150                             |
|  62256  62256  C157  C157                      62256         |
|                                                              |
|  C157   C157                                                 |
|                                                              |
|  C157   C157                             62256               |
| |-------| |-------| |-------|                                |
| |L7A0080| |L7A0080| |L7A0081|   M5M5178  62256               |
| |110FAI | |110FAI | |111MUR |                                |
| |-------| |-------| |-------|   M5M5178  62256               |
|                                                              |
|   2018  2018              |---------|    62256               |
|                   20MHz   |         |                        |
|   2018  2018  2018        |  C167   |                        |
|        C157  C157         |         |               PAL16L8  |
|        C157  C157         |---------|              (WR-P1.1B)|
|                                                              |
|----|----------------------|----|------------------------|----|
     |----------------------|    |------------------------|


PCB#3 (3rd down):

2252960101
(2252970101)       RGB_CABLE     |------------------------|
|--------------------||||||------|------------------------|----|
|              TL084C             |---|              MB3771    |
|      MB87077           LB1760   |C65|  PAL16L8    PC910 PC900|
|MB87077   34063                  |---|  (SYS87B-2.3W)         |
|TL084C  TL084C                               DSW1(8) M5M5179  |
|LC7880  YM3012          PAL12L10  SYS2C65C.5P          |----| |
|MB8464  YM2151           (WR-C1.5P)                    |C139| |
|   |----|          C137          3.579545MHz   HN58C65 |----| |
|   |C121| 49.152MHz       8422    C149                        |
|   |----|          PAL12L10                65256     65256    |
|                   (WR-C2.8K)                                 |
|     2018     2018           62256  62256  MPRU.3K   MPRL.1K  |
|                                                              |
| VOI3.11E          MB8464                       68000         |
|                                                              |
| VOI2.11D |------| SND1.7D   C148    C148  DATA3U.3D DATA3L.1D|
|          | C140 |                                            |
| VOI1.11C |------| SND0.7C   65256  65256  DATA2U.3C DATA2L.1C|
|                                                              |
| VOI0.11B          6809   SPRU.6B  SPRL.4B DATA1U.3B DATA1L.1B|
|             JP3                       JP1                    |
|             O=O               68000   OO  DATA0U.3A DATAOL.1A|
|             O=O                       ||                     |
|                                       OO                     |
|----|----------------------|----|------------------------|----|
     |----------------------|    |------------------------|


PCB#4 (bottom):

2252960401
(2252970401)       RGB_CABLE
|--------------------||||||------------------------------------|
|                                                              |
| |----|                                 |----|                |
| |C164|                                 |C148|     68000      |
| |----|                                 |----|                |
|                                MB3771                        |
|                                PAL20V8    62256    62256     |
|                               (WR-G3.4S)                     |
|                           62256         GDT1L.3S  GDT1U.1S   |
| MB81461  MB81461               JP10 O-O                JP9 OO|
|                           62256 PAL16L8 GDT0L.3P  GDT0U.1P |||
| MB81461  MB81461               (WR-G2.4P)                  OO|
|                           62256         GPR1L.3L  GPR1U.1L   |
| MB81461  MB81461                 J J J                 JP8 OO|
|                                  P P P  GPR0L.3J  GPR0U.1J |||
| MB81461  MB81461         |----|  6 5 4                     OO|
|                          |C138|  O|O|O|  JP12                |
| MB81461  MB81461         |----|  O|O|O|    O|          JP7 OO|
|                   PAL16L8        O O O     O|     MB8422   |||
| MB81461  MB81461 (WR-G4.7L)      49.152MHz O               OO|
|                                          C165     MB8422     |
| MB81461  MB81461                       38.808MHz             |
|                                            PAL16L8           |
| MB81461  MB81461                          (WR-G1.3A)         |
|----|----------------------|----|------------------------|----|
     |----------------------|    |------------------------|


I/O & Drive Board For Motion Cabinet
------------------------------------

2286964100 (2286974100)
|--------------------------------|
|SW2 SW1  8255   8251     TLP521 |
|DSW(6)         EMPTY_SOCKET     |
| 4.9152MHz                      |
|RESET MB3773    ADC0809     J201|
|  68B09   EMPTY_SOCKET          |
| WR_DR1.5A                S1WB  |
|  8464                  LM324   |
|                        TLP521  |
| PAL20L10                 S2VB  |
|(MH1-DR2)             A490  J205|
|                      A490      |
|J206                   TLP511   |
|       8253     PAL14H8   TLP511|
|TLP511         (MH1-DR3)        |
|      J204 J202 J203            |
|TLP511                          |
|BCR16DM BCR16DM BCR16DM BCR16DM |
|--------------------------------|
Notes:
      J201 - 50 pin flat cable connector for controls
      J202 - 6 pin Power connector
      J203 - 2-pin Power connector
      J204 - 3-pin Power connector
      J205 - 4-pin connector for DC feedback motor
      J206 - 5-pin connector for main board communication
      S1WB - Bridge Rectifier
      S2VB - Bridge Rectifier
      A490 - Transistor
   BCR16DM - Transistor
    TLP511 - Toshiba TLP511 GAAS Infrared & Photo Thyristor
    TLP521 - Toshiba TLP521 Programmable Controller AC/DC-Input Module Solid State Relay
      8255 - Mitsubishi M5L8255AP-5 Programmable Peripheral Interface
      8251 - Mitsubishi M5L8251AP-5 Programmable Communication Interface
      8253 - Mitsubishi M5L8253P-5 Programmable Interval Timer
   ADC0809 - National Semiconductor ADC0809CCN 8-Bit Microprocessor Compatible A/D Converters with 8-Channel Multiplexer
     68B09 - Hitachi HD68B09EP CPU
      8464 - 8k x8-bit SRAM
     LM324 - National Semiconductor LM324 General Purpose Operational Amplifier
    MB3773 - Fujitsu MB3773 Power Supply Monitor with Watch-Dog Timer


Slot PCB
--------
2252960502 (2252970502)
V21 MOTHER PCB
|--------------------------------------------------------------|
|                                  PWR_CONN                    |
|    |--------SLOT1---------|    |---------SLOT5----------|    |
|    |--------SLOT2---------|    |---------SLOT6----------|    |
|    |--------SLOT3---------|    |---------SLOT7----------|    |
|    |--------SLOT4---------|    |---------SLOT8----------|    |
|--------------------------------------------------------------|


Filter Board
------------
2252960801 (2252970801)
|--------------------------------|
|    |----------------------|    |
|    |----------------------|    |
|                                |
|-------|60-PIN FLAT CABLE|------|
        |-------CONN------|

****************************

*/

#include "emu.h"
#include "screen.h"
#include "emupal.h"
#include "speaker.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6805/m6805.h"
#include "cpu/m6809/m6809.h"
#include "cpu/tms32025/tms32025.h"
#include "machine/timer.h"
#include "machine/nvram.h"
#include "namco65.h"
#include "namcos21_dsp.h"
#include "namco_c139.h"
#include "namco_c148.h"
#include "namcos21_3d.h"
#include "sound/c140.h"
#include "sound/ymopm.h"


namespace {

#define ENABLE_LOGGING      0

class namcos21_state : public driver_device
{
public:
	namcos21_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_slave(*this, "slave"),
		m_c65(*this, "c65mcu"),
		m_sci(*this, "sci"),
		m_master_intc(*this, "master_intc"),
		m_slave_intc(*this, "slave_intc"),
		m_c140(*this, "c140"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_audiobank(*this, "audiobank"),
		m_c140_region(*this, "c140"),
		m_dpram(*this, "dpram"),
		m_gpu_intc(*this, "gpu_intc"),
		m_namcos21_3d(*this, "namcos21_3d"),
		m_namcos21_dsp(*this, "namcos21dsp")
	{ }

	void configure_c148_standard(machine_config &config);
	void winrun(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_slave;
	required_device<namcoc65_device> m_c65;
	required_device<namco_c139_device> m_sci;
	required_device<namco_c148_device> m_master_intc;
	required_device<namco_c148_device> m_slave_intc;
	required_device<c140_device> m_c140;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_memory_bank m_audiobank;
	required_region_ptr<u16> m_c140_region;
	required_shared_ptr<uint8_t> m_dpram;
	required_device<namco_c148_device> m_gpu_intc;
	required_device<namcos21_3d_device> m_namcos21_3d;
	required_device<namcos21_dsp_device> m_namcos21_dsp;

	std::unique_ptr<uint8_t[]> m_gpu_videoram;
	std::unique_ptr<uint8_t[]> m_gpu_maskram;

	uint16_t m_video_enable;

	uint16_t m_winrun_color;
	uint16_t m_winrun_gpu_register[0x10/2];
	uint16_t video_enable_r();
	void video_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t dpram_word_r(offs_t offset);
	void dpram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t dpram_byte_r(offs_t offset);
	void dpram_byte_w(offs_t offset, uint8_t data);

	uint16_t winrun_gpu_color_r();
	void winrun_gpu_color_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t winrun_gpu_register_r(offs_t offset);
	void winrun_gpu_register_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void winrun_gpu_videoram_w(offs_t offset, uint16_t data);
	uint16_t winrun_gpu_videoram_r(offs_t offset);

	void eeprom_w(offs_t offset, uint8_t data);
	uint8_t eeprom_r(offs_t offset);

	void sound_bankselect_w(uint8_t data);

	void sound_reset_w(uint8_t data);
	void system_reset_w(uint8_t data);
	void reset_all_subcpus(int state);

	std::unique_ptr<uint8_t[]> m_eeprom;

	TIMER_DEVICE_CALLBACK_MEMBER(screen_scanline);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void winrun_bitmap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void configure_c65_namcos21(machine_config &config);

	void winrun_master_map(address_map &map) ATTR_COLD;
	void winrun_slave_map(address_map &map) ATTR_COLD;
	void winrun_gpu_map(address_map &map) ATTR_COLD;

	void sound_map(address_map &map) ATTR_COLD;
	void c140_map(address_map &map) ATTR_COLD;
};

void namcos21_state::video_start()
{
	m_gpu_videoram = std::make_unique<uint8_t[]>(0x80000);
	m_gpu_maskram = std::make_unique<uint8_t[]>(0x80000);

	save_pointer(NAME(m_gpu_videoram), 0x80000);
	save_pointer(NAME(m_gpu_maskram), 0x80000);
}

uint16_t namcos21_state::winrun_gpu_color_r()
{
	return m_winrun_color;
}

void namcos21_state::winrun_gpu_color_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA( &m_winrun_color );
}

uint16_t namcos21_state::winrun_gpu_register_r(offs_t offset)
{
	return m_winrun_gpu_register[offset];
}

void namcos21_state::winrun_gpu_register_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA( &m_winrun_gpu_register[offset] );
	m_screen->update_partial(m_screen->vpos());
}

void namcos21_state::winrun_gpu_videoram_w(offs_t offset, uint16_t data)
{
	int color = data>>8;
	int mask  = data&0xff;
	for( int i=0; i<8; i++ )
	{
		if( mask&(0x01<<i) )
		{
			m_gpu_videoram[(offset+i)&0x7ffff] = color;
			m_gpu_maskram[(offset+i)&0x7ffff] = mask;
		}
	}
}

uint16_t namcos21_state::winrun_gpu_videoram_r(offs_t offset)
{
	return (m_gpu_videoram[offset]<<8) | m_gpu_maskram[offset];
}

void namcos21_state::winrun_bitmap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const videoram = m_gpu_videoram.get();
	//printf("%d %d (%d %d) - %04x %04x %04x|%04x %04x\n",cliprect.top(),cliprect.bottom(),m_screen->vpos(),m_gpu_intc->get_posirq_line(),m_winrun_gpu_register[0],m_winrun_gpu_register[2/2],m_winrun_gpu_register[4/2],m_winrun_gpu_register[0xa/2],m_winrun_gpu_register[0xc/2]);

	int const yscroll = -cliprect.top()+(int16_t)m_winrun_gpu_register[0x2/2];
	int const xscroll = 0;//m_winrun_gpu_register[0xc/2] >> 7;
	int const base = 0x1000+0x100*(m_winrun_color&0xf);
	for( int sy=cliprect.top(); sy<=cliprect.bottom(); sy++ )
	{
		uint8_t const *const pSource = &videoram[((yscroll+sy)&0x3ff)*0x200];
		uint16_t *const pDest = &bitmap.pix(sy);
		for( int sx=cliprect.left(); sx<=cliprect.right(); sx++ )
		{
			int const pen = pSource[(sx+xscroll) & 0x1ff];
			switch( pen )
			{
			case 0xff:
				break;
			// TODO: additive blending? winrun car select uses register [0xc] for a xscroll value
			case 0x00:
				pDest[sx] = (pDest[sx]&0x1fff)+0x4000;
				break;
			case 0x01:
				pDest[sx] = (pDest[sx]&0x1fff)+0x6000;
				break;
			default:
				pDest[sx] = base|pen;
				break;
			}
		}
	}
}


uint32_t namcos21_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0xff, cliprect );

	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0x7fc0, 0x7ffe);
	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0, 0x7fbf);
	winrun_bitmap_draw(bitmap,cliprect);

	//popmessage("%04x %04x %04x|%04x %04x",m_winrun_gpu_register[0],m_winrun_gpu_register[2/2],m_winrun_gpu_register[4/2],m_winrun_gpu_register[0xa/2],m_winrun_gpu_register[0xc/2]);

	return 0;
}



[[maybe_unused]] uint16_t namcos21_state::video_enable_r()
{
	return m_video_enable;
}

[[maybe_unused]] void namcos21_state::video_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA( &m_video_enable ); /* 0x40 = enable */
	if( m_video_enable!=0 && m_video_enable!=0x40 )
	{
		logerror( "unexpected video_enable_w=0x%x\n", m_video_enable );
	}
}

/***********************************************************/

/* dual port ram memory handlers */

uint16_t namcos21_state::dpram_word_r(offs_t offset)
{
	return m_dpram[offset];
}

void namcos21_state::dpram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if( ACCESSING_BITS_0_7 )
	{
		m_dpram[offset] = data&0xff;
	}
}

uint8_t namcos21_state::dpram_byte_r(offs_t offset)
{
	return m_dpram[offset];
}

void namcos21_state::dpram_byte_w(offs_t offset, uint8_t data)
{
	m_dpram[offset] = data;
}

/******************************************************************************/

void namcos21_state::winrun_master_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram(); /* work RAM */
	map(0x180000, 0x183fff).rw(FUNC(namcos21_state::eeprom_r), FUNC(namcos21_state::eeprom_w)).umask16(0x00ff);
	map(0x1c0000, 0x1fffff).m(m_master_intc, FUNC(namco_c148_device::map));

	// DSP Related
	map(0x250000, 0x25ffff).ram().share("namcos21dsp:winrun_polydata");
	map(0x260000, 0x26ffff).ram(); /* unused? */
	map(0x280000, 0x281fff).w(m_namcos21_dsp, FUNC(namcos21_dsp_device::winrun_dspbios_w));
	map(0x380000, 0x38000f).rw(m_namcos21_dsp, FUNC(namcos21_dsp_device::winrun_dspcomram_control_r), FUNC(namcos21_dsp_device::winrun_dspcomram_control_w));
	map(0x3c0000, 0x3c1fff).rw(m_namcos21_dsp, FUNC(namcos21_dsp_device::winrun_68k_dspcomram_r), FUNC(namcos21_dsp_device::winrun_68k_dspcomram_w));
	map(0x400000, 0x400001).w(m_namcos21_dsp, FUNC(namcos21_dsp_device::pointram_control_w));
	map(0x440000, 0x440001).rw(m_namcos21_dsp, FUNC(namcos21_dsp_device::pointram_data_r), FUNC(namcos21_dsp_device::pointram_data_w));

	map(0x600000, 0x60ffff).ram().share("gpu_comram");
	map(0x800000, 0x87ffff).rom().region("data", 0);
	map(0x900000, 0x90ffff).ram().share("sharedram");
	map(0xa00000, 0xa00fff).rw(FUNC(namcos21_state::dpram_word_r), FUNC(namcos21_state::dpram_word_w));
	map(0xb00000, 0xb03fff).rw(m_sci, FUNC(namco_c139_device::ram_r), FUNC(namco_c139_device::ram_w));
	map(0xb80000, 0xb8000f).m(m_sci, FUNC(namco_c139_device::regs_map));
}

void namcos21_state::winrun_slave_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x13ffff).ram();
	map(0x1c0000, 0x1fffff).m(m_slave_intc, FUNC(namco_c148_device::map));
	map(0x600000, 0x60ffff).ram().share("gpu_comram");
	map(0x800000, 0x87ffff).rom().region("data", 0);
	map(0x900000, 0x90ffff).ram().share("sharedram");
	map(0xa00000, 0xa00fff).rw(FUNC(namcos21_state::dpram_word_r), FUNC(namcos21_state::dpram_word_w));
	map(0xb00000, 0xb03fff).rw(m_sci, FUNC(namco_c139_device::ram_r), FUNC(namco_c139_device::ram_w));
	map(0xb80000, 0xb8000f).m(m_sci, FUNC(namco_c139_device::regs_map));
}


void namcos21_state::winrun_gpu_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100001).rw(FUNC(namcos21_state::winrun_gpu_color_r), FUNC(namcos21_state::winrun_gpu_color_w)); /* ? */
	map(0x180000, 0x19ffff).ram(); /* work RAM */
	map(0x1c0000, 0x1fffff).m(m_gpu_intc, FUNC(namco_c148_device::map));
	map(0x200000, 0x20ffff).ram().share("gpu_comram");
	map(0x400000, 0x40ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x410000, 0x41ffff).ram().w(m_palette, FUNC(palette_device::write16_ext)).share("palette_ext");
	map(0x600000, 0x6fffff).rom().region("gdata", 0);
	map(0xc00000, 0xcfffff).rw(FUNC(namcos21_state::winrun_gpu_videoram_r), FUNC(namcos21_state::winrun_gpu_videoram_w));
	map(0xd00000, 0xd0000f).rw(FUNC(namcos21_state::winrun_gpu_register_r), FUNC(namcos21_state::winrun_gpu_register_w));
	map(0xe0000d, 0xe0000d).rw(m_gpu_intc, FUNC(namco_c148_device::ext_posirq_line_r), FUNC(namco_c148_device::ext_posirq_line_w));
}


/*************************************************************/
/* SOUND 6809 CPU Memory declarations                        */
/*************************************************************/

void namcos21_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr("audiobank"); /* banked */
	map(0x3000, 0x3003).nopw(); /* ? */
	map(0x4000, 0x4001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x5000, 0x51ff).mirror(0x0e00).rw(m_c140, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w));
	map(0x6000, 0x61ff).mirror(0x0e00).rw(m_c140, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w)); // mirrored
	map(0x7000, 0x77ff).mirror(0x0800).rw(FUNC(namcos21_state::dpram_byte_r), FUNC(namcos21_state::dpram_byte_w)).share("dpram");
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).nopw(); /* amplifier enable on 1st write */
	map(0xc000, 0xffff).nopw(); /* avoid debug log noise; games write frequently to 0xe000 */
	map(0xc000, 0xc001).w(FUNC(namcos21_state::sound_bankselect_w));
	map(0xd001, 0xd001).nopw(); /* watchdog */
	map(0xd000, 0xffff).rom().region("audiocpu", 0x01000);
}

void namcos21_state::c140_map(address_map &map)
{
	map.global_mask(0x7fffff);
	// TODO: LSB not used? verify from schematics/real hardware
	map(0x000000, 0x7fffff).lr16([this](offs_t offset) { return m_c140_region[((offset & 0x300000) >> 1) | (offset & 0x7ffff)]; }, "c140_rom_r");
}


/*************************************************************/
/* I/O HD63705 MCU Memory declarations                       */
/*************************************************************/

void namcos21_state::configure_c65_namcos21(machine_config &config)
{
	NAMCOC65(config, m_c65, 2048000);
	m_c65->in_pb_callback().set_ioport("MCUB");
	m_c65->in_pc_callback().set_ioport("MCUC");
	m_c65->in_ph_callback().set_ioport("MCUH");
	m_c65->in_pdsw_callback().set_ioport("DSW");
	m_c65->di0_in_cb().set_ioport("MCUDI0");
	m_c65->di1_in_cb().set_ioport("MCUDI1");
	m_c65->di2_in_cb().set_ioport("MCUDI2");
	m_c65->di3_in_cb().set_ioport("MCUDI3");
	m_c65->an0_in_cb().set_ioport("AN0");
	m_c65->an1_in_cb().set_ioport("AN1");
	m_c65->an2_in_cb().set_ioport("AN2");
	m_c65->an3_in_cb().set_ioport("AN3");
	m_c65->an4_in_cb().set_ioport("AN4");
	m_c65->an5_in_cb().set_ioport("AN5");
	m_c65->an6_in_cb().set_ioport("AN6");
	m_c65->an7_in_cb().set_ioport("AN7");
	m_c65->dp_in_callback().set(FUNC(namcos21_state::dpram_byte_r));
	m_c65->dp_out_callback().set(FUNC(namcos21_state::dpram_byte_w));
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

/* "SCI - ? */
static INPUT_PORTS_START( winrun )
	PORT_INCLUDE(s21default)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x20, 0x20, "PCM ROM")
	PORT_DIPSETTING(    0x20, "2M" )
	PORT_DIPSETTING(    0x00, "4M" )

	PORT_MODIFY("MCUB")        /* 63B05Z0 - PORT B */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 ) /* ? */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) /* ? */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) /* ? */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) /* ? */

	PORT_MODIFY("AN0")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("AN1")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(10) PORT_NAME("Gas Pedal")
	PORT_MODIFY("AN2")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(10) PORT_NAME("Steering Wheel")
	PORT_MODIFY("AN3")      /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(10) PORT_NAME("Brake Pedal")
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
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Shift Down")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_NAME("Shift Up")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( winrungp )
	PORT_INCLUDE(winrun)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x20, 0x00, "PCM ROM")
	PORT_DIPSETTING(    0x20, "2M" )
	PORT_DIPSETTING(    0x00, "4M" )
INPUT_PORTS_END

void namcos21_state::sound_bankselect_w(uint8_t data)
{
	m_audiobank->set_entry(data>>4);
}

void namcos21_state::sound_reset_w(uint8_t data)
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
}

void namcos21_state::system_reset_w(uint8_t data)
{
	reset_all_subcpus(data & 1 ? CLEAR_LINE : ASSERT_LINE);

	if (data & 0x01)
		m_maincpu->yield();
}

void namcos21_state::reset_all_subcpus(int state)
{
	m_slave->set_input_line(INPUT_LINE_RESET, state);
	m_c65->ext_reset(state);
}

void namcos21_state::eeprom_w(offs_t offset, uint8_t data)
{
	m_eeprom[offset] = data;
}

uint8_t namcos21_state::eeprom_r(offs_t offset)
{
	return m_eeprom[offset];
}

void namcos21_state::machine_reset()
{
	/* Initialise the bank select in the sound CPU */
	m_audiobank->set_entry(0); /* Page in bank 0 */

	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE );

	/* Place CPU2 & CPU3 into the reset condition */
	reset_all_subcpus(ASSERT_LINE);
}



void namcos21_state::machine_start()
{
	m_eeprom = std::make_unique<uint8_t[]>(0x2000);
	subdevice<nvram_device>("nvram")->set_base(m_eeprom.get(), 0x2000);

	uint32_t max = memregion("audiocpu")->bytes() / 0x4000;
	for (int i = 0; i < 0x10; i++)
		m_audiobank->configure_entry(i, memregion("audiocpu")->base() + (i % max) * 0x4000);

	save_item(NAME(m_video_enable));
	save_item(NAME(m_winrun_color));
	save_item(NAME(m_winrun_gpu_register));
}

TIMER_DEVICE_CALLBACK_MEMBER(namcos21_state::screen_scanline)
{
	int scanline = param;
	//  int cur_posirq = get_posirq_scanline()*2;

	if (scanline == 240 * 2)
	{
		m_master_intc->vblank_irq_trigger();
		m_slave_intc->vblank_irq_trigger();
		m_gpu_intc->vblank_irq_trigger();
		m_c65->ext_interrupt(HOLD_LINE);
	}

	if (scanline == (0xff - m_gpu_intc->get_posirq_line()) * 2)
		m_gpu_intc->pos_irq_trigger();
}

void namcos21_state::configure_c148_standard(machine_config &config)
{
	NAMCO_C148(config, m_master_intc, 0, m_maincpu, true);
	m_master_intc->link_c148_device(m_slave_intc);
	m_master_intc->out_ext1_callback().set(FUNC(namcos21_state::sound_reset_w));
	m_master_intc->out_ext2_callback().set(FUNC(namcos21_state::system_reset_w));

	NAMCO_C148(config, m_slave_intc, 0, m_slave, false);
	m_slave_intc->link_c148_device(m_master_intc);
}

void namcos21_state::winrun(machine_config &config)
{
	M68000(config, m_maincpu, 49.152_MHz_XTAL / 4); /* Master */
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos21_state::winrun_master_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(namcos21_state::screen_scanline), "screen", 0, 1);

	M68000(config, m_slave, 49.152_MHz_XTAL / 4); /* Slave */
	m_slave->set_addrmap(AS_PROGRAM, &namcos21_state::winrun_slave_map);

	MC6809E(config, m_audiocpu, 49.152_MHz_XTAL / 24); /* Sound */
	m_audiocpu->set_addrmap(AS_PROGRAM, &namcos21_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(namcos21_state::irq0_line_hold), attotime::from_hz(2*60));

	configure_c65_namcos21(config);

	NAMCOS21_DSP(config, m_namcos21_dsp, 0);
	m_namcos21_dsp->set_renderer_tag("namcos21_3d");

	m68000_device &gpu(M68000(config, "gpu", 49.152_MHz_XTAL / 4)); /* graphics coprocessor */
	gpu.set_addrmap(AS_PROGRAM, &namcos21_state::winrun_gpu_map);

	configure_c148_standard(config);
	NAMCO_C148(config, m_gpu_intc, 0, "gpu", false);
	NAMCO_C139(config, m_sci, 0);

	config.set_maximum_quantum(attotime::from_hz(6000)); /* 100 CPU slices per frame */

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: basic parameters to get 60.606060 Hz, x2 is for interlace
	m_screen->set_raw(49.152_MHz_XTAL / 4 * 2, 768, 0, 496, 264*2, 0, 480);
	m_screen->set_screen_update(FUNC(namcos21_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBRG_888, 0x10000/2);

	NAMCOS21_3D(config, m_namcos21_3d, 0);
	m_namcos21_3d->set_fixed_palbase(0x4000);
	m_namcos21_3d->set_zz_shift_mult(10, 0x100);
	m_namcos21_3d->set_depth_reverse(true);
	m_namcos21_3d->set_framebuffer_size(496,480);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	C140(config, m_c140, 49.152_MHz_XTAL / 2304);
	m_c140->set_addrmap(0, &namcos21_state::c140_map);
	m_c140->int1_callback().set_inputline(m_audiocpu, M6809_FIRQ_LINE);
	m_c140->add_route(0, "lspeaker", 0.50);
	m_c140->add_route(1, "rspeaker", 0.50);

	YM2151(config, "ymsnd", 3.579545_MHz_XTAL).add_route(0, "lspeaker", 0.30).add_route(1, "rspeaker", 0.30);
}


ROM_START( winrun )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "wr2-mpub.3k",  0x000000, 0x20000, CRC(3bb0ea17) SHA1(201cedf5865224c1c4a0c9b017982e36ec9b8243) )
	ROM_LOAD16_BYTE( "wr2-mplb.1k",  0x000001, 0x20000, CRC(95465062) SHA1(7d010ff92e87949b7b9109f8320ab61de7d0400a) )

	ROM_REGION( 0x40000, "slave", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "wr1-spu.6b",  0x000000, 0x20000, CRC(d4915d44) SHA1(57056051138b259d021b3e7cc1c43a9f951b5cc1) )
	ROM_LOAD16_BYTE( "wr1-spl.4b",  0x000001, 0x20000, CRC(0c336505) SHA1(edee073bf6fabe45c577b0b9375295183eb30c62) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "wr1-snd0.7c",  0x000000, 0x020000, CRC(698bae12) SHA1(86d22c1c639a9489e3c95820e4e3f04c30407e41) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x80000, "gpu", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "wr1-gp0u.1k",  0x00000, 0x20000, CRC(c66a43be) SHA1(88ec02c5c18c8bb91a95934c14e9ae530ae09880) )
	ROM_LOAD16_BYTE( "wr1-gp0l.3k",  0x00001, 0x20000, CRC(91a70e6f) SHA1(e613e2544f63cd386588445a2a199ae6b84d741e) )
	ROM_LOAD16_BYTE( "wr1-gp1u.1l",  0x40000, 0x20000, CRC(8ff51a3d) SHA1(81fbcd4e8c51742f35607537e1b1a86fd7782827) )
	ROM_LOAD16_BYTE( "wr1-gp1l.3l",  0x40001, 0x20000, CRC(9360d34e) SHA1(e558eb540c02acfe84f2dfe2d65afd609b7f3207) )

	ROM_REGION16_BE( 0x80000, "data", 0 )
	ROM_LOAD16_BYTE( "wr1-d0u.3a", 0x00000, 0x20000, CRC(84ea1492) SHA1(56274b39bd5be076c9904d9ed9ce3f6e29d9f038) )
	ROM_LOAD16_BYTE( "wr1-d0l.1a", 0x00001, 0x20000, CRC(b81508f9) SHA1(51c03c3dff86cece9790667b1557de940ebccbe9) )
	ROM_LOAD16_BYTE( "wr1-d1u.3b", 0x40000, 0x20000, CRC(bbd1fdd7) SHA1(026e9410525fa0e93f155949bbc1d3b8a2785bd1) )
	ROM_LOAD16_BYTE( "wr1-d1l.1b", 0x40001, 0x20000, CRC(8ddd7eac) SHA1(b873b253b0a095e66a9f68d45a2cb41fa025ba16) )

	ROM_REGION16_BE( 0x100000, "gdata", 0 ) /* bitmapped graphics */
	ROM_LOAD16_BYTE( "wr1-gd0u-2.1p",  0x00000, 0x40000, CRC(9752eef5) SHA1(d6df0faf9c2696247bdf463f53c1e474ec595dd0) )
	ROM_LOAD16_BYTE( "wr1-gd0l-2.3p",  0x00001, 0x40000, CRC(349c95cc) SHA1(8898eecf5918485ec683900520f123483077df28) )

	ROM_REGION16_BE( 0x80000, "namcos21dsp:point16", 0 ) /* 3d objects */
	ROM_LOAD16_BYTE( "wr1-pt0u.8j", 0x00000, 0x20000, CRC(7ec4cf6b) SHA1(92ec92567b9f7321efb4a3724cbcdba216eb22f9) )
	ROM_LOAD16_BYTE( "wr1-pt0l.8d", 0x00001, 0x20000, CRC(58c14b73) SHA1(e34a26866cd870743e166669f7fa5915a82104e9) )

	ROM_REGION16_BE( 0x400000, "c140", ROMREGION_ERASE00 ) /* sound samples */
	ROM_LOAD16_BYTE("wr-voi-0.11b", 0x080000, 0x40000, CRC(8040b645) SHA1(7ccafb3073fa79910e26cf9b8b6e8e9ae22e55fc) )
	ROM_LOAD16_BYTE("wr-voi-1.11c", 0x180000, 0x40000, CRC(d347e904) SHA1(620cd07e6230322c306283e45a43fa1e217028d4) )
	ROM_LOAD16_BYTE("wr-voi-2.11d", 0x280000, 0x40000, CRC(b34747af) SHA1(7e0b55631bffa0583bf4f7f5368db9f09e411ba1) )
	ROM_LOAD16_BYTE("wr-voi-3.11e", 0x380000, 0x40000, CRC(43085303) SHA1(9f743055c20df3548879118194244e37a0b91f7e) )

	ROM_REGION( 0x1000, "pals", 0 )
	/* Main PCB (2252960101) */
	ROM_LOAD("sys87b-2.3w", 0x000, 0x104, CRC(18f43c22) SHA1(72849c5b842678bb9037541d26d4c99cdf879982) ) /* PAL16L8ACN */
	ROM_LOAD("wr-c1.6p",    0x000, 0x040, CRC(d6c33258) SHA1(5baf71fae1ad73a75d91ee2dededca2254b4f414) ) /* PAL12L10CNS */
	ROM_LOAD("wr-c2.8k",    0x000, 0x040, CRC(a3e77ade) SHA1(b4282fc4d21ce7813aba7772a85d721fb25144b6) ) /* PAL12L10CNS */

	/* Framebuffer PCB (2252960900) */
	ROM_LOAD("wr-g1.3a",    0x000, 0x104, CRC(d6b4373d) SHA1(b4db4526a37b7d6862cec1d223f2281e5120f225) ) /* PAL16L8ACN */
	ROM_LOAD("wr-g2b.4p",   0x000, 0x104, NO_DUMP ) /* PAL16L8ACN */
	ROM_LOAD("wr-g3.4s",    0x000, 0x104, NO_DUMP ) /* GAL20V8 */
	ROM_LOAD("wr-g4.7f",    0x000, 0x104, CRC(f858b32b) SHA1(59af496e4416c9becb116315542858ac917cbe46) ) /* PAL16L8ACN */

	/* 3D PCB (2252960701) */
	ROM_LOAD("wr-p1.1b",    0x000, 0x104, CRC(5856cc43) SHA1(4578c7a7d731c61d678d594dd071370db46cc3f7) ) /* PAL16L8ACN */

	/* DSP PCB (2252960601) */
	ROM_LOAD("wr-d1.3b",    0x000, 0x104, CRC(7a072b71) SHA1(45b8d9405a2c1a40f2cd9c6a33d105565136c538) ) /* PAL16L8ACN */
	ROM_LOAD("wr-d2.4j",    0x000, 0x104, CRC(614de474) SHA1(80c3ab287896e07073a70dd42e7b47f515dd3931) ) /* PAL16L8ACN */

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "nvram", 0x0000, 0x2000, CRC(93b4c4b5) SHA1(2fb5b9437304c97a2436b446671284aae69d545d) )
ROM_END

ROM_START( winrungp )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "sg1-mp-ub.3k", 0x000000, 0x20000, CRC(7f9b855a) SHA1(6d39a3a9959dbcd0047dbaab0fcd68adc81f5508) )
	ROM_LOAD16_BYTE( "sg1-mp-lb.1k", 0x000001, 0x20000, CRC(a45e8543) SHA1(f9e583a988e4661026ee7873a48d078225778df3) )

	ROM_REGION( 0x40000, "slave", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "sg1-sp-u.6b", 0x000000, 0x20000, CRC(7c9c3a3f) SHA1(cacb45c9111ac66c6e60b7a0cacd8bf47fd00752) )
	ROM_LOAD16_BYTE( "sg1-sp-l.4b", 0x000001, 0x20000, CRC(5068fc5d) SHA1(7f6e80f74985959509d824318a4a7ff2b11953da) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "sg1-snd0.7c", 0x000000, 0x020000, CRC(de04b794) SHA1(191f4d79ac2375d7060f3d83ec753185e92f28ea) )

	ROM_REGION16_BE( 0x2000, "tms", 0 )
	ROM_LOAD( "tms320c25fnl_wybux8j1_japan", 0x0000, 0x2000, CRC(01f3fd3a) SHA1(1fa5185fa60c8c4097ed0e51a5e0fe1fa49b4b75) ) // decapped. TODO: verify, hook up and check if same for all games

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin", 0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x80000, "gpu", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "sg1-gp0-u.1j", 0x00000, 0x20000, CRC(475da78a) SHA1(6e69bcc6caf2e3cd28fed75796c8992e754f9323) )
	ROM_LOAD16_BYTE( "sg1-gp0-l.3j", 0x00001, 0x20000, CRC(580479bf) SHA1(ba682190cba0d3cdc49aa4937c898ba7ed2a25f5) )
	ROM_LOAD16_BYTE( "sg1-gp1-u.1l", 0x40000, 0x20000, CRC(f5f2e927) SHA1(ebf709f16f01f1a634de9121454537cda74e891b) )
	ROM_LOAD16_BYTE( "sg1-gp1-l.3l", 0x40001, 0x20000, CRC(17ed90a5) SHA1(386bdcb11dcbe400f5be1fe4a7418158b46e50ef) )

	ROM_REGION16_BE( 0x80000, "data", 0 )
	ROM_LOAD16_BYTE( "sg1-data0-u.3a", 0x00000, 0x20000, CRC(1dde2ac2) SHA1(2d20a434561c04e48b52a2137a8c9047e17c1013) )
	ROM_LOAD16_BYTE( "sg1-data0-l.1a", 0x00001, 0x20000, CRC(2afeb77e) SHA1(ac1552f6e2788158d3477b6a0981d001d6cbdf13) )
	ROM_LOAD16_BYTE( "sg1-data1-u.3b", 0x40000, 0x20000, CRC(5664b09e) SHA1(10c1c29614eee2cffcfd69085f0450d81ba2e25f) )
	ROM_LOAD16_BYTE( "sg1-data1-l.1b", 0x40001, 0x20000, CRC(2dbc7de4) SHA1(824304c95942c7296f8e8dcf8ee7e22bf56154b1) )

	ROM_REGION16_BE( 0x100000, "gdata", 0 ) /* bitmapped graphics */
	ROM_LOAD16_BYTE( "sg1-gd0-u.1p", 0x00000, 0x40000, CRC(7838fcde) SHA1(45e31269eed1999b73c41c2f5d2c5bfbbdaf23df) )
	ROM_LOAD16_BYTE( "sg1-gd0-l.3p", 0x00001, 0x40000, CRC(4bd02b9a) SHA1(b2fdfd1c1325864aaad87f5358ab9bbdd79ff6ae) )
	ROM_LOAD16_BYTE( "sg1-gd1-u.1s", 0x80000, 0x40000, CRC(271db29b) SHA1(8b35fcf273b9aec28d4c606c41c0626dded697e1) )
	ROM_LOAD16_BYTE( "sg1-gd1-l.3s", 0x80001, 0x40000, CRC(a6c4da96) SHA1(377dbf21a1bede01de16708c96c112abab4417ce) )

	ROM_REGION16_BE( 0x80000, "namcos21dsp:point16", 0 ) /* 3d objects */
	ROM_LOAD16_BYTE( "sg1-pt0-u.8j", 0x00000, 0x20000, CRC(160c3634) SHA1(485d20d6cc459f17d77682201dee07bdf76bf343) )
	ROM_LOAD16_BYTE( "sg1-pt0-l.8d", 0x00001, 0x20000, CRC(b5a665bf) SHA1(5af6ec492f31395c0492e14590b025b120067b8d) )
	ROM_LOAD16_BYTE( "sg1-pt1-u.8l", 0x40000, 0x20000, CRC(b63d3006) SHA1(78e78619766b0fd91b1e830cfb066495d6773981) )
	ROM_LOAD16_BYTE( "sg1-pt1-l.8e", 0x40001, 0x20000, CRC(6385e325) SHA1(d50bceb2e9c0d0a38d7b0f918f99c482649e260d) )

	ROM_REGION16_BE( 0x400000, "c140", ROMREGION_ERASE00 ) /* sound samples */
	ROM_LOAD16_BYTE("sg-voi-1.11c", 0x100000, 0x80000,CRC(7dcccb31) SHA1(4441b37691434b13eae5dee2d04dc12a56b04d2a) )
	ROM_LOAD16_BYTE("sg-voi-3.11e", 0x300000, 0x80000,CRC(a198141c) SHA1(b4ca352e6aedd9d7a7e5e39e840f1d3a7145900e) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "nvram", 0x0000, 0x2000, CRC(93cca84c) SHA1(e39510d9f066266a77780662a6d991c3dd0348d1) )
ROM_END

ROM_START( winrun91 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "r911-mpu.3k", 0x000000, 0x20000, CRC(80a0e5be) SHA1(6613b95e164c2032ea9043e4161130c6b3262492) )
	ROM_LOAD16_BYTE( "r911-mpl.1k", 0x000001, 0x20000, CRC(942172d8) SHA1(21d8dfd2165b5ceb0399fdb53d9d0f51f1255803) )

	ROM_REGION( 0x40000, "slave", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "r911-spu.6b", 0x000000, 0x20000, CRC(0221d4b2) SHA1(65fd38b1cfaa6693d71248561d764a9ea1098c56) )
	ROM_LOAD16_BYTE( "r911-spl.4b", 0x000001, 0x20000, CRC(288799e2) SHA1(2c4bf0cf9c71458fff4dd77e426a76685d9e1bab) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Sound */
	ROM_LOAD( "r911-snd0.7c", 0x000000, 0x020000, CRC(6a321e1e) SHA1(b2e77cac4ed7609593fa5a462c9d78526451e477) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin", 0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x80000, "gpu", 0 ) /* 68k code */
	ROM_LOAD16_BYTE( "r911-gp0u.1j", 0x00000, 0x20000, CRC(f5469a29) SHA1(38b6ea1fbe482b69fbb0e2f44f44a0ca2a49f6bc) )
	ROM_LOAD16_BYTE( "r911-gp0l.3j", 0x00001, 0x20000, CRC(5c18f596) SHA1(215cbda62254e31b4ff6431623384df1639bfdb7) )
	ROM_LOAD16_BYTE( "r911-gp1u.1l", 0x40000, 0x20000, CRC(146ab6b8) SHA1(aefb89585bf311f8d33f18298fea326ef1f19f1e) )
	ROM_LOAD16_BYTE( "r911-gp1l.3l", 0x40001, 0x20000, CRC(96c2463c) SHA1(e43db580e7b454af04c22e894108fbb56da0eeb5) )

	ROM_REGION16_BE( 0x80000, "data", 0 )
	ROM_LOAD16_BYTE( "r911-d0u.3a", 0x00000, 0x20000, CRC(dcb27da5) SHA1(ecd72397d10313fe8dcb8589bdc5d88d4298b26c) )
	ROM_LOAD16_BYTE( "r911-d0l.1a", 0x00001, 0x20000, CRC(f692a8f3) SHA1(4c29f60400b18d9ef0425de149618da6cf762ca4) )
	ROM_LOAD16_BYTE( "r911-d1u.3b", 0x40000, 0x20000, CRC(ac2afd1b) SHA1(510eb41931164b086c85ba0a86d6f10b88f5e534) )
	ROM_LOAD16_BYTE( "r911-d1l.1b", 0x40001, 0x20000, CRC(ebb51af1) SHA1(87b7b64ee662bf652add1e1199e42391d0e2f7e8) )

	ROM_REGION16_BE( 0x100000, "gdata", 0 ) /* bitmapped graphics */
	ROM_LOAD16_BYTE( "r911-gd0u.1p", 0x00000, 0x40000, CRC(33f5a19b) SHA1(b1dbd242168007f80e13e11c78b34abc1668883e) )
	ROM_LOAD16_BYTE( "r911-gd0l.3p", 0x00001, 0x40000, CRC(9a29500e) SHA1(c605f86b138e0a4c3163ffd967482e298a15fbe7) )
	ROM_LOAD16_BYTE( "r911-gd1u.1s", 0x80000, 0x40000, CRC(17e5a61c) SHA1(272ebd7daa56847f1887809535362331b5465dec) )
	ROM_LOAD16_BYTE( "r911-gd1l.3s", 0x80001, 0x40000, CRC(64df59a2) SHA1(1e9d0945b94780bb0be16803e767466d2cda07e8) )

	ROM_REGION16_BE( 0x80000, "namcos21dsp:point16", 0 ) /* winrun91 - 3d objects */
	ROM_LOAD16_BYTE( "r911-pt0u.8j", 0x00000, 0x20000, CRC(abf512a6) SHA1(e86288039d6c4dedfa95b11cb7e4b87637f90c09) ) /* Version on SYSTEM21B CPU only has R911 PTU @ 8W */
	ROM_LOAD16_BYTE( "r911-pt0l.8d", 0x00001, 0x20000, CRC(ac8d468c) SHA1(d1b457a19a5d3259d0caf933f42b3a02b485867b) ) /* and R911 PTL @ 12W with rom type 27C020 */
	ROM_LOAD16_BYTE( "r911-pt1u.8l", 0x40000, 0x20000, CRC(7e5dab74) SHA1(5bde219d5b4305d38d17b494b2e759f05d05329f) )
	ROM_LOAD16_BYTE( "r911-pt1l.8e", 0x40001, 0x20000, CRC(38a54ec5) SHA1(5c6017c98cae674868153ff2d64532027cf0ab83) )

	ROM_REGION16_BE( 0x400000, "c140", ROMREGION_ERASE00 ) /* sound samples */
	ROM_LOAD16_BYTE("r911-avo1.11c", 0x100000, 0x80000,CRC(9fb33af3) SHA1(666630a8e5766ca4c3275961963c3e713dfdda2d) )
	ROM_LOAD16_BYTE("r911-avo3.11e", 0x300000, 0x80000,CRC(76e22f92) SHA1(0e1b8d35a5b9c20cc3192d935f0c9da1e69679d2) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "nvram", 0x0000, 0x2000, CRC(75bcbc22) SHA1(1e7e785735d27aa8cd8393b16b589a46ecd7956a) )
ROM_END

} // Anonymous namespace


/*    YEAR  NAME       PARENT    MACHINE   INPUT       CLASS           INIT           MONITOR  COMPANY  FULLNAME                                 FLAGS */

// Original 'Namco System 21' with C65 I/O MCU, uses TMS320C25 DSP with no custom part number
GAME( 1988, winrun,    0,        winrun,   winrun,     namcos21_state, empty_init,   ROT0,    "Namco", "Winning Run (World) (89/06/06, Ver.09)",                   MACHINE_IMPERFECT_GRAPHICS ) // Sub Ver.09, 1989, Graphic Ver .06, 89/01/14, Sound Ver.2.00
GAME( 1989, winrungp,  0,        winrun,   winrungp,   namcos21_state, empty_init,   ROT0,    "Namco", "Winning Run Suzuka Grand Prix (Japan) (89/12/03, Ver.02)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN ) // Sub Ver.02, 1989, Graphic Ver.02 89/12/03, Sound Ver.0000
// Available on a size/cost reduced 2 PCB set with 'Namco System 21B' printed on each board, still C65 I/O MCU, appears to be functionally identical to original NS21
GAME( 1991, winrun91,  0,        winrun,   winrungp,   namcos21_state, empty_init,   ROT0,    "Namco", "Winning Run '91 (Japan) (1991/03/05, Main Ver 1.0, Sub Ver 1.0)",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN )
