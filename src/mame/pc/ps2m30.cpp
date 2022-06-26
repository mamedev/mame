// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic, Carl

/* The ISA PS/2s, Model 25 and Model 30. These are different enough to the MCA ones they need a different driver. */

#include "emu.h"
#include "cpu/i86/i286.h"
#include "machine/at.h"
#include "machine/ibmps2.h"
#include "machine/ram.h"
#include "bus/isa/isa_cards.h"
#include "softlist_dev.h"

// According to http://nerdlypleasures.blogspot.com/2014/04/the-original-8-bit-ide-interface.html
// the IBM PS/2 Model 25-286 and Model 30-286 use a customised version of the XTA (8-bit IDE) harddisk interface

class ps2_m30_state : public driver_device
{
public:
	ps2_m30_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_ram(*this, RAM_TAG)
	{ }
	required_device<cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;

	void ps2_base(machine_config &config);

	void ps2m30286(machine_config &config);
	void ps2386(machine_config &config);
	void ps2386sx(machine_config &config);

	void at_softlists(machine_config &config);
	void ps2_16_io(address_map &map);
	void ps2_16_map(address_map &map);
protected:
	void machine_start() override;
};

void ps2_m30_state::at_softlists(machine_config &config)
{
	/* software lists */
	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
	SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");
}

void ps2_m30_state::ps2_16_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x09ffff).bankrw("bank10");
	map(0x0e0000, 0x0fffff).rom().region("bios", 0);
	map(0xfe0000, 0xffffff).rom().region("bios", 0);
}

void ps2_m30_state::ps2_16_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).m(m_mb, FUNC(at_mb_device::map));
}

void ps2_m30_state::machine_start()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);

	/* managed RAM */
	membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > 0xa0000)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - 0xa0000;
		space.install_ram(0x100000,  ram_limit - 1, m_ram->pointer() + 0xa0000);
	}
}

void ps2_m30_state::ps2m30286(machine_config &config)
{
	/* basic machine hardware */
	i80286_cpu_device &maincpu(I80286(config, m_maincpu, 10000000));
	maincpu.set_addrmap(AS_PROGRAM, &ps2_m30_state::ps2_16_map);
	maincpu.set_addrmap(AS_IO, &ps2_m30_state::ps2_16_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.shutdown_callback().set("mb", FUNC(at_mb_device::shutdown));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1664K").set_extra_options("2M,4M,8M,15M");

    // These aren't MCA.
	AT_MB(config, m_mb, 20000000);

	config.set_maximum_quantum(attotime::from_hz(60));
	at_softlists(config);
}

ROM_START( ibm8530_286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// saved from running machine
	ROM_LOAD16_BYTE("ps2m30.0", 0x00000, 0x10000, CRC(9965a634) SHA1(c237b1760f8a4561ec47dc70fe2e9df664e56596))
	ROM_LOAD16_BYTE("ps2m30.1", 0x00001, 0x10000, CRC(1448d3cb) SHA1(13fa26d895ce084278cd5ab1208fc16c80115ebe))
ROM_END

/*
8530-H31 (Model 30/286)
======================
  P/N          Date
33F5381A EC C01446 1990
*/
ROM_START( ibm8530_h31 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "33f5381a.bin", 0x00000, 0x20000, CRC(ff57057d) SHA1(d7f1777077a8df43c3c14d175b9709bd3969c4b1))
ROM_END

COMP( 1990, ibm8530_h31, 0,           ibm5170, ps2m30286, 0, ps2_m30_state, empty_init, "International Business Machines", "IBM PS/2 8530-H31 (Model 30/286)", MACHINE_IS_SKELETON )
COMP( 1988, ibm8530_286, ibm8530_h31, 0,       ps2m30286, 0, ps2_m30_state, empty_init, "International Business Machines", "IBM PS/2 Model 30-286", MACHINE_IS_SKELETON )
