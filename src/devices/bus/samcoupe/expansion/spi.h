// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    S.P.I. SAM Parallel Interface for SAM Coupe

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_EXPANSION_SPI_H
#define MAME_BUS_SAMCOUPE_EXPANSION_SPI_H

#pragma once

#include "expansion.h"
#include "bus/centronics/ctronics.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sam_spi_device

class sam_spi_device : public device_t, public device_samcoupe_expansion_interface
{
public:
	// construction/destruction
	sam_spi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual void print_w(int state) override;

	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<output_latch_device> m_data_out;
	required_device<centronics_device> m_centronics;

	void centronics_busy_w(int state);

	int m_print;
	int m_busy;
	int m_mode;
};

// device type definition
DECLARE_DEVICE_TYPE(SAM_SPI, sam_spi_device)

#endif // MAME_BUS_SAMCOUPE_EXPANSION_SPI_H
