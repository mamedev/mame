// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Microsoft Natural Keyboard

***************************************************************************/

#ifndef __KB_MSNAT_H__
#define __KB_MSNAT_H__

#include "pc_kbdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pc_kbd_microsoft_natural_device : public device_t,
										public device_pc_kbd_interface
{
public:
	// construction/destruction
	pc_kbd_microsoft_natural_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	required_device<cpu_device> m_cpu;

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const rom_entry *device_rom_region() const override;

	virtual DECLARE_WRITE_LINE_MEMBER(clock_write) override;
	virtual DECLARE_WRITE_LINE_MEMBER(data_write) override;

	DECLARE_READ8_MEMBER(p0_read);
	DECLARE_WRITE8_MEMBER(p0_write);
	DECLARE_WRITE8_MEMBER(p1_write);
	DECLARE_WRITE8_MEMBER(p2_write);
	DECLARE_READ8_MEMBER(p3_read);
	DECLARE_WRITE8_MEMBER(p3_write);

protected:
	required_ioport m_p2_0;
	required_ioport m_p2_1;
	required_ioport m_p2_2;
	required_ioport m_p2_3;
	required_ioport m_p2_4;
	required_ioport m_p2_5;
	required_ioport m_p2_6;
	required_ioport m_p2_7;
	required_ioport m_p1_0;
	required_ioport m_p1_1;
	required_ioport m_p1_2;
	required_ioport m_p1_3;
	required_ioport m_p1_4;
	required_ioport m_p1_5;
	required_ioport m_p1_6;
	required_ioport m_p1_7;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	UINT8   m_p0;
	UINT8   m_p1;
	UINT8   m_p2;
	UINT8   m_p3;
};


// device type definition
extern const device_type PC_KBD_MICROSOFT_NATURAL;

#endif  /* __KB_MSNAT_H__ */
