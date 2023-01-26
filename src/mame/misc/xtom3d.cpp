// license:BSD-3-Clause
// copyright-holders:Guru, Angelo Salese
/**************************************************************************************************

X Tom 3D

Customized i440bx Award based BIOS, "OKSAN MK III /EVATE Ver99.04.20"
"04/20/1999-i440BX-2A69KEIC-00"

Oksan is the old company name that became Andamiro.

TODO:
- Doesn't really accesses a Super I/O, which implies that the Holtek keyboard and the RTC chips are on a separate ISA plane.
- DIR texture folder will throw mangled file structure the second time around;


This game runs on PC-based hardware.
Major components are....

MAIN BOARD
----------
    CPU: Intel Celeron (socket 370) 333MHz
Chipset: Intel AGPset FW82443ZX, PCIset FW82371EB
    RAM: Samsung KMM366S823CTS 8M x 64-bit SDRAM DIMM
  Video: 3DFX 500-0013-04 PCB-mounted BGA
         EliteMT M32L1632512A video RAM (x4)
         14.31818MHz XTAL
   BIOS: Atmel 29C010 flash ROM
  Other: Holtek HT6542B i8042-based keyboard controller
         3V coin battery

SOUND BOARD
-----------
A40MX04 QFP84 CPLD
Yamaha YMZ280B + YAC516
16MHz XTAL
PIC12C508 (secured, not read)
Atmel 93C46 EEPROM
LM358 OP AMP (x3)

ROM BOARD
---------
MX29F1610MC 16M FlashROM (x7)

**************************************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/pci-smbus.h"
#include "machine/i82443bx_host.h"
#include "machine/i82371eb_isa.h"
#include "machine/i82371eb_ide.h"
#include "machine/i82371eb_acpi.h"
#include "machine/i82371eb_usb.h"
#include "video/virge_pci.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
//#include "bus/rs232/hlemouse.h"
//#include "bus/rs232/null_modem.h"
//#include "bus/rs232/rs232.h"
//#include "bus/rs232/sun_kbd.h"
//#include "bus/rs232/terminal.h"
//#include "machine/fdc37c93x.h"
#include "machine/mc146818.h"
#include "machine/8042kbdc.h"
#include "video/voodoo_pci.h"


/*
 * ISA16 Oksan ROM DISK
 */

class isa16_oksan_rom_disk : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_oksan_rom_disk(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_rom_tag(T &&tag) { m_flash_rom.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_memory_region m_flash_rom;

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void remap(int space_id, offs_t start, offs_t end) override;

	u8 m_flash_cmd = 0;
	u32 m_flash_addr = 0;
	bool m_flash_unlock = false;
	u8 m_flash_state = 0;
};

// "OKSAN (R) ROM DISK for MK-III Version 1.00.0305"
// "Copyright (C) OKSAN Co., Ltd. 1989-1999" (!)
DEFINE_DEVICE_TYPE(ISA16_OKSAN_ROM_DISK, isa16_oksan_rom_disk, "isa16_oksan_rom_disk", "ISA16 Oksan ROM DISK for MK-III")

isa16_oksan_rom_disk::isa16_oksan_rom_disk(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_OKSAN_ROM_DISK, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_flash_rom(*this, finder_base::DUMMY_TAG)
{
}

void isa16_oksan_rom_disk::device_start()
{
	set_isa_device();
}

void isa16_oksan_rom_disk::device_reset()
{
	m_flash_state = 0;
}

void isa16_oksan_rom_disk::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_isa->install_device(0x02d0, 0x02df, read8sm_delegate(*this, FUNC(isa16_oksan_rom_disk::read)), write8sm_delegate(*this, FUNC(isa16_oksan_rom_disk::write)));
	}
}

u8 isa16_oksan_rom_disk::read(offs_t offset)
{
	//printf("%02x\n", offset);
	if (offset == 0xa || offset == 0xb)
	{
		if (m_flash_cmd == 0xf0 && m_flash_unlock)
		{
			u8 rom_data = m_flash_rom->base()[((m_flash_addr << 1) + (offset & 1)) & 0xffffff];
			if (offset & 1 && !machine().side_effects_disabled())
				m_flash_addr ++;

			return rom_data;
		}
	}
	return 0;
}

