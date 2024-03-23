// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_DIMM_SPD_H
#define MAME_MACHINE_DIMM_SPD_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dimm_spd_device

class dimm_spd_device :  public device_t
{
public:
	// construction/destruction
	dimm_spd_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: dimm_spd_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	dimm_spd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration helpers
	void set_address(u16 address) { m_address = address; }

	auto sda_callback() { return write_sda.bind(); }

	void sda_write(int state);
	void scl_write(int state);

	devcb_write_line write_sda;

	typedef enum
	{
		SIZE_SLOT_EMPTY = 0,
		SIZE_4_MIB,
		SIZE_8_MIB,
		SIZE_16_MIB,
		SIZE_32_MIB,
		SIZE_64_MIB,
		SIZE_128_MIB,
		SIZE_256_MIB
	} dimm_size_t;

	void set_dimm_size(dimm_size_t size);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u8 m_data[256];
	u8 m_latch;
	u8 m_bit;
	u16 m_address;
	u16 m_last_address;
	int m_sda, m_scl;
	u32 m_state, m_state_next;
	u16 m_data_offset;
	bool m_just_acked;
	dimm_size_t m_size;
};

// device type definition
DECLARE_DEVICE_TYPE(DIMM_SPD, dimm_spd_device)

#endif // MAME_MACHINE_DIMM_SPD_H
