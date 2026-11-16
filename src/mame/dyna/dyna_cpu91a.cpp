// license:BSD-3-Clause
// copyright-holders: David Haywood, Roberto Zandona, Peter Wilhelmsen

/*************************************************************************************************

This driver covers Dyna games running on the DYNA CPU91A-011 custom CPU.
It is an encrypted NEC V25. It has been seen on the following PCBs:
D9203, D9205 or D9205B (sub PCB), D9304, D9401, D9701 (sub PCB), D9702 and D9805

Cherry Bonus 2001  (c)2000/2001 Dyna

Produttore  Dyna
N.revisione
CPU

1x DYNA CPU91A-011-0016JK004 (QFP84) custom
1x DYNA DC3001-0051A (QFP128) custom
1x DYNA 22A078803 (DIP42) (similar to an I8255)
1x WINBOND WF19054 (equivalent to AY-3-8910)
1x oscillator 24.000MHz

ROMs

1x M27C4002 (12a)
1x M27C1001 (11f)
2x AM27S29PC (9b,11b)
2x GAL16V8D (not dumped)

Note

1x 36x2 edge connector
1x 10x2 edge connector
1x trimmer (volume)
1x pushbutton (sw8)
1x battery
7x 8x switches dip (sw1-7)
------------------------------

In title screen (c) is 2001
In test mode (c) is 2000

------------------------------

To enter input test, keep '9' pressed and press 'F3'.

TODO:
- complete CPU decryption and verify the provisional 3b, 3c, 79, 7c and d0 mappings;
- investigate crzybell's attendant fallback: E2593 decreases credits without
  updating the retained copy, unlike the ordinary Key Out path at E2444;
- the DC3001 GFX custom is suspected to have internal ROM (seems used by scherrym and clones
  and possibly by pdouble8 for the reels). nmondop also selects a reel bank beyond
  its dumped graphics ROM; determine whether this is internal ROM or different banking;
- dynastye uses a bitmap blitter rather than the later tile/reel hardware. Verify
  its page/unused-layer controls, busy timing, I/O controller and sound clocks;
- verify remaining I/O assignments, printer interfaces and sound clocks.

*************************************************************************************************/
#include "emu.h"

#include "cpu/mcs51/i80c51.h"
#include "cpu/nec/v25.h"
#include "machine/eeprom28.ipp"
#include "machine/i8255.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>
#include <array>
#include <iterator>


// configurable logging
#define LOG_VIDEOREGS     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_VIDEOREGS)

#include "logmacro.h"

#define LOGVIDEOREGS(...)     LOGMASKED(LOG_VIDEOREGS,     __VA_ARGS__)


namespace {

// D9902 programs 128-byte pages using AA/55/A0 software-protection commands.
// The dump contains the initial settings at 0200 as well as the program code.
// The exact parallel EEPROM/flash part and its timings have not been identified.
class bsbonanza_eeprom_device : public eeprom28_nvram_device<18, 128, 150, 5'000>
{
public:
	bsbonanza_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(BSBONANZA_EEPROM, bsbonanza_eeprom_device, bsbonanza_eeprom_device, "bsbonanza_eeprom", "Bonanza program EEPROM")

namespace {

bsbonanza_eeprom_device::bsbonanza_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	eeprom28_nvram_device(mconfig, BSBONANZA_EEPROM, tag, owner, clock)
{
}

class cb2001_state : public driver_device
{
public:
	cb2001_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vram_fg(*this, "vrafg"),
		m_vram_bg(*this, "vrabg"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void cb2001(machine_config &config) ATTR_COLD;
	void scherrym(machine_config &config) ATTR_COLD;

	void init_smaller_proms() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void base(machine_config &config) ATTR_COLD;
	void cb5(machine_config &config) ATTR_COLD;
	void scherrymp(machine_config &config) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
	template <uint32_t NvramSize> void nvram_map(address_map &map) ATTR_COLD;
	void cb5_io_map(address_map &map) ATTR_COLD;
	void scherrym_base_io_map(address_map &map) ATTR_COLD;
	void scherrym_io_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint16_t> m_vram_fg;
	required_shared_ptr<uint16_t> m_vram_bg;
	required_device<v35_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

private:
	uint8_t m_videobank = 0;
	uint8_t m_reel_colour = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_reel_tilemap[4]{};
	uint8_t m_other1 = 0;
	uint8_t m_other2 = 0;

	void vidctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vidctrl2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_reel_tile_info);
	void palette_init(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	uint8_t irq_ack_r();
};

class pdouble8_state : public cb2001_state
{
public:
	pdouble8_state(const machine_config &mconfig, device_type type, const char *tag) :
		cb2001_state(mconfig, type, tag),
		m_hopper(*this, "hopper"),
		m_inputs(*this, "IN%u", 0U),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void pdouble8(machine_config &config) ATTR_COLD;

private:
	required_device<ticket_dispenser_device> m_hopper;
	required_ioport_array<3> m_inputs;
	output_finder<8> m_lamps;

	uint8_t input_r(offs_t offset);
	void lamps_w(uint8_t data);
	void output_w(uint8_t data);
	void io_map(address_map &map) ATTR_COLD;
};

class cb4_state : public cb2001_state
{
public:
	cb4_state(const machine_config &mconfig, device_type type, const char *tag) :
		cb2001_state(mconfig, type, tag),
		m_hopper(*this, "hopper"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void cb4(machine_config &config) ATTR_COLD;

protected:
	void cherry_bonus(machine_config &config) ATTR_COLD;

private:
	required_device<ticket_dispenser_device> m_hopper;
	output_finder<16> m_lamps;

	void output_w(uint8_t data);
	template <uint32_t Player> void lamps_w(uint8_t data);
	void io_map(address_map &map) ATTR_COLD;
};

class cb5_state : public cb4_state
{
public:
	cb5_state(const machine_config &mconfig, device_type type, const char *tag) :
		cb4_state(mconfig, type, tag)
	{ }

	void cb5(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	std::array<emu_timer *, 5> m_counter_timers{};
	uint8_t m_counter_state = 0;

	void counters_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(counter_tick);
};

class cherry2001_state : public cb2001_state
{
public:
	cherry2001_state(const machine_config &mconfig, device_type type, const char *tag) :
		cb2001_state(mconfig, type, tag),
		m_hopper(*this, "hopper")
	{ }

	void cherry2001(machine_config &config) ATTR_COLD;

private:
	required_device<ticket_dispenser_device> m_hopper;

	void output_w(uint8_t data);
	void io_map(address_map &map) ATTR_COLD;
};

class scherrymp_state : public cb2001_state
{
public:
	scherrymp_state(const machine_config &mconfig, device_type type, const char *tag) :
		cb2001_state(mconfig, type, tag),
		m_hopper(*this, "hopper")
	{ }

	void scherrymp(machine_config &config) ATTR_COLD;

protected:
	void output_w(uint8_t data);
	void io_map(address_map &map) ATTR_COLD;

private:
	required_device<ticket_dispenser_device> m_hopper;
};

class mystjb_state : public scherrymp_state
{
public:
	mystjb_state(const machine_config &mconfig, device_type type, const char *tag) :
		scherrymp_state(mconfig, type, tag),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void mystjb(machine_config &config) ATTR_COLD;
	void nmondop(machine_config &config) ATTR_COLD;
	void ndongmul2(machine_config &config) ATTR_COLD;
	void tripjack(machine_config &config) ATTR_COLD;
	void tripjack16(machine_config &config) ATTR_COLD;

private:
	output_finder<8> m_lamps;

	void tripjack_io_map(address_map &map) ATTR_COLD;
	void lamps_w(uint8_t data);
	void output_w(uint8_t data);
	void io_map(address_map &map) ATTR_COLD;
};

// D9401 and D9902 store tile codes and attributes in separate byte planes.
class dyna_plane_base : public cb2001_state
{
protected:
	dyna_plane_base(const machine_config &mconfig, device_type type, const char *tag) :
		cb2001_state(mconfig, type, tag),
		m_hopper(*this, "hopper"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	required_device<ticket_dispenser_device> m_hopper;

	void plane(machine_config &config) ATTR_COLD;
	void lamps_w(uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	template <uint32_t Layer> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void video_regs_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	output_finder<8> m_lamps;

	uint8_t m_video_regs[7]{};
	tilemap_t *m_layer_tilemap[2]{}; // foreground, background
	tilemap_t *m_reel_tilemap = nullptr;

	static uint8_t vram_byte(const uint16_t *ram, offs_t offset);
	template <uint32_t Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_reel_tile_info);
};

class crzybell_state : public dyna_plane_base
{
public:
	crzybell_state(const machine_config &mconfig, device_type type, const char *tag) :
		dyna_plane_base(mconfig, type, tag)
	{ }

	void crzybell(machine_config &config) ATTR_COLD;

private:
	void output_w(uint8_t data);
	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};

class bsbonanza_state : public dyna_plane_base
{
public:
	bsbonanza_state(const machine_config &mconfig, device_type type, const char *tag) :
		dyna_plane_base(mconfig, type, tag),
		m_eeprom(*this, "boot_prg")
	{ }

	void bsbonanza(machine_config &config) ATTR_COLD;

private:
	required_device<bsbonanza_eeprom_device> m_eeprom;

	void output_w(uint8_t data);
	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};

// Opcode bytes and prefixes are encrypted; operands and NEC extension bytes are not.
// TODO: resolve unknown entries and distinguish duplicate mappings.
// Five mappings are PROVISIONAL and UNCONFIRMED in both the CPU and debugger:
//   3b -> 10 (ADC r/m8,r8): Crazy Bell uses DL,DL; the opposite direction and
//         RCL DL,1 produce the same observed result, with flags overwritten.
//   3c -> 3e (DS0 override): the observed Triple Jack/Bonanza uses have DS0=DS1.
//   79 -> 9b (WAIT/POLL): Dynasty brackets blits with it; busy wiring is unknown.
//   7c -> 68 (PUSH imm16): all observed immediate operands are zero.
//   d0 -> 72 (JB/BC): Bonanza's small positive operands also fit signed branches.
// Unknown entries retain the placeholder NOP; they are not established NOPs.
constexpr uint8_t UNKN = 0x90;

constexpr uint8_t CPU91A_DECRYPTION_TABLE[256] =
{
	0xe8, UNKN, 0xf7, UNKN, 0x80, 0xe4, 0x12, 0x2f, 0x3c, UNKN, UNKN, 0x0b, UNKN, UNKN, UNKN, 0x5f, // 00
	0x86, UNKN, UNKN, 0x27, 0x1c, 0xfa, UNKN, UNKN, 0x32, 0x40, 0xa0, 0xd3, 0x3a, 0x14, 0x89, 0x1f, // 10
	0x08, 0x8e, UNKN, 0x0f, UNKN, 0x49, 0xb5, UNKN, 0x56, UNKN, UNKN, 0x75, 0x33, 0xb6, 0x01, 0x3b, // 20
	0x39, 0x5d, UNKN, UNKN, UNKN, 0x22, 0x5b, UNKN, UNKN, 0x4a, 0xe3, 0x10, 0x3e, 0xa6, UNKN, 0x74, // 30
	UNKN, 0xea, UNKN, UNKN, 0xd0, 0xb0, 0x5e, UNKN, UNKN, 0xa2, 0x90, UNKN, 0xa3, UNKN, UNKN, 0xb3, // 40
	0x13, UNKN, 0x2c, UNKN, 0x9d, UNKN, 0x42, 0xc0, 0x04, UNKN, 0xb7, UNKN, 0xeb, 0xab, 0x2d, UNKN, // 50
	UNKN, UNKN, UNKN, UNKN, 0x0a, UNKN, UNKN, UNKN, 0xa1, 0xa5, UNKN, UNKN, UNKN, 0xbb, 0xba, 0x8c, // 60
	0xc3, 0x53, 0x02, 0x58, 0x23, UNKN, 0x24, 0xbd, 0x72, 0x9b, 0xf3, UNKN, 0x68, 0x43, UNKN, 0x0c, // 70
	0x26, UNKN, 0xd1, UNKN, UNKN, 0x3d, 0xfb, 0xf6, UNKN, 0x55, 0x59, UNKN, 0x73, UNKN, 0x2a, 0xf8, // 80
	UNKN, 0x25, 0xe9, UNKN, UNKN, 0xbe, 0xf9, UNKN, UNKN, UNKN, 0x57, UNKN, 0xb9, UNKN, 0xbf, UNKN, // 90
	0xc1, 0x76, 0xe6, 0x06, 0xaa, 0x9c, 0xad, 0xb8, 0x4e, UNKN, 0x8d, 0x50, 0x51, 0xa4, 0x36, 0x1a, // A0
	0xac, UNKN, 0xb4, 0x77, UNKN, 0x83, UNKN, UNKN, UNKN, 0x05, 0x03, UNKN, 0x1e, 0x4b, 0x07, 0xcf, // B0
	0xcb, 0xec, 0xee, 0x2b, 0x41, 0xe2, 0x87, UNKN, UNKN, 0xfd, 0x76, 0x61, 0x48, UNKN, 0x2e, 0x6c, // C0
	0x72, 0xf2, 0x46, UNKN, 0x60, 0x77, 0x4f, 0x47, 0x88, UNKN, UNKN, 0xff, 0xf5, 0xfc, 0xc7, 0x8b, // D0
	0x8a, 0xb1, 0x45, 0xc6, 0x79, 0x5a, UNKN, 0xb2, 0x9a, 0x52, UNKN, UNKN, UNKN, 0x6e, UNKN, UNKN, // E0
	UNKN, 0xae, 0xfe, 0x6a, UNKN, UNKN, UNKN, 0x3a, UNKN, UNKN, 0x34, UNKN, 0x81, UNKN, UNKN, UNKN, // F0
};

void configure_cpu_decryption(v25_common_device &cpu) ATTR_COLD;

void configure_cpu_decryption(v25_common_device &cpu)
{
	cpu.set_decryption_table(CPU91A_DECRYPTION_TABLE);
	cpu.set_unconfirmed_decryption(0x3b);
	cpu.set_unconfirmed_decryption(0x3c);
	cpu.set_unconfirmed_decryption(0x79);
	cpu.set_unconfirmed_decryption(0x7c);
	cpu.set_unconfirmed_decryption(0xd0);
}

void cb2001_state::machine_start()
{
	save_item(NAME(m_videobank));
	save_item(NAME(m_reel_colour));
	save_item(NAME(m_other1));
	save_item(NAME(m_other2));
}


uint32_t cb2001_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	// render bg as 8x8 tilemaps
	if (m_other1 & 0x02)
	{
		if (!(m_other1 & 0x04))
		{
			m_bg_tilemap->draw(screen, bitmap, cliprect);
		}
		else
		{
			for (uint32_t reel = 0; reel < std::size(m_reel_tilemap); ++reel)
			{
				for (uint32_t col = 0; col < 64; ++col)
				{
					uint16_t const scroll = m_vram_bg[(0x800 + reel * 0x200 + col) / 2];
					m_reel_tilemap[reel]->set_scrolly(col, (scroll >> ((col & 1) * 8)) & 0xff);
				}
			}

			// CB2001 E3167 uploads 256 scanline selectors to 21f00. Bit 7
			// enables the line and bits 0-1 choose one of four reel maps.
			// The normal table at E87A2 uses reels 0-2; E88A6 uses reel 3.
			auto const selector = [this] (int y) -> uint8_t
			{
				return m_vram_bg[(0xf00 + (y & 0xff)) / 2] >> ((y & 1) * 8);
			};
			for (int y = cliprect.top(); y <= cliprect.bottom(); )
			{
				uint8_t const select = selector(y);
				int last = y;
				while (last < cliprect.bottom() && selector(last + 1) == select)
					++last;
				if (BIT(select, 7))
					m_reel_tilemap[select & 3]->draw(screen, bitmap, rectangle(cliprect.left(), cliprect.right(), y, last));
				y = last + 1;
			}
		}
	}

	if (BIT(m_other1, 0))
		m_fg_tilemap->draw(screen, bitmap, cliprect);

	LOGVIDEOREGS("%02x %02x %02x %02x\n", m_videobank, m_reel_colour, m_other1, m_other2);

