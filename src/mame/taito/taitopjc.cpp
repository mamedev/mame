// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*
   Taito Power-JC System

   Preliminary driver

   Hardware appears sufficiently different to JC system to warrant
   a separate driver.

   TODO:
   - Inconsistent frame rate compared to PCB recordings; game has a tendency to freeze frame hiccup
   - Analog inputs update at a slower rate (about every 8-10 frames instead of every frame); this issue
     is also present in taitotz.cpp
   - Hook up remaining sound / Zoom hardware - see /src/sony/taito_zm.cpp for reference


PCB layout by Brian Troha, thanks to Luke Morse for photos & verification of parts.

Operation Tiger, (c)1998
POWER JC MOTHER-G PCB     K11X0870A  OPERATION TIGER
+----------------------------------------------------------------------------------------------------+
|  |-----------M Connector---------|    |-----------X Connector---------|                            |
|                                     +---------+                                                    |
|  QS32X245 x 3 ABT16245 x 3 E63-05   |D481850GF|   D4516161  D4516161                               |
|                                     +---------+                                                    |
|                                                                                                    |
|        +----------+                 +----------+                                                   |
|  D3N03 |          |                 |          |             IS61LV256                             |
|        | PowerPC  |                 | Taito    |   +------+                                        |
| MAX767 |  603E    |                 | TCG010PJC|   |QS5993|  IS61LV256                             |
|        | QFP240   |                 |  QFP240  |   +------+                 +--------+             |
|        +----------+                 +----------+             IS61LV256      |CXD1178Q|   AD813     |
|                                                                             +--------+             |
|                                           16.5000MHz                                               |
|                                                                                                    |
|  40.0000MHz                  TMS418160    +----------+                                            +-+
|                                           |          |        D4624456    +----------+            |C|
|  +----------+  +----------+  TMS418160    | Taito    |                    |          |            |o|
|  |          |  |          |               | TCO780PFA|        D4624456    |   Heat   |            |n|
|  | TMS320C53|  | IDT7024  |   E63-06      |  QFP240  |                    |   Sink   |            |n| Connects to
|  |      PQ80|  |  QFP84   |     QS32X245  +----------+                    |   for    |            |e| filter board
|  |    QFP116|  |          |   E63-07                                      |   AMP    |            |c|
|  +----------+  +----------+               +----------+        D4624456    |          |            |t|
|                                           |          |                    +----------+            |o|
|                              TMS418160    | Taito    |        D4624456                            |r|
|       IS61C256  E63-01                    | TCO780PFA|                    +----------+            +-+
|                              TMS418160    |  QFP240  |                    |          |             |
|                                           +----------+                    |   Heat   |             |
|       IS61C256  E63-02  ABT16245                                M514256   |   Sink   |             |
|                                          +---------+                      |   for    |             |
|       ABT16245   E63-03 E63-04           |NM1020819|         +-------+    |   AMP    |             |
|                                          +---------+         | ZFX-2 |    |          |             |
|  IDT7201 IDT7201              E63-08                         +-------+    +----------+             |
|                                                                                                    |
|                                   IS61C256                   +-------+    25.0000MHz               |
|                                                              | ZSG-2 |                             |
|                       ABT16245    IS61C256    ABT16245 x 2   +-------+    MB87078  6379   NJM2100  |
|                                                                                                    |
|  |-----------J Connector---------|    |-----------K Connector---------|                            |
+----------------------------------------------------------------------------------------------------+

OSC1 40.0000MHz
OSC2 16.5000MHz
OSC3 25.0000MHz

Main CPU:
   PowerPC-603E QFP240 (driven by IDT QS5993 Clock Driver @ 100MHz?)

Video:
   TMS320BC53PQ80 QFP116 (rated for 80MHz, driven by 40.0000MHz OSC or 40.0000MHz x 2 by IDT QS5993 Clock Driver?)
   Taito TCG010PJC QFP240
   Taito TCO780PFG QFP240 Polygon Renderer
   Sony CXD1178Q QFP48 8-bit High-Speed D/A converter for video band (RGB 3_channel input/output)

Sound:
   Panasonic MN1020819 QFP64 16-Bit Single Chip Microcomputer with 64K x 8 ROM, 3K x 8 RAM (Operational speed of 10MHz-20MHz, driven by 25.0000MHz OSC / 2)
   ZOOM ZFX-2 QFP100 DSP (Functionally identical to TMS57002, driven by 25.0000MHz OSC)
   ZOOM ZSG-2 QFP100 Sound PCM chip (driven by 25.0000MHz OSC)
   uDP6379 2-Channel 16-Bit D/A Audio Converter
   MB87078 6-Bit / 4-Channel Electric Volume Controller
   NJM2100 Dual Operational AMP

ROMs:
   E63-03 & E63-04 are AT27C512 EPROMs and are TMS320BC53PQ80 program ROMs

RAM:
   IDT7024 QFP84 4K x 16 Dual-Port SRAM
   IDT7201 PLCC32 512 x 9 First-In/First-Out Dual-Port Memory
   NEC D4516161AG5 1M x 8 x 2 16M-bit Synchronous DRAM (silkscreened MB81171622)
   NEC D481850GF-A12 QFP100 2 x 128K x 16 32-bit Synchronous Graphics RAM (silscreened NEC MB81G83222)
   IS61LV256-15J 32K x 8 Low Voltage CMOS SRAM  (silkscreened IS61LV256AH)
   TMS418160ADZ-60 1Mb x 16-Bit DRAM (Character Sprite RAM)
   IS61C256AH-15J 32K x 8 High-Speed CMOS SRAM
   NEC uDP482445GW-70 128K x 8 CMOS SRAM
   OKI M514256C-70J 256K x 4 Fast Page DRAM (silkscreened MSM514256)

Misc:
   IDT QS5993 PLCC32 PLL Clock Driver Turboclock, 6.25MHz-100MHz (silkscreened CY7B991)
   D3N03 Power MOSFET 3 Amps, 30 Volts
   Maxim MAX767CAP 5V to 3.3V Synchronous Step-Down Power-Supply Controller
   QS32X245 High-Speed CMOS QuickSwitch Double Width Bus Switch
   ABT16245 16-Bit Bus Transceiver with TriState Outputs
   J,K, M & X 3x32 pin connector connect to POWER JC DAUGHTER PCB (Male with KEY blocks)

PALs:
   E63-01 CE16C8H PAL
   E63-02 CE22C10H PAL
   E63-05 CE16C8H PAL
   E63-06 CE16C8H PAL
   E63-07 CE16C8H PAL
   E63-08 CE16C8H PAL


POWER JC DAUGHTER PCB     K91E0717B  OPERATION TIGER
+----------------------------------------------------------------------------------------------------+
|  |-----------J Connector---------|    |-----------K Connector---------|                            |
|                                                                                                    |
|   IDT74FCT                                                                                         |
|                                                                                                    |
|       IDT74FCT                                                                                     |
|                                                                      LC321664AM                    |
|                                                                                                    |
|   Taito E63-09       E63 17-1              Taito E63-23               E63 27-1                     |
|                                                                                                    |
|                                                                                                    |
|   Taito E63-10       E63 18-1              Taito E63-24    IDT74FCT   E63 28-1                     |
|                                                                                                    |
|                                                                                                    |
|   Taito E63-11        M66220               Taito E63-25     LC3564SM   RESET                       |
|                                                                                                   +-+
|                                                                                                   |C|
|   *                                        Taito E63-26     RTC64613A  C5                         |o|
|                                                                                                   |n|
|                                                                                                   |n| Connects to
|   E63 30-1       E63 32-1                                                             25.0000MHz  |e| filter board
|                                                                           +----------+            |c|
|                                                                           |Toshiba   |            |t|
|   E63 31-1       E63 33-1                                                 |TMP95C063F|            |o|
|                                                                           +----------+ 1.84320MHz |r|
|                                                                                                   +-+
|   Taito E63-15     Taito E63-21                                                           TLP121-4 |
|                                                                                                    |
|                                                                                                    |
|   Taito E63-16     Taito E63-22                                             +----------+  TLP121-4 |
|                                                                             |          |           |
|                                                                             | IDT7024  |           |
|                                                                             |  QFP84   |  TLP121-4 |
|   IDT74FCT      IDT74FCT IDT74FCT                                           |          |           |
|                                                                             +----------+           |
|                                                                                           TLP121-4 |
|                                                                                                    |
|                                                                                                    |
|  |-----------W Connector---------|    |-----------X Connector---------|                            |
+----------------------------------------------------------------------------------------------------+

*  Unpopulated socket for 4th polygon data ROM

OSC1 25.0000MHz
OSC2 1.84320MHz

CPU:
   Toshiba TMP95C063F 16-Bit CMOS Microcontroller (driven by 25.0000MHz OSC)

ROMs:
   8 EPROMs
    - E63 30-1 through E63 32-1 are PowerPC program ROMs
    - E63 17-1 & E63 18-1 are Panasonic MN1020819 program ROMs to drive Zoom chips / sound
    - E63 27-1 & E62 28-1 are Toshiba TMP95C063F program ROMs for I/O
   11 mask ROMs
    - 4 graphics ROMs (E63-15, E63-16, E63-21 & E63-22)
    - 4 Zoom ZSG-2 samples (E63-23 through E63-26)
    - 3 polygon data ROMs (E63-09 through E63-11)

