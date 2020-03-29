// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  nes_vt.c

  The 'VT' series are SoC solutions that implement enhanced NES hardware
  there are several generations of these chips each adding additional
  functionality.

  This list is incomplete

  VT01 - plain famiclone?
  VT02 - banking scheme to access 32MB, Dual APU with PCM support
  VT03 - above + 4bpp sprite / bg modes, enhanced palette

  VT08 - ?

  VT09 - alt 4bpp modes?

  VT16 - ?
  VT18 - ?

    VT33 (?) - used in FC Pocket, dgun2573
        Adds scrambled opcodes (XORed with 0xA1) and RGB444 palette mode,
        and more advanced PCM modes (CPU and video working, sound NYI)

    VT368 (?) - used in DGUN2561, lxcmcy
        Various enhancements not yet emulated. Different banking, possibly an ALU,
        larger palette space

    VT36x (?) - used in SY889
        Uses SQI rather than parallel flash
        Vaguely OneBus compatible but some registers different ($411C in particular)
        Uses RGB format for palettes
        Credit to NewRisingSun2 for much of the reverse engineering
                 same chipset used in Mogis M320, but uses more advanced feature set.

  (more)
  


  todo (VT03):

  APU refactoring to allow for mostly doubled up functionality + PCM channel
  *more*

  todo (newer VTxx):

  new PCM audio in FC Pocket and DGUN-2573
    add support for VT368 (?) in DGUN-2561 and lxcmcy
    add support for the VT369 (?) featurs used by the MOGIS M320

  todo (general)

  Add more Famiclone roms here, there should be plenty more dumps of VTxx
  based systems floating about.

  Make sure that none of the unenhanced sets were actually sold as cartridges
  instead, there is a lack of information for some of the older dumps and
  still some dumps in nes.xml that might belong here.

  NON-bugs (same happens on real hardware)

  Pause screen has corrupt GFX on enhanced version of Octopus

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/n2a03.h"
#include "machine/bankdev.h"
#include "video/ppu2c0x_vt.h"
#include "machine/nes_vt.h"
#include "machine/m6502_vtscr.h"
#include "machine/m6502_swap_op_d5_d6.h"
#include "screen.h"
#include "speaker.h"

class nes_vt_state : public driver_device
{
public:
	nes_vt_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soc(*this, "soc"),
		m_io0(*this, "IO0"),
		m_io1(*this, "IO1"),
		m_screen(*this, "screen"),
		m_ppu(*this, "ppu"),
		m_apu(*this, "apu"),
		m_cartsel(*this, "CARTSEL"),
		m_exin0(*this, "EXTRAIN0"),
		m_exin1(*this, "EXTRAIN1"),
		m_exin2(*this, "EXTRAIN2"),
		m_exin3(*this, "EXTRAIN3"),
		m_vt_external_space(*this, "vt_ext_space"),
		m_prgrom(*this, "mainrom"),
		m_initial_e000_bank(0xff),
		m_ntram(nullptr),
		m_chrram(nullptr)
	{ }

	void nes_vt_base(machine_config& config);
	void nes_vt_base_pal(machine_config& config);

	void nes_vt_pal(machine_config& config);
	void nes_vt_pal_2mb(machine_config& config);
	void nes_vt_pal_4mb(machine_config& config);
	void nes_vt_pal_8mb(machine_config& config);

	void nes_vt(machine_config& config);
	void nes_vt_512kb(machine_config& config);
	void nes_vt_1mb(machine_config& config);
	void nes_vt_2mb(machine_config& config);
	void nes_vt_4mb(machine_config& config);
	void nes_vt_8mb(machine_config& config);
	void nes_vt_16mb(machine_config& config);
	void nes_vt_32mb(machine_config& config);

	void nes_vt_4k_ram(machine_config& config);
	void nes_vt_4k_ram_16mb(machine_config& config);


	void nes_vt_4k_ram_pal(machine_config& config);

	/* OneBus read callbacks for getting sprite and tile data during rendering */
	DECLARE_READ8_MEMBER(spr_r);
	DECLARE_READ8_MEMBER(chr_r);

	/* Misc PPU */
	DECLARE_WRITE8_MEMBER(vt_dma_w);
	DECLARE_WRITE8_MEMBER(vt_fixed_dma_w);

	// TODO: give these better register names so it's clearer what is remapped
	void set_8000_scramble(uint8_t reg0, uint8_t reg1, uint8_t reg2, uint8_t reg3, uint8_t reg4, uint8_t reg5, uint8_t reg6, uint8_t reg7);
	void set_410x_scramble(uint8_t reg0, uint8_t reg1);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual DECLARE_READ8_MEMBER(in0_r);
	virtual DECLARE_READ8_MEMBER(in1_r);
	virtual DECLARE_WRITE8_MEMBER(in0_w);

	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	void nes_vt_map(address_map& map);

	required_device<cpu_device> m_maincpu;
	required_device<nes_vt_soc_device> m_soc;

	optional_ioport m_io0;
	optional_ioport m_io1;
	uint8_t m_latch0;
	uint8_t m_latch1;

	required_device<screen_device> m_screen;
	required_device<ppu_vt03_device> m_ppu;
	required_device<nesapu_device> m_apu;

	/* APU handling */
	DECLARE_READ8_MEMBER(psg1_4014_r);
	DECLARE_READ8_MEMBER(psg1_4015_r);
	DECLARE_WRITE8_MEMBER(psg1_4015_w);
	DECLARE_WRITE8_MEMBER(psg1_4017_w);

	/* VT03 extension handling */
	DECLARE_WRITE8_MEMBER(vt03_410x_w);
	DECLARE_READ8_MEMBER(vt03_410x_r);

	/* Misc */
	uint32_t m_ahigh; // external banking bits
	void update_banks();
	uint8_t m_4242;
	uint8_t m_411c;
	uint8_t m_411d;
	uint8_t m_413x[8]; // CY only?

	uint8_t m_8000_scramble[8];
	uint8_t m_410x_scramble[2];

	void scrambled_410x_w(uint16_t offset, uint8_t data);
	void scrambled_8000_w(address_space& space, uint16_t offset, uint8_t data);

	DECLARE_WRITE8_MEMBER(vt03_8000_mapper_w);
	DECLARE_WRITE8_MEMBER(vt03_4034_w);

	optional_ioport m_cartsel;
	optional_ioport m_exin0;
	optional_ioport m_exin1;
	optional_ioport m_exin2;
	optional_ioport m_exin3;

	required_device<address_map_bank_device> m_vt_external_space;
	required_region_ptr<uint8_t> m_prgrom;

	void nes_vt_4k_ram_map(address_map& map);

	/* Misc */
	DECLARE_READ8_MEMBER(rs232flags_region_r);

	uint8_t m_initial_e000_bank;

	void vt_external_space_map_32mbyte(address_map& map);
	void vt_external_space_map_16mbyte(address_map& map);
	void vt_external_space_map_8mbyte(address_map& map);
	void vt_external_space_map_4mbyte(address_map& map);
	void vt_external_space_map_2mbyte(address_map& map);
	void vt_external_space_map_1mbyte(address_map& map);
	void vt_external_space_map_512kbyte(address_map& map);

	DECLARE_READ8_MEMBER(vt_rom_r);
	DECLARE_WRITE8_MEMBER(vtspace_w);

	DECLARE_READ8_MEMBER(external_space_read) { int bank = (offset & 0x6000) >> 13; int address = (m_bankaddr[bank] * 0x2000) + (offset&0x1fff); m_real_access_address = offset + 0x8000; return m_vt_external_space->read8(address); };
	DECLARE_WRITE8_MEMBER(external_space_write) { int bank = (offset & 0x6000) >> 13; int address = (m_bankaddr[bank] * 0x2000) + (offset&0x1fff); m_real_access_address = offset + 0x8000; m_vt_external_space->write8(address, data); };

private:
	/* APU handling */
	DECLARE_WRITE_LINE_MEMBER(apu_irq);
	DECLARE_READ8_MEMBER(apu_read_mem);

	/* Extra IO */
	DECLARE_WRITE8_MEMBER(extra_io_control_w);
	virtual DECLARE_READ8_MEMBER(extrain_01_r);
	virtual DECLARE_READ8_MEMBER(extrain_23_r);
	virtual DECLARE_WRITE8_MEMBER(extraout_01_w);
	virtual DECLARE_WRITE8_MEMBER(extraout_23_w);


	DECLARE_WRITE8_MEMBER(chr_w);

	void prg_map(address_map& map);

	/* expansion nametable - todo, see if we can refactor NES code to be reusable without having to add full NES bus etc. */
	std::unique_ptr<uint8_t[]> m_ntram;
	std::unique_ptr<uint8_t[]> m_chrram;

	DECLARE_READ8_MEMBER(nt_r);
	DECLARE_WRITE8_MEMBER(nt_w);

	void scanline_irq(int scanline, int vblank, int blanked);
	void hblank_irq(int scanline, int vblank, int blanked);
	void video_irq(bool hblank, int scanline, int vblank, int blanked);

	uint8_t m_410x[0xc];

	uint8_t m_vdma_ctrl;

	int m_timer_irq_enabled;
	int m_timer_running;
	int m_timer_val;

	uint8_t m_8000_addr_latch;

	/* banking etc. */
	uint32_t get_banks(uint8_t bnk);

	int calculate_real_video_address(int addr, int extended, int readtype);

	uint16_t m_real_access_address;


	int m_bankaddr[4];


	uint16_t decode_nt_addr(uint16_t addr);
	void do_dma(uint8_t data, bool has_ntsc_bug);

};

class nes_vt_swap_op_d5_d6_state : public nes_vt_state
{
public:
	nes_vt_swap_op_d5_d6_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void nes_vt_vh2009(machine_config& config);
	void nes_vt_vh2009_4mb(machine_config& config);
	void nes_vt_vh2009_8mb(machine_config& config);

	void nes_vt_senwld_512kb(machine_config& config);

protected:
	void vt_external_space_map_senwld_512kbyte(address_map& map);

private:
};

class nes_vt_pjoy_state : public nes_vt_state
{
public:
	nes_vt_pjoy_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override;
};

class nes_vt_waixing_state : public nes_vt_state
{
public:
	nes_vt_waixing_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override;
};

class nes_vt_waixing_alt_state : public nes_vt_waixing_state
{
public:
	nes_vt_waixing_alt_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_waixing_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override;
};


class nes_vt_timetp36_state : public nes_vt_state
{
public:
	nes_vt_timetp36_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

protected:
};

class nes_vt_hum_state : public nes_vt_state
{
public:
	nes_vt_hum_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override;
};

class nes_vt_sp69_state : public nes_vt_state
{
public:
	nes_vt_sp69_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override;
};

class nes_vt_ablping_state : public nes_vt_sp69_state
{
public:
	nes_vt_ablping_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_sp69_state(mconfig, type, tag)
	{ }

	void nes_vt_2mb_ablping(machine_config& config);

private:
	void nes_vt_ablping_map(address_map& map);

	DECLARE_READ8_MEMBER(ablping_410f_r);
	DECLARE_WRITE8_MEMBER(ablping_410f_w);
};

class nes_vt_cy_state : public nes_vt_state
{
public:
	nes_vt_cy_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void nes_vt_cy(machine_config& config);

	void nes_vt_cy_bigger(machine_config& config);


	void nes_vt_bt(machine_config& config);
	void nes_vt_bt_2x16mb(machine_config& config);

	void vt_external_space_map_bitboy_2x16mbyte(address_map& map);

private:
	void nes_vt_cy_map(address_map& map);
	void nes_vt_bt_map(address_map& map);

	DECLARE_WRITE8_MEMBER(vt03_41bx_w);
	DECLARE_READ8_MEMBER(vt03_41bx_r);
	DECLARE_WRITE8_MEMBER(vt03_411c_w);
	DECLARE_WRITE8_MEMBER(vt03_412c_extbank_w);

	DECLARE_WRITE8_MEMBER(vt03_48ax_w);
	DECLARE_READ8_MEMBER(vt03_48ax_r);

	DECLARE_WRITE8_MEMBER(vt03_413x_w);
	DECLARE_READ8_MEMBER(vt03_413x_r);
	DECLARE_READ8_MEMBER(vt03_414f_r);
	DECLARE_READ8_MEMBER(vt03_415c_r);

	DECLARE_READ8_MEMBER(vt_rom_banked_r);
};

class nes_vt_cy_lexibook_state : public nes_vt_cy_state
{
public:
	nes_vt_cy_lexibook_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_cy_state(mconfig, type, tag),
		m_previous_port0(0),
		m_latch0_bit(0),
		m_latch1_bit(0)
	{ }

protected:
	virtual DECLARE_READ8_MEMBER(in0_r) override;
	virtual DECLARE_READ8_MEMBER(in1_r) override;
	virtual DECLARE_WRITE8_MEMBER(in0_w) override;

private:
	int m_previous_port0;
	uint8_t m_latch0_bit;
	uint8_t m_latch1_bit;
};


class nes_vt_dg_state : public nes_vt_state
{
public:
	nes_vt_dg_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void nes_vt_dg(machine_config& config);
	void nes_vt_dg_baddma_16mb(machine_config& config);
	void nes_vt_dg_1mb(machine_config& config);

	void nes_vt_fa(machine_config& config);
	void nes_vt_fa_4x16mb(machine_config& config);

protected:
	DECLARE_WRITE8_MEMBER(vtfp_4242_w);

private:
	READ8_MEMBER(vt_rom_banked_r);
	void vt_external_space_map_fapocket_4x16mbyte(address_map& map);

	void nes_vt_dg_map(address_map& map);
	void nes_vt_dg_baddma_map(address_map& map);
	void nes_vt_fa_map(address_map& map);

	DECLARE_READ8_MEMBER(vtfa_412c_r);
	DECLARE_WRITE8_MEMBER(vtfa_412c_extbank_w);
	DECLARE_WRITE8_MEMBER(vt03_411c_w);
};

class nes_vt_dg_fapocket_state : public nes_vt_dg_state
{
public:
	nes_vt_dg_fapocket_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_dg_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override;

private:
};




class nes_vt_hh_state : public nes_vt_dg_state
{
public:
	nes_vt_hh_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_dg_state(mconfig, type, tag)
	{ }

	void nes_vt_hh(machine_config& config);
	void nes_vt_hh_4mb(machine_config& config);
	void nes_vt_hh_8mb(machine_config& config);

	void nes_vt_vg(machine_config& config);
	void nes_vt_vg_4mb(machine_config& config);
	void nes_vt_vg_8mb(machine_config& config);
	void nes_vt_vg_16mb(machine_config& config);


	void nes_vt_vg_1mb_majkon(machine_config& config);
	void nes_vt_fp(machine_config& config);
	void nes_vt_fp_16mb(machine_config& config);
	void nes_vt_fp_32mb(machine_config& config);
	void nes_vt_fp_bigger(machine_config& config);
	void nes_vt_fp_4x16mb(machine_config& config);

	void nes_vt_fp_pal(machine_config& config);
	void nes_vt_fp_pal_32mb(machine_config& config);

private:
	DECLARE_READ8_MEMBER(vt_rom_banked_r);
	void vt_external_space_map_fp_2x32mbyte(address_map& map);

	void nes_vt_hh_map(address_map& map);
	void nes_vt_hh_baddma_map(address_map& map);
	void nes_vt_fp_map(address_map& map);

	DECLARE_WRITE8_MEMBER(vtfp_411e_w);
	DECLARE_WRITE8_MEMBER(vtfp_4a00_w);
	DECLARE_WRITE8_MEMBER(vtfp_412c_extbank_w);
	DECLARE_READ8_MEMBER(vtfp_412d_r);
	DECLARE_READ8_MEMBER(vtfp_4119_r);

	DECLARE_READ8_MEMBER(vthh_414a_r);
	DECLARE_WRITE8_MEMBER(vtfp_411d_w);
};

class nes_vt_ablpinb_state : public nes_vt_state
{
public:
	nes_vt_ablpinb_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag),
		m_ablpinb_in0_val(0),
		m_plunger(*this, "PLUNGER")
	{ }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	virtual DECLARE_READ8_MEMBER(in0_r) override;
	virtual DECLARE_READ8_MEMBER(in1_r) override;
	virtual DECLARE_WRITE8_MEMBER(in0_w) override;

	uint8_t m_ablpinb_in0_val;

	int m_plunger_off;
	int m_plunger_state_count;

	required_ioport m_plunger;
};

class nes_vt_sudoku_state : public nes_vt_state
{
public:
	nes_vt_sudoku_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void init_sudoku();

