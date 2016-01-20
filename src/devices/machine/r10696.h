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

#ifndef __R10696_H__
#define __R10696_H__

#include "device.h"

/*************************************
 *
 *  Device configuration macros
 *
 *************************************/

/* Set the read and write group (4-bit; nibble) delegates */
#define MCFG_R10696_IO(_devcb_rd,_devcb_wr) \
	r10696_device::set_iord(*device, DEVCB_##_devcb_rd); \
	r10696_device::set_iowr(*device, DEVCB_##_devcb_wr);

class r10696_device : public device_t
{
public:
	r10696_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~r10696_device() {}

	DECLARE_READ8_MEMBER ( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

	template<class _Object> static devcb_base &set_iord(device_t &device, _Object object) { return downcast<r10696_device &>(device).m_iord.set_callback(object); }
	template<class _Object> static devcb_base &set_iowr(device_t &device, _Object object) { return downcast<r10696_device &>(device).m_iowr.set_callback(object); }
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT8         m_io_a;   //!< input/output flip-flops group A
	UINT8         m_io_b;   //!< input/output flip-flops group B
	UINT8         m_io_c;   //!< input/output flip-flops group C
	devcb_read8   m_iord;   //!< input line (read, offset = group, data = 4 bits)
	devcb_write8  m_iowr;   //!< output line (write, offset = group, data = 4 bits)
};

extern const device_type R10696;

#endif /* __R10696_H__ */
