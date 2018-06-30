// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************************

    Rockwell 10696 General Purpose Input/Output (I/O)

    Juergen Buchmueller <pullmoll@t-online.de>

    The device decodes reads/write to a 16 byte I/O range defined
    by four wired inputs SC1, SC2, SC3 and SC4.
    It provides 12 inputs and 12 outputs in groups of three
    time 4 bits each.

**********************************************************************/

#ifndef MAME_MACHINE_R10696_H
#define MAME_MACHINE_R10696_H

#pragma once

#include "device.h"

/*************************************
 *
 *  Device configuration macros
 *
 *************************************/

/* Set the read and write group (4-bit; nibble) delegates */
#define MCFG_R10696_IO(_devcb_rd,_devcb_wr) \
	downcast<r10696_device &>(*device).set_iord(DEVCB_##_devcb_rd); \
	downcast<r10696_device &>(*device).set_iowr(DEVCB_##_devcb_wr);

class r10696_device : public device_t
{
public:
	r10696_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER ( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

	template <class Object> devcb_base &set_iord(Object &&cb) { return m_iord.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_iowr(Object &&cb) { return m_iowr.set_callback(std::forward<Object>(cb)); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t         m_io_a;   //!< input/output flip-flops group A
	uint8_t         m_io_b;   //!< input/output flip-flops group B
	uint8_t         m_io_c;   //!< input/output flip-flops group C
	devcb_read8   m_iord;   //!< input line (read, offset = group, data = 4 bits)
	devcb_write8  m_iowr;   //!< output line (write, offset = group, data = 4 bits)
};

DECLARE_DEVICE_TYPE(R10696, r10696_device)

#endif // MAME_MACHINE_R10696_H
