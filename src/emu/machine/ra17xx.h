// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************************

    Rockwell RA17xx (e.g. A1752, A1753) ROM, RAM and I/O chip

    Juergen Buchmueller <pullmoll@t-online.de>

    The device integrates a 2048 x 8 ROM, a 128 x 4 RAM and
    and 16 I/O ports at one of the port ranges 00 ... 0f,
    20 ... 2f, 40 ... 4f or 60 ... 6f.

**********************************************************************/

#ifndef __RA17XX_H__
#define __RA17XX_H__

#include "device.h"

/*************************************
 *
 *  Device configuration macros
 *
 *************************************/

/* Set the read line handler */
#define MCFG_RA17XX_READ(_devcb) \
	ra17xx_device::set_iord(*device, DEVCB_##_devcb);
/* Set the write line handler */
#define MCFG_RA17XX_WRITE(_devcb) \
	ra17xx_device::set_iowr(*device, DEVCB_##_devcb);
class ra17xx_device : public device_t
{
public:
	ra17xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ra17xx_device() {}

	DECLARE_READ8_MEMBER ( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

	template<class _Object> static devcb_base &set_iord(device_t &device, _Object object) { return downcast<ra17xx_device &>(device).m_iord.set_callback(object); }
	template<class _Object> static devcb_base &set_iowr(device_t &device, _Object object) { return downcast<ra17xx_device &>(device).m_iowr.set_callback(object); }
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	UINT8           m_line[16];   //!< input/output flip-flops for 16 I/O lines
	UINT8           m_bl;         //!< value of BL during the most recent output
	bool            m_enable;     //!< true if outputs are enabled
	devcb_read8     m_iord;       //!< input line (read, offset = line, data = 0/1)
	devcb_write8    m_iowr;       //!< output line (write, offset = line, data = 0/1)
};

extern const device_type RA17XX;

#endif /* __RA17XX_H__ */
