// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    QJump/Quanta QL Internal Mouse Interface emulation

**********************************************************************/

#ifndef MAME_SINCLAIR_QIMI_H
#define MAME_SINCLAIR_QIMI_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> qimi_device

class qimi_device :  public device_t
{
public:
	// construction/destruction
	qimi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto extint_wr_callback() { return m_write_extint.bind(); }

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	uint8_t read(offs_t offset, uint8_t data);
	void write(offs_t offset, uint8_t data);

	DECLARE_INPUT_CHANGED_MEMBER( mouse_x_changed );
	DECLARE_INPUT_CHANGED_MEMBER( mouse_y_changed );

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum
	{
		ST_Y_DIR = 0x01,
		ST_X_INT = 0x04,
		ST_X_DIR = 0x10,
		ST_Y_INT = 0x20
	};

	devcb_write_line m_write_extint;

	required_ioport m_buttons;

	uint8_t m_status;
	bool m_extint_en;
};


// device type definition
DECLARE_DEVICE_TYPE(QIMI, qimi_device)


#endif // MAME_SINCLAIR_QIMI_H