	void nes_vt_sudoku_512kb(machine_config& config);

protected:
	//virtual void machine_start() override;
	//virtual void machine_reset() override;

private:
	virtual DECLARE_READ8_MEMBER(in0_r) override;
	virtual DECLARE_READ8_MEMBER(in1_r) override;
	virtual DECLARE_WRITE8_MEMBER(in0_w) override;
};

class nes_vt_vg_1mb_majgnc_state : public nes_vt_state
{
public:
	nes_vt_vg_1mb_majgnc_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void nes_vt_vg_1mb_majgnc(machine_config& config);

protected:

private:
};

READ8_MEMBER(nes_vt_state::vt_rom_r)
{
	return m_prgrom[offset];
}

WRITE8_MEMBER(nes_vt_state::vtspace_w)
{
	logerror("%s: vtspace_w %08x : %02x", machine().describe_context(), offset, data);
}

// VTxx can address 25-bit address space (32MB of ROM) so use maps with mirroring in depending on ROM size
void nes_vt_state::vt_external_space_map_32mbyte(address_map &map)
{
	map(0x0000000, 0x1ffffff).rw(FUNC(nes_vt_state::vt_rom_r), FUNC(nes_vt_state::vt03_8000_mapper_w));
}

void nes_vt_state::vt_external_space_map_16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).rw(FUNC(nes_vt_state::vt_rom_r), FUNC(nes_vt_state::vt03_8000_mapper_w));
}

void nes_vt_state::vt_external_space_map_8mbyte(address_map &map)
{
	map(0x0000000, 0x07fffff).mirror(0x1800000).rw(FUNC(nes_vt_state::vt_rom_r), FUNC(nes_vt_state::vt03_8000_mapper_w));
}

void nes_vt_state::vt_external_space_map_4mbyte(address_map &map)
{
	map(0x0000000, 0x03fffff).mirror(0x1c00000).rw(FUNC(nes_vt_state::vt_rom_r), FUNC(nes_vt_state::vt03_8000_mapper_w));
}

void nes_vt_state::vt_external_space_map_2mbyte(address_map &map)
{
	map(0x0000000, 0x01fffff).mirror(0x1e00000).rw(FUNC(nes_vt_state::vt_rom_r), FUNC(nes_vt_state::vt03_8000_mapper_w));
}

void nes_vt_state::vt_external_space_map_1mbyte(address_map &map)
{
	map(0x0000000, 0x00fffff).mirror(0x1f00000).rw(FUNC(nes_vt_state::vt_rom_r), FUNC(nes_vt_state::vt03_8000_mapper_w));
}

void nes_vt_state::vt_external_space_map_512kbyte(address_map &map)
{
	map(0x0000000, 0x007ffff).mirror(0x1f80000).rw(FUNC(nes_vt_state::vt_rom_r), FUNC(nes_vt_state::vt03_8000_mapper_w));
}

// Win Lose Draw has RAM as well as ROM
void nes_vt_swap_op_d5_d6_state::vt_external_space_map_senwld_512kbyte(address_map &map)
{
	map(0x0000000, 0x007ffff).rw(FUNC(nes_vt_swap_op_d5_d6_state::vt_rom_r), FUNC(nes_vt_swap_op_d5_d6_state::vtspace_w));
	map(0x0100000, 0x010ffff).ram();
	map(0x0180000, 0x01fffff).rw(FUNC(nes_vt_swap_op_d5_d6_state::vt_rom_r), FUNC(nes_vt_swap_op_d5_d6_state::vtspace_w));
}

// bitboy is 2 16Mbyte banks
READ8_MEMBER(nes_vt_cy_state::vt_rom_banked_r)
{
	return m_prgrom[m_ahigh | offset];
}

void nes_vt_cy_state::vt_external_space_map_bitboy_2x16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).rw(FUNC(nes_vt_cy_state::vt_rom_banked_r), FUNC(nes_vt_cy_state::vt03_8000_mapper_w));
}

// fapocket is 4 16Mbyte banks
READ8_MEMBER(nes_vt_dg_state::vt_rom_banked_r)
{
	return m_prgrom[m_ahigh | offset];
}

void nes_vt_dg_state::vt_external_space_map_fapocket_4x16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).rw(FUNC(nes_vt_dg_state::vt_rom_banked_r), FUNC(nes_vt_dg_state::vt03_8000_mapper_w));
}

READ8_MEMBER(nes_vt_hh_state::vt_rom_banked_r)
{
	return m_prgrom[m_ahigh | offset];
}

void nes_vt_hh_state::vt_external_space_map_fp_2x32mbyte(address_map &map)
{
	map(0x0000000, 0x1ffffff).rw(FUNC(nes_vt_hh_state::vt_rom_banked_r), FUNC(nes_vt_hh_state::vt03_8000_mapper_w));
}

/* Standard I/O handlers (NES Controller clone) */

READ8_MEMBER(nes_vt_state::in0_r)
{
	//logerror("%s: in0_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch0 & 1;
	m_latch0 >>= 1;
	return ret;
}

READ8_MEMBER(nes_vt_state::in1_r)
{
	//logerror("%s: in1_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch1 & 1;
	m_latch1 >>= 1;
	return ret;
}

WRITE8_MEMBER(nes_vt_state::in0_w)
{
	//logerror("%s: in0_w %02x\n", machine().describe_context(), data);
	if (data & 0x01)
		return;

	m_latch0 = m_io0->read();
	m_latch1 = m_io1->read();
}

/* Lexibook I/O handlers */

READ8_MEMBER(nes_vt_cy_lexibook_state::in0_r)
{
	//logerror("%s: in0_r\n", machine().describe_context());
	uint8_t ret = m_latch0_bit;
	return ret;
}

READ8_MEMBER(nes_vt_cy_lexibook_state::in1_r)
{
	//logerror("%s: in1_r\n", machine().describe_context());
	uint8_t ret = m_latch1_bit;
	return ret;
}

WRITE8_MEMBER(nes_vt_cy_lexibook_state::in0_w)
{
	//logerror("%s: in0_w %02x\n", machine().describe_context(), data);
	if ((!(data & 0x01)) && (m_previous_port0 & 0x01)) // 0x03 -> 0x02 transition
	{
		m_latch0 = m_io0->read();
		m_latch1 = m_io1->read();
	}

	if ((!(data & 0x02)) && (m_previous_port0 & 0x02)) // 0x02 -> 0x00 transition
	{
		m_latch0_bit = m_latch0 & 0x01;
		m_latch0 >>= 1;
		m_latch1_bit = m_latch1 & 0x01;
		m_latch1 >>= 1;
	}

	m_previous_port0 = data;
}


uint32_t nes_vt_state::get_banks(uint8_t bnk)
{
	switch (m_410x[0xb] & 0x07)
	{
	case 0: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xC0) | (bnk & 0x3F)); // makes bank 0xff at 0xe000 map to 0x07e000 by default for vectors at 0x007fffx
	case 1: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xE0) | (bnk & 0x1F));
	case 2: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xF0) | (bnk & 0x0F));
	case 3: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xF8) | (bnk & 0x07));
	case 4: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xFC) | (bnk & 0x03));
	case 5: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xFE) | (bnk & 0x01));
	case 6: return ((m_410x[0x0] & 0xF0) << 4) + (m_410x[0xa]);
	case 7: return ((m_410x[0x0] & 0xF0) << 4) + bnk;
	}

	return 0;
}

// 8000 needs to bank in 60000  ( bank 0x30 )
void nes_vt_state::update_banks()
{
	//uint32_t amod = m_ahigh >> 13;

	uint8_t bank;

	// 8000-9fff
	if ((m_410x[0xb] & 0x40) != 0 || (m_410x[0x5] & 0x40) == 0)
	{
		if ((m_410x[0x5] & 0x40) == 0)
			bank = m_410x[0x7];
		else
			bank = m_410x[0x9];
	}
	else
		bank = 0xfe;

	m_bankaddr[0] = ((/*amod |*/ get_banks(bank)) );

	// a000-bfff
	bank = m_410x[0x8];
	m_bankaddr[1] = ((/*amod |*/ get_banks(bank)) );

	// c000-dfff
	if ((m_410x[0xb] & 0x40) != 0 || (m_410x[0x5] & 0x40) != 0)
	{
		if ((m_410x[0x5] & 0x40) == 0)
			bank = m_410x[0x9];
		else
			bank = m_410x[0x7];
	}
	else
		bank = 0xfe;

	m_bankaddr[2] = ((/*amod |*/ get_banks(bank)) );

	// e000 - ffff
	bank = m_initial_e000_bank;
	m_bankaddr[3] = ((/*amod |*/ get_banks(bank)) );
}

uint16_t nes_vt_state::decode_nt_addr(uint16_t addr)
{
	bool vert_mirror = !(m_410x[0x6] & 0x01);
	int a11 = (addr >> 11) & 0x01;
	int a10 = (addr >> 10) & 0x01;
	uint16_t base = (addr & 0x3FF);
	return ((vert_mirror ? a10 : a11) << 10) | base;
}

WRITE8_MEMBER(nes_vt_state::vt03_410x_w)
{
	scrambled_410x_w(offset, data);
}

READ8_MEMBER(nes_vt_state::vt03_410x_r)
{
	return m_410x[offset];
}

// ablping polls this (also writes here) what is it? 4-bit DAC? PCM? (inputs only start responding once it finishes writing data on startup but takes longer than a sample should)
// (this is the extended IO port on VT)
READ8_MEMBER(nes_vt_ablping_state::ablping_410f_r)
{
	// needs to change at least
	return machine().rand();//0x00;
};

WRITE8_MEMBER(nes_vt_ablping_state::ablping_410f_w)
{
	popmessage("ablping_410f_w %02x", data);
};

// Source: https://wiki.nesdev.com/w/index.php/NES_2.0_submappers/Proposals#NES_2.0_Mapper_256

void nes_vt_state::scrambled_410x_w(uint16_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x0:
		m_410x[0x0] = data;
		update_banks();
		break;

	case 0x1:
		// latch timer value
		m_410x[0x1] = data;
		m_timer_running = 0;
		break;

	case 0x2:
		//logerror("vt03_4102_w %02x\n", data);
		// load latched value and start counting
		m_410x[0x2] = data; // value doesn't matter?
		m_timer_val = m_410x[0x1];
		m_timer_running = 1;
		break;

	case 0x3:
		//logerror("vt03_4103_w %02x\n", data);
		m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
		// disable timer irq
		m_410x[0x3] = data; // value doesn't matter?
		m_timer_irq_enabled = 0;
		break;

	case 0x4:
		//logerror("vt03_4104_w %02x\n", data);
		// enable timer irq
		m_410x[0x4] = data; // value doesn't matter?
		m_timer_irq_enabled = 1;
		break;

	case 0x5:
		logerror("vt03_4105_w %02x\n", data);
		m_410x[0x5] = data;
		update_banks();
		break;

	case 0x6:
		m_410x[0x6] = data;
		break;

	case 0x7:
		m_410x[m_410x_scramble[0]] = data;
		update_banks();
		break;

	case 0x8:
		m_410x[m_410x_scramble[1]] = data;
		update_banks();
		break;

	case 0x9:
		logerror("vt03_4109_w %02x\n", data);
		m_410x[0x9] = data;
		update_banks();
		break;

	case 0xa:
		logerror("vt03_410aw %02x\n", data);
		m_410x[0xa] = data;
		update_banks();
		break;

	case 0xb:
		/*

		D7 TSYNEN - Timer clock select 0:AD12, 1:HSYNC
		D6 Prg Bank 0 Reg 2 enable / disable  0:Disable 1:Enable
		D5 RS232 enable / disable  0:Disable 1:Enable
		D4 Bus output control  0: normal  1: tristate
		D3 6000-7fff and 8000-ffff control - 0 will not active XRWB, 1 will activate
		D2-D0 - program bank 0 selector

		*/

		logerror("vt03_410b_w %02x\n", data);
		m_410x[0xb] = data;
		update_banks();
		break;
	}
}

WRITE8_MEMBER(nes_vt_cy_state::vt03_41bx_w)
{
	logerror("vt03_41bx_w %02x %02x\n", offset, data);
}

WRITE8_MEMBER(nes_vt_dg_state::vt03_411c_w)
{
	logerror("vt03_411c_w  %02x\n", data);
	m_411c = data;
	update_banks();
}

WRITE8_MEMBER(nes_vt_cy_state::vt03_411c_w)
{
	logerror("vt03_411c_w  %02x\n", data);
	m_411c = data;
	update_banks();
}

WRITE8_MEMBER(nes_vt_cy_state::vt03_412c_extbank_w)
{
	// bittboy (ok), mc_pg150 (not working)
	logerror("%s: vt03_412c_extbank_w %02x\n", machine().describe_context(),  data);
	m_ahigh = (data & 0x04) ? (1 << 24) : 0x0;
}

WRITE8_MEMBER(nes_vt_hh_state::vtfp_412c_extbank_w)
{
	// fcpocket
	logerror("%s: vtfp_412c_extbank_w %02x\n", machine().describe_context(), data);
	m_ahigh = (data & 0x01) ? (1 << 25) : 0x0;
}

WRITE8_MEMBER(nes_vt_dg_state::vtfp_4242_w)
{
	logerror("vtfp_4242_w %02x\n", data);
	m_4242 = data;
}

WRITE8_MEMBER(nes_vt_hh_state::vtfp_411d_w)
{
	// controls chram access and mapper emulation modes in later models
	logerror("vtfp_411d_w  %02x\n", data);
	m_411d = data;
	update_banks();
}

WRITE8_MEMBER(nes_vt_dg_state::vtfa_412c_extbank_w)
{
	// fapocket (ok?) (also uses bank from config switch for fake cartridge slot)
	logerror("%s: vtfa_412c_extbank_w %02x\n", machine().describe_context(), data);
	m_ahigh = 0;
	m_ahigh |= (data & 0x01) ? (1 << 25) : 0x0;
	m_ahigh |= (data & 0x02) ? (1 << 24) : 0x0;
}

READ8_MEMBER(nes_vt_dg_state::vtfa_412c_r)
{
	if (m_cartsel)
		return m_cartsel->read();
	else
		return 0;
}

READ8_MEMBER(nes_vt_hh_state::vtfp_412d_r)
{
	if (m_cartsel)
		return m_cartsel->read();
	else
		return 0;
}

READ8_MEMBER(nes_vt_cy_state::vt03_41bx_r)
{
	switch (offset)
	{
	case 0x07:
		return 0x04;
	default:
		return 0x00;
	}
}

WRITE8_MEMBER(nes_vt_cy_state::vt03_48ax_w)
{
	logerror("vt03_48ax_w %02x %02x\n", offset, data);
}

READ8_MEMBER(nes_vt_cy_state::vt03_48ax_r)
{
	switch (offset)
	{
	case 0x04:
		return 0x01;
	case 0x05:
		return 0x01;
	default:
		return 0x00;
	}
}

WRITE8_MEMBER(nes_vt_cy_state::vt03_413x_w)
{
	logerror("vt03_413x_w %02x %02x\n", offset, data);
	// VT168 style ALU ??
	m_413x[offset] = data;
	if (offset == 0x5)
	{
		uint32_t res = uint32_t((m_413x[5] << 8) | m_413x[4]) * uint32_t((m_413x[1] << 8) | m_413x[0]);
		m_413x[0] = res & 0xFF;
		m_413x[1] = (res >> 8) & 0xFF;
		m_413x[2] = (res >> 16) & 0xFF;
		m_413x[3] = (res >> 24) & 0xFF;
		m_413x[6] = 0x00;

	}
	else if (offset == 0x6)
	{
		/*uint32_t res = uint32_t((m_413x[5] << 8) | m_413x[4]) * uint32_t((m_413x[1] << 8) | m_413x[0]);
		m_413x[0] = res & 0xFF;
		m_413x[1] = (res >> 8) & 0xFF;
		m_413x[2] = (res >> 16) & 0xFF;
		m_413x[3] = (res >> 24) & 0xFF;*/
		m_413x[6] = 0x00;
	}
}


READ8_MEMBER(nes_vt_cy_state::vt03_413x_r)
{
	logerror("vt03_413x_r %02x\n", offset);
	return m_413x[offset];
}

READ8_MEMBER(nes_vt_cy_state::vt03_414f_r)
{
	return 0xff;
}

READ8_MEMBER(nes_vt_cy_state::vt03_415c_r)
{
	return 0xff;
}

READ8_MEMBER(nes_vt_hh_state::vthh_414a_r)
{
	return 0x80;
}


