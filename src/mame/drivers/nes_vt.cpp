// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  nes_vt.cpp

  TODO

  Add more Famiclone roms here, there should be plenty more dumps of VTxx
  based systems floating about.

  Make sure that none of the unenhanced sets were actually sold as cartridges
  instead, there is a lack of information for some of the older dumps and
  still some dumps in nes.xml that might belong here.

  NON-bugs (same happens on real hardware)

  Pause screen has corrupt GFX on enhanced version of Octopus

***************************************************************************/

#include "emu.h"
#include "machine/nes_vt_soc.h"

class nes_vt_base_state : public driver_device
{
public:
	nes_vt_base_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_io0(*this, "IO0"),
		m_io1(*this, "IO1"),
		m_cartsel(*this, "CARTSEL"),
		m_exin0(*this, "EXTRAIN0"),
		m_exin1(*this, "EXTRAIN1"),
		m_exin2(*this, "EXTRAIN2"),
		m_exin3(*this, "EXTRAIN3"),
		m_prgrom(*this, "mainrom")
	{ }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual uint8_t in0_r();
	virtual uint8_t in1_r();
	virtual void in0_w(uint8_t data);

	void nes_vt_map(address_map& map);


	optional_ioport m_io0;
	optional_ioport m_io1;
	uint8_t m_latch0;
	uint8_t m_latch1;

	/* Misc */
	uint32_t m_ahigh; // external banking bits
	uint8_t m_4242;
	uint8_t m_411c;
	uint8_t m_411d;

	optional_ioport m_cartsel;
	optional_ioport m_exin0;
	optional_ioport m_exin1;
	optional_ioport m_exin2;
	optional_ioport m_exin3;

	required_region_ptr<uint8_t> m_prgrom;

	uint8_t vt_rom_r(offs_t offset);
	void vtspace_w(offs_t offset, uint8_t data);

	void configure_soc(nes_vt_soc_device* soc);

	uint8_t upper_412c_r();
	uint8_t upper_412d_r();
	void upper_412c_w(uint8_t data);

private:
	/* APU handling */

	/* Extra IO */
	uint8_t extrain_0_r();
	uint8_t extrain_1_r();
	uint8_t extrain_2_r();
	uint8_t extrain_3_r();

};

class nes_vt_state : public nes_vt_base_state
{
public:
	nes_vt_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_base_state(mconfig, type, tag),
		m_soc(*this, "soc")
	{ }

	void nes_vt_pal_2mb(machine_config& config);
	void nes_vt_pal_4mb(machine_config& config);
	void nes_vt_pal_8mb(machine_config& config);

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

	void vt_external_space_map_32mbyte(address_map& map);
	void vt_external_space_map_16mbyte(address_map& map);
	void vt_external_space_map_8mbyte(address_map& map);
	void vt_external_space_map_4mbyte(address_map& map);
	void vt_external_space_map_2mbyte(address_map& map);
	void vt_external_space_map_1mbyte(address_map& map);
	void vt_external_space_map_512kbyte(address_map& map);

protected:
	required_device<nes_vt_soc_device> m_soc;

	void vt03_8000_mapper_w(offs_t offset, uint8_t data) { m_soc->vt03_8000_mapper_w(offset, data); }
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
};

class nes_vt_pjoy_state : public nes_vt_state
{
public:
	nes_vt_pjoy_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void nes_vt_pjoy_4mb(machine_config& config);
};

class nes_vt_waixing_state : public nes_vt_state
{
public:
	nes_vt_waixing_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void nes_vt_waixing_512kb(machine_config& config);
	void nes_vt_waixing_2mb(machine_config& config);
};

class nes_vt_waixing_alt_state : public nes_vt_waixing_state
{
public:
	nes_vt_waixing_alt_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_waixing_state(mconfig, type, tag)
	{ }

	void nes_vt_waixing_alt_4mb(machine_config& config);
	void nes_vt_waixing_alt_pal_8mb(machine_config& config);
};


class nes_vt_timetp36_state : public nes_vt_state
{
public:
	nes_vt_timetp36_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }
};

class nes_vt_hum_state : public nes_vt_state
{
public:
	nes_vt_hum_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void nes_vt_hummer_2mb(machine_config& config);
	void nes_vt_hummer_4mb(machine_config& config);
};

class nes_vt_sp69_state : public nes_vt_state
{
public:
	nes_vt_sp69_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void nes_vt_4mb_sp69(machine_config& config);
};

class nes_vt_ablping_state : public nes_vt_state
{
public:
	nes_vt_ablping_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void nes_vt_2mb_ablping(machine_config& config);

private:
	uint8_t ablping_extraio_r();
	void ablping_extraio_w(uint8_t data);
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

	void bittboy_412c_w(uint8_t data);

	uint8_t vt_rom_banked_r(offs_t offset);
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
	virtual uint8_t in0_r() override;
	virtual uint8_t in1_r() override;
	virtual void in0_w(uint8_t data) override;

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

private:
	uint8_t vt_rom_banked_r(offs_t offset);
	void vt_external_space_map_fapocket_4x16mbyte(address_map& map);

