// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    Cybiko Wireless Inter-tainment System

    (c) 2001-2007 Tim Schuerewegen

    Cybiko Classic (V1)
    Cybiko Classic (V2)
    Cybiko Xtreme

*/

#include "emu.h"
#include "cybiko.h"

#define LOG_LEVEL  1
#define _logerror(level,x)  do { if (LOG_LEVEL > level) logerror x; } while (0)

#define RAMDISK_SIZE (512 * 1024)


////////////////////////
// DRIVER INIT & EXIT //
////////////////////////

void cybiko_state::init_cybiko()
{
	_logerror( 0, ("init_cybikov1\n"));
	m_maincpu->space(AS_PROGRAM).install_ram(0x200000, 0x200000 + m_ram->size() - 1, m_ram->pointer());
}

void cybiko_state::init_cybikoxt()
{
	_logerror( 0, ("init_cybikoxt\n"));
	m_maincpu->space(AS_PROGRAM).install_ram(0x400000, 0x400000 + m_ram->size() - 1, m_ram->pointer());
}

QUICKLOAD_LOAD_MEMBER(cybiko_state::quickload_cybiko)
{
	image.fread(m_flash1->get_ptr(), std::min(image.length(), uint64_t(0x84000)));

	return image_init_result::PASS;
}

QUICKLOAD_LOAD_MEMBER(cybiko_state::quickload_cybikoxt)
{
	address_space &dest = m_maincpu->space(AS_PROGRAM);
	uint32_t size = std::min(image.length(), uint64_t(RAMDISK_SIZE));

	std::vector<uint8_t> buffer(size);
	image.fread(&buffer[0], size);
	for (int byte = 0; byte < size; byte++)
		dest.write_byte(0x400000 + byte, buffer[byte]);

	return image_init_result::PASS;
}

///////////////////
// MACHINE START //
///////////////////

void cybiko_state::machine_start()
{
	int nvram_size = RAMDISK_SIZE;

	if (m_ram->size() < nvram_size)
	{
		nvram_size = m_ram->size();
	}

	m_nvram->set_base(m_ram->pointer(), nvram_size);
}

///////////////////
// MACHINE RESET //
///////////////////

void cybiko_state::machine_reset()
{
}

/////////////////////////
// READ/WRITE HANDLERS //
/////////////////////////

uint16_t cybiko_state::cybiko_lcd_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;
	if (ACCESSING_BITS_8_15) data |= (m_crtc->reg_idx_r() << 8);
	if (ACCESSING_BITS_0_7) data |= (m_crtc->reg_dat_r() << 0);
	return data;
}

void cybiko_state::cybiko_lcd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15) m_crtc->reg_idx_w((data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7) m_crtc->reg_dat_w((data >> 0) & 0xff);
}

int cybiko_state::cybiko_key_r(offs_t offset, int mem_mask)
{
	uint16_t data = 0xFFFF;
	for (uint8_t i = 0; i < 15; i++)
	{
		if (m_input[i].found() && !BIT(offset, i))
			data &= ~m_input[i]->read();
	}
	if (data != 0xFFFF)
	{
		_logerror( 1, ("cybiko_key_r (%08X/%04X) %04X\n", offset, mem_mask, data));
	}
	return data;
}

uint16_t cybiko_state::cybikov1_key_r(offs_t offset, uint16_t mem_mask)
{
	return cybiko_key_r(offset, mem_mask);
}

uint16_t cybiko_state::cybikov2_key_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = cybiko_key_r(offset, mem_mask);
	if (!BIT(offset, 0))
		data |= 0x0002; // or else [esc] does not work in "lost in labyrinth"
	return data;
}

uint16_t cybiko_state::cybikoxt_key_r(offs_t offset, uint16_t mem_mask)
{
	return cybiko_key_r(offset, mem_mask);
}

