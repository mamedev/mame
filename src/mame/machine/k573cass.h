// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Konami 573 Security Cassette
 *
 */

#pragma once

#ifndef __K573CASS_H__
#define __K573CASS_H__

#include "machine/adc083x.h"
#include "machine/ds2401.h"
#include "machine/x76f041.h"
#include "machine/x76f100.h"
#include "machine/zs01.h"

#define MCFG_KONAMI573_CASSETTE_DSR_HANDLER(_devcb) \
	devcb = &konami573_cassette_slot_device::set_dsr_handler(*device, DEVCB_##_devcb);


extern const device_type KONAMI573_CASSETTE_SLOT;

class konami573_cassette_interface;

class konami573_cassette_slot_device : public device_t,
	public device_slot_interface
{
	friend class konami573_cassette_interface;

public:
	konami573_cassette_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_dsr_handler(device_t &device, _Object object) { return downcast<konami573_cassette_slot_device &>(device).m_dsr_handler.set_callback(object); }

	void write_line_d0(int state);
	void write_line_d1(int state);
	void write_line_d2(int state);
	void write_line_d3(int state);
	void write_line_d4(int state);
	void write_line_d5(int state);
	void write_line_d6(int state);
	void write_line_d7(int state);
	void write_line_zs01_sda(int state);
	int read_line_ds2401();
	int read_line_secflash_sda();
	int read_line_adc083x_do();
	int read_line_adc083x_sars();

protected:
	virtual void device_start() override;

	devcb_write_line m_dsr_handler;

private:
	konami573_cassette_interface *m_dev;
};

class konami573_cassette_interface : public device_slot_card_interface
{
	friend class konami573_cassette_slot_device;

public:
	konami573_cassette_interface(const machine_config &mconfig, device_t &device);
	virtual ~konami573_cassette_interface();

	void output_dsr(int state) { m_slot->m_dsr_handler(state); }

	virtual void write_line_d0(int state);
	virtual void write_line_d1(int state) = 0;
	virtual void write_line_d2(int state) = 0;
	virtual void write_line_d3(int state) = 0;
	virtual void write_line_d4(int state);
	virtual void write_line_d5(int state);
	virtual void write_line_d6(int state);
	virtual void write_line_d7(int state);
	virtual void write_line_zs01_sda(int state);
	virtual int read_line_ds2401();
	virtual int read_line_secflash_sda() = 0;
	virtual int read_line_adc083x_do();
	virtual int read_line_adc083x_sars();

	konami573_cassette_slot_device *m_slot;
};


extern const device_type KONAMI573_CASSETTE_X;

class konami573_cassette_x_device: public device_t,
	public konami573_cassette_interface
{
public:
	konami573_cassette_x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	konami573_cassette_x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock,const char *shortname, const char *source);

	virtual int read_line_secflash_sda() override;
	virtual void write_line_d0(int state) override;
	virtual void write_line_d1(int state) override;
	virtual void write_line_d2(int state) override;
	virtual void write_line_d3(int state) override;

protected:
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<x76f041_device> m_x76f041;
};


extern const device_type KONAMI573_CASSETTE_XI;

class konami573_cassette_xi_device: public konami573_cassette_x_device
{
public:
	konami573_cassette_xi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual int read_line_ds2401() override;
	virtual void write_line_d4(int state) override;

	virtual void write_line_d0(int state) override;
	virtual void write_line_d1(int state) override;
	virtual int read_line_adc083x_do() override;
	virtual int read_line_adc083x_sars() override;
	virtual void write_line_d5(int state) override;

	ADC083X_INPUT_CB(punchmania_inputs_callback);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<ds2401_device> m_ds2401;
	required_device<adc0838_device> m_adc0838;
};


extern const device_type KONAMI573_CASSETTE_Y;


