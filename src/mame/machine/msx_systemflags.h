// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MACHINE_MSX_SYSTEMFLAGS_H
#define MAME_MACHINE_MSX_SYSTEMFLAGS_H

/*
Some MSX2+ and TurboR machines have a 'system flags' I/O port ($F4).
The value in this register is cleared on power up, but it keeps it's
value during a reset of the system.
*/

DECLARE_DEVICE_TYPE(MSX_SYSTEMFLAGS, msx_systemflags_device)


#define MCFG_MSX_SYSTEMFLAGS_ADD(_tag, _initial_value) \
	MCFG_DEVICE_ADD(_tag, MSX_SYSTEMFLAGS, 0) \
	downcast<msx_systemflags_device &>(*device).set_initial_value(_initial_value);


class msx_systemflags_device : public device_t
{
public:
	msx_systemflags_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_initial_value(uint8_t initial_value) { m_initial_value = initial_value; }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	virtual void device_start() override;

private:
	uint8_t m_initial_value;
	uint8_t m_system_flags;
};

#endif // MAME_MACHINE_MSX_SYSTEMFLAGS_H
