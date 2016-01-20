// license:BSD-3-Clause
// copyright-holders:Carl
#include "emu.h"
#include "dp8390.h"

#define DP8390_BYTE_ORDER(w) ((m_regs.dcr & 3) == 3 ? ((data << 8) | (data >> 8)) : data)
#define LOOPBACK (!(m_regs.dcr & 8) && (m_regs.tcr & 6))

const device_type DP8390D = &device_creator<dp8390d_device>;
const device_type RTL8019A = &device_creator<rtl8019a_device>;

dp8390d_device::dp8390d_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: dp8390_device(mconfig, DP8390D, "DP8390D", tag, owner, clock, 10.0f, "dp8390d", __FILE__) {
		m_type = TYPE_DP8390D;
}

rtl8019a_device::rtl8019a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: dp8390_device(mconfig, RTL8019A, "RTL8019A", tag, owner, clock, 10.0f, "rtl8019a", __FILE__) {
		m_type = TYPE_RTL8019A;
}

dp8390_device::dp8390_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, float bandwidth, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_network_interface(mconfig, *this, bandwidth), m_type(0),
		m_irq_cb(*this),
		m_breq_cb(*this),
		m_mem_read_cb(*this),
		m_mem_write_cb(*this), m_reset(0), m_cs(false), m_rdma_active(0)
{
}

void dp8390_device::device_start() {
	m_irq_cb.resolve_safe();
	m_breq_cb.resolve_safe();
	m_mem_read_cb.resolve_safe(0);
	m_mem_write_cb.resolve_safe();
}

void dp8390_device::stop() {
	m_regs.isr = 0x80; // is this right?
	m_regs.cr |= 1;
	m_irq_cb(CLEAR_LINE);
	m_reset = 1;
}

void dp8390_device::device_reset() {
	memset(&m_regs, 0, sizeof(m_regs));
	m_regs.cr =  0x21;
	m_regs.isr = 0x80;
	m_regs.dcr = 0x04;
	memset(&m_8019regs, 0, sizeof(m_8019regs));
	m_8019regs.config1 = 0x80;
	m_8019regs.config3 = 0x01;
	m_irq_cb(CLEAR_LINE);

	m_reset = 1;
}

void dp8390_device::check_dma_complete() {
	if(m_regs.rbcr) return;
	m_regs.isr |= 0x40;
	check_irq();
	m_rdma_active = 0;
}

void dp8390_device::do_tx() {
	dynamic_buffer buf;
	int i;
	UINT32 high16 = (m_regs.dcr & 4)?m_regs.rsar<<16:0;
	if(m_reset) return;
	if(LOOPBACK) return;  // TODO: loopback
	m_regs.tsr = 0;
	if(m_regs.tbcr > 1518) logerror("dp8390: trying to send overlong frame\n");
	if(!m_regs.tbcr) { // ? Bochs says solaris actually does this
		m_regs.tsr = 1;
		m_regs.cr &= ~4;
		return;
	}

	buf.resize(m_regs.tbcr);
	for(i = 0; i < m_regs.tbcr; i++) buf[i] = m_mem_read_cb(high16 + (m_regs.tpsr << 8) + i);

	if(send(&buf[0], m_regs.tbcr)) {
		m_regs.tsr = 1;
		m_regs.isr |= 2;
	} else {
		m_regs.tsr = 8; // not quite right but there isn't a generic "tx failed"
		m_regs.isr |= 8;
	}
	m_regs.cr &= ~4;
	check_irq();
}

void dp8390_device::set_cr(UINT8 newcr) {
	int ostate = ((m_regs.cr & 3) == 2);
	m_regs.cr = newcr;
	if((newcr & 1) && (ostate == 1)) return stop();
	if((newcr & 3) == 2) {
		m_reset = 0;
		m_regs.isr &= ~0x80;
	}
	if(newcr & 0x20) m_rdma_active = 0;
	if(m_reset) return;
	if(newcr & 4) do_tx();
	if((newcr & 0x38) == 8) {
		m_rdma_active = 1;
		check_dma_complete();
	}
	if((newcr & 0x38) == 0x10) m_rdma_active = 2;
}

void dp8390_device::recv_overflow() {
	m_regs.rsr = 0x10;
	m_regs.isr |= 0x10;
	check_irq();
	m_regs.cntr2++;
	return;
}

