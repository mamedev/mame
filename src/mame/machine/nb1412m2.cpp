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

	0x4ef9,0x0000,0x62fa,0x0000,0x4ef9,0x0000,0x805E,0x0000, 	<- code (0x40d92) 
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

- mightguy:
 * sound "protection" uses address/data to ports 2/3 (R/W)
 * Register map:
 * 0x32: always 5, rom read trigger?
 * 0x33: - address 1 (source?)
 * 0x34: /
 * 0x35: - address 2 (adjust value in rom?)
 * 0x36: /
 * 0x37: R reused for ym3526 register set, read protection rom (same as amatelas)
 *
 * 0x40: counter set, always 1?
 * 0x41: R bit 0 pulse timer? W oneshot timer?
 * 0x42: counter set, always 3?
 * 0x43: counter set, always 3?
 *
 * 0x92: data in/out (for dac?)
 * 0x94: test register? (W 0xaa, checks if R 0xaa)
  
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

void nb1412m2_device::nb1412m2_map(address_map &map)
{
	map(0x32, 0x32).w(this,FUNC(nb1412m2_device::rom_op_w));
	map(0x33, 0x34).w(this,FUNC(nb1412m2_device::rom_address_w));
	map(0x35, 0x36).w(this,FUNC(nb1412m2_device::rom_adjust_w));
	map(0x37, 0x37).r(this,FUNC(nb1412m2_device::rom_decrypt_r));
	
	map(0x40, 0x40).nopw();
	map(0x41, 0x41).r(this,FUNC(nb1412m2_device::timer_r)).nopw();
	map(0x42, 0x43).nopw();
	
	map(0x92, 0x92).ram(); // latch?
	map(0x94, 0x94).rw(this,FUNC(nb1412m2_device::xor_r),FUNC(nb1412m2_device::xor_w));
	
	// 16-bit registers (1=upper), more latches?
	map(0xa0, 0xa1).ram();
	map(0xa2, 0xa3).ram();
}

//-------------------------------------------------
//  nb1412m2_device - constructor
//-------------------------------------------------

nb1412m2_device::nb1412m2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NB1412M2, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(), address_map_constructor(FUNC(nb1412m2_device::nb1412m2_map), this))
	, m_data(*this, DEVICE_SELF)
{
}

device_memory_interface::space_config_vector nb1412m2_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nb1412m2_device::device_start()
{
	save_item(NAME(m_command));
	save_item(NAME(m_rom_address));
	save_item(NAME(m_adj_address));
	save_item(NAME(m_rom_op));
	save_item(NAME(m_xor));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nb1412m2_device::device_reset()
{
	m_command = 0xff;
	m_rom_address = 0;
	m_adj_address = 0;
	m_rom_op = 0;
	m_xor = 0;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE8_MEMBER( nb1412m2_device::rom_op_w )
{
	m_rom_op = data;
}

WRITE8_MEMBER( nb1412m2_device::rom_address_w )
{
	if(offset == 0) // rom hi address
	{
		m_rom_address &= 0x00ff;
		m_rom_address |= data << 8;
	}
	else // rom lo address
	{
		m_rom_address &= 0xff00;
		m_rom_address |= data;
	}
}

WRITE8_MEMBER( nb1412m2_device::rom_adjust_w )
{
	if(offset == 0) // rom hi address
	{
		m_adj_address &= 0x00ff;
		m_adj_address |= data << 8;
	}
	else // rom lo address
	{
		m_adj_address &= 0xff00;
		m_adj_address |= data;
	}
}

// readback from ROM data
READ8_MEMBER( nb1412m2_device::rom_decrypt_r )
{
	// TODO: provided by commands 0x35 & 0x36 (maybe 0x32 too)
	uint8_t prot_adj = m_data[m_adj_address] == 0xff ? 0x44 : 0x82;

//	printf("%02x %04x %02x\n",m_data[m_adj_address],m_adj_address,m_rom_op);
	
	return m_data[m_rom_address & 0x1fff] - prot_adj;	
}

// Mighty Guy specifics
READ8_MEMBER( nb1412m2_device::timer_r )
{
//	return (this->clock().cycles() / 0x34) & 1; // wrong
	return machine().rand() & 1; 
}

WRITE8_MEMBER( nb1412m2_device::xor_w )
{
	m_xor = data ^ m_xor;
}

READ8_MEMBER( nb1412m2_device::xor_r )
{
	// write 0xaa, reads back if register is equal to 0xaa (xor reg?)
	return m_xor;
}

// public accessors
READ8_MEMBER( nb1412m2_device::data_r )
{
	return this->space().read_byte(m_command);
}

WRITE8_MEMBER( nb1412m2_device::data_w )
{
	this->space().write_byte(m_command, data);
}

WRITE8_MEMBER( nb1412m2_device::command_w )
{
	m_command = data;
}
