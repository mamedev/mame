// license:BSD-3-Clause
// copyright-holders:R. Belmont, Nathan Woods
/***************************************************************************

    drivers/bebox.cpp
    Emulation by R. Belmont, based on a skeleton by Nathan Woods

    The BeBox is a dual-CPU PowerPC-based personal computer developed by
    Be Inc, a company founded by Jean-Louis Gassée after he left Apple.
    BeOS was considered to be the top candidate for Apple's next-generation
    operating system after the Copland project failed.

***************************************************************************/

#include "emu.h"
#include "bebox.h"

#include "bus/nscsi/devices.h"
#include "bus/pci/clgd543x_alpine.h"
#include "bus/pci/ncr53c810.h"
#include "machine/i82091aa.h"
#include "machine/pckeybrd.h"
#include "sound/ymopl.h"

#include "speaker.h"


namespace {

// separate from the driver so the joysticks follow the 8042's mouse in player order
class bebox_joyport_device : public device_t
{
public:
	bebox_joyport_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD { }
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

INPUT_PORTS_START( bebox_joyport )
	PORT_START("JOYBUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(4)

	PORT_START("JOY0")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_MINMAX(0, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_PLAYER(1)

	PORT_START("JOY1")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_MINMAX(0, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_PLAYER(1)

	PORT_START("JOY2")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_MINMAX(0, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_PLAYER(2)

	PORT_START("JOY3")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_MINMAX(0, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_PLAYER(2)

	PORT_START("JOY4")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_MINMAX(0, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_PLAYER(3)

	PORT_START("JOY5")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_MINMAX(0, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_PLAYER(3)

	PORT_START("JOY6")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_MINMAX(0, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_PLAYER(4)

	PORT_START("JOY7")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_MINMAX(0, 0xfff) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_PLAYER(4)
INPUT_PORTS_END

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(BEBOX_JOYPORT, bebox_joyport_device, bebox_joyport_device, "bebox_joyport", "BeBox joystick ports")

bebox_joyport_device::bebox_joyport_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, BEBOX_JOYPORT, tag, owner, clock)
{
}

ioport_constructor bebox_joyport_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(bebox_joyport);
}


void bebox_state::main_mem(address_map &map)
{
	map(0x7FFFF0F0, 0x7FFFF0F7).rw(FUNC(bebox_state::bebox_cpu0_imask_r), FUNC(bebox_state::bebox_cpu0_imask_w));
	map(0x7FFFF1F0, 0x7FFFF1F7).rw(FUNC(bebox_state::bebox_cpu1_imask_r), FUNC(bebox_state::bebox_cpu1_imask_w));
	map(0x7FFFF2F0, 0x7FFFF2F7).r(FUNC(bebox_state::bebox_interrupt_sources_r));
	map(0x7FFFF3F0, 0x7FFFF3F7).rw(FUNC(bebox_state::bebox_crossproc_interrupts_r), FUNC(bebox_state::bebox_crossproc_interrupts_w));
	map(0x7FFFF4F0, 0x7FFFF4F7).w(FUNC(bebox_state::bebox_processor_resets_w));

	map(0xBFFFFFF0, 0xBFFFFFFF).r(FUNC(bebox_state::bebox_interrupt_ack_r));
	map(0xFFF00000, 0xFFF03FFF).bankr("bank2");
	map(0xFFF04000, 0xFFFFFFFF).rw(FUNC(bebox_state::bebox_flash_r), FUNC(bebox_state::bebox_flash_w));
}

// Board devices outside the 82378/82091 pair.
void bebox_state::io_map(address_map &map)
{
	map(0x0060, 0x0060).rw(m_kbdc, FUNC(kbdc8042_device::port60_r), FUNC(kbdc8042_device::port60_w));
	map(0x0064, 0x0064).rw(m_kbdc, FUNC(kbdc8042_device::port64_r), FUNC(kbdc8042_device::port64_w));
	map(0x0360, 0x0361).rw(FUNC(bebox_state::adc_control_r), FUNC(bebox_state::adc_control_w));
	map(0x0362, 0x0363).r(FUNC(bebox_state::adc_data_r));
	map(0x036e, 0x036f).rw(FUNC(bebox_state::joy_mux_r), FUNC(bebox_state::joy_mux_w));
	map(0x0380, 0x0387).rw(m_uart[0], FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x0388, 0x038f).rw(m_uart[1], FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x0397, 0x0397).r(FUNC(bebox_state::joy_buttons_r));
}

static void bebox_superio(device_slot_interface &device)
{
	device.option_add_internal("aip", I82091AA);
}

/*************************************
 *
 *  Keyboard
 *
 *************************************/

void bebox_state::bebox_keyboard_interrupt(int state)
{
	bebox_set_irq_bit(16, state);
	m_sio->pc_irq1_w(state);
}

void bebox_state::bebox_peripherals(machine_config &config)
{
	config.set_maximum_quantum(attotime::from_hz(60));

	NS16550(config, m_uart[0], 1.8432_MHz_XTAL).out_int_callback().set([this] (int state) { bebox_set_irq_bit(25, state); });
	NS16550(config, m_uart[1], 1.8432_MHz_XTAL).out_int_callback().set([this] (int state) { bebox_set_irq_bit(24, state); });

	speaker_device &speaker(SPEAKER(config, "mono"));
	speaker.front_center();

	ym3812_device &ym3812(YM3812(config, "ym3812", 3579545));
	ym3812.add_route(ALL_OUTPUTS, speaker, 1.0);

	FUJITSU_29F016A(config, "flash");
	MAX186(config, m_adc).set_input_callback(FUNC(bebox_state::adc_input));

	pci_root_device &pci(PCI_ROOT(config, "pci"));
	pci.set_pin_mapper(pci_pin_mapper(*this, FUNC(bebox_state::map_pci_pin)));
	pci.set_irq_handler(pci_irq_handler(*this, FUNC(bebox_state::pci_irq_w)));
	MPC105(config, m_mpc105, mpc105_host_device::MAP_TYPE_A, m_ppc[0], nullptr);
	m_mpc105->set_secondary_cpu_tag(m_ppc[1]);
	GD5434_PCI(config, "pci:01.0");

	ncr53c810_pci_device &ncr(NCR53C810_PCI(config, "pci:0c.0", 33'000'000));
	ncr.set_irq_map(21);
	nscsi_bus_device &scsi(NSCSI_BUS(config, "scsi"));
	scsi.set_external_device(7, ncr.scripts());
	for (u32 id = 0; id != 7; ++id)
		NSCSI_CONNECTOR(config, util::string_format("scsi:%u", id).c_str(), default_scsi_devices,
			id == 0 ? "harddisk" : id == 3 ? "cdrom" : nullptr);

	I82378ZB_SIO(config, m_sio, 0, m_ppc[0]);
	m_sio->intr().set([this] (int state) { bebox_set_irq_bit(5, state); });
	m_sio->rtcale().set(m_rtc, FUNC(mc146818_device::address_w));
	m_sio->rtccs_read().set(m_rtc, FUNC(mc146818_device::data_r));
	m_sio->rtccs_write().set(m_rtc, FUNC(mc146818_device::data_w));

	// These lines also feed the BeBox's interrupt-source register, before PIC masking.
	isa16_device &isa(*subdevice<isa16_device>("pci:0b.0:isabus"));
	isa.irq3_callback().append([this] (int state) { bebox_set_irq_bit(26, state); });
	isa.irq4_callback().append([this] (int state) { bebox_set_irq_bit(27, state); });
	isa.irq5_callback().append([this] (int state) { bebox_set_irq_bit(14, state); });
	isa.irq6_callback().append([this] (int state) { bebox_set_irq_bit(13, state); });
	isa.irq7_callback().append([this] (int state) { bebox_set_irq_bit(12, state); });
	isa.irq2_callback().append([this] (int state) { bebox_set_irq_bit(11, state); });
	isa.irq10_callback().append([this] (int state) { bebox_set_irq_bit(10, state); });
	isa.irq11_callback().append([this] (int state) { bebox_set_irq_bit(9, state); });
	isa.irq12_callback().append([this] (int state) { bebox_set_irq_bit(8, state); });
	isa.irq14_callback().append([this] (int state) { bebox_set_irq_bit(7, state); });
	isa.irq15_callback().append([this] (int state) { bebox_set_irq_bit(6, state); });

	ISA16_SLOT(config, "superio", 0, m_isabus, bebox_superio, "aip", true).set_option_machine_config("aip", [this] (device_t *device)
	{
		downcast<i82091aa_device &>(*device).set_config_address(0x398);
		downcast<i82091aa_device &>(*device).set_ide_tag(m_ide);
		device->subdevice<floppy_connector>("fdc:1")->set_default_option(nullptr);
	});

	IDE_CONTROLLER(config, m_ide);
	m_ide->options(ata_devices, "hdd", nullptr, false);
	m_ide->irq_handler().set(m_isabus, FUNC(isa16_device::irq14_w));

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_sio, FUNC(i82378zb_sio_device::pc_irq8n_w));
	m_rtc->irq().append([this] (int state) { bebox_set_irq_bit(15, state); });

	KBDC8042(config, m_kbdc);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_PS2);
	m_kbdc->set_interrupt_type(kbdc8042_device::KBDC8042_DOUBLE);
	m_kbdc->system_reset_callback().set_inputline(m_ppc[0], INPUT_LINE_RESET);
	m_kbdc->input_buffer_full_callback().set(FUNC(bebox_state::bebox_keyboard_interrupt));
	m_kbdc->input_buffer_full_mouse_callback().set(m_isabus, FUNC(isa16_device::irq12_w));
	m_kbdc->set_keyboard_tag("at_keyboard");

	BEBOX_JOYPORT(config, "joyport");

	at_keyboard_device &at_keyb(AT_KEYB(config, "at_keyboard", pc_keyboard_device::KEYBOARD_TYPE::AT, 2));
	at_keyb.keypress().set(m_kbdc, FUNC(kbdc8042_device::keyboard_w));

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("32M");
	m_ram->set_extra_options("8M,16M");
}

void bebox_state::bebox(machine_config &config)
{
	PPC603(config, m_ppc[0], 66000000);  /* 66 MHz */
	m_ppc[0]->set_addrmap(AS_PROGRAM, &bebox_state::main_mem);

	PPC603(config, m_ppc[1], 66000000);  /* 66 MHz */
	m_ppc[1]->set_addrmap(AS_PROGRAM, &bebox_state::main_mem);

	bebox_peripherals(config);
}

void bebox_state::bebox2(machine_config &config)
{
	PPC603E(config, m_ppc[0], 133000000);  /* 133 MHz */
	m_ppc[0]->set_addrmap(AS_PROGRAM, &bebox_state::main_mem);

	PPC603E(config, m_ppc[1], 133000000);  /* 133 MHz */
	m_ppc[1]->set_addrmap(AS_PROGRAM, &bebox_state::main_mem);

	bebox_peripherals(config);
}

ROM_START(bebox)
	ROM_REGION(0x00200000, "user1", ROMREGION_64BIT | ROMREGION_BE)
	ROM_SYSTEM_BIOS(0, "v11d7", "Be Boot ROM 1.1d7")
	ROMX_LOAD("bootmain.rom", 0x000000, 0x20000, CRC(df2d19e0) SHA1(da86a7d23998dc953dd96a2ac5684faaa315c701), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "r3", "BeOS Release 3 boot ROM")
	// /beos/etc/bebox_bootmain.image from the PowerPC Release 3 CD.
	ROMX_LOAD("r3_bebox_bootmain.image", 0x000000, 0x5c155, CRC(13fcec34) SHA1(f7c1585680865c2fbdb22cc7f1cbd484e679de63), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "r4", "BeOS Release 4 boot ROM")
	// /beos/etc/bebox_bootmain.image from the PowerPC Release 4 CD.
	ROMX_LOAD("r4_bebox_bootmain.image", 0x000000, 0x4ec05, CRC(49ea7676) SHA1(ec073c6216ec6e97a75d31358a67747292787c7a), ROM_BIOS(2))
	ROM_REGION(0x4000, "user2", ROMREGION_64BIT | ROMREGION_BE)
	ROM_LOAD( "bootnub.rom", 0x000000, 0x4000, CRC(5348d09a) SHA1(1b637a3d7a2b072aa128dd5c037bbb440d525c1a) )
ROM_END

#define rom_bebox2 rom_bebox

/*    YEAR   NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME */
COMP( 1995,  bebox,  0,      0,      bebox,   0,     bebox_state, init_bebox, "Be Inc", "BeBox Dual603-66",  MACHINE_NOT_WORKING )
COMP( 1996,  bebox2, bebox,  0,      bebox2,  0,     bebox_state, init_bebox, "Be Inc", "BeBox Dual603-133", MACHINE_NOT_WORKING )
