// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Konami 573 Security Cassette
 *
 */
#ifndef MAME_KONAMI_K573CASS_H
#define MAME_KONAMI_K573CASS_H

#pragma once


#include "machine/adc083x.h"
#include "machine/ds2401.h"
#include "machine/x76f041.h"
#include "machine/x76f100.h"
#include "zs01.h"


DECLARE_DEVICE_TYPE(KONAMI573_CASSETTE_SLOT, konami573_cassette_slot_device)

class konami573_cassette_interface;

class konami573_cassette_slot_device : public device_t, public device_single_card_slot_interface<konami573_cassette_interface>
{
	friend class konami573_cassette_interface;

public:
	konami573_cassette_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto dsr_handler() { return m_dsr_handler.bind(); }

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
	virtual void device_start() override ATTR_COLD;

	devcb_write_line m_dsr_handler;

private:
	konami573_cassette_interface *m_dev;
};

class konami573_cassette_interface : public device_interface
{
	friend class konami573_cassette_slot_device;

public:
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

protected:
	konami573_cassette_interface(const machine_config &mconfig, device_t &device);

	konami573_cassette_slot_device *m_slot;
};


DECLARE_DEVICE_TYPE(KONAMI573_CASSETTE_X, konami573_cassette_x_device)

class konami573_cassette_x_device: public device_t, public konami573_cassette_interface
{
public:
	konami573_cassette_x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual int read_line_secflash_sda() override;
	virtual void write_line_d0(int state) override;
	virtual void write_line_d1(int state) override;
	virtual void write_line_d2(int state) override;
	virtual void write_line_d3(int state) override;

protected:
	konami573_cassette_x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<x76f041_device> m_x76f041;
};


DECLARE_DEVICE_TYPE(KONAMI573_CASSETTE_XI, konami573_cassette_xi_device)

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

	double punchmania_inputs_callback(uint8_t input);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ds2401_device> m_ds2401;
	required_device<adc0838_device> m_adc0838;
};


DECLARE_DEVICE_TYPE(KONAMI573_CASSETTE_Y, konami573_cassette_y_device)

class konami573_cassette_y_device: public device_t, public konami573_cassette_interface
{
public:
	konami573_cassette_y_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto d0_handler() { return m_d0_handler.bind(); }
	auto d1_handler() { return m_d1_handler.bind(); }
	auto d2_handler() { return m_d2_handler.bind(); }
	auto d3_handler() { return m_d3_handler.bind(); }
	auto d4_handler() { return m_d4_handler.bind(); }
	auto d5_handler() { return m_d5_handler.bind(); }
	auto d6_handler() { return m_d6_handler.bind(); }
	auto d7_handler() { return m_d7_handler.bind(); }

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
	konami573_cassette_y_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<x76f100_device> m_x76f100;

private:
	devcb_write_line m_d0_handler;
	devcb_write_line m_d1_handler;
	devcb_write_line m_d2_handler;
	devcb_write_line m_d3_handler;
	devcb_write_line m_d4_handler;
	devcb_write_line m_d5_handler;
	devcb_write_line m_d6_handler;
	devcb_write_line m_d7_handler;
};


DECLARE_DEVICE_TYPE(KONAMI573_CASSETTE_YI, konami573_cassette_yi_device)

class konami573_cassette_yi_device: public konami573_cassette_y_device
{
public:
	konami573_cassette_yi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual int read_line_ds2401() override;
	virtual void write_line_d4(int state) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ds2401_device> m_ds2401;
};


DECLARE_DEVICE_TYPE(KONAMI573_CASSETTE_ZI, konami573_cassette_zi_device)

class konami573_cassette_zi_device: public device_t, public konami573_cassette_interface
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
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<zs01_device> m_zs01;
	required_device<ds2401_device> m_ds2401;
};


#endif // MAME_KONAMI_K573CASS_H
