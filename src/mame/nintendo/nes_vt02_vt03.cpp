// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  nes_vt.cpp

  VT02/VT03 systems go in here

  - 2KB RAM
  - VT03 adds alt palette style (not available on VT02?)

  NON-bugs (same happens on real hardware)

  Pause screen has corrupt GFX on enhanced version of Octopus

***************************************************************************/

#include "emu.h"
#include "nes_vt_soc.h"


namespace {

class nes_vt_base_state : public driver_device
{
public:
	nes_vt_base_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_io0(*this, "IO0"),
		m_io1(*this, "IO1"),
		m_exin(*this, "EXTRAIN%u", 0U),
		m_prgrom(*this, "mainrom")
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint8_t in0_r();
	virtual uint8_t in1_r();
	virtual void in0_w(uint8_t data);

	void nes_vt_map(address_map &map) ATTR_COLD;

	optional_ioport m_io0;
	optional_ioport m_io1;

	uint8_t m_latch0;
	uint8_t m_latch1;
	uint8_t m_previous_port0;

	optional_ioport_array<4> m_exin;

	required_region_ptr<uint8_t> m_prgrom;

	uint8_t vt_rom_r(offs_t offset);
	[[maybe_unused]] void vtspace_w(offs_t offset, uint8_t data);

	void configure_soc(nes_vt02_vt03_soc_device* soc);

	[[maybe_unused]] uint8_t upper_412c_r();
	[[maybe_unused]] uint8_t upper_412d_r();
	[[maybe_unused]] void upper_412c_w(uint8_t data);

private:
	/* Extra IO */
	template <uint8_t NUM> uint8_t extrain_r();
};

class nes_vt_state : public nes_vt_base_state
{
public:
	nes_vt_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_base_state(mconfig, type, tag),
		m_soc(*this, "soc")
	{ }

	void nes_vt_pal_1mb(machine_config& config);
	void nes_vt_pal_2mb(machine_config& config);
	void nes_vt_pal_4mb(machine_config& config);
	void nes_vt_pal_8mb(machine_config& config);
	void nes_vt_pal_16mb(machine_config& config);

	void nes_vt_512kb(machine_config& config);
	void nes_vt_1mb(machine_config& config);
	void nes_vt_2mb(machine_config& config);
	void nes_vt_4mb(machine_config& config);
	void nes_vt_8mb(machine_config& config);
	void nes_vt_16mb(machine_config& config);
	void nes_vt_32mb(machine_config& config);

	void nes_vt_1mb_majkon(machine_config& config);

	void vt_external_space_map_32mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_16mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_8mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_4mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_2mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_1mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_512kbyte(address_map &map) ATTR_COLD;

	void vt_external_space_map_1mbyte_majkon(address_map &map) ATTR_COLD;

	void init_protpp();
	void init_gamezn2();

protected:
	required_device<nes_vt02_vt03_soc_device> m_soc;
};


class nes_vt_swap_op_d5_d6_state : public nes_vt_state
{
public:
	nes_vt_swap_op_d5_d6_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

	void nes_vt_vh2009(machine_config& config);
	void nes_vt_vh2009_pal(machine_config& config);
	void nes_vt_vh2009_1mb(machine_config& config);
	void nes_vt_vh2009_2mb(machine_config& config);
	void nes_vt_vh2009_pal_2mb(machine_config& config);
	void nes_vt_vh2009_4mb(machine_config& config);
	void nes_vt_vh2009_8mb(machine_config& config);

	void nes_vt_senwld_512kb(machine_config& config);

protected:
	void vt_external_space_map_senwld_512kbyte(address_map &map) ATTR_COLD;
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
	void nes_vt_waixing_512kb_rasterhack(machine_config& config);
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

class nes_vt_waixing_alt_sporzpp_state : public nes_vt_waixing_alt_state
{
public:
	nes_vt_waixing_alt_sporzpp_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_waixing_alt_state(mconfig, type, tag)
	{ }

	void nes_vt_waixing_alt_4mb_sporzpp(machine_config& config);
	void nes_vt_pal_4mb_sporzbxa(machine_config& config);

private:
	uint8_t in1_r() override
	{
		uint8_t i = machine().rand() & 0x18;
		uint8_t ret = m_io1->read() & ~0x18;
		return i | ret;
	}
};

class nes_vt_wldsoctv_state : public nes_vt_state
{
public:
	nes_vt_wldsoctv_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag)
	{ }

private:
	uint8_t in1_r() override
	{
		uint8_t i = machine().rand() & 0x18;
		uint8_t ret = m_io1->read() & ~0x18;
		return i | ret;
	}
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
	void nes_vt_2mb_vfootbal(machine_config& config);

private:
	uint8_t ablping_extraio_r();
	void ablping_extraio_w(uint8_t data);
};


class nes_vt_ablpinb_state : public nes_vt_state
{
public:
	nes_vt_ablpinb_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt_state(mconfig, type, tag),
		m_ablpinb_in0_val(0),
		m_plunger(*this, "PLUNGER")
	{ }

