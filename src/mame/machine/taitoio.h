// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    taitoio.h

    Implementation of various Taito custom I/O ICs

**************************************************************************/

#ifndef MAME_MACHINE_TAITOIO_H
#define MAME_MACHINE_TAITOIO_H

#pragma once

#include "machine/watchdog.h"


class tc0040ioc_device : public device_t
{
public:
	tc0040ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_read_0_callback(Object &&cb) { return m_read_0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_1_callback(Object &&cb) { return m_read_1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_2_callback(Object &&cb) { return m_read_2_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_3_callback(Object &&cb) { return m_read_3_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_4_callback(Object &&cb) { return m_write_4_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_7_callback(Object &&cb) { return m_read_7_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( watchdog_r );
	DECLARE_READ8_MEMBER( port_r );
	DECLARE_WRITE8_MEMBER( port_w );
	DECLARE_READ8_MEMBER( portreg_r );
	DECLARE_WRITE8_MEMBER( portreg_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	// internal state
	uint8_t      m_regs[8];
	uint8_t      m_port;

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8 m_read_0_cb;
	devcb_read8 m_read_1_cb;
	devcb_read8 m_read_2_cb;
	devcb_read8 m_read_3_cb;
	devcb_write8 m_write_4_cb;
	devcb_read8 m_read_7_cb;
};

DECLARE_DEVICE_TYPE(TC0040IOC, tc0040ioc_device)

class tc0220ioc_device : public device_t
{
public:
	tc0220ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_read_0_callback(Object &&cb) { return m_read_0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_1_callback(Object &&cb) { return m_read_1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_2_callback(Object &&cb) { return m_read_2_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_3_callback(Object &&cb) { return m_read_3_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_3_callback(Object &&cb) { return m_write_3_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_4_callback(Object &&cb) { return m_write_4_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_7_callback(Object &&cb) { return m_read_7_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	// internal state
	uint8_t      m_regs[8];

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8 m_read_0_cb;
	devcb_read8 m_read_1_cb;
	devcb_read8 m_read_2_cb;
	devcb_read8 m_read_3_cb;
	devcb_write8 m_write_3_cb;
	devcb_write8 m_write_4_cb;
	devcb_read8 m_read_7_cb;
};

DECLARE_DEVICE_TYPE(TC0220IOC, tc0220ioc_device)

class tc0510nio_device : public device_t
{
public:
	tc0510nio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_read_0_callback(Object &&cb) { return m_read_0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_1_callback(Object &&cb) { return m_read_1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_2_callback(Object &&cb) { return m_read_2_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_3_callback(Object &&cb) { return m_read_3_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_3_callback(Object &&cb) { return m_write_3_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_4_callback(Object &&cb) { return m_write_4_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_7_callback(Object &&cb) { return m_read_7_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ16_MEMBER( halfword_r );
	DECLARE_WRITE16_MEMBER( halfword_w );
	DECLARE_READ16_MEMBER( halfword_wordswap_r );
	DECLARE_WRITE16_MEMBER( halfword_wordswap_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	// internal state
	uint8_t   m_regs[8];

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8 m_read_0_cb;
	devcb_read8 m_read_1_cb;
	devcb_read8 m_read_2_cb;
	devcb_read8 m_read_3_cb;
	devcb_write8 m_write_3_cb;
	devcb_write8 m_write_4_cb;
	devcb_read8 m_read_7_cb;
};

DECLARE_DEVICE_TYPE(TC0510NIO, tc0510nio_device)

class tc0640fio_device : public device_t
{
public:
	tc0640fio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_read_0_callback(Object &&cb) { return m_read_0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_1_callback(Object &&cb) { return m_read_1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_2_callback(Object &&cb) { return m_read_2_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_3_callback(Object &&cb) { return m_read_3_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_4_callback(Object &&cb) { return m_write_4_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_7_callback(Object &&cb) { return m_read_7_cb.set_callback(std::forward<Object>(cb)); }


	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ16_MEMBER( halfword_r );
	DECLARE_WRITE16_MEMBER( halfword_w );
	DECLARE_READ16_MEMBER( halfword_byteswap_r );
	DECLARE_WRITE16_MEMBER( halfword_byteswap_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	private:
	// internal state
	uint8_t   m_regs[8];

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8 m_read_0_cb;
	devcb_read8 m_read_1_cb;
	devcb_read8 m_read_2_cb;
	devcb_read8 m_read_3_cb;
	devcb_write8 m_write_4_cb;
	devcb_read8 m_read_7_cb;
};

DECLARE_DEVICE_TYPE(TC0640FIO, tc0640fio_device)


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TC0040IOC_READ_0_CB(_devcb) \
	devcb = &downcast<tc0040ioc_device &>(*device).set_read_0_callback(DEVCB_##_devcb);

#define MCFG_TC0040IOC_READ_1_CB(_devcb) \
	devcb = &downcast<tc0040ioc_device &>(*device).set_read_1_callback(DEVCB_##_devcb);

#define MCFG_TC0040IOC_READ_2_CB(_devcb) \
	devcb = &downcast<tc0040ioc_device &>(*device).set_read_2_callback(DEVCB_##_devcb);

#define MCFG_TC0040IOC_READ_3_CB(_devcb) \
	devcb = &downcast<tc0040ioc_device &>(*device).set_read_3_callback(DEVCB_##_devcb);

#define MCFG_TC0040IOC_WRITE_4_CB(_devcb) \
	devcb = &downcast<tc0040ioc_device &>(*device).set_write_4_callback(DEVCB_##_devcb);

#define MCFG_TC0040IOC_READ_7_CB(_devcb) \
	devcb = &downcast<tc0040ioc_device &>(*device).set_read_7_callback(DEVCB_##_devcb);


#define MCFG_TC0220IOC_READ_0_CB(_devcb) \
	devcb = &downcast<tc0220ioc_device &>(*device).set_read_0_callback(DEVCB_##_devcb);

#define MCFG_TC0220IOC_READ_1_CB(_devcb) \
	devcb = &downcast<tc0220ioc_device &>(*device).set_read_1_callback(DEVCB_##_devcb);

#define MCFG_TC0220IOC_READ_2_CB(_devcb) \
	devcb = &downcast<tc0220ioc_device &>(*device).set_read_2_callback(DEVCB_##_devcb);

#define MCFG_TC0220IOC_READ_3_CB(_devcb) \
	devcb = &downcast<tc0220ioc_device &>(*device).set_read_3_callback(DEVCB_##_devcb);

#define MCFG_TC0220IOC_WRITE_3_CB(_devcb) \
	devcb = &downcast<tc0220ioc_device &>(*device).set_write_3_callback(DEVCB_##_devcb);

#define MCFG_TC0220IOC_WRITE_4_CB(_devcb) \
	devcb = &downcast<tc0220ioc_device &>(*device).set_write_4_callback(DEVCB_##_devcb);

#define MCFG_TC0220IOC_READ_7_CB(_devcb) \
	devcb = &downcast<tc0220ioc_device &>(*device).set_read_7_callback(DEVCB_##_devcb);


#define MCFG_TC0510NIO_READ_0_CB(_devcb) \
	devcb = &downcast<tc0510nio_device &>(*device).set_read_0_callback(DEVCB_##_devcb);

#define MCFG_TC0510NIO_READ_1_CB(_devcb) \
	devcb = &downcast<tc0510nio_device &>(*device).set_read_1_callback(DEVCB_##_devcb);

#define MCFG_TC0510NIO_READ_2_CB(_devcb) \
	devcb = &downcast<tc0510nio_device &>(*device).set_read_2_callback(DEVCB_##_devcb);

#define MCFG_TC0510NIO_READ_3_CB(_devcb) \
	devcb = &downcast<tc0510nio_device &>(*device).set_read_3_callback(DEVCB_##_devcb);

#define MCFG_TC0510NIO_WRITE_3_CB(_devcb) \
	devcb = &downcast<tc0510nio_device &>(*device).set_write_3_callback(DEVCB_##_devcb);

#define MCFG_TC0510NIO_WRITE_4_CB(_devcb) \
	devcb = &downcast<tc0510nio_device &>(*device).set_write_4_callback(DEVCB_##_devcb);

#define MCFG_TC0510NIO_READ_7_CB(_devcb) \
	devcb = &downcast<tc0510nio_device &>(*device).set_read_7_callback(DEVCB_##_devcb);


#define MCFG_TC0640FIO_READ_0_CB(_devcb) \
	devcb = &downcast<tc0640fio_device &>(*device).set_read_0_callback(DEVCB_##_devcb);

#define MCFG_TC0640FIO_READ_1_CB(_devcb) \
	devcb = &downcast<tc0640fio_device &>(*device).set_read_1_callback(DEVCB_##_devcb);

#define MCFG_TC0640FIO_READ_2_CB(_devcb) \
	devcb = &downcast<tc0640fio_device &>(*device).set_read_2_callback(DEVCB_##_devcb);

#define MCFG_TC0640FIO_READ_3_CB(_devcb) \
	devcb = &downcast<tc0640fio_device &>(*device).set_read_3_callback(DEVCB_##_devcb);

#define MCFG_TC0640FIO_WRITE_4_CB(_devcb) \
	devcb = &downcast<tc0640fio_device &>(*device).set_write_4_callback(DEVCB_##_devcb);

#define MCFG_TC0640FIO_READ_7_CB(_devcb) \
	devcb = &downcast<tc0640fio_device &>(*device).set_read_7_callback(DEVCB_##_devcb);


#endif // MAME_MACHINE_TAITOIO_H
