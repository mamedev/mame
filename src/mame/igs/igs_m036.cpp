// license:BSD-3-Clause
// copyright-holders: David Haywood

/* IGS Gambling games using IGS036 CPU
   pgm2.cpp also uses this CPU

   Preliminary emulation for the sets with dumped internal ROMs:
   - The original ARM boot code uploads the key and checks the program ROM.
     The cipher hardware uses igs036_decryptor, as on PGM2; no boot checks are patched.
   - Atmel-style AIC and timer counters; vblank uses IRQ2, TC0-2 use IRQ6-8.
   - Packed 7bpp sprites, provisional RLE, RGB555 palette, 512x240 / 800x600 video.
   - Several external bus layouts, GPIO input matrices, TT5665 / banked M6295.
   - Battery-backed SRAM, coin hoppers and Hou Ye Leyuan's ticket dispenser.

   TODO: correct compressed/animated sprites (especially HD), zoom/blending,
   display control, verify CPU/peripheral clocks, complete timer modes, serial
   links, real-time timer, lamps/counters and remaining DIP settings. Mechanical
   payout timing is provisional; alternate control modes still need checking.

   The 20 MHz oscillator is documented on several boards; the CPU's internal
   PLL/multiplier has not been established. PGM2's 100 MHz is also marked unknown.
   HD boards provisionally use 100 MHz: their ROMs select 0x1b rather than 0x18
   at 0xfffffa0c, and 20 MHz leaves Hou Ye Leyuan's sprite rebuild unfinished
   on alternate frames. The register's PLL/divider fields still need decoding.

   Is the video hardware here part of IGS036 that goes unused by PGM2?

   Operator password recovery
   --------------------------
   These fixed recovery sequences were traced in the decrypted external program
   ROMs and verified through the original menus for the revisions listed below.
   They are distinct from the operator's configurable password. ROM addresses
   refer to the decrypted CPU view, not necessarily plaintext in the ROM dump.

   Joystick codes use the on-screen digit legend, not keyboard number keys:
     0 = Start, 1 = S1, 2 = S2/Bet, 3 = S3,
     4 = Up, 5 = Down, 6 = Left, 7 = Right.
   Press and release each button separately.

     Set       Revision   Joystick code   Recovery sequence in ROM
     cjddzsp   V122CN     22775047        0x1007c248
     cjbq      V205CN     22775047        0x10072c4c
     dahuaxy   V201CN     22775047        0x1007b0a8
     zjddz     V204CN     22775047        0x1009b984
     xyddz     V216CN     22775047        0x100dad70
     hlddz     V104CN     22775047        0x1009b870
     cjgdy     V110CN     54163274        0x10066224
     mjzhizun  S100CN     13120021        0x10068174
     lhtb      S101CN     21301330        0x100536f8
     sydh      V104CN     24064400        0x1006f648
     lhfy      V206CN     55543210        0x10091618
     slqzsp    V104CN     31123132        0x1004c39c
     qhzb      V100CN     31123132        0x100412f8

   Mahjong controls:
     slqzsp V104CN: A, H, K, D, H, C, M, B  (ROM 0x1004c394)
     qhzb   V100CN: A, H, K, D, H, C, M, B  (ROM 0x100412f0)
     jhzb   V113CN: Bet, Kan, Start, Chi, Kan, Kan, Start, Start
                   (ROM 0x100297d4; select Mahjong controls and restart).
   The jhzb recovery sequence is not directly enterable with its default
   joystick mapping.

   hyleyuan S103CN:
     Test, Test, Test, Service Key, Service Key, Test, Test, Test
     (ROM 0x1003b86c).

   Enter these sequences at System Adjustment / System Settings, reached from
   Bookkeeping (Test for hyleyuan). Most open Set/Change Password directly.
   slqzsp, qhzb and lhfy accept the code and then offer Bet to change it;
   hyleyuan opens a Continue / Set Password menu. Enter the new eight-button
   sequence twice, as prompted. In slqzsp/qhzb Mahjong mode, Bookkeeping -> F
   selects System Adjustment; joystick mode uses Down and S1. In jhzb Mahjong
   mode, Kan/Pon move up/down and Bet selects System Settings.

   Verified initial-state sequences with fresh NVRAM:
     slqzsp:  Start eight times opens first-time password setup.
     cjddzsp: Start eight times is accepted; Bet opens Change Password.
     hyleyuan: Test eight times (the initial stored password).
   These initial sequences are not recovery codes for a changed password.

   Configurable passwords are eight bytes of button IDs or digit indices in
   external RAM at 0x20000000, persisted by the "mainram" NVRAM device. slqzsp
   and qhzb use 0x200002d8 for Mahjong and 0x200002e0 for joystick, with the
   password-set flag at 0x200002d4. Normal emulator exit saves this bank.
   Changing passwords through the menus and accepting them after a cold restart
   was verified in slqzsp and cjddzsp; their fixed recovery codes also remain
   usable after a configurable password has been set.

   No password route or recovery code has been confirmed for huoqilin or jxry.

<Chaoji Da Heng 2>
cjdh2.zip
---------------------------------------------------
CPU: IGS036
GFX: IGS036
SND: 6295
==============================================
a IGS game use IGS036 chip
IGS036 could be a upgraded version of IGS027A
but with GFX processor integrated

I don't know the CPU core (should be ARM based due to fail test)
the chip has internal rom built-in
the FLASH(u33, EV29LV160AB-90PCR) is external rom and encrypted
if the external rom is decrypted then we can
try to trojan the internal rom
here we offer several revisions of the same game to see
if anyone could find any clue, these 4 revision can
be programmed and running on a same PCB.
===================================================
filename                        |
---------------------------------------------------
cjdh2_s215cn.u33                | PRG (Ver S215CN)
cjdh2_s311cn.rom                | PRG (Ver S311CN)
cjdh2_s311cna.rom               | PRG (Ver S311CNA)
cjdh2_s311cnb.rom               | PRG (Ver S311CNB)
cjdh2_cg1.u8                    | GFX ROM
cjdh2_cg2.u24                   | GFX ROM
cjdh2_sp.u20                    | SND ROM
DSC00257.JPG                    | PCB Photo
---------------------------------------------------

(dump by XingXing)


<Super Dou Di Zhu Special>
cjddzsp.zip
---------------------------------------------------
CPU: IGS036
GFX: IGS036
SND: TT5665
----------------------------------------------------------------------------
Filename            TYPE        CRC32           SHA1
cjddzsp_s122cn.u27  GFX     797e5ba3    784fae513ac8cfd1143f0d0ce0936f74e2e64e48
cjddzsp_s122cn.u28  GFX     d0441a6b    e1c948f94472398aa5887963cf8e87be28dd66e0
cjddzsp_s122cn.u30  SND     e0e02a57    96074a5226dd24d0bc150adff7324b5349cb5dc2
cjddzsp_s122cn.u18  PRG     4a42aad6    96805e5bfbd50686177fe50020229ea8787ade17
----------------------------------------------------------------------------
check more info and photo from cjdh2.zip!!!

(dump by XingXing)

*/

#include "emu.h"

#include "igs036.h"
#include "igs036crypt.h"
#include "igsmahjong.h"

#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"
#include "sound/tt5665.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "endianness.h"
#include "multibyte.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <vector>


namespace {

class m036_state : public driver_device
{
public:
	m036_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	void m036_tt(machine_config &config) ATTR_COLD;
	void m036(machine_config &config) ATTR_COLD;

	void init_m036() ATTR_COLD;
	template <const uint8_t (&Key)[0x100]> void init_key() ATTR_COLD;
	void init_mghammer() ATTR_COLD;

private:
	uint32_t screen_update_m036(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<igs036_cpu_device> m_maincpu;

	void pgm_create_dummy_internal_arm_region() ATTR_COLD;

	void m036_map(address_map &map) ATTR_COLD;
};


// Common IGS036 peripherals for boards with a dumped internal ROM.
class m036_base_state : public driver_device
{
public:
	void init_jxry() ATTR_COLD;

protected:
	m036_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_external_rom(*this, "user1"),
		m_gfx(*this, "gfx"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_service(*this, "SERVICE"),
		m_direct(*this, "DIRECT"),
		m_keys(*this, "KEY%u", 0U),
		m_dsw(*this, "DSW%u", 1U) { }

	// driver_device implementation
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	void base_config(machine_config &config) ATTR_COLD;
	void m036_boot_map(address_map &map) ATTR_COLD;

	template <bool Wide> u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u16 keys_r(u16 select);
	u32 inputs_r();
	u8 dips_r(offs_t offset);
	u32 dips_packed_r();
	void key_select_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void gpio_outputs_w(u32 data);

	required_device<igs036_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_memory_region m_external_rom;
	required_region_ptr<u8> m_gfx;
	required_shared_ptr<u32> m_spriteram;
	required_shared_ptr<u32> m_paletteram;
	required_ioport m_service;
	required_ioport m_direct;
	required_ioport_array<5> m_keys;
	required_ioport_array<3> m_dsw;

	u32 m_pio_out;
	u16 m_key_select;

private:
	std::unique_ptr<u8 []> m_encrypted_copy;

	void restore_decrypted_state();
	void decrypt(int state);

	void screen_vblank(int state);
};

class hopper_state : public m036_base_state
{
public:
	int hopper_r() { return m_hopper->line_r(); }

protected:
	required_device<hopper_device> m_hopper;

	hopper_state(const machine_config &mconfig, device_type type, const char *tag)
		: m036_base_state(mconfig, type, tag),
		m_hopper(*this, "hopper") { }

	void hopper_config(machine_config &config) ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void gpio_hopper_w(u32 data);
};

class m036_oki_state : public hopper_state
{
public:
	m036_oki_state(const machine_config &mconfig, device_type type, const char *tag)
		: hopper_state(mconfig, type, tag),
		m_oki(*this, "oki"),
		m_oki_rom(*this, "oki"),
		m_oki_banks(*this, "oki_bank%u", 0U) { }

	void m036_boot(machine_config &config) ATTR_COLD;
	void m036_oki_2mhz(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<okim6295_device> m_oki;
	required_region_ptr<u8> m_oki_rom;
	required_memory_bank_array<2> m_oki_banks;

	void m036_oki_boot_map(address_map &map) ATTR_COLD;
	void oki_rom_map(address_map &map) ATTR_COLD;
	void oki_banks_w(u8 data);
};

class m036_tt_state : public hopper_state
{
public:
	m036_tt_state(const machine_config &mconfig, device_type type, const char *tag)
		: hopper_state(mconfig, type, tag),
		m_tt5665(*this, "tt5665") { }

	void m036_tt_boot(machine_config &config) ATTR_COLD;
	void m036_sydh(machine_config &config) ATTR_COLD;
	void m036_sram(machine_config &config) ATTR_COLD;
	void m036_cjgdy(machine_config &config) ATTR_COLD;
	void m036_am001(machine_config &config) ATTR_COLD;

protected:
	required_device<tt5665_device> m_tt5665;

	void tt_config(machine_config &config) ATTR_COLD;

private:
	void m036_tt_boot_map(address_map &map) ATTR_COLD;
	void sydh_map(address_map &map) ATTR_COLD;
	void extra_sram_map(address_map &map) ATTR_COLD;
	void am001_map(address_map &map) ATTR_COLD;
};

class m036_mj_state : public m036_tt_state
{
public:
	m036_mj_state(const machine_config &mconfig, device_type type, const char *tag)
		: m036_tt_state(mconfig, type, tag) { }

	void m036_mjzhizun(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u16 m_secondary_select;

	void mjzhizun_map(address_map &map) ATTR_COLD;
	void outputs_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
};

class m036_hd_base_state : public m036_base_state
{
protected:
	required_device<tt5665_device> m_tt5665;
	required_ioport_array<5> m_scan;

