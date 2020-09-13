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
	return (BIT(gbd, 0, 2) << 6) | (BIT(gbd, 2, 2) << 4) | (BIT(gad, 0, 2) << 6) | (BIT(gad, 2, 2) << 4);
}


void fatfury2_prot_device::protection_w(offs_t offset, uint16_t data)
{
	// /PORTOEL connected into PRO-CT0 CLK pin
	m_pro_ct0->clk_w(true);

	m_pro_ct0->load_w(BIT(offset, 0)); // A1
	m_pro_ct0->even_w(BIT(offset, 1)); // A2
	m_pro_ct0->h_w(BIT(offset, 2)); // A3

	// C16-31 = A4-A19, C0-C15 = D0-D15
	m_pro_ct0->c_w((u32(bitswap<16>(BIT(offset, 3, 16), 15, 13, 11, 9, 14, 12, 10, 8, 7, 5, 3, 1, 6, 4, 2, 0)) << 16) |
		bitswap<16>(data, 15, 13, 11, 9, 14, 12, 10, 8, 7, 5, 3, 1, 6, 4, 2, 0));

	// release /PORTOEL
	m_pro_ct0->clk_w(false);
}
