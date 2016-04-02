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
#include "machine/tmc0430.h"
#include "gromport.h"
#include "bus/ti99_peb/peribox.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "bus/ti99x/videowrp.h"

extern const device_type DATAMUX;

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

	DECLARE_WRITE_LINE_MEMBER( gromclk_in );

	template<class _Object> static devcb_base &static_set_ready_callback(device_t &device, _Object object)
	{
		return downcast<ti99_datamux_device &>(device).m_ready.set_callback(object);
	}

protected:
	/* Constructor */
	void device_start() override;
	void device_stop() override;
	void device_reset() override;
	void device_config_complete() override;
	ioport_constructor device_input_ports() const override;

private:
	// Link to the video processor
	bus8z_device* m_video;

	// Link to the sound processor
	sn76496_base_device* m_sound;

	// Link to the peripheral expansion box
	peribox_device* m_peb;

	// Link to the cartridge port (aka GROM port)
	gromport_device* m_gromport;

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

	// Address latch (emu). In reality, the address bus remains constant.
	UINT16 m_addr_buf;

	// DBIN line
	line_state m_dbin;

	// Own ready state.
	line_state  m_muxready;

	// Ready state. Needed to control wait state generation via inbound READY
	line_state  m_sysready;

	/* Latch which stores the first (odd) byte */
	UINT8 m_latch;

	/* Counter for the wait states. */
	int   m_waitcount;

	/* Memory expansion (internal, 16 bit). */
	std::unique_ptr<UINT16[]> m_ram16b;

	// Console RAM
	std::unique_ptr<UINT16[]> m_padram;

	// Console ROM
	UINT16* m_consolerom;

	// Console GROMs
	tmc0430_device* m_grom[3];

	/* Use the memory expansion? */
	bool m_use32k;

	/* Memory base for piggy-back 32K expansion. If 0, expansion is not used. */
	UINT16  m_base32k;

	// Console GROMs are available (the HSGPL expects them to be removed)
	bool m_console_groms_present;
};

/******************************************************************************/

#define MCFG_DMUX_READY_HANDLER( _intcallb ) \
	devcb = &ti99_datamux_device::static_set_ready_callback( *device, DEVCB_##_intcallb );

#endif
