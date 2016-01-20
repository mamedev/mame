// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont
/*********************************************************************

    applefdc.h

    Implementation of various Apple Floppy Disk Controllers, including
    the classic Apple controller and the IWM (Integrated Woz Machine)
    chip

    Nate Woods
    Raphael Nabet
    R. Belmont

*********************************************************************/

#ifndef __APPLEFDC_H__
#define __APPLEFDC_H__

#include "emu.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define APPLEFDC_PH0    0x01
#define APPLEFDC_PH1    0x02
#define APPLEFDC_PH2    0x04
#define APPLEFDC_PH3    0x08

extern const device_type APPLEFDC;
extern const device_type IWM;
extern const device_type SWIM;



/***************************************************************************
    INTERFACE
***************************************************************************/

struct applefdc_interface
{
	void (*set_lines)(device_t *device, UINT8 lines);
	void (*set_enable_lines)(device_t *device, int enable_mask);

	UINT8 (*read_data)(device_t *device);
	void (*write_data)(device_t *device, UINT8 data);
	int (*read_status)(device_t *device);
};



/***************************************************************************
    BASE DEVICE
***************************************************************************/

class applefdc_base_device : public device_t
{
public:
	// read/write handlers
	virtual UINT8 read(UINT8 offset);
	virtual void write(UINT8 offset, UINT8 data);

	// read/write handlers overloads
	UINT8 read(offs_t offset)               { return read((UINT8) offset); }
	void write(offs_t offset, UINT8 data)   { write((UINT8) offset, data); }
	DECLARE_READ8_MEMBER( read )            { return read((UINT8) offset); }
	DECLARE_WRITE8_MEMBER( write )          { write((UINT8) offset, data); }

	// accessor
	UINT8 get_lines();

protected:
	enum applefdc_t
	{
		APPLEFDC_APPLE2,    /* classic Apple II disk controller (pre-IWM) */
		APPLEFDC_IWM,       /* Integrated Woz Machine */
		APPLEFDC_SWIM       /* Sander/Woz Integrated Machine */
	};

	// constructor
	applefdc_base_device(applefdc_t fdc_type, const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// other protecteds
	virtual void iwm_modereg_w(UINT8 data);

private:
	// data that is constant for the lifetime of the emulation
	emu_timer * m_motor_timer;
	applefdc_t  m_type;

	// data that changes at emulation time
	UINT8       m_write_byte;
	UINT8       m_lines;                    /* flags from IWM_MOTOR - IWM_Q7 */
	UINT8       m_mode;                     /* 0-31; see above */
	UINT8       m_handshake_hack;           /* not sure what this is for */

	// functions
	const applefdc_interface *get_interface();
	int iwm_enable2();
	UINT8 iwm_readenable2handshake();
	UINT8 statusreg_r();
	UINT8 read_reg(int lines);
	void write_reg(UINT8 data);
	void turn_motor_onoff(bool status);
	void iwm_access(int offset);
};



/***************************************************************************
    APPLE FDC - Used on Apple II
***************************************************************************/

class applefdc_device : public applefdc_base_device
{
public:
	applefdc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};



/***************************************************************************
    IWM - Used on early Macs
***************************************************************************/

class iwm_device : public applefdc_base_device
{
public:
	iwm_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_APPLEFDC_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, APPLEFDC, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_APPLEFDC_MODIFY(_tag, _intrf) \
	MCFG_DEVICE_MODIFY(_tag)          \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_IWM_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, IWM, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_IWM_MODIFY(_tag, _intrf) \
	MCFG_DEVICE_MODIFY(_tag)          \
	MCFG_DEVICE_CONFIG(_intrf)


#endif /* __APPLEFDC_H__ */
