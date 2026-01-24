// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#include "emu.h"
#include "sab80c535.h"
#include "mcs51dasm.h"

DEFINE_DEVICE_TYPE(SAB80C535, sab80c535_device, "sab80c535", "Siemens SAB80C535")

// 80:*p0
// 81:*sp
// 82:*dpl
// 83:*dph
// 87:*pcon, power control
// 88:*tcon, timer control
// 89:*tmod
// 8a:*tl0
// 8b:*tl1
// 8c:*th0
// 8d:*th1
// 90:*p1
// 98:*scon, serial control
// 99:*sbuf, serial buffer
// a0:*p2
// a8:*ien0
// a9:!ip0
// b0:*p3
// b8:!ien1
// b9: ip1
// c0: ircon, interrupt request control
// c1: ccen
// c2: ccl1
// c3: cch1
// c4: ccl2
// c5: cch2
// c6: ccl3
// c7: cch3
// c8:*t2con
// ca:*crcl
// cb:*crch
// cc:*tl2
// cd:*th2
// d0:*psw
// d8: adcon, a/d control
// d9: addat, a/d data
// da: dapr, d/a program
// db: p6 (also a/d)
// e0:*acc
// e8: p4
// f0:*b
// f8: p5



void sab80c535_device::p4_w(u8 data)
{
	m_p4 = data;
	m_port_out_cb[4](m_p4);
}

u8 sab80c535_device::p4_r()
{
	if(m_p4 == 0)
		return 0;
	return m_p4 & m_port_in_cb[4]();
}

void sab80c535_device::p5_w(u8 data)
{
	m_p5 = data;
	m_port_out_cb[5](m_p5);
}

u8 sab80c535_device::p5_r()
{
	if(m_p5 == 0)
		return 0;
	return m_p5 & m_port_in_cb[5]();
}

void sab80c535_device::adcon_w(u8 data)
{
	m_adcon = data;
}

u8 sab80c535_device::adcon_r()
{
	return m_adcon;
}

u8 sab80c535_device::addat_r()
{
	return m_an_func[m_adcon & 7]() * 2;
}


void sab80c535_device::sfr_map(address_map &map)
{
	i8052_device::sfr_map(map);
	map(0xd8, 0xd8).rw(FUNC(sab80c535_device::adcon_r), FUNC(sab80c535_device::adcon_w));
	map(0xd9, 0xd9).r(FUNC(sab80c535_device::addat_r));
	map(0xe8, 0xe8).rw(FUNC(sab80c535_device::p4_r), FUNC(sab80c535_device::p4_w));
	map(0xf8, 0xf8).rw(FUNC(sab80c535_device::p5_r), FUNC(sab80c535_device::p5_w));
}

sab80c535_device::sab80c535_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i8052_device(mconfig, SAB80C535, tag, owner, clock, 0)
	, m_an_func(*this, 0)
{
}

void sab80c535_device::device_start()
{
	i8052_device::device_start();
	save_item(NAME(m_p4));
	save_item(NAME(m_p5));
	save_item(NAME(m_adcon));
}

void sab80c535_device::device_reset()
{
	i8052_device::device_reset();
	m_p4 = 0xff;
	m_p5 = 0xff;
	m_adcon = 0x00;
}

std::unique_ptr<util::disasm_interface> sab80c535_device::create_disassembler()
{
	return std::make_unique<sab80c515_disassembler>();
}
