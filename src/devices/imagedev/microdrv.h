// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    microdrv.h

    MAME interface to the Sinclair Microdrive image abstraction code

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_MICRODRV_H
#define MAME_DEVICES_IMAGEDEV_MICRODRV_H

#pragma once

#include "magtape.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDV_1 "mdv1"
#define MDV_2 "mdv2"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> microdrive_image_device

class microdrive_image_device : public microtape_image_device
{
public:
	// construction/destruction
	microdrive_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~microdrive_image_device();

	auto comms_out_wr_callback() { return m_write_comms_out.bind(); }

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_creatable() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return "ql_cass"; }
	virtual const char *file_extensions() const noexcept override { return "mdv,mdr"; }

	// specific implementation
	void clk_w(int state);
	void comms_in_w(int state);
	void erase_w(int state);
	void read_write_w(int state);
	void data1_w(int state);
	void data2_w(int state);
	int data1_r();
	int data2_r();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(bit_timer);

private:
	devcb_write_line m_write_comms_out;

	int m_clk;
	int m_comms_in;
	int m_comms_out;
	int m_erase;
	int m_read_write;

	std::unique_ptr<uint8_t[]> m_left;
	std::unique_ptr<uint8_t[]> m_right;

	int m_bit_offset;
	int m_byte_offset;

	emu_timer *m_bit_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(MICRODRIVE, microdrive_image_device)

#endif // MAME_DEVICES_IMAGEDEV_MICRODRV_H