RAM:
   IDT7024 QFP84 4K x 16 Dual-Port SRAM
   Sanyo LC3564BM-70 64K x 8 SRAM
   Sanyo LC321664AM-80 64K x 16 Fast Page DRAM
   M66220FP 256 x 8-bit Dual Port CMOS Mail Box RAM

MISC:
   Reset Push Button
   IDT 74 FCT 16245 CTPA 16-Bit Fast CMOS Bidirection Transceiver (silkscreened FTC16245C)
   Epson RTC64613A 8-Bit Real Time Clock Module with builtin crystal
   C5 is a Super-Cap, supplies back-up power to the RTC
   TLP121-4 Four Channel Photo Transistor, optically coupled to a gallium arsenide InfraRed emitting diode
   J,K, W & X 3x32 pin connector to connect to POWER JC MOTHER-G PCB (Female with KEY receivers)

Note:
 PCB found labeled as K91E0717B came with Operation Tiger Ver 2.14 O
 PCB found labeled as K91E0717A came with Operation Tiger Ver 2.10 O


POWER JC FILTER PCB
+----------------------------------------------------------------------------------------------------+
|                                                                                                    |
|                                                                                                    |
|                                          S......             ......V                               |
|                                           6    1             6    1                                |
|                                                                                *RS                 |
|                                                                                                    |
|                                            111112222233333444445                                   |
|                                        2468024680246802468024680                                   |
|                 P.........            |=========================|G                                 |
|                  1       9                  11111222223333344444                     *MIDI         |
|                                        1357913579135791357913579                                   |
+----------------------------------------------------------------------------------------------------+

*  Unpopulated components

V)ideo Connector, 6 pins    |    S)ound Connector, 6 pins
----------------------------+--------------------------------
   1] Red                   |       1] Sound R(-)
   2] Green                 |       2] Sound R(+)
   3] Blue                  |       3] Sound L(-)
   4] SYNC                  |       4] Sound L(+)
   5] GND                   |       5] W SND (-)  [Subwoofer]
   6] GND                   |       6] W SND (+)  [Subwoofer]


P)ower Connector, 9 pins
------------------------
   1] +12 Volts
   2] +13 Volts
   3] +5 Volts
   4] +5 Volts
   5] +5 Volts
   6] Ground
   7] Ground
   8] Ground
   9] Ground

G)ame Connector, dual row 50 pins
----------------------------------------------
   1] Meter                    2]
   3] Lockout                  4]
   5] 1P Solenoid A            6] 1P Solenoid B
   7] 2P Solenoid A            8] 2P Solenoid B
   9] 1P Start Lamp           10] 2P Start Lamp
  11]                         12]
  13]                         14]
  15]                         16]
  17]                         18]
  19] 1P X Position           20] 1P Y Position
  21] 2P X Position           22] 2P Y Position
  23]                         24]
  25]                         26]
  27]                         28] 1P Trigger
  29]                         30] 1P Bomb
  31]                         32] 2P Trigger
  33] Service                 34] 2P Bomb
  35] Test                    36]
  37]                         38] 1P Start
  39] 2P Start                40] Coin
  41]                         42] +12 Volts
  43] +12 Volts               44] +5 Volts
  45] +5 Volts                46] +5 Volts
  47] Ground                  48] Ground
  49] Ground                  50] Ground

*/

/*
    PPC -> TLCS Commands:
        0x5010:            ?                                        RTC?
        0x5020:            ?                                        RTC?
        0x6000:            ?                                        Backup RAM init?
        0x6010:            ?                                        Backup RAM Read. Address in io_shared[0x1d00].
        0x6020:            ?                                        Backup RAM Write. Address in io_shared[0x1d00].
        0x6030:            ?                                        ?
        0x6040:            ?                                        ?
        0x4000:            ?                                        Sound?
        0x4001:            ?
        0x4002:            ?
        0x4003:            ?
        0x4004:            ?
        0xf055:
        0xf0ff:
        0xf000:
        0xf001:
        0xf010:
        0xf020:

    TLCS -> PPC Commands:
        0x7000:                                                     DSP ready
        0xd000:                                                     Vblank

*/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/tlcs900/tmp95c063.h"
#include "cpu/mn10200/mn10200.h"
#include "cpu/tms32051/tms32051.h"
#include "tc0780fpa.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#define LOG_TLCS_TO_PPC_COMMANDS (1U << 1)
#define LOG_PPC_TO_TLCS_COMMANDS (1U << 2)
#define LOG_DISPLAY_LIST         (1U << 3)
#define LOG_VIDEO                (1U << 4)
#define LOG_IO                   (1U << 5)
#define LOG_DSP                  (1U << 6)
#define LOG_SOUND                (1U << 7)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGVIDEO(...)  LOGMASKED(LOG_VIDEO, __VA_ARGS__)
#define LOGIO(...)     LOGMASKED(LOG_IO, __VA_ARGS__)
#define LOGDSP(...)    LOGMASKED(LOG_DSP, __VA_ARGS__)
#define LOGSOUND(...)  LOGMASKED(LOG_SOUND, __VA_ARGS__)

namespace {

#define FATAL_UNKNOWN_CALLS (0)

class taitopjc_state : public driver_device
{
public:
	taitopjc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_iocpu(*this, "iocpu")
		, m_soundcpu(*this, "mn10200")
		, m_dsp(*this, "dsp")
		, m_tc0780fpa(*this, "tc0780fpa")
		, m_palette(*this, "palette")
		, m_polyrom(*this, "poly")
		, m_gfxdecode(*this, "gfxdecode")
		, m_main_ram(*this, "main_ram")
		, m_dsp_ram(*this, "dsp_ram")
		, m_io_share_ram(*this, "io_share_ram", 0x2000, ENDIANNESS_LITTLE) // or ENDIANNESS_BIG?
		, m_screen_ram(*this, "screen_ram", 0x40000*4, ENDIANNESS_BIG)
		, m_paletteram(*this, "paletteram", 0x8000*4, ENDIANNESS_BIG)
	{ }

	void taitopjc(machine_config &config);

	void init_optiger();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<ppc603e_device> m_maincpu;
	required_device<tmp95c063_device> m_iocpu;
	required_device<cpu_device> m_soundcpu;
	required_device<cpu_device> m_dsp;
	required_device<tc0780fpa_device> m_tc0780fpa;
	required_device<palette_device> m_palette;
	required_memory_region m_polyrom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint64_t> m_main_ram;
	required_shared_ptr<uint16_t> m_dsp_ram;
	memory_share_creator<uint16_t> m_io_share_ram;
	memory_share_creator<uint32_t> m_screen_ram;
	memory_share_creator<uint32_t> m_paletteram;

	uint64_t video_r(offs_t offset, uint64_t mem_mask = ~0);
	void video_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t ppc_common_r(offs_t offset, uint64_t mem_mask = ~0);
	void ppc_common_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t dsp_r(offs_t offset, uint64_t mem_mask = ~0);
	void dsp_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint8_t tlcs_common_r(offs_t offset);
	void tlcs_common_w(offs_t offset, uint8_t data);
	uint8_t tlcs_sound_r(offs_t offset);
	void tlcs_sound_w(offs_t offset, uint8_t data);
	void tlcs_unk_w(offs_t offset, uint16_t data);
	void tms_dspshare_w(offs_t offset, uint16_t data);
	uint16_t dsp_rom_r();
	void dsp_roml_w(uint16_t data);
	void dsp_romh_w(uint16_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vbi);
	uint32_t videochip_r(offs_t address);
	void videochip_w(offs_t address, uint32_t data);
	void video_exit();
	[[maybe_unused]] void print_display_list();
	template <unsigned Offset> TILE_GET_INFO_MEMBER(get_tile_info);

	tilemap_t *m_tilemap[2]{};

	uint32_t m_video_address = 0;

	uint32_t m_dsp_rom_address = 0;
	uint16_t m_scroll_x = 0;
	uint16_t m_scroll_y = 0;

	uint32_t m_tlcs_sound_ptr = 0;

