// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Brad Oliver, Fabio Priuli
/***************************************************************************

Nintendo VS UniSystem and DualSystem - (c) 1984 Nintendo of America

    Portions of this code are heavily based on
    Brad Oliver's MESS implementation of the NES.

RC2C03B/RP2C03B:
- Duck Hunt
- Mahjong
- Pinball (Japan)
- Star Luster *
- Stroke & Match Golf (Japan)
- Tennis

RP2C04-0001:
- Atari R.B.I. Baseball *
- Baseball
- Battle City *
- Freedom Force
- Gradius
- Hogan's Alley
- Mach Rider (Japan, Fighting Course)
- Pinball
- Platoon
- Super Skykid *
- Super Xevious *
- Tetris *

RP2C04-0002:
- Castlevania
- Mach Rider (Endurance Course)
- Raid on Bungeling Bay (Japan)
- Slalom
- Soccer (Japan)
- Stroke & Match Golf
- Stroke & Match Golf (Ladies)
- Wrecking Crew

RP2C04-0003:
- Balloon Fight
- Dr. Mario
- Excitebike
- Goonies
- Soccer
- T.K.O. Boxing *
  (Manual states this is an RP2C04-0004 - though the palette doesn't agree)

RP2C04-0004:
- Clu Clu Land
- Excitebike (Japan)
- Ice Climber
- Ice Climber Dual (Japan)
- Super Mario Bros.

   - Games marked with a star are compatible with multiple PPU types,
     but you need to set dipswitches corresponding to the PPU installed
     or else the game will display incorrect colors.
     Here the games are listed under the PPU type that works with the
     PPU selection dipswitches all set to OFF.

RC2C05-01:
- Ninja Jajamaru Kun (Japan)

RC2C05-02:
- Mighty Bomb Jack (Japan)

RC2C05-03:
- Gumshoe

RC2C05-04:
- Top Gun

Graphic hack games:
- Skate Kids                 (by Two-Bit Score, 1988; hack of Vs. Super Mario Bros.)

Needed roms:
- Babel no Tou               (by Namco, 1986)
- Family Boxing              (by Namco/Wood Place, 1987; Japan version of TKO Boxing)
- Family Stadium '87         (by Namco, 1987; sequel to RBI Baseball)
- Family Stadium '88         (by Namco, 1988; sequel to RBI Baseball)
- Family Tennis              (by Namco, 1987)
- Head to Head Baseball      (ever finished/released?, by Nintendo, 1986)
- Lionex                     (prototype by Sun Electronics, 1986, DRGN-D board, unreleased dump exists)
- The Wings of Madoola       (prototype by Sun Electronics, 1987, DRGN-D board, unreleased dump exists)
- Pro Yakyuu Family Stadium  (by Namco, 1986; Japan version of RBI Baseball)
- Quest of Ki                (by Namco/Game Studio, 1988)
- Super Chinese              (by Namco/Culture Brain, 1988)
- Toukaidou 53tsugi          (prototype by Sunsoft, 1985)
- Trojan                     (by Capcom, 1987)
- Urban Champion             (1984)
- Volleyball                 (1986)
- Walkure no Bouken          (by Namco, 1986)
- Wild Gunman                (1984, light gun game)

BTANB:
    - Coin sound clipped when first coin is inserted in VS. Castlevania.

TO DO:
    - Merge mapper implementations with playch10, NES, etc
    - Check others bits in coin counter
    - Check other values in bnglngby irq
    - Top Gun: cpu #0 (PC=00008016): unmapped memory byte read from 00007FFF ???

Changes:

  27/05/2004 Pierpaolo Prazzoli

  - Fixed Vs. Gumshoe (fully working now)
  - Changed coin inputs to use bit impulse

  16/10/2003 Pierpaolo Prazzoli

  - Added
        - Vs. Freedom Force
        - Vs. Super Xevious

  24/12/2002 Pierpaolo Prazzoli

  - Added
        - Vs. Mighty Bomb Jack (Japan)
        - Vs. Ninja Jajamaru Kun (Japan)
        - Vs. Raid on Bungeling Bay (Japan)
        - Vs. Top Gun
        - Vs. Mach Rider (Japan, Fighting Course Version)
        - Vs. Ice Climber (Japan)
        - Vs. Gumshoe (partially working)
        - Vs. Freedom Force (not working)
        - Vs. Stroke and Match Golf (Men set 2) (not working)
        - Vs. Baseball (Japan set 3) (not working)
  - Added coin counter
  - Added Extra Ram in vstetris
  - Added Demo Sound in vsmahjng
  - Fixed vsskykid inputs
  - Fixed protection in Vs. Super Xevious
  - Corrected or checked dip-switches in Castlevania, Duck Hunt, Excitebike,
    Gradius, Hogan's Alley, Ice Climber, R.B.I. Baseball, Slalom, Soccer,
    Super Mario Bros., Top Gun, Baseball, Tennis, Stroke and Match Golf

***************************************************************************/

#include "emu.h"

#include "bus/nes_ctrl/zapper_sensor.h"
#include "cpu/m6502/rp2a03.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/sn76496.h"
#include "video/ppu2c0x.h"

#include "screen.h"
#include "speaker.h"

#include "layout/generic.h"


namespace {

class vs_base_state : public driver_device
{
protected:
	vs_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_ppu1(*this, "ppu1")
		, m_ppu2(*this, "ppu2")
		, m_sensor(*this, "sensor")
		, m_nvram(*this, "nvram")
		, m_watchdog(*this, "watchdog")
		, m_gfx1_rom(*this, "gfx1")
		, m_prg_banks(*this, "prg%u", 0U)
		, m_prg_view(*this, "prg_view")
		, m_chr_banks(*this, "chr%u", 0U)
		, m_chr_view(*this, "chr_view")
		, m_in(*this, "IN%u", 0U)
		, m_coins(*this, "COINS%u", 1U)
		, m_dsw(*this, "DSW%u", 0U)
		, m_gunx(*this, "GUNX")
		, m_guny(*this, "GUNY")
	{
	}

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	template <u8 Side> void vsnes_coin_counter_w(offs_t offset, u8 data);
	template <u8 Side> u8 vsnes_coin_counter_r(offs_t offset);
	template <u8 Side> void vsnes_in0_w(u8 data);
	template <u8 Side> u8 vsnes_in0_r();
	template <u8 Side> u8 vsnes_in1_r();

	void vsnes_cpu1_map(address_map &map) ATTR_COLD;
	void vsnes_cpu2_map(address_map &map) ATTR_COLD;
	void vsnes_ppu1_map(address_map &map) ATTR_COLD;
	void vsnes_ppu2_map(address_map &map) ATTR_COLD;

	void init_prg_banking();
	void prg32(int bank);
	void prg16(int slot, int bank);
	void prg8(int slot, int bank);

	enum { MAIN = 0, SUB = 1 };

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<ppu2c0x_device> m_ppu1;
	optional_device<ppu2c0x_device> m_ppu2;

	optional_device<nes_zapper_sensor_device> m_sensor;
	optional_device<nvram_device> m_nvram;
	optional_device<watchdog_timer_device> m_watchdog;

	optional_memory_region m_gfx1_rom;

	memory_bank_array_creator<4> m_prg_banks;
	memory_view m_prg_view;
	u32 m_prg_chunks = 0;

	memory_bank_array_creator<8> m_chr_banks;
	memory_view m_chr_view;
	u32 m_chr_chunks = 0;

	bool m_has_gun = false;

private:
	optional_ioport_array<4> m_in;
	optional_ioport_array<2> m_coins;
	optional_ioport_array<2> m_dsw;
	optional_ioport m_gunx;
	optional_ioport m_guny;

	u8 m_coin = 0;
	u32 m_input_latch[4]{};
	u8 m_input_strobe[2]{};
};

class vs_uni_state : public vs_base_state
{
public:
	vs_uni_state(const machine_config &mconfig, device_type type, const char *tag)
		: vs_base_state(mconfig, type, tag)
	{
	}

	void vsnes(machine_config &config);
	void jajamaru(machine_config &config);
	void mightybj(machine_config &config);
	void topgun(machine_config &config);
	void vsgshoe(machine_config &config);

	void init_vskonami();
	void init_vsvram();
	void init_bnglngby();
	void init_drmario();
	void init_vs108();
	void init_vsfdf();
	void init_tkoboxng();
	void init_vsgun();
	void init_supxevs();
	void init_vsgshoe();
	void init_vsnormal();
	void init_platoon();
	void init_rbibb();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void vsnormal_vrom_banking_w(u8 data);
	void vskonami_rom_banking_w(offs_t offset, u8 data);
	void vsgshoe_gun_in0_w(u8 data);
	void drmario_rom_banking_w(offs_t offset, u8 data);
	void vsvram_rom_banking_w(u8 data);
	void vs108_rom_banking_w(offs_t offset, u8 data);
	u8 rbibb_prot_r(offs_t offset);
	u8 supxevs_prot_1_r();
	u8 supxevs_prot_2_r();
	u8 supxevs_prot_3_r();
	u8 supxevs_prot_4_r();
	u8 tkoboxng_prot_r(offs_t offset);
	void sunsoft3_rom_banking_w(offs_t offset, u8 data);
	void set_bnglngby_irq_w(u8 data);
	u8 set_bnglngby_irq_r();

	void v_set_videorom_bank(int start, int count, int vrom_start_bank);

	u8 m_mmc1_shiftreg = 0;
	u8 m_mmc1_shiftcount = 0;
	bool m_mmc1_prg16k = false;
	u8 m_mmc1_switchlow = 0;
	bool m_mmc1_chr4k = false;
	u8 m_108_reg = 0;
	u8 m_prot_index = 0;
	u8 m_ret = 0;
};

class vs_dual_state : public vs_base_state
{
public:
	vs_dual_state(const machine_config &mconfig, device_type type, const char *tag)
		: vs_base_state(mconfig, type, tag)
	{
	}

	void vsdual(machine_config &config);
	void init_vsdual();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	template <u8 Side> void vsdual_vrom_banking_w(u8 data);
};

class vs_smbbl_state : public vs_base_state
{
public:
	vs_smbbl_state(const machine_config &mconfig, device_type type, const char *tag)
		: vs_base_state(mconfig, type, tag)
		, m_sn1(*this, "sn1")
		, m_sn2(*this, "sn2")
	{
	}

	void vs_smbbl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void smbbl_6502_sn_w(offs_t offset, u8 data);
	void smbbl_z80_sn_w(offs_t offset, u8 data);
	u8 smbbl_z80_data_r();
	u8 smbbl_z80_address_r(offs_t offset);
	void smbbl_scanline_cb(int scanline, bool vblank, bool blanked);
	u8 smbbl_ppu_data_r();

	void smbbl_6502_map(address_map &map) ATTR_COLD;
	void smbbl_z80_map(address_map &map) ATTR_COLD;
	void smbbl_ppu_map(address_map &map) ATTR_COLD;

	required_device<sn76489_device> m_sn1;
	required_device<sn76489_device> m_sn2;

	u8 m_bootleg_sound_offset = 0;
	u8 m_bootleg_sound_data = 0;
	s32 m_bootleg_latched_scanline = 0;
};


//******************************************************************************


template <u8 Side>
void vs_base_state::vsnes_coin_counter_w(offs_t offset, u8 data)
{
	machine().bookkeeping().coin_counter_w(Side, data & 0x01);
	m_coin = data;

	// "bnglngby" and "cluclu", vsbball service mode
	if(data & 0xfe)
		logerror("vsnes_coin_counter_w: cpu %hhu, pc = 0x%04x - data = 0x%02x\n", Side, (Side == MAIN) ? m_maincpu->pc() : m_subcpu->pc(), data);
}

template <u8 Side>
u8 vs_base_state::vsnes_coin_counter_r(offs_t offset)
{
	// reads effectively write MSB of address (via open bus) to coin counter
	if (!machine().side_effects_disabled())
		machine().bookkeeping().coin_counter_w(Side, BIT(offset, 8));

	// only for platoon
	return m_coin;
}

template <u8 Side>
void vs_base_state::vsnes_in0_w(u8 data)
{
	// Toggling bit 0 high then low resets both controllers
	if (m_input_strobe[Side] & ~data & 1)
	{
		// load up the latches
		const u8 p1 = 2 * Side;
		const u8 p2 = p1 + 1;
		m_input_latch[p1] = m_in[p1]->read();
		m_input_latch[p2] = m_in[p2]->read();

		if (m_has_gun && m_sensor->detect_light(m_gunx->read(), m_guny->read()))
			m_input_latch[p1] |= 0x40;
	}

	m_input_strobe[Side] = data;
}

template <u8 Side>
u8 vs_base_state::vsnes_in0_r()
{
	const u8 p1 = 2 * Side;

	if (!machine().side_effects_disabled())
	{
		if (m_input_strobe[Side] & 1)
		{
			m_input_latch[p1] = m_in[p1]->read();
			if (m_has_gun && m_sensor->detect_light(m_gunx->read(), m_guny->read()))
				m_input_latch[p1] |= 0x40;
		}
	}

	u8 ret = m_input_latch[p1] & 1;
	if (!machine().side_effects_disabled())
		m_input_latch[p1] >>= 1;

// FIXME: UniSystem's wiring harness connects S Coin 2 to S Coin 1 edge hardness
// This should mean games that don't check Coin 2 inputs respond on UniSys, but on DualSys they'd miss the Coin 2 inserts as all 4 inputs are separate?
	ret |= m_coins[Side]->read();             // merge coins, etc
	ret |= (m_dsw[Side]->read() & 3) << 3;    // merge 2 dipswitches
	ret |= Side << 7;                         // CPU indicator bit

	return ret;
}

template <u8 Side>
u8 vs_base_state::vsnes_in1_r()
{
	// Only the Sub side CPU can kick the watchdog, which it must do by periodically reading $4017.
	// This is one reason UniSystem games too must be installed on the Sub side.
	if (!machine().side_effects_disabled())
	{
		if (m_watchdog && Side == SUB)
			m_watchdog->watchdog_reset();
	}

	const u8 p2 = 2 * Side + 1;

	if (!machine().side_effects_disabled())
	{
		if (m_input_strobe[Side] & 1)
			m_input_latch[p2] = m_in[p2]->read();
	}

	u8 ret = m_input_latch[p2] & 1;
	if (!machine().side_effects_disabled())
		m_input_latch[p2] >>= 1;

	ret |= m_dsw[Side]->read() & ~3;          // merge the rest of the dipswitches

	return ret;
}

/*************************************
 *
 *  Init machine
 *
 *************************************/

void vs_base_state::machine_reset()
{
	m_input_latch[0] = m_input_latch[1] = 0;
	m_input_latch[2] = m_input_latch[3] = 0;
	m_input_strobe[0] = m_input_strobe[1] = 0;
}

/*************************************
 *
 *  Machine start functions
 *
 *************************************/

void vs_base_state::init_prg_banking()
{
	u8 *base = memregion("prg")->base();
	m_prg_chunks = memregion("prg")->bytes() / 0x2000;

	for (int i = 0; i < 4; i++)
	{
		m_prg_banks[i]->configure_entries(0, m_prg_chunks, base, 0x2000);
		m_prg_banks[i]->set_entry(m_prg_chunks - 4 + i);
	}

	m_prg_view.select(0);
}

// safe banking helpers (only work when PRG size is a power of 2, i.e. not Gumshoe)

void vs_base_state::prg32(int bank)
{
	bank = (bank << 2) & (m_prg_chunks - 1);

	for (int i = 0; i < 4; i++)
		m_prg_banks[i]->set_entry(bank + i);
}

void vs_base_state::prg16(int slot, int bank)
{
	bank = (bank << 1) & (m_prg_chunks - 1);
	slot = (slot & 1) << 1;

	for (int i = 0; i < 2; i++)
		m_prg_banks[slot + i]->set_entry(bank + i);
}

void vs_base_state::prg8(int slot, int bank)
{
	m_prg_banks[slot & 0x03]->set_entry(bank & (m_prg_chunks - 1));
}

void vs_uni_state::v_set_videorom_bank(int start, int count, int vrom_start_bank)
{
	assert(start + count <= 8);

	vrom_start_bank &= m_chr_chunks - 1;
	assert(vrom_start_bank + count <= m_chr_chunks);

	// count determines the size of the area mapped
	for (int i = 0; i < count; i++)
		m_chr_banks[i + start]->set_entry(vrom_start_bank + i);
}

void vs_base_state::machine_start()
{
	save_item(NAME(m_coin));
	save_item(NAME(m_input_latch));
	save_item(NAME(m_input_strobe));
}

void vs_uni_state::machine_start()
{
	vs_base_state::machine_start();

	// establish chr banks
	// DRIVER_INIT is called first - means we can handle this different for VRAM games!
	if (m_gfx1_rom != nullptr)
	{
		u8 *base = m_gfx1_rom->base();
		m_chr_chunks = m_gfx1_rom->bytes() / 0x400;

		for (int i = 0; i < 8; i++)
		{
			m_ppu1->space(AS_PROGRAM).install_read_bank(0x0400 * i, 0x0400 * i + 0x03ff, m_chr_banks[i]);
			m_chr_banks[i]->configure_entries(0, m_chr_chunks, base, 0x400);
		}
		v_set_videorom_bank(0, 8, 0);
	}
	else
		m_chr_view.select(0);

	save_item(NAME(m_mmc1_shiftreg));
	save_item(NAME(m_mmc1_shiftcount));
	save_item(NAME(m_mmc1_prg16k));
	save_item(NAME(m_mmc1_switchlow));
	save_item(NAME(m_mmc1_chr4k));
	save_item(NAME(m_108_reg));
	save_item(NAME(m_prot_index));
	save_item(NAME(m_ret));
}

void vs_dual_state::machine_start()
{
	vs_base_state::machine_start();

	for (int i = 0; i < 2; i++)
	{
		const char *region = i ? "gfx2" : "gfx1";
		u8 *base = memregion(region)->base();
		const u32 entries = memregion(region)->bytes() / 0x2000;
		m_chr_banks[i]->configure_entries(0, entries, base, 0x2000);
		m_chr_banks[i]->set_entry(0);
	}
}

void vs_smbbl_state::machine_start()
{
	vs_base_state::machine_start();

	m_ppu1->set_scanline_callback(*this, FUNC(vs_smbbl_state::smbbl_scanline_cb));

	u8 *base = m_gfx1_rom->base();
	const u32 entries = m_gfx1_rom->bytes() / 0x2000;
	m_chr_banks[0]->configure_entries(0, entries, base, 0x2000);
	m_chr_banks[0]->set_entry(0);

	save_item(NAME(m_bootleg_sound_offset));
	save_item(NAME(m_bootleg_sound_data));
	save_item(NAME(m_bootleg_latched_scanline));
}

/**********************************************************************************
 *
 *  Game and Board-specific initialization
 *
 **********************************************************************************/

//**********************************************************************************
// Most games: VROM Banking in controller 0 write

void vs_uni_state::vsnormal_vrom_banking_w(u8 data)
{
	// switch vrom
	v_set_videorom_bank(0, 8, (data & 4) ? 8 : 0);

	// bit 1 ( data & 2 ) enables writes to extra ram, we ignore it

	// move along
	vsnes_in0_w<MAIN>(data);
}

void vs_uni_state::init_vsnormal()
{
	// vrom switching is enabled with bit 2 of $4016
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vs_uni_state::vsnormal_vrom_banking_w)));
}

//**********************************************************************************
// Gun games: VROM Banking in controller 0 write

void vs_uni_state::init_vsgun()
{
	init_vsnormal();
	m_has_gun = true;
}

//**********************************************************************************
// Konami VRC1 games: ROM banking at $8000-$ffff

void vs_uni_state::vskonami_rom_banking_w(offs_t offset, u8 data)
{
	int reg = BIT(offset, 12, 3);

	switch (reg)
	{
		case 0: // prg bank 0
		case 2: // prg bank 1
		case 4: // prg bank 2
			prg8(reg >> 1, data);
			break;

		case 6: // vrom bank 0
			v_set_videorom_bank(0, 4, data * 4);
			break;

		case 7: // vrom bank 1
			v_set_videorom_bank(4, 4, data * 4);
			break;
	}
}

void vs_uni_state::init_vskonami()
{
	// point program banks to last 32K
	init_prg_banking();

	// banking is done with writes to the $8000-$ffff area
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vs_uni_state::vskonami_rom_banking_w)));
}

//**********************************************************************************
// Vs. Gumshoe

void vs_uni_state::vsgshoe_gun_in0_w(u8 data)
{
	// Gumshoe uniquely has a bankable 16K EPROM in addition to the normal unbanked 8K slots
	m_prg_banks[0]->set_entry(BIT(data, 2));

	// otherwise do normal CHR banking and IO write
	vsnormal_vrom_banking_w(data);
}

void vs_uni_state::init_vsgshoe()
{
	// point program banks to last 32K
	init_prg_banking();

	// vrom switching is enabled with bit 2 of $4016
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vs_uni_state::vsgshoe_gun_in0_w)));

	m_has_gun = true;
}

//**********************************************************************************
// MMC1 (Dr Mario): ROM banking at $8000-$ffff

