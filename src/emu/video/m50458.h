/***************************************************************************

Mitsubishi M50458 OSD chip

***************************************************************************/

#pragma once

#ifndef __M50458DEV_H__
#define __M50458DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_M50458_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, M50458,_freq) \


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef enum
{
	OSD_SET_ADDRESS = 0,
	OSD_SET_DATA
} m50458_state_t;


// ======================> m50458_device

class m50458_device :	public device_t
{
public:
	// construction/destruction
	m50458_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	WRITE_LINE_MEMBER( write_bit );
	WRITE_LINE_MEMBER( set_cs_line );
	WRITE_LINE_MEMBER( set_clock_line );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();

	int m_latch;
	int	m_reset_line;
	int	m_clock_line;
	int m_current_cmd;
	int m_cmd_stream_pos;
	UINT16 m_osd_addr;

	m50458_state_t m_osd_state;
};


// device type definition
extern const device_type M50458;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
