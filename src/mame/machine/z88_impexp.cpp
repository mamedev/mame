// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Z88 Imp-Export protocol

    Send to Z88:
    - Launch Imp-Export popdown on Z88.
    - On Z88, select batch receive by pressing b and then the Enter key.
    - Go in the MAME file manager and load the file to import in the serial device.

    Receive from Z88:
    - Launch Imp-Export popdown on Z88.
    - Go in the MAME file manager and create a new file in the serial device.
    - On Z88, select send by pressing s and then the Enter key.
    - Enter the filename to transfer, and then the Enter key.
    - When transfer is complete go in the MAME file manager and unload the image.

    Protocol:
    - ESC + N: start of file name
    - ESC + F: start of file data
    - ESC + E: end of file
    - ESC + Z: end of batch
    - ESC + BXX: escaped byte, where XX is the uppercase hexadecimal

*********************************************************************/

#include "emu.h"
#include "z88_impexp.h"

static constexpr uint32_t Z88_RS232_BAUD = 9600;

// device type definition
DEFINE_DEVICE_TYPE(Z88_IMPEXP, z88_impexp_device, "z88_impexp", "Z88 Imp-Export protocol");

z88_impexp_device::z88_impexp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, Z88_IMPEXP, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, device_rs232_port_interface(mconfig, *this)
	, device_image_interface(mconfig, *this)
	, m_timer_poll(nullptr)
	, m_mode(MODE_IDLE)
	, m_rts(1)
	, m_dtr(1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z88_impexp_device::device_start()
{
	m_timer_poll = timer_alloc();
}

//-------------------------------------------------
//  device_reset - reset up the device
//-------------------------------------------------

void z88_impexp_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(Z88_RS232_BAUD);

	output_rxd(1);
	output_dcd(0);
	output_dsr(0);
	output_cts(0);

	m_rts = 1;
	m_dtr = 1;
	m_queue = std::queue<uint8_t>();
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void z88_impexp_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	queue();
}

WRITE_LINE_MEMBER( z88_impexp_device::input_rts )
{
	if (!state && m_rts)
	{
		m_rts = state;
		queue();
	}
	else
		m_rts = state;
}

void z88_impexp_device::queue()
{
	if (is_transmit_register_empty())
	{
		if (!m_queue.empty() && m_rts == 0)
		{
			transmit_register_setup(m_queue.front());
			m_queue.pop();
			m_timer_poll->adjust(attotime::never);
			return;
		}

		m_timer_poll->adjust(attotime::from_hz(Z88_RS232_BAUD));
	}
}

void z88_impexp_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void z88_impexp_device::tra_complete()
{
	if (m_mode == MODE_SEND)
		queue();
}

void z88_impexp_device::rcv_complete()
{
	receive_register_extract();
	if (m_mode == MODE_RECV)
		m_queue.push(get_received_char());
}


void z88_impexp_device::check_filename(std::string &filename)
{
	for (auto &c : filename)
		if (c != '-' && !std::isalnum(c))
			c = '-';
}


image_init_result z88_impexp_device::call_load()
{
	m_mode = MODE_SEND;
	m_queue = std::queue<uint8_t>();

	std::string name;
	if (basename())
	{
		if (basename_noext())
			name = basename_noext();
		else
			name = basename();

		if (name.length() > 12)
			name.resize(12);

		check_filename(name);

		if (filetype().length())
		{
			std::string ext = filetype();
			if (ext.length() > 3)
				ext.resize(3);

			check_filename(ext);
			name += "." + ext;
		}
	}
	else
		name = "UNKNOWN";

	// file name
	m_queue.push(0x1b);
	m_queue.push('N');
	for (char const &c: name)
		m_queue.push(c);

	// file data
	m_queue.push(0x1b);
	m_queue.push('F');
	while (!image_feof())
	{
		uint8_t b;
		if (fread(&b, 1) != 1)
			return image_init_result::FAIL;

		// Escape non printable characters
		if ((b < 0x20 || b >= 0x7f) && b != 0x0a && b != 0x0d && b != 0x09)
		{
			m_queue.push(0x1b);
			m_queue.push('B');
			for (int i = 4; i >= 0; i -= 4)
			{
				uint8_t n = (b >> i) & 0x0f;
				if (n < 10)
					m_queue.push('0' + n);
				else
					m_queue.push('A' + n - 10);
			}
		}
		else
			m_queue.push(b);
	}

	// end of file
	m_queue.push(0x1b);
	m_queue.push('E');
	queue();

	return image_init_result::PASS;
}


image_init_result z88_impexp_device::call_create(int format_type, util::option_resolution *format_options)
{
	m_queue = std::queue<uint8_t>();
	m_mode = MODE_RECV;
	return image_init_result::PASS;
}


void z88_impexp_device::call_unload()
{
	if (m_mode == MODE_RECV && !m_queue.empty())
	{
		std::string name;
		char mode = 0;
		while (!m_queue.empty())
		{
			uint8_t b = m_queue.front();
			m_queue.pop();
			if (b != 0x1b)
			{
				if (mode == 'F')
					fwrite(&b, 1);
				else if (mode == 'N')
					name.push_back(b);
			}
			else if (!m_queue.empty())
			{
				b = m_queue.front();
				m_queue.pop();
				if      (b == 'N')  mode = 'N';     // File name
				else if (b == 'F')  mode = 'F';     // File data
				else if (b == 'E')  break;          // End of file
				else if (b == 'Z')  break;          // End of Batch
				else if (b == 'B')                  // Escaped byte
				{
					uint8_t val = 0;
					for (int i = 4; !m_queue.empty() && i >= 0; i -= 4)
					{
						b = m_queue.front();
						m_queue.pop();
						if (b >= '0' && b <= '9')
							val |= (b - '0') << i;
						else if (b >= 'A' && b <= 'F')
							val |= (b - 'A' + 10) << i;
						else
							logerror("Invalid escaped byte 0x%02x", b);
					}

					if (mode == 'F')
						fwrite(&val, 1);
				}
				else
					logerror("Unknown escape 0x%02x\n", b);
			}
		}

		logerror("Received file '%s'\n", name);
	}

	m_queue = std::queue<uint8_t>();
	m_mode = MODE_IDLE;
}