#if 0
uint16_t cybiko_state::cybikov1_io_reg_r(offs_t offset)
{
	uint16_t data = 0;
#if 0
	_logerror( 2, ("cybikov1_io_reg_r (%08X)\n", offset));
	switch (offset)
	{
		// keyboard
		case h8_device::PORT_1:
		{
			if (!BIT(ioport("A1")->read(), 1))
				data |= (1 << 3); // "esc" key
		}
		break;
		// serial dataflash
		case h8_device::PORT_3:
		{
			if (m_flash1->so_r())
				data |= H8S_P3_RXD1;
		}
		break;
		// rs232
		case h8_device::PORT_5:
		{
			if (cybiko_rs232_pin_rxd())
				data |= H8S_P5_RXD2;
		}
		break;
		// real-time clock
		case h8_device::PORT_F:
		{
			data = H8S_PF_PF2;
			if (m_rtc->sda_r())
				data |= H8S_PF_PF0;
		}
		break;
		// serial 2
		case H8S_IO_SSR2 :
		{
			if (cybiko_rs232_rx_queue())
				data |= H8S_SSR_RDRF;
		}
		break;
	}
#endif
	return data;
}

uint16_t cybiko_state::cybikov2_io_reg_r(offs_t offset)
{
	uint16_t data = 0;
#if 0
	_logerror( 2, ("cybikov2_io_reg_r (%08X)\n", offset));
	switch (offset)
	{
		// keyboard
		case h8_device::PORT_1 :
		{
			if (!BIT(ioport("A1")->read(), 1))
				data |= (1 << 3); // "esc" key
		}
		break;
		// serial dataflash
		case h8_device::PORT_3 :
		{
			if (m_flash1->so_r())
				data |= H8S_P3_RXD1;
		}
		break;
		// rs232
		case h8_device::PORT_5 :
		{
			if (cybiko_rs232_pin_rxd())
				data |= H8S_P5_RXD2;
		}
		break;
		// real-time clock
		case h8_device::PORT_F :
		{
			data = H8S_PF_PF2;
			if (m_rtc->sda_r())
				data |= H8S_PF_PF0;
		}
		break;
		// serial 2
		case H8S_IO_SSR2 :
		{
			if (cybiko_rs232_rx_queue())
				data |= H8S_SSR_RDRF;
		}
		break;
	}
#endif
	return data;
}

uint16_t cybiko_state::cybikoxt_io_reg_r(offs_t offset)
{
	uint16_t data = 0;
#if 0
	_logerror( 2, ("cybikoxt_io_reg_r (%08X)\n", offset));
	switch (offset)
	{
		// rs232
		case h8_device::PORT_3 :
		{
			if (cybiko_rs232_pin_rxd())
				data |= H8S_P3_RXD1;
		}
		break;
		// ...
		case h8_device::PORT_A :
		{
			data |= (1 << 6); // recharge batteries (xtreme)
			data |= (1 << 7); // on/off key (xtreme)
		}
		break;
		// real-time clock
		case h8_device::PORT_F :
		{
			if (m_rtc->sda_r())
				data |= H8S_PF_PF6;
		}
		break;
		// serial 1
		case H8S_IO_SSR1 :
		{
			if (cybiko_rs232_rx_queue())
				data |= H8S_SSR_RDRF;
		}
		break;
	}
#endif
	return data;
}

