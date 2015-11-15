// license:BSD-3-Clause
// copyright-holders:Carl
#include "emu.h"
#include "ne1000.h"

static MACHINE_CONFIG_FRAGMENT(ne1000_config)
	MCFG_DEVICE_ADD("dp8390d", DP8390D, 0)
	MCFG_DP8390D_IRQ_CB(WRITELINE(ne1000_device, ne1000_irq_w))
	MCFG_DP8390D_MEM_READ_CB(READ8(ne1000_device, ne1000_mem_read))
	MCFG_DP8390D_MEM_WRITE_CB(WRITE8(ne1000_device, ne1000_mem_write))
MACHINE_CONFIG_END

const device_type NE1000 = &device_creator<ne1000_device>;

machine_config_constructor ne1000_device::device_mconfig_additions() const {
	return MACHINE_CONFIG_NAME(ne1000_config);
}

ne1000_device::ne1000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, NE1000, "NE1000 Network Adapter", tag, owner, clock, "ne1000", __FILE__),
		device_isa8_card_interface(mconfig, *this),
		m_dp8390(*this, "dp8390d") {
}

void ne1000_device::device_start() {
	char mac[7];
	UINT32 num = rand();
	memset(m_prom, 0x57, 16);
	sprintf(mac+2, "\x1b%c%c%c", (num >> 16) & 0xff, (num >> 8) & 0xff, num & 0xff);
	mac[0] = 0; mac[1] = 0;  // avoid gcc warning
	memcpy(m_prom, mac, 6);
	m_dp8390->set_mac(mac);
	set_isa_device();
	m_isa->install_device(0x0300, 0x031f, 0, 0, read8_delegate(FUNC(ne1000_device::ne1000_port_r), this), write8_delegate(FUNC(ne1000_device::ne1000_port_w), this));
}

void ne1000_device::device_reset() {
	memcpy(m_prom, m_dp8390->get_mac(), 6);
	m_irq = ioport("CONFIG")->read() & 3;
}

READ8_MEMBER(ne1000_device::ne1000_port_r) {
	if(offset < 16) {
		m_dp8390->dp8390_cs(CLEAR_LINE);
		return m_dp8390->dp8390_r(space, offset, mem_mask);
	}
	switch(offset) {
	case 16:
		m_dp8390->dp8390_cs(ASSERT_LINE);
		return m_dp8390->dp8390_r(space, offset, mem_mask);
	case 31:
		m_dp8390->dp8390_reset(CLEAR_LINE);
		return 0;
	default:
		logerror("ne1000: invalid register read %02X\n", offset);
	}
	return 0;
}

WRITE8_MEMBER(ne1000_device::ne1000_port_w) {
	if(offset < 16) {
		m_dp8390->dp8390_cs(CLEAR_LINE);
		m_dp8390->dp8390_w(space, offset, data, mem_mask);
		return;
	}
	switch(offset) {
	case 16:
		m_dp8390->dp8390_cs(ASSERT_LINE);
		m_dp8390->dp8390_w(space, offset, data, mem_mask);
		return;
	case 31:
		m_dp8390->dp8390_reset(ASSERT_LINE);
		return;
	default:
		logerror("ne1000: invalid register write %02X\n", offset);
	}
	return;
}

WRITE_LINE_MEMBER(ne1000_device::ne1000_irq_w) {
	switch(m_irq) {
	case 0:
		m_isa->irq2_w(state);
		break;
	case 1:
		m_isa->irq3_w(state);
		break;
	case 2:
		m_isa->irq4_w(state);
		break;
	case 3:
		m_isa->irq5_w(state);
		break;
	}
}

READ8_MEMBER(ne1000_device::ne1000_mem_read) {
	offset &= ~0xc000; // verify
	if(offset < 16) return m_prom[offset];
	if((offset < (8*1024)) || (offset >= (16*1024))) {
		logerror("ne1000: invalid memory read %04X\n", offset);
		return 0xff;
	}
	return m_board_ram[offset - (8*1024)];
}

WRITE8_MEMBER(ne1000_device::ne1000_mem_write) {
	offset &= ~0xc000; // verify
	if((offset < (8*1024)) || (offset >= (16*1024))) {
		logerror("ne1000: invalid memory write %04X\n", offset);
		return;
	}
	m_board_ram[offset - (8*1024)] = data;
}

static INPUT_PORTS_START( ne1000 )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x03, 0x01, "NE1000 IRQ jumper (W12-15)")
	PORT_CONFSETTING( 0x00, "IRQ2/9")
	PORT_CONFSETTING( 0x01, "IRQ3")
	PORT_CONFSETTING( 0x02, "IRQ4")
	PORT_CONFSETTING( 0x03, "IRQ5")
	//PORT_CONFNAME(0x30, 0x00, "NE1000 IO port jumper (W9-10)")
	//PORT_CONFSETTING( 0x00, "300")
	//PORT_CONFSETTING( 0x10, "320")
	//PORT_CONFSETTING( 0x20, "340")
	//PORT_CONFSETTING( 0x30, "360")
INPUT_PORTS_END

ioport_constructor ne1000_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ne1000);
}