	void nes_vt_waixing_alt_4mb_sporzpp(machine_config& config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	virtual uint8_t in0_r() override;
	virtual uint8_t in1_r() override;
	virtual void in0_w(uint8_t data) override;

	uint8_t m_ablpinb_in0_val;

	int m_plunger_off;
	int m_plunger_state_count;

	required_ioport m_plunger;
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
	map(0x0000000, 0x1ffffff).r(FUNC(nes_vt_state::vt_rom_r));
}

void nes_vt_state::vt_external_space_map_16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(nes_vt_state::vt_rom_r));
}

void nes_vt_state::vt_external_space_map_8mbyte(address_map &map)
{
	map(0x0000000, 0x07fffff).mirror(0x1800000).r(FUNC(nes_vt_state::vt_rom_r));
}

void nes_vt_state::vt_external_space_map_4mbyte(address_map &map)
{
	map(0x0000000, 0x03fffff).mirror(0x1c00000).r(FUNC(nes_vt_state::vt_rom_r));
}

void nes_vt_state::vt_external_space_map_2mbyte(address_map &map)
{
	map(0x0000000, 0x01fffff).mirror(0x1e00000).r(FUNC(nes_vt_state::vt_rom_r));
}

void nes_vt_state::vt_external_space_map_1mbyte(address_map &map)
{
	map(0x0000000, 0x00fffff).mirror(0x1f00000).r(FUNC(nes_vt_state::vt_rom_r));
}

void nes_vt_state::vt_external_space_map_512kbyte(address_map &map)
{
	map(0x0000000, 0x007ffff).mirror(0x1f80000).r(FUNC(nes_vt_state::vt_rom_r));
}

// Win Lose Draw has RAM as well as ROM
void nes_vt_swap_op_d5_d6_state::vt_external_space_map_senwld_512kbyte(address_map &map)
{
	map(0x0000000, 0x007ffff).r(FUNC(nes_vt_swap_op_d5_d6_state::vt_rom_r));
	map(0x0100000, 0x010ffff).ram();
	map(0x0180000, 0x01fffff).r(FUNC(nes_vt_swap_op_d5_d6_state::vt_rom_r));
}

void nes_vt_state::vt_external_space_map_1mbyte_majkon(address_map &map)
{
	map(0x0000000, 0x00fffff).mirror(0x1f00000).r(FUNC(nes_vt_state::vt_rom_r));
	map(0x1400000, 0x1401fff).ram(); // rush'n attack writes to chr space, after setting the program and character outer bank to a mirror, is the correct way to handle it?
}

template <uint8_t NUM> uint8_t nes_vt_base_state::extrain_r()
{
	if (m_exin[NUM])
		return m_exin[NUM]->read();
	else
	{
		logerror("%s: extrain_r (port %d) (not hooked up)\n", NUM, machine().describe_context());
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

	// need to check this or some games (eg cybar120 Aero Engine) won't have working inputs as they repeatedly write a pattern of 02 / 00 here between fetches which resets the latch
	if ((data & 0x01) != (m_previous_port0 & 0x01))
	{
		if (data & 0x01)
		{
			m_latch0 = m_io0->read();
			m_latch1 = m_io1->read();
		}
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
	m_latch0 = 0;
	m_latch1 = 0;
	m_previous_port0 = 0;

	save_item(NAME(m_latch0));
	save_item(NAME(m_latch1));
	save_item(NAME(m_previous_port0));
}

void nes_vt_base_state::machine_reset()
{

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


void nes_vt_base_state::configure_soc(nes_vt02_vt03_soc_device* soc)
{
	soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_32mbyte);
	soc->read_0_callback().set(FUNC(nes_vt_base_state::in0_r));
	soc->read_1_callback().set(FUNC(nes_vt_base_state::in1_r));
	soc->write_0_callback().set(FUNC(nes_vt_base_state::in0_w));

	soc->extra_read_0_callback().set(FUNC(nes_vt_base_state::extrain_r<0>));
	soc->extra_read_1_callback().set(FUNC(nes_vt_base_state::extrain_r<1>));
	soc->extra_read_2_callback().set(FUNC(nes_vt_base_state::extrain_r<2>));
	soc->extra_read_3_callback().set(FUNC(nes_vt_base_state::extrain_r<3>));
}

void nes_vt_state::nes_vt_512kb(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_512kbyte);
}

void nes_vt_state::nes_vt_1mb(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_1mbyte);
}

void nes_vt_state::nes_vt_2mb(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_2mbyte);
}



void nes_vt_state::nes_vt_4mb(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_4mbyte);
}

void nes_vt_state::nes_vt_8mb(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_8mbyte);
}

void nes_vt_state::nes_vt_16mb(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_16mbyte);
}

void nes_vt_state::nes_vt_32mb(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_32mbyte);
}

void nes_vt_state::nes_vt_pal_1mb(machine_config& config)
{
	NES_VT02_VT03_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_1mbyte);
}

void nes_vt_state::nes_vt_pal_2mb(machine_config& config)
{
	NES_VT02_VT03_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_2mbyte);
}

void nes_vt_state::nes_vt_pal_4mb(machine_config& config)
{
	NES_VT02_VT03_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_4mbyte);
}

void nes_vt_state::nes_vt_pal_8mb(machine_config& config)
{
	NES_VT02_VT03_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_8mbyte);
}

void nes_vt_state::nes_vt_pal_16mb(machine_config& config)
{
	NES_VT02_VT03_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_16mbyte);
}


