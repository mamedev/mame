// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Prize Zone (c) 199? Lazer-Tron

TODO:
- identify proper motherboard/BIOS;
- hookup vibra16 ISA card in place of sblaster_16;
- Printer error B0 keeps popping in attract/game select (disable in service mode as workaround);
- Trackball in place of PS/2 mouse (doesn't seem to use serial);
- CD-ROM reading test is too fast (739% of what is reported as 8X);
- Tri-Scraper / Super Solitaire / Super 11's: timer doesn't count down (goes only up),
  it does with mouse click, likely bug for wrong input device?
- Outputs, writes to $301-$305 aren't (comms with a CPU?)

===================================================================================================

Prize Zone Gold Version 2.01 its a CDRDAO dumped cd
The redemption arcade game uses an old Pentium I - 200Mhz 233MMX etc SB16 vibra16,
a S3 virge vga card and a custom ISA IO card.
It has games like gem run, chip away, squirm (a centipede rip off) scud attack (a middle command
rip off) these are games that atari got mad about and were later removed

This one doesnt have a dongle
but the sound card, s3 virge video card
and isa custom board has to be there
or the disc wont load and gives an error

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "bus/pci/pci_slot.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/fdc37c93x.h"
#include "machine/i82371sb.h"
#include "machine/i82439hx.h"
#include "machine/pci.h"

class isa16_przone_jamma_if : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_przone_jamma_if(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	std::unique_ptr<uint8_t[]> m_nvram_data;
	//required_ioport_array<5> m_iocard;
	required_ioport m_iocard;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint16_t iocard_r();
	uint8_t nvram_r(offs_t offset);
	void nvram_w(offs_t offset, uint8_t data);
};

DEFINE_DEVICE_TYPE(ISA16_PRZONE_JAMMA_IF, isa16_przone_jamma_if, "przone_jamma_if", "ISA16 Prize Zone custom JAMMA I/F")

isa16_przone_jamma_if::isa16_przone_jamma_if(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_PRZONE_JAMMA_IF, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_iocard(*this, "IOCARD")
{
}

void isa16_przone_jamma_if::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1); // unknown NVRAM type
}

void isa16_przone_jamma_if::device_start()
{
	set_isa_device();

	// 0x4000 enough to pass initial string check at $d0000-7,
	// 0x8000 confirmed later thru NVRAM test in service mode
	m_nvram_data = std::make_unique<uint8_t[]>(0x8000);
	subdevice<nvram_device>("nvram")->set_base(m_nvram_data.get(), 0x8000);
}

void isa16_przone_jamma_if::device_reset()
{
}

void isa16_przone_jamma_if::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_isa->install_memory(0x000d0000, 0x000d7fff, *this, &isa16_przone_jamma_if::mem_map);
	}

	if (space_id == AS_IO)
	{
		m_isa->install_device(0x0300, 0x030f, *this, &isa16_przone_jamma_if::io_map);
	}
}

uint8_t isa16_przone_jamma_if::nvram_r(offs_t offset)
{
	return m_nvram_data[offset];
}

void isa16_przone_jamma_if::nvram_w(offs_t offset, uint8_t data)
{
	m_nvram_data[offset] = data;
}

void isa16_przone_jamma_if::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(isa16_przone_jamma_if::nvram_r), FUNC(isa16_przone_jamma_if::nvram_w));
}

void isa16_przone_jamma_if::io_map(address_map &map)
{
	map(0x04, 0x05).lr16(NAME([this] () { return m_iocard->read(); }));
}

static INPUT_PORTS_START( przone_jamma )
	PORT_START("IOCARD")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN6 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "1-1" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

ioport_constructor isa16_przone_jamma_if::device_input_ports() const
{
	return INPUT_PORTS_NAME(przone_jamma);
}

namespace {

class przone_state : public driver_device
{
public:
	przone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void przone(machine_config &config);

private:
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	static void smc_superio_config(device_t *device);
};

void przone_state::main_map(address_map &map)
{
	map.unmap_value_high();
}

void przone_state::main_io(address_map &map)
{
	map.unmap_value_high();
}

static INPUT_PORTS_START( przone )
INPUT_PORTS_END

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("fdc37c93x", FDC37C93X);
}

