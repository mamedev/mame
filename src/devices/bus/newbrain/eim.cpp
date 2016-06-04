// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Grundy NewBrain Expansion Interface Module emulation

**********************************************************************/

/*

    TODO:

    - everything

*/

#include "eim.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define MC6850_TAG      "459"
#define ADC0809_TAG     "427"
#define DAC0808_TAG     "461"
#define Z80CTC_TAG      "458"
#define RS232_TAG       "rs232"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NEWBRAIN_EIM = &device_creator<newbrain_eim_t>;


//-------------------------------------------------
//  ROM( newbrain_eim )
//-------------------------------------------------

ROM_START( newbrain_eim )
	ROM_REGION( 0x10000, "eim", 0 )
	ROM_LOAD( "e415-2.rom", 0x4000, 0x2000, CRC(5b0e390c) SHA1(0f99cae57af2e64f3f6b02e5325138d6ba015e72) )
	ROM_LOAD( "e415-3.rom", 0x4000, 0x2000, CRC(2f88bae5) SHA1(04e03f230f4b368027442a7c2084dae877f53713) ) // 18/8/83.aci
	ROM_LOAD( "e416-3.rom", 0x6000, 0x2000, CRC(8b5099d8) SHA1(19b0cfce4c8b220eb1648b467f94113bafcb14e0) ) // 10/8/83.mtv
	ROM_LOAD( "e417-2.rom", 0x8000, 0x2000, CRC(6a7afa20) SHA1(f90db4f8318777313a862b3d5bab83c2fd260010) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *newbrain_eim_t::device_rom_region() const
{
	return ROM_NAME( newbrain_eim );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( newbrain_eim )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( newbrain_eim )
	// devices
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_16MHz/8)
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE(MC6850_TAG, acia6850_device, write_rxc))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE(MC6850_TAG, acia6850_device, write_txc))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(newbrain_eim_t, ctc_z2_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("z80ctc_c2", newbrain_eim_t, ctc_c2_tick, attotime::from_hz(XTAL_16MHz/4/13))
	MCFG_DEVICE_ADD(ADC0809_TAG, ADC0808, 500000)
	MCFG_ADC0808_OUT_EOC_CB(WRITELINE(newbrain_eim_t, adc_eoc_w))
	MCFG_ADC0808_IN_VREF_POS_CB(newbrain_eim_t, adc_vref_pos_r)
	MCFG_ADC0808_IN_VREF_NEG_CB(newbrain_eim_t, adc_vref_neg_r)
	MCFG_ADC0808_IN_IN_0_CB(newbrain_eim_t, adc_input_r)
	MCFG_ADC0808_IN_IN_1_CB(newbrain_eim_t, adc_input_r)
	MCFG_ADC0808_IN_IN_2_CB(newbrain_eim_t, adc_input_r)
	MCFG_ADC0808_IN_IN_3_CB(newbrain_eim_t, adc_input_r)
	MCFG_ADC0808_IN_IN_4_CB(newbrain_eim_t, adc_input_r)
	MCFG_ADC0808_IN_IN_5_CB(newbrain_eim_t, adc_input_r)
	MCFG_ADC0808_IN_IN_6_CB(newbrain_eim_t, adc_input_r)
	MCFG_ADC0808_IN_IN_7_CB(newbrain_eim_t, adc_input_r)

	MCFG_DEVICE_ADD(MC6850_TAG, ACIA6850, 0)
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(newbrain_eim_t, acia_interrupt))
	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)

	MCFG_NEWBRAIN_EXPANSION_SLOT_ADD(NEWBRAIN_EXPANSION_SLOT_TAG, XTAL_16MHz/8, newbrain_expansion_cards, "fdc")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("96K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor newbrain_eim_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( newbrain_eim );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  newbrain_eim_t - constructor
//-------------------------------------------------

newbrain_eim_t::newbrain_eim_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, NEWBRAIN_EIM, "Newbrain EIM", tag, owner, clock, "newbrain_eim", __FILE__),
	device_newbrain_expansion_slot_interface(mconfig, *this),
	m_ctc(*this, Z80CTC_TAG),
	m_acia(*this, MC6850_TAG),
	m_exp(*this, NEWBRAIN_EXPANSION_SLOT_TAG),
	m_rom(*this, "eim"),
	m_ram(*this, RAM_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void newbrain_eim_t::device_start()
{
	// state saving
	save_item(NAME(m_aciaint));
	save_item(NAME(m_anint));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void newbrain_eim_t::device_reset()
{
}


//-------------------------------------------------
//  mreq_r - memory request read
//-------------------------------------------------

UINT8 newbrain_eim_t::mreq_r(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh)
{
	return m_exp->mreq_r(space, offset, data, romov, exrm, raminh);
}


//-------------------------------------------------
//  mreq_w - memory request write
//-------------------------------------------------

void newbrain_eim_t::mreq_w(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh)
{
	m_exp->mreq_w(space, offset, data, romov, exrm, raminh);
}


//-------------------------------------------------
//  iorq_r - I/O request read
//-------------------------------------------------

UINT8 newbrain_eim_t::iorq_r(address_space &space, offs_t offset, UINT8 data, bool &prtov)
{
	return m_exp->iorq_r(space, offset, data, prtov);
}


//-------------------------------------------------
//  iorq_w - I/O request write
//-------------------------------------------------

void newbrain_eim_t::iorq_w(address_space &space, offs_t offset, UINT8 data, bool &prtov)
{
	m_exp->iorq_w(space, offset, data, prtov);
}


//-------------------------------------------------
//  anout_r -
//-------------------------------------------------

READ8_MEMBER( newbrain_eim_t::anout_r )
{
	return 0xff;
}


//-------------------------------------------------
//  anout_w -
//-------------------------------------------------

WRITE8_MEMBER( newbrain_eim_t::anout_w )
{
}


//-------------------------------------------------
//  anin_r -
//-------------------------------------------------

READ8_MEMBER( newbrain_eim_t::anin_r )
{
	return 0;
}


//-------------------------------------------------
//  anio_w -
//-------------------------------------------------

WRITE8_MEMBER( newbrain_eim_t::anio_w )
{
}


//-------------------------------------------------
//  adc_eoc_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( newbrain_eim_t::adc_eoc_w )
{
	m_anint = state;
}


//-------------------------------------------------
//  adc_vref_pos_r -
//-------------------------------------------------

ADC0808_ANALOG_READ_CB( newbrain_eim_t::adc_vref_pos_r )
{
	return 5.0;
}


//-------------------------------------------------
//  adc_vref_neg_r -
//-------------------------------------------------

ADC0808_ANALOG_READ_CB( newbrain_eim_t::adc_vref_neg_r )
{
	return 0.0;
}


//-------------------------------------------------
//  adc_input_r -
//-------------------------------------------------

ADC0808_ANALOG_READ_CB( newbrain_eim_t::adc_input_r )
{
	return 0.0;
}


//-------------------------------------------------
//  acia_interrupt -
//-------------------------------------------------

WRITE_LINE_MEMBER( newbrain_eim_t::acia_interrupt )
{
	m_aciaint = state;
}


//-------------------------------------------------
//  ctc_z2_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( newbrain_eim_t::ctc_z2_w )
{
	// connected to CTC channel 0/1 clock inputs
	m_ctc->trg0(state);
	m_ctc->trg1(state);
}


//-------------------------------------------------
//  adc_input_r -
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(newbrain_eim_t::ctc_c2_tick)
{
	m_ctc->trg2(1);
	m_ctc->trg2(0);
}
