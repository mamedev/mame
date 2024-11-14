// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// HLE emulation of the MKS3 keyboard scanning PCB used in the psr340
// and psr540 among others.

// Uses a 63B05 with a not-yet-dumped internal rom

// Messages 0x81 and 0x82 are special, but not understood.

// What should happen so that the test mode works is not understood either.

#include "emu.h"
#include "mks3.h"

DEFINE_DEVICE_TYPE(MKS3, mks3_device, "mks3", "Yamaha MKS3 piano keyboard scanner")

static INPUT_PORTS_START(piano)
	PORT_START("P0")
	PORT_BIT(0xffffffff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("P1")
	PORT_BIT(0x0000000f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C1
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS1
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D1
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS1
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E1
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F1
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS1
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G1
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS1
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A1
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS1
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B1
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C2
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS2
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D2
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS2
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E2
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F2
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS2
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G2
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS2
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A2
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS2
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B2
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C3
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS3
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D3
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS3

	PORT_START("P2")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E3
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F3
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS3
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G3
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS3
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A3
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS3
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B3
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C4
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS4
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D4
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS4
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E4
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F4
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS4
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G4
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS4
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A4
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS4
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B4
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C5
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS5
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D5
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS5
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E5
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F5
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS5
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G5
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS5
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A5
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS5
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B5

	PORT_START("P3")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C6
	PORT_BIT(0xfffffffe, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


mks3_device::mks3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MKS3, tag, owner, clock),
	m_port(*this, "P%d", 0),
	m_write_da(*this),
	m_write_clk(*this)
{
}

ioport_constructor mks3_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(piano);
}

void mks3_device::device_start()
{
	save_item(NAME(m_ic));
	save_item(NAME(m_req));
	save_item(NAME(m_step));
	save_item(NAME(m_byte));
	save_item(NAME(m_bytes));
	save_item(NAME(m_byte_count));
	save_item(NAME(m_sent_state));
	save_item(NAME(m_current_state));

	m_ic = 1;
	m_req = 1;

	m_scan_timer = timer_alloc(FUNC(mks3_device::scan_tick), this);
	m_transmit_timer = timer_alloc(FUNC(mks3_device::transmit_tick), this);
}

void mks3_device::device_reset()
{
	m_write_da(1);
	m_write_clk(1);
	m_step = 0xff;
	m_byte = 0;
	m_byte_count = 0;
	std::fill(m_bytes.begin(), m_bytes.end(), 0);
	std::fill(m_sent_state.begin(), m_sent_state.end(), 0);

	for(int i=0; i != 4; i++)
		m_current_state[i] = m_port[i]->read();


	m_scan_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));
}

void mks3_device::ic_w(int state)
{
	if(state == m_ic)
		return;

	m_ic = state;
	if(!m_ic)
		reset();
}

void mks3_device::req_w(int state)
{
	if(state == m_req)
		return;

	m_req = state;
	if(m_req == 0 && m_step == 0xff)
		transmit_next();
}

TIMER_CALLBACK_MEMBER(mks3_device::transmit_tick)
{
	if(m_step == 15) {
		transmit_next();
		return;
	}

	if(m_step & 1) {
		m_write_clk(0);
		m_write_da(BIT(m_byte, 6 - (m_step >> 1)));
	} else
		m_write_clk(1);
	m_step++;

	m_transmit_timer->adjust(attotime::from_hz(500000));
}

void mks3_device::transmit_next()
{
	if(m_byte_count == 0) {
		m_step = 0xff;
		send_next();
		return;
	}

	m_byte = m_bytes[0];
	m_byte_count --;
	if(m_byte_count)
		memmove(m_bytes.data(), m_bytes.data() + 1, m_byte_count);

	m_write_clk(0);
	m_write_da(BIT(m_byte, 7));
	m_step = 0;
	m_transmit_timer->adjust(attotime::from_hz(500000));
}

u32 mks3_device::find_next()
{
	for(int i=0; i != 4; i++)
		if(m_current_state[i] != m_sent_state[i])
			for(int j=0; j != 32; j++)
				if(BIT(m_current_state[i]^m_sent_state[i], j)) {
					m_sent_state[i] ^= 1 << j;
					return (i << 5) | j | (BIT(m_current_state[i], j) ? 0x100 : 0);
			}
	return 0xffffffff;
}

void mks3_device::send_next()
{
	u32 key = find_next();
	if(key == 0xffffffff)
		return;

	// velocity is 10 bits, 0 = max, 3ff = min
	//
	// 80 | key number
	// 20 for maximal time(?), velocity >> 5 otherwise
	// 60 | (velocity & 1f) on) || 00 (keyoff)

	m_bytes[0] = 0x80 | (key & 0x7f);
	m_bytes[1] = 0; //key & 0x100 ? 0 : 0x20;
	m_bytes[2] = (key & 0x100) ? 0x60 : 0x00;
	m_byte_count = 3;
	transmit_next();
}


TIMER_CALLBACK_MEMBER(mks3_device::scan_tick)
{
	for(int i=0; i != 4; i++)
		m_current_state[i] = m_port[i]->read();

	if(m_step == 0xff)
		send_next();
}