void nes_vt_waixing_state::nes_vt_waixing_512kb(machine_config &config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_waixing_state::vt_external_space_map_512kbyte);
	m_soc->set_201x_descramble(0x3, 0x2, 0x7, 0x6, 0x5, 0x4);
}

void nes_vt_waixing_state::nes_vt_waixing_512kb_rasterhack(machine_config &config)
{
	nes_vt_waixing_512kb(config);
	m_soc->force_raster_timing_hack();
}


void nes_vt_waixing_state::nes_vt_waixing_2mb(machine_config &config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_waixing_state::vt_external_space_map_2mbyte);
	m_soc->set_201x_descramble(0x3, 0x2, 0x7, 0x6, 0x5, 0x4);
}

void nes_vt_waixing_alt_state::nes_vt_waixing_alt_4mb(machine_config &config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_waixing_state::vt_external_space_map_4mbyte);
	m_soc->set_201x_descramble(0x3, 0x2, 0x7, 0x6, 0x5, 0x4);
	m_soc->set_8000_scramble(0x5, 0x4, 0x3, 0x2, 0x7, 0x6, 0x7, 0x8);
}

void nes_vt_waixing_alt_state::nes_vt_waixing_alt_pal_8mb(machine_config &config)
{
	NES_VT02_VT03_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_waixing_alt_state::vt_external_space_map_8mbyte);
	m_soc->set_201x_descramble(0x3, 0x2, 0x7, 0x6, 0x5, 0x4);
	m_soc->set_8000_scramble(0x5, 0x4, 0x3, 0x2, 0x7, 0x6, 0x7, 0x8);
}

void nes_vt_waixing_alt_sporzpp_state::nes_vt_waixing_alt_4mb_sporzpp(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_ablping_state::vt_external_space_map_4mbyte);
	m_soc->set_201x_descramble(0x3, 0x2, 0x7, 0x6, 0x5, 0x4);
	m_soc->set_8000_scramble(0x5, 0x4, 0x3, 0x2, 0x7, 0x6, 0x7, 0x8);
}

void nes_vt_waixing_alt_sporzpp_state::nes_vt_pal_4mb_sporzbxa(machine_config& config)
{
	NES_VT02_VT03_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_4mbyte);
}

void nes_vt_hum_state::nes_vt_hummer_2mb(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_sp69_state::vt_external_space_map_2mbyte);
	m_soc->set_201x_descramble(0x7, 0x6, 0x5, 0x4, 0x2, 0x3);
	m_soc->set_8000_scramble(0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x7, 0x8);
}

void nes_vt_hum_state::nes_vt_hummer_4mb(machine_config& config)
{
	nes_vt_hummer_2mb(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_hum_state::vt_external_space_map_4mbyte);
}

void nes_vt_pjoy_state::nes_vt_pjoy_4mb(machine_config &config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_sp69_state::vt_external_space_map_4mbyte);
	m_soc->set_201x_descramble(0x2, 0x3, 0x4, 0x5, 0x6, 0x7);
	m_soc->set_8000_scramble(0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x8, 0x7);
	m_soc->set_410x_scramble(0x8, 0x7);
}


void nes_vt_sp69_state::nes_vt_4mb_sp69(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_sp69_state::vt_external_space_map_4mbyte);
	m_soc->set_201x_descramble(0x4, 0x7, 0x2, 0x6, 0x5, 0x3);
	m_soc->set_8000_scramble(0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x7, 0x8);
}

void nes_vt_ablping_state::nes_vt_2mb_ablping(machine_config &config)
{
	NES_VT02_VT03_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_ablping_state::vt_external_space_map_2mbyte);
	m_soc->set_201x_descramble(0x4, 0x7, 0x2, 0x6, 0x5, 0x3);
	m_soc->set_8000_scramble(0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x7, 0x8);

	m_soc->extra_read_2_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_r));
	m_soc->extra_read_3_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_r));
	m_soc->extra_write_2_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_w));
	m_soc->extra_write_3_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_w));
}

void nes_vt_ablping_state::nes_vt_2mb_vfootbal(machine_config &config)
{
	NES_VT02_VT03_SOC_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_ablping_state::vt_external_space_map_2mbyte);

	m_soc->extra_read_2_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_r));
	m_soc->extra_read_3_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_r));
	m_soc->extra_write_2_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_w));
	m_soc->extra_write_3_callback().set(FUNC(nes_vt_ablping_state::ablping_extraio_w));
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


void nes_vt_state::nes_vt_1mb_majkon(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_state::vt_external_space_map_1mbyte_majkon);
	m_soc->force_raster_timing_hack();
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