	m036_hd_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: m036_base_state(mconfig, type, tag),
		m_tt5665(*this, "tt5665"),
		m_scan(*this, "SCAN%u", 0U) { }

	void hd_config(machine_config &config) ATTR_COLD;
	template <u32 Rows> u32 scan_inputs_r();
};

class m036_hd_state : public m036_hd_base_state
{
public:
	m036_hd_state(const machine_config &mconfig, device_type type, const char *tag)
		: m036_hd_base_state(mconfig, type, tag),
		m_hopper(*this, "hopper") { }

	void m036_hd(machine_config &config) ATTR_COLD;
	int hopper_r() { return m_hopper->line_r(); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<hopper_device> m_hopper;
	u16 m_secondary_select;

	void hd_map(address_map &map) ATTR_COLD;
	u32 gpio_inputs_r();
	void outputs_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
};

class m036_hyleyuan_state : public m036_hd_base_state
{
public:
	m036_hyleyuan_state(const machine_config &mconfig, device_type type, const char *tag)
		: m036_hd_base_state(mconfig, type, tag),
		m_ticket(*this, "ticket") { }

	void m036_hyleyuan(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<ticket_dispenser_device> m_ticket;

	void hyleyuan_map(address_map &map) ATTR_COLD;
};


uint32_t m036_state::screen_update_m036(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


template <bool Wide>
u32 m036_base_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);
	// Convert each palette entry once, rather than for every opaque pixel.
	u32 colours[0x2000];
	for (u32 i = 0; i < std::size(colours); ++i)
	{
		u32 const colour = m_paletteram[i];
		colours[i] = rgb_t(pal5bit(colour >> 10), pal5bit(colour >> 5), pal5bit(colour));
	}

	// Four packed 7bpp pixels per 32-bit word. In compressed sprites the top
	// nibble is a repeat count (0 = one group, 15 = sixteen groups).
	// The 800x600 boards fetch two consecutive words from four 16-bit ROMs.
	// TODO: zoom, blending, priority and display/window control.
	u32 const half = m_gfx.bytes() / 2;
	constexpr u32 GROUPS_PER_ADDRESS = Wide ? 2 : 1;
	constexpr u32 PIXELS_PER_ADDRESS = GROUPS_PER_ADDRESS * 4;
	auto const read_group = [this, half] (u32 address, u32 index, bool reverse)
	{
		u32 const offset = (reverse ? address - index / GROUPS_PER_ADDRESS : address + index / GROUPS_PER_ADDRESS) * 2;
		u32 const lane = (index % GROUPS_PER_ADDRESS) ^ (reverse ? GROUPS_PER_ADDRESS - 1 : 0);
		u32 const low_base = Wide ? lane * half : half;
		u32 const high_base = Wide ? low_base + half / 2 : 0;
		if (offset + 1 >= (Wide ? half / 2 : half))
			return 0U;
		return u32(get_u16le(&m_gfx[low_base + offset])) | (u32(get_u16le(&m_gfx[high_base + offset])) << 16);
	};

