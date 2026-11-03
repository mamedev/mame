// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Grull Osgo, Peter Ferrie
/**************************************************************************************************

California Chase (c) 1999 The Game Room
Eggs Playing Chicken (c) 2002 The Game Room
Host Invaders (c) 1998 The Game Room

driver by Angelo Salese & Grull Osgo

Hardware is known as "AUSCOM System 1" hardware.

Other games that may be on this hardware (if ever finished):
- Agro (c) The Game Room (prototype v1 perhaps?)
- Tiger Odds (c) The Game Room
- one other unnamed/unfinished game

Notes:
- calchase: type boot to load game

TODO:
- Convert to PCI, needs its own Super I/O, pre-AGP VIA chipset (VT82C580VPX Apollo VPX) and
  TGUI9680 conversion;
- calchase: needs a redump, most of Win98 is damaged (has no folder structure);
- calchase: requires franticf style wheel, service mode just black screen, has various gfx bugs
  during gameplay;
- hostinv: is an El Torito CD-ROM, can't boot here until PCI section is rewritten;
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

************************************************************************************/
/*
Grull Osgo - Improvements

- Changes about BIOS memory management so ROM Shadow now works properly.
  The changes are:
  Rom Memory Map remmapped to 128K size map(0xfffe0000, 0xffffffff).rom().region("bios", 0);

- Changes in mtxc write handler and bios_ram write handler. Now The internal register access are
  compatible with chipset VIA.
  (this motherboard has VIA Apollo VXPro chipset. It is not compatible with Intel i430).
  With this changes now BIOS Shadow ram works fine, BIOS can relocate and decompress the full code
  necessary to run the Extended BIOS, POST and Boot). No more BIOS Checksum error.

- Suppressed all video related items wich will be replaced with VGA driver.

- Temporarily added a VGA driver that is working based on original IBM VGA BIOS.(From MESS)
  (This VGA driver doesn't work yet with TRIDENT VGA BIOS as I can see).

- Added the flag READONLY to the calchase imagen rom load function, to avoid
  "DIFF CHD ERROR".

- Minor changes and NOPS into address maps for debugging purposes.

- Now BIOS is looking for the disk (BIOS Auto detection). It seems all works fine but must be there
  something wrong in the disk geometry reported by calchase.chd (20,255,63) since BIOS does not accept
  255 heads as parameter. Perhaps a bad dump?

- update by peter ferrie:
- corrected memory map to 64kb blocks
- corrected access to PAM register
*/


#include "emu.h"

#include "pcshare.h"

#include "bus/isa/isa.h"
#include "bus/isa/svga_trident.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "machine/ds128x.h"
#include "machine/nvram.h"
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

	// TODO: implement me once conversion to PCI kicks in
//  virtual void remap(int space_id, offs_t start, offs_t end) override;
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

	m_isa->install_memory(0x000d0000, 0x000d0fff, *this, &isa16_calchase_jamma_if::io_map);
}

void isa16_calchase_jamma_if::device_reset()
{
	// TODO: timing and actual source
	m_hb_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
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

// VIA VT82C585VPX north bridge (Apollo VP), config space only
class calchase_host_device : public pci_host_device
{
public:
	template <typename T>
	calchase_host_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: calchase_host_device(mconfig, tag, owner)
	{
		m_host_cpu.set_tag(std::forward<T>(cpu_tag));
	}
	calchase_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto pam_cb() { return m_pam_cb.bind(); }

protected:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	virtual void device_start() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	required_device<cpu_device> m_host_cpu;
	devcb_write8 m_pam_cb;

	address_space_config m_pci_mem_config;
	address_space_config m_pci_io_config;

	uint8_t m_config_reg[256];

	uint8_t config_r(offs_t offset);
	void config_w(offs_t offset, uint8_t data);
};

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(CALCHASE_HOST, calchase_host_device, calchase_host_device, "calchase_host", "VIA VT82C585VPX north bridge (California Chase)")

calchase_host_device::calchase_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, CALCHASE_HOST, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_pam_cb(*this)
	, m_pci_mem_config("pci_mem", ENDIANNESS_LITTLE, 32, 32)
	, m_pci_io_config("pci_io", ENDIANNESS_LITTLE, 32, 16)
{
	set_ids_host(0x11060585, 0x00, 0x00000000);
}

device_memory_interface::space_config_vector calchase_host_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_pci_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_pci_io_config));
	return r;
}

