// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_DP83932C_H
#define MAME_MACHINE_DP83932C_H

#pragma once

#include "dinetwork.h"

class dp83932c_device
	: public device_t
	, public device_network_interface
{
public:
	dp83932c_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// configuration
	template <typename T> void set_bus(T &&tag, int spacenum) { m_bus.set_tag(std::forward<T>(tag), spacenum); }
	auto out_int_cb() { return m_out_int.bind(); }

	// external interface
	void map(address_map &map) ATTR_COLD;
	u16 reg_r(offs_t offset) { return m_reg[offset]; }
	void reg_w(offs_t offset, u16 data);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_network_interface overrides
	virtual void send_complete_cb(int result) override;
	virtual int recv_start_cb(u8 *buf, int length) override;
	virtual void recv_complete_cb(int result) override;

	// command helpers
	void command(s32 param);
	void transmit();
	void read_rra(bool command = false);
	void load_cam();
	void update_interrupts();
	bool address_filter(u8 *buf);

	// diagnostic helper
	void dump_bytes(u8 *buf, int length);

	enum registers : unsigned
	{
		CR    = 0x00, // command
		DCR   = 0x01, // data configuration
		RCR   = 0x02, // receive control
		TCR   = 0x03, // transmit control
		IMR   = 0x04, // interrupt mask
		ISR   = 0x05, // interrupt status
		UTDA  = 0x06, // upper transmit descriptor address
		CTDA  = 0x07, // current transmit descriptor address
		TPS   = 0x08, // transmit packet size
		TFC   = 0x09, // transmit fragment count
		TSA0  = 0x0a, // transmit start address 0
		TSA1  = 0x0b, // transmit start address 1
		TFS   = 0x0c, // transmit fragment size
		URDA  = 0x0d, // upper receive descriptor address
		CRDA  = 0x0e, // current receive descriptor address
		CRBA0 = 0x0f, // current receive buffer address 0
		CRBA1 = 0x10, // current receive buffer address 1
		RBWC0 = 0x11, // remaining buffer word count 0
		RBWC1 = 0x12, // remaining buffer word count 1
		EOBC  = 0x13, // end of buffer word count
		URRA  = 0x14, // upper receive resource address
		RSA   = 0x15, // resource start address
		REA   = 0x16, // resource end address
		RRP   = 0x17, // resource read pointer
		RWP   = 0x18, // resource write pointer
		TRBA0 = 0x19, // temporary receive buffer address 0
		TRBA1 = 0x1a, // temporary receive buffer address 1
		TBWC0 = 0x1b, // temporary buffer word count 0
		TBWC1 = 0x1c, // temporary buffer word count 1
		ADDR0 = 0x1d, // address generator 0
		ADDR1 = 0x1e, // address generator 1
		LLFA  = 0x1f, // last link field address
		TTDA  = 0x20, // temporary transmit descriptor address
		CEP   = 0x21, // cam entry pointer
		CAP2  = 0x22, // cam address port 2
		CAP1  = 0x23, // cam address port 1
		CAP0  = 0x24, // cam address port 0
		CE    = 0x25, // cam enable
		CDP   = 0x26, // cam descriptor pointer
		CDC   = 0x27, // cam descriptor count
		SR    = 0x28, // silicon revision
		WT0   = 0x29, // watchdog timer 0
		WT1   = 0x2a, // watchdog timer 1
		RSC   = 0x2b, // receive sequence counter
		CRCT  = 0x2c, // crc error tally
		FAET  = 0x2d, // fae tally
		MPT   = 0x2e, // missed packet tally
		MDT   = 0x2f, // maximum deferral timer
					  // factory use only
		DCR2  = 0x3f, // data configuration 2
	};

	enum cr_mask : u16
	{
		CR_HTX   = 0x0001, // halt transmission
		CR_TXP   = 0x0002, // transmit packets
		CR_RXDIS = 0x0004, // receiver disable
		CR_RXEN  = 0x0008, // receiver enable
		CR_STP   = 0x0010, // stop timer
		CR_ST    = 0x0020, // start timer
		CR_RST   = 0x0080, // software reset
		CR_RRRA  = 0x0100, // read rra
		CR_LCAM  = 0x0200, // load cam
	};
	enum dcr_mask : u16
	{
		DCR_TFT   = 0x0003, // transmit fifo threshold
		DCR_RFT   = 0x000c, // receive fifo threshold
		DCR_BMS   = 0x0010, // block mode select for dma
		DCR_DW    = 0x0020, // data width select
		DCR_WC    = 0x00c0, // wait state control
		DCR_USR   = 0x0300, // user definable pins
		DCR_SBUS  = 0x0400, // synchronous bus mode
		DCR_PO    = 0x1800, // programmable outputs
		DCR_LBR   = 0x2000, // latched bus retry
		DCR_EXBUS = 0x8000, // extended bus mode
	};
	enum rcr_mask : u16
	{
		RCR_PRX   = 0x0001, // packet received ok
		RCR_LBK   = 0x0002, // loopback packet received
		RCR_FAER  = 0x0004, // frame alignment error
		RCR_CRCR  = 0x0008, // crc error
		RCR_COL   = 0x0010, // collision activity
		RCR_CRS   = 0x0020, // carrier sense activity
		RCR_LPKT  = 0x0040, // last packet in rba
		RCR_BC    = 0x0080, // broadcast packet received
		RCR_MC    = 0x0100, // multicast packet received
		RCR_LB    = 0x0600, // loopback control
		RCR_AMC   = 0x0800, // accept all multicast packets
		RCR_PRO   = 0x1000, // physical promiscuous mode
		RCR_BRD   = 0x2000, // accept broadcast packets
		RCR_RNT   = 0x4000, // accept runt packets
		RCR_ERR   = 0x8000, // accept packet with crc errors or collisions
	};
	enum tcr_mask : u16
	{
		TCR_PTX   = 0x0001, // packet transmitted ok
		TCR_BCM   = 0x0002, // byte count mismatch
		TCR_FU    = 0x0004, // fifo underrun
		TCR_PMB   = 0x0008, // packet monitored bad
		TCR_OWC   = 0x0020, // out of window collision
		TCR_EXC   = 0x0040, // excessive collisions
		TCR_CRSL  = 0x0080, // crs lost
		TCR_NCRS  = 0x0100, // no crs
		TCR_DEF   = 0x0200, // deferred transmission
		TCR_EXD   = 0x0400, // excessive deferral
		TCR_EXDIS = 0x1000, // disable excessive deferral timer
		TCR_CRCI  = 0x2000, // crc inhibit
		TCR_POWC  = 0x4000, // programmed out of window collision timer
		TCR_PINT  = 0x8000, // programmable interrupt

		TCR_TPC   = 0xf000, // tx packet descriptor config
		TCR_TPS   = 0x07ff, // tx packet descriptor status

	};
	enum imr_mask : u16
	{
		IMR_RFOEN  = 0x0001, // receive fifo overrun
		IMR_MPEN   = 0x0002, // missed packet tally counter warning
		IMR_FAEEN  = 0x0004, // frame alignment error tally counter warning
		IMR_CRCEN  = 0x0008, // crc tally counter warning
		IMR_RBAEEN = 0x0010, // receive buffer area exceeded
		IMR_RBEEN  = 0x0020, // receive buffers exhausted
		IMR_RDEEN  = 0x0040, // receive descriptors exhausted
		IMR_TCEN   = 0x0080, // general purpose timer complete
		IMR_TXEREN = 0x0100, // transmit error
		IMR_PTXEN  = 0x0200, // packet transmitted ok
		IMR_PRXEN  = 0x0400, // packet received
		IMR_PINTEN = 0x0800, // programmable interrupt
		IMR_LCDEN  = 0x1000, // load cam done
		IMR_HBLEN  = 0x2000, // heartbeat lost
		IMR_BREN   = 0x4000, // bus retry occurred
	};
	enum isr_mask : u16
	{
		ISR_RFO    = 0x0001, // receive fifo overrun
		ISR_MP     = 0x0002, // missed packet counter rollover
		ISR_FAE    = 0x0004, // frame alignment error tally counter rollover
		ISR_CRC    = 0x0008, // crc tally counter rollover
		ISR_RBAE   = 0x0010, // receive buffer area exceeded
		ISR_RBE    = 0x0020, // receive buffer exhausted
		ISR_RDE    = 0x0040, // receive descriptors exhausted
		ISR_TC     = 0x0080, // general purpose timer complete
		ISR_TXER   = 0x0100, // transmit error
		ISR_TXDN   = 0x0200, // transmission done
		ISR_PKTRX  = 0x0400, // packet received
		ISR_PINT   = 0x0800, // programmed interrupt
		ISR_LCD    = 0x1000, // load cam done
		ISR_HBL    = 0x2000, // cd heartbeat lost
		ISR_BR     = 0x4000, // bus retry occurred
	};

private:
	required_address_space m_bus;
	devcb_write_line m_out_int;

	emu_timer *m_command;

	bool m_int_state;
	u16 m_reg[64];
	u64 m_cam[16];

	// These wrappers handle 16-bit data stored on 32-bit word boundaries when in 32-bit mode
	u16 read_bus_word(offs_t address);
	void write_bus_word(offs_t address, u16 data);
};

DECLARE_DEVICE_TYPE(DP83932C, dp83932c_device)

#endif // MAME_MACHINE_DP83932C_H
