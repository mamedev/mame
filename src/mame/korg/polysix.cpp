// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg Polysix (PS-6) & Poly-61 synthesizers.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"


namespace {

class ps6_base_state : public driver_device
{
protected:
	ps6_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_progmcu(*this, "progmcu")
		, m_keymcu(*this, "keymcu")
	{
	}

	virtual void machine_start() override ATTR_COLD;

	required_device<mcs48_cpu_device> m_progmcu;
	required_device<mcs48_cpu_device> m_keymcu;
	std::unique_ptr<u8[]> m_nvram_ptr;
};

class polysix_state : public ps6_base_state
{
public:
	polysix_state(const machine_config &mconfig, device_type type, const char *tag)
		: ps6_base_state(mconfig, type, tag)
	{
	}

	void polysix(machine_config &config);

private:
	u8 ext_r(offs_t offset);
	void ext_w(offs_t offset, u8 data);

	void prog_ext_map(address_map &map) ATTR_COLD;
};

class poly61_state : public ps6_base_state
{
public:
	poly61_state(const machine_config &mconfig, device_type type, const char *tag)
		: ps6_base_state(mconfig, type, tag)
		, m_ppi(*this, "ppi%u", 0U)
		, m_pit(*this, "pit%u", 0U)
	{
	}

	void poly61(machine_config &config);

private:
	u8 ext_r(offs_t offset);
	void ext_w(offs_t offset, u8 data);

	void prog_ext_map(address_map &map) ATTR_COLD;

	required_device_array<i8255_device, 3> m_ppi;
	required_device_array<pit8253_device, 4> m_pit;
};

void ps6_base_state::machine_start()
{
	m_nvram_ptr = make_unique_clear<u8[]>(0x400);
	subdevice<nvram_device>("nvram")->set_base(&m_nvram_ptr[0], 0x400);

	save_pointer(NAME(m_nvram_ptr), 0x400);
}

u8 polysix_state::ext_r(offs_t offset)
{
	if (BIT(m_progmcu->p1_r(), 7))
	{
		u8 p2 = m_progmcu->p2_r();
		return m_nvram_ptr[(p2 & 1) | (offset << 1) | (p2 & 2) << 8] | 0xf0;
	}

	return 0xff;
}

void polysix_state::ext_w(offs_t offset, u8 data)
{
	if (BIT(m_progmcu->p1_r(), 7))
	{
		u8 p2 = m_progmcu->p2_r();
		m_nvram_ptr[(p2 & 1) | (offset << 1) | (p2 & 2) << 8] = data & 0x0f;
	}
}

u8 poly61_state::ext_r(offs_t offset)
{
	if (!BIT(m_progmcu->p2_r(), 5))
	{
		u8 p1 = m_progmcu->p1_r();
		return m_nvram_ptr[(p1 & 1) | (offset << 1) | (p1 & 2) << 8] | 0xf0;
	}

	return 0xff;
}

void poly61_state::ext_w(offs_t offset, u8 data)
{
	u8 p2 = m_progmcu->p2_r();

	if (!BIT(p2, 5))
	{
		u8 p1 = m_progmcu->p1_r();
		m_nvram_ptr[(p1 & 1) | (offset << 1) | (p1 & 2) << 8] = data & 0x0f;
	}

	if (!BIT(p2, 6))
	{
		if (!BIT(offset, 5) && (offset & 0x0c) != 0x0c)
			m_ppi[(offset & 0x0c) >> 2]->write(offset & 0x03, data);
		if (!BIT(offset, 6))
			m_pit[(offset & 0x0c) >> 2]->write(offset & 0x03, data);
	}
}

void polysix_state::prog_ext_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(polysix_state::ext_r), FUNC(polysix_state::ext_w));
}

void poly61_state::prog_ext_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(poly61_state::ext_r), FUNC(poly61_state::ext_w));
}

static INPUT_PORTS_START(polysix)
INPUT_PORTS_END

void polysix_state::polysix(machine_config &config)
{
	I8048(config, m_progmcu, 6_MHz_XTAL);
	m_progmcu->set_addrmap(AS_IO, &polysix_state::prog_ext_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5514APL-3 + battery

	I8049(config, m_keymcu, 6_MHz_XTAL);
}

void poly61_state::poly61(machine_config &config)
{
	I8049(config, m_progmcu, 6_MHz_XTAL);
	m_progmcu->set_addrmap(AS_IO, &poly61_state::prog_ext_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // M58981P-45 + battery

	I8255(config, m_ppi[0]);
	I8255(config, m_ppi[1]);
	I8255(config, m_ppi[2]);

	PIT8253(config, m_pit[0]);
	PIT8253(config, m_pit[1]);
	PIT8253(config, m_pit[2]);
	PIT8253(config, m_pit[3]);

	I8049(config, m_keymcu, 6_MHz_XTAL);
}

ROM_START(polysix)
	ROM_REGION(0x400, "progmcu", 0)
	ROM_LOAD("d8048c-345.bin", 0x000, 0x400, CRC(130fb945) SHA1(feaf59d7694de9c3f8009d883a250039f219d046))

	ROM_REGION(0x800, "keymcu", 0)
	ROM_LOAD("d8049c-217.bin", 0x000, 0x800, CRC(246d7767) SHA1(5b608c750e7fe7832070a53a74df416fd132ecb7))
ROM_END

ROM_START(poly61)
	ROM_REGION(0x800, "progmcu", 0)
	ROM_LOAD("d8049c-337.bin", 0x000, 0x800, CRC(51edf723) SHA1(9001687b25a841b7a5f78bbae03745f3e995fa83))

	ROM_REGION(0x800, "keymcu", 0)
	ROM_DEFAULT_BIOS("new")
	ROM_SYSTEM_BIOS(0, "old", "Older MCU")
	ROMX_LOAD("d8049c-217.bin", 0x000, 0x800, CRC(246d7767) SHA1(5b608c750e7fe7832070a53a74df416fd132ecb7), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "new", "Newer MCU")
	ROMX_LOAD("d8049c-384.bin", 0x000, 0x800, CRC(bbc421b5) SHA1(be683cdce5cbf867c5b8a52802288c1296f5fbd1), ROM_BIOS(1))
ROM_END

} // anonymous namespace


SYST(1980, polysix, 0, 0, polysix, polysix, polysix_state, empty_init, "Korg", "Polysix Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
SYST(1982, poly61,  0, 0, poly61,  polysix, poly61_state,  empty_init, "Korg", "Poly-61 Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