	void nes_vt_dg_map(address_map& map);
	void nes_vt_dg_baddma_map(address_map& map);
	void nes_vt_fa_map(address_map& map);

	uint8_t fapocket_412c_r();
	void fapocket_412c_w(uint8_t data);

};

class nes_vt_dg_fapocket_state : public nes_vt_dg_state
{
public:
	nes_vt_dg_fapocket_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_dg_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override;
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
	uint8_t vt_rom_banked_r(offs_t offset);
	void vt_external_space_map_fp_2x32mbyte(address_map& map);

	void nes_vt_fp_map(address_map& map);

	uint8_t fcpocket_412d_r();
	void fcpocket_412c_w(uint8_t data);
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
	virtual uint8_t in0_r() override;
	virtual uint8_t in1_r() override;
	virtual void in0_w(uint8_t data) override;

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

private:
	virtual uint8_t in0_r() override;
	virtual uint8_t in1_r() override;
	virtual void in0_w(uint8_t data) override;
};

class nes_vt_vg_1mb_majgnc_state : public nes_vt_state
{
public:
	nes_vt_vg_1mb_majgnc_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void nes_vt_vg_1mb_majgnc(machine_config& config);
};

uint8_t nes_vt_base_state::vt_rom_r(offs_t offset)
{
	return m_prgrom[offset];
}

void nes_vt_base_state::vtspace_w(offs_t offset, uint8_t data)
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
uint8_t nes_vt_cy_state::vt_rom_banked_r(offs_t offset)
{
	return m_prgrom[m_ahigh | offset];
}

void nes_vt_cy_state::vt_external_space_map_bitboy_2x16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).rw(FUNC(nes_vt_cy_state::vt_rom_banked_r), FUNC(nes_vt_cy_state::vt03_8000_mapper_w));
}

// fapocket is 4 16Mbyte banks
uint8_t nes_vt_dg_state::vt_rom_banked_r(offs_t offset)
{
	return m_prgrom[m_ahigh | offset];
}

void nes_vt_dg_state::vt_external_space_map_fapocket_4x16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).rw(FUNC(nes_vt_dg_state::vt_rom_banked_r), FUNC(nes_vt_dg_state::vt03_8000_mapper_w));
}

uint8_t nes_vt_hh_state::vt_rom_banked_r(offs_t offset)
{
	return m_prgrom[m_ahigh | offset];
}

void nes_vt_hh_state::vt_external_space_map_fp_2x32mbyte(address_map &map)
{
	map(0x0000000, 0x1ffffff).rw(FUNC(nes_vt_hh_state::vt_rom_banked_r), FUNC(nes_vt_hh_state::vt03_8000_mapper_w));
}

uint8_t nes_vt_base_state::extrain_0_r()
{
	if (m_exin0)
		return m_exin0->read();
	else
	{
		logerror("%s: extrain_0_r (not hooked up)\n", machine().describe_context());
	}
	return 0x00;
}

uint8_t nes_vt_base_state::extrain_1_r()
{
	if (m_exin1)
		return m_exin1->read();
	else
	{
		logerror("%s: extrain_1_r (not hooked up)\n", machine().describe_context());
	}
	return 0x00;
}

uint8_t nes_vt_base_state::extrain_2_r()
{
	if (m_exin2)
		return m_exin2->read();
	else
	{
		logerror("%s: extrain_2_r (not hooked up)\n", machine().describe_context());
	}
	return 0x00;
}

uint8_t nes_vt_base_state::extrain_3_r()
{
	if (m_exin3)
		return m_exin3->read();
	else
	{
		logerror("%s: extrain_3_r (not hooked up)\n", machine().describe_context());
	}
	return 0x00;
}

/* Standard I/O handlers (NES Controller clone) */

uint8_t nes_vt_base_state::in0_r()
{
	//logerror("%s: in0_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch0 & 1;
	m_latch0 >>= 1;
	return ret;
}

uint8_t nes_vt_base_state::in1_r()
{
	//logerror("%s: in1_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch1 & 1;
	m_latch1 >>= 1;
	return ret;
}

void nes_vt_base_state::in0_w(uint8_t data)
{
	//logerror("%s: in0_w %02x\n", machine().describe_context(), data);
	if (data & 0x01)
		return;

	m_latch0 = m_io0->read();
	m_latch1 = m_io1->read();
}

/* Lexibook I/O handlers */

uint8_t nes_vt_cy_lexibook_state::in0_r()
{
	//logerror("%s: in0_r\n", machine().describe_context());
	uint8_t ret = m_latch0_bit;
	return ret;
}

uint8_t nes_vt_cy_lexibook_state::in1_r()
{
	//logerror("%s: in1_r\n", machine().describe_context());
	uint8_t ret = m_latch1_bit;
	return ret;
}

void nes_vt_cy_lexibook_state::in0_w(uint8_t data)
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



