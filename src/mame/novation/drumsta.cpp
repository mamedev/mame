// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Novation Drum Station drum machine.

****************************************************************************/

#include "emu.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/mn1880/mn1880.h"
#include "machine/eeprompar.h"

#include "multibyte.h"


namespace {

class drumsta_state : public driver_device
{
public:
	drumsta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp")
		, m_samples(*this, "samples")
		, m_dsppmem(*this, "dsppmem")
		, m_control_latch(0)
		, m_idma_latch(0)
	{
	}

	void drumsta(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void control_latch_w(u8 data);
	void idma_lsb_w(u8 data);
	void idma_msb_w(u8 data);

	void drumsta_prog(address_map &map) ATTR_COLD;
	void drumsta_data(address_map &map) ATTR_COLD;
	void adsp2181_prog(address_map &map) ATTR_COLD;
	void adsp2181_data(address_map &map) ATTR_COLD;

	required_device<mn1880_device> m_maincpu;
	required_device<adsp2181_device> m_dsp;
	required_region_ptr<u8> m_samples;
	required_shared_ptr<u32> m_dsppmem;

	u8 m_control_latch;
	u8 m_idma_latch;
};


void drumsta_state::control_latch_w(u8 data)
{
	m_control_latch = data;
}

void drumsta_state::idma_msb_w(u8 data)
{
	m_idma_latch = data;
}

void drumsta_state::idma_lsb_w(u8 data)
{
	switch (m_control_latch & 0xc0)
	{
	case 0x00:
		m_dsp->idma_data_w(u16(m_idma_latch) << 8 | data);
		break;

	case 0xc0:
		m_dsp->idma_addr_w(u16(m_idma_latch) << 8 | data);
		break;
	}
}

void drumsta_state::machine_start()
{
	save_item(NAME(m_control_latch));
	save_item(NAME(m_idma_latch));
}

void drumsta_state::machine_reset()
{
	// FIXME: BDMA bootstrap should be handled internally by ADSP-2181
	u32 addr = 0;
	for (unsigned i = 0; i < 0x20; i++, addr += 3)
		m_dsppmem[i] = get_u24be(&m_samples[addr]);
}

void drumsta_state::drumsta_prog(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
}

void drumsta_state::drumsta_data(address_map &map)
{
	map(0x0001, 0x0001).noprw();
	map(0x0003, 0x0003).noprw();
	map(0x000e, 0x000f).noprw();
	map(0x0060, 0x031f).ram();
	map(0x8000, 0x8000).w(FUNC(drumsta_state::idma_msb_w));
	map(0x8800, 0x8800).w(FUNC(drumsta_state::idma_lsb_w));
	map(0x9000, 0x9000).w(FUNC(drumsta_state::control_latch_w));
	map(0xb000, 0xb7ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
}

void drumsta_state::adsp2181_prog(address_map &map)
{
	map(0x0000, 0x3fff).ram().share(m_dsppmem);
}

void drumsta_state::adsp2181_data(address_map &map)
{
	map(0x0000, 0x3fdf).ram();
}


static INPUT_PORTS_START(drumsta)
INPUT_PORTS_END

void drumsta_state::drumsta(machine_config &config)
{
	MN1880(config, m_maincpu, 8000000); // type and clock unknown (custom silkscreen)
	m_maincpu->set_addrmap(AS_PROGRAM, &drumsta_state::drumsta_prog);
	m_maincpu->set_addrmap(AS_DATA, &drumsta_state::drumsta_data);

	ADSP2181(config, m_dsp, 16000000); // clock unknown
	m_dsp->set_addrmap(AS_PROGRAM, &drumsta_state::adsp2181_prog);
	m_dsp->set_addrmap(AS_DATA, &drumsta_state::adsp2181_data);

	EEPROM_2864(config, "eeprom"); // Atmel AT28C64
}

ROM_START(drumsta)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("v1.2.u15", 0x0000, 0x8000, CRC(ab9a8654) SHA1(1914254c714d35031a456c71b1f9f9191df9126d))

	ROM_REGION(0x80000, "samples", 0)
	ROM_LOAD("v1.2.u28", 0x00000, 0x80000, CRC(dbbc9cfe) SHA1(61474c0bc6cfff3efe95527c57e4891f886b02aa))
ROM_END

} // anonymous namespace


SYST(1995, drumsta, 0, 0, drumsta, drumsta, drumsta_state, empty_init, "Novation", "Drum Station", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
