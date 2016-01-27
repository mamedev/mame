// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    corvfdc02.h

    Implemention of the Corvus Systems CORVUS02 floppy controller

*********************************************************************/

#ifndef __A2BUS_CORVFDC02__
#define __A2BUS_CORVFDC02__

#include "emu.h"
#include "a2bus.h"
#include "machine/upd765.h"
#include "formats/imd_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_corvfdc02_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_corvfdc02_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a2bus_corvfdc02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	DECLARE_WRITE_LINE_MEMBER(intrq_w);
	DECLARE_WRITE_LINE_MEMBER(drq_w);

	DECLARE_FLOPPY_FORMATS(corv_floppy_formats);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset) override;

	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_con1;
	required_device<floppy_connector> m_con2;
	required_device<floppy_connector> m_con3;
	required_device<floppy_connector> m_con4;

private:
	UINT8 *m_rom;
	UINT8 m_fdc_local_status, m_fdc_local_command;
	UINT16 m_bufptr;
	UINT8 m_buffer[2048];   // 1x6116 SRAM
	floppy_image_device *m_curfloppy;
	bool m_in_drq;
	emu_timer *m_timer;
};

// device type definition
extern const device_type A2BUS_CORVFDC02;

#endif /* __A2BUS_CORVFDC02__ */
