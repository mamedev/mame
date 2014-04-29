/*
 * cpc_rs232.c
 *
 *  Created on: 22/04/2014
 */

#include "cpc_rs232.h"
#include "includes/amstrad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CPC_RS232 = &device_creator<cpc_rs232_device>;

// device machine config
static MACHINE_CONFIG_FRAGMENT( cpc_rs232 )
	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(2000000)
	MCFG_PIT8253_CLK1(2000000)
	MCFG_PIT8253_CLK2(2000000)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(cpc_rs232_device, pit_out0_w))
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(cpc_rs232_device, pit_out1_w))
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(cpc_rs232_device, pit_out2_w))

	MCFG_Z80DART_ADD("dart", XTAL_4MHz, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232",default_rs232_devices,NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("dart",z80dart_device,rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("dart",z80dart_device,dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("dart",z80dart_device,ctsa_w))
//	MCFG_RS232_RI_HANDLER(DEVWRITELINE("dart",z80dart_device,ria_w))

	// pass-through
	MCFG_DEVICE_ADD("exp", CPC_EXPANSION_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(cpc_exp_cards, NULL, false)
	MCFG_CPC_EXPANSION_SLOT_OUT_IRQ_CB(DEVWRITELINE("^", cpc_expansion_slot_device, irq_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_NMI_CB(DEVWRITELINE("^", cpc_expansion_slot_device, nmi_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_ROMDIS_CB(DEVWRITELINE("^", cpc_expansion_slot_device, romdis_w))  // ROMDIS
	MCFG_CPC_EXPANSION_SLOT_OUT_ROMEN_CB(DEVWRITELINE("^", cpc_expansion_slot_device, romen_w))  // /ROMEN

MACHINE_CONFIG_END

machine_config_constructor cpc_rs232_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cpc_rs232 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_rs232_device::cpc_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CPC_RS232, "Amstrad/Pace RS232C interface", tag, owner, clock, "cpc_ser", __FILE__),
	device_cpc_expansion_card_interface(mconfig, *this),
	m_pit(*this,"pit"),
	m_dart(*this,"dart"),
	m_rs232(*this,"rs232")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_rs232_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_IO);
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());

	space.install_readwrite_handler(0xfadc,0xfadf,0,0,read8_delegate(FUNC(cpc_rs232_device::dart_r),this),write8_delegate(FUNC(cpc_rs232_device::dart_w),this));
	space.install_readwrite_handler(0xfbdc,0xfbdf,0,0,read8_delegate(FUNC(cpc_rs232_device::pit_r),this),write8_delegate(FUNC(cpc_rs232_device::pit_w),this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_rs232_device::device_reset()
{
}


WRITE_LINE_MEMBER(cpc_rs232_device::pit_out0_w)
{
	m_dart->txca_w(state);
}

WRITE_LINE_MEMBER(cpc_rs232_device::pit_out1_w)
{
	m_dart->rxca_w(state);
}

WRITE_LINE_MEMBER(cpc_rs232_device::pit_out2_w)
{
	m_dart->txcb_w(state);
	m_dart->rxcb_w(state);
}

READ8_MEMBER(cpc_rs232_device::dart_r)
{
	return m_dart->ba_cd_r(space,offset);
}

WRITE8_MEMBER(cpc_rs232_device::dart_w)
{
	m_dart->ba_cd_w(space,offset,data);
}

READ8_MEMBER(cpc_rs232_device::pit_r)
{
	return m_pit->read(space,offset);
}

WRITE8_MEMBER(cpc_rs232_device::pit_w)
{
	m_pit->write(space,offset,data);
}
