// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC05 32016 2nd processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC05_320162ndproc.html

    Acorn ANC06 Cambridge Co-Processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC06_CamCoPro.html

    IC1  (ULA) TUBE
    IC2  (MMU) NS32082 Not fitted
    IC3  (CPU) NS32016
    IC4  (TCU) NS32201
    IC20 (FPU) NS32081

**********************************************************************/


#include "emu.h"
#include "tube_32016.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_32016, bbc_tube_32016_device, "bbc_tube_32016", "Acorn 32016 2nd processor")
DEFINE_DEVICE_TYPE(BBC_TUBE_32016L, bbc_tube_32016l_device, "bbc_tube_32016l", "Acorn Large 32016 2nd processor")


//-------------------------------------------------
//  ADDRESS_MAP( tube_32016_mem )
//-------------------------------------------------

void bbc_tube_32016_device::tube_32016_mem(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(bbc_tube_32016_device::read), FUNC(bbc_tube_32016_device::write));
	map(0xf00000, 0xf07fff).rom().region("rom", 0).mirror(0x0f8000);
	map(0xf90000, 0xf90001).portr("CONFIG");
	map(0xfffff0, 0xffffff).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask16(0x00ff);
}

//-------------------------------------------------
//  ROM( tube_32016 )
//-------------------------------------------------

