// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Kawai SX-240 synthesizer.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"


namespace {

class kawai_sx240_state : public driver_device
{
public:
	kawai_sx240_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_master(*this, "master")
		, m_slave(*this, "slave")
	{
	}

	void sx240(machine_config &config);

private:
	void sh_outport_w(u8 data);
	void asn_w(u8 data);
	void g_w(u8 data);

	void master_prog(address_map &map) ATTR_COLD;
	void master_ext(address_map &map) ATTR_COLD;
	void slave_prog(address_map &map) ATTR_COLD;
	void slave_ext(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_master;
	required_device<mcs51_cpu_device> m_slave;
};


void kawai_sx240_state::sh_outport_w(u8 data)
{
	// SAD1–SAD3 = SHA0–SHA2
	// SAD4, SAD5 -> NE555 for buzzer
}

void kawai_sx240_state::asn_w(u8 data)
{
}

void kawai_sx240_state::g_w(u8 data)
{
}

void kawai_sx240_state::master_prog(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).rom().region("mcode", 0);
}

void kawai_sx240_state::master_ext(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("ram0");
	map(0x0800, 0x0fff).ram().share("ram1");
	map(0x1000, 0x17ff).ram().share("ram2");
	map(0x1800, 0x1fff).ram().share("ram3");
	map(0x2100, 0x210f).nopr();
}

void kawai_sx240_state::slave_prog(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x1fff).rom().region("scode", 0);
}

void kawai_sx240_state::slave_ext(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x800).ram();
	map(0x1000, 0x1003).mirror(0xfc0).rw("pit0", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1004, 0x1007).mirror(0xfc0).rw("pit1", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1008, 0x100b).mirror(0xfc0).rw("pit2", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x100c, 0x100f).mirror(0xfc0).rw("pit3", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1010, 0x1013).mirror(0xfc0).rw("pit4", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1014, 0x1017).mirror(0xfc0).rw("pit5", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1018, 0x101b).mirror(0xfc0).rw("pit6", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x101c, 0x101f).mirror(0xfc0).rw("pit7", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1020, 0x1023).mirror(0xfd8).rw("pit8", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1024, 0x1027).mirror(0xfd8).rw("pit9", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x2000, 0x202f).mirror(0xfc0).nopw();
	map(0x2030, 0x2030).mirror(0xfc7).w(FUNC(kawai_sx240_state::sh_outport_w));
	map(0x2038, 0x2038).mirror(0xfc7).w(FUNC(kawai_sx240_state::asn_w));
	map(0x3000, 0x3000).mirror(0xfff).w(FUNC(kawai_sx240_state::g_w));
}


static INPUT_PORTS_START(sx240)
INPUT_PORTS_END

void kawai_sx240_state::sx240(machine_config &config)
{
	I8031(config, m_master, 12_MHz_XTAL);
	m_master->set_addrmap(AS_PROGRAM, &kawai_sx240_state::master_prog);
	m_master->set_addrmap(AS_IO, &kawai_sx240_state::master_ext);

	NVRAM(config, "ram0", nvram_device::DEFAULT_ALL_0); // HM6116ALSP-20 (I9) + battery
	NVRAM(config, "ram1", nvram_device::DEFAULT_ALL_0); // HM6116ALSP-20 (I10) + battery
	NVRAM(config, "ram2", nvram_device::DEFAULT_ALL_0); // HM6116ALSP-20 (I11) + battery
	NVRAM(config, "ram3", nvram_device::DEFAULT_ALL_0); // HM6116ALSP-20 (I12) + battery

	I8031(config, m_slave, 12_MHz_XTAL);
	m_slave->set_addrmap(AS_PROGRAM, &kawai_sx240_state::slave_prog);
	m_slave->set_addrmap(AS_IO, &kawai_sx240_state::slave_ext);

	PIT8253(config, "pit0"); // MSM82C53-5 (I25)
	PIT8253(config, "pit1"); // MSM82C53-5 (I26)
	PIT8253(config, "pit2"); // MSM82C53-5 (I27)
	PIT8253(config, "pit3"); // MSM82C53-5 (I28)
	PIT8253(config, "pit4"); // MSM82C53-5 (I29)
	PIT8253(config, "pit5"); // MSM82C53-5 (I30)
	PIT8253(config, "pit6"); // MSM82C53-5 (I31)
	PIT8253(config, "pit7"); // MSM82C53-5 (I32)
	PIT8253(config, "pit8"); // MSM82C53-5 (I33)
	PIT8253(config, "pit9"); // MSM82C53-5 (I34)
}

ROM_START(sx240)
	ROM_REGION(0x4000, "mcode", 0) // 27128-25
	ROM_SYSTEM_BIOS(0, "firmware", "Firmware (unknown)")
	ROMX_LOAD("kawai sx-240 rom firmware binary.bin", 0x0000, 0x4000, CRC(b057c3ee) SHA1(3d3f98ac2c5ccca70690e4fb3482d704a67a6f65), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "tauntek", "Tauntek CFW")
	ROMX_LOAD("sx240mst.i8", 0x0000, 0x4000, CRC(426aba62) SHA1(d19c14a9ce6c06f021fa2641f072a82332023b1e), ROM_BIOS(1))

	ROM_REGION(0x2000, "scode", 0) // 2764-25
	// From Tauntek package: "New Slave EPROM is only required for saving patches or sequences via sysex"
	ROM_LOAD("sx240slv.i38", 0x0000, 0x2000, CRC(c6bc6439) SHA1(4d51d8c5a198ff139956cb0b07225448357d17ef))
ROM_END

} // anonymous namespace


SYST(1984, sx240, 0, 0, sx240, sx240, kawai_sx240_state, empty_init, "Kawai Musical Instrument Manufacturing", "SX-240 8-Voice Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