READ8_MEMBER(nes_vt_state::spr_r)
{
	if (m_4242 & 0x1 || m_411d & 0x04)
	{
		return m_chrram[offset];
	}
	else
	{
		int realaddr = calculate_real_video_address(offset, 0, 1);

		return m_vt_external_space->read8(realaddr);
	}
}

READ8_MEMBER(nes_vt_state::chr_r)
{
	if (m_4242 & 0x1 || m_411d & 0x04)
	{
		return m_chrram[offset];
	}
	else
	{
		int realaddr = calculate_real_video_address(offset, 1, 0);
		return m_vt_external_space->read8(realaddr);
	}
}


WRITE8_MEMBER(nes_vt_state::chr_w)
{
	if (m_4242 & 0x1 || m_411d & 0x04)
	{
		logerror("vram write %04x %02x\n", offset, data);
		m_chrram[offset] = data;
	}
}

WRITE8_MEMBER(nes_vt_hh_state::vtfp_411e_w)
{
	logerror("411e_w %02x\n", data);
	if (data == 0x05)
		dynamic_cast<m6502_vtscr&>(*m_maincpu).set_next_scramble(true);
	else if (data == 0x00)
		dynamic_cast<m6502_vtscr&>(*m_maincpu).set_next_scramble(false);
}

WRITE8_MEMBER(nes_vt_hh_state::vtfp_4a00_w)
{
	logerror("4a00_w %02x\n", data);
	//if(data == 0x80)
	//  dynamic_cast<m6502_vtscr&>(*m_maincpu).set_scramble(false);
}


void nes_vt_state::scanline_irq(int scanline, int vblank, int blanked)
{
	video_irq(false, scanline, vblank, blanked);
}

void nes_vt_state::hblank_irq(int scanline, int vblank, int blanked)
{
	video_irq(true, scanline, vblank, blanked);
}

void nes_vt_state::video_irq(bool hblank, int scanline, int vblank, int blanked)
{
	//TSYNEN
	if (((m_410x[0xb] >> 7) & 0x01) == hblank)
	{
		int irqstate = 0;

		//logerror("scanline_irq %d\n", scanline);

		if (m_timer_running && scanline < 0xe0)
		{
			m_timer_val--;

			if (m_timer_val < 0)
			{
				if (m_timer_irq_enabled && !blanked)
				{
					logerror("scanline_irq %d\n", scanline);
					irqstate = 1;
				}
			}
		}

		if (irqstate)
			m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
		//else
		//  m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	}
}

/* todo, handle custom VT nametable stuff here */
READ8_MEMBER(nes_vt_state::nt_r)
{
	return m_ntram[decode_nt_addr(offset)];
}

WRITE8_MEMBER(nes_vt_state::nt_w)
{
	//logerror("nt wr %04x %02x", offset, data);
	m_ntram[decode_nt_addr(offset)] = data;
}

void nes_vt_state::machine_start()
{
	save_item(NAME(m_410x));
	save_item(NAME(m_413x));

	save_item(NAME(m_411c));
	save_item(NAME(m_411d));
	save_item(NAME(m_4242));

	save_item(NAME(m_8000_addr_latch));

	save_item(NAME(m_timer_irq_enabled));
	save_item(NAME(m_timer_running));
	save_item(NAME(m_timer_val));
	save_item(NAME(m_vdma_ctrl));

	m_ntram = std::make_unique<uint8_t[]>(0x2000);
	save_pointer(NAME(m_ntram), 0x2000);

	m_chrram = std::make_unique<uint8_t[]>(0x2000);
	save_pointer(NAME(m_chrram), 0x2000);

	m_ppu->set_scanline_callback(*this, FUNC(nes_vt_state::scanline_irq));
	m_ppu->set_hblank_callback(*this, FUNC(nes_vt_state::hblank_irq));

	//m_ppu->set_hblank_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::hblank_irq)));
	//m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::chr_r)), write8_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::chr_w)));
	m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8_delegate(*this, FUNC(nes_vt_state::nt_r)), write8_delegate(*this, FUNC(nes_vt_state::nt_w)));
	m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8_delegate(*this, FUNC(nes_vt_state::chr_r)), write8_delegate(*this, FUNC(nes_vt_state::chr_w)));
}

void nes_vt_state::machine_reset()
{
	// what are the actual defaults?
	m_410x[0x0] = 0x00;
	m_410x[0x1] = 0x00;
	m_410x[0x2] = 0x00;
	m_410x[0x3] = 0x00;
	m_410x[0x4] = 0x00;
	m_410x[0x5] = 0x00;
	m_410x[0x6] = 0x00;
	m_410x[0x7] = 0x00;
	m_410x[0x8] = 0x01;
	m_410x[0x9] = 0x02;
	m_410x[0xa] = 0x00;
	m_410x[0xb] = 0x00;
	m_411c = 0x00;
	m_411d = 0x00;
	m_4242 = 0x00;

	m_timer_irq_enabled = 0;
	m_timer_running = 0;
	m_timer_val = 0;
	m_vdma_ctrl = 0;

	update_banks();

	// 'no scramble' configuration
	m_8000_scramble[0x0] = 0x6;
	m_8000_scramble[0x1] = 0x7;
	m_8000_scramble[0x2] = 0x2;
	m_8000_scramble[0x3] = 0x3;
	m_8000_scramble[0x4] = 0x4;
	m_8000_scramble[0x5] = 0x5;
	m_8000_scramble[0x6] = 0x7;
	m_8000_scramble[0x7] = 0x8;

	m_410x_scramble[0x0] = 0x7;
	m_410x_scramble[0x1] = 0x8;
}

void nes_vt_dg_fapocket_state::machine_reset()
{
	nes_vt_state::machine_reset();

	// fapocket needs this, fcpocket instead reads the switch in software?
	if (m_cartsel)
		m_ahigh = (m_cartsel->read() == 0x01) ? (1 << 25) : 0x0;
	else
		m_ahigh = 0;
}

int nes_vt_state::calculate_real_video_address(int addr, int extended, int readtype)
{
	// might be a VT09 only feature (alt 4bpp mode?)
	int alt_order = m_ppu->get_201x_reg(0x0) & 0x40;

	if (readtype == 0)
	{
		if (m_ppu->get_201x_reg(0x0) & 0x10)
		{
			extended = 1;
		}
		else
		{
			extended = 0;
		}
	}
	else if (readtype == 1)
	{
		if (m_ppu->get_201x_reg(0x0) & 0x08)
		{
			extended = 1;
		}
		else
		{
			extended = 0;
		}
	}

	/*
	Calculating TVA17 - TVA10

	--------------------------------------------------------------------------------------------
	| COMR7        | AD[12:10] | TVA17 | TVA16 | TVA15 | TVA14 | TVA13 | TVA12 | TVA11 | TVA10 |
	| (4105, 0x80) | (2006)    |       |       |       |       |       |       |       |       |
	--------------------------------------------------------------------------------------------
	| 0/1/C/D                  | RV47  | RV46  | RV45  | RV44  | RV43  | RV42  | RV41  | AD10  | ** RV40 is never used
	| 2/3/E/F                  | RV57  | RV56  | RV55  | RV54  | RV53  | RV52  | RV51  | AD10  | ** RV50 is never used
	| 4/8                      | RV07  | RV06  | RV05  | RV04  | RV03  | RV02  | RV01  | RV00  |
	| 5/9                      | RV17  | RV16  | RV15  | RV14  | RV13  | RV12  | RV11  | RV10  |
	| 6/A                      | RV27  | RV26  | RV25  | RV24  | RV23  | RV22  | RV21  | RV20  |
	| 7/B                      | RV37  | RV36  | RV35  | RV34  | RV33  | RV32  | RV31  | RV30  |
	--------------------------------------------------------------------------------------------

	m_r2012 = rv0x
	m_r2013 = rv1x
	m_r2014 = rv2x
	m_r2015 = rv3x
	m_r2016 = rv4x
	m_r2017 = rv5x

	*/
	int finaladdr = 0;

	int sel = (addr & 0x1c00) | ((m_410x[0x5] & 0x80) ? 0x2000 : 0x000);

	int vbank_tva17_tva10 = 0x00;

	switch ((sel >> 10) & 0xf)
	{
	case 0x0:
	case 0x1:
	case 0xc:
	case 0xd:
		vbank_tva17_tva10 = (m_ppu->get_201x_reg(0x6) & 0xfe) | ((addr & 0x0400) ? 1 : 0);
		break;

	case 0x2:
	case 0x3:
	case 0xe:
	case 0xf:
		vbank_tva17_tva10 = (m_ppu->get_201x_reg(0x7) & 0xfe) | ((addr & 0x0400) ? 1 : 0);
		break;

	case 0x4:
	case 0x8:
		vbank_tva17_tva10 = m_ppu->get_201x_reg(0x2);
		break;

	case 0x5:
	case 0x9:
		vbank_tva17_tva10 = m_ppu->get_201x_reg(0x3);
		break;

	case 0x6:
	case 0xa:
		vbank_tva17_tva10 = m_ppu->get_201x_reg(0x4);
		break;

	case 0x7:
	case 0xb:
		vbank_tva17_tva10 = m_ppu->get_201x_reg(0x5);
		break;

	}

	/*
	Calculating VA17 - VA10 (requires TVA17-TVA10 to have been calculated)

	------------------------------------------------------------------------------
	|  VB0S[2:0] |   VA[17:10]                                                   |
	| 201a & 0x7 |  VA17 |  VA16 |  VA15 |  VA14 |  VA13 |  VA12 |  VA11 |  VA10 |
	|-----------------------------------------------------------------------------
	| 0x0        | TVA17 | TVA16 | TVA15 | TVA14 | TVA13 | TVA12 | TVA11 | TVA10 |
	| 0x1        |  TV67 | TVA16 | TVA15 | TVA14 | TVA13 | TVA12 | TVA11 | TVA10 |
	| 0x2        |  RV67 |  RV66 | TVA15 | TVA14 | TVA13 | TVA12 | TVA11 | TVA10 |
	| 0x3        | INVALID ***************************************************** |
	| 0x4        |  RV67 |  RV66 |  RV65 | TVA14 | TVA13 | TVA12 | TVA11 | TVA10 |
	| 0x5        |  RV67 |  RV66 |  RV65 |  RV64 | TVA13 | TVA12 | TVA11 | TVA10 |
	| 0x6        |  RV67 |  RV66 |  RV65 |  RV64 |  RV63 | TVA12 | TVA11 | TVA10 |
	| 0x7        | INVALID ***************************************************** |
	------------------------------------------------------------------------------

	RV67- RV63 = 0x201a & 0xf8

	*/

	int va17_va10 = 0;

	int swit = m_ppu->get_201x_reg(0xa);

	switch (swit & 0x07)
	{
	case 0x0: va17_va10 = vbank_tva17_tva10; break;
	case 0x1: va17_va10 = (vbank_tva17_tva10 & 0x7f) | (m_ppu->get_201x_reg(0xa) & 0x80); break;
	case 0x2: va17_va10 = (vbank_tva17_tva10 & 0x3f) | (m_ppu->get_201x_reg(0xa) & 0xc0); break;
	case 0x3: return -1;
	case 0x4: va17_va10 = (vbank_tva17_tva10 & 0x1f) | (m_ppu->get_201x_reg(0xa) & 0xe0); break;
	case 0x5: va17_va10 = (vbank_tva17_tva10 & 0x0f) | (m_ppu->get_201x_reg(0xa) & 0xf0); break;
	case 0x6: va17_va10 = (vbank_tva17_tva10 & 0x07) | (m_ppu->get_201x_reg(0xa) & 0xf8); break;
	case 0x7: return -1;
	}

	int va34 = m_ppu->get_va34();

	if (!extended)
	{
		int is4bpp = 0;
		if (readtype == 0) is4bpp = m_ppu->get_201x_reg(0x0) & 0x02;
		else if (readtype == 1) is4bpp = m_ppu->get_201x_reg(0x0) & 0x04;

		int va20_va18 = (m_ppu->get_201x_reg(0x8) & 0x70) >> 4;

		finaladdr = ((m_410x[0x0] & 0x0F) << 21) | (va20_va18 << 18) | (va17_va10 << 10) | (addr & 0x03ff);

		if (is4bpp)
		{
			if (!alt_order)
			{
				finaladdr = ((finaladdr & ~0xf) << 1) | (va34 << 4) | (finaladdr & 0xf);
			}
			else
			{
				finaladdr = (finaladdr << 1) | va34;
			}
		}
	}
	else
	{
		int eva2_eva0 = 0x00;
		int is4bpp = 0;

		switch (readtype)
		{
		case 0: // background display
			is4bpp = m_ppu->get_201x_reg(0x0) & 0x02;

			eva2_eva0 |= m_ppu->get_m_read_bg4_bg3();

			if (m_ppu->get_201x_reg(0x1) & 0x02)
			{
				if (m_410x[0x6] & 0x1) eva2_eva0 |= 0x4;
			}
			else
			{
				if (m_ppu->get_201x_reg(0x8) & 0x08) eva2_eva0 |= 0x4;
			}
			break;

		case 1: // sprite display
			is4bpp = m_ppu->get_201x_reg(0x0) & 0x04; // 16 colors or 16-pixel wide (both adjust the read)

			eva2_eva0 |= m_ppu->get_speva2_speva0();

			break;

		case 2: // CPU R/W access
			// todo
			break;
		}

		finaladdr = ((m_410x[0x0] & 0x0f) << 21) | (va17_va10 << 13) | (eva2_eva0 << 10) | (addr & 0x03ff);

		if (is4bpp)
		{
			if (!alt_order)
			{
				finaladdr = ((finaladdr & ~0xf) << 1) | (va34 << 4) | (finaladdr & 0xf);
			}
			else
			{
				finaladdr = (finaladdr << 1) | va34;
			}

		}
	}
	return /*m_ahigh |*/ finaladdr;
}

/*
   nes_vt_state::vt03_8000_mapper_w notes

     used for MMC3/other mapper compatibility
     some consoles have scrambled registers for crude copy protection

	is this always there with VT based games? it maps where mappers would be on a NES cartridge
	but then seems to be able to alter internal state of extended PPU registers, which is awkward
*/

void nes_vt_state::scrambled_8000_w(address_space& space, uint16_t offset, uint8_t data)
{
	offset &= 0x7fff;

	uint16_t addr = m_real_access_address; // we need the actual write address, not the translated one, to keep bittboy happy
	if ((m_411d & 0x01) && (m_411d & 0x03))
	{
		//CNROM compat
		logerror("%s: vtxx_cnrom_8000_w real address: (%04x) translated address: (%04x) %02x\n", machine().describe_context(), addr, offset + 0x8000, data);
		m_ppu->set_201x_reg(0x6, data * 8);
		m_ppu->set_201x_reg(0x7, data * 8 + 2);
		m_ppu->set_201x_reg(0x2, data * 8 + 4);
		m_ppu->set_201x_reg(0x3, data * 8 + 5);
		m_ppu->set_201x_reg(0x4, data * 8 + 6);
		m_ppu->set_201x_reg(0x5, data * 8 + 7);

	}
	else if (m_411d & 0x01)
	{
		//MMC1 compat, TODO
		logerror("%s: vtxx_mmc1_8000_w real address: (%04x) translated address: (%04x) %02x\n", machine().describe_context(), addr, offset + 0x8000, data);

	}
	else if (m_411d & 0x02)
	{
		//UNROM compat
		logerror("%s: vtxx_unrom_8000_w real address: (%04x) translated address: (%04x) %02x\n", machine().describe_context(), addr, offset + 0x8000, data);

		m_410x[0x7] = ((data & 0x0F) << 1);
		m_410x[0x8] = ((data & 0x0F) << 1) + 1;
		update_banks();
	}
	else
	{
		//logerror("%s: vtxx_mmc3_8000_w real address: (%04x) translated address: (%04x) %02x\n",  machine().describe_context(), addr, offset+0x8000, data );

		//MMC3 compat
		if ((addr < 0xA000) && !(addr & 0x01))
		{
			logerror("%s: scrambled_8000_w real address: (%04x) translated address: (%04x) %02x (banking)\n",  machine().describe_context(), addr, offset + 0x8000, data);
			// Bank select
			m_8000_addr_latch = data & 0x07;
			// Bank config
			m_410x[0x05] = data & ~(1 << 5);
			update_banks();
		}
		else if ((addr < 0xA000) && (addr & 0x01))
		{
			logerror("%s: scrambled_8000_w real address: (%04x) translated address: (%04x) %02x (other scrambled stuff)\n",  machine().describe_context(), addr, offset + 0x8000, data);

			switch (m_410x[0x05] & 0x07)
			{
			case 0x00:
				m_ppu->set_201x_reg(m_8000_scramble[0], data);
				break;

			case 0x01:
				m_ppu->set_201x_reg(m_8000_scramble[1], data);
				break;

			case 0x02: // hand?
				m_ppu->set_201x_reg(m_8000_scramble[2], data);
				break;

			case 0x03: // dog?
				m_ppu->set_201x_reg(m_8000_scramble[3], data);
				break;

			case 0x04: // ball thrown
				m_ppu->set_201x_reg(m_8000_scramble[4], data);
				break;

			case 0x05: // ball thrown
				m_ppu->set_201x_reg(m_8000_scramble[5], data);
				break;
			case 0x06:
				m_410x[m_8000_scramble[6]] = data;
				//m_410x[0x9] = data;
				update_banks();
				break;

			case 0x07:
				m_410x[m_8000_scramble[7]] = data;
				update_banks();
				break;
			}
		}
		else if ((addr >= 0xA000) && (addr < 0xC000) && !(addr & 0x01))
		{
			// Mirroring
			m_410x[0x6] &= 0xFE;
			m_410x[0x6] |= data & 0x01;
		}
		else if ((addr >= 0xA000) && (addr < 0xC000) && (addr & 0x01))
		{
			// PRG RAM control, ignore
		}
		else if ((addr >= 0xC000) && (addr < 0xE000) && !(addr & 0x01))
		{
			// IRQ latch
			vt03_410x_w(space, 1, data);
		}
		else if ((addr >= 0xC000) && (addr < 0xE000) && (addr & 0x01))
		{
			// IRQ reload
			vt03_410x_w(space, 2, data);
		}
		else if ((addr >= 0xE000) && !(addr & 0x01))
		{
			// IRQ disable
			vt03_410x_w(space, 3, data);
		}
		else if ((addr >= 0xE000) && (addr & 0x01))
		{
			// IRQ enable
			vt03_410x_w(space, 4, data);
		}
		else
		{

		}
	}
}

