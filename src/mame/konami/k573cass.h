// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Konami 573 Security Cassette
 *
 */
#ifndef MAME_MACHINE_K573CASS_H
#define MAME_MACHINE_K573CASS_H

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

	DECLARE_WRITE_LINE_MEMBER(write_line_d0);
	DECLARE_WRITE_LINE_MEMBER(write_line_d1);
	DECLARE_WRITE_LINE_MEMBER(write_line_d2);
	DECLARE_WRITE_LINE_MEMBER(write_line_d3);
	DECLARE_WRITE_LINE_MEMBER(write_line_d4);
	DECLARE_WRITE_LINE_MEMBER(write_line_d5);
	DECLARE_WRITE_LINE_MEMBER(write_line_d6);
	DECLARE_WRITE_LINE_MEMBER(write_line_d7);
	DECLARE_WRITE_LINE_MEMBER(write_line_zs01_sda);
	DECLARE_READ_LINE_MEMBER(read_line_ds2401);
	DECLARE_READ_LINE_MEMBER(read_line_secflash_sda);
	DECLARE_READ_LINE_MEMBER(read_line_adc083x_do);
	DECLARE_READ_LINE_MEMBER(read_line_adc083x_sars);

protected:
	virtual void device_start() override;

	devcb_write_line m_dsr_handler;

private:
	konami573_cassette_interface *m_dev;
};

class konami573_cassette_interface : public device_interface
{
	friend class konami573_cassette_slot_device;

public:
	virtual ~konami573_cassette_interface();

	DECLARE_WRITE_LINE_MEMBER(output_dsr) { m_slot->m_dsr_handler(state); }

	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d0);
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d1) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d2) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d3) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d4);
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d5);
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d6);
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d7);
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_zs01_sda);
	virtual DECLARE_READ_LINE_MEMBER(read_line_ds2401);
	virtual DECLARE_READ_LINE_MEMBER(read_line_secflash_sda) = 0;
	virtual DECLARE_READ_LINE_MEMBER(read_line_adc083x_do);
	virtual DECLARE_READ_LINE_MEMBER(read_line_adc083x_sars);

protected:
	konami573_cassette_interface(const machine_config &mconfig, device_t &device);

	konami573_cassette_slot_device *m_slot;
};


DECLARE_DEVICE_TYPE(KONAMI573_CASSETTE_X, konami573_cassette_x_device)

class konami573_cassette_x_device: public device_t, public konami573_cassette_interface
{
public:
	konami573_cassette_x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ_LINE_MEMBER(read_line_secflash_sda) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d0) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d1) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d2) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d3) override;

protected:
	konami573_cassette_x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<x76f041_device> m_x76f041;
};


DECLARE_DEVICE_TYPE(KONAMI573_CASSETTE_XI, konami573_cassette_xi_device)

class konami573_cassette_xi_device: public konami573_cassette_x_device
{
public:
	konami573_cassette_xi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ_LINE_MEMBER(read_line_ds2401) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d4) override;

	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d0) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d1) override;
	virtual DECLARE_READ_LINE_MEMBER(read_line_adc083x_do) override;
	virtual DECLARE_READ_LINE_MEMBER(read_line_adc083x_sars) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d5) override;

	double punchmania_inputs_callback(uint8_t input);

protected:
	virtual void device_add_mconfig(machine_config &config) override;

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

	virtual DECLARE_READ_LINE_MEMBER(read_line_secflash_sda) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d0) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d1) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d2) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d3) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d4) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d5) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d6) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d7) override;

protected:
	konami573_cassette_y_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

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

	virtual DECLARE_READ_LINE_MEMBER(read_line_ds2401) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d4) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<ds2401_device> m_ds2401;
};


DECLARE_DEVICE_TYPE(KONAMI573_CASSETTE_ZI, konami573_cassette_zi_device)

class konami573_cassette_zi_device: public device_t, public konami573_cassette_interface
{
public:
	konami573_cassette_zi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ_LINE_MEMBER(read_line_ds2401) override;
	virtual DECLARE_READ_LINE_MEMBER(read_line_secflash_sda) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d4) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d1) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d2) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_d3) override;
	virtual DECLARE_WRITE_LINE_MEMBER(write_line_zs01_sda) override;

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<zs01_device> m_zs01;
	required_device<ds2401_device> m_ds2401;
};


#endif // MAME_MACHINE_K573CASS_H
