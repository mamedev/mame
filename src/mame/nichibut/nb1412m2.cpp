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
- DAC timer is guessworked;

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
#include "nb1412m2.h"



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
	// data decrypter
	map(0x32, 0x32).w(FUNC(nb1412m2_device::rom_op_w));
	map(0x33, 0x34).w(FUNC(nb1412m2_device::rom_address_w));
	map(0x35, 0x36).w(FUNC(nb1412m2_device::rom_adjust_w));
	map(0x37, 0x37).r(FUNC(nb1412m2_device::rom_decrypt_r));

	// timer
	map(0x40, 0x40).w(FUNC(nb1412m2_device::timer_w));
	map(0x41, 0x41).rw(FUNC(nb1412m2_device::timer_r), FUNC(nb1412m2_device::timer_ack_w));
	map(0x42, 0x43).nopw(); // always 0x03

	// DAC control
	map(0x11, 0x11).w(FUNC(nb1412m2_device::dac_control_w));
	map(0x18, 0x18).w(FUNC(nb1412m2_device::dac_timer_w)); // timer frequency
	map(0x19, 0x19).nopw(); // 2 written at POST
	map(0x51, 0x52).w(FUNC(nb1412m2_device::dac_address_w)); //  start address

	// latches?
	map(0x90, 0x90).rw(FUNC(nb1412m2_device::const90_r),FUNC(nb1412m2_device::const90_w)); //ram();
	map(0x92, 0x92).ram();
	map(0x94, 0x94).ram(); //rw(this,FUNC(nb1412m2_device::xor_r),FUNC(nb1412m2_device::xor_w));

	// 16-bit registers (1=upper address), more latches?
	map(0xa0, 0xa1).ram();
	map(0xa2, 0xa3).ram();
}

//-------------------------------------------------
//  nb1412m2_device - constructor
//-------------------------------------------------

nb1412m2_device::nb1412m2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NB1412M2, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(nb1412m2_device::nb1412m2_map), this))
	, m_data(*this, DEVICE_SELF)
	, m_dac_cb(*this)
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
	// assumes it can make an address mask with m_data.length() - 1
	assert(!(m_data.length() & (m_data.length() - 1)));

	save_item(NAME(m_command));
	save_item(NAME(m_rom_address));
	save_item(NAME(m_dac_start_address));
	save_item(NAME(m_dac_current_address));
	save_item(NAME(m_adj_address));
	save_item(NAME(m_rom_op));
	save_item(NAME(m_timer_reg));
	save_item(NAME(m_dac_playback));
	save_item(NAME(m_dac_frequency));
	save_item(NAME(m_const90));
	save_item(NAME(m_timer_rate));

	m_timer = timer_alloc(FUNC(nb1412m2_device::main_timer_tick), this);
	m_timer->adjust(attotime::never);
	m_dac_timer = timer_alloc(FUNC(nb1412m2_device::update_dac), this);

	m_dac_cb.resolve_safe();
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
	m_const90 = 0x18; // fixes coin sample if inserted at first title screen
	m_dac_current_address = m_dac_start_address = 0;
	m_timer_reg = false;
	m_dac_playback = false;
	dac_timer_w(0xd0);
	m_dac_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(nb1412m2_device::main_timer_tick)
{
	m_timer_reg = true;
}

TIMER_CALLBACK_MEMBER(nb1412m2_device::update_dac)
{
	if(m_dac_playback == true)
	{
		uint8_t dac_value;
		dac_value = m_data[m_dac_current_address];
		m_dac_cb(dac_value);
		m_dac_current_address++;
		m_dac_current_address &= m_data.length() - 1;
		if(dac_value == 0x80)
		{
			m_dac_playback = false;
		}
	}
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void nb1412m2_device::rom_op_w(uint8_t data)
{
//  data == 2 starts DAC playback
	if(data == 2)
	{
		m_dac_current_address = m_dac_start_address;
		m_dac_timer->adjust(attotime::never);
		m_dac_timer->adjust(attotime::from_hz(m_dac_frequency), 0, attotime::from_hz(m_dac_frequency));
		m_dac_playback = true;
	}
//  TODO: data == 5 probably loads the result into 0x37
//  if(data != 5)
//      printf("%02x rom_op\n",data);

	m_rom_op = data;
}

void nb1412m2_device::rom_address_w(offs_t offset, uint8_t data)
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

void nb1412m2_device::rom_adjust_w(offs_t offset, uint8_t data)
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
uint8_t nb1412m2_device::rom_decrypt_r()
{
	uint8_t prot_adj;

	// all games but Mighty Guy uses this form as protection:
	// 0xff: -> 0x44
	// Mighty Guy variants
	// 0x86: SFXs -> 0xbd
	// 0x94: BGM  -> 0xaf
	// 0x00: DAC  -> 0x43
	// summing adjust entry and actual value needed always make 0x143 therefore:
	prot_adj = (0x43 - m_data[m_adj_address]) & 0xff;

//  printf("%02x %04x %04x %02x\n",m_data[m_adj_address],m_rom_address,m_adj_address,m_rom_op);

	return m_data[m_rom_address & 0x1fff] - prot_adj;
}

// Mighty Guy specifics
uint8_t nb1412m2_device::timer_r()
{
	return (m_timer_reg) ? 1 : 0;
}

void nb1412m2_device::timer_w(uint8_t data)
{
	if(data != 1)
		logerror("nb1412m2: timer_w with data == %02x\n",data);

	// The DAC frequency and timer clock are linked.
	// When the DAC clock changes, sound driver sets wait loop count ($C010)
	// in the range of 2 to 4 in order to keep the tempo of BGM even if clock changed.
	m_timer->adjust(attotime::from_hz(double(clock()) / (256 * 35) * m_timer_rate));
}

void nb1412m2_device::timer_ack_w(uint8_t data)
{
	m_timer_reg = false;
	m_timer->adjust(attotime::never);
}

void nb1412m2_device::dac_address_w(offs_t offset, uint8_t data)
{
	if(offset == 0) // dac hi address
	{
		m_dac_start_address &= 0x00ff;
		m_dac_start_address |= data << 8;
	}
	else // dac lo address
	{
		m_dac_start_address &= 0xff00;
		m_dac_start_address |= data;
	}
}

void nb1412m2_device::dac_control_w(uint8_t data)
{
	if (data == 0)
	{
		// Mighty Guy uses this to stop psycho gun sound.
		m_dac_playback = false;
	}
}

// seems directly correlated to the DAC timer frequency
// 0xd0 - 0xe0 - 0xf0 are the settings used
void nb1412m2_device::dac_timer_w(uint8_t data)
{
	// TODO: Algorithm is unknown
	m_timer_rate = ((data & 0x30) >> 4) + 1;
	m_dac_frequency = double(clock()) / (256 * 12) * m_timer_rate;
}

// controls music tempo
uint8_t nb1412m2_device::const90_r()
{
	// TODO: likely wrong
	return m_const90;
}

void nb1412m2_device::const90_w(uint8_t data)
{
	m_const90 = data;
}

// public accessors
uint8_t nb1412m2_device::data_r()
{
	return this->space().read_byte(m_command);
}

void nb1412m2_device::data_w(uint8_t data)
{
	this->space().write_byte(m_command, data);
}

void nb1412m2_device::command_w(uint8_t data)
{
	m_command = data;
}