void vs_uni_state::drmario_rom_banking_w(offs_t offset, u8 data)
{
	// reset mapper
	if (data & 0x80)
	{
		m_mmc1_shiftcount = 0;
		m_mmc1_prg16k = true;
		m_mmc1_switchlow = 1;

		return;
	}

	// update shift register
	m_mmc1_shiftreg = (m_mmc1_shiftreg >> 1) | (data & 1) << 4;
	m_mmc1_shiftcount = (m_mmc1_shiftcount + 1) % 5;

	// are we done shifting?
	if (!m_mmc1_shiftcount)
	{
		// apply data to registers
		switch (BIT(offset, 13, 2))
		{
			case 0: // mirroring and options
				m_mmc1_chr4k = m_mmc1_shiftreg & 0x10;
				m_mmc1_prg16k = m_mmc1_shiftreg & 0x08;
				m_mmc1_switchlow = m_mmc1_shiftreg & 0x04;
				// 0x03: mirroring bits unused on VS
				break;

			case 1: // video rom banking - bank 0 - 4k or 8k
				if (m_mmc1_chr4k)
					v_set_videorom_bank(0, 4, m_mmc1_shiftreg * 4);
				else
					v_set_videorom_bank(0, 8, (m_mmc1_shiftreg & ~1) * 4);
				break;

			case 2: // video rom banking - bank 1 - 4k only
				if (m_mmc1_chr4k)
					v_set_videorom_bank(4, 4, m_mmc1_shiftreg * 4);
				break;

			case 3: // program banking
				if (m_mmc1_prg16k)
					prg16(!m_mmc1_switchlow, m_mmc1_shiftreg);
				else
					prg32(m_mmc1_shiftreg >> 1);
				break;
		}
	}
}

void vs_uni_state::init_drmario()
{
	// point program banks to last 32K
	init_prg_banking();

	// MMC1 mapper at $8000-$ffff
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vs_uni_state::drmario_rom_banking_w)));

	m_mmc1_shiftreg = 0;
	m_mmc1_shiftcount = 0;
}

//**********************************************************************************
// (UNROM) Games with VRAM instead of graphics ROMs: ROM banking at $8000-$ffff

void vs_uni_state::vsvram_rom_banking_w(u8 data)
{
	prg16(0, data);
}

void vs_uni_state::init_vsvram()
{
	// point program banks to last 32K
	init_prg_banking();

	// banking is done with writes to the $8000-$ffff area
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8smo_delegate(*this, FUNC(vs_uni_state::vsvram_rom_banking_w)));
}

//**********************************************************************************
// (Namco) 108 (MMC3 predecessor) games

void vs_uni_state::vs108_rom_banking_w(offs_t offset, u8 data)
{
	switch (offset & 0x6001)
	{
		case 0x0000: // $8000
			m_108_reg = data & 0x07;
			break;

		case 0x0001: // $8001
			switch (m_108_reg)
			{
				case 0: case 1:
					v_set_videorom_bank(m_108_reg * 2, 2, data);
					break;
				case 2: case 3: case 4: case 5:
					v_set_videorom_bank(m_108_reg + 2, 1, data);
					break;
				case 6: case 7:
					prg8(m_108_reg - 6, data);
					break;
			}
			break;

		default:
			logerror("vs108_rom_banking_w uncaught: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

// Common init for (Namco) 108 games

void vs_uni_state::init_vs108()
{
	// point program banks to last 32K
	init_prg_banking();

	m_108_reg = 0;

	// 108 chip at $8000-$9fff
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vs_uni_state::vs108_rom_banking_w)));
}

// Vs. RBI Baseball
// rbibb uses a protection chip labeled '127'
// Tests indicate that '127' uses a LFSR with taps: 23, 18, 0
// It is still unknown how bits are then shuffled for readback at 0x5601/0x5e01
u8 vs_uni_state::rbibb_prot_r(offs_t offset)
{
	static constexpr u8 prot_data[32] = {
		0xff, 0xfd, 0xf5, 0xf4, 0xb4, 0xb4, 0xa6, 0x2e,
		0x2f, 0x6f, 0x6f, 0x7d, 0xd5, 0xd4, 0x94, 0x94,
		0x86, 0x2e, 0x2f, 0x6f, 0x6b, 0x79, 0xd1, 0xd0,
		0x92, 0x92, 0x8d, 0x65, 0x64, 0x34, 0xb0, 0xa2
	};

	if (offset == 0)
	{
		if (!machine().side_effects_disabled())
			m_prot_index = 0;
		return 0;
	}

	const u8 prot_index = m_prot_index;
	if (!machine().side_effects_disabled())
		m_prot_index++;
	return prot_data[prot_index & 0x1f];
}

void vs_uni_state::init_rbibb()
{
	init_vs108();

	// RBI Baseball protection, address fully decoded except A11
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5600, 0x5601, read8sm_delegate(*this, FUNC(vs_uni_state::rbibb_prot_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5e00, 0x5e01, read8sm_delegate(*this, FUNC(vs_uni_state::rbibb_prot_r)));
}

// Vs. Super Xevious

u8 vs_uni_state::supxevs_prot_1_r()
{
	if (!machine().side_effects_disabled())
		m_prot_index ^= 1;
	return 0x05;
}

u8 vs_uni_state::supxevs_prot_2_r()
{
	return m_prot_index ? 0x01 : 0x00;
}

u8 vs_uni_state::supxevs_prot_3_r()
{
	return m_prot_index ? 0x89 : 0xd1;
}

u8 vs_uni_state::supxevs_prot_4_r()
{
	return m_prot_index ? 0x37 : 0x3e;
}

void vs_uni_state::init_supxevs()
{
	init_vs108();

	// Vs. Super Xevious Protection
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x54ff, 0x54ff, read8smo_delegate(*this, FUNC(vs_uni_state::supxevs_prot_1_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5678, 0x5678, read8smo_delegate(*this, FUNC(vs_uni_state::supxevs_prot_2_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x578f, 0x578f, read8smo_delegate(*this, FUNC(vs_uni_state::supxevs_prot_3_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5567, 0x5567, read8smo_delegate(*this, FUNC(vs_uni_state::supxevs_prot_4_r)));

	m_prot_index = 0;
}

// Vs. TKO Boxing
// tkoboxng uses a protection chip labeled '128'
u8 vs_uni_state::tkoboxng_prot_r(offs_t offset)
{
	static constexpr u8 prot_data[32] = {
		0xff, 0xbf, 0xb7, 0x97, 0x97, 0x17, 0x57, 0x4f,
		0x6f, 0x6b, 0xeb, 0xa9, 0xb1, 0x90, 0x94, 0x14,
		0x56, 0x4e, 0x6f, 0x6b, 0xeb, 0xa9, 0xb1, 0x90,
		0xd4, 0x5c, 0x3e, 0x26, 0x87, 0x83, 0x13, 0x00
	};

	if (offset == 0)
	{
		if (!machine().side_effects_disabled())
			m_prot_index = 0;
		return 0;
	}

	const u8 prot_index = m_prot_index;
	if (!machine().side_effects_disabled())
		m_prot_index++;
	return prot_data[prot_index & 0x1f];
}

void vs_uni_state::init_tkoboxng()
{
	init_vs108();

	// security device at $5e00-$5e01
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5e00, 0x5e01, read8sm_delegate(*this, FUNC(vs_uni_state::tkoboxng_prot_r)));
}

// Vs. Freedom Force

void vs_uni_state::init_vsfdf()
{
	init_vs108();
	m_has_gun = true;
}

//**********************************************************************************
// Sunsoft-3 (Platoon) rom banking

void vs_uni_state::sunsoft3_rom_banking_w(offs_t offset, u8 data)
{
	switch (offset & 0x7800)
	{
		case 0x0800:
		case 0x1800:
		case 0x2800:
		case 0x3800:
			v_set_videorom_bank((offset >> 11) & 0x06, 2, data * 2);
			break;

		case 0x7800:
			prg16(0, data);
			break;
	}

}

void vs_uni_state::init_platoon()
{
	// point program banks to last 32K
	init_prg_banking();

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vs_uni_state::sunsoft3_rom_banking_w)));
}

//**********************************************************************************
// Vs. Raid on Bungeling Bay (Japan)

// FIXME: this is a bad hack! The unused 8K ROM in the driver supposedly runs on the main CPU (protection?)
// and does some sort of handshake (with IRQs) with the game running on the sub CPU.
void vs_uni_state::set_bnglngby_irq_w(u8 data)
{
	m_ret = data;
	m_maincpu->set_input_line(0, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
	// other values ???
	// 0, 4, 84
}

u8 vs_uni_state::set_bnglngby_irq_r()
{
	return m_ret;
}

void vs_uni_state::init_bnglngby()
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0231, 0x0231, read8smo_delegate(*this, FUNC(vs_uni_state::set_bnglngby_irq_r)), write8smo_delegate(*this, FUNC(vs_uni_state::set_bnglngby_irq_w)));

	m_ret = 0;

	// normal banking
	init_vsnormal();
}

//**********************************************************************************
// VS DualSystem

template <u8 Side>
void vs_dual_state::vsdual_vrom_banking_w(u8 data)
{
	// switch vrom
	m_chr_banks[Side]->set_entry(BIT(data, 2));

	// bit 1 ( data & 2 ) triggers irq on the other cpu
	auto &cpu = (Side == SUB) ? *m_maincpu : *m_subcpu;
	cpu.set_input_line(0, (data & 2) ? CLEAR_LINE : ASSERT_LINE);

	// move along
	vsnes_in0_w<Side>(data);
}

void vs_dual_state::init_vsdual()
{
	// vrom switching is enabled with bit 2 of $4016
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vs_dual_state::vsdual_vrom_banking_w<MAIN>)));
	m_subcpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vs_dual_state::vsdual_vrom_banking_w<SUB>)));
}

//**********************************************************************************
// Vs. Super Mario Bros (Bootleg)

void vs_smbbl_state::smbbl_scanline_cb(int scanline, bool vblank, bool blanked)
{
	// Z80 IRQ is controlled by two factors:
	// - bit 6 of current (next) scanline number
	// - bit 6 of latched scanline number from Z80 reading $4000
	if (!BIT(m_bootleg_latched_scanline, 6))
	{
		m_subcpu->set_input_line(INPUT_LINE_IRQ0, BIT(scanline + 1, 6) ? ASSERT_LINE : CLEAR_LINE);
	}
}

u8 vs_smbbl_state::smbbl_ppu_data_r()
{
	// CPU always reads higher CHR ROM banks from $2007, PPU always reads lower ones
	m_chr_banks[0]->set_entry(1);
	const u8 data = m_ppu1->read(0x2007);
	m_chr_banks[0]->set_entry(0);

	return data;
}


//******************************************************************************


void vs_base_state::vsnes_cpu1_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x3fff).rw(m_ppu1, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x4014, 0x4014).w(m_ppu1, FUNC(ppu2c0x_device::spriteram_dma));
	map(0x4016, 0x4016).rw(FUNC(vs_base_state::vsnes_in0_r<MAIN>), FUNC(vs_base_state::vsnes_in0_w<MAIN>));
	map(0x4017, 0x4017).r(FUNC(vs_base_state::vsnes_in1_r<MAIN>)); // IN1 - input port 2 / PSG second control register
	map(0x4020, 0x5fff).rw(FUNC(vs_base_state::vsnes_coin_counter_r<MAIN>), FUNC(vs_base_state::vsnes_coin_counter_w<MAIN>));
	map(0x6000, 0x67ff).mirror(0x1800).ram().share("nvram");
	// Games that don't bank PRG
	map(0x8000, 0xffff).rom().region("prg", 0);
	// Games that bank PRG
	map(0x8000, 0xffff).view(m_prg_view);
	m_prg_view[0](0x8000, 0x9fff).bankr(m_prg_banks[0]);
	m_prg_view[0](0xa000, 0xbfff).bankr(m_prg_banks[1]);
	m_prg_view[0](0xc000, 0xdfff).bankr(m_prg_banks[2]);
	m_prg_view[0](0xe000, 0xffff).bankr(m_prg_banks[3]);
}

void vs_base_state::vsnes_cpu2_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x3fff).rw(m_ppu2, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x4014, 0x4014).w(m_ppu2, FUNC(ppu2c0x_device::spriteram_dma));
	map(0x4016, 0x4016).rw(FUNC(vs_base_state::vsnes_in0_r<SUB>), FUNC(vs_base_state::vsnes_in0_w<SUB>));
	map(0x4017, 0x4017).r(FUNC(vs_base_state::vsnes_in1_r<SUB>));  // IN1 - input port 2 / PSG second control register
	map(0x4020, 0x5fff).rw(FUNC(vs_base_state::vsnes_coin_counter_r<SUB>), FUNC(vs_base_state::vsnes_coin_counter_w<SUB>));
	map(0x6000, 0x67ff).mirror(0x1800).ram().share("nvram");
	map(0x8000, 0xffff).rom();
}

void vs_base_state::vsnes_ppu1_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr(m_chr_banks[0]);
	map(0x0000, 0x1fff).view(m_chr_view);
	m_chr_view[0](0x0000, 0x1fff).ram();
	map(0x2000, 0x2fff).mirror(0x1000).ram();
	map(0x3f00, 0x3fff).rw(m_ppu1, FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
}

void vs_base_state::vsnes_ppu2_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr(m_chr_banks[1]);
	map(0x2000, 0x2fff).mirror(0x1000).ram();
	map(0x3f00, 0x3fff).rw(m_ppu2, FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
}



u8 vs_smbbl_state::smbbl_z80_address_r(offs_t offset)
{
	// NMI routine uses the value read here as the low part of an offset from 0x2000 to store a value read at 0x4000
	// before reading 0x6000 and returning
	if (!(offset & 1))
	{
		m_subcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}

	return 0xf0 | m_bootleg_sound_offset;
}

void vs_smbbl_state::smbbl_6502_sn_w(offs_t offset, u8 data)
{
	m_bootleg_sound_offset = offset & 0xf;
	m_bootleg_sound_data = data;
	m_subcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

u8 vs_smbbl_state::smbbl_z80_data_r()
{
	//printf("Z80 read data %02x\n", m_bootleg_sound_data);
	m_bootleg_latched_scanline = m_ppu1->get_current_scanline();
	m_subcpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

	return m_bootleg_sound_data;
}

// A0 = SN #1 enable, A1 = SN #2 enable, A2 = SN write enables (both) on edge
void vs_smbbl_state::smbbl_z80_sn_w(offs_t offset, u8 data)
{
//  printf("sn_w: ofs %x\n", offset & 7);
	if (!(offset & 4))
	{
		if (offset & 1)
		{
			m_sn1->write(data);
		}

		if (offset & 2)
		{
			m_sn2->write(data);
		}
	}
}


// the bootleg still makes writes to the PSG addresses, it seems the Z80 should interpret them to play the sounds
void vs_smbbl_state::smbbl_6502_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x0800).ram();
	map(0x2000, 0x3fff).rw(m_ppu1, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x2007, 0x2007).r(FUNC(vs_smbbl_state::smbbl_ppu_data_r));
	map(0x4000, 0x400f).w(FUNC(vs_smbbl_state::smbbl_6502_sn_w));
	map(0x4016, 0x4016).rw(FUNC(vs_smbbl_state::vsnes_in0_r<MAIN>), FUNC(vs_smbbl_state::vsnes_in0_w<MAIN>));
	map(0x4017, 0x4017).r(FUNC(vs_smbbl_state::vsnes_in1_r<MAIN>)); // IN1 - input port 2 / PSG second control register
	map(0x4020, 0x4020).w(FUNC(vs_smbbl_state::vsnes_coin_counter_w<MAIN>));
	map(0x6000, 0x67ff).mirror(0x0800).ram();
	map(0x8000, 0xffff).rom();
}

// from kevtris' schematics, 2 74LS139s do the Z80 address decoding.
// A13 = 0, A14 = 0 is ROM
// A13 = 1, A14 = 0 is RAM
// A13 = 0, A14 = 1 is 6502 data.  Read also acks the Z80 IRQ.
// A13 = 1, A14 = 1 is 6502 lower 4 address bits on read, SN76489 on write.  Read also acks the Z80 NMI.
//                     A0 = SN #1 enable, A1 = SN #2 enable, A2 = SN write enables (both)

// x00x xxxx xxxx xxxx = ROM (0000)
// x01x xxxx xxxx xxxx = RAM (2000)
// x10x xxxx xxxx xxxx = 6502 data (4000)
// x11x xxxx xxxx xxxx = 6502 address (6000) (read)
// x11x xxxx xxxx xW21 = SN76489s (6000) (write)

void vs_smbbl_state::smbbl_z80_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("sub", 0);
	map(0x2000, 0x27ff).mirror(0x1800).ram();
	map(0x4000, 0x5fff).r(FUNC(vs_smbbl_state::smbbl_z80_data_r)); // read in IRQ & NMI
	map(0x6000, 0x7fff).rw(FUNC(vs_smbbl_state::smbbl_z80_address_r), FUNC(vs_smbbl_state::smbbl_z80_sn_w));
}

void vs_smbbl_state::smbbl_ppu_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr(m_chr_banks[0]);
	map(0x2000, 0x27ff).mirror(0x1800).ram();
	map(0x3f00, 0x3fff).rw(m_ppu1, FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
}

//******************************************************************************

#define VS_UNI_JOYSTICK(port_tag, player, type1, name1)  \
	PORT_START(port_tag)                                 \
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(player) PORT_NAME("%p A")  \
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(player) PORT_NAME("%p B")  \
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, type1 ) PORT_NAME(name1)                 \
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )                             \
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(player)    \
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(player)  \
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(player)  \
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player)

#define VS_DUAL_JOYSTICK(port_tag, player, type1, type2, name1, name2)  \
	PORT_START(port_tag)                                                \
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(player) PORT_NAME("%p A")  \
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(player) PORT_NAME("%p B")  \
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, type1 ) PORT_NAME(name1)                 \
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, type2 ) PORT_NAME(name2)                 \
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(player)    \
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(player)  \
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(player)  \
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player)

// bit 0: serial controller pin
// bit 1: always 0 (from floating input on 74LS240)
// bit 2: service switch
// bit 3-4: DSW SW:1,2
// bit 5-6: coins
// bit 7: CPU indicator bit (Main: 0, Sub: 1)
// Note: Port impulse is 3 frames since 1 frame is too short for some games (T.K.O. Boxing).
// Testing on VS. hardware shows the actual coin drop time varies between 2.5 to 4 frames.
#define VS_COINS_AND_SERVICE(coin_tag, service, coin1, coin2)  \
	PORT_START(coin_tag)                                       \
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )               \
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )               \
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, service) PORT_IMPULSE(3)   \
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )               \
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )               \
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, coin1 ) PORT_IMPULSE(3)    \
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, coin2 ) PORT_IMPULSE(3)    \
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

// So-called "reversed" controls. This represents the actual VS. wiring. The left controller
// and buttons are read from $4017, while the right controller and buttons are read from $4016.
// Most games follow the standard convention and consider the left controller P1, when it matters.
// Note, this is reversed from NES where P1 is read from $4016 and P2 is read from $4017.
static INPUT_PORTS_START( vsnes_rev )
	VS_UNI_JOYSTICK("IN0", 2, IPT_START1, "Select 1 (Blue)")
	VS_UNI_JOYSTICK("IN1", 1, IPT_START2, "Select 2 (Green)")
	VS_COINS_AND_SERVICE("COINS1", IPT_SERVICE1, IPT_COIN1, IPT_COIN2)

	// There is also a DSW which is defined per game, below
INPUT_PORTS_END

// Only used by SMB, TKO Boxing, and Mighty Bomb Jack. In 2 player games these
// games consider the right controls to be P1, so we relabel them for convenience.
static INPUT_PORTS_START( vsnes )
	VS_UNI_JOYSTICK("IN0", 1, IPT_START1, "Select 1 (Blue)")
	VS_UNI_JOYSTICK("IN1", 2, IPT_START2, "Select 2 (Green)")
	VS_COINS_AND_SERVICE("COINS1", IPT_SERVICE1, IPT_COIN1, IPT_COIN2)

	// There is also a DSW which is defined per game, below
INPUT_PORTS_END

// Standard "reversed" controls for DualSystem.
static INPUT_PORTS_START( vsnes_dual_rev )
	// Left Side Controls
	VS_DUAL_JOYSTICK("IN0", 2, IPT_START1, IPT_START3, "Select 1 (Blue)", "Select 3 (Purple)")
	VS_DUAL_JOYSTICK("IN1", 1, IPT_START2, IPT_START4, "Select 2 (Green)", "Select 4 (Yellow)")
	VS_COINS_AND_SERVICE("COINS1", IPT_SERVICE1, IPT_COIN1, IPT_COIN2)

	// Right Side Controls
	VS_DUAL_JOYSTICK("IN2", 4, IPT_START5, IPT_START7, "2nd Side Select 1 (Blue)", "2nd Side Select 3 (Purple)")
	VS_DUAL_JOYSTICK("IN3", 3, IPT_START6, IPT_START8, "2nd Side Select 2 (Green)", "2nd Side Select 4 (Yellow)")
	VS_COINS_AND_SERVICE("COINS2", IPT_SERVICE2, IPT_COIN3, IPT_COIN4)

	// Both sides also have a DSW (#0 & #1) which are defined per game, below
INPUT_PORTS_END

