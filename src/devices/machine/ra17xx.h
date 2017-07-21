// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************************

    Rockwell RA17xx (e.g. A1752, A1753) ROM, RAM and I/O chip

    Juergen Buchmueller <pullmoll@t-online.de>

    The device integrates a 2048 x 8 ROM, a 128 x 4 RAM and
    and 16 I/O ports at one of the port ranges 00 ... 0f,
    20 ... 2f, 40 ... 4f or 60 ... 6f.

**********************************************************************/

#ifndef MAME_MACHINE_RA17XX_H
#define MAME_MACHINE_RA17XX_H

#include "device.h"
#include "cpu/pps4/pps4.h"

/*************************************
 *
 *  Device configuration macros
 *
 *************************************/

// Set the read line handler
#define MCFG_RA17XX_READ(devcb) \
		ra17xx_device::set_iord(*device, DEVCB_##devcb);
// Set the write line handler
#define MCFG_RA17XX_WRITE(devcb) \
		ra17xx_device::set_iowr(*device, DEVCB_##devcb);

#define MCFG_RA17XX_CPU(tag) \
		ra17xx_device::set_cpu_tag(*device, "^" tag);

class ra17xx_device : public device_t
{
public:
	ra17xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER ( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

	template <class Object> static devcb_base &set_iord(device_t &device, Object &&cb) { return downcast<ra17xx_device &>(device).m_iord.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_iowr(device_t &device, Object &&cb) { return downcast<ra17xx_device &>(device).m_iowr.set_callback(std::forward<Object>(cb)); }
	static void set_cpu_tag(device_t &device, const char *tag) { downcast<ra17xx_device &>(device).m_cpu.set_tag(tag); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t           m_line[16];   //!< input/output flip-flops for 16 I/O lines
	uint8_t           m_bl;         //!< value of BL during the most recent output
	bool            m_enable;     //!< true if outputs are enabled
	devcb_read8     m_iord;       //!< input line (read, offset = line, data = 0/1)
	devcb_write8    m_iowr;       //!< output line (write, offset = line, data = 0/1)
	required_device<pps4_device> m_cpu;
};

DECLARE_DEVICE_TYPE(RA17XX, ra17xx_device)

#endif // MAME_MACHINE_RA17XX_H