	for (u32 i = 0; i < 0x1000; i += 4)
	{
		u32 const pos = m_spriteram[i];
		u32 const size = m_spriteram[i + 1];
		u32 const attr = m_spriteram[i + 2];
		if (attr & 0x7400'0000) // attribute modes not yet understood
			continue;

		int const x = util::sext(pos >> 16, 11);
		int const y = util::sext(pos, 11);
		u32 const width = ((size & 0x1ff) + 1) * PIXELS_PER_ADDRESS;
		u32 const height = ((size >> 16) & 0x7ff) + 1;
		if (x > cliprect.right() || y > cliprect.bottom() || x + int(width) <= cliprect.left() || y + int(height) <= cliprect.top())
			continue;
		u32 const address = (attr & 0xffff) | (m_spriteram[i + 3] & 0x03ff'0000);
		u32 const palette = ((attr >> 16) & 0x3f) * 0x80;
		bool const compressed = !BIT(attr, 27);
		bool const reverse = !BIT(attr, 25);
		bool const flipx = !BIT(attr, 24);
		u32 const *const pens = &colours[palette];
		u32 index = 0, pixels = 0;
		u32 repeats = 0;

		// Raw sprites can seek straight to the first visible row. RLE sprites
		// must consume the preceding groups to preserve runs crossing a row.
		u32 const first_y = compressed ? 0 : std::max(0, cliprect.top() - y);
		index = first_y * (width / 4);
		for (u32 sy = first_y; sy < height && y + int(sy) <= cliprect.bottom(); ++sy)
		{
			int const dy = y + int(sy);
			u32 *const dest = (dy >= cliprect.top()) ? &bitmap.pix(dy) : nullptr;
			for (u32 sx = 0; sx < width; sx += 4)
			{
				if (!repeats)
				{
					pixels = read_group(address, index++, reverse);
					repeats = compressed ? (pixels >> 28) + 1 : 1;
					// Put the four pens in left-to-right order for the hot draw path.
					if (reverse != flipx)
						pixels = ((pixels >> 21) & 0x000'007f) | ((pixels >> 7) & 0x000'3f80)
								| ((pixels << 7) & 0x01f'c000) | ((pixels << 21) & 0xfe0'0000);
				}
				--repeats;
				int const dx = x + int(flipx ? width - 4 - sx : sx);
				if (dy < cliprect.top() || dx > cliprect.right() || dx + 3 < cliprect.left())
					continue;
				if (dx >= cliprect.left() && dx + 3 <= cliprect.right())
				{
					if (u8 const pen = (pixels >>  0) & 0x7f)
						dest[dx] = pens[pen];
					if (u8 const pen = (pixels >>  7) & 0x7f)
						dest[dx + 1] = pens[pen];
					if (u8 const pen = (pixels >> 14) & 0x7f)
						dest[dx + 2] = pens[pen];
					if (u8 const pen = (pixels >> 21) & 0x7f)
						dest[dx + 3] = pens[pen];
				}
				else
				{
					for (u32 p = 0; p < 4; ++p)
					{
						u8 const pen = (pixels >> (p * 7)) & 0x7f;
						if (pen && dx + int(p) >= cliprect.left() && dx + int(p) <= cliprect.right())
							dest[dx + int(p)] = pens[pen];
					}
				}
			}
		}
	}
	return 0;
}


void m036_base_state::machine_start()
{
	m_encrypted_copy = std::make_unique<u8 []>(m_external_rom->bytes());
	std::copy_n(m_external_rom->base(), m_external_rom->bytes(), m_encrypted_copy.get());

	save_item(NAME(m_key_select));
	save_item(NAME(m_pio_out));
}


void m036_base_state::machine_reset()
{
	m_key_select = 0xffff;
	m_pio_out = 0;
	restore_decrypted_state();
}


void m036_base_state::device_post_load()
{
	restore_decrypted_state();
}

void m036_base_state::decrypt(int state)
{
	restore_decrypted_state();
}

void m036_base_state::restore_decrypted_state()
{
	std::copy_n(m_encrypted_copy.get(), m_external_rom->bytes(), m_external_rom->base());
	if (m_maincpu->get_has_decrypted())
	{
		igs036_decryptor decrypter(m_maincpu->get_encryption_table());
		auto const rom = util::little_endian_cast<u16>(reinterpret_cast<u32 *>(m_external_rom->base()));
		for (u32 i = 0; i < m_external_rom->bytes() / 2; ++i)
			rom[i] = decrypter.decrypt(rom[i], i);
	}
}

void hopper_state::machine_reset()
{
	m036_base_state::machine_reset();
	m_hopper->motor_w(0);
}

void hopper_state::gpio_hopper_w(u32 data)
{
	gpio_outputs_w(data);
	m_hopper->motor_w(!BIT(data, 2));
}

void m036_oki_state::machine_start()
{
	hopper_state::machine_start();
	for (auto &bank : m_oki_banks)
	{
		for (u32 entry = 0; entry < 16; ++entry)
			bank->configure_entry(entry, &m_oki_rom[(entry * 0x2'0000) % m_oki_rom.bytes()]);
	}
}

void m036_oki_state::machine_reset()
{
	hopper_state::machine_reset();
	oki_banks_w(0);
}

void m036_oki_state::oki_banks_w(u8 data)
{
	m_oki_banks[0]->set_entry(BIT(data, 0, 4));
	m_oki_banks[1]->set_entry(BIT(data, 4, 4));
}

void m036_oki_state::oki_rom_map(address_map &map)
{
	map(0x0'0000, 0x1'ffff).bankr(m_oki_banks[0]);
	map(0x2'0000, 0x3'ffff).bankr(m_oki_banks[1]);
}

u16 m036_base_state::keys_r(u16 select)
{
	u16 keys = 0xffff;
	for (u32 row = 0; row < m_keys.size(); ++row)
	{
		if (!BIT(select, row))
			keys &= m_keys[row]->read() | 0xffc0;
	}
	return keys;
}

u32 m036_base_state::inputs_r()
{
	return m_service->read() | (u32(m_direct->read() & keys_r(m_key_select)) << 16);
}

u8 m036_base_state::dips_r(offs_t offset)
{
	return (offset < 6 && !BIT(offset, 0)) ? m_dsw[offset / 2]->read() : 0xff;
}

u32 m036_base_state::dips_packed_r()
{
	return m_dsw[0]->read() | (m_dsw[1]->read() << 8) | (m_dsw[2]->read() << 16) | 0xff00'0000;
}

void m036_base_state::key_select_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_key_select);
}

void m036_base_state::gpio_outputs_w(u32 data)
{
	m_pio_out = data;
}

template <u32 Rows>
u32 m036_hd_base_state::scan_inputs_r()
{
	u16 result = 0xffff;
	for (u32 row = 0; row < Rows; ++row)
	{
		if (!BIT(m_pio_out, 8 + row))
			result &= m_scan[row]->read() | 0xff00;
	}
	return result;
}

u32 m036_hd_state::gpio_inputs_r()
{
	u16 result = scan_inputs_r<5>();
	if (!BIT(m_pio_out, 10))
		result &= keys_r(m_secondary_select >> 5);
	return result;
}

void m036_hd_state::outputs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_secondary_select);
	if (BIT(mem_mask, 2))
		m_hopper->motor_w(BIT(m_secondary_select, 2));
}

void m036_hd_state::machine_start()
{
	m036_hd_base_state::machine_start();
	save_item(NAME(m_secondary_select));
}

void m036_hd_state::machine_reset()
{
	m036_hd_base_state::machine_reset();
	m_secondary_select = 0xffff;
	m_hopper->motor_w(0);
}

void m036_mj_state::outputs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_secondary_select);
	m_key_select = BIT(m_secondary_select, 9, 5);
	if (BIT(mem_mask, 2))
		m_hopper->motor_w(BIT(m_secondary_select, 2));
}

void m036_mj_state::machine_start()
{
	m036_tt_state::machine_start();
	save_item(NAME(m_secondary_select));
}

void m036_mj_state::machine_reset()
{
	m036_tt_state::machine_reset();
	m_secondary_select = 0xffff;
}

void m036_hyleyuan_state::machine_reset()
{
	m036_hd_base_state::machine_reset();
	m_ticket->motor_w(0);
}



void m036_base_state::screen_vblank(int state)
{
	// The internal ROM acknowledges this latched source through the AIC.
	if (state)
	{
		m_maincpu->set_aic_irq(2, ASSERT_LINE);
	}
}


void m036_state::m036_map(address_map &map)
{
	map(0x08000000, 0x081fffff).rom().region("user1", 0); // not 100% sure it maps here.
}

void m036_base_state::m036_boot_map(address_map &map)
{

	// Video RAM ranges cleared by the internal ROM. Only basic sprites are drawn.
	// Unlike PGM2, this board uses 0x03xxxxxx, not 0x30xxxxxx, for video/I/O.
	map(0x0300'0000, 0x0300'3fff).ram().share(m_spriteram);
	map(0x0310'0000, 0x0310'7fff).ram().share(m_paletteram); // xRGB555 in low 16 bits of each word
	map(0x0320'0000, 0x0320'0bff).ram();
	map(0x0340'0000, 0x0340'0003).r(FUNC(m036_base_state::inputs_r));
	map(0x0330'0000, 0x0330'0003).nopw();
	map(0x0334'0000, 0x0334'0003).nopw();
	map(0x0338'0000, 0x0338'004b).nopw();
	map(0x0350'0000, 0x0350'0003).nopw();
	map(0x0352'0000, 0x0352'0003).lr32(NAME([] () -> u32 { return 1; })); // TODO: video ready/busy timing
	map(0x0354'0000, 0x0354'0003).nopw();
	map(0x0356'0000, 0x0356'0003).nopw();

	map(0x1000'0000, 0x101f'ffff).rom().region("user1", 0);
	// External SRAM also holds settings, credits and bookkeeping. The startup
	// code reinitializes its working areas but preserves valid accounting data.
	map(0x2000'0000, 0x2001'ffff).ram().share("mainram"); // stacks at the top

	map(0x3000'0000, 0x3000'0003).umask32(0xffff'0000).w(FUNC(m036_base_state::key_select_w));
	map(0x3000'0004, 0x3000'0007).lr32(NAME([] () -> u32 { return 0xffff'ffff; }));
	map(0x3000'0008, 0x3000'000f).r(FUNC(m036_base_state::dips_r));

	map(0x5000'0004, 0x5000'0007).nopw(); // overridden for sydh's output at byte address 0x50000006
}


void m036_tt_state::m036_tt_boot_map(address_map &map)
{
	m036_boot_map(map);
	map(0x4000'0000, 0x4000'0003).umask32(0x0000'00ff).rw(m_tt5665, FUNC(tt5665_device::csl_r), FUNC(tt5665_device::csl_w));
	map(0x5000'0000, 0x5000'0003).umask32(0x0000'00ff).rw(m_tt5665, FUNC(tt5665_device::csr_r), FUNC(tt5665_device::csr_w));
}


void m036_oki_state::m036_oki_boot_map(address_map &map)
{
	m036_boot_map(map);
	map(0x4000'0000, 0x4000'0003).umask32(0x0000'00ff).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x3000'0000, 0x3000'0003).umask32(0x0000'00ff).w(FUNC(m036_oki_state::oki_banks_w));
	// Later OKI boards move the two DIP banks here.
	map(0x5000'0000, 0x5000'0003).portr(m_dsw[0]);
	map(0x6000'0000, 0x6000'0003).portr(m_dsw[1]);
}


void m036_tt_state::sydh_map(address_map &map)
{
	m036_boot_map(map);
	map(0x4000'0004, 0x4000'0007).umask32(0xffff'0000).lr16(NAME([this] () -> u16 { return m_dsw[2]->read(); }));
	map(0x4000'0008, 0x4000'000b).umask32(0xffff'0000).lr16(NAME([this] () -> u16 { return m_dsw[1]->read(); }));
	map(0x4000'000c, 0x4000'000f).portr(m_dsw[0]);
	map(0x5000'0004, 0x5000'0007).umask32(0x00ff'0000).lw8(NAME([this] (u8 data) { m_hopper->motor_w(BIT(data, 2)); }));
	map(0x5000'0008, 0x5000'000f).nopw(); // TODO: lamps/counters
	map(0x6000'000c, 0x6000'000f).umask32(0x0000'00ff).rw(m_tt5665, FUNC(tt5665_device::csl_r), FUNC(tt5665_device::csl_w));
	map(0x6000'0008, 0x6000'000b).umask32(0x00ff'0000).rw(m_tt5665, FUNC(tt5665_device::csr_r), FUNC(tt5665_device::csr_w));
}


void m036_hd_state::hd_map(address_map &map)
{
	m036_boot_map(map);
	map(0x0340'0000, 0x0340'0003).r(FUNC(m036_hd_state::dips_packed_r));

	map(0x4000'0000, 0x4000'0003).umask32(0x0000'00ff).rw(m_tt5665, FUNC(tt5665_device::csl_r), FUNC(tt5665_device::csl_w));
	map(0x4020'0000, 0x4020'0003).umask32(0x0000'00ff).rw(m_tt5665, FUNC(tt5665_device::csr_r), FUNC(tt5665_device::csr_w));

	map(0x5000'0000, 0x5000'0007).r(FUNC(m036_hd_state::dips_r));
	map(0x5000'0000, 0x5000'0003).umask32(0x0000'ffff).w(FUNC(m036_hd_state::outputs_w));
	map(0x5020'0000, 0x5020'0003).umask32(0x0000'ffff).w(FUNC(m036_hd_state::key_select_w));
}


void m036_hyleyuan_state::hyleyuan_map(address_map &map)
{
	m036_boot_map(map);
	map(0x3000'0000, 0x3000'0003).umask32(0x0000'00ff).rw(m_tt5665, FUNC(tt5665_device::csl_r), FUNC(tt5665_device::csl_w));
	map(0x4000'0000, 0x4000'0003).umask32(0x0000'00ff).rw(m_tt5665, FUNC(tt5665_device::csr_r), FUNC(tt5665_device::csr_w));
	map(0x5000'0000, 0x5000'0003).lr32(NAME([] () -> u32 { return 0xffff'ffff; })).nopw();
	map(0x6000'0000, 0x6000'000f).lr32(NAME([] () -> u32 { return 0xffff'ffff; })).nopw(); // TODO: remaining daughterboard I/O
	// Bit 1 drives the ticket motor; the return sensor is on scan row 3, bit 5.
	// Without sensor pulses the game stops the motor after five seconds and
	// displays the ticket-dispenser fault with the unpaid count.
	map(0x6000'0000, 0x6000'0003).umask32(0x0000'00ff).lw8(NAME([this] (u8 data) { m_ticket->motor_w(BIT(data, 1)); }));
}

void m036_mj_state::mjzhizun_map(address_map &map)
{
	m036_boot_map(map);

	map(0x0340'0000, 0x0340'0003).lr32(NAME([this] () -> u32 {
		return (dips_packed_r() & 0x00ff'ffff) | (u32(m_direct->read() & keys_r(m_key_select)) << 24);
	}));

	map(0x3000'0000, 0x3000'0003).portr(m_service);

	map(0x3000'0000, 0x3000'0003).umask32(0x0000'ffff).w(FUNC(m036_mj_state::outputs_w));
	map(0x3001'0000, 0x3001'0003).lr32(NAME([] () -> u32 { return 0xffff'ffff; })).nopw();

	map(0x4000'0000, 0x4000'0003).umask32(0x0000'00ff).rw(m_tt5665, FUNC(tt5665_device::csl_r), FUNC(tt5665_device::csl_w));
	map(0x4001'0000, 0x4001'0003).umask32(0x0000'00ff).rw(m_tt5665, FUNC(tt5665_device::csr_r), FUNC(tt5665_device::csr_w));

	map(0x5000'0000, 0x5000'0003).nopw(); // TODO: sound sample banking?
}


void m036_tt_state::am001_map(address_map &map)
{
	m036_tt_boot_map(map);
	// S127CN reads the DIP banks here when the board ID input (bit 31 at 0x03400000)
	// is high, and from 0x30000008 otherwise. The same input selects AM001 or TT5665 sound.
	map(0x3000'0000, 0x3000'0007).r(FUNC(m036_tt_state::dips_r));
}


void m036_tt_state::extra_sram_map(address_map &map)
{
	m036_tt_boot_map(map);
	// U19: the program tests all 32 KiB and keeps bookkeeping data here.
	map(0x6000'0000, 0x6000'7fff).ram().share("extra_nvram");
}


static INPUT_PORTS_START( m036 )
INPUT_PORTS_END

static INPUT_PORTS_START( hopper )
	PORT_INCLUDE(igs_mahjong_matrix)

	// Service assignments verified against slqzsp's input test and ROM table.
	PORT_START("SERVICE")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_SERVICE_NO_TOGGLE(0x0002, IP_ACTIVE_LOW)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(5)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_START1) PORT_NAME("Start (Joystick)")
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("S1")
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("S2")
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("S3")
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(hopper_state::hopper_r))

	PORT_START("DIRECT")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DSW1")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("DSW2")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("DSW3")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START( jxry )
	PORT_INCLUDE(hopper)
	// Default JAMMA control mode, verified in the game's logical input table.
	PORT_MODIFY("SERVICE")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(hopper_state::hopper_r))
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_LOW)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(5)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_POKER_HOLD4)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_POKER_HOLD3)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_POKER_HOLD5)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_BET)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK)
	PORT_MODIFY("DIRECT")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_START1)
	PORT_SERVICE_NO_TOGGLE(0x0002, IP_ACTIVE_LOW)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD1)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD2)
	PORT_BIT(0xfff0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( m036_hd )
	PORT_INCLUDE(hopper)
	PORT_MODIFY("SERVICE")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("SCAN0")
	PORT_SERVICE_NO_TOGGLE(0x01, IP_ACTIVE_LOW)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_START("SCAN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(m036_hd_state::hopper_r))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_START("SCAN2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_START("SCAN3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_START("SCAN4")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

static INPUT_PORTS_START( hyleyuan )
	PORT_INCLUDE(m036_hd)
	// The application uses the extended key IDs 0x4b-0x4e. The earlier
	// mahjong-style key table is shared library code, not this game's panel.
	PORT_MODIFY("KEY0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_MODIFY("KEY1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_MODIFY("KEY2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_MODIFY("KEY3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_MODIFY("KEY4")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_MODIFY("SCAN0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_MODIFY("SCAN1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_MODIFY("SCAN2")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Left Button")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Right Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_NAME("Service Key")
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_LOW)
	PORT_MODIFY("SCAN3")
	// Coin and ticket filters sample this row separately from the key table.
	PORT_BIT(0xde, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("ticket", FUNC(ticket_dispenser_device::line_r))
INPUT_PORTS_END



// DIP functions read from the games' own test screens. Unidentified switches
// use their physical locations; link and accounting settings need further tests.
static INPUT_PORTS_START( ddz )
	PORT_INCLUDE(hopper)
	PORT_MODIFY("DSW2")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Demo_Sounds)) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "Controls") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x02, "Joystick")
	PORT_DIPSETTING(0x00, "Mahjong")
	PORT_DIPNAME(0x04, 0x04, "Continue") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "Alarm Sound") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, "Automatic Pass") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START( ddz_hd )
	PORT_INCLUDE(m036_hd)
	PORT_MODIFY("DSW2")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Demo_Sounds)) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "Controls") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x02, "Joystick")
	PORT_DIPSETTING(0x00, "Mahjong")
	PORT_DIPNAME(0x04, 0x04, "Continue") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "Alarm Sound") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, "Automatic Pass") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START( cjgdy )
	PORT_INCLUDE(ddz)
	PORT_MODIFY("DSW2")
	PORT_DIPNAME(0x10, 0x10, "Automatic Pass") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START( cjbq )
	PORT_INCLUDE(hopper)
	PORT_MODIFY("DSW1")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Demo_Sounds)) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "Continue") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "Controls") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x04, "Joystick")
	PORT_DIPSETTING(0x00, "Mahjong")
INPUT_PORTS_END

static INPUT_PORTS_START( lhfy )
	PORT_INCLUDE(m036_hd)
	PORT_MODIFY("DSW1")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Demo_Sounds)) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "Controls") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x02, "Joystick")
	PORT_DIPSETTING(0x00, "Mahjong")
	PORT_DIPNAME(0x04, 0x04, "Continue") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START( jhzb )
	PORT_INCLUDE(hopper)
	PORT_MODIFY("DSW1")
	PORT_DIPNAME(0x01, 0x01, "Controls") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x01, "Joystick")
	PORT_DIPSETTING(0x00, "Mahjong")
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Demo_Sounds)) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START( lhtb )
	PORT_INCLUDE(hopper)
	PORT_MODIFY("DSW2")
	PORT_DIPNAME(0x01, 0x01, "Controls") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x01, "Joystick")
	PORT_DIPSETTING(0x00, "Mahjong")
INPUT_PORTS_END

static INPUT_PORTS_START( sydh )
	PORT_INCLUDE(hopper)
	PORT_MODIFY("DSW2")
	PORT_DIPNAME(0x01, 0x01, "Controls") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x01, "Joystick")
	PORT_DIPSETTING(0x00, "Mahjong")
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Demo_Sounds)) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START( qhzb )
	PORT_INCLUDE(hopper)
	PORT_MODIFY("DSW1")
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Demo_Sounds)) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "Controls") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(0x80, "Mahjong")
	PORT_DIPSETTING(0x00, "Joystick")
	PORT_MODIFY("DSW2")
	PORT_DIPNAME(0x04, 0x04, "Continue") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