static INPUT_PORTS_START( lxnoddy )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1) // not used?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) // not used?
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) // not used?
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY // steer left?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY // steer right?

	PORT_START("IO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
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


void nes_vt_swap_op_d5_d6_state::nes_vt_vh2009(machine_config &config)
{
	NES_VT02_VT03_SOC_SCRAMBLE(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
}

void nes_vt_swap_op_d5_d6_state::nes_vt_vh2009_pal(machine_config &config)
{
	NES_VT02_VT03_SOC_SCRAMBLE_PAL(config, m_soc, PAL_APU_CLOCK);
	configure_soc(m_soc);
}

void nes_vt_swap_op_d5_d6_state::nes_vt_vh2009_1mb(machine_config& config)
{
	nes_vt_vh2009(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_swap_op_d5_d6_state::vt_external_space_map_1mbyte);
}

void nes_vt_swap_op_d5_d6_state::nes_vt_vh2009_2mb(machine_config& config)
{
	nes_vt_vh2009(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_swap_op_d5_d6_state::vt_external_space_map_2mbyte);
}

void nes_vt_swap_op_d5_d6_state::nes_vt_vh2009_pal_2mb(machine_config& config)
{
	nes_vt_vh2009_pal(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt_swap_op_d5_d6_state::vt_external_space_map_2mbyte);
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
}




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


static INPUT_PORTS_START( sporzpp )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("IO1")
	PORT_DIPNAME( 0x0001, 0x0000, "P2:0001" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0001, "0001" )
	PORT_DIPNAME( 0x0002, 0x0000, "P2:0002" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0002, "0002" )
	PORT_DIPNAME( 0x0004, 0x0000, "P2:0004" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0004, "0004" )
	PORT_DIPNAME( 0x0008, 0x0000, "P2:0008" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0008, "0008" )
	PORT_DIPNAME( 0x0010, 0x0000, "P2:0010" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0010, "0010" )
	PORT_DIPNAME( 0x0020, 0x0000, "P2:0020" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0020, "0020" )
	PORT_DIPNAME( 0x0040, 0x0000, "P2:0040" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0040, "0040" )
	PORT_DIPNAME( 0x0080, 0x0000, "P2:0080" )
	PORT_DIPSETTING(      0x0000, "0000" )
	PORT_DIPSETTING(      0x0080, "0080" )
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

ROM_START( sudopptv )
	ROM_REGION( 0x80000, "mainrom", ROMREGION_ERASEFF )
	ROM_LOAD( "sudokupnptvgame_29lv400tc_000422b9.bin", 0x00000, 0x80000, CRC(722cc36d) SHA1(1f6d1f57478cf175a36722b39c52eded4b669f81) )
ROM_END

ROM_START( mc_dgear )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "dreamgear 75-in-1.prg", 0x00000, 0x400000, CRC(9aabcb8f) SHA1(aa9446b7777fa64503871225fcaf2a17aafd9af1) )
ROM_END

ROM_START( sudo6in1 )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "6n1sudoku.bin", 0x00000, 0x100000, CRC(31089cd4) SHA1(dbfe41d327278dbfa46c7ad7ef327c20648562c1) )
ROM_END

ROM_START( sen101 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "101n1.bin", 0x00000, 0x400000, CRC(b03e1824) SHA1(c9ac4e16220414c1aa679133191140ced9986e9c) )
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

ROM_START( vsmaxx17 )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "vsmaxx17.bin", 0x00000, 0x200000, CRC(f3fccbb9) SHA1(8b70b10d28f03e72f6b35199001955033a65fd5d) )  // M6MG3D641RB
ROM_END

ROM_START( vsmax25v )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "vsmaxx25n1_2.bin", 0x00000, 0x400000, CRC(e17e076d) SHA1(0f4e3b6b33ab75dcc12dc02d3347ddb53275c777) )
ROM_END

ROM_START( dgun851 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "dgun851.bin", 0x00000, 0x400000, CRC(9d51c9fc) SHA1(6f49ea3343eb6e90938aabc9660783f1fc7f6084) )
ROM_END

ROM_START( dgun853 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "dgpnp50n1dgun853.bin", 0x00000, 0x800000, CRC(118f7286) SHA1(0f4ad7141e887bddba1ab37e75de08e9d56ad841) )
ROM_END

ROM_START( vsmaxx77 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "vsmaxx77.bin", 0x00000, 0x800000, CRC(03f1f4b5) SHA1(13f7ecea3765cffcd3065de713abdabd24946b99) )
ROM_END

ROM_START( vsmaxxvd )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "vsmaxxvideo.bin", 0x00000, 0x800000, CRC(af365a77) SHA1(8119fcef3e1a2ade93d36740d5df451919f0e541) )
ROM_END

ROM_START( polmega )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "megamax.bin", 0x00000, 0x400000, CRC(ef3aade3) SHA1(0c130080ace000cbe43e70a805d4301e05840294) )
ROM_END

ROM_START( silv35 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "silverlit35.bin", 0x00000, 0x400000, CRC(7540e350) SHA1(a0cb456136560fa4d8a365dd44d815ec0e9fc2e7) )
ROM_END

ROM_START( sporzpp )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "gamesporzduetpingpong.bin", 0x00000, 0x400000, CRC(96af199b) SHA1(c14ff15683545c1edf03376cebcee7ac408da733) )
ROM_END

ROM_START( sporzbx )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "sporzboxing.bin", 0x00000, 0x400000, CRC(8b832c0b) SHA1(8193955a81e894a01308a80d5153f2ecfe134da6) )
ROM_END

ROM_START( sporzbxa )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "wlboxing.bin", 0x00000, 0x400000, CRC(5df7beb9) SHA1(dadcec310e4a7b3ca061c6fe6be319cda2445b24) )
ROM_END

ROM_START( sporztn )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "wirelesstennis.bin", 0x00000, 0x400000, CRC(e60f5ee1) SHA1(838ba7f4e9dcd0101eaaef5be883206d8856f45c) )
ROM_END

