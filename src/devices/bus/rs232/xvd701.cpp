// license:BSD-3-Clause
// copyright-holders:smf, DragonMinded, windyfairy

#include "emu.h"
#include "xvd701.h"

#define LOG_COMMAND    (1 << 1)
// #define VERBOSE      (LOG_COMMAND)
// #define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGCMD(...)    LOGMASKED(LOG_COMMAND, __VA_ARGS__)


jvc_xvd701_device::jvc_xvd701_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, JVC_XVD701, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	m_media_type(JVC_MEDIA_VCD), // TODO: This should be changed based on the type of disc inserted or else seeking won't work properly
	m_response_index(0),
	m_timer_response(nullptr)
{
}

void jvc_xvd701_device::device_add_mconfig(machine_config &config)
{
}

static INPUT_PORTS_START(xvd701)
INPUT_PORTS_END

ioport_constructor jvc_xvd701_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xvd701);
}

void jvc_xvd701_device::device_start()
{
	int startbits = 1;
	int databits = 8;
	parity_t parity = PARITY_ODD;
	stop_bits_t stopbits = STOP_BITS_1;

	set_data_frame(startbits, databits, parity, stopbits);

	int txbaud = 9600;
	set_tra_rate(txbaud);

	int rxbaud = 9600;
	set_rcv_rate(rxbaud);

	output_rxd(1);

	// TODO: make this configurable
	output_dcd(0);
	output_dsr(0);
	output_ri(0);
	output_cts(0);

	m_timer_response = timer_alloc(TIMER_RESPONSE);
}

void jvc_xvd701_device::device_reset()
{
	memset(m_command, 0, sizeof(m_command));

	m_response_index = sizeof(m_response);

	m_jlip_id = 33; // Twinkle default
	m_is_powered = false;
	m_chapter = 0;
	m_playback_status = STATUS_STOP;
}

void jvc_xvd701_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_RESPONSE:
		send_response();
		break;

	default:
		break;
	}
}

void jvc_xvd701_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void jvc_xvd701_device::tra_complete()
{
	m_timer_response->adjust(attotime::from_msec(100));
}

unsigned char jvc_xvd701_device::sum(unsigned char *buffer, int length)
{
	int sum = 0x80;

	for (int i = 0; i < length; i++)
		sum -= buffer[i] & 0x7f;

	return sum & 0x7f;
}

void jvc_xvd701_device::create_packet(unsigned char status, const unsigned char response[6])
{
	m_response[0] = 0xfc;
	m_response[1] = 0xff;
	m_response[2] = m_jlip_id;
	m_response[3] = status;
	memcpy(&m_response[4], response, 6);
	m_response[10] = sum(m_response, sizeof(m_response) - 1);

	m_response_index = 0;
	m_timer_response->adjust(attotime::from_msec(100));
}

void jvc_xvd701_device::send_response()
{
	if (m_response_index < sizeof(m_response) && is_transmit_register_empty())
	{
//      printf("sending %02x\n", m_response[m_response_index]);
		transmit_register_setup(m_response[m_response_index++]);
	}
}

bool jvc_xvd701_device::seek_chapter(int chapter)
{
	if (chapter <= 0)
	{
		// Chapters are from 1 and up
		return false;
	}

	m_chapter = chapter;

	if (m_playback_status != STATUS_PAUSE)
		m_playback_status = STATUS_PLAYING;

	return true;
}

