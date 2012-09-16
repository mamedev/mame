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
	const char				*name;				// Name of the device (used for looking up the device)
	UINT16					select;				// State of the address line bits when addressing this device
	UINT16					address_mask;		// Bits of the address bus used to address this device
	UINT16					write_select;		// Bits set when doing write accesses to this device (ffff = write-only)
	const char				*setting;			// configuration switch that may have an effect for the presence of this device
	UINT8					set;				// bits that must be set for this switch so that this device is present
	UINT8					unset;				// bits that must be reset for this switch so that this device is present
};

#define DMUX_CONFIG(name) \
	const datamux_config(name) =

struct datamux_config 
{
	devcb_write_line				ready;
	const dmux_device_list_entry	*devlist;
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
	:	m_device(busdevice), m_config(&entry) { };

private:
	attached_device					*m_next;
	device_t						*m_device;		// the actual device
	const dmux_device_list_entry	*m_config;
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

	void clock_in(int state);

protected:
	/* Constructor */
	virtual void device_start(void);
	virtual void device_stop(void);
	virtual void device_reset(void);
	ioport_constructor device_input_ports() const;

private:
	// Ready line to the CPU
	devcb_resolved_write_line m_ready;

	/* All devices that are attached to the 8-bit bus. */
	simple_list<attached_device> m_devices;

	/* Latch which stores the first (odd) byte */
	UINT8 m_latch;

	/* Counter for the wait states. */
	int   m_waitcount;

	/* Memory expansion (internal, 16 bit). */
	UINT16 *m_ram16b;

	/* Use the memory expansion? */
	bool m_use32k;

	/* Reference to the CPU; avoid lookups. */
	device_t *m_cpu;

	/* Reference to the address space, maybe unnecessary. */
	address_space	*m_space;
};

/******************************************************************************/

#define MCFG_DMUX_ADD(_tag, _devices)			\
	MCFG_DEVICE_ADD(_tag, DATAMUX, 0) \
	MCFG_DEVICE_CONFIG( _devices )
#endif

