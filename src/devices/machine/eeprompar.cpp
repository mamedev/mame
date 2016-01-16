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

***************************************************************************/

#include "emu.h"
#include "machine/eeprompar.h"



//**************************************************************************
//  BASE DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  eeprom_parallel_base_device - constructor
//-------------------------------------------------

eeprom_parallel_base_device::eeprom_parallel_base_device(const machine_config &mconfig, device_type devtype, std::string name, std::string tag, device_t *owner, std::string shortname, std::string source)
	: eeprom_base_device(mconfig, devtype, name, tag, owner, shortname, source)
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

eeprom_parallel_28xx_device::eeprom_parallel_28xx_device(const machine_config &mconfig, device_type devtype, std::string name, std::string tag, device_t *owner, std::string shortname, std::string source)
	: eeprom_parallel_base_device(mconfig, devtype, name, tag, owner, shortname, source)
{
}


//-------------------------------------------------
//  read/write - read/write handlers
//-------------------------------------------------

WRITE8_MEMBER(eeprom_parallel_28xx_device::write)
{
	eeprom_base_device::write(offset, data);
}

READ8_MEMBER(eeprom_parallel_28xx_device::read)
{
	return eeprom_base_device::read(offset);
}



//**************************************************************************
//  DERIVED TYPES
//**************************************************************************

// macro for defining a new device class
#define DEFINE_PARALLEL_EEPROM_DEVICE(_baseclass, _lowercase, _uppercase, _bits, _cells) \
eeprom_parallel_##_lowercase##_device::eeprom_parallel_##_lowercase##_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) \
	: eeprom_parallel_##_baseclass##_device(mconfig, EEPROM_PARALLEL_##_uppercase, "Parallel EEPROM " #_uppercase " (" #_cells "x" #_bits ")", tag, owner, #_lowercase, __FILE__) \
{ \
	static_set_size(*this, _cells, _bits); \
} \
const device_type EEPROM_PARALLEL_##_uppercase = &device_creator<eeprom_parallel_##_lowercase##_device>;
// standard 28XX class of 8-bit EEPROMs
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 2804, 2804, 8, 512)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 2816, 2816, 8, 2048)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 2864, 2864, 8, 8192)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 28256, 28256, 8, 32768)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 28512, 28512, 8, 65536)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 28010, 28010, 8, 131072)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 28020, 28020, 8, 262144)
DEFINE_PARALLEL_EEPROM_DEVICE(28xx, 28040, 28040, 8, 524288)