// MMC3 compatibility mode

void nes_vt_state::set_8000_scramble(uint8_t reg0, uint8_t reg1, uint8_t reg2, uint8_t reg3, uint8_t reg4, uint8_t reg5, uint8_t reg6, uint8_t reg7)
{
	m_8000_scramble[0] = reg0; // TODO: name the regs
	m_8000_scramble[1] = reg1;
	m_8000_scramble[2] = reg2;
	m_8000_scramble[3] = reg3;
	m_8000_scramble[4] = reg4;
	m_8000_scramble[5] = reg5;
	m_8000_scramble[6] = reg6;
	m_8000_scramble[7] = reg7;
}

void nes_vt_state::set_410x_scramble(uint8_t reg0, uint8_t reg1)
{
	m_410x_scramble[0] = reg0; // TODO: name the regs
	m_410x_scramble[1] = reg1;
}

WRITE8_MEMBER(nes_vt_state::vt03_8000_mapper_w)
{
	scrambled_8000_w(space, offset, data);
	//logerror("%s: vt03_8000_mapper_w (%04x) %02x\n", machine().describe_context(), offset+0x8000, data );
}

/* APU plumbing, this is because we have a plain M6502 core in the VT03, otherwise this is handled in the core */

READ8_MEMBER(nes_vt_state::psg1_4014_r)
{
	//return m_apu->read(0x14);
	return 0x00;
}

READ8_MEMBER(nes_vt_state::psg1_4015_r)
{
	return m_apu->read(0x15);
}

WRITE8_MEMBER(nes_vt_state::psg1_4015_w)
{
	m_apu->write(0x15, data);
}

WRITE8_MEMBER(nes_vt_state::psg1_4017_w)
{
	m_apu->write(0x17, data);
}

// early units (VT03?) have a DMA bug in NTSC mode
WRITE8_MEMBER(nes_vt_state::vt_dma_w)
{
	do_dma(data, true);
}

// later units (VT09?) don't appear to have the NTSC DMA bug? (or something else is incorrectly compensating for it in our emulation)
WRITE8_MEMBER(nes_vt_state::vt_fixed_dma_w)
{
	do_dma(data, false);
}


void nes_vt_state::do_dma(uint8_t data, bool has_ntsc_bug)
{
	// only NTSC systems have 'broken' DMA which requires the DMA addresses to be shifted by 1, PAL systems work as expected
	if (m_ppu->get_is_pal())
		has_ntsc_bug = false;

	uint8_t dma_mode = m_vdma_ctrl & 0x01;
	uint8_t dma_len = (m_vdma_ctrl >> 1) & 0x07;
	uint8_t src_nib_74 = (m_vdma_ctrl >> 4) & 0x0F;

	int length = 256;
	switch (dma_len)
	{
	case 0x0: length = 256; break;
	case 0x4: length = 16; break;
	case 0x5: length = 32; break;
	case 0x6: length = 64; break;
	case 0x7: length = 128; break;
	}

	uint16_t src_addr = (data << 8) | (src_nib_74 << 4);
	logerror("vthh dma start ctrl=%02x addr=%04x\n", m_vdma_ctrl, src_addr);

	if (dma_mode == 1)
	{
		logerror("vdma dest %04x\n", m_ppu->get_vram_dest());
	}

	if (has_ntsc_bug && (dma_mode == 1) && ((m_ppu->get_vram_dest() & 0xFF00) == 0x3F00) && !(m_ppu->get_201x_reg(0x1) & 0x80))
	{
		length -= 1;
		src_addr += 1;
	}
	else if ((dma_mode == 1) && ((m_ppu->get_vram_dest() & 0xFF00) == 0x3F01) && !(m_ppu->get_201x_reg(0x1) & 0x80))
	{
		// Legacy mode for DGUN-2573 compat
		m_ppu->set_vram_dest(0x3F00);
		m_ppu->set_palette_mode(PAL_MODE_VT0x);
	}

	for (int i = 0; i < length; i++)
	{
		uint8_t spriteData = m_maincpu->space(AS_PROGRAM).read_byte(src_addr + i);
		if (dma_mode)
		{
			m_maincpu->space(AS_PROGRAM).write_byte(0x2007, spriteData);
		}
		else
		{
			m_maincpu->space(AS_PROGRAM).write_byte(0x2004, spriteData);
		}
		//if(((src_addr + i) & 0xFF) == length && (i != 0)) break;
	}

	// should last (length * 4 - 1) CPU cycles.
	//((device_t*)m_maincpu)->execute().adjust_icount(-(length * 4 - 1));
}


WRITE8_MEMBER(nes_vt_state::vt03_4034_w)
{
	logerror("vt03_4034_w %02x\n", data);
	m_vdma_ctrl = data;
}

WRITE8_MEMBER(nes_vt_state::extra_io_control_w)
{
	/*
	410d Extra I/O control

	0x01 Extra I/O port 0 mode (1 = output, 0 = input)
	0x02 Extra I/O port 0 enable (1 = enable, 0 = disable)
	0x04 Extra I/O port 1 mode (1 = output, 0 = input)
	0x08 Extra I/O port 1 enable (1 = enable, 0 = disable)
	0x10 Extra I/O port 2 mode (1 = output, 0 = input)
	0x20 Extra I/O port 2 enable (1 = enable, 0 = disable)
	0x40 Extra I/O port 3 mode (1 = output, 0 = input)
	0x80 Extra I/O port 3 enable (1 = enable, 0 = disable)
	*/

	logerror("%s: extra_io_control_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(nes_vt_state::extrain_01_r)
{
	// TODO: check status of 410d port to make sure we only read from enabled ports
	uint8_t in0 = 0x00, in1 = 0x00;
	if (m_exin0)
		in0 = m_exin0->read() & 0x0f;
	if (m_exin1)
		in1 = m_exin1->read() & 0x0f;

	return in0 | (in1<<4);
}

READ8_MEMBER(nes_vt_state::extrain_23_r)
{
	// TODO: check status of 410d port to make sure we only read from enabled ports
	uint8_t in2 = 0x00, in3 = 0x00;
	if (m_exin2)
		in2 = m_exin2->read() & 0x0f;
	if (m_exin3)
		in3 = m_exin3->read() & 0x0f;

	return in2 | (in3<<4);
}

WRITE8_MEMBER(nes_vt_state::extraout_01_w)
{
	// TODO: use callbacks for this as output can be hooked up to anything
	logerror("%s: extraout_01_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(nes_vt_state::extraout_23_w)
{
	// TODO: use callbacks for this as output can be hooked up to anything
	logerror("%s: extraout_23_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(nes_vt_state::rs232flags_region_r)
{
	/*
	0x4119 RS232 Flags + Region

	0x01 - RX bit 8
	0x02 - RERFF (error status)
	0x04 - unused
	0x08 - XPORN (PAL = 1 NTSC = 0)
	0x10 - XF5OR6 (50hz = 1 60hz = 0)
	0x20 - RINGF (receive status)
	0x40 - TIFLAG (completed sending data status)
	0x80 - RIFLAG (completed receiving data status)
	*/
	uint8_t ret = 0x00;

	// Palette DMA is buggy on NTSC systems (at least for regular VT03)
	// so the palette DMA writes will change based on the reading of these flags

	ret |= m_ppu->get_is_pal() ? 0x08 : 0x00;
	ret |= m_ppu->get_is_50hz() ? 0x10 : 0x00;

	return ret;
}

void nes_vt_ablpinb_state::machine_start()
{
	nes_vt_state::machine_start();

	save_item(NAME(m_plunger_off));
	save_item(NAME(m_plunger_state_count));
	save_item(NAME(m_ablpinb_in0_val));

}

void nes_vt_ablpinb_state::machine_reset()
{
	nes_vt_state::machine_reset();

	m_plunger_off = 0;
	m_plunger_state_count = 0;
	m_ablpinb_in0_val = 0;
}


READ8_MEMBER(nes_vt_ablpinb_state::in0_r)
{
	if (m_plunger_off)
	{
		m_plunger_state_count++;

		if (m_plunger_state_count == 5) // make sure it's high for enough reads to keep the code flowing
		{
			m_plunger_off = 0;
			m_plunger_state_count = 0;
		}
	}
	else
	{
		m_plunger_state_count++;

		if ((m_plunger_state_count >= m_plunger->read()) || (m_plunger_state_count >= 0x80)) // if it stays low for too many frames the gfx corrupt,
		{
			m_plunger_off = 1;
			m_plunger_state_count = 0;
		}
	}

	uint8_t ret = m_io0->read() & ~0x01;

	return m_plunger_off | ret;
}


READ8_MEMBER(nes_vt_ablpinb_state::in1_r)
{
	uint8_t i = machine().rand() & 0x10;

	// maybe this transition takes some time in reality?
	i |= (m_ablpinb_in0_val & 0x04) ? 0x00 : 0x08;

	uint8_t ret = m_io1->read() & ~0x18;

	return i | ret;
}

WRITE8_MEMBER(nes_vt_ablpinb_state::in0_w)
{
	// write 0x04 to 0x4016 sets bit 0x08 in 0x4017
	// write 0x00 to 0x4016 clears bit 0x08 in 0x4017
	// could be related to vibration motor?

	m_ablpinb_in0_val = data;
	logerror("in0_w %02x\n", data);
}

READ8_MEMBER(nes_vt_sudoku_state::in0_r)
{
	return machine().rand();
}

READ8_MEMBER(nes_vt_sudoku_state::in1_r)
{
	return machine().rand();
}

WRITE8_MEMBER(nes_vt_sudoku_state::in0_w)
{
}

void nes_vt_state::nes_vt_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	// ddrdismx relies on the mirroring
	map(0x2000, 0x2007).mirror(0x00e0).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));                      /* standard PPU registers */
	map(0x2010, 0x201f).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::read_extended), FUNC(ppu_vt03_device::write_extended));   /* extra VT PPU registers */

	map(0x4000, 0x4013).rw(m_apu, FUNC(nesapu_device::read), FUNC(nesapu_device::write));
	map(0x4014, 0x4014).r(FUNC(nes_vt_state::psg1_4014_r)).w(FUNC(nes_vt_state::vt_dma_w));
	map(0x4015, 0x4015).rw(FUNC(nes_vt_state::psg1_4015_r), FUNC(nes_vt_state::psg1_4015_w)); /* PSG status / first control register */
	map(0x4016, 0x4016).rw(FUNC(nes_vt_state::in0_r), FUNC(nes_vt_state::in0_w));
	map(0x4017, 0x4017).r(FUNC(nes_vt_state::in1_r)).w(FUNC(nes_vt_state::psg1_4017_w));

	map(0x4034, 0x4034).w(FUNC(nes_vt_state::vt03_4034_w));

	map(0x4100, 0x410b).r(FUNC(nes_vt_state::vt03_410x_r)).w(FUNC(nes_vt_state::vt03_410x_w));
	// 0x410c unused
	map(0x410d, 0x410d).w(FUNC(nes_vt_state::extra_io_control_w));
	map(0x410e, 0x410e).rw(FUNC(nes_vt_state::extrain_01_r), FUNC(nes_vt_state::extraout_01_w));
	map(0x410f, 0x410f).rw(FUNC(nes_vt_state::extrain_23_r), FUNC(nes_vt_state::extraout_23_w));
	// 0x4114 RS232 timer (low)
	// 0x4115 RS232 timer (high)
	// 0x4116 unused
	// 0x4117 unused
	// 0x4118 unused
	map(0x4119, 0x4119).r(FUNC(nes_vt_state::rs232flags_region_r));
	// 0x411a RS232 TX data
	// 0x411b RS232 RX data

	map(0x8000, 0xffff).rw(FUNC(nes_vt_state::external_space_read), FUNC(nes_vt_state::external_space_write));
	map(0x6000, 0x7fff).ram();
}

/* Some later VT models have more RAM */
void nes_vt_state::nes_vt_4k_ram_map(address_map &map)
{
	nes_vt_map(map);
	map(0x0800, 0x0fff).ram();
}

void nes_vt_ablping_state::nes_vt_ablping_map(address_map &map)
{
	nes_vt_map(map);
	map(0x410f, 0x410f).rw(FUNC(nes_vt_ablping_state::ablping_410f_r), FUNC(nes_vt_ablping_state::ablping_410f_w));
}

void nes_vt_cy_state::nes_vt_cy_map(address_map &map)
{
	nes_vt_4k_ram_map(map);
	map(0x41b0, 0x41bf).r(FUNC(nes_vt_cy_state::vt03_41bx_r)).w(FUNC(nes_vt_cy_state::vt03_41bx_w));
	map(0x48a0, 0x48af).r(FUNC(nes_vt_cy_state::vt03_48ax_r)).w(FUNC(nes_vt_cy_state::vt03_48ax_w));
	map(0x4130, 0x4136).r(FUNC(nes_vt_cy_state::vt03_413x_r)).w(FUNC(nes_vt_cy_state::vt03_413x_w));
	map(0x414F, 0x414F).r(FUNC(nes_vt_cy_state::vt03_414f_r));
	map(0x415C, 0x415C).r(FUNC(nes_vt_cy_state::vt03_415c_r));

}

void nes_vt_cy_state::nes_vt_bt_map(address_map &map)
{
	nes_vt_4k_ram_map(map);
	map(0x412c, 0x412c).w(FUNC(nes_vt_cy_state::vt03_412c_extbank_w));
}


void nes_vt_hh_state::nes_vt_hh_map(address_map &map)
{
	map(0x0000, 0x1fff).mask(0x0fff).ram();
	map(0x2000, 0x2007).mirror(0x00e0).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));                      /* standard PPU registers */
	map(0x2010, 0x201f).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::read_extended), FUNC(ppu_vt03_device::write_extended));   /* extra VT PPU registers */

	map(0x4000, 0x4013).rw(m_apu, FUNC(nesapu_device::read), FUNC(nesapu_device::write));
	map(0x4015, 0x4015).rw(FUNC(nes_vt_hh_state::psg1_4015_r), FUNC(nes_vt_hh_state::psg1_4015_w)); /* PSG status / first control register */
	map(0x4016, 0x4016).rw(FUNC(nes_vt_hh_state::in0_r), FUNC(nes_vt_hh_state::in0_w));
	map(0x4017, 0x4017).r(FUNC(nes_vt_hh_state::in1_r)).w(FUNC(nes_vt_hh_state::psg1_4017_w));

	map(0x4100, 0x410b).r(FUNC(nes_vt_hh_state::vt03_410x_r)).w(FUNC(nes_vt_hh_state::vt03_410x_w));

	map(0x8000, 0xffff).rw(FUNC(nes_vt_hh_state::external_space_read), FUNC(nes_vt_hh_state::external_space_write));

	map(0x4034, 0x4034).w(FUNC(nes_vt_hh_state::vt03_4034_w));
	map(0x4014, 0x4014).r(FUNC(nes_vt_hh_state::psg1_4014_r)).w(FUNC(nes_vt_hh_state::vt_fixed_dma_w));

	map(0x4119, 0x4119).r(FUNC(nes_vt_hh_state::rs232flags_region_r));
	map(0x414a, 0x414a).r(FUNC(nes_vt_hh_state::vthh_414a_r));
	map(0x411d, 0x411d).w(FUNC(nes_vt_hh_state::vtfp_411d_w));

	map(0x6000, 0x7fff).ram();
}

