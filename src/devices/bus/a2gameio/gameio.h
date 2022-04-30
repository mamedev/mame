// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Apple II Game I/O Connector

*********************************************************************/

#ifndef MAME_BUS_A2GAMEIO_GAMEIO_H
#define MAME_BUS_A2GAMEIO_GAMEIO_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class device_a2gameio_interface;

// ======================> apple2_gameio_device

class apple2_gameio_device : public device_t, public device_single_card_slot_interface<device_a2gameio_interface>
{
public:
	// construction/destruction
	apple2_gameio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T>
	apple2_gameio_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: apple2_gameio_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	// configuration
	void set_sw_pullups(bool enabled) { m_sw_pullups = enabled; }
	bool has_sw_pullups() const { return m_sw_pullups; }

	// standard options
	static void default_options(device_slot_interface &slot);
	static void iiandplus_options(device_slot_interface &slot);

	// analog paddles
	u8 pdl0_r();
	u8 pdl1_r();
	u8 pdl2_r();
	u8 pdl3_r();

	// digital switches
	DECLARE_READ_LINE_MEMBER(sw0_r);
	DECLARE_READ_LINE_MEMBER(sw1_r);
	DECLARE_READ_LINE_MEMBER(sw2_r);
	DECLARE_READ_LINE_MEMBER(sw3_r);

	// annunciator outputs
	DECLARE_WRITE_LINE_MEMBER(an0_w);
	DECLARE_WRITE_LINE_MEMBER(an1_w);
	DECLARE_WRITE_LINE_MEMBER(an2_w);
	DECLARE_WRITE_LINE_MEMBER(an3_w);
	DECLARE_WRITE_LINE_MEMBER(an4_w);

	// utility strobe (active low)
	DECLARE_WRITE_LINE_MEMBER(strobe_w);

	// check if a device is connected
	bool is_device_connected() { return (m_intf != nullptr); }

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

private:
	// selected device
	device_a2gameio_interface *m_intf;

	bool m_sw_pullups;
};

// ======================> device_a2gameio_interface

class device_a2gameio_interface : public device_interface
{
	friend class apple2_gameio_device;

public:
	virtual ~device_a2gameio_interface();

protected:
	// construction/destruction
	device_a2gameio_interface(const machine_config &mconfig, device_t &device);

	// optional input overrides
	virtual u8 pdl0_r() { return 0; }
	virtual u8 pdl1_r() { return 0; }
	virtual u8 pdl2_r() { return 0; }
	virtual u8 pdl3_r() { return 0; }
	virtual DECLARE_READ_LINE_MEMBER(sw0_r) { return m_connector->has_sw_pullups() ? 1 : 0; }
	virtual DECLARE_READ_LINE_MEMBER(sw1_r) { return m_connector->has_sw_pullups() ? 1 : 0; }
	virtual DECLARE_READ_LINE_MEMBER(sw2_r) { return m_connector->has_sw_pullups() ? 1 : 0; }
	virtual DECLARE_READ_LINE_MEMBER(sw3_r) { return m_connector->has_sw_pullups() ? 1 : 0; }

	// optional output overrides
	virtual DECLARE_WRITE_LINE_MEMBER(an0_w) { }
	virtual DECLARE_WRITE_LINE_MEMBER(an1_w) { }
	virtual DECLARE_WRITE_LINE_MEMBER(an2_w) { }
	virtual DECLARE_WRITE_LINE_MEMBER(an3_w) { }
	virtual DECLARE_WRITE_LINE_MEMBER(an4_w) { }
	virtual DECLARE_WRITE_LINE_MEMBER(strobe_w) { }

private:
	apple2_gameio_device *m_connector;
};

// device type declaration
DECLARE_DEVICE_TYPE(APPLE2_GAMEIO, apple2_gameio_device)

#endif // MAME_BUS_A2GAMEIO_GAMEIO_H
