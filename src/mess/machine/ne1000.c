#include "emu.h"
#include "machine/ne1000.h"
#include "machine/devhelpr.h"

static const dp8390_interface ne1000_dp8390_interface = {
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ne1000_device, ne1000_irq_w),
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ne1000_device, ne1000_mem_read),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ne1000_device, ne1000_mem_write)
};

static MACHINE_CONFIG_FRAGMENT(ne1000_config)
	MCFG_DP8390D_ADD("dp8390d", ne1000_dp8390_interface)
MACHINE_CONFIG_END

const device_type NE1000 = &device_creator<ne1000_device>;

machine_config_constructor ne1000_device::device_mconfig_additions() const {
	return MACHINE_CONFIG_NAME(ne1000_config);
}

ne1000_device::ne1000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
        : device_t(mconfig, NE1000, "NE1000 Network Adapter", tag, owner, clock),
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
	m_isa->irq3_w(state);
}

READ8_MEMBER(ne1000_device::ne1000_mem_read) {
	if(offset < 16) return m_prom[offset];
	if((offset < (8*1024)) || (offset >= (16*1024))) {
		logerror("ne1000: invalid memory read %04X\n", offset);
		return 0xff;
	}
	return m_board_ram[offset - (8*1024)];
}

WRITE8_MEMBER(ne1000_device::ne1000_mem_write) {
	if((offset < (8*1024)) || (offset >= (16*1024))) {
		logerror("ne1000: invalid memory write %04X\n", offset);
		return;
	}
	m_board_ram[offset - (8*1024)] = data;
}
