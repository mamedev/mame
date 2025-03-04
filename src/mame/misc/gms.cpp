// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina, David Haywood
// thanks-to: Hammy

/*******************************************************************
实战麻将王 (Shízhàn Májiàng Wáng), GMS, 1998
实战頂凰麻雀 (Shízhàn Dǐng Huáng Máquè), GMS, 1998
实战三国记加強版 (Shízhàn Sānguó Jì Jiāqiáng Bǎn), GMS, 1998
Hardware info by Guru
---------------------

Note: Hardware is called 'Game Men System'. Company is Game Master System Co., Ltd.

These games run on a near identical board with a slightly different sound chip.
6899-B: YM2151 and the PIC circuit is part of the main board but not used. Pin header holes for a small PCB are present but not used.
9899-B: YM3812 and the PIC is on a separate PCB soldered into the same pin header holes and the PCB area under it contains nothing.

No.6899-B (Shizhan Majiang Wang and Shizhan Ding Huang Maque)
No.9899-B (Shizhan Sanguo Ji Jiaqiang Ban)
|--------------------------------------------------------|
|UPC1241H          YM3014  SNDCHIP   14.31818MHz         |
|     VOL       358                  89C51        B1/M1  |
|          M6295                                         |
|                  S1      PAL                           |
|                                             A1         |
|            T518B                                       |
|J                                           6116        |
|A                 P1                        6116        |
|M    SW3                                                |
|M    SW2                                                |
|A    SW1    SW4                                         |
|                               |-------|    6116        |
|                               |LATTICE|    6116  PAL   |
|               62256    62256  |1032E  |                |
|                               |       |    T1          |
|                    68HC000    |-------|                |
| 3.6V_BATT     |-------------|                          |
|               |        93C46|                          |
|               |             |                          |
|SW5            |  *          |              6116        |
|         7805  |             |  22MHz       6116        |
|---------------|PLASTIC COVER|--------------------------|
Notes:
      68HC000 - Toshiba TMP68HC000P-16 CPU. Clock 11.0MHz [22/2]
         6116 - 2kB x8-bit SRAM
        62256 - 32kB x8-bit SRAM (only one is battery-backed)
         6295 - OKI M6295 4-Channel ADPCM Voice Synthesis LSI. Clock input 1.100MHz [22/20]. Pin 7 HIGH
      SNDCHIP - 6899-B PCB - Yamaha YM2151 FM Operator Type-M (OPM). Clock 2.750MHz [22/8]. Chip is marked 'K-666'. YM3014 chip is marked 'K-664'
                9899-B PCB - Yamaha YM3812 FM Operator Type-L (OPL2). Clock 2.750MHz [22/8]. Chip is marked 'KS8001' YM3014 chip is marked 'KS8002'
            * - 6899-B PCB - Unpopulated position for PIC16F84
                9899-B PCB - Separate PCB containing two unknown chips (SOIC16, SOIC8) and a TO92 IC marked 'H HE 9013I'
        89C51 - Atmel AT89C51 Microcontroller (protected). Clock input 14.31818MHz
        93C46 - On 6899-B PCB - 93C46 EEPROM, covered by a plastic cover. No other parts populated. There is an unpopulated position for a PIC16F84
        T518B - Mitsumi PST518B Master Reset IC (TO92)
          358 - LM358 Dual Operational Amplifier
        SW1-4 - 8-position DIP Switch
          SW5 - Reset/Clear Switch
     uPC1241H - NEC uPC1241H Audio Power Amp
         7805 - LM7805 5V Linear Regulator
    3.6V_BATT - Back-up battery maintains power to one 62256 RAM when main power supply is off.
           P1 - 27C4096 (Main PRG)
           T1 - 27C4000 / 234000 mask ROM (GFX)
           A1 - 27C080 / 238000 mask ROM (GFX)
        B1/M1 - 26C1000 / 27C1001 (PRG/data for 89C51?)
           S1 - 27C2000 / 232000 mask ROM (OKI samples)

Hold service credit (9) and reset (F3) to enter service mode.


TODO:
- correct EEPROM hookup for all games (this would get rid of a lot of ROM patches)
- hookup MCU and YM2151 / YM3812 sound for the mahjong games
- hookup PIC16F84 for rbspm
- emulate protection devices correctly instead of patching
- hookup lamps and do layouts
- keyboard inputs for mahjong games
- use real values for reel tilemaps offsets instead of hardcoded ones (would fix magslot)
- complete inputs for baile, yyhm, jinpaish (needs someone who understands Chinese and
  knows how to play)
- game logic in baile seems broken (you always win), maybe due to the patches?
- broken title GFX in yyhm (transparent pen problem?)
- broken title GFX in cjdlz. How should the extra ROM be loaded?
- the newer games seem to use range 0x9e1000-0x9e1fff during gameplay

Video references:
rbspm: https://www.youtube.com/watch?v=pPk-6N1wXoE
sc2in1: https://www.youtube.com/watch?v=RNwW1IhKHXw
super555: https://www.youtube.com/watch?v=CCUKdbQ5O-U
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/pic16x8x/pic16x8x.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_TILEATTR (1U << 1)

// #define VERBOSE (LOG_GENERAL | LOG_TILEATTR)

#include "logmacro.h"

#define LOGTILEATTR(...) LOGMASKED(LOG_TILEATTR, __VA_ARGS__)


namespace {

class gms_2layers_state : public driver_device
{
public:
	gms_2layers_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vidram(*this, "vidram%u", 1U)
		, m_reelram(*this, "reelram%u", 1U)
		, m_scrolly(*this, "scrolly%u", 1U)
		, m_maincpu(*this, "maincpu")
		, m_mcu(*this, "mcu")
		, m_eeprom(*this, "eeprom")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_oki(*this, "oki")
		, m_ymsnd(*this, "ymsnd")
		, m_dsw(*this, "DSW%u", 1U)
	{
	}

	void rbmk(machine_config &config) ATTR_COLD;
	void rbspm(machine_config &config) ATTR_COLD;
	void ssanguoj(machine_config &config) ATTR_COLD;

	void super555(machine_config &config) ATTR_COLD;

	void init_ballch() ATTR_COLD;
	void init_cjldz() ATTR_COLD;
	void init_cots() ATTR_COLD;
	void init_hgly() ATTR_COLD;
	void init_rbspm() ATTR_COLD;
	void init_sball2k1() ATTR_COLD;
	void init_ssanguoj() ATTR_COLD;
	void init_sscs() ATTR_COLD;
	void init_super555() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	optional_shared_ptr_array<uint16_t, 2> m_vidram;
	required_shared_ptr_array<uint16_t, 4> m_reelram;
	required_shared_ptr_array<uint16_t, 4> m_scrolly;

	required_device<cpu_device> m_maincpu;
	optional_device<at89c4051_device> m_mcu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<okim6295_device> m_oki;
	optional_device<ym2151_device> m_ymsnd;
	optional_ioport_array<4> m_dsw;

	uint16_t m_reels_toggle = 0;
	uint16_t m_tilebank = 0;
	tilemap_t *m_reel_tilemap[4];
	tilemap_t *m_tilemap[2]{};

	void super555_mem(address_map &map) ATTR_COLD;

	template <uint8_t Which> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	template <uint8_t Which> void reelram_w(offs_t offset, uint16_t data, uint16_t mem_mask);

private:
	uint8_t m_mux_data = 0;
	uint16_t m_input_matrix = 0;
	//uint16_t m_prot_data = 0;

	void mcu_io(address_map &map) ATTR_COLD;
	void rbmk_mem(address_map &map) ATTR_COLD;
	void rbspm_mem(address_map &map) ATTR_COLD;
	void ssanguoj_mem(address_map &map) ATTR_COLD;

	uint16_t unk_r();
	uint16_t input_matrix_r();
	void input_matrix_w(uint16_t data);
	void tilebank_w(uint16_t data);
	void reels_toggle_w(uint16_t data);
	uint8_t mcu_io_r(offs_t offset);
	void mcu_io_w(offs_t offset, uint8_t data);
	void mcu_io_mux_w(uint8_t data);
	void eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_reel_tile_info);
	TILE_GET_INFO_MEMBER(get_tile0_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

class gms_3layers_state : public gms_2layers_state
{
public:
	gms_3layers_state(const machine_config &mconfig, device_type type, const char *tag)
		: gms_2layers_state(mconfig, type, tag)
	{
	}

	void init_baile() ATTR_COLD;
	void init_jinpaish() ATTR_COLD;
	void init_sc2in1() ATTR_COLD;
	void init_yyhm() ATTR_COLD;

	void magslot(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void magslot_mem(address_map &map) ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_tile1_info);
};

uint16_t gms_2layers_state::unk_r()
{
	return machine().rand();
}

uint16_t gms_2layers_state::input_matrix_r()
{
	uint16_t res = 0xffff;

	// TODO: & 0x00ff are the inputs for keyboard mode in rbmk
	if (m_input_matrix & 0x1000) res &= m_dsw[0]->read();
	if (m_input_matrix & 0x2000) res &= m_dsw[1].read_safe(0xffff);
	if (m_input_matrix & 0x4000) res &= m_dsw[2].read_safe(0xffff);

	return res;
}

void gms_2layers_state::tilebank_w(uint16_t data)
{
	m_tilebank = data;

	// fedcba98 76543210
	// xxxx              // unknown (never seen set, possibly a 4th tilemap not used by the dumped games?)
	//     x             // 3rd tilemap enable (probably)
	//      xx           // bank 3rd tilemap
	//        x          // unknown (never seen set)
	//          x        // unknown (never seen set)
	//           x       // unknown (set during most screens in the mahjong games and in sc2in1' title screen)
	//            x      // priority between 1st and 2nd tilemaps
	//             x     // bank 1st tilemap
	//              x    // 1st tilemap enable (probably)
	//               xx  // bank 2nd tilemap
	//                 x // 2nd tilemap enable (probably)

	if (m_tilebank & 0xf1c0)
		LOGTILEATTR("unknown tilemap attribute: %04x\n", m_tilebank & 0xf1c0);
}

void gms_2layers_state::reels_toggle_w(uint16_t data)
{
	m_reels_toggle = BIT(data, 0);

	if (m_reels_toggle & 0xfffe)
		logerror("unknown reels toggle attribute: %04x\n", m_reels_toggle);
}

template <uint8_t Which>
void gms_2layers_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vidram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void gms_2layers_state::reelram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_reelram[Which][offset]);
	m_reel_tilemap[Which]->mark_tile_dirty(offset);
}

void gms_2layers_state::input_matrix_w(uint16_t data)
{
	m_input_matrix = data & 0x7fff;

	m_oki->set_rom_bank(BIT(data, 15));
}

void gms_2layers_state::eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// bad ?
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->di_write((data & 0x04) >> 2);
		m_eeprom->cs_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);

		m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
	}
}


void gms_2layers_state::rbmk_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom().nopw();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x200001).w(FUNC(gms_2layers_state::reels_toggle_w));
	map(0x500000, 0x50ffff).ram();
	map(0x900000, 0x900fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x940000, 0x9403ff).ram().w(FUNC(gms_2layers_state::reelram_w<0>)).share(m_reelram[0]);
	map(0x940400, 0x9407ff).ram().w(FUNC(gms_2layers_state::reelram_w<1>)).share(m_reelram[1]);
	map(0x940800, 0x940bff).ram().w(FUNC(gms_2layers_state::reelram_w<2>)).share(m_reelram[2]);
	map(0x940c00, 0x940fff).ram().w(FUNC(gms_2layers_state::reelram_w<3>)).share(m_reelram[3]);
	map(0x980000, 0x983fff).ram(); // 0x2048  words ???, byte access
	map(0x980180, 0x9801ff).ram().share(m_scrolly[0]);
	map(0x980280, 0x9802ff).ram().share(m_scrolly[1]);
	map(0x980300, 0x98037f).ram().share(m_scrolly[2]);
	map(0x980380, 0x9803ff).ram().share(m_scrolly[3]);
	map(0x9c0000, 0x9c0fff).ram().w(FUNC(gms_2layers_state::vram_w<0>)).share(m_vidram[0]);
	map(0xb00000, 0xb00001).w(FUNC(gms_2layers_state::eeprom_w));
	map(0xc00000, 0xc00001).rw(FUNC(gms_2layers_state::input_matrix_r), FUNC(gms_2layers_state::input_matrix_w));
	map(0xc08000, 0xc08001).portr("IN1").w(FUNC(gms_2layers_state::tilebank_w));
	map(0xc10000, 0xc10001).portr("IN2");
	map(0xc18080, 0xc18081).r(FUNC(gms_2layers_state::unk_r));  // TODO: from MCU?
	map(0xc20000, 0xc20000).r(m_oki, FUNC(okim6295_device::read));
	//map(0xc20080, 0xc20081) // TODO: to MCU?
	map(0xc28000, 0xc28000).w(m_oki, FUNC(okim6295_device::write));
}

void gms_2layers_state::rbspm_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x200001).w(FUNC(gms_2layers_state::reels_toggle_w));
	map(0x300000, 0x300001).rw(FUNC(gms_2layers_state::input_matrix_r), FUNC(gms_2layers_state::input_matrix_w));
	map(0x308000, 0x308001).portr("IN1").w(FUNC(gms_2layers_state::tilebank_w)); // ok
	map(0x310000, 0x310001).portr("IN2");
	map(0x318080, 0x318081).r(FUNC(gms_2layers_state::unk_r));
	map(0x320000, 0x320000).r(m_oki, FUNC(okim6295_device::read));
	map(0x328000, 0x328000).w(m_oki, FUNC(okim6295_device::write));
	map(0x340002, 0x340003).nopw();
	map(0x500000, 0x50ffff).ram();
	map(0x900000, 0x900fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x940000, 0x9403ff).ram().w(FUNC(gms_2layers_state::reelram_w<0>)).share(m_reelram[0]);
	map(0x940400, 0x9407ff).ram().w(FUNC(gms_2layers_state::reelram_w<1>)).share(m_reelram[1]);
	map(0x940800, 0x940bff).ram().w(FUNC(gms_2layers_state::reelram_w<2>)).share(m_reelram[2]);
	map(0x940c00, 0x940fff).ram().w(FUNC(gms_2layers_state::reelram_w<3>)).share(m_reelram[3]);
	map(0x980000, 0x983fff).ram(); // 0x2048  words ???, byte access, u25 and u26 according to test mode
	map(0x980180, 0x9801ff).ram().share(m_scrolly[0]);
	map(0x980280, 0x9802ff).ram().share(m_scrolly[1]);
	map(0x980300, 0x98037f).ram().share(m_scrolly[2]);
	map(0x980380, 0x9803ff).ram().share(m_scrolly[3]);
	map(0x9c0000, 0x9c0fff).ram().w(FUNC(gms_2layers_state::vram_w<0>)).share(m_vidram[0]);
}

void gms_2layers_state::ssanguoj_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom().nopw();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x200001).w(FUNC(gms_2layers_state::reels_toggle_w));
	map(0x800000, 0x80ffff).ram();
	map(0x900000, 0x900fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x940000, 0x9403ff).ram().w(FUNC(gms_2layers_state::reelram_w<0>)).share(m_reelram[0]);
	map(0x940400, 0x9407ff).ram().w(FUNC(gms_2layers_state::reelram_w<1>)).share(m_reelram[1]);
	map(0x940800, 0x940bff).ram().w(FUNC(gms_2layers_state::reelram_w<2>)).share(m_reelram[2]);
	map(0x940c00, 0x940fff).ram().w(FUNC(gms_2layers_state::reelram_w<3>)).share(m_reelram[3]);
	map(0x980000, 0x983fff).ram(); // 0x2048  words ???, byte access, u25 and u26 according to test mode
	map(0x980180, 0x9801ff).ram().share(m_scrolly[0]);
	map(0x980280, 0x9802ff).ram().share(m_scrolly[1]);
	map(0x980300, 0x98037f).ram().share(m_scrolly[2]);
	map(0x980380, 0x9803ff).ram().share(m_scrolly[3]);
	map(0x9c0000, 0x9c0fff).ram().w(FUNC(gms_2layers_state::vram_w<0>)).share(m_vidram[0]);
	map(0xa00000, 0xa00001).rw(FUNC(gms_2layers_state::input_matrix_r), FUNC(gms_2layers_state::input_matrix_w));
	map(0xa08000, 0xa08001).portr("IN1").w(FUNC(gms_2layers_state::tilebank_w));
	map(0xa10000, 0xa10001).portr("IN2");
	map(0xa18080, 0xa18081).r(FUNC(gms_2layers_state::unk_r));  // TODO: from MCU?
	map(0xa20000, 0xa20000).r(m_oki, FUNC(okim6295_device::read));
	//map(0xa20080, 0xa20081) // TODO: to MCU?
	map(0xa28000, 0xa28000).w(m_oki, FUNC(okim6295_device::write));
	map(0xe00000, 0xe00001).w(FUNC(gms_2layers_state::eeprom_w));
}

void gms_2layers_state::super555_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x300000, 0x300001).w(FUNC(gms_2layers_state::reels_toggle_w));
	map(0x600000, 0x600001).rw(FUNC(gms_2layers_state::input_matrix_r), FUNC(gms_2layers_state::input_matrix_w));
	map(0x608000, 0x608001).portr("IN1").w(FUNC(gms_2layers_state::tilebank_w)); // ok
	map(0x610000, 0x610001).portr("IN2");
	map(0x618080, 0x618081).nopr();//.lr16(NAME([this] () -> uint16_t { return m_prot_data; })); // reads something here from below, if these are hooked up booting stops with '0x09 U64 ERROR', like it's failing some checksum test
	map(0x620000, 0x620000).r(m_oki, FUNC(okim6295_device::read)); // Oki controlled through a GAL at 18C
	// map(0x620080, 0x620081).lw16(NAME([this] (uint16_t data) { m_prot_data = data; })); // writes something here that expects to read above
	map(0x628000, 0x628000).w(m_oki, FUNC(okim6295_device::write));
	map(0x638000, 0x638001).nopw(); // lamps / outputs?
	map(0x900000, 0x900fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x940000, 0x9403ff).ram().w(FUNC(gms_2layers_state::reelram_w<0>)).share(m_reelram[0]);
	map(0x940400, 0x9407ff).ram().w(FUNC(gms_2layers_state::reelram_w<1>)).share(m_reelram[1]);
	map(0x940800, 0x940bff).ram().w(FUNC(gms_2layers_state::reelram_w<2>)).share(m_reelram[2]);
	map(0x940c00, 0x940fff).ram().w(FUNC(gms_2layers_state::reelram_w<3>)).share(m_reelram[3]);
	map(0x980000, 0x983fff).ram(); // 0x2048  words ???, byte access, u25 and u26 according to test mode
	map(0x980180, 0x9801ff).ram().share(m_scrolly[0]);
	map(0x980280, 0x9802ff).ram().share(m_scrolly[1]);
	map(0x980300, 0x98037f).ram().share(m_scrolly[2]);
	map(0x980380, 0x9803ff).ram().share(m_scrolly[3]);
	map(0x9c0000, 0x9c0fff).ram().w(FUNC(gms_2layers_state::vram_w<0>)).share(m_vidram[0]);
	//map(0xf00000, 0xf00001).w(FUNC(gms_2layers_state::eeprom_w)); // wrong?
}

void gms_3layers_state::magslot_mem(address_map &map)
{
	super555_mem(map);

	map(0x9e0000, 0x9e0fff).ram().w(FUNC(gms_3layers_state::vram_w<1>)).share(m_vidram[1]);
}

uint8_t gms_2layers_state::mcu_io_r(offs_t offset)
{
	if (m_mux_data & 8)
	{
		return m_ymsnd->read(offset & 1);
	}
	else if (m_mux_data & 4)
	{
		// printf("%02x R\n",offset);
		// ...
		return 0xff;
	}
	else
		printf("Warning: mux data R = %02x", m_mux_data);

	return 0xff;
}

void gms_2layers_state::mcu_io_w(offs_t offset, uint8_t data)
{
	if (m_mux_data & 8) { m_ymsnd->write(offset & 1, data); }
	else if (m_mux_data & 4)
	{
		//printf("%02x %02x W\n", offset, data);
		// ...
	}
	else
		printf("Warning: mux data W = %02x", m_mux_data);
}

void gms_2layers_state::mcu_io_mux_w(uint8_t data)
{
	m_mux_data = ~data;
}

void gms_2layers_state::mcu_io(address_map &map)
{
	map(0x0ff00, 0x0ffff).rw(FUNC(gms_2layers_state::mcu_io_r), FUNC(gms_2layers_state::mcu_io_w));
}

static INPUT_PORTS_START( rbmk )
	PORT_START("IN1")   // 16bit
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT  )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   // 16bit
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_TOGGLE
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_TOGGLE
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	// Only 4 DIP banks are actually populated on PCBs (2 empty spaces), but test mode reads all 6.
	// Dips based on manuals for both rbmk and rbspm
	PORT_START("DSW1")   // 16bit, in test mode first 8 are recognized as dsw1, second 8 as dsw4.
	PORT_DIPNAME( 0x0007, 0x0000, "Pay Out Rate" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(      0x0000, "70%" )
	PORT_DIPSETTING(      0x0001, "75%" )
	PORT_DIPSETTING(      0x0002, "80%" )
	PORT_DIPSETTING(      0x0003, "82%" )
	PORT_DIPSETTING(      0x0004, "85%" )
	PORT_DIPSETTING(      0x0005, "88%" )
	PORT_DIPSETTING(      0x0006, "90%" )
	PORT_DIPSETTING(      0x0007, "95%" )
	PORT_DIPNAME( 0x0008, 0x0000, "Yakupai Rate" ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( High ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSW1:5") // computer level
	PORT_DIPSETTING(      0x0010, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Double Up Game" ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x0000, "Double Up Rate" ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(      0x0000, "80%" )
	PORT_DIPSETTING(      0x0040, "85%" )
	PORT_DIPSETTING(      0x0080, "90%" )
	PORT_DIPSETTING(      0x00c0, "95%" )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e00, 0x0000, "Break Max" ) PORT_DIPLOCATION("DSW4:2,3,4")
	PORT_DIPSETTING(      0x0000, "1000" )
	PORT_DIPSETTING(      0x0200, "2000" )
	PORT_DIPSETTING(      0x0400, "3000" )
	PORT_DIPSETTING(      0x0600, "5000" )
	PORT_DIPSETTING(      0x0800, "10000" )
	PORT_DIPSETTING(      0x0a00, "20000" )
	PORT_DIPSETTING(      0x0c00, "30000" )
	PORT_DIPSETTING(      0x0e00, "50000" )
	PORT_DIPNAME( 0x3000, 0x0000, "Credits Max" ) PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPSETTING(      0x1000, "1000" )
	PORT_DIPSETTING(      0x2000, "2000" )
	PORT_DIPSETTING(      0x3000, "5000" )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Version ) ) PORT_DIPLOCATION("DSW4:7") // this affects the version shown on title screen, however the dip sheet says "Yakuman Times" (on more often, off less)
	PORT_DIPSETTING(      0x4000, "8.8" )
	PORT_DIPSETTING(      0x0000, "8.8-" )
	PORT_DIPNAME( 0x8000, 0x0000, "Hide Gambling" ) PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(      0x8000, "Mahjong" )
	PORT_DIPSETTING(      0x0000, "Chess" )


	PORT_START("DSW2")   // 16bit, in test mode first 8 are recognized as dsw2, second 8 as dsw5
	PORT_DIPNAME( 0x0007, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0004, "1 Coin/10 Credits" )
	PORT_DIPSETTING(      0x0005, "1 Coin/20 Credits" )
	PORT_DIPSETTING(      0x0006, "1 Coin/50 Credits" )
	PORT_DIPSETTING(      0x0007, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x0018, 0x0000, "Credits per Note" ) PORT_DIPLOCATION("DSW2:4,5,")
	PORT_DIPSETTING(      0x0018, "1 Note/5 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Note/10 Credits" )
	PORT_DIPSETTING(      0x0008, "1 Note/20 Credits" )
	PORT_DIPSETTING(      0x0010, "1 Note/50 Credits" )
	PORT_DIPNAME( 0x0020, 0x0000, "Show Tiles after Reach" ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Pay Out Type" ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0040, "Credits" )
	PORT_DIPSETTING(      0x0000, "Coins" )
	PORT_DIPNAME( 0x0080, 0x0080, "Controls" ) PORT_DIPLOCATION("DSW2:8") // should default to keyboard, but set on joystick since the former isn't emulated yet
	PORT_DIPSETTING(      0x0080, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0000, "Keyboard" )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")      // 16bit, in test mode first 8 are recognized as dsw3, second 8 as dsw6
	PORT_DIPNAME( 0x0003, 0x0000, "Min Bet" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0002, "5" )
	PORT_DIPSETTING(      0x0003, "10" )
	PORT_DIPNAME( 0x000c, 0x0000, "Max Bet" ) PORT_DIPLOCATION("DSW3:3,4")
	PORT_DIPSETTING(      0x0000, "10" )
	PORT_DIPSETTING(      0x0004, "20" )
	PORT_DIPSETTING(      0x0008, "30" )
	PORT_DIPSETTING(      0x000c, "50" )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:5") // not listed in the dip sheet
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0060, 0x0000, "Insert Coin Continue" ) PORT_DIPLOCATION("DSW3:6,7")
	PORT_DIPSETTING(      0x0000, "30 Seconds" )
	PORT_DIPSETTING(      0x0020, "60 Seconds" )
	PORT_DIPSETTING(      0x0040, "90 Seconds" )
	PORT_DIPSETTING(      0x0060, "Unlimited" )
	PORT_DIPNAME( 0x0080, 0x0000, "Tiles Sound" ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW6:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW6:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW6:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW6:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW6:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW6:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW6:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( rbspm )
	PORT_INCLUDE( rbmk )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Version ) ) PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(      0x4000, "4.1" )
	PORT_DIPSETTING(      0x0000, "4.2" )
INPUT_PORTS_END


static INPUT_PORTS_START( ssanguoj )
	PORT_START("IN1")   // 16bit
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT  )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   // 16bit
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_TOGGLE
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_TOGGLE
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	// Only 4 DIP banks are actually populated on PCBs (2 empty spaces), but test mode reads all 6.
	// TODO: dips
	PORT_START("DSW1")   // 16bit, in test mode first 8 are recognized as dsw1, second 8 as dsw4.
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Controls" ) PORT_DIPLOCATION("DSW1:8") // should default to keyboard, but set on joystick since the former isn't emulated yet
	PORT_DIPSETTING(      0x0080, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0000, "Keyboard" )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )


	PORT_START("DSW2")   // 16bit, in test mode first 8 are recognized as dsw2, second 8 as dsw5
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Version ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0040, "8.9" )
	PORT_DIPSETTING(      0x0000, "8.9-" )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")      // 16bit, in test mode first 8 are recognized as dsw3, second 8 as dsw6
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( magslot )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Bet Max") // seems to be used to bet the allowed maximum
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Show Info")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("9 Lines")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("3 Lines")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("5 Lines")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1 Line")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("7 Lines")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_TOGGLE
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_TOGGLE
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)) // TODO: verify

	// 3 8-dip banks on PCB
	PORT_START("DSW1") // Game setup is password protected, needs reverse engineering of the password
	PORT_DIPNAME(         0x0001, 0x0000, "Game Password" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, "Power On" )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( super555 )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start / Take" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME( "Hold 2 / Double Up" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Paytable" )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME( "Hold 1 / Double Up / Big" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME( "Hold 3 / Double Up / Small" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)) // TODO: verify


	// There are 4 banks of 8 DIP switches on the PCB but only 3 are shown in test mode. DIP switch settings as per test mode.
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0000, "Main Game Rate" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0003, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Limit Over Score" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, "100.000" )
	PORT_DIPSETTING(      0x0004, "200.000" )
	PORT_DIPNAME( 0x0008, 0x0000, "Coin/Key In Over Score" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, "30.000" )
	PORT_DIPSETTING(      0x0008, "50.000" )
	PORT_DIPNAME( 0x0010, 0x0000, "W-Up Game" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "W-Up Game Rate" ) PORT_DIPLOCATION("SW1:6") // only has effect if the above one is on.
	PORT_DIPSETTING(      0x0020, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Auto Mode" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, "Good" )
	PORT_DIPSETTING(      0x0000, "Hits" )
	PORT_DIPNAME( 0x0080, 0x0000, "Five Bars" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0000, "SW4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0000, "SW4:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0000, "SW4:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0000, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x0000, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x0000, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x0000, "SW4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x0000, "SW4:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0000, "Coin Rate" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0002, "10" )
	PORT_DIPSETTING(      0x0003, "20" )
	PORT_DIPSETTING(      0x0004, "30" )
	PORT_DIPSETTING(      0x0000, "50" )
	PORT_DIPSETTING(      0x0005, "100" )
	PORT_DIPSETTING(      0x0006, "200" )
	PORT_DIPSETTING(      0x0007, "300" )
	PORT_DIPNAME( 0x0018, 0x0000, "Coin x Times Rate" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0008, "5" )
	PORT_DIPSETTING(      0x0010, "10" )
	PORT_DIPSETTING(      0x0018, "20" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Counter Jumping" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "By Keyin Rate" )
	PORT_DIPSETTING(      0x0000, "By Coin Rate" )
	PORT_DIPNAME( 0x0080, 0x0000, "Cards Voice" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0003, 0x0000, "Min. Bet" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0000, "10" )
	PORT_DIPSETTING(      0x0001, "20" )
	PORT_DIPSETTING(      0x0002, "30" )
	PORT_DIPSETTING(      0x0003, "50" )
	PORT_DIPNAME( 0x000c, 0x0000, "Max. Bet" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x0000, "50" )
	PORT_DIPSETTING(      0x0004, "100" )
	PORT_DIPSETTING(      0x0008, "150" )
	PORT_DIPSETTING(      0x000c, "200" )
	PORT_DIPNAME( 0x0010, 0x0000, "Connector" ) PORT_DIPLOCATION("SW3:5") // Hardcoded to JAMMA
	PORT_DIPSETTING(      0x0010, "JAMMA" )
	PORT_DIPSETTING(      0x0000, "JAMMA" )
	PORT_DIPNAME( 0x0020, 0x0000, "Card Choice" ) PORT_DIPLOCATION("SW3:6") // also changes title screen
	PORT_DIPSETTING(      0x0020, "Car" ) // city skyline title screen
	PORT_DIPSETTING(      0x0000, "Poker" ) // lady in red with card title screen
	PORT_DIPNAME( 0x0040, 0x0000, "Last Game Mode" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0040, "Rechoice Card" )
	PORT_DIPSETTING(      0x0000, "Only 6 Card" )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8") // not shown in test mode
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sscs )
	PORT_START("IN1") // TODO: PORT_NAMEs are machine translated, should be checked and adapted
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Open card" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME( "1 key" ) // also used for up in the 'secret' test mode
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) // used for down in the 'secret' test mode
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Abandon" )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Look at the card" )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)) // TODO: verify


	// There are 4 banks of 8 DIP switches on PCB, but only 3 are shown in test mode.
	// DIP switch settings as per test mode. 'Secret' test mode shows all 4 banks.
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0000, "Main Game Pay Out Rate" )        PORT_DIPLOCATION("SW1:1,2")   // 主遊戲機率
	PORT_DIPSETTING(      0x0003, DEF_STR( Easy ) )                                               // 易
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )                                             // 中
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )                                               // 難
	PORT_DIPSETTING(      0x0001, DEF_STR( Hardest ) )                                            // 最難
	PORT_DIPNAME( 0x000c, 0x0000, "Double-Up Game Pay Out Rate" )   PORT_DIPLOCATION("SW1:3,4")   // 比倍遊戲機率 - disabled if SW1:5 is off
	PORT_DIPSETTING(      0x000c, DEF_STR( Easy ) )                                               // 易
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )                                             // 中
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )                                               // 難
	PORT_DIPSETTING(      0x0004, DEF_STR( Hardest ) )                                            // 最難
	PORT_DIPNAME( 0x0010, 0x0000, "Double-Up On/Off" )              PORT_DIPLOCATION("SW1:5")     // 比倍有無
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )                                                // 無
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )                                                 // 有
	PORT_DIPNAME( 0x0020, 0x0000, "Double-Up Nudity" )              PORT_DIPLOCATION("SW1:6")     // 比倍脫衣 - disabled if SW1:5 is off
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )                                             // 正常
	PORT_DIPSETTING(      0x0020, "Nudity" )                                                      // 脫衣
	PORT_DIPNAME( 0x0040, 0x0000, "Double-Up Win Points" )          PORT_DIPLOCATION("SW1:7")     // 比倍爆機分數 - disabled if SW1:5 is off
	PORT_DIPSETTING(      0x0000, "10,000" )
	PORT_DIPSETTING(      0x0040, "30,000" )
	PORT_DIPNAME( 0x0080, 0x0000, "Main Game Background Music")     PORT_DIPLOCATION("SW1:8")     // 主遊戲背景音樂
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )                                                // 無
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )                                                 // 有
	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0000, "SW4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0000, "SW4:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0000, "SW4:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0000, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x0000, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x0000, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x0000, "SW4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x0000, "SW4:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0000, "Coin Rate" )                     PORT_DIPLOCATION("SW2:1,2,3") // 投幣比例
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0002, "10" )
	PORT_DIPSETTING(      0x0003, "20" )
	PORT_DIPSETTING(      0x0004, "30" )
	PORT_DIPSETTING(      0x0000, "50" )
	PORT_DIPSETTING(      0x0005, "100" )
	PORT_DIPSETTING(      0x0006, "200" )
	PORT_DIPSETTING(      0x0007, "300" )
	PORT_DIPNAME( 0x0018, 0x0000, "Coin Rate x Key In Multiplier" ) PORT_DIPLOCATION("SW2:4,5")   // 投幣比例ｘ開分倍率 - Key In rate as a multiple of coin rate
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0008, "5" )
	PORT_DIPSETTING(      0x0010, "10" )
	PORT_DIPSETTING(      0x0018, "20" )
	PORT_DIPNAME( 0x0020, 0x0000, "Show Title" )                    PORT_DIPLOCATION("SW2:6")     // 片頭名稱
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )                                                // 無
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )                                                 // 有
	PORT_DIPNAME( 0x0040, 0x0000, "Double-Up Win Points Method" )   PORT_DIPLOCATION("SW2:7")     // 比倍爆機得分方式
	PORT_DIPSETTING(      0x0000, "By Key Out" )                                                  // 按洗分键
	PORT_DIPSETTING(      0x0040, "By Pressing Button" )                                          // 按得键
	PORT_DIPNAME( 0x0080, 0x0000, "Control Panel" )                 PORT_DIPLOCATION("SW2:8")     // 操作介面
	PORT_DIPSETTING(      0x0000, "Amusement/Poker Panel" )                                       // 娛樂/撲克介面
	PORT_DIPSETTING(      0x0080, "Mahjong Panel" )                                               // 麻將介面

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0003, 0x0000, "Ante Points" )                   PORT_DIPLOCATION("SW3:1,2")   // 底注分數
	PORT_DIPSETTING(      0x0000, "10" )
	PORT_DIPSETTING(      0x0001, "20" )
	PORT_DIPSETTING(      0x0002, "50" )
	PORT_DIPSETTING(      0x0003, "80" )
	PORT_DIPNAME( 0x000c, 0x0000, "Main Game Win Points" )          PORT_DIPLOCATION("SW3:3,4")   // 主遊戲爆機分數
	PORT_DIPSETTING(      0x0004, "10,000" )
	PORT_DIPSETTING(      0x0000, "20,000" )
	PORT_DIPSETTING(      0x0008, "50,000" )
	PORT_DIPSETTING(      0x000c, "100.000" )
	PORT_DIPNAME( 0x0010, 0x0000, "Score Upper Limit" )             PORT_DIPLOCATION("SW3:5")     // 進分上限
	PORT_DIPSETTING(      0x0000, "10,000" )
	PORT_DIPSETTING(      0x0010, "20,000" )
	PORT_DIPNAME( 0x0020, 0x0000, "Mahjong Numbers" )               PORT_DIPLOCATION("SW3:6")     // 麻將數字 - only affects display when SW3:7,8 set to Mahjong Cards
	PORT_DIPSETTING(      0x0020, "Don't Show" )                                                  // 不顯示 - only shows characters for winds
	PORT_DIPSETTING(      0x0000, "Show" )                                                        // 顯示 - additionally shows numeric values for winds
	PORT_DIPNAME( 0x00c0, 0x0000, "Card Display" )                  PORT_DIPLOCATION("SW3:7,8")   // 畫面顯示 - changes in-game card face style
	PORT_DIPSETTING(      0x0000, "Poker Cards" )                                                 // 撲克畫面 - playing cards
	PORT_DIPSETTING(      0x0040, "Mahjong Cards" )                                               // 麻將畫面 - mahjong numbers and winds
	PORT_DIPSETTING(      0x0080, "Caishen Cards" )                                               // 財神財神 - plain coloured numbers
	PORT_DIPSETTING(      0x00c0, "Poker Cards" )                                                 // 撲克畫面 - playing cards
INPUT_PORTS_END

static INPUT_PORTS_START( sc2in1 )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start / Take" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME( "Hold 2 / Double Up" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME( "Hold 1 / Double Up / Big" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME( "Hold 3 / Double Up / Small" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)) // TODO: verify


	// Only 1 8-DIP bank on PCB. Dips' effects as per test mode.
	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW1:1")
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW1:2")
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW1:3")
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW1:4")
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW1:5")
	PORT_DIPNAME(          0x0020, 0x0000, "Game Setup" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(       0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(       0x0020, "Power On" )
	PORT_DIPNAME(          0x0040, 0x0000, "Game Password" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(       0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(       0x0040, "Power On" )
	PORT_DIPNAME(          0x0080, 0x0000, "Connector" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(       0x0000, "Joystick" ) // hardcoded
	PORT_DIPSETTING(       0x0080, "Joystick" )
INPUT_PORTS_END

static INPUT_PORTS_START( jinpaish )
	PORT_INCLUDE( sc2in1 )

	PORT_MODIFY("IN1") // TODO: likely incomplete
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Select" )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Show Card" )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// Only 1 8-DIP bank on PCB. Dips' effects as per test mode.
	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW1:1") // first 5 seem hardcoded to Single Player / No connection
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW1:2")
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW1:3")
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW1:4")
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW1:5")
	PORT_DIPNAME(          0x0020, 0x0000, "Display Game Title" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(               0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(               0x0000, DEF_STR( On ) )
	PORT_DIPNAME(          0x0040, 0x0000, "Game Password" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(               0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(               0x0040, "Power On" )
	PORT_DIPNAME(          0x0080, 0x0000, "Connector" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(               0x0000, "Joystick" )
	PORT_DIPSETTING(               0x0080, "Mahjong" )
INPUT_PORTS_END

static INPUT_PORTS_START( baile )
	PORT_INCLUDE( sc2in1 )

	PORT_MODIFY("IN1") // TODO: likely incomplete
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Tie Bet" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Player Bet" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Banker Bet" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Bet Modifier" )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME( "Flip Card / Show Odds" )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// Only 1 8-DIP bank on PCB. Most options appear to be software settings.
	PORT_MODIFY("DSW1")
	PORT_DIPNAME(           0x0001, 0x0000, DEF_STR( Test ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(                0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(                0x0001, DEF_STR( On ) )
	PORT_DIPNAME(           0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(                0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(                0x0002, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW1:7")
	PORT_DIPNAME(           0x0080, 0x0000, "Connector" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(                0x0000, "Joystick" )
	PORT_DIPSETTING(                0x0080, "Mahjong" )
INPUT_PORTS_END

static INPUT_PORTS_START( yyhm )
	PORT_INCLUDE( sc2in1 )

	PORT_MODIFY("IN1") // TODO: likely incomplete
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) // also used to select in test mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// Only 1 8-DIP bank on PCB. Dips' effects as per test mode.
	PORT_MODIFY("DSW1")
	PORT_DIPNAME(          0x0001, 0x0001, DEF_STR( Test ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(               0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(               0x0000, DEF_STR( On ) )
	PORT_DIPNAME(          0x0002, 0x0002, "Voice Announcements" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(               0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(               0x0002, DEF_STR( On ) )
	PORT_DIPNAME(          0x0004, 0x0004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(               0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(               0x0004, DEF_STR( On ) )
	PORT_DIPNAME(          0x0008, 0x0008, "Scoring Type" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(               0x0000, "Mahjong Tile Scoring" )
	PORT_DIPSETTING(               0x0008, "Numeric Scoring" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5") // No effect listed in test mode
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6") // "
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7") // "
	PORT_DIPNAME(          0x0080, 0x0000, "Connector" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(               0x0000, "Joystick" )
	PORT_DIPSETTING(               0x0080, "Mahjong" )
INPUT_PORTS_END

static INPUT_PORTS_START( ballch )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start / Stop" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) // play in test mode
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) // button A in test mode
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) // button B in test mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) // button C in test mode
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)) // TODO: verify


	// There are 3 8-DIP banks on PCB. Dips' effects as per test mode.
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0000, "Main Game Rate" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0001, "91" )
	PORT_DIPSETTING(      0x0002, "92" )
	PORT_DIPSETTING(      0x0003, "93" )
	PORT_DIPSETTING(      0x0004, "94" )
	PORT_DIPSETTING(      0x0005, "95" )
	PORT_DIPSETTING(      0x0000, "96" )
	PORT_DIPSETTING(      0x0006, "97" )
	PORT_DIPSETTING(      0x0007, "98" )
	PORT_DIPNAME( 0x0008, 0x0000, "Lamp Speed" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( High ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Score Feature" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Play Score" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Title" ) PORT_DIPLOCATION("SW1:7") // enables / disables the title screen, if disabled attract is always running
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8") // not shown in test mode
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0000, "Coin Rate" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0002, "10" )
	PORT_DIPSETTING(      0x0003, "25" )
	PORT_DIPSETTING(      0x0004, "50" )
	PORT_DIPSETTING(      0x0005, "75" )
	PORT_DIPSETTING(      0x0006, "100" )
	PORT_DIPSETTING(      0x0007, "500" )
	PORT_DIPNAME( 0x0038, 0x0000, "Key In Rate" ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0008, "5" )
	PORT_DIPSETTING(      0x0010, "10" )
	PORT_DIPSETTING(      0x0018, "25" )
	PORT_DIPSETTING(      0x0020, "50" )
	PORT_DIPSETTING(      0x0028, "100" )
	PORT_DIPSETTING(      0x0030, "500" )
	PORT_DIPSETTING(      0x0038, "1000" )
	PORT_DIPNAME( 0x0040, 0x0000, "Key Out Meter" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "Every 100" )
	PORT_DIPSETTING(      0x0000, "By Coin" )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8") // not shown in test mode
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0003, 0x0000, "Min. Bet" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPSETTING(      0x0002, "8" )
	PORT_DIPSETTING(      0x0003, "16" )
	PORT_DIPNAME( 0x000c, 0x0000, "Max. Bet" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x0000, "16" )
	PORT_DIPSETTING(      0x0004, "32" )
	PORT_DIPSETTING(      0x0008, "64" )
	PORT_DIPSETTING(      0x000c, "80" )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5") // not shown in test mode
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6") // not shown in test mode
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7") // not shown in test mode
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8") // not shown in test mode
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cots )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SLOT_STOP4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)) // TODO: verify


	// There are 3 8-DIP banks on PCB, but settings seem to be selected via test mode?
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sball2k1 ) // default password for accessing game settings is all Start
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_SERVICE_NO_TOGGLE(0x01, IP_ACTIVE_LOW)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)) // TODO: verify


	// There are 4 banks of 8 DIP switches on the PCB but only 1 is shown in test mode. DIP switch settings as per test mode. Other settings seem to be determined by software.
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0000, "Play Mode" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "Games" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Lives ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR ( Unused ) ) PORT_DIPLOCATION("SW1:2") // 'No Use' according to test mode
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Ability Game" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "Reg. Ability" ) PORT_DIPLOCATION("SW1:4") // sic. Regulate?
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "One Game" ) PORT_DIPLOCATION("SW1:5") // "One Partite" in test mode, seems a mix of English and Italian
	PORT_DIPSETTING(      0x0000, "10 Lives" )
	PORT_DIPSETTING(      0x0010, "20 Lives" )
	PORT_DIPNAME( 0x0020, 0x0000, "Game Setup" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, "Power On" ) // asks for password on start up, says to move the switch to On afterward
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Time Limit" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Comma" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, "No. 6" )
	PORT_DIPSETTING(      0x0000, "No. 5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0100, "SW4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW4:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW4:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW4:8" )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW2:8" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW3:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( cjdlz ) // TODO
	PORT_START("IN1")   // 16bit
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT  )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   // 16bit
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_TOGGLE
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_TOGGLE
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	// Only 4 DIP banks are actually populated on PCBs, but test mode reads all 6.
	// TODO: dips
	PORT_START("DSW1")   // 16bit, in test mode first 8 are recognized as dsw1, second 8 as dsw4.
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Controls" ) PORT_DIPLOCATION("DSW1:8") // should default to keyboard, but set on joystick since the former isn't emulated yet
	PORT_DIPSETTING(      0x0080, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0000, "Keyboard" )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )


	PORT_START("DSW2")   // 16bit, in test mode first 8 are recognized as dsw2, second 8 as dsw5
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")      // 16bit, in test mode first 8 are recognized as dsw3, second 8 as dsw6
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hgly )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 ) // start in test mode
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Paytable" )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Start Slot" )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(4)

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)) // TODO: verify

	// Only 4 DIP banks are actually populated on PCBs but test mode reads all 6.
	// TODO: DIPs
	PORT_START("DSW1")   // 16bit, in test mode first 8 are recognized as dsw1, second 8 as dsw4.
	PORT_DIPNAME( 0x0007, 0x0000, "Card Play Rate" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(      0x0001, "91" )
	PORT_DIPSETTING(      0x0002, "92" )
	PORT_DIPSETTING(      0x0003, "93" )
	PORT_DIPSETTING(      0x0004, "94" )
	PORT_DIPSETTING(      0x0005, "95" )
	PORT_DIPSETTING(      0x0000, "96" )
	PORT_DIPSETTING(      0x0006, "97" )
	PORT_DIPSETTING(      0x0007, "98" )
	PORT_DIPNAME( 0x0008, 0x0000, "Hold Card" ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Direct Double Up" ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Double Up Option" ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x0000, "Double Up Probability" ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e00, 0x0000, "Break Taiwan Restrictions" ) PORT_DIPLOCATION("DSW4:2,3,4") // TODO: fishy machine translations
	PORT_DIPSETTING(      0x0600, "5000" )
	PORT_DIPSETTING(      0x0800, "10000" )
	PORT_DIPSETTING(      0x0a00, "20000" )
	PORT_DIPSETTING(      0x0000, "30000" )
	PORT_DIPSETTING(      0x0c00, "50000" )
	PORT_DIPSETTING(      0x0e00, "90000" )
	PORT_DIPSETTING(      0x0200, "200000" )
	PORT_DIPSETTING(      0x0400, "500000" )
	PORT_DIPNAME( 0x3000, 0x0000, "Score Limit" ) PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(      0x0000, "5000" )
	PORT_DIPSETTING(      0x1000, "10000" )
	PORT_DIPSETTING(      0x2000, "30000" )
	PORT_DIPSETTING(      0x3000, "50000" )
	PORT_DIPNAME( 0x4000, 0x0000, "Double Explosive Machine" ) PORT_DIPLOCATION("DSW4:7") // TODO: fishy machine translations
	PORT_DIPSETTING(      0x4000, "10000" )
	PORT_DIPSETTING(      0x0000, "Unlimited" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:8") // not definition in test mode
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )


	PORT_START("DSW2")   // 16bit, in test mode first 8 are recognized as dsw2, second 8 as dsw5
	PORT_DIPNAME( 0x0007, 0x0000, "Coin Ratio" ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "5" )
	PORT_DIPSETTING(      0x0000, "10" )
	PORT_DIPSETTING(      0x0004, "20" )
	PORT_DIPSETTING(      0x0005, "50" )
	PORT_DIPSETTING(      0x0006, "100" )
	PORT_DIPSETTING(      0x0007, "300" )
	PORT_DIPNAME( 0x0018, 0x0000, "Coin x Score Multiplier" ) PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0010, "5" )
	PORT_DIPSETTING(      0x0000, "10" )
	PORT_DIPSETTING(      0x0018, "20" )
	PORT_DIPNAME( 0x0020, 0x0000, "Bonus Minimum Bet" ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0000, "32" )
	PORT_DIPSETTING(      0x0020, "64" )
	PORT_DIPNAME( 0x0040, 0x0000, "Payout Model" ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0040, "Key Out" )
	PORT_DIPSETTING(      0x0000, "Coin" )
	PORT_DIPNAME( 0x0080, 0x0000, "Pool Initial Score" ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0080, "500" )
	PORT_DIPSETTING(      0x0000, "1000" )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")      // 16bit, in test mode first 8 are recognized as dsw3, second 8 as dsw6
	PORT_DIPNAME( 0x0003, 0x0000, "Minimum Bet" ) PORT_DIPLOCATION("DSW3:1,2") // hard-coded
	PORT_DIPSETTING(      0x0000, "32" )
	PORT_DIPSETTING(      0x0001, "32 (duplicate)" )
	PORT_DIPSETTING(      0x0002, "32 (duplicate)" )
	PORT_DIPSETTING(      0x0003, "32 (duplicate)" )
	PORT_DIPNAME( 0x000c, 0x0000, "Minimum Bet" ) PORT_DIPLOCATION("DSW3:3,4") // semi hard-coded
	PORT_DIPSETTING(      0x0000, "200" )
	PORT_DIPSETTING(      0x0004, "200 (duplicate)" )
	PORT_DIPSETTING(      0x0008, "200 (duplicate)" )
	PORT_DIPSETTING(      0x000c, "360" )
	PORT_DIPNAME( 0x0010, 0x0000, "Bet Every Time" ) PORT_DIPLOCATION("DSW3:5") // hard-coded
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "4 (duplicate)" )
	PORT_DIPNAME( 0x0020, 0x0000, "Scoring" ) PORT_DIPLOCATION("DSW3:6") // TODO: fishy machine translations
	PORT_DIPSETTING(      0x0020, "Numbers" )
	PORT_DIPSETTING(      0x0000, "Tubes" )
	PORT_DIPNAME( 0x0040, 0x0040, "Controls" ) PORT_DIPLOCATION("DSW3:7") // should default to keyboard, but set on joystick since the former isn't emulated yet
	PORT_DIPSETTING(      0x0040, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0000, "Keyboard" )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:8") // not definition in test mode
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout rbmk32_layout =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
		16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32, 24*32, 25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32 },
	32*32
};

static const gfx_layout magslot32_layout =
{
	8,32,
	RGN_FRAC(1,1),
	8,
	{ 8, 9, 10, 11, 0, 1, 2, 3 },
	{ 4, 0, 20, 16, 36, 32, 52, 48,
	64+4, 64+0, 64+20, 64+16, 64+36, 64+32, 64+52, 64+48},
	{ STEP32(0,8*8) },
	32*64
};


static GFXDECODE_START( gfx_rbmk )
	GFXDECODE_ENTRY( "gfx1", 0, rbmk32_layout,            0x0, 32  )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_lsb,   0x100, 16  )
GFXDECODE_END

static GFXDECODE_START( gfx_magslot )
	GFXDECODE_ENTRY( "gfx1", 0, magslot32_layout,         0x000, 32  )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_lsb,     0x100, 16  )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x4_packed_lsb,     0x400, 16  )
GFXDECODE_END

void gms_2layers_state::video_start()
{
	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gms_2layers_state::get_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gms_2layers_state::get_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gms_2layers_state::get_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gms_2layers_state::get_reel_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gms_2layers_state::get_tile0_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	for (uint8_t i = 0; i < 4; i++)
	{
		m_reel_tilemap[i]->set_scroll_cols(64);
		m_reel_tilemap[i]->set_transparent_pen(0);
	}

	//m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[0]->set_transparent_pen(0);

	save_item(NAME(m_reels_toggle));
	save_item(NAME(m_tilebank));
	save_item(NAME(m_mux_data));
	save_item(NAME(m_input_matrix));
}

void gms_3layers_state::video_start()
{
	gms_2layers_state::video_start();

	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gms_3layers_state::get_tile1_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[1]->set_transparent_pen(0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(gms_2layers_state::get_reel_tile_info)
{
	const int tile = m_reelram[Which][tile_index];
	tileinfo.set(0, (tile & 0x0fff) + ((m_tilebank & 0x10) >> 4) * 0x1000, tile >> 12, 0);
}

TILE_GET_INFO_MEMBER(gms_2layers_state::get_tile0_info)
{
	const int tile = m_vidram[0][tile_index];
	tileinfo.set(1, (tile & 0x0fff) + ((m_tilebank >> 1) & 3) * 0x1000, tile >> 12, 0);
}

TILE_GET_INFO_MEMBER(gms_3layers_state::get_tile1_info)
{
	const int tile = m_vidram[1][tile_index];
	tileinfo.set(2, (tile & 0x0fff) + ((m_tilebank >> 9) & 3) * 0x1000, tile >> 12, 0);
}


uint32_t gms_2layers_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_reels_toggle == 0)
	{
		 // TODO: values hardcoded for cots, don't work correctly for magslot. Find the registers!
		const rectangle visible1(0 * 8, 64 * 8 - 1, 0 * 8, 10 * 8 - 1);
		const rectangle visible2(0 * 8, 64 * 8 - 1, 10 * 8, 18 * 8 - 1);
		const rectangle visible3(0 * 8, 64 * 8 - 1, 18 * 8, 24 * 8 - 1);
		const rectangle visible4(0 * 8, 64 * 8 - 1, 24 * 8, 32 * 8 - 1);

		for (int i = 3; i >=  0; i--)
			for (int j = 0; j < 64; j++)
				m_reel_tilemap[i]->set_scrolly(j, m_scrolly[i][j]);

		if (BIT(m_tilebank, 3) && BIT(m_tilebank, 5))
		{
			for (int i = 3; i >=  0; i--)
				m_reel_tilemap[i]->set_transparent_pen(0xff);
			 m_reel_tilemap[0]->draw(screen, bitmap, visible1);
			 m_reel_tilemap[1]->draw(screen, bitmap, visible2);
			 m_reel_tilemap[2]->draw(screen, bitmap, visible3);
			 m_reel_tilemap[3]->draw(screen, bitmap, visible4);
		}

		if (BIT(m_tilebank, 0))
			m_tilemap[0]->draw(screen, bitmap, cliprect);

		if (BIT(m_tilebank, 3) && !BIT(m_tilebank, 5))
		{
			for (int i = 3; i >=  0; i--)
				m_reel_tilemap[i]->set_transparent_pen(0x00);
			 m_reel_tilemap[0]->draw(screen, bitmap, visible1);
			 m_reel_tilemap[1]->draw(screen, bitmap, visible2);
			 m_reel_tilemap[2]->draw(screen, bitmap, visible3);
			 m_reel_tilemap[3]->draw(screen, bitmap, visible4);
		}

		if (BIT(m_tilebank, 11))
			if (m_tilemap[1])
				m_tilemap[1]->draw(screen, bitmap, cliprect);
	}
	else
	{
		for (int j = 0; j < 64; j++)
			m_reel_tilemap[3]->set_scrolly(j, m_scrolly[3][j]);

		if (BIT(m_tilebank, 3) && BIT(m_tilebank, 5))
		{
			m_reel_tilemap[3]->draw(screen, bitmap, cliprect);
			m_reel_tilemap[3]->set_transparent_pen(0xff);
		}

		if (BIT(m_tilebank, 0))
			m_tilemap[0]->draw(screen, bitmap, cliprect);

		if (BIT(m_tilebank, 3) && !BIT(m_tilebank, 5))
		{
			m_reel_tilemap[3]->draw(screen, bitmap, cliprect);
			m_reel_tilemap[3]->set_transparent_pen(0x00);
		}

		if (BIT(m_tilebank, 11))
			if (m_tilemap[1])
				m_tilemap[1]->draw(screen, bitmap, cliprect);
	}

	return 0;
}


void gms_2layers_state::rbmk(machine_config &config)
{
	M68000(config, m_maincpu, 22_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gms_2layers_state::rbmk_mem);
	m_maincpu->set_vblank_int("screen", FUNC(gms_2layers_state::irq1_line_hold));

	AT89C4051(config, m_mcu, 22_MHz_XTAL / 4); // frequency isn't right
	m_mcu->set_addrmap(AS_IO, &gms_2layers_state::mcu_io);
	m_mcu->port_out_cb<3>().set(FUNC(gms_2layers_state::mcu_io_mux_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rbmk);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(gms_2layers_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x800);

	EEPROM_93C46_16BIT(config, m_eeprom);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM6295(config, m_oki, 22_MHz_XTAL / 20, okim6295_device::PIN7_HIGH); // pin 7 not verified, but seems to match recordings
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 0.47);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 0.47);

	YM2151(config, m_ymsnd, 22_MHz_XTAL / 8);
	m_ymsnd->add_route(0, "lspeaker", 0.60);
	m_ymsnd->add_route(1, "rspeaker", 0.60);
}

void gms_2layers_state::rbspm(machine_config &config)
{
	rbmk(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &gms_2layers_state::rbspm_mem);

	PIC16F84(config, "pic", 4'000'000).set_disable(); // TODO: hook up, verify clock
}

void gms_2layers_state::ssanguoj(machine_config &config)
{
	rbmk(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &gms_2layers_state::ssanguoj_mem);

	m_mcu->set_disable(); // undumped internal ROM

	config.device_remove("ymsnd");

	ym3812_device &ym(YM3812(config, "ym3812", 22_MHz_XTAL / 8));
	ym.add_route(0, "lspeaker", 0.60);
	ym.add_route(1, "rspeaker", 0.60);
}

void gms_2layers_state::super555(machine_config &config)
{
	rbmk(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &gms_2layers_state::super555_mem);

	config.device_remove("mcu");
	config.device_remove("ymsnd");
}

void gms_3layers_state::magslot(machine_config &config)
{
	super555(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &gms_3layers_state::magslot_mem);

	m_gfxdecode->set_info(gfx_magslot);
}


// 实战麻将王 (Shízhàn Májiàng Wáng)
ROM_START( rbmk )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "p1.u64", 0x00000, 0x80000, CRC(83b3c505) SHA1(b943d7312dacdf46d4a55f9dc3cf92e291c40ce7) )

	ROM_REGION( 0x1000, "mcu", 0 ) // protected MCU?
	ROM_LOAD( "89c51.bin", 0x0, 0x1000, CRC(c6d58031) SHA1(5c61ce4eef1ef29bd870d0678bdba24e5aa43eae) )

	ROM_REGION( 0x20000, "user1", 0 ) // ??? MCU data / code
	ROM_LOAD( "b1.u72", 0x00000, 0x20000,  CRC(1a4991ac) SHA1(523b58caa21b4a073c664c076d2d7bb07a4253cd) )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "s1.u83", 0x00000, 0x40000, CRC(44b20e47) SHA1(54691af73aa5d20f9a9afe145447ef1cf34c9a0c) )

	ROM_REGION( 0x100000, "gfx1", 0 ) // 8x32 tiles, lots of girls etc.
	ROM_LOAD( "a1.u41", 0x00000, 0x100000,  CRC(1924de6b) SHA1(1a72ee2fd0abca51893f0985a591573bfd429389) )

	ROM_REGION( 0x80000, "gfx2", 0 ) // 8x8 tiles? cards etc
	ROM_LOAD( "t1.u39", 0x00000, 0x80000, CRC(adf67429) SHA1(ab03c7f68403545f9e86a069581dc3fc3fa6b9c4) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u51", 0x00, 0x080, CRC(4ca6ff01) SHA1(66c456eac5b0d1176ef9130baf2e746efdf30152) )
ROM_END

/*
实战頂凰麻雀 (Shízhàn Dǐng Huáng Máquè)
Gameplay videos:
http://youtu.be/pPk-6N1wXoE
http://youtu.be/VGbrR7GfDck
*/