// Only used by vstennis and defined for convenience. In a doubles match P1 and P3 will serve
// first for their respective side. On hardware this is the right controls on either panel.
static INPUT_PORTS_START( vsnes_dual )
	// Left Side Controls
	VS_DUAL_JOYSTICK("IN0", 1, IPT_START1, IPT_START3, "Select 1 (Blue)", "Select 3 (Purple)")
	VS_DUAL_JOYSTICK("IN1", 2, IPT_START2, IPT_START4, "Select 2 (Green)", "Select 4 (Yellow)")
	VS_COINS_AND_SERVICE("COINS1", IPT_SERVICE1, IPT_COIN1, IPT_COIN2)

	// Right Side Controls
	VS_DUAL_JOYSTICK("IN2", 3, IPT_START5, IPT_START7, "2nd Side Select 1 (Blue)", "2nd Side Select 3 (Purple)")
	VS_DUAL_JOYSTICK("IN3", 4, IPT_START6, IPT_START8, "2nd Side Select 2 (Green)", "2nd Side Select 4 (Yellow)")
	VS_COINS_AND_SERVICE("COINS2", IPT_SERVICE2, IPT_COIN3, IPT_COIN4)

	// Both sides also have a DSW (#0 & #1) which are defined per game, below
INPUT_PORTS_END

static INPUT_PORTS_START( vsnes_zapper )
	PORT_START("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x10, 0x10, "Gun Alarm Wire" )           // ALM wire (ignored by Gumshoe and Freedom Force?)
	PORT_CONFSETTING(    0x00, "Disconnected" )
	PORT_CONFSETTING(    0x10, "Connected" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM )            // gun HIT
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )           // gun TRG

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	VS_COINS_AND_SERVICE("COINS1", IPT_SERVICE1, IPT_COIN1, IPT_COIN2)

	PORT_START("GUNX")  // FAKE - Gun X pos
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_MINMAX(0, 255)

	PORT_START("GUNY")  // FAKE - Gun Y pos
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_MINMAX(0, 239)
INPUT_PORTS_END


static INPUT_PORTS_START( topgun )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, "Lives per Coin" )        PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, "3 - 12 Max" )
	PORT_DIPSETTING(    0x08, "2 - 9 Max" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "30k and Every 50k" )
	PORT_DIPSETTING(    0x20, "50k and Every 100k" )
	PORT_DIPSETTING(    0x10, "100k and Every 150k" )
	PORT_DIPSETTING(    0x30, "200k and Every 200k" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( platoon )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hard ) )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!6,!7,!8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( golf )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, "Hole Size" )             PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, "Large" )
	PORT_DIPSETTING(    0x08, "Small" )
	PORT_DIPNAME( 0x10, 0x00, "Points per Stroke" )     PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easier ) )       // See table below for "OFF" setting
	PORT_DIPSETTING(    0x10, DEF_STR( Harder ) )       // See table below for "ON" setting
/*
Stroke Play   OFF/ON
Hole in 1     +5  +4
Double Eagle  +4  +3
Eagle         +3  +2
Birdie        +2  +1
Par           +1   0
Bogey          0  -1
Other          0  -2 (IE: Double Bogey and Over)

Match Play  OFF/ON
Win Hole    +1  +2
Tie          0   0
Lose Hole   -1  -2
*/
	PORT_DIPNAME( 0x60, 0x20, "Starting Points" )       PORT_DIPLOCATION("SW1:!6,7")
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x40, "13" )
	PORT_DIPSETTING(    0x20, "16" )
	PORT_DIPSETTING(    0x60, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Difficulty Vs. Computer" )   PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( golfj )
	PORT_INCLUDE( golf )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x60, 0x20, "Starting Points" )       PORT_DIPLOCATION("SW1:!6,7")
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x40, "400" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x60, "600" )
INPUT_PORTS_END

// Same as 'golf', but 4 start buttons
// FIXME: since UniSystems only have P1/P2 start buttons this implies the golf4s versions are for DualSystem only
static INPUT_PORTS_START( golf4s )
	PORT_INCLUDE( golf )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("Select 3 (Purple)")  // START on a NES

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START4 ) PORT_NAME("Select 4 (Yellow)")  // START on a NES
INPUT_PORTS_END

static INPUT_PORTS_START( vstennis )
	PORT_INCLUDE( vsnes_dual )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x03, 0x00, "Difficulty Vs. Computer" )   PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0c, 0x00, "Difficulty Vs. Player" ) PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, "Raquet Size" )           PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, "Large" )
	PORT_DIPSETTING(    0x10, "Small" )
	PORT_DIPNAME( 0x20, 0x00, "Extra Score" )           PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "1 Set" )
	PORT_DIPSETTING(    0x20, "1 Game" )
	PORT_DIPNAME( 0x40, 0x00, "Court Color" )           PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPSETTING(    0x40, "Blue" )
	PORT_DIPNAME( 0x80, 0x00, "Copyright" )             PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x80, DEF_STR( USA ) )

	PORT_START("DSW1")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH )                PORT_DIPLOCATION("SW2:!1")
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:!2,!3")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x08, "Game Mode - Credits: 1VsC/2VsC/1Vs1/2Vs2" )  PORT_DIPLOCATION("SW2:!4,!5")
	PORT_DIPSETTING(    0x00, "A - 1/1/2/2" )
	PORT_DIPSETTING(    0x10, "B - 1/2/2/2" )
	PORT_DIPSETTING(    0x08, "C - 1/1/2/4" )
	PORT_DIPSETTING(    0x18, "D - 1/2/2/4" )
	PORT_DIPNAME( 0x60, 0x00, "Rackets Per Game" )      PORT_DIPLOCATION("SW2:!6,!7")
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wrecking )
	PORT_INCLUDE( vsnes_dual_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x18, "1st Bonus Life" )        PORT_DIPLOCATION("SW1:!3,!4,!5,")
	PORT_DIPSETTING(    0x00, "20,000 Pts" )
	PORT_DIPSETTING(    0x10, "30,000 Pts" )
	PORT_DIPSETTING(    0x08, "40,000 Pts" )
	PORT_DIPSETTING(    0x18, "50,000 Pts" )
	PORT_DIPSETTING(    0x04, "70,000 Pts" )
	PORT_DIPSETTING(    0x14, "80,000 Pts" )
	PORT_DIPSETTING(    0x0c, "100,000 Pts" )
	PORT_DIPSETTING(    0x1c, DEF_STR( None ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Additional Bonus Lives" )    PORT_DIPLOCATION("SW1:!6,!7,!8") // Manual shows NONE as Factory Shipment Setting
	PORT_DIPSETTING(    0x00, "20,000 Pts" )
	PORT_DIPSETTING(    0x80, "30,000 Pts" )
	PORT_DIPSETTING(    0x40, "40,000 Pts" )
	PORT_DIPSETTING(    0xc0, "50,000 Pts" )
	PORT_DIPSETTING(    0x20, "70,000 Pts" )
	PORT_DIPSETTING(    0x60, "80,000 Pts" )
	PORT_DIPSETTING(    0xa0, "100,000 Pts" )
	PORT_DIPSETTING(    0xe0, DEF_STR( None ) )

	PORT_START("DSW1")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:!4,!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x00, "Copyright" )             PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Japan ) )        // (c) Nintendo Co., Ltd.
	PORT_DIPSETTING(    0x20, DEF_STR( USA ) )          // (c) Nintendo of America Inc.
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:!7" )       // Manual states this is Unused
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:!8" )       // Manual states this is Unused
INPUT_PORTS_END

static INPUT_PORTS_START( balonfgt )
	PORT_INCLUDE( vsnes_dual_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW1:!4" )       // Manual states this is Unused
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW1:!5" )       // Manual states this is Unused
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW1:!6" )       // Manual states this is Unused
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW1:!7" )       // Manual states this is Unused
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )                PORT_DIPLOCATION("SW1:!8")

	PORT_START("DSW1")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, "Enemy Regeneration" )    PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x10, DEF_STR( High ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!6,!7")
	PORT_DIPSETTING(    0x00, "10,000 Pts" )
	PORT_DIPSETTING(    0x40, "20,000 Pts" )
	PORT_DIPSETTING(    0x20, "40,000 Pts" )
	PORT_DIPSETTING(    0x60, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:!8" )       // Manual states this is Unused
INPUT_PORTS_END

// TODO: verify DIPs for Rules, Difficulty, and Dora. Finding a copy of the manual would certainly help.
static INPUT_PORTS_START( vsmahjng )
	PORT_INCLUDE( vsnes_dual_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x01, 0x00, "Rules" )                 PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x00, "Kansai" )
	PORT_DIPSETTING(    0x01, "Kantou" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!2,!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x08, 0x00, "Timer" )                 PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, "Initial Time" )          PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, "30" )
	PORT_DIPSETTING(    0x10, "45" )
	PORT_DIPSETTING(    0x20, "60" )
	PORT_DIPSETTING(    0x00, "90" )
	PORT_DIPNAME( 0xc0, 0x00, "Additional Time" )       PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x40, "12" )
	PORT_DIPSETTING(    0x80, "15" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW1")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH )                PORT_DIPLOCATION("SW2:!1")
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:!2,!3")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, "Mawashi" )               PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(    0x08, "Tonnan" )
	PORT_DIPSETTING(    0x00, "Tonton" )
	PORT_DIPNAME( 0x10, 0x00, "Dora" )                  PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x00, "Genbutsu" )
	PORT_DIPSETTING(    0x10, "Next" )
	PORT_DIPNAME( 0x60, 0x00, "Starting Points" )       PORT_DIPLOCATION("SW2:!6,!7")
	PORT_DIPSETTING(    0x60, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x40, "25000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( vsbball )
	PORT_INCLUDE( vsnes_dual_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x03, 0x01, "Player Defense Strength" )   PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "Weak" )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x03, "Strong" )
	PORT_DIPNAME( 0x0c, 0x04, "Player Offense Strength" )   PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x00, "Weak" )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x0c, "Strong" )
	PORT_DIPNAME( 0x30, 0x20, "Computer Defense Strength" ) PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "Weak" )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x30, "Strong" )
	PORT_DIPNAME( 0xc0, 0x80, "Computer Offense Strength" ) PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "Weak" )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0xc0, "Strong" )

	PORT_START("DSW1")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH )                PORT_DIPLOCATION("SW2:!1")
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:!2,!3")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x30, "Starting Points" )       PORT_DIPLOCATION("SW2:!4,!5,!6")
	PORT_DIPSETTING(    0x00, "80 Pts" )
	PORT_DIPSETTING(    0x20, "100 Pts" )
	PORT_DIPSETTING(    0x10, "150 Pts" )
	PORT_DIPSETTING(    0x30, "200 Pts" )
	PORT_DIPSETTING(    0x08, "250 Pts" )
	PORT_DIPSETTING(    0x28, "300 Pts" )
	PORT_DIPSETTING(    0x18, "350 Pts" )
	PORT_DIPSETTING(    0x38, "400 Pts" )
	PORT_DIPNAME( 0x40, 0x00, "Bonus Play" )            PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( iceclmrj )
	PORT_INCLUDE( vsnes_dual_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, "Coinage (Left Side)" )   PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x00, "Lives (Left Side)" )     PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x18, "7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW1:!7" )
	PORT_DIPNAME( 0x80, 0x00, "Service Mode (Left Side)" )  PORT_TOGGLE PORT_CODE(KEYCODE_F2)   PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, "Coinage (Right Side)" )  PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x00, "Lives (Right Side)" )    PORT_DIPLOCATION("SW2:!4,!5")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x18, "7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW2:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW2:!7" )
	PORT_DIPNAME( 0x80, 0x00, "Service Mode (Right Side)" ) PORT_TOGGLE PORT_CODE(KEYCODE_F1)   PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( drmario )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x03, 0x03, "Drop Rate Increases After" ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "7 Pills" )
	PORT_DIPSETTING(    0x01, "8 Pills" )
	PORT_DIPSETTING(    0x02, "9 Pills" )
	PORT_DIPSETTING(    0x03, "10 Pills" )
	PORT_DIPNAME( 0x0c, 0x04, "Virus Level" )           PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "7" )
	PORT_DIPNAME( 0x30, 0x00, "Drop Speed Up" )         PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, "Fast" )
	PORT_DIPSETTING(    0x30, "Fastest" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( rbibb )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "1p/inn, 2p/inn, Time/Min" )  PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, "2, 1, 3" )
	PORT_DIPSETTING(    0x0c, "2, 2, 4" )
	PORT_DIPSETTING(    0x00, "3, 2, 6" )
	PORT_DIPSETTING(    0x08, "4, 3, 7" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0x00, "PPU Type" )              PORT_DIPLOCATION("SW1:!6,!7,!8")
	PORT_DIPSETTING(    0x00, "RP2C04-0001" )
	PORT_DIPSETTING(    0x20, "RP2C03" )
	PORT_DIPSETTING(    0x40, "RP2C04-0002" )
//  PORT_DIPSETTING(    0x60, "RP2C03" )
	PORT_DIPSETTING(    0x80, "RP2C04-0003" )
//  PORT_DIPSETTING(    0xa0, "RP2C03" )
	PORT_DIPSETTING(    0xc0, "RP2C04-0004" )
//  PORT_DIPSETTING(    0xe0, "RP2C03" )
INPUT_PORTS_END

// Note: does not check Coin 2 inputs
static INPUT_PORTS_START( btlecity )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x01, 0x01, "Credits for 2 Players" ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:!4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:!5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:!6" )
	PORT_DIPNAME( 0xc0, 0x00, "PPU Type" )              PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "RP2C04-0001" )
	PORT_DIPSETTING(    0x40, "RP2C04-0002" )
	PORT_DIPSETTING(    0x80, "RP2C04-0003" )
	PORT_DIPSETTING(    0xc0, "RP2C04-0004" )
INPUT_PORTS_END

static INPUT_PORTS_START( cluclu )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:!4" )
	PORT_DIPNAME( 0x10, 0x10, "Credits for 2 Players" ) PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!6,!7")
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!8" )
INPUT_PORTS_END

static INPUT_PORTS_START( cstlevna )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x30, 0x00, "Bonus points" )          PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "Timer x 4, Hearts x 40" )
	PORT_DIPSETTING(    0x20, "Timer x 6, Hearts x 60" )
	PORT_DIPSETTING(    0x10, "Timer x 8, Hearts x 80" )
	PORT_DIPSETTING(    0x30, "Timer x 9, Hearts x 90" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!7")  // Damage taken
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )                     // Normal
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )                     // Double
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW1:!8" )       // Manual states Must be Set to "OFF"
INPUT_PORTS_END

static INPUT_PORTS_START( iceclimb )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x18, "7" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x00, "Time before bear appears" )  PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "Long" )
	PORT_DIPSETTING(    0x40, "Short" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW1:!8" )
INPUT_PORTS_END

// Same as 'iceclimb', but different buttons mapping and input protection
static INPUT_PORTS_START( iceclmbj )
	PORT_INCLUDE( iceclimb )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM )        // protection?  START on a NES

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM )        // protection?  START on a NES
INPUT_PORTS_END

static INPUT_PORTS_START( excitebk )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x00, "Bonus Bike" )            PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, "100k and Every 50k" )
	PORT_DIPSETTING(    0x10, "Every 100k" )
	PORT_DIPSETTING(    0x08, "100k Only" )
	PORT_DIPSETTING(    0x18, DEF_STR( None ) )
	PORT_DIPNAME( 0x20, 0x00, "1st Half Qualifying Time" )  PORT_DIPLOCATION("SW1:!6") // Manual calls this "Time/Challenge"
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x00, "2nd Half Qualifying Time" )  PORT_DIPLOCATION("SW1:!7") // Manual calls this "Time/Excitebike"
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!8" )
INPUT_PORTS_END

// Note: does not check Coin 2 inputs
static INPUT_PORTS_START( jajamaru )
	PORT_INCLUDE( vsnes_rev )

	// FIXME: jajamaru needs button 3 to select a 2 player game. This is installed here for now, but really 2 player mode should only work on a DualSystem.
	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("Select 3 (Purple)")  // START on a NES

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x18, "6" )                     // status bar only shows up to 4 reserve lives
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW1:!7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( machridr )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x00, "Time" )                  PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, "280 (Easy)" )
	PORT_DIPSETTING(    0x10, "250" )
	PORT_DIPSETTING(    0x08, "220" )
	PORT_DIPSETTING(    0x18, "200 (Hard)" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:!6" )
	PORT_DIPNAME( 0x40, 0x00, "Enemies" )               PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, "More" )
	PORT_DIPSETTING(    0x00, "Less" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!8" )
INPUT_PORTS_END

// Note: does not check Coin 2 inputs
static INPUT_PORTS_START( machridj )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x00, "Max Lives After Level 1" )  PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x08, "6" )
// # of lives is determined by remaining energy bars at the end of level 1
// formula: lives = min(floor(energy / divisor) + 1, maxval)
// maxval:     3   4   5   6
// divisor:    4   3   2   1
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:!6" )
	PORT_DIPNAME( 0x40, 0x00, "Enemies" )               PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, "More" )
	PORT_DIPSETTING(    0x00, "Less" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!8" )
INPUT_PORTS_END

static INPUT_PORTS_START( suprmrio )
	PORT_INCLUDE( vsnes )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "100 Coins" )
	PORT_DIPSETTING(    0x20, "150 Coins" )
	PORT_DIPSETTING(    0x10, "200 Coins" )
	PORT_DIPSETTING(    0x30, "250 Coins" )
	PORT_DIPNAME( 0x40, 0x00, "Timer" )                 PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x40, "Fast" )
	PORT_DIPNAME( 0x80, 0x00, "Continue Lives" )        PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( duckhunt )
	PORT_INCLUDE( vsnes_zapper )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Misses per game" )       PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
	PORT_DIPSETTING(    0x80, "80000" )
	PORT_DIPSETTING(    0xc0, "100000" )
INPUT_PORTS_END

static INPUT_PORTS_START( hogalley )
	PORT_INCLUDE( vsnes_zapper )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Misses per game" )       PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
	PORT_DIPSETTING(    0x80, "80000" )
	PORT_DIPSETTING(    0xc0, "100000" )
INPUT_PORTS_END

static INPUT_PORTS_START( vsgshoe )
	PORT_INCLUDE( vsnes_zapper )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Bullets per Balloon" )   PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPNAME( 0x80, 0x00, "Bonus Man Awarded" )     PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, "At 50000" )
	PORT_DIPSETTING(    0x80, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( vsfdf )
	PORT_INCLUDE( vsnes_zapper )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x10, "Health Awarded At" )     PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, "20k/60k" )
	PORT_DIPSETTING(    0x08, "20k/60k/Every 60k" )
	PORT_DIPSETTING(    0x10, "10k/50k")
	PORT_DIPSETTING(    0x18, "10k/50k/Every 50k" )
	PORT_DIPNAME( 0x60, 0x00, "Difficulty (Damage)" )   PORT_DIPLOCATION("SW1:!6,!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest) )
	PORT_DIPNAME( 0x80, 0x00, "Difficulty (Enemy)" )    PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( vstetris )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW1:!1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW1:!2" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:!5" )
	PORT_DIPNAME( 0xe0, 0x00, "PPU Type" )              PORT_DIPLOCATION("SW1:!6,!7,!8")
	PORT_DIPSETTING(    0x00, "RP2C04-0001" )
	PORT_DIPSETTING(    0x20, "RP2C03" )
	PORT_DIPSETTING(    0x40, "RP2C04-0002" )
//  PORT_DIPSETTING(    0x60, "RP2C03" )
	PORT_DIPSETTING(    0x80, "RP2C04-0003" )
//  PORT_DIPSETTING(    0xa0, "RP2C03" )
	PORT_DIPSETTING(    0xc0, "RP2C04-0004" )
//  PORT_DIPSETTING(    0xe0, "RP2C03" )
INPUT_PORTS_END

// Note: does not check Coin 2 inputs
static INPUT_PORTS_START( vsskykid )
	PORT_INCLUDE( vsnes_rev )

	// FIXME: according to the manual, the operator "must solder a jumper wire connecting traces 9 and 10 on the edge connector P2"
	// This mod will make Select 2 start a two player game on a UniSystem, as the game is really hard coded to use the nonexistent Select 3.
	// Similar fake button for jajamaru can also be removed when this is fixed.
	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("Select 3 (Purple)")  // START on a NES

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xe0, 0x00, "PPU Type" )              PORT_DIPLOCATION("SW1:!6,!7,!8")
	PORT_DIPSETTING(    0x00, "RP2C04-0001" )
	PORT_DIPSETTING(    0x20, "RP2C03" )
	PORT_DIPSETTING(    0x40, "RP2C04-0002" )
//  PORT_DIPSETTING(    0x60, "RP2C03" )
	PORT_DIPSETTING(    0x80, "RP2C04-0003" )
//  PORT_DIPSETTING(    0xa0, "RP2C03" )
	PORT_DIPSETTING(    0xc0, "RP2C04-0004" )
//  PORT_DIPSETTING(    0xe0, "RP2C03" )
INPUT_PORTS_END

// In US Pinball both P1/P2 B buttons move left flipper, both A buttons move right flipper. All four operate the plunger.
// In JP Pinball both P1 B/A buttons move left flipper, both P2 B/A move right flipper. Both A buttons operate the plunger.
static INPUT_PORTS_START( vspinbal )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )        PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, "Side Drain Walls" )      PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Low ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPSETTING(    0x10, "70000" )
	PORT_DIPNAME( 0x60, 0x00, "Balls" )                 PORT_DIPLOCATION("SW1:!6,!7")
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x80, 0x00, "Ball speed" )            PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x80, "Fast" )
INPUT_PORTS_END

