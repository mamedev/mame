// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Sega TeraDrive

IBM PS/2 Model 30 + Sega Mega Drive

TODO:
- Throws "102 TIMER FAIL"
- Many unknown ports;
- MD side;

**************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i286.h"
#include "cpu/i386/i386.h"
#include "machine/at.h"
#include "machine/ram.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "softlist_dev.h"

namespace {

class teradrive_state : public driver_device
{
public:
	teradrive_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mb(*this, "mb")
		, m_ram(*this, RAM_TAG)
	{ }
	required_device<i80286_cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;

	void teradrive(machine_config &config);
	void at_softlists(machine_config &config);
	void teradrive_io(address_map &map) ATTR_COLD;
	void teradrive_map(address_map &map) ATTR_COLD;

protected:
	void machine_start() override ATTR_COLD;

private:
	u16 m_heartbeat = 0;
};

void teradrive_state::at_softlists(machine_config &config)
{
	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
//  SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");

//  TODO: MD portion
//  TODO: Teradrive SW list
}

void teradrive_state::teradrive_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x09ffff).bankrw("bank10");
	map(0x0e0000, 0x0fffff).rom().region("bios", 0);
	map(0xfe0000, 0xffffff).rom().region("bios", 0);
}

void teradrive_state::teradrive_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
	map(0xfc72, 0xfc73).lr16(
		NAME([this] () {
			u16 res = m_heartbeat & 0x8000;
			if (!machine().side_effects_disabled())
				m_heartbeat ^= 0x8000;
			// other bits read
			return 0x7fff | res;
		})
	);
}

void teradrive_state::machine_start()
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

void teradrive_state::teradrive(machine_config &config)
{
	/* basic machine hardware */
	I80286(config, m_maincpu, XTAL(10'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &teradrive_state::teradrive_map);
	m_maincpu->set_addrmap(AS_IO, &teradrive_state::teradrive_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->shutdown_callback().set("mb", FUNC(at_mb_device::shutdown));

	AT_MB(config, m_mb);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	config.set_maximum_quantum(attotime::from_hz(60));

	at_softlists(config);

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "vga", true);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, "fdc", false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, "ide", false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, "comat", false);

	pc_kbdc_device &kbd(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_84));
	kbd.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	kbd.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1664K").set_extra_options("640K,2688K");
}

ROM_START( teradrive )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "bios-27c010.bin", 0x00000, 0x20000, CRC(32642518) SHA1(6bb6d0325b8e4150c4258fd16f3a870b92e88f75))

	ROM_REGION16_LE(0x100000, "romdisk", 0)
	// contains bootable PC-DOS 3.x + a MENU.EXE
	ROM_LOAD( "tru-27c800.bin", 0x00000, 0x100000,  CRC(c2fe9c9e) SHA1(06ec0461dab425f41fb5c3892d9beaa8fa53bbf1))

	// MD 68k initial boot code, "TERA286 INITIALIZE" in header
	// shows Sega logo + TMSS "produced by" + 1990 copyright at bottom if loaded thru megadrij
	// + non-canonical accesses in $a***** range
	ROM_REGION16_BE(0x4000, "tmss", ROMREGION_ERASEFF)
	ROM_LOAD( "tera_tmss.bin", 0x0000,  0x1000, CRC(424a9d11) SHA1(1c470a9a8d0b211c5feea1c1c2376aa1f7934b16) )
ROM_END

} // anonymous namespace

COMP( 1991, teradrive, 0, 0,       teradrive, 0, teradrive_state, empty_init, "Sega / International Business Machines", "TeraDrive (Japan)", MACHINE_NOT_WORKING )

