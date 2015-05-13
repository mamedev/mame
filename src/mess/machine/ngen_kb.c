// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent NGEN keyboard device

#include "ngen_kb.h"

ngen_keyboard_device::ngen_keyboard_device(const machine_config& mconfig, const char* tag, device_t* owner, UINT32 clock) :
	serial_keyboard_device(mconfig, NGEN_KEYBOARD, "NGEN Keyboard", tag, owner, 0, "ngen_keyboard", __FILE__),
	m_keys_down(false)
{
}


void ngen_keyboard_device::write(UINT8 data)
{
	// To be figured out
	// Code 0x92 is sent on startup, perhaps resets the keyboard MCU
	// Codes 0xAx and 0xBx appear to control the keyboard LEDs, lower nibbles controlling the state of the LEDs
	// When setting an error code via the LEDs, 0xB0 then 0xAE is sent (presumably for error code 0xE0),
	// so that means that 0xAx controls the Overtype, Lock, F1 and F2 LEDs, and 0xBx controls the F3, F8, F9 and F10 LEDs.
	logerror("KB: received character %02x\n",data);
	switch(data)
	{
		case 0x92:  // reset(?)
			m_last_reset = true;
			break;
	}
}

UINT8 ngen_keyboard_device::row_number(UINT8 code)
{
	if BIT(code,0) return 0;
	if BIT(code,1) return 1;
	if BIT(code,2) return 2;
	if BIT(code,3) return 3;
	if BIT(code,4) return 4;
	if BIT(code,5) return 5;
	if BIT(code,6) return 6;
	if BIT(code,7) return 7;
	return 0;
}

UINT8 ngen_keyboard_device::keyboard_handler(UINT8 last_code, UINT8 *scan_line)
{
	int i;
	UINT8 code = 0;
	UINT8 key_code = 0;
	UINT8 retVal = 0x00;
	UINT8 shift = BIT(m_io_kbdc->read(), 1);
	UINT8 caps  = BIT(m_io_kbdc->read(), 2);
	UINT8 ctrl  = BIT(m_io_kbdc->read(), 0);
	i = *scan_line;
	{
		if (i == 0) code = m_io_kbd0->read();
		else
		if (i == 1) code = m_io_kbd1->read();
		else
		if (i == 2) code = m_io_kbd2->read();
		else
		if (i == 3) code = m_io_kbd3->read();
		else
		if (i == 4) code = m_io_kbd4->read();
		else
		if (i == 5) code = m_io_kbd5->read();
		else
		if (i == 6) code = m_io_kbd6->read();
		else
		if (i == 7) code = m_io_kbd7->read();
		else
		if (i == 8) code = m_io_kbd8->read();
		else
		if (i == 9) code = m_io_kbd9->read();

		if (code != 0)
		{
			if (i==0 && shift==0) {
				key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
			}
			if (i==0 && shift==1) {
				key_code = 0x20 + row_number(code) + 8*i; // for shifted numbers
			}
			if (i==1 && shift==0) {
				if (row_number(code) < 4) {
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
				} else {
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
				}
			}
			if (i==1 && shift==1) {
				if (row_number(code) < 4) {
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
				} else {
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
				}
			}
			if (i>=2 && i<=4 && (shift ^ caps)==0 && ctrl==0) {
				key_code = 0x60 + row_number(code) + (i-2)*8; // for small letters
			}
			if (i>=2 && i<=4 && (shift ^ caps)==1 && ctrl==0) {
				key_code = 0x40 + row_number(code) + (i-2)*8; // for big letters
			}
			if (i>=2 && i<=5 && ctrl==1) {
				key_code = 0x00 + row_number(code) + (i-2)*8; // for CTRL + letters
			}
			if (i==5 && shift==1 && ctrl==0) {
				if (row_number(code)<7) {
					if (row_number(code)<3) {
						key_code = (caps ? 0x60 : 0x40) + row_number(code) + (i-2)*8; // for big letters
					} else {
						key_code = 0x60 + row_number(code) + (i-2)*8; // for upper symbols letters
					}
				} else {
					key_code = 0x40 + row_number(code) + (i-2)*8; // for DEL it is switched
				}
			}
			if (i==5 && shift==0 && ctrl==0) {
				if (row_number(code)<7) {
					if (row_number(code)<3) {
						key_code = (caps ? 0x40 : 0x60) + row_number(code) + (i-2)*8; // for small letters
					} else {
						key_code = 0x40 + row_number(code) + (i-2)*8; // for lower symbols letters
					}
				} else {
					key_code = 0x60 + row_number(code) + (i-2)*8; // for DEL it is switched
				}
			}

			if (i==6) {
				switch(row_number(code))
				{
/*                  case 0: key_code = 0x11; break;
                    case 1: key_code = 0x12; break;
                    case 2: key_code = 0x13; break;
                    case 3: key_code = 0x14; break;*/
					case 4: key_code = 0x20; break; // Space
					case 5: key_code = 0x0A; break; // LineFeed
					case 6: key_code = 0x09; break; // TAB
					case 7: key_code = 0x0D; break; // Enter
				}
			}
			if (i==7)
			{
				switch(row_number(code))
				{
					case 0: key_code = 0x1B; break; // Escape
					case 1: key_code = 0x08; break; // Backspace
				}
			}
			else
			if (i==8)
			{
				key_code = row_number(code)+0x81;
				if (ctrl) key_code+=0x10;
				if (shift) key_code+=0x20;
			}
			else
			if (i==9)
			{
				key_code = row_number(code)+0x89;
				if (ctrl) key_code+=0x10;
				if (shift) key_code+=0x20;
			}
			m_keys_down = true;
			m_last_reset = false;
			retVal = key_code;
		}
		else
		{
			*scan_line += 1;
			if (*scan_line==10)
				*scan_line = 0;
			if(m_keys_down)
			{
				retVal = 0xc0;
				m_keys_down = false;
			}
		}
	}
	return retVal;
}

static INPUT_PORTS_START( ngen_keyboard )
	PORT_INCLUDE(generic_keyboard)

	PORT_START("RS232_TXBAUD")
	PORT_CONFNAME(0xff, RS232_BAUD_19200, "TX Baud") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_BAUD_19200, "19200") // TODO: Based on the RAM refresh timer (~78kHz) to be 19530Hz

	PORT_START("RS232_STARTBITS")
	PORT_CONFNAME(0xff, RS232_STARTBITS_1, "Start Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_STARTBITS_1, "1")

	PORT_START("RS232_DATABITS")
	PORT_CONFNAME(0xff, RS232_DATABITS_8, "Data Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_DATABITS_8, "8")

	PORT_START("RS232_PARITY")
	PORT_CONFNAME(0xff, RS232_PARITY_NONE, "Parity") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_PARITY_NONE, "None")

	PORT_START("RS232_STOPBITS")
	PORT_CONFNAME(0xff, RS232_STOPBITS_2, "Stop Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_STOPBITS_2, "2")
INPUT_PORTS_END


ioport_constructor ngen_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ngen_keyboard);
}

void ngen_keyboard_device::device_start()
{
	serial_keyboard_device::device_start();
	set_rcv_rate(19200);
}

void ngen_keyboard_device::device_reset()
{
	serial_keyboard_device::device_reset();
	m_keys_down = false;
	m_last_reset = true;
}

void ngen_keyboard_device::rcv_complete()
{
	receive_register_extract();
	write(get_received_char());
}

const device_type NGEN_KEYBOARD = &device_creator<ngen_keyboard_device>;