void przone_isa16_cards(device_slot_interface &device)
{
	device.option_add("przone_jamma_if", ISA16_PRZONE_JAMMA_IF);
}

static void isa_com(device_slot_interface &device)
{
	device.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
	device.option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
	device.option_add("wheel_mouse", WHEEL_HLE_SERIAL_MOUSE);
	device.option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
	device.option_add("rotatable_mouse", ROTATABLE_HLE_SERIAL_MOUSE);
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("null_modem", NULL_MODEM);
	device.option_add("sun_kbd", SUN_KBD_ADAPTOR);
}

void przone_state::smc_superio_config(device_t *device)
{
	fdc37c93x_device &fdc = *downcast<fdc37c93x_device *>(device);
	fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	fdc.gp25_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
	fdc.irq1().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq8n_w));
	fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

void przone_state::przone(machine_config &config)
{
	// unknown CPU, lowered to PCI clock for ViRGE being really the bottleneck here.
	pentium_device &maincpu(PENTIUM(config, "maincpu", 33'333'333));
	maincpu.set_addrmap(AS_PROGRAM, &przone_state::main_map);
	maincpu.set_addrmap(AS_IO, &przone_state::main_io);
	maincpu.set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.smiact().set("pci:00.0", FUNC(i82439hx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	// TODO: confirm size
	I82439HX(config, "pci:00.0", 0, "maincpu", 256*1024*1024);

	i82371sb_isa_device &isa(I82371SB_ISA(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371sb_ide_device &ide(I82371SB_IDE(config, "pci:07.1", 0, "maincpu"));
	ide.irq_pri().set("pci:07.0", FUNC(i82371sb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371sb_isa_device::pc_mirq0_w));
	ide.subdevice<bus_master_ide_controller_device>("ide1")->slot(0).set_default_option("cdrom");
//  ide.subdevice<bus_master_ide_controller_device>("ide1")->slot(0).set_option_machine_config("cdrom", cdrom_config);
	ide.subdevice<bus_master_ide_controller_device>("ide2")->slot(0).set_default_option(nullptr);

	PCI_SLOT(config, "pci:1", pci_cards, 15, 0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards, 16, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 17, 2, 3, 0, 1, nullptr);
	PCI_SLOT(config, "pci:4", pci_cards, 18, 3, 0, 1, 2, "virge");

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "fdc37c93x", true).set_option_machine_config("fdc37c93x", smc_superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", przone_isa16_cards, "przone_jamma_if", true);
	// TODO: one slot for vibra16
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, "sblaster_16", false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	rs232_port_device& serport0(RS232_PORT(config, "serport0", isa_com, nullptr));
	serport0.rxd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::rxd1_w));
	serport0.dcd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndcd1_w));
	serport0.dsr_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndsr1_w));
	serport0.ri_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::nri1_w));
	serport0.cts_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::rxd2_w));
	serport1.dcd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndcd2_w));
	serport1.dsr_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndsr2_w));
	serport1.ri_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::nri2_w));
	serport1.cts_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ncts2_w));
}


ROM_START( przonegd )
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)
	// borrowed from pcipc -bios 2 (El Torito), technically a BAD_DUMP
	ROM_LOAD("m7s04.rom",   0, 0x40000, CRC(3689f5a9) SHA1(8daacdb0dc6783d2161680564ffe83ac2515f7ef))

	DISK_REGION( "pci:07.1:ide1:0:cdrom" )
	DISK_IMAGE_READONLY( "prizezonegold_201", 0, SHA1(54eec0d7b8629f8e8a4c9b99184ece0be0e8eb06) )
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/


GAME( 1997, przonegd,  0,        przone, przone, przone_state, empty_init, ROT0, "Lazer-Tron", "Prize Zone Gold (USA, v2.01)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // Wed Dec 31 11:13:17 1997