// ablping polls this (also writes here) what is it? 4-bit DAC? PCM? (inputs only start responding once it finishes writing data on startup but takes longer than a sample should)
// (this is the extended IO port on VT)
uint8_t nes_vt_ablping_state::ablping_extraio_r()
{
	// needs to change at least
	return machine().rand()&0xf;
};

void nes_vt_ablping_state::ablping_extraio_w(uint8_t data)
{
	popmessage("ablping_extraio_w %02x", data);
};


void nes_vt_base_state::machine_start()
{
}

void nes_vt_base_state::machine_reset()
{

}

void nes_vt_dg_fapocket_state::machine_reset()
{
	nes_vt_base_state::machine_reset();

	// fapocket needs this, fcpocket instead reads the switch in software?
	if (m_cartsel)
		m_ahigh = (m_cartsel->read() == 0x01) ? (1 << 25) : 0x0;
	else
		m_ahigh = 0;
}

void nes_vt_ablpinb_state::machine_start()
{
	nes_vt_base_state::machine_start();

	save_item(NAME(m_plunger_off));
	save_item(NAME(m_plunger_state_count));
	save_item(NAME(m_ablpinb_in0_val));

}

void nes_vt_ablpinb_state::machine_reset()
{
	nes_vt_base_state::machine_reset();

	m_plunger_off = 0;
	m_plunger_state_count = 0;
	m_ablpinb_in0_val = 0;
}


uint8_t nes_vt_ablpinb_state::in0_r()
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


uint8_t nes_vt_ablpinb_state::in1_r()
{
	uint8_t i = machine().rand() & 0x10;

	// maybe this transition takes some time in reality?
	i |= (m_ablpinb_in0_val & 0x04) ? 0x00 : 0x08;

	uint8_t ret = m_io1->read() & ~0x18;

	return i | ret;
}

void nes_vt_ablpinb_state::in0_w(uint8_t data)
{
	// write 0x04 to 0x4016 sets bit 0x08 in 0x4017
	// write 0x00 to 0x4016 clears bit 0x08 in 0x4017
	// could be related to vibration motor?

	m_ablpinb_in0_val = data;
	logerror("in0_w %02x\n", data);
}

uint8_t nes_vt_sudoku_state::in0_r()
{
	return machine().rand();
}

uint8_t nes_vt_sudoku_state::in1_r()
{
	return machine().rand();
}

void nes_vt_sudoku_state::in0_w(uint8_t data)
{
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

void nes_vt_base_state::configure_soc(nes_vt_soc_device* soc)
{
	soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_32mbyte);
	soc->read_0_callback().set(FUNC(nes_vt_base_state::in0_r));
	soc->read_1_callback().set(FUNC(nes_vt_base_state::in1_r));
	soc->write_0_callback().set(FUNC(nes_vt_base_state::in0_w));

	soc->extra_read_0_callback().set(FUNC(nes_vt_base_state::extrain_0_r));
	soc->extra_read_1_callback().set(FUNC(nes_vt_base_state::extrain_1_r));
	soc->extra_read_2_callback().set(FUNC(nes_vt_base_state::extrain_2_r));
	soc->extra_read_3_callback().set(FUNC(nes_vt_base_state::extrain_3_r));
}



void nes_vt_sudoku_state::nes_vt_sudoku_512kb(machine_config &config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_sudoku_state::vt_external_space_map_512kbyte);
}

void nes_vt_vg_1mb_majgnc_state::nes_vt_vg_1mb_majgnc(machine_config &config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_default_palette_mode(PAL_MODE_NEW_VG);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_vg_1mb_majgnc_state::vt_external_space_map_1mbyte);
}

void nes_vt_state::nes_vt_512kb(machine_config& config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_512kbyte);
}

void nes_vt_state::nes_vt_1mb(machine_config& config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_1mbyte);
}

void nes_vt_state::nes_vt_2mb(machine_config& config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_2mbyte);
}

void nes_vt_state::nes_vt_4mb(machine_config& config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_4mbyte);
}

void nes_vt_state::nes_vt_8mb(machine_config& config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_8mbyte);
}

void nes_vt_state::nes_vt_16mb(machine_config& config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_16mbyte);
}

void nes_vt_state::nes_vt_32mb(machine_config& config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_32mbyte);
}

void nes_vt_state::nes_vt_pal_2mb(machine_config& config)
{
	NES_VT_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_2mbyte);
}

void nes_vt_state::nes_vt_pal_4mb(machine_config& config)
{
	NES_VT_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_4mbyte);
}

void nes_vt_state::nes_vt_pal_8mb(machine_config& config)
{
	NES_VT_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_8mbyte);
}

void nes_vt_waixing_state::nes_vt_waixing_512kb(machine_config &config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_waixing_state::vt_external_space_map_512kbyte);
	m_soc->set_201x_descramble(0x3, 0x2, 0x7, 0x6, 0x5, 0x4);
}

void nes_vt_waixing_state::nes_vt_waixing_2mb(machine_config &config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_waixing_state::vt_external_space_map_2mbyte);
	m_soc->set_201x_descramble(0x3, 0x2, 0x7, 0x6, 0x5, 0x4);
}