	return 0;
}


// Control bits share a word with the foreground tile-bank selector.
void cb2001_state::vidctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		LOGVIDEOREGS("%s: vidctrl_w %04x %04x\n", machine().describe_context(), data, mem_mask);
		uint8_t const bank = BIT(data, 10, 3);
		if (m_videobank != bank)
		{
			m_videobank = bank;
			m_fg_tilemap->mark_all_dirty();
		}
	}
	if (ACCESSING_BITS_0_7)
		m_other1 = data & 0x00ff;
}

void cb2001_state::vidctrl2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	bool reels_dirty = false;
	if (ACCESSING_BITS_8_15)
	{
		// nmondop DDD22-DDD4A splits a reel tile attribute into a bank
		// at OUT32 and a four-bit palette at OUT33. scherrymp's gallery
		// likewise selects palettes 0e/0f for the artwork in banks 11-17.
		uint8_t const colour = BIT(data, 8, 4);
		reels_dirty = m_reel_colour != colour;
		m_reel_colour = colour;
	}
	if (ACCESSING_BITS_0_7)
	{
		if ((m_other2 ^ data) & 0x1c)
			m_bg_tilemap->mark_all_dirty();
		reels_dirty |= bool((m_other2 ^ data) & 0x1f);
		m_other2 = data & 0x00ff;
	}
	if (reels_dirty)
	{
		for (auto &tilemap : m_reel_tilemap)
			tilemap->mark_all_dirty();
	}
}


TILE_GET_INFO_MEMBER(cb2001_state::get_bg_tile_info)
{
	// scherrymp C5778 changes foreground bank 31 to 0c, retaining background
	// bank 32 = 1c for the title animation. The two tile banks are independent.
	int const code = (m_vram_bg[tile_index] & 0x0fff) | ((m_other2 & 0x1c) << 10);
	int const colour = BIT(m_vram_bg[tile_index], 12, 4);

	tileinfo.set(0, code, colour, 0);
}

TILE_GET_INFO_MEMBER(cb2001_state::get_fg_tile_info)
{
	int const code = (m_vram_fg[tile_index] & 0x0fff) | (m_videobank * 0x1000);
	int const colour = BIT(m_vram_fg[tile_index], 12, 4);

	tileinfo.set(0, code, colour, 0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(cb2001_state::get_reel_tile_info)
{
	int code = m_vram_bg[(Which * 0x200 / 2) + tile_index / 2];

	if (tile_index & 1)
		code >>= 8;

	code &= 0xff;

	// Reel maps contain eight-bit codes, so OUT32 bits 0-4 select 32 KiB
	// banks. Unlike the 8x8 maps, they need the low two bank bits as well:
	// mystjb uses 28 for its symbols and 29 for the Cherry Bonus numbers.
	int const reel_bank = (m_other2 & 0x1f) << 8;

	tileinfo.set(1, code | reel_bank, m_reel_colour, 0);
}


void cb2001_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cb2001_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cb2001_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);

	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cb2001_state::get_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cb2001_state::get_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cb2001_state::get_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cb2001_state::get_reel_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	for (auto &tilemap : m_reel_tilemap)
		tilemap->set_scroll_cols(64);
}

void cb2001_state::bg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t const old_data = m_vram_bg[offset];
	COMBINE_DATA(&m_vram_bg[offset]);
	uint16_t const changed = old_data ^ m_vram_bg[offset];
	if (!changed)
		return;

	m_bg_tilemap->mark_tile_dirty(offset);

	// The first 0x800 bytes also contain four reels, with two tiles per word.
	// Scroll-table writes do not change cached tile contents.
	if (offset < 0x800 / 2)
	{
		tilemap_t &reel = *m_reel_tilemap[offset / (0x200 / 2)];
		offs_t const tile = (offset % (0x200 / 2)) * 2;
		if (changed & 0x00ff)
			reel.mark_tile_dirty(tile);
		if (changed & 0xff00)
			reel.mark_tile_dirty(tile + 1);
	}
}

void cb2001_state::fg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram_fg[offset]);

	m_fg_tilemap->mark_tile_dirty(offset);
}

// D9401/D9902 use separate byte planes: 000-7ff holds tile codes, 800-fff
// holds the high code nibble and colour. In reel mode, background 000-1ff
// is a single 64x8 map of 8x32 tiles and 800-83f holds column scroll values.
uint8_t dyna_plane_base::vram_byte(const uint16_t *ram, offs_t offset)
{
	return ram[offset >> 1] >> ((offset & 1) * 8);
}

void dyna_plane_base::machine_start()
{
	cb2001_state::machine_start();
	save_item(NAME(m_video_regs));
}

void dyna_plane_base::video_start()
{
	m_layer_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dyna_plane_base::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_layer_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dyna_plane_base::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_layer_tilemap[0]->set_transparent_pen(0);
	m_reel_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dyna_plane_base::get_reel_tile_info)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap->set_scroll_cols(64);
}

template <uint32_t Layer>
TILE_GET_INFO_MEMBER(dyna_plane_base::get_tile_info)
{
	const uint16_t *const ram = Layer ? m_vram_bg.target() : m_vram_fg.target();
	uint8_t const attr = vram_byte(ram, 0x800 + tile_index);
	int const code = vram_byte(ram, tile_index) | ((attr & 0x0f) << 8) | ((m_video_regs[1 + Layer] & 0x1c) << 10);
	tileinfo.set(0, code, attr >> 4, 0);
}

TILE_GET_INFO_MEMBER(dyna_plane_base::get_reel_tile_info)
{
	int const code = vram_byte(m_vram_bg, tile_index) | ((m_video_regs[2] & 0x1c) << 8);
	tileinfo.set(1, code, 0, 0);
}

template <uint32_t Layer>
void dyna_plane_base::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t *const ram = Layer ? m_vram_bg.target() : m_vram_fg.target();
	uint16_t const previous = ram[offset];
	COMBINE_DATA(&ram[offset]);
	uint16_t const changed = previous ^ ram[offset];
	for (uint32_t lane = 0; lane < 2; ++lane)
	{
		if (changed & (0xff << (lane * 8)))
		{
			offs_t const byte = offset * 2 + lane;
			m_layer_tilemap[Layer]->mark_tile_dirty(byte & 0x7ff);
			if (Layer && byte < 0x200)
				m_reel_tilemap->mark_tile_dirty(byte);
		}
	}
}

void dyna_plane_base::video_regs_w(offs_t offset, uint8_t data)
{
	uint8_t const changed = m_video_regs[offset] ^ data;
	m_video_regs[offset] = data;
	if ((offset == 1 || offset == 2) && (changed & 0x1c))
	{
		m_layer_tilemap[offset - 1]->mark_all_dirty();
		if (offset == 2)
			m_reel_tilemap->mark_all_dirty();
	}
	// Register 3 selects the reel palette; register 5 positions the background
	// vertically. Registers 4/6 and the remaining control bits need further decoding.
	LOGVIDEOREGS("%s: plane video register %02x = %02x\n", machine().describe_context(), offset, data);
}

uint32_t dyna_plane_base::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// The background position also applies to the reels and their line-enable table.
	int const bg_scroll = -int8_t(m_video_regs[5]);

	bitmap.fill(m_palette->black_pen(), cliprect);
	if (BIT(m_video_regs[0], 1))
	{
		if (BIT(m_video_regs[0], 4))
		{
			// Crazy Bell E03AE selects palette 0a; Bonanza D2182 selects 05.
			m_reel_tilemap->set_palette_offset((m_video_regs[3] & 0x0f) << 4);

			for (uint32_t col = 0; col < 64; ++col)
				m_reel_tilemap->set_scrolly(col, vram_byte(m_vram_bg, 0x800 + col) + bg_scroll);

			// D283D uploads a per-scanline reel window to the last four rows
			// of foreground code RAM. Crazy Bell E2F0C/E4C53 does the same.
			// Bit 7 enables a line; other bits need further verification.
			for (int y = cliprect.top(); y <= cliprect.bottom(); ++y)
			{
				if (BIT(vram_byte(m_vram_fg, 0x700 + ((y + bg_scroll) & 0xff)), 7))
					m_reel_tilemap->draw(screen, bitmap, rectangle(cliprect.left(), cliprect.right(), y, y));
			}
		}
		else
		{
			m_layer_tilemap[1]->set_scrolly(0, bg_scroll);
			m_layer_tilemap[1]->draw(screen, bitmap, cliprect);
		}
	}
	if (BIT(m_video_regs[0], 0))
		m_layer_tilemap[0]->draw(screen, bitmap, cliprect);
	return 0;
}

void cb2001_state::program_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0x20000, 0x20fff).ram().w(FUNC(cb2001_state::fg_w)).share(m_vram_fg);
	map(0x21000, 0x21fff).ram().w(FUNC(cb2001_state::bg_w)).share(m_vram_bg);
	map(0xc0000, 0xfffff).rom().region("boot_prg", 0);
}

// Retained RAM is 4 KiB on the early boards and 8 KiB on Triple Jack.
// Each program validates a signature and redundant credit/accounting values.
template <uint32_t NvramSize>
void cb2001_state::nvram_map(address_map &map)
{
	program_map(map);
	map(0x00000, NvramSize - 1).ram().share("nvram");
}

void pdouble8_state::io_map(address_map &map)
{
	scherrym_base_io_map(map);
	// E2FD4/E4384 read inputs independently of the OUT30/31 latches;
	// the program never sets an 8255 direction register.
	map(0x30, 0x32).r(FUNC(pdouble8_state::input_r));
	map(0x30, 0x30).w(FUNC(pdouble8_state::lamps_w));
	map(0x31, 0x31).w(FUNC(pdouble8_state::output_w));
}

uint8_t pdouble8_state::input_r(offs_t offset)
{
	return m_inputs[offset]->read();
}

void pdouble8_state::lamps_w(uint8_t data)
{
	// E3049 writes the panel latch, mirrored at RAM 04B2.
	for (uint32_t bit = 0; bit < 8; ++bit)
		m_lamps[bit] = BIT(data, bit);
}

void pdouble8_state::output_w(uint8_t data)
{
	// E5B9E/E5BFB set/clear bit 7 through E30B4/E30E4.
	m_hopper->motor_w(BIT(data, 7));
	// E3080 queues Coin A/B/C/D pulses; E5D49, E5CEA and E6031
	// pulse the hopper, attendant and Key Out meters respectively.
	for (uint32_t counter = 0; counter < 7; ++counter)
		machine().bookkeeping().coin_counter_w(counter, BIT(data, 6 - counter));
}

void crzybell_state::program_map(address_map &map)
{
	// E01D1 tests/restores 8 KiB. E0282 checks the retained signature
	// and credit copies at 00D0/01D0; invalid contents clear 0080-1BFF.
	cb2001_state::program_map(map);
	map(0x00000, 0x01fff).ram().share("nvram");
	// E32B8 writes AL at IY and AH at IY+0800, then advances IY by one.
	map(0x20000, 0x20fff).ram().w(FUNC(crzybell_state::vram_w<0>)).share(m_vram_fg);
	map(0x21000, 0x21fff).ram().w(FUNC(crzybell_state::vram_w<1>)).share(m_vram_bg);
}

void crzybell_state::io_map(address_map &map)
{
	scherrym_io_map(map);
	map(0x00, 0x06).w(FUNC(crzybell_state::video_regs_w));
	// The ROM reads inputs and writes independent latches without setting
	// an 8255 direction register (E018D/E3013/E2DDF).
	map(0x30, 0x30).w(FUNC(crzybell_state::lamps_w));
	map(0x31, 0x31).w(FUNC(crzybell_state::output_w));
}

void crzybell_state::output_w(uint8_t data)
{
	// E25F2/E2636 set/clear bit 7 through E2DDA/E2DE5.
	m_hopper->motor_w(BIT(data, 7));
	// E2CD6 drains the six pulse queues: Coin A/B/C, hopper units,
	// attendant fallback (E25AC), and Key Out (E2451).
	static constexpr uint8_t COUNTER_BITS[] = { 6, 5, 3, 2, 1, 0 };
	for (uint32_t counter = 0; counter < std::size(COUNTER_BITS); ++counter)
		machine().bookkeeping().coin_counter_w(counter, BIT(data, COUNTER_BITS[counter]));
}

void bsbonanza_state::program_map(address_map &map)
{
	// D0128 tests/restores the first 8 KiB.  D01A9 checks the retained
	// signature at 0080 and the credit/backup pair at 0254/0a30 before
	// deciding whether to initialise bookkeeping or preserve it.
	map(0x00000, 0x01fff).ram().share("nvram");
	map(0x02000, 0x1ffff).ram();
	// Both revisions copy 256 little-endian colour words from D000:D3C9
	// to 0000:8000 in the palette upload routine at D000:D658.
	map(0x08000, 0x081ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x20000, 0x20fff).ram().w(FUNC(bsbonanza_state::vram_w<0>)).share(m_vram_fg);
	map(0x21000, 0x21fff).ram().w(FUNC(bsbonanza_state::vram_w<1>)).share(m_vram_bg);
	map(0xc0000, 0xfffff).rw(m_eeprom, FUNC(bsbonanza_eeprom_device::read), FUNC(bsbonanza_eeprom_device::write));
}

void cb5_state::machine_start()
{
	cb4_state::machine_start();
	for (auto &timer : m_counter_timers)
		timer = timer_alloc(FUNC(cb5_state::counter_tick), this);
	save_item(NAME(m_counter_state));
}

void cb4_state::output_w(uint8_t data)
{
	// E5ce3/E5d40 set/clear mask 80 via E3155/E3164, which complement
	// the latch before writing PPI2 port C (B822). Wing W-4 uses /HOPPER.
	// V1.2 uses E5d99/E5df6 via E3234/E3243 with the same polarity.
	// CB5 V1.1/V1.3/V1.4 use E6a0f/E6a6c via E37c0/E37cf.
	m_hopper->motor_w(!BIT(data, 7));
	// The other bits control coin acceptance and board functions.
}

TIMER_CALLBACK_MEMBER(cb5_state::counter_tick)
{
	machine().bookkeeping().coin_counter_w(param, 1);
}

void cb5_state::counters_w(uint8_t data)
{
	// CB5 E37da/E381d pulse and release B821 bits 6-3 for Coin A,
	// Coin B, Coin C and Coin D. E6bba pulses bit 2 for each hopper coin.
	// The program complements the latch before writing PPI2 port B.
	data = ~data;
	for (uint32_t counter = 0; counter < m_counter_timers.size(); ++counter)
	{
		bool const active = BIT(data, 6 - counter);
		if (active != BIT(m_counter_state, 6 - counter))
		{
			// Reject the brief all-low output when the 8255 is configured.
			// Real meter pulses last three frames; 25 ms is an unmeasured
			// mechanical response time, as used by MAME's meter model.
			m_counter_timers[counter]->adjust(active ? attotime::from_msec(25) : attotime::never, counter);
			if (!active)
				machine().bookkeeping().coin_counter_w(counter, 0);
		}
	}
	m_counter_state = data;
}

template <uint32_t Player>
void cb4_state::lamps_w(uint8_t data)
{
	// E3107 (CB4) and E3775 (CB5) select B850/B860 by player.
	// Bits 0-5 are Big, Double Up, Take, Bet, Info and Start respectively.
	for (uint32_t bit = 0; bit < 8; ++bit)
		m_lamps[Player * 8 + bit] = BIT(data, bit);
}

void cb4_state::io_map(address_map &map)
{
	cb2001_state::cb5_io_map(map);
	map(0xb850, 0xb850).w(FUNC(cb4_state::lamps_w<0>));
	map(0xb860, 0xb860).w(FUNC(cb4_state::lamps_w<1>));
}

void cherry2001_state::output_w(uint8_t data)
{
	// E4B42/E4BA1 set/clear bit 7 via the independent OUT01 latch at E3043.
	// The remaining bits drive meters; their assignments need verification.
	m_hopper->motor_w(BIT(data, 7));
}

void cherry2001_state::io_map(address_map &map)
{
	cb2001_state::io_map(map);
	map(0x01, 0x01).w(FUNC(cherry2001_state::output_w));
}

void scherrymp_state::output_w(uint8_t data)
{
	// C60b9/C612e set/clear bit 7 through the OUT01 latch at C3ca2/C3cd6.
	// V1.0U does the same at C6000/C6075 through C3c1a/C3c4e.
	// Mystery J & B E5840/E58b5 use E374c/E377c with the same polarity.
	// Other bits drive counters/lamps; their physical assignments are unknown.
	m_hopper->motor_w(BIT(data, 7));
}

void scherrymp_state::io_map(address_map &map)
{
	cb2001_state::io_map(map);
	map(0x01, 0x01).w(FUNC(scherrymp_state::output_w));
}

