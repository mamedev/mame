// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    GROM emulation
    See grom.c for documentation,

    Michael Zapf

    February 2012: Rewritten as class

***************************************************************************/
#ifndef __TI99GROM__
#define __TI99GROM__

#include "ti99defs.h"

struct ti99grom_config
{
	bool                writable;
	int                 ident;
	const char          *regionname;
	offs_t              offset_reg;
	int                 size;
	int                 clockrate;
};

#define GROM_CONFIG(name) \
	const ti99grom_config(name) =

#define MCFG_GROM_READY_CALLBACK(_write) \
	devcb = &ti99_grom_device::set_ready_wr_callback(*device, DEVCB_##_write);

extern const device_type GROM;

/*
    ti99_grom. For bus8z_device see ti99defs.h
*/
class ti99_grom_device : public bus8z_device, ti99grom_config
{
public:
	ti99_grom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_ready_wr_callback(device_t &device, _Object object) { return downcast<ti99_grom_device &>(device).m_gromready.set_callback(object); }

	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

private:
	// Is this a GRAM (never seen actually, but obviously planned)
	bool        m_writable;

	// Identification of this GROM (0-7)
	int         m_ident;

	// If the GROM has only 6 KiB, the remaining 2 KiB are filled with a
	// specific byte pattern which is created by a logical OR of lower
	// regions
	int         m_size;

	// Ready callback. This line is usually connected to the READY pin of the CPU.
	devcb_write_line   m_gromready;

	// Frequency of the incoming GROM clock. In most application cases the
	// GROM gets its clock from the video display processor (TMS9918)
	int     m_clockrate;

	/* Address pointer. */
	// This value is always expected to be in the range 0x0000 - 0xffff, even
	// when this GROM is not addressed.
	UINT16 m_address;

	/* GROM data buffer. */
	UINT8 m_buffer;

	/* Internal flip-flop. Used when retrieving the address counter. */
	bool m_raddr_LSB;

	/* Internal flip-flops. Used when writing the address counter.*/
	bool m_waddr_LSB;

	/* Pointer to the memory region contained in this GROM. */
	UINT8 *m_memptr;

	// Timer for READY line operation
	emu_timer *m_timer;

	/* Indicates whether this device will react on the next read/write data access. */
	inline int is_selected()
	{
		return (((m_address >> 13)&0x07)==m_ident);
	}

	// Calling this method causes the READY line to be cleared, which puts the
	// CPU into wait state mode. A timer is set to raise READY again.
	void clear_ready();

	virtual void device_start(void);
	virtual void device_reset(void);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};


#define MCFG_GROM_ADD(_tag, _config)    \
	MCFG_DEVICE_ADD(_tag, GROM, 0)  \
	MCFG_DEVICE_CONFIG(_config)

#endif