ROM_START( wldsoctv )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "worldsoccer.bin", 0x00000, 0x400000, CRC(8c0b184b) SHA1(fe1e7e83b9a2ae50dca1e7ea3bf7d691b8407511) )
ROM_END

ROM_START( solargm )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "solargames.bin", 0x00000, 0x800000, CRC(b49f0985) SHA1(68231614b333911c25168c533f1ae9bc79c36c38) )
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

ROM_START( joysti30 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "joystick30.bin", 0x00000, 0x400000, CRC(b3f089af) SHA1(478d53d38eeffdbc4a1271d0e060aeb29e919502) )
ROM_END

ROM_START( lxnoddy )
	ROM_REGION( 0x200000, "mainrom", 0 )
	// PCB had a ROM twice this capacity, but lower half didn't give consistent reads, and upper address line was tied to VCC, making lower half inaccessible anyway
	// They likely just used a known failed flash ROM
	ROM_LOAD( "noddy.bin", 0x00000, 0x200000, CRC(a955307f) SHA1(26bee6403bfe3c93831b02c090383593218f60a9) )
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

ROM_START( gprnrs1 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "gprnrs1.bin", 0x00000, 0x800000, CRC(c3ffcec8) SHA1(313a790fb51d0b155257f9de84726ed67da43a8f) )
ROM_END

ROM_START( gprnrs16 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "gprnrs16.bin", 0x00000, 0x2000000, CRC(bdffa40a) SHA1(3d01907211f18e8415171dfc6c1d23cf5952e7bb) )
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

ROM_START( timetp7 )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "gm802m.bin", 0x00000, 0x200000, CRC(2ab17abf) SHA1(8e7818043f8e670a35f8dbaebe318b872d95f3ca) )
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

ROM_START( vt25in1 )
	ROM_REGION( 0x100000, "mainrom", ROMREGION_ERASEFF )
	ROM_LOAD( "25in1.bin", 0x00000, 0x100000, CRC(1038b5ec) SHA1(e7d1ccafe0edcfa44c11412d2aa771f4ba96b5b8) )
ROM_END

ROM_START( pumpactv )
	ROM_REGION( 0x100000, "mainrom", ROMREGION_ERASEFF )
	ROM_LOAD( "pumpactive.bin", 0x00000, 0x100000, CRC(e3c07561) SHA1(2bfff426d72d481ba0647e9110f942d142a4625f) )
ROM_END

ROM_START( zudugo )
	ROM_REGION( 0x400000, "mainrom", ROMREGION_ERASEFF )
	ROM_LOAD( "zudugo.bin", 0x00000, 0x400000, CRC(0fa9d9ad) SHA1(7533eaf51785d8fcced900ea0498281b0cf49dbf) )
ROM_END

ROM_START( ablping )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "abl_pingpong.bin", 0x00000, 0x200000, CRC(b31de1fb) SHA1(94e8afb2315ba1fa0892191c8e1832391e401c70) )
ROM_END

ROM_START( mc_89in1 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "89in1.bin", 0x00000, 0x400000, CRC(b97f8ce5) SHA1(1a8e67f2b58a671ceec2b0ed18ec5954a71ae63a) )
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

ROM_START( protpp )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "vpingpong_s29al008d70tfi02_0001225b.bin", 0x00000, 0x100000, CRC(8cf46272) SHA1(298a6341d26712ec1f282e7514e995a7af5ac012) )
ROM_END

ROM_START( vfootbal )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "vfootball.u3", 0x00000, 0x200000, CRC(3b586f64) SHA1(92ee41ccfad32f7629bd43503cfb15e9624283ce) )
ROM_END

ROM_START( zdog )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "zdog.bin", 0x00000, 0x400000, CRC(5ed3485b) SHA1(5ab0e9370d4ed1535205deb0456878c4e400dd81) )
ROM_END

ROM_START( dgun2500 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "dgun2500.bin", 0x00000, 0x1000000, CRC(a2f963f3) SHA1(e29ed20ccdcf25b5640a607b3d2c9e6a4944e172) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(0x1000000)
ROM_END

ROM_START( ppgc200g )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "m29dw641.u2", 0x00000, 0x800000, CRC(b16dc677) SHA1(c1984fde4caf9345d41d127db946d1c21ec43ae0) )
ROM_END

ROM_START( dgun2869 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "myarcaderetromicro_s29gl128p11tfiv1_0001227e.bin", 0x00000, 0x1000000, CRC(5e7fded2) SHA1(cf55ae7a128e3254a22933150caf94e269303ffb) ) // 29GL128
	ROM_IGNORE(0x100)
ROM_END

ROM_START( dgun2959 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "dgun2959.bin", 0x00000, 0x1000000, CRC(6e9b2f45) SHA1(abac2c1783e99b02f9c44f714d5184aea86661ae) )
ROM_END

ROM_START( 88in1joy )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "88in1joystick.bin", 0x00000, 0x400000, CRC(86b8d819) SHA1(6da387b2e6ce02a3ec203e2af8a961959ba1cf62) )
ROM_END

ROM_START( gamezn2 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD16_WORD_SWAP( "gamezone2.bin", 0x00000, 0x400000, CRC(f7b2d609) SHA1(7d2d8f6e822c4e6b97e9accaa524b7910c6b97bf) ) // byteswapped as protection?
ROM_END

ROM_START( tvmjfc )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "s29al016m90tfir2_tsop48.bin", 0x00000, 0x200000, CRC(28ef6219) SHA1(7ac2592f2a88532f537629660074ebae08efab82) )
ROM_END




