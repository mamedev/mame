// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CST Q+4 emulation

**********************************************************************/

#include "emu.h"
#include "cst_q_plus4.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC6821_TAG "mc6821"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CST_Q_PLUS4, cst_q_plus4_device, "ql_qplus4", "CST Q+4")


//-------------------------------------------------
//  ROM( cst_q_plus4 )
//-------------------------------------------------

ROM_START( cst_q_plus4 )
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_LOAD( "qplus4.rom", 0x0000, 0x2000, CRC(53a078fb) SHA1(53d6828c1b6ba052b862fd80ac8a364b0078330d) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *cst_q_plus4_device::device_rom_region() const
{
	return ROM_NAME( cst_q_plus4 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cst_q_plus4_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, MC6821_TAG, 0);

	QL_EXPANSION_SLOT(config, m_exp1, DERIVED_CLOCK(1, 1), ql_expansion_cards, nullptr);
	m_exp1->extintl_wr_callback().set(FUNC(cst_q_plus4_device::exp1_extintl_w));

	QL_EXPANSION_SLOT(config, m_exp2, DERIVED_CLOCK(1, 1), ql_expansion_cards, nullptr);
	m_exp2->extintl_wr_callback().set(FUNC(cst_q_plus4_device::exp2_extintl_w));

	QL_EXPANSION_SLOT(config, m_exp3, DERIVED_CLOCK(1, 1), ql_expansion_cards, nullptr);
	m_exp3->extintl_wr_callback().set(FUNC(cst_q_plus4_device::exp3_extintl_w));

	QL_EXPANSION_SLOT(config, m_exp4, DERIVED_CLOCK(1, 1), ql_expansion_cards, nullptr);
	m_exp4->extintl_wr_callback().set(FUNC(cst_q_plus4_device::exp4_extintl_w));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cst_q_plus4_device - constructor
//-------------------------------------------------

cst_q_plus4_device::cst_q_plus4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CST_Q_PLUS4, tag, owner, clock),
	device_ql_expansion_card_interface(mconfig, *this),
	m_exp1(*this, "exp1"),
	m_exp2(*this, "exp2"),
	m_exp3(*this, "exp3"),
	m_exp4(*this, "exp4"),
	m_rom(*this, "rom"),
	m_exp1_extinl(CLEAR_LINE),
	m_exp2_extinl(CLEAR_LINE),
	m_exp3_extinl(CLEAR_LINE),
	m_exp4_extinl(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cst_q_plus4_device::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t cst_q_plus4_device::read(address_space &space, offs_t offset, uint8_t data)
{
	if (offset >= 0xc000 && offset < 0xc200)
	{
		data = m_rom->base()[offset & 0x1fff];
	}

	data = m_exp1->read(space, offset, data);
	data = m_exp2->read(space, offset, data);
	data = m_exp3->read(space, offset, data);
	data = m_exp4->read(space, offset, data);

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void cst_q_plus4_device::write(address_space &space, offs_t offset, uint8_t data)
{
	m_exp1->write(space, offset, data);
	m_exp2->write(space, offset, data);
	m_exp3->write(space, offset, data);
	m_exp4->write(space, offset, data);
}
