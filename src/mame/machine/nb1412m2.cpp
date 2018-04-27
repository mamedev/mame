// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Nichibutsu 1412M2 device emulation

Written by Angelo Salese

Fancy data decrypter + timer + ?
Most creative usage is hooked up in Mighty Guy sound CPU,
other games just uses it as a much simpler protection chip providing code
snippets.
It is unknown at current stage if inside the chip there's a MCU
(with internal ROM).

TODO:
- understand how Mighty Guy decrypts data (uses port adjuster in a
  different way than the other games);

Legacy notes from drivers:
- m_mAmazonProtReg[4] bit 0 used on hiscore data (clear on code),
  0x29f vs 0x29e (not an offset?)
- static const uint16_t mAmazonProtData[] =
{
    0x0000,0x5000,0x5341,0x4b45,0x5349,0x4755,0x5245, <- default high scores (0x40db4) - wrong data ?
    0x0000,0x4000,0x0e4b,0x4154,0x5544,0x4f4e,0x0e0e,
    0x0000,0x3000,0x414e,0x4b41,0x4b45,0x5544,0x4f4e,
    0x0000,0x2000,0x0e0e,0x4b49,0x5455,0x4e45,0x0e0e,
    0x0000,0x1000,0x0e4b,0x414b,0x4553,0x4f42,0x410e,

    0x4ef9,0x0000,0x62fa,0x0000,0x4ef9,0x0000,0x805E,0x0000,    <- code (0x40d92)
    0xc800 <- checksum
};
- static const uint16_t mAmatelasProtData[] =
{
    0x0000,0x5000,0x5341,0x4b45,0x5349,0x4755,0x5245, <- default high scores (0x40db4)
    0x0000,0x4000,0x0e4b,0x4154,0x5544,0x4f4e,0x0e0e,
    0x0000,0x3000,0x414e,0x4b41,0x4b45,0x5544,0x4f4e,
    0x0000,0x2000,0x0e0e,0x4b49,0x5455,0x4e45,0x0e0e,
    0x0000,0x1000,0x0e4b,0x414b,0x4553,0x4f42,0x410e,
    0x4ef9,0x0000,0x632e,0x0000,0x4ef9,0x0000,0x80C2,0x0000, <- code (0x40d92)
    0x6100 <- checksum
};
- static const uint16_t mHoreKidProtData[] =
{
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, <- N/A
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x4e75,0x4e75,0x4e75,0x4e75,0x4e75,0x4e75,0x4e75,0x4e75, <- code (0x40dba) It actually never jumps there?
    0x1800 <- checksum
};

***************************************************************************/

#include "emu.h"
#include "machine/nb1412m2.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(NB1412M2, nb1412m2_device, "nb1412m2", "NB1412M2 Mahjong Custom")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nb1412m2_device - constructor
//-------------------------------------------------

nb1412m2_device::nb1412m2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NB1412M2, tag, owner, clock)
	, m_data(*this, DEVICE_SELF)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nb1412m2_device::device_start()
{
	save_item(NAME(m_command));
	save_item(NAME(m_rom_address));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nb1412m2_device::device_reset()
{
	m_command = 0xff;
	m_rom_address = 0;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( nb1412m2_device::data_r )
{
	if(m_command == 0x37) // readback from ROM data
	{
		// TODO: provided by commands 0x35 & 0x36 (maybe 0x32 too)
		uint8_t prot_adj = 0x44;

		return m_data[m_rom_address & 0x1fff] - prot_adj;
	}

	return 0;
}

WRITE8_MEMBER( nb1412m2_device::data_w )
{
	switch(m_command)
	{
		case 0x33: // rom hi
			m_rom_address &= 0x00ff;
			m_rom_address |= data << 8;
			break;
		case 0x34: // rom lo
			m_rom_address &= 0xff00;
			m_rom_address |= data;
			break;
	}
}

WRITE8_MEMBER( nb1412m2_device::command_w )
{
	m_command = data;
}
