// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
#ifndef MAME_TVGAMES_XAVIX_XAVIX2000_H
#define MAME_TVGAMES_XAVIX_XAVIX2000_H

#pragma once

#include "xavix_base.h"
#include "xavix_io_card.h"
#include "xavix_io_duelmast_scanner.h"
#include "xavix_io_ebox_gloves.h"
#include "xavix_io_image_sensor.h"
#include "xavix_io_omt_drive.h"

class xavix2000_epo_ebox_state : public xavix_state
{
public:
	xavix2000_epo_ebox_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_gloves(*this, "gloves")
	{ }

	void xavix2000_epo_ebox(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_ebox_gloves_device> m_gloves;
};

class xavix2000_epo_es2j_state : public xavix_state
{
public:
	xavix2000_epo_es2j_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_controllers(*this, "P%u", 1U)
		, m_card_reader(*this, "card_reader")
	{ }

	void xavix2000_epo_es2j(machine_config &config) ATTR_COLD;

private:
	virtual u8 io0_input_r(u8 direction) override;

	required_ioport_array<2> m_controllers;
	required_device<xavix_epo_es2j_card_reader_device> m_card_reader;
};

class xavix2000_duelmast_state : public xavix_i2c_cart_state
{
public:
	xavix2000_duelmast_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_cart_state(mconfig, type, tag, 0x800000)
		, m_scanner(*this, "scanner")
	{ }

	void xavix2000_duelmast(machine_config &config) ATTR_COLD;

protected:
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

	virtual void xavix_extbus_map(address_map &map) override ATTR_COLD;

private:
	required_device<xavix_duelmast_scanner_device> m_scanner;
};

class xavix2000_camera_state : public xavix_state
{
public:
	xavix2000_camera_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_camera(*this, "camera")
	{ }

	void xavix2000_epo_hamc(machine_config &config) ATTR_COLD;

protected:
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

private:
	required_device<xavix_analog_image_sensor_device> m_camera;
};

class xavix2000_i2c_camera_state : public xavix_i2c_state
{
public:
	xavix2000_i2c_camera_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
		, m_camera(*this, "camera")
	{ }

	void xavix2000_epo_golf(machine_config &config) ATTR_COLD;
	void xavix2000_epo_bowl(machine_config &config) ATTR_COLD;
	void xavix2000_epo_mini(machine_config &config) ATTR_COLD;
	void xavix2000_ttv_sw(machine_config &config) ATTR_COLD;
	void xavix2000_i2c_24c02_camera(machine_config &config) ATTR_COLD;
	void xavix2000_ban_onep(machine_config &config) ATTR_COLD;
	void xavix2000_tom_dpgm(machine_config &config) ATTR_COLD;

protected:
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

	required_device<xavix_analog_image_sensor_device> m_camera;

	void configure_camera(machine_config &config, xavix_analog_image_sensor_device::target_shape shape) ATTR_COLD;
};

class xavix2000_drgqst_state : public xavix2000_i2c_camera_state
{
public:
	xavix2000_drgqst_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix2000_i2c_camera_state(mconfig, type, tag)
		, m_factory_contacts(*this, "FACTORY")
	{ }

	void xavix2000_drgqst(machine_config &config) ATTR_COLD;
	DECLARE_INPUT_CHANGED_MEMBER(factory_contact_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

private:
	void update_eeprom_bus();

	required_ioport m_factory_contacts;
	u8 m_io1_data;
	u8 m_io1_direction;
};

class xavix2000_ban_omt_state : public xavix2000_i2c_camera_state
{
public:
	xavix2000_ban_omt_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix2000_i2c_camera_state(mconfig, type, tag)
		, m_drive(*this, "drive")
	{ }

	void xavix2000_ban_omt(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_omt_drive_device> m_drive;
};

#endif // MAME_TVGAMES_XAVIX_XAVIX2000_H
