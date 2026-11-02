// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    6883sam.cpp

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


#include "emu.h"
#include "6883sam.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define LOG_FBITS   (1U << 1)
#define LOG_VBITS   (1U << 2)
#define LOG_PBITS   (1U << 3)
#define LOG_TBITS   (1U << 4)
#define LOG_MBITS   (1U << 5)
#define LOG_RBITS   (1U << 6)

#define VERBOSE (0)
// #define VERBOSE (LOG_FBITS)
// #define VERBOSE (LOG_FBITS | LOG_VBITS | LOG_PBITS | LOG_TBITS | LOG_MBITS | LOG_RBITS)

#include "logmacro.h"

#define LOGFBITS(...) LOGMASKED(LOG_FBITS, __VA_ARGS__)
#define LOGVBITS(...) LOGMASKED(LOG_VBITS, __VA_ARGS__)
#define LOGPBITS(...) LOGMASKED(LOG_PBITS, __VA_ARGS__)
#define LOGTBITS(...) LOGMASKED(LOG_TBITS, __VA_ARGS__)
#define LOGMBITS(...) LOGMASKED(LOG_MBITS, __VA_ARGS__)
#define LOGRBITS(...) LOGMASKED(LOG_RBITS, __VA_ARGS__)

DEFINE_DEVICE_TYPE(SAM6883, sam6883_device, "sam6883", "MC6883 SAM")



//**************************************************************************
//  DEVICE SETUP
//**************************************************************************

//-------------------------------------------------
//  constructor
//-------------------------------------------------

sam6883_device::sam6883_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SAM6883, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, sam6883_friend_device_interface(mconfig, *this, 4)
	, m_ram_config("ram", ENDIANNESS_BIG, 8, 16, 0)
	, m_rom0_config("rom0", ENDIANNESS_BIG, 8, 13, 0)
	, m_rom1_config("rom1", ENDIANNESS_BIG, 8, 13, 0)
	, m_rom2_config("rom2", ENDIANNESS_BIG, 8, 14, 0)
	, m_io0_config("io0", ENDIANNESS_BIG, 8, 5, 0)
	, m_io1_config("io1", ENDIANNESS_BIG, 8, 5, 0)
	, m_io2_config("io2", ENDIANNESS_BIG, 8, 5, 0)
	, m_boot_config("boot", ENDIANNESS_BIG, 8, 7, 0)
{
}

sam6883_friend_device_interface::sam6883_friend_device_interface(const machine_config &mconfig, device_t &device, int divider)
	: device_interface(device, "sam6883")
	, m_cpu(device, finder_base::DUMMY_TAG)
	, m_sam_state(0x0000)
	, m_divider(divider)
	, m_video_flush_cb(device)
{
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  for the address spaces
//-------------------------------------------------

device_memory_interface::space_config_vector sam6883_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_ram_config),
		std::make_pair(1, &m_rom0_config),
		std::make_pair(2, &m_rom1_config),
		std::make_pair(3, &m_rom2_config),
		std::make_pair(4, &m_io0_config),
		std::make_pair(5, &m_io1_config),
		std::make_pair(6, &m_io2_config),
		std::make_pair(7, &m_boot_config)
	};
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam6883_device::device_start()
{
	// get spaces
	space(0).cache(m_ram_space);
	for (int i = 0; i < 3; i++)
		space(i + 1).cache(m_rom_space[i]);
	for (int i = 0; i < 3; i++)
		space(i + 4).specific(m_io_space[i]);
	space(7).cache(m_boot_space);

	// save state support
	save_item(NAME(m_sam_state));
	save_item(NAME(m_divider));
	save_item(NAME(m_counter_mask));
	save_item(NAME(m_counter));
	save_item(NAME(m_counter_xdiv));
	save_item(NAME(m_counter_ydiv));
	save_item(NAME(m_counter_bit4_input));
	save_item(NAME(m_counter_bit5_input));
}


//-------------------------------------------------
//  read - read from one of the eight spaces
//-------------------------------------------------