static INPUT_PORTS_START( goonies )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x10, "60000" )
	PORT_DIPSETTING(    0x30, "70000" )
	PORT_DIPNAME(0x40,  0x00, "Timer" )                 PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x40, "Fast" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

// Note: set "SC4-2 A" does not check Coin 2 inputs
static INPUT_PORTS_START( vssoccer )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x00, "Points Timer" )          PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, "1000 Pts" )
	PORT_DIPSETTING(    0x10, "1500 Pts" )
	PORT_DIPSETTING(    0x08, "2000 Pts" )
	PORT_DIPSETTING(    0x18, "600 Pts" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!6,!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW1:!8" )
INPUT_PORTS_END

static INPUT_PORTS_START( vssoccera )
	PORT_INCLUDE( vssoccer )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x18, 0x18, "Points Timer" )          PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, "600 Pts" )
	PORT_DIPSETTING(    0x10, "800 Pts" )
	PORT_DIPSETTING(    0x08, "1000 Pts" )
	PORT_DIPSETTING(    0x18, "1200 Pts" )
INPUT_PORTS_END

static INPUT_PORTS_START( vsgradus )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x30, 0x00, "Bonus" )                 PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "100k" )
	PORT_DIPSETTING(    0x20, "200k" )
	PORT_DIPSETTING(    0x10, "300k" )
	PORT_DIPSETTING(    0x30, "400k" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( vsslalom )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, "Freestyle Points" )      PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, "Left / Right" )
	PORT_DIPSETTING(    0x08, "Hold Time" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Inverted input" )        PORT_DIPLOCATION("SW1:!8")       // Manual states Not Used - Must be Set to "OFF"
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

// Note: does not check Coin 2 inputs
static INPUT_PORTS_START( starlstr )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:!3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:!4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:!5" )
	PORT_DIPNAME( 0xe0, 0x00, "PPU Type" )              PORT_DIPLOCATION("SW1:!6,!7,!8")
	PORT_DIPSETTING(    0x00, "RP2C03" )
	PORT_DIPSETTING(    0x20, "RP2C04-0001" )
//  PORT_DIPSETTING(    0x40, "RP2C03" )
	PORT_DIPSETTING(    0x60, "RP2C04-0002" )
//  PORT_DIPSETTING(    0x80, "RP2C03" )
	PORT_DIPSETTING(    0xa0, "RP2C04-0003" )
//  PORT_DIPSETTING(    0xc0, "RP2C03" )
	PORT_DIPSETTING(    0xe0, "RP2C04-0004" )
INPUT_PORTS_END

static INPUT_PORTS_START( tkoboxng )
	PORT_INCLUDE( vsnes )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Very_Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW1:!5" )
	PORT_DIPNAME( 0xe0, 0x00, "PPU Type" )              PORT_DIPLOCATION("SW1:!6,!7,!8")
	PORT_DIPSETTING(    0x00, "RP2C04-0003" )
//  PORT_DIPSETTING(    0x20, "RP2C03" )
//  PORT_DIPSETTING(    0x40, "RP2C03" )
//  PORT_DIPSETTING(    0x60, "RP2C03" )
//  PORT_DIPSETTING(    0x80, "RP2C03" )
//  PORT_DIPSETTING(    0xa0, "RP2C03" )
//  PORT_DIPSETTING(    0xc0, "RP2C03" )
	PORT_DIPSETTING(    0xe0, "RP2C03" )
INPUT_PORTS_END

static INPUT_PORTS_START( bnglngby )
	PORT_INCLUDE( vsnes_rev )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM )        // protection?  START on a NES

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM )        // protection?  START on a NES

	// FIXME: This is faking that the game is running on the Sub CPU and can be removed once it really IS running there
	PORT_MODIFY("COINS1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:!5" )      // stored in $f5, is it ever used?
// TODO: check if there's a more specific meaning than difficulty for this.
// The game seems to peg several things to level# ($d2) + DIP determined value ($0233).
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!6,!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!8" )
INPUT_PORTS_END

static INPUT_PORTS_START( mightybj )
	PORT_INCLUDE( vsnes )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW1:!7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!8" )
INPUT_PORTS_END

// Note: does not check Coin 2 inputs
static INPUT_PORTS_START( supxevs )
	PORT_INCLUDE( vsnes_rev )

	PORT_START("DSW0")  // bit 0 and 1 read from bit 3 and 4 on $4016, rest of the bits read on $4017
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x00, "50k, 150k, every 150k" )
	PORT_DIPSETTING(    0x01, "70k, 200k, every 200k" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:!3" )
	PORT_DIPNAME( 0x08, 0x00, "Hidden Password Screen" )  PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
// Set to Yes and hold both A and B while pressing P1 start to access Password Screen
//     Level 2:  5135
//     Level 3:  3706
//     Level 4:  6853
//     Level 5:  3381
//     Level 6:  3913
//     Level 7:  2311
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0x00, "PPU Type" )              PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "RP2C04-0001" )
	PORT_DIPSETTING(    0x40, "RP2C04-0002" )
	PORT_DIPSETTING(    0x80, "RP2C04-0003" )
	PORT_DIPSETTING(    0xc0, "RP2C04-0004" )
INPUT_PORTS_END