ROM_START( rbspm ) // PCB NO.6899-B
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "mj-dfmj-p1.bin", 0x00000, 0x80000, CRC(8f81f154) SHA1(50a9a373dec96b0265907f053d068d636bdabd61) )

	ROM_REGION( 0x1000, "mcu", 0 ) // protected MCU
	ROM_LOAD( "mj-dfmj_at89c51.bin", 0x0000, 0x1000, CRC(c6c48161) SHA1(c3ecf998820d758286b18896ff7860221dd0cf43) ) // decapped

	ROM_REGION( 0x4280, "pic", 0 ) // pic was populated on this board
	ROM_LOAD( "c016_pic16f84_code.bin", 0x000, 0x800, CRC(1eb5cd2b) SHA1(9e747235e39eaea337f9325fa55fbfec1c03168d) )
	ROM_LOAD( "c016_pic16f84_data.bin", 0x800, 0x080, CRC(ee882e11) SHA1(aa5852a95a89b17270bb6f315dfa036f9f8155cf) )

	ROM_REGION( 0x20000, "user1", 0 ) // ??? MCU data / code
	ROM_LOAD( "mj-dfmj-2.2-xx.bin", 0x00000, 0x20000,  CRC(58a9eea2) SHA1(1a251e9b049bc8dafbc0728b3d876fdd5a1c8dd9) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "mj-dfmj-2.2-s1.bin", 0x00000, 0x80000, CRC(2410bb61) SHA1(54e258e4af089841a63e45f25aad70310a28d76b) )  // 1st and 2nd half identical

	ROM_REGION( 0x80000, "gfx1", 0 ) // 8x32 tiles, lots of girls etc.
	ROM_LOAD( "mj-dfmj-4.2-a1.bin", 0x00000, 0x80000,  CRC(b0a3a866) SHA1(cc950532160a066fc6ce427f6df9d58ee4589821) )

	ROM_REGION( 0x80000, "gfx2", 0 ) // 8x8 tiles? cards etc
	ROM_LOAD( "mj-dfmj-4.8-t1.bin", 0x00000, 0x80000, CRC(2b8b689d) SHA1(65ab643fac1e734af8b3a86caa06b532baafa0fe) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u51", 0x00, 0x080, NO_DUMP )
ROM_END


ROM_START( ssanguoj )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "sgc-pro_8.9_4af0.u64", 0x00000, 0x80000, CRC(c779e5c4) SHA1(d6c4833e4e2b3f8af1b7cf38fb4eef879259f214) )

	ROM_REGION( 0x1000, "mcu", 0 ) // protected MCU
	ROM_LOAD( "at89c51.bin", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x20000, "user1", 0 ) // ??? MCU data / code
	ROM_LOAD( "scg-m1.u72", 0x00000, 0x20000, CRC(9ad7be80) SHA1(c5d3a55520173662034650a4a8458200f47e76ac) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "mje2bf-s1.u83", 0x00000, 0x80000, CRC(dabb41a1) SHA1(ad809d8a4d5362a02b3ca49a75278943a824df1e) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "99a-a02_m5042j-a1.u41", 0x000000, 0x100000,  CRC(4b0823f4) SHA1(69e5448a9fe06430625c7c407ff4a7fd5b58d445) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "sgc-t1.u39", 0x00000, 0x80000, CRC(50776a8f) SHA1(141bd23fc237a0e8d31ae0504ea2b9cf39859319) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u51", 0x00, 0x080, NO_DUMP )
ROM_END


ROM_START( super555 ) // GMS branded chips: A66, A68, M06
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "super555-v1.5e-0d9b.u64", 0x00000, 0x80000, CRC(9a9c16cc) SHA1(95609dbd45feb591190a2b62dee8846cdcec3462) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "pk-s1-s06.u83", 0x00000, 0x80000, CRC(e329b9ce) SHA1(9fc31daaacc7b3a1a1cf99ab30035021b7cbb78f) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "pk-a1-a09.u41", 0x00000, 0x80000, CRC(f48e74bd) SHA1(68e2a0384964e04c526e4002ffae5fa4f2835d66) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "super555-t1-e67d.u39", 0x00000, 0x80000,  CRC(ee092a9c) SHA1(4123d45d21ca60b0d38f36f59353c56d4fdfcddf) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u138", 0x00, 0x080, CRC(60407223) SHA1(10f766b5431709ab11b16bf5ad7adbfdced0e7ac) )
ROM_END


ROM_START( sball2k1 ) // GMS branded chips: A66, A68, no stickers on ROMs
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "p1.u64", 0x00000, 0x80000, CRC(148fde1d) SHA1(4e846939d66bebb9a40b5a8a8d2b4c8b9ecdac4e) ) // M27C4002, 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "s1.u83", 0x00000, 0x20000, BAD_DUMP CRC(f668dc38) SHA1(b9ff20bd3675f591a46d71f7d7599a1005abc0b6) ) // TMS27C010A, proved difficult to dump, marked as bad as precaution

	ROM_REGION( 0x80000, "gfx1", ROMREGION_ERASE00 )
	// not populated

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "a1.u41", 0x00000, 0x20000,  CRC(8567a2f7) SHA1(18f187fb533a23fbb554b941361c9d3b03d1c0ce) ) // D27010

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u138", 0x00, 0x080, NO_DUMP )
ROM_END