ROM_START(tube_32016)
	ROM_REGION16_LE(0x8000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "200", "Pandora v2.00")
	ROMX_LOAD("pan200lo.rom", 0x0000, 0x4000, CRC(b1980fd0) SHA1(8084f8896cd22953abefbd43c51e1a422b30e28d), ROM_SKIP(1) | ROM_BIOS(0)) // 0201-764-02 Pandora Lo
	ROMX_LOAD("pan200hi.rom", 0x0001, 0x4000, CRC(cab98d6b) SHA1(dfad1f4180c50757a74fcfe3a0ee7d7b48eb1bee), ROM_SKIP(1) | ROM_BIOS(0)) // 0201-763-02 Pandora Hi
	ROM_SYSTEM_BIOS(1, "100", "Pandora v1.00")
	ROMX_LOAD("pan100lo.rom", 0x0000, 0x2000, CRC(101e8aca) SHA1(4998e64afe98b2f227df6daa7ae0af512ce8907a), ROM_SKIP(1) | ROM_BIOS(1)) // 0201-764-01 Pandora Lo
	ROM_RELOAD(               0x4000, 0x2000)
	ROMX_LOAD("pan100hi.rom", 0x0001, 0x2000, CRC(139de9ed) SHA1(1871e65c9ebd3eac835b0317b0ad8272ff207c4b), ROM_SKIP(1) | ROM_BIOS(1)) // 0201-763-01 Pandora Hi
	ROM_RELOAD(               0x4001, 0x2000)
	ROM_SYSTEM_BIOS(2, "061", "Pandora v0.61")
	ROMX_LOAD("pan061lo.rom", 0x0000, 0x4000, CRC(6f801b35) SHA1(ce31f7c10603f3d15a06a8e32bde40df0639e446), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("pan061hi.rom", 0x0001, 0x4000, CRC(c00b1ab0) SHA1(e6a705232278c518340ddc69ea51af91965fa332), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

ROM_START(tube_32016l)
	ROM_REGION16_LE(0x8000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "200", "Pandora v2.00")
	ROMX_LOAD("pan200lo.rom", 0x0000, 0x4000, CRC(b1980fd0) SHA1(8084f8896cd22953abefbd43c51e1a422b30e28d), ROM_SKIP(1) | ROM_BIOS(0)) // 0201-764-02 Pandora Lo
	ROMX_LOAD("pan200hi.rom", 0x0001, 0x4000, CRC(cab98d6b) SHA1(dfad1f4180c50757a74fcfe3a0ee7d7b48eb1bee), ROM_SKIP(1) | ROM_BIOS(0)) // 0201-763-02 Pandora Hi
ROM_END

//-------------------------------------------------
//  INPUT_PORTS( tube_32016 )
//-------------------------------------------------

static INPUT_PORTS_START(tube_32016)
	PORT_START("CONFIG")
	PORT_DIPNAME(0x80, 0x80, "H") PORT_DIPLOCATION("LKS:1")
	PORT_DIPSETTING(0x80, "FPU")
	PORT_DIPSETTING(0x00, "No FPU")

	PORT_DIPNAME(0x40, 0x00, "G") PORT_DIPLOCATION("LKS:2")
	PORT_DIPSETTING(0x40, "MMU")
	PORT_DIPSETTING(0x00, "No MMU")

	PORT_DIPNAME(0x20, 0x00, "F") PORT_DIPLOCATION("LKS:3")
	PORT_DIPSETTING(0x20, "Reserved")
	PORT_DIPSETTING(0x00, "Reserved")

	PORT_DIPNAME(0x10, 0x00, "E") PORT_DIPLOCATION("LKS:4")
	PORT_DIPSETTING(0x10, "Reserved")
	PORT_DIPSETTING(0x00, "Reserved")

	PORT_DIPNAME(0x08, 0x00, "D") PORT_DIPLOCATION("LKS:5")
	PORT_DIPSETTING(0x08, "Reserved")
	PORT_DIPSETTING(0x00, "Reserved")

	PORT_DIPNAME(0x04, 0x00, "C") PORT_DIPLOCATION("LKS:6")
	PORT_DIPSETTING(0x04, "Reserved")
	PORT_DIPSETTING(0x00, "Reserved")

	PORT_DIPNAME(0x02, 0x00, "B") PORT_DIPLOCATION("LKS:7")
	PORT_DIPSETTING(0x02, "Reserved")
	PORT_DIPSETTING(0x00, "Reserved")

	PORT_DIPNAME(0x01, 0x00, "A") PORT_DIPLOCATION("LKS:8")
	PORT_DIPSETTING(0x01, "Reserved")
	PORT_DIPSETTING(0x00, "Reserved")
INPUT_PORTS_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_32016_device::device_add_mconfig(machine_config &config)
{
	NS32016(config, m_maincpu, 12_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_32016_device::tube_32016_mem);

	TUBE(config, m_ula);
	m_ula->pnmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ula->pirq_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	/* internal ram */
	RAM(config, m_ram).set_default_size("1M").set_extra_options("256K");

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_32016").set_original("bbc_flop_32016");
}


void bbc_tube_32016l_device::device_add_mconfig(machine_config &config)
{
	bbc_tube_32016_device::device_add_mconfig(config);

	m_maincpu->set_clock(16_MHz_XTAL / 2);

	/* internal ram */
	m_ram->set_default_size("4M");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_32016_device::device_rom_region() const
{
	return ROM_NAME( tube_32016 );
}

const tiny_rom_entry *bbc_tube_32016l_device::device_rom_region() const
{
	return ROM_NAME( tube_32016l );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_tube_32016_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tube_32016 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_32016_device - constructor
//-------------------------------------------------

bbc_tube_32016_device::bbc_tube_32016_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_tube_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_ula(*this, "ula")
	, m_ram(*this, "ram")
	, m_rom(*this, "rom")
	, m_rom_enabled(true)
{
}

bbc_tube_32016_device::bbc_tube_32016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_32016_device(mconfig, BBC_TUBE_32016, tag, owner, clock)
{
}

bbc_tube_32016l_device::bbc_tube_32016l_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_32016_device(mconfig, BBC_TUBE_32016L, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_32016_device::device_start()
{
	save_item(NAME(m_rom_enabled));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_32016_device::device_reset()
{
	m_rom_enabled = true;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_tube_32016_device::host_r(offs_t offset)
{
	return m_ula->host_r(offset);
}

void bbc_tube_32016_device::host_w(offs_t offset, uint8_t data)
{
	m_ula->host_w(offset, data);
}


uint8_t bbc_tube_32016_device::read(offs_t offset)
{
	uint16_t data;

	if (m_rom_enabled)
		data = m_rom->base()[offset & 0x7fff];
	else
		data = m_ram->pointer()[offset & m_ram->mask()];

	return data;
}

void bbc_tube_32016_device::write(offs_t offset, uint8_t data)
{
	/* clear ROM select on first write */
	m_rom_enabled = false;

	m_ram->pointer()[offset & m_ram->mask()] = data;
}