void vs_uni_state::vsnes(machine_config &config)
{
	// basic machine hardware
	rp2a03_device &maincpu(RP2A03(config, m_maincpu, NTSC_APU_CLOCK));
	maincpu.set_addrmap(AS_PROGRAM, &vs_uni_state::vsnes_cpu1_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen1(SCREEN(config, "screen1", SCREEN_TYPE_RASTER));
	screen1.set_raw(RP2A03_NTSC_XTAL / 4, 341, 0, VISIBLE_SCREEN_WIDTH, ppu2c0x_device::NTSC_SCANLINES_PER_FRAME, 0, VISIBLE_SCREEN_HEIGHT);
	screen1.set_screen_update("ppu1", FUNC(ppu2c0x_device::screen_update));

	PPU_2C04(config, m_ppu1);
	m_ppu1->set_addrmap(0, &vs_uni_state::vsnes_ppu1_map);
	m_ppu1->set_screen("screen1");
	m_ppu1->set_cpu_tag(m_maincpu);
	m_ppu1->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	NES_ZAPPER_SENSOR(config, m_sensor, 0).set_screen_tag("screen1");

	// sound hardware
	SPEAKER(config, "mono").front_center();
	maincpu.add_route(ALL_OUTPUTS, "mono", 0.50);

	// watchdog resets system between 1.23s and 1.33s in hardware tests, exact timing unknown
	WATCHDOG_TIMER(config, m_watchdog); // .set_time(attotime::from_msec(1300)); // FIXME: this won't work yet since the UniSystem emulation uses the wrong side of the PCB
}

void vs_uni_state::jajamaru(machine_config &config)
{
	vsnes(config);

	PPU_2C05_01(config.replace(), m_ppu1);
	m_ppu1->set_screen("screen1");
	m_ppu1->set_cpu_tag(m_maincpu);
	m_ppu1->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
}

void vs_uni_state::mightybj(machine_config &config)
{
	vsnes(config);

	PPU_2C05_02(config.replace(), m_ppu1);
	m_ppu1->set_screen("screen1");
	m_ppu1->set_cpu_tag(m_maincpu);
	m_ppu1->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
}

void vs_uni_state::vsgshoe(machine_config &config)
{
	vsnes(config);

	PPU_2C05_03(config.replace(), m_ppu1);
	m_ppu1->set_screen("screen1");
	m_ppu1->set_cpu_tag(m_maincpu);
	m_ppu1->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
}

void vs_uni_state::topgun(machine_config &config)
{
	vsnes(config);

	PPU_2C05_04(config.replace(), m_ppu1);
	m_ppu1->set_addrmap(0, &vs_uni_state::vsnes_ppu1_map);
	m_ppu1->set_screen("screen1");
	m_ppu1->set_cpu_tag(m_maincpu);
	m_ppu1->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
}

void vs_dual_state::vsdual(machine_config &config)
{
	// basic machine hardware
	rp2a03_device &maincpu(RP2A03(config, m_maincpu, NTSC_APU_CLOCK));
	maincpu.set_addrmap(AS_PROGRAM, &vs_dual_state::vsnes_cpu1_map);

	rp2a03_device &subcpu(RP2A03(config, m_subcpu, NTSC_APU_CLOCK));
	subcpu.set_addrmap(AS_PROGRAM, &vs_dual_state::vsnes_cpu2_map);

	// need high level of interleave to keep screens in sync in Balloon Fight.
	config.set_perfect_quantum(m_maincpu);

	config.set_default_layout(layout_dualhsxs);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen1(SCREEN(config, "screen1", SCREEN_TYPE_RASTER));
	screen1.set_raw(RP2A03_NTSC_XTAL / 4, 341, 0, VISIBLE_SCREEN_WIDTH, ppu2c0x_device::NTSC_SCANLINES_PER_FRAME, 0, VISIBLE_SCREEN_HEIGHT);
	screen1.set_screen_update("ppu1", FUNC(ppu2c0x_device::screen_update));

	screen_device &screen2(SCREEN(config, "screen2", SCREEN_TYPE_RASTER));
	screen2.set_raw(RP2A03_NTSC_XTAL / 4, 341, 0, VISIBLE_SCREEN_WIDTH, ppu2c0x_device::NTSC_SCANLINES_PER_FRAME, 0, VISIBLE_SCREEN_HEIGHT);
	screen2.set_screen_update("ppu2", FUNC(ppu2c0x_device::screen_update));

	PPU_2C04(config, m_ppu1);
	m_ppu1->set_addrmap(0, &vs_dual_state::vsnes_ppu1_map);
	m_ppu1->set_screen("screen1");
	m_ppu1->set_cpu_tag(m_maincpu);
	m_ppu1->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PPU_2C04(config, m_ppu2);
	m_ppu2->set_addrmap(0, &vs_dual_state::vsnes_ppu2_map);
	m_ppu2->set_screen("screen2");
	m_ppu2->set_cpu_tag(m_subcpu);
	m_ppu2->int_callback().set_inputline(m_subcpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	maincpu.add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	subcpu.add_route(ALL_OUTPUTS, "rspeaker", 0.50);

	// watchdog resets system between 1.23s and 1.33s in hardware tests, exact timing unknown
	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(1300));
}

void vs_smbbl_state::vs_smbbl(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, XTAL(16'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &vs_smbbl_state::smbbl_6502_map);

	Z80(config, m_subcpu, XTAL(16'000'000)/8); // Z8400APS-Z80CPU
	m_subcpu->set_addrmap(AS_PROGRAM, &vs_smbbl_state::smbbl_z80_map);

	// video hardware
	screen_device &screen1(SCREEN(config, "screen1", SCREEN_TYPE_RASTER));
	screen1.set_refresh_hz(56.69);
	screen1.set_size(32*8, 280);
	screen1.set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	screen1.set_screen_update("ppu1", FUNC(ppu2c0x_device::screen_update));

	PPU_2C04C(config, m_ppu1);
	m_ppu1->set_addrmap(0, &vs_smbbl_state::smbbl_ppu_map);
	m_ppu1->set_cpu_tag(m_maincpu);
	m_ppu1->set_screen("screen1");
	m_ppu1->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76489(config, "sn1", XTAL(16'000'000)/8).add_route(ALL_OUTPUTS, "mono", 0.50); // one runs at 2 MHz, other runs at 4 MHz
	SN76489(config, "sn2", XTAL(16'000'000)/4).add_route(ALL_OUTPUTS, "mono", 0.50);
}

//******************************************************************************

/*  Palettes were obtained for each RGB PPU type by running a test program
    that fills the screen with a single color and analyzing the RGB output.
    The output of every PPU neatly quantizes to 8 levels, so the palettes
    are assumed to be stored using 3 bits per component in an internal ROM
    on each PPU.
*/

#define PALETTE_STANDARD(_tag)    \
	ROM_REGION( 0xc0, _tag, 0 )    \
	ROM_LOAD( "rp2c0x.pal", 0x00, 0xc0, CRC(48de65dc) SHA1(d10acafc8da9ff479c270ec01180cca61efe62f5) )

#define PALETTE_2C04_0001(_tag)   \
	ROM_REGION( 0xc0, _tag, 0 )    \
	ROM_LOAD( "rp2c04-0001.pal", 0x00, 0xc0, CRC(a6293faa) SHA1(190a410a3d818e124b2b9d2ef9fb3da003bb5d4c) )

#define PALETTE_2C04_0002(_tag)   \
	ROM_REGION( 0xc0, _tag, 0 )    \
	ROM_LOAD( "rp2c04-0002.pal", 0x00, 0xc0, CRC(fd19ae5e) SHA1(8ed14347c5a0b1a8a4d6365d6727e0951a00131c) )

#define PALETTE_2C04_0003(_tag)   \
	ROM_REGION( 0xc0, _tag, 0 )    \
	ROM_LOAD( "rp2c04-0003.pal", 0x00, 0xc0, CRC(fd6c578b) SHA1(653182ce0cbaff66a8fc5788e32cc088b6735f2e) )

#define PALETTE_2C04_0004(_tag)   \
	ROM_REGION( 0xc0, _tag, 0 )    \
	ROM_LOAD( "rp2c04-0004.pal", 0x00, 0xc0, CRC(0c2e8e4d) SHA1(0f9090225eb1f08ae5072d40af3e95547cbce05f) )


/*  correct label format. revision number is stamped(red), other letters are printed(black)
    game code in line 1, ROM position in line 2
    i.e.
    MDS-DH3
    1B or 6B e
*/


/* Notes for Super Mario:

mds-sm4-4__1cor6c_e.1c or 6c CRC32 0x5E3FB550 verified on 3 PCBs / dumped sets
(__suprmrioa).1c or 6c       CRC32 0x0011FC5A differs by 2 bytes: 0x1634 = 0x0D (vs 0x11) & 0x163B = 0x11 (vs 0x14)
                     Each change is part of a LDA #$ statement IE: A9 0D  LDA #$0D (vs A9 11  LDA #$11)
                     It's unknown if it's an official alt version or hack.

These 2 bytes affect timer speed, making 'suprmrioa' harder.
*/

ROM_START( suprmrio ) // Vs. Super Mario Bros. (Set E Rev 4)
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-sm4-4__1dor6d_e.1d or 6d", 0x0000, 0x2000, CRC(be4d5436) SHA1(08162a7c987f1939d09bebdb676f596c86abf465) )
	ROM_LOAD( "mds-sm4-4__1cor6c_e.1c or 6c", 0x2000, 0x2000, CRC(5e3fb550) SHA1(de4494e4dd52f7f7b04cf1d9019fd89fb90eaca9) )
	ROM_LOAD( "mds-sm4-4__1bor6b_e.1b or 6b", 0x4000, 0x2000, CRC(b1b87893) SHA1(8563ceaca664cf4495ef1020c07179ca7e4af9f3) )
	ROM_LOAD( "mds-sm4-4__1aor6a_e.1a or 6a", 0x6000, 0x2000, CRC(1abf053c) SHA1(f17db88ce0c9bf1ed88dc16b9650f11d10835cec) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-sm4-4__2bor8b_e.2b or 8b", 0x0000, 0x2000, CRC(42418d40) SHA1(22ab61589742cfa4cc6856f7205d7b4b8310bc4d) )
	ROM_LOAD( "mds-sm4-4__2aor8a_e.2a or 8a", 0x2000, 0x2000, CRC(15506b86) SHA1(69ecf7a3cc8bf719c1581ec7c0d68798817d416f) )

	PALETTE_2C04_0004("ppu1:palette")
ROM_END

ROM_START( suprmrioa ) // Vs. Super Mario Bros. (Set unknown, possibly operator hack of E rev 4 to make the game harder)
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-sm4-4__1dor6d_e.1d or 6d", 0x0000, 0x2000, CRC(be4d5436) SHA1(08162a7c987f1939d09bebdb676f596c86abf465) )
	ROM_LOAD( "__suprmrioa,.1c or 6c",        0x2000, 0x2000, CRC(0011fc5a) SHA1(5c2c49938a12affc03e64e5bdab307998be20020) ) // Need to verify correct label
	ROM_LOAD( "mds-sm4-4__1bor6b_e.1b or 6b", 0x4000, 0x2000, CRC(b1b87893) SHA1(8563ceaca664cf4495ef1020c07179ca7e4af9f3) )
	ROM_LOAD( "mds-sm4-4__1aor6a_e.1a or 6a", 0x6000, 0x2000, CRC(1abf053c) SHA1(f17db88ce0c9bf1ed88dc16b9650f11d10835cec) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-sm4-4__2bor8b_e.2b or 8b", 0x0000, 0x2000, CRC(42418d40) SHA1(22ab61589742cfa4cc6856f7205d7b4b8310bc4d) )
	ROM_LOAD( "mds-sm4-4__2aor8a_e.2a or 8a", 0x2000, 0x2000, CRC(15506b86) SHA1(69ecf7a3cc8bf719c1581ec7c0d68798817d416f) )

	PALETTE_2C04_0004("ppu1:palette")
ROM_END

ROM_START( suprmriobl )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 6502 memory
	ROM_LOAD( "4.bin",  0x8000, 0x8000, CRC(6f857416) SHA1(05e2df8ac01a03bf09b73e34c30aaf5bf4715809) )

	ROM_REGION( 0x10000, "sub", 0 ) // Z80 memory
	ROM_LOAD( "1.bin",  0x0000, 0x2000, CRC(9e3557f2) SHA1(11a0de2c0154f7ac120d9774cb5d1051e0156822) )

	ROM_REGION( 0x10000, "unk", 0 ) // Table used by clone PPU for sprite flipping, etc.
	ROM_LOAD( "3.bin",  0x0000, 0x8000, CRC(67a467f9) SHA1(61cd1db7cd52faa31153b89f6b98c9b78bf4ca4f) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "2.bin",  0x0000, 0x2000, CRC(42418d40) SHA1(22ab61589742cfa4cc6856f7205d7b4b8310bc4d) )
	ROM_LOAD( "5.bin",  0x2000, 0x2000, CRC(15506b86) SHA1(69ecf7a3cc8bf719c1581ec7c0d68798817d416f) )

	// this set has some extra files compared to "suprmriobl2", they probably exist on that pcb too though
	ROM_REGION( 0x100, "ppu1:palette", 0 )
	ROM_LOAD_NIB_HIGH( "prom6301.1",  0x000, 0x100, CRC(a31dc330) SHA1(b652003f7e252bac3bdb19412839c2f03af7f8b8) )
	ROM_LOAD_NIB_LOW ( "prom6301.2",  0x000, 0x100, CRC(019c6141) SHA1(fdeda4dea6506807a3324fa941f0684208aa3b4b) )

	ROM_REGION( 0x4000, "pals", 0 )
	ROM_LOAD( "pal16l8.1",  0x000, 0x104, CRC(bd76fb53) SHA1(2d0634e8edb3289a103719466465e9777606086e) )
	ROM_LOAD( "pal16r6a.2", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r8.3",  0x000, 0x104, CRC(bd76fb53) SHA1(2d0634e8edb3289a103719466465e9777606086e) )
	ROM_LOAD( "pal16l8.4",  0x000, 0x104, CRC(6f6de82d) SHA1(3d59b222d25457b2f89b559409721db37d6a81d8) )
	ROM_LOAD( "pal16r6.5",  0x000, 0x104, CRC(ceff7c7c) SHA1(52fd344c591478469369cd0862d1facfe23e12fb) )
	ROM_LOAD( "pal16r8.6",  0x000, 0x104, CRC(bd76fb53) SHA1(2d0634e8edb3289a103719466465e9777606086e) )
	ROM_LOAD( "pal16r8a.7", 0x000, 0x104, CRC(bd76fb53) SHA1(2d0634e8edb3289a103719466465e9777606086e) )

	ROM_REGION(0x0100, "epld", 0)
	ROM_LOAD("ep1200.bin", 0x000, 0x100, NO_DUMP) // Not dumped
ROM_END

ROM_START( suprmriobl2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 6502 memory
	ROM_LOAD( "4-27256.bin",  0x8000, 0x8000, CRC(663b1753) SHA1(b0d2057c4545f2d6534cafb16086826c8ba49f5a) )

	ROM_REGION( 0x10000, "sub", 0 ) // Z80 memory
	ROM_LOAD( "1-2764.bin",  0x0000, 0x2000, CRC(95856e07) SHA1(c681cfdb656e687bc59080df56c9c38e13be4bb8) )

	ROM_REGION( 0x10000, "unk", 0 ) // Table used by clone PPU for sprite flipping, etc.
	ROM_LOAD( "3-27256.bin",  0x0000, 0x8000, CRC(67a467f9) SHA1(61cd1db7cd52faa31153b89f6b98c9b78bf4ca4f) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "2-2764.bin",  0x0000, 0x2000, CRC(42418d40) SHA1(22ab61589742cfa4cc6856f7205d7b4b8310bc4d) )
	ROM_LOAD( "5-2764.bin",  0x2000, 0x2000, CRC(15506b86) SHA1(69ecf7a3cc8bf719c1581ec7c0d68798817d416f) )

	ROM_REGION(0x100, "ppu1:palette", 0)
	ROM_LOAD_NIB_HIGH( "prom6301.1", 0x000, 0x100, BAD_DUMP CRC(a31dc330) SHA1(b652003f7e252bac3bdb19412839c2f03af7f8b8) ) // Not from this PCB
	ROM_LOAD_NIB_LOW ( "prom6301.2", 0x000, 0x100, BAD_DUMP CRC(019c6141) SHA1(fdeda4dea6506807a3324fa941f0684208aa3b4b) ) // Not from this PCB

	ROM_REGION(0x4000, "pals", 0)
	ROM_LOAD("pal16l8.1", 0x000, 0x104, BAD_DUMP CRC(bd76fb53) SHA1(2d0634e8edb3289a103719466465e9777606086e)) // Not from this PCB
	ROM_LOAD("pal16r6a.2", 0x000, 0x104, NO_DUMP)
	ROM_LOAD("pal16r8.3", 0x000, 0x104, BAD_DUMP CRC(bd76fb53) SHA1(2d0634e8edb3289a103719466465e9777606086e)) // Not from this PCB
	ROM_LOAD("pal16l8.4", 0x000, 0x104, BAD_DUMP CRC(6f6de82d) SHA1(3d59b222d25457b2f89b559409721db37d6a81d8)) // Not from this PCB
	ROM_LOAD("pal16r6.5", 0x000, 0x104, BAD_DUMP CRC(ceff7c7c) SHA1(52fd344c591478469369cd0862d1facfe23e12fb)) // Not from this PCB
	ROM_LOAD("pal16r8.6", 0x000, 0x104, BAD_DUMP CRC(bd76fb53) SHA1(2d0634e8edb3289a103719466465e9777606086e)) // Not from this PCB
	ROM_LOAD("pal16r8a.7", 0x000, 0x104, BAD_DUMP CRC(bd76fb53) SHA1(2d0634e8edb3289a103719466465e9777606086e)) // Not from this PCB

	ROM_REGION(0x0100, "epld", 0)
	ROM_LOAD("ep1200.bin", 0x000, 0x100, NO_DUMP) // Not dumped
ROM_END

ROM_START( skatekds )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-sm4-4__1dor6d_e.1d or 6d", 0x0000, 0x2000, CRC(be4d5436) SHA1(08162a7c987f1939d09bebdb676f596c86abf465) )
	ROM_LOAD( "mds-sm4-4__1cor6c_e.1c or 6c", 0x2000, 0x2000, CRC(5e3fb550) SHA1(de4494e4dd52f7f7b04cf1d9019fd89fb90eaca9) )
	ROM_LOAD( "mds-sm4-4__1bor6b_e.1b or 6b", 0x4000, 0x2000, CRC(b1b87893) SHA1(8563ceaca664cf4495ef1020c07179ca7e4af9f3) )
	ROM_LOAD( "mds-sm4-4__1aor6a_e.1a or 6a", 0x6000, 0x2000, CRC(1abf053c) SHA1(f17db88ce0c9bf1ed88dc16b9650f11d10835cec) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "__skatekds,.2b",  0x0000, 0x2000,CRC(f3980303) SHA1(b9a25c906d1861c89e2e40e878a34d318daf6619) )
	ROM_LOAD( "__skatekds,.2a",  0x2000, 0x2000,CRC(7a0ab7eb) SHA1(b6c32791481fafddc8504adb4eaed30a2fb3a03e) )

	PALETTE_2C04_0004("ppu1:palette")
ROM_END

ROM_START( iceclimb )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-ic4-4 b.6d",  0x0000, 0x2000, CRC(0ea5f9cb) SHA1(3ba6228ac8011371fc36ce9dde4fc158a81a99a2) )
	ROM_LOAD( "mds-ic4-4 b.6c",  0x2000, 0x2000, CRC(51fe438e) SHA1(f40812d4275dabaac6f9539e1300c08d07992654) )
	ROM_LOAD( "mds-ic4-4 b-1.6b",0x4000, 0x2000, CRC(a8afdc62) SHA1(f798da6c107926790026d4a4d384961dbff2380e) )
	ROM_LOAD( "mds-ic4-4 b.6a",  0x6000, 0x2000, CRC(96505d4d) SHA1(0fb913853decebec1d5d15ee5adc8027cd66f016) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-ic4-4 b.8b",  0x0000, 0x2000, CRC(331460b4) SHA1(4cf94d711cdb5715d14f1ab3cadec245e0adfb1e) ) // Matches Ice Climber Dual screen rev A rom
	ROM_LOAD( "mds-ic4-4 b.8a",  0x2000, 0x2000, CRC(4ec44fb3) SHA1(676e0ab574dec08df562c6f278e8a9cc7c8afa41) ) // Matches Ice Climber Dual screen rev A rom

	PALETTE_2C04_0004("ppu1:palette")
ROM_END

ROM_START( iceclimba ) // Version A?
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-ic4-4.1d",  0x0000, 0x2000, CRC(65e21765) SHA1(900f1efe5e8005ee8cdccbf5039914dfe466aa3d) ) // Need to verify correct label
	ROM_LOAD( "mds-ic4-4.1c",  0x2000, 0x2000, CRC(a7909c51) SHA1(04708a9e429cbddab6988ff7b3ec5aa0109f6228) ) // Need to verify correct label
	ROM_LOAD( "mds-ic4-4.1b",  0x4000, 0x2000, CRC(7fb3cc21) SHA1(bed673211f2251d4112ea41c4a1f917fee32d93c) ) // Need to verify correct label
	ROM_LOAD( "mds-ic4-4.1a",  0x6000, 0x2000, CRC(bf196bf7) SHA1(7d7b34894caab41ac51ca9c89d09e72053798784) ) // Need to verify correct label

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-ic4-4 b.8b",  0x0000, 0x2000, CRC(331460b4) SHA1(4cf94d711cdb5715d14f1ab3cadec245e0adfb1e) ) // Matches Ice Climber Dual screen rev A rom
	ROM_LOAD( "mds-ic4-4 b.8a",  0x2000, 0x2000, CRC(4ec44fb3) SHA1(676e0ab574dec08df562c6f278e8a9cc7c8afa41) ) // Matches Ice Climber Dual screen rev A rom

	PALETTE_2C04_0004("ppu1:palette")
ROM_END

// Gun games
ROM_START( duckhunt ) // Vs. Duck Hunt (Set E)
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-dh3 e.1d or 6d", 0x0000, 0x2000, CRC(3f51f0ed) SHA1(984d8a5cecddde776ffd4f718ee0ca7a9959228b) )
	ROM_LOAD( "mds-dh3 e.1c or 6c", 0x2000, 0x2000, CRC(8bc7376c) SHA1(d90d663c5e5b6d5247089c8ba618912305049b19) )
	ROM_LOAD( "mds-dh3 e.1b or 6b", 0x4000, 0x2000, CRC(a042b6e1) SHA1(df571c31a6a52df56869eda0621f7615a625e66d) )
	ROM_LOAD( "mds-dh3 e.1a or 6a", 0x6000, 0x2000, CRC(1906e3ab) SHA1(bff68829a96e2d251dd12129f84bdf1dbdf61d06) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-dh3 e.2b or 8b", 0x0000, 0x2000, CRC(0c52ec28) SHA1(c8fb6a5d4c13a7075d313326e2da9ce88780a88d) )
	ROM_LOAD( "mds-dh3 e.2a or 8a", 0x2000, 0x2000, CRC(3d238df3) SHA1(e868ef3d5357ef5294e4faeecc9dbf801c5253e8) )

	PALETTE_STANDARD("ppu1:palette")
ROM_END

ROM_START( hogalley )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-ha4-1 e-1.1d or 6d",  0x0000, 0x2000, CRC(2089e166) SHA1(7db09b5b6bcd87589bed89a5fc1a4b772155a0f3) )
	ROM_LOAD( "mds-ha4-1 e-1.1c or 6c",  0x2000, 0x2000, CRC(a85934ae) SHA1(f26af4f60a4072c45e900dff7f74d9907bc2e1e0) )
	ROM_LOAD( "mds-ha4-1 e-1.1b or 6b",  0x4000, 0x2000, CRC(718e25b3) SHA1(2710827931d3cd55984c3107c3b8e0f691965eaa) )
	ROM_LOAD( "mds-ha4-1 e-1.1a or 6a",  0x6000, 0x2000, CRC(f9526852) SHA1(244c6a12801d4aa774a416f7c3dd8465d01dbca2) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-ha4-1 e-1.2b or 8b",  0x0000, 0x2000, CRC(fc5a91ad) SHA1(7ce6c64d81a9626d0b34bdc0a2a28fee457ebcb1) )
	// there is another dump of mds-ha4-1 e-1.2b or 8b ( CRC 0x7623e954 ) where 0xFE1 = 04,  the dump we use is probably the correct one
	ROM_LOAD( "mds-ha4-1 e-1.2a or 8a",  0x2000, 0x2000, CRC(78c842b6) SHA1(39f2a7fc1f1cbe2378a369e45b5cbb05057db3f0) )

	PALETTE_2C04_0001("ppu1:palette")
ROM_END

/* From Gumshoe readme:

NOTE:  There is a small board modification which consists of 2 extra jumpers added.
They appear to be:

pin 26 of 6D to pin 1, 74ls04 at 5E.
pin 26 of 1D to pin 12, 74s32 at 4b.

The 27128 installed in either 1D or 6D can then be accessed in 8K banks (pin 26 is A13).
*/

ROM_START( vsgshoe )
	ROM_REGION( 0xa000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-gm5.1d",  0x0000, 0x4000, CRC(063b342f) SHA1(66f69de27db5b08969f9250d0a6760e7311bd9bf) ) // 2 banks mapped at 0x8000
	ROM_LOAD( "mds-gm5.1c",  0x4000, 0x2000, CRC(e1b7915e) SHA1(ed0fdf74b05a3ccd1645c4f580436fd439f81dea) )
	ROM_LOAD( "mds-gm5.1b",  0x6000, 0x2000, CRC(5b73aa3c) SHA1(4069a6139091fbff48758953bd894808a8356d46) )
	ROM_LOAD( "mds-gm5.1a",  0x8000, 0x2000, CRC(70e606bc) SHA1(8207ded20cb9109d605ce73deb722de3514ed9bf) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-gm5.2b",  0x0000, 0x2000, CRC(192c3360) SHA1(5ddbe007d8bc693a0b7c92f33e6ed6b27dc1c08e) )
	ROM_LOAD( "mds-gm5.2a",  0x2000, 0x2000, CRC(823dd178) SHA1(77578a48ded0c244d1ae30aafaa9259b7dd0dfc4) )

	PALETTE_STANDARD("ppu1:palette")
ROM_END

ROM_START( vsfdf )
	ROM_REGION( 0x20000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "prg2", 0x00000, 0x10000, CRC(3bce8f0f) SHA1(5a9b91bae4b28c1df54fb290efdec4805f4f217e) )
	ROM_LOAD( "prg1", 0x10000, 0x10000, CRC(c74499ce) SHA1(14f50d4d11c363e761a6472a6e57a5e5a6dab9ce) )

	ROM_REGION( 0x10000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "cha2.1",  0x00000, 0x10000, CRC(a2f88df0) SHA1(10ef432d3132b01a1fcb38d8f521edd2a029ac5e) )

	PALETTE_2C04_0001("ppu1:palette")
ROM_END

ROM_START( goonies ) // Vs. The Goonies (Set E)
	ROM_REGION( 0x10000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-gn prg e.u7", 0x00000, 0x10000, CRC(1e438d52) SHA1(ac187904c125e56a71acff979e53f3398a05c075) )

	ROM_REGION( 0x10000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-gn chr e.u4", 0x0000, 0x10000, CRC(4c4b61b0) SHA1(7221c2499531e591a5a99e2cb339ae3a76b662c2) )

	PALETTE_2C04_0003("ppu1:palette")
ROM_END

ROM_START( vsgradus )
	ROM_REGION( 0x10000, "prg", 0  ) // 6502 memory
	ROM_LOAD( "mds-gr__prg_e.u7",  0x00000, 0x10000, CRC(d99a2087) SHA1(b26efe78798453a903921723f3c9ac69f579b7d2) )

	ROM_REGION( 0x10000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-gr__chr_e.u4",  0x0000, 0x10000, CRC(23cf2fc3) SHA1(0a3f48aec529b92abc261952e632af7ff766b1ef) )

	PALETTE_2C04_0001("ppu1:palette")
ROM_END

ROM_START( btlecity )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "bc.1d",  0x0000, 0x2000, CRC(6aa87037) SHA1(f3313700955498800a3d59c523ba2a4e0cf443bc) )
	ROM_LOAD( "bc.1c",  0x2000, 0x2000, CRC(bdb317db) SHA1(a8b3e8deb1e625d764aaffe86a513bc7ede51a46) )
	ROM_LOAD( "bc.1b",  0x4000, 0x2000, CRC(1a0088b8) SHA1(ba90d8178a23caedbf0e7188256b7cbfebf35eeb) )
	ROM_LOAD( "bc.1a",  0x6000, 0x2000, CRC(86307c89) SHA1(e4e73e4dcaa5c2374d7e3844d6d3fdb192ac9674) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "bc.2b",  0x0000, 0x2000, CRC(634f68bd) SHA1(db1a18083667fdaf6cdd9ed7666bec6bf6e39f29) )
	ROM_LOAD( "bc.2a",  0x2000, 0x2000, CRC(a9b49a05) SHA1(c14706e6a5524f81e79c101e32deef9f3d60de3f) )

	// also compatible with 2C04-0002, 2C04-0003, 2C04-0004 via dipswitches
	PALETTE_2C04_0001("ppu1:palette")
ROM_END

ROM_START( cluclu )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "cl.6d",  0x0000, 0x2000, CRC(1e9f97c9) SHA1(47d847632145d8160d006f014f9e0a7483783d0e) )
	ROM_LOAD( "cl.6c",  0x2000, 0x2000, CRC(e8b843a7) SHA1(03827b31d47d2a8a132bf9944fee724c6c1c6d2e) )
	ROM_LOAD( "cl.6b",  0x4000, 0x2000, CRC(418ee9ea) SHA1(a68e8a97899e850884cb9484fe539b86c419f10f) )
	ROM_LOAD( "cl.6a",  0x6000, 0x2000, CRC(5e8a8457) SHA1(8e53de132db2e1299bd8f2329758f3ccb096584a) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "cl.8b",  0x0000, 0x2000, CRC(960d9a6c) SHA1(2569d59fd880cfc2eb4638294d1429ba749f5dcb) )
	ROM_LOAD( "cl.8a",  0x2000, 0x2000, CRC(e3139791) SHA1(33d9e6d2a3233ee311c2cef2d0a425ded2cf3b0f) )

	PALETTE_2C04_0004("ppu1:palette")
ROM_END


ROM_START( excitebk ) // EB4-3 = Excitebike, Palette 3, revision 'E'
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-eb4-3e.1d",  0x0000, 0x2000, CRC(f58a392e) SHA1(ca721e47e5dbe72d6f231d9b20b1ca33304c5370) )
	ROM_LOAD( "mds-eb4-3e.1c",  0x2000, 0x2000, CRC(6ae01102) SHA1(f3f49644f7e2887e14655e8934c7c75d4b92968e) )
	ROM_LOAD( "mds-eb4-3e.1b",  0x4000, 0x2000, CRC(fbc53465) SHA1(81c9ebac895dfbc5c451b4b32e5990aba3734714) )
	ROM_LOAD( "mds-eb4-3e.1a",  0x6000, 0x2000, CRC(e2902b4e) SHA1(f559517c735847e2ad622418b8fa525d9c9adac3) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-eb4-3e.2b",  0x0000, 0x2000, CRC(80be1f50) SHA1(d8544b9a0a9d8719ab601fa9c68c4305385b14c7) )
	ROM_LOAD( "mds-eb4-3e.2a",  0x2000, 0x2000, CRC(001e8e0b) SHA1(c4c2a11ade7788bac9dd61ea8de319c4111d2592) )

	PALETTE_2C04_0003("ppu1:palette")
ROM_END

ROM_START( excitebko ) // EB4-3 = Excitebike, Palette 3, unknown revision
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-eb4-3.1d",  0x0000, 0x2000, CRC(7e54df1d) SHA1(38d878041976386e8608c73133040b18d0e4b9cd) ) // Need to verify correct label
	ROM_LOAD( "mds-eb4-3.1c",  0x2000, 0x2000, CRC(89baae91) SHA1(6aebf13c415e3246edf7daa847533b7e3ae0425f) ) // Need to verify correct label
	ROM_LOAD( "mds-eb4-3.1b",  0x4000, 0x2000, CRC(4c0c2098) SHA1(078f24ce02f5fb91d7ed7fa59aec8efbec38aed1) ) // Need to verify correct label
	ROM_LOAD( "mds-eb4-3.1a",  0x6000, 0x2000, CRC(b9ab7110) SHA1(89e3bd5f42b5b5e869ee46afe4f25a1a17d3814d) ) // Need to verify correct label

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-eb4-3.2b",  0x0000, 0x2000, CRC(80be1f50) SHA1(d8544b9a0a9d8719ab601fa9c68c4305385b14c7) ) // Need to verify correct label
	ROM_LOAD( "mds-eb4-3.2a",  0x2000, 0x2000, CRC(a9b49a05) SHA1(c14706e6a5524f81e79c101e32deef9f3d60de3f) ) // Need to verify correct label

	PALETTE_2C04_0003("ppu1:palette")
ROM_END

ROM_START( excitebkj ) // EB4-4 A = Excitebike, Palette 4, rev A
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-eb4-4 a.6d",  0x0000, 0x2000, CRC(6aa87037) SHA1(f3313700955498800a3d59c523ba2a4e0cf443bc) )
	ROM_LOAD( "mds-eb4-4 a.6c",  0x2000, 0x2000, CRC(bdb317db) SHA1(a8b3e8deb1e625d764aaffe86a513bc7ede51a46) )
	ROM_LOAD( "mds-eb4-4 a.6b",  0x4000, 0x2000, CRC(d1afe2dd) SHA1(ef0f44d98464b7dab7c51be4379242f7a4e4fcdd) )
	ROM_LOAD( "mds-eb4-4 a.6a",  0x6000, 0x2000, CRC(46711d0e) SHA1(6ce2f395b3f407671a87c6e1133ab63a637022f2) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-eb4-4 a.8b",  0x0000, 0x2000, CRC(62a76c52) SHA1(7ebd0dac976abe8636f4f75a3b2a473d7a54934d) )
	ROM_LOAD( "mds-eb4-4 a.8a",  0x2000, 0x2000, CRC(a9b49a05) SHA1(c14706e6a5524f81e79c101e32deef9f3d60de3f) )

	PALETTE_2C04_0004("ppu1:palette")
ROM_END

ROM_START( jajamaru )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "10.bin", 0x0000, 0x2000, CRC(16af1704) SHA1(ebcf9ad06e302c51ee4432631a6b0fb85a9630ed) )
	ROM_LOAD( "9.bin",  0x2000, 0x2000, CRC(db7d1814) SHA1(6a0c9cf97006a8a41dc2f025a5f8acbb798dec60) )
	ROM_LOAD( "8.bin",  0x4000, 0x2000, CRC(ce263271) SHA1(1e5e2a9e0dcebeccd7df59491ca0bc5ac4d0d42b) )
	ROM_LOAD( "7.bin",  0x6000, 0x2000, CRC(a406d0e4) SHA1(1f67b58bacb145a3ff8b8380b44cd60251051c71) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "12.bin",  0x0000, 0x2000, CRC(c91d536a) SHA1(8cb4b0819652df484553b9dd1f82391d51c90fcc) )
	ROM_LOAD( "11.bin",  0x2000, 0x2000, CRC(f0034c04) SHA1(402dcf6ad443baeee3038ecab12db008a1ad2787) )

	PALETTE_STANDARD("ppu1:palette")
ROM_END

ROM_START( smgolf )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-gf4-2 f.1d or 6d",  0x0000, 0x2000, CRC(4a723087) SHA1(87dc063d295f55871598a1e1eb4e62ce298b2f0c) )
	ROM_LOAD( "mds-gf4-2 f.1c or 6c",  0x2000, 0x2000, CRC(2debda63) SHA1(33b42eb5641ec947b2f2dcbc632ee6c81fa2ffe3) )
	ROM_LOAD( "mds-gf4-2 f.1b or 6b",  0x4000, 0x2000, CRC(6783652f) SHA1(7165ee59d3787cb56eed4791351da07f4bcc68ed) )
	ROM_LOAD( "mds-gf4-2 f.1a or 6a",  0x6000, 0x2000, CRC(bfc17263) SHA1(9e3b46fe08be893935138247ed3168d19d55312e) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-gf4-2 f.2b or 8b",  0x0000, 0x2000, CRC(2782a3e5) SHA1(4e57aab58fb250da951a9aecd21d6aa79e697bcb) )
	ROM_LOAD( "mds-gf4-2 f.2a or 8a",  0x2000, 0x2000, CRC(6e93fdef) SHA1(44f46421adabbc40135c681592cb5226b7c9012a) )

	PALETTE_2C04_0002("ppu1:palette")
ROM_END

ROM_START( smgolfb ) // Set ID should be something like MDS-GF4-2 xx
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-gf4-2.1d",  0x0000, 0x2000, CRC(a3e286d3) SHA1(ee7539a46e0e062ffd63d84e8b83de29b860a501) ) // Need to verify correct label
	ROM_LOAD( "mds-gf4-2.1c",  0x2000, 0x2000, CRC(e477e48b) SHA1(2ebcc548ac8defc521860d2d2f585be0eee6620e) ) // Need to verify correct label
	ROM_LOAD( "mds-gf4-2.1b",  0x4000, 0x2000, CRC(7d80b511) SHA1(52aa7e798ff8d933b023bcade81a39f7e27d02c5) ) // Need to verify correct label
	ROM_LOAD( "mds-gf4-2.1a",  0x6000, 0x2000, CRC(7b767da6) SHA1(0f0f3a24b844265c304b10016f33e91b323a9a98) ) // Need to verify correct label

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-gf4-2 f.2b or 8b",  0x0000, 0x2000, CRC(2782a3e5) SHA1(4e57aab58fb250da951a9aecd21d6aa79e697bcb) ) // Need to verify correct label
	ROM_LOAD( "mds-gf4-2 f.2a or 8a",  0x2000, 0x2000, CRC(6e93fdef) SHA1(44f46421adabbc40135c681592cb5226b7c9012a) ) // Need to verify correct label

	PALETTE_2C04_0002("ppu1:palette")
ROM_END

ROM_START( smgolfj )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-gf3 b.6d",  0x0000, 0x2000, CRC(8ce375b6) SHA1(f787f5ebe584cc95428b63660cd41e2b3df6ddf2) )
	ROM_LOAD( "mds-gf3 b.6c",  0x2000, 0x2000, CRC(50a938d3) SHA1(5f5c5e50024fe113240f1b0b3b6d38cbf9130133) )
	ROM_LOAD( "mds-gf3 b.6b",  0x4000, 0x2000, CRC(7dc39f1f) SHA1(12ff2f0ec7418754f9b6e600746e15f345e3ddaa) )
	ROM_LOAD( "mds-gf3 b.6a",  0x6000, 0x2000, CRC(9b8a2106) SHA1(008ab9098f9ce564bcb4beb17285c2bc18b529ff) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-gf3 b.8b",  0x0000, 0x2000, CRC(7ef68029) SHA1(a639e463fd0abfb1bff0dd17aa0c9f70a72ee139) )
	ROM_LOAD( "mds-gf3 b.8a",  0x2000, 0x2000, CRC(f2285878) SHA1(e0d34161a1879975f51c12222cf366228170b0e3) )

	PALETTE_STANDARD("ppu1:palette")
ROM_END

ROM_START( ladygolfe ) // Vs. Stroke & Match Golf (Ladies Version, set E)
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-lg4 e.1d or 6d", 0x0000, 0x2000, CRC(408ea247) SHA1(48cbcf9b87e522b20599bcf2b61df81bfd55db7b) )
	ROM_LOAD( "mds-lg4 e.1c or 6c", 0x2000, 0x2000, CRC(75214cf3) SHA1(8024a2a89573700250c1c12ab8f59f4848f87f78) )
	ROM_LOAD( "mds-lg4 e.1b or 6b", 0x4000, 0x2000, CRC(67f40126) SHA1(891c2bc2b6e1ee1d58baea22714133cae6e38b8d) )
	ROM_LOAD( "mds-lg4 e.1a or 6a", 0x6000, 0x2000, CRC(44393845) SHA1(5c36dcb115d4233a1e03faa28e95d2662953ca91) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-lg4 e.2b or 8b", 0x0000, 0x2000, CRC(95618947) SHA1(e8f09bffa3fa1a1cac8fa25df9fba962951c1fb3) )
	ROM_LOAD( "mds-lg4 e.2a or 8a", 0x2000, 0x2000, CRC(d07407b1) SHA1(b998b46fe83e76fac3d7b71495d1da8580a731f9) )

	PALETTE_2C04_0002("ppu1:palette")
ROM_END

ROM_START( ladygolf )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "lg-1d",  0x0000, 0x2000, CRC(8b2ab436) SHA1(145a75f30f1fab5b1babf01ada9ed23f59c2c18d) ) // Need to verify correct label
	ROM_LOAD( "lg-1c",  0x2000, 0x2000, CRC(bda6b432) SHA1(c8322f07df0adbd70cb49f2284b046478a3a57c1) ) // Need to verify correct label
	ROM_LOAD( "lg-1b",  0x4000, 0x2000, CRC(dcdd8220) SHA1(563028f8db9ad221d8ac8f8096b4587b822eedb7) ) // Need to verify correct label
	ROM_LOAD( "lg-1a",  0x6000, 0x2000, CRC(26a3cb3b) SHA1(00131637eb76154c4f04eb54707e0e7b453d4580) ) // Need to verify correct label

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-lg4 e.2b or 8b", 0x0000, 0x2000, CRC(95618947) SHA1(e8f09bffa3fa1a1cac8fa25df9fba962951c1fb3) ) // Need to verify correct label
	ROM_LOAD( "mds-lg4 e.2a or 8a", 0x2000, 0x2000, CRC(d07407b1) SHA1(b998b46fe83e76fac3d7b71495d1da8580a731f9) ) // Need to verify correct label

	PALETTE_2C04_0002("ppu1:palette")
ROM_END

ROM_START( nvs_machrider ) // Set ID should be something like MDS-MR4-2 xx
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mr-1d",  0x0000, 0x2000, CRC(379c44b9) SHA1(7b148ba7f7eea64509733f94b4eaafe5bfcf3527) ) // Need to verify correct label
	ROM_LOAD( "mr-1c",  0x2000, 0x2000, CRC(cb864802) SHA1(65f06a8eaca3347432f3f2f673a24692415d869f) ) // Need to verify correct label
	ROM_LOAD( "mr-1b",  0x4000, 0x2000, CRC(5547261f) SHA1(aedb7ab1ef0cd32f325ec9fc948ca1e21a78aa7a) ) // Need to verify correct label
	ROM_LOAD( "mr-1a",  0x6000, 0x2000, CRC(e3e3900d) SHA1(c66807ca372d2e5ac11030fdf3d30e30617d4e72) ) // Need to verify correct label

	ROM_REGION( 0x4000, "gfx1", 0) // PPU memory
	ROM_LOAD( "mr-2b",  0x0000, 0x2000, CRC(33a2b41a) SHA1(671f37bce742e63250296e62c143f8a82f860b04) ) // Need to verify correct label
	ROM_LOAD( "mr-2a",  0x2000, 0x2000, CRC(685899d8) SHA1(02b6a9bc21367c481d0091fa8a8f2d1b841244bf) ) // Need to verify correct label

	PALETTE_2C04_0002("ppu1:palette")
ROM_END

ROM_START( nvs_machridera )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-mr4-1 a.1d or 6d",  0x0000, 0x2000, CRC(ab7e0594) SHA1(fc5982a93791608a20e5ec9e3a4b71d702bda354) )
	ROM_LOAD( "mds-mr4-1 a.1c or 6c",  0x2000, 0x2000, CRC(d4a341c3) SHA1(c799e40d0ebd1447032d8767fb2caeee6b33f31a) )
	ROM_LOAD( "mds-mr4-1 a.1b or 6b",  0x4000, 0x2000, CRC(cbdcfece) SHA1(91f3a0e1e91bdbb61721e9777009299f7e8efa96) )
	ROM_LOAD( "mds-mr4-1 a.1a or 6a",  0x6000, 0x2000, CRC(e5b1e350) SHA1(ab30f84597cbf470a02a2d083587cdc589a29a3c) )

	ROM_REGION( 0x4000, "gfx1", 0) // PPU memory
	ROM_LOAD( "mds-mr4-1 a.2b or 8b",  0x0000, 0x2000, CRC(59867e36) SHA1(2b5546aa9f140277d611d6d5516b1343e5e672a0) )
	ROM_LOAD( "mds-mr4-1 a.2a or 8a",  0x2000, 0x2000, CRC(ccfedc5a) SHA1(3d6321681fbe256d7c71037205d45d22fc264569) )

	PALETTE_2C04_0001("ppu1:palette")
ROM_END

ROM_START( vspinbal ) // E-1 Set; used Nintendo labels, e and 1 written in red marker
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-pn4_1__6d_e.6d",  0x0000, 0x2000, CRC(69fc575e) SHA1(d5165959c3569f5ebccd03d2cad4714f9240cc4c) )
	ROM_LOAD( "mds-pn4_1__6c_e.6c",  0x2000, 0x2000, CRC(fa9472d2) SHA1(d20ffb156bea1f474ad7d9776e217cb05048f00f) )
	ROM_LOAD( "mds-pn4_1__6b_e.6b",  0x4000, 0x2000, CRC(f57d89c5) SHA1(03f3a27d806c61fef13b0d8b2d8b9a15ee968e80) )
	ROM_LOAD( "mds-pn4_1__6a_e.6a",  0x6000, 0x2000, CRC(640c4741) SHA1(930bed577bfc75b03d064dc0ef523c45186fc3c4) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-pn4_1__8b_e.8b",  0x0000, 0x2000, CRC(8822ee9e) SHA1(950113952e6d356e45e03479ba5dd5a8cb131609) )
	ROM_LOAD( "mds-pn4_1__8a_e.8a",  0x2000, 0x2000, CRC(cbe98a28) SHA1(c00c5f15a33611bfe3ad420b93b1cc2cae011c3e) )

	PALETTE_2C04_0001("ppu1:palette")
ROM_END

ROM_START( vspinbalj )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-pn3 b.6d",  0x0000, 0x2000, CRC(fd50c42e) SHA1(4a3ea9e85b60caf8b6975fd2798bc59e86ec257f) )
	ROM_LOAD( "mds-pn3 b.6c",  0x2000, 0x2000, CRC(59beb9e5) SHA1(682b31dfbdf1ee44fd5d5d63169ab35409e93546) )
	ROM_LOAD( "mds-pn3 b.6b",  0x4000, 0x2000, CRC(ce7f47ce) SHA1(c548c1b94d3807b4968629c7fdce8aae3a61e6e0) )
	ROM_LOAD( "mds-pn3 b.6a",  0x6000, 0x2000, CRC(5685e2ee) SHA1(a38fbf25c93dfc73658d3837b2b6397736e8d2f2) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-pn3 b.8b",  0x0000, 0x2000, CRC(1e3fec3e) SHA1(aef18cee358af202ec48c1c36986e42e134466b1) )
	ROM_LOAD( "mds-pn3 b.8a",  0x2000, 0x2000, CRC(6f963a65) SHA1(af69564b51aa42ef0815c952e0d0d0d928651685) )

	PALETTE_STANDARD("ppu1:palette")
ROM_END

ROM_START( vsslalom )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-sl4.1d",  0x0000, 0x2000, CRC(6240a07d) SHA1(c9a3743a1caaa417c3828365a4c7a75272c20146) )
	ROM_LOAD( "mds-sl4.1c",  0x2000, 0x2000, CRC(27c355e4) SHA1(ba55258396a17858e136fe45332f6cc13a46b072) )
	ROM_LOAD( "mds-sl4.1b",  0x4000, 0x2000, CRC(d4825fbf) SHA1(5e7fcfa1999c52f94be28c693acffc6e5d434674) )
	ROM_LOAD( "mds-sl4.1a",  0x6000, 0x2000, CRC(82333f80) SHA1(fa85f8a481f3847b33fd9df005df4fde59080bce) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-sl4.2a",  0x0000, 0x2000, CRC(977bb126) SHA1(9b12cd37246237c24a8077c6184a2f71d342ac47) )

	PALETTE_2C04_0002("ppu1:palette")
ROM_END

ROM_START( vssoccer )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-sc4-2 a.1d or 6d",  0x0000, 0x2000, CRC(722fc5bc) SHA1(1ac06ade7412c476b834190034d94420863fe301) )
	ROM_LOAD( "mds-sc4-2 a.1c or 6c",  0x2000, 0x2000, CRC(6407cb08) SHA1(95e79633dfa4468e2a2af6d0a2cf5a23e4f23eb1) )
	ROM_LOAD( "mds-sc4-2 a.1b or 6b",  0x4000, 0x2000, CRC(d156b824) SHA1(2b7916805a0f09c10a334ba3c13481ce1da82be1) )
	ROM_LOAD( "mds-sc4-2 a.1a or 6a",  0x6000, 0x2000, CRC(2d75ca32) SHA1(9509bd984ca1ca6fce83874a6e13e75cdf91726b) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-sc4-2 a.2b or 8b",  0x0000, 0x2000, CRC(307b19ab) SHA1(b35ef4c2cf071db77cec1b4529b43a20cfcce172) )
	ROM_LOAD( "mds-sc4-2 a.2a or 8a",  0x2000, 0x2000, CRC(7263613a) SHA1(aa5673b57833d1f32c2cb0230a809397ec6103b4) )

	PALETTE_2C04_0002("ppu1:palette")
ROM_END

ROM_START( vssoccera ) // Set ID should be something like MDS-SC4-3 xx
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-sc4-3.1d",  0x0000, 0x2000, CRC(0ac52145) SHA1(148d9850cd80fb64e28f478891c16dac71e67e96) ) // Need to verify correct label
	ROM_LOAD( "mds-sc4-3.1c",  0x2000, 0x2000, CRC(f132e794) SHA1(f289f5acec7e2a62fc569a401e7ab5200df302f5) ) // Need to verify correct label
	ROM_LOAD( "mds-sc4-3.1b",  0x4000, 0x2000, CRC(26bb7325) SHA1(80e97a36c364a07cf9862202454651fb2872cd51) ) // Need to verify correct label
	ROM_LOAD( "mds-sc4-3.1a",  0x6000, 0x2000, CRC(e731635a) SHA1(8089bc49a0115225d26c4cbaaf08431376eafa59) ) // Need to verify correct label

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-sc4-3.2b",  0x0000, 0x2000, CRC(307b19ab) SHA1(b35ef4c2cf071db77cec1b4529b43a20cfcce172) ) // Need to verify correct label
	ROM_LOAD( "mds-sc4-3.2a",  0x2000, 0x2000, CRC(7263613a) SHA1(aa5673b57833d1f32c2cb0230a809397ec6103b4) ) // Need to verify correct label

	PALETTE_2C04_0003("ppu1:palette")
ROM_END

ROM_START( starlstr )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "sl_04.1d",  0x0000, 0x2000, CRC(4fd5b385) SHA1(a4cfdb9d74538a162825d9fbbed67e2a645bcc2c) )
	ROM_LOAD( "sl_03.1c",  0x2000, 0x2000, CRC(f26cd7ca) SHA1(f6fd5a6028b111a8fca68684bad651a92e0fd7be) )
	ROM_LOAD( "sl_02.1b",  0x4000, 0x2000, CRC(9308f34e) SHA1(4438d13dad793bbc158a5d163ccd4ae26f914fb5) )
	ROM_LOAD( "sl_01.1a",  0x6000, 0x2000, CRC(d87296e4) SHA1(a1220313f4c6ee1ee0beee9792f2e9038eaa4cb3) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "sl_06.2b",  0x0000, 0x2000, CRC(25f0e027) SHA1(4fcbe4bb959689948cb8f505d5c495dabb893f7b) )
	ROM_LOAD( "sl_05.2a",  0x2000, 0x2000, CRC(2bbb45fd) SHA1(53c3588bd25baa6b8ff41f4755db9e0e806c9719) )

	// also compatible with 2C04-0001, 2C04-0002, 2C04-0003, 2C04-0004 via dipswitches
	PALETTE_STANDARD("ppu1:palette")
ROM_END

ROM_START( vstetris )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "a000.6c",  0x2000, 0x2000, CRC(92a1cf10) SHA1(463f62aec3f26d70b35e804398a38baf8f41a5e3) )
	ROM_LOAD( "c000.6b",  0x4000, 0x2000, CRC(9e9cda9d) SHA1(27d91b957ff0b3abd5567341574318548470fb3c) )
	ROM_LOAD( "e000.6a",  0x6000, 0x2000, CRC(bfeaf6c1) SHA1(2f2150138c023cb7962f3e04d34bd01be9fa2e24) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "char.8b",  0x0000, 0x2000, CRC(51e8d403) SHA1(ed734994d164c4b59794249a13bce333896b3ee5) )

	// also compatible with 2C03, 2C04-0002, 2C04-0003, 2C04-0004 via dipswitches
	PALETTE_2C04_0001("ppu1:palette")
ROM_END

ROM_START( drmario )
	ROM_REGION( 0x10000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "dm-uiprg",  0x00000, 0x10000, CRC(d5d7eac4) SHA1(cd74c3a7a2fc7c25420037ae5f4a25307aff6587) )

	ROM_REGION( 0x8000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "dm-u3chr",  0x0000, 0x8000, CRC(91871aa5) SHA1(32a4299ead7b37f49877dc9597653b07a73ddbf3) )

	PALETTE_2C04_0003("ppu1:palette")
ROM_END

ROM_START( cstlevna )
	ROM_REGION( 0x20000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-cv.u7",  0x00000, 0x20000, CRC(ffbef374) SHA1(9eb3b75e7b45df51b8bcd29df84689a7e8557f4f) )

	// No cart gfx - uses vram

	PALETTE_2C04_0002("ppu1:palette")
ROM_END

ROM_START( topgun )
	ROM_REGION( 0x20000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "rc-003.u7",  0x00000, 0x20000, CRC(8c0c2df5) SHA1(d9b1b87204e025a637821a0168475e1209ce0c8a) )

	// No cart gfx - uses vram

	PALETTE_STANDARD("ppu1:palette")
ROM_END

ROM_START( tkoboxng )
	ROM_REGION( 0x10000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "tkoprg.bin",  0x00000, 0x10000, CRC(eb2dba63) SHA1(257c9f3565ff1d136094e99636ca57e300352b7e) )

	ROM_REGION( 0x10000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "tkochr.bin",  0x00000, 0x10000, CRC(21275ba5) SHA1(160131586aeeca848deabff258a2ce5f62b17c5f) )

	// also compatible with 2C03 via dipswitches
	// Manual clearly states "RP2C04 0004" as PPU type, but the palette files argue that this is incorrect
	PALETTE_2C04_0003("ppu1:palette")
ROM_END

ROM_START( rbibb )
	ROM_REGION( 0x10000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "rbi-prg",  0x00000, 0x10000, CRC(135adf7c) SHA1(e090b0aec98463c565e300a910561499d8bd9676) )

	ROM_REGION( 0x8000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "rbi-cha",  0x0000, 0x8000, CRC(a3c14889) SHA1(ef00f4fbf21cf34e946957b9b6825b8e2cb16536) )

	// also compatible with 2C03, 2C04-0002, 2C04-0003, 2C04-0004 via dipswitches
	PALETTE_2C04_0001("ppu1:palette")
ROM_END

ROM_START( rbibba )
	ROM_REGION( 0x10000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "rbi-prga", 0x00000, 0x10000, CRC(a5939d0d) SHA1(476ac2a3974b69082bb8eebdfc0d15befaa2e165) )

	ROM_REGION( 0x8000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "rbi-cha",  0x0000, 0x8000, CRC(a3c14889) SHA1(ef00f4fbf21cf34e946957b9b6825b8e2cb16536) )

	// also compatible with 2C03, 2C04-0002, 2C04-0003, 2C04-0004 via dipswitches
	PALETTE_2C04_0001("ppu1:palette")
ROM_END

ROM_START( vsskykid )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "sk-prg1",  0x0000, 0x08000, CRC(cf36261e) SHA1(e4a3d2a223f066c231631d92504f08e60b303dfd) )

	ROM_REGION( 0x8000, "gfx1", 0) // PPU memory
	ROM_LOAD( "sk-cha",  0x0000, 0x8000, CRC(9bd44dad) SHA1(bf33d175b6ab991d63a0acaf83ba22d5b7ab11b9) )

	// also compatible with 2C03, 2C04-0002, 2C04-0003, 2C04-0004 via dipswitches
	PALETTE_2C04_0001("ppu1:palette")
ROM_END

ROM_START( nvs_platoon )
	ROM_REGION( 0x20000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "prgver0.ic4",  0x00000, 0x20000, CRC(e2c0a2be) SHA1(1f8e33d6da8402be6a376668a424bfde38471021) )

	ROM_REGION( 0x20000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "chrver0.ic6",  0x00000, 0x20000, CRC(689df57d) SHA1(854aaa9feb16e3f239fba6069fbf65e69858fe73) )

	PALETTE_2C04_0001("ppu1:palette")
ROM_END

ROM_START( bnglngby )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-rb4-2 b.6d", 0x0000, 0x2000, CRC(d152d8c2) SHA1(d127195be8219df1c6f7bdd86658ed26c658470e) )
	ROM_LOAD( "mds-rb4-2 b.6c", 0x2000, 0x2000, CRC(c3383935) SHA1(8ed1e8ed36069e5e6f2f3c672aae5e1f3dabbdd0) )
	ROM_LOAD( "mds-rb4-2 b.6b", 0x4000, 0x2000, CRC(e2a24af8) SHA1(89cca4188b859882487fe64776c1ca0173fee142) )
	ROM_LOAD( "mds-rb4-2 b.6a", 0x6000, 0x2000, CRC(024ad874) SHA1(b02241c3d2ae90ccd5402410fa650741034a2f78) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-rb4-2 b.8b", 0x0000, 0x2000, CRC(d3d946ab) SHA1(e2ed8af0cf6edb925c1ff47fccb5caabd0b8c09f) )
	ROM_LOAD( "mds-rb4-2 b.8a", 0x2000, 0x2000, CRC(ca08126a) SHA1(48b315e3e90b19b2d74dcd88c734dcdf3539d6ca) )

	ROM_REGION( 0x2000, "user1", 0 ) // unknown
	ROM_LOAD( "mds-rb4-2 b.1a", 0x0000, 0x2000, CRC(b49939ad) SHA1(ebaab2864d9ff9876e9d2666746c4bab57e49ec3) ) // Unknown, maps at 0xe000, maybe from another set, but we have other roms?

	PALETTE_2C04_0002("ppu1:palette")
ROM_END

ROM_START( supxevs )
	ROM_REGION( 0x20000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "prg2",  0x00000, 0x10000, CRC(645669f0) SHA1(3b18c0bb33dd5a95f52a2de7b9a5730990517ad9) )
	ROM_LOAD( "prg1",  0x10000, 0x10000, CRC(ff762ceb) SHA1(04ca386ef4ad79f99d1efdc0a4d908ef0e523d75) )

	ROM_REGION( 0x8000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "cha",   0x00000, 0x08000, CRC(e27c7434) SHA1(a033bbaf0c28504ed2a641dea28f016a88ef03ac) )

	// also compatible with 2C04-0002, 2C04-0003, 2C04-0004 via dipswitches
	PALETTE_2C04_0001("ppu1:palette")
ROM_END

ROM_START( nvs_mightybj )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "1d.bin",  0x0000, 0x2000, CRC(55dc8d77) SHA1(eafb8636d994a10caee9eb0ba544260281706058) )
	ROM_LOAD( "1c.bin",  0x2000, 0x2000, CRC(151a6d15) SHA1(2652aef97aae122711ef471d9dc1d42f6393b91f) )
	ROM_LOAD( "1b.bin",  0x4000, 0x2000, CRC(9f9944bc) SHA1(58b1aca3e0cd32769978c704177d6ddeb70ac95a) )
	ROM_LOAD( "1a.bin",  0x6000, 0x2000, CRC(76f49b65) SHA1(c50fd29ea91bba3d59e943496d0941fe0e4efcb2) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "2b.bin",  0x0000, 0x2000, CRC(5425a4d0) SHA1(09eb9d93b680c9eefde5ee6e16cf81de931cccb9) )

	PALETTE_STANDARD("ppu1:palette")
ROM_END

// Dual System

ROM_START( balonfgt )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-bf4 a-3.1d",  0x0000, 0x2000, CRC(1248a6d6) SHA1(0f6c586e8e021a0710ec4e967750b55a74229d74) )
	ROM_LOAD( "mds-bf4 a-3.1c",  0x2000, 0x2000, CRC(14af0e42) SHA1(ceb749eca2dfe81fddc6cb57e4aa87a4bfac0316) )
	ROM_LOAD( "mds-bf4 a-3.1b",  0x4000, 0x2000, CRC(a420babf) SHA1(ab296a86132bb9103cbb107518b4ac9beb8b2e11) )
	ROM_LOAD( "mds-bf4 a-3.1a",  0x6000, 0x2000, CRC(9c31f94d) SHA1(19bccd6b79423f495b0ee49dd3b219ffc4676470) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-bf4 a-3.2b",  0x0000, 0x2000, CRC(f27d9aa0) SHA1(429a1ad2a07947e4c4809495bfab55bf0f0e428f) )
	ROM_LOAD( "mds-bf4 a-3.2a",  0x2000, 0x2000, CRC(76e6bbf8) SHA1(a4cae3a129a787162050187453b1583c8735fb46) )

	ROM_REGION( 0x10000, "sub", 0 ) // 6502 memory
	ROM_LOAD( "mds-bf4 a-3.6d",  0x08000, 0x02000, CRC(ef4ebff1) SHA1(17153ad44a402f05f7ddfe3ac364a0e4adb6f16b) )
	ROM_LOAD( "mds-bf4 a-3.6c",  0x0a000, 0x02000, CRC(14af0e42) SHA1(ceb749eca2dfe81fddc6cb57e4aa87a4bfac0316) )
	ROM_LOAD( "mds-bf4 a-3.6b",  0x0c000, 0x02000, CRC(a420babf) SHA1(ab296a86132bb9103cbb107518b4ac9beb8b2e11) )
	ROM_LOAD( "mds-bf4 a-3.6a",  0x0e000, 0x02000, CRC(3aa5c095) SHA1(3815016e5615c9327200150e0181357f16f3d636) )

	ROM_REGION( 0x4000, "gfx2", 0 ) // PPU memory
	ROM_LOAD( "mds-bf4 a-3.8b",  0x0000, 0x2000, CRC(f27d9aa0) SHA1(429a1ad2a07947e4c4809495bfab55bf0f0e428f) )
	ROM_LOAD( "mds-bf4 a-3.8a",  0x2000, 0x2000, CRC(76e6bbf8) SHA1(a4cae3a129a787162050187453b1583c8735fb46) )

	PALETTE_2C04_0003("ppu1:palette")
	ROM_REGION( 0xc0, "ppu2:palette", 0 )
	ROM_COPY( "ppu1:palette", 0x00, 0x00, 0xc0 )
ROM_END

ROM_START( vsmahjng )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mj.1c",  0x2000, 0x2000, CRC(ec77671f) SHA1(3716a4d5ab1efee0416dd7f6466d29379dc6f296) )
	ROM_LOAD( "mj.1b",  0x4000, 0x2000, CRC(ac53398b) SHA1(2582c73efec233a389900949d6af7c4c9a9e7148) )
	ROM_LOAD( "mj.1a",  0x6000, 0x2000, CRC(62f0df8e) SHA1(5628397c5d9acf470cc0cbffdba20e9e4cc8ea91) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mj.2b",  0x0000, 0x2000, CRC(9dae3502) SHA1(b7ffbc17af35eeac1b06c651f6c25f71827e9c3b) )

	ROM_REGION( 0x10000, "sub", 0 ) // 6502 memory
	ROM_LOAD( "mj.6c",  0x0a000, 0x02000, CRC(3cee11e9) SHA1(03ae904a98a12b5571374417069e50f8bc824c24) )
	ROM_LOAD( "mj.6b",  0x0c000, 0x02000, CRC(e8341f7b) SHA1(cf3c43e4f87dbcd4ae9a74f2808282883c8ba38a) )
	ROM_LOAD( "mj.6a",  0x0e000, 0x02000, CRC(0ee69f25) SHA1(078e8f51887be58336ff23f90bacfa90c1730f36) )

	ROM_REGION( 0x4000, "gfx2", 0 ) // PPU memory
	ROM_LOAD( "mj.8b",  0x0000, 0x2000, CRC(9dae3502) SHA1(b7ffbc17af35eeac1b06c651f6c25f71827e9c3b) )

	PALETTE_STANDARD("ppu1:palette")
	ROM_REGION( 0xc0, "ppu2:palette", 0 )
	ROM_COPY( "ppu1:palette", 0x00, 0x00, 0xc0 )
ROM_END

ROM_START( vsbball )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-ba__1d_e-1.1d",  0x0000, 0x2000, CRC(7ec792bc) SHA1(92d1f8809db89a8d99f7ea1d2ba3f9be69195866) )
	ROM_LOAD( "mds-ba__1c_e-1.1c",  0x2000, 0x2000, CRC(b631f8aa) SHA1(0ee8a8def9512552037fdac1a14a3ea9393bb943) )
	ROM_LOAD( "mds-ba__1b_e-1.1b",  0x4000, 0x2000, CRC(c856b45a) SHA1(7f15613120d72859ea1ed647c9eee3074f63f0b9) )
	ROM_LOAD( "mds-ba__1a_e-1.1a",  0x6000, 0x2000, CRC(06b74c18) SHA1(9a61161b4856b88e40eee6edb39e0a608748cf0b) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-ba__2b_e.2b",  0x0000, 0x2000, CRC(3ff8bec3) SHA1(28c1bf89ed1046243ca8cf122cefa0752c242577) )
	ROM_LOAD( "mds-ba__2a_e.2a",  0x2000, 0x2000, CRC(13b20cfd) SHA1(cb333cbea09557a9d2bdc351fabc61fc7760c35d) )

	ROM_REGION( 0x10000, "sub", 0 ) // 6502 memory
	ROM_LOAD( "mds-ba__6d_e-1.6d",  0x08000, 0x02000, CRC(0cc5225f) SHA1(a8eb3153ce3f1282901c305177347112df0fb3b2) )
	ROM_LOAD( "mds-ba__6c_e-1.6c",  0x0a000, 0x02000, CRC(9856ac60) SHA1(f033171c3dea6af63f1f328fee74e695c67adc92) )
	ROM_LOAD( "mds-ba__6b_e-1.6b",  0x0c000, 0x02000, CRC(d1312e63) SHA1(0fc46a4ef0fb8a304320f8b3cac3edd1cd9ed286) )
	ROM_LOAD( "mds-ba__6a_e-1.6a",  0x0e000, 0x02000, CRC(28199b4d) SHA1(e63d69662d3b70b883028d3103c8f65de8f5edda) )

	ROM_REGION( 0x4000, "gfx2", 0 ) // PPU memory
	ROM_LOAD( "mds-ba__8b_e.8b",  0x0000, 0x2000, CRC(3ff8bec3) SHA1(28c1bf89ed1046243ca8cf122cefa0752c242577) )
	ROM_LOAD( "mds-ba__8a_e.8a",  0x2000, 0x2000, CRC(13b20cfd) SHA1(cb333cbea09557a9d2bdc351fabc61fc7760c35d) )

	PALETTE_2C04_0001("ppu1:palette")
	ROM_REGION( 0xc0, "ppu2:palette", 0 )
	ROM_COPY( "ppu1:palette", 0x00, 0x00, 0xc0 )
ROM_END

ROM_START( vsbballj )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-ba a-3.1d",  0x0000, 0x2000, CRC(e234d609) SHA1(a148d6b57fbc9d5f91737fa30c2df2c2b66df404) )
	ROM_LOAD( "mds-ba a-3.1c",  0x2000, 0x2000, CRC(ca1a9591) SHA1(3544f244c59d3dab40c2745e84775b7c1defaf54) )
	ROM_LOAD( "mds-ba a-3.1b",  0x4000, 0x2000, CRC(50e1f6cf) SHA1(8eb4ccb4817295084280ffd1ee5261eee02485c5) )
	ROM_LOAD( "mds-ba a-3.1a",  0x6000, 0x2000, CRC(f796df5b) SHA1(7e7e1c69b27f2fe41c509107abbe802877e3a92e) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-ba a.2b",    0x0000, 0x2000, CRC(919147d0) SHA1(9fccdfccc2a3ec634e350880ded7053f36c377bc) )
	ROM_LOAD( "mds-ba a.2a",    0x2000, 0x2000, CRC(3f7edb00) SHA1(f59d24f15bdb8903187eabc1578dcb60443614ed) )

	ROM_REGION( 0x10000, "sub", 0 ) // 6502 memory
	ROM_LOAD( "mds-ba a-3.6d",  0x08000, 0x02000, CRC(6eb9e36e) SHA1(3877dee54a1a11417296150f7e7a1ae2c2847484) )
	ROM_LOAD( "mds-ba a-3.6c",  0x0a000, 0x02000, CRC(dca4dc75) SHA1(231819edb58caf96b4f5c56a44163fbb666dc67d) )
	ROM_LOAD( "mds-ba a-3.6b",  0x0c000, 0x02000, CRC(46cf6f84) SHA1(125af20e1e9066e4b92174ba0a7f59271ef57557) )
	ROM_LOAD( "mds-ba a-3.6a",  0x0e000, 0x02000, CRC(4cbc2cac) SHA1(90bed7694836075738d99aa8fe672dbffa7bbd6d) )

	ROM_REGION( 0x4000, "gfx2", 0 ) // PPU memory
	ROM_LOAD( "mds-ba a.8b",    0x0000, 0x2000, CRC(919147d0) SHA1(9fccdfccc2a3ec634e350880ded7053f36c377bc) )
	ROM_LOAD( "mds-ba a.8a",    0x2000, 0x2000, CRC(3f7edb00) SHA1(f59d24f15bdb8903187eabc1578dcb60443614ed) )

	PALETTE_2C04_0001("ppu1:palette")
	ROM_REGION( 0xc0, "ppu2:palette", 0 )
	ROM_COPY( "ppu1:palette", 0x00, 0x00, 0xc0 )
ROM_END

ROM_START( vsbballja )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-ba a-2.1d",  0x0000, 0x2000, CRC(f3820b70) SHA1(c50d0c2e34f646dd186ee0f2774e94add733f21d) )
	ROM_LOAD( "mds-ba a-2.1c",  0x2000, 0x2000, CRC(39fbbf28) SHA1(9941defda548f2c51cf62f0ad62a041ee9a69c37) )
	ROM_LOAD( "mds-ba a-2.1b",  0x4000, 0x2000, CRC(b1377b12) SHA1(9afca83f343b768de8ac51c5967f8825de9d7883) )
	ROM_LOAD( "mds-ba a-2.1a",  0x6000, 0x2000, CRC(08fab347) SHA1(b6ecd1464c47afac922355b8d5e961892e58a0ed) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-ba a.2b",    0x0000, 0x2000, CRC(919147d0) SHA1(9fccdfccc2a3ec634e350880ded7053f36c377bc) )
	ROM_LOAD( "mds-ba a.2a",    0x2000, 0x2000, CRC(3f7edb00) SHA1(f59d24f15bdb8903187eabc1578dcb60443614ed) )

	ROM_REGION( 0x10000, "sub", 0 ) // 6502 memory
	ROM_LOAD( "mds-ba a-2.6d",  0x08000, 0x02000, CRC(c69561b0) SHA1(4234d88ffa957e7f70ef9da8c61db4e251c3bc66) )
	ROM_LOAD( "mds-ba a-2.6c",  0x0a000, 0x02000, CRC(17d1ca39) SHA1(2fa61a2c39495b72a22f001a72e4526e86d9544e) )
	ROM_LOAD( "mds-ba a-2.6b",  0x0c000, 0x02000, CRC(37481900) SHA1(dbab48d6c95e365ee4ab6ca4c61224b2c813e538) )
	ROM_LOAD( "mds-ba a-2.6a",  0x0e000, 0x02000, CRC(a44ffc4b) SHA1(ec65c3b52659dacfd2b7afe1e744e7bbd61fd6e1) )

	ROM_REGION( 0x4000, "gfx2", 0 ) // PPU memory
	ROM_LOAD( "mds-ba a.8b",    0x0000, 0x2000, CRC(919147d0) SHA1(9fccdfccc2a3ec634e350880ded7053f36c377bc) )
	ROM_LOAD( "mds-ba a.8a",    0x2000, 0x2000, CRC(3f7edb00) SHA1(f59d24f15bdb8903187eabc1578dcb60443614ed) )

	PALETTE_2C04_0001("ppu1:palette")
	ROM_REGION( 0xc0, "ppu2:palette", 0 )
	ROM_COPY( "ppu1:palette", 0x00, 0x00, 0xc0 )
ROM_END

ROM_START( vsbballjb )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-ba a-1.1d",  0x0000, 0x2000, CRC(6dbc129b) SHA1(3e786632563364bf7ae13c7d25c522999f237009) )
	ROM_LOAD( "mds-ba a-1.1c",  0x2000, 0x2000, CRC(2a684b3a) SHA1(316aa1051a5ff33e5a2369f9e984b34f637595ff) )
	ROM_LOAD( "mds-ba a-1.1b",  0x4000, 0x2000, CRC(7ca0f715) SHA1(cf87e530c15c142efa48d6462870bbdf44002f45) )
	ROM_LOAD( "mds-ba a-1.1a",  0x6000, 0x2000, CRC(926bb4fc) SHA1(b9b8611b90d73f39f65166010058e03d0aad5bb0) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-ba a.2b",    0x0000, 0x2000, CRC(919147d0) SHA1(9fccdfccc2a3ec634e350880ded7053f36c377bc) )
	ROM_LOAD( "mds-ba a.2a",    0x2000, 0x2000, CRC(3f7edb00) SHA1(f59d24f15bdb8903187eabc1578dcb60443614ed) )

	ROM_REGION( 0x10000, "sub", 0 ) // 6502 memory
	ROM_LOAD( "mds-ba a-1.6d",  0x08000, 0x02000, CRC(d534dca4) SHA1(6d454a2b5944f98c95d3a1bdeee8e8e52524cb21) )
	ROM_LOAD( "mds-ba a-1.6c",  0x0a000, 0x02000, CRC(73904bbc) SHA1(d32a0f659d628b98a0b06f846842432f83e79a07) )
	ROM_LOAD( "mds-ba a-1.6b",  0x0c000, 0x02000, CRC(7c130724) SHA1(99134180e158eaa4b260d1dacf9aa56a6d48ad73) )
	ROM_LOAD( "mds-ba a-1.6a",  0x0e000, 0x02000, CRC(d938080e) SHA1(35e00bd76364ec88fb3bb8908bc9171df9cd26de) )

	ROM_REGION( 0x4000, "gfx2", 0 ) // PPU memory
	ROM_LOAD( "mds-ba a.8b",    0x0000, 0x2000, CRC(919147d0) SHA1(9fccdfccc2a3ec634e350880ded7053f36c377bc) )
	ROM_LOAD( "mds-ba a.8a",    0x2000, 0x2000, CRC(3f7edb00) SHA1(f59d24f15bdb8903187eabc1578dcb60443614ed) )

	PALETTE_2C04_0001("ppu1:palette")
	ROM_REGION( 0xc0, "ppu2:palette", 0 )
	ROM_COPY( "ppu1:palette", 0x00, 0x00, 0xc0 )
ROM_END

ROM_START( vstennis )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-te a-3.1d", 0x0000, 0x2000, CRC(8d88fbe5) SHA1(1aa172d02d0d47325edf2f5ea4fc3c1c52f1efbe) )
	ROM_LOAD( "mds-te a-2.1c", 0x2000, 0x2000, CRC(5f00c129) SHA1(e9954ebedc037be0a177286bbfc2ecdaa9223d85) )
	ROM_LOAD( "mds-te a-2.1b", 0x4000, 0x2000, CRC(4b57910c) SHA1(1baeb31e0e9085ac6a9406a1802dfa47952d833e) )
	ROM_LOAD( "mds-te a-2.1a", 0x6000, 0x2000, CRC(41097060) SHA1(aec457f7780dcd693dd93076cc185d5db38d5b93) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-te a.2b",  0x0000, 0x2000, CRC(9de19c9c) SHA1(1cb65e423a6c2d2a56c67ad08ecf7e746551c322) )
	ROM_LOAD( "mds-te a.2a",  0x2000, 0x2000, CRC(67a5800e) SHA1(7bad1b486d9dac962fa8c87984038be4ac6b699b) )

	ROM_REGION( 0x10000, "sub", 0 ) // 6502 memory
	ROM_LOAD( "mds-te a-3.6d", 0x08000, 0x2000, CRC(b18fd769) SHA1(152413e065d1f5af0a70f9272a908dfbd162fe65) )
	ROM_LOAD( "mds-te a-2.6c", 0x0a000, 0x2000, CRC(315d8178) SHA1(2165c8a42004fb5b1e6b8904a59159cd4157538e) )
	ROM_LOAD( "mds-te a-2.6b", 0x0c000, 0x2000, CRC(18114f8d) SHA1(351894e0cc791028a43da0ec27d78d669cdeea27) )
	ROM_LOAD( "mds-te a-2.6a", 0x0e000, 0x2000, CRC(50a2de11) SHA1(3e22e50c9ae2521dc7f4416ac834cdbd3988369e) )

	ROM_REGION( 0x4000, "gfx2", 0) // PPU memory
	ROM_LOAD( "mds-te a.8b",  0x0000, 0x2000, CRC(c81e9260) SHA1(6d4809a05364cc05485ee1add833428529af2be6) )
	ROM_LOAD( "mds-te a.8a",  0x2000, 0x2000, CRC(d91eb295) SHA1(6b69bcef5421a6bcde89a2d1f514853f9f7992c3) )

	PALETTE_STANDARD("ppu1:palette")
	ROM_REGION( 0xc0, "ppu2:palette", 0 )
	ROM_COPY( "ppu1:palette", 0x00, 0x00, 0xc0 )
ROM_END

ROM_START( vstennisa )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-te.1d",  0x0000, 0x2000, CRC(f4e9fca0) SHA1(05b91f578bc0a118ab75ce487b14adcd1fb6e714) )
	ROM_LOAD( "mds-te.1c",  0x2000, 0x2000, CRC(7e52df58) SHA1(a5ddebfa1f7f1a2b6b46d4b4a7f2c36477158e7e) )
	ROM_LOAD( "mds-te.1b",  0x4000, 0x2000, CRC(1a0d809a) SHA1(44ce2f9250940bf5f754918b4a2ae63f76181eff) )
	ROM_LOAD( "mds-te.1a",  0x6000, 0x2000, CRC(8483a612) SHA1(c854f72d86fe4e99c4c6426cfc5ea6f2997bfc8c) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-te a.2b",  0x0000, 0x2000, CRC(9de19c9c) SHA1(1cb65e423a6c2d2a56c67ad08ecf7e746551c322) )
	ROM_LOAD( "mds-te a.2a",  0x2000, 0x2000, CRC(67a5800e) SHA1(7bad1b486d9dac962fa8c87984038be4ac6b699b) )

	ROM_REGION( 0x10000, "sub", 0 ) // 6502 memory
	ROM_LOAD( "mds-te.6d",  0x08000, 0x02000, CRC(3131b1bf) SHA1(ed26df260df3a295b5c9747530428efec29676c0) )
	ROM_LOAD( "mds-te.6c",  0x0a000, 0x02000, CRC(27195d13) SHA1(a1d6960a194cb048c5c26f9378b49da7d6e7d1af) )
	ROM_LOAD( "mds-te.6b",  0x0c000, 0x02000, CRC(4b4e26ca) SHA1(68821357f473a0e1c575b547cc8c67be965fe73a) )
	ROM_LOAD( "mds-te.6a",  0x0e000, 0x02000, CRC(b6bfee07) SHA1(658458931efbb260faec3a11ee530326c56e63a9) )

	ROM_REGION( 0x4000, "gfx2", 0) // PPU memory
	ROM_LOAD( "mds-te a.8b",  0x0000, 0x2000, CRC(c81e9260) SHA1(6d4809a05364cc05485ee1add833428529af2be6) )
	ROM_LOAD( "mds-te a.8a",  0x2000, 0x2000, CRC(d91eb295) SHA1(6b69bcef5421a6bcde89a2d1f514853f9f7992c3) )

	PALETTE_STANDARD("ppu1:palette")
	ROM_REGION( 0xc0, "ppu2:palette", 0 )
	ROM_COPY( "ppu1:palette", 0x00, 0x00, 0xc0 )
ROM_END

ROM_START( vstennisb )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "1d", 0x0000, 0x2000, CRC(34ba9922) SHA1(709d3034d98eaf519ab1a824483bf14470291dab) )
	ROM_LOAD( "1c", 0x2000, 0x2000, CRC(d4874908) SHA1(53c00468f87185dcc37f5f3874b395f639e7d34f) )
	ROM_LOAD( "1b", 0x4000, 0x2000, CRC(eee3fc51) SHA1(02e22506106026c84150e3d937d08d4bd2edefdd) )
	ROM_LOAD( "1a", 0x6000, 0x2000, CRC(83cc5e96) SHA1(edd5b7d466d2f7d5714c376cb781382d4e6c6fcc) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "2b",  0x0000, 0x2000, CRC(9de19c9c) SHA1(1cb65e423a6c2d2a56c67ad08ecf7e746551c322) )
	ROM_LOAD( "2a",  0x2000, 0x2000, CRC(67a5800e) SHA1(7bad1b486d9dac962fa8c87984038be4ac6b699b) )

	ROM_REGION( 0x10000, "sub", 0 ) // 6502 memory
	ROM_LOAD( "6d", 0x08000, 0x2000, BAD_DUMP CRC(5da8a6a0) SHA1(a153a5bfb9e64978eb33d7ed4f4cf96d9bc7ee4a) )
	ROM_LOAD( "6c", 0x0a000, 0x2000, BAD_DUMP CRC(154af5fe) SHA1(2459a31a3cd427609c427ade093b12abc3df2d78) )
	ROM_LOAD( "6b", 0x0c000, 0x2000, BAD_DUMP CRC(0da89d4e) SHA1(f183729477b410e04ad5ccf8b7212f45bc0e466b) )
	ROM_LOAD( "6a", 0x0e000, 0x2000, BAD_DUMP CRC(a5bc72be) SHA1(8046c2b7679d7a4bc9bd5a178404053113d3527e) )

	ROM_REGION( 0x4000, "gfx2", 0) // PPU memory
	ROM_LOAD( "8b",  0x0000, 0x2000, CRC(c81e9260) SHA1(6d4809a05364cc05485ee1add833428529af2be6) )
	ROM_LOAD( "8a",  0x2000, 0x2000, CRC(d91eb295) SHA1(6b69bcef5421a6bcde89a2d1f514853f9f7992c3) )

	PALETTE_STANDARD("ppu1:palette")
	ROM_REGION( 0xc0, "ppu2:palette", 0 )
	ROM_COPY( "ppu1:palette", 0x00, 0x00, 0xc0 )
ROM_END

ROM_START( wrecking )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "wr.1d",  0x0000, 0x2000, CRC(8897e1b9) SHA1(7d33f6ee78d8663d62e6e05e231fd3d19ad09baa) )
	ROM_LOAD( "wr.1c",  0x2000, 0x2000, CRC(d4dc5ebb) SHA1(bce9b2ebabe7b882f1bc71e2dd50906365521d78) )
	ROM_LOAD( "wr.1b",  0x4000, 0x2000, CRC(8ee4a454) SHA1(58e970780a2ef5d44950dba6b44e501d320c9588) )
	ROM_LOAD( "wr.1a",  0x6000, 0x2000, CRC(63d6490a) SHA1(40f573713b7729bc26f41978583defca47f75033) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "wr.2b",  0x0000, 0x2000, CRC(455d77ac) SHA1(fa09d0be51cc780f6c16cd314facc84043e1e69b) )
	ROM_LOAD( "wr.2a",  0x2000, 0x2000, CRC(653350d8) SHA1(d9aa699394654deaf50fadd8a652f08a340377eb) )

	ROM_REGION( 0x10000, "sub", 0 ) // 6502 memory
	ROM_LOAD( "wr.6d",  0x08000, 0x02000, CRC(90e49ce7) SHA1(dca3004305979dc09500fae4667084363fac761f) )
	ROM_LOAD( "wr.6c",  0x0a000, 0x02000, CRC(a12ae745) SHA1(15deabebc4ef59f08aa8ead3f576ed5cbde4c62e) )
	ROM_LOAD( "wr.6b",  0x0c000, 0x02000, CRC(03947ca9) SHA1(02f0404d2351d2475240818b6b103a6e01691daf) )
	ROM_LOAD( "wr.6a",  0x0e000, 0x02000, CRC(2c0a13ac) SHA1(47e6a50c210508fab51062eb5c8a3e1129c18125) )

	ROM_REGION( 0x4000, "gfx2", 0 ) // PPU memory
	ROM_LOAD( "wr.8b",  0x0000, 0x2000, CRC(455d77ac) SHA1(fa09d0be51cc780f6c16cd314facc84043e1e69b) )
	ROM_LOAD( "wr.8a",  0x2000, 0x2000, CRC(653350d8) SHA1(d9aa699394654deaf50fadd8a652f08a340377eb) )

	PALETTE_2C04_0002("ppu1:palette")
	ROM_REGION( 0xc0, "ppu2:palette", 0 )
	ROM_COPY( "ppu1:palette", 0x00, 0x00, 0xc0 )
ROM_END

ROM_START( iceclmrd )
	ROM_REGION( 0x8000, "prg", 0 ) // 6502 memory
	ROM_LOAD( "mds-ic4-4 a.1d",  0x0000, 0x2000, CRC(94e3197d) SHA1(414156809a3fe2c072d8947a91708f3ed40008b2) )
	ROM_LOAD( "mds-ic4-4 a.1c",  0x2000, 0x2000, CRC(b253011e) SHA1(abc2c84e342d1f8e8d0dbb580370733ef4b38413) )
	ROM_LOAD( "mds-ic4-4 a-1.1b",0x4000, 0x2000, CRC(f3795874) SHA1(f22f786960a27ab886a7fad7e312bdf28ffa5362) )
	ROM_LOAD( "mds-ic4-4 a.1a",  0x6000, 0x2000, CRC(094c246c) SHA1(82aba548706041c2de0cda02d21409fe8a09338c) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // PPU memory
	ROM_LOAD( "mds-ic4-4 a.2b",  0x0000, 0x2000, CRC(331460b4) SHA1(4cf94d711cdb5715d14f1ab3cadec245e0adfb1e) )
	ROM_LOAD( "mds-ic4-4 a.2a",  0x2000, 0x2000, CRC(4ec44fb3) SHA1(676e0ab574dec08df562c6f278e8a9cc7c8afa41) )

	ROM_REGION( 0x10000, "sub", 0 ) // 6502 memory
	ROM_LOAD( "mds-ic4-4 a.6d",  0x08000, 0x02000, CRC(94e3197d) SHA1(414156809a3fe2c072d8947a91708f3ed40008b2) )
	ROM_LOAD( "mds-ic4-4 a.6c",  0x0a000, 0x02000, CRC(b253011e) SHA1(abc2c84e342d1f8e8d0dbb580370733ef4b38413) )
	ROM_LOAD( "mds-ic4-4 a.6b",  0x0c000, 0x02000, CRC(2ee9c1f9) SHA1(71619cff6d41cf5a8f74a689e30c2a24020f7d06) )
	ROM_LOAD( "mds-ic4-4 a.6a",  0x0e000, 0x02000, CRC(094c246c) SHA1(82aba548706041c2de0cda02d21409fe8a09338c) )

	ROM_REGION( 0x4000, "gfx2", 0 ) // PPU memory
	ROM_LOAD( "mds-ic4-4 a.8b",  0x0000, 0x2000, CRC(331460b4) SHA1(4cf94d711cdb5715d14f1ab3cadec245e0adfb1e) )
	ROM_LOAD( "mds-ic4-4 a.8a",  0x2000, 0x2000, CRC(4ec44fb3) SHA1(676e0ab574dec08df562c6f278e8a9cc7c8afa41) )

	PALETTE_2C04_0004("ppu1:palette")
	ROM_REGION( 0xc0, "ppu2:palette", 0 )
	ROM_COPY( "ppu1:palette", 0x00, 0x00, 0xc0 )
ROM_END

} // anonymous namespace

//******************************************************************************
/* Sets by region:
   World (C) Nintendo
   US    (C) Nintendo of America
   Japan (C) Nintendo Co., Ltd.
*/

//    YEAR  NAME            PARENT         MACHINE        INPUT     STATE           INIT           MONITOR
GAME( 1985, btlecity,       0,             vsnes,         btlecity, vs_uni_state,   init_vsnormal, ROT0, "Namco",                  "Vs. Battle City",                                          0 )
GAME( 1985, starlstr,       0,             vsnes,         starlstr, vs_uni_state,   init_vsnormal, ROT0, "Namco",                  "Vs. Star Luster",                                          0 )
GAME( 1987, cstlevna,       0,             vsnes,         cstlevna, vs_uni_state,   init_vsvram,   ROT0, "Konami",                 "Vs. Castlevania",                                          0 )
GAME( 1984, cluclu,         0,             vsnes,         cluclu,   vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Clu Clu Land",                                         0 )
GAME( 1990, drmario,        0,             vsnes,         drmario,  vs_uni_state,   init_drmario,  ROT0, "Nintendo",               "Vs. Dr. Mario",                                            0 )
GAME( 1984, excitebk,       0,             vsnes,         excitebk, vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Excitebike (set EB4-3 E)",                             0 )
GAME( 1984, excitebko,      excitebk,      vsnes,         excitebk, vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Excitebike (set EB4-3 ?)",                             0 ) // older
GAME( 1984, excitebkj,      excitebk,      vsnes,         excitebk, vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Excitebike (set EB4-4 A)",                             0 ) // likely Japan
GAME( 1986, goonies,        0,             vsnes,         goonies,  vs_uni_state,   init_vskonami, ROT0, "Konami",                 "Vs. The Goonies (set E)",                                  0 )
GAME( 1984, iceclimb,       0,             vsnes,         iceclmbj, vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Ice Climber (set IC4-4 B-1)",                          0 )
GAME( 1984, iceclimba,      iceclimb,      vsnes,         iceclimb, vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Ice Climber (set IC4-4 ?)",                            0 )
GAME( 1985, nvs_machrider,  0,             vsnes,         machridr, vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Mach Rider (Endurance Course Version)",                MACHINE_IMPERFECT_GRAPHICS )
GAME( 1985, nvs_machridera, nvs_machrider, vsnes,         machridj, vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Mach Rider (Fighting Course Version, set MR4-1 A)",    MACHINE_IMPERFECT_GRAPHICS )
GAME( 1986, rbibb,          0,             vsnes,         rbibb,    vs_uni_state,   init_rbibb,    ROT0, "Namco",                  "Vs. Atari R.B.I. Baseball (set 1)",                        0 )
GAME( 1986, rbibba,         rbibb,         vsnes,         rbibb,    vs_uni_state,   init_rbibb,    ROT0, "Namco",                  "Vs. Atari R.B.I. Baseball (set 2)",                        0 )
GAME( 1986, suprmrio,       0,             vsnes,         suprmrio, vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Super Mario Bros. (set SM4-4 E)",                      0 )
GAME( 1986, suprmrioa,      suprmrio,      vsnes,         suprmrio, vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Super Mario Bros. (set ?, harder)",                    0 )
GAME( 1986, suprmriobl,     suprmrio,      vs_smbbl,      suprmrio, vs_smbbl_state, empty_init,    ROT0, "bootleg",                "Vs. Super Mario Bros. (bootleg with Z80, set 1)",          0 ) // timer starts at 200(!)
GAME( 1986, suprmriobl2,    suprmrio,      vs_smbbl,      suprmrio, vs_smbbl_state, empty_init,    ROT0, "bootleg",                "Vs. Super Mario Bros. (bootleg with Z80, set 2)",          0 ) // timer starts at 300
GAME( 1988, skatekds,       suprmrio,      vsnes,         suprmrio, vs_uni_state,   init_vsnormal, ROT0, "hack (Two-Bit Score)",   "Vs. Skate Kids. (Graphic hack of Super Mario Bros.)",      0 )
GAME( 1985, vsskykid,       0,             vsnes,         vsskykid, vs_uni_state,   init_vs108,    ROT0, "Namco",                  "Vs. Super SkyKid",                                         0 )
GAME( 1987, tkoboxng,       0,             vsnes,         tkoboxng, vs_uni_state,   init_tkoboxng, ROT0, "Namco / Data East USA",  "Vs. T.K.O. Boxing",                                        0 )
GAME( 1984, smgolf,         0,             vsnes,         golf,     vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Stroke & Match Golf (Men Version, set GF4-2 F)",       0 )
GAME( 1984, smgolfb,        smgolf,        vsnes,         golf4s,   vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Stroke & Match Golf (Men Version, set GF4-2 ?)",       0 )
GAME( 1984, smgolfj,        smgolf,        vsnes,         golfj,    vs_uni_state,   init_vsnormal, ROT0, "Nintendo Co., Ltd.",     "Vs. Stroke & Match Golf (Men Version) (Japan, set GF3 B)", 0 )
GAME( 1984, ladygolfe,      smgolf,        vsnes,         golf4s,   vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Stroke & Match Golf (Ladies Version, set LG4 E)",      0 )
GAME( 1984, ladygolf,       smgolf,        vsnes,         golf,     vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Stroke & Match Golf (Ladies Version, set LG4 ?)",      0 )
GAME( 1984, vspinbal,       0,             vsnes,         vspinbal, vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Pinball (US, set PN4 E-1)",                            0 )
GAME( 1984, vspinbalj,      vspinbal,      vsnes,         vspinbal, vs_uni_state,   init_vsnormal, ROT0, "Nintendo Co., Ltd.",     "Vs. Pinball (Japan, set PN3 B)",                           0 )
GAME( 1986, vsslalom,       0,             vsnes,         vsslalom, vs_uni_state,   init_vsnormal, ROT0, "Rare Coin-It Inc.",      "Vs. Slalom",                                               MACHINE_IMPERFECT_GRAPHICS )
GAME( 1985, vssoccer,       0,             vsnes,         vssoccer, vs_uni_state,   init_vsnormal, ROT0, "Nintendo",               "Vs. Soccer (set SC4-2 A)",                                 0 )
GAME( 1985, vssoccera,      vssoccer,      vsnes,         vssoccera, vs_uni_state,  init_vsnormal, ROT0, "Nintendo",               "Vs. Soccer (set SC4-3 ?)",                                 0 )
GAME( 1986, vsgradus,       0,             vsnes,         vsgradus, vs_uni_state,   init_vskonami, ROT0, "Konami",                 "Vs. Gradius (US, set GR E)",                               0 )
GAME( 1987, nvs_platoon,    0,             vsnes,         platoon,  vs_uni_state,   init_platoon,  ROT0, "Ocean Software Limited", "Vs. Platoon",                                              0 )
GAME( 1987, vstetris,       0,             vsnes,         vstetris, vs_uni_state,   init_vsnormal, ROT0, "Academysoft-Elorg",      "Vs. Tetris" ,                                              0 )
GAME( 1986, nvs_mightybj,   0,             mightybj,      mightybj, vs_uni_state,   init_vsnormal, ROT0, "Tecmo",                  "Vs. Mighty Bomb Jack (Japan)",                             0 )
GAME( 1985, jajamaru,       0,             jajamaru,      jajamaru, vs_uni_state,   init_vsnormal, ROT0, "Jaleco",                 "Vs. Ninja Jajamaru Kun (Japan)",                           0 )
GAME( 1987, topgun,         0,             topgun,        topgun,   vs_uni_state,   init_vsvram,   ROT0, "Konami",                 "Vs. Top Gun",                                              0 )
GAME( 1985, bnglngby,       0,             vsnes,         bnglngby, vs_uni_state,   init_bnglngby, ROT0, "Nintendo / Broderbund Software Inc.", "Vs. Raid on Bungeling Bay (RD4-2 B)",         0 )
GAME( 1986, supxevs,        0,             vsnes,         supxevs,  vs_uni_state,   init_supxevs,  ROT0, "Namco",                  "Vs. Super Xevious",                                        0 )

// Light Gun games
GAME( 1985, duckhunt,       0,             vsnes,         duckhunt, vs_uni_state,   init_vsgun,    ROT0, "Nintendo",               "Vs. Duck Hunt (set DH3 E)",         0 )
GAME( 1985, hogalley,       0,             vsnes,         hogalley, vs_uni_state,   init_vsgun,    ROT0, "Nintendo",               "Vs. Hogan's Alley (set HA4-1 E-1)", 0 )
GAME( 1986, vsgshoe,        0,             vsgshoe,       vsgshoe,  vs_uni_state,   init_vsgshoe,  ROT0, "Nintendo",               "Vs. Gumshoe (set GM5)",             0 )
GAME( 1988, vsfdf,          0,             vsnes,         vsfdf,    vs_uni_state,   init_vsfdf,    ROT0, "Sunsoft",                "Vs. Freedom Force",                 0 )

// Dual games
GAME( 1984, vstennis,       0,             vsdual,        vstennis, vs_dual_state,  init_vsdual,   ROT0, "Nintendo Co., Ltd.",     "Vs. Tennis (Japan/USA, set TE A-3)" ,  0 )
GAME( 1984, vstennisa,      vstennis,      vsdual,        vstennis, vs_dual_state,  init_vsdual,   ROT0, "Nintendo Co., Ltd.",     "Vs. Tennis (Japan/USA, set 2)",        0 )
GAME( 1984, vstennisb,      vstennis,      vsdual,        vstennis, vs_dual_state,  init_vsdual,   ROT0, "Nintendo Co., Ltd.",     "Vs. Tennis (Japan/USA, set 3)",        MACHINE_IMPERFECT_GRAPHICS )
GAME( 1984, wrecking,       0,             vsdual,        wrecking, vs_dual_state,  init_vsdual,   ROT0, "Nintendo",               "Vs. Wrecking Crew",                    0 )
GAME( 1984, balonfgt,       0,             vsdual,        balonfgt, vs_dual_state,  init_vsdual,   ROT0, "Nintendo",               "Vs. Balloon Fight (set BF4 A-3)",      0 )
GAME( 1984, vsmahjng,       0,             vsdual,        vsmahjng, vs_dual_state,  init_vsdual,   ROT0, "Nintendo Co., Ltd.",     "Vs. Mahjong (Japan)",                  0 )
GAME( 1984, vsbball,        0,             vsdual,        vsbball,  vs_dual_state,  init_vsdual,   ROT0, "Nintendo of America",    "Vs. Baseball (US, set BA E-1)",        0 )
GAME( 1984, vsbballj,       vsbball,       vsdual,        vsbball,  vs_dual_state,  init_vsdual,   ROT0, "Nintendo Co., Ltd.",     "Vs. Baseball (Japan, set BA A-3)",     0 )
GAME( 1984, vsbballja,      vsbball,       vsdual,        vsbball,  vs_dual_state,  init_vsdual,   ROT0, "Nintendo Co., Ltd.",     "Vs. Baseball (Japan, set BA A-2)",     0 )
GAME( 1984, vsbballjb,      vsbball,       vsdual,        vsbball,  vs_dual_state,  init_vsdual,   ROT0, "Nintendo Co., Ltd.",     "Vs. Baseball (Japan, set BA A-1)",     0 )
GAME( 1984, iceclmrd,       0,             vsdual,        iceclmrj, vs_dual_state,  init_vsdual,   ROT0, "Nintendo",               "Vs. Ice Climber Dual (set IC4-4 A-1)", 0 )