void mystjb_state::tripjack_io_map(address_map &map)
{
	io_map(map);
	// V1.6G E0398 reads 13 date/time digits, setting HOLD at 4d and
	// polling BUSY. E0342 writes the digits, then control E=6 and F=4
	// (24-hour mode). This is an MSM6242-compatible interface; the exact
	// RTC part and its external interrupt wiring have not been identified.
	map(0x40, 0x4f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
}

void mystjb_state::lamps_w(uint8_t data)
{
	// E36d3/E36ec update the separate OUT00 latch. The masks match IN0:
	// Big, unknown, Stop 2, Stop 1/Double Up, Take, Bet, Small/Info, Start.
	// Triple Jack uses the same eight-bit latch (C35d3/E30c7/E30bd).
	// New Mondo Plus C3b51 writes the latch at 0557 to the same port.
	// New DongmulDongmul 2 C3c45 uses the latch at 054b.
	for (uint32_t bit = 0; bit < 8; ++bit)
		m_lamps[bit] = BIT(data, bit);
}

void mystjb_state::output_w(uint8_t data)
{
	scherrymp_state::output_w(data);
	// E3718 pulses bits 6-3 for Coin A-D. E5a46 pulses bit 2 for each
	// hopper coin, E59ca bit 1 for attendant-paid coins, E5b92 bit 0 for
	// Key Out units. The independent latch starts at zero (E010c).
	// Triple Jack has the same seven assignments: V1.6G C3618/C5d2a/
	// C5ca3/C5e81; V1.1 E310c/E5398/E5338/E54b7; V0.7 E3102/E5418/
	// E53b8/E5537. Bit 7 controls its hopper (C5af8/E51b4/E5234).
	// New Mondo Plus: C3b88/C606e/C600e/C6199 use the same meters;
	// C5ea0/C5f15 start/stop its hopper through the latch at 0543.
	// New DongmulDongmul 2: C3c7c/C5ff4/C5f94/C611f use these meters;
	// C5e26/C5e9b start/stop the hopper through the latch at 0537.
	for (uint32_t counter = 0; counter < 7; ++counter)
		machine().bookkeeping().coin_counter_w(counter, BIT(data, 6 - counter));
}

void mystjb_state::io_map(address_map &map)
{
	scherrymp_state::io_map(map);
	map(0x00, 0x00).w(FUNC(mystjb_state::lamps_w));
	map(0x01, 0x01).w(FUNC(mystjb_state::output_w));
}

void bsbonanza_state::output_w(uint8_t data)
{
	// D13F2 starts the hopper with mask 80; D1467/D14A2/D15C7 stop it.
	m_hopper->motor_w(BIT(data, 7));
	// D24f2-D2619 drains the seven pending meter counts through OUT01:
	// bits 6-3 are Coin A, Key In, Coin C and Token In; bits 2-0 are
	// hopper, attendant and Key Down. V1.0 uses the same code, with the
	// shadow/counts one byte earlier (0ab8-0ad4 instead of 0ab9-0ad5).
	for (uint32_t counter = 0; counter < 7; ++counter)
		machine().bookkeeping().coin_counter_w(counter, BIT(data, 6 - counter));
}

void bsbonanza_state::io_map(address_map &map)
{
	cb2001_state::io_map(map);
	// Ports 00/01 have independent input and output latches. Neither revision
	// programs an 8255 direction register before reading coins/driving the motor.
	map(0x00, 0x00).w(FUNC(bsbonanza_state::lamps_w));
	map(0x01, 0x01).w(FUNC(bsbonanza_state::output_w));
	map(0x30, 0x36).w(FUNC(bsbonanza_state::video_regs_w));
	// Both revisions use HOLD/BUSY at 4d, 13 calendar digits at 40-4c,
	// and controls E=6/F=4 (E78b8/E790e): an MSM6242-compatible RTC.
	// The exact part and its external interrupt connection are unverified.
	map(0x40, 0x4f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
}

void cb2001_state::io_map(address_map &map)
{
	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));   // Input ports
	map(0x10, 0x11).portr("DSW1-2");
	map(0x12, 0x13).portr("DSW3");
	map(0x21, 0x21).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x22, 0x23).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x30, 0x30).r(FUNC(cb2001_state::irq_ack_r));
	map(0x30, 0x31).w(FUNC(cb2001_state::vidctrl_w));
	map(0x32, 0x33).w(FUNC(cb2001_state::vidctrl2_w));
}

void cb2001_state::scherrym_base_io_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(cb2001_state::irq_ack_r));
	map(0x00, 0x01).w(FUNC(cb2001_state::vidctrl_w));
	map(0x02, 0x03).w(FUNC(cb2001_state::vidctrl2_w));
	map(0x10, 0x11).portr("DSW1-2");
	map(0x12, 0x13).portr("DSW3");
	map(0x21, 0x21).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x22, 0x23).w("aysnd", FUNC(ay8910_device::data_address_w));
}

void cb2001_state::scherrym_io_map(address_map &map)
{
	scherrym_base_io_map(map);
	map(0x30, 0x33).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));   // Input ports
}

void cb2001_state::cb5_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).r(FUNC(cb2001_state::irq_ack_r));
	map(0x0000, 0x0001).w(FUNC(cb2001_state::vidctrl_w));
	map(0x0002, 0x0003).w(FUNC(cb2001_state::vidctrl2_w));
	// this one has the main PCB hw mapped in the 0xb800 - 0xbfff area
	map(0xb800, 0xb803).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb810, 0xb813).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb820, 0xb823).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb830, 0xb830).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xb840, 0xb840).w("aysnd", FUNC(ay8910_device::address_w));
	// TODO: identify the write-only port at 0xb870 (legacy sound hardware?).
}

