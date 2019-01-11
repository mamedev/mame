// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Generic 8bit and 16 bit latch devices

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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GENERIC_LATCH_8_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GENERIC_LATCH_8, 0)

#define MCFG_GENERIC_LATCH_16_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GENERIC_LATCH_16, 0)

#define MCFG_GENERIC_LATCH_DATA_PENDING_CB(_devcb) \
	devcb = &downcast<generic_latch_base_device &>(*device).set_data_pending_callback(DEVCB_##_devcb);

#define MCFG_GENERIC_LATCH_SEPARATE_ACKNOWLEDGE(_ack) \
	downcast<generic_latch_base_device &>(*device).set_separate_acknowledge(_ack);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> generic_latch_base_device

class generic_latch_base_device : public device_t
{
protected:
	// construction/destruction
	generic_latch_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

public:
	// configuration
	template <class Object> devcb_base &set_data_pending_callback(Object &&cb) { return m_data_pending_cb.set_callback(std::forward<Object>(cb)); }
	void set_separate_acknowledge(bool ack) { m_separate_acknowledge = ack; }

	DECLARE_READ_LINE_MEMBER(pending_r);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	bool has_separate_acknowledge() const { return m_separate_acknowledge; }
	bool is_latch_written() const { return m_latch_written; }
	void set_latch_written(bool latch_written);

private:
	void init_callback(void *ptr, s32 param);

	bool                    m_separate_acknowledge;
	bool                    m_latch_written;
	devcb_write_line        m_data_pending_cb;
};


// ======================> generic_latch_8_device

class generic_latch_8_device : public generic_latch_base_device
{
public:
	// construction/destruction
	generic_latch_8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE8_MEMBER( preset_w );
	DECLARE_WRITE8_MEMBER( clear_w );
	DECLARE_WRITE_LINE_MEMBER( preset_w );
	DECLARE_WRITE_LINE_MEMBER( clear_w );

	DECLARE_READ8_MEMBER( acknowledge_r );
	DECLARE_WRITE8_MEMBER( acknowledge_w );

	void preset_w(u8 value) { m_latched_value = value; }

protected:
	virtual void device_start() override;

	void sync_callback(void *ptr, s32 param);

private:
	u8 m_latched_value;
};


// ======================> generic_latch_16_device

class generic_latch_16_device : public generic_latch_base_device
{
public:
	// construction/destruction
	generic_latch_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	DECLARE_WRITE16_MEMBER( preset_w );
	DECLARE_WRITE16_MEMBER( clear_w );
	DECLARE_WRITE_LINE_MEMBER( preset_w );
	DECLARE_WRITE_LINE_MEMBER( clear_w );

	void preset_w(u16 value) { m_latched_value = value; }

protected:
	virtual void device_start() override;

	void sync_callback(void *ptr, s32 param);

private:
	u16 m_latched_value;
};


#endif  // MAME_MACHINE_GEN_LATCH_H
