// license:BSD-3-Clause
// copyright-holders:Carl
#include "emu.h"
#include "3c503.h"

#define SADDR 0xcc000

static MACHINE_CONFIG_FRAGMENT(el2_3c503_config)
	MCFG_DEVICE_ADD("dp8390d", DP8390D, 0)
	MCFG_DP8390D_IRQ_CB(WRITELINE(el2_3c503_device, el2_3c503_irq_w))
	MCFG_DP8390D_MEM_READ_CB(READ8(el2_3c503_device, el2_3c503_mem_read))
	MCFG_DP8390D_MEM_WRITE_CB(WRITE8(el2_3c503_device, el2_3c503_mem_write))
MACHINE_CONFIG_END

const device_type EL2_3C503 = &device_creator<el2_3c503_device>;

machine_config_constructor el2_3c503_device::device_mconfig_additions() const {
	return MACHINE_CONFIG_NAME(el2_3c503_config);
}

el2_3c503_device::el2_3c503_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, EL2_3C503, "3C503 Network Adapter", tag, owner, clock, "el2_3c503", __FILE__),
		device_isa8_card_interface(mconfig, *this),
		m_dp8390(*this, "dp8390d") {
}

void el2_3c503_device::device_start() {
	char mac[7];
	UINT32 num = rand();
	memset(m_prom, 0x57, 16);
	sprintf(mac, "\x02\x60\x8c%c%c%c", (num >> 16) & 0xff, (num >> 8) & 0xff, num & 0xff);
	memcpy(m_prom, mac, 6);
	memset(m_rom, 0, 8*1024); // empty
	m_dp8390->set_mac(mac);
	set_isa_device();
	m_isa->install_device(0x0300, 0x030f, 0, 0, read8_delegate(FUNC(el2_3c503_device::el2_3c503_loport_r), this), write8_delegate(FUNC(el2_3c503_device::el2_3c503_loport_w), this));
	m_isa->install_device(0x0700, 0x070f, 0, 0, read8_delegate(FUNC(el2_3c503_device::el2_3c503_hiport_r), this), write8_delegate(FUNC(el2_3c503_device::el2_3c503_hiport_w), this));

	// TODO: This is wrong, fix if anything actually uses it
	//  DMA can change in runtime
	int chan = 0, idcfr = m_regs.idcfr & 0x0f;
	if((m_regs.streg & 0x08)) {
		while(idcfr) {
			chan++;
			idcfr >>= 1;
		}
		m_isa->set_dma_channel(chan, this, FALSE);
	}
}

void el2_3c503_device::device_reset() {
	memcpy(m_prom, m_dp8390->get_mac(), 6);
	memset(&m_regs, 0, sizeof(m_regs));
	m_regs.bcfr = 0x80; // port 0x300
	m_regs.pcfr = 0x20; // address 0xcc000
	m_regs.ctrl = 0x0a;
	m_irq_state = CLEAR_LINE;
	m_isa->unmap_bank(SADDR, SADDR + 0x1fff, 0, 0);
	m_isa->install_bank(SADDR, SADDR + 0x1fff, 0, 0, "3c503 rom", m_rom);
}

void el2_3c503_device::set_irq(int state) {
	switch(m_regs.idcfr & 0xf0) {
	case 0x10:
		m_isa->irq2_w(state);
		break;
	case 0x20:
		m_isa->irq3_w(state);
		break;
	case 0x40:
		m_isa->irq4_w(state);
		break;
	case 0x80:
		m_isa->irq5_w(state);
		break;
	}
}

void el2_3c503_device::set_drq(int state) {
	switch(m_regs.idcfr & 0x0f) {
	case 0x01:
		m_isa->drq1_w(state);
		break;
	case 0x02:
		m_isa->drq2_w(state);
		break;
	case 0x04:
		m_isa->drq3_w(state);
		break;
	}
}

void el2_3c503_device::eop_w(int state) {
	if((m_regs.streg & 0x08) && (state == ASSERT_LINE)) {
		m_regs.streg |= 0x10;
		m_regs.streg &= ~0x08;
		if(!(m_regs.gacfr & 0x40)) set_irq(ASSERT_LINE);
	}
}

UINT8 el2_3c503_device::dack_r(int line) {
	set_drq(CLEAR_LINE);
	return el2_3c503_mem_read(m_regs.da++);
}

void el2_3c503_device::dack_w(int line, UINT8 data) {
	set_drq(CLEAR_LINE);
	el2_3c503_mem_write(m_regs.da++, data);
}

READ8_MEMBER(el2_3c503_device::el2_3c503_loport_r) {
	switch((m_regs.ctrl >> 2) & 3) {
	case 0:
		m_dp8390->dp8390_cs(CLEAR_LINE);
		return m_dp8390->dp8390_r(space, offset, mem_mask);
	case 1:
		return m_prom[offset];
	case 2:
		return m_prom[offset + 16];
	case 3:
		logerror("3c503: invalid low register read, page 3\n");
	}
	return 0;
}

WRITE8_MEMBER(el2_3c503_device::el2_3c503_loport_w) {
	switch((m_regs.ctrl >> 2) & 3) {
	case 0:
		m_dp8390->dp8390_cs(CLEAR_LINE);
		return m_dp8390->dp8390_w(space, offset, data, mem_mask);
	case 1:
	case 2:
		logerror("3c503: invalid attempt to write to prom\n");
		return;
	case 3:
		logerror("3c503: invalid low register write, page 3\n");
		return;
	}
}

