// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "korean.h"

const device_type MSX_CART_KOREAN_80IN1 = &device_creator<msx_cart_korean_80in1>;
const device_type MSX_CART_KOREAN_90IN1 = &device_creator<msx_cart_korean_90in1>;
const device_type MSX_CART_KOREAN_126IN1 = &device_creator<msx_cart_korean_126in1>;


msx_cart_korean_80in1::msx_cart_korean_80in1(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_KOREAN_80IN1, "MSX Cartridge - Korean 80-in-1", tag, owner, clock, "msx_cart_korean_80in1", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_bank_mask(0)
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
		m_bank_base[i] = NULL;
	}
}


void msx_cart_korean_80in1::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_korean_80in1::restore_banks), this));
}


void msx_cart_korean_80in1::setup_bank(UINT8 bank)
{
	m_bank_base[bank] = get_rom_base() + ( m_selected_bank[bank] & m_bank_mask ) * 0x2000;
}


void msx_cart_korean_80in1::restore_banks()
{
	for (int i = 0; i < 4; i++)
	{
		setup_bank(i);
	}
}


void msx_cart_korean_80in1::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
	}
}


void msx_cart_korean_80in1::initialize_cartridge()
{
	UINT32 size = get_rom_size();

	if ( size > 256 * 0x2000 )
	{
		fatalerror("korean_80in1: ROM is too big\n");
	}

	UINT16 banks = size / 0x2000;

	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		fatalerror("korean_80in1: Invalid ROM size\n");
	}

	m_bank_mask = banks - 1;

	restore_banks();
}


READ8_MEMBER(msx_cart_korean_80in1::read_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		return m_bank_base[(offset - 0x4000) >> 13][offset & 0x1fff];
	}

	return 0xff;
}


WRITE8_MEMBER(msx_cart_korean_80in1::write_cart)
{
	if (offset >= 0x4000 && offset < 0x4004)
	{
		UINT8 bank = offset & 3;

		m_selected_bank[bank] = data;
		setup_bank(bank);
	}
}





msx_cart_korean_90in1::msx_cart_korean_90in1(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_KOREAN_90IN1, "MSX Cartridge - Korean 90-in-1", tag, owner, clock, "msx_cart_korean_90in1", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_bank_mask(0)
	, m_selected_bank(0)
{
	for (int i = 0; i < 4; i++)
	{
		m_bank_base[i] = NULL;
	}
}


void msx_cart_korean_90in1::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_korean_90in1::restore_banks), this));

	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0x77, 0x77, write8_delegate(FUNC(msx_cart_korean_90in1::banking), this));
}


void msx_cart_korean_90in1::restore_banks()
{
	UINT8 *base = get_rom_base();

	switch (m_selected_bank & 0xc0)
	{
		case 0x80:
			base += (m_selected_bank & 0x3e & m_bank_mask) * 0x4000;
			m_bank_base[0] = base;
			m_bank_base[1] = base + 0x2000;
			m_bank_base[2] = base + 0x4000;
			m_bank_base[3] = base + 0x6000;
			break;

		case 0xc0:
			base += (m_selected_bank & m_bank_mask) * 0x4000;
			m_bank_base[0] = base;
			m_bank_base[1] = base + 0x2000;
			m_bank_base[2] = base + 0x2000;
			m_bank_base[3] = base;
			break;

		default:
			base += (m_selected_bank & m_bank_mask) * 0x4000;
			m_bank_base[0] = base;
			m_bank_base[1] = base + 0x2000;
			m_bank_base[2] = base;
			m_bank_base[3] = base + 0x2000;
			break;
	}
}


void msx_cart_korean_90in1::device_reset()
{
	m_selected_bank = 0;
}


void msx_cart_korean_90in1::initialize_cartridge()
{
	UINT32 size = get_rom_size();

	if ( size > 64 * 0x4000 )
	{
		fatalerror("korean_90in1: ROM is too big\n");
	}

	UINT16 banks = size / 0x4000;

	if (size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		fatalerror("korean_90in1: Invalid ROM size\n");
	}

	m_bank_mask = banks - 1;

	restore_banks();
}


READ8_MEMBER(msx_cart_korean_90in1::read_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		return m_bank_base[(offset - 0x4000) >> 13][offset & 0x1fff];
	}

	return 0xff;
}


WRITE8_MEMBER(msx_cart_korean_90in1::banking)
{
	m_selected_bank = data;
	restore_banks();
}





msx_cart_korean_126in1::msx_cart_korean_126in1(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_KOREAN_126IN1, "MSX Cartridge - Korean 126-in-1", tag, owner, clock, "msx_cart_korean_126in1", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_bank_mask(0)
{
	for (int i = 0; i < 2; i++)
	{
		m_selected_bank[i] = i;
		m_bank_base[i] = NULL;
	}
}


void msx_cart_korean_126in1::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_korean_126in1::restore_banks), this));
}


void msx_cart_korean_126in1::setup_bank(UINT8 bank)
{
	m_bank_base[bank] = get_rom_base() + ( m_selected_bank[bank] & m_bank_mask ) * 0x4000;
}


void msx_cart_korean_126in1::restore_banks()
{
	for (int i = 0; i < 2; i++)
	{
		setup_bank(i);
	}
}


void msx_cart_korean_126in1::device_reset()
{
	for (int i = 0; i < 2; i++)
	{
		m_selected_bank[i] = i;
	}
}


void msx_cart_korean_126in1::initialize_cartridge()
{
	UINT32 size = get_rom_size();

	if ( size > 256 * 0x4000 )
	{
		fatalerror("korean_126in1: ROM is too big\n");
	}

	UINT16 banks = size / 0x4000;

	if (size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		fatalerror("korean_126in1: Invalid ROM size\n");
	}

	m_bank_mask = banks - 1;

	restore_banks();
}


READ8_MEMBER(msx_cart_korean_126in1::read_cart)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		return m_bank_base[offset >> 15][offset & 0x3fff];
	}

	return 0xff;
}


WRITE8_MEMBER(msx_cart_korean_126in1::write_cart)
{
	if (offset >= 0x4000 && offset < 0x4002)
	{
		UINT8 bank = offset & 1;

		m_selected_bank[bank] = data;
		setup_bank(bank);
	}
}
