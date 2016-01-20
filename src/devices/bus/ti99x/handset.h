// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/4 handset
    See handset.c for documentation.

    This file also contains the implementation of the twin joystick;
    actually, no big deal, as it contains no logic but only switches.

    Michael Zapf, October 2010
    February 2012: Rewritten as class
    June 2012: Added joystick

*****************************************************************************/

#ifndef __HANDSET__
#define __HANDSET__

#include "emu.h"
#include "joyport.h"

#define MAX_HANDSETS 4

extern const device_type HANDSET;

class ti99_handset_device : public joyport_attached_device
{
public:
	ti99_handset_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	UINT8 read_dev() override;
	void  write_dev(UINT8 data) override;

	void pulse_clock() override;

protected:
	virtual void device_start(void) override;
	virtual void device_reset(void) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	void do_task();
	void post_message(int message);
	bool poll_keyboard(int num);
	bool poll_joystick(int num);
	void set_acknowledge(int data);

	int     m_ack;
	bool    m_clock_high;
	int     m_buf;
	int     m_buflen;
	UINT8   previous_joy[MAX_HANDSETS];
	UINT8   previous_key[MAX_HANDSETS];

	emu_timer *m_delay_timer;
};

#define MCFG_HANDSET_ADD(_tag, _intf, _clock )  \
	MCFG_DEVICE_ADD(_tag, HANDSET, _clock)  \
	MCFG_DEVICE_CONFIG(_intf)

#define TI99_HANDSET_INTERFACE(name)    \
	const ti99_handset_intf(name) =

/****************************************************************************/

extern const device_type TI99_JOYSTICK;

class ti99_twin_joystick : public joyport_attached_device
{
public:
	ti99_twin_joystick(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual void device_start(void) override;

	UINT8 read_dev() override;
	void  write_dev(UINT8 data) override;

protected:
	virtual ioport_constructor device_input_ports() const override;

private:
	// Which joystick is selected?
	// In reality this is no latch but GND is put on one of the selector lines
	// and then routed back to the port via the joystick
	int m_joystick;
};


#endif
