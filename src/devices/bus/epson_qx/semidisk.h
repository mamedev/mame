// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * Semidisk RAM disk card
 *
 *******************************************************************/

#ifndef MAME_BUS_EPSON_QX_SEMIDISK_H
#define MAME_BUS_EPSON_QX_SEMIDISK_H

#pragma once

#include "bus/epson_qx/option.h"
#include "machine/nvram.h"

namespace bus::epson_qx {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* Epson Semidisk Device */

class semidisk_device : public device_t, public device_nvram_interface, public device_option_expansion_interface
{
public:
	// construction/destruction
	semidisk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	// nvram interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	uint8_t data_r(offs_t offset);
	void data_w(offs_t offset, uint8_t data);
	void reg_w(offs_t offset, uint8_t data);

	void map(address_map &map);
private:
	required_ioport m_config;

	std::unique_ptr<uint8_t[]> m_ram;

	uint8_t m_track;
	uint8_t m_sector;
};

} // namespace bus::epson_qx

// device type definition
DECLARE_DEVICE_TYPE_NS(EPSON_QX_OPTION_SEMIDISK, bus::epson_qx, semidisk_device)


#endif // MAME_BUS_EPSON_QX_SEMIDISK_H