static INPUT_PORTS_START( cb2001 )
	// TODO: identify IN0 bits 0/1 on Super Cherry Master.
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Unknown Player Input 0") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Unknown Player Input 1") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)  // Coin B
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)  // Coin D
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)  // Coin C
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)  // Coin A

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1-2")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Reel Speed" )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( High ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0000, "Main Game Payout Rate" )  PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0700, "55%" )
	PORT_DIPSETTING(      0x0600, "60%" )
	PORT_DIPSETTING(      0x0500, "65%" )
	PORT_DIPSETTING(      0x0400, "70%" )
	PORT_DIPSETTING(      0x0300, "75%" )
	PORT_DIPSETTING(      0x0200, "80%" )
	PORT_DIPSETTING(      0x0100, "85%" )
	PORT_DIPSETTING(      0x0000, "90%" )
	PORT_DIPNAME( 0x0800, 0x0000, "Double Up Game Payout Rate" )  PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0800, "80%" )
	PORT_DIPSETTING(      0x0000, "90%" )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "Maximum Bet" )  PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0000, "10" )
	PORT_DIPSETTING(      0x2000, "20" )
	PORT_DIPNAME( 0x4000, 0x0000, "Minimum Bet" )  PORT_DIPLOCATION("DSW2:7") // fixed at 1 in the service screen but shows 8 or 16 during attract
	PORT_DIPSETTING(      0x4000, "8" )
	PORT_DIPSETTING(      0x0000, "16" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pdouble8 )
	PORT_INCLUDE(cb2001)

	PORT_MODIFY("IN0")
	// E4FA4 uses bits 0/6 for Big/Small. E0C8D selects the stop
	// button table using DSW5; these stop numbers describe the default.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Big")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// E41B0/E426D maintain separate bets for the two sides.
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Bet B / Stop 2 / D-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Take / Stop")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet A / Stop")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / Small / Info")

	PORT_MODIFY("IN1")
	// E4384 masks F8; bits 7/6/5/4 are Coin A/B/C/D, bit 3 adds
	// one service credit (E43F9). It does not pulse a coin meter.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Service Credit")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_MODIFY("IN2")
	// E5B86 checks the active-high empty input. E5D57 applies the
	// selected sensor polarity; the default waits for a low-to-high pulse.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Empty") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Settings / Input Test (hold at boot)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Bookkeeping / Gallery (hold at boot)")

	PORT_MODIFY("DSW1-2")
	PORT_DIPNAME( 0x0002, 0x0000, "Double Up" ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Double Up 7" ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0000, "Lose" )
	PORT_DIPSETTING(      0x0004, "Draw" )
	PORT_DIPNAME( 0x0008, 0x0000, "Double Up Gallery" ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Win Collection" ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0000, "Hopper" )
	PORT_DIPSETTING(      0x0010, "Credits" )
	PORT_DIPNAME( 0x0040, 0x0000, "Hopper Sensor" ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0000, "Active High" )
	PORT_DIPSETTING(      0x0040, "Active Low" )
	// E2E7B indexes four BCD limits at E80D1; E3936 displays the value.
	PORT_DIPNAME( 0x3000, 0x0000, "Maximum Bet" ) PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(      0x3000, "8" )
	PORT_DIPSETTING(      0x2000, "16" )
	PORT_DIPSETTING(      0x1000, "32" )
	PORT_DIPSETTING(      0x0000, "64" )

	PORT_MODIFY("DSW3")
	// E2EA7-E2F19: the other denominations are multiples of Coin A.
	// Coin D also supplies the hopper conversion at E5D95/E60AE.
	PORT_DIPNAME( 0x0007, 0x0000, "Coin A" ) PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(      0x0007, "1 Coin / 1 Credit" )
	PORT_DIPSETTING(      0x0006, "1 Coin / 2 Credits" )
	PORT_DIPSETTING(      0x0005, "1 Coin / 4 Credits" )
	PORT_DIPSETTING(      0x0004, "1 Coin / 5 Credits" )
	PORT_DIPSETTING(      0x0003, "1 Coin / 8 Credits" )
	PORT_DIPSETTING(      0x0002, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(      0x0001, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Coin / 25 Credits" )
	PORT_DIPNAME( 0x0018, 0x0000, "Coin B" ) PORT_DIPLOCATION("DSW3:4,5")
	PORT_DIPSETTING(      0x0018, "5 x Coin A" )
	PORT_DIPSETTING(      0x0010, "10 x Coin A" )
	PORT_DIPSETTING(      0x0008, "20 x Coin A" )
	PORT_DIPSETTING(      0x0000, "50 x Coin A" )
	PORT_DIPNAME( 0x0020, 0x0000, "Coin C" ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0020, "5 x Coin A" )
	PORT_DIPSETTING(      0x0000, "10 x Coin A" )
	PORT_DIPNAME( 0x0040, 0x0000, "Coin D / Hopper" ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0040, "1 x Coin A" )
	PORT_DIPSETTING(      0x0000, "2 x Coin A" )

	PORT_MODIFY("DSW4")
	// E2BBA suspends further play above this balance; E2B95 displays
	// the limit only when bit 3 is clear. Table: E8099 (four-byte BCD).
	PORT_DIPNAME( 0x0007, 0x0007, "Credit Limit for Play" ) PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(      0x0007, "5000" )
	PORT_DIPSETTING(      0x0006, "10000" )
	PORT_DIPSETTING(      0x0005, "20000" )
	PORT_DIPSETTING(      0x0004, "30000" )
	PORT_DIPSETTING(      0x0003, "40000" )
	PORT_DIPSETTING(      0x0002, "50000" )
	PORT_DIPSETTING(      0x0001, "100000" )
	PORT_DIPSETTING(      0x0000, "Unlimited" )
	PORT_DIPNAME( 0x0008, 0x0008, "Show Credit Limit" ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	// E4368 compares the balance with the BCD table selected at E2F59.
	PORT_DIPNAME( 0x0030, 0x0030, "Coin Acceptance Limit" ) PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(      0x0030, "1000" )
	PORT_DIPSETTING(      0x0020, "5000" )
	PORT_DIPSETTING(      0x0010, "10000" )
	PORT_DIPSETTING(      0x0000, "20000" )
	// E5DEF compares the pending coin count, not the credit amount.
	PORT_DIPNAME( 0x00c0, 0x00c0, "Hopper Limit (Coins)" ) PORT_DIPLOCATION("DSW4:7,8")
	PORT_DIPSETTING(      0x00c0, "300" )
	PORT_DIPSETTING(      0x0080, "500" )
	PORT_DIPSETTING(      0x0040, "1000" )
	PORT_DIPSETTING(      0x0000, "Unlimited" )

	PORT_MODIFY("DSW5")
	// E0C64-E0CF9: three button tables and a sequential mode.
	PORT_DIPNAME( 0x0003, 0x0003, "Reel Stop Buttons (Left to Right)" ) PORT_DIPLOCATION("DSW5:1,2")
	PORT_DIPSETTING(      0x0003, "Small, D-Up, Big" )
	PORT_DIPSETTING(      0x0002, "Small (Sequential)" )
	PORT_DIPSETTING(      0x0001, "Take, Big, Small" )
	PORT_DIPSETTING(      0x0000, "Take, Bet A, Small" )
	PORT_DIPNAME( 0x0004, 0x0004, "Manual Reel Stop" ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Start Stops All Reels" ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( crzybell )
	PORT_INCLUDE(cb2001)

	PORT_MODIFY("IN0")
	// E3D8E: D-UP selects DIPs, Take inputs, Big sound, Small lamps.
	// E1910 maps Take/Big/Small (bits 4/0/6) to reel masks 1/2/4.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Big / Half")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Stop All") // E14DE-E14E8
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Small / Info")

	PORT_MODIFY("IN1")
	// E2BAB masks D0, then handles bits 4/6/7 as Coin C/B/A.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_MODIFY("IN2")
	// E2709 reads bit 2 and applies the software-selected polarity.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	// E28A8 reads five banks; E3E3C-E4240 displays their meanings.
	PORT_MODIFY("DSW1-2")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "Double Up" ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Double Up 7" ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0000, "Draw" )
	PORT_DIPSETTING(      0x0004, "Lose" )
	PORT_DIPNAME( 0x0008, 0x0000, "Double Up Girl" ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0000, "Display" )
	PORT_DIPSETTING(      0x0008, "Hide" )
	PORT_DIPNAME( 0x0010, 0x0000, "Hopper" ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0000, "Used" )
	PORT_DIPSETTING(      0x0010, "Not Used" )
	PORT_DIPNAME( 0x0020, 0x0000, "Win Collection" ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0000, "Payout Switch" )
	PORT_DIPSETTING(      0x0020, "Automatic" )
	PORT_DIPNAME( 0x0040, 0x0000, "Hopper Sensor" ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0000, "Active High" )
	PORT_DIPSETTING(      0x0040, "Active Low" )
	PORT_DIPNAME( 0x0080, 0x0000, "Progressive Link" ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0000, "Online" )
	PORT_DIPSETTING(      0x0080, "Not Used" )
	PORT_DIPNAME( 0x0700, 0x0000, "Main Game Payout Rate" ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0200, "70%" )
	PORT_DIPSETTING(      0x0100, "70%" )
	PORT_DIPSETTING(      0x0000, "70%" )
	PORT_DIPSETTING(      0x0300, "75%" )
	PORT_DIPSETTING(      0x0400, "80%" )
	PORT_DIPSETTING(      0x0500, "85%" )
	PORT_DIPSETTING(      0x0600, "90%" )
	PORT_DIPSETTING(      0x0700, "95%" )
	PORT_DIPNAME( 0x0800, 0x0000, "Double Up Game Payout Rate" ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0000, "80%" )
	PORT_DIPSETTING(      0x0800, "90%" )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	// Coin C is also the hopper denomination (E272D/E269B).
	PORT_DIPNAME( 0x0007, 0x0000, "Coin A (Coin)" ) PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(      0x0000, "1 Coin / 1 Credit" )
	PORT_DIPSETTING(      0x0001, "1 Coin / 2 Credits" )
	PORT_DIPSETTING(      0x0002, "1 Coin / 4 Credits" )
	PORT_DIPSETTING(      0x0003, "1 Coin / 5 Credits" )
	PORT_DIPSETTING(      0x0004, "1 Coin / 8 Credits" )
	PORT_DIPSETTING(      0x0005, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(      0x0006, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(      0x0007, "1 Coin / 25 Credits" )
	PORT_DIPNAME( 0x0018, 0x0000, "Coin B (Note)" ) PORT_DIPLOCATION("DSW3:4,5")
	PORT_DIPSETTING(      0x0000, "Coin A x5" )
	PORT_DIPSETTING(      0x0008, "Coin A x10" )
	PORT_DIPSETTING(      0x0010, "Coin A x20" )
	PORT_DIPSETTING(      0x0018, "Coin A x50" )
	PORT_DIPNAME( 0x00e0, 0x0000, "Coin C / Payout Unit (Token)" ) PORT_DIPLOCATION("DSW3:6,7,8")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0040, "5" )
	PORT_DIPSETTING(      0x0060, "10" )
	PORT_DIPSETTING(      0x0080, "20" )
	PORT_DIPSETTING(      0x00a0, "40" )
	PORT_DIPSETTING(      0x00c0, "50" )
	PORT_DIPSETTING(      0x00e0, "100" )

	PORT_MODIFY("DSW4")
	PORT_DIPNAME( 0x0007, 0x0007, "Credit Limit" ) PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(      0x0007, "5000" )
	PORT_DIPSETTING(      0x0006, "10000" )
	PORT_DIPSETTING(      0x0005, "20000" )
	PORT_DIPSETTING(      0x0004, "30000" )
	PORT_DIPSETTING(      0x0003, "40000" )
	PORT_DIPSETTING(      0x0002, "50000" )
	PORT_DIPSETTING(      0x0001, "100000" )
	PORT_DIPSETTING(      0x0000, "Unlimited" )
	PORT_DIPNAME( 0x0008, 0x0008, "Display Credit Limit" ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, "Coin Acceptance Limit" ) PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(      0x0030, "1000" )
	PORT_DIPSETTING(      0x0020, "5000" )
	PORT_DIPSETTING(      0x0010, "10000" )
	PORT_DIPSETTING(      0x0000, "20000" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Hopper Limit" ) PORT_DIPLOCATION("DSW4:7,8")
	PORT_DIPSETTING(      0x00c0, "300" )
	PORT_DIPSETTING(      0x0080, "500" )
	PORT_DIPSETTING(      0x0040, "1000" )
	PORT_DIPSETTING(      0x0000, "Unlimited" )

	PORT_MODIFY("DSW5")
	PORT_DIPNAME( 0x0001, 0x0001, "Counter Speed" ) PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( High ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cherry2001 )
	PORT_INCLUDE(cb2001)

	PORT_MODIFY("IN0")
	// Settings/bookkeeping are IN2 bits 6/7, not duplicate player buttons.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	// E4B30 branches to HOPPER EMPTY when this input is high.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Empty") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_MODIFY("DSW1-2")
	PORT_DIPNAME( 0x4000, 0x0000, "Minimum Bet for Bonus" ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x4000, "8" )
	PORT_DIPSETTING(      0x0000, "16" )

	PORT_MODIFY("DSW3")
	// E5449-E5463 derives the payout unit from Coin A, in packed BCD.
	PORT_DIPNAME( 0x0040, 0x0000, "Payout Unit" ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0040, "Coin A" )
	PORT_DIPSETTING(      0x0000, "Coin A x10" )
	// E5473 copies bit 7 to 0513; E5200/E523C use it to bypass CC == 0.
	PORT_DIPNAME( 0x0080, 0x0000, "Payout with Credits Remaining" ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bsbonanza )
	PORT_INCLUDE(cb2001)

	PORT_MODIFY("IN0")
	// Both revisions' input test at D3A24 labels the bits using the table at D564D.
	// Bits 1 and 2 have no labels; settings and bookkeeping are on IN2.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Big")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Stop 5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP4 ) PORT_NAME("Stop 4 / Small / Info")

	PORT_MODIFY("IN1")
	// D236b masks out bits 0-2. D23ec adds one service credit; the input
	// test calls bit 4 TOKEN IN, and bit 6 is KEY IN (not Coin B).
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Service Credit")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Token In") PORT_IMPULSE(2)

	PORT_MODIFY("IN2")
	// D0c0c selects the history menu (DE019; V1.0 DE012).
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("History")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	// D13DC treats a high empty input as HOPPER EMPTY / CALL ATTENDANT.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Empty") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	// The D9902 dump uses the software configuration stored at C0200.
	// Do not expose cb2001's unrelated DIP settings. A second V1.1 board
	// with different disclaimer text was reported with two switch banks.
	PORT_MODIFY("DSW1-2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("DSW3")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("DSW4")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("DSW5")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static INPUT_PORTS_START( scherrymp )
	PORT_INCLUDE(cb2001)

	PORT_START("DSW6")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	 // test mode shows a 7th bank and it's read, but it wasn't populated on the dumped PCB. Leaving it here until the controls are done.
	PORT_START("DSW7")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( scmplus )
	PORT_INCLUDE(scherrymp)
	// Both Plus revisions use these selectors. V1.0U has identical tables
	// at C7cc7/C7ccf (bet limits), C7bfb (Coin A), and C7c6b (payout unit).

	PORT_MODIFY("IN0")
	// These player bits select extra boot tests at C49de, independently of
	// settings/bookkeeping on IN2. Avoid binding both to the same keys.
	// D03e8 reads bit 0 for Big and bit 6 for Small during Double Up.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Graphics Test")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	// C60a7/C6125 enter the empty/fault path when this input is high.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Empty") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_MODIFY("DSW1-2")
	// V1.6 C3956-C395c / V1.0U C38d6-C38dc: DSW1 bit 2 is reel speed.
	PORT_DIPNAME( 0x0004, 0x0000, "Reel Speed" ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( High ) )
	// C7da6 / C7cd7 contain 100, 10, 1 and FFFF (all credits), used by
	// C6361 / C62a8 when Key Out is held. These bits do not set reel speed.
	PORT_DIPNAME( 0x0060, 0x0000, "Key Out Unit" ) PORT_DIPLOCATION("DSW1:6,7")
	PORT_DIPSETTING(      0x0040, "1" )
	PORT_DIPSETTING(      0x0020, "10" )
	PORT_DIPSETTING(      0x0000, "100" )
	PORT_DIPSETTING(      0x0060, "All Credits" )
	// C39e4-C3a1d uses two-bit selectors, unlike the older input definition.
	PORT_DIPNAME( 0x1800, 0x0000, "Maximum Bet" ) PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(      0x1800, "8" )
	PORT_DIPSETTING(      0x1000, "16" )
	PORT_DIPSETTING(      0x0800, "32" )
	PORT_DIPSETTING(      0x0000, "64" )
	PORT_DIPNAME( 0x6000, 0x0000, "Minimum Bet for Bonus" ) PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(      0x6000, "8" )
	PORT_DIPSETTING(      0x4000, "8" ) // duplicate in the ROM table
	PORT_DIPSETTING(      0x2000, "16" )
	PORT_DIPSETTING(      0x0000, "32" )
	// C3a24 stores this bit at 04fd; C6350 conditionally inverts the sensor.
	PORT_DIPNAME( 0x8000, 0x0000, "Hopper Sensor Polarity" ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0000, "Active Low" )
	PORT_DIPSETTING(      0x8000, "Active High" )

	PORT_MODIFY("DSW3")
	// C3a2b-C3a48 inverts the switch bits and reads eight BCD values at 7cca.
	PORT_DIPNAME( 0x0007, 0x0000, "Coin A Credits" ) PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(      0x0007, "1" )
	PORT_DIPSETTING(      0x0006, "2" )
	PORT_DIPSETTING(      0x0005, "4" )
	PORT_DIPSETTING(      0x0004, "5" )
	PORT_DIPSETTING(      0x0003, "8" )
	PORT_DIPSETTING(      0x0002, "10" )
	PORT_DIPSETTING(      0x0001, "20" )
	PORT_DIPSETTING(      0x0000, "25" )
	// C3aa0-C3abb selects the packed-BCD payout unit relative to Coin A.
	PORT_DIPNAME( 0x0040, 0x0000, "Payout Unit" ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0040, "Coin A" )
	PORT_DIPSETTING(      0x0000, "Coin A x2" )

	PORT_MODIFY("DSW6")
	// C3b7a-C3b86 reads P0 bit 0 into 04e6; C08d1 gates Double Up.
	PORT_DIPNAME( 0x01, 0x01, "Double Up Game" ) PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(  0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tripjack )
	PORT_INCLUDE(scherrymp)

	PORT_MODIFY("IN0")
	// C462d selects printing here; IN2.7 independently enters bookkeeping.
	// D03f9 (V1.6G) / F03ca (V1.1/V0.7) read Big on bit 0, Small on bit 6.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Print / Graphics Test")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2")

	PORT_MODIFY("IN1")
	// C4f1c/E4705/E46fb mask F8: bit 3 is Key In, bit 6 is Coin B.
	// Key In adds 1 credit in V1.6G, 10 in V1.1/V0.7; Coin D uses the payout unit.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_MODIFY("IN2")
	// C5b03/E51bf/E523f wait for both sensor edges; C5ae6/E51a2/E5222
	// test empty high. The normal sensor setting expects an active-low pulse.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Empty") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_MODIFY("DSW1-2")
	PORT_DIPNAME( 0x0004, 0x0000, "Reel Speed" ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( High ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	// C31fa-C3255: C7114 contains packed BCD, not binary bet amounts.
	PORT_DIPNAME( 0x1800, 0x0000, "Maximum Bet" ) PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(      0x1800, "10" )
	PORT_DIPSETTING(      0x1000, "16" )
	PORT_DIPSETTING(      0x0800, "32" )
	PORT_DIPSETTING(      0x0000, "64" )
	PORT_DIPNAME( 0x6000, 0x0000, "Minimum Bet for Bonus" ) PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(      0x4000, "8" )
	PORT_DIPSETTING(      0x6000, "8" ) // duplicate in C711c
	PORT_DIPSETTING(      0x2000, "16" )
	PORT_DIPSETTING(      0x0000, "32" )

	PORT_MODIFY("DSW3")
	// C7048/C7058/C7098: inverted selectors and exact Coin A multiples.
	PORT_DIPNAME( 0x07, 0x00, "Coin A Credits" ) PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(  0x07, "1" )
	PORT_DIPSETTING(  0x06, "2" )
	PORT_DIPSETTING(  0x05, "4" )
	PORT_DIPSETTING(  0x04, "5" )
	PORT_DIPSETTING(  0x03, "8" )
	PORT_DIPSETTING(  0x02, "10" )
	PORT_DIPSETTING(  0x01, "20" )
	PORT_DIPSETTING(  0x00, "25" )
	PORT_DIPNAME( 0x18, 0x00, "Coin B Credits" ) PORT_DIPLOCATION("DSW3:4,5")
	PORT_DIPSETTING(  0x18, "Coin A x5" )
	PORT_DIPSETTING(  0x10, "Coin A x10" )
	PORT_DIPSETTING(  0x08, "Coin A x20" )
	PORT_DIPSETTING(  0x00, "Coin A x50" )
	PORT_DIPNAME( 0x20, 0x00, "Coin C Credits" ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(  0x20, "Coin A x5" )
	PORT_DIPSETTING(  0x00, "Coin A x10" )

	PORT_MODIFY("DSW4")
	// C70fc -> 05f2; C4efa inhibits coins when the balance reaches this limit.
	PORT_DIPNAME( 0x30, 0x30, "Coin Acceptance Limit" ) PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(  0x30, "1000" )
	PORT_DIPSETTING(  0x20, "5000" )
	PORT_DIPSETTING(  0x10, "10000" )
	PORT_DIPSETTING(  0x00, "20000" )

	PORT_MODIFY("DSW5")
	// C3374/C337d; C5e18/C60a8 consume these independently of DSW2.
	PORT_DIPNAME( 0x04, 0x04, "Hopper Sensor Polarity" ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(  0x04, "Active Low" )
	PORT_DIPSETTING(  0x00, "Active High" )
	PORT_DIPNAME( 0x08, 0x08, "Win Collection" ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(  0x08, "Credits" )
	PORT_DIPSETTING(  0x00, "Hopper" )
	// C7124 / E626b / E62ff; the routines recognise FFFF as all credits.
	PORT_DIPNAME( 0x30, 0x30, "Key Out Unit" ) PORT_DIPLOCATION("DSW5:5,6")
	PORT_DIPSETTING(  0x10, "1" )
	PORT_DIPSETTING(  0x20, "10" )
	PORT_DIPSETTING(  0x30, "100" )
	PORT_DIPSETTING(  0x00, "All Credits" )

	PORT_MODIFY("DSW6")
	PORT_DIPNAME( 0x01, 0x01, "Double Up Game" ) PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(  0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW7")
	// C7028 (V1.6G), E6203 (V0.7): the selector is inverted. V1.1
	// replaces this with four amounts on DSW3:7,8 and does not read P2.
	PORT_DIPNAME( 0xf0, 0xf0, "Payout Unit / Coin D Credits" ) PORT_DIPLOCATION("DSW7:5,6,7,8")
	PORT_DIPSETTING(  0x00, "1" )
	PORT_DIPSETTING(  0x10, "2" )
	PORT_DIPSETTING(  0x20, "4" )
	PORT_DIPSETTING(  0x30, "5" )
	PORT_DIPSETTING(  0x40, "10" )
	PORT_DIPSETTING(  0x50, "15" )
	PORT_DIPSETTING(  0x60, "20" )
	PORT_DIPSETTING(  0x70, "25" )
	PORT_DIPSETTING(  0x80, "40" )
	PORT_DIPSETTING(  0x90, "50" )
	PORT_DIPSETTING(  0xa0, "60" )
	PORT_DIPSETTING(  0xb0, "75" )
	PORT_DIPSETTING(  0xc0, "80" )
	PORT_DIPSETTING(  0xd0, "100" )
	PORT_DIPSETTING(  0xe0, "200" )
	PORT_DIPSETTING(  0xf0, "500" )
INPUT_PORTS_END

static INPUT_PORTS_START( tripjack07 )
	PORT_INCLUDE(tripjack)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Graphics Test")

	PORT_MODIFY("DSW1-2")
	// E2e01 (V0.7), E2e21 (V1.1): two pay-rate bits, followed by the
	// bet selectors, sensor polarity, and automatic win collection.
	PORT_DIPNAME( 0x0300, 0x0000, "Main Game Payout Rate" ) PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(      0x0300, "70%" )
	PORT_DIPSETTING(      0x0200, "75%" )
	PORT_DIPSETTING(      0x0100, "80%" )
	PORT_DIPSETTING(      0x0000, "85%" )
	PORT_DIPNAME( 0x0c00, 0x0000, "Maximum Bet" ) PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(      0x0c00, "10" )
	PORT_DIPSETTING(      0x0800, "16" )
	PORT_DIPSETTING(      0x0400, "32" )
	PORT_DIPSETTING(      0x0000, "64" )
	PORT_DIPNAME( 0x3000, 0x0000, "Minimum Bet for Bonus" ) PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(      0x2000, "8" )
	PORT_DIPSETTING(      0x3000, "8" )
	PORT_DIPSETTING(      0x1000, "16" )
	PORT_DIPSETTING(      0x0000, "32" )
	PORT_DIPNAME( 0x4000, 0x0000, "Hopper Sensor Polarity" ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0000, "Active Low" )
	PORT_DIPSETTING(      0x4000, "Active High" )
	PORT_DIPNAME( 0x8000, 0x0000, "Win Collection" ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0000, "Credits" )
	PORT_DIPSETTING(      0x8000, "Hopper" )

	PORT_MODIFY("DSW4")
	// E62d7 (V0.7), E6243 (V1.1); consumed at E46d5/E46df.
	PORT_DIPNAME( 0x30, 0x30, "Coin Acceptance Limit" ) PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(  0x30, "100" )
	PORT_DIPSETTING(  0x20, "200" )
	PORT_DIPSETTING(  0x10, "500" )
	PORT_DIPSETTING(  0x00, "1000" )

	PORT_MODIFY("DSW5")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tripjack11 )
	PORT_INCLUDE(tripjack07)

	PORT_MODIFY("DSW3")
	// E2f2d indexes the four packed-BCD words at E6187 with raw bits 6-7.
	PORT_DIPNAME( 0xc0, 0x00, "Payout Unit / Coin D Credits" ) PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(  0xc0, "1" )
	PORT_DIPSETTING(  0x80, "2" )
	PORT_DIPSETTING(  0x40, "5" )
	PORT_DIPSETTING(  0x00, "10" )

	PORT_MODIFY("DSW7")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "DSW7:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "DSW7:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "DSW7:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DSW7:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "DSW7:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "DSW7:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "DSW7:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSW7:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( mystjb )
	PORT_INCLUDE(scherrymp)

	PORT_MODIFY("IN0")
	// E44c6 selects the graphics test with bit 0, or the colour test with bit 3.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Graphics Test")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2")

	PORT_MODIFY("IN1")
	// E4c46 masks F8: bit 3 adds ten service credits; bit 6 uses Coin B.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_MODIFY("IN2")
	// E584b/E5875 wait for a complete sensor pulse. E582e tests empty high.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Empty") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_MODIFY("DSW1-2")
	// E3372 inverts bit 1 into 0501; E5d97 selects automatic hopper collection.
	PORT_DIPNAME( 0x0002, 0x0000, "Win Collection" ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0000, "Hopper" )
	PORT_DIPSETTING(      0x0002, "Credits" )
	PORT_DIPNAME( 0x0004, 0x0000, "Reel Speed" ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( High ) )
	// E7b3e: packed BCD 100, 10, 1, then FFFF for all credits (E5b44).
	PORT_DIPNAME( 0x0060, 0x0000, "Key Out Unit" ) PORT_DIPLOCATION("DSW1:6,7")
	PORT_DIPSETTING(      0x0040, "1" )
	PORT_DIPSETTING(      0x0020, "10" )
	PORT_DIPSETTING(      0x0000, "100" )
	PORT_DIPSETTING(      0x0060, "All Credits" )
	// E7b2e/E7b36 are indexed by the selectors decoded at E3408/E342a.
	PORT_DIPNAME( 0x1800, 0x0000, "Maximum Bet" ) PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(      0x1800, "8" )
	PORT_DIPSETTING(      0x1000, "16" )
	PORT_DIPSETTING(      0x0800, "32" )
	PORT_DIPSETTING(      0x0000, "64" )
	PORT_DIPNAME( 0x6000, 0x0000, "Minimum Bet for Bonus" ) PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(      0x4000, "8" )
	PORT_DIPSETTING(      0x6000, "8" ) // duplicate in the ROM table
	PORT_DIPSETTING(      0x2000, "16" )
	PORT_DIPSETTING(      0x0000, "32" )
	PORT_DIPNAME( 0x8000, 0x0000, "Hopper Sensor Polarity" ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0000, "Active Low" )
	PORT_DIPSETTING(      0x8000, "Active High" )

	PORT_MODIFY("DSW3")
	// E3450 inverts the selector; E7a62 contains eight packed-BCD amounts.
	PORT_DIPNAME( 0x0007, 0x0000, "Coin A Credits" ) PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(      0x0007, "1" )
	PORT_DIPSETTING(      0x0006, "2" )
	PORT_DIPSETTING(      0x0005, "4" )
	PORT_DIPSETTING(      0x0004, "5" )
	PORT_DIPSETTING(      0x0003, "8" )
	PORT_DIPSETTING(      0x0002, "10" )
	PORT_DIPSETTING(      0x0001, "20" )
	PORT_DIPSETTING(      0x0000, "25" )

	// E3471/E34a7 combine these selectors with Coin A's index. The ROM
	// tables at E7a72/E7ab2 contain exact multiples for every denomination.
	PORT_DIPNAME( 0x0018, 0x0000, "Coin B Credits" ) PORT_DIPLOCATION("DSW3:4,5")
	PORT_DIPSETTING(      0x0018, "Coin A x5" )
	PORT_DIPSETTING(      0x0010, "Coin A x10" )
	PORT_DIPSETTING(      0x0008, "Coin A x20" )
	PORT_DIPSETTING(      0x0000, "Coin A x50" )
	PORT_DIPNAME( 0x0020, 0x0000, "Coin C Credits" ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0020, "Coin A x5" )
	PORT_DIPSETTING(      0x0000, "Coin A x10" )

	PORT_MODIFY("DSW4")
	// E3510 selects a four-byte BCD limit from E7b16. E4c20 inhibits coin
	// acceptance at this balance; the last accepted coin can exceed it.
	PORT_DIPNAME( 0x30, 0x30, "Coin Acceptance Limit" ) PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(      0x30, "1000" )
	PORT_DIPSETTING(      0x20, "5000" )
	PORT_DIPSETTING(      0x10, "10000" )
	PORT_DIPSETTING(      0x00, "20000" )

	PORT_MODIFY("DSW6")
	// E358b reads P0 bit 0 into 0500; E0843 gates the gamble entry at F0000.
	PORT_DIPNAME( 0x01, 0x01, "Double Up Game" ) PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW7")
	// E3624 indexes E7ad2 directly with the upper nibble of P2. Each sensor
	// pulse pays this many credits, independently of the Coin A denomination.
	PORT_DIPNAME( 0xf0, 0xf0, "Payout Unit" ) PORT_DIPLOCATION("DSW7:5,6,7,8")
	PORT_DIPSETTING(      0x00, "1" )
	PORT_DIPSETTING(      0x10, "2" )
	PORT_DIPSETTING(      0x20, "4" )
	PORT_DIPSETTING(      0x30, "5" )
	PORT_DIPSETTING(      0x40, "10" )
	PORT_DIPSETTING(      0x50, "15" )
	PORT_DIPSETTING(      0x60, "20" )
	PORT_DIPSETTING(      0x70, "25" )
	PORT_DIPSETTING(      0x80, "40" )
	PORT_DIPSETTING(      0x90, "50" )
	PORT_DIPSETTING(      0xa0, "60" )
	PORT_DIPSETTING(      0xb0, "75" )
	PORT_DIPSETTING(      0xc0, "80" )
	PORT_DIPSETTING(      0xd0, "100" )
	PORT_DIPSETTING(      0xe0, "200" )
	PORT_DIPSETTING(      0xf0, "500" )
INPUT_PORTS_END

static INPUT_PORTS_START( nmondop )
	PORT_INCLUDE(scherrymp)

	PORT_MODIFY("IN0")
	// C4a07/C4a0d select the colour/graphics tests while Settings is held at boot.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Graphics Test")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2")

	PORT_MODIFY("IN1")
	// C5197 masks F8; C51e4 selects Coin B with bit 6, C5202 adds ten
	// service credits with bit 3. Coin A/B/C are fixed at 10/100/10 credits.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_MODIFY("IN2")
	// C5eab/C5ed5 wait for both sensor edges; C5e8e tests hopper empty high.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Empty") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	// C3acd opens the points game. A successful stop at DC98e reaches the hopper.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Punti Game / Payout")

	PORT_MODIFY("DSW1-2")
	PORT_DIPNAME( 0x0004, 0x0000, "Reel Speed" ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( High ) )
	// C3862 indexes C7c68: packed BCD 100, 10, 1, or FFFF for all credits.
	PORT_DIPNAME( 0x0060, 0x0000, "Key Out Unit" ) PORT_DIPLOCATION("DSW1:6,7")
	PORT_DIPSETTING(      0x0040, "1" )
	PORT_DIPSETTING(      0x0020, "10" )
	PORT_DIPSETTING(      0x0000, "100" )
	PORT_DIPSETTING(      0x0060, "All Credits" )
	// C38c9/C38eb index C7c58/C7c60. The last maximum differs from Mystery J & B.
	PORT_DIPNAME( 0x1800, 0x0000, "Maximum Bet" ) PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(      0x1800, "10" )
	PORT_DIPSETTING(      0x1000, "16" )
	PORT_DIPSETTING(      0x0800, "32" )
	PORT_DIPSETTING(      0x0000, "64" )
	PORT_DIPNAME( 0x6000, 0x0000, "Minimum Bet for Bonus" ) PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(      0x4000, "8" )
	PORT_DIPSETTING(      0x6000, "8" ) // duplicate in the ROM table
	PORT_DIPSETTING(      0x2000, "16" )
	PORT_DIPSETTING(      0x0000, "32" )
	PORT_DIPNAME( 0x8000, 0x0000, "Hopper Sensor Polarity" ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0000, "Active Low" )
	PORT_DIPSETTING(      0x8000, "Active High" )

	PORT_MODIFY("DSW3")
	// C3928 selects credits per hopper coin; Coin D uses the same unit (C51c4).
	// The 10-credit mode also changes collection rules (C3acd/C63d7).
	// DC9ea pays one coin in mode 2, or all whole coins in the pool in mode 3.
	PORT_DIPNAME( 0x03, 0x00, "Coin D / Payout Unit" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(      0x00, "50" )
	PORT_DIPSETTING(      0x01, "10" )
	PORT_DIPSETTING(      0x02, "100 (Single Coin)" )
	PORT_DIPSETTING(      0x03, "100 (Whole Pool)" )

	PORT_MODIFY("DSW4")
	// C39bf selects C7c40; C5171 inhibits coins when the balance reaches the limit.
	PORT_DIPNAME( 0x30, 0x30, "Coin Acceptance Limit" ) PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(      0x30, "1000" )
	PORT_DIPSETTING(      0x20, "5000" )
	PORT_DIPSETTING(      0x10, "10000" )
	PORT_DIPSETTING(      0x00, "20000" )
	// C39de selects C7c50. C6105 compares this limit with the coin count at 0738.
	PORT_DIPNAME( 0xc0, 0xc0, "Maximum Hopper Coins" ) PORT_DIPLOCATION("DSW4:7,8")
	PORT_DIPSETTING(      0xc0, "300" )
	PORT_DIPSETTING(      0x80, "500" )
	PORT_DIPSETTING(      0x40, "1000" )
	PORT_DIPSETTING(      0x00, "Unlimited" )

	PORT_MODIFY("DSW6")
	// C3a33 reads P0, inverts bit 0 into 04f2; C08e2 gates the gamble game.
	PORT_DIPNAME( 0x01, 0x01, "Double Up Game" ) PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cherry_bonus )
	// Wing W-4 wiring: B800/B801 are separate player panels; B810 carries coin/key inputs.
	// CB5 selects B800 + player at E4BF2/E5C25 and handles B810's F4 mask at E4DAF.
	PORT_START("IN0") // PPI0 port A, B800
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("P1 Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("P1 Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1") // PPI0 port B, B801
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// The standard gambling input types only define player one.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_PLAYER(2) PORT_NAME("P2 Big / Switch Controls") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_PLAYER(2) PORT_NAME("P2 Double Up") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_PLAYER(2) PORT_NAME("P2 Take Score") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_PLAYER(2) PORT_NAME("P2 Bet") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_PLAYER(2) PORT_NAME("P2 Small / Info") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2") // PPI1 port A, B810 (also currently connected to PPI0 port C)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("IN3") // PPI1 port B, B811
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cb5 )
	PORT_INCLUDE(cherry_bonus)

	PORT_MODIFY("IN2")
	// E4e02 uses the Coin B table for bit 6; E4e24 adds ten key-in credits
	// for bit 2. These two inputs are reversed from the inherited labels.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_MODIFY("IN3")
	// E6bc8 reads bit 2, optionally inverted by DSW1:7; E69f7 tests empty.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Empty") PORT_CODE(KEYCODE_J) PORT_TOGGLE

	PORT_MODIFY("DSW1")
	// E34de -> 04d4; E0752/E07b7 gate entry to the F000 Double Up code.
	PORT_DIPNAME( 0x01, 0x01, "Double Up Game" ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// E3506 -> 04d8; E6fca selects hopper payout or credit collection.
	PORT_DIPNAME( 0x10, 0x10, "Win Payout" ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "Hopper" )
	PORT_DIPSETTING(    0x00, "Credits" )
	PORT_DIPNAME( 0x20, 0x20, "Reel Speed" ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, "Hopper Sensor Polarity" ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )

	PORT_MODIFY("DSW2")
	// E3539 indexes the sixteen-byte records at E92c0 (E92b9 in V1.1).
	PORT_DIPNAME( 0x07, 0x07, "Main Game Payout Rate" ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	// E357f indexes E943e (E9437 in V1.1), storing packed BCD in 04e5.
	PORT_DIPNAME( 0x18, 0x18, "Maximum Bet" ) PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "16" )
	PORT_DIPSETTING(    0x10, "32" )
	PORT_DIPSETTING(    0x18, "64" )
	// E358d-E3597 -> 04e7, the minimum stake eligible for the bonus.
	PORT_DIPNAME( 0x20, 0x20, "Minimum Bet for Bonus" ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x20, "16" )

	PORT_MODIFY("DSW3")
	// E35c5 indexes E938a (E9383 in V1.1): coins required, BCD credits.
	PORT_DIPNAME( 0x07, 0x07, "Coin A Credits" ) PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x04, "8" )
	PORT_DIPSETTING(    0x05, "10" )
	PORT_DIPSETTING(    0x06, "20" )
	PORT_DIPSETTING(    0x07, "25" )
	// E3628 -> 04f0/04f1; used by Coin D (E4de6) and payout (E6ba0).
	PORT_DIPNAME( 0xc0, 0xc0, "Payout / Coin D Unit" ) PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "10" )
INPUT_PORTS_END

static INPUT_PORTS_START( cb4 )
	PORT_INCLUDE(cherry_bonus)

	PORT_MODIFY("IN3")
	// E5e9c reads bit 2, optionally inverted by DSW1:7; E5ccf tests empty.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Empty") PORT_CODE(KEYCODE_J) PORT_TOGGLE

	PORT_MODIFY("DSW1")
	// E495c reads this bit directly; low enables the other panel's Big button
	// to switch the selected player at E4985.
	PORT_DIPNAME( 0x01, 0x01, "Control Panels" ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	// E2e8d-E2e93 -> 0458; E06d3/E06fd gate the Double Up choice.
	PORT_DIPNAME( 0x02, 0x02, "Double Up Game" ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// E2eaa-E2eb0 -> 045b; E619d selects hopper payout or credit collection.
	PORT_DIPNAME( 0x10, 0x10, "Win Payout" ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "Hopper" )
	PORT_DIPSETTING(    0x00, "Credits" )
	// E2eb5-E2eb9 -> 045c; displayed by E39df and used by reel timing.
	PORT_DIPNAME( 0x20, 0x20, "Reel Speed" ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, "Hopper Sensor Polarity" ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )

	PORT_MODIFY("DSW2")
	// E2f21 indexes E81f4 (BCD 64/32/16/8) into 0467.
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" ) PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x10, "16" )
	PORT_DIPSETTING(    0x20, "32" )
	PORT_DIPSETTING(    0x30, "64" )
	// E2f32-E2f3f -> 0468; runtime settings show 8 or 16.
	PORT_DIPNAME( 0x40, 0x40, "Minimum Bet for Bonus" ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )

	PORT_MODIFY("DSW3")
	// E2f58 indexes E8140, with one coin per packed-BCD credit value.
	PORT_DIPNAME( 0x07, 0x07, "Coin A Credits" ) PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x04, "8" )
	PORT_DIPSETTING(    0x05, "10" )
	PORT_DIPSETTING(    0x06, "20" )
	PORT_DIPSETTING(    0x07, "25" )
	// E2fab indexes E81b0 into 0475/0476: one pulse per 1/2/5/10 credits.
	// The same ratio is also used by the Coin D input at E44a2.
	PORT_DIPNAME( 0xc0, 0xc0, "Payout / Coin D Unit" ) PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "10" )

	// The port test labels the V25 P2 input as DIP SW6 (E3bd1). The game
	// does not otherwise read it; whether this bank was populated is unknown.
	PORT_START("DSW6")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW6:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW6:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW6:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW6:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW6:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW6:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW6:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW6:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( cb4a )
	PORT_INCLUDE(cb4)

	PORT_MODIFY("IN2")
	// V1.2 masks B810 with E4 at E454B, omitting the later versions' Coin D input.
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1")
	// V1.2 omits V5.0's main-loop call to the panel-switch routine. The
	// leftover code at E4a23 is unused; the selected player stays zero.
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "DSW1:1" )
	// E2e58 forces 045a to zero; E61d2 always collects wins into credits.
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "DSW1:5" )

	PORT_MODIFY("DSW3")
	// E2f55-E2f6e selects a packed-BCD word from E0000 (100/200/600/1000)
	// for 0475/0476. Unlike V5.0, this ratio is not used by a Coin D input.
	PORT_DIPNAME( 0xc0, 0xc0, "Payout Unit" ) PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x40, "200" )
	PORT_DIPSETTING(    0x80, "600" )
	PORT_DIPSETTING(    0xc0, "1000" )

	PORT_MODIFY("DSW5")
	// E3020-E3027 -> 0477; E30ae selects sensor-driven hopper payout or
	// a twelve-frame, active-low pulse on PPI2 port B bit 3 (E30c7-E30d6).
	PORT_DIPNAME( 0x80, 0x80, "Payout Mode" ) PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(    0x80, "Hopper" )
	PORT_DIPSETTING(    0x00, "Pulse" )
INPUT_PORTS_END

static INPUT_PORTS_START( ndongmul2 )
	PORT_INCLUDE(scmplus)
	// C3950-C3bcd decodes the Plus selectors. C7b06/C7b0e/C7b16
	// contain the same bet limits and Key Out units; C7a3a is Coin A.
	// C49dc selects the boot tests; D03de reads Big/Small on IN0 bits 0/6.

	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Bet B / Stop 1 / D-UP")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet A")

	PORT_MODIFY("IN1")
	// C51e7 masks F8: C523e uses Coin B on bit 6, C525c adds ten
	// service credits on bit 3. Bits 0-2 are not coin inputs.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_MODIFY("DSW1-2")
	// C395c stores the inverted bit in 0528; C50ae gates the second bet.
	PORT_DIPNAME( 0x0001, 0x0000, "Side B Betting" ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	// C3967 stores the inverted bit in 04e7; C6239 selects hopper or credit.
	PORT_DIPNAME( 0x0002, 0x0000, "Win Collection" ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0000, "Hopper" )
	PORT_DIPSETTING(      0x0002, "Credits" )

	PORT_MODIFY("DSW3")
	// C3a5f/C3a95 combine these selectors with Coin A's index.
	// C7a4a/C7a8a contain exact multiples for all eight denominations.
	PORT_DIPNAME( 0x0018, 0x0000, "Coin B Credits" ) PORT_DIPLOCATION("DSW3:4,5")
	PORT_DIPSETTING(      0x0018, "Coin A x5" )
	PORT_DIPSETTING(      0x0010, "Coin A x10" )
	PORT_DIPSETTING(      0x0008, "Coin A x20" )
	PORT_DIPSETTING(      0x0000, "Coin A x50" )
	PORT_DIPNAME( 0x0020, 0x0000, "Coin C Credits" ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0020, "Coin A x5" )
	PORT_DIPSETTING(      0x0000, "Coin A x10" )
	// C3ab4-C3acf selects C7aaa/C7aba; 0507-050b serve Coin D and payout.
	PORT_DIPNAME( 0x0040, 0x0000, "Coin D / Payout Unit" ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0040, "Coin A" )
	PORT_DIPSETTING(      0x0000, "Coin A x2" )

	PORT_MODIFY("DSW4")
	// C3b1c indexes the four-byte BCD table C7aee; C51cf tests this limit
	// before accepting coins (the last coin can take the balance above it).
	PORT_DIPNAME( 0x30, 0x30, "Coin Acceptance Limit" ) PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(      0x30, "1000" )
	PORT_DIPSETTING(      0x20, "5000" )
	PORT_DIPSETTING(      0x10, "10000" )
	PORT_DIPSETTING(      0x00, "20000" )
INPUT_PORTS_END


INTERRUPT_GEN_MEMBER(cb2001_state::vblank_irq)
{
	m_maincpu->set_input_line(NEC_INPUT_LINE_INTP0, ASSERT_LINE);
}

uint8_t cb2001_state::irq_ack_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(NEC_INPUT_LINE_INTP0, CLEAR_LINE);
	return 0xff;
}

static const gfx_layout cb2001_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28, 16,20 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32 },
	8*32
};


static const gfx_layout cb2001_layout32 =
	{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28, 16,20 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32,14*32,15*32,16*32,17*32,18*32,19*32,20*32,21*32,22*32,23*32,24*32,25*32,26*32,27*32,28*32,29*32,30*32,31*32 },
	32*32
};

static GFXDECODE_START( gfx_cb2001 )
	GFXDECODE_ENTRY( "gfx", 0, cb2001_layout,   0x0, 32 )
	GFXDECODE_ENTRY( "gfx", 0, cb2001_layout32, 0x0, 32 )
GFXDECODE_END

void cb2001_state::palette_init(palette_device &palette) const
{
	uint8_t const *const proms = memregion("proms")->base();

	for (int i = 0; i < 0x200; i++)
	{
		uint16_t dat = (proms[0x000 + i] << 8) | proms[0x200 + i];

		int const b = ((dat >> 1) & 0x1f) << 3;
		int const r = ((dat >> 6) & 0x1f) << 3;
		int const g = ((dat >> 11) & 0x1f) << 3;

		if (!(i & 0x20)) palette.set_pen_color((i & 0x1f) | ((i & ~0x3f) >> 1), rgb_t(r, g, b));
	}
}

void cb2001_state::base(machine_config &config)
{
	V35(config, m_maincpu, 20'000'000); // CPU91A-011-0016JK004; encrypted CPU like NEC V25/35 used in some Irem games
	configure_cpu_decryption(*m_maincpu);
	m_maincpu->set_addrmap(AS_PROGRAM, &cb2001_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &cb2001_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(cb2001_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cb2001);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(cb2001_state::screen_update));

	PALETTE(config, m_palette, FUNC(cb2001_state::palette_init), 0x100);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", 1'500'000)); // wrong
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW5");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void cb2001_state::cb2001(machine_config &config)
{
	base(config);

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set_ioport("IN0");
	ppi0.in_pb_callback().set_ioport("IN1");
	ppi0.in_pc_callback().set_ioport("IN2");
}

void cherry2001_state::cherry2001(machine_config &config)
{
	cb2001(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &cherry2001_state::nvram_map<0x1000>);
	m_maincpu->set_addrmap(AS_IO, &cherry2001_state::io_map);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	HOPPER(config, m_hopper, attotime::from_msec(100)); // period unmeasured
}

void pdouble8_state::pdouble8(machine_config &config)
{
	base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pdouble8_state::nvram_map<0x1000>);
	m_maincpu->set_addrmap(AS_IO, &pdouble8_state::io_map);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	HOPPER(config, m_hopper, attotime::from_msec(100)); // half-period unmeasured
}

void dyna_plane_base::lamps_w(uint8_t data)
{
	for (uint32_t bit = 0; bit < 8; ++bit)
		m_lamps[bit] = BIT(data, bit);
}

void dyna_plane_base::plane(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	HOPPER(config, m_hopper, attotime::from_msec(100)); // period unmeasured

	screen_device &screen(*subdevice<screen_device>("screen"));
	// The final four foreground rows hold the reel-window lookup table.
	screen.set_visarea(0, 64 * 8 - 1, 0, 28 * 8 - 1);
	screen.set_screen_update(FUNC(dyna_plane_base::screen_update));
}

void crzybell_state::crzybell(machine_config &config)
{
	scherrym(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &crzybell_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &crzybell_state::io_map);
	plane(config);
}

void bsbonanza_state::bsbonanza(machine_config &config)
{
	cb2001(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &bsbonanza_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &bsbonanza_state::io_map);
	BSBONANZA_EEPROM(config, m_eeprom);
	plane(config);
	MSM6242(config, "rtc", 32'768); // compatible interface; part and oscillator unverified
	// D9902 has writable colour RAM instead of the earlier boards' PROMs.
	PALETTE(config.replace(), m_palette).set_format(palette_device::GGGGGRRRRRBBBBBx, 0x100);
}

void cb2001_state::cb5(machine_config &config)
{
	cb2001(config);

	m_maincpu->set_addrmap(AS_IO, &cb2001_state::cb5_io_map);
	m_maincpu->p0_in_cb().set_ioport("DSW5");

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("IN2");
	ppi1.in_pb_callback().set_ioport("IN3");
	ppi1.in_pc_callback().set_ioport("DSW1");

	i8255_device &ppi2(I8255A(config, "ppi8255_2"));
	ppi2.in_pa_callback().set_ioport("DSW2");

	subdevice<ay8910_device>("aysnd")->port_b_read_callback().set_ioport("DSW3");
}

void cb4_state::cb4(machine_config &config)
{
	cherry_bonus(config);
	m_maincpu->p2_in_cb().set_ioport("DSW6");
}

void cb5_state::cb5(machine_config &config)
{
	cherry_bonus(config);
	subdevice<i8255_device>("ppi8255_2")->out_pb_callback().set(FUNC(cb5_state::counters_w));
}

void cb4_state::cherry_bonus(machine_config &config)
{
	cb2001_state::cb5(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &cb4_state::nvram_map<0x1000>);
	m_maincpu->set_addrmap(AS_IO, &cb4_state::io_map);
	subdevice<i8255_device>("ppi8255_2")->out_pc_callback().set(FUNC(cb4_state::output_w));
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	HOPPER(config, m_hopper, attotime::from_msec(100)); // period unmeasured
}

void cb2001_state::scherrym(machine_config &config)
{
	cb2001(config);

	m_maincpu->set_addrmap(AS_IO, &cb2001_state::scherrym_io_map);
}

void cb2001_state::scherrymp(machine_config &config)
{
	cb2001(config);

	m_maincpu->set_clock(24_MHz_XTAL);
	m_maincpu->p0_in_cb().set_ioport("DSW6");
	m_maincpu->p2_in_cb().set_ioport("DSW7");
}

void scherrymp_state::scherrymp(machine_config &config)
{
	cb2001_state::scherrymp(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &scherrymp_state::nvram_map<0x1000>);
	m_maincpu->set_addrmap(AS_IO, &scherrymp_state::io_map);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	HOPPER(config, m_hopper, attotime::from_msec(100)); // pulse period unmeasured
}

void mystjb_state::mystjb(machine_config &config)
{
	scherrymp(config);
	m_maincpu->set_addrmap(AS_IO, &mystjb_state::io_map);
}

void mystjb_state::nmondop(machine_config &config)
{
	mystjb(config);
	// D9702 oscillator frequency is unverified.
	m_maincpu->set_clock(20'000'000);
}

void mystjb_state::tripjack(machine_config &config)
{
	mystjb(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mystjb_state::nvram_map<0x2000>);
}

void mystjb_state::tripjack16(machine_config &config)
{
	tripjack(config);
	m_maincpu->set_addrmap(AS_IO, &mystjb_state::tripjack_io_map);
	MSM6242(config, "rtc", 32'768); // compatible interface; part and oscillator unverified
}

void mystjb_state::ndongmul2(machine_config &config)
{
	mystjb(config);

	I80C51(config, "mcu", 12_MHz_XTAL).set_disable(); // Actually an AT89C51, currently undumped so disabled
}


// D9203 has a different memory map and a serial FPGA bitstream with XC2018 framing.
// The FPGA part is inferred from the stream, not a confirmed PCB marking.
// Graphics registers are reconstructed from the program's drawing routines.
// The register layout resembles DC4000: page/scroll pairs at 00-07, blit at 08-0f,
// and CLUT at 10-1f. Only the first two display layers are used by this program.
// TODO: page selection, unused scroll pairs 04-07, blitter timing/flags, FPGA mixing, I/O control 0b.
class dynastye_state : public driver_device
{
public:
	dynastye_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_gfxrom(*this, "gfx"),
		m_hopper(*this, "hopper"),
		m_dsw(*this, "DSW%u", 1U),
		m_inputs(*this, { "PLAYER", "COIN", "TEST" })
	{ }

	void dynastye(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<v35_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_gfxrom;
	required_device<hopper_device> m_hopper;
	required_ioport_array<3> m_dsw;
	required_ioport_array<3> m_inputs;
	uint8_t m_dsw_control = 0xff;
	bitmap_ind16 m_bitmap[2]; // persistent blitter framebuffers (emulated video RAM)
	uint8_t m_video_address = 0;
	uint8_t m_video_regs[0x20] = {};
	uint8_t m_display = 0;
	uint8_t m_palette_bank[2] = {};

	uint8_t m_p0 = 0xff;
	uint8_t m_config_phase = 0;
	uint8_t m_config_bit = 0;
	uint16_t m_config_frames = 0;
	uint32_t m_config_clocks = 0;
	uint32_t m_config_shift = 0;
	uint32_t m_config_length = 0;
	bool m_config_valid = true;
	bool m_config_done = false;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	template <uint32_t Bank> uint8_t dsw_r();
	uint8_t input_r(offs_t offset);
	void video_address_w(uint8_t data);
	void video_data_w(uint8_t data);
	void blit();
	void palette_init(palette_device &palette) const ATTR_COLD;
	void fpga_reset();
	uint8_t p0_r();
	void p0_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void dynastye_state::machine_start()
{
	for (auto &bitmap : m_bitmap)
	{
		bitmap.allocate(512, 256);
		bitmap.fill(0);
	}
	save_item(NAME(m_bitmap[0]));
	save_item(NAME(m_bitmap[1]));
	save_item(NAME(m_video_address));
	save_item(NAME(m_video_regs));
	save_item(NAME(m_display));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_dsw_control));
	save_item(NAME(m_p0));
	save_item(NAME(m_config_phase));
	save_item(NAME(m_config_bit));
	save_item(NAME(m_config_frames));
	save_item(NAME(m_config_clocks));
	save_item(NAME(m_config_shift));
	save_item(NAME(m_config_length));
	save_item(NAME(m_config_valid));
	save_item(NAME(m_config_done));
}

void dynastye_state::machine_reset()
{
	m_video_address = 0;
	std::fill(std::begin(m_video_regs), std::end(m_video_regs), 0);
	m_display = 0;
	std::fill(std::begin(m_palette_bank), std::end(m_palette_bank), 0);
	m_dsw_control = 0xff;
	m_p0 = 0xff;
	fpga_reset();
}

void dynastye_state::program_map(address_map &map)
{
	// FE0DD jumps to 0000:0104 (V5.1G) or 0000:0101 (V1.2); interrupt vectors are at 0000:0000.
	map(0x00000, 0x0ffff).rom().region("boot_prg", 0x30000);
	// Main code sets DS/ES=2000, tests 20000-20fff and sets SS:SP=21c0:0200.
	// V5.1G checks the battery-backed signature/accounting copies at 001c5 and
	// clears the first 4 KiB at 00207 on failure (V1.2: 001c2/00204). The stack
	// is above the retained area.
	map(0x20000, 0x20fff).ram().share("nvram");
	map(0x21000, 0x21fff).ram();
	map(0xf0000, 0xfffff).rom().region("boot_prg", 0x30000);
}

void dynastye_state::io_map(address_map &map)
{
	map(0x01, 0x01).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x02, 0x02).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x03, 0x03).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x04, 0x04).w(FUNC(dynastye_state::video_address_w));
	map(0x05, 0x05).w(FUNC(dynastye_state::video_data_w));
	map(0x08, 0x08).r(FUNC(dynastye_state::dsw_r<0>));
	map(0x09, 0x09).r(FUNC(dynastye_state::dsw_r<1>));
	map(0x0a, 0x0a).r(FUNC(dynastye_state::dsw_r<2>));
	map(0x0b, 0x0b).lw8(NAME([this] (uint8_t data) { m_dsw_control = data; }));
	map(0x0c, 0x0c).lw8(NAME([this] (uint8_t data) { m_display = data; }));
	map(0x0d, 0x0e).lw8(NAME([this] (offs_t offset, uint8_t data) { m_palette_bank[offset] = data & 0x0f; }));
	map(0x10, 0x12).r(FUNC(dynastye_state::input_r));
	// The independent OUT11 latch drives the hopper on bit 7; TEST bit 2
	// supplies its sensor. V5.1G starts/stops it at 05dfd/05e58, while V1.2
	// uses 057fe/05862 and also services automatic payout in its IRQ handler.
	map(0x11, 0x11).lw8(NAME([this] (uint8_t data) { m_hopper->motor_w(BIT(data, 7)); }));
}

uint8_t dynastye_state::input_r(offs_t offset)
{
	return m_inputs[offset]->read();
}

template <uint32_t Bank>
uint8_t dynastye_state::dsw_r()
{
	// Inferred D9203 I/O protocol, not a verified chip identification. 0596b
	// selects ff for normal DIP reads. 0080a/00816 select fc/fb and expect
	// complementary readback at 00828 before accepting the player inputs.
	// V1.2 uses the same sequence at 007fb/00807, comparing at 0081e.
	// Fixed DIP reads instead produce spurious bets in the fallback path.
	// Goldstar's cmv4 also uses nonstandard PPI modes, currently patched in
	// init_cmv4; that workaround does not establish this controller's behavior.
	// TODO: verify the per-bank polarity bits. The observed fc/fb writes do
	// not distinguish bits 0 and 1 or establish the other control bits.
	return m_dsw[Bank]->read() ^ (BIT(m_dsw_control, Bank) ? 0x00 : 0xff);
}

void dynastye_state::video_address_w(uint8_t data)
{
	m_video_address = data & 0x1f;
}

void dynastye_state::video_data_w(uint8_t data)
{
	m_video_regs[m_video_address] = data;
	if (m_video_address == 0x0f)
		blit();
	m_video_address = (m_video_address + 1) & 0x1f;
}

void dynastye_state::blit()
{
	// 0392d constructs eight bytes: X, Y, width-1, height-1, source (24 bits),
	// layer mask/control. 03953 writes them to 08-0f, auto-incrementing the
	// register index. 04026's 8x8 font uses four bytes per row, high nibble first.
	const uint32_t width = uint32_t(m_video_regs[0x0a]) + 1;
	const uint32_t height = uint32_t(m_video_regs[0x0b]) + 1;
	uint32_t source = uint32_t(m_video_regs[0x0c]) | (uint32_t(m_video_regs[0x0d]) << 8) | (uint32_t(m_video_regs[0x0e]) << 16);
	const uint8_t control = m_video_regs[0x0f];
	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			const uint8_t data = source < m_gfxrom.bytes() ? m_gfxrom[source] : 0xff;
			source = (source + 1) & 0xffffff;
			for (uint32_t pixel = 0; pixel < 2; ++pixel)
			{
				const uint8_t source_pen = (data >> (pixel ? 0 : 4)) & 0x0f;
				// 0300c/06bcb clear bit 4 when overlaying symbols on an existing
				// image. Text and rectangular clears set it to replace pen zero.
				if (!BIT(control, 4) && !source_pen)
					continue;
				const uint8_t pen = m_video_regs[0x10 + source_pen] & 0x0f;
				for (uint32_t layer = 0; layer < 2; ++layer)
				{
					if (BIT(control, layer))
						m_bitmap[layer].pix((m_video_regs[9] + y) & 0xff, ((m_video_regs[8] + x) * 2 + pixel) & 0x1ff) = pen;
				}
			}
		}
	}
	// Writes complete synchronously for now. The inferred busy connection to
	// POLL and its duration need evidence before adding a timing model.
}

void dynastye_state::palette_init(palette_device &palette) const
{
	const uint8_t *const prom = memregion("proms")->base();
	// Same GGGGGRRRRRBBBBBx encoding as the later Dyna boards; 256 entries.
	for (uint32_t i = 0; i < 0x100; ++i)
	{
		const uint16_t data = (prom[i] << 8) | prom[0x100 + i];
		palette.set_pen_color(i, pal5bit(data >> 6), pal5bit(data >> 11), pal5bit(data >> 1));
	}
}

void dynastye_state::fpga_reset()
{
	m_config_phase = 0;
	m_config_bit = 0;
	m_config_frames = 0;
	m_config_clocks = 0;
	m_config_shift = 0;
	m_config_length = 0;
	m_config_valid = true;
	m_config_done = false;
}

uint8_t dynastye_state::p0_r()
{
	// P0.0/P0.1 are inputs; the other output pins must retain their levels for TEST1/SET1/CLR1.
	return (m_p0 & ~0x23) | 0x03 | (m_config_done ? 0x20 : 0x00);
}

void dynastye_state::p0_w(uint8_t data)
{
	const uint8_t previous = m_p0;
	m_p0 = data;

	// P0.4=RESET, P0.5=DONE/PROG, P0.6=CCLK, P0.7=DIN.
	// RESET aborts an incomplete stream. After configuration, PROG must also be low.
	// Bootstrap FE0E2 asserts both signals to restart configuration.
	if (!BIT(data, 4))
	{
		if (!m_config_done || !BIT(data, 5))
			fpga_reset();
		return;
	}
	if (BIT(previous, 6) || !BIT(data, 6) || m_config_done || !m_config_valid)
		return;

	// XC2000 slave configuration, Xilinx 1989 Data Book pp. 2-66, 2-67 and 2-70:
	// https://archive.decromancer.ca/bitsavers.org/components/xilinx/_dataBooks/1989_Xilinx_Programmable_Gate_Array_Data_Book.pdf
	// Only the configuration interface is modelled, not the programmed FPGA fabric.
	const uint8_t bit = BIT(data, 7);
	++m_config_clocks;
	m_config_shift = ((m_config_shift << 1) | bit) & 0xffffff;
	switch (m_config_phase)
	{
	case 0: // At least four dummy ones, followed by preamble 0010.
		if ((m_config_shift & 0xff) == 0xf2)
		{
			m_config_phase = 1;
			m_config_bit = 0;
		}
		break;
	case 1: // 24-bit configuration length, most significant bit first.
		if (++m_config_bit == 24)
		{
			m_config_length = m_config_shift;
			m_config_valid = m_config_length >= m_config_clocks + 4 + 196 * 91 + 4;
			m_config_phase = 2;
			m_config_bit = 0;
		}
		break;
	case 2: // At least four dummy ones precede the first frame's start bit.
		if (bit)
		{
			if (m_config_bit < 4)
				++m_config_bit;
		}
		else
		{
			m_config_valid = m_config_bit == 4;
			m_config_phase = 3;
			m_config_bit = 1;
		}
		break;
	case 3: // XC2018: 196 frames, each 0 + 87 data bits + 111.
		if ((m_config_bit == 0 && bit) || (m_config_bit >= 88 && !bit))
			m_config_valid = false;
		if (++m_config_bit == 91)
		{
			m_config_bit = 0;
			if (++m_config_frames == 196)
				m_config_phase = 4;
		}
		break;
	case 4: // Postamble and three additional start-up clocks beyond the length count.
		if (!bit)
			m_config_valid = false;
		if (m_config_bit < 4)
			++m_config_bit;
		if (m_config_valid && m_config_bit == 4 && m_config_clocks >= m_config_length + 3)
			m_config_done = true;
		break;
	}
}

uint32_t dynastye_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// 056dc uploads progressively taller portions of the temple at Y=200 to
	// layer 1. 05738 advances register 03 once per frame (0..112), moving that
	// image upwards under the fixed layer-0 frame. Like the DC4000 register
	// layout, 01/03 are independent vertical offsets; destination blits remain
	// in unscrolled coordinates. The even page selectors stay f1/f3/f5/f7 in
	// both available revisions, and scroll registers 05/07 stay zero.
	bitmap.fill(rgb_t::black(), cliprect);
	if (!BIT(m_display, 3))
		return 0;
	for (int y = cliprect.top(); y <= cliprect.bottom(); ++y)
	{
		const uint16_t *const foreground = &m_bitmap[0].pix((y + m_video_regs[0x01]) & 0xff);
		const uint16_t *const background = &m_bitmap[1].pix((y + m_video_regs[0x03]) & 0xff);
		uint32_t *const dest = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); ++x)
		{
			uint32_t color = 0;
			if (BIT(m_display, 1))
				color = (m_palette_bank[1] << 4) | background[x];
			if (BIT(m_display, 0) && foreground[x])
				color = (m_palette_bank[0] << 4) | foreground[x];
			dest[x] = m_palette->pen_color(color);
		}
	}
	return 0;
}