/*******************************************************************
Sān Sè Cáishén (三色財神), GMS, 1999
Hardware Info by Guru
---------------------

M3-16-V3 89-01-A2
GAME MASTER SYSTEM CO.,LTD.
|-------------------------------------------------|
|         A1      T1     6116 6116                |
|                    6116         |-------| 18CV8 |
|                        6116 6116|       |       |
|-|                               |GMS-A68|  22MHz|
  |                               |       |       |
|-|                               |-------|       |
|                                      14.31818MHz|
|1      6116 6116                                 |
|8                                        |----|  |
|W                  |-----|               |M05 |  |
|A      SW1   SW3   |99A-A1  93C46        |----|  |
|Y         SW2  SW4 |A66  |    |-------|          |
|-|                 |-----|    |       |    62256 |
  |                            |68HC000|    62256 |
|-|  6295                      |FN16   |   P1     |
|          S1        TLP521    |-------|          |
|10  VOL             TLP521                       |
|WA      7805 T518B  TLP521                       |
|  uPC1241H          TLP521                   BATT|
|-|           JAMMA        |--|            SW5    |
  |------------------------|  |-------------------|
Notes:
    68HC000 - Motorola MC68HC000FN16 CPU. Clock 11.0MHz [22/2]
       6295 - OKI M6295 4-Channel ADPCM Voice Synthesis LSI. Clock input 1.100MHz [22/20]. Pin 7 HIGH
       6116 - 2kB x8-bit SRAM
      62256 - 32kB x8-bit SRAM (both battery-backed)
 99A-A1 A66 - GMS Custom PLCC44 Chip (CPLD)
        M05 - GMS Custom PLCC68 Chip (CPLD)
    GMS-A68 - GMS Custom PLCC44 Chip (CPLD)
     TLP521 - Toshiba TLP521 Photocoupler
   uPC1241H - NEC uPC1241H Audio Power Amp
       7805 - LM7805 5V Linear Regulator
      18CV8 - GAL18CV8 PLD
      93C46 - Atmel AT93C46 EEPROM
      T518B - Mitsumi PST518B Master Reset IC (TO92)
      SW1-4 - 8-position DIP Switch
        SW5 - Reset/Clear Switch
        VOL - Volume Pot
       BATT - CR2032 3V Lithium Battery
         P1 - 27C4096 EPROM (main program)
         S1 - 23C4000 mask ROM (oki samples)
      A1/T1 - 23C8000 mask ROM (gfx)

*******************************************************************/

