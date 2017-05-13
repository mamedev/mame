// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    i8243.h

    Intel 8243 Port Expander

***************************************************************************/

#pragma once

#ifndef __I8243_H__
#define __I8243_H__



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_I8243_ADD(_tag, _read, _write) \
	MCFG_DEVICE_ADD(_tag, I8243, 0) \
	MCFG_I8243_READHANDLER(_read) \
	MCFG_I8243_WRITEHANDLER(_write)
#define MCFG_I8243_READHANDLER(_devcb) \
	devcb = &i8243_device::set_read_handler(*device, DEVCB_##_devcb);
#define MCFG_I8243_WRITEHANDLER(_devcb) \
	devcb = &i8243_device::set_write_handler(*device, DEVCB_##_devcb);
/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> i8243_device

class i8243_device :  public device_t
{
public:
	// construction/destruction
	i8243_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_handler(device_t &device, _Object object) { return downcast<i8243_device &>(device).m_readhandler.set_callback(object); }
	template<class _Object> static devcb_base &set_write_handler(device_t &device, _Object object) { return downcast<i8243_device &>(device).m_writehandler.set_callback(object); }

	DECLARE_READ8_MEMBER(p2_r);
	DECLARE_WRITE8_MEMBER(p2_w);

	DECLARE_WRITE_LINE_MEMBER(prog_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

private:

	uint8_t       m_p[4];             /* 4 ports' worth of data */
	uint8_t       m_p2out;            /* port 2 bits that will be returned */
	uint8_t       m_p2;               /* most recent port 2 value */
	uint8_t       m_opcode;           /* latched opcode */
	uint8_t       m_prog;             /* previous PROG state */

	devcb_read8    m_readhandler;
	devcb_write8   m_writehandler;
};


// device type definition
extern const device_type I8243;

#endif  /* __I8243_H__ */