void dp8390_device::recv(UINT8 *buf, int len) {
	int i;
	UINT16 start = (m_regs.curr << 8), offset;
	UINT32 high16;
	if(m_reset) return;
	if(m_regs.curr == m_regs.pstop) start = m_regs.pstart << 8;
	offset = start + 4;
	high16 = (m_regs.dcr & 4)?m_regs.rsar<<16:0;
	if(buf[0] & 1) {
		if(!memcmp((const char *)buf, "\xff\xff\xff\xff\xff\xff", 6)) {
			if(!(m_regs.rcr & 4)) return;
		} else return; // multicast
		m_regs.rsr = 0x20;
	} else m_regs.rsr = 0;
	len &= 0xffff;

	for(i = 0; i < len; i++) {
		m_mem_write_cb(high16 + offset, buf[i]);
		offset++;
		if(!(offset & 0xff)) {
			if((offset >> 8) == m_regs.pstop) offset = m_regs.pstart << 8;
			if((offset >> 8) == m_regs.bnry) return recv_overflow();
		}
	}
	if(len < 60) {
		// this can't pass to the next page
		for(; i < 60; i++) {
			m_mem_write_cb(high16 + offset, 0);
			offset++;
		}
		len = 60;
	}

	m_regs.rsr |= 1;
	m_regs.isr |= 1;
	m_regs.curr = (offset >> 8) + ((offset & 0xff)?1:0);
	if(m_regs.curr == m_regs.pstop) m_regs.curr = m_regs.pstart;
	len += 4;
	m_mem_write_cb((offs_t)start, m_regs.rsr);
	m_mem_write_cb((offs_t)start+1, m_regs.curr);
	m_mem_write_cb((offs_t)start+2, len & 0xff);
	m_mem_write_cb((offs_t)start+3, len >> 8);
	check_irq();
}

void dp8390_device::recv_cb(UINT8 *buf, int len) {
	if(!LOOPBACK) recv(buf, len);
}

WRITE_LINE_MEMBER(dp8390_device::dp8390_cs) {
	m_cs = state;
}

WRITE_LINE_MEMBER(dp8390_device::dp8390_reset) {
	if(!state) device_reset();
}

READ16_MEMBER(dp8390_device::dp8390_r) {
	UINT16 data;
	if(m_cs) {
		UINT32 high16 = (m_regs.dcr & 4)?m_regs.rsar<<16:0;
		if(m_regs.dcr & 1) {
			m_regs.crda &= ~1;
			data = m_mem_read_cb(high16 + m_regs.crda++);
			data |= m_mem_read_cb(high16 + m_regs.crda++) << 8;
			m_regs.rbcr -= (m_regs.rbcr < 2)?m_regs.rbcr:2;
			check_dma_complete();
			return DP8390_BYTE_ORDER(data);
		} else {
			m_regs.rbcr -= (m_regs.rbcr)?1:0;
			data = m_mem_read_cb(high16 + m_regs.crda++);
			check_dma_complete();
			return data;
		}
	}

	switch((offset & 0x0f)|(m_regs.cr & 0xc0)) {
	case 0x00:
	case 0x40:
	case 0x80:
		data = m_regs.cr;
		break;
	case 0x01:
		data = m_regs.clda & 0xff;
		break;
	case 0x02:
		data = m_regs.clda >> 8;
		break;
	case 0x03:
		data = m_regs.bnry;
		break;
	case 0x04:
		data = m_regs.tsr;
		break;
	case 0x05:
		data = m_regs.ncr;
		break;
	case 0x06:
		data = m_regs.fifo;
		break;
	case 0x07:
		data = m_regs.isr;
		break;
	case 0x08:
		data = m_regs.crda & 0xff;
		break;
	case 0x09:
		data = m_regs.crda >> 8;
		break;
	case 0x0c:
		data = m_regs.rsr;
		break;
	case 0x0d:
		data = m_regs.cntr0;
		break;
	case 0x0e:
		data = m_regs.cntr1;
		break;
	case 0x0f:
		data = m_regs.cntr2;
		break;
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
		data = m_regs.par[(offset & 0x7)-1];
		break;
	case 0x47:
		data = m_regs.curr;
		break;
	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4b:
	case 0x4c:
	case 0x4d:
	case 0x4e:
	case 0x4f:
		data = m_regs.mar[offset & 0x7];
		break;
	case 0x81:
		data = m_regs.pstart;
		break;
	case 0x82:
		data = m_regs.pstop;
		break;
	case 0x83:
		data = m_regs.rnpp;
		break;
	case 0x84:
		data = m_regs.tpsr;
		break;
	case 0x85:
		data = m_regs.lnpp;
		break;
	case 0x86:
		data = m_regs.ac >> 8;
		break;
	case 0x87:
		data = m_regs.ac & 0xff;
		break;
	case 0x8c:
		data = m_regs.rcr;
		break;
	case 0x8d:
		data = m_regs.tcr;
		break;
	case 0x8e:
		data = m_regs.dcr;
		break;
	case 0x8f:
		data = m_regs.imr;
		break;
	case 0xc0:
		data = m_regs.cr;
		break;
	default:
		if(m_type == TYPE_RTL8019A) {
			switch((offset & 0x0f)|(m_regs.cr & 0xc0)) {
				case 0x0a:
					data = 'P';
					break;
				case 0x0b:
					data = 'p';
					break;

				case 0xc1:
					data = m_8019regs.cr9346;
					break;
				case 0xc2:
					data = m_8019regs.bpage;
					break;
				case 0xc3:
					data = m_8019regs.config0;
					break;
				case 0xc4:
					data = m_8019regs.config1;
					break;
				case 0xc5:
					data = m_8019regs.config2;
					break;
				case 0xc6:
					data = m_8019regs.config3;
					break;
				case 0xcd:
					data = m_8019regs.config4;
					break;
				case 0xc8:
					data = m_8019regs.csnsav;
					break;
				case 0xcb:
					data = m_8019regs.intr;
					break;
				default:
					logerror("rtl8019: invalid read page %01X reg %02X\n", (m_regs.cr & 0xc0) >> 6, offset & 0x0f);
					return 0;
			}
		} else {
			logerror("dp8390: invalid read page %01X reg %02X\n", (m_regs.cr & 0xc0) >> 6, offset & 0x0f);
			return 0;
		}
	}
	return data;
}