	void mn10200_map(address_map &map) ATTR_COLD;
	void ppc603e_mem(address_map &map) ATTR_COLD;
	void tlcs900h_mem(address_map &map) ATTR_COLD;
	void tms_data_map(address_map &map) ATTR_COLD;
	void tms_io_map(address_map &map) ATTR_COLD;
	void tms_program_map(address_map &map) ATTR_COLD;
};

void taitopjc_state::video_exit()
{
#if 0
	FILE *file;
	int i;

	file = fopen("pjc_screen_ram.bin","wb");
	for (i=0; i < 0x40000; i++)
	{
		fputc((uint8_t)(m_screen_ram[i] >> 24), file);
		fputc((uint8_t)(m_screen_ram[i] >> 16), file);
		fputc((uint8_t)(m_screen_ram[i] >> 8), file);
		fputc((uint8_t)(m_screen_ram[i] >> 0), file);
	}
	fclose(file);

	file = fopen("pjc_pal_ram.bin","wb");
	for (i=0; i < 0x8000; i++)
	{
		fputc((uint8_t)(m_pal_ram[i] >> 24), file);
		fputc((uint8_t)(m_pal_ram[i] >> 16), file);
		fputc((uint8_t)(m_pal_ram[i] >> 8), file);
		fputc((uint8_t)(m_pal_ram[i] >> 0), file);
	}
	fclose(file);
#endif
}

template <unsigned Offset>
TILE_GET_INFO_MEMBER(taitopjc_state::get_tile_info)
{
	uint32_t val = m_screen_ram[Offset + (tile_index >> 1)];

	if (BIT(~tile_index, 0))
		val >>= 16;

	int const color = (val >> 12) & 0xf;
	int const tile = (val & 0xfff);

	tileinfo.set(0, tile, color, 0);
}

void taitopjc_state::video_start()
{
	static const gfx_layout char_layout =
	{
		16, 16,
		4096,
		8,
		{ 0,1,2,3,4,5,6,7 },
		{ 3*8, 2*8, 1*8, 0*8, 7*8, 6*8, 5*8, 4*8, 11*8, 10*8, 9*8, 8*8, 15*8, 14*8, 13*8, 12*8 },
		{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
		8*256
	};

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taitopjc_state::get_tile_info<0x3f000>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taitopjc_state::get_tile_info<0x3f800>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);

	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, char_layout, (uint8_t*)&m_screen_ram[0], 0, m_palette->entries() / 256, 0));

	m_palette->basemem().set(m_paletteram, m_paletteram.bytes(), 32, ENDIANNESS_BIG, 4);

	save_item(NAME(m_video_address));
	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));

	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&taitopjc_state::video_exit, this));
}

uint32_t taitopjc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_tc0780fpa->draw(bitmap, cliprect);

	m_tilemap[0]->set_scrollx(m_scroll_x);
	m_tilemap[0]->set_scrolly(m_scroll_y);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0);

	return 0;
}

uint32_t taitopjc_state::videochip_r( offs_t address)
{
	uint32_t r = 0;

	if (address >= 0x10000000 && address < 0x10040000)
	{
		r = m_screen_ram[address - 0x10000000];
	}

	return r;
}

void taitopjc_state::videochip_w(offs_t address, uint32_t data)
{
	if (address >= 0x20000000 && address < 0x20008000)
	{
		m_palette->write32(address - 0x20000000, data);
	}
	else if (address >= 0x10000000 && address < 0x10040000)
	{
		uint32_t const addr = address - 0x10000000;
		m_screen_ram[addr] = data;

		if (address >= 0x1003f000 && address < 0x10040000)
		{
			uint32_t const a = address & 0x7ff;
			m_tilemap[BIT(address, 11)]->mark_tile_dirty((a * 2) + 0);
			m_tilemap[BIT(address, 11)]->mark_tile_dirty((a * 2) + 1);
		}
		m_gfxdecode->gfx(0)->mark_dirty(addr / 64);
	}
	else if (address == 0x00000006)
	{
		m_scroll_y = (data >> 16) & 0xffff;
		m_scroll_x = data & 0xffff;
	}
	else
	{
		LOGVIDEO("Video Address %08X = %08X\n", address, data);
	}
}

uint64_t taitopjc_state::video_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t r = 0;

	if (offset == 0)
	{
		if (ACCESSING_BITS_32_63)
		{
			r |= (uint64_t)(videochip_r(m_video_address)) << 32;
		}
	}

	return r;
}

void taitopjc_state::video_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (offset == 0)
	{
		if (ACCESSING_BITS_32_63)
		{
			LOGVIDEO("Video Address %08X = %08X\n", m_video_address, (uint32_t)(data >> 32));
			videochip_w(m_video_address, (uint32_t)(data >> 32));
		}
	}
	if (offset == 1)
	{
		if (ACCESSING_BITS_32_63)
		{
			m_video_address = (uint32_t)(data >> 32);
		}
	}
}

uint64_t taitopjc_state::ppc_common_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t r = 0;

	LOGIO("ppc_common_r: %08X, %016X\n", offset, mem_mask);

	uint32_t const address = offset * 2;

	if (ACCESSING_BITS_48_63)
	{
		r |= uint64_t(m_io_share_ram[address]) << 48;
	}
	if (ACCESSING_BITS_16_31)
	{
		r |= uint64_t(m_io_share_ram[address + 1]) << 16;
	}

	return r;
}

void taitopjc_state::ppc_common_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	uint32_t const address = offset * 2;

	LOGIO("ppc_common_w: %08X, %X, %X\n", offset, data, mem_mask);

	if (ACCESSING_BITS_48_63)
	{
		m_io_share_ram[address] = (uint16_t)(data >> 48);
	}
	if (ACCESSING_BITS_16_31)
	{
		m_io_share_ram[address + 1] = (uint16_t)(data >> 16);
	}

	if (offset == 0x7ff && ACCESSING_BITS_48_63)
	{
		if (m_io_share_ram[0xfff] != 0x0000)
		{
			LOGMASKED(LOG_PPC_TO_TLCS_COMMANDS, "PPC -> TLCS cmd %04X\n", m_io_share_ram[0xfff]);
		}

		m_iocpu->set_input_line(TLCS900_INT6, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

		m_maincpu->abort_timeslice();
	}
}

uint64_t taitopjc_state::dsp_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t r = 0;

	if (ACCESSING_BITS_48_63)
	{
		int const addr = offset * 2;
		r |= (uint64_t)(m_dsp_ram[addr + 0]) << 48;
	}
	if (ACCESSING_BITS_16_31)
	{
		int const addr = offset * 2;
		r |= (uint64_t)(m_dsp_ram[addr + 1]) << 16;
	}

	return r;
}

void taitopjc_state::print_display_list()
{
	int ptr = 0;

	uint16_t const cmd = m_dsp_ram[0xffe];
	if (cmd == 0x5245)
	{
		logerror("DSP command RE\n");
		bool end = false;
		do
		{
			uint16_t const w = m_dsp_ram[ptr++];
			if (BIT(w, 15))
			{
				int const count = (w & 0x7fff) + 1;
				uint16_t d = m_dsp_ram[ptr++];
				for (int i = 0; i < count; i++)
				{
					uint16_t const s = m_dsp_ram[ptr++];
					logerror("   %04X -> [%04X]\n", s, d);
					d++;
				}
			}
			else if (w == 0)
			{
				end = true;
			}
			else
			{
				switch (w)
				{
					case 0x406d:
						logerror("   Call %04X [%04X %04X]\n", w, m_dsp_ram[ptr], m_dsp_ram[ptr+1]);
						ptr += 2;
						break;
					case 0x40cd:
						logerror("   Call %04X [%04X %04X]\n", w, m_dsp_ram[ptr], m_dsp_ram[ptr+1]);
						ptr += 2;
						break;
					case 0x40ac:
						logerror("   Call %04X [%04X %04X %04X %04X %04X %04X %04X %04X]\n", w, m_dsp_ram[ptr], m_dsp_ram[ptr+1], m_dsp_ram[ptr+2], m_dsp_ram[ptr+3], m_dsp_ram[ptr+4], m_dsp_ram[ptr+5], m_dsp_ram[ptr+6], m_dsp_ram[ptr+7]);
						ptr += 8;
						break;
					case 0x4774:
						logerror("   Call %04X [%04X %04X %04X]\n", w, m_dsp_ram[ptr], m_dsp_ram[ptr+1], m_dsp_ram[ptr+2]);
						ptr += 3;
						break;
					case 0x47d9:
						logerror("   Call %04X [%04X %04X %04X %04X %04X %04X %04X %04X]\n", w, m_dsp_ram[ptr], m_dsp_ram[ptr+1], m_dsp_ram[ptr+2], m_dsp_ram[ptr+3], m_dsp_ram[ptr+4], m_dsp_ram[ptr+5], m_dsp_ram[ptr+6], m_dsp_ram[ptr+7]);
						ptr += 8;
						break;
					default:
					{
						logerror("Unknown call %04X\n", w);
						for (int i = 0; i < 10; i++)
						{
							logerror("%04X\n", m_dsp_ram[ptr++]);
						}
						if (FATAL_UNKNOWN_CALLS)
							fatalerror("Unknown call %04X\n", w);
						break;
					}
				}
			}
		} while(!end);
	}
	else
	{
		if (cmd != 0)
			logerror("DSP command %04X\n", cmd);
		return;
	}
}

void taitopjc_state::dsp_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	LOGDSP("dsp_w: %08X, %08X%08X, %08X%08X at %08X\n", offset, (uint32_t)(data >> 32), (uint32_t)(data), (uint32_t)(mem_mask >> 32), (uint32_t)(mem_mask), m_maincpu->pc());

	if (offset == 0x7fe)
	{
		#if 0
		{
			FILE *f = fopen("dspram.bin", "wb");
			for (int i = 0; i < 0x1000; i++)
			{
				fputc((dsp_ram[i] >> 0) & 0xff, f);
				fputc((dsp_ram[i] >> 8) & 0xff, f);
			}
			fclose(f);
		}
		#endif

#if (VERBOSE & LOG_DISPLAY_LIST)
		print_display_list();
#endif
	}

	if (ACCESSING_BITS_48_63)
	{
		int const addr = offset * 2;
		m_dsp_ram[addr + 0] = (data >> 48) & 0xffff;
	}
	if (ACCESSING_BITS_16_31)
	{
		int const addr = offset * 2;
		m_dsp_ram[addr + 1] = (data >> 16) & 0xffff;
	}
}

