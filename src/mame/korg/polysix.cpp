// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg Polysix (PS-6)

****************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/nvram.h"


namespace {

class polysix_state : public driver_device
{
public:
	polysix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_progmcu(*this, "progmcu")
		, m_keymcu(*this, "keymcu")
	{
	}

	virtual void machine_start() override ATTR_COLD;

	required_device<mcs48_cpu_device> m_progmcu;
	required_device<mcs48_cpu_device> m_keymcu;
	std::unique_ptr<u8[]> m_nvram_ptr;
	void polysix(machine_config &config);

private:
	u8 ext_r(offs_t offset);
	void ext_w(offs_t offset, u8 data);

	void prog_ext_map(address_map &map) ATTR_COLD;
};


void polysix_state::machine_start()
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

void polysix_state::prog_ext_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(polysix_state::ext_r), FUNC(polysix_state::ext_w));
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

ROM_START(polysix)
	ROM_REGION(0x400, "progmcu", 0)
	ROM_LOAD("d8048c-345.bin", 0x000, 0x400, CRC(130fb945) SHA1(feaf59d7694de9c3f8009d883a250039f219d046))

	ROM_REGION(0x800, "keymcu", 0)
	ROM_LOAD("d8049c-217.bin", 0x000, 0x800, CRC(246d7767) SHA1(5b608c750e7fe7832070a53a74df416fd132ecb7))
ROM_END

} // anonymous namespace


SYST(1980, polysix, 0, 0, polysix, polysix, polysix_state, empty_init, "Korg", "Polysix Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