void nes_vt_waixing_alt_state::nes_vt_waixing_alt_4mb(machine_config &config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_waixing_state::vt_external_space_map_4mbyte);
	m_soc->set_201x_descramble(0x3, 0x2, 0x7, 0x6, 0x5, 0x4);
	m_soc->set_8000_scramble(0x5, 0x4, 0x3, 0x2, 0x7, 0x6, 0x7, 0x8);
}

void nes_vt_waixing_alt_state::nes_vt_waixing_alt_pal_8mb(machine_config &config)
{
	NES_VT_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_waixing_alt_state::vt_external_space_map_8mbyte);
	m_soc->set_201x_descramble(0x3, 0x2, 0x7, 0x6, 0x5, 0x4);
	m_soc->set_8000_scramble(0x5, 0x4, 0x3, 0x2, 0x7, 0x6, 0x7, 0x8);
}

void nes_vt_hum_state::nes_vt_hummer_2mb(machine_config& config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_sp69_state::vt_external_space_map_2mbyte);
	m_soc->set_201x_descramble(0x7, 0x6, 0x5, 0x4, 0x2, 0x3);
	m_soc->set_8000_scramble(0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x7, 0x8);
	GFXDECODE(config, "gfxdecode", "soc:ppu", vt03_gfx_helper);
}

void nes_vt_hum_state::nes_vt_hummer_4mb(machine_config& config)
{
	nes_vt_hummer_2mb(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hum_state::vt_external_space_map_4mbyte);
}

void nes_vt_pjoy_state::nes_vt_pjoy_4mb(machine_config &config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_sp69_state::vt_external_space_map_4mbyte);
	m_soc->set_201x_descramble(0x2, 0x3, 0x4, 0x5, 0x6, 0x7);
	m_soc->set_8000_scramble(0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x8, 0x7);
	m_soc->set_410x_scramble(0x8, 0x7);
	GFXDECODE(config, "gfxdecode", "soc:ppu", vt03_gfx_helper);
}


void nes_vt_sp69_state::nes_vt_4mb_sp69(machine_config& config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_sp69_state::vt_external_space_map_4mbyte);
	m_soc->set_201x_descramble(0x4, 0x7, 0x2, 0x6, 0x5, 0x3);
	m_soc->set_8000_scramble(0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x7, 0x8);
	GFXDECODE(config, "gfxdecode", "soc:ppu", vt03_gfx_helper);
}

void nes_vt_ablping_state::nes_vt_2mb_ablping(machine_config &config)
{
	NES_VT_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_ablping_state::vt_external_space_map_2mbyte);
	m_soc->set_201x_descramble(0x4, 0x7, 0x2, 0x6, 0x5, 0x3);
	m_soc->set_8000_scramble(0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x7, 0x8);

	m_soc->extra_read_2_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_r));
	m_soc->extra_read_3_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_r));
	m_soc->extra_write_2_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_w));
	m_soc->extra_write_3_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_w));

	GFXDECODE(config, "gfxdecode", "soc:ppu", vt03_gfx_helper);
}

uint8_t nes_vt_base_state::upper_412c_r()
{
	logerror("%s: upper_412c_r\n", machine().describe_context());
	return 0x00;
}

uint8_t nes_vt_base_state::upper_412d_r()
{
	logerror("%s: upper_412d_r\n", machine().describe_context());
	return 0x00;
}

void nes_vt_base_state::upper_412c_w(uint8_t data)
{
	logerror("%s: upper_412c_w %02x\n", machine().describe_context(), data);
}


void nes_vt_state::nes_vt_4k_ram(machine_config &config)
{
	/* basic machine hardware */
	NES_VT_SOC_4KRAM(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	GFXDECODE(config, "gfxdecode", "soc:ppu", vt03_gfx_helper);

	dynamic_cast<nes_vt_soc_4kram_device&>(*m_soc).upper_read_412c_callback().set(FUNC(nes_vt_state::upper_412c_r));
	dynamic_cast<nes_vt_soc_4kram_device&>(*m_soc).upper_read_412d_callback().set(FUNC(nes_vt_state::upper_412d_r));
	dynamic_cast<nes_vt_soc_4kram_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt_state::upper_412c_w));
}

void nes_vt_state::nes_vt_4k_ram_16mb(machine_config &config)
{
	nes_vt_4k_ram(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_16mbyte);
}

void nes_vt_state::nes_vt_4k_ram_pal(machine_config &config)
{
	nes_vt_4k_ram(config); // TODO, use PAL
}