uint8_t sam6883_device::read(offs_t offset)
{
	bool mode_64k = (m_sam_state & SAM_STATE_M1) == SAM_STATE_M1;
	if (offset < (mode_64k && (m_sam_state & SAM_STATE_TY) ? 0xff00 : 0x8000))
	{
		// RAM reads: 0000–7FFF or 0000–FEFF
		if (mode_64k && (m_sam_state & (SAM_STATE_TY | SAM_STATE_P1)) == SAM_STATE_P1)
			offset |= 0x8000;
		return m_ram_space.read_byte(offset);
	}
	else if (offset < 0xc000 || offset >= 0xffe0)
	{
		// ROM spaces: 8000–9FFF and A000–BFFF + FFE0–FFFF
		return m_rom_space[BIT(offset, 13)].read_byte(offset & 0x1fff);
	}
	else if (offset < 0xff00)
	{
		// ROM2 space: C000–FEFF
		return m_rom_space[2].read_byte(offset & 0x3fff);
	}
	else if (offset < 0xff60)
	{
		// I/O spaces: FF00–FF1F (slow), FF20–FF3F, FF40–FF5F
		return m_io_space[BIT(offset, 5, 2)].read_byte(offset & 0x1f);
	}
	else
	{
		// FF60–FFDF
		return m_boot_space.read_byte(offset - 0xff60);
	}
}


//-------------------------------------------------
//  write - write to RAM, I/O or internal register
//-------------------------------------------------

