// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SGI_CTL1_H
#define MAME_SGI_CTL1_H

#pragma once

class sgi_ctl1_device
	: public device_t
{
public:
	sgi_ctl1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_bus(T &&tag, int spacenum) { m_bus.set_tag(std::forward<T>(tag), spacenum); }

	auto serdin() { return m_serdin.bind(); }
	auto serdout() { return m_serdout.bind(); }
	auto cpuberr() { return m_cpuberr.bind(); }

	u8 clrerr_r(offs_t offset);
	u16 cpucfg_r() { return m_cpucfg; }
	u32 erradr_r() { m_cpuberr(0); return m_erradr; }
	u8 memcfg_r() { return m_memcfg.value_or(0); }
	u8 parerr_r() { return m_parerr; }
	u32 refadr_r();

	void clrerr_w(offs_t offset, u8 data);
	void cpucfg_w(u16 data);
	void memcfg_w(u8 data);
	void refadr_w(u32 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void parity_r(offs_t offset, u32 &data, u32 mem_mask);
	void parity_w(offs_t offset, u32 &data, u32 mem_mask);

	required_address_space m_bus;
	required_ioport m_simms;

	// i/o lines
	devcb_read_line m_serdin;
	devcb_write_line m_serdout;
	devcb_write_line m_cpuberr;

	// registers
	u16 m_cpucfg;
	std::optional<u8> m_memcfg;
	u32 m_erradr;
	u8 m_parerr;
	u32 m_refadr;

	// other state
	std::unique_ptr<u8[]> m_ram[16];
	std::unique_ptr<u8[]> m_parity;
	memory_passthrough_handler m_parity_mph;
	u32 m_parity_bad;
	attotime m_refresh_timer;
};

DECLARE_DEVICE_TYPE(SGI_CTL1, sgi_ctl1_device)

#endif // MAME_SGI_CTL1_H
