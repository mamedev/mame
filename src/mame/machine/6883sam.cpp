// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    6883sam.c

    Motorola 6883 Synchronous Address Multiplexer

    The Motorola 6883 SAM has 16 bits worth of state, but the state is changed
    by writing into a 32 byte address space; odd bytes set bits and even bytes
    clear bits.  Here is the layout:

        31  Set     TY  Map Type            0: RAM/ROM  1: All RAM
        30  Clear   TY  Map Type
        29  Set     M1  Memory Size         00: 4K      10: 64K Dynamic
        28  Clear   M1  Memory Size         01: 16K     11: 64K Static
        27  Set     M0  Memory Size
        26  Clear   M0  Memory Size
        25  Set     R1  MPU Rate            00: Slow    10: Fast
        24  Clear   R1  MPU Rate            01: Dual    11: Fast
        23  Set     R0  MPU Rate
        22  Clear   R0  MPU Rate
        21  Set     P1  Page #1             0: Low      1: High
        20  Clear   P1  Page #1
        19  Set     F6  Display Offset
        18  Clear   F6  Display Offset
        17  Set     F5  Display Offset
        16  Clear   F5  Display Offset
        15  Set     F4  Display Offset
        14  Clear   F4  Display Offset
        13  Set     F3  Display Offset
        12  Clear   F3  Display Offset
        11  Set     F2  Display Offset
        10  Clear   F2  Display Offset
         9  Set     F1  Display Offset
         8  Clear   F1  Display Offset
         7  Set     F0  Display Offset
         6  Clear   F0  Display Offset
         5  Set     V2  VDG Mode
         4  Clear   V2  VDG Mode
         3  Set     V1  VDG Mode
         2  Clear   V1  VDG Mode
         1  Set     V0  VDG Mode
         0  Clear   V0  VDG Mode

    All parts of the SAM are fully emulated except R1/R0 (the changes in the
    MPU rate are approximated) and M1/M0

***************************************************************************/


#include "machine/6883sam.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define LOG_SAM     0

const device_type SAM6883 = &device_creator<sam6883_device>;



//**************************************************************************
//  DEVICE SETUP
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

