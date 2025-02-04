// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    DMAC

    DMA controller used in Amiga systems

***************************************************************************/

#ifndef MAME_MACHINE_DMAC_H
#define MAME_MACHINE_DMAC_H

#pragma once

#include "autoconfig.h"


class amiga_dmac_device : public device_t, public amiga_autoconfig
{
public:
	amiga_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto cfgout_cb() { return m_cfgout_cb.bind(); }
	auto int_cb() { return m_int_cb.bind(); }
	auto css_read_cb() { return m_css_read_cb.bind(); }
	auto css_write_cb() { return m_css_write_cb.bind(); }
	auto csx0_read_cb() { return m_csx0_read_cb.bind(); }
	auto csx0_write_cb() { return m_csx0_write_cb.bind(); }
	auto csx1_read_cb() { return m_csx1_read_cb.bind(); }
	auto csx1_write_cb() { return m_csx1_write_cb.bind(); }
	auto sdack_read_cb() { return m_sdack_read_cb.bind(); }
	auto sdack_write_cb() { return m_sdack_write_cb.bind(); }
	auto xdack_read_cb() { return m_xdack_read_cb.bind(); }
	auto xdack_write_cb() { return m_xdack_write_cb.bind(); }

	void set_address_space(address_space *space) { m_space = space; }
	void set_rom(const char *region) { m_rom.set_tag(region); }
	void set_ram(uint16_t *ram) { m_ram = ram; }

	// input lines
	void configin_w(int state);
	void ramsz_w(int state);
	void rst_w(int state);
	void intx_w(int state);
	void sdreq_w(int state);
	void xdreq_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	void map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_dma);
	void update_interrupts();
	void start_dma();
	void stop_dma();

	uint16_t autoconfig_r(offs_t offset, uint16_t mem_mask);
	void autoconfig_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t istr_r(offs_t offset, uint16_t mem_mask);
	uint16_t cntr_r(offs_t offset, uint16_t mem_mask);
	void cntr_w(offs_t offset, uint16_t data,uint16_t mem_mask);
	void wtc_hi_w(offs_t offset, uint16_t data,uint16_t mem_mask);
	void wtc_lo_w(offs_t offset, uint16_t data,uint16_t mem_mask);
	void acr_hi_w(offs_t offset, uint16_t data,uint16_t mem_mask);
	void acr_lo_w(offs_t offset, uint16_t data,uint16_t mem_mask);
	void dawr_w(offs_t offset, uint16_t data,uint16_t mem_mask);

	uint8_t css_r(offs_t offset);
	void css_w(offs_t offset, uint8_t data);
	uint8_t csx0_r(offs_t offset);
	void csx0_w(offs_t offset, uint8_t data);
	uint8_t csx1_r(offs_t offset);
	void csx1_w(offs_t offset, uint8_t data);

	// strobe register
	uint16_t st_dma_r(offs_t offset, uint16_t mem_mask);
	void st_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t sp_dma_r(offs_t offset, uint16_t mem_mask);
	void sp_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t cint_r(offs_t offset, uint16_t mem_mask);
	void cint_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t flush_r(offs_t offset, uint16_t mem_mask);
	void flush_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// control register definitions
	static constexpr uint16_t CNTR_TCEN = 0x80;  // terminal count enable
	static constexpr uint16_t CNTR_PREST = 0x40; // peripheral reset
	static constexpr uint16_t CNTR_PDMD = 0x20;  // peripheral device mode select (1=scsi/0=xt)
	static constexpr uint16_t CNTR_INTEN = 0x10; // interrupt enable
	static constexpr uint16_t CNTR_DDIR = 0x08;  // device direction (1=from host to peripheral)

	// interrupt status register definitions
	static constexpr uint16_t ISTR_INT_F = 0x80;  // interrupt follow
	static constexpr uint16_t ISTR_INTS = 0x40;   // scsi peripheral interrupt
	static constexpr uint16_t ISTR_E_INT = 0x20;  // end-of-process interrupt
	static constexpr uint16_t ISTR_INT_P = 0x10;  // interrupt pending
	static constexpr uint16_t ISTR_UE_INT = 0x08; // fifo underrun interrupt
	static constexpr uint16_t ISTR_OE_INT = 0x04; // fifo overflow interrupt
	static constexpr uint16_t ISTR_FF_FLG = 0x02; // fifo full flag
	static constexpr uint16_t ISTR_FE_FLG = 0x01; // fifo empty flag

	static constexpr uint16_t ISTR_INT_MASK = 0x6c;

	// callbacks
	devcb_write_line m_cfgout_cb;
	devcb_write_line m_int_cb;
	devcb_read8 m_css_read_cb;
	devcb_write8 m_css_write_cb;
	devcb_read8 m_csx0_read_cb;
	devcb_write8 m_csx0_write_cb;
	devcb_read8 m_csx1_read_cb;
	devcb_write8 m_csx1_write_cb;
	devcb_read8 m_sdack_read_cb;
	devcb_write8 m_sdack_write_cb;
	devcb_read8 m_xdack_read_cb;
	devcb_write8 m_xdack_write_cb;

	optional_memory_region m_rom;
	address_space *m_space;
	uint16_t *m_ram;
	int m_ram_size;

	// internal registers
	uint16_t m_cntr; // control register
	uint16_t m_istr; // interrupt status register
	uint32_t m_wtc;  // word transfer count
	uint32_t m_acr;  // address control register

	// state of input lines
	bool m_intx;
	bool m_sdreq;
	bool m_xdreq;

	emu_timer *m_dma_timer;

	// autoconfig state
	bool m_autoconfig_dmac_done;
	bool m_autoconfig_ram_done;
	offs_t m_dmac_address;
	offs_t m_ram_address;
};

// device type definition
DECLARE_DEVICE_TYPE(AMIGA_DMAC, amiga_dmac_device)

#endif // MAME_MACHINE_DMAC_H
