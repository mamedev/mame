// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    eeprompar.c

    Parallel EEPROM devices.

****************************************************************************

    Parallel EEPROMs are generally simpler than serial EEPROMs, though
    they require more pins to provide the full set of address and data
    lines necessary. They also require more pins the larger the EEPROM
    is, whereas serial EEPROMs all share the same pinout regardless of
    size.

    At a basic level, there are 5 sets of signals involved:

        * /CE = chip enable
        * /OE = output enable
        * /WE = write enable
        * D0-Dn = data lines
        * A0-An = address lines

    To access the chip, the various enable states must be asserted or
    cleared. Note that these are generally active-low, so asserted means
    pulled to GND, and cleared means pulled to Vcc:

        /CE     /OE     /WE     Action
       ASSERT  ASSERT  CLEAR    Read (D0-Dn contain output data)
       ASSERT  CLEAR   ASSERT   Write/Erase (D0-Dn are input data)

    Erase is performed by doing a write with D0-Dn all set to 1.

    In general, it is slow to write or erase (9ms is quoted in the 2816A
    datasheet, for example), and the /WE must be held low for the entire
    write/erase duration in order to guarantee the data is written.

    Though it is possible for the /OE line to be strobed directly upon
    read accesses, it may also be controlled independently of /CS. For the
    sake of convenience, the device here can also be configured to emulate
    a small amount of external circuitry (1/2 of a LS74 flip-flop and 1
    gate of a LS02 or LS08), typically used by Atari Games, that reasserts
    /OE low to lock the EEPROM after each byte of data is written and upon
    reset, with extra writes required to unlock the EEPROM in between.

***************************************************************************/

#include "emu.h"
#include "eeprompar.h"

//#define VERBOSE 1
#include "logmacro.h"

// set this to 1 to break Prop Cycle (28C64 page write emulation needed)
#define EMULATE_POLLING 0


//**************************************************************************
//  BASE DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  eeprom_parallel_base_device - constructor
//-------------------------------------------------

eeprom_parallel_base_device::eeprom_parallel_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner)
	: eeprom_base_device(mconfig, devtype, tag, owner)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void eeprom_parallel_base_device::device_start()
{
	// start the base class
	eeprom_base_device::device_start();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void eeprom_parallel_base_device::device_reset()
{
	// reset the base class
	eeprom_base_device::device_reset();
}



//**************************************************************************
//  28XX INTERFACE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  eeprom_parallel_28xx_device - constructor
//-------------------------------------------------

eeprom_parallel_28xx_device::eeprom_parallel_28xx_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner)
	: eeprom_parallel_base_device(mconfig, devtype, tag, owner),
		m_lock_after_write(false),
		m_oe(-1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void eeprom_parallel_28xx_device::device_start()
{
	// start the base class
	eeprom_parallel_base_device::device_start();

	save_item(NAME(m_oe));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void eeprom_parallel_28xx_device::device_reset()
{
	// reset the base class
	eeprom_parallel_base_device::device_reset();

	if (m_lock_after_write)
		m_oe = 0;
}


//-------------------------------------------------
//  read/write - read/write handlers
//-------------------------------------------------

void eeprom_parallel_28xx_device::write(offs_t offset, uint8_t data)
{
	if (m_oe == 0)
	{
		// Master Boy writes every byte twice, resetting a control line in between, for some reason not clear
		if (internal_read(offset) != data)
			LOG("%s: Write attempted while /OE active (offset = %X, data = %02X)\n", machine().describe_context(), offset, data);
	}
	else
	{
		LOG("%s: Write cycle started (offset = %X, data = %02X)\n", machine().describe_context(), offset, data);
		eeprom_base_device::write(offset, data);
		if (m_lock_after_write)
			m_oe = 0;
	}
}

uint8_t eeprom_parallel_28xx_device::read(address_space &space, offs_t offset)
{
	if (m_oe == 1)
	{
		LOG("%s: Read attempted while /OE inactive (offset = %X)\n", machine().describe_context(), offset);
		return space.unmap();
	}

	// if a write has not completed yet, the highest bit of data written will be read back inverted when polling the offset
	if (ready() || !EMULATE_POLLING)
		return eeprom_base_device::read(offset);
	else
	{
		LOG("%s: Data read back before write completed (offset = %X)\n", machine().describe_context(), offset);
		return ~internal_read(offset) & 0x80;
	}
}


//-------------------------------------------------
//  oe_w - direct write to /OE (true line state)
//-------------------------------------------------

void eeprom_parallel_28xx_device::oe_w(int state)
{
	LOG("%s: EEPROM %s for writing\n", machine().describe_context(), state ? "unlocked" : "locked");
	m_oe = state ? 1 : 0;
}


//-------------------------------------------------
//  unlock_write - unlock EEPROM by deasserting
//  /OE line through external flip-flop
//-------------------------------------------------

void eeprom_parallel_28xx_device::unlock_write8(uint8_t data) { oe_w(1); }
void eeprom_parallel_28xx_device::unlock_write16(uint16_t data) { oe_w(1); }
void eeprom_parallel_28xx_device::unlock_write32(uint32_t data) { oe_w(1); }


//**************************************************************************
//  DERIVED TYPES
//**************************************************************************

// macro for defining a new device class
#define DEFINE_PARALLEL_EEPROM_DEVICE(_baseclass, _lowercase, _uppercase, _bits, _cells) \
eeprom_parallel_##_lowercase##_device::eeprom_parallel_##_lowercase##_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) \
	: eeprom_parallel_##_baseclass##_device(mconfig, EEPROM_##_uppercase, tag, owner) \
{ \
	size(_cells, _bits); \
} \
DEFINE_DEVICE_TYPE(EEPROM_##_uppercase, eeprom_parallel_##_lowercase##_device, #_lowercase, "Parallel EEPROM " #_uppercase " (" #_cells "x" #_bits ")")

// standard 28XX class of 8-bit EEPROMs
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 2804, 2804, 8, 512)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 2816, 2816, 8, 2048)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 2864, 2864, 8, 8192)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 28256, 28256, 8, 32768)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 28512, 28512, 8, 65536)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 28010, 28010, 8, 131072)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 28020, 28020, 8, 262144)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 28040, 28040, 8, 524288)