void nes_vt_cy_state::nes_vt_cy(machine_config &config)
{
	nes_vt_4k_ram(config);

	NES_VT_SOC_4KRAM_CY(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
}

void nes_vt_cy_state::nes_vt_cy_bigger(machine_config &config)
{
	nes_vt_cy(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_cy_state::vt_external_space_map_32mbyte); // must be some banking of this kind of VT can address over 32mb
}

void nes_vt_cy_state::nes_vt_bt(machine_config &config)
{
	nes_vt_4k_ram(config);

	NES_VT_SOC_4KRAM_BT(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
}


void nes_vt_cy_state::bittboy_412c_w(uint8_t data)
{
	//bittboy (ok), mc_pg150 (not working)
	logerror("%s: vt03_412c_extbank_w %02x\n", machine().describe_context(),  data);
	m_ahigh = (data & 0x04) ? (1 << 24) : 0x0;
}

void nes_vt_cy_state::nes_vt_bt_2x16mb(machine_config& config)
{
	nes_vt_bt(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_cy_state::vt_external_space_map_bitboy_2x16mbyte);

	dynamic_cast<nes_vt_soc_4kram_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt_cy_state::bittboy_412c_w));
}

void nes_vt_dg_state::nes_vt_dg(machine_config &config)
{
	nes_vt_4k_ram(config);

	NES_VT_SOC_8KRAM_DG(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	/*
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_dg_state::nes_vt_dg_map);

	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((106.53/(PAL_APU_CLOCK.dvalue()/1000000)) *
	                         (ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 312);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	*/
}

void nes_vt_dg_state::nes_vt_dg_1mb(machine_config& config)
{
	nes_vt_dg(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_dg_state::vt_external_space_map_1mbyte);
}

void nes_vt_dg_state::nes_vt_dg_baddma_16mb(machine_config& config)
{
	nes_vt_dg(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_dg_state::vt_external_space_map_16mbyte);

}

void nes_vt_hh_state::nes_vt_vg(machine_config &config)
{
	nes_vt_dg(config);

	NES_VT_SOC_8KRAM_DG(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_default_palette_mode(PAL_MODE_NEW_VG);
	m_soc->force_bad_dma();
}

void nes_vt_hh_state::nes_vt_vg_8mb(machine_config& config)
{
	nes_vt_vg(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::vt_external_space_map_8mbyte);
}

void nes_vt_hh_state::nes_vt_vg_4mb(machine_config& config)
{
	nes_vt_vg(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::vt_external_space_map_4mbyte);
}

void nes_vt_hh_state::nes_vt_vg_16mb(machine_config& config)
{
	nes_vt_vg(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::vt_external_space_map_16mbyte);
}

void nes_vt_hh_state::nes_vt_vg_1mb_majkon(machine_config &config)
{
	nes_vt_dg(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::vt_external_space_map_1mbyte);

	m_soc->set_default_palette_mode(PAL_MODE_NEW_VG);
}


// New mystery handheld architecture, VTxx derived
void nes_vt_hh_state::nes_vt_hh(machine_config &config)
{
	nes_vt_4k_ram(config);

	NES_VT_SOC_4KRAM_HH(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB);
	m_soc->force_bad_dma();

	/*
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::nes_vt_hh_map);

	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((106.53/(PAL_APU_CLOCK.dvalue()/1000000)) *
	                         (ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 312);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	*/
}

void nes_vt_hh_state::nes_vt_hh_8mb(machine_config& config)
{
	nes_vt_hh(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::vt_external_space_map_8mbyte);
}

void nes_vt_hh_state::nes_vt_hh_4mb(machine_config& config)
{
	nes_vt_hh(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::vt_external_space_map_4mbyte);
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


static INPUT_PORTS_START( dbdancem )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_NAME("P1 Up Arrow") PORT_16WAY // NOT A JOYSTICK!!
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_NAME("P1 Down Arrow") PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_NAME("P1 Left Arrow") PORT_16WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("P1 Right Arrow") PORT_16WAY

	PORT_START("IO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_NAME("P2 Up Arrow") PORT_16WAY // NOT A JOYSTICK!!
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_NAME("P2 Down Arrow") PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_NAME("P2 Left Arrow") PORT_16WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_NAME("P2 Right Arrow") PORT_16WAY
INPUT_PORTS_END


uint8_t nes_vt_hh_state::fcpocket_412d_r()
{
	if (m_cartsel)
		return m_cartsel->read();
	else
		return 0;
}

void nes_vt_hh_state::fcpocket_412c_w(uint8_t data)
{
	// fcpocket
	logerror("%s: vtfp_412c_extbank_w %02x\n", machine().describe_context(), data);
	m_ahigh = (data & 0x01) ? (1 << 25) : 0x0;
}

void nes_vt_hh_state::nes_vt_fp(machine_config &config)
{
	nes_vt_4k_ram(config);

	NES_VT_SOC_4KRAM_FP(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB12);
	m_soc->force_bad_dma();
}



void nes_vt_hh_state::nes_vt_fp_4x16mb(machine_config& config)
{
	nes_vt_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::vt_external_space_map_fp_2x32mbyte);

	dynamic_cast<nes_vt_soc_4kram_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt_hh_state::fcpocket_412c_w));
	dynamic_cast<nes_vt_soc_4kram_device&>(*m_soc).upper_read_412d_callback().set(FUNC(nes_vt_hh_state::fcpocket_412d_r));
}

void nes_vt_hh_state::nes_vt_fp_32mb(machine_config& config)
{
	nes_vt_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::vt_external_space_map_32mbyte);
}