void cybiko_state::cybikov1_io_reg_w(offs_t offset, uint16_t data)
{
#if 0
	_logerror( 2, ("cybikov1_io_reg_w (%08X/%02X)\n", offset, data));
	switch (offset)
	{
		// speaker
		case H8S_IO_P1DR :
		{
			m_speaker->level_w((data & H8S_P1_TIOCB1) ? 1 : 0);
		}
		break;
		// serial dataflash
		case H8S_IO_P3DR :
		{
			m_flash1->cs_w ((data & H8S_P3_SCK0) ? 0 : 1);
			m_flash1->si_w ((data & H8S_P3_TXD1) ? 1 : 0);
			m_flash1->sck_w((data & H8S_P3_SCK1) ? 1 : 0);
		}
		break;
		// rs232
		case H8S_IO_P5DR :
		{
			cybiko_rs232_pin_txd((data & H8S_P5_TXD2) ? 1 : 0);
			cybiko_rs232_pin_sck((data & H8S_P5_SCK2) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDR :
		{
			m_rtc->scl_w((data & H8S_PF_PF1) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDDR :
		{
			m_rtc->sda_w((data & H8S_PF_PF0) ? 0 : 1);
		}
		break;
	}
#endif
}

void cybiko_state::cybikov2_io_reg_w(offs_t offset, uint16_t data)
{
#if 0
	_logerror( 2, ("cybikov2_io_reg_w (%08X/%02X)\n", offset, data));
	switch (offset)
	{
		// speaker
		case H8S_IO_P1DR :
		{
			m_speaker->level_w((data & H8S_P1_TIOCB1) ? 1 : 0);
		}
		break;
		// serial dataflash
		case H8S_IO_P3DR :
		{
			m_flash1->cs_w ((data & H8S_P3_SCK0) ? 0 : 1);
			m_flash1->si_w ((data & H8S_P3_TXD1) ? 1 : 0);
			m_flash1->sck_w((data & H8S_P3_SCK1) ? 1 : 0);
		}
		break;
		// rs232
		case H8S_IO_P5DR :
		{
			cybiko_rs232_pin_txd((data & H8S_P5_TXD2) ? 1 : 0);
			cybiko_rs232_pin_sck((data & H8S_P5_SCK2) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDR :
		{
			m_rtc->scl_w((data & H8S_PF_PF1) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDDR :
		{
			m_rtc->sda_w((data & H8S_PF_PF0) ? 0 : 1);
		}
		break;
	}
#endif
}

void cybiko_state::cybikoxt_io_reg_w(offs_t offset, uint16_t data)
{
#if 0
	_logerror( 2, ("cybikoxt_io_reg_w (%08X/%02X)\n", offset, data));
	switch (offset)
	{
		// speaker
		case H8S_IO_P1DR :
		{
			m_speaker->level_w((data & H8S_P1_TIOCB1) ? 1 : 0);
		}
		break;
		// rs232
		case H8S_IO_P3DR :
		{
			cybiko_rs232_pin_txd((data & H8S_P3_TXD1) ? 1 : 0);
			cybiko_rs232_pin_sck((data & H8S_P3_SCK1) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDR :
		{
			m_rtc->scl_w((data & H8S_PF_PF1) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDDR :
		{
			m_rtc->sda_w((data & H8S_PF_PF6) ? 0 : 1);
		}
		break;
	}
#endif
}
#endif

// Cybiko Xtreme writes following byte pairs to 0x200003/0x200000
// 00/01, 00/C0, 0F/32, 0D/03, 0B/03, 09/50, 07/D6, 05/00, 04/00, 20/00, 23/08, 27/01, 2F/08, 2C/02, 2B/08, 28/01
// 04/80, 05/02, 00/C8, 00/C8, 00/C0, 1B/2C, 00/01, 00/C0, 1B/6C, 0F/10, 0D/07, 0B/07, 09/D2, 07/D6, 05/00, 04/00,
// 20/00, 23/08, 27/01, 2F/08, 2C/02, 2B/08, 28/01, 37/08, 34/04, 33/08, 30/03, 04/80, 05/02, 1B/6C, 00/C8
void cybiko_state::cybiko_usb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15) _logerror( 2, ("[%08X] <- %02X\n", 0x200000 + offset * 2 + 0, (data >> 8) & 0xFF));
	if (ACCESSING_BITS_0_7) _logerror( 2, ("[%08X] <- %02X\n", 0x200000 + offset * 2 + 1, (data >> 0) & 0xFF));
}