// BAT Config:
// IBAT0 U: 0x40000002   L: 0x40000022      (0x40000000...0x4001ffff)
// IBAT1 U: 0x0000007f   L: 0x00000002      (0x00000000...0x003fffff)
// IBAT2 U: 0xc0000003   L: 0xc0000022      (0xc0000000...0xc001ffff)
// IBAT3 U: 0xfe0003ff   L: 0xfe000022      (0xfe000000...0xffffffff)
// DBAT0 U: 0x40000002   L: 0x40000022      (0x40000000...0x4001ffff)
// DBAT1 U: 0x0000007f   L: 0x00000002      (0x00000000...0x003fffff)
// DBAT2 U: 0xc0000003   L: 0xc0000022      (0xc0000000...0xc001ffff)
// DBAT3 U: 0xfe0003ff   L: 0xfe000022      (0xfe000000...0xffffffff)

void taitopjc_state::ppc603e_mem(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share(m_main_ram); // Work RAM
	map(0x40000000, 0x4000000f).rw(FUNC(taitopjc_state::video_r), FUNC(taitopjc_state::video_w));
	map(0x80000000, 0x80003fff).rw(FUNC(taitopjc_state::dsp_r), FUNC(taitopjc_state::dsp_w));
	map(0xc0000000, 0xc0003fff).rw(FUNC(taitopjc_state::ppc_common_r), FUNC(taitopjc_state::ppc_common_w));
	map(0xfe800000, 0xff7fffff).rom().region("maingfx", 0);
	map(0xffe00000, 0xffffffff).rom().region("maindata", 0);
}




uint8_t taitopjc_state::tlcs_common_r(offs_t offset)
{
	if (BIT(offset, 0))
	{
		return (uint8_t)(m_io_share_ram[offset / 2] >> 8);
	}
	else
	{
		return (uint8_t)(m_io_share_ram[offset / 2]);
	}
}

void taitopjc_state::tlcs_common_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
	{
		m_io_share_ram[offset / 2] &= 0x00ff;
		m_io_share_ram[offset / 2] |= (uint16_t)(data) << 8;
	}
	else
	{
		m_io_share_ram[offset / 2] &= 0xff00;
		m_io_share_ram[offset / 2] |= data;
	}

	if (offset == 0x1fff)
	{
		m_iocpu->set_input_line(TLCS900_INT6, CLEAR_LINE);
	}

	if (offset == 0x1ffd)
	{
		if (m_io_share_ram[0xffe] != 0xd000 &&
			m_io_share_ram[0xffe] != 0x7000)
		{
			LOGMASKED(LOG_TLCS_TO_PPC_COMMANDS, "TLCS -> PPC cmd %04X\n", m_io_share_ram[0xffe]);
		}

		if (m_io_share_ram[0xffe] == 0xd000)
			m_iocpu->set_input_line(TLCS900_INT1, CLEAR_LINE);
		if (m_io_share_ram[0xffe] == 0x7000)
			m_iocpu->set_input_line(TLCS900_INT2, CLEAR_LINE);

		if (m_io_share_ram[0xffe] != 0)
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);

		m_iocpu->abort_timeslice();
	}
}

uint8_t taitopjc_state::tlcs_sound_r(offs_t offset)
{
	if (offset == 0x15)
	{
		return m_tlcs_sound_ptr & 0x7f;
	}
	else if (offset == 0x17)
	{
		return 0x55;
	}
	else if (offset >= 0x80 && offset < 0x100)
	{
		if (!machine().side_effects_disabled())
			m_tlcs_sound_ptr++;
	}

	return 0;
}

void taitopjc_state::tlcs_sound_w(offs_t offset, uint8_t data)
{
	LOGSOUND("tlcs_sound_w: %08X, %02X\n", offset, data);
}

void taitopjc_state::tlcs_unk_w(offs_t offset, uint16_t data)
{
	if (offset == 0xc/2)
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, BIT(data, 2));
	}
}

// TLCS900 interrupt vectors
// 0xfc0100: reset
// 0xfc00ea: INT0 (dummy)
// 0xfc00eb: INT1 vblank?
// 0xfc00f0: INT2 DSP ready?
// 0xfc00f5: INT3 (dummy)
// 0xfc00f6: INT4 (dummy)
// 0xfc00f7: INT5 (dummy)
// 0xfc00f8: INT6 PPC command
// 0xfc00fd: INT7 (dummy)
// 0xfc00fe: int8_t (dummy)
// 0xfc0663: INTT1
// 0xfc0f7d: INTRX0
// 0xfc0f05: INTTX0
// 0xfc0fb5: INTRX1
// 0xfc0f41: INTTX1

void taitopjc_state::tlcs900h_mem(address_map &map)
{
	map(0x010000, 0x02ffff).ram(); // Work RAM
	map(0x040000, 0x0400ff).rw(FUNC(taitopjc_state::tlcs_sound_r), FUNC(taitopjc_state::tlcs_sound_w));
	map(0x044000, 0x045fff).ram().share("nvram");
	map(0x060000, 0x061fff).rw(FUNC(taitopjc_state::tlcs_common_r), FUNC(taitopjc_state::tlcs_common_w));
	map(0x06c000, 0x06c00f).w(FUNC(taitopjc_state::tlcs_unk_w));
	map(0xfc0000, 0xffffff).rom().region("iocpu", 0);
}

void taitopjc_state::mn10200_map(address_map &map)
{
	map(0x080000, 0x0fffff).rom().region("mn10200", 0);
}



void taitopjc_state::tms_dspshare_w(offs_t offset, uint16_t data)
{
	if (offset == 0xffc)
	{
		m_iocpu->set_input_line(TLCS900_INT2, ASSERT_LINE);
	}
	m_dsp_ram[offset] = data;
}

uint16_t taitopjc_state::dsp_rom_r()
{
	assert(m_dsp_rom_address < 0x800000);

	uint16_t const data = ((uint16_t*)m_polyrom->base())[m_dsp_rom_address];
	if (!machine().side_effects_disabled())
		m_dsp_rom_address++;
	return data;
}

void taitopjc_state::dsp_roml_w(uint16_t data)
{
	m_dsp_rom_address &= 0xffff0000;
	m_dsp_rom_address |= data;
}

void taitopjc_state::dsp_romh_w(uint16_t data)
{
	m_dsp_rom_address &= 0xffff;
	m_dsp_rom_address |= (uint32_t)(data) << 16;
}


void taitopjc_state::tms_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("dspdata", 0);
	map(0x4c00, 0xefff).rom().region("dspdata", 0x9800);
}

void taitopjc_state::tms_data_map(address_map &map)
{
	map(0x4000, 0x6fff).rom().region("dspdata", 0x8000);
	map(0x7000, 0xefff).ram();
	map(0xf000, 0xffff).ram().w(FUNC(taitopjc_state::tms_dspshare_w)).share(m_dsp_ram);
}

void taitopjc_state::tms_io_map(address_map &map)
{
	map(0x0053, 0x0053).w(FUNC(taitopjc_state::dsp_roml_w));
	map(0x0057, 0x0057).w(FUNC(taitopjc_state::dsp_romh_w));
	map(0x0058, 0x0058).w(m_tc0780fpa, FUNC(tc0780fpa_device::poly_fifo_w));
	map(0x005a, 0x005a).w(m_tc0780fpa, FUNC(tc0780fpa_device::tex_w));
	map(0x005b, 0x005b).rw(m_tc0780fpa, FUNC(tc0780fpa_device::tex_addr_r), FUNC(tc0780fpa_device::tex_addr_w));
	map(0x005e, 0x005e).noprw(); // ?? 0x0001 written every frame
	map(0x005f, 0x005f).r(FUNC(taitopjc_state::dsp_rom_r));
}


static INPUT_PORTS_START( taitopjc )
	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_COIN1 ) // Coin A
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUTS2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7) // Service switch
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW) // Test Button
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START1 ) // Select 1
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_START2 ) // Select 2
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUTS3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // P1 trigger
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // P1 bomb
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // P2 trigger
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // P2 bomb
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANALOG1") // Player 1 X
	PORT_BIT(0x3ff, 0x200, IPT_AD_STICK_X) PORT_CROSSHAIR(X, -1.0, 0.0, 0) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1) PORT_REVERSE

	PORT_START("ANALOG2") // Player 1 Y
	PORT_BIT(0x3ff, 0x200, IPT_AD_STICK_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ANALOG3") // Player 2 X
	PORT_BIT(0x3ff, 0x200, IPT_AD_STICK_X) PORT_CROSSHAIR(X, -1.0, 0.0, 0) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2) PORT_REVERSE

	PORT_START("ANALOG4") // Player 2 Y
	PORT_BIT(0x3ff, 0x200, IPT_AD_STICK_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


void taitopjc_state::machine_start()
{
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x003fffff, false, m_main_ram);

	save_item(NAME(m_dsp_rom_address));
	save_item(NAME(m_tlcs_sound_ptr));
}