void nes_vt_hh_state::nes_vt_fp_bigger(machine_config& config)
{
	nes_vt_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::vt_external_space_map_32mbyte); // must be some kind of banking, or this VT can address > 32Mbyte
}

void nes_vt_hh_state::nes_vt_fp_16mb(machine_config& config)
{
	nes_vt_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::vt_external_space_map_16mbyte);
}

void nes_vt_hh_state::nes_vt_fp_pal(machine_config &config)
{
	nes_vt_fp(config);

	// set to PAL
}

void nes_vt_hh_state::nes_vt_fp_pal_32mb(machine_config& config)
{
	nes_vt_4k_ram(config);

	NES_VT_SOC_4KRAM_FP_PAL(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB12);
	m_soc->force_bad_dma();

	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hh_state::vt_external_space_map_32mbyte);
}


void nes_vt_dg_state::nes_vt_fa(machine_config& config)
{
	nes_vt_4k_ram(config);

	NES_VT_SOC_8KRAM_FA(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
}


uint8_t nes_vt_dg_state::fapocket_412c_r()
{
	if (m_cartsel)
		return m_cartsel->read();
	else
		return 0;
}

void nes_vt_dg_state::fapocket_412c_w(uint8_t data)
{
	// fapocket (ok?) (also uses bank from config switch for fake cartridge slot)
	logerror("%s: vtfa_412c_extbank_w %02x\n", machine().describe_context(), data);
	m_ahigh = 0;
	m_ahigh |= (data & 0x01) ? (1 << 25) : 0x0;
	m_ahigh |= (data & 0x02) ? (1 << 24) : 0x0;
}

void nes_vt_dg_state::nes_vt_fa_4x16mb(machine_config& config) // fapocket
{
	nes_vt_fa(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_dg_state::vt_external_space_map_fapocket_4x16mbyte);

	dynamic_cast<nes_vt_soc_4kram_device&>(*m_soc).upper_read_412c_callback().set(FUNC(nes_vt_dg_state::fapocket_412c_r));
	dynamic_cast<nes_vt_soc_4kram_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt_dg_state::fapocket_412c_w));
}


void nes_vt_swap_op_d5_d6_state::nes_vt_vh2009(machine_config &config)
{
	NES_VT_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	NES_VT_SOC_SCRAMBLE(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	//m_soc->set_default_palette_mode(PAL_MODE_NEW_VG); // gives better title screens, but worse ingame, must be able to switch
}

void nes_vt_swap_op_d5_d6_state::nes_vt_vh2009_4mb(machine_config& config)
{
	nes_vt_vh2009(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_swap_op_d5_d6_state::vt_external_space_map_4mbyte);
}

void nes_vt_swap_op_d5_d6_state::nes_vt_vh2009_8mb(machine_config& config)
{
	nes_vt_vh2009(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_swap_op_d5_d6_state::vt_external_space_map_8mbyte);
}


void nes_vt_swap_op_d5_d6_state::nes_vt_senwld_512kb(machine_config &config)
{
	nes_vt_vh2009(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_swap_op_d5_d6_state::vt_external_space_map_senwld_512kbyte);
	m_soc->set_default_palette_mode(PAL_MODE_NEW_VG);
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

ROM_START( lxcmcysp )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "lexibookspiderman.bin", 0x00000, 0x4000000, CRC(ef6e8847) SHA1(0012df193c52fd48595d85886fd431619c5d5e3e) )
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

ROM_START( dbdancem )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "dancemania_29lv160bt_00c222a7.bin", 0x00000, 0x200000, CRC(7250a837) SHA1(7205936215df84e3642c34a8b5991e8125da1785) )
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

ROM_START( zudugo )
	ROM_REGION( 0x400000, "mainrom", ROMREGION_ERASEFF )
	ROM_LOAD( "zudugo.bin", 0x00000, 0x400000, CRC(0fa9d9ad) SHA1(7533eaf51785d8fcced900ea0498281b0cf49dbf) )
ROM_END

ROM_START( ablping )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "abl_pingpong.bin", 0x00000, 0x200000, CRC(b31de1fb) SHA1(94e8afb2315ba1fa0892191c8e1832391e401c70) )
ROM_END



ROM_START( dgun2573 ) // this one lacked a DreamGear logo but was otherwise physically identical, is it a clone product or did DreamGear drop the logo in favor of just using the 'My Arcade' brand?
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "myarcadegamerportable_s29gl256p10tfi01_0001227e.bin", 0x00000, 0x2000000, CRC(8f8c8da7) SHA1(76a18458922e39abe1982f05f184babb5e65acf2) )
ROM_END

ROM_START( dgun2573a )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "myarcadegamerportabledreamgear_s29gl256p10tfi01_0001227e.bin", 0x00000, 0x2000000, CRC(928c41ad) SHA1(c0119a13a47a5b784d0b834d1451973ff0b4a84f) )
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
CONS( 200?, sudopptv,  0, 0,  nes_vt_waixing_512kb,        nes_vt, nes_vt_waixing_state, empty_init, "Smart Planet", "Sudoku Plug & Play TV Game '6 Intelligent Games'", MACHINE_NOT_WORKING )