void calchase_host_device::device_start()
{
	pci_host_device::device_start();

	// no PCI devices behind the bridge, keep the PCI side off the CPU spaces
	set_spaces(&space(AS_PCI_MEM), &space(AS_PCI_IO));
	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;

	m_host_cpu->space(AS_IO).install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	std::fill(std::begin(m_config_reg), std::end(m_config_reg), 0);
	m_config_reg[0x60] = 0x02;
	m_config_reg[0x61] = 0x02;
	m_config_reg[0x62] = 0x02;
	m_config_reg[0x63] = 0x02;
	m_config_reg[0x64] = 0x02;
	m_config_reg[0x65] = 0x02;

	save_item(NAME(m_config_reg));
}

void calchase_host_device::config_map(address_map &map)
{
	// plain register file, the header is not decoded either
	map(0x00, 0x03).lr32(NAME([this] () -> uint32_t { return (main_id >> 16) | (main_id << 16); }));
	map(0x04, 0xff).rw(FUNC(calchase_host_device::config_r), FUNC(calchase_host_device::config_w));
	map(0x08, 0x0b).lr32(NAME([this] () -> uint32_t { return (pclass << 8) | revision; }));
}

uint8_t calchase_host_device::config_r(offs_t offset)
{
	return m_config_reg[0x04 + offset];
}

void calchase_host_device::config_w(offs_t offset, uint8_t data)
{
	const offs_t reg = 0x04 + offset;
	m_config_reg[reg] = data;
	if (reg == 0x63)
		m_pam_cb(0, data);
}


namespace {

// VIA VT82C586B south bridge (Apollo VP), config space only
class calchase_south_device : public pci_device
{
public:
	calchase_south_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	uint8_t m_config_reg[256];
};

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(CALCHASE_SOUTH, calchase_south_device, calchase_south_device, "calchase_south", "VIA VT82C586B south bridge (California Chase)")

calchase_south_device::calchase_south_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, CALCHASE_SOUTH, tag, owner, clock)
{
	set_ids(0x11060586, 0x41, 0x060100, 0x00000000);
}

void calchase_south_device::device_start()
{
	pci_device::device_start();

	std::fill(std::begin(m_config_reg), std::end(m_config_reg), 0);
	save_item(NAME(m_config_reg));
}

void calchase_south_device::config_map(address_map &map)
{
	// plain register file, the header is not decoded either
	map(0x00, 0x03).lr32(NAME([this] () -> uint32_t { return (main_id >> 16) | (main_id << 16); }));
	map(0x04, 0xff).lrw8(
			NAME([this] (offs_t offset) -> uint8_t { return m_config_reg[0x04 + offset]; }),
			NAME([this] (offs_t offset, uint8_t data) { m_config_reg[0x04 + offset] = data; }));
	map(0x08, 0x0b).lr32(NAME([this] () -> uint32_t { return (pclass << 8) | revision; }));
}


namespace {

class calchase_state : public pcat_base_state
{
public:
	calchase_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
	{
	}

	void calchase(machine_config &config);
	void hostinv(machine_config &config);
	void init_calchase();
	void init_hostinv();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	std::unique_ptr<uint32_t[]> m_bios_ram;
	std::unique_ptr<uint32_t[]> m_bios_ext_ram;
	uint8_t m_pam = 0x02;

	void bios_ext_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void bios_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void pam_w(uint8_t data);
	void calchase_io(address_map &map) ATTR_COLD;
	void calchase_map(address_map &map) ATTR_COLD;
};

void calchase_state::pam_w(uint8_t data)
{
	/*
	memory banking with North Bridge:
	0x63 (PAM)  xx-- ---- BIOS extension 0xe0000 - 0xeffff
	            --xx ---- BIOS area 0xf0000-0xfffff
	            ---- xx-- ISA add-on BIOS 0xc0000 - 0xcffff
	            ---- --xx ISA add-on BIOS 0xd0000 - 0xdffff

	10 -> 1 = Write Enable, 0 = Read Enable
	*/

	if (data & 0x20)        // enable RAM access to region 0xf0000 - 0xfffff
		membank("bios_bank")->set_base(m_bios_ram.get());
	else                    // disable RAM access (reads go to BIOS ROM)
		membank("bios_bank")->set_base(memregion("bios")->base() + 0x10000);
	if (data & 0x80)        // enable RAM access to region 0xe0000 - 0xeffff
		membank("bios_ext")->set_base(m_bios_ext_ram.get());
	else
		membank("bios_ext")->set_base(memregion("bios")->base() + 0);

	m_pam = data;
}

void calchase_state::bios_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_pam & 0x10)       // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram.get() + offset);
	}
}

