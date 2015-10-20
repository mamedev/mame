// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    Cybiko Wireless Inter-tainment System

    (c) 2001-2007 Tim Schuerewegen

    Cybiko Classic (V1)
    Cybiko Classic (V2)
    Cybiko Xtreme

*/

#include "includes/cybiko.h"

#define LOG_LEVEL  1
#define _logerror(level,x)  do { if (LOG_LEVEL > level) logerror x; } while (0)

#define RAMDISK_SIZE (512 * 1024)


////////////////////////
// DRIVER INIT & EXIT //
////////////////////////

DRIVER_INIT_MEMBER(cybiko_state,cybiko)
{
	_logerror( 0, ("init_cybikov1\n"));
	m_maincpu->space(AS_PROGRAM).install_ram(0x200000, 0x200000 + m_ram->size() - 1, 0, 0x200000 - m_ram->size(), m_ram->pointer());
}

DRIVER_INIT_MEMBER(cybiko_state,cybikoxt)
{
	_logerror( 0, ("init_cybikoxt\n"));
	m_maincpu->space(AS_PROGRAM).install_ram(0x400000, 0x400000 + m_ram->size() - 1, 0, 0x200000 - m_ram->size(), m_ram->pointer());
}

QUICKLOAD_LOAD_MEMBER( cybiko_state, cybiko )
{
	image.fread(m_flash1->get_ptr(), MIN(image.length(), 0x84000));

	return IMAGE_INIT_PASS;
}

QUICKLOAD_LOAD_MEMBER( cybiko_state, cybikoxt )
{
	address_space &dest = m_maincpu->space(AS_PROGRAM);
	UINT32 size = MIN(image.length(), RAMDISK_SIZE);

	dynamic_buffer buffer(size);
	image.fread(&buffer[0], size);
	for (int byte = 0; byte < size; byte++)
		dest.write_byte(0x400000 + byte, buffer[byte]);

	return IMAGE_INIT_PASS;
}

///////////////////
// MACHINE START //
///////////////////

void cybiko_state::machine_start()
{
	_logerror( 0, ("machine_start_cybikov1\n"));
	// serial port
	cybiko_rs232_init();
	// other
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(cybiko_state::machine_stop_cybiko),this));

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
	_logerror( 0, ("machine_reset_cybikov1\n"));
	cybiko_rs232_reset();
}

//////////////////
// MACHINE STOP //
//////////////////

void cybiko_state::machine_stop_cybiko()
{
	_logerror( 0, ("machine_stop_cybikov1\n"));
	// serial port
	cybiko_rs232_exit();
}

///////////
// RS232 //
///////////


void cybiko_state::cybiko_rs232_init()
{
	_logerror( 0, ("cybiko_rs232_init\n"));
	memset( &m_rs232, 0, sizeof(m_rs232));
//  machine().scheduler().timer_pulse(TIME_IN_HZ( 10), FUNC(rs232_timer_callback));
}

void cybiko_state::cybiko_rs232_exit()
{
	_logerror( 0, ("cybiko_rs232_exit\n"));
}

void cybiko_state::cybiko_rs232_reset()
{
	_logerror( 0, ("cybiko_rs232_reset\n"));
}

void cybiko_state::cybiko_rs232_write_byte( int data )
{
//  printf( "%c", data);
}

void cybiko_state::cybiko_rs232_pin_sck( int data )
{
	_logerror( 3, ("cybiko_rs232_pin_sck (%d)\n", data));
	// clock high-to-low
	if ((m_rs232.pin.sck == 1) && (data == 0))
	{
		// transmit
		if (m_rs232.pin.txd) m_rs232.tx_byte = m_rs232.tx_byte | (1 << m_rs232.tx_bits);
		m_rs232.tx_bits++;
		if (m_rs232.tx_bits == 8)
		{
			m_rs232.tx_bits = 0;
			cybiko_rs232_write_byte(m_rs232.tx_byte);
			m_rs232.tx_byte = 0;
		}
		// receive
		m_rs232.pin.rxd = (m_rs232.rx_byte >> m_rs232.rx_bits) & 1;
		m_rs232.rx_bits++;
		if (m_rs232.rx_bits == 8)
		{
			m_rs232.rx_bits = 0;
			m_rs232.rx_byte = 0;
		}
	}
	// save sck
	m_rs232.pin.sck = data;
}

void cybiko_state::cybiko_rs232_pin_txd( int data )
{
	_logerror( 3, ("cybiko_rs232_pin_txd (%d)\n", data));
	m_rs232.pin.txd = data;
}

int cybiko_state::cybiko_rs232_pin_rxd()
{
	_logerror( 3, ("cybiko_rs232_pin_rxd\n"));
	return m_rs232.pin.rxd;
}

int cybiko_state::cybiko_rs232_rx_queue()
{
	return 0;
}

/////////////////////////
// READ/WRITE HANDLERS //
/////////////////////////

READ16_MEMBER( cybiko_state::cybiko_lcd_r )
{
	UINT16 data = 0;
	if (ACCESSING_BITS_8_15) data |= (m_crtc->reg_idx_r(space, offset) << 8);
	if (ACCESSING_BITS_0_7) data |= (m_crtc->reg_dat_r(space, offset) << 0);
	return data;
}

WRITE16_MEMBER( cybiko_state::cybiko_lcd_w )
{
	if (ACCESSING_BITS_8_15) m_crtc->reg_idx_w(space, offset, (data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7) m_crtc->reg_dat_w(space, offset, (data >> 0) & 0xff);
}

int cybiko_state::cybiko_key_r( offs_t offset, int mem_mask)
{
	UINT16 data = 0xFFFF;
	for (UINT8 i = 0; i < 15; i++)
	{
		if (m_input[i] && !BIT(offset, i))
			data &= ~m_input[i]->read();
	}
	if (data != 0xFFFF)
	{
		_logerror( 1, ("cybiko_key_r (%08X/%04X) %04X\n", offset, mem_mask, data));
	}
	return data;
}

READ16_MEMBER( cybiko_state::cybikov1_key_r )
{
	return cybiko_key_r(offset, mem_mask);
}

READ16_MEMBER( cybiko_state::cybikov2_key_r )
{
	UINT16 data = cybiko_key_r(offset, mem_mask);
	if (!BIT(offset, 0))
		data |= 0x0002; // or else [esc] does not work in "lost in labyrinth"
	return data;
}

READ16_MEMBER( cybiko_state::cybikoxt_key_r )
{
	return cybiko_key_r(offset, mem_mask);
}

#if 0
READ16_MEMBER( cybiko_state::cybikov1_io_reg_r )
{
	UINT16 data = 0;
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

READ16_MEMBER( cybiko_state::cybikov2_io_reg_r )
{
	UINT16 data = 0;
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

READ16_MEMBER( cybiko_state::cybikoxt_io_reg_r )
{
	UINT16 data = 0;
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

WRITE16_MEMBER( cybiko_state::cybikov1_io_reg_w )
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

WRITE16_MEMBER( cybiko_state::cybikov2_io_reg_w )
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

WRITE16_MEMBER( cybiko_state::cybikoxt_io_reg_w )
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
WRITE16_MEMBER( cybiko_state::cybiko_usb_w )
{
	if (ACCESSING_BITS_8_15) _logerror( 2, ("[%08X] <- %02X\n", 0x200000 + offset * 2 + 0, (data >> 8) & 0xFF));
	if (ACCESSING_BITS_0_7) _logerror( 2, ("[%08X] <- %02X\n", 0x200000 + offset * 2 + 1, (data >> 0) & 0xFF));
}
