// license:BSD-3-Clause
// copyright-holders:XingXing, Vas Crabb
#ifndef MAME_IGS_XAMCU_H
#define MAME_IGS_XAMCU_H

#pragma once

#include "cpu/xa/xa.h"
#include "sound/ics2115.h"


class igs_xa_mcu_device_base : public device_t
{
public:
	virtual ~igs_xa_mcu_device_base();

	auto irq() { return m_irq_cb.bind(); }

	int irq_r() const { return m_irq; }

protected:
	igs_xa_mcu_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void set_irq(int state);

	u8 mcu_p0_r();
	u8 mcu_p1_r();
	u8 mcu_p2_r();
	u8 mcu_p3_r();

	void mcu_p0_w(u8 data);
	void mcu_p1_w(u8 data);
	void mcu_p2_w(u8 data);
	void mcu_p3_w(u8 data);

	TIMER_CALLBACK_MEMBER(do_cmd_w);

	required_device<mx10exa_cpu_device> m_mcu;

	devcb_write_line m_irq_cb;

	u16 m_command;
	s16 m_num_params;

	u8 m_port_dat[4];
	u8 m_port0_latch;
	u8 m_port2_latch;

	u8 m_irq;
};


class igs_xa_mcu_ics_sound_device : public igs_xa_mcu_device_base
{
public:
	igs_xa_mcu_ics_sound_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	virtual ~igs_xa_mcu_ics_sound_device();

	void cmd_w(u16 data);
	u16 response_low_r() { return m_response[0]; }
	u16 response_high_r() { return m_response[1]; }

	u16 cmd_r() const { return m_command; } // hack for HLE'ing unimplemented functionality

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void mcu_p1_w(u8 data);
	void mcu_p3_w(u8 data);

	required_device<ics2115_device> m_ics;

	u16 m_response[2];
};


class igs_xa_mcu_subcpu_device : public igs_xa_mcu_device_base
{
public:
	igs_xa_mcu_subcpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	virtual ~igs_xa_mcu_subcpu_device();

	void set_disable() { m_mcu.lookup()->set_disable(); } // for systems where the microcontroller has not been dumped

	void cmd_w(offs_t offset, u16 data);
	u16 response_r() { return m_response; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void mcu_p3_w(u8 data);

	u16 m_response;
};


DECLARE_DEVICE_TYPE(IGS_XA_ICS_SOUND, igs_xa_mcu_ics_sound_device)
DECLARE_DEVICE_TYPE(IGS_XA_SUBCPU,    igs_xa_mcu_subcpu_device)

#endif // MAME_IGS_XAMCU_H