INPUT_PORTS_END



void m036_state::m036(machine_config &config)
{
	IGS036(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &m036_state::m036_map);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(m036_state::screen_update_m036));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x200);

	SPEAKER(config, "speaker").front_center();
	OKIM6295(config, "oki", 1'000'000, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "speaker", 1.0); // clock and pin 7 not verified
}


void m036_state::m036_tt(machine_config &config)
{
	IGS036(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &m036_state::m036_map);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(m036_state::screen_update_m036));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x200);

	SPEAKER(config, "speaker").front_center();
	TT5665(config, "tt5665", 4.952_MHz_XTAL, tt5665_device::ss_state::SS_HIGH, 0).add_route(1, "speaker", 1.0);
}


void m036_base_state::base_config(machine_config &config)
{
	IGS036(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &m036_base_state::m036_boot_map);
	m_maincpu->set_gpio_r().set_ioport(m_service);
	m_maincpu->set_gpio_w().set(FUNC(m036_base_state::gpio_outputs_w));
	m_maincpu->set_do_decrypt().set(FUNC(m036_base_state::decrypt));

	SCREEN(config, m_screen);
	// FIXME: pixel clock and blanking intervals are not known.
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 511, 0, 239);
	m_screen->set_screen_update(FUNC(m036_base_state::screen_update<false>));
	m_screen->screen_vblank().set(FUNC(m036_base_state::screen_vblank));

	NVRAM(config, "mainram", nvram_device::DEFAULT_ALL_0);
	SPEAKER(config, "speaker").front_center();
}

void hopper_state::hopper_config(machine_config &config)
{
	base_config(config);
	HOPPER(config, m_hopper, attotime::from_msec(100)); // mechanical period is provisional
}

void m036_oki_state::m036_boot(machine_config &config)
{
	hopper_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m036_oki_state::m036_oki_boot_map);
	m_maincpu->set_gpio_w().set(FUNC(m036_oki_state::gpio_hopper_w));
	OKIM6295(config, m_oki, 1'000'000, okim6295_device::PIN7_LOW); // clock and pin 7 not verified
	m_oki->set_addrmap(0, &m036_oki_state::oki_rom_map);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void m036_oki_state::m036_oki_2mhz(machine_config &config)
{
	m036_boot(config);
	// FIXME: clock and pin 7 require measurement; 2 MHz gives the expected sample pitch.
	m_oki->set_clock(2'000'000);
}

void m036_tt_state::tt_config(machine_config &config)
{
	hopper_config(config);
	TT5665(config, m_tt5665, 4.952_MHz_XTAL, tt5665_device::ss_state::SS_HIGH, 0);
	m_tt5665->add_route(ALL_OUTPUTS, "speaker", 0.5);
}

void m036_tt_state::m036_tt_boot(machine_config &config)
{
	tt_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m036_tt_state::m036_tt_boot_map);
	m_maincpu->set_gpio_w().set(FUNC(m036_tt_state::gpio_hopper_w));
}

void m036_tt_state::m036_sydh(machine_config &config)
{
	tt_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m036_tt_state::sydh_map);
	m_maincpu->set_gpio_w().set(FUNC(m036_tt_state::gpio_hopper_w));
}

void m036_tt_state::m036_sram(machine_config &config)
{
	m036_tt_boot(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m036_tt_state::extra_sram_map);
	NVRAM(config, "extra_nvram", nvram_device::DEFAULT_ALL_0);
}

void m036_tt_state::m036_am001(machine_config &config)
{
	m036_tt_boot(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m036_tt_state::am001_map);
}

void m036_tt_state::m036_cjgdy(machine_config &config)
{
	m036_sram(config);
	// FIXME: oscillator and S1/S0/SS wiring require verification; DAOL uses the /2 divider.
	m_tt5665->config_s0_s1(1);
}

void m036_mj_state::m036_mjzhizun(machine_config &config)
{
	tt_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m036_mj_state::mjzhizun_map);
}

void m036_hd_base_state::hd_config(machine_config &config)
{
	base_config(config);
	m_maincpu->set_clock(100'000'000); // provisional PLL clock, as in PGM2
	m_screen->set_size(800, 600);
	m_screen->set_visarea(0, 799, 0, 599);
	m_screen->set_screen_update(FUNC(m036_hd_base_state::screen_update<true>));
	TT5665(config, m_tt5665, 4.952_MHz_XTAL, tt5665_device::ss_state::SS_HIGH, 0);
	m_tt5665->add_route(ALL_OUTPUTS, "speaker", 0.5);
}

void m036_hd_state::m036_hd(machine_config &config)
{
	hd_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m036_hd_state::hd_map);
	m_maincpu->set_gpio_r().set(FUNC(m036_hd_state::gpio_inputs_r));

	HOPPER(config, m_hopper, attotime::from_msec(100)); // mechanical period is provisional
}

void m036_hyleyuan_state::m036_hyleyuan(machine_config &config)
{
	hd_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m036_hyleyuan_state::hyleyuan_map);
	m_maincpu->set_gpio_r().set(FUNC(m036_hyleyuan_state::scan_inputs_r<4>));

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(100)); // mechanical period is provisional
}


ROM_START( cjdh2 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s311cn.u33",  0x000000, 0x200000, CRC(a6fb72f0) SHA1(1d9583eafaea21d5ec078b7f2e3dc426571a9550) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END

ROM_START( cjdh2a )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s311cna.u33", 0x000000, 0x200000, CRC(0bc6bc1b) SHA1(c891a7051cda1fd250d9380d7f33b47c375db74d) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END

ROM_START( cjdh2b )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s311cnb.u33", 0x000000, 0x200000, CRC(ddcf50bd) SHA1(39a3ed728be5894a2fec5cf0858f6f40be5ccae1) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END

ROM_START( cjdh2c )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s215cn.u33",  0x000000, 0x200000, CRC(ebe35131) SHA1(1f167e70a80b39e0658fd97c249982a0aa622683) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END

ROM_START( cjdh2d ) // PCB-0674-03-IK. IGS036 + M6295 compatible + Altera EPM3032ALC44-10N + 2 banks of 8 DIP switches
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "d7_igs036.u18", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s311cn.u33", 0x000000, 0x200000, CRC(a6fb72f0) SHA1(1d9583eafaea21d5ec078b7f2e3dc426571a9550) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "ik_sp_u20.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROM_LOAD( "ik-cg_u5.u5",   0x0000000, 0x0800000, CRC(701b4197) SHA1(e84d9d79db5eb8641b196f95b260079abaf12572) )
	ROM_LOAD( "ik-cg_u9.u9",   0x0800000, 0x0200000, CRC(ca402fa0) SHA1(058fc76dd712535e7c2f7851604b4c561cc5494b) )
	ROM_LOAD( "ik-cg_u24.u24", 0x0a00000, 0x0800000, CRC(6699d712) SHA1(812a4aa43066a78c1691317005099ffc4526696b) )
	ROM_LOAD( "ik-cg_u28.u28", 0x1200000, 0x0200000, CRC(53226179) SHA1(59cdb1762416c593dd00aa6ab9c3bbdbd80479f4) )
ROM_END


ROM_START( cjddzsp ) // IGS PCB-0825-02-IU-2. Uses an unemulated DigiArt AM001 ADPCM & MP3 sound chip
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "cjddzsp_igs036.bin", 0x00000, 0x4000, CRC(1146d5f4) SHA1(910662ac3c1d3f3f94abdf0f654a2fb5d96b7b97) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjddzsp_s128cn.u18",  0x000000, 0x200000, CRC(0eff886e) SHA1(0cfed5cd0ea5b4c9b497d31d531d8311d804a15d) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "cjddzsp_s122cn.u27", 0x000000, 0x800000, CRC(bfd4b347) SHA1(3618793f45ae473812b421363eb30a78886c39c5) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "cjddzsp_s122cn.u28",  0x000000, 0x800000, CRC(c1dd1809) SHA1(ee542a661ae486f766f2353430dd1116dcb3c321) )
	ROM_LOAD( "cjddzsp_s127cn.u30",  0x800000, 0x800000, CRC(cb82eaeb) SHA1(938258a8e96c0266d368dbc7c8cf6af110b1a3f1) )
ROM_END

ROM_START( cjddzsp127cn ) // IGS PCB-0825-02-IU-2. Uses an unemulated DigiArt AM001 ADPCM & MP3 sound chip
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "cjddzsp_igs036.bin", 0x00000, 0x4000, CRC(1146d5f4) SHA1(910662ac3c1d3f3f94abdf0f654a2fb5d96b7b97) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjddzsp_s127cn.u18",  0x000000, 0x200000, CRC(9840aa30) SHA1(98131721981ad1e7bfa1d0da56c3c406e9de26c2) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "cjddzsp_s122cn.u27", 0x000000, 0x800000, CRC(bfd4b347) SHA1(3618793f45ae473812b421363eb30a78886c39c5) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "cjddzsp_s122cn.u28",  0x000000, 0x800000, CRC(c1dd1809) SHA1(ee542a661ae486f766f2353430dd1116dcb3c321) )
	ROM_LOAD( "cjddzsp_s127cn.u30",  0x800000, 0x800000, CRC(cb82eaeb) SHA1(938258a8e96c0266d368dbc7c8cf6af110b1a3f1) )
ROM_END

ROM_START( cjddzsp126cn )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "cjddzsp_igs036.bin", 0x00000, 0x4000, CRC(1146d5f4) SHA1(910662ac3c1d3f3f94abdf0f654a2fb5d96b7b97) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjddzsp_s126cn.u18",  0x000000, 0x200000, CRC(96daefd7) SHA1(f1337901fd0a3e4cc2a92458f16535416f84f0c9) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "cjddzsp_s122cn.u27", 0x000000, 0x800000, CRC(bfd4b347) SHA1(3618793f45ae473812b421363eb30a78886c39c5) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "cjddzsp_s122cn.u28",  0x000000, 0x800000, CRC(c1dd1809) SHA1(ee542a661ae486f766f2353430dd1116dcb3c321) )
	ROM_LOAD( "cjddzsp_s127cn.u30",  0x800000, 0x800000, CRC(cb82eaeb) SHA1(938258a8e96c0266d368dbc7c8cf6af110b1a3f1) )
ROM_END

ROM_START( cjddzsp122cn )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "cjddzsp_igs036.bin", 0x00000, 0x4000, CRC(1146d5f4) SHA1(910662ac3c1d3f3f94abdf0f654a2fb5d96b7b97) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjddzsp_s122cn.u18",  0x000000, 0x200000, CRC(4a42aad6) SHA1(96805e5bfbd50686177fe50020229ea8787ade17) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "cjddzsp_s122cn.u27", 0x000000, 0x800000, CRC(66e7e482) SHA1(452eae795238fb41dbd9521405b11ff09dd97205) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "cjddzsp_s122cn.u28",  0x000000, 0x800000, CRC(c1dd1809) SHA1(ee542a661ae486f766f2353430dd1116dcb3c321) )
	ROM_LOAD( "cjddzsp_s122cn.u30",  0x800000, 0x800000, CRC(cb82eaeb) SHA1(938258a8e96c0266d368dbc7c8cf6af110b1a3f1) )
ROM_END


/*******************************************************************
Long Hu Zhengba Tebie Ban, IGS, 2009
Que Huang Zhengba, IGS, 2007
(Year not shown on title screens but date shown on error screen when program ROM removed)
Hardware Info by Guru
---------------------

PCB-0701-01-IU   (Que Huang Zhengba)
PCB-0799-02-IU-1 (Long Hu Zhengba Tebie Ban)
|----|  |-------------------------------|  |------|
|    |--|             JAMMA             |--|      |
|                              ULN2004    TDA1519 |
|-| 7407                       ULN2004        VOL |
  |     LV245 LV245                  4.952MHz     |
|-|      LV245 LV245  EPM3032   |------|    UPC844|
|       20MHz                   |TT5665|       U43|
|1        |------|              |------| QS3257   |
|8        |IGS036|                       QS3257   |
|W        |      |                                |
|A        |------|                 U27        CON3|
|Y  7404     32.768kHz                            |
|   7404                           U28            |
|-| HC132     U18                                 |
  | SW4    16LV1024                U29            |
  |                     LV245                  JP8|
|-| BATT                           U30            |
|           1117-18       SW1                     |
|10   TYN408G             SW2      U31            |
|WAY        1084-33       SW3                     |
|                                              JP7|
|-------------------------------------------------|
Notes:
      IGS036 - Custom SOC in BGA package with internal ROM. SOC is possibly ARM-based. Clock input 20.000MHz
      TT5665 - Tontek Design Technology TT5665 8-Channel (or 4-Channel L/R Stereo) ADPCM Voice Synthesis LSI
               Clock input 4.952MHz, pin 25 (SS) high, S0 and S1 low. Game sound is very low quality.
     EPM3032 - Altera EPM3032 CPLD in PLCC44 package
               Sticker on Que Huang Zhengba: "IU U23"
               Sticker on Long Hu Zhengba Tebie Ban: "IU-1 U23"
     TDA1519 - NXP TDA1519 6W Stereo Power Amplifier IC
      UPC844 - NEC uPC844 Quad Operational Amplifier
      QS3257 - IDT QS3257 Mux/Demux (=74LS257)
     ULN2004 - Texas Instruments ULN2004 Darlington Array
       LV245 - 74LV245A Octal Bus Transceiver
        7407 - 7407D Hex Buffer
        7404 - 7404D Hex Inverter
       HC132 - 74HC132D Quadruple 2-Input Positive-NAND Schmitt Trigger
    16LV1024 - Chiplus CS16LV10243GCR70 64kBx16-bit SRAM
         U43 - PS/2 Port (not populated on PCB-0799)
        CON3 - DB9 Port (not populated on PCB-0799)
         JP7 - 4-pin power connector
         JP8 - 10-pin connector (not populated, might be for JTAG programming)
        BATT - 3V Coin Battery
       SW1-3 - 8-position DIP Switch
         SW4 - SPDT Switch (clears high scores/NVRAM)
     1117-18 - 1.8V 800mA LDO Regulator
     1084-33 - 3.3V 5A LDO Regulator
     TYN408G - ST TYN408G SCR
         U18 - EV29LV160AB 2Mx8-bit/1Mx16-bit DIP42 3.3V Flash ROM. ROM data is 16-bit.
               The ROM is not pin compatible with 27C160 and must be dumped using a custom adapter.
               This ROM is the game-specific program code. There is common code inside the IGS036 SOC because
               if the program ROM is removed and booted the screen shows 'PROGRAM ROM ERROR'
                - Que Huang Zhengba: V100CN_U18.U18
                - Long Hu Zhengba Tebie Ban: S101CN_U18.U18
     U27-U31 - EV29LV640MT 8Mx8-bit/4Mx16-bit DIP48 Flash ROM (U29 & U31 not populated). ROM data is 16-bit.
               U27 - Audio Samples for TT5665
               Other ROMs - Graphics

*******************************************************************/

ROM_START( lhtb ) // PCB-0799-02-IU-1, every ROM label starts with 龍虎特別版
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "cn1012_igs036.bin", 0x00000, 0x4000, CRC(b321b813) SHA1(55a5e6898920acf2a7a4afb581a974c77ecaba2f) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "s-101cn.u18",  0x000000, 0x200000, CRC(1020f4b5) SHA1(953bb776a804738c624a1dca336e42beb10238f7) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "s101cn_u27.u27", 0x000000, 0x800000, CRC(8db377ae) SHA1(769c146104d8577b337d5aa8b388b9c8b726dd21) )

	ROM_REGION( 0x800000*2, "gfx", 0 )
	ROM_LOAD( "s101cn_u28.u28",  0x000000, 0x800000, CRC(75029040) SHA1(d1d0fd696a6c2819034b089e5d2e97b6d5ccc5f9) )
	// u29 not populated
	ROM_LOAD( "s101cn_u30.u30",  0x800000, 0x800000, CRC(4024bd96) SHA1(c3f1c616c9783560a5ea0f8727e39c20c64d46f2) )
	// u31 not populated, etched CG1-L on PCB