ROM_START( sscs )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "p1_7177.u64", 0x00000, 0x80000, CRC(687ad5c8) SHA1(176a635753243882933e8db1aebcde142dc611f9 ) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1_sh-s1-s08.u83", 0x00000, 0x80000, CRC(9112ece2) SHA1(0ec9859b8925cdda2edfb93c8fc0d747933b365f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "a1_bj-a1-a06.u41", 0x000000, 0x100000, CRC(f758d95e) SHA1(d1da16f3ef618a8c1118784bdc39dd93acf86aff) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "t1_a11u-t07d.u39", 0x080000, 0x080000, CRC(f0ecbc72) SHA1(536288d21a5720111cb3392c974ee5ccdc4a2c6b) )
	ROM_CONTINUE(                 0x000000, 0x080000 ) // TODO: shouldn't be needed but the game doesn't seem to enable tile bank?

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u136", 0x00, 0x080, CRC(9ad1b39c) SHA1(2fed7e0918119b2354a9f1944d501dc817ffd5dc) )
ROM_END


// Basically same PCB as magslot, but with only 1 dip bank. Most labels have been covered with other labels with 'TETRIS' hand-written
// GMS-branded chips: A66, A89, A201, A202. Not populated: M88
ROM_START( sc2in1 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "u64", 0x00000, 0x80000, CRC(c0ad5df0) SHA1(a51f30e76493ea9fb5313c0064dac9a2a4f70cc3) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "u83", 0x00000, 0x80000, CRC(d7ff589b) SHA1(38e61dd7509862dec1299708da8785d1df713fe9) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "u178", 0x000000, 0x200000,  CRC(eaceb446) SHA1(db312f555e060eea6450f506cbbdca8874a05d58) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "u41", 0x00000, 0x40000, CRC(9ea462f7) SHA1(8cec497691f0121693a482b452ddf7a7dcedaf87) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "u169", 0x00000, 0x80000, CRC(f442fa70) SHA1(d06a84080e0196e1917b6f942adc29f97314be58) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "is93c46.u136", 0x00, 0x080, CRC(f0552ce8) SHA1(2dae746d9808d8a37f4f928dedda500063efdcfe) )
ROM_END

