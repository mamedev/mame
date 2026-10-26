// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    6883sam.h

    Motorola 6883 Synchronous Address Multiplexer

***************************************************************************/

#ifndef MAME_MACHINE_6883SAM_H
#define MAME_MACHINE_6883SAM_H

#pragma once


//**************************************************************************
//  SAM6883 CORE
//**************************************************************************

// base class so that GIME emulation can use some functionality
class sam6883_friend_device_interface : public device_interface
{
public:
	sam6883_friend_device_interface(const machine_config &mconfig, device_t &device, int divider);

	auto video_flush_callback() { return m_video_flush_cb.bind(); }

protected:
	// SAM state constants
	static const uint16_t SAM_STATE_TY = 0x8000;
	static const uint16_t SAM_STATE_M1 = 0x4000;
	static const uint16_t SAM_STATE_M0 = 0x2000;
	static const uint16_t SAM_STATE_R1 = 0x1000;
	static const uint16_t SAM_STATE_R0 = 0x0800;
	static const uint16_t SAM_STATE_P1 = 0x0400;
	static const uint16_t SAM_STATE_F6 = 0x0200;
	static const uint16_t SAM_STATE_F5 = 0x0100;
	static const uint16_t SAM_STATE_F4 = 0x0080;
	static const uint16_t SAM_STATE_F3 = 0x0040;
	static const uint16_t SAM_STATE_F2 = 0x0020;
	static const uint16_t SAM_STATE_F1 = 0x0010;
	static const uint16_t SAM_STATE_F0 = 0x0008;
	static const uint16_t SAM_STATE_V2 = 0x0004;
	static const uint16_t SAM_STATE_V1 = 0x0002;
	static const uint16_t SAM_STATE_V0 = 0x0001;

	// incidentals
	required_device<cpu_device> m_cpu;

	// device state
	uint16_t m_sam_state;

	// base clock divider (/4 for MC6883, /8 for GIME)
	int m_divider;

	devcb_write_line m_video_flush_cb;

	ATTR_FORCE_INLINE uint16_t display_offset()
	{
		return ((m_sam_state & (SAM_STATE_F0|SAM_STATE_F1|SAM_STATE_F2|SAM_STATE_F3|SAM_STATE_F4|SAM_STATE_F5|SAM_STATE_F6)) / SAM_STATE_F0) << 9;
	}

	ATTR_FORCE_INLINE uint16_t alter_sam_state(offs_t offset)
	{
		/* determine the mask */
		uint16_t mask = 1 << (offset >> 1);

		/* determine the new state */
		uint16_t new_state;
		if (offset & 0x0001)
			new_state = m_sam_state | mask;
		else
			new_state = m_sam_state & ~mask;

		/* specify the new state */
		uint16_t xorval = m_sam_state ^ new_state;
		m_sam_state = new_state;
		return xorval;
	}

	void update_cpu_clock();
};

class sam6883_device : public device_t, public device_memory_interface, public sam6883_friend_device_interface
{
public:
	template <typename T>
	sam6883_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: sam6883_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
	}

	sam6883_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// CPU read/write handlers
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// typically called by VDG
	ATTR_FORCE_INLINE uint8_t display_read(offs_t offset)
	{
		if (offset == (offs_t) ~0)
		{
			/* the VDG is telling the counter to reset */
			counter_reset(display_offset());
		}
		else if ((offset & 1) != (m_counter & 0x0001))
		{
			/* DA0 has been toggled - first bump B0-B3 of the counter */
			uint16_t old_counter = m_counter;
			m_counter = (m_counter & ~0x000F) | ((m_counter + 1) & 0x000F);

			// Track both edges: a later mode write can select this clock level.
			if ((old_counter ^ m_counter) & 0x0008)
				counter_update_bit3(!BIT(m_counter, 3));
		}
		return m_ram_space.read_byte(m_counter & m_counter_mask);
	}

	void hs_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// memory space configuration
	address_space_config        m_ram_config;
	address_space_config        m_rom0_config;
	address_space_config        m_rom1_config;
	address_space_config        m_rom2_config;
	address_space_config        m_io0_config;
	address_space_config        m_io1_config;
	address_space_config        m_io2_config;
	address_space_config        m_boot_config;

	// memory spaces
	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_ram_space;
	memory_access<14, 0, 0, ENDIANNESS_BIG>::cache m_rom_space[3];
	memory_access< 5, 0, 0, ENDIANNESS_BIG>::specific m_io_space[3];
	memory_access< 7, 0, 0, ENDIANNESS_BIG>::cache m_boot_space;
	uint16_t                    m_counter_mask = 0;

	// SAM state
	uint16_t                    m_counter = 0;
	uint8_t                     m_counter_xdiv[2] = {}; // independent /2 and /3 phases
	uint8_t                     m_counter_ydiv[3] = {}; // independent /2, /3 and cascaded /4 phases
	bool                        m_counter_bit4_input = false;
	bool                        m_counter_bit5_input = false;

	// typically called by CPU
	void internal_write(offs_t offset, uint8_t data);

	// Divider outputs clock B4 and B5 on falling edges, including mode writes.
	void counter_reset(uint16_t address);
	bool counter_x_output() const;
	bool counter_y_output() const;
	void counter_update_bit3(bool falling);
	void counter_update_bit4(bool falling);
	void counter_set_bit4_input(bool level);
	void counter_set_bit5_input(bool level);
	void counter_mode_changed(uint8_t old_mode);

	// other members
	void horizontal_sync();
	void update_state();
	void update_memory();
};

DECLARE_DEVICE_TYPE(SAM6883, sam6883_device)

#endif // MAME_MACHINE_6883SAM_H
