// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    6883sam.h

    Motorola 6883 Synchronous Address Multiplexer

***************************************************************************/

#pragma once

#ifndef __6883SAM__
#define __6883SAM__

#include "emu.h"

#define MCFG_SAM6883_ADD(_tag, _clock, _cputag, _cpuspace) \
	MCFG_DEVICE_ADD(_tag, SAM6883, _clock) \
	sam6883_device::configure_cpu(*device, _cputag, _cpuspace);

#define MCFG_SAM6883_RES_CALLBACK(_read) \
	devcb = &sam6883_device::set_res_rd_callback(*device, DEVCB_##_read);


//**************************************************************************
//  SAM6883 CORE
//**************************************************************************

// base class so that GIME emulation can use some functionality
class sam6883_friend_device
{
public:
	sam6883_friend_device() { m_cpu = NULL; m_sam_state = 0x0000; }

protected:
	// SAM state constants
	static const UINT16 SAM_STATE_TY = 0x8000;
	static const UINT16 SAM_STATE_M1 = 0x4000;
	static const UINT16 SAM_STATE_M0 = 0x2000;
	static const UINT16 SAM_STATE_R1 = 0x1000;
	static const UINT16 SAM_STATE_R0 = 0x0800;
	static const UINT16 SAM_STATE_P1 = 0x0400;
	static const UINT16 SAM_STATE_F6 = 0x0200;
	static const UINT16 SAM_STATE_F5 = 0x0100;
	static const UINT16 SAM_STATE_F4 = 0x0080;
	static const UINT16 SAM_STATE_F3 = 0x0040;
	static const UINT16 SAM_STATE_F2 = 0x0020;
	static const UINT16 SAM_STATE_F1 = 0x0010;
	static const UINT16 SAM_STATE_F0 = 0x0008;
	static const UINT16 SAM_STATE_V2 = 0x0004;
	static const UINT16 SAM_STATE_V1 = 0x0002;
	static const UINT16 SAM_STATE_V0 = 0x0001;

	// incidentals
	cpu_device *            m_cpu;

	// device state
	UINT16                  m_sam_state;

	ATTR_FORCE_INLINE UINT16 display_offset(void)
	{
		return ((m_sam_state & (SAM_STATE_F0|SAM_STATE_F1|SAM_STATE_F2|SAM_STATE_F3|SAM_STATE_F4|SAM_STATE_F5|SAM_STATE_F6)) / SAM_STATE_F0) << 9;
	}

	ATTR_FORCE_INLINE UINT16 alter_sam_state(offs_t offset)
	{
		/* determine the mask */
		UINT16 mask = 1 << (offset >> 1);

		/* determine the new state */
		UINT16 new_state;
		if (offset & 0x0001)
			new_state = m_sam_state | mask;
		else
			new_state = m_sam_state & ~mask;

		/* specify the new state */
		UINT16 xorval = m_sam_state ^ new_state;
		m_sam_state = new_state;
		return xorval;
	}

	void update_cpu_clock(void);
};

class sam6883_device : public device_t, public sam6883_friend_device
{
public:
	sam6883_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_res_rd_callback(device_t &device, _Object object) { return downcast<sam6883_device &>(device).m_read_res.set_callback(object); }

	static void configure_cpu(device_t &device, const char *tag, address_spacenum space)
	{
		sam6883_device &dev = downcast<sam6883_device &>(device);
		dev.m_cpu_tag = tag;
		dev.m_cpu_space_ref = space;
	}

	// called to configure banks
	void configure_bank(int bank, UINT8 *memory, UINT32 memory_size, bool is_read_only);
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
		return m_read_res((m_counter & m_counter_mask) | m_counter_or);
	}

	DECLARE_WRITE_LINE_MEMBER( hs_w );

	// typically called by machine
	address_space *mpu_address_space(void) const { return m_cpu_space; }
	void set_bank_offset(int bank, offs_t offset);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();

private:
	// represents an external memory bank - memory or IO that the SAM
	// points to with the S2/S1/S0 output
	struct sam_bank
	{
		UINT8 *             m_memory;
		UINT32              m_memory_size;
		offs_t              m_memory_offset;
		bool                m_memory_read_only;
		read8_delegate      m_rhandler;
		write8_delegate     m_whandler;
	};

	// represents one of the memory "spaces" (e.g. - $8000-$9FFF) that
	// can ultimately point to a bank
	template<UINT16 _addrstart, UINT16 _addrend>
	class sam_space
	{
	public:
		sam_space(sam6883_device &owner);
		void point(const sam_bank *bank, UINT16 offset, UINT16 mask);

	private:
		sam6883_device &    m_owner;
		memory_bank *       m_read_bank;
		memory_bank *       m_write_bank;
		UINT16              m_mask;

		address_space &cpu_space() const;
		void point_specific_bank(const sam_bank *bank, UINT16 offset, UINT16 mask, memory_bank *&memory_bank, INT32 addrstart, INT32 addrend, bool is_write);
	};

	const char *        m_cpu_tag;
	address_spacenum    m_cpu_space_ref;

	// incidentals
	address_space *             m_cpu_space;
	devcb_read8                m_read_res;
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
	UINT16                      m_counter_mask;
	UINT16                      m_counter_or;

	// SAM state
	UINT16                      m_counter;
	UINT8                       m_counter_xdiv;
	UINT8                       m_counter_ydiv;

	// dummy scratch memory
	UINT8                       m_dummy[0x8000];

	// typically called by CPU
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	// called when there is a carry out of bit 3 on the counter
	ATTR_FORCE_INLINE void counter_carry_bit3(void)
	{
		UINT8 x_division;
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
		UINT8 y_division;
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
	void configure_bank(int bank, UINT8 *memory, UINT32 memory_size, bool is_read_only, read8_delegate rhandler, write8_delegate whandler);
	void horizontal_sync(void);
	void update_state(void);
	void update_memory(void);
};

extern const device_type SAM6883;

#endif /* __6883SAM__ */
