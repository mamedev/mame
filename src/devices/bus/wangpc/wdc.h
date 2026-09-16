// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM001 Winchester Disk Controller emulation

**********************************************************************/

#ifndef MAME_BUS_WANGPC_WDC_H
#define MAME_BUS_WANGPC_WDC_H

#pragma once

#include "wangpc.h"
#include "cpu/z80/z80.h"
#include "imagedev/harddriv.h"
#include "machine/z80ctc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_wdc_device

class wangpc_wdc_device : public device_t,
							public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_wdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_wangpcbus_card_interface overrides
	virtual uint16_t wangpcbus_mrdc_r(offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_amwc_w(offs_t offset, uint16_t mem_mask, uint16_t data) override;
	virtual uint16_t wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data) override;
	virtual uint8_t wangpcbus_dack_r(int line) override;
	virtual void wangpcbus_dack_w(int line, uint8_t data) override;
	virtual bool wangpcbus_have_dack(int line) override;
	virtual void wangpcbus_tc_w(int state) override;

private:
	inline void set_irq(int state);

	uint8_t drive_r();
	void drive_w(uint8_t data);
	uint8_t buffer_r(offs_t offset);
	void buffer_w(offs_t offset, uint8_t data);
	uint8_t cmd_r();
	uint8_t handshake_r();
	void handshake_w(uint8_t data);
	void status_w(uint8_t data);
	void response_w(uint8_t data);
	uint8_t serializer_r();
	uint8_t ctc_ch0_r();
	void ctc_ch0_w(uint8_t data);
	uint8_t ctc_ch1_r();
	void ctc_ch1_w(uint8_t data);
	uint8_t ctc_ch2_r();
	void ctc_ch2_w(uint8_t data);
	uint8_t ctc_ch3_r();
	void ctc_ch3_w(uint8_t data);

	void wangpc_wdc_io(address_map &map) ATTR_COLD;
	void wangpc_wdc_mem(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;

	uint8_t m_status;       // Z80 port 0x03 -> host +0x00
	uint8_t m_response;     // Z80 port 0x20 -> host +0x02
	uint8_t m_cmd;          // host +0x02 -> Z80 port 0x01
	uint8_t m_drive_ctrl;   // Z80 port 0x00 out: head select/step/dir
	uint8_t m_handshake;    // Z80 port 0x02 out
	uint8_t m_option;
	bool m_cmd_pending;     // set by a host command write, cleared when the Z80 reads it
	int m_irq;

	void strobe_cmd_ctc();
	void set_drq(int state);
	void update_drq();
	void sector_throttle();
	TIMER_CALLBACK_MEMBER(index_tick);
	TIMER_CALLBACK_MEMBER(transfer_done);
	void start_transfer(bool writing);

	emu_timer *m_index_timer;
	emu_timer *m_xfer_timer;
	required_device<harddisk_image_device> m_harddisk;
	uint8_t m_serializer;
	uint8_t m_dma_select;   // DMA/IRQ channel mask (+0x06 or option DREQ bits)

	bool m_dma_active;      // current DREQ level (mirror of the request latch)
	bool m_dma_enabled;     // request latch: handshake bit 5 sets, bit 7 clears
	uint16_t m_dma_count;   // bytes served since the last buffer touch
	offs_t m_dma_addr;      // DMA address counter, loaded by buffer reads
	uint8_t m_buffer[0x800]; // sector buffer at 0x2000
};


// device type definition
DECLARE_DEVICE_TYPE(WANGPC_WDC, wangpc_wdc_device)

#endif // MAME_BUS_WANGPC_WDC_H
