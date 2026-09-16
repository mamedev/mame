// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Generic 8 bit and 16 bit latch devices

***************************************************************************/

#ifndef MAME_MACHINE_GEN_LATCH_H
#define MAME_MACHINE_GEN_LATCH_H

#pragma once



//**************************************************************************
//  DEVICE TYPE DECLARATIONS
//**************************************************************************

DECLARE_DEVICE_TYPE(GENERIC_LATCH_8, generic_latch_8_device)
DECLARE_DEVICE_TYPE(GENERIC_LATCH_16, generic_latch_16_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> generic_latch_base_device

class generic_latch_base_device : public device_t
{
public:
	// configuration
	auto data_pending_callback() { return m_data_pending_cb.bind(); }
	void set_separate_acknowledge(bool ack) { m_separate_acknowledge = ack; }

	int pending_r();

	u8 acknowledge_r(address_space &space);
	void acknowledge_w(u8 data = 0);

protected:
	// construction/destruction
	generic_latch_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	bool has_separate_acknowledge() const { return m_separate_acknowledge; }
	bool is_latch_written() const { return m_latch_written; }
	void set_latch_written(bool latch_written);

private:
	void init_callback(s32 param);

	bool                    m_separate_acknowledge;
	bool                    m_latch_written;
	devcb_write_line        m_data_pending_cb;
};


// ======================> generic_latch_8_device

class generic_latch_8_device : public generic_latch_base_device
{
public:
	// construction/destruction
	generic_latch_8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u8 read();
	void write(u8 data);

	void preset_w(u8 data = 0xff);
	void clear_w(u8 data = 0);
	void preset(int state);
	void clear(int state);

protected:
	virtual void device_start() override ATTR_COLD;

	void sync_callback(s32 param);

private:
	u8 m_latched_value;
};


// ======================> generic_latch_16_device

class generic_latch_16_device : public generic_latch_base_device
{
public:
	// construction/destruction
	generic_latch_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u16 read();
	void write(u16 data);

	void preset_w(u16 data = 0xffff);
	void clear_w(u16 data = 0);
	void preset(int state);
	void clear(int state);

protected:
	virtual void device_start() override ATTR_COLD;

	void sync_callback(s32 param);

private:
	u16 m_latched_value;
};

#endif  // MAME_MACHINE_GEN_LATCH_H