void isa16_oksan_rom_disk::write(offs_t offset, u8 data)
{
//  if (offset < 8 && ((offset & 1) == 0) && m_flash_cmd == 0xf0)
	//  printf("%04x %04x \n", offset, data);

	switch(offset)
	{
		// address port
		case 0x0:
			m_flash_addr &= 0xffffff00;
			m_flash_addr |= data & 0xff;
			break;
		case 0x2:
			m_flash_addr &= 0xffff00ff;
			m_flash_addr |= (data & 0xff) << 8;
			break;
		case 0x4:
			//if (data)
			//	printf("%02x\n", data);
			m_flash_addr &= 0xff00ffff;
			m_flash_addr |= (data & 0xff) << 16;
			break;
		case 0x6:
			if (data)
				popmessage("%02x", data);
			//m_flash_addr &= 0xfe01ffff;
			//m_flash_addr |= data * 0x20000;
			//m_flash_addr &= 0x01ffffff;
			//m_flash_addr |= data * 0x2000000;
			break;
		// data port
		case 0xa:
			if (data == 0xaa && m_flash_addr == 0x5555 && m_flash_state == 0)
			{
				m_flash_state = 1;
			}
			else if (data == 0x55 && m_flash_addr == 0x2aaa && m_flash_state == 1)
				m_flash_state = 2;
			else if (m_flash_state == 2 && m_flash_addr == 0x5555)
			{
				m_flash_state = 0;
				m_flash_cmd = data;
				//printf("%02x %08x\n", data, m_flash_addr);
			}
			break;
		// chip enable, 0 -> 1 transitions
		case 0xc:
			m_flash_unlock = bool(BIT(data, 3));
			break;
	}
}

/*
 * ISA16 Oksan LPC
 */

class isa16_oksan_lpc : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_oksan_lpc(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<mc146818_device> m_rtc;
	required_device<kbdc8042_device> m_kbdc;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void remap(int space_id, offs_t start, offs_t end) override;
};

DEFINE_DEVICE_TYPE(ISA16_OKSAN_LPC, isa16_oksan_lpc, "isa16_oksan_lpc", "ISA16 Oksan LPC")

isa16_oksan_lpc::isa16_oksan_lpc(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_OKSAN_LPC, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
	, m_kbdc(*this, "kbdc")
{
}

void isa16_oksan_lpc::device_add_mconfig(machine_config &config)
{
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	//m_rtc->irq().set(m_pic8259_2, FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);

	KBDC8042(config, m_kbdc, 0);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	m_kbdc->system_reset_callback().set_inputline(":maincpu", INPUT_LINE_RESET);
	m_kbdc->gate_a20_callback().set_inputline(":maincpu", INPUT_LINE_A20);
	m_kbdc->input_buffer_full_callback().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq1_w));
}


void isa16_oksan_lpc::device_start()
{
	set_isa_device();
}

void isa16_oksan_lpc::device_reset()
{

}

void isa16_oksan_lpc::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_isa->install_device(0x60, 0x6f, read8sm_delegate(m_kbdc, FUNC(kbdc8042_device::data_r)), write8sm_delegate(m_kbdc, FUNC(kbdc8042_device::data_w)));
		m_isa->install_device(0x70, 0x7f, read8sm_delegate(m_rtc, FUNC(mc146818_device::read)), write8sm_delegate(m_rtc, FUNC(mc146818_device::write)));
	}
}


namespace {

#define PCI_J4D2_ID "pci:0d.0"

class xtom3d_state : public driver_device
{
public:
	xtom3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_voodoo(*this, PCI_J4D2_ID)
	{
	}

	void xtom3d(machine_config &config);

private:
	required_device<pentium2_device> m_maincpu;
	// TODO: optional for debugging
	optional_device<voodoo_banshee_pci_device> m_voodoo;

	void xtom3d_map(address_map &map);
//  void xtom3d_io(address_map &map);

