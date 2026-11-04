// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Grull Osgo, Peter Ferrie
/**************************************************************************************************

California Chase (c) 1999 The Game Room
Eggs Playing Chicken (c) 2002 The Game Room
Host Invaders (c) 1998 The Game Room

Hardware is known as "AUSCOM System 1" hardware.

Other games that may be on this hardware (if ever finished):
- Agro (c) The Game Room (prototype v1 perhaps?)
- Tiger Odds (c) The Game Room
- one other unnamed/unfinished game

Notes:
- calchase: type boot to load game
- hostinv: need to set CMOS to load from CD-ROM (BIOS features setup -> Boot Sequence)

TODO:
- Convert TGUI9680 to be PCI;
- Hookup actual Super I/O (currently getting a port conflict out of it);
- Unmapped writes to 0x04xx'xxxx, chipset feature?
- calchase: needs a redump, most of Win98 is damaged (has no folder structure);
- calchase: requires franticf style wheel, service mode just black screen, has various gfx bugs
  during gameplay;
- hostinv: crashes around ROM-DOS "Qualitas MAX" memory manager, which eventually leads to a
  DOS-extender page fault. Passes that check if RAM is 256MB but then it will reset along the way;
- eggsplc: fix input labels (needs card subclassing), ticket dispenser, should require a default
  NVRAM (has "63" for every winning zone, unlikely), becomes unwinnable after first ticket
  win (propagates to successive reboots), EMM386 fails during boot, should be heavily idle
  skippable;

===================================================================================================

California Chase
The Game Room, 1999

This is a rip off of Chase HQ running on PC hardware
and standard 15kHz arcade monitor

Main board is a cheap Taiwanese one made by SOYO.
Model SY-5EAS
Major main board chips are....
Cyrix 686MX-PR200 CPU
1M BIOS (Winbond 29C011)
64M RAM (2x 32M 72 pin SIMMs)
ETEQ EQ82C6617'97 MB817A0AG12 (QFP100, x2)
UT6164C32Q-6 (QFP100, x2)
ETEQ 82C6619'97 MB13J15001 (QFP208)
ETEQ 82C6618'97 MB14B10971 (QFP208)
UM61256
PLL52C61
SMC FD37C669QF (QFP100)
3V coin battery
3x PCI slots
4x 16-bit ISA slots
4x 72 pin RAM slots
connectors for COM1, COM2, LPT1, IDE0, IDE1, floppy etc
uses standard AT PSU

Video card is Trident TGUI9680 with 512k on-board VRAM
Card is branded "Union UTD73" - these are all over eBay, for instance
RAM is AS4C256K16EO-50JC (x2)
Trident BIOS V5.5 (DIP28). Actual size unknown, dumped as 64k, 128k, 256k and 512k (can only be one of these sizes)
6.5536MHz xtal

Custom JAMMA I/O board plugs into one ISA slot
Major components are...
XILINX XC3042
Dallas DS1220Y NVRAM (dumped)
MACH210 (protected)
16MHz OSC
VGA connector (tied to VGA card with a cable)
2x 4-pin connectors for controls
AD7547
ULN2803
LM324 (op amp)
TDA1552 (power amp)
ST TS272 (dual op amp)
2 volume pots
power input connector (from AT PSU)
JAMMA edge connector
ISA edge connector

HDD is WD Caviar 2170. C/H/S = 1010/6/55. Capacity = 170.6MB
The HDD is DOS-readable and in fact the OS is just Windows 98 DOS and can
be easily copied. Tested with another HDD.... formatted with DOS, copied
all files across to new HDD, boots up fine.


Host Invaders is the same motherboard and video card as above, but instead of an HDD,
there is a CD-ROM drive.

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "bus/isa/svga_trident.h"
#include "bus/pci/pci_slot.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/ds128x.h"
#include "machine/fdc37c665ir.h"
#include "machine/idectrl.h"
#include "machine/nvram.h"
#include "machine/pci.h"
#include "machine/vt82c585vp.h"
#include "machine/vt82c586b_acpi.h"
#include "machine/vt82c586b_ide.h"
#include "machine/vt82c586b_isa.h"
#include "machine/vt82c586b_usb.h"
#include "sound/dac.h"
#include "video/pc_vga.h"

#include "screen.h"
#include "speaker.h"


class isa16_calchase_jamma_if : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_calchase_jamma_if(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	ioport_value heartbeat_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;
private:
	required_ioport_array<5> m_iocard;
	std::unique_ptr<uint8_t[]> m_nvram_data;
	TIMER_CALLBACK_MEMBER(heartbeat_cb);
	emu_timer *m_hb_timer = nullptr;
	bool m_hb_state = false;

	void io_map(address_map &map) ATTR_COLD;

	template <unsigned N> uint16_t iocard_r();
	uint8_t nvram_r(offs_t offset);
	void nvram_w(offs_t offset, uint8_t data);
};

DEFINE_DEVICE_TYPE(ISA16_CALCHASE_JAMMA_IF, isa16_calchase_jamma_if, "calchase_jamma_if", "ISA16 AUSCOM System 1 custom JAMMA I/F")

isa16_calchase_jamma_if::isa16_calchase_jamma_if(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_CALCHASE_JAMMA_IF, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_iocard(*this, "IOCARD%u", 1U)
{
}

void isa16_calchase_jamma_if::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // DS1220Y

	SPEAKER(config, "speaker", 2).front();
	DAC_12BIT_R2R(config, "ldac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 0); // unknown DAC
	DAC_12BIT_R2R(config, "rdac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 1); // unknown DAC
}

void isa16_calchase_jamma_if::device_start()
{
	set_isa_device();

	m_nvram_data = std::make_unique<uint8_t[]>(0x800);
	subdevice<nvram_device>("nvram")->set_base(m_nvram_data.get(), 0x800);

	m_hb_timer = timer_alloc(FUNC(isa16_calchase_jamma_if::heartbeat_cb), this);
}

void isa16_calchase_jamma_if::device_reset()
{
	// TODO: timing and actual source
	m_hb_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}

void isa16_calchase_jamma_if::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_isa->install_memory(0x000d0000, 0x000d0fff, *this, &isa16_calchase_jamma_if::io_map);
	}
}

template <unsigned N>
uint16_t isa16_calchase_jamma_if::iocard_r()
{
	return m_iocard[N]->read();
}

uint8_t isa16_calchase_jamma_if::nvram_r(offs_t offset)
{
	return m_nvram_data[offset];
}

void isa16_calchase_jamma_if::nvram_w(offs_t offset, uint8_t data)
{
	m_nvram_data[offset] = data;
}

void isa16_calchase_jamma_if::io_map(address_map &map)
{
//  map(0x0000, 0x0003).ram();  // XYLINX - Synchronous serial communication
	map(0x0004, 0x0005).r(FUNC(isa16_calchase_jamma_if::iocard_r<0>));
	map(0x0008, 0x000b).nopw(); // ???
	map(0x000c, 0x000d).r(FUNC(isa16_calchase_jamma_if::iocard_r<1>));
	map(0x0024, 0x0025).w("ldac", FUNC(dac_word_interface::data_w));
	map(0x0028, 0x0029).w("rdac", FUNC(dac_word_interface::data_w));
	map(0x0032, 0x0033).r(FUNC(isa16_calchase_jamma_if::iocard_r<2>));
	map(0x0030, 0x0031).r(FUNC(isa16_calchase_jamma_if::iocard_r<3>)); // These two controls wheel pot or whatever this game uses ...
	map(0x0034, 0x0035).r(FUNC(isa16_calchase_jamma_if::iocard_r<4>));
	map(0x0800, 0x0fff).rw(FUNC(isa16_calchase_jamma_if::nvram_r), FUNC(isa16_calchase_jamma_if::nvram_w)); // GAME_CMOS
}

// bit 13 of IOCARD3 needs to be flipped back and forth for logic to update
// (causing an hang if this is either one of the two states).
// This is either mirrored from PCI VGA card itself (unlikely) or it runs on its own free running timer.
// Also cfr. ice/lethalj.cpp franticf, where they run an analog steering wheel thru a serial pot and an irq
// (which is probably reused here)
TIMER_CALLBACK_MEMBER(isa16_calchase_jamma_if::heartbeat_cb)
{
	m_hb_state ^= 1;
}

ioport_value isa16_calchase_jamma_if::heartbeat_r()
{
	return m_hb_state;
}

static INPUT_PORTS_START( calchase_jamma )
	PORT_START("IOCARD1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x0008, 0x0008, "1" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Accelerator")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IOCARD2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 ) // guess
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset SW")
	PORT_DIPNAME( 0x0004, 0x0004, "2" )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Turbo")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // returns back to MS-DOS (likely to be unmapped and actually used as a lame protection check)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IOCARD3")
//  PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("isa2:tgui9680:screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(isa16_calchase_jamma_if::heartbeat_r))
	PORT_BIT( 0xdfff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IOCARD4")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSWA" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
//  PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("isa2:tgui9680:screen", FUNC(screen_device::vblank)) // eggsplc
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(isa16_calchase_jamma_if::heartbeat_r))
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("IOCARD5")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSWA" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

ioport_constructor isa16_calchase_jamma_if::device_input_ports() const
{
	return INPUT_PORTS_NAME(calchase_jamma);
}

namespace {

class calchase_state : public driver_device
{
public:
	calchase_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void calchase(machine_config &config);
	void hostinv(machine_config &config);

protected:

private:
	void calchase_io(address_map &map) ATTR_COLD;
	void calchase_map(address_map &map) ATTR_COLD;

	static void superio_config(device_t *device);
};

void calchase_state::calchase_map(address_map &map)
{
	map.unmap_value_high();
}

void calchase_state::calchase_io(address_map &map)
{
	map.unmap_value_high();
}


void calchase_isa16_cards(device_slot_interface &device)
{
	device.option_add("calchase_jamma_if", ISA16_CALCHASE_JAMMA_IF);
	device.option_add("tgui9680", ISA16_SVGA_TGUI9680);
	device.option_add_internal("superio", FDC37C665IR);
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

void calchase_state::superio_config(device_t *device)
{
	// TODO: upgrade to FDC37C669
	fdc37c665ir_device &fdc = *downcast<fdc37c665ir_device *>(device);
	fdc.fintr().set(":pci:07.0", FUNC(vt82c586b_isa_device::pc_irq6_w));
	fdc.pintr1().set(":pci:07.0", FUNC(vt82c586b_isa_device::pc_irq7_w));
	fdc.irq3().set(":pci:07.0", FUNC(vt82c586b_isa_device::pc_irq3_w));
	fdc.irq4().set(":pci:07.0", FUNC(vt82c586b_isa_device::pc_irq4_w));
	fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

void calchase_state::calchase(machine_config &config)
{
	// Cyrix 686MX-PR200 CPU
	pentium_device &maincpu(PENTIUM(config, "maincpu", 133'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &calchase_state::calchase_map);
	maincpu.set_addrmap(AS_IO, &calchase_state::calchase_io);
	maincpu.set_irq_acknowledge_callback("pci:07.0:pic0", FUNC(pic8259_device::inta_cb));
	maincpu.smiact().set("pci:00.0", FUNC(vt82c585vp_host_device::smi_act_w));

	PCI_ROOT(config, "pci");
	VT82C585VP_HOST(config, "pci:00.0", 0, "maincpu", 64*1024*1024);

	vt82c586b_isa_device &isa(VT82C586B_ISA(config, "pci:07.0", XTAL(33'000'000), "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.a20m().set_inputline("maincpu", INPUT_LINE_A20);
	isa.cpureset().set_inputline("maincpu", INPUT_LINE_RESET);
	isa.pcirst().set([this] (int state) {
		if (state)
			machine().schedule_soft_reset();
	});
	//isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	vt82c586b_ide_device &ide(VT82C586B_IDE(config, "pci:07.1", 0, "maincpu"));
	ide.irq_pri().set("pci:07.0", FUNC(vt82c586b_isa_device::pc_ide0_w));
	ide.irq_sec().set("pci:07.0", FUNC(vt82c586b_isa_device::pc_ide1_w));

	VT82C586B_USB (config, "pci:07.2", 0);

	vt82c586b_acpi_device &acpi_pci(VT82C586B_ACPI(config, "pci:07.3", 0));
	acpi_pci.sci_pin_cb().set("pci:07.0", FUNC(vt82c586b_isa_device::acpi_pin_config_w));
	acpi_pipc_device &acpi_dev(ACPI_PIPC(config, "pci:07.3:acpi"));
	acpi_dev.smi().set_inputline("maincpu", INPUT_LINE_SMI);
	acpi_dev.sci().set("pci:07.0", FUNC(vt82c586b_isa_device::pc_acpi_w));

	PCI_SLOT(config, "pci:1", pci_cards,  8, 0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards,  9, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 10, 2, 3, 0, 1, nullptr);

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", calchase_isa16_cards, "superio", true).set_option_machine_config("superio", superio_config);
	// TODO: temp, to be converted to PCI slot
	ISA16_SLOT(config, "isa1",   0, "pci:07.0:isabus", calchase_isa16_cards, "tgui9680", true);
	ISA16_SLOT(config, "isa2",   0, "pci:07.0:isabus", calchase_isa16_cards, "calchase_jamma_if", true);
	ISA16_SLOT(config, "isa3",   0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4",   0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, nullptr));
	serport0.rxd_handler().set("board4:superio", FUNC(fdc37c665ir_device::rxd1_w));
	serport0.dcd_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndcd1_w));
	serport0.dsr_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndsr1_w));
	serport0.ri_handler().set("board4:superio", FUNC(fdc37c665ir_device::nri1_w));
	serport0.cts_handler().set("board4:superio", FUNC(fdc37c665ir_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:superio", FUNC(fdc37c665ir_device::rxd2_w));
	serport1.dcd_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndcd2_w));
	serport1.dsr_handler().set("board4:superio", FUNC(fdc37c665ir_device::ndsr2_w));
	serport1.ri_handler().set("board4:superio", FUNC(fdc37c665ir_device::nri2_w));
	serport1.cts_handler().set("board4:superio", FUNC(fdc37c665ir_device::ncts2_w));
}

void calchase_state::hostinv(machine_config &config)
{
	calchase(config);

	// TODO: change up JAMMA i/f (this is a lightgun game)

	vt82c586b_ide_device &ide(VT82C586B_IDE(config.replace(), "pci:07.1", 0, "maincpu"));
	ide.irq_pri().set("pci:07.0", FUNC(vt82c586b_isa_device::pc_ide0_w));
	ide.irq_sec().set("pci:07.0", FUNC(vt82c586b_isa_device::pc_ide1_w));

	ide.subdevice<bus_master_ide_controller_device>("ide1")->slot(0).set_default_option("cdrom");
//  ide.subdevice<bus_master_ide_controller_device>("ide1")->slot(0).set_option_machine_config("cdrom", cdrom_config);
	ide.subdevice<bus_master_ide_controller_device>("ide2")->slot(0).set_default_option(nullptr);

}

ROM_START( calchase )
	ROM_REGION32_LE( 0x20000, "pci:07.0", 0 )
	ROM_LOAD( "mb_bios.bin", 0x00000, 0x20000, CRC(dea7a51b) SHA1(e2028c00bfa6d12959fc88866baca8b06a1eab68) )

	ROM_REGION( 0x800, "isa1:calchase_jamma_if:nvram", 0 )
	ROM_LOAD( "ds1220y_nv.bin", 0x000, 0x800, CRC(7912c070) SHA1(b4c55c7ca76bcd8dad1c4b50297233349ae02ed3) )

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE_READONLY( "calchase", 0, BAD_DUMP SHA1(6ae51a9b3f31cf4166322328a98c0235b0874eb3) )
ROM_END

ROM_START( hostinv )
	ROM_REGION32_LE( 0x20000, "pci:07.0", 0 )
	ROM_LOAD( "hostinv_bios.bin", 0x000000, 0x020000, CRC(5111e4b8) SHA1(20ab93150b61fd068f269368450734bba5dcb284) )

	ROM_REGION( 0x800, "isa1:calchase_jamma_if:nvram", ROMREGION_ERASEFF )
	ROM_LOAD( "ds1220y_hostinv.bin", 0x000, 0x800, NO_DUMP )

	DISK_REGION( "pci:07.1:ide1:0:cdrom" )
	DISK_IMAGE_READONLY( "hostinv", 0, SHA1(3cb86c62e80be98a717172b717f7276a0e5f6830) )
ROM_END

ROM_START( eggsplc )
	ROM_REGION32_LE( 0x20000, "pci:07.0", 0 )
	ROM_LOAD( "hostinv_bios.bin", 0x000000, 0x020000, CRC(5111e4b8) SHA1(20ab93150b61fd068f269368450734bba5dcb284) )

	ROM_REGION( 0x800, "isa1:calchase_jamma_if:nvram", ROMREGION_ERASEFF )
	ROM_LOAD( "ds1220y_eggsplc.bin", 0x000, 0x800, NO_DUMP )

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE_READONLY( "eggsplc", 0, SHA1(fa38dd6b0d25cde644f68cf639768f137c607eb5) )
ROM_END

} // Anonymous namespace


GAME( 1998, hostinv,  0, hostinv,  0, calchase_state, empty_init,  ROT0, "The Game Room", "Host Invaders",        MACHINE_NOT_WORKING )
GAME( 1999, calchase, 0, calchase, 0, calchase_state, empty_init,  ROT0, "The Game Room", "California Chase",     MACHINE_NOT_WORKING )
GAME( 2002, eggsplc,  0, calchase, 0, calchase_state, empty_init,  ROT0, "The Game Room", "Eggs Playing Chicken", MACHINE_NOT_WORKING )