WRITE16_MEMBER(dp8390_device::dp8390_w) {
	if(m_cs) {
		UINT32 high16 = (m_regs.dcr & 4)?m_regs.rsar<<16:0;
		if(m_regs.dcr & 1) {
			data = DP8390_BYTE_ORDER(data);
			m_regs.crda &= ~1;
			m_mem_write_cb(high16 + m_regs.crda++, data & 0xff);
			m_mem_write_cb(high16 + m_regs.crda++, data >> 8);
			m_regs.rbcr -= (m_regs.rbcr < 2)?m_regs.rbcr:2;
			check_dma_complete();
		} else {
			data &= 0xff;
			m_mem_write_cb(high16 + m_regs.crda++, data);
			m_regs.rbcr -= (m_regs.rbcr)?1:0;
			check_dma_complete();
		}
		return;
	}

	data &= 0xff;
	switch((offset & 0x0f)|(m_regs.cr & 0xc0)) {
	case 0x00:
	case 0x40:
	case 0x80:
		set_cr(data);
		break;
	case 0x01:
		m_regs.pstart = data;
		break;
	case 0x02:
		m_regs.pstop = data;
		break;
	case 0x03:
		m_regs.bnry = data;
		break;
	case 0x04:
		m_regs.tpsr = data;
		break;
	case 0x05:
		m_regs.tbcr = (m_regs.tbcr & 0xff00) | data;
		break;
	case 0x06:
		m_regs.tbcr = (m_regs.tbcr & 0xff) | (data << 8);
		break;
	case 0x07:
		m_regs.isr &= ~data;
		check_irq();
		break;
	case 0x08:
		m_regs.rsar = (m_regs.rsar & 0xff00) | data;
		m_regs.crda = m_regs.rsar;
		break;
	case 0x09:
		m_regs.rsar = (m_regs.rsar & 0xff) | (data << 8);
		m_regs.crda = m_regs.rsar;
		break;
	case 0x0a:
		m_regs.rbcr = (m_regs.rbcr & 0xff00) | data;
		break;
	case 0x0b:
		m_regs.rbcr = (m_regs.rbcr & 0xff) | (data << 8);
		break;
	case 0x0c:
		m_regs.rcr = data;
		set_promisc((data & 0x10)?true:false);
		break;
	case 0x0d:
		m_regs.tcr = data;
		break;
	case 0x0e:
		m_regs.dcr = data;
		break;
	case 0x0f:
		m_regs.imr = data;
		check_irq();
		break;
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
		m_regs.par[(offset & 0x7)-1] = data;
		set_mac((const char *)m_regs.par);
		break;
	case 0x47:
		m_regs.curr = data;
		break;
	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4b:
	case 0x4c:
	case 0x4d:
	case 0x4e:
	case 0x4f:
		m_regs.mar[offset & 0x7] = data;
		break;
	case 0x81:
		m_regs.clda = (m_regs.clda & 0xff00) | data;
		break;
	case 0x82:
		m_regs.clda = (m_regs.clda & 0xff) | (data << 8);
		break;
	case 0x83:
		m_regs.rnpp = data;
		break;
	case 0x85:
		m_regs.lnpp = data;
		break;
	case 0x86:
		m_regs.ac = (m_regs.ac & 0xff) | (data << 8);
		break;
	case 0x87:
		m_regs.ac = (m_regs.ac & 0xff00) | data;
		break;
	case 0xc0:
		set_cr(data);
		break;
	default:
		if(m_type == TYPE_RTL8019A) {
			switch((offset & 0x0f)|(m_regs.cr & 0xc0)) {
				// XXX: rest of the regs
				default:
					logerror("rtl8019: invalid write page %01X reg %02X data %04X\n", (m_regs.cr & 0xc0) >> 6, offset & 0x0f, data);
					return;
			}
		} else {
			logerror("dp8390: invalid write page %01X reg %02X data %04X\n", (m_regs.cr & 0xc0) >> 6, offset & 0x0f, data);
			return;
		}
	}
}
