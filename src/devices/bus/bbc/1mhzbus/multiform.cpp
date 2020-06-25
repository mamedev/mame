// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    PEDL Multiform Z80 2nd processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Technomatic_MultiformZ80.html

    TODO:
    - Find utility disc, library disc, and manual

**********************************************************************/


#include "emu.h"
#include "multiform.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_MULTIFORM, bbc_multiform_device, "bbc_multiform", "PEDL Multiform Z80")


//-------------------------------------------------
//  ADDRESS_MAP( z80_mem )
//-------------------------------------------------

void bbc_multiform_device::z80_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(bbc_multiform_device::mem_r), FUNC(bbc_multiform_device::mem_w));
}

//-------------------------------------------------
//  ADDRESS_MAP( z80_io )
//-------------------------------------------------

void bbc_multiform_device::z80_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(m_host_latch[0], FUNC(generic_latch_8_device::read)).w(m_parasite_latch[0], FUNC(generic_latch_8_device::write));
	map(0x01, 0x01).r(m_host_latch[1], FUNC(generic_latch_8_device::read)).w(m_parasite_latch[1], FUNC(generic_latch_8_device::write));
	map(0x80, 0x80).w(FUNC(bbc_multiform_device::rom_disable_w));
}

//-------------------------------------------------
//  ROM( multiform )
//-------------------------------------------------

ROM_START(multiform)
	ROM_REGION(0x2000, "osm", 0)
	ROM_LOAD("osmv200.rom", 0x0000, 0x2000, CRC(67ce1713) SHA1(314ee6211a7af6cab659640fbdd11f1078460f08))

	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_LOAD("osmz80v137.rom", 0x0000, 0x4000, CRC(79a8de8f) SHA1(0bc06ce54ab9a2242294bbbe9ce8f07e464bb674))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_multiform_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_z80, 8_MHz_XTAL / 2);
	m_z80->set_addrmap(AS_PROGRAM, &bbc_multiform_device::z80_mem);
	m_z80->set_addrmap(AS_IO, &bbc_multiform_device::z80_io);
	m_z80->set_irq_acknowledge_callback(FUNC(bbc_multiform_device::irq_callback));

	/* 74ls374 */
	GENERIC_LATCH_8(config, m_host_latch[0]);
	GENERIC_LATCH_8(config, m_host_latch[1]);
	GENERIC_LATCH_8(config, m_parasite_latch[0]);
	GENERIC_LATCH_8(config, m_parasite_latch[1]);

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_z80").set_original("bbc_flop_z80");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_multiform_device::device_rom_region() const
{
	return ROM_NAME( multiform );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_multiform_device - constructor
//-------------------------------------------------

bbc_multiform_device::bbc_multiform_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_MULTIFORM, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_z80(*this, "z80")
	, m_host_latch(*this, "host_latch%u", 0U)
	, m_parasite_latch(*this, "parasite_latch%u", 0U)
	, m_osm(*this, "osm")
	, m_rom_enabled(true)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_multiform_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x10000);

	/* register for save states */
	save_pointer(NAME(m_ram), 0x10000);
	save_item(NAME(m_rom_enabled));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_multiform_device::device_reset()
{
	m_rom_enabled = true;
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_multiform_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0x00:
		data = m_parasite_latch[0]->read();
		break;
	case 0x01:
		data = m_parasite_latch[1]->read();
		break;
	}

	return data;
}

void bbc_multiform_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		m_host_latch[0]->write(data);
		break;
	case 0x01:
		m_host_latch[1]->write(data);
		break;
	}
}

uint8_t bbc_multiform_device::mem_r(offs_t offset)
{
	uint8_t data;

	if (m_rom_enabled && offset < 0x2000)
		data = m_osm->base()[offset & 0x1fff];
	else
		data = m_ram[offset];

	return data;
}

void bbc_multiform_device::mem_w(offs_t offset, uint8_t data)
{
	m_ram[offset] = data;
}

void bbc_multiform_device::rom_disable_w(uint8_t data)
{
	if (!machine().side_effects_disabled())
		m_rom_enabled = false;
}

//-------------------------------------------------
//  irq vector callback
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(bbc_multiform_device::irq_callback)
{
	return 0xfe;
}
