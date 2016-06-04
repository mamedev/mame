// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    DMAC

    DMA controller used in Amiga systems

***************************************************************************/

#pragma once

#ifndef __DMAC_H__
#define __DMAC_H__

#include "emu.h"
#include "autoconfig.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DMAC_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, DMAC, _clock)
#define MCFG_DMAC_CFGOUT_HANDLER(_devcb) \
	devcb = &dmac_device::set_cfgout_handler(*device, DEVCB_##_devcb);

#define MCFG_DMAC_INT_HANDLER(_devcb) \
	devcb = &dmac_device::set_int_handler(*device, DEVCB_##_devcb);

#define MCFG_DMAC_XDACK_HANDLER(_devcb) \
	devcb = &dmac_device::set_xdack_handler(*device, DEVCB_##_devcb);

#define MCFG_DMAC_SCSI_READ_HANDLER(_devcb) \
	devcb = &dmac_device::set_scsi_read_handler(*device, DEVCB_##_devcb);

#define MCFG_DMAC_SCSI_WRITE_HANDLER(_devcb) \
	devcb = &dmac_device::set_scsi_write_handler(*device, DEVCB_##_devcb);

#define MCFG_DMAC_IO_READ_HANDLER(_devcb) \
	devcb = &dmac_device::set_io_read_handler(*device, DEVCB_##_devcb);

#define MCFG_DMAC_IO_WRITE_HANDLER(_devcb) \
	devcb = &dmac_device::set_io_write_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmac_device

class dmac_device : public device_t, public amiga_autoconfig
{
public:
	// construction/destruction
	dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// callbacks
	template<class _Object> static devcb_base &set_cfgout_handler(device_t &device, _Object object)
		{ return downcast<dmac_device &>(device).m_cfgout_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_int_handler(device_t &device, _Object object)
		{ return downcast<dmac_device &>(device).m_int_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_xdack_handler(device_t &device, _Object object)
		{ return downcast<dmac_device &>(device).m_xdack_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_scsi_read_handler(device_t &device, _Object object)
		{ return downcast<dmac_device &>(device).m_scsi_read_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_scsi_write_handler(device_t &device, _Object object)
		{ return downcast<dmac_device &>(device).m_scsi_write_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_io_read_handler(device_t &device, _Object object)
		{ return downcast<dmac_device &>(device).m_io_read_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_io_write_handler(device_t &device, _Object object)
		{ return downcast<dmac_device &>(device).m_io_write_handler.set_callback(object); }

	void set_address_space(address_space *space) { m_space = space; };
	void set_rom(UINT8 *rom) { m_rom = rom; };
	void set_ram(UINT8 *ram) { m_ram = ram; };

	// input lines
	DECLARE_WRITE_LINE_MEMBER( configin_w );
	DECLARE_WRITE_LINE_MEMBER( ramsz_w );
	DECLARE_WRITE_LINE_MEMBER( rst_w );
	DECLARE_WRITE_LINE_MEMBER( intx_w );
	DECLARE_WRITE_LINE_MEMBER( xdreq_w );

	// dmac register access
	DECLARE_READ16_MEMBER( register_read );
	DECLARE_WRITE16_MEMBER( register_write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

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

	static const int ISTR_INT_MASK = 0x1ec;

	// callbacks
	devcb_write_line m_cfgout_handler;
	devcb_write_line m_int_handler;
	devcb_write_line m_xdack_handler;
	devcb_read8 m_scsi_read_handler;
	devcb_write8 m_scsi_write_handler;
	devcb_read8 m_io_read_handler;
	devcb_write8 m_io_write_handler;

	address_space *m_space;
	UINT8 *m_rom;
	UINT8 *m_ram;
	int m_ram_size;

	// autoconfig state
	bool m_configured;

	// state of lines
	int m_rst;

	// register
	UINT16 m_cntr;  // control register
	UINT16 m_istr;  // interrupt status register
	UINT32 m_wtc;   // word transfer count
	UINT32 m_acr;   // address control register

	bool m_dma_active;

	void check_interrupts();
	void start_dma();
	void stop_dma();
};


// device type definition
extern const device_type DMAC;


#endif  /* __DMAC_H__ */