void taitopjc_state::machine_reset()
{
	// halt sound CPU since we don't emulate this yet
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_dsp_rom_address = 0;
}


INTERRUPT_GEN_MEMBER(taitopjc_state::vbi)
{
	m_iocpu->set_input_line(TLCS900_INT1, ASSERT_LINE);
}


void taitopjc_state::taitopjc(machine_config &config)
{
	PPC603E(config, m_maincpu, 100000000);
	m_maincpu->set_bus_frequency(XTAL(66'666'700)); // Multiplier 1.5, Bus = 66MHz, Core = 100MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &taitopjc_state::ppc603e_mem);

	// TMP95C063F I/O CPU
	TMP95C063(config, m_iocpu, 25_MHz_XTAL);
	m_iocpu->port5_read().set_ioport("INPUTS1");
	m_iocpu->portd_read().set_ioport("INPUTS2");
	m_iocpu->porte_read().set_ioport("INPUTS3");
	m_iocpu->an_read<0>().set_ioport("ANALOG1");
	m_iocpu->an_read<1>().set_ioport("ANALOG2");
	m_iocpu->an_read<2>().set_ioport("ANALOG3");
	m_iocpu->an_read<3>().set_ioport("ANALOG4");
	m_iocpu->set_addrmap(AS_PROGRAM, &taitopjc_state::tlcs900h_mem);
	m_iocpu->set_vblank_int("screen", FUNC(taitopjc_state::vbi));

	// TMS320C53 DSP
	TMS32053(config, m_dsp, 40_MHz_XTAL); // 80MHz rated part, should be 40.0000MHz x 2?
	m_dsp->set_addrmap(AS_PROGRAM, &taitopjc_state::tms_program_map);
	m_dsp->set_addrmap(AS_DATA, &taitopjc_state::tms_data_map);
	m_dsp->set_addrmap(AS_IO, &taitopjc_state::tms_io_map);

	// MN1020819DA sound CPU
	MN1020012A(config, m_soundcpu, 25_MHz_XTAL/2); // clock divisor unverified - NOTE: May have 64kB internal ROM
	m_soundcpu->set_addrmap(AS_PROGRAM, &taitopjc_state::mn10200_map);

	config.set_maximum_quantum(attotime::from_hz(200000));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(480, 384);
	screen.set_visarea(0, 479, 0, 383);
	screen.set_screen_update(FUNC(taitopjc_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 32768);
	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	TC0780FPA(config, m_tc0780fpa, 0);
}


void taitopjc_state::init_optiger()
{
	uint8_t *rom = (uint8_t*)memregion("iocpu")->base();

	// skip sound check
	rom[BYTE_XOR_LE(0x217)] = 0x00;
	rom[BYTE_XOR_LE(0x218)] = 0x00;

#if 0
	uint32_t *mr = (uint32_t*)memregion("maindata")->base();
	//mr[(0x23a5c^4)/4] = 0x60000000;
	mr[((0x513b0-0x40000)^4)/4] = 0x38600001;
#endif
}


ROM_START( optiger ) // Ver 2.14 O
	ROM_REGION64_BE( 0x200000, "maindata", 0 )
	ROM_LOAD32_BYTE( "e63_33-1.ic23", 0x000000, 0x080000, CRC(5ab176e2) SHA1(a0a5b7c0e91928d0a49987f88f6ae647f5cb3e34) ) // PCB silkscreened IC23 (P-HH)
	ROM_LOAD32_BYTE( "e63_32-1.ic22", 0x000001, 0x080000, CRC(cca8bacc) SHA1(e5a081f5c12a52601745f5b67fe3412033581b00) ) // PCB silkscreened IC22 (P-HL)
	ROM_LOAD32_BYTE( "e63_31-1.ic8",  0x000002, 0x080000, CRC(ad69e649) SHA1(9fc853d2cb6e7cac87dc06bad91048f191b799c5) ) // PCB silkscreened IC8 (P-LH)
	ROM_LOAD32_BYTE( "e63_30-1.ic7",  0x000003, 0x080000, CRC(a6183479) SHA1(e556c3edf100342079e680ec666f018fca7a82b0) ) // PCB silkscreened IC7 (P-LL)

	ROM_REGION( 0x8000, "dsp", 0 ) // surface mounted on K11X0870A POWER JC MOTHER-G PCB
	ROM_LOAD( "tms320bc53pq80.ic8", 0x0000, 0x8000, CRC(4b8e7fd6) SHA1(07d354a2e4d7554e215fa8d91b5eeeaf573766b0) ) // decapped. TODO: believed to be a generic TI part, verify if it is and if dump is good, if so move in the CPU core

	ROM_REGION16_LE( 0x20000, "dspdata", 0 )
	ROM_LOAD16_BYTE( "taito_e63_04.ic29", 0x000000, 0x010000, CRC(eccae391) SHA1(e5293c16342cace54dc4b6dfb827558e18ac25a4) ) // PCB silkscreened IC29 (L)     AT27C512
	ROM_LOAD16_BYTE( "taito_e63_03.ic28", 0x000001, 0x010000, CRC(58fce52f) SHA1(1e3d9ee034b25e658ca45a8b900de2aa54b00135) ) // PCB silkscreened IC28 (H)     AT27C512

	ROM_REGION( 0x40000, "iocpu", 0 )
	ROM_LOAD16_BYTE( "e63_28-1.ic59", 0x000000, 0x020000, CRC(ef41ffaf) SHA1(419621f354f548180d37961b861304c469e43a65) ) // PCB silkscreened IC59 (0)     27C1001
	ROM_LOAD16_BYTE( "e63_27-1.ic58", 0x000001, 0x020000, CRC(facc17a7) SHA1(40d69840cfcfe5a509d69824c2994de56a3c6ece) ) // PCB silkscreened IC58 (1)     27C1001

	ROM_REGION( 0x80000, "mn10200", 0 ) // sound CPU to drive Zoom ZSG-2/Zoom ZFX-2
	ROM_LOAD16_BYTE( "e63_17-1.ic18", 0x000000, 0x040000, CRC(2a063d5b) SHA1(a2b2fe4d8bad1aef7d9dcc0be607cc4e5bc4f0eb) ) // PCB silkscreened IC18 (S-L)    27C2001
	ROM_LOAD16_BYTE( "e63_18-1.ic19", 0x000001, 0x040000, CRC(2f590881) SHA1(7fb827a676f45b24380558b0068b76cb858314f6) ) // PCB silkscreened IC19 (S-H)    27C2001

	ROM_REGION64_BE( 0x1000000, "maingfx", 0 ) // mask ROMs
	ROM_LOAD32_WORD_SWAP( "e63-21.ic24", 0x000000, 0x400000, CRC(c818b211) SHA1(dce07bfe71a9ba11c3f028a640226c6e59c6aece) ) // PCB silkscreened IC24 (C-H)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-15.ic9",  0x000002, 0x400000, CRC(4ec6a2d7) SHA1(2ee6270cff7ea2459121961a29d42e000cee2921) ) // PCB silkscreened IC9 (C-L)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-22.ic25", 0x800000, 0x400000, CRC(6d895eb6) SHA1(473795da42fd29841a926f18a93e5992f4feb27c) ) // PCB silkscreened IC25 (M-H)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-16.ic10", 0x800002, 0x400000, CRC(d39c1e34) SHA1(6db0ce2251841db3518a9bd9c4520c3c666d19a0) ) // PCB silkscreened IC10 (M-L)    23C32000

	ROM_REGION16_BE( 0x1000000, "poly", ROMREGION_ERASEFF ) // mask ROMs
	ROM_LOAD16_WORD_SWAP( "e63-09.ic3", 0x000000, 0x400000, CRC(c3e2b1e0) SHA1(ee71f3f59b46e26dbe2ff724da2c509267c8bf2f) ) // PCB silkscreened IC3 (POLY0)    23C32000
	ROM_LOAD16_WORD_SWAP( "e63-10.ic4", 0x400000, 0x400000, CRC(f4a56390) SHA1(fc3c51a7f4639479e66ad50dcc94255d94803c97) ) // PCB silkscreened IC4 (POLY1)    23C32000
	ROM_LOAD16_WORD_SWAP( "e63-11.ic5", 0x800000, 0x400000, CRC(2293d9f8) SHA1(16adaa0523168ee63a7a34b29622c623558fdd82) ) // PCB silkscreened IC5 (POLY2)    23C32000
	// IC6 (POLY3) is not populated

	ROM_REGION( 0x800000, "sound_data", 0 ) // mask ROMs - Zoom ZSG-2 samples
	ROM_LOAD( "e63-23.ic36", 0x000000, 0x200000, CRC(d69e196e) SHA1(f738bb9e1330f6dabb5e0f0378a1a8eb48a4fa40) ) // PCB silkscreened IC36 (WD0)    23C16000
	ROM_LOAD( "e63-24.ic37", 0x200000, 0x200000, CRC(cd55f17b) SHA1(08f847ef2fd592dbaf63ef9e370cdf1f42012f74) ) // PCB silkscreened IC37 (WD1)    23C16000
	ROM_LOAD( "e63-25.ic38", 0x400000, 0x200000, CRC(bd35bdac) SHA1(5cde6c1a6b74659507b31fcb88257e65f230bfe2) ) // PCB silkscreened IC38 (WD2)    23C16000
	ROM_LOAD( "e63-26.ic39", 0x600000, 0x200000, CRC(346bd413) SHA1(0f6081d22db88eef08180278e7ae97283b5e8452) ) // PCB silkscreened IC39 (WD3)    23C16000

	ROM_REGION( 0x850, "plds", 0 )
	ROM_LOAD( "e63-01_palce16v8h-5-5.ic23",  0x000, 0x117, CRC(f114c13f) SHA1(ca9ec41d5c16347bdf107b340e6e1b9e6b7c74a9) )
	ROM_LOAD( "e63-02_palce22v10h-5-5.ic25", 0x117, 0x2dd, CRC(8418da84) SHA1(b235761f78ecb16d764fbefb00d04092d3a22ca9) )
	ROM_LOAD( "e63-05_palce16v8h-10-4.ic36", 0x3f4, 0x117, CRC(e27e9734) SHA1(77dadfbedb625b65617640bb73c59c9e5b0c927f) )
	ROM_LOAD( "e63-06_palce16v8h-10-4.ic41", 0x50b, 0x117, CRC(75184422) SHA1(d35e98e0278d713139eb1c833f41f57ed0dd3c9f) )
	ROM_LOAD( "e63-07_palce16v8h-10-4.ic43", 0x622, 0x117, CRC(eb77b03f) SHA1(567f92a4fd1fa919d5e9047ee15c058bf40855fb) )
	ROM_LOAD( "e63-08_palce16v8h-15-4.ic49", 0x739, 0x117, CRC(c305c56d) SHA1(49592fa43c548ac6b08951d03677a3f23e9c8de8) )
ROM_END

ROM_START( optigera ) // Ver 2.10 O
	ROM_REGION64_BE( 0x200000, "maindata", 0 )
	ROM_LOAD32_BYTE( "e63_33.ic23", 0x000000, 0x080000, CRC(414a7c77) SHA1(d4bbaa13244f1e5f4d418354f40303b9bcc00411) ) // PCB silkscreened IC23 (P-HH)
	ROM_LOAD32_BYTE( "e63_32.ic22", 0x000001, 0x080000, CRC(8fec33e8) SHA1(1eb0c5613937cd63dc2f54efa33c98920c55f251) ) // PCB silkscreened IC22 (P-HL)
	ROM_LOAD32_BYTE( "e63_31.ic8",  0x000002, 0x080000, CRC(672f9d4f) SHA1(7eb79963a5d4fb504ffbcf3f51c9bdf659ae053b) ) // PCB silkscreened IC8 (P-LH)
	ROM_LOAD32_BYTE( "e63_30.ic7",  0x000003, 0x080000, CRC(b5a63d08) SHA1(c12dd21008fb58179413934a5952ebd2d2040111) ) // PCB silkscreened IC7 (P-LL)

	ROM_REGION( 0x8000, "dsp", 0 ) // surface mounted on K11X0870A POWER JC MOTHER-G PCB
	ROM_LOAD( "tms320bc53pq80.ic8", 0x0000, 0x8000, CRC(4b8e7fd6) SHA1(07d354a2e4d7554e215fa8d91b5eeeaf573766b0) ) // decapped. TODO: believed to be a generic TI part, verify if it is and if dump is good, if so move in the CPU core

	ROM_REGION16_LE( 0x20000, "dspdata", 0 )
	ROM_LOAD16_BYTE( "taito_e63_04.ic29", 0x000000, 0x010000, CRC(eccae391) SHA1(e5293c16342cace54dc4b6dfb827558e18ac25a4) ) // PCB silkscreened IC29 (L)     AT27C512
	ROM_LOAD16_BYTE( "taito_e63_03.ic28", 0x000001, 0x010000, CRC(58fce52f) SHA1(1e3d9ee034b25e658ca45a8b900de2aa54b00135) ) // PCB silkscreened IC28 (H)     AT27C512

	ROM_REGION( 0x40000, "iocpu", 0 )
	ROM_LOAD16_BYTE( "e63_28.ic59", 0x000000, 0x020000, CRC(601dc916) SHA1(49b1629c4b5a5482c932ebd69b46b40489118012) ) // PCB silkscreened IC59 (0)     27C1001
	ROM_LOAD16_BYTE( "e63_27.ic58", 0x000001, 0x020000, CRC(930d899f) SHA1(fef194020c8c8b5906a6f2a954a1a0312d970f3d) ) // PCB silkscreened IC58 (1)     27C1001

	ROM_REGION( 0x80000, "mn10200", 0 ) // sound CPU to drive Zoom ZSG-2/Zoom ZFX-2
	ROM_LOAD16_BYTE( "e63_17.ic18", 0x000000, 0x040000, CRC(daac9e43) SHA1(9ef779a9a5e991ffcfcf30e94ef75329c1030fc2) ) // PCB silkscreened IC18 (S-L)    27C2001
	ROM_LOAD16_BYTE( "e63_18.ic19", 0x000001, 0x040000, CRC(69c97004) SHA1(65dc3dee0eb7faa1422c38947510abaeb23da7e3) ) // PCB silkscreened IC19 (S-H)    27C2001

	ROM_REGION64_BE( 0x1000000, "maingfx", 0 ) // mask ROMs
	ROM_LOAD32_WORD_SWAP( "e63-21.ic24", 0x000000, 0x400000, CRC(c818b211) SHA1(dce07bfe71a9ba11c3f028a640226c6e59c6aece) ) // PCB silkscreened IC24 (C-H)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-15.ic9",  0x000002, 0x400000, CRC(4ec6a2d7) SHA1(2ee6270cff7ea2459121961a29d42e000cee2921) ) // PCB silkscreened IC9 (C-L)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-22.ic25", 0x800000, 0x400000, CRC(6d895eb6) SHA1(473795da42fd29841a926f18a93e5992f4feb27c) ) // PCB silkscreened IC25 (M-H)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-16.ic10", 0x800002, 0x400000, CRC(d39c1e34) SHA1(6db0ce2251841db3518a9bd9c4520c3c666d19a0) ) // PCB silkscreened IC10 (M-L)    23C32000

	ROM_REGION16_BE( 0x1000000, "poly", ROMREGION_ERASEFF ) // mask ROMs
	ROM_LOAD16_WORD_SWAP( "e63-09.ic3", 0x000000, 0x400000, CRC(c3e2b1e0) SHA1(ee71f3f59b46e26dbe2ff724da2c509267c8bf2f) ) // PCB silkscreened IC3 (POLY0)    23C32000
	ROM_LOAD16_WORD_SWAP( "e63-10.ic4", 0x400000, 0x400000, CRC(f4a56390) SHA1(fc3c51a7f4639479e66ad50dcc94255d94803c97) ) // PCB silkscreened IC4 (POLY1)    23C32000
	ROM_LOAD16_WORD_SWAP( "e63-11.ic5", 0x800000, 0x400000, CRC(2293d9f8) SHA1(16adaa0523168ee63a7a34b29622c623558fdd82) ) // PCB silkscreened IC5 (POLY2)    23C32000
	// IC6 (POLY3) is not populated

	ROM_REGION( 0x800000, "sound_data", 0 ) // mask ROMs - Zoom ZSG-2 samples
	ROM_LOAD( "e63-23.ic36", 0x000000, 0x200000, CRC(d69e196e) SHA1(f738bb9e1330f6dabb5e0f0378a1a8eb48a4fa40) ) // PCB silkscreened IC36 (WD0)    23C16000
	ROM_LOAD( "e63-24.ic37", 0x200000, 0x200000, CRC(cd55f17b) SHA1(08f847ef2fd592dbaf63ef9e370cdf1f42012f74) ) // PCB silkscreened IC37 (WD1)    23C16000
	ROM_LOAD( "e63-25.ic38", 0x400000, 0x200000, CRC(bd35bdac) SHA1(5cde6c1a6b74659507b31fcb88257e65f230bfe2) ) // PCB silkscreened IC38 (WD2)    23C16000
	ROM_LOAD( "e63-26.ic39", 0x600000, 0x200000, CRC(346bd413) SHA1(0f6081d22db88eef08180278e7ae97283b5e8452) ) // PCB silkscreened IC39 (WD3)    23C16000

	ROM_REGION( 0x850, "plds", 0 )
	ROM_LOAD( "e63-01_palce16v8h-5-5.ic23",  0x000, 0x117, CRC(f114c13f) SHA1(ca9ec41d5c16347bdf107b340e6e1b9e6b7c74a9) )
	ROM_LOAD( "e63-02_palce22v10h-5-5.ic25", 0x117, 0x2dd, CRC(8418da84) SHA1(b235761f78ecb16d764fbefb00d04092d3a22ca9) )
	ROM_LOAD( "e63-05_palce16v8h-10-4.ic36", 0x3f4, 0x117, CRC(e27e9734) SHA1(77dadfbedb625b65617640bb73c59c9e5b0c927f) )
	ROM_LOAD( "e63-06_palce16v8h-10-4.ic41", 0x50b, 0x117, CRC(75184422) SHA1(d35e98e0278d713139eb1c833f41f57ed0dd3c9f) )
	ROM_LOAD( "e63-07_palce16v8h-10-4.ic43", 0x622, 0x117, CRC(eb77b03f) SHA1(567f92a4fd1fa919d5e9047ee15c058bf40855fb) )
	ROM_LOAD( "e63-08_palce16v8h-15-4.ic49", 0x739, 0x117, CRC(c305c56d) SHA1(49592fa43c548ac6b08951d03677a3f23e9c8de8) )
ROM_END

ROM_START( optigerj ) // ver 2.09 J
	ROM_REGION64_BE( 0x200000, "maindata", 0 )
	ROM_LOAD32_BYTE( "e63_20.ic23", 0x000000, 0x080000, CRC(04b9820b) SHA1(7a5bd7fd7003948b57d862dc1ecd38ddd25a4ca2) ) // PCB silkscreened IC23 (P-HH)
	ROM_LOAD32_BYTE( "e63_19.ic22", 0x000001, 0x080000, CRC(a25ff024) SHA1(46e8023a028a384609177d00a47cdfdbbda100be) ) // PCB silkscreened IC22 (P-HL)
	ROM_LOAD32_BYTE( "e63_14.ic8",  0x000002, 0x080000, CRC(2e68fc12) SHA1(a8ee3e51ad8eb1477db8f67380261cee1dada104) ) // PCB silkscreened IC8 (P-LH)
	ROM_LOAD32_BYTE( "e63_13.ic7",  0x000003, 0x080000, CRC(d1c877e6) SHA1(05314984b7fbc21478b50f7b1281d1f3745496d4) ) // PCB silkscreened IC7 (P-LL)

	ROM_REGION( 0x8000, "dsp", 0 ) // surface mounted on K11X0870A POWER JC MOTHER-G PCB
	ROM_LOAD( "tms320bc53pq80.ic8", 0x0000, 0x8000, CRC(4b8e7fd6) SHA1(07d354a2e4d7554e215fa8d91b5eeeaf573766b0) ) // decapped. TODO: believed to be a generic TI part, verify if it is and if dump is good, if so move in the CPU core

	ROM_REGION16_LE( 0x20000, "dspdata", 0 )
	ROM_LOAD16_BYTE( "taito_e63_04.ic29", 0x000000, 0x010000, CRC(eccae391) SHA1(e5293c16342cace54dc4b6dfb827558e18ac25a4) ) // PCB silkscreened IC29 (L)     AT27C512
	ROM_LOAD16_BYTE( "taito_e63_03.ic28", 0x000001, 0x010000, CRC(58fce52f) SHA1(1e3d9ee034b25e658ca45a8b900de2aa54b00135) ) // PCB silkscreened IC28 (H)     AT27C512

	ROM_REGION( 0x40000, "iocpu", 0 )
	ROM_LOAD16_BYTE( "e63_28.ic59", 0x000000, 0x020000, CRC(601dc916) SHA1(49b1629c4b5a5482c932ebd69b46b40489118012) ) // PCB silkscreened IC59 (0)     27C1001
	ROM_LOAD16_BYTE( "e63_27.ic58", 0x000001, 0x020000, CRC(930d899f) SHA1(fef194020c8c8b5906a6f2a954a1a0312d970f3d) ) // PCB silkscreened IC58 (1)     27C1001

	ROM_REGION( 0x80000, "mn10200", 0 ) // sound CPU to drive Zoom ZSG-2/Zoom ZFX-2
	ROM_LOAD16_BYTE( "e63_17.ic18", 0x000000, 0x040000, CRC(daac9e43) SHA1(9ef779a9a5e991ffcfcf30e94ef75329c1030fc2) ) // PCB silkscreened IC18 (S-L)    27C2001
	ROM_LOAD16_BYTE( "e63_18.ic19", 0x000001, 0x040000, CRC(69c97004) SHA1(65dc3dee0eb7faa1422c38947510abaeb23da7e3) ) // PCB silkscreened IC19 (S-H)    27C2001

	ROM_REGION64_BE( 0x1000000, "maingfx", 0 ) // mask ROMs
	ROM_LOAD32_WORD_SWAP( "e63-21.ic24", 0x000000, 0x400000, CRC(c818b211) SHA1(dce07bfe71a9ba11c3f028a640226c6e59c6aece) ) // PCB silkscreened IC24 (C-H)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-15.ic9",  0x000002, 0x400000, CRC(4ec6a2d7) SHA1(2ee6270cff7ea2459121961a29d42e000cee2921) ) // PCB silkscreened IC9 (C-L)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-22.ic25", 0x800000, 0x400000, CRC(6d895eb6) SHA1(473795da42fd29841a926f18a93e5992f4feb27c) ) // PCB silkscreened IC25 (M-H)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-16.ic10", 0x800002, 0x400000, CRC(d39c1e34) SHA1(6db0ce2251841db3518a9bd9c4520c3c666d19a0) ) // PCB silkscreened IC10 (M-L)    23C32000

	ROM_REGION16_BE( 0x1000000, "poly", ROMREGION_ERASEFF ) // mask ROMs
	ROM_LOAD16_WORD_SWAP( "e63-09.ic3", 0x000000, 0x400000, CRC(c3e2b1e0) SHA1(ee71f3f59b46e26dbe2ff724da2c509267c8bf2f) ) // PCB silkscreened IC3 (POLY0)    23C32000
	ROM_LOAD16_WORD_SWAP( "e63-10.ic4", 0x400000, 0x400000, CRC(f4a56390) SHA1(fc3c51a7f4639479e66ad50dcc94255d94803c97) ) // PCB silkscreened IC4 (POLY1)    23C32000
	ROM_LOAD16_WORD_SWAP( "e63-11.ic5", 0x800000, 0x400000, CRC(2293d9f8) SHA1(16adaa0523168ee63a7a34b29622c623558fdd82) ) // PCB silkscreened IC5 (POLY2)    23C32000
	// IC6 (POLY3) is not populated

	ROM_REGION( 0x800000, "sound_data", 0 ) // mask ROMs - Zoom ZSG-2 samples
	ROM_LOAD( "e63-23.ic36", 0x000000, 0x200000, CRC(d69e196e) SHA1(f738bb9e1330f6dabb5e0f0378a1a8eb48a4fa40) ) // PCB silkscreened IC36 (WD0)    23C16000
	ROM_LOAD( "e63-24.ic37", 0x200000, 0x200000, CRC(cd55f17b) SHA1(08f847ef2fd592dbaf63ef9e370cdf1f42012f74) ) // PCB silkscreened IC37 (WD1)    23C16000
	ROM_LOAD( "e63-25.ic38", 0x400000, 0x200000, CRC(bd35bdac) SHA1(5cde6c1a6b74659507b31fcb88257e65f230bfe2) ) // PCB silkscreened IC38 (WD2)    23C16000
	ROM_LOAD( "e63-26.ic39", 0x600000, 0x200000, CRC(346bd413) SHA1(0f6081d22db88eef08180278e7ae97283b5e8452) ) // PCB silkscreened IC39 (WD3)    23C16000

	ROM_REGION( 0x850, "plds", 0 )
	ROM_LOAD( "e63-01_palce16v8h-5-5.ic23",  0x000, 0x117, CRC(f114c13f) SHA1(ca9ec41d5c16347bdf107b340e6e1b9e6b7c74a9) )
	ROM_LOAD( "e63-02_palce22v10h-5-5.ic25", 0x117, 0x2dd, CRC(8418da84) SHA1(b235761f78ecb16d764fbefb00d04092d3a22ca9) )
	ROM_LOAD( "e63-05_palce16v8h-10-4.ic36", 0x3f4, 0x117, CRC(e27e9734) SHA1(77dadfbedb625b65617640bb73c59c9e5b0c927f) )
	ROM_LOAD( "e63-06_palce16v8h-10-4.ic41", 0x50b, 0x117, CRC(75184422) SHA1(d35e98e0278d713139eb1c833f41f57ed0dd3c9f) )
	ROM_LOAD( "e63-07_palce16v8h-10-4.ic43", 0x622, 0x117, CRC(eb77b03f) SHA1(567f92a4fd1fa919d5e9047ee15c058bf40855fb) )
	ROM_LOAD( "e63-08_palce16v8h-15-4.ic49", 0x739, 0x117, CRC(c305c56d) SHA1(49592fa43c548ac6b08951d03677a3f23e9c8de8) )
ROM_END

ROM_START( optigersm ) // Second Mission ver 2.02 J (build date shows 1999 but still (c) 1998)
	ROM_REGION64_BE( 0x200000, "maindata", 0 )
	ROM_LOAD32_BYTE( "e63_37.ic23", 0x000000, 0x080000, CRC(16692400) SHA1(0d9d6e90c763de66c2a99790cfe22e11c5d7ec42) ) // PCB silkscreened IC23 (P-HH)
	ROM_LOAD32_BYTE( "e63_36.ic22", 0x000001, 0x080000, CRC(99d6eed1) SHA1(96d7cff9fe5bedf79d3eaef0f19a4c69d0d233bf) ) // PCB silkscreened IC22 (P-HL)
	ROM_LOAD32_BYTE( "e63_35.ic8",  0x000002, 0x080000, CRC(85468b85) SHA1(7b24fb6eca29afbfe3ad0e6943145782d8d3b103) ) // PCB silkscreened IC8 (P-LH)
	ROM_LOAD32_BYTE( "e63_34.ic7",  0x000003, 0x080000, CRC(ba3cc7d4) SHA1(91fb1366c2d225483621beabcdc7ee2f337c8fc0) ) // PCB silkscreened IC7 (P-LL)

	ROM_REGION( 0x8000, "dsp", 0 ) // surface mounted on K11X0870A POWER JC MOTHER-G PCB
	ROM_LOAD( "tms320bc53pq80.ic8", 0x0000, 0x8000, CRC(4b8e7fd6) SHA1(07d354a2e4d7554e215fa8d91b5eeeaf573766b0) ) // decapped. TODO: believed to be a generic TI part, verify if it is and if dump is good, if so move in the CPU core

	ROM_REGION16_LE( 0x20000, "dspdata", 0 )
	ROM_LOAD16_BYTE( "taito_e63_04.ic29", 0x000000, 0x010000, CRC(eccae391) SHA1(e5293c16342cace54dc4b6dfb827558e18ac25a4) ) // PCB silkscreened IC29 (L)     AT27C512
	ROM_LOAD16_BYTE( "taito_e63_03.ic28", 0x000001, 0x010000, CRC(58fce52f) SHA1(1e3d9ee034b25e658ca45a8b900de2aa54b00135) ) // PCB silkscreened IC28 (H)     AT27C512

	ROM_REGION( 0x40000, "iocpu", 0 )
	ROM_LOAD16_BYTE( "e63_28-1.ic59", 0x000000, 0x020000, CRC(ef41ffaf) SHA1(419621f354f548180d37961b861304c469e43a65) ) // PCB silkscreened IC59 (0)     27C1001
	ROM_LOAD16_BYTE( "e63_27-1.ic58", 0x000001, 0x020000, CRC(facc17a7) SHA1(40d69840cfcfe5a509d69824c2994de56a3c6ece) ) // PCB silkscreened IC58 (1)     27C1001

	ROM_REGION( 0x80000, "mn10200", 0 ) // sound CPU to drive Zoom ZSG-2/Zoom ZFX-2
	ROM_LOAD16_BYTE( "e63_17-1.ic18", 0x000000, 0x040000, CRC(2a063d5b) SHA1(a2b2fe4d8bad1aef7d9dcc0be607cc4e5bc4f0eb) ) // PCB silkscreened IC18 (S-L)    27C2001
	ROM_LOAD16_BYTE( "e63_18-1.ic19", 0x000001, 0x040000, CRC(2f590881) SHA1(7fb827a676f45b24380558b0068b76cb858314f6) ) // PCB silkscreened IC19 (S-H)    27C2001

	ROM_REGION64_BE( 0x1000000, "maingfx", 0 ) // mask ROMs
	ROM_LOAD32_WORD_SWAP( "e63-21.ic24", 0x000000, 0x400000, CRC(c818b211) SHA1(dce07bfe71a9ba11c3f028a640226c6e59c6aece) ) // PCB silkscreened IC24 (C-H)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-15.ic9",  0x000002, 0x400000, CRC(4ec6a2d7) SHA1(2ee6270cff7ea2459121961a29d42e000cee2921) ) // PCB silkscreened IC9 (C-L)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-22.ic25", 0x800000, 0x400000, CRC(6d895eb6) SHA1(473795da42fd29841a926f18a93e5992f4feb27c) ) // PCB silkscreened IC25 (M-H)    23C32000
	ROM_LOAD32_WORD_SWAP( "e63-16.ic10", 0x800002, 0x400000, CRC(d39c1e34) SHA1(6db0ce2251841db3518a9bd9c4520c3c666d19a0) ) // PCB silkscreened IC10 (M-L)    23C32000

	ROM_REGION16_BE( 0x1000000, "poly", ROMREGION_ERASEFF ) // mask ROMs
	ROM_LOAD16_WORD_SWAP( "e63-09.ic3", 0x000000, 0x400000, CRC(c3e2b1e0) SHA1(ee71f3f59b46e26dbe2ff724da2c509267c8bf2f) ) // PCB silkscreened IC3 (POLY0)    23C32000
	ROM_LOAD16_WORD_SWAP( "e63-10.ic4", 0x400000, 0x400000, CRC(f4a56390) SHA1(fc3c51a7f4639479e66ad50dcc94255d94803c97) ) // PCB silkscreened IC4 (POLY1)    23C32000
	ROM_LOAD16_WORD_SWAP( "e63-11.ic5", 0x800000, 0x400000, CRC(2293d9f8) SHA1(16adaa0523168ee63a7a34b29622c623558fdd82) ) // PCB silkscreened IC5 (POLY2)    23C32000
	// IC6 (POLY3) is not populated

	ROM_REGION( 0x800000, "sound_data", 0 ) // mask ROMs - Zoom ZSG-2 samples
	ROM_LOAD( "e63-23.ic36", 0x000000, 0x200000, CRC(d69e196e) SHA1(f738bb9e1330f6dabb5e0f0378a1a8eb48a4fa40) ) // PCB silkscreened IC36 (WD0)    23C16000
	ROM_LOAD( "e63-24.ic37", 0x200000, 0x200000, CRC(cd55f17b) SHA1(08f847ef2fd592dbaf63ef9e370cdf1f42012f74) ) // PCB silkscreened IC37 (WD1)    23C16000
	ROM_LOAD( "e63-25.ic38", 0x400000, 0x200000, CRC(bd35bdac) SHA1(5cde6c1a6b74659507b31fcb88257e65f230bfe2) ) // PCB silkscreened IC38 (WD2)    23C16000
	ROM_LOAD( "e63-26.ic39", 0x600000, 0x200000, CRC(346bd413) SHA1(0f6081d22db88eef08180278e7ae97283b5e8452) ) // PCB silkscreened IC39 (WD3)    23C16000

	ROM_REGION( 0x850, "plds", 0 )
	ROM_LOAD( "e63-01_palce16v8h-5-5.ic23",  0x000, 0x117, CRC(f114c13f) SHA1(ca9ec41d5c16347bdf107b340e6e1b9e6b7c74a9) )
	ROM_LOAD( "e63-02_palce22v10h-5-5.ic25", 0x117, 0x2dd, CRC(8418da84) SHA1(b235761f78ecb16d764fbefb00d04092d3a22ca9) )
	ROM_LOAD( "e63-05_palce16v8h-10-4.ic36", 0x3f4, 0x117, CRC(e27e9734) SHA1(77dadfbedb625b65617640bb73c59c9e5b0c927f) )
	ROM_LOAD( "e63-06_palce16v8h-10-4.ic41", 0x50b, 0x117, CRC(75184422) SHA1(d35e98e0278d713139eb1c833f41f57ed0dd3c9f) )
	ROM_LOAD( "e63-07_palce16v8h-10-4.ic43", 0x622, 0x117, CRC(eb77b03f) SHA1(567f92a4fd1fa919d5e9047ee15c058bf40855fb) )
	ROM_LOAD( "e63-08_palce16v8h-15-4.ic49", 0x739, 0x117, CRC(c305c56d) SHA1(49592fa43c548ac6b08951d03677a3f23e9c8de8) )
ROM_END

} // anonymous namespace


GAME( 1998, optiger,   0,       taitopjc, taitopjc, taitopjc_state, init_optiger, ROT0, "Taito", "Operation Tiger (Ver 2.14 O)",                MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NO_SOUND ) // Operation Tiger Ver 2.14 O, Oct  5 1998  13:58:13
GAME( 1998, optigera,  optiger, taitopjc, taitopjc, taitopjc_state, init_optiger, ROT0, "Taito", "Operation Tiger (Ver 2.10 O)",                MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NO_SOUND ) // Operation Tiger Ver 2.10 O, Sep 19 1998  14:06:18
GAME( 1998, optigerj,  optiger, taitopjc, taitopjc, taitopjc_state, init_optiger, ROT0, "Taito", "Operation Tiger (Ver 2.09 J)",                MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NO_SOUND ) // Operation Tiger Ver 2.09 J, Sep 12 1998  19:28:59
GAME( 1998, optigersm, 0,       taitopjc, taitopjc, taitopjc_state, init_optiger, ROT0, "Taito", "Operation Tiger Second Mission (Ver 2.02 J)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NO_SOUND ) // Operation Tiger Ver 2.02 J, Second Mission, Feb 16 1999  14:23:36