	static void romdisk_config(device_t *device);
	static void lpc_config(device_t *device);
};


void xtom3d_state::xtom3d_map(address_map &map)
{
	map.unmap_value_high();
}

void xtom3d_isa_cards(device_slot_interface &device)
{
	device.option_add_internal("oksan_romdisk", ISA16_OKSAN_ROM_DISK);
	device.option_add_internal("oksan_lpc", ISA16_OKSAN_LPC);
}

void xtom3d_state::romdisk_config(device_t *device)
{
	isa16_oksan_rom_disk &romdisk = *downcast<isa16_oksan_rom_disk *>(device);
	romdisk.set_rom_tag("user2");
}

void xtom3d_state::lpc_config(device_t *device)
{
//  auto *state = device->subdevice<xtom3d_state>(":");
	//isa16_oksan_lpc &lpc = *downcast<isa16_oksan_lpc *>(device);
//  lpc.m_kbdc->system_reset_callback().set_inputline(":maincpu", INPUT_LINE_RESET);
//  m_kbdc->gate_a20_callback().set_inputline(":maincpu", INPUT_LINE_A20);
//  m_kbdc->input_buffer_full_callback().set(":, FUNC(pic8259_device::ir1_w));
}

// TODO: unverified PCI config space
void xtom3d_state::xtom3d(machine_config &config)
{
	PENTIUM2(config, m_maincpu, 450'000'000 / 16); // actually Pentium II 450
	m_maincpu->set_addrmap(AS_PROGRAM, &xtom3d_state::xtom3d_map);
//  m_maincpu->set_addrmap(AS_IO, &xtom3d_state::xtom3d_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->smiact().set("pci:00.0", FUNC(i82443bx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	I82443BX_HOST(config, "pci:00.0", 0, "maincpu", 32*1024*1024);
	I82443BX_BRIDGE(config, "pci:01.0", 0 ); //"pci:01.0:00.0");
	//I82443BX_AGP   (config, "pci:01.0:00.0");

	i82371eb_isa_device &isa(I82371EB_ISA(config, "pci:07.0", 0));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371eb_ide_device &ide(I82371EB_IDE(config, "pci:07.1", 0));
	ide.irq_pri().set("pci:07.0", FUNC(i82371eb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371eb_isa_device::pc_mirq0_w));

	I82371EB_USB (config, "pci:07.2", 0);
	I82371EB_ACPI(config, "pci:07.3", 0);
	LPC_ACPI     (config, "pci:07.3:acpi", 0);
	SMBUS        (config, "pci:07.3:smbus", 0);

	ISA16_SLOT(config, "board1", 0, "pci:07.0:isabus", xtom3d_isa_cards, "oksan_romdisk", true).set_option_machine_config("oksan_romdisk", romdisk_config);
	ISA16_SLOT(config, "board2", 0, "pci:07.0:isabus", xtom3d_isa_cards, "oksan_lpc", true).set_option_machine_config("oksan_lpc", lpc_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	// YMF740G goes thru "pci:0c.0"
	// Expansion slots, mapping SVGA for debugging
	// TODO: all untested, check clock
	// TODO: confirm Voodoo going in J4D2, may really go in AGP slot under bridge instead?
	#if 0
	VOODOO_BANSHEE_PCI(config, m_voodoo, 0, m_maincpu, "screen"); // "pci:0d.0" J4D2
	m_voodoo->set_fbmem(2);
	m_voodoo->set_tmumem(4, 4);
	m_voodoo->set_status_cycles(1000);

	// TODO: fix legacy raw setup here
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_size(640, 480);
	screen.set_visarea(0, 640 - 1, 0, 480 - 1);
	screen.set_screen_update(PCI_J4D2_ID, FUNC(voodoo_banshee_pci_device::screen_update));
	#else
	VIRGE_PCI(config, "pci:0e.0", 0); // J4C1
	#endif
	// "pci:0d.0" J4D2
	// "pci:0e.0" J4D1
}


ROM_START( xtom3d )
	ROM_REGION32_LE(0x20000, "pci:07.0", 0)
	ROM_LOAD( "bios.u22", 0x000000, 0x020000, CRC(f7c58044) SHA1(fd967d009e0d3c8ed9dd7be852946f2b9dee7671) )

	ROM_REGION(0x1000000, "board1:user2", ROMREGION_ERASEFF)
	ROM_LOAD( "u3",  0x000000, 0x200000, CRC(f332e030) SHA1(f04fc7fc97e6ada8122ea7d111455043d7cc42df) )
	ROM_LOAD( "u4",  0x200000, 0x200000, CRC(ac40ea0b) SHA1(6fcb86f493885d62d20df6bddaa1a1b19d478c65) )
	ROM_LOAD( "u5",  0x400000, 0x200000, CRC(0fb98a20) SHA1(d21f33b0ca65dc6f90a411a9682f960e9c60244c) )
	ROM_LOAD( "u6",  0x600000, 0x200000, CRC(5c092c58) SHA1(d347e1ed957cc989dc71f4f347af926589ae926d) )
	ROM_LOAD( "u7",  0x800000, 0x200000, CRC(833c179c) SHA1(586555f5a4066a762fc05a43ef01be9fa202bb7f) )
	ROM_LOAD( "u19", 0xa00000, 0x200000, CRC(a1ae73d0) SHA1(232c73bfee426b5f651a015c505c26b8ed7176b7) )
	ROM_LOAD( "u20", 0xc00000, 0x200000, CRC(452131d9) SHA1(f62a0f1a7da9025ac1f7d5de4df90166871ac1e5) )
ROM_END

} // anonymous namespace

GAME(1999, xtom3d, 0, xtom3d, 0, xtom3d_state, empty_init, ROT0, "Jamie System Development", "X Tom 3D", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