void jvc_xvd701_device::rcv_complete()
{
	receive_register_extract();

	for (int i = 0; i < sizeof(m_command) - 1; i++)
		m_command[i] = m_command[i + 1];

	m_command[sizeof(m_command) - 1] = get_received_char();

	if (m_command[0] == 0xff &&
		m_command[1] == 0xff &&
		m_command[10] == sum(m_command, sizeof(m_command) - 1))
	{
		if (m_command[3] == 0x0c && m_command[4] == 0x43 && m_command[5] == 0x6d)
		{
			// FF FF 21 0C 43 6D 00 00 00 00 25 PAUSE
			LOGCMD("xvd701: Playback PAUSE\n");
			m_playback_status = STATUS_PAUSE;
			create_packet(STATUS_OK, NO_RESPONSE);
		}
		else if (m_command[3] == 0x0c && m_command[4] == 0x43 && m_command[5] == 0x75)
		{
			// FF FF 21 0C 43 75 00 00 00 00 1D PLAY
			LOGCMD("xvd701: Playback PLAY\n");

			auto status = STATUS_OK;
			if (m_playback_status == STATUS_STOP)
			{
				// Force video to load again if the video was stopped then started again
				if (!seek_chapter(m_chapter))
					status = STATUS_ERROR;
			}

			if (status == STATUS_OK)
				m_playback_status = STATUS_PLAYING;

			create_packet(status, NO_RESPONSE);
		}
		else if (m_command[3] == 0x0c && m_command[4] == 0x44 && m_command[5] == 0x60)
		{
			// FF FF 21 0C 44 60 00 00 00 00 31 STOP
			LOGCMD("xvd701: Playback STOP\n");

			m_playback_status = STATUS_STOP;
			create_packet(STATUS_OK, NO_RESPONSE);
		}
		else if (m_command[3] == 0x0c && m_command[4] == 0x50 && m_command[5] == 0x20)
		{
			// FF FF 21 0C 50 20 00 00 00 00 63 SEEK TO SPECIFIC CHAPTER
			auto chapter = ((m_command[6] % 10) * 100) + ((m_command[7] % 10) * 10) + (m_command[8] % 10);

			if (m_media_type == JVC_MEDIA_VCD)
			{
				// VCD can only go to 99, so it sticks the data in the first two spots
				chapter /= 10;
			}

			auto status = seek_chapter(chapter);
			LOGCMD("xvd701: Seek chapter %d -> %d\n", chapter, status);
			create_packet(status ? STATUS_OK : STATUS_ERROR, NO_RESPONSE);
		}
		else if (m_command[3] == 0x0c && m_command[4] == 0x50 && m_command[5] == 0x61)
		{
			// FF FF 21 0C 50 61 00 00 00 00 24 PREV (SEEK TO PREVIOUS CHAPTER)
			auto chapter = m_chapter - 1;
			if (m_playback_status != STATUS_PLAYING && chapter == 0)
				chapter = 1;

			auto status = seek_chapter(chapter);
			LOGCMD("xvd701: Seek prev -> %d\n", status);
			create_packet(status ? STATUS_OK : STATUS_ERROR, NO_RESPONSE);
		}
		else if (m_command[3] == 0x0c && m_command[4] == 0x50 && m_command[5] == 0x73)
		{
			// FF FF 21 0C 50 73 00 00 00 00 12 FF (SEEK TO NEXT CHAPTER)
			auto status = seek_chapter(m_chapter + 1);
			LOGCMD("xvd701: Seek FF -> %d\n", status);
			create_packet(status ? STATUS_OK : STATUS_ERROR, NO_RESPONSE);
		}

		else if (m_command[3] == 0x3e && m_command[4] == 0x40 && m_command[5] == 0x60)
		{
			// FF FF 21 3E 40 60 00 00 00 00 03 DEVICE OFF
			LOGCMD("xvd701: Device OFF\n");

			auto status = m_is_powered ? STATUS_OK : STATUS_ERROR;
			if (m_is_powered)
				m_is_powered = false;

			create_packet(status, NO_RESPONSE);

		}
		else if (m_command[3] == 0x3e && m_command[4] == 0x40 && m_command[5] == 0x70)
		{
			// FF FF 21 3E 40 70 00 00 00 00 73 DEVICE ON
			LOGCMD("xvd701: Device ON\n");

			auto status = !m_is_powered ? STATUS_OK : STATUS_ERROR;
			if (!m_is_powered)
				m_is_powered = true;

			create_packet(status, NO_RESPONSE);
		}
		else if (m_command[3] == 0x3e && m_command[4] == 0x4e && m_command[5] == 0x20)
		{
			LOGCMD("xvd701: Device power status request\n");
			const unsigned char response[6] = { m_is_powered, 0x20, 0, 0, 0, 0 };
			create_packet(STATUS_OK, response);
		}
		else if (m_command[3] == 0x7c && m_command[4] == 0x41)
		{
			auto new_id = m_command[5];
			LOGCMD("xvd701: Change JLIP ID to %02x\n", new_id);

			if (new_id > 0 && new_id < 64)
			{
				m_jlip_id = new_id;
				create_packet(STATUS_OK, NO_RESPONSE);
			}
			else
			{
				create_packet(STATUS_ERROR, NO_RESPONSE);
			}
		}
		else if (m_command[3] == 0x7c && m_command[4] == 0x45 && m_command[5] == 0x00)
		{
			LOGCMD("xvd701: Machine code request\n");

			const unsigned char response[6] = { 0x00, 0x01, 0x03, 0x00, 0x03, 0x01 };
			create_packet(STATUS_OK, response);
		}
		else if (m_command[3] == 0x7c && m_command[4] == 0x48 && m_command[5] == 0x20)
		{
			LOGCMD("xvd701: Baud rate request\n");

			// Hardcoded to 9600 baud
			const unsigned char response[6] = { 0x20, 0x00, 0x00, 0x00, 0x00, 0x00 };
			create_packet(STATUS_OK, response);
		}
		else if (m_command[3] == 0x7c && m_command[4] == 0x49 && m_command[5] == 0x00)
		{
			LOGCMD("xvd701: Device code request\n");

			const unsigned char response[6] = { 0x03, 0x0C, 0x7F, 0x7F, 0x7F, 0x7F };
			create_packet(STATUS_OK, response);
		}
		else if (m_command[3] == 0x7c && m_command[4] == 0x4c && m_command[5] == 0x00)
		{
			LOGCMD("xvd701: Device name first half request\n");

			const unsigned char response[6] = { 'D', 'V', 'D', ' ', 'P', 'L' };
			create_packet(STATUS_OK, response);
		}
		else if (m_command[3] == 0x7c && m_command[4] == 0x4d && m_command[5] == 0x00)
		{
			LOGCMD("xvd701: Device name last half request\n");

			const unsigned char response[6] = { 'A', 'Y', 'E', 'R', 0x7F, 0x7F };
			create_packet(STATUS_OK, response);
		}
		else if (m_command[3] == 0x7c && m_command[4] == 0x4e && m_command[5] == 0x20)
		{
			LOGCMD("xvd701: NOP request\n");

			const unsigned char response[6] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };
			create_packet(STATUS_OK, response);
		}
	}
}

DEFINE_DEVICE_TYPE(JVC_XVD701, jvc_xvd701_device, "xvd701", "JVC XV-D701")
