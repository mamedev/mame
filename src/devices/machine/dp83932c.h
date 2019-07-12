// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_DP83932C_H
#define MAME_MACHINE_DP83932C_H

#pragma once

#include "machine/ram.h"

class dp83932c_device
	: public device_t
	, public device_network_interface
{
public:
	dp83932c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	static constexpr feature_type imperfect_features() { return feature::LAN; }

	// configuration
	template <typename T> void set_ram(T &&tag) { m_ram.set_tag(std::forward<T>(tag)); }
	auto out_int_cb() { return m_out_int.bind(); }

	void map(address_map &map);

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_network_interface overrides
	virtual void send_complete_cb(int result) override;
	virtual int recv_start_cb(u8 *buf, int length) override;
	virtual void recv_complete_cb(int result) override;

	enum cr_mask : u16
	{
		HTX   = 0x0001, // halt transmission
		TXP   = 0x0002, // transmit packets
		RXDIS = 0x0004, // receiver disable
		RXEN  = 0x0008, // receiver enable
		STP   = 0x0010, // stop timer
		ST    = 0x0020, // start timer
		RST   = 0x0080, // software reset
		RRRA  = 0x0100, // read rra
		LCAM  = 0x0200, // load cam

		CR_WMASK = 0x03bf
	};

	enum tcr_mask : u16
	{
		PTX   = 0x0001, // packet transmitted ok
		BCM   = 0x0002, // byte count mismatch
		FU    = 0x0004, // fifo underrun
		PMB   = 0x0008, // packet monitored bad
		OWC   = 0x0020, // out of window collision
		EXC   = 0x0040, // excessive collisions
		CRSL  = 0x0080, // crs lost
		NCRS  = 0x0100, // no crs
		DEF   = 0x0200, // deferred transmission
		EXD   = 0x0400, // excessive deferral
		EXDIS = 0x1000, // disable excessive deferral timer
		CRCI  = 0x2000, // crc inhibit
		POWC  = 0x4000, // programmed out of window collision timer
		PINT  = 0x8000, // programmable interrupt

		TCR_WMASK = 0xf000
	};

	u16 cr_r() { return m_cr; }
	u16 tcr_r() { return m_tcr; }
	u16 utda_r() { return m_utda; }
	u16 crda_r() { return m_crda; }
	u16 rrp_r() { return m_rrp; }
	u16 rwp_r() { return m_rwp; }
	u16 isr_r() { return m_isr; }
	u16 ce_r() { return m_ce; }
	u16 wt0_r() { return m_wt0; }
	u16 wt1_r() { return m_wt1; }
	u16 rsc_r() { return m_rsc; }
	u16 faet_r() { return m_faet; }

	void cr_w(u16 data) { m_cr = (data & CR_WMASK) | (m_cr & ~CR_WMASK); }
	void tcr_w(u16 data) { m_tcr = (data & TCR_WMASK) | (m_tcr & ~TCR_WMASK); }
	void utda_w(u16 data) { m_utda = data; }
	void crda_w(u16 data) { m_crda = data; }
	void rrp_w(u16 data) { m_rrp = data; }
	void rwp_w(u16 data) { m_rwp = data; }
	void isr_w(u16 data) { m_isr = data; }
	void ce_w(u16 data) { m_ce = data; }
	void wt0_w(u16 data) { m_wt0 = data; }
	void wt1_w(u16 data) { m_wt1 = data; }
	void rsc_w(u16 data) { m_rsc = data; }
	void faet_w(u16 data) { m_faet = ~data; }

private:
	required_device<ram_device> m_ram;

	devcb_write_line m_out_int;

	u16 m_cr;
	u16 m_tcr;
	u16 m_utda;
	u16 m_crda;
	u16 m_rrp;
	u16 m_rwp;
	u16 m_isr;
	u16 m_ce;
	u16 m_wt0;
	u16 m_wt1;
	u16 m_rsc;
	u16 m_faet;
};

DECLARE_DEVICE_TYPE(DP83932C, dp83932c_device)

#endif // MAME_MACHINE_DP83932C_H
