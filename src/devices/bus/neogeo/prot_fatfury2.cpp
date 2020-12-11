// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#include "emu.h"
#include "prot_fatfury2.h"



DEFINE_DEVICE_TYPE(NG_FATFURY2_PROT, fatfury2_prot_device, "ng_fatfury_prot", "Neo Geo Fatal Fury 2 Protection")


fatfury2_prot_device::fatfury2_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NG_FATFURY2_PROT, tag, owner, clock),
	m_pro_ct0(*this, "pro_ct0")
{
}


void fatfury2_prot_device::device_add_mconfig(machine_config &config)
{
	ALPHA_8921(config, m_pro_ct0, 0); // PRO-CT0 or SNK-9201
}

void fatfury2_prot_device::device_start()
{
}

void fatfury2_prot_device::device_reset()
{
}



/************************ Fatal Fury 2 *************************/

/* the protection involves reading and writing addresses in the */
/* 0x2xxxxx range. There are several checks all around the code. */
uint16_t fatfury2_prot_device::protection_r(offs_t offset)
{
	m_pro_ct0->even_w(BIT(offset, 1));
	m_pro_ct0->h_w(BIT(offset, 2));
	u8 gad = m_pro_ct0->gad_r();
	u8 gbd = m_pro_ct0->gbd_r();
	/*
	    Data pin from PRO-CT0
	    D0   D1   D2   D3   D4   D5   D6   D7
	    GAD2 GAD3 GAD0 GAD1 GBD2 GBD3 GBD0 GBD1
	*/
	return (BIT(gbd, 0, 2) << 6) | (BIT(gbd, 2, 2) << 4) | (BIT(gad, 0, 2) << 2) | (BIT(gad, 2, 2) << 0);
}


void fatfury2_prot_device::protection_w(offs_t offset, uint16_t data)
{
	// /PORTOEL connected into PRO-CT0 CLK pin
	m_pro_ct0->clk_w(true);

	m_pro_ct0->load_w(BIT(offset, 0)); // A1
	m_pro_ct0->even_w(BIT(offset, 1)); // A2
	m_pro_ct0->h_w(BIT(offset, 2)); // A3

	// C16-31 = A4-A19, C0-C15 = D0-D15
	/*
	    Address/Data pin mapping into PRO-CT0
	    C0  C1  C2  C3  C4  C5  C6  C7  C8  C9  C10 C11 C12 C13 C14 C15 C16 C17 C18 C19 C20 C21 C22 C23 C24 C25 C26 C27 C28 C29 C30 C31 LOAD EVEN H
	    D0  D4  D1  D5  D2  D6  D3  D7  D8  D12 D9  D13 D10 D14 D11 D15 A4  A8  A5  A9  A6  A10 A7  A11 A12 A16 A13 A17 A14 A18 A15 A19 A1   A2   A3
	*/
	m_pro_ct0->c_w((u32(bitswap<16>(BIT(offset, 3, 16), 15, 11, 14, 10, 13, 9, 12, 8, 7, 3, 6, 2, 5, 1, 4, 0)) << 16) |
		u32(bitswap<16>(data, 15, 11, 14, 10, 13, 9, 12, 8, 7, 3, 6, 2, 5, 1, 4, 0)));

	// release /PORTOEL
	m_pro_ct0->clk_w(false);
}