sam6883_device::sam6883_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SAM6883, "SAM6883", tag, owner, clock, "sam6883", __FILE__),
		m_cpu_tag(nullptr),
		m_cpu_space_ref(AS_PROGRAM),
		m_read_res(*this),
		m_space_0000(*this),
		m_space_8000(*this),
		m_space_A000(*this),
		m_space_C000(*this),
		m_space_FF00(*this),
		m_space_FF20(*this),
		m_space_FF40(*this),
		m_space_FF60(*this),
		m_space_FFE0(*this)
{
	memset(m_banks, '\0', sizeof(m_banks));
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam6883_device::device_start()
{
	// find the CPU
	m_cpu = machine().device<cpu_device>(m_cpu_tag);
	m_cpu_space = &m_cpu->space(m_cpu_space_ref);

	// resolve callbacks
	m_read_res.resolve_safe(0);

	// install SAM handlers
	m_cpu_space->install_read_handler(0xFFC0, 0xFFDF, 0, 0, read8_delegate(FUNC(sam6883_device::read), this));
	m_cpu_space->install_write_handler(0xFFC0, 0xFFDF, 0, 0, write8_delegate(FUNC(sam6883_device::write), this));

	// save state support
	save_item(NAME(m_sam_state));
	save_item(NAME(m_counter));
	save_item(NAME(m_counter_xdiv));
	save_item(NAME(m_counter_ydiv));
}



//-------------------------------------------------
//  configure_bank - bank configuration
//-------------------------------------------------

void sam6883_device::configure_bank(int bank, UINT8 *memory, UINT32 memory_size, bool is_read_only)
{
	configure_bank(bank, memory, memory_size, is_read_only, read8_delegate(), write8_delegate());
}



//-------------------------------------------------
//  configure_bank - bank configuration
//-------------------------------------------------

void sam6883_device::configure_bank(int bank, read8_delegate rhandler, write8_delegate whandler)
{
	configure_bank(bank, nullptr, 0, false, rhandler, whandler);
}



//-------------------------------------------------
//  configure_bank - bank configuration
//-------------------------------------------------

void sam6883_device::configure_bank(int bank, UINT8 *memory, UINT32 memory_size, bool is_read_only, read8_delegate rhandler, write8_delegate whandler)
{
	assert((bank >= 0) && (bank < ARRAY_LENGTH(m_banks)));
	m_banks[bank].m_memory = memory;
	m_banks[bank].m_memory_size = memory_size;
	m_banks[bank].m_memory_read_only = is_read_only;
	m_banks[bank].m_rhandler = rhandler;
	m_banks[bank].m_whandler = whandler;

	/* if we're configuring a bank that never changes, update it now */
	switch(bank)
	{
		case 4:
			m_space_FF00.point(&m_banks[4], 0x0000, 0);
			break;
		case 5:
			m_space_FF20.point(&m_banks[5], 0x0000, 0);
			break;
		case 6:
			m_space_FF40.point(&m_banks[6], 0x0000, 0);
			break;
		case 7:
			m_space_FF60.point(&m_banks[7], 0x0000, 0);
			break;
		case 2:
			m_space_FFE0.point(&m_banks[2], 0x1FE0, 0);
			break;
	}
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sam6883_device::device_reset()
{
	m_counter = 0;
	m_counter_xdiv = 0;
	m_counter_ydiv = 0;
	m_sam_state = 0x0000;
	update_state();
}



//-------------------------------------------------
//  device_post_load - device-specific post load
//-------------------------------------------------

void sam6883_device::device_post_load()
{
	device_t::device_post_load();
	update_state();
}



//-------------------------------------------------
//  update_state
//-------------------------------------------------

void sam6883_device::update_state(void)
{
	update_memory();
	update_cpu_clock();
}



//-------------------------------------------------
//  update_memory
//-------------------------------------------------

void sam6883_device::update_memory(void)
{
	// Memory size - allowed restricting memory accesses to something less than
	// 32k
	//
	// This was a SAM switch that occupied 4 addresses:
	//
	//      $FFDD   (set)   R1
	//      $FFDC   (clear) R1
	//      $FFDB   (set)   R0
	//      $FFDA   (clear) R0
	//
	// R1:R0 formed the following states:
	//      00  - 4k
	//      01  - 16k
	//      10  - 64k
	//      11  - static RAM (??)
	//
	// If something less than 64k was set, the low RAM would be smaller and
	// mirror the other parts of the RAM
	//
	// TODO:  Find out what "static RAM" is
	// TODO:  This should affect _all_ memory accesses, not just video ram
	// TODO:  Verify that the CoCo 3 ignored this

	// for now, lets assume that the caller is nice and made m_memory_size
	// a power of two
	UINT16 ram_mask = (UINT16) m_banks[0].m_memory_size - 1;

	// switch depending on the M1/M0 variables
	bool setup_rom = true;
	switch(m_sam_state & (SAM_STATE_M1|SAM_STATE_M0))
	{
		case 0:
			// 4K mode
			m_space_0000.point(&m_banks[0], 0x0000, ram_mask);
			m_counter_mask = 0x0FFF;
			m_counter_or = 0x0000;
			break;

		case SAM_STATE_M0:
			// 16K mode
			m_space_0000.point(&m_banks[0], 0x0000, ram_mask);
			m_counter_mask = 0x3FFF;
			m_counter_or = 0x0000;
			break;

		case SAM_STATE_M1:
		case SAM_STATE_M1|SAM_STATE_M0:
			// 64k mode
			if (m_sam_state & SAM_STATE_TY)
			{
				// full 64k RAM
				m_space_0000.point(&m_banks[0], 0x0000, ram_mask);
				m_space_8000.point(&m_banks[0], 0x8000, 0);
				m_space_A000.point(&m_banks[0], 0xA000, 0);
				m_space_C000.point(&m_banks[0], 0xC000, 0);
				m_counter_mask = 0xFFFF;
				m_counter_or = 0x0000;
				setup_rom = false;
			}
			else
			{
				// ROM/RAM
				UINT16 ram_base = (m_sam_state & SAM_STATE_P1) ? 0x8000 : 0x0000;
				m_space_0000.point(&m_banks[0], ram_base, ram_mask);
				m_counter_mask = 0x7FFF;
				m_counter_or = ram_base;
			}
			break;
	}

	if (setup_rom)
	{
		m_space_8000.point(&m_banks[1], m_banks[1].m_memory_offset, 0);
		m_space_A000.point(&m_banks[2], m_banks[2].m_memory_offset, 0);
		m_space_C000.point(&m_banks[3], m_banks[3].m_memory_offset, 0);
	}

	// update $FFE0-$FFFF
	m_space_FFE0.point(&m_banks[2], m_banks[2].m_memory_offset + 0x1FE0, 0);
}



//-------------------------------------------------
//  update_cpu_clock - adjusts the speed of the CPU
//  clock
//-------------------------------------------------

void sam6883_friend_device::update_cpu_clock(void)
{
	// The infamous speed up poke.
	//
	// This was a SAM switch that occupied 4 addresses:
	//
	//      $FFD9   (set)   R1
	//      $FFD8   (clear) R1
	//      $FFD7   (set)   R0
	//      $FFD6   (clear) R0
	//
	// R1:R0 formed the following states:
	//      00  - slow          0.89 MHz
	//      01  - dual speed    ???
	//      1x  - fast          1.78 MHz
	//
	// R1 controlled whether the video addressing was speeded up and R0
	// did the same for the CPU.  On pre-CoCo 3 machines, setting R1 caused
	// the screen to display garbage because the M6847 could not display
	// fast enough.
	//
	// TODO:  Make the overclock more accurate.  In dual speed, ROM was a fast
	// access but RAM was not.  I don't know how to simulate this.

	int speed = (m_sam_state & (SAM_STATE_R1|SAM_STATE_R0)) / SAM_STATE_R0;

	// the line below is weird because we are not strictly emulating the M6809E with emphasis on the 'E'
	m_cpu->set_clock_scale(speed ? 2 : 1);
}



//-------------------------------------------------
//  set_bank_offset
//-------------------------------------------------

void sam6883_device::set_bank_offset(int bank, offs_t offset)
{
	if (m_banks[bank].m_memory_offset != offset)
	{
		m_banks[bank].m_memory_offset = offset;
		update_memory();
	}
}



//-------------------------------------------------
//  read
//-------------------------------------------------

READ8_MEMBER( sam6883_device::read )
{
	return 0;
}



//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE8_MEMBER( sam6883_device::write )
{
	/* alter the SAM state */
	UINT16 xorval = alter_sam_state(offset);

	/* based on the mask, apply effects */
	if (xorval & (SAM_STATE_TY|SAM_STATE_M1|SAM_STATE_M0|SAM_STATE_P1))
		update_memory();
	if (xorval & (SAM_STATE_R1|SAM_STATE_R0))
		update_cpu_clock();
}



//-------------------------------------------------
//  horizontal_sync
//-------------------------------------------------

void sam6883_device::horizontal_sync(void)
{
	bool carry;

	// When horizontal sync occurs, bits B1-B3 or B1-B4 may be cleared (except in DMA mode).  The catch
	// is that the SAM's counter is a chain of flip-flops.  Clearing the counter can cause carries to
	// occur just as they can when the counter is bumped.
	//
	// This is critical in getting certain semigraphics modes to work correctly.  Guardian uses this
	// mode (see bug #1153).  Special thanks to Ciaran Anscomb and Phill Harvey-Smith for figuring this
	// out
	switch((m_sam_state & (SAM_STATE_V2|SAM_STATE_V1|SAM_STATE_V0)) / SAM_STATE_V0)
	{
		case 0x01:
		case 0x03:
		case 0x05:
			/* these SAM modes clear bits B1-B3 */
			carry = (m_counter & 0x0008) ? true : false;
			m_counter &= ~0x000F;
			if (carry)
				counter_carry_bit3();
			break;

		case 0x00:
		case 0x02:
		case 0x04:
		case 0x06:
			/* clear bits B1-B4 */
			carry = (m_counter & 0x0010) ? true : false;
			m_counter &= ~0x001F;
			if (carry)
				counter_carry_bit4();
			break;

		case 0x07:
			/* DMA mode - do nothing */
			break;

		default:
			fatalerror("Should not get here\n");
	}
}



//-------------------------------------------------
//  hs_w
//-------------------------------------------------

WRITE_LINE_MEMBER( sam6883_device::hs_w )
{
	if (state)
	{
		horizontal_sync();
	}
}



//-------------------------------------------------
//  sam_space::ctor
//-------------------------------------------------

template<UINT16 _addrstart, UINT16 _addrend>
sam6883_device::sam_space<_addrstart, _addrend>::sam_space(sam6883_device &owner)
	: m_owner(owner)
{
	m_read_bank = nullptr;
	m_write_bank = nullptr;
	m_mask = 0;
}



//-------------------------------------------------
//  sam_space::cpu_space
//-------------------------------------------------

template<UINT16 _addrstart, UINT16 _addrend>
address_space &sam6883_device::sam_space<_addrstart, _addrend>::cpu_space() const
{
	assert(m_owner.m_cpu_space != nullptr);
	return *m_owner.m_cpu_space;
}



//-------------------------------------------------
//  sam_space::point
//-------------------------------------------------

template<UINT16 _addrstart, UINT16 _addrend>
void sam6883_device::sam_space<_addrstart, _addrend>::point(const sam_bank *bank, UINT16 offset, UINT16 mask)
{
	if (LOG_SAM)
	{
		m_owner.logerror("sam6883_device::sam_space::point():  addrstart=0x%04X addrend=0x%04X offset=0x%04X mask=0x%04X bank->m_memory=0x%p bank->m_memory_read_only=%s\n",
			(unsigned) _addrstart,
			(unsigned) _addrend,
			(unsigned) offset,
			(unsigned) mask,
			bank->m_memory,
			bank->m_memory_read_only ? "true" : "false");
	}

	point_specific_bank(bank, offset, mask, m_read_bank, _addrstart, _addrend, false);
	point_specific_bank(bank, offset, mask, m_write_bank, _addrstart, _addrend, true);
}



//-------------------------------------------------
//  sam_space::point_specific_bank
//-------------------------------------------------
template<UINT16 _addrstart, UINT16 _addrend>
void sam6883_device::sam_space<_addrstart, _addrend>::point_specific_bank(const sam_bank *bank, UINT16 offset, UINT16 mask, memory_bank *&memory_bank, INT32 addrstart, INT32 addrend, bool is_write)
{
	char buffer[16];

	if (bank->m_memory != nullptr)
	{
		// normalize offset
		if (mask != 0)
			offset &= mask;

		// this bank is a memory bank - first ensure that we have a bank
		if (!memory_bank || !memory_bank->matches_exactly(addrstart, addrend) || (mask != m_mask))
		{
			// name the bank
			snprintf(buffer, ARRAY_LENGTH(buffer), "bank%04X_%c", addrstart, is_write ? 'w' : 'r');

			// install it
			if (is_write)
				cpu_space().install_write_bank(addrstart, addrend, mask, 0, buffer);
			else
				cpu_space().install_read_bank(addrstart, addrend, mask, 0, buffer);
			m_mask = mask;

			// and get it
			memory_bank = cpu_space().device().owner()->membank(buffer);
		}

		// point the bank
		if (memory_bank != nullptr)
		{
			if (is_write && bank->m_memory_read_only)
				memory_bank->set_base(m_owner.m_dummy);
			else
				memory_bank->set_base(bank->m_memory + offset);
		}
	}
	else
	{
		// this bank uses handlers
		assert((offset == 0) && (mask == 0));   // changes to the offset are not supported

		if (is_write)
		{
			if (!bank->m_whandler.isnull())
				cpu_space().install_write_handler(addrstart, addrend, 0, 0, bank->m_whandler);
		}
		else
		{
			if (!bank->m_rhandler.isnull())
				cpu_space().install_read_handler(addrstart, addrend, 0, 0, bank->m_rhandler);
		}
	}
}