CONS( 200?, megapad,   0, 0,  nes_vt_waixing_2mb,        nes_vt, nes_vt_waixing_state, empty_init, "Waixing", "Megapad 31-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Happy Biqi has broken sprites, investigate before promoting

// 060303 date code on PCB
CONS( 2006, ablmini,   0, 0,  nes_vt_waixing_alt_pal_8mb, nes_vt, nes_vt_waixing_alt_state, empty_init, "Advance Bright Ltd", "Double Players Mini Joystick 80-in-1 (MJ8500, ABL TV Game)", MACHINE_IMPERFECT_GRAPHICS )

// silver 'N64' type controller design
CONS( 200?, zudugo,    0, 0,  nes_vt_waixing_alt_4mb,     nes_vt, nes_vt_waixing_alt_state, empty_init, "<unknown>", "Zudu-go / 2udu-go", MACHINE_IMPERFECT_GRAPHICS ) // the styling on the box looks like a '2' in places, a 'Z' in others.

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
// VG Pocket Caplet is SunPlus hardware instead, see spg2xx_lexibook.cpp

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
CONS( 200?, lxcmcysp,  0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Lexibook", "Lexibook Compact Cyber Arcade - Marvel Ultimate Spider-Man", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme

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
	Lexibook Retro TV Game Console (300 Games) - PJ Masks

    (more?)
*/

// intial code isn't valid? scrambled?
CONS( 201?, red5mam,  0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Red5", "Mini Arcade Machine (Red5)", MACHINE_NOT_WORKING ) // 128Mbyte ROM, must be externally banked or different addressing scheme



CONS( 201?, denv150,   0,  0,  nes_vt_cy_bigger, nes_vt, nes_vt_cy_lexibook_state, empty_init, "Denver", "Denver Game Console GMP-240C 150-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )


// CPU die is marked 'VH2009' There's also a 62256 RAM chip on the PCB, some scrambled opcodes
CONS( 200?, polmega,   0,  0,  nes_vt_vh2009_4mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "Polaroid", "Megamax GPD001SDG", MACHINE_NOT_WORKING )
CONS( 200?, silv35,    0,  0,  nes_vt_vh2009_4mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "SilverLit", "35 in 1 Super Twins", MACHINE_NOT_WORKING )
// die is marked as VH2009, as above, but no scrambled opcodes here
CONS( 201?, techni4,   0,  0,  nes_vt_pal_2mb,      nes_vt, nes_vt_state,        empty_init, "Technigame", "Technigame Super 4-in-1 Sports (PAL)", MACHINE_IMPERFECT_GRAPHICS )


// same encryption as above, but seems like newer hardware (or the above aren't using most of the features)
CONS( 200?, lpgm240,    0,  0,  nes_vt_vh2009_8mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "<unknown>", "Let's Play! Game Machine 240 in 1", MACHINE_NOT_WORKING ) // mini 'retro-arcade' style cabinet

// this has 'Shark' and 'Octopus' etc. like mc_dgear but uses scrambled bank registers
// This was also often found in cart form with SunPlus / Famiclone hybrid systems to boost the game count, eg. the WiWi (ROM verified to match)
CONS( 200?, mc_sp69,   0,  0,  nes_vt_4mb_sp69,    nes_vt, nes_vt_sp69_state, empty_init, "<unknown>", "Sports Game 69 in 1", MACHINE_IMPERFECT_GRAPHICS  | MACHINE_IMPERFECT_SOUND)

// this game was also sold by dreamGEAR and several others companies, each time with a different name and different case, although the dumped version was from ABL, and it hasn't been confirmed that the ROMs are identical for the other units
// Super Ping Pong appears on the title screen, but not the box / product art which simply has "Ping Pong Plug & Play TV Game" on front/back/bottom/manual, and "Table Tennis Plug & Play TV Game" on left/right sides.  Product code is PP1100
// PCB has PP1100-MB 061110 on it, possible date YYMMDD code? (pinball is 050329, guitar fever is 070516, air blaster 050423, kickboxing 061011 etc.)
CONS( 2006, ablping,   0,        0,  nes_vt_2mb_ablping, nes_vt, nes_vt_ablping_state, empty_init, "Advance Bright Ltd", "Ping Pong / Table Tennis / Super Ping Pong (PP1100, ABL TV Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// Hummer systems, scrambled bank register
CONS( 200?, mc_sam60,  0,  0,  nes_vt_hummer_2mb,    nes_vt, nes_vt_hum_state, empty_init, "Hummer Technology Co., Ltd.", "Samuri (60 in 1)", MACHINE_IMPERFECT_GRAPHICS  | MACHINE_IMPERFECT_SOUND )
CONS( 200?, zdog,      0,  0,  nes_vt_hummer_4mb,    nes_vt, nes_vt_hum_state, empty_init, "Hummer Technology Co., Ltd.", "ZDog (44 in 1)", MACHINE_IMPERFECT_GRAPHICS  | MACHINE_IMPERFECT_SOUND )

// very plain menus
CONS( 200?, pjoyn50,    0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "PowerJoy Navigator 50 in 1", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, pjoys30,    0,        0,  nes_vt_pjoy_4mb,    nes_vt, nes_vt_pjoy_state, empty_init, "<unknown>", "PowerJoy Supermax 30 in 1", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, pjoys60,    0,        0,  nes_vt_pjoy_4mb,    nes_vt, nes_vt_pjoy_state, empty_init, "<unknown>", "PowerJoy Supermax 60 in 1", MACHINE_IMPERFECT_GRAPHICS )
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

// there is also a 'Spectra Light Edition' which could be a different ROM as the title screen on this one does show the unit type.
CONS( 2006, dbdancem,   0,        0,  nes_vt_2mb, dbdancem, nes_vt_state, empty_init, "Senario", "Double Dance Mania - Techno Light Edition",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

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


// available in a number of colours, with various brands, but likely all the same.
// This was a red coloured pad, contains various unlicensed bootleg reskinned NES game eg Blob Buster is a hack of Dig Dug 2 and there are also hacks of Xevious, Donkey Kong Jr, Donkey Kong 3 and many others.
CONS( 201?, ppgc200g,   0,         0,  nes_vt_8mb, nes_vt, nes_vt_state, empty_init, "<unknown>", "Plug & Play Game Controller with 200 Games (Supreme 200)", MACHINE_IMPERFECT_GRAPHICS )


// Probably VT09 or similar
// Use DIP switch to select console or cartridge, as cartridge is fake and just toggles a ROM high address bit
// (which can also be overriden by GPIO)
CONS( 2017, fapocket,   0,        0,  nes_vt_fa_4x16mb, nes_vt_fa, nes_vt_dg_fapocket_state, empty_init, "<unknown>",   "Family Pocket 638 in 1", MACHINE_IMPERFECT_GRAPHICS ) // has external banking (4x 16mbyte banks)


CONS( 2017, otrail,     0,        0,  nes_vt_dg_1mb, nes_vt, nes_vt_dg_state, empty_init, "Basic Fun", "The Oregon Trail", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2005, senwld,   0,          0,  nes_vt_senwld_512kb,    nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "Senario", "Win, Lose or Draw (Senario)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // needs RAM in banked space, Alpha display emulating, Touchpad emulating etc.



// Runs well, minor GFX issues in intro
CONS( 2017, sy889,      0,        0,  nes_vt_hh_8mb, nes_vt, nes_vt_hh_state, empty_init, "SY Corp",   "SY-889 300 in 1 Handheld", MACHINE_IMPERFECT_GRAPHICS )
CONS( 2016, sy888b,     0,        0,  nes_vt_hh_4mb, nes_vt, nes_vt_hh_state, empty_init, "SY Corp",   "SY-888B 288 in 1 Handheld", MACHINE_IMPERFECT_GRAPHICS )

// Same hardware as SY-889
CONS( 201?, mc_cb280,   0,        0,  nes_vt_hh_4mb, nes_vt, nes_vt_hh_state, empty_init, "CoolBoy",   "Coolboy RS-18 (280 in 1)", MACHINE_IMPERFECT_GRAPHICS )

// Plays intro music but then crashes. same hardware as SY-88x but uses more features
CONS( 2016, mog_m320,   0,        0,  nes_vt_hh_8mb, nes_vt, nes_vt_hh_state, empty_init, "MOGIS",    "MOGIS M320 246 in 1 Handheld", MACHINE_NOT_WORKING )


// similar menus to above, but with opcode scrambling
CONS( 2015, dgun2573,  0,         0,  nes_vt_fp_32mb,     nes_vt, nes_vt_hh_state, empty_init, "dreamGEAR", "My Arcade Gamer V Portable Gaming System (DGUN-2573) (set 1, newer)",  MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2015, dgun2573a, dgun2573,  0,  nes_vt_fp_32mb,     nes_vt, nes_vt_hh_state, empty_init, "dreamGEAR", "My Arcade Gamer V Portable Gaming System (DGUN-2573) (set 2, older)",  MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // some menu graphics haven't been updated to reflect 'Panda' theme to the sports games

CONS( 2015, rminitv,   0,  0,  nes_vt_fp_pal_32mb, nes_vt, nes_vt_hh_state, empty_init, "Orb Gaming", "Retro 'Mini TV' Console 300-in-1", MACHINE_IMPERFECT_GRAPHICS ) // single 32Mbyte bank!
// New platform with scrambled opcodes, same as DGUN-2561. Runs fine with minor GFX and sound issues in menu
// Use DIP switch to select console or cartridge, as cartridge is fake and just toggles a GPIO
CONS( 2016, fcpocket,  0,  0,  nes_vt_fp_4x16mb,   nes_vt_fp, nes_vt_hh_state, empty_init, "<unknown>",   "FC Pocket 600 in 1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )  // has external banking (2x 32mbyte banks)