ROM_END

ROM_START( qhzb ) // PCB-0701-01-IU
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "qhzb_igs036.bin", 0x00000, 0x4000, CRC(b294079b) SHA1(5f1e63e302497fd8c9144fbc5c128cac215b28ec) ) // same of slqzsp, but both were dumped from the correct game

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v100cn_u18.u18", 0x000000, 0x200000, CRC(8c2733b7) SHA1(226768eead8d7754b2468384e5e691812d0ac96d) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "sp_u27.u27", 0x000000, 0x800000, CRC(624ff81a) SHA1(99f06f725b761418e88198493276982e367bd432) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x800000*2, "gfx", 0 )
	ROM_LOAD( "cg_u28.u28",  0x000000, 0x800000, CRC(04105f6f) SHA1(e917825c2304bba3362aecbfab73d2e2b340478e) ) // FIXED BITS (xxxxxxxx0000xxxx)
	// u29 not populated
	ROM_LOAD( "cg_u30.u30",  0x800000, 0x800000, CRC(37c7d442) SHA1(81682aa1ba6912919c85ee8f68c2e9821f4a3171) )
	// u31 not populated, etched CG1-L on PCB
ROM_END


ROM_START( lhzb3in1 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "lhzb3in1_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "lhzb3in1_v100cn.u17",  0x000000, 0x200000, CRC(03caaba4) SHA1(701b97d791e9329bad2ddc4d365748e65c430758) )

	ROM_REGION( 0x1000000, "tt5665", 0 ) // samples
	ROM_LOAD( "lhzb3in1_v100cn.u29", 0x000000, 0x800000, CRC(d8c160a9) SHA1(4b567571764db679a265ae075136128db495acdd) )
	ROM_LOAD( "lhzb3in1_v100cn.u28", 0x800000, 0x800000, CRC(68624630) SHA1(56e638d59c4533136f69db22f562b39120b516c1) ) // 11xxxxxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROM_LOAD( "lhzb3in1_v100cn.u30",  0x0000000, 0x800000, CRC(fb4124d7) SHA1(324fe2ade17b0ee9833decf2cab9dd4654a04cec) )
	ROM_LOAD( "lhzb3in1_v100cn.u31",  0x0800000, 0x800000, CRC(4572ff90) SHA1(5d4a40ddec1505edc8a1e35130abd7f2c97b1094) ) // FIXED BITS (xxxxxxxx0000xxxx)
	ROM_LOAD( "lhzb3in1_v100cn.u32",  0x1000000, 0x800000, CRC(04fe8ca2) SHA1(039009dd535e1388236bd0fd699eeaf593ae5323) )
	ROM_LOAD( "lhzb3in1_v100cn.u33",  0x1800000, 0x800000, CRC(9afa55d1) SHA1(0a19e1c54b271b21fb9931e7c81a9e7d9e77295a) )
ROM_END


ROM_START( igsm312 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "igsunk_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "m312cn.rom", 0x000000, 0x200000, CRC(5069c310) SHA1(d53a2e8acddfbb7afc27c68c0b3167419a3ec3e6) )

	ROM_REGION( 0x800100, "tt5665", ROMREGION_ERASE00 ) // samples
	// missing

	ROM_REGION( 0x800100*2, "gfx", ROMREGION_ERASE00 )
	// missing
ROM_END


// PCB-0999-00-KO main with PCB-0634-02-IN riser board for GFX ROMs + PCB-0998-01-KO-C for I/O
ROM_START( mghammer )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036E ARM based MCU (1119 0T7643 IGS036E)
	ROM_LOAD( "mghammer_igs036e", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v_100jp.u26", 0x000000, 0x200000, CRC(d78a4dbb) SHA1(149e68c1294b31f4b039d2cd36d36f17873c247e) )

	ROM_REGION( 0x1000000, "tt5665", ROMREGION_ERASE00 ) // samples
	ROM_LOAD( "sp_u17.u17", 0x000000, 0x800000, CRC(dd9b43b6) SHA1(5fa3191a2ebb0ea7fb737e44b1f651987c6c1bbb) )
	ROM_LOAD( "sp_u18.u18", 0x800000, 0x800000, CRC(0998e0c6) SHA1(bfa2b42248dc52f83682bbdec7bb7aaa53b4ac29) )

	ROM_REGION( 0x4000400, "gfx", 0 )
	ROM_LOAD( "cg_v_100jp.u1",  0x0000000, 0x1000100, CRC(6800418e) SHA1(81eb23f8d5c6d4eeac6bf7b06897d88d744b9681) )
	ROM_LOAD( "cg_v_100jp.u2",  0x1000100, 0x1000100, CRC(decffe69) SHA1(6161208eb399a8f418244c19e0236b4db79d0300) )
	ROM_LOAD( "cg_v_100jp.u3",  0x2000200, 0x1000100, CRC(6b6df061) SHA1(ba1e020626ce50a7fc5859f41ec717b0b39229cb) )
	ROM_LOAD( "cg_v_100jp.u4",  0x3000300, 0x1000100, CRC(5813401d) SHA1(7675a691ce0f6b99fa15a7c0004a733121772c3c) )

	ROM_REGION( 0x04000, "iocpu", 0 )
	// Internal ROM of IGS036E ARM based MCU (1119 0T7643 IGS036E), on I/O board (yes, same IGS036 code as main)
	ROM_LOAD( "io_mghammer_igs036e", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "io", 0 ) // external ARM data / prg
	ROM_LOAD( "io_v_100jp.u1", 0x000000, 0x200000, CRC(cf6c7440) SHA1(53d7b8240b153045569cac72683670df45c6ed32) )
ROM_END


ROM_START( lhfy )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "lhfy_igs036.bin", 0x00000, 0x4000, CRC(09694d46) SHA1(bf05bafd31cfe389b689da4ff3a5a14de3ba30b1) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-206cn.u11", 0x000000, 0x200000, CRC(45bd9c9f) SHA1(3a9b06bf9c66520136522d67ed12f800569580f5) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v-206cn.u19", 0x000000, 0x800000, CRC(f7990ed4) SHA1(e8a72bc0926911ba5c079b02dd324ac060e8c768) ) // same as lhzbgqb

	ROM_REGION( 0x10000000, "gfx", 0 )
	// 4x 64MB flash ROMs (U1, U2, U3, U4) mounted onto a custom SODIMM at CN1 with a sticker "CG V206CN"
	ROM_LOAD( "cg_v206cn.u1", 0x0000000, 0x4000000, CRC(6abd4a31) SHA1(6c5eefc3671c4192a689cb1e82605e65fec44384) )
	// Suspect read corruption at the starts of 0x800-byte blocks: this lane alone
	// produces periodic dots in raw sprites, and compressed image lengths disagree.
	// FIXME: independent redump required to confirm the suspected corruption.
	ROM_LOAD( "cg_v206cn.u2", 0x4000000, 0x4000000, BAD_DUMP CRC(b37e94b2) SHA1(da9e8b38070412a1b4057ddaf1df65878f3e135a) )
	ROM_LOAD( "cg_v206cn.u3", 0x8000000, 0x4000000, CRC(b379503c) SHA1(fc7d5c4afcef6a7448cc6547fa73e51378a67b3d) )
	ROM_LOAD( "cg_v206cn.u4", 0xc000000, 0x4000000, CRC(f96be9f1) SHA1(fe957af2e31007b570c4f4b707400a87be1b4301) )
ROM_END


// 欢乐斗地主 (Huānlè Dòu Dìzhǔ)
ROM_START( hlddz )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "hlddz_igs036.bin", 0x00000, 0x4000, CRC(5e24cfb4) SHA1(40d1fae77c316b7367d046d3936b615405ad8a33) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-104cn.u11", 0x000000, 0x200000, CRC(abc11224) SHA1(cf8119ac6bb75e63da716d643d89619f8f6cd017) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v-104cn.u19", 0x000000, 0x800000, CRC(4c2a0c14) SHA1(d98c852273795701a723f4daf7697811acf6e2ff) )

	ROM_REGION( 0x10000000, "gfx", 0 )
	// 4x 64MB flash ROMs (U1, U2, U3, U4) mounted onto a custom SODIMM at CN1 with a sticker "CG V-104CN"
	ROM_LOAD( "cg_v-104cn.u1", 0x0000000, 0x4000000, CRC(2b00c8b8) SHA1(6dced939047dd142e75ae6efb1f3cb3ac5188db7) )
	ROM_LOAD( "cg_v-104cn.u2", 0x4000000, 0x4000000, CRC(42bad72d) SHA1(0a821ed89201b7e14d4ccc93fe2967e60773fb3c) )
	ROM_LOAD( "cg_v-104cn.u3", 0x8000000, 0x4000000, CRC(b58b4b10) SHA1(d9df10fcb182dc2b01a3b73ac402288facf4eaea) )
	ROM_LOAD( "cg_v-104cn.u4", 0xc000000, 0x4000000, CRC(c9895b0c) SHA1(532f859d431ab84903485c7c5918529614ea4fe8) )
ROM_END


// 终极斗地主 (Zhōngjí Dòu Dìzhǔ)
ROM_START( zjddz )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "zjddz_igs036.bin", 0x00000, 0x4000, CRC(a804a1c4) SHA1(5f3b496086b82abdf84c41c693bbe614e050dd07) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-204cn.u11", 0x000000, 0x200000, CRC(c6043975) SHA1(5b7a867e0e3023b831abf35f84f3f60b9ae0e41c) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v-204cn.u19", 0x000000, 0x800000, CRC(4c2a0c14) SHA1(d98c852273795701a723f4daf7697811acf6e2ff) )

	ROM_REGION( 0x10000000, "gfx", 0 )
	// 4x 64MB flash ROMs (U1, U2, U3, U4) mounted onto a custom SODIMM at CN1 with a sticker "CG V-204CN"
	ROM_LOAD( "cg_v-204cn.u1", 0x0000000, 0x4000000, CRC(2b00c8b8) SHA1(6dced939047dd142e75ae6efb1f3cb3ac5188db7) ) // same as hlddz
	ROM_LOAD( "cg_v-204cn.u2", 0x4000000, 0x4000000, CRC(42bad72d) SHA1(0a821ed89201b7e14d4ccc93fe2967e60773fb3c) ) // "
	ROM_LOAD( "cg_v-204cn.u3", 0x8000000, 0x4000000, CRC(b58b4b10) SHA1(d9df10fcb182dc2b01a3b73ac402288facf4eaea) ) // "
	ROM_LOAD( "cg_v-204cn.u4", 0xc000000, 0x4000000, CRC(c9895b0c) SHA1(532f859d431ab84903485c7c5918529614ea4fe8) ) // "