void dynastye_state::dynastye(machine_config &config)
{
	V35(config, m_maincpu, 20'000'000); // CPU type and clock need PCB confirmation.
	configure_cpu_decryption(*m_maincpu);
	m_maincpu->set_addrmap(AS_PROGRAM, &dynastye_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &dynastye_state::io_map);
	m_maincpu->p0_in_cb().set(FUNC(dynastye_state::p0_r));
	m_maincpu->p0_out_cb().set(FUNC(dynastye_state::p0_w));
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	HOPPER(config, m_hopper, attotime::from_msec(100)); // period unmeasured

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_size(512, 256);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(dynastye_state::screen_update));
	// Vector 18h points to the frame service at 0000:0097. The program
	// enables INTP0 and uses the handler's RAM flag to pace its display work
	// (07fa in V5.1G, 07f3 in V1.2).
	screen.screen_vblank().set_inputline(m_maincpu, NEC_INPUT_LINE_INTP0);
	PALETTE(config, m_palette, FUNC(dynastye_state::palette_init), 0x100);

	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", 1'500'000)); // clock unverified
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW5");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( cb2001 ) // DYNA D9702 PCB; DYNA CO1 V1.1I in bookkeeping screen
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "c01111.11f", 0x020000, 0x20000, CRC(ec6269f1) SHA1(f2428562a10e30192f2c95053f5ce448302e7cf5) )

	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "c0111.12a", 0x000000, 0x80000, CRC(342b760e) SHA1(bc168bec384ccacd73543f604e3ab5b2b8f4f441) )

	ROM_REGION( 0x400, "proms", 0 ) // ?
	ROM_LOAD( "am27s29.9b",  0x000, 0x200, CRC(6c90f6a2) SHA1(f3f592954000d189ded0ed8c6c4444ace0b616a4) )
	ROM_LOAD( "am27s29.11b", 0x200, 0x200, CRC(e5aa3ec7) SHA1(675711dd6788b3d0c37573b49b6297cbcd8c8209) )