#define MCFG_KONAMI573_CASSETTE_Y_D0_HANDLER(_devcb) \
	devcb = &konami573_cassette_y_device::set_d0_handler(*device, DEVCB_##_devcb);

#define MCFG_KONAMI573_CASSETTE_Y_D1_HANDLER(_devcb) \
	devcb = &konami573_cassette_y_device::set_d1_handler(*device, DEVCB_##_devcb);

#define MCFG_KONAMI573_CASSETTE_Y_D2_HANDLER(_devcb) \
	devcb = &konami573_cassette_y_device::set_d2_handler(*device, DEVCB_##_devcb);

#define MCFG_KONAMI573_CASSETTE_Y_D3_HANDLER(_devcb) \
	devcb = &konami573_cassette_y_device::set_d3_handler(*device, DEVCB_##_devcb);

#define MCFG_KONAMI573_CASSETTE_Y_D4_HANDLER(_devcb) \
	devcb = &konami573_cassette_y_device::set_d4_handler(*device, DEVCB_##_devcb);

#define MCFG_KONAMI573_CASSETTE_Y_D5_HANDLER(_devcb) \
	devcb = &konami573_cassette_y_device::set_d5_handler(*device, DEVCB_##_devcb);

#define MCFG_KONAMI573_CASSETTE_Y_D6_HANDLER(_devcb) \
	devcb = &konami573_cassette_y_device::set_d6_handler(*device, DEVCB_##_devcb);

#define MCFG_KONAMI573_CASSETTE_Y_D7_HANDLER(_devcb) \
	devcb = &konami573_cassette_y_device::set_d7_handler(*device, DEVCB_##_devcb);

class konami573_cassette_y_device: public device_t,
	public konami573_cassette_interface
{
public:
	konami573_cassette_y_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	konami573_cassette_y_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock,const char *shortname, const char *source);

	// static configuration helpers
	template<class _Object> static devcb_base &set_d0_handler(device_t &device, _Object object) { return downcast<konami573_cassette_y_device &>(device).m_d0_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_d1_handler(device_t &device, _Object object) { return downcast<konami573_cassette_y_device &>(device).m_d1_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_d2_handler(device_t &device, _Object object) { return downcast<konami573_cassette_y_device &>(device).m_d2_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_d3_handler(device_t &device, _Object object) { return downcast<konami573_cassette_y_device &>(device).m_d3_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_d4_handler(device_t &device, _Object object) { return downcast<konami573_cassette_y_device &>(device).m_d4_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_d5_handler(device_t &device, _Object object) { return downcast<konami573_cassette_y_device &>(device).m_d5_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_d6_handler(device_t &device, _Object object) { return downcast<konami573_cassette_y_device &>(device).m_d6_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_d7_handler(device_t &device, _Object object) { return downcast<konami573_cassette_y_device &>(device).m_d7_handler.set_callback(object); }

	virtual int read_line_secflash_sda() override;
	virtual void write_line_d0(int state) override;
	virtual void write_line_d1(int state) override;
	virtual void write_line_d2(int state) override;
	virtual void write_line_d3(int state) override;
	virtual void write_line_d4(int state) override;
	virtual void write_line_d5(int state) override;
	virtual void write_line_d6(int state) override;
	virtual void write_line_d7(int state) override;

protected:
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<x76f100_device> m_x76f100;
	devcb_write_line m_d0_handler;
	devcb_write_line m_d1_handler;
	devcb_write_line m_d2_handler;
	devcb_write_line m_d3_handler;
	devcb_write_line m_d4_handler;
	devcb_write_line m_d5_handler;
	devcb_write_line m_d6_handler;
	devcb_write_line m_d7_handler;
};


extern const device_type KONAMI573_CASSETTE_YI;

class konami573_cassette_yi_device: public konami573_cassette_y_device
{
public:
	konami573_cassette_yi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual int read_line_ds2401() override;
	virtual void write_line_d4(int state) override;

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<ds2401_device> m_ds2401;
};


extern const device_type KONAMI573_CASSETTE_ZI;

class konami573_cassette_zi_device: public device_t,
	public konami573_cassette_interface
{
public:
	konami573_cassette_zi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual int read_line_ds2401() override;
	virtual int read_line_secflash_sda() override;
	virtual void write_line_d4(int state) override;
	virtual void write_line_d1(int state) override;
	virtual void write_line_d2(int state) override;
	virtual void write_line_d3(int state) override;
	virtual void write_line_zs01_sda(int state) override;

protected:
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<zs01_device> m_zs01;
	required_device<ds2401_device> m_ds2401;
};


#endif