ROM_END


ROM_START( lhzbgqb )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "lhzbgqb_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-105cn.u11", 0x000000, 0x200000, CRC(cecbb560) SHA1(262d7df19b57e57aa50cc241ddc25090411d6b74) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v105cn_u19.u19", 0x000000, 0x800000, CRC(f7990ed4) SHA1(e8a72bc0926911ba5c079b02dd324ac060e8c768) ) // same as lhfy

	ROM_REGION( 0x10000000, "gfx", ROMREGION_ERASE00 )
	// 4x 64MB flash ROMs (U1, U2, U3, U4) mounted onto a custom SODIMM at CN1 with a sticker "CG V105CN"
	ROM_LOAD( "cg_v105cn.u1", 0x0000000, 0x4000000, NO_DUMP )
	ROM_LOAD( "cg_v105cn.u2", 0x4000000, 0x4000000, NO_DUMP )
	ROM_LOAD( "cg_v105cn.u3", 0x8000000, 0x4000000, NO_DUMP )
	ROM_LOAD( "cg_v105cn.u4", 0xc000000, 0x4000000, NO_DUMP )
ROM_END


// this PCB has IGS036 MCU, R5F21256SN MCU, TT5665, ALTERA EPM3032ALC44-10N (stickered IS U15)
// ROM labels actually are written "super 70's..."
// X1 = 4.3520H7J
//
// IS U15 = ALTERA EPM3032ALC44-10N
// F9 U28 = IGS036
// U32 = R5F21256SN
// U14 = TT5665 / 0749 / A78QE31
ROM_START( super70s )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "f9_igs036.u28", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x8000, "mcu", 0 )
	ROM_LOAD( "r5f21256sn.u32", 0x0000, 0x8000, NO_DUMP )

	// this ROM is bad and can't be read correctly. Needs another PCB.
	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "super_70s_v-100us.u31", 0x000000, 0x200000, BAD_DUMP CRC(0c499853) SHA1(f939dc81653680db2a8172789d9f490c0fb06f2b) ) // EV29LV160, BADADDR   xxxxxxxxxxx-xxxxxxxxx

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "super_70s_v100us_u27.u27", 0x000000, 0x800000, CRC(626ea089) SHA1(3bd0c8db432d23d6482045eda0af234e2de6bdfa) ) // EV29LV640

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "super_70s_v100us_u25.u25", 0x000000, 0x800000, CRC(1b8f3be3) SHA1(ce0cbe7c0ac9f103f1a621c6f9878d235bceba4c) ) // EV29LV640
	ROM_LOAD( "super_70s_v100us_u26.u26", 0x800000, 0x800000, CRC(3f829def) SHA1(94708eef23ceca287c64dc06e9a2d21584372ba9) ) // EV29LV640
ROM_END


// 金花争霸 (Jīn Huā Zhēngbà)
ROM_START( jhzb ) // Has an Altera EPM CPLD
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "jhzb_igs036.bin", 0x0000, 0x4000, CRC(5436812e) SHA1(f579044b3984598013648a637611e6041cd6055e) ) // stickered K7

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v113cn.u17", 0x000000, 0x200000, CRC(1c08099b) SHA1(125a5302935e8bef8d4a4d8a6db748110bcab692) ) // 11xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "v113cn.u26", 0x000000, 0x200000, CRC(99fc5dcd) SHA1(e737299d770b0fc3606085a566ad9fbbaca15ff5) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "v113cn-cg0-h.u27", 0x000000, 0x800000, CRC(a8fb40e4) SHA1(074ab61158a0baa5811f7d33792bc71051faeb62) ) // FIXED BITS (xxxxxxxx0000xxxx)
	ROM_LOAD( "v113cn-cg0-l.u29", 0x800000, 0x800000, CRC(a5f1952c) SHA1(3ae52eb604974414b6b97ff1e0156c39aa05a0cb) )
ROM_END


// 吉祥如意 (Jíxiáng Rúyì)
ROM_START( jxry ) // Has an Altera EPM CPLD, stickered IL U14
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "jxry_igs036.bin", 0x0000, 0x4000, CRC(cb1d4974) SHA1(c1a29330f2f844d514841d1e30eeec9a28743084) ) // stickered A3

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-116cn.u20", 0x000000, 0x200000, CRC(a8cd3431) SHA1(b978b2e27fc106a02213b6aa42620fbf287a4d24) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "sp.u18", 0x000000, 0x200000, CRC(725950e1) SHA1(7226d3e9c2cb149dae598479cddeb12446944a1e) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "cg1.u22", 0x000000, 0x800000, CRC(bfbedd8c) SHA1(f4d97e9633de6dd87f11eade5752881d61a28a38) ) // FIXED BITS (xxxxxxxx0000xxxx)
	ROM_LOAD( "cg2.u24", 0x800000, 0x800000, CRC(c40627e5) SHA1(795217425533c569bad868b16e5b31ba29c587fb) )
ROM_END


// 双龙抢珠特别版 (Shuāng Lóng Qiǎng Zhū Tèbié Bǎn)
ROM_START( slqzsp ) // Has an Altera EPM CPLD, stickered IU U23
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "slqzsp_igs036.bin", 0x0000, 0x4000, CRC(b294079b) SHA1(5f1e63e302497fd8c9144fbc5c128cac215b28ec) ) // stickered F8

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v104cn.u18", 0x000000, 0x200000, CRC(7819aa1a) SHA1(c63b375916d0009e065b6239bcc752a56d2ee127) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v104cn.u27", 0x000000, 0x800000, CRC(440617cc) SHA1(87019bde5f0cf2215b6e28434a0593d0b910cbed) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "v104cn.u28", 0x000000, 0x800000, CRC(e102b951) SHA1(619cf2aa77996e1831d6ff2116d6ae8045b9066c) )
	ROM_LOAD( "v104cn.u30", 0x800000, 0x800000, CRC(eecf61c6) SHA1(59d330508482dcc0971c5255c6c70c83ba72323c) )
ROM_END


// 鲨鱼大亨 (Shāyú Dàhēng)
ROM_START( sydh ) // PCB-0802-03-JL. Has an Altera EPM CPLD
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "sydh_igs036.bin", 0x0000, 0x4000, CRC(2f581a7a) SHA1(783ddb2876e56f096288ec6a1166949299fa4bc5) ) // stickered C3

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-104cn.u20", 0x000000, 0x200000, CRC(43219633) SHA1(0f709c700c661f20a3f47c6df5a35aff9eb05bc6) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v-104cn.u31", 0x000000, 0x800000, CRC(794a4ebf) SHA1(b6ea2228fc943dc65514d113c36a1e9d69564e9a) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "v-104cn.u33", 0x800000, 0x800000, CRC(cd8a1633) SHA1(2f63167fe282fc648d9e22c6a32d593f41546ba3) )
	ROM_LOAD( "v-104cn.u34", 0x000000, 0x800000, CRC(cb33469a) SHA1(cf2c52ccc688880574452bc806b80519eeff0298) )
ROM_END


// 逍遥斗地主 (Xiāoyáo Dòu Dìzhǔ)
ROM_START( xyddz ) // Has an Altera EPM CPLD
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "xyddz_igs036.bin", 0x0000, 0x4000, CRC(865e7daf) SHA1(1fc1c43f94394a5a03b268a1113c0af03d8f9a44) ) // stickered K8

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "xyddz.u18", 0x000000, 0x200000, CRC(5c569b88) SHA1(760bb48bb4739ae6f34921b8ed499174a8960001) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "m2401.u27", 0x000000, 0x800000, CRC(cfaa177a) SHA1(b1ba246a51c6ec451bcd04392460f7a7ccb75cda) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "m2403-cg0-l.u30", 0x800000, 0x800000, CRC(3065ee91) SHA1(9bb1a0a739c244513f7785305aff13c9314a5d16) )
	ROM_LOAD( "s2402-cg0-h.u28", 0x000000, 0x800000, CRC(b0f43dd4) SHA1(5d82fdf3bd5eef1791e7204a040ede06c6028187) )