void nes_vt_hh_state::nes_vt_hh_baddma_map(address_map &map)
{
	nes_vt_hh_map(map);
	map(0x4014, 0x4014).w(FUNC(nes_vt_hh_state::vt_dma_w));
}


READ8_MEMBER(nes_vt_hh_state::vtfp_4119_r)
{
	// would be PAL/NTSC etc. in base system, maybe different here?
	return 0x00;
}

void nes_vt_hh_state::nes_vt_fp_map(address_map &map)
{
	nes_vt_hh_map(map);
	map(0x411e, 0x411e).w(FUNC(nes_vt_hh_state::vtfp_411e_w));
	map(0x4a00, 0x4a00).w(FUNC(nes_vt_hh_state::vtfp_4a00_w));
	map(0x412c, 0x412c).w(FUNC(nes_vt_hh_state::vtfp_412c_extbank_w));
	map(0x412d, 0x412d).r(FUNC(nes_vt_hh_state::vtfp_412d_r));
	map(0x4242, 0x4242).w(FUNC(nes_vt_hh_state::vtfp_4242_w));
	map(0x4119, 0x4119).r(FUNC(nes_vt_hh_state::vtfp_4119_r));
}


void nes_vt_dg_state::nes_vt_dg_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2007).mirror(0x00e0).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));                      /* standard PPU registers */
	map(0x2010, 0x201f).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::read_extended), FUNC(ppu_vt03_device::write_extended));   /* extra VT PPU registers */

	map(0x4000, 0x4013).rw(m_apu, FUNC(nesapu_device::read), FUNC(nesapu_device::write));
	map(0x4015, 0x4015).rw(FUNC(nes_vt_dg_state::psg1_4015_r), FUNC(nes_vt_dg_state::psg1_4015_w)); /* PSG status / first control register */
	map(0x4016, 0x4016).rw(FUNC(nes_vt_dg_state::in0_r), FUNC(nes_vt_dg_state::in0_w));
	map(0x4017, 0x4017).r(FUNC(nes_vt_dg_state::in1_r)).w(FUNC(nes_vt_dg_state::psg1_4017_w));

	map(0x4100, 0x410b).r(FUNC(nes_vt_dg_state::vt03_410x_r)).w(FUNC(nes_vt_dg_state::vt03_410x_w));

	map(0x411c, 0x411c).w(FUNC(nes_vt_dg_state::vt03_411c_w));

	map(0x8000, 0xffff).rw(FUNC(nes_vt_dg_state::external_space_read), FUNC(nes_vt_dg_state::external_space_write));

	map(0x4034, 0x4034).w(FUNC(nes_vt_dg_state::vt03_4034_w));
	map(0x4014, 0x4014).r(FUNC(nes_vt_dg_state::psg1_4014_r)).w(FUNC(nes_vt_dg_state::vt_fixed_dma_w));
	map(0x6000, 0x7fff).ram();
}

void nes_vt_dg_state::nes_vt_dg_baddma_map(address_map& map)
{
	nes_vt_dg_map(map);
	map(0x4014, 0x4014).w(FUNC(nes_vt_hh_state::vt_dma_w));
}

void nes_vt_dg_state::nes_vt_fa_map(address_map &map)
{
	nes_vt_dg_map(map);

	map(0x412c, 0x412c).r(FUNC(nes_vt_dg_state::vtfa_412c_r)).w(FUNC(nes_vt_dg_state::vtfa_412c_extbank_w));
	map(0x4242, 0x4242).w(FUNC(nes_vt_dg_state::vtfp_4242_w));
}


WRITE_LINE_MEMBER(nes_vt_state::apu_irq)
{
//  set_input_line(N2A03_APU_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(nes_vt_state::apu_read_mem)
{
	return 0x00;//mintf->program->read_byte(offset);
}

/* not strictly needed, but helps us see where things are in ROM to aid with figuring out banking schemes*/
static const gfx_layout helper_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0*64, 1*64, 2*64, 3*64 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	4*64
};

static const gfx_layout helper2_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0*8, 1*8, 2*8, 3*8 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*16, 1*16, 2*16, 3*16,4*16,5*16,5*16,6*16,7*16 },
	4*64
};

static GFXDECODE_START( vt03_gfx_helper )
	GFXDECODE_ENTRY( "mainrom", 0, helper_layout,  0x0, 2  )
	GFXDECODE_ENTRY( "mainrom", 0, helper2_layout,  0x0, 2  )
GFXDECODE_END

uint32_t nes_vt_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	return m_ppu->screen_update(screen, bitmap, cliprect);
}

void nes_vt_state::nes_vt_base(machine_config &config)
{
	/* basic machine hardware */
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_32mbyte);

	M6502_VTSCR(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_state::nes_vt_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.0988);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(NTSC_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_NTSC-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(nes_vt_state::screen_update));

	GFXDECODE(config, "gfxdecode", "ppu", vt03_gfx_helper);

	PPU_VT03(config, m_ppu, N2A03_NTSC_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ppu->read_bg().set(FUNC(nes_vt_state::chr_r));
	m_ppu->read_sp().set(FUNC(nes_vt_state::spr_r));

	ADDRESS_MAP_BANK(config, m_vt_external_space); 
	m_vt_external_space->set_options(ENDIANNESS_NATIVE, 8, 25, 0x2000000);
//	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_32mbyte); // no default map, set according to the ROM size and other things so mirroring works

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* this should actually be a custom *almost* doubled up APU, however requires more thought
	   than just using 2 APUs as registers in the 2nd one affect the PCM channel mode but the
	   DMA control still comes from the 1st, but in the new mode, sound always outputs via the
	   2nd.  Probably need to split the APU into interface and sound gen logic. */
	NES_APU(config, m_apu, NTSC_APU_CLOCK);
	m_apu->irq().set(FUNC(nes_vt_state::apu_irq));
	m_apu->mem_read().set(FUNC(nes_vt_state::apu_read_mem));
	m_apu->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void nes_vt_state::nes_vt_base_pal(machine_config &config)
{
	nes_vt_base(config);

	m_maincpu->set_clock(PALC_APU_CLOCK);

	PPU_VT03PAL(config.replace(), m_ppu, N2A03_PAL_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ppu->read_bg().set(FUNC(nes_vt_state::chr_r));
	m_ppu->read_sp().set(FUNC(nes_vt_state::spr_r));

	/* video hardware */
	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66 / (PALC_APU_CLOCK.dvalue() / 1000000)) *
		(ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL - ppu2c0x_device::VBLANK_FIRST_SCANLINE_PALC + 1 + 2)));
	m_screen->set_size(32 * 8, 312);
	m_screen->set_visarea(0 * 8, 32 * 8 - 1, 0 * 8, 30 * 8 - 1);
}



void nes_vt_sudoku_state::nes_vt_sudoku_512kb(machine_config &config)
{
	nes_vt_base(config);
	m_vt_external_space->set_map(&nes_vt_sudoku_state::vt_external_space_map_512kbyte);
}

void nes_vt_vg_1mb_majgnc_state::nes_vt_vg_1mb_majgnc(machine_config &config)
{
	nes_vt_base(config);
	m_ppu->set_palette_mode(PAL_MODE_NEW_VG);
	m_vt_external_space->set_map(&nes_vt_vg_1mb_majgnc_state::vt_external_space_map_1mbyte);
}

void nes_vt_state::nes_vt(machine_config &config)
{
	nes_vt_base(config);
}


void nes_vt_state::nes_vt_512kb(machine_config& config)
{
	nes_vt(config);
	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_512kbyte);
}

void nes_vt_state::nes_vt_1mb(machine_config& config)
{
	nes_vt(config);
	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_1mbyte);
}

void nes_vt_state::nes_vt_2mb(machine_config& config)
{
	nes_vt(config);
	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_2mbyte);
}

void nes_vt_state::nes_vt_4mb(machine_config& config)
{
	nes_vt(config);
	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_2mbyte);
}

void nes_vt_state::nes_vt_8mb(machine_config& config)
{
	nes_vt(config);
	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_8mbyte);
}

void nes_vt_state::nes_vt_16mb(machine_config& config)
{
	nes_vt(config);
	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_16mbyte);
}

void nes_vt_state::nes_vt_32mb(machine_config& config)
{
	nes_vt(config);
	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_32mbyte);
}


void nes_vt_state::nes_vt_pal(machine_config &config)
{
	nes_vt_base_pal(config);
}

void nes_vt_state::nes_vt_pal_2mb(machine_config& config)
{
	nes_vt_pal(config);
	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_2mbyte);
}

void nes_vt_state::nes_vt_pal_4mb(machine_config& config)
{
	nes_vt_pal(config);
	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_4mbyte);
}

void nes_vt_state::nes_vt_pal_8mb(machine_config& config)
{
	nes_vt_pal(config);
	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_8mbyte);
}

void nes_vt_waixing_state::machine_reset()
{
	nes_vt_state::machine_reset();
	m_ppu->set_201x_descramble(0x3, 0x2, 0x7, 0x6, 0x5, 0x4); // reasonable
}

void nes_vt_waixing_alt_state::machine_reset()
{
	nes_vt_waixing_state::machine_reset();
	set_8000_scramble(0x5, 0x4, 0x3, 0x2, 0x7, 0x6, 0x7, 0x8);
}

void nes_vt_hum_state::machine_reset()
{
	nes_vt_state::machine_reset();

	m_ppu->set_201x_descramble(0x7, 0x6, 0x5, 0x4, 0x2, 0x3);
	set_8000_scramble(0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x7, 0x8);
}

void nes_vt_pjoy_state::machine_reset()
{
	nes_vt_state::machine_reset();

	m_ppu->set_201x_descramble(0x2, 0x3, 0x4, 0x5, 0x6, 0x7);
	set_8000_scramble(0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x8, 0x7);
	set_410x_scramble(0x8, 0x7);
}

void nes_vt_sp69_state::machine_reset()
{
	nes_vt_state::machine_reset();

	m_ppu->set_201x_descramble(0x4, 0x7, 0x2, 0x6, 0x5, 0x3);
	set_8000_scramble(0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x7, 0x8);
}

void nes_vt_ablping_state::nes_vt_2mb_ablping(machine_config &config)
{
	nes_vt(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_ablping_state::nes_vt_ablping_map);
	m_vt_external_space->set_map(&nes_vt_ablping_state::vt_external_space_map_2mbyte);
}

void nes_vt_state::nes_vt_4k_ram(machine_config &config)
{
	nes_vt(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_state::nes_vt_4k_ram_map);
}

void nes_vt_state::nes_vt_4k_ram_16mb(machine_config &config)
{
	nes_vt(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_state::nes_vt_4k_ram_map);
	m_vt_external_space->set_map(&nes_vt_state::vt_external_space_map_16mbyte);
}

void nes_vt_state::nes_vt_4k_ram_pal(machine_config &config)
{
	nes_vt_base_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_state::nes_vt_4k_ram_map);
}

void nes_vt_cy_state::nes_vt_cy(machine_config &config)
{
	nes_vt_4k_ram(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_cy_state::nes_vt_cy_map);
}

void nes_vt_cy_state::nes_vt_cy_bigger(machine_config &config)
{
	nes_vt_cy(config);
	m_vt_external_space->set_map(&nes_vt_cy_state::vt_external_space_map_32mbyte); // must be some banking of this kind of VT can address over 32mb
}

void nes_vt_cy_state::nes_vt_bt(machine_config &config)
{
	nes_vt_4k_ram(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_cy_state::nes_vt_bt_map);
}

void nes_vt_cy_state::nes_vt_bt_2x16mb(machine_config& config)
{
	nes_vt_bt(config);
	m_vt_external_space->set_map(&nes_vt_cy_state::vt_external_space_map_bitboy_2x16mbyte);
}