ROM_END

// D9203 port test at 046c2 labels three input bytes and six DIP banks.
// The sixth DIP row is hardcoded to zero at 047bc; only five banks are read.
// Individual control assignments and DIP meanings are still being traced.
static INPUT_PORTS_START( dynastye )
	PORT_START("PLAYER")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Player 0") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Player 1") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Player 2") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Player 6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Unknown Coin Input 0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Unknown Coin Input 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Unknown Coin Input 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Unknown Coin Input 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Unknown Coin Input 5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(3)

	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Test 0") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Test 1") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Empty") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service / Port Test")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Bookkeeping" ) PORT_DIPLOCATION("SW1:1") // 047e5 entry / 04829 exit
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Double Up" ) PORT_DIPLOCATION("SW1:2") // V5.1G: 05975/00738; V1.2: 05379/00739
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Win Payout" ) PORT_DIPLOCATION("SW1:5") // V5.1G: 05992/062b8; V1.2: 05396/05cc2
	PORT_DIPSETTING( 0x00, "Hopper" )
	PORT_DIPSETTING( 0x10, "Credits" )
	PORT_DIPNAME( 0x20, 0x20, "Reel Speed" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING( 0x00, DEF_STR( Low ) )
	PORT_DIPSETTING( 0x20, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, "Hopper Sensor" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING( 0x40, "Active Low" )
	PORT_DIPSETTING( 0x00, "Active High" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Bonus Minimum Bet" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING( 0x40, "8" )
	PORT_DIPSETTING( 0x00, "16" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW4:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW4:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW4:8" )

	PORT_START("DSW5")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW5:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW5:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW5:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW5:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW5:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW5:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW5:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW5:8" )

INPUT_PORTS_END

ROM_START( dynastye ) // DYNA D9203 PCB; DYNA DYN3 V5.1G in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "ds_51g.16f", 0x30000, 0x10000, CRC(ebc3397f) SHA1(8870dcf465757f1c4fedabe863fb41e4d42e4952) )

	ROM_REGION( 0x120000, "gfx", 0 )
	ROM_LOAD( "dm9203.1h", 0x000000, 0x100000, CRC(dde16e06) SHA1(0d1949de7b4b51e008bcd89948a1a3cf605c49ca) ) // mask
	ROM_LOAD( "ds_3h.3h",  0x100000, 0x020000, CRC(3d1a7f92) SHA1(2bb85a6738a8ffe29a238f7276b7afd59c5ebafe) ) // D27C010

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s135.11e", 0x000, 0x100, CRC(b1d8c6b4) SHA1(ef9aa1627a5025be3cfaa188c2972ce81e57c474) )
	ROM_LOAD( "82s135.12e", 0x100, 0x100, CRC(b7fa3d99) SHA1(a8102d1637596a13733446dfdb37ec8c13185412) )
ROM_END

ROM_START( dynastye12 ) // DYNA D9203 PCB; DYNA DYN3 V1.2 in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "ds_13d.16f", 0x30000, 0x10000, CRC(465c3d70) SHA1(f11f7120487d90a7a2537e90cb6b1a1a56a99f2d) )

	ROM_REGION( 0x120000, "gfx", 0 )
	ROM_LOAD( "dm9203.1h", 0x000000, 0x100000, CRC(dde16e06) SHA1(0d1949de7b4b51e008bcd89948a1a3cf605c49ca) ) // mask
	ROM_LOAD( "ds_3h.3h",  0x100000, 0x020000, CRC(3d1a7f92) SHA1(2bb85a6738a8ffe29a238f7276b7afd59c5ebafe) ) // D27C010

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s135.11e", 0x000, 0x100, CRC(b1d8c6b4) SHA1(ef9aa1627a5025be3cfaa188c2972ce81e57c474) )
	ROM_LOAD( "82s135.12e", 0x100, 0x100, CRC(b7fa3d99) SHA1(a8102d1637596a13733446dfdb37ec8c13185412) )
ROM_END

ROM_START( scherrym ) // DYNA D9304 PCB; DYNA SCM V5.2 in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "v5.2.11b", 0x20000, 0x10000, CRC(26417279) SHA1(a83b8c050f1a2ab379f69175f2416e6b0b43b940) )
	ROM_RELOAD(                  0x30000, 0x10000)

	ROM_REGION( 0x20000, "gfx", 0 )
	// this PCB has an empty socket near the D3001 custom GFX chip, but it works correctly.
	// Does the D3001 have a stock internal ROM which can be overridden by an external ROM chip?
	ROM_LOAD( "d3001", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.3d", 0x000, 0x100, CRC(16af0d6d) SHA1(a2004091aec05ee85ae8b82766e7c3013ca87bc4) )
	ROM_LOAD( "82s135.2d", 0x100, 0x100, CRC(e87ed5c9) SHA1(ecdfa9586f9daffdb366154b02febcdb535a1427) )
ROM_END

ROM_START( scherrym12 ) // DYNA D9304 PCB; DYNA SCM V1.2 in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "v1.2.11b", 0x20000, 0x10000, CRC(94d452c1) SHA1(a99b54f71318c82a9a5005ff4cc8efc17de6a327) )
	ROM_RELOAD(                  0x30000, 0x10000)

	ROM_REGION( 0x20000, "gfx", 0 )
	// this PCB has an empty socket near the D3001 custom GFX chip, but it works correctly.
	// Does the D3001 have a stock internal ROM which can be overridden by an external ROM chip?
	ROM_LOAD( "d3001", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.3d", 0x000, 0x100, CRC(16af0d6d) SHA1(a2004091aec05ee85ae8b82766e7c3013ca87bc4) )
	ROM_LOAD( "82s135.2d", 0x100, 0x100, CRC(e87ed5c9) SHA1(ecdfa9586f9daffdb366154b02febcdb535a1427) )
ROM_END

ROM_START( scherrymp ) // DYNA D9702 PCB; DYNA PLUS V1.6 in bookkeeping screen
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "supercherrymaster_v1.6d.11f", 0x000000, 0x40000, CRC(8967f58d) SHA1(eb01a16b7d108f5fbe5de8f611b4f77869aedbf1) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "d9701.12c", 0x000000, 0x100000, CRC(07d711a6) SHA1(6b5a4017eb1d31dc184831f85d786331f4a8e01f) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.9b",  0x000, 0x100, CRC(66ed363f) SHA1(65bd37842c441c2e712844b07c0cfe37ef16d0ef) )
	ROM_LOAD( "82s135.11b", 0x100, 0x100, CRC(a19821db) SHA1(62dda90dd67dfbc0b96f161f1f2b7a46a5805eae) )
ROM_END

ROM_START( scherrymp10u ) // DYNA D9702 PCB; DYNA PLUS V1.0U in bookkeeping screen
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "m27c2001.bin", 0x000000, 0x40000, CRC(6e797b3f) SHA1(cc333e3dc2d416f1059559ce958bfe25a3869fc8) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "d9701.12c", 0x000000, 0x100000, CRC(07d711a6) SHA1(6b5a4017eb1d31dc184831f85d786331f4a8e01f) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "82s147.9b",  0x000, 0x200, CRC(dcf976d2) SHA1(73a08e4587f3516d694a8060b79470cf71df3925) )
	ROM_LOAD( "82s147.11b", 0x200, 0x200, CRC(a67e7a63) SHA1(b23e0eb9af13e57bbc8602ddc7fb381ba5c8267e) )
ROM_END

ROM_START( cb4 ) // Wing W4 board + DYNA D9205 subboard; DYNA CB4 V5.0 in bookkeeping screen.
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "5mk.2g",  0x20000, 0x10000, CRC(ecc6f80e) SHA1(b6de63cd5231ef9481ee79d841a6ea591add7e4d) )
	ROM_RELOAD(                 0x30000, 0x10000)

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD16_BYTE( "cb4.4i", 0x000000, 0x040000, CRC(c1799150) SHA1(50e80607b93f6ee35e3e8ff5d854dc83afe76505) )
	ROM_LOAD16_BYTE( "cb4.4j", 0x000001, 0x040000, CRC(3a12cf69) SHA1(232eeca78cdabcd952825aba0ad397e3dde79747) )

	ROM_REGION( 0x400, "proms", 0 ) // not dumped yet
	ROM_LOAD( "82s147.9b",  0x000, 0x200, BAD_DUMP CRC(dcf976d2) SHA1(73a08e4587f3516d694a8060b79470cf71df3925) )
	ROM_LOAD( "82s147.11b", 0x200, 0x200, BAD_DUMP CRC(a67e7a63) SHA1(b23e0eb9af13e57bbc8602ddc7fb381ba5c8267e) )
ROM_END

ROM_START( cb4a ) // DYNA D9300 + DYNA D9205 subboard; DYNA CB4T V1.2 in bookkeeping screen.
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "12fb.2g", 0x20000, 0x10000, CRC(2d0e519a) SHA1(a538e3003d8a008dd45ea9fd10b249744b87f3a6) )
	ROM_RELOAD(                 0x30000, 0x10000)

	ROM_REGION( 0x100000, "gfx", 0 ) // not dumped for this set
	ROM_LOAD16_BYTE( "cb4.4i", 0x000000, 0x040000, BAD_DUMP CRC(c1799150) SHA1(50e80607b93f6ee35e3e8ff5d854dc83afe76505) )
	ROM_LOAD16_BYTE( "cb4.4j", 0x000001, 0x040000, BAD_DUMP CRC(3a12cf69) SHA1(232eeca78cdabcd952825aba0ad397e3dde79747) )

	ROM_REGION( 0x400, "proms", 0 ) // not dumped yet
	ROM_LOAD( "82s147.9b",  0x000, 0x200, BAD_DUMP CRC(dcf976d2) SHA1(73a08e4587f3516d694a8060b79470cf71df3925) )
	ROM_LOAD( "82s147.11b", 0x200, 0x200, BAD_DUMP CRC(a67e7a63) SHA1(b23e0eb9af13e57bbc8602ddc7fb381ba5c8267e) )
ROM_END

ROM_START( cb5 ) // DYNA CB5 V1.4 in bookkeeping screen
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "cb5-140.1g", 0x020000, 0x20000, CRC(05b604a2) SHA1(580b5a0e115fec75e892c07645ae02b7edab2fec) )

	ROM_REGION( 0x100000, "gfx", 0 ) // not dumped for this set, but seems to work fine. Pics of another PCB show D9801 marked on the flash, so it could be different.
	ROM_LOAD( "flash", 0x000000, 0x100000, BAD_DUMP CRC(07d711a6) SHA1(6b5a4017eb1d31dc184831f85d786331f4a8e01f) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "n82s135n.2b", 0x000, 0x100, CRC(502be98c) SHA1(4591d1d5cfe9e83032705139e630dfa5df79689a) )
	ROM_LOAD( "n82s135n.2d", 0x100, 0x100, CRC(bb1865c9) SHA1(58acf909dd6de519d9675482d130b697856e1bf4) )
ROM_END

ROM_START( cb5_13 ) // Wing W4 board + DYNA D9701 subboard; DYNA CB5 V1.3 in bookkeeping screen. Appears to be the missing link to igs/goldstar.cpp hw.
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "cb5-131.1g", 0x020000, 0x20000, CRC(7d47192c) SHA1(bc65f0b3223789fbcd78a7f3ba4f1c0e2a1ee4da) )

	ROM_REGION( 0x100000, "gfx", 0 ) // not dumped for this set, but seems to work fine. Pics of another PCB show D9801 marked on the flash, so it could be different.
	ROM_LOAD( "flash", 0x000000, 0x100000, BAD_DUMP CRC(07d711a6) SHA1(6b5a4017eb1d31dc184831f85d786331f4a8e01f) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "n82s135n.2b", 0x000, 0x100, CRC(502be98c) SHA1(4591d1d5cfe9e83032705139e630dfa5df79689a) )
	ROM_LOAD( "n82s135n.2d", 0x100, 0x100, CRC(bb1865c9) SHA1(58acf909dd6de519d9675482d130b697856e1bf4) )
ROM_END

ROM_START( cb5_11) // Wing W4 board + DYNA D9701 subboard; DYNA CB5 V1.1 in bookkeeping screen.
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "cb5.11d", 0x020000, 0x20000, CRC(ea99dad0) SHA1(eaa899583b199db140dcc0fe750d388996b111a5) )

	ROM_REGION( 0x100000, "gfx", 0 ) // not dumped for this set, but seems to work fine. Pics of another PCB show D9801 marked on the flash, so it could be different.
	ROM_LOAD( "flash", 0x000000, 0x100000, BAD_DUMP CRC(07d711a6) SHA1(6b5a4017eb1d31dc184831f85d786331f4a8e01f) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "n82s135n.2b", 0x000, 0x100, CRC(502be98c) SHA1(4591d1d5cfe9e83032705139e630dfa5df79689a) )
	ROM_LOAD( "n82s135n.2d", 0x100, 0x100, CRC(bb1865c9) SHA1(58acf909dd6de519d9675482d130b697856e1bf4) )
