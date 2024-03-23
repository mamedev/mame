// license:BSD-3-Clause
// copyright-holders:Carl
#include "emu.h"
#include "ne1000.h"

#include "multibyte.h"


DEFINE_DEVICE_TYPE(NE1000, ne1000_device, "ne1000", "NE1000 Network Adapter")

void ne1000_device::device_add_mconfig(machine_config &config)
{
	DP8390D(config, m_dp8390, 0);
	m_dp8390->irq_callback().set(FUNC(ne1000_device::ne1000_irq_w));
	m_dp8390->mem_read_callback().set(FUNC(ne1000_device::ne1000_mem_read));
	m_dp8390->mem_write_callback().set(FUNC(ne1000_device::ne1000_mem_write));
}

ne1000_device::ne1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NE1000, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_dp8390(*this, "dp8390d"),
	m_irq(0)
{
}

void ne1000_device::device_start() {
	uint8_t mac[6];
	uint32_t num = machine().rand();
	memset(m_prom, 0x57, 16);
	mac[2] = 0x1b;
	put_u24be(mac+3, num);
	mac[0] = 0; mac[1] = 0;  // avoid gcc warning
	memcpy(m_prom, mac, 6);
	m_dp8390->set_mac(mac);
	set_isa_device();
	m_isa->install_device(0x0300, 0x031f, read8sm_delegate(*this, FUNC(ne1000_device::ne1000_port_r)), write8sm_delegate(*this, FUNC(ne1000_device::ne1000_port_w)));
}

void ne1000_device::device_reset() {
	memcpy(m_prom, &m_dp8390->get_mac()[0], 6);
	m_irq = ioport("CONFIG")->read() & 3;
}

uint8_t ne1000_device::ne1000_port_r(offs_t offset) {
	if(offset < 16) {
		return m_dp8390->cs_read(offset);
	}
	switch(offset) {
	case 16:
		return m_dp8390->remote_read();
	case 31:
		m_dp8390->dp8390_reset(CLEAR_LINE);
		return 0;
	default:
		logerror("ne1000: invalid register read %02X\n", offset);
	}
	return 0;
}

void ne1000_device::ne1000_port_w(offs_t offset, uint8_t data) {
	if(offset < 16) {
		m_dp8390->cs_write(offset, data);
		return;
	}
	switch(offset) {
	case 16:
		m_dp8390->remote_write(data);
		return;
	case 31:
		m_dp8390->dp8390_reset(ASSERT_LINE);
		return;
	default:
		logerror("ne1000: invalid register write %02X\n", offset);
	}
	return;
}

void ne1000_device::ne1000_irq_w(int state) {
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

uint8_t ne1000_device::ne1000_mem_read(offs_t offset) {
	offset &= ~0xc000; // verify
	if(offset < 16) return m_prom[offset];
	if((offset < (8*1024)) || (offset >= (16*1024))) {
		logerror("ne1000: invalid memory read %04X\n", offset);
		return 0xff;
	}
	return m_board_ram[offset - (8*1024)];
}

void ne1000_device::ne1000_mem_write(offs_t offset, uint8_t data) {
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
