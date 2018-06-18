// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_MACHINE_NAMCO06_H
#define MAME_MACHINE_NAMCO06_H

#pragma once


struct namco_06xx_config
{
	const char *nmicpu;
	const char *chip0;
	const char *chip1;
	const char *chip2;
	const char *chip3;
};


#define MCFG_NAMCO_06XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NAMCO_06XX, _clock)

#define MCFG_NAMCO_06XX_MAINCPU(_tag) \
	downcast<namco_06xx_device &>(*device).set_maincpu(_tag);

#define MCFG_NAMCO_06XX_READ_0_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_read_callback<0>(DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_1_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_read_callback<1>(DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_2_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_read_callback<2>(DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_3_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_read_callback<3>(DEVCB_##_devcb);


#define MCFG_NAMCO_06XX_READ_REQUEST_0_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_read_request_callback<0>(DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_REQUEST_1_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_read_request_callback<1>(DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_REQUEST_2_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_read_request_callback<2>(DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_REQUEST_3_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_read_request_callback<3>(DEVCB_##_devcb);


#define MCFG_NAMCO_06XX_WRITE_0_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_write_callback<0>(DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_WRITE_1_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_write_callback<1>(DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_WRITE_2_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_write_callback<2>(DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_WRITE_3_CB(_devcb) \
	devcb = &downcast<namco_06xx_device &>(*device).set_write_callback<3>(DEVCB_##_devcb);


/* device get info callback */
class namco_06xx_device : public device_t
{
public:
	namco_06xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_maincpu(const char *tag) { m_nmicpu.set_tag(tag); }

	template <unsigned N, class Object> devcb_base &set_read_callback(Object &&cb) { return m_read[N].set_callback(std::forward<Object>(cb)); }

	template <unsigned N, class Object> devcb_base &set_read_request_callback(Object &&cb) { return m_readreq[N].set_callback(std::forward<Object>(cb)); }

	template <unsigned N, class Object> devcb_base &set_write_callback(Object &&cb) { return m_write[N].set_callback(std::forward<Object>(cb)); }

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