ROM_END

/* New DongmulDongmul 2 (뉴 동물동물 2, New AnimalAnimal 2) runs on slightly different hardware, but with same CPU, custom and I/O.
   Video from the real hardware: https://youtu.be/1K9e_7RzeiM
   _______________________________________________________________________________
  |    ________    ________    ________    __________    _________________     _  |
  |KM6264BLG-10L  |74HC00P| KM6264BLG-10L |29F1610ML|   |  HM27C4096     |    (_) <- Switch (SW8)
  |                                       |__U15____|   |_____U24________|        |
  |  _______________               ___________                                    |
  | | 27C020       |              | DYNA     |         __U23_____  __________     |
  | |___U10________|              | DC3001   |        |N82S147AN| |74HC374AN|  ___|
  |                               |          |                                |
  | __       ___________          |__________|         __U22_____  __________ |___
  ||SW7     | DYNA     |                     Xtal     |N82S147AN| |74HC374AN|   __|
  ||Unused  | CPU 91A  |                   24.000 MHz  __________               __|
  ||__|     |          |                              |74LS245N_|               __|
  |         |__________|                  ___U8_____   __________               __|
  | ________      _________              |PALCE16V8|  |74LS245N_|               __|
  ||74HC04AN     |__SW6___|               ___U7_____   __________               __|
  |          _________                   |GAL16V8D_|  |74LS245N_|               __|
  |         |__SW1___|          ____________________   __________               __|
  |          _________         | DYNA 22A078803    |  |74LS138N_|               __|
  |         |__SW2___|         |___________________|   __________   __________  __|
  |          _________          ____________________  |74LS273N_|  |ULN2003AN|  __|
  |         |__SW3___|         | JFC 95101         |   __________               __|
  |          _________         |___________________|  |74LS273N_|             ____|
  |         |__SW4___|                                                        |
  |          _________                             Xtal                       |
  |         |__SW5___|   _______U1___________   12.000 MHz                    |___
  |         __________  | AT89C51           |                                     |
  | ___     |MACH 110|  |___________________|                                     |
  ||   |    |AMD     |    _________   _________                                   |
  |TDA2009  |________|   |74LS245N_| |74LS245N_|                                  |
  ||___|     _________    ____ ____                                               |
  |         |__SW0___|   PC817 PC817                                              |
  |_______________________________________________________________________________|

*/
ROM_START( ndongmul2 ) // 뉴 동물동물 2 (bootleg MIA 94V-0 PCB; DYNA PLUS V1.2N in bookkeeping screen - based on the Super Cherry Master Plus codebase)
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 ) // CPU91A-011-9915JK001
	ROM_LOAD16_WORD( "am27c020.u10", 0x000000, 0x040000, CRC(550e53e5) SHA1(a90ee66e7ae9b58005b6ed412669d86532c75156) )

	ROM_REGION( 0x001000, "mcu", 0)
	ROM_LOAD( "at89c51.u1",          0x000000, 0x001000, NO_DUMP ) // AT89C51, protected

	ROM_REGION( 0x400000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "hn27c4096.u24",       0x000000, 0x080000, CRC(d6d14e2a) SHA1(ee6d663f7c31fb76fa56d080aa2cf1c690da61b8) )
	ROM_LOAD( "mx29f1610ml.u15",     0x080000, 0x200000, NO_DUMP ) // Flash placement is provisional; external address decoding is unverified.

	ROM_REGION( 0x000400, "proms", 0 )
	ROM_LOAD( "n82s147an.u22",       0x000000, 0x000200, CRC(54b76f79) SHA1(d8eca94fda3436a204e71869a88fba5fc4daed18) )
	ROM_LOAD( "n82s147an.u23",       0x000200, 0x000200, CRC(2e3063c8) SHA1(b1b3d23063faabe7f588dfafe4a1439573d41cb4) )

	ROM_REGION( 0x000400, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "palce16v8h-25.u8",    0x000000, 0x000117, CRC(f1de9b90) SHA1(3b2e76e1f6dc34d16fa1dded9bc8205683e59c0c) )
	ROM_LOAD( "gal16v8d.u7",         0x000200, 0x000117, CRC(55e39258) SHA1(4546fdbd343290c2a7953b4cd0f8db5aab2fad18) )