ROM_START( jinpaish ) // some of the labels were partly unreadable, all labels have 金牌 梭哈 before what's reported below
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "0922_1a59.u64", 0x00000, 0x80000, CRC(e0d9d814) SHA1(f3c9adabdfe517c2b944a82b36483af1088819b4) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1_dce4.u83", 0x00000, 0x80000, BAD_DUMP CRC(e236a02d) SHA1(21361739c2d9b62249dfccc176638a6f375c313c) ) // same as cots??? label seems original, but sounds are clearly wrong

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "a1.u178", 0x000000, 0x200000, CRC(eaceb446) SHA1(db312f555e060eea6450f506cbbdca8874a05d58) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t1_9269.u39", 0x00000, 0x80000, CRC(b87f62c0) SHA1(108c32271fb4802aec0606ff70d10be4fb0846bd) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "u1_2b6_.u169", 0x00000, 0x80000, CRC(31cdca7c) SHA1(eb60bc85408ecfc40dabac2b11f3d9bfc5467d3e) )
ROM_END

ROM_START( baile ) // all labels have 百乐 before what's reported below
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "2005_v3.2_918a.u64", 0x00000, 0x80000, CRC(f1d01a43) SHA1(a36af064cf45261d360bf3c8abc9c7a919fe40c0) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "2005_s1_78ac.u83", 0x00000, 0x80000, CRC(ab51ca24) SHA1(a867b52af2938779c83f4d5a24bc99ec7c2bf90e) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "2005_a1_1a5e.u178", 0x000000, 0x200000, CRC(0e338aeb) SHA1(8c645b0658bbbbd53bab7d769723abe08eee7acd) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "2005_t1_20cb.u39", 0x00000, 0x80000, CRC(bdb9a0d3) SHA1(0e8f675d244e7fe2eada90d02e836afc0e2840ca) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "2005_u1_7fe2.u169", 0x00000, 0x20000, CRC(d6216c9d) SHA1(693c6cd44e5d74f372ee3c8e5a0b1bd59f42bf22) )
ROM_END

