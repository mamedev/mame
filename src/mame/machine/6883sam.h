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

	ATTR_FORCE_INLINE uint16_t display_offset(void)
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

	void update_cpu_clock(void);
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

	auto res_rd_callback() { return m_read_res.bind(); }

	// called to configure banks
	void configure_bank(int bank, uint8_t *memory, uint32_t memory_size, bool is_read_only);
	void configure_bank(int bank, read8_delegate rhandler, write8_delegate whandler);

	// typically called by VDG
	ATTR_FORCE_INLINE DECLARE_READ8_MEMBER( display_read )
	{
		if (offset == (offs_t) ~0)
		{
			/* the VDG is telling the counter to reset */
			m_counter = display_offset();
			m_counter_xdiv = 0;
			m_counter_ydiv = 0;
		}
		else if ((offset & 1) != (m_counter & 0x0001))
		{
			/* DA0 has been toggled - first bump B0-B3 of the counter */
			bool bit3_carry = (m_counter & 0x000F) == 0x000F;
			m_counter = (m_counter & ~0x000F) | ((m_counter + 1) & 0x000F);

			/* and apply the carry (if applicable */
			if (bit3_carry)
				counter_carry_bit3();
		}
		return m_read_res(m_counter & m_counter_mask);
	}

	DECLARE_WRITE_LINE_MEMBER( hs_w );

	// typically called by machine
	address_space *mpu_address_space(void) const { return m_cpu_space; }
	void set_bank_offset(int bank, offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	// represents an external memory bank - memory or IO that the SAM
	// points to with the S2/S1/S0 output
	struct sam_bank
	{
		uint8_t *             m_memory;
		uint32_t              m_memory_size;
		offs_t              m_memory_offset;
		bool                m_memory_read_only;
		read8_delegate      m_rhandler;
		write8_delegate     m_whandler;
	};

	// represents one of the memory "spaces" (e.g. - $8000-$9FFF) that
	// can ultimately point to a bank
	template <uint16_t _addrstart, uint16_t _addrend>
	class sam_space
	{
	public:
		sam_space(sam6883_device &owner);
		void point(const sam_bank &bank, uint16_t offset, uint32_t length = ~0);

	private:
		sam6883_device &    m_owner;
		memory_bank *       m_read_bank;
		memory_bank *       m_write_bank;
		uint32_t              m_length;

		address_space &cpu_space() const;
		void point_specific_bank(const sam_bank &bank, uint32_t offset, uint32_t mask, memory_bank *&memory_bank, uint32_t addrstart, uint32_t addrend, bool is_write);
	};

	// incidentals
	address_space *             m_cpu_space;
	devcb_read8                 m_read_res;
	sam_bank                    m_banks[8];
	sam_space<0x0000, 0x7FFF>   m_space_0000;
	sam_space<0x8000, 0x9FFF>   m_space_8000;
	sam_space<0xA000, 0xBFFF>   m_space_A000;
	sam_space<0xC000, 0xFEFF>   m_space_C000;
	sam_space<0xFF00, 0xFF1F>   m_space_FF00;
	sam_space<0xFF20, 0xFF3F>   m_space_FF20;
	sam_space<0xFF40, 0xFF5F>   m_space_FF40;
	sam_space<0xFF60, 0xFFBF>   m_space_FF60;
	sam_space<0xFFE0, 0xFFFF>   m_space_FFE0;
	uint16_t                      m_counter_mask;

	// SAM state
	uint16_t                      m_counter;
	uint8_t                       m_counter_xdiv;
	uint8_t                       m_counter_ydiv;

	// dummy scratch memory
	uint8_t                       m_dummy[0x8000];

	// typically called by CPU
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	// called when there is a carry out of bit 3 on the counter
	ATTR_FORCE_INLINE void counter_carry_bit3(void)
	{
		uint8_t x_division;
		switch((m_sam_state & (SAM_STATE_V2|SAM_STATE_V1|SAM_STATE_V0)) / SAM_STATE_V0)
		{
			case 0x00:  x_division = 1; break;
			case 0x01:  x_division = 3; break;
			case 0x02:  x_division = 1; break;
			case 0x03:  x_division = 2; break;
			case 0x04:  x_division = 1; break;
			case 0x05:  x_division = 1; break;
			case 0x06:  x_division = 1; break;
			case 0x07:  x_division = 1; break;
			default:
				fatalerror("Should not get here\n");
				return;
		}

		if (++m_counter_xdiv >= x_division)
		{
			m_counter_xdiv = 0;
			m_counter ^= 0x0010;
			if ((m_counter & 0x0010) == 0x0000)
				counter_carry_bit4();
		}
	}

	// called when there is a carry out of bit 4 on the counter
	ATTR_FORCE_INLINE void counter_carry_bit4(void)
	{
		uint8_t y_division;
		switch((m_sam_state & (SAM_STATE_V2|SAM_STATE_V1|SAM_STATE_V0)) / SAM_STATE_V0)
		{
			case 0x00:  y_division = 12;    break;
			case 0x01:  y_division = 1;     break;
			case 0x02:  y_division = 3;     break;
			case 0x03:  y_division = 1;     break;
			case 0x04:  y_division = 2;     break;
			case 0x05:  y_division = 1;     break;
			case 0x06:  y_division = 1;     break;
			case 0x07:  y_division = 1;     break;
			default:
				fatalerror("Should not get here\n");
				return;
		}

		if (++m_counter_ydiv >= y_division)
		{
			m_counter_ydiv = 0;
			m_counter += 0x0020;
		}
	}

	// other members
	void configure_bank(int bank, uint8_t *memory, uint32_t memory_size, bool is_read_only, read8_delegate rhandler, write8_delegate whandler);
	void horizontal_sync(void);
	void update_state(void);
	void update_memory(void);
};

DECLARE_DEVICE_TYPE(SAM6883, sam6883_device)

#endif // MAME_MACHINE_6883SAM_H
