// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    HDC9234 Hard and Floppy Disk Controller
    For details see hdc9234.c
*/
#ifndef __HDC9234_H__
#define __HDC9234_H__

#include "emu.h"

extern const device_type HDC9234;

//===================================================================

/* Interrupt line. To be connected with the controller PCB. */
#define MCFG_HDC9234_INTRQ_CALLBACK(_write) \
	devcb = &hdc9234_device::set_intrq_wr_callback(*device, DEVCB_##_write);

/* DMA in progress line. To be connected with the controller PCB. */
#define MCFG_HDC9234_DIP_CALLBACK(_write) \
	devcb = &hdc9234_device::set_dip_wr_callback(*device, DEVCB_##_write);

/* Auxiliary Bus. These 8 lines need to be connected to external latches
   and to a counter circuitry which works together with the external RAM.
   We use the S0/S1 lines as address lines. */
#define MCFG_HDC9234_AUXBUS_OUT_CALLBACK(_write) \
	devcb = &hdc9234_device::set_auxbus_wr_callback(*device, DEVCB_##_write);

/* Auxiliary Bus. This is only used for S0=S1=0. */
#define MCFG_HDC9234_AUXBUS_IN_CALLBACK(_read) \
	devcb = &hdc9234_device::set_auxbus_rd_callback(*device, DEVCB_##_read);

/* Callback to read the contents of the external RAM via the data bus.
   Note that the address must be set and automatically increased
   by external circuitry. */
#define MCFG_HDC9234_DMA_IN_CALLBACK(_read) \
	devcb = &hdc9234_device::set_dma_rd_callback(*device, DEVCB_##_read);

/* Callback to write the contents of the external RAM via the data bus.
   Note that the address must be set and automatically increased
   by external circuitry. */
#define MCFG_HDC9234_DMA_OUT_CALLBACK(_write) \
	devcb = &hdc9234_device::set_dma_wr_callback(*device, DEVCB_##_write);

//===================================================================

class hdc9234_device : public device_t
{
public:
	hdc9234_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// Accesors from the CPU side
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	// Callbacks
	template<class _Object> static devcb_base &set_intrq_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_intrq.set_callback(object); }
	template<class _Object> static devcb_base &set_dip_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_dip.set_callback(object); }
	template<class _Object> static devcb_base &set_auxbus_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_auxbus.set_callback(object); }
	template<class _Object> static devcb_base &set_auxbus_rd_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_in_auxbus.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_rd_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_in_dma.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_dma.set_callback(object); }

protected:
	void device_start();
	void device_reset();

private:
	devcb_write_line   m_out_intrq;    // INT line
	devcb_write_line   m_out_dip;      // DMA in progress line
	devcb_write8       m_out_auxbus;   // AB0-7 lines (using S0,S1 as address)
	devcb_read8        m_in_auxbus;    // AB0-7 lines (S0=S1=0)
	devcb_read8        m_in_dma;       // DMA read access to the cache buffer
	devcb_write8       m_out_dma;      // DMA write access to the cache buffer
};

#endif