void nes_vt_state::init_protpp()
{
	// this gets the tiles correct
	u8 *src = memregion("mainrom")->base();
	int len = memregion("mainrom")->bytes();

	std::vector<u8> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			buffer[i] = bitswap<8>(src[i],3,1,2,0,7,5,6,4);
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}
}

void nes_vt_state::init_gamezn2()
{
	u8 *src = memregion("mainrom")->base();
	int len = memregion("mainrom")->bytes();

	std::vector<u8> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			buffer[i] = bitswap<8>(src[i],7,6,5,4,3,2,0,1); // bottom 2 bits are swapped?
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}
}

} // anonymous namespace


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

// need to map 2 player controls for Ping Pong, 'Eat-Bean' (the PacMan hack) gets stuck during intermission? (same happens on hardware?)
CONS( 2004, sporzpp,   0,  0,  nes_vt_waixing_alt_4mb_sporzpp,        sporzpp, nes_vt_waixing_alt_sporzpp_state, empty_init, "Macro Winners", "Game Sporz Wireless Duet Play Ping-Pong", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
// has some longer than expected delays when sounds should play on the Boxing part, but NES hacks are all functional
CONS( 2004, sporzbx,   0,        0,  nes_vt_waixing_alt_4mb_sporzpp,        sporzpp, nes_vt_waixing_alt_sporzpp_state, empty_init, "Macro Winners",                       "Game Sporz Wireless Boxing", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2004, sporzbxa,  sporzbx,  0,  nes_vt_pal_4mb_sporzbxa,               sporzpp, nes_vt_waixing_alt_sporzpp_state, empty_init, "Macro Winners (Play Vision license)", "Wireless Boxing (PAL, Play Vision)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
// has some longer than expected delays when sounds should play on the Tennis part, but NES hacks are all functional, US version is sold in DreamGear branded box.
CONS( 2004, sporztn,   0,  0,  nes_vt_pal_4mb,        sporzpp, nes_vt_wldsoctv_state, empty_init, "Macro Winners (Play Vision license)", "Wireless Tennis (PAL, Play Vision)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
// missing PCM audio, but regular APU SFX work
CONS( 200?, wldsoctv,  0,  0,  nes_vt_pal_4mb,        nes_vt,  nes_vt_wldsoctv_state, empty_init, "Taikee", "World Soccer TV Game 10-in-1 (PAL)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// should be VT03 based
// for testing 'Shark', 'Octopus', 'Harbor', and 'Earth Fighter' use the extended colour modes, other games just seem to use standard NES modes
CONS( 200?, mc_dgear,  0,  0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "dreamGEAR", "dreamGEAR 75-in-1", MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, sudo6in1,  0,  0,  nes_vt_pal_1mb,    nes_vt, nes_vt_state, empty_init, "Nice Code", "6-in-1 Sudoku Plug & Play", MACHINE_IMPERFECT_GRAPHICS ) // no manufacturer info on packaging, games seem to be from Nice Code, although this isn't certain


// small black unit, dpad on left, 4 buttons (A,B,X,Y) on right, Start/Reset/Select in middle, unit text "Sudoku Plug & Play TV Game"
CONS( 200?, sudopptv,  0, 0,  nes_vt_waixing_512kb_rasterhack,        nes_vt, nes_vt_waixing_state, empty_init, "Smart Planet", "Sudoku Plug & Play TV Game '6 Intelligent Games'", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

CONS( 200?, megapad,   0, 0,  nes_vt_waixing_2mb,        nes_vt, nes_vt_waixing_state, empty_init, "Waixing", "Megapad 31-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Happy Biqi has broken sprites, investigate before promoting

// 060303 date code on PCB
CONS( 2006, ablmini,   0, 0,  nes_vt_waixing_alt_pal_8mb, nes_vt, nes_vt_waixing_alt_state, empty_init, "Advance Bright Ltd", "Double Players Mini Joystick 80-in-1 (MJ8500, ABL TV Game)", MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, solargm,   0,  0, nes_vt_waixing_alt_pal_8mb, nes_vt, nes_vt_waixing_alt_state, empty_init, "<unknown>", "Solar Games 80-in-1 (PAL)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Solar Games logo is also found in the SunPlus based Millennium Arcade units

// silver 'N64' type controller design
CONS( 200?, zudugo,    0, 0,  nes_vt_waixing_alt_4mb,     nes_vt, nes_vt_waixing_alt_state, empty_init, "Macro Winners / Waixing", "Zudu-go / 2udu-go", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // the styling on the box looks like a '2' in places, a 'Z' in others.

 // needs PCM samples, Y button is not mapped (not used by any of the games? some sources indicate it's just a hardware autofire button)
CONS( 200?, timetp36,  0, 0,  nes_vt_pal_4mb, timetp36, nes_vt_timetp36_state,        empty_init, "TimeTop", "Super Game 36-in-1 (TimeTop SuperGame) (PAL)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

CONS( 200?, timetp7,   0, 0,  nes_vt_pal_2mb, timetp36, nes_vt_timetp36_state,        empty_init, "TimeTop", "Super Game 7-in-1 (TimeTop SuperGame) (PAL)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

CONS( 2004, majkon,    0, 0,  nes_vt_1mb_majkon, nes_vt, nes_vt_state, empty_init, "Majesco (licensed from Konami) / JungleTac", "Konami Collector's Series Arcade Advanced", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // raster timing for Frogger needs a hack, Rush'n Attack also has raster timing issues on the status bar split

CONS( 200?, majgnc,    0, 0,  nes_vt_1mb, majgnc, nes_vt_state,  empty_init, "Majesco / JungleTac", "Golden Nugget Casino", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

CONS( 2004, vt25in1,   0, 0,  nes_vt_1mb, nes_vt, nes_vt_state,  empty_init, "<unknown>", "unknown VT02 based 25-in-1 handheld", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// might be a later VT type, 'pump' control is mapped on extra IO address that I don't think is present on 02/03
CONS( 200?, pumpactv,  0, 0,  nes_vt_1mb, nes_vt, nes_vt_state,  empty_init, "Giggle", "TV Pump Active", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// CPU die is marked 'VH2009' There's also a 62256 RAM chip on the PCB, some scrambled opcodes
CONS( 2004, vsmaxx17,  0,  0,  nes_vt_vh2009_2mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "Senario / JungleTac",   "Vs Maxx 17-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // from a Green unit, '17 Classic & Racing Game'
CONS( 200?, vsmax25v,  0,  0,  nes_vt_vh2009_4mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "Senario / JungleTac",   "Vs Maxx 25-in-1 (VT03 hardware)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2004, polmega,   0,  0,  nes_vt_vh2009_4mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "Polaroid / JungleTac",  "TV MegaMax active power game system 30-in-1 (MegaMax GPD001SDG)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, dgun851,   0,  0,  nes_vt_vh2009_4mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "dreamGEAR / JungleTac", "Plug 'N' Play 30-in-1 (DGUN-851)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, dgun853,   0,  0,  nes_vt_vh2009_8mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "dreamGEAR / JungleTac", "Plug 'N' Play 50-in-1 (DGUN-853)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, silv35,    0,  0,  nes_vt_vh2009_4mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "SilverLit / JungleTac", "35 in 1 Super Twins", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2004, vsmaxxvd,  0,  0,  nes_vt_vh2009_8mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "Senario / JungleTac",   "Vs Maxx Video Extreme 50-in-1 (with Speed Racer and Snood)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, vsmaxx77,  0,  0,  nes_vt_vh2009_8mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "Senario / JungleTac",   "Vs Maxx Wireless 77-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, joysti30,  0,  0,  nes_vt_vh2009_4mb,        nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "WinFun / JungleTac",    "Joystick 30", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // doesn't show WinFun onscreen, but packaging does

// has no audio, is there extra hardware, or is it just using unemulated VT features?
CONS( 2005, lxnoddy,   0,  0,  nes_vt_vh2009_pal_2mb,        lxnoddy, nes_vt_swap_op_d5_d6_state, empty_init, "Lexibook",   "Noddy's TV Console", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )


// mostly bootleg NES games, but also has Frogger, Scramble and Asteroids in it
CONS( 200?, gamezn2,   0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, init_gamezn2, "<unknown>", "Game Zone II 128-in-1", MACHINE_IMPERFECT_GRAPHICS ) // was this PAL? (lots of raster splits are broken at the moment either way)

// die is marked as VH2009, as above, but no scrambled opcodes here
CONS( 201?, techni4,   0,  0,  nes_vt_pal_2mb,           nes_vt, nes_vt_state,               empty_init, "Technigame", "Technigame Super 4-in-1 Sports (PAL)", MACHINE_IMPERFECT_GRAPHICS )

CONS( 2005, senwld,   0,          0,  nes_vt_senwld_512kb,    nes_vt, nes_vt_swap_op_d5_d6_state, empty_init, "Senario", "Win, Lose or Draw (Senario)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // needs RAM in banked space, Alpha display emulating, Touchpad emulating etc.

 // seems to use PCM for all sound, some garbage at bottom of screen, needs correct inputs (seems to respond to start, and any direction input for 'hit' - check if they're power related)
CONS( 200?, protpp,   0,  0,  nes_vt_vh2009_1mb,      nes_vt, nes_vt_swap_op_d5_d6_state,        init_protpp, "Protocol", "Virtual Ping Pong (Protocol)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

// this has 'Shark' and 'Octopus' etc. like mc_dgear but uses scrambled bank registers
// This was also often found in cart form with SunPlus / Famiclone hybrid systems to boost the game count, eg. the WiWi (ROM verified to match)
CONS( 200?, mc_sp69,   0,  0,  nes_vt_4mb_sp69,    nes_vt, nes_vt_sp69_state, empty_init, "<unknown>", "Sports Game 69 in 1", MACHINE_IMPERFECT_GRAPHICS  | MACHINE_IMPERFECT_SOUND)

// this game was also sold by dreamGEAR and several others companies, each time with a different name and different case, although the dumped version was from ABL, and it hasn't been confirmed that the ROMs are identical for the other units
// Super Ping Pong appears on the title screen, but not the box / product art which simply has "Ping Pong Plug & Play TV Game" on front/back/bottom/manual, and "Table Tennis Plug & Play TV Game" on left/right sides.  Product code is PP1100
// PCB has PP1100-MB 061110 on it, possible date YYMMDD code? (pinball is 050329, guitar fever is 070516, air blaster 050423, kickboxing 061011 etc.)
CONS( 2006, ablping,   0,        0,  nes_vt_2mb_ablping, nes_vt, nes_vt_ablping_state, empty_init, "Advance Bright Ltd", "Ping Pong / Table Tennis / Super Ping Pong (PP1100, ABL TV Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

CONS( 200?, vfootbal,  0,        0,  nes_vt_2mb_vfootbal,nes_vt, nes_vt_ablping_state, empty_init, "<unknown>", "Virtual Football (with 3 bonus games)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// Hummer systems, scrambled bank register
CONS( 200?, mc_sam60,  0,  0,  nes_vt_hummer_2mb,    nes_vt, nes_vt_hum_state, empty_init, "Hummer Technology Co., Ltd.", "Samuri (60 in 1)", MACHINE_IMPERFECT_GRAPHICS  | MACHINE_IMPERFECT_SOUND )
CONS( 200?, zdog,      0,  0,  nes_vt_hummer_4mb,    nes_vt, nes_vt_hum_state, empty_init, "Hummer Technology Co., Ltd.", "ZDog (44 in 1)", MACHINE_IMPERFECT_GRAPHICS  | MACHINE_IMPERFECT_SOUND )

// very plain menus
CONS( 200?, pjoyn50,    0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "PowerJoy Navigator 50 in 1", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, pjoys30,    0,        0,  nes_vt_pjoy_4mb,    nes_vt, nes_vt_pjoy_state, empty_init, "<unknown>", "PowerJoy Supermax 30 in 1", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, pjoys60,    0,        0,  nes_vt_pjoy_4mb,    nes_vt, nes_vt_pjoy_state, empty_init, "<unknown>", "PowerJoy Supermax 60 in 1", MACHINE_IMPERFECT_GRAPHICS )
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

// Unlike other Senario products this one is mostly just NES games, it also appears to be one of the final Senario products.
// Some games aren't what they claim to be, Warpman is Soccer for example.
// Senario had another 101 unit with different games (not JungleTac or NES bootlegs) it might be the same as the dreamGEAR 101 below
CONS( 2009, sen101,   0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "Senario", "101 Games in 1 (Senario, NES/Famicom bootlegs)", MACHINE_IMPERFECT_GRAPHICS )

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

// Runs well, all games seem to work
CONS( 201?, mc_89in1,   0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "<unknown>", "89 in 1 Mini Game Console (060-92023011V1.0)", MACHINE_IMPERFECT_GRAPHICS )

// Works fine, uses ony VT02 features
CONS( 201?, mc_tv200,   0,        0,  nes_vt_8mb,    nes_vt, nes_vt_state, empty_init, "Thumbs Up", "200 in 1 Retro TV Game", MACHINE_IMPERFECT_GRAPHICS )

// TODO: add cart port and hook up nes_vt_cart.xml
CONS( 201?, 88in1joy,   0,        0,  nes_vt_4mb,    nes_vt, nes_vt_state, empty_init, "Play Vision", "Joystick88", MACHINE_IMPERFECT_GRAPHICS )


// Runs fine, non-sport 121 in 1 games perfect, but minor graphical issues in
// sport games, also no sound in menu or sport games due to missing PCM
// emulation
CONS( 200?, dgun2500,  0,  0,  nes_vt_16mb, nes_vt, nes_vt_state, empty_init, "dreamGEAR", "dreamGEAR Wireless Motion Control with 130 games (DGUN-2500)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)

// available in a number of colours, with various brands, but likely all the same.
// This was a red coloured pad, contains various unlicensed bootleg reskinned NES game eg Blob Buster is a hack of Dig Dug 2 and there are also hacks of Xevious, Donkey Kong Jr, Donkey Kong 3 and many others.
// Also available in handheld form where Supreme 200 is also shown on the main menu background
// unclear if this is VT03 or VT09, the boot logo needs either VT09 or PAL mode for the DMA to be correct, dump is from a PAL unit
CONS( 201?, ppgc200g,   0,         0,  nes_vt_pal_8mb, nes_vt, nes_vt_state, empty_init, "Fizz Creations", "Plug & Play Game Controller with 200 Games (Supreme 200)", MACHINE_IMPERFECT_GRAPHICS )

// unknown tech level, it's most likely a vt09 or vt369 but isn't using any of the extended features
CONS( 201?, dgun2869,  0,         0,  nes_vt_16mb,     nes_vt, nes_vt_state, empty_init, "dreamGEAR", "My Arcade Retro Micro Controller - 220 Built-In Video Games (DGUN-2869)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 201?, dgun2959,  0,         0,  nes_vt_pal_16mb, nes_vt, nes_vt_state, empty_init, "dreamGEAR", "My Arcade Plug And Play 220 Game Retro Controller (DGUN-2959)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// needs inputs - unit is a Mahjong controller.  This is said to be a hack(?) of a Famicom game (unless it was licensed by the original developer)
CONS( 200?, tvmjfc,    0,        0,  nes_vt_2mb,    nes_vt, nes_vt_state, empty_init, "bootleg?", "TV Mahjong Game (VTxx hardware)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