void sam6883_device::write(offs_t offset, uint8_t data)
{
	bool mode_64k = (m_sam_state & SAM_STATE_M1) == SAM_STATE_M1;
	if (offset < 0x8000)
	{
		// RAM write space: 0000–7FFF (nominally space 7)
		if (mode_64k && (m_sam_state & (SAM_STATE_TY | SAM_STATE_P1)) == SAM_STATE_P1)
			offset |= 0x8000;
		m_ram_space.write_byte(offset, data);
	}
	else if (offset < 0xc000 || offset >= 0xffe0)
	{
		// ROM spaces: 8000–9FFF and A000–BFFF + FFE0–FFFF (may write through to RAM)
		if (offset < 0xc000 && mode_64k && (m_sam_state & SAM_STATE_TY))
			m_ram_space.write_byte(offset, data);
		m_rom_space[BIT(offset, 13)].write_byte(offset & 0x1fff, data);
	}
	else if (offset < 0xff00)
	{
		// ROM2 space: C000–FEFF (may write through to RAM)
		if (mode_64k && (m_sam_state & SAM_STATE_TY))
			m_ram_space.write_byte(offset, data);
		m_rom_space[2].write_byte(offset & 0x3fff, data);
	}
	else if (offset < 0xff60)
	{
		// I/O spaces: FF00–FF1F (slow), FF20–FF3F, FF40–FF5F
		m_io_space[BIT(offset, 5, 2)].write_byte(offset & 0x1f, data);
	}
	else
	{
		// FF60–FFDF
		m_boot_space.write_byte(offset - 0xff60, data);
		if (offset >= 0xffc0)
			internal_write(offset & 0x1f, data);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sam6883_device::device_reset()
{
	counter_reset(0);
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

void sam6883_device::update_state()
{
	update_memory();
	update_cpu_clock();
}



//-------------------------------------------------
//  update_memory
//-------------------------------------------------

void sam6883_device::update_memory()
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

	// switch depending on the M1/M0 variables
	switch(m_sam_state & (SAM_STATE_M1|SAM_STATE_M0))
	{
		case 0:
			// 4K mode
			m_counter_mask = 0x0FFF;
			break;

		case SAM_STATE_M0:
			// 16K mode
			m_counter_mask = 0x3FFF;
			break;

		case SAM_STATE_M1:
			// 64k mode (dynamic)
		case SAM_STATE_M1|SAM_STATE_M0:
			// 64k mode (static)
			// full 64k RAM or ROM/RAM
			// CoCo Max requires these two be treated the same
			m_counter_mask = 0xfFFF;
			break;
	}
}



//-------------------------------------------------
//  update_cpu_clock - adjusts the speed of the CPU
//  clock
//-------------------------------------------------

void sam6883_friend_device_interface::update_cpu_clock()
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
	m_cpu->owner()->set_unscaled_clock(device().clock() / (m_divider * (speed ? 2 : 4)));
}



//-------------------------------------------------
//  internal_write
//-------------------------------------------------

void sam6883_device::internal_write(offs_t offset, uint8_t data)
{
	// data is ignored
	(void)data;

	// Record elapsed video with the old V bits, even for same-value writes.
	if (offset < 6)
		m_video_flush_cb(1);

	// alter the SAM state
	uint8_t old_mode = m_sam_state & 7;
	uint16_t xorval = alter_sam_state(offset);
	if (xorval & (SAM_STATE_V0|SAM_STATE_V1|SAM_STATE_V2))
		counter_mode_changed(old_mode);

	// based on the mask, apply effects
	if (xorval & (SAM_STATE_TY|SAM_STATE_M1|SAM_STATE_M0|SAM_STATE_P1))
		update_memory();
	if (xorval & (SAM_STATE_R1|SAM_STATE_R0))
		update_cpu_clock();

	if (xorval & (SAM_STATE_F6|SAM_STATE_F5|SAM_STATE_F4|SAM_STATE_F3|SAM_STATE_F2|SAM_STATE_F1|SAM_STATE_F0))
	{
		LOGFBITS("%s: SAM F Address: $%04x\n",
			machine().describe_context(),
			display_offset());
	}

	if (xorval & (SAM_STATE_V0|SAM_STATE_V1|SAM_STATE_V2))
	{
		LOGVBITS("%s: SAM V Bits: $%02x\n",
			machine().describe_context(),
			(m_sam_state & (SAM_STATE_V0|SAM_STATE_V1|SAM_STATE_V2)));
	}

	if (xorval & (SAM_STATE_P1))
	{
		LOGPBITS("%s: SAM P1 Bit: $%02x\n",
			machine().describe_context(),
			(m_sam_state & (SAM_STATE_P1)) >> 10);
	}

	if (xorval & (SAM_STATE_TY))
	{
		LOGTBITS("%s: SAM TY Bit: $%02x\n",
			machine().describe_context(),
			(m_sam_state & (SAM_STATE_TY)) >> 15);
	}

	if (xorval & (SAM_STATE_M0|SAM_STATE_M1))
	{
		LOGMBITS("%s: SAM M Bits: $%02x\n",
			machine().describe_context(),
			(m_sam_state & (SAM_STATE_M0|SAM_STATE_M1)) >> 13);
	}

	if (xorval & (SAM_STATE_R0|SAM_STATE_R1))
	{
		LOGRBITS("%s: SAM R Bits: $%02x\n",
			machine().describe_context(),
			(m_sam_state & (SAM_STATE_R0|SAM_STATE_R1)) >> 11);
	}
}



//-------------------------------------------------
//  display counter divider network
//-------------------------------------------------

// The dividers run even when their outputs are not selected. Mode writes
// reconnect clock inputs, so a high-to-low transition can advance the address
// immediately. See the hardware investigations by Ciaran Anscomb and Stewart
// Orchard: https://www.6809.org.uk/dev/tm002/ (and its linked counter model).

void sam6883_device::counter_reset(uint16_t address)
{
	m_counter = address;
	m_counter_xdiv[0] = m_counter_xdiv[1] = 0;
	m_counter_ydiv[0] = m_counter_ydiv[1] = m_counter_ydiv[2] = 0;
	m_counter_bit4_input = false;
	m_counter_bit5_input = false;
}


bool sam6883_device::counter_x_output() const
{
	switch (m_sam_state & 7)
	{
	case 1: return BIT(m_counter_xdiv[1], 1); // /3
	case 3: return BIT(m_counter_xdiv[0], 0); // /2
	default: return BIT(m_counter, 3);
	}
}


bool sam6883_device::counter_y_output() const
{
	switch (m_sam_state & 7)
	{
	case 0: return BIT(m_counter_ydiv[2], 1); // /3 followed by /4
	case 2: return BIT(m_counter_ydiv[1], 1); // /3
	case 4: return BIT(m_counter_ydiv[0], 0); // /2
	default: return BIT(m_counter, 4);
	}
}


void sam6883_device::counter_update_bit3(bool falling)
{
	if (falling)
	{
		m_counter_xdiv[0] ^= 1;
		m_counter_xdiv[1] = (m_counter_xdiv[1] + 1) % 3;
	}
	counter_set_bit4_input(counter_x_output());
}


void sam6883_device::counter_update_bit4(bool falling)
{
	if (falling)
	{
		m_counter_ydiv[0] ^= 1;
		bool old_div3_output = BIT(m_counter_ydiv[1], 1);
		m_counter_ydiv[1] = (m_counter_ydiv[1] + 1) % 3;
		if (old_div3_output && !BIT(m_counter_ydiv[1], 1))
			m_counter_ydiv[2] = (m_counter_ydiv[2] + 1) & 3;
	}
	counter_set_bit5_input(counter_y_output());
}


void sam6883_device::counter_set_bit4_input(bool level)
{
	bool falling = m_counter_bit4_input && !level;
	m_counter_bit4_input = level;
	if (falling)
	{
		m_counter ^= 0x0010;
		counter_update_bit4(!BIT(m_counter, 4));
	}
}


void sam6883_device::counter_set_bit5_input(bool level)
{
	bool falling = m_counter_bit5_input && !level;
	m_counter_bit5_input = level;
	if (falling)
		m_counter += 0x0020;
}


void sam6883_device::counter_mode_changed(uint8_t old_mode)
{
	static constexpr uint8_t x_divisors[8] = { 1, 3, 1, 2, 1, 1, 1, 1 };
	static constexpr uint8_t y_divisors[8] = { 12, 1, 3, 1, 2, 1, 1, 1 };
	uint8_t new_mode = m_sam_state & 7;
	uint8_t old_y = y_divisors[old_mode];
	uint8_t new_y = y_divisors[new_mode];
	uint8_t old_x = x_divisors[old_mode];
	uint8_t new_x = x_divisors[new_mode];

	// The Y selector settles first. Switching between /12 and /3 briefly
	// selects low; switching between /12 and /2 briefly selects raw B4.
	if (old_y != new_y)
	{
		if ((old_y == 12 && new_y == 3) || (old_y == 3 && new_y == 12))
			counter_set_bit5_input(false);
		else if ((old_y == 12 && new_y == 2) || (old_y == 2 && new_y == 12))
			counter_set_bit5_input(BIT(m_counter, 4));
		counter_set_bit5_input(counter_y_output());
	}

	// Changing between the two divided X clocks briefly selects low.
	if (old_x != new_x)
	{
		if ((old_x == 2 && new_x == 3) || (old_x == 3 && new_x == 2))
			counter_set_bit4_input(false);
		counter_set_bit4_input(counter_x_output());
	}
}


//-------------------------------------------------
//  horizontal_sync
//-------------------------------------------------

void sam6883_device::horizontal_sync()
{
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
			// these SAM modes clear bits B1-B3
		{
			bool falling = BIT(m_counter, 3);
			m_counter &= ~0x000F;
			counter_update_bit3(falling);
			break;
		}

		case 0x00:
		case 0x02:
		case 0x04:
		case 0x06:
			// clear bits B1-B4
		{
			bool falling = BIT(m_counter, 4);
			m_counter &= ~0x001F;
			// B3 clearing is not clocked through X in these modes.
			m_counter_bit4_input = false;
			counter_update_bit4(falling);
			break;
		}

		case 0x07:
			// DMA mode - do nothing
			break;

		default:
			fatalerror("Should not get here\n");
	}
}



//-------------------------------------------------
//  hs_w
//-------------------------------------------------

void sam6883_device::hs_w(int state)
{
	if (state)
	{
		horizontal_sync();
	}
}
