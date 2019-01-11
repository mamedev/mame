// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Applied Engineering Quadralink 4-port serial card

***************************************************************************/

#include "emu.h"
#include "quadralink.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/terminal.h"
#include "bus/rs232/null_modem.h"
#include "screen.h"

static void isa_com(device_slot_interface &device)
{
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("null_modem", NULL_MODEM);
}

#define QUADRALINK_ROM_REGION  "qdlink_rom"

ROM_START( quadralink )
	ROM_REGION(0x4000, QUADRALINK_ROM_REGION, 0)
	ROM_LOAD( "ql_2.2.bin",   0x000000, 0x004000, CRC(88c323b8) SHA1(d70bc32ad50ceb5cb75c6251293cacd65d8313aa) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_QUADRALINK, nubus_quadralink_device, "nb_qdlink", "Applied Engineering Quadralink serial card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(nubus_quadralink_device::device_add_mconfig)
	MCFG_SCC8530_ADD("scc1", XTAL(3'686'400), 0, 0, 0, 0)
	MCFG_Z80SCC_OUT_TXDA_CB(WRITELINE("serport0", rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_TXDB_CB(WRITELINE("serport1", rs232_port_device, write_txd))

	MCFG_SCC8530_ADD("scc2", XTAL(3'686'400), 0, 0, 0, 0)
	MCFG_Z80SCC_OUT_TXDA_CB(WRITELINE("serport2", rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_TXDB_CB(WRITELINE("serport3", rs232_port_device, write_txd))

	MCFG_DEVICE_ADD( "serport0", RS232_PORT, isa_com, nullptr )
	MCFG_RS232_RXD_HANDLER(WRITELINE("scc1", z80scc_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("scc1", z80scc_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("scc1", z80scc_device, ctsa_w))

	MCFG_DEVICE_ADD( "serport1", RS232_PORT, isa_com, nullptr )
	MCFG_RS232_RXD_HANDLER(WRITELINE("scc1", z80scc_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("scc1", z80scc_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("scc1", z80scc_device, ctsb_w))

	MCFG_DEVICE_ADD( "serport2", RS232_PORT, isa_com, nullptr )
	MCFG_RS232_RXD_HANDLER(WRITELINE("scc2", z80scc_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("scc2", z80scc_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("scc2", z80scc_device, ctsa_w))

	MCFG_DEVICE_ADD( "serport3", RS232_PORT, isa_com, nullptr )
	MCFG_RS232_RXD_HANDLER(WRITELINE("scc2", z80scc_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("scc2", z80scc_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("scc2", z80scc_device, ctsb_w))
MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_quadralink_device::device_rom_region() const
{
	return ROM_NAME( quadralink );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_quadralink_device - constructor
//-------------------------------------------------

nubus_quadralink_device::nubus_quadralink_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_quadralink_device(mconfig, NUBUS_QUADRALINK, tag, owner, clock)
{
}

nubus_quadralink_device::nubus_quadralink_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_scc1(*this, "scc1"),
	m_scc2(*this, "scc2")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_quadralink_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(this, QUADRALINK_ROM_REGION);

	slotspace = get_slotspace();

	nubus().install_device(slotspace, slotspace+0xefffff, read32_delegate(FUNC(nubus_quadralink_device::dev_r), this), write32_delegate(FUNC(nubus_quadralink_device::dev_w), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_quadralink_device::device_reset()
{
}

WRITE32_MEMBER( nubus_quadralink_device::dev_w )
{
	//printf("write %x to QL space @ %x, mask %08x\n", data, offset, mem_mask);
	switch (offset)
	{
		case 0x0000:    // SCC 2 A control
			m_scc2->ca_w(space, 0, data & 0xff);
			break;

		case 0x0002:    // SCC 2 A data
			m_scc2->da_w(space, 0, data & 0xff);
			break;

		case 0x0004:    // SCC 2 B control
			m_scc2->cb_w(space, 0, data & 0xff);
			break;

		case 0x0006:    // SCC 2 B data
			m_scc2->db_w(space, 0, data & 0xff);
			break;

		case 0x10000:   // SCC 1 A control
			m_scc1->ca_w(space, 0, data & 0xff);
			break;

		case 0x10002:   // SCC 1 A data
			m_scc1->da_w(space, 0, data & 0xff);
			break;

		case 0x10004:   // SCC 1 B control
			m_scc1->cb_w(space, 0, data & 0xff);
			break;

		case 0x10006:   // SCC 1 B data
			m_scc1->db_w(space, 0, data & 0xff);
			break;
	}
}

READ32_MEMBER( nubus_quadralink_device::dev_r )
{
	//printf("read QL space @ %x, mask %08x\n", offset, mem_mask);
	switch (offset)
	{
		case 0x0000:
			return m_scc2->ca_r(space, 0);

		case 0x0002:
			return m_scc2->da_r(space, 0);

		case 0x0004:
			return m_scc2->cb_r(space, 0);

		case 0x0006:
			return m_scc2->db_r(space, 0);

		case 0x10000:
			return m_scc1->ca_r(space, 0);

		case 0x10002:
			return m_scc1->da_r(space, 0);

		case 0x10004:
			return m_scc1->cb_r(space, 0);

		case 0x10006:
			return m_scc1->db_r(space, 0);
	}
	return 0xffffffff;
}
