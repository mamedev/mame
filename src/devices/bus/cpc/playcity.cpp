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
void cpc_playcity_device::device_add_mconfig(machine_config &config)
{
	Z80CTC(config, m_ctc, DERIVED_CLOCK(1, 1));
	m_ctc->zc_callback<1>().set(FUNC(cpc_playcity_device::ctc_zc1_cb));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));
	m_ctc->intr_callback().set(FUNC(cpc_playcity_device::ctc_intr_cb));

	SPEAKER(config, "speaker", 2).front();
	YMZ294(config, m_ymz1, DERIVED_CLOCK(1, 1));  // when timer is not set, operates at 4MHz (interally divided by 2, so equivalent to the ST)
	m_ymz1->add_route(ALL_OUTPUTS, "speaker", 0.30, 1);
	YMZ294(config, m_ymz2, DERIVED_CLOCK(1, 1));
	m_ymz2->add_route(ALL_OUTPUTS, "speaker", 0.30, 0);

	// pass-through
	cpc_expansion_slot_device &exp(CPC_EXPANSION_SLOT(config, "exp", DERIVED_CLOCK(1, 1), cpc_exp_cards, nullptr));
	exp.irq_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::irq_w));
	exp.nmi_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::nmi_w));
	exp.romdis_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::romdis_w));  // ROMDIS
}


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
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());
	address_space &space = m_slot->cpu().space(AS_IO);

	space.install_readwrite_handler(0xf880,0xf883, read8sm_delegate(*this, FUNC(cpc_playcity_device::ctc_r)), write8sm_delegate(*this, FUNC(cpc_playcity_device::ctc_w)));
	space.install_readwrite_handler(0xf884,0xf884, read8smo_delegate(*this, FUNC(cpc_playcity_device::ymz1_data_r)), write8smo_delegate(*this, FUNC(cpc_playcity_device::ymz1_data_w)));
	space.install_readwrite_handler(0xf888,0xf888, read8smo_delegate(*this, FUNC(cpc_playcity_device::ymz2_data_r)), write8smo_delegate(*this, FUNC(cpc_playcity_device::ymz2_data_w)));
	space.install_write_handler(0xf984,0xf984, write8smo_delegate(*this, FUNC(cpc_playcity_device::ymz1_address_w)));
	space.install_write_handler(0xf988,0xf988, write8smo_delegate(*this, FUNC(cpc_playcity_device::ymz2_address_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_playcity_device::device_reset()
{
}


uint8_t cpc_playcity_device::ctc_r(offs_t offset)
{
	return m_ctc->read(offset);
}

void cpc_playcity_device::ctc_w(offs_t offset, uint8_t data)
{
	m_ctc->write(offset,data);
	if(offset == 0)
		update_ymz_clock();
}

void cpc_playcity_device::ymz1_address_w(uint8_t data)
{
	m_ymz1->address_w(data);
}

void cpc_playcity_device::ymz2_address_w(uint8_t data)
{
	m_ymz2->address_w(data);
}

void cpc_playcity_device::ymz1_data_w(uint8_t data)
{
	m_ymz1->data_w(data);
}

void cpc_playcity_device::ymz2_data_w(uint8_t data)
{
	m_ymz2->data_w(data);
}

uint8_t cpc_playcity_device::ymz1_data_r()
{
	return m_ymz1->data_r();
}

uint8_t cpc_playcity_device::ymz2_data_r()
{
	return m_ymz2->data_r();
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