void nes_vt_dg_state::nes_vt_dg(machine_config &config)
{
	nes_vt_4k_ram(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_dg_state::nes_vt_dg_map);

	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((106.53/(PAL_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 312);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
}

void nes_vt_dg_state::nes_vt_dg_1mb(machine_config& config)
{
	nes_vt_dg(config);
	m_vt_external_space->set_map(&nes_vt_dg_state::vt_external_space_map_1mbyte);
}

void nes_vt_dg_state::nes_vt_dg_baddma_16mb(machine_config& config)
{
	nes_vt_dg(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_dg_state::nes_vt_dg_baddma_map);
	m_vt_external_space->set_map(&nes_vt_dg_state::vt_external_space_map_16mbyte);
}

void nes_vt_hh_state::nes_vt_vg(machine_config &config)
{
	nes_vt_dg(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::nes_vt_hh_map);

	m_ppu->set_palette_mode(PAL_MODE_NEW_VG);
}

void nes_vt_hh_state::nes_vt_vg_8mb(machine_config& config)
{
	nes_vt_vg(config);
	m_vt_external_space->set_map(&nes_vt_hh_state::vt_external_space_map_8mbyte);
}

void nes_vt_hh_state::nes_vt_vg_4mb(machine_config& config)
{
	nes_vt_vg(config);
	m_vt_external_space->set_map(&nes_vt_hh_state::vt_external_space_map_4mbyte);
}

void nes_vt_hh_state::nes_vt_vg_16mb(machine_config& config)
{
	nes_vt_vg(config);
	m_vt_external_space->set_map(&nes_vt_hh_state::vt_external_space_map_16mbyte);
}

void nes_vt_hh_state::nes_vt_vg_1mb_majkon(machine_config &config)
{
	nes_vt_dg(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::nes_vt_hh_baddma_map);

	m_ppu->set_palette_mode(PAL_MODE_NEW_VG);

	m_vt_external_space->set_map(&nes_vt_hh_state::vt_external_space_map_1mbyte);

}


// New mystery handheld architecture, VTxx derived
void nes_vt_hh_state::nes_vt_hh(machine_config &config)
{
	nes_vt_4k_ram(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::nes_vt_hh_map);
	m_ppu->set_palette_mode(PAL_MODE_NEW_RGB);

	/* video hardware */
	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((106.53/(PAL_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 312);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
}

void nes_vt_hh_state::nes_vt_hh_8mb(machine_config& config)
{
	nes_vt_hh(config);
	m_vt_external_space->set_map(&nes_vt_hh_state::vt_external_space_map_8mbyte);
}

void nes_vt_hh_state::nes_vt_hh_4mb(machine_config& config)
{
	nes_vt_hh(config);
	m_vt_external_space->set_map(&nes_vt_hh_state::vt_external_space_map_4mbyte);
}

static INPUT_PORTS_START( nes_vt )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY

	PORT_START("IO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( nes_vt_ddr )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_NAME("Up Arrow") PORT_16WAY // NOT A JOYSTICK!!
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_NAME("Down Arrow") PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_NAME("Left Arrow") PORT_16WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Right Arrow") PORT_16WAY

	PORT_START("IO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void nes_vt_hh_state::nes_vt_fp(machine_config &config)
{
	nes_vt_4k_ram(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::nes_vt_fp_map);

	m_ppu->set_palette_mode(PAL_MODE_NEW_RGB12);
}

void nes_vt_hh_state::nes_vt_fp_4x16mb(machine_config& config)
{
	nes_vt_fp(config);
	m_vt_external_space->set_map(&nes_vt_hh_state::vt_external_space_map_fp_2x32mbyte);
}

void nes_vt_hh_state::nes_vt_fp_32mb(machine_config& config)
{
	nes_vt_fp(config);
	m_vt_external_space->set_map(&nes_vt_hh_state::vt_external_space_map_32mbyte);
}

void nes_vt_hh_state::nes_vt_fp_bigger(machine_config& config)
{
	nes_vt_fp(config);
	m_vt_external_space->set_map(&nes_vt_hh_state::vt_external_space_map_32mbyte); // must be some kind of banking, or this VT can address > 32Mbyte
}

void nes_vt_hh_state::nes_vt_fp_16mb(machine_config& config)
{
	nes_vt_fp(config);
	m_vt_external_space->set_map(&nes_vt_hh_state::vt_external_space_map_16mbyte);
}

void nes_vt_hh_state::nes_vt_fp_pal(machine_config &config)
{
	nes_vt_4k_ram_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::nes_vt_fp_map);

	m_ppu->set_palette_mode(PAL_MODE_NEW_RGB12);
}

void nes_vt_hh_state::nes_vt_fp_pal_32mb(machine_config& config)
{
	nes_vt_fp_pal(config);
	m_vt_external_space->set_map(&nes_vt_hh_state::vt_external_space_map_32mbyte);
}


void nes_vt_dg_state::nes_vt_fa(machine_config &config)
{
	nes_vt_4k_ram(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_dg_state::nes_vt_fa_map);
}

void nes_vt_dg_state::nes_vt_fa_4x16mb(machine_config& config)
{
	nes_vt_fa(config);
	m_vt_external_space->set_map(&nes_vt_dg_state::vt_external_space_map_fapocket_4x16mbyte);
}


void nes_vt_swap_op_d5_d6_state::nes_vt_vh2009(machine_config &config)
{
	nes_vt(config);

	M6502_SWAP_OP_D5_D6(config.replace(), m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_swap_op_d5_d6_state::nes_vt_map);

	//m_ppu->set_palette_mode(PAL_MODE_NEW_VG); // gives better title screens, but worse ingame, must be able to switch
}

void nes_vt_swap_op_d5_d6_state::nes_vt_vh2009_4mb(machine_config& config)
{
	nes_vt_vh2009(config);
	m_vt_external_space->set_map(&nes_vt_swap_op_d5_d6_state::vt_external_space_map_4mbyte);
}

void nes_vt_swap_op_d5_d6_state::nes_vt_vh2009_8mb(machine_config& config)
{
	nes_vt_vh2009(config);
	m_vt_external_space->set_map(&nes_vt_swap_op_d5_d6_state::vt_external_space_map_8mbyte);
}


void nes_vt_swap_op_d5_d6_state::nes_vt_senwld_512kb(machine_config &config)
{
	nes_vt_vh2009(config);
		
	m_ppu->set_palette_mode(PAL_MODE_NEW_VG);

	m_vt_external_space->set_map(&nes_vt_swap_op_d5_d6_state::vt_external_space_map_senwld_512kbyte);
}

static INPUT_PORTS_START( nes_vt_fp )
	PORT_INCLUDE(nes_vt)

	PORT_START("CARTSEL")
	PORT_DIPNAME( 0x06, 0x00, "Cartridge Select" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x00, "472-in-1" )
	PORT_DIPSETTING(    0x06, "128-in-1" )
INPUT_PORTS_END

static INPUT_PORTS_START( nes_vt_fa )
	PORT_INCLUDE(nes_vt)

	PORT_START("CARTSEL")
	PORT_DIPNAME( 0x01, 0x00, "Cartridge Select" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x00, "508-in-1" )
	PORT_DIPSETTING(    0x01, "130-in-1" )
INPUT_PORTS_END


static INPUT_PORTS_START( ablpinb )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // (analog plunger) has to toggle or code gets stuck in interrupt and dies due to nested interrupts
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Select" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED ) // not stored

	PORT_START("IO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Flipper" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Flipper" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // has to toggle or code gets stuck on startup (maybe should cycle automatically when different inputs are available?)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // has to toggle once on the ABL logo or gets stuck in loop, checked in multiple places tho
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED ) // not stored

	PORT_START("PLUNGER")
	PORT_BIT(0x00ff, 0x0000, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0xbf) PORT_NAME("Plunger")  PORT_CENTERDELTA(255)

	PORT_START("EXTRAIN3")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("NUDGE" )
INPUT_PORTS_END

static INPUT_PORTS_START( sudoku )
	PORT_INCLUDE(nes_vt)
INPUT_PORTS_END

// the test mode shows 2 gamepads, however this is not the control scheme the game uses
// there is a reset button too but it doesn't seem to be a software switch
static INPUT_PORTS_START( majgnc )
	PORT_INCLUDE(nes_vt)

	PORT_MODIFY("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_MODIFY("IO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("EXTRAIN0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRAIN1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRAIN2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRAIN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5 / BET")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4")
INPUT_PORTS_END

static INPUT_PORTS_START( timetp36 )
	PORT_INCLUDE(nes_vt)

	PORT_MODIFY("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("B")

	PORT_MODIFY("IO1") // no 2nd player
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	// where does the 'Y' button map? no games use it?
	PORT_START("EXTRAIN0") // see code at 8084, stored at 0x66
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("X") // used in the NAM-1975 rip-off 'Army Strike'
	PORT_DIPNAME( 0x02, 0x02, "Unknown Bit 0" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x04, 0x04, "Unknown Bit 1" ) // see code at 808D, stored at 0x68 (never read?)
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown Bit 2" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("EXTRAIN1") // code at 809A reads this in, stored at 0x156
	PORT_DIPNAME( 0x01, 0x01, "Unknown Bit 3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Difficulty ) ) // 3 possible slider positions
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) ) // 3 minutes timer in Bombs Away
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) ) // 2 minute 30
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) ) // 2 minute
	PORT_DIPSETTING(    0x00, "Hard (duplicate)" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown Bit 4" ) //  ... code at 8064 instead seems to be reading 8 bits with a shifter? stored at 0x67 (investigate)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("EXTRAIN2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("EXTRAIN3")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void nes_vt_sudoku_state::init_sudoku()
{
	u8 *src = memregion("mainrom")->base();
	int len = memregion("mainrom")->bytes();

	std::vector<u8> buffer(len);
	{
		for (int i = 0; i < len; i += 8)
		{
			buffer[i+0] = src[i+5];
			buffer[i+1] = src[i+4];
			buffer[i+2] = src[i+7];
			buffer[i+3] = src[i+6];
			buffer[i+4] = src[i+1];
			buffer[i+5] = src[i+0];
			buffer[i+6] = src[i+3];
			buffer[i+7] = src[i+2];
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}

	if (0)
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine().system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(src, len, 1, fp);
			fclose(fp);
		}
	}
}



ROM_START( vdogdeme )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "vdog.bin", 0x00000, 0x100000, CRC(29dae36d) SHA1(e7192c5b16f3e658b0802e5c50fab244e974d9c2) )
ROM_END

ROM_START( vdogdemo )
	ROM_REGION( 0x80000, "mainrom", 0 )
	ROM_LOAD( "rom.bin", 0x00000, 0x80000, CRC(054af705) SHA1(e730aeaa94b9cc28aa8b512a5bf411ec45226831) )
ROM_END


ROM_START( pinkjelly )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "seesaw.bin", 0x00000, 0x200000, CRC(67b5a079) SHA1(36ebfd64809af072b73acfa3a426b57017851bf4) )
ROM_END

ROM_START( vtpinball )
	ROM_REGION( 0x80000, "mainrom", 0 )
	ROM_LOAD( "rom.bin", 0x00000, 0x80000, CRC(62e52c23) SHA1(b83b82c928b9fe82abfaa915196322153787c8ce) )
ROM_END

ROM_START( ablpinb )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "abl_pinball.bin", 0x00000, 0x200000, CRC(b2ce20fb) SHA1(f2af7f26fcdce6f26db5c71727ab380240f44f74) )
ROM_END

ROM_START( vtsndtest )
	ROM_REGION( 0x80000, "mainrom", 0 )
	ROM_LOAD( "rom.bin", 0x00000, 0x80000, CRC(ddc2bc9c) SHA1(fb9209c62d1496ba7fe379e8a078cabd48cccd9b) )
ROM_END

ROM_START( vtboxing )
	ROM_REGION( 0x80000, "mainrom", 0 )
	ROM_LOAD( "rom.bin", 0x00000, 0x80000, CRC(c115b1af) SHA1(82106e1c11c3279c5d8731c112f713fa3f290125) )
ROM_END

ROM_START( papsudok )
	ROM_REGION( 0x80000, "mainrom", 0 )
	ROM_LOAD( "sudoku2.bin", 0x00000, 0x80000, CRC(d1ffcc1e) SHA1(2010e60933a08d0b9271ef37f338758aacba6d2d) )
ROM_END

ROM_START( mc_dgear )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "dreamgear 75-in-1.prg", 0x00000, 0x400000, CRC(9aabcb8f) SHA1(aa9446b7777fa64503871225fcaf2a17aafd9af1) )
ROM_END

ROM_START( dgun2500 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "dgun2500.bin", 0x00000, 0x1000000, CRC(a2f963f3) SHA1(e29ed20ccdcf25b5640a607b3d2c9e6a4944e172) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(0x1000000)
ROM_END

ROM_START( dgun2561 )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "dgun2561.bin", 0x00000, 0x4000000, CRC(a6e627b4) SHA1(2667d2feb02de349387f9dcfa5418e7ed3afeef6) )
ROM_END

ROM_START( dgun2593 )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	ROM_LOAD( "dreamgear300.bin", 0x00000, 0x8000000, CRC(4fe0ed02) SHA1(a55590557bacca65ed9a17c5bcf0a4e5cb223126) )
ROM_END

ROM_START( rtvgc300 )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	// some of the higher address lines might be swapped
	ROM_LOAD( "lexibook300.bin", 0x00000, 0x4000000, CRC(015c4067) SHA1(a12986c4a366a23c4c7ca7b3d33e421a8dfdffc0) )
ROM_END

ROM_START( rtvgc300fz )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	// some of the higher address lines might be swapped
	ROM_LOAD( "jg7800fz.bin", 0x00000, 0x4000000, CRC(c9d319d2) SHA1(9d0d1435b802f63ce11b94ce54d11f4065b324cc) )
ROM_END

// The maximum address space a VT chip can see is 32MB, so these 64MB roms are actually 2 programs (there are vectors in the first half and the 2nd half)
// there must be a bankswitch bit that switches the whole 32MB space.  Loading the 2nd half in Star Wars does actually boot straight to a game.
ROM_START( lxcmcy )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "lxcmcy.bin", 0x00000, 0x4000000, CRC(3f3af72c) SHA1(76127054291568fcce1431d21af71f775cfb05a6) )
ROM_END

ROM_START( lxcmcysw )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "jl2365swr-1.u2", 0x2000000, 0x2000000, CRC(60ece391) SHA1(655de6b36ba596d873de2839522b948ccf45e006) )
	ROM_CONTINUE(0x0000000, 0x2000000)
ROM_END

ROM_START( lxcmcyfz )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "jl2365_frozen.u1", 0x00000, 0x4000000, CRC(64d4c708) SHA1(1bc2d161326ce3039ab9ba46ad62695060cfb2e1) )
ROM_END

ROM_START( lxcmcydp )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "cyberarcade-disneyprincess.bin", 0x00000, 0x4000000, CRC(05946f81) SHA1(33eea2b70f5427e7613c836b8a08148731fac231) )
ROM_END

ROM_START( lxcmc250 )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "cca250in1.u1", 0x00000, 0x4000000, CRC(6ccd6ad6) SHA1(fafed339097c3d1538faa306021a8373c1b799b3) )
ROM_END

ROM_START( red5mam )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	ROM_LOAD( "mam.u3", 0x00000, 0x8000000, CRC(0c0a0ecd) SHA1(2dfd8437de17fc9975698f1933dd81fbac78466d) )
ROM_END

ROM_START( cybar120 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "m2500p-vt09-epson,20091222ver05,_30r-sx1067-01_pcb,_12r0cob128m_12001-3d05_fw.bin", 0x00000, 0x1000000, CRC(f7138980) SHA1(de31264ee3a5a5c77a86733b2e2d6845fee91ea5) )
ROM_END


ROM_START( mc_dg101 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "dreamgear 101-in-1.prg", 0x00000, 0x400000, CRC(6a7cd8f4) SHA1(9a5ceb8e5e38eb93699dbb14c2c36f3a501d9c45) )
ROM_END

ROM_START( mc_aa2 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "100 in 1 arcade action ii.prg", 0x00000, 0x400000, CRC(33923995) SHA1(a206e8c0ee6e86adb800cf66697defabcbd01902) )
ROM_END

ROM_START( mc_105te )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "2011 super hik 105-in-1 turbo edition.prg", 0x00000, 0x800000, CRC(c0f85771) SHA1(8c4182b1de3be10dd895089823cc67a9d12589ef) )
ROM_END

ROM_START( mc_sp69 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "sports game 69-in-1.prg", 0x00000, 0x400000, CRC(1242da7f) SHA1(bb8f99b1f4a4783b3f7e54d74f1f2a6a628da154) )
ROM_END

ROM_START( polmega )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "megamax.bin", 0x00000, 0x400000, CRC(ef3aade3) SHA1(0c130080ace000cbe43e70a805d4301e05840294) )
ROM_END

ROM_START( silv35 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "silverlit35.bin", 0x00000, 0x400000, CRC(7540e350) SHA1(a0cb456136560fa4d8a365dd44d815ec0e9fc2e7) )
ROM_END

ROM_START( lpgm240 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "w25q64jv.u1", 0x00000, 0x800000, CRC(b973e65b) SHA1(36ff137068ea56b4679c2db386ac0067de0a9eaf) )
ROM_END

ROM_START( pjoyn50 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "power joy navigator 50-in-1.prg", 0x00000, 0x400000, CRC(d1bbadd4) SHA1(2186c71bcedf6c2eedf58233faa26fca9586aa40) )
ROM_END

ROM_START( pjoys30 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "power joy supermax 30-in-1.prg", 0x00000, 0x400000, CRC(947ac898) SHA1(08bb99a8ad39c56780bc66f4e0a9830fba7372dc) )
ROM_END

ROM_START( pjoys60 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "power joy supermax 60-in-1.prg", 0x00000, 0x400000, CRC(1ab45228) SHA1(d148924afc39fc588235331a1a30df6e0d8e1e18) )
ROM_END

ROM_START( sarc110 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "ic1.prg", 0x00000, 0x400000, CRC(de76f71f) SHA1(ff6b37a76c6463af7ae901918fc008b4a2863951) )
ROM_END

ROM_START( sarc110a )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "ic1_ver2.prg", 0x00000, 0x400000, CRC(b97a0dc7) SHA1(bace32d73184df914113de5336e29a7a6f4c03fa) )
ROM_END

// CoolBoy AEF-390 8bit Console, B8VPCBVer03 20130703 0401E2015897A
ROM_START( mc_8x6cb )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "888888-in-1,coolboy aef-390 8bit console, b8vpcbver03 20130703 0401e2015897a.prg", 0x00000, 0x400000, CRC(ca4bd948) SHA1(cfd6c0b03bb432de43d070100031b223c9ee7496) )
ROM_END

ROM_START( mc_110cb )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "29w320dt.bin", 0x00000, 0x400000, CRC(a4bed7eb) SHA1(f1aa89916264ba781d3f1390a2336ef42129b607) )
ROM_END

ROM_START( mc_138cb )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "138-in-1 coolbaby, coolboy rs-5, pcb060-10009011v1.3.bin", 0x00000, 0x400000, CRC(6b5b1a1a) SHA1(2df0cd717bd0de0b0c973ac356426ddbb0d736fa) )
ROM_END

ROM_START( mc_7x6ss )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "777777-in-1, 8 bit slim station, newpxp-dvt22-a pcb.bin", 0x00000, 0x100000, CRC(7790c21a) SHA1(f320f3dd18b88ae5f65bb51f58d4cb869997bab3) )
ROM_END

ROM_START( mc_8x6ss )
	ROM_REGION( 0x100000, "mainrom", 0 ) // odd size rom, does it need stripping?
	ROM_LOAD( "888888-in-1, 8 bit slim station, newpxp-dvt22-a pcb.bin", 0x00000, 0x100000, CRC(47149d0b) SHA1(5a8733886b550e3235dd90fb415b5a602e967f91) )
	ROM_IGNORE(0xce1)
ROM_END

// PXP2 8Bit Slim Station
ROM_START( mc_9x6ss )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "s29gl032.u3", 0x00000, 0x400000, CRC(9f4194e8) SHA1(bd2a356aea56188ea78169095cbbe603d00e0063) )
ROM_END

// same machine as above? is one of these bad?
ROM_START( mc_9x6sa )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "999999-in-1, 8 bit slim station, newpxp-dvt22-a pcb.bin", 0x00000, 0x200000, CRC(6a47c6a0) SHA1(b4dd376167a57dbee3dea70eb16f1a38e16bcdaa) )
ROM_END

