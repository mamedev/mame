// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************************

    Rockwell 10788 General Purpose Keyboard and Display circuit

    Juergen Buchmueller <pullmoll@t-online.de>

    The device decodes reads/write to a 16 byte I/O range defined
    by three wired inputs SC5, SC6 and SC7. The range is one of
    80-8f, 90-9f, ..., f0-ff depending on the wiring.

**********************************************************************/

#ifndef __R10788_H__
#define __R10788_H__

#include "device.h"

/*************************************
 *
 *  Device configuration macros
 *
 *************************************/

/* Set the writer used to update a display digit */
#define MCFG_R10788_UPDATE(_devcb) \
	r10788_device::set_update(*device, DEVCB_##_devcb);

class r10788_device : public device_t
{
public:
	r10788_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~r10788_device() {}

	enum {
		KTR = 0xc,  //!< Transfer Keyboard Return
		KTS = 0xa,  //!< Transfer Keyboard Strobe
		KLA = 0xe,  //!< Load Display Register A
		KLB = 0xd,  //!< Load Display Register B
		KDN = 0x3,  //!< Turn On Display
		KAF = 0xb,  //!< Turn Off A
		KBF = 0x7,  //!< Turn Off B
		KER = 0x6   //!< Reset Keyboard Error
	};

	DECLARE_READ8_MEMBER ( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

	template<class _Object> static devcb_base &set_update(device_t &device, _Object object) { return downcast<r10788_device &>(device).m_display.set_callback(object); }
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	static const device_timer_id TIMER_DISPLAY = 0;

	UINT8        m_reg[2][16];          //!< display registers
	UINT8        m_ktr;                 //!< transfer keyboard return value
	UINT8        m_kts;                 //!< transfer keyboard strobe value
	UINT8        m_kla;                 //!< display register A value
	UINT8        m_klb;                 //!< display register B value
	UINT8        m_mask_a;              //!< display enable bits for A
	UINT8        m_mask_b;              //!< display enable bits for B
	UINT8        m_ker;                 //!< keyboard error value
	int          m_io_counter;          //!< current I/O register index
	int          m_scan_counter;        //!< current display scan
	devcb_write8 m_display;             //!< display updater
	emu_timer*   m_timer;               //!< timer running at clock / 18 / 36
};

extern const device_type R10788;

#endif /* __R10788_H__ */
