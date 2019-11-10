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

class msx_systemflags_device : public device_t
{
public:
	template <typename T>
	msx_systemflags_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&maincpu_tag, uint8_t initial_value)
		: msx_systemflags_device(mconfig, tag, owner, 0)
	{
		set_maincpu_tag(std::forward<T>(maincpu_tag));
		set_initial_value(initial_value);
	}

	msx_systemflags_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <typename T> void set_maincpu_tag(T &&maincpu_tag) { m_maincpu.set_tag(std::forward<T>(maincpu_tag)); }
	void set_initial_value(uint8_t initial_value) { m_initial_value = initial_value; }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	virtual void device_start() override;

private:
	uint8_t m_initial_value;
	uint8_t m_system_flags;
	required_device<cpu_device> m_maincpu;
};

#endif // MAME_MACHINE_MSX_SYSTEMFLAGS_H
