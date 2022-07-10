// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    PC-9801 MEMSW interface

    A CMOS-like interface that maps in the TVRAM area

    Reference URL:
    http://ohta.music.coocan.jp/packen/board/memsw.htm
    Running the MON command under BASIC allows the user to change these
    settings.
    ssw -> for displaying current settings on screen;
    ssw# -> to change the given #

    List of settings, parenthesis for default if not zero
    SW1 $A3FE2
    xx-- ---- stop bit length (01)
    --x- ---- parity specification
    ---x ---- parity check
    ---- xx-- data bit length (10)
    ---- --x- communication method
    ---- ---x X parameter

    SW2 $A3FE6
    x--- ---- S parameter
    -x-- ---- line feed code when sending
    --x- ---- line feed code when receiving
    ---x ---- Japanese shift code
    ---- xxxx transfer speed (0101)

    SW3 $A3FEA
    x--- ---- Operation when DEL code is received (input / output mode)
    x--- ---- Operation when DEL code is received (terminal mode)
    -x-- ---- Text screen color
    --x- ---- Maximum operating frequency for V30 coprocessor
    ---x ---- With or without V30 coprocessor
    ---- x--- Coprocessor for 80286,386
    ---- -xxx Conventional memory size (100)

    SW4 $A3FEE
    x--- ---- Expansion ROM CE000-CFFFF
    -x-- ---- Expansion ROM CA000-CBFFF
    --x- ---- Expansion ROM D4000-D5FFF
    ---x ---- Expansion ROM D0000-D3FFF
    ---- x--- Expansion ROM CC000-CFFFF
    ---- -x-- Expansion ROM C8000-C9FFF
    ---- --xx (Unused)

    SW5 $A3FF2
    xxxx ---- Select boot device (0000)
    1100 ---- SCSI HDD #1
    1011 ---- HDD #2
    1010 ---- HDD #1
    1000 ---- ROM BASIC
    0110 ---- MO disk
    0100 ---- 1MB FDD
    0010 ---- 640K FDD
    0000 ---- standard
    ???? ---- ROM BASIC
    ---- x--- Screen Hard copy color
    ---- -x-- Use HDD user ID
    ---- --x- Prioritize HDD with device name
    ---- ---x PC-PR201 series used (1)

    SW5 $A3FF6
    --x- ---- Use modem-NCU control function
    ---x ---- Extended screen hard copy function
    ---- x--- Use monitor mode (Use extended monitor mode)
    xx-- -xxx (Unused)


    TODO:
    - Is the mapping truly aligned to 2 bytes? Looks more like 4, needs real
      HW verification.

***************************************************************************/

#include "emu.h"
#include "pc9801_memsw.h"

#include "coreutil.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(PC9801_MEMSW, pc9801_memsw_device, "pc9801_memsw", "NEC PC-9801 Memory Switch device")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc9801_memsw_device - constructor
//-------------------------------------------------

pc9801_memsw_device::pc9801_memsw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_MEMSW, tag, owner, clock),
	device_nvram_interface(mconfig, *this)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc9801_memsw_device::device_start()
{
	save_pointer(NAME(m_bram), m_bram_size);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_memsw_device::device_reset()
{
}


void pc9801_memsw_device::nvram_default()
{
	system_time systime;

	const uint8_t default_memsw_data[0x10] =
	{
		0xe1, 0x48, 0xe1, 0x05,
		0xe1, 0x04, 0xe1, 0x00,
		0xe1, 0x01, 0xe1, 0x00,
		0xe1, 0x00, 0xe1, 0x00
	};

	memcpy(m_bram, default_memsw_data, m_bram_size);

	machine().current_datetime(systime);
	m_bram[0xf] = dec_2_bcd(systime.local_time.year - 2000) & 0xff;
}

bool pc9801_memsw_device::nvram_read(util::read_stream &file)
{
	size_t actual_size;
	return !file.read(m_bram, m_bram_size, actual_size) && actual_size == m_bram_size;
}

bool pc9801_memsw_device::nvram_write(util::write_stream &file)
{
	size_t actual_size;
	return !file.write(m_bram, m_bram_size, actual_size) && actual_size == m_bram_size;
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint8_t pc9801_memsw_device::read(uint8_t offset)
{
	return m_bram[offset];
}

void pc9801_memsw_device::write( uint8_t offset, uint8_t data )
{
	m_bram[offset] = data;
}