ROM_START( yyhm ) // some of the labels were partly unreadable, all have 鸳鸯蝴蝶梦 before what's reported below
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "8a_9___.u64", 0x00000, 0x80000, CRC(ed1572fd) SHA1(d9bddc105b1c4eaa22226785d12152a058f283e6) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1_f___.u83", 0x00000, 0x80000, CRC(beaf22fb) SHA1(cccb547360a6b694bc3c976406ad36a5b7cc785d) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "a1_c___.u178", 0x000000, 0x200000, CRC(a8e8aad5) SHA1(7576549fc23d5863d0affc27717492199bda2a6f) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t1_aabe.u39", 0x00000, 0x80000, CRC(767bc6c3) SHA1(c1ccd6940e00c82278030a2c0875c411f1a0c1af) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "u1_333a.u169", 0x00000, 0x40000, CRC(4ceec182) SHA1(6c43db0ccf8f6c9c4350b072ebe7101cfbb1763f) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
ROM_END


// the PCB is slightly different from the others, both layout-wise and component-wise, but it's mostly compatible. It seems to use one more GFX layer and not to have the 89C51.
// GMS-branded chips: A66, A89, A201, A202. Not populated: M88
ROM_START( magslot ) // All labels have SLOT canceled with a black pen. No sum matches the one on label.
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "magic 1.0c _ _ _ _.u64", 0x00000, 0x80000, CRC(84544dd7) SHA1(cf10ad3373c2f35f5fa7986be0865f760a454c28) ) // no sum on label, 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "magic s1.0c ba8d.u83", 0x00000, 0x80000, CRC(46df3564) SHA1(6b740ca1fd839f7e7e35f097457e87d1260a6aaf) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "magic a1.0c _ _ _ _.u178", 0x000000, 0x200000,  CRC(11028627) SHA1(80b38acab1cd12462d8fc36a9cdce5e5e76f6403) ) // no sum on label, 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "magic t1.0c ec43.u41", 0x00000, 0x80000, CRC(18df608d) SHA1(753b8090e8fd89e50131a22259ef3280d7e6b282) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "magic u1.0c f7f6.u169", 0x00000, 0x40000, CRC(582631d3) SHA1(92d1b767bc7ef15eed6dad599392c17620210678) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "is93c46.u136", 0x00, 0x080, CRC(47ef702d) SHA1(269f3aff70cbf5144795b77953eb582d8c4da22a) )
ROM_END

