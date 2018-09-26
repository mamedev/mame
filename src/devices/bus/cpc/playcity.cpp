// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
   PlayCity expansion device

   Z80 CTC
   2x YMZ294 (clocks provided by CTC)

   TODO:
   IRQs aren't working currently, the Z80CTC core requires the daisy chain setup to acknowledge IRQs properly, and that can't be used in a slot device currently.
*/

#include "emu.h"
#include "playcity.h"
#include "speaker.h"


void cpc_exp_cards(device_slot_interface &device);

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CPC_PLAYCITY, cpc_playcity_device, "cpc_playcity", "PlayCity")

// device machine config
MACHINE_CONFIG_START(cpc_playcity_device::device_add_mconfig)
	Z80CTC(config, m_ctc, XTAL(4'000'000));
	m_ctc->zc_callback<1>().set(FUNC(cpc_playcity_device::ctc_zc1_cb));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));
	m_ctc->intr_callback().set(FUNC(cpc_playcity_device::ctc_intr_cb));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	MCFG_DEVICE_ADD("ymz_1",YMZ294,XTAL(4'000'000))  // when timer is not set, operates at 4MHz (interally divided by 2, so equivalent to the ST)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)
	MCFG_DEVICE_ADD("ymz_2",YMZ294,XTAL(4'000'000))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)

	// pass-through
	MCFG_DEVICE_ADD("exp", CPC_EXPANSION_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(cpc_exp_cards, nullptr, false)
	MCFG_CPC_EXPANSION_SLOT_OUT_IRQ_CB(WRITELINE("^", cpc_expansion_slot_device, irq_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_NMI_CB(WRITELINE("^", cpc_expansion_slot_device, nmi_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_ROMDIS_CB(WRITELINE("^", cpc_expansion_slot_device, romdis_w))  // ROMDIS

MACHINE_CONFIG_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_playcity_device::cpc_playcity_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CPC_PLAYCITY, tag, owner, clock),
	device_cpc_expansion_card_interface(mconfig, *this),
	m_slot(nullptr),
	m_ctc(*this,"ctc"),
	m_ymz1(*this,"ymz_1"),
	m_ymz2(*this,"ymz_2")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_playcity_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_IO);
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());

	space.install_readwrite_handler(0xf880,0xf883,read8_delegate(FUNC(cpc_playcity_device::ctc_r),this),write8_delegate(FUNC(cpc_playcity_device::ctc_w),this));
	space.install_readwrite_handler(0xf884,0xf884,read8_delegate(FUNC(cpc_playcity_device::ymz1_data_r),this),write8_delegate(FUNC(cpc_playcity_device::ymz1_data_w),this));
	space.install_readwrite_handler(0xf888,0xf888,read8_delegate(FUNC(cpc_playcity_device::ymz2_data_r),this),write8_delegate(FUNC(cpc_playcity_device::ymz2_data_w),this));
	space.install_write_handler(0xf984,0xf984,write8_delegate(FUNC(cpc_playcity_device::ymz1_address_w),this));
	space.install_write_handler(0xf988,0xf988,write8_delegate(FUNC(cpc_playcity_device::ymz2_address_w),this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_playcity_device::device_reset()
{
}


READ8_MEMBER(cpc_playcity_device::ctc_r)
{
	return m_ctc->read(space,offset);
}

WRITE8_MEMBER(cpc_playcity_device::ctc_w)
{
	m_ctc->write(space,offset,data);
	if(offset == 0)
		update_ymz_clock();
}

WRITE8_MEMBER(cpc_playcity_device::ymz1_address_w)
{
	m_ymz1->address_w(space,offset,data);
}

WRITE8_MEMBER(cpc_playcity_device::ymz2_address_w)
{
	m_ymz2->address_w(space,offset,data);
}

WRITE8_MEMBER(cpc_playcity_device::ymz1_data_w)
{
	m_ymz1->data_w(space,offset,data);
}

WRITE8_MEMBER(cpc_playcity_device::ymz2_data_w)
{
	m_ymz2->data_w(space,offset,data);
}

READ8_MEMBER(cpc_playcity_device::ymz1_data_r)
{
	return m_ymz1->data_r(space,offset);
}

READ8_MEMBER(cpc_playcity_device::ymz2_data_r)
{
	return m_ymz2->data_r(space,offset);
}

void cpc_playcity_device::update_ymz_clock()
{
	// Bit of a hack job here, since there is no way currently to connect the CTC channel output directly to the YMZ clocks.
	uint8_t rate = m_ctc->get_channel_constant(0);
	uint32_t clk = 4000000;

	switch(rate)
	{
	case 0x00: clk = 3980000; break;
	case 0x01: clk = 2000000; break;
	case 0x02: clk = 3000000; break;
	case 0x03: clk = 3330000; break;
	case 0x04: clk = 3500000; break;
	case 0x05: clk = 3600000; break;
	case 0x06: clk = 3670000; break;
	case 0x07: clk = 3710000; break;
	case 0x08: clk = 3750000; break;
	case 0x09: clk = 3780000; break;
	case 0x0a: clk = 3800000; break;
	case 0x0b: clk = 3820000; break;
	case 0x0c: clk = 3830000; break;
	case 0x0d: clk = 3850000; break;
	case 0x0e: clk = 3860000; break;
	case 0x0f: clk = 3870000; break;
	}

	clk = clk / 2;  // YMZ294 has an internal /2 divider (not handled in AY core?)
	m_ymz1->ay_set_clock(clk);
	m_ymz2->ay_set_clock(clk);
	popmessage("YMZ clocks set to %d Hz",clk);
}