void calchase_state::bios_ext_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_pam & 0x40)       // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext_ram.get() + offset);
	}
}

void calchase_state::calchase_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
//  map(0x000a0000, 0x000bffff).rw("vga", FUNC(trident_vga_device::mem_r), FUNC(trident_vga_device::mem_w)); // VGA VRAM
//  map(0x000c0000, 0x000c7fff).rom().region("video_bios", 0);
//  map(0x000c8000, 0x000cffff).noprw();
//  map(0x000d0000, 0x000d0fff) ISA custom JAMMA (above)
	map(0x000e0000, 0x000effff).bankr("bios_ext").w(FUNC(calchase_state::bios_ext_ram_w));
	map(0x000f0000, 0x000fffff).bankr("bios_bank").w(FUNC(calchase_state::bios_ram_w));
	map(0x00100000, 0x03ffffff).ram();  // 64MB
	map(0x04000000, 0x28ffffff).noprw();
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);    /* System BIOS */
}

void calchase_state::calchase_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x00e8, 0x00ef).noprw(); //AMI BIOS write to this ports as delays between I/O ports operations sending al value -> NEWIODELAY
//  map(0x0170, 0x0177).noprw(); //To debug
	map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));
	map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));
//  map(0x03f8, 0x03ff).noprw(); // To debug Serial Port COM1:
//  map(0x0a78, 0x0a7b).nopw();//.w(FUNC(calchase_state::pnp_data_w));
//  map(0x43c4, 0x43cb).rw("vga", FUNC(trident_vga_device::port_43c6_r), FUNC(trident_vga_device::port_43c6_w));  // Trident Memory and Video Clock register
//  map(0x83c4, 0x83cb).rw("vga", FUNC(trident_vga_device::port_83c6_r), FUNC(trident_vga_device::port_83c6_w));  // Trident LUTDAC
}


void calchase_state::machine_start()
{
	m_bios_ram = std::make_unique<uint32_t[]>(0x10000/4);
	m_bios_ext_ram = std::make_unique<uint32_t[]>(0x10000/4);

	save_item(NAME(m_pam));
}

void calchase_state::machine_reset()
{
	//membank("bank1")->set_base(memregion("bios")->base() + 0x10000);
	membank("bios_bank")->set_base(memregion("bios")->base() + 0x10000);
	membank("bios_ext")->set_base(memregion("bios")->base() + 0);
}

void calchase_isa16_cards(device_slot_interface &device)
{
	device.option_add("calchase_jamma_if", ISA16_CALCHASE_JAMMA_IF);
	device.option_add("tgui9680", ISA16_SVGA_TGUI9680);
}

