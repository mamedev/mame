// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    DMAC

    DMA controller used in Amiga systems

***************************************************************************/

#ifndef MAME_MACHINE_DMAC_H
#define MAME_MACHINE_DMAC_H

#pragma once

#include "autoconfig.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class amiga_dmac_device : public device_t, public amiga_autoconfig
{
public:
	// construction/destruction
	amiga_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto cfgout_handler() { return m_cfgout_handler.bind(); }
	auto int_handler() { return m_int_handler.bind(); }
	auto xdack_handler() { return m_xdack_handler.bind(); }
	auto scsi_read_handler() { return m_scsi_read_handler.bind(); }
	auto scsi_write_handler() { return m_scsi_write_handler.bind(); }
	auto io_read_handler() { return m_io_read_handler.bind(); }
	auto io_write_handler() { return m_io_write_handler.bind(); }

	void set_address_space(address_space *space) { m_space = space; }
	void set_rom(uint8_t *rom) { m_rom = rom; }
	void set_ram(uint8_t *ram) { m_ram = ram; }

	// input lines
	void configin_w(int state);
	void ramsz_w(int state);
	void rst_w(int state);
	void intx_w(int state);
	void xdreq_w(int state);

	// dmac register access
	uint16_t register_read(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void register_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	// control register flags
	enum
	{
		CNTR_TCEN  = 0x80,  // terminal count enable
		CNTR_PREST = 0x40,  // peripheral reset
		CNTR_PDMD  = 0x20,  // peripheral device mode select (1=scsi, 0=xt)
		CNTR_INTEN = 0x10,  // interrupt enable
		CNTR_DDIR  = 0x08   // device direction (1=rd host, wr to peripheral)
	};

	// interrupt status register
	enum
	{
		ISTR_INTX   = 0x100,    // xt interrupt pending
		ISTR_INT_F  = 0x080,    // interrupt follow
		ISTR_INTS   = 0x040,    // scsi peripheral interrupt
		ISTR_E_INT  = 0x020,    // end-of-process interrupt
		ISTR_INT_P  = 0x010,    // interrupt pending
		ISTR_UE_INT = 0x008,    // under-run fifo error interrupt
		ISTR_OE_INT = 0x004,    // over-run fifo error interrupt
		ISTR_FF_FLG = 0x002,    // fifo-full flag
		ISTR_FE_FLG = 0x001     // fifo-empty flag
	};

	static constexpr int ISTR_INT_MASK = 0x1ec;

	// callbacks
	devcb_write_line m_cfgout_handler;
	devcb_write_line m_int_handler;
	devcb_write_line m_xdack_handler;
	devcb_read8 m_scsi_read_handler;
	devcb_write8 m_scsi_write_handler;
	devcb_read8 m_io_read_handler;
	devcb_write8 m_io_write_handler;

	address_space *m_space;
	uint8_t *m_rom;
	uint8_t *m_ram;
	int m_ram_size;

	// autoconfig state
	bool m_configured;

	// state of lines
	int m_rst;

	// register
	uint16_t m_cntr;  // control register
	uint16_t m_istr;  // interrupt status register
	uint32_t m_wtc;   // word transfer count
	uint32_t m_acr;   // address control register

	bool m_dma_active;

	void check_interrupts();
	void start_dma();
	void stop_dma();
};


// device type definition
DECLARE_DEVICE_TYPE(AMIGA_DMAC, amiga_dmac_device)

#endif // MAME_MACHINE_DMAC_H
