// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    6883sam.h

    Motorola 6883 Synchronous Address Multiplexer

***************************************************************************/

#ifndef MAME_MACHINE_6883SAM_H
#define MAME_MACHINE_6883SAM_H

#pragma once

#include "machine/ram.h"

#include <type_traits>
#include <utility>
#include <vector>


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

class sam6883_device : public device_t, public sam6883_friend_device_interface
{
public:
	template <typename T>
	sam6883_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: sam6883_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
	}

	sam6883_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// The host maps amap() over $0000-$FFFF of the CPU's program space.  It
	// holds the RAM views and the host's ROM and I/O maps, each relative to
	// the start of its range:
	//  1 ROM0  $8000-$9FFF
	//  2 ROM1  $A000-$BFFF and the vectors at $FFE0-$FFFF
	//  3 ROM2  $C000-$FEFF
	//  4 IO0   $FF00-$FF1F
	//  5 IO1   $FF20-$FF3F
	//  6 IO2   $FF40-$FF5F
	//  7 BOOT  $FF60-$FFDF
	// Maps are submapped, so they can't contain views.
	template <typename T, typename Ret, typename... Params>
	void set_addrmap(int select, Ret (T::*func)(Params...))
	{
		device_t &dev(mconfig().current_device());
		if constexpr (std::is_base_of_v<device_t, T>)
			set_addrmap(select, address_map_constructor(func, dev.tag(), &downcast<T &>(dev)));
		else
			set_addrmap(select, address_map_constructor(func, dev.tag(), &dynamic_cast<T &>(dev)));
	}
	void set_addrmap(int select, address_map_constructor map);

	void amap(address_map &map) ATTR_COLD;

	// physical RAM; without a RAM device, the host maps it with map_ram()
	template <typename T> void set_ram(T &&tag) { m_ram.set_tag(std::forward<T>(tag)); }

	// point a 256 byte aligned range of physical RAM at other memory; a null
	// read pointer reads as $00 and a null write pointer drops writes
	void map_ram(offs_t start, offs_t end, uint8_t *read, uint8_t *write);

	// configure a 256 byte aligned range of physical RAM with alternative
	// read/write memory, installed once and switched with select_ram_bank();
	// entry 0 is selected initially
	void configure_ram_bank(offs_t start, offs_t end, std::vector<std::pair<uint8_t *, uint8_t *> > entries);
	void select_ram_bank(offs_t start, int entry);

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
		uint16_t const address = m_counter & m_counter_mask;
		uint8_t const *const page = m_ram_read[address >> 8];
		return page ? page[address & 0xff] : 0x00;
	}

	void hs_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	optional_device<ram_device> m_ram;
	memory_view                 m_low_view;     // $0000-$7FFF: 0 = page 0, 1 = page 1
	memory_view                 m_high_view;    // $8000-$FEFF: 0 = RAM (map type 1), disabled = ROM
	address_map_constructor     m_maps[8];

	// physical RAM, in 256 byte pages
	uint8_t *                   m_ram_read[256] = {};
	uint8_t *                   m_ram_write[256] = {};

	// switchable ranges of physical RAM
	struct ram_bank
	{
		offs_t start;
		offs_t end;
		memory_bank *read;
		memory_bank *write;
		std::vector<std::pair<uint8_t *, uint8_t *> > entries;
	};
	std::vector<ram_bank>       m_ram_banks;
	int8_t                      m_ram_page_bank[256];   // index into m_ram_banks, or -1

	memory_passthrough_handler  m_high_write_tap;
	memory_passthrough_handler  m_ad_tap;
	bool                        m_ad_tap_installed = false;
	uint16_t                    m_counter_mask = 0;

	// SAM state
	uint16_t                    m_counter = 0;
	uint8_t                     m_counter_xdiv[2] = {}; // independent /2 and /3 phases
	uint8_t                     m_counter_ydiv[3] = {}; // independent /2, /3 and cascaded /4 phases
	bool                        m_counter_bit4_input = false;
	bool                        m_counter_bit5_input = false;

	// address dependent rate timing, in fast CPU cycles
	uint64_t                    m_ad_base = 0;      // a slow cycle boundary
	uint64_t                    m_ad_slow_end = ~uint64_t(0); // end of the last slow access

	// typically called by CPU
	void internal_write(offs_t offset, uint8_t data);

	// In address dependent rate mode, $0000-$7FFF and $FF00-$FF1F are slow
	ATTR_FORCE_INLINE void address_dependent_rate(offs_t offset)
	{
		if ((m_sam_state & (SAM_STATE_R1|SAM_STATE_R0)) == SAM_STATE_R0
				&& !machine().side_effects_disabled())
			address_dependent_cycle(offset < 0x8000 || (offset & 0xffe0) == 0xff00);
	}
	void address_dependent_cycle(bool slow);
	void update_ad_tap();

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
	void check_ram_range(offs_t start, offs_t end);
	void set_ram_pages(offs_t start, offs_t end, uint8_t *read, uint8_t *write);
	void install_ram(memory_view::memory_view_entry &entry, offs_t start, offs_t end, offs_t physical, bool writable);
	void install_ram_views();
	void horizontal_sync();
	void update_state();
	void update_memory();
};

DECLARE_DEVICE_TYPE(SAM6883, sam6883_device)

#endif // MAME_MACHINE_6883SAM_H
