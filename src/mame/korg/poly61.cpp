// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg Poly-61

****************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"


namespace {

class poly61_state : public driver_device
{
public:
	poly61_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_progmcu(*this, "progmcu")
		, m_keymcu(*this, "keymcu")
		, m_ppi(*this, "ppi%u", 0U)
		, m_pit(*this, "pit%u", 0U)
	{
	}

	virtual void machine_start() override ATTR_COLD;

	required_device<mcs48_cpu_device> m_progmcu;
	required_device<mcs48_cpu_device> m_keymcu;
	std::unique_ptr<u8[]> m_nvram_ptr;

	void poly61(machine_config &config);

private:
	u8 ext_r(offs_t offset);
	void ext_w(offs_t offset, u8 data);

	void prog_ext_map(address_map &map) ATTR_COLD;

	required_device_array<i8255_device, 3> m_ppi;
	required_device_array<pit8253_device, 4> m_pit;
};

void poly61_state::machine_start()
{
	m_nvram_ptr = make_unique_clear<u8[]>(0x400);
	subdevice<nvram_device>("nvram")->set_base(&m_nvram_ptr[0], 0x400);

	save_pointer(NAME(m_nvram_ptr), 0x400);
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

void poly61_state::prog_ext_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(poly61_state::ext_r), FUNC(poly61_state::ext_w));
}

static INPUT_PORTS_START(poly61)
INPUT_PORTS_END

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


SYST(1982, poly61,  0, 0, poly61,  poly61, poly61_state,  empty_init, "Korg", "Poly-61 Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
