// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    DIMM "Dual In-line Memory Module"

***************************************************************************/

#ifndef MAME_BUS_DIMM_DIMM_H
#define MAME_BUS_DIMM_DIMM_H

#pragma once

#include "machine/i2chle.h"


class device_dimm_interface;

class dimm_slot_device : public device_t, public device_single_card_slot_interface<device_dimm_interface>
{
public:
	dimm_slot_device &set_address(uint8_t address) { m_address = address; return *this; }
	auto sda_cb() { return m_sda_cb.bind(); }

	// module -> host
	void host_sda_w(int state) { m_sda_cb(state); }

	// host -> module
	void sda_w(int state);
	void scl_w(int state);

protected:
	dimm_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual ~dimm_slot_device();

protected:
	virtual void device_start() override ATTR_COLD;

private:
	device_dimm_interface *m_module;
	devcb_write_line m_sda_cb;

	uint8_t m_address;
};

class sdr_sdram_slot_device : public dimm_slot_device
{
public:
	sdr_sdram_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *dflt) :
		sdr_sdram_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_default_option(dflt);
	}

	sdr_sdram_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ddr_sdram_slot_device : public dimm_slot_device
{
public:
	ddr_sdram_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *dflt) :
		ddr_sdram_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_default_option(dflt);
	}

	ddr_sdram_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


class device_dimm_interface : private i2c_hle_interface
{
public:
	virtual ~device_dimm_interface();

	virtual void sda_w(int state) { sda_write(state); };
	virtual void scl_w(int state) { scl_write(state); };

	void set_address(uint8_t address) { i2c_hle_interface::set_address(address); }

protected:
	device_dimm_interface(const machine_config &mconfig, device_t &device);

	dimm_slot_device *m_slot;
};


class spd_dimm_device : public device_t, public device_dimm_interface
{
public:
	virtual uint8_t read_data(uint16_t offset) override;

protected:
	spd_dimm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

private:
	required_region_ptr<uint8_t> m_spd;
};


// device type definition
DECLARE_DEVICE_TYPE(SDR_SDRAM_SLOT, sdr_sdram_slot_device)
DECLARE_DEVICE_TYPE(DDR_SDRAM_SLOT, ddr_sdram_slot_device)


// include here so drivers don't need to
#include "modules.h"


#endif // MAME_BUS_DIMM_DIMM_H
