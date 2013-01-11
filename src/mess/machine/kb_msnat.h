/***************************************************************************

    Microsoft Natural Keyboard

***************************************************************************/

#ifndef __KB_MSNAT_H__
#define __KB_MSNAT_H__

#include "devcb.h"
#include "machine/pc_kbdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pc_kbd_microsoft_natural_device : public device_t,
										public device_pc_kbd_interface
{
public:
	// construction/destruction
	pc_kbd_microsoft_natural_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	required_device<cpu_device> m_cpu;

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual const rom_entry *device_rom_region() const;

	virtual DECLARE_WRITE_LINE_MEMBER(clock_write);
	virtual DECLARE_WRITE_LINE_MEMBER(data_write);

	DECLARE_READ8_MEMBER(p0_read);
	DECLARE_WRITE8_MEMBER(p0_write);
	DECLARE_WRITE8_MEMBER(p1_write);
	DECLARE_WRITE8_MEMBER(p2_write);
	DECLARE_READ8_MEMBER(p3_read);
	DECLARE_WRITE8_MEMBER(p3_write);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	UINT8   m_p0;
	UINT8   m_p1;
	UINT8   m_p2;
	UINT8   m_p3;
};


// device type definition
extern const device_type PC_KBD_MICROSOFT_NATURAL;

#endif  /* __KB_MSNAT_H__ */
