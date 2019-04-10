// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    GAYLE

    Gate array used in the Amiga 600 and Amiga 1200 computers.

***************************************************************************/

#ifndef MAME_MACHINE_GAYLE_H
#define MAME_MACHINE_GAYLE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gayle_device

class gayle_device : public device_t
{
public:
	// construction/destruction
	gayle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto int2_handler() { return m_int2_w.bind(); }
	auto cs0_read_handler() { return m_cs0_read.bind(); }
	auto cs0_write_handler() { return m_cs0_write.bind(); }
	auto cs1_read_handler() { return m_cs1_read.bind(); }
	auto cs1_write_handler() { return m_cs1_write.bind(); }

	// interface
	DECLARE_WRITE_LINE_MEMBER( ide_interrupt_w );

	DECLARE_READ16_MEMBER( gayle_r );
	DECLARE_WRITE16_MEMBER( gayle_w );
	DECLARE_READ16_MEMBER( gayle_id_r );
	DECLARE_WRITE16_MEMBER( gayle_id_w );

	// inline configuration
	void set_id(uint8_t id) { m_gayle_id = id; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		GAYLE_CS = 0,   // interrupt status
		GAYLE_IRQ,      // interrupt change
		GAYLE_INTEN,    // interrupt enable register
		GAYLE_CFG       // config register
	};

	devcb_write_line m_int2_w;

	devcb_read16 m_cs0_read;
	devcb_write16 m_cs0_write;
	devcb_read16 m_cs1_read;
	devcb_write16 m_cs1_write;

	uint8_t m_gayle_id;
	int m_gayle_id_count;
	uint8_t m_gayle_reg[4];
};

// device type definition
DECLARE_DEVICE_TYPE(GAYLE, gayle_device)

#endif // MAME_MACHINE_GAYLE_H