ROM_END


// 超级憋七 (Chāojí Biē Qī). Has an Altera EPM3032.
ROM_START( cjbq )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "cjbq_igs036.bin", 0x00000, 0x4000, CRC(90c88f38) SHA1(717eb05b5d1699e725810e9f5abc4b28d275920f) ) // stickered K9

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v205cn.u18",  0x000000, 0x200000, CRC(c6904007) SHA1(c5bc81e6e03ecd1dd22926cee77723a5ab7ff71a) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v205cn-speech.u27", 0x000000, 0x800000, CRC(7cf3db49) SHA1(cefbcdbc0dee686f14c126789a0c4760a1f9b970) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "v205cn-cg0-h.u28", 0x000000, 0x800000, CRC(af10d242) SHA1(926e44ebae4d60071fb72b1a4495eb353aee6ab4) )
	ROM_LOAD( "v205cn-cg0-l.u30", 0x800000, 0x800000, CRC(a451f944) SHA1(6756dbd8c6f890b01ed7767ac754a274b6ba5b13) )
ROM_END


// 超级接龙 (Chāojí Jiēlóng). PCB-0799-02-IU-1. Has an Altera EPM3032.
ROM_START( cjjl )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "igs036.u16", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "system_v100cn.u18", 0x000000, 0x200000, CRC(cca9f4c0) SHA1(4df69657804b6da4508958a7d1b40089669e4a5a) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "sp_v100cn.u27", 0x000000, 0x800000, CRC(b9b6d289) SHA1(dd3cbe47cc0201e897d34ca182f9c0ec6899c929) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "cg_v100cn.u28", 0x000000, 0x800000, CRC(af10d242) SHA1(926e44ebae4d60071fb72b1a4495eb353aee6ab4) )
	ROM_LOAD( "cg_v100cn.u30", 0x800000, 0x800000, CRC(a451f944) SHA1(6756dbd8c6f890b01ed7767ac754a274b6ba5b13) )
ROM_END


// 超级干瞪眼 (Chāojí Gān Dèngyǎn). Has an Altera EPM3032.
ROM_START( cjgdy )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "cjgdy_igs036.bin", 0x00000, 0x4000, CRC(9f11d926) SHA1(8dc7491fb0794daed630cb3427763a77fd5704ec) ) // stickered Y11

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v110cn.u18",  0x000000, 0x200000, CRC(63be1ddd) SHA1(0293bfb16f2c75f56685fe99eec2e29076ed3a61) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v110cn.u27", 0x000000, 0x800000, CRC(0888b24c) SHA1(f5caa667eb7765473920125c05be04742e7ca2c0) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "v110cn.u28", 0x000000, 0x800000, CRC(f747ed62) SHA1(aa1acf5ba757338d4b7a83d8c236f71a06c96172) )
	ROM_LOAD( "v110cn.u30", 0x800000, 0x800000, CRC(2d743a41) SHA1(6c79150303030d9607de5765f4be83c69131010f) )
ROM_END


// 西游斗牛 (Xīyóu Dòuniú). Has an Altera EPM3032.
ROM_START( xydn )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "igs036", 0x00000, 0x4000, CRC(c91f51aa) SHA1(1dd6004fab553e867a53f9f8a4fef484ffb2a8a7) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "s-110cn.u18",  0x000000, 0x200000, CRC(3212dbf0) SHA1(3fac70ca9bb537f29a36e2453f90ac4d3abbb392) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "s-110cn.u27", 0x000000, 0x800000, CRC(b729b73e) SHA1(d1262bb2bcd9e0b4161fdab046db75b468f36fc7) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "s-110cn.u28", 0x000000, 0x800000, CRC(903adb21) SHA1(8aa922383956da50fc390eff076e2e0cac26421d) )
	ROM_LOAD( "s-110cn.u30", 0x800000, 0x800000, CRC(6cf3bb3b) SHA1(8c0a6daa80b155b36b83d7102080a8da93ea58b7) )
ROM_END


// 麻将至尊 (Májiàng Zhìzūn)
ROM_START( mjzhizun ) // PCB-0918-02-JV. Has a Xilinx XC9536XL
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "mjzhizun_igs036.bin", 0x00000, 0x4000, CRC(6c84b00e) SHA1(161c98f7c76e24d0bbf09be616efc1e431d33b45) ) // stickered V21

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "s100cn.u40",  0x000000, 0x200000, CRC(4593e1de) SHA1(d50bb4de47cb6f01ba73c72749712c17f241f8ff) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "s100cn.u21", 0x000000, 0x800000, CRC(fc9b4366) SHA1(70ea7cd1d48981283934f76dd927c04d42ada2e0) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "s100cn.u32", 0x000000, 0x800000, CRC(015e2192) SHA1(2c2689b93b7f6294e142c494e858543675c2374c) )
	ROM_LOAD( "s100cn.u34", 0x800000, 0x800000, CRC(fe19ad3f) SHA1(74e5c6a724370dca6bbb4ec828f99dcd8bac7fa3) )
ROM_END


// 大话西游 (Dàhuà Xīyóu). Has an Altera EPM3032.
ROM_START( dahuaxy )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "dahuaxy_igs036.bin", 0x0000, 0x4000, CRC(55ba469a) SHA1(df83645ae59ac3678aaef250b90ca0cbc5c9e1a8) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "s201.u18", 0x000000, 0x200000, CRC(82e4d309) SHA1(23faf921b4a7bd386b0d737fb96f36b0502f113a) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "s201.u26", 0x000000, 0x200000, CRC(0e0965c7) SHA1(3c044eefd4da7f9056147901aa387a5998eba301) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "s201.u27", 0x000000, 0x800000, CRC(3da8bb03) SHA1(93ed8c836469115970a64d92acc20acc4d13704e) )
	ROM_LOAD( "s201.u29", 0x800000, 0x800000, CRC(1b127a19) SHA1(a00df092388ad9da21f06f85a04ef6bc68904063) )
ROM_END


// 火麒麟 (Huǒ Qílín)
ROM_START( huoqilin )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "huoqilin_igs036.bin", 0x0000, 0x4000, CRC(6474b1d3) SHA1(3d6d778ac0fc37bac5ef7767efd98414e5d738b9) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v116cn.u20", 0x000000, 0x200000, CRC(9139ec5e) SHA1(46d7da21bfa047649c09cc6ce569eefba665ac9d) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "v103cn.u18", 0x000000, 0x200000, CRC(edcf49d4) SHA1(ac3ad0a62a8b88417a6d6bf6f8498cd2a138b1f6) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROM_LOAD( "v103cn.u22", 0x0000000, 0x0800000, CRC(910a8c1f) SHA1(e69b9d84d62d1ea57567b8a81aac2213f3da94dc) )
	ROM_LOAD( "v103cn.u24", 0x0a00000, 0x0800000, CRC(a520af26) SHA1(7b05b739f8fd8548f85155246374aea8e1223aba) )
	ROM_LOAD( "v103cn.u28", 0x0800000, 0x0200000, CRC(cf3691b5) SHA1(fb283568d0fbb79d52ce028f24d4fd67aaae4f2a) )
	ROM_LOAD( "v103cn.u30", 0x1200000, 0x0200000, CRC(1eb6463e) SHA1(2979d7ef627ffbddd1c5236d4dcc1b622a2a9a38) )
ROM_END

// 猴椰乐园 (Hóu Yē Lèyuán) - PCB-0733-04-JC
ROM_START( hyleyuan )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "hyleyuan_igs036.bin", 0x0000, 0x4000, CRC(ea7cd4da) SHA1(cec3c04be1fdc15a7e11ca86bb98f73cbb8e6ebd) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "s_103cn.u24", 0x000000, 0x200000, CRC(248aa7e3) SHA1(d24ef32021dc03d9ae875f53767d742c91dd7d78) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "sp.u39", 0x000000, 0x800000, CRC(27ae2ffb) SHA1(1ce14d426157012a56c7d03c361b0a5d0599b2e1) )

	ROM_REGION( 0x10000000, "gfx", 0 )
	ROM_LOAD( "gfx.u1", 0x0000000, 0x4000000, CRC(a12df89d) SHA1(4e860ce9d776f01b4dc58737906bd593bf709c77) ) // 0xff filled after the first 0x1'000'000 block
	ROM_LOAD( "gfx.u2", 0x4000000, 0x4000000, CRC(2d7fdb3f) SHA1(86da3f86460a6b70c894eb94af14f416e670eae4) ) // "
	ROM_LOAD( "gfx.u3", 0x8000000, 0x4000000, CRC(1369a551) SHA1(8f0ba7a0997c31c6eb5a443223ee6a0270ac17c5) ) // "
	ROM_LOAD( "gfx.u4", 0xc000000, 0x4000000, CRC(39f2cdfe) SHA1(73ba10eebd2efb0e153033d23f572cf489ea8db1) ) // "
ROM_END

// 小丑嘉年华 (Xiǎochǒu Jiāniánhuá) - PCB-1014-00-KF-1
ROM_START( xcjnh )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "igs036", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "xcjnh_s100cn_prog.bin", 0x000000, 0x800000, CRC(81f87896) SHA1(7a4d91050bd25021e8afb60054030928a9f69274) )

	ROM_REGION( 0x800000, "oki", 0 ) // samples
	ROM_LOAD( "xcjnh_s100cn-sp.u29", 0x000000, 0x800000, CRC(957275cd) SHA1(45a6c271a24db47d0401f97d201c71302fd44545) )

	ROM_REGION( 0x8000000, "gfx", 0 )
	ROM_LOAD( "xcjnh_s100cn_cg.u1", 0x0000000, 0x2000000, CRC(0256c055) SHA1(72c8d84781679d1b1f4f5f0a1a23bd8012ac9f35) )
	ROM_LOAD( "xcjnh_s100cn_cg.u2", 0x2000000, 0x2000000, CRC(094ba5e1) SHA1(f71b0f8f6ffe90bc7a0773636cc5812245eb33a5) )
	ROM_LOAD( "xcjnh_s100cn_cg.u3", 0x4000000, 0x2000000, CRC(f479a413) SHA1(ca7436ea4ba39f1fe27cc246ebebda3c818ed368) )
	ROM_LOAD( "xcjnh_s100cn_cg.u4", 0x6000000, 0x2000000, CRC(b94e2565) SHA1(24168183c84012c3ef12e82ae076ee8ca2e4d4e6) )
ROM_END

// 欢乐敲敲敲 (Huānlè Qiāo Qiāo Qiāo)
// KO-0999-02
ROM_START( hlqqq )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "hlqqq_igs036.bin", 0x0000, 0x4000, CRC(9a752539) SHA1(38d40b344b308bc24e93b6fbd7e38f52089fc58b) )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "hlqqq_v100cn.u26", 0x000000, 0x200000, CRC(39fb6a4c) SHA1(fc391b74910c26c6b46c218dc387ca539fc4949f) )

	ROM_REGION( 0x1000000, "oki", 0 ) // samples
	ROM_LOAD( "hlqqq_sp.u17", 0x000000, 0x800000, CRC(55d24d28) SHA1(b467fcc485e6748300a59928f6689439952b70b4) )
	ROM_LOAD( "hlqqq_sp.u18", 0x000000, 0x800000, CRC(69addcf2) SHA1(216e8a14dae32a8cd8547215132b183dc0d12fe1) )

	ROM_REGION( 0x8000000, "gfx", 0 )
	ROM_LOAD( "hlqqq_cg_v100cn.u1", 0x0000000, 0x2000000, NO_DUMP )
	ROM_LOAD( "hlqqq_cg_v100cn.u2", 0x2000000, 0x2000000, NO_DUMP )
	ROM_LOAD( "hlqqq_cg_v100cn.u3", 0x4000000, 0x2000000, NO_DUMP )
	ROM_LOAD( "hlqqq_cg_v100cn.u4", 0x6000000, 0x2000000, NO_DUMP )