READ8_MEMBER(el2_3c503_device::el2_3c503_hiport_r) {
	switch(offset) {
	case 0:
		return m_regs.pstr;
	case 1:
		return m_regs.pspr;
	case 2:
		return m_regs.dqtr;
	case 3:
		return m_regs.bcfr;
	case 4:
		return m_regs.pcfr;
	case 5:
		return m_regs.gacfr;
	case 6:
		return m_regs.ctrl;
	case 7:
		return m_regs.streg;
	case 8:
		return m_regs.idcfr;
	case 9:
		return m_regs.da >> 8;
	case 10:
		return m_regs.da & 0xff;
	case 11:
		return (m_regs.vptr >> 12) & 0xff;
	case 12:
		return (m_regs.vptr >> 4) & 0xff;
	case 13:
		return (m_regs.vptr & 0x0f) << 4;
	case 14:
		if(!(m_regs.ctrl & 0x80)) return 0xff;
		return el2_3c503_mem_read(space, m_regs.da++, mem_mask);
	case 15:
		if(!(m_regs.ctrl & 0x80)) return 0xff;
		return el2_3c503_mem_read(space, m_regs.da++, mem_mask);
	}
	return 0;
}

WRITE8_MEMBER(el2_3c503_device::el2_3c503_hiport_w) {
	switch(offset) {
	case 0:
		m_regs.pstr = data;  // pstr and pspr are supposed to be set same as 8390 pstart and pstop
		return;              // what happens if they aren't?
	case 1:
		m_regs.pspr = data;
		return;
	case 2:
		m_regs.dqtr = data;
		return;
	case 5:
		if((m_regs.gacfr & 0xf) != (data & 0xf)) {
			m_isa->unmap_bank(SADDR, SADDR + 0x1fff, 0, 0);
			switch(data & 0xf) {
			case 0:
				m_isa->install_bank(SADDR, SADDR + 0x1fff, 0, 0, "3c503 rom", m_rom);
				break;
			case 9:
				m_isa->install_bank(SADDR, SADDR + 0x1fff, 0, 0, "3c503 ram", m_board_ram);
				break;
			default:
				m_isa->install_bank(SADDR, SADDR + 0x1fff, 0, 0, "3c503 no map", m_rom);
				break;
			}
		}

		if(!(data & 0x80)) set_irq(m_irq_state);
		else set_irq(CLEAR_LINE);

		m_regs.gacfr = data;
		return;
	case 6:
		if(data & 1) {
			device_reset();
			m_regs.ctrl = 0x0b;
			return;
		}
		if((data & 0x80) != (m_regs.ctrl & 0x80)) {
			if(data & 0x80) m_regs.streg |= 0x88;
			else m_regs.streg &= ~0x88;
			m_regs.streg &= ~0x10;
		}
		m_regs.ctrl = data;
		return;
	case 8:
		// allow only one irq and drq to be set, hw may not enforce this
		switch(data & 0xf0) {
		case 0x00:
		case 0x10:
		case 0x20:
		case 0x40:
		case 0x80:
			m_regs.idcfr = (m_regs.idcfr & 0xf) | (data & 0xf0);
			break;
		default:
			logerror("3c503: trying to set multiple irqs %X\n", data);
		}
		switch(data & 0x0f) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x04:
			m_regs.idcfr = (m_regs.idcfr & 0xf0) | (data & 0xf);
			break;
		case 0x08:
			break;
		default:
			logerror("3c503: trying to set multiple drqs %X\n", data);
		}
	case 9:
		if(m_regs.ctrl & 0x80) logerror("3c503: changing dma address during dma is undefined\n");
		m_regs.da = (data << 8) | (m_regs.da & 0xff);
		return;
	case 10:
		if(m_regs.ctrl & 0x80) logerror("3c503: changing dma address during dma is undefined\n");
		m_regs.da = (m_regs.da & 0xff00) | data;
		return;
	case 11:
		// vptr requires access to system memory address bus and so isn't currently emulatable
		// it enables mmio to be set to rom on soft reset in case the machine is remote booted
		m_regs.vptr = (data << 12) | (m_regs.vptr & 0xfff);
		return;
	case 12:
		m_regs.vptr = (data << 4) | (m_regs.vptr & 0xff00f);
		return;
	case 13:
		m_regs.vptr = (data >> 4) | (m_regs.vptr & 0xffff0);
		return;
	case 14:
		if(!(m_regs.ctrl & 0x80)) return;
		el2_3c503_mem_write(space, m_regs.da++, data, mem_mask);
		return;
	case 15:
		if(!(m_regs.ctrl & 0x80)) return;
		el2_3c503_mem_write(space, m_regs.da++, data, mem_mask);
		return;
	default:
		logerror("3c503: invalid high register write %02x\n", offset);
	}
}

WRITE_LINE_MEMBER(el2_3c503_device::el2_3c503_irq_w) {
	m_irq_state = state;
	if(!(m_regs.gacfr & 0x80)) set_irq(state);
}

READ8_MEMBER(el2_3c503_device::el2_3c503_mem_read) {
	return el2_3c503_mem_read(offset);
}

WRITE8_MEMBER(el2_3c503_device::el2_3c503_mem_write) {
	el2_3c503_mem_write(offset, data);
}

UINT8 el2_3c503_device::el2_3c503_mem_read(offs_t offset) {
	if((offset < 8*1024) || (offset >= 16*1024)) return 0xff;
	return m_board_ram[offset - (8*1024)];
}

void el2_3c503_device::el2_3c503_mem_write(offs_t offset, UINT8 data) {
	if((offset < 8*1024) || (offset >= 16*1024)) return;
	m_board_ram[offset - (8*1024)] = data;
}
