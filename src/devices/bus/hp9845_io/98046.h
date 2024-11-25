// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98046.h

    98046 module (data communications interface)

*********************************************************************/

#ifndef MAME_BUS_HP9845_IO_98046_H
#define MAME_BUS_HP9845_IO_98046_H

#pragma once

#include "hp9845_io.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/z80sio.h"
#include "bus/rs232/rs232.h"
#include "machine/f4702.h"

class hp98046_io_card_device : public device_t, public device_hp9845_io_interface
{
public:
	// construction/destruction
	hp98046_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98046_io_card_device();

	virtual uint16_t reg_r(address_space &space, offs_t offset) override;
	virtual void reg_w(address_space &space, offs_t offset, uint16_t data) override;

	virtual bool has_dual_sc() const override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<i8048_device> m_cpu;
	required_device<z80sio_device> m_sio;
	required_device<rs232_port_device> m_rs232;
	required_device<f4702_device> m_tx_brg;
	required_device<f4702_device> m_rx_brg;
	required_ioport m_loopback_en;

	std::unique_ptr<uint8_t[]> m_ram;
	util::fifo<uint16_t , 32> m_tx_fifo;    // A1U7
	util::fifo<uint16_t , 32> m_rx_fifo;    // A1U11
	bool m_rx_fifo_out_b8;  // Bit 8 of rx FIFO output

	uint16_t m_tx_fifo_in;  // A1U1 & A1U9-8
	bool m_tx_fifo_pending; // A1U18-7
	uint8_t m_r6_r7;        // A1U2
	bool m_r6_r7_pending;   // A1U18-9
	bool m_r6_r7_select;    // A1U18-13
	bool m_rxfifo_overrun;  // A1U21-9
	bool m_rxfifo_irq;      // A1U21-6
	bool m_inten;           // A1U15-2
	bool m_enoutint;        // A1U15-6
	uint8_t m_hs_out;       // A2U4
	uint8_t m_actual_hs_out;    // A2U4 output
	bool m_sio_int;
	uint8_t m_port_2;
	bool m_loopback;
	bool m_last_rxc;
	bool m_last_txc;

	uint8_t m_rxc_sel;
	uint8_t m_txc_sel;

	void cpu_program_map(address_map &map) ATTR_COLD;
	void cpu_io_map(address_map &map) ATTR_COLD;
	uint8_t ram_r(offs_t offset);
	uint8_t cpu_r(offs_t offset);
	void cpu_w(offs_t offset, uint8_t data);
	uint8_t p1_r();
	void p2_w(uint8_t data);
	void sio_int_w(int state);
	void sio_txd_w(int state);
	void rs232_rxd_w(int state);
	void rs232_dcd_w(int state);
	void rs232_dsr_w(int state);
	void rs232_cts_w(int state);
	void rs232_rxc_w(int state);
	void rs232_txc_w(int state);
	bool rx_fifo_flag() const;
	bool tx_fifo_flag() const;
	void update_flg();
	void update_sts();
	void update_irq();
	void update_hs_out();
	void load_tx_fifo();
	void set_r6_r7_pending(bool state);
	uint8_t get_hs_input() const;
	void rxc_w(int state);
	void txc_w(int state);
	TIMER_CALLBACK_MEMBER(sync_rx_im_w);
	TIMER_CALLBACK_MEMBER(sync_tx_im_w);
};

// device type definitions
DECLARE_DEVICE_TYPE(HP98046_IO_CARD, hp98046_io_card_device)

#endif // MAME_BUS_HP9845_IO_98046_H
