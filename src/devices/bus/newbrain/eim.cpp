// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Grundy NewBrain Expansion Interface Module emulation

**********************************************************************/

/*

    TODO:

    - everything

*/

#include "emu.h"
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

DEFINE_DEVICE_TYPE(NEWBRAIN_EIM, newbrain_eim_device, "newbrain_eim", "NewBrain EIM")


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

const tiny_rom_entry *newbrain_eim_device::device_rom_region() const
{
	return ROM_NAME( newbrain_eim );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void newbrain_eim_device::device_add_mconfig(machine_config &config)
{
	// devices
	Z80CTC(config, m_ctc, XTAL(16'000'000)/8);
	m_ctc->zc_callback<0>().set(m_acia, FUNC(acia6850_device::write_rxc));
	m_ctc->zc_callback<1>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_ctc->zc_callback<2>().set(FUNC(newbrain_eim_device::ctc_z2_w));

	TIMER(config, "z80ctc_c2").configure_periodic(FUNC(newbrain_eim_device::ctc_c2_tick), attotime::from_hz(XTAL(16'000'000)/4/13));

	adc0809_device &adc(ADC0809(config, ADC0809_TAG, 500000));
	adc.eoc_callback().set(FUNC(newbrain_eim_device::adc_eoc_w));
	adc.in_callback<0>().set_constant(0);
	adc.in_callback<1>().set_constant(0);
	adc.in_callback<2>().set_constant(0);
	adc.in_callback<3>().set_constant(0);
	adc.in_callback<4>().set_constant(0);
	adc.in_callback<5>().set_constant(0);
	adc.in_callback<6>().set_constant(0);
	adc.in_callback<7>().set_constant(0);

	ACIA6850(config, m_acia, 0);
	m_acia->irq_handler().set(FUNC(newbrain_eim_device::acia_interrupt));

	RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr);

	NEWBRAIN_EXPANSION_SLOT(config, m_exp, XTAL(16'000'000)/8, newbrain_expansion_cards, "fdc");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("96K");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  newbrain_eim_device - constructor
//-------------------------------------------------

newbrain_eim_device::newbrain_eim_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEWBRAIN_EIM, tag, owner, clock),
	device_newbrain_expansion_slot_interface(mconfig, *this),
	m_ctc(*this, Z80CTC_TAG),
	m_acia(*this, MC6850_TAG),
	m_exp(*this, "exp"),
	m_rom(*this, "eim"),
	m_ram(*this, RAM_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void newbrain_eim_device::device_start()
{
	// state saving
	save_item(NAME(m_aciaint));
	save_item(NAME(m_anint));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void newbrain_eim_device::device_reset()
{
}


//-------------------------------------------------
//  mreq_r - memory request read
//-------------------------------------------------

uint8_t newbrain_eim_device::mreq_r(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh)
{
	return m_exp->mreq_r(offset, data, romov, exrm, raminh);
}


//-------------------------------------------------
//  mreq_w - memory request write
//-------------------------------------------------

void newbrain_eim_device::mreq_w(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh)
{
	m_exp->mreq_w(offset, data, romov, exrm, raminh);
}


//-------------------------------------------------
//  iorq_r - I/O request read
//-------------------------------------------------

uint8_t newbrain_eim_device::iorq_r(offs_t offset, uint8_t data, bool &prtov)
{
	return m_exp->iorq_r(offset, data, prtov);
}


//-------------------------------------------------
//  iorq_w - I/O request write
//-------------------------------------------------

void newbrain_eim_device::iorq_w(offs_t offset, uint8_t data, bool &prtov)
{
	m_exp->iorq_w(offset, data, prtov);
}


//-------------------------------------------------
//  anout_r -
//-------------------------------------------------

uint8_t newbrain_eim_device::anout_r()
{
	return 0xff;
}


//-------------------------------------------------
//  anout_w -
//-------------------------------------------------

void newbrain_eim_device::anout_w(uint8_t data)
{
}


//-------------------------------------------------
//  anin_r -
//-------------------------------------------------

uint8_t newbrain_eim_device::anin_r()
{
	return 0;
}


//-------------------------------------------------
//  anio_w -
//-------------------------------------------------

void newbrain_eim_device::anio_w(uint8_t data)
{
}


//-------------------------------------------------
//  adc_eoc_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( newbrain_eim_device::adc_eoc_w )
{
	m_anint = state;
}


//-------------------------------------------------
//  acia_interrupt -
//-------------------------------------------------

WRITE_LINE_MEMBER( newbrain_eim_device::acia_interrupt )
{
	m_aciaint = state;
}


//-------------------------------------------------
//  ctc_z2_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( newbrain_eim_device::ctc_z2_w )
{
	// connected to CTC channel 0/1 clock inputs
	m_ctc->trg0(state);
	m_ctc->trg1(state);
}


//-------------------------------------------------
//  ctc_c2_tick -
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(newbrain_eim_device::ctc_c2_tick)
{
	m_ctc->trg2(1);
	m_ctc->trg2(0);
}
