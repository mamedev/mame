// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    i8243.h

    Intel 8243 Port Expander

***************************************************************************/

#ifndef MAME_MACHINE_I8243_H
#define MAME_MACHINE_I8243_H

#pragma once

class i8243_device :  public device_t
{
public:
	// construction/destruction
	i8243_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration helpers
	auto read_handler() { return m_readhandler.bind(); }
	auto write_handler() { return m_writehandler.bind(); }

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
DECLARE_DEVICE_TYPE(I8243, i8243_device)

#endif /* __I8243_H__ */
