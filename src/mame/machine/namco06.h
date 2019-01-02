// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_MACHINE_NAMCO06_H
#define MAME_MACHINE_NAMCO06_H

#pragma once


/* device get info callback */
class namco_06xx_device : public device_t
{
public:
	namco_06xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_maincpu(T &&tag) { m_nmicpu.set_tag(std::forward<T>(tag)); }

	template <unsigned N> auto read_callback() { return m_read[N].bind(); }

	template <unsigned N> auto read_request_callback() { return m_readreq[N].bind(); }

	template <unsigned N> auto write_callback() { return m_write[N].bind(); }

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( ctrl_r );
	DECLARE_WRITE8_MEMBER( ctrl_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:

	TIMER_CALLBACK_MEMBER( nmi_generate );

	// internal state
	uint8_t m_control;
	emu_timer *m_nmi_timer;

	required_device<cpu_device> m_nmicpu;

	devcb_read8 m_read[4];

	devcb_write_line m_readreq[4];

	devcb_write8 m_write[4];
};

DECLARE_DEVICE_TYPE(NAMCO_06XX, namco_06xx_device)


#endif // MAME_MACHINE_NAMCO06_H