ROM_END

// 敲击大冒险 (Qiāojī Dà Màoxiǎn)
// PCB-0670-02-IE
ROM_START( qiaojidm )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "igs036", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "qiaojidm_v100cn.u33", 0x000000, 0x200000, CRC(7eb744cd) SHA1(25976322f84001c56e053cf7d925b8a01184b50a) )

	ROM_REGION( 0x800000, "oki", 0 ) // samples
	ROM_LOAD( "qiaojidm_sp.u31", 0x000000, 0x800000, CRC(b4f76f8f) SHA1(4cd0a94cb09ae4d040091637bdf97a2ed2e94c3d) )

	ROM_REGION( 0x8000000, "gfx", 0 )
	ROM_LOAD( "qiaojidm_v-100cn.u1", 0x0000000, 0x2000000, NO_DUMP )
	ROM_LOAD( "qiaojidm_v-100cn.u2", 0x2000000, 0x2000000, NO_DUMP )
	ROM_LOAD( "qiaojidm_v-100cn.u3", 0x4000000, 0x2000000, NO_DUMP )
	ROM_LOAD( "qiaojidm_v-100cn.u4", 0x6000000, 0x2000000, NO_DUMP )
ROM_END


void m036_state::pgm_create_dummy_internal_arm_region()
{
	uint16_t *temp16 = (uint16_t *)memregion("maincpu")->base();
	for (int i = 0; i < 0x4000 / 2; i += 2)
	{
		temp16[i] = 0xfffe;
		temp16[i + 1] = 0xeaff;

	}
	int base = 0;

	// jump straight to where we've mapped the external ROM for testing (should really set up a fake stack etc. too)


	temp16[(base) / 2] = 0x0004; base += 2;
	temp16[(base) / 2] = 0xe59f; base += 2;
	temp16[(base) / 2] = 0x0000; base += 2;
	temp16[(base) / 2] = 0xe590; base += 2;
	temp16[(base) / 2] = 0xff10; base += 2;
	temp16[(base) / 2] = 0xe12f; base += 2;
	temp16[(base) / 2] = 0x0010; base += 2;
	temp16[(base) / 2] = 0x0000; base += 2;

#if 0
	temp16[(base) / 2] = 0x03c9; base += 2;
	temp16[(base) / 2] = 0x0800; base += 2;
#else
	temp16[(base) / 2] = 0x0000; base += 2;
	temp16[(base) / 2] = 0x0800; base += 2;
#endif

}

void m036_state::init_m036()
{
	pgm_create_dummy_internal_arm_region();
}

template <const uint8_t (&Key)[0x100]>
void m036_state::init_key()
{
	init_m036();

	igs036_decryptor decrypter(Key);
	decrypter.decrypter_rom((uint16_t*)memregion("user1")->base(), memregion("user1")->bytes(), 0);
}

void m036_state::init_mghammer()
{
	init_m036();

	igs036_decryptor decrypter(mghammer_key);
	decrypter.decrypter_rom((uint16_t*)memregion("user1")->base(), memregion("user1")->bytes(), 0);

	igs036_decryptor decrypter2(mghammer_io_key);
	decrypter2.decrypter_rom((uint16_t*)memregion("io")->base(), memregion("io")->bytes(), 0);
}


void m036_base_state::init_jxry()
{
	// Same byte-address permutation as sdwx's tile ROMs on IGS017/031.
	std::vector<u8> const original(&m_gfx[0], &m_gfx[0] + m_gfx.bytes());
	for (u32 i = 0; i < m_gfx.bytes(); ++i)
		m_gfx[i] = original[(i & ~0x7ffU) | bitswap<11>(i, 7, 8, 6, 10, 9, 5, 4, 3, 2, 1, 0)];
}

} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

// sets with OKI6295 for sound, 512x240 resolution

GAME( 200?, jhzb,         0,       m036_oki_2mhz, jhzb,     m036_oki_state,      empty_init,      ROT0, "IGS",           "Jin Hua Zhengba (V113CN)", MACHINE_NOT_WORKING )

GAME( 200?, jxry,         0,       m036_oki_2mhz, jxry,     m036_oki_state,      init_jxry,       ROT0, "IGS",           "Jixiang Ruyi (V116CN)", MACHINE_NOT_WORKING )

GAME( 200?, huoqilin,     0,       m036_oki_2mhz, jxry,     m036_oki_state,      init_jxry,       ROT0, "IGS",           "Huo Qilin (V116CN)", MACHINE_NOT_WORKING )

GAME( 200?, dahuaxy,      0,       m036_boot,     hopper,   m036_oki_state,      empty_init,      ROT0, "IGS",           "Dahua Xiyou (V201CN)", MACHINE_NOT_WORKING )

// sets with a AM001 for sound, 512x240 resolution

GAME( 200?, cjddzsp,      0,       m036_am001,    ddz,      m036_tt_state,       empty_init,      ROT0, "IGS",           "Super Dou Di Zhu Special (V128CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, cjddzsp127cn, cjddzsp, m036_am001,    ddz,      m036_tt_state,       empty_init,      ROT0, "IGS",           "Super Dou Di Zhu Special (V127CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, cjddzsp126cn, cjddzsp, m036_am001,    ddz,      m036_tt_state,       empty_init,      ROT0, "IGS",           "Super Dou Di Zhu Special (V126CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// sets with a TT5665 for sound, 512x240 resolution

GAME( 200?, cjddzsp122cn, cjddzsp, m036_tt_boot,  ddz,      m036_tt_state,       empty_init,      ROT0, "IGS",           "Super Dou Di Zhu Special (V122CN)", MACHINE_NOT_WORKING )

GAME( 2007, qhzb,         0,       m036_tt_boot,  qhzb,     m036_tt_state,       empty_init,      ROT0, "IGS",           "Que Huang Zhengba (V100CN)", MACHINE_NOT_WORKING )

GAME( 2009, lhtb,         0,       m036_tt_boot,  lhtb,     m036_tt_state,       empty_init,      ROT0, "IGS",           "Long Hu Tebie Ban (S101CN)", MACHINE_NOT_WORKING ) // 龍虎特別版 - Lónghǔ tèbié bǎn

GAME( 2008, slqzsp,       0,       m036_tt_boot,  qhzb,     m036_tt_state,       empty_init,      ROT0, "IGS",           "Shuang Long Qiang Zhu Tebie Ban (V104CN)", MACHINE_NOT_WORKING )

GAME( 200?, sydh,         0,       m036_sydh,     sydh,     m036_tt_state,       empty_init,      ROT0, "IGS",           "Shayu Daheng (V104CN)", MACHINE_NOT_WORKING )

GAME( 200?, xyddz,        0,       m036_sram,     ddz,      m036_tt_state,       empty_init,      ROT0, "IGS",           "Xiaoyao Dou Dizhu (V216CN)", MACHINE_NOT_WORKING )

GAME( 200?, cjbq,         0,       m036_tt_boot,  cjbq,     m036_tt_state,       empty_init,      ROT0, "IGS",           "Chaoji Bie Qi (V205CN)", MACHINE_NOT_WORKING )

GAME( 2009, cjgdy,        0,       m036_cjgdy,    cjgdy,    m036_tt_state,       empty_init,      ROT0, "IGS",           "Chaoji Gan Dengyan (V110CN)", MACHINE_NOT_WORKING )

GAME( 200?, xydn,         0,       m036_cjgdy,    cjgdy,    m036_tt_state,       empty_init,      ROT0, "IGS",           "Xiyou Douniu (S110CN)",  MACHINE_NOT_WORKING )

GAME( 2010, mjzhizun,     0,       m036_mjzhizun, lhtb,     m036_mj_state,       empty_init,      ROT0, "IGS",           "Majiang Zhizun (S100CN)", MACHINE_NOT_WORKING )

// sets with a TT5665 for sound and 800x600 display

GAME( 2010, lhfy,         0,       m036_hd,       lhfy,     m036_hd_state,       empty_init,      ROT0, "IGS",           "Long Hu Feng Yun Gao Qing Ban (V206CN)", MACHINE_NOT_WORKING )

GAME( 2010, hlddz,        0,       m036_hd,       ddz_hd,   m036_hd_state,       empty_init,      ROT0, "IGS",           "Huanle Dou Dizhu (V104CN)", MACHINE_NOT_WORKING )

GAME( 2010, zjddz,        0,       m036_hd,       ddz_hd,   m036_hd_state,       empty_init,      ROT0, "IGS",           "Zhongji Dou Dizhu (V204CN)", MACHINE_NOT_WORKING )

GAME( 2009, hyleyuan,     0,       m036_hyleyuan, hyleyuan, m036_hyleyuan_state, empty_init,      ROT90, "IGS",          "Hou Ye Leyuan (S103CN)", MACHINE_NOT_WORKING )

// sets with missing GFX ROM dumps

GAME( 201?, hlqqq,        0,       m036_hd,       ddz_hd,   m036_hd_state,       empty_init,      ROT0, "IGS",           "Huanle Qiao Qiao Qiao (V100CN)", MACHINE_NOT_WORKING )

// sets with no IGS036 ROM dump

GAME( 200?, cjdh2,        0,       m036,    m036, m036_state, init_key<cjdh2_key>,    ROT0, "IGS",           "Chao Ji Da Heng 2 (V311CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, cjdh2a,       cjdh2,   m036,    m036, m036_state, init_key<cjdh2_key>,    ROT0, "IGS",           "Chao Ji Da Heng 2 (V311CNA)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, cjdh2b,       cjdh2,   m036,    m036, m036_state, init_key<cjdh2_key>,    ROT0, "IGS",           "Chao Ji Da Heng 2 (V311CNB)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, cjdh2c,       cjdh2,   m036,    m036, m036_state, init_key<cjdh2_key>,    ROT0, "IGS",           "Chao Ji Da Heng 2 (V215CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, cjdh2d,       cjdh2,   m036,    m036, m036_state, init_key<cjdh2_key>,    ROT0, "IGS",           "Chao Ji Da Heng 2 (V311CN, alternate GFX ROMs)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 200?, super70s,     0,       m036_tt, m036, m036_state, init_m036,          ROT0, "IGS",           "Super 70's (V100US)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 2010, lhzb3in1,     0,       m036_tt, m036, m036_state, init_key<lhzb3in1_key>, ROT0, "IGS",           "Long Hu Zhengba San He Yi (V100CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 龙虎争霸三合一

GAME( 200?, igsm312,      0,       m036_tt, m036, m036_state, init_key<m312cn_key>,   ROT0, "IGS",           "unknown 'IGS 6POKER2' game (V312CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // there's very little code and no gfx ROMs, might be a 'set/clear' chip for a gambling game.

GAME( 200?, cjjl,         0,       m036_tt, m036, m036_state, init_key<cjjl_key>,     ROT0, "IGS",           "Chaoji Jielong (V100CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 2010, lhzbgqb,      0,       m036_tt, m036, m036_state, init_key<lhzbgqb_key>,  ROT0, "IGS",           "Long Hu Zhengba Gao Qing Ban (V105CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 201?, xcjnh,        0,       m036_tt, m036, m036_state, init_key<xcjnh_key>,    ROT0, "IGS",           "Xiaochou Jianianhua (S100CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 201?, qiaojidm,     0,       m036_tt, m036, m036_state, init_m036,          ROT0, "IGS",           "Qiaoji Da Maoxian (V100CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 2015, mghammer,     0,       m036_tt, m036, m036_state, init_mghammer,          ROT0, "IGS / Enheart", "Medal Get Hammer (V100JP)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // but has S033CN strings?