/*
Creatures of the Sea
(c) 2005 ECM

GMS based 8-liner PCB

Major components:

  CPU: MC68HC00F16
Sound: OKI 6295
  OSC: 22.00MHz
EEPROM: ISSI 93C46
  DSW: 3 x 8-position switches
  BAT: 3.6v Varta battery

GMS branded chips:
 GMS-A201
 GMS-A202
 GMS-A89

GMS protection device labeled F.M.  200 (other COTS boards labeled FISHING M)

ROMs not labeled
*/

ROM_START( cots )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "3.u64", 0x00000, 0x80000, CRC(5a1a70d8) SHA1(356d93edd6af4bef72c3d613059a6658c9342d28) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "cos_s1_.u83", 0x00000, 0x80000, CRC(e236a02d) SHA1(21361739c2d9b62249dfccc176638a6f375c313c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "1_a1_.u41", 0x000000, 0x100000, CRC(0ca98ccd) SHA1(45f4c8a93d387f2790fee46c05597628ff238c2d) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "2_t1_.u39", 0x00000, 0x80000,  CRC(8c85dbc7) SHA1(c860949e5a61a4426b1409cefde9651c1d3a2765) )
ROM_END

/*
BALL CHALLENGE
(c) 2002 TVE

GMS based 8-liner PCB

Major components:

  CPU: MC68HC00F16
Sound: OKI 6295
  OSC: 22.00MHz
EEPROM: ISSI 93C46
  DSW: 3 x 8-position switches
  BAT: 3.6v Varta battery

GMS branded chips:
 GMS-A201
 GMS-A202
 GMS-A89

GMS protection device labeled B.CHALLENGE

ROMs labeled as:

ST 27C4002 @ U64:
 B.CHALLENGE
 0607 C757

ST 27C4001 @ U39:
 B.CHALLENGE
 T1 F4CB

ST 27C801 @ U41:
 B.CHALLENGE
 A1 0179

ST 27C4001 @ U83:
 B.CHALLENGE
 S1 15EF
*/

ROM_START( ballch )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "b.challenge_0607_c757.u64", 0x00000, 0x80000, CRC(d7c507e0) SHA1(482c06afb1ffeae99d43a1f4f50cbcd5f231c9bf) ) // 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "b.challenge_s1_15ef.u83", 0x00000, 0x80000, CRC(39c3bc0f) SHA1(1a0299f7774f7c95ee43858cf4f12b22eb652f02) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b.challenge_a1_0179.u41", 0x000000, 0x100000, CRC(b3c49a74) SHA1(a828fd007443ee08ece0c4cad80bd4f84471bb49) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b.challenge_t1_f4cb.u39", 0x00000, 0x80000,  CRC(a401072a) SHA1(f80ed4ef873393c36bb0446445bfb3a45e3efb97) )
ROM_END

/*
皇冠樂園 Huáng Guàn Lè Yuán (Crown Amusement Park), GMS, 1999
超级大连庄 Chāojí Dà Liánzhuāng, GMS, 1999
Hardware info by Guru
---------------------

GMS PCB 98-9-1 for Huang Guan Le Yuan (this layout below)
GMS PCB 98-8-2 for Chao Ji Da Lian Zhuang. 99.9% same board with minor part shuffling.
   |---------|        |-------------------|
|--|  10WAY  |--------|      18WAY        |--------------|
|UPC1241H VOL                                 14.31818MHz|
|                            S2.U72  |-----|     A2.U22  |
|                      PAL   S1.U83  | GMS |     A1.U41  |
|                     M6295          | M06 |             |
|                                    |-----|             |
|            ULN2003                               6116  |
|J           ULN2003                 T518B         6116  |
|A           SW1                                   6116  |
|M                                                 6116  |
|M           SW2                                   6116  |
|A                                        PAL            |
|            SW3                |-------|                |
|                               |GMS-A69|                |
|            SW4                |QFP100 |                |
|                   6116        |       |        T2.U29  |
|                   6116        |-------|                |
|                                                T1.U39  |
|                   24L257      |-------|                |
|                   24L257      |  GMS  |                |
|         68000         22MHz   |99A-A1 |  93C46    7805 |
| BATT         P1.U64           |  A66  |                |
|-------------------------------|-------|----------------|
Notes:
        68000 - Motorola MC68000FN-10 CPU. Clock 11.0MHz [22/2]
         6116 - 2kB x8-bit SRAM
       24L257 - Winbond W24L257 Low Voltage 32kB x8-bit SRAM (both chips are battery-backed)
        M6295 - OKI M6295 4-Channel ADPCM Voice Synthesis LSI. Clock input 1.100MHz [22/20]. Pin 7 HIGH
       GMSM06 - 89C51 microcontroller (or some variant of it) rebadged 'GMS M06'.
                Clock input 14.31818MHz on pins 20 and 21 (clock pins match 89C51/89C52/8751/8051)
                Note game is fully playable without this chip so maybe it's being used for payout or similar functions?
        93C46 - 93C46 EEPROM. The DI / DO pins are connected to custom chip GMS-99A-A1 A66
      ULN2003 - ULN2003 7-Channel Darlington Transistor Array
        T518B - Mitsumi PST518B Master Reset IC (TO92)
        SW1-4 - 8-position DIP Switch
     uPC1241H - NEC uPC1241H Audio Power Amp
         7805 - LM7805 5V Linear Regulator
         BATT - 3.6V Ni-Cad Battery. Maintains power to both 24L257 RAMs when main power supply is off.
                Note there is a position for a memory reset switch but it's not populated.
           P1 - 27C4096 (Main PRG)
  A1/A2/T1/T2 - 27C020/27C040 or 2M/4M mask ROM (GFX)
           S1 - 27C040 or 4M mask ROM (OKI samples)
           S2 - Not populated
           A2 - Not populated on Chao Ji Da Lian Zhuang
           T2 - Not populated on Huang Guan Le Yuan
      GMS-A69 - Custom graphics chip (QFP100)
                On Huang Guan Le Yuan this is GMS-A68 (PLCC84)
      GMS-99A - PLCC44 custom chip. Seems to be different for each GMS game that uses this chip and is likely
                to be a microcontroller with internal ROM. When identically marked chips are swapped between
                these two games the POST reports an error with this chip. When the chip is swapped back to
                the correct board it works fine. The same chip was also swapped from San Se Caishen and also
                shows this chip with a POST error.
*/

