// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/4(A) databus multiplexer circuit
    See datamux.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __DMUX__
#define __DMUX__

#include "ti99defs.h"

extern const device_type DATAMUX;

/*
    Device that is attached to this datamux.
    The configuration setting is used for certain configurations
    where devices may only be used if others are turned off. In particular,
    if the HGSPL expansion card is used, the GROMs in the console must be
    removed.
*/
struct dmux_device_list_entry
{
	const char              *name;              // Name of the device (used for looking up the device)
	UINT16                  select;             // State of the address line bits when addressing this device
	UINT16                  address_mask;       // Bits of the address bus used to address this device
	UINT16                  write_select;       // Bits set when doing write accesses to this device (ffff = write-only)
	const char              *setting;           // configuration switch that may have an effect for the presence of this device
	UINT8                   set;                // bits that must be set for this switch so that this device is present
	UINT8                   unset;              // bits that must be reset for this switch so that this device is present
};

#define DMUX_CONFIG(name) \
	const datamux_config(name) =

struct datamux_config
{
	const dmux_device_list_entry    *devlist;
};

/*
    Device list of this datamux.
*/
class attached_device
{
	friend class simple_list<attached_device>;
	friend class ti99_datamux_device;

public:
	attached_device(device_t *busdevice, const dmux_device_list_entry &entry)
	: m_next(nullptr), m_device(busdevice), m_config(&entry) { };

private:
	attached_device                 *m_next;
	device_t                        *m_device;      // the actual device
	const dmux_device_list_entry    *m_config;
};

/*
    Main class
*/
class ti99_datamux_device : public device_t
{
public:
	ti99_datamux_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );
	DECLARE_SETOFFSET_MEMBER( setoffset );

	DECLARE_WRITE_LINE_MEMBER( clock_in );
	DECLARE_WRITE_LINE_MEMBER( dbin_in );
	DECLARE_WRITE_LINE_MEMBER( ready_line );

	template<class _Object> static devcb_base &static_set_ready_callback(device_t &device, _Object object)
	{
		return downcast<ti99_datamux_device &>(device).m_ready.set_callback(object);
	}

protected:
	/* Constructor */
	void device_start() override;
	void device_stop() override;
	void device_reset() override;
	ioport_constructor device_input_ports() const override;

private:
	// Keeps the address space pointer
	address_space* m_spacep;

	// Common read routine
	void read_all(address_space& space, UINT16 addr, UINT8 *target);

	// Common write routine
	void write_all(address_space& space, UINT16 addr, UINT8 value);

	// Common set address method
	void setaddress_all(address_space& space, UINT16 addr);

	// Debugger access
	UINT16 debugger_read(address_space& space, UINT16 addr);
	void debugger_write(address_space& space, UINT16 addr, UINT16 data);

	// Join own READY and external READY
	void ready_join();

	// Ready line to the CPU
	devcb_write_line m_ready;

	// Own ready state.
	line_state  m_muxready;

	// Ready state. Needed to control wait state generation via inbound READY
	line_state  m_sysready;

	/* Address latch (emu). In reality, the address bus remains constant. */
	UINT16 m_addr_buf;

	/* Stores the state of the DBIN line. */
	bool    m_read_mode;

	/* All devices that are attached to the 8-bit bus. */
	simple_list<attached_device> m_devices;

	/* Latch which stores the first (odd) byte */
	UINT8 m_latch;

	/* Counter for the wait states. */
	int   m_waitcount;

	/* Memory expansion (internal, 16 bit). */
	std::unique_ptr<UINT16[]> m_ram16b;

	/* Use the memory expansion? */
	bool m_use32k;

	/* Memory base for piggy-back 32K expansion. If 0, expansion is not used. */
	UINT16  m_base32k;

	/* Reference to the CPU; avoid lookups. */
	device_t *m_cpu;
};

/******************************************************************************/

#define MCFG_DMUX_ADD(_tag, _devices)           \
	MCFG_DEVICE_ADD(_tag, DATAMUX, 0) \
	MCFG_DEVICE_CONFIG( _devices )
#endif

#define MCFG_DMUX_READY_HANDLER( _intcallb ) \
	devcb = &ti99_datamux_device::static_set_ready_callback( *device, DEVCB_##_intcallb );