ROM_END

ROM_START( mystjb ) // DYNA D9702 PCB; DYNA MYST V1.3G in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "mjb_3g.11f", 0x20000, 0x20000, CRC(b67d1311) SHA1(7694bee009c5720dde65226ec19544c5e6e5077e) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD( "mjb_1g.12b", 0x00000, 0x80000, CRC(00244991) SHA1(952095011ff766018b9ad310afc9dbd3a4c8f5a8) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.9b",  0x000, 0x100, CRC(777d4f6e) SHA1(6825514676ff1c2a2bef7c67819787d698eb32bf) )
	ROM_LOAD( "82s135.11b", 0x100, 0x100, CRC(ee087df2) SHA1(b60a1097e997459e788dcfafe15237c95a55bbed) )
ROM_END

ROM_START( tripjack ) // DYNA D9805 PCB; DYNA TRJ V1.6G in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "27c2001.11f", 0x00000, 0x40000, CRC(f7b6226a) SHA1(f01329eff728547c369075a1bc0c2455438499a5) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD( "27c4002.12b", 0x00000, 0x80000, CRC(5b6221a9) SHA1(b777cc4aace17969d0357ba5e8c75c9f8b596da4) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.9b",  0x000, 0x100, CRC(2d2237fb) SHA1(9b71801bd465d2a823f648f4d3c1823b5ba3340e) )
	ROM_LOAD( "82s135.11b", 0x100, 0x100, CRC(9940ef22) SHA1(42b0c6410d8db34e0316e95b7b7007abc3098341) )
ROM_END

ROM_START( tripjack11 ) // DYNA D9702 PCB; DYNA TRJ V1.1 in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "tj_11d.11f", 0x20000, 0x20000, CRC(cf27aaa8) SHA1(1c8dd217992ecdc4a94572e0b53fb2f82b7db281) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD( "tj_1d.12b", 0x00000, 0x80000, CRC(82f39c9a) SHA1(063e5747bf69266eaee977f48a0e7d5e642d070e) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.9b",  0x000, 0x100, CRC(2d2237fb) SHA1(9b71801bd465d2a823f648f4d3c1823b5ba3340e) )
	ROM_LOAD( "82s135.11b", 0x100, 0x100, CRC(9940ef22) SHA1(42b0c6410d8db34e0316e95b7b7007abc3098341) )
ROM_END

ROM_START( tripjack07 ) // DYNA D9702 PCB; DYNA TRJ2 V0.7 in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "11f", 0x20000, 0x20000, CRC(618f1bac) SHA1(e39c96333e91f3ef001a31646dbe31fa7befa3c9) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD( "tj_1d.12b", 0x00000, 0x80000, CRC(82f39c9a) SHA1(063e5747bf69266eaee977f48a0e7d5e642d070e) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.9b",  0x000, 0x100, CRC(2d2237fb) SHA1(9b71801bd465d2a823f648f4d3c1823b5ba3340e) )
	ROM_LOAD( "82s135.11b", 0x100, 0x100, CRC(9940ef22) SHA1(42b0c6410d8db34e0316e95b7b7007abc3098341) )
ROM_END

ROM_START( crzybell ) // DYNA D9401 PCB; DYNA CRBL1 V1.2D in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "cbl12d.6e", 0x20000, 0x20000, CRC(a5d43b00) SHA1(b89fd8eb7675b3dd4f47bf95326da1b881997a56) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD( "cbl1d.2g", 0x00000, 0x80000, CRC(83c6a91f) SHA1(3f802777e4a5581f7efe86309992f0e7a79851ee) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.j5", 0x000, 0x100, CRC(a70ba127) SHA1(08ed172038b2f3f237093d46f48d612c6132cfb3) )
	ROM_LOAD( "82s135.k5", 0x100, 0x100, CRC(6e653f22) SHA1(d9a72c292d07976897f44a9b776907a9a5400f40) )
ROM_END

ROM_START( nmondop ) // DYNA D9702 PCB; DYNA NMP V0.6I in bookkeeping screen
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "nmp12i.11f", 0x000000, 0x40000, CRC(a02d70f9) SHA1(26de0e09432fa5cccef502553bc6d65ec179c7a4) )

	// C780A-C7828 selects OUT32=10, as does scherrymp at C78C4-C78E4.
	// Their 14 reel-symbol maps (CA302 / CA442, 14 bytes each) are identical:
	// ten consecutive 8x32 strips per symbol, with two blank columns at each
	// side. In scherrymp these correctly address graphics at 80000-87fff;
	// the supplied image ends at 7ffff and contains no equivalent native
	// reel bank under the current decoder.
	// The first three 8x8 pages also share much of scherrymp's graphics.
	// The reel source/bank selection is unresolved. An incomplete external
	// dump and an undumped internal source are both possible; this does not
	// establish that the supplied external ROM is bad. A D3001 (and any
	// internal ROM) on this board still needs physical-board confirmation.
	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "nmp1i.12a", 0x000000, 0x80000, CRC(291ca4d1) SHA1(404439c0e73098e253160af1d36f7ceb7f98f49d) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "9b",  0x000, 0x100, CRC(d31a9e0a) SHA1(7a72df263cd75d7b83ec74e8d272447f609e6ef6) )
	ROM_LOAD( "11b", 0x100, 0x100, CRC(cf66ea05) SHA1(5cab7cf203b2b5987c4ae27a3baa61f3bb088872) )
ROM_END

ROM_START( pdouble8 ) // DYNA D9304 PCB; DYNA SCM V5.3FA 1993 in bookkeeping screen, but Premium Double Eight 1996 on title screen
	ROM_REGION16_LE( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "scm_53f.11b", 0x20000, 0x10000, CRC(e078c8d1) SHA1(f7fc42a495987900e1b6864e3b9a6f0dc05a3661) )
	ROM_RELOAD(                     0x30000, 0x10000)

	// This dump contains coherent foreground/title artwork. E79EF selects
	// reel bank 04, but none of its eight 32 KiB banks supplies the symbols.
	// Gallery banks 06-08 are also unresolved. Suspect the D3001's internal
	// graphics (see scherrym); external/internal selection still needs evidence.
	ROM_REGION( 0x40000, "gfx", 0 )
	ROM_LOAD( "db_1d.7c", 0x00000, 0x40000, CRC(c302a66b) SHA1(c376b9d66d0111df1647a33d488cde2097478b44) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.3d", 0x000, 0x100, CRC(be321344) SHA1(a4f092faa4751b28bb28f7ba278022406133f411) )
	ROM_LOAD( "82s135.2d", 0x100, 0x100, CRC(43b5fe34) SHA1(81fd9767443f04a96b2eb879a20168cd5de83004) )
ROM_END

ROM_START( bsbonanza ) // DYNA D9902 PCB; DYNA DYNA BSB  V1.1G in bookkeeping screen. Has no DIPs and no PROMs.
	ROM_REGION( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD( "bsb_11g.4c", 0x00000, 0x40000, CRC(6d58b5db) SHA1(ed4f44ecdd66efc433d956f4c571492affa77ff3) )
	// also seen with "DYNA CASINO HALL HIRANO OSAKA JAPAN" disclaimer text with following hash
	// CRC(0ceae7c6) SHA1(2a5d216761c2057ad4c17248e872973eec765784)
	// this second PCB also had 2 banks of 8 switches present

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD16_BYTE( "bsb_1d.12e", 0x00000, 0x40000, CRC(bb74f46c) SHA1(7a969df9adf28c1fa747e553b6cda99cc866ee0c) )
	// 11e not populated
	ROM_LOAD16_BYTE( "bsb_3g.9e",  0x00001, 0x40000, CRC(30e2065e) SHA1(6ada4b4c2ff84f1213ced0ce3603c90d7b17c43e) )
	// 7e not populated

	// no PROMs on PCB
ROM_END

ROM_START( bsbonanza10 ) // DYNA D9902 PCB; DYNA DYNA BSB  V1.0G in bookkeeping screen. Has no PROMs.
	ROM_REGION( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD( "bsb_10g.4c", 0x00000, 0x40000, CRC(1f48ad0d) SHA1(aae8229bce307b9f3ead3f6481da11bcb914566f) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD16_BYTE( "bsb_1d.12e", 0x00000, 0x40000, CRC(bb74f46c) SHA1(7a969df9adf28c1fa747e553b6cda99cc866ee0c) )
	// 11e not populated
	ROM_LOAD16_BYTE( "bsb_3g.9e",  0x00001, 0x40000, CRC(30e2065e) SHA1(6ada4b4c2ff84f1213ced0ce3603c90d7b17c43e) )
	// 7e not populated

	// no PROMs on PCB
ROM_END


void cb2001_state::init_smaller_proms()
{
	uint8_t *const proms = memregion("proms")->base();
	std::array<uint8_t, 0x400> buffer;
	std::copy_n(proms, buffer.size(), buffer.begin());

	for (int i = 0; i < 0x400; i++)
	{
		if (!(i & 0x20))
			proms[i] = buffer[(i & 0x1f) | ((i & 0x3c0) >> 1)];
		else
			proms[i] = 0x00;
	}

	m_palette->update();
}

} // anonymous namespace


//    YEAR  NAME          PARENT     MACHINE     INPUT       CLASS               INIT                ROT   COMPANY                   FULLNAME                           FLAGS
GAME( 2000, cb2001      , 0        , cherry2001, cherry2001, cherry2001_state, empty_init        , ROT0, "Dyna"                 , "Cherry Bonus 2001 (V1.1I)"       , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, ndongmul2   , 0        , ndongmul2 , ndongmul2 , mystjb_state    , empty_init        , ROT0, "Dyna"                 , "New DongmulDongmul 2 (V1.2N)"    , MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // Undumped MCU and graphics flash; flash banking remains unverified.
GAME( 1992, dynastye    , 0        , dynastye  , dynastye  , dynastye_state  , empty_init        , ROT0, "Dyna"                 , "Dynasty (1992, V5.1G)"           , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1992, dynastye12  , dynastye , dynastye  , dynastye  , dynastye_state  , empty_init        , ROT0, "Dyna"                 , "Dynasty (1992, V1.2)"            , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1993, scherrym    , 0        , scherrym  , cb2001    , cb2001_state    , init_smaller_proms, ROT0, "Dyna"                 , "Super Cherry Master (V5.2)"      , MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1993, scherrym12  , scherrym , scherrym  , cb2001    , cb2001_state    , init_smaller_proms, ROT0, "Dyna"                 , "Super Cherry Master (V1.2)"      , MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1997, scherrymp   , 0        , scherrymp , scmplus   , scherrymp_state , init_smaller_proms, ROT0, "Dyna"                 , "Super Cherry Master Plus (V1.6)" , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, scherrymp10u, scherrymp, scherrymp , scmplus   , scherrymp_state , empty_init        , ROT0, "Dyna"                 , "Super Cherry Master Plus (V1.0U)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1993, cb4         , 0        , cb4       , cb4       , cb4_state       , empty_init        , ROT0, "Dyna"                 , "Cherry Bonus IV (V5.0)"          , MACHINE_IMPERFECT_SOUND | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1993, cb4a        , cb4      , cb4       , cb4a      , cb4_state       , empty_init        , ROT0, "Dyna"                 , "Cherry Bonus IV (V1.2)"          , MACHINE_IMPERFECT_SOUND | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1997, cb5         , 0        , cb5       , cb5       , cb5_state       , init_smaller_proms, ROT0, "Dyna"                 , "Cherry Bonus V Five (V1.4)"      , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, cb5_13      , cb5      , cb5       , cb5       , cb5_state       , init_smaller_proms, ROT0, "Dyna"                 , "Cherry Bonus V Five (V1.3)"      , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, cb5_11      , cb5      , cb5       , cb5       , cb5_state       , init_smaller_proms, ROT0, "Dyna"                 , "Cherry Bonus V Five (V1.1)"      , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1998, mystjb      , 0        , mystjb    , mystjb    , mystjb_state    , init_smaller_proms, ROT0, "Dyna"                 , "Mystery J & B (V1.3G)"           , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1998, tripjack    , 0        , tripjack16, tripjack  , mystjb_state    , init_smaller_proms, ROT0, "Dyna"                 , "Triple Jack (V1.6G)"             , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1998, tripjack11  , tripjack , tripjack  , tripjack11, mystjb_state    , init_smaller_proms, ROT0, "Dyna"                 , "Triple Jack (V1.1)"              , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1998, tripjack07  , tripjack , tripjack  , tripjack07, mystjb_state    , init_smaller_proms, ROT0, "Dyna"                 , "Triple Jack (V0.7)"              , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1995, crzybell    , 0        , crzybell  , crzybell  , crzybell_state  , init_smaller_proms, ROT0, "Dyna"                 , "Crazy Bell (V1.2D)"              , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1998, nmondop     , 0        , nmondop   , nmondop   , mystjb_state    , init_smaller_proms, ROT0, "Dyna"                 , "New Mondo Plus (V0.6I)"          , MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1996, pdouble8    , 0        , pdouble8  , pdouble8  , pdouble8_state  , init_smaller_proms, ROT0, "Dyna & MK Electronics", "Premium Double Eight (V5.3FA)"   , MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 2001, bsbonanza   , 0        , bsbonanza , bsbonanza , bsbonanza_state , empty_init        , ROT0, "Dyna"                 , "Bonus Spin Bonanza (V1.1G)"      , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, bsbonanza10 , bsbonanza, bsbonanza , bsbonanza , bsbonanza_state , empty_init        , ROT0, "Dyna"                 , "Bonus Spin Bonanza (V1.0G)"      , MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