ROM_START( mc_sam60 )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "29lv160b.bin", 0x00000, 0x200000, CRC(7dac8efe) SHA1(ffb27ebb4299d5b9a4b976c418fcc7695200060c) )
ROM_END

ROM_START( mc_dcat8 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "100-in-1, d-cat8 8bit console, v5.01.11-frd, bl 20041217.prg", 0x00000, 0x800000, CRC(97d20611) SHA1(d49796e66d7b1dff0ee2781cb0e48b777969d83f) )
ROM_END

ROM_START( mc_dcat8a )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "s29gl064.u6", 0x00000, 0x800000, CRC(e28b1ef8) SHA1(4a6f107d2189cbe1bb0b86b3738d0af58e24e0f7) )
ROM_END

ROM_START( vgtablet )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "vgtablet.bin", 0x00000, 0x400000, CRC(99ef3978) SHA1(0074445708d66a04ab02b4993069ce1ae0514c2f) )
ROM_END

ROM_START( gprnrs1 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "gprnrs1.bin", 0x00000, 0x800000, CRC(c3ffcec8) SHA1(313a790fb51d0b155257f9de84726ed67da43a8f) )
ROM_END

ROM_START( gprnrs16 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "gprnrs16.bin", 0x00000, 0x2000000, CRC(bdffa40a) SHA1(3d01907211f18e8415171dfc6c1d23cf5952e7bb) )
ROM_END

ROM_START( vgpocket )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "vgpocket.bin", 0x00000, 0x400000, CRC(843634c6) SHA1(c59dab0e43d364f59eb3a138abb585bc54e5d674) )
	// there was a dump of a 'secure' area with this, but it was just the top 0x10000 bytes of the existing rom.
ROM_END

ROM_START( vgpmini )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "vgpmini.bin", 0x00000, 0x400000, CRC(a1121843) SHA1(c96013ae6cf2f8173e65a167d45685cb61536d36) )
	// there was a dump of a 'secure' area with this, but it was just the bottom 0x10000 bytes of the existing rom.
ROM_END

ROM_START( sy889 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "sy889_w25q64.bin", 0x00000, 0x800000, CRC(fcdaa6fc) SHA1(0493747facf2172b8af22010851668bb18cbb3e4) )
ROM_END

ROM_START( sy888b )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "sy888b_f25q32.bin", 0x00000, 0x400000, CRC(a8298c33) SHA1(7112dd13d5fb5f9f9d496816758defd22773ec6e) )
ROM_END

ROM_START( ddrdismx )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "disney-ddr.bin", 0x00000, 0x200000, CRC(17fb3abb) SHA1(4d0eda4069ff46173468e579cdf9cc92b350146a) ) // 29LV160 Flash
ROM_END

ROM_START( megapad )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "megapad.bin", 0x00000, 0x200000, CRC(1eb603a8) SHA1(3de6f0620a0db0558daa7fd7ccf08d9d5607a6af) )
ROM_END

ROM_START( timetp36 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "36in1.bin", 0x00000, 0x400000, CRC(e2fb8a6c) SHA1(163d257dd0e6dc19c8fab19cc363ea8be659c40a) )
ROM_END

ROM_START( ddrstraw )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "straws-ddr.bin", 0x00000, 0x200000, CRC(ce94e53a) SHA1(10c6970205a4df28086029c0a348225f57bf0cc5) ) // 26LV160 Flash
ROM_END

ROM_START( majkon )
	ROM_REGION( 0x100000, "mainrom", ROMREGION_ERASEFF )
	ROM_LOAD( "konamicollectorsseries.bin", 0x00000, 0x100000, CRC(47505e51) SHA1(3bfb05d7cfa2bb4c115335f0383fa4aa59db0b28) )
ROM_END

ROM_START( majgnc )
	ROM_REGION( 0x100000, "mainrom", ROMREGION_ERASEFF )
	ROM_LOAD( "majescogoldennuggetcasino_st29w800at_002000d7.bin", 0x00000, 0x100000, CRC(1a156a9d) SHA1(08be4079dd68c9cf05bb92e11a3da4f092d7cfea) )
ROM_END

ROM_START( sudopptv )
	ROM_REGION( 0x80000, "mainrom", ROMREGION_ERASEFF )
	ROM_LOAD( "sudokupnptvgame_29lv400tc_000422b9.bin", 0x00000, 0x80000, CRC(722cc36d) SHA1(1f6d1f57478cf175a36722b39c52eded4b669f81) )
ROM_END


ROM_START( ablping )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "abl_pingpong.bin", 0x00000, 0x200000, CRC(b31de1fb) SHA1(94e8afb2315ba1fa0892191c8e1832391e401c70) )
ROM_END



#if 0
ROM_START( mc_15kin1 )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "15000in1.bin", 0x00000, 0x200000, CRC(29a8cb96) SHA1(c4b31964fbfc5ee97d4a4c7e4d418ea5d84a568d) )
ROM_END

ROM_START( mc_18kin1 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "18000in1.bin", 0x00000, 0x400000, CRC(23c0c325) SHA1(4ad53b5e5a8e65571fd39760278cdf7a6371da47) )
ROM_END

ROM_START( gx121in1 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "gx121in1.bin", 0x00000, 0x400000, CRC(0282d975) SHA1(9ead7505b99a60834724a5818ee120e03c8bf975) )
ROM_END
#endif
ROM_START( dgun2573 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "dgun2573.bin", 0x00000, 0x2000000, BAD_DUMP CRC(cde71a53) SHA1(d0d4c1965876291861781ecde46b1142b062f1f3) )
ROM_END

ROM_START( bittboy )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "bittboy_flash_read_s29gl256n-tf-v2.bin", 0x00000, 0x2000000, CRC(24c802d7) SHA1(c1300ff799b93b9b53060b94d3985db4389c5d3a) )
ROM_END

ROM_START( mc_89in1 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "89in1.bin", 0x00000, 0x400000, CRC(b97f8ce5) SHA1(1a8e67f2b58a671ceec2b0ed18ec5954a71ae63a) )
ROM_END

ROM_START( mc_cb280 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "w25q32.u5", 0x00000, 0x400000, CRC(c9541bdf) SHA1(f0ce46f18658ca5dbed881e5a80460e59820bbd0) )
ROM_END

ROM_START( mc_pg150 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "pocketgames150-in1.bin", 0x00000, 0x2000000, CRC(32f1176b) SHA1(2cfd9b61ebdfc328f020ae9bd5e5e2219321e828) )
ROM_END

ROM_START( mc_hh210 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "msp55lv128t.u4", 0x00000, 0x1000000, CRC(9ba520d4) SHA1(627f811b24314197e289a2ade668ff4115421bed) )
ROM_END

ROM_START( dvnimbus )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "2012-7-4-v1.bin", 0x00000, 0x1000000, CRC(a91d7aa6) SHA1(9421b70b281bb630752bc352c3715258044c0bbe) )
ROM_END

ROM_START( cbrs8 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "rs-8.bin", 0x00000, 0x1000000, BAD_DUMP CRC(10b2bed0) SHA1(0453a1e6769818ccf25dcf22b2c6198a5688a1d4) )
ROM_END

ROM_START( rfcp168 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "winbond_w29gl128c.bin", 0x00000, 0x1000000, CRC(d11caf71) SHA1(64b269cee30a51549a2d0491bbeed07751771559) )
ROM_END

ROM_START( mc_tv200 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "s29gl064n90.bin", 0x00000, 0x800000, CRC(ae1905d2) SHA1(11582055713ba937c1ad32c4ada8683eebc1c83c) )
ROM_END

ROM_START( senwld )
	ROM_REGION( 0x80000, "mainrom", 0 )
	ROM_LOAD( "winlosedraw.bin", 0x00000, 0x80000, CRC(55910bf8) SHA1(c3a7594979d2167be13bf5235c454a22e1f4bb44))
ROM_END

ROM_START( ablmini )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "ablmini.bin", 0x00000, 0x800000, CRC(e65a2c3a) SHA1(9b4811e5b50b67d74b9602471767b8bcd24dd59b) )
ROM_END

ROM_START( techni4 )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "technigame.bin", 0x00000, 0x200000, CRC(3c96b1b1) SHA1(1acc81b26e740327bd6d9faa5a96ab027a48dd77) )
ROM_END

ROM_START( unkra200 ) // "Winbond 25Q64FVSIG 1324" SPI ROM
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "retro_machine_rom", 0x00000, 0x800000, CRC(0e824aa7) SHA1(957e98868559ecc22b3fa42c76692417b76bf132) )
ROM_END

ROM_START( rminitv )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "29gl256.bin", 0x00000, 0x2000000, CRC(cb4048d4) SHA1(9877ce5716d13f8498abfc1cbfaefa9426205d3e) )
ROM_END

ROM_START( denv150 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "denver150in1.bin", 0x00000, 0x1000000, CRC(6b3819d7) SHA1(b0039945ce44a52ea224ab736d5f3c6980409b5d) ) // 2nd half is blank
ROM_END


ROM_START( ppgc200g )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "m29dw641.u2", 0x00000, 0x800000, CRC(b16dc677) SHA1(c1984fde4caf9345d41d127db946d1c21ec43ae0) )
ROM_END



ROM_START( mog_m320 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "w25q64fv.bin", 0x00000, 0x800000, CRC(3c5e1b36) SHA1(4bcbf35ebf2b1714ccde5de758a89a6a39528f89) )
ROM_END

ROM_START( fcpocket )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	ROM_LOAD( "s29gl01gp.bin", 0x00000, 0x8000000, CRC(8703b18a) SHA1(07943443294e80ca93f83181c8bdbf950b87c52f) ) // 2nd half = 0x00 (so 64MByte of content)
ROM_END

ROM_START( fapocket )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "s29gl512n.bin", 0x00000, 0x4000000, CRC(37d0fb06) SHA1(0146a2fae32e23b65d4032c508f0d12cedd399c3) )
ROM_END

ROM_START( zdog )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "zdog.bin", 0x00000, 0x400000, CRC(5ed3485b) SHA1(5ab0e9370d4ed1535205deb0456878c4e400dd81) )
ROM_END

ROM_START( otrail )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "g25q80cw.bin", 0x00000, 0x100000, CRC(b20a03ba) SHA1(c4ca8e590b07baaebed747537bc8f92e44bdd219) ) // dumped as QD25Q80C

	ROM_REGION( 0x200, "seeprom", 0 )
	ROM_LOAD( "t24c04a.bin", 0x000, 0x200, CRC(ce1fad6f) SHA1(82878996765739edba42042b6336460d5c8f8096) )
ROM_END




// earlier version of vdogdemo
CONS( 200?, vdogdeme,  0,  0,  nes_vt_1mb,    nes_vt, nes_vt_state, empty_init, "VRT", "V-Dog (prototype, earlier)", MACHINE_NOT_WORKING )

// this is glitchy even in other emulators, might just be entirely unfinished, it selects banks but they don't contain the required gfx?
CONS( 200?, vdogdemo,  0,  0,  nes_vt_512kb,    nes_vt, nes_vt_state, empty_init, "VRT", "V-Dog (prototype)", MACHINE_NOT_WORKING )

// Bundled as "VT03 Demo" on the V.R. Technology VT SDK
CONS( 200?, pinkjelly, 0,  0,  nes_vt_2mb,    nes_vt, nes_vt_state, empty_init, "VRT / Simmer Technology Co., Ltd.", "VRT VT SDK 'Pink Jelly' (VT03 Demo)", MACHINE_IMPERFECT_GRAPHICS )

// Bundled as "C-Compiler Demo Program 2" on the V.R. Technology VT SDK
CONS( 200?, vtpinball, 0,  0,  nes_vt_512kb,    nes_vt, nes_vt_state, empty_init, "VRT / OJ-Jungle", "VRT VT SDK 'Pinball' (C-Compiler Demo Program 2)", MACHINE_NOT_WORKING )

// Bundled as "Sound Generator FMDemo" on the V.R. Technology VT SDK
CONS( 200?, vtsndtest, 0,  0,  nes_vt_512kb,    nes_vt, nes_vt_state, empty_init, "VRT", "VRT VT SDK 'VT03 Sound Test' (Sound Generator FMDemo)", MACHINE_IMPERFECT_CONTROLS )

// Bundled as "Demo for VT03 Pic32" on the V.R. Technology VT SDK
CONS( 200?, vtboxing,     0,  0,  nes_vt_512kb, nes_vt, nes_vt_state, empty_init, "VRT", "VRT VT SDK 'Boxing' (Demo for VT03 Pic32)", MACHINE_NOT_WORKING )