void calchase_state::calchase(machine_config &config)
{
	PENTIUM(config, m_maincpu, 133000000); // Cyrix 686MX-PR200 CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &calchase_state::calchase_map);
	m_maincpu->set_addrmap(AS_IO, &calchase_state::calchase_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	ide_controller_32_device &ide(IDE_CONTROLLER_32(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	PCI_ROOT(config, "pci");
	CALCHASE_HOST(config, "pci:00.0", m_maincpu).pam_cb().set(FUNC(calchase_state::pam_w));
	CALCHASE_SOUTH(config, "pci:07.0");

	// FIXME: determine ISA bus clock
	isa16_device &isa(ISA16(config, "isa"));
	isa.set_memspace("maincpu", AS_PROGRAM);
	isa.set_iospace("maincpu", AS_IO);
	ISA16_SLOT(config, "isa1", 0, "isa", calchase_isa16_cards, "calchase_jamma_if", true);
	// TODO: temp, to be converted to PCI slot
	ISA16_SLOT(config, "isa2", 0, "isa", calchase_isa16_cards, "tgui9680", true);

	ds12885_device &rtc(DS12885(config.replace(), "rtc"));
	rtc.irq().set("pic8259_2", FUNC(pic8259_device::ir0_w));
	rtc.set_century_index(0x32);

}

void calchase_state::hostinv(machine_config &config)
{
	PENTIUM(config, m_maincpu, 133000000); // Cyrix 686MX-PR200 CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &calchase_state::calchase_map);
	m_maincpu->set_addrmap(AS_IO, &calchase_state::calchase_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	ide_controller_32_device &ide(IDE_CONTROLLER_32(config, "ide").options(ata_devices, "cdrom", nullptr, true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	PCI_ROOT(config, "pci");
	CALCHASE_HOST(config, "pci:00.0", m_maincpu).pam_cb().set(FUNC(calchase_state::pam_w));
	CALCHASE_SOUTH(config, "pci:07.0");

	// TODO: determine isa bus clock
	isa16_device &isa(ISA16(config, "isa"));
	isa.set_memspace("maincpu", AS_PROGRAM);
	isa.set_iospace("maincpu", AS_IO);
	ISA16_SLOT(config, "isa1", 0, "isa", calchase_isa16_cards, "calchase_jamma_if", true);
	// TODO: temp, to be converted to PCI slot
	ISA16_SLOT(config, "isa2", 0, "isa", calchase_isa16_cards, "tgui9680", true);
}

void calchase_state::init_calchase()
{
	m_bios_ram = std::make_unique<uint32_t[]>(0x20000/4);
}

void calchase_state::init_hostinv()
{
	m_bios_ram = std::make_unique<uint32_t[]>(0x20000/4);
}

ROM_START( calchase )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_LOAD( "mb_bios.bin", 0x00000, 0x20000, CRC(dea7a51b) SHA1(e2028c00bfa6d12959fc88866baca8b06a1eab68) )

	ROM_REGION( 0x800, "isa1:calchase_jamma_if:nvram", 0 )
	ROM_LOAD( "ds1220y_nv.bin", 0x000, 0x800, CRC(7912c070) SHA1(b4c55c7ca76bcd8dad1c4b50297233349ae02ed3) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE_READONLY( "calchase", 0, BAD_DUMP SHA1(6ae51a9b3f31cf4166322328a98c0235b0874eb3) )
ROM_END

ROM_START( hostinv )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_LOAD( "hostinv_bios.bin", 0x000000, 0x020000, CRC(5111e4b8) SHA1(20ab93150b61fd068f269368450734bba5dcb284) )

	ROM_REGION( 0x800, "isa1:calchase_jamma_if:nvram", ROMREGION_ERASEFF )
	ROM_LOAD( "ds1220y_hostinv.bin", 0x000, 0x800, NO_DUMP )

	DISK_REGION( "ide:0:cdrom" )
	DISK_IMAGE_READONLY( "hostinv", 0, SHA1(3cb86c62e80be98a717172b717f7276a0e5f6830) )
ROM_END

ROM_START( eggsplc )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_LOAD( "hostinv_bios.bin", 0x000000, 0x020000, CRC(5111e4b8) SHA1(20ab93150b61fd068f269368450734bba5dcb284) )

	ROM_REGION( 0x800, "isa1:calchase_jamma_if:nvram", ROMREGION_ERASEFF )
	ROM_LOAD( "ds1220y_eggsplc.bin", 0x000, 0x800, NO_DUMP )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE_READONLY( "eggsplc", 0, SHA1(fa38dd6b0d25cde644f68cf639768f137c607eb5) )
ROM_END

} // Anonymous namespace


GAME( 1998, hostinv,  0, hostinv,  0, calchase_state, init_hostinv,  ROT0, "The Game Room", "Host Invaders",        MACHINE_NOT_WORKING )
GAME( 1999, calchase, 0, calchase, 0, calchase_state, init_calchase, ROT0, "The Game Room", "California Chase",     MACHINE_NOT_WORKING )
GAME( 2002, eggsplc,  0, calchase, 0, calchase_state, init_hostinv,  ROT0, "The Game Room", "Eggs Playing Chicken", MACHINE_NOT_WORKING )