ROM_START( hgly )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "p1.u64", 0x00000, 0x80000, CRC(047c59f8) SHA1(f0dd39add2d28e80628e621c8c7053bde312ccd5) ) // 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "s1.u83", 0x00000, 0x80000, CRC(980ef3a3) SHA1(bf75e693ed25aa3ff704409491be7a399f7a9e08) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "a1.u41", 0x00000, 0x80000, CRC(179e85d6) SHA1(5224aced4769f1c7e43512650e5b67cc26210abe) )
	ROM_LOAD( "a2.u22", 0x80000, 0x80000, CRC(34f4449b) SHA1(797233bb9e8dcda7c07401414a3ed7f8a564e985) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t1.u39", 0x00000, 0x80000, CRC(272945c8) SHA1(89db8e23c58b185c1b4e44f74bcfef9b8c0baa04) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u136", 0x00, 0x080, CRC(7372eba5) SHA1(2ccaa4e4ffb8f3ff38f75f286e0ff8dc595a1541) )
ROM_END

ROM_START( cjdlz )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "p1.u64", 0x00000, 0x80000, CRC(e3379fff) SHA1(913bdb0f8bc2545bcd94a5738ec9190d55485961) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "mj-s1-s03.u83", 0x00000, 0x80000, CRC(27cf4e44) SHA1(ee7f3fbc0c9cc777cc4f5ef730c30b952ad61fbf) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "mj-a1-a07.u41", 0x00000, 0x80000, CRC(8f16d01b) SHA1(7dc0bb4ee230e1f1d61d055574e6790284369b8c) )

	ROM_REGION( 0xc0000, "gfx2", 0)
	ROM_LOAD( "rmj-t1-t05.u39", 0x00000, 0x80000, CRC(30638e20) SHA1(8082b7616ef759823be4265e902b503d15916197) )
	ROM_LOAD( "t2.u29",         0x80000, 0x40000, CRC(a7417ce3) SHA1(fb2a789169149f22af62d0c73cd2d652c7005f3e) ) // TODO: actually overlayed?

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u136", 0x00, 0x080, CRC(28d0db8c) SHA1(fb214d10f1c3a1f2e38cb22c620dcc314896ee54) )
ROM_END


	// the following inits patch out protection (?) checks to allow for testing
	// unfortunately the various U errors shown don't always correspond to correct PCB locations

void gms_2layers_state::init_rbspm()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	// 0x REPAIR
	rom[0x00520 / 2] = 0x600a;
	rom[0x00772 / 2] = 0x4e71;
	rom[0x00774 / 2] = 0x4e71;
	rom[0x1f1fc / 2] = 0x6000;
}

void gms_2layers_state::init_ssanguoj()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x2fc0 / 2] = 0x6000; // loops endlessly after ROM / RAM test
}

void gms_2layers_state::init_sball2k1()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x14f6c / 2] = 0x4e71; // U135 ERROR
	rom[0x14f6e / 2] = 0x4e71; // U135 ERROR
	rom[0x14f9a / 2] = 0x6000; // U136 ERROR
	rom[0x15528 / 2] = 0x4e71; // U135 ERROR
	rom[0x1552a / 2] = 0x4e71; // U135 ERROR
}

void gms_3layers_state::init_baile()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	// U135 ERROR
	rom[0xb494 / 2] = 0x6000;
	rom[0xb4a6 / 2] = 0x4e71;
	rom[0xb4a8 / 2] = 0x4e71;

	// U136 ERROR
	rom[0xb4ba / 2] = 0x6000;
	rom[0xb530 / 2] = 0x6000;
	rom[0xb542 / 2] = 0x4e71;
	rom[0xb544 / 2] = 0x4e71;
}

void gms_3layers_state::init_jinpaish()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	// U135 ERROR
	rom[0x319f0 / 2] = 0x4e71;
	rom[0x319f2 / 2] = 0x4e71;
	rom[0x31a0a / 2] = 0x6000;
	rom[0x31a1a / 2] = 0x4e71;
	rom[0x31a1c / 2] = 0x4e71;

	// U136 ERROR
	rom[0x31a4a / 2] = 0x6000;
	rom[0x31f38 / 2] = 0x6000;
	rom[0x31f4a / 2] = 0x4e71;
	rom[0x31f4c / 2] = 0x4e71;

	// U181 ERROR
	rom[0x31f64 / 2] = 0x6000;
	rom[0x31f74 / 2] = 0x4e71;
	rom[0x31f76 / 2] = 0x4e71;
}

void gms_3layers_state::init_sc2in1()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	// U135 ERROR
	rom[0x45f46 / 2] = 0x4e71;
	rom[0x45f48 / 2] = 0x4e71;
	rom[0x46818 / 2] = 0x4e71;
	rom[0x4681a / 2] = 0x4e71;

	rom[0x45f60 / 2] = 0x6000;

	// loops endlessly
	rom[0x45f70 / 2] = 0x4e71;
	rom[0x45f72 / 2] = 0x4e71;

	// U136 ERROR
	rom[0x45f9e / 2] = 0x6000;

	// U181 ERROR
	rom[0x46842 / 2] = 0x4e71;
	rom[0x46844 / 2] = 0x4e71;
}

void gms_3layers_state::init_yyhm()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	// REPAIR ERROR
	rom[0x9a2 / 2] = 0x6000;
	rom[0x9b4 / 2] = 0x4e71;
	rom[0x9b6 / 2] = 0x4e71;
	rom[0x9d4 / 2] = 0x6000;
	rom[0xb7a / 2] = 0x6000;
	rom[0xb8c / 2] = 0x4e71;
	rom[0xb8e / 2] = 0x4e71;
}

void gms_2layers_state::init_super555()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x46f54 / 2] = 0x6000; // loops endlessly after ROM / RAM test
	rom[0x474b4 / 2] = 0x4e71; // 0x09 U64 ERROR
	rom[0x4782e / 2] = 0x6000; // 0x0A U135 ERROR
}

void gms_2layers_state::init_ballch()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x12f0c / 2] = 0x4e71; // U135 ERROR
	rom[0x13212 / 2] = 0x4e71; // U64 U136 ERROR
}

void gms_2layers_state::init_cots()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x186c8 / 2] = 0x6000; // U64 U136 ERROR
	rom[0x198f6 / 2] = 0x62fe; // "
	rom[0x19566 / 2] = 0x62fe; // A88 ERROR U135 ERROR

	// the password to enter test mode is all Start
}

void gms_2layers_state::init_sscs()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x32b2 / 2] = 0x6000; // loops endlessly after ROM / RAM test
	rom[0xcc1c / 2] = 0x6000; // U135 ERROR
	rom[0xcc2e / 2] = 0x4e71; // U135 ERROR
	rom[0xcc30 / 2] = 0x4e71; // U135 ERROR
	rom[0xcc5e / 2] = 0x6000; // U136 ERROR
	rom[0xce2a / 2] = 0x6000; // U136 ERROR
	rom[0xce3c / 2] = 0x4e71; // U136 ERROR
	rom[0xce3e / 2] = 0x4e71; // U136 ERROR
	rom[0x19c1a / 2] = 0x6000; // U85 ERROR
}

void gms_2layers_state::init_cjldz()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();


	rom[0x00518 / 2] = 0x4e71; // 0xD REPAIR
	rom[0x0c628 / 2] = 0x6000; // 0x99 REPAIR
	rom[0x0c8e6 / 2] = 0x4e71; // loop
	rom[0x0ca00 / 2] = 0x6000; // 0xA REPAIR
	rom[0x0ca24 / 2] = 0x4e71; // 0xE REPAIR
	rom[0x38664 / 2] = 0x6000; // 0xD REPAIR
	rom[0x38980 / 2] = 0x6000; // 0xD REPAIR
}

void gms_2layers_state::init_hgly()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x0feda / 2] = 0x6004; // U35 ERROR
	rom[0x10128 / 2] = 0x6004; // U36 ERROR
	rom[0x1393e / 2] = 0x6000; // U64 ERROR
}

} // anonymous namespace


// mahjong
GAME( 1998, rbmk,     0, rbmk,     rbmk,     gms_2layers_state, empty_init,    ROT0,  "GMS", "Shizhan Majiang Wang (Version 8.8)",                    MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // misses YM2151 hookup
GAME( 1998, rbspm,    0, rbspm,    rbspm,    gms_2layers_state, init_rbspm,    ROT0,  "GMS", "Shizhan Ding Huang Maque (Version 4.1)",                MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Misses YM2151 hookup
GAME( 1998, ssanguoj, 0, ssanguoj, ssanguoj, gms_2layers_state, init_ssanguoj, ROT0,  "GMS", "Shizhan Sanguo Ji Jiaqiang Ban (Version 8.9 980413)",   MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // stops during boot, patched for now. YM3812 isn't hooked up (goes through undumped MCU).
GAME( 1999, cjdlz,    0, super555, cjdlz,    gms_2layers_state, init_cjldz,    ROT0,  "GMS", "Chaoji Da Lianzhuang (Version 1.1)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Also needs EEPROM support.
GAME( 2005, yyhm,     0, magslot,  yyhm,     gms_3layers_state, init_yyhm,     ROT0,  "GMS", "Yuanyang Hudie Meng (Version 8.8A 2005-09-25)",         MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Also needs EEPROM support.

// card games
GAME( 1999, super555, 0, super555, super555, gms_2layers_state, init_super555, ROT0,  "GMS", "Super 555 (English version V1.5)",                      MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Also needs EEPROM support.
GAME( 1999, sscs,     0, super555, sscs,     gms_2layers_state, init_sscs,     ROT0,  "GMS", "San Se Caishen (Version 0502)",                         MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Also needs EEPROM support.
GAME( 2001, sball2k1, 0, super555, sball2k1, gms_2layers_state, init_sball2k1, ROT0,  "GMS", "Super Ball 2001 (Italy version 5.23)",                  MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Also needs EEPROM support.
GAME( 2001, sc2in1,   0, magslot,  sc2in1,   gms_3layers_state, init_sc2in1,   ROT0,  "GMS", "Super Card 2 in 1 (English version 03.23)",             MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Also needs EEPROM support.
GAME( 2004, jinpaish, 0, magslot,  jinpaish, gms_3layers_state, init_jinpaish, ROT0,  "GMS", "Jinpai Suoha - Show Hand (Chinese version 2004-09-22)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Also needs EEPROM support. Also needs correct controls.
GAME( 2005, baile,    0, magslot,  baile,    gms_3layers_state, init_baile,    ROT0,  "GMS", "Baile 2005 (V3.2 2005-01-12)",                          MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Also needs EEPROM support.

// slots
GAME( 2003, magslot,  0, magslot,  magslot,  gms_3layers_state, empty_init,    ROT0,  "GMS", "Magic Slot (normal 1.0C)",                              MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // reel / tilemaps priorities are wrong, inputs to be verified. Also needs EEPROM support.

// train games
GAME( 1999, hgly,     0, super555, hgly,     gms_2layers_state, init_hgly,     ROT0,  "GMS", "Huang Guan Le Yuan (990726 CRG1.1)",                    MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Also needs EEPROM support.
GAME( 2002, ballch,   0, super555, ballch,   gms_2layers_state, init_ballch,   ROT0,  "TVE", "Ball Challenge (20020607 1.0 OVERSEA)",                 MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Also needs EEPROM support.
GAME( 2005, cots,     0, super555, cots,     gms_2layers_state, init_cots,     ROT0,  "ECM", "Creatures of the Sea (20050328 USA 6.3)",               MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // stops during boot, patched for now. Also needs EEPROM support.