// Menu system clearly started off as 'vtpinball'  Many elements seem similar to Family Pinball for the Famicom.
// 050329 (29th March 2005) date on PCB
CONS( 2005, ablpinb, 0,  0,  nes_vt_pal_2mb,    ablpinb, nes_vt_ablpinb_state, empty_init, "Advance Bright Ltd", "Pinball (P8002, ABL TV Game)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )


// Black pad marked 'SUDOKU' with tails on the S and U characters looping over the logo.  Box says "Plug and Play Sudoku"
// Has 2 sets of 4 buttons in circular 'direction pad' layouts (on the left for directions, on the right for functions) and 9 red numbered buttons with red power LED on left of them, and reset button on right
CONS( 200?, papsudok,     0,  0,  nes_vt_sudoku_512kb, sudoku, nes_vt_sudoku_state, init_sudoku, "<unknown>", "Plug and Play Sudoku (VT based?)", MACHINE_NOT_WORKING )


// should be VT03 based
// for testing 'Shark', 'Octopus', 'Harbor', and 'Earth Fighter' use the extended colour modes, other games just seem to use standard NES modes
CONS( 200?, mc_dgear,  0,  0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "dreamGEAR", "dreamGEAR 75-in-1", MACHINE_IMPERFECT_GRAPHICS )


// all software in this runs in the VT03 enhanced mode, it also includes an actual licensed VT03 port of Frogger.
// all games work OK except Frogger which has serious graphical issues
CONS( 2006, vgtablet,  0, 0,  nes_vt_vg_4mb,        nes_vt, nes_vt_hh_state, empty_init, "Performance Designed Products (licensed by Konami)", "VG Pocket Tablet (VG-4000)", MACHINE_NOT_WORKING ) // raster timing is broken for Frogger
// There is a 2004 Majesco Frogger "TV game" that appears to contain the same version of Frogger as above but with no other games, so probably fits here.
CONS( 2004, majkon,    0, 0,  nes_vt_vg_1mb_majkon, nes_vt, nes_vt_hh_state, empty_init, "Majesco (licensed from Konami)", "Konami Collector's Series Arcade Advanced", MACHINE_NOT_WORKING ) // raster timing is broken for Frogger, palette issues

CONS( 200?, majgnc,    0, 0,  nes_vt_vg_1mb_majgnc, majgnc, nes_vt_vg_1mb_majgnc_state,  empty_init, "Majesco", "Golden Nugget Casino", MACHINE_NOT_WORKING )

// small black unit, dpad on left, 4 buttons (A,B,X,Y) on right, Start/Reset/Select in middle, unit text "Sudoku Plug & Play TV Game"
CONS( 200?, sudopptv,  0, 0,  nes_vt_512kb,        nes_vt, nes_vt_waixing_state, empty_init, "Smart Planet", "Sudoku Plug & Play TV Game '6 Intelligent Games'", MACHINE_NOT_WORKING )

CONS( 200?, megapad,   0, 0,  nes_vt_2mb,        nes_vt, nes_vt_waixing_state, empty_init, "Waixing", "Megapad 31-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Happy Biqi has broken sprites, investigate before promoting

// 060303 date code on PCB
CONS( 2006, ablmini,   0, 0,  nes_vt_pal_8mb, nes_vt, nes_vt_waixing_alt_state, empty_init, "Advance Bright Ltd", "Double Players Mini Joystick 80-in-1 (MJ8500, ABL TV Game)", MACHINE_IMPERFECT_GRAPHICS )

 // needs PCM samples, Y button is not mapped (not used by any of the games?)
CONS( 200?, timetp36,  0, 0,  nes_vt_pal_4mb, timetp36, nes_vt_timetp36_state,        empty_init, "TimeTop", "Super Game 36-in-1 (TimeTop SuperGame) (PAL)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// this is VT09 based
// it boots, most games correct, but palette issues in some games still (usually they appear greyscale)
// and colors overall a bit off
CONS( 2009, cybar120,  0,  0,  nes_vt_vg_16mb, nes_vt, nes_vt_hh_state, empty_init, "Defender", "Defender M2500P 120-in-1", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005, vgpocket,  0,  0,  nes_vt_vg_4mb, nes_vt, nes_vt_hh_state, empty_init, "Performance Designed Products", "VG Pocket (VG-2000)", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, vgpmini,   0,  0,  nes_vt_vg_4mb, nes_vt, nes_vt_hh_state, empty_init, "Performance Designed Products", "VG Pocket Mini (VG-1500)", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS )
// VG Pocket Max (VG-2500) (blue case, 75 games)
// VG Pocket Max (VG-3000) (white case, 75 games) (does the game selection differ, or only the case?)
// VG Pocket Caplet is likely SunPlus hardware instead. (has common SunPlus games)

// Runs fine, non-sport 121 in 1 games perfect, but minor graphical issues in
// sport games, also no sound in menu or sport games due to missing PCM
// emulation
CONS( 200?, dgun2500,  0,  0,  nes_vt_dg_baddma_16mb, nes_vt, nes_vt_dg_state, empty_init, "dreamGEAR", "dreamGEAR Wireless Motion Control with 130 games (DGUN-2500)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)

// don't even get to menu. very enhanced chipset, VT368/9?
CONS( 2012, dgun2561,  0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "dreamGEAR", "My Arcade Portable Gaming System (DGUN-2561)", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 2016, dgun2593,  0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "dreamGEAR", "My Arcade Retro Arcade Machine - 300 Handheld Video Games (DGUN-2593)", MACHINE_NOT_WORKING ) // 128Mbyte ROM, must be externally banked or different addressing scheme

CONS( 200?, lxcmcy,    0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Lexibook", "Lexibook Compact Cyber Arcade", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmc250,  0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Lexibook", "Lexibook Compact Cyber Arcade - 250-in-1 (JL2375)", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcysw,  0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Lexibook", "Lexibook Compact Cyber Arcade - Star Wars Rebels", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcyfz,  0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Lexibook", "Lexibook Compact Cyber Arcade - Frozen", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcydp,  0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Lexibook", "Lexibook Compact Cyber Arcade - Disney Princess", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme

// GB-NO13-Main-VT389-2 on PCBs
CONS( 2016, rtvgc300,  0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Lexibook", "Lexibook Retro TV Game Console - 300 Games", MACHINE_NOT_WORKING )  // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 2017, rtvgc300fz,0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Lexibook", "Lexibook Retro TV Game Console - Frozen - 300 Games", MACHINE_NOT_WORKING )  // 64Mbyte ROM, must be externally banked, or different addressing scheme


/* The following are also confirmed to be NES/VT derived units, most having a standard set of games with a handful of lazy graphic mods thrown in to fit the unit theme

    (handhekd units, use standard AAA batteries)
    Lexibook Compact Cyber Arcade - Cars
    Lexibook Compact Cyber Arcade - Paw Patrol
    Lexibook Compact Cyber Arcade - Barbie
    Lexibook Compact Cyber Arcade - Finding Dory
    Lexibook Compact Cyber Arcade - Marvel Ultimate Spiderman
    Lexibook Compact Cyber Arcade - PJ Masks

    (Handheld units, but different form factor to Compact Cyber Arcade, charged via USB)
    Lexibook Console Colour - Minnie Mouse
    Lexibook Console Colour - Disney's Planes
    Lexibook Console Colour - Barbie

    (units for use with TV)
    Lexibook Retro TV Game Console (300 Games) - Cars

    (more?)
*/

// intial code isn't valid? scrambled?
CONS( 201?, red5mam,  0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Red5", "Mini Arcade Machine (Red5)", MACHINE_NOT_WORKING ) // 128Mbyte ROM, must be externally banked or different addressing scheme


// boots, same platform with scrambled opcodes as FC pocket
// palette issues in some games because they actually use the old VT style palette
// but no way to switch?
// some menu gfx broken, probably because this is a bad dump
CONS( 2015, dgun2573,  0,  0,  nes_vt_fp_32mb, nes_vt, nes_vt_hh_state, empty_init, "dreamGEAR", "My Arcade Gamer V Portable Gaming System (DGUN-2573)",  MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

CONS( 201?, denv150,   0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Denver", "Denver Game Console GMP-240C 150-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )


// CPU die is marked 'VH2009' There's also a 62256 RAM chip on the PCB, some scrambled opcodes
CONS( 200?, polmega,   0,  0,  nes_vt_vh2009_4mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "Polaroid", "Megamax GPD001SDG", MACHINE_NOT_WORKING )
CONS( 200?, silv35,    0,  0,  nes_vt_vh2009_4mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "SilverLit", "35 in 1 Super Twins", MACHINE_NOT_WORKING )
// die is marked as VH2009, as above, but no scrambled opcodes here
CONS( 201?, techni4,   0,  0,  nes_vt_pal_2mb,      nes_vt, nes_vt_state,        empty_init, "Technigame", "Technigame Super 4-in-1 Sports (PAL)", MACHINE_IMPERFECT_GRAPHICS )


// same encryption as above, but seems like newer hardware (or the above aren't using most of the features)
CONS( 200?, lpgm240,    0,  0,  nes_vt_vh2009_8mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "<unknown>", "Let's Play! Game Machine 240 in 1", MACHINE_NOT_WORKING ) // mini 'retro-arcade' style cabinet

// this has 'Shark' and 'Octopus' etc. like mc_dgear but uses scrambled bank registers
CONS( 200?, mc_sp69,   0,  0,  nes_vt_4mb,    nes_vt, nes_vt_sp69_state, empty_init, "<unknown>", "Sports Game 69 in 1", MACHINE_IMPERFECT_GRAPHICS  | MACHINE_IMPERFECT_SOUND)

// this game was also sold by dreamGEAR and several others companies, each time with a different name and different case, although the dumped version was from ABL, and it hasn't been confirmed that the ROMs are identical for the other units
// Super Ping Pong appears on the title screen, but not the box / product art which simply has "Ping Pong Plug & Play TV Game" on front/back/bottom/manual, and "Table Tennis Plug & Play TV Game" on left/right sides.  Product code is PP1100
// PCB has PP1100-MB 061110 on it, possible date YYMMDD code? (pinball is 050329, guitar fever is 070516, air blaster 050423, kickboxing 061011 etc.)
CONS( 2006, ablping,   0,        0,  nes_vt_2mb_ablping, nes_vt, nes_vt_ablping_state, empty_init, "Advance Bright Ltd", "Ping Pong / Table Tennis / Super Ping Pong (PP1100, ABL TV Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// Hummer systems, scrambled bank register
CONS( 200?, mc_sam60,  0,  0,  nes_vt_2mb,    nes_vt, nes_vt_hum_state, empty_init, "Hummer Technology Co., Ltd.", "Samuri (60 in 1)", MACHINE_IMPERFECT_GRAPHICS  | MACHINE_IMPERFECT_SOUND )
CONS( 200?, zdog,      0,  0,  nes_vt_4mb,    nes_vt, nes_vt_hum_state, empty_init, "Hummer Technology Co., Ltd.", "ZDog (44 in 1)", MACHINE_IMPERFECT_GRAPHICS  | MACHINE_IMPERFECT_SOUND )

// very plain menus
CONS( 200?, pjoyn50,    0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "PowerJoy Navigator 50 in 1", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, pjoys30,    0,        0,  nes_vt_4mb,    nes_vt, nes_vt_pjoy_state, empty_init, "<unknown>", "PowerJoy Supermax 30 in 1", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, pjoys60,    0,        0,  nes_vt_4mb,    nes_vt, nes_vt_pjoy_state, empty_init, "<unknown>", "PowerJoy Supermax 60 in 1", MACHINE_IMPERFECT_GRAPHICS )
// has a non-enhanced version of 'Octopus' as game 30
CONS( 200?, sarc110,    0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "Super Arcade 110 (set 1)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, sarc110a,   sarc110,  0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "Super Arcade 110 (set 2)", MACHINE_IMPERFECT_GRAPHICS )
// both offer chinese or english menus
CONS( 200?, mc_110cb,   0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "CoolBoy", "110 in 1 CoolBaby (CoolBoy RS-1S)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_138cb,   0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "CoolBoy", "138 in 1 CoolBaby (CoolBoy RS-5, PCB060-10009011V1.3)", MACHINE_IMPERFECT_GRAPHICS )

// doesn't boot, bad dump
CONS( 201?, cbrs8,      0,        0,  nes_vt_16mb,    nes_vt, nes_vt_state, empty_init, "CoolBoy", "CoolBoy RS-8 168 in 1", MACHINE_NOT_WORKING )

CONS( 201?, rfcp168,    0,        0,  nes_vt_16mb,    nes_vt, nes_vt_state, empty_init, "<unknown>",   "Retro FC Plus 168 in 1 Handheld", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // "RETRO_FC_V3.5"  (doesn't boot, ends up in weeds after jumping to bank with no code, dump not verified)

CONS( 200?, gprnrs1,    0,        0,  nes_vt_8mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "Game Prince RS-1", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, gprnrs16,   0,        0,  nes_vt_32mb,   nes_vt, nes_vt_state, empty_init, "<unknown>", "Game Prince RS-16", MACHINE_IMPERFECT_GRAPHICS )

// Notes about the DDR games:
// * Missing PCM sounds (unsupported in NES VT APU code right now)
// * Console has stereo output (dual RCA connectors).
CONS( 2006, ddrdismx,   0,        0,  nes_vt_2mb, nes_vt_ddr, nes_vt_state, empty_init, "Majesco (licensed from Konami, Disney)", "Dance Dance Revolution Disney Mix",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // shows (c)2001 Disney onscreen, but that's recycled art from the Playstation release, actual release was 2006
CONS( 2006, ddrstraw,   0,        0,  nes_vt_2mb, nes_vt_ddr, nes_vt_state, empty_init, "Majesco (licensed from Konami)",         "Dance Dance Revolution Strawberry Shortcake", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )


// unsorted, these were all in nes.xml listed as ONE BUS systems
CONS( 200?, mc_dg101,   0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "dreamGEAR", "dreamGEAR 101 in 1", MACHINE_IMPERFECT_GRAPHICS ) // dreamGear, but no enhanced games?
CONS( 200?, mc_aa2,     0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "100 in 1 Arcade Action II (AT-103)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_105te,   0,        0,  nes_vt_8mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "2011 Super HiK 105 in 1 Turbo Edition", MACHINE_NOT_WORKING )
CONS( 200?, mc_8x6cb,   0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "CoolBoy",   "888888 in 1 (Coolboy AEF-390)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, mc_9x6ss,   0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "999999 in 1 (PXP2 Slim Station)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_9x6sa,   mc_9x6ss, 0,  nes_vt_2mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "999999 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_7x6ss,   0,        0,  nes_vt_1mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "777777 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_8x6ss,   0,        0,  nes_vt_1mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "888888 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 2004, mc_dcat8,   0,        0,  nes_vt_8mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "100 in 1 (D-CAT8 8bit Console, set 1) (v5.01.11-frd, BL 20041217)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 2004, mc_dcat8a,  mc_dcat8, 0,  nes_vt_8mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "100 in 1 (D-CAT8 8bit Console, set 2)", MACHINE_IMPERFECT_GRAPHICS )

// Runs well, minor GFX issues in intro
CONS( 2017, sy889,      0,        0,  nes_vt_hh_8mb, nes_vt, nes_vt_hh_state, empty_init, "SY Corp",   "SY-889 300 in 1 Handheld", MACHINE_IMPERFECT_GRAPHICS )
CONS( 2016, sy888b,     0,        0,  nes_vt_hh_4mb, nes_vt, nes_vt_hh_state, empty_init, "SY Corp",   "SY-888B 288 in 1 Handheld", MACHINE_IMPERFECT_GRAPHICS )


// Same hardware as SY-889
CONS( 201?, mc_cb280,   0,        0,  nes_vt_hh_4mb, nes_vt, nes_vt_hh_state, empty_init, "CoolBoy",   "Coolboy RS-18 (280 in 1)", MACHINE_IMPERFECT_GRAPHICS )

// Runs well, only issues in SMB3 which crashes
CONS( 2017, bittboy,    0,        0,  nes_vt_bt_2x16mb, nes_vt, nes_vt_cy_state, empty_init, "BittBoy",   "BittBoy Mini FC 300 in 1", MACHINE_IMPERFECT_GRAPHICS ) // has external banking (2x 16mbyte banks)
// Runs well, all games seem to work
CONS( 201?, mc_89in1,   0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "89 in 1 Mini Game Console (060-92023011V1.0)", MACHINE_IMPERFECT_GRAPHICS )
// Broken GFX, investigate
CONS( 201?, mc_pg150,   0,        0,  nes_vt_bt_2x16mb, nes_vt, nes_vt_cy_state, empty_init, "<unknown>", "Pocket Games 150 in 1", MACHINE_NOT_WORKING ) // has external banking
// No title screen, but press start and menu and games run fine. Makes odd
// memory accesses which probably explain broken title screen
CONS( 201?, mc_hh210,   0,        0,  nes_vt_4k_ram_16mb, nes_vt, nes_vt_state, empty_init, "<unknown>", "Handheld 210 in 1", MACHINE_NOT_WORKING )
// First half of games don't work, probably bad dump
CONS( 201?, dvnimbus,   0,        0,  nes_vt_vg_16mb, nes_vt, nes_vt_hh_state, empty_init, "<unknown>", "DVTech Nimbus 176 in 1", MACHINE_NOT_WORKING )
// Works fine, VT02 based
CONS( 201?, mc_tv200,   0,        0,  nes_vt_8mb,    nes_vt, nes_vt_state, empty_init, "Thumbs Up", "200 in 1 Retro TV Game", MACHINE_IMPERFECT_GRAPHICS )
 // probably another Thumbs Up product? cursor doesn't work unless nes_vt_hh machine is used? possibly newer than VT02 as it runs from an SPI ROM, might just not use enhanced features.  Some minor game name changes to above (eg Smackdown just becomes Wrestling)
CONS( 201?, unkra200,   mc_tv200, 0,  nes_vt_hh_8mb, nes_vt, nes_vt_hh_state, empty_init, "<unknown>", "200 in 1 Retro Arcade", MACHINE_IMPERFECT_GRAPHICS )

CONS( 2015, rminitv,     0,        0,  nes_vt_fp_pal_32mb, nes_vt, nes_vt_hh_state, empty_init, "Orb Gaming", "Retro 'Mini TV' Console 300-in-1", MACHINE_IMPERFECT_GRAPHICS ) // single 32Mbyte bank!

// available in a number of colours, with various brands, but likely all the same.
// This was a red coloured pad, contains various unlicensed bootleg reskinned NES game eg Blob Buster is a hack of Dig Dug 2 and there are also hacks of Xevious, Donkey Kong Jr, Donkey Kong 3 and many others.
CONS( 201?, ppgc200g,   0,         0,  nes_vt_8mb, nes_vt, nes_vt_state, empty_init, "<unknown>", "Plug & Play Game Controller with 200 Games (Supreme 200)", MACHINE_IMPERFECT_GRAPHICS )

// New platform with scrambled opcodes, same as DGUN-2561. Runs fine with minor GFX and sound issues in menu
// Use DIP switch to select console or cartridge, as cartridge is fake and just toggles a GPIO
CONS( 2016, fcpocket,   0,        0,  nes_vt_fp_4x16mb, nes_vt_fp, nes_vt_hh_state, empty_init, "<unknown>",   "FC Pocket 600 in 1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )  // has external banking (2x 32mbyte banks)
// Probably VT09 or similar
// Use DIP switch to select console or cartridge, as cartridge is fake and just toggles a ROM high address bit
// (which can also be overriden by GPIO)
CONS( 2017, fapocket,   0,        0,  nes_vt_fa_4x16mb, nes_vt_fa, nes_vt_dg_fapocket_state, empty_init, "<unknown>",   "Family Pocket 638 in 1", MACHINE_IMPERFECT_GRAPHICS ) // has external banking (4x 16mbyte banks)

// Plays intro music but then crashes. same hardware as SY-88x but uses more features
CONS( 2016, mog_m320,   0,        0,  nes_vt_hh_8mb, nes_vt, nes_vt_hh_state, empty_init, "MOGIS",    "MOGIS M320 246 in 1 Handheld", MACHINE_NOT_WORKING )

CONS( 2017, otrail,     0,        0,  nes_vt_dg_1mb, nes_vt, nes_vt_dg_state, empty_init, "Basic Fun", "The Oregon Trail", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2005, senwld,   0,          0,  nes_vt_senwld_512kb,    nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "Senario", "Win, Lose or Draw (Senario)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // needs RAM in banked space, Alpha display emulating, Touchpad emulating etc.

