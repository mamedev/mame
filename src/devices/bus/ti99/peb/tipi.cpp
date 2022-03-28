// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TIPI adapter card
    designed and realized by Matthew Splett (aka jedimatt42), Peter and others

    The TIPI system (pronouced "tipee") is an expansion for the TI-99/4A and
    other computers of this family. It consists of a Raspberry Pi computer
    attached to an adapter card that fits into the PEB (Peripheral Expansion
    Box) or to the console side connector.

    The adapter card provides an interface between the TI system and the
    Raspberry Pi, in particular for exchanging files. More than that, features
    of the Raspberry like network connectivity, may be offered to the TI
    system.

    The general concept is that there is a set of memory-mapped ports between
    the TI and the Raspberry Pi.

    0x5ff9 (RCIN):   PI Control Signal (input from RPi)
    0x5ffb (RDIN):   PI Data (input)
    0x5ffd (TCOUT):  TI Control Signal (output to RPi)
    0x5fff (TDOUT):  TI Data (output)

    CRU map
    -------
    1x00: Activate the card and its ROM  (x=0...f)
    1x02: Send RESET to Raspberry Pi


    Raspberry connection
    --------------------
    The good thing in terms of emulation is that the Raspberry Pi is completely
    outside of the emulation. It allows for connecting via a WebSocket, which
    is especially provided for emulators. In the real machine, the Raspberry Pi
    gets the data from the adapter card.

    It is recommended to run a virtual Raspberry Pi as a QEMU image. An image
    is available on the TIPI website.

    The connection is specified by -conn rpi.<hostname>[:<port>],
    like "-conn rpi.localhost" or "-conn rpi.some.host.net"

    This will create a file in the current directory of that name, which is
    a side effect of the MAME image handling. The port default is 9901.


    Mouse support
    -------------
    The websocket server accepts messages for mouse movements and button actions
    from the mouse input of the connected emulator. However, in MAME, mouse
    input is only available if there is a proper mouse device, like the busmouse
    or the Mechatronics mouse. Hence, we cannot send any mouse input to the
    Raspberry without one of these mice connected.

    However, if a mouse is connected to the real or virtual Raspberry Pi,
    the mouse operations can be polled from the Raspberry in the
    intended way.

    Michael Zapf
    March 20222

    References

    [1] TIPI description on Github: https://github.com/jedimatt42/tipi

    TODO: Add ASYNC mode

*****************************************************************************/

#include "emu.h"
#include "tipi.h"

#define LOG_WARN       (1U<<1)
#define LOG_EPROM      (1U<<2)
#define LOG_CRU        (1U<<3)
#define LOG_PORTS      (1U<<4)
#define LOG_CONFIG     (1U<<5)
#define LOG_RPI        (1U<<6)

#define VERBOSE ( LOG_GENERAL | LOG_WARN | LOG_CONFIG )
#define RASPI "rpi"

#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_TIPI, bus::ti99::peb::tipi_card_device, "ti99_tipi", "TIPI card")
DEFINE_DEVICE_TYPE(TI99_TIPI_RPI, bus::ti99::peb::tipi_attached_device, "ti99_tipi_atttached", "Connection to external device")

namespace bus::ti99::peb {

tipi_card_device::tipi_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_TIPI, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_rpi(*this, RASPI),
	m_address(0),
	m_dsr(false),
	m_portaccess(false)
{
}

void tipi_card_device::setaddress_dbin(offs_t offset, int state)
{
	if (machine().side_effects_disabled()) return;
	m_address = offset;
	m_dsr = false;
	m_portaccess = false;

	if (in_dsr_space(offset, true) && m_selected)
	{
		m_portaccess = ((offset & 0x1ff8)==0x1ff8);
		m_dsr = !m_portaccess;
	}
}

void tipi_card_device::readz(offs_t offset, uint8_t *value)
{
	if (machine().side_effects_disabled())
	{
		debug_read(offset, value);
		return;
	}

	if (m_dsr)
	{
		// Lines A0-A12 directly connected to the EPROM (chip pin order)
		// Lines A13-A15 connected to PLD
		int base = 0;
		uint8_t* rom = &m_eprom[base | (m_address & 0x1fff)];
		*value = *rom;

		if (WORD_ALIGNED(m_address))
		{
			// Do logging by 16 bit words, as we typically have instructions in
			// the eprom
			uint16_t val = (*rom << 8) | (*(rom+1));
			LOGMASKED(LOG_EPROM, "DSR: %04x -> %04x\n", m_address & 0xffff, val);
		}
	}

	if (m_portaccess)
	{
		if ((m_address & 5)==1)
		{
			int val = 0;
			if (m_address & 2)
			{
				val = m_rd;
				LOGMASKED(LOG_PORTS, "RDIN -> %02x\n", val);
			}
			else
			{
				val = m_rc;
				LOGMASKED(LOG_PORTS, "RCIN -> %02x\n", val);
			}
			*value = val;
		}
	}
}

void tipi_card_device::write(offs_t offset, uint8_t data)
{
	if (machine().side_effects_disabled())
	{
		debug_write(offset, data);
		return;
	}

	if (m_dsr)
	{
		LOGMASKED(LOG_WARN, "Writing to DSR space: %04x <- %02x\n", m_address & 0xffff, data);
	}

	if (m_portaccess)
	{
		if (m_address & 1)
		{
			if (m_address & 2)
			{
				LOGMASKED(LOG_PORTS, "TDOUT <- %02x\n", data);
				set_td(data);
			}
			else
			{
				LOGMASKED(LOG_PORTS, "TCOUT <- %02x\n", data);
				set_tc(data);
			}
		}
	}
}

void tipi_card_device::set_td(u8 data)
{
	m_td = data;
}

void tipi_card_device::set_tc(u8 data)
{
	if (m_tc != data)
	{
		m_tc = data;
		process_message();
	}
}

/*
    Debugger access.
*/
void tipi_card_device::debug_read(offs_t offset, uint8_t* value)
{
}

/*
    Debugger access.
*/
void tipi_card_device::debug_write(offs_t offset, uint8_t data)
{
}

/*
    CRU read access
*/
void tipi_card_device::crureadz(offs_t offset, uint8_t *value)
{
}

/*
    CRU write access.
*/
void tipi_card_device::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xff00) == m_cru_base)
	{
		int bit = (offset >> 1) & 0x1f;
		switch (bit)
		{
		case 0:           // card activation
			LOGMASKED(LOG_CRU, "DSR %s\n", (data!=0)? "on" : "off");
			m_selected = (data != 0);
			break;
		case 1:
			LOGMASKED(LOG_CRU, "CRU Bit 1 %s\n", (data!=0)? "on" : "off");
			if (data != 0) send("RESET");
			break;
		default:
			LOGMASKED(LOG_CRU, "Unknown CRU Bit %d\n", bit);
			break;
		}
	}
}

void tipi_card_device::send(const char* message)
{
	*m_send_stream << message;
	m_wsclient->send(m_send_stream);
}

void tipi_card_device::send(u8* message, int len)
{
	for (int i=0; i < len; i++)
	{
		LOGMASKED(LOG_RPI, "Sending byte %02x\n", message[i]);
		*m_send_stream << message[i];
	}
	m_wsclient->send(m_send_stream, nullptr, 130);  // binary (10000010)
}

enum
{
	TSWB = 0x02,
	TSRB = 0x06,
	TSRSET = 0xf1
};

/*
    Messages from server:
    "RD=xxx"
    "RC=xxx"

    processMessage occurs for each incoming message
    and each time that TC has been changed

    Write a message to RPi:

    for i=-2..(n-1)
       Write byte to TD
       Write (02 | (i & 1)) to TC

    Byte sequence: len1 len2 m0 m1 m2 m3 ... m(n-1), where n = len1<<8 | len2;

    Message will be written to RPi when complete


    Read a message from RPi:

    n = message.length
    n = n1<<8 | n2

    Write 06 to TC
    n1 = Read byte from TD
    Write 07 to TC
    n2 = Read byte from TD

    for i=0..(n-1)
       Write (06 | (i & 1)) to TC
       Read byte from TD

*/
void tipi_card_device::process_message()
{
	if (m_tc == TSRSET)   // Reset command
	{
		m_rpimessage = nullptr;
		m_msgindex = -2;
		m_rc = m_tc;
		send("SYNC");
	}
	else
	{
		if ((m_tc & 0xfe) == TSWB)    // 02 and 03: write byte (LSB as strobe)
		{
			if (m_msgindex == -2)
				m_msglength = m_td << 8;
			else
			{
				if (m_msgindex == -1)
				{
					m_msglength |= m_td;
					m_rpimessage = std::make_unique<u8[]>(m_msglength);
				}
				else
				{
					if (m_rpimessage != nullptr)
						m_rpimessage[m_msgindex] = m_td;
				}
			}

			m_msgindex++;
			if (m_msgindex == m_msglength)
			{
				// Message is complete - transmit it
				LOGMASKED(LOG_RPI, "Sending message, length %d\n", m_msglength);
				send(m_rpimessage.get(), m_msglength);
				m_rpimessage = nullptr;
			}
			m_rc = m_tc;   // Auto-acknowledge
		}
		else
		{
			if ((m_tc & 0xfe) == TSRB)    // 06 and 07: read byte (LSB as strobe)
			{
				if (m_rpimessage != nullptr)
				{
					if (m_msgindex == -2)
						m_rd = (m_msglength >> 8)& 0xff;
					else
					{
						if (m_msgindex == -1)
							m_rd = m_msglength & 0xff;
						else
						{
							m_rd = m_rpimessage[m_msgindex];
						}
					}
					m_msgindex++;
					m_rc = m_tc;   // Auto-acknowledge
				}
			}
		}
	}
}

/*
    Syntax for connecting is
    -conn rpi.<host>[:<port>]

    Default for port is 9901.
*/
void tipi_card_device::open_websocket()
{
	if (m_rpi->exists())
	{
		// We are just interested in the file name
		// Cannot call it "socket.xxx" because this will make sdlsocket open
		// the connection
		std::string sconn(m_rpi->filename());
		int pos = sconn.find("rpi.");

		if (pos != -1)
		{
			std::string shost(sconn.substr(pos + 4));
			if (shost.find(":")==-1)
				shost += ":9901";

			shost += "/tipi";

			LOGMASKED(LOG_CONFIG, "Connection to Raspberry Pi at %s\n", shost.c_str());

			m_wsclient = std::make_unique<webpp::ws_client>(shost);
			webpp::ws_client* wsc = m_wsclient.get();
			wsc->on_open =
				[this]()
				{ websocket_opened(); };

			wsc->on_message =
				[this](std::shared_ptr<webpp::ws_client::Message> message)
				{  websocket_incoming(message); };

			wsc->on_close =
				[this](int i, const std::string& msg)
				{  websocket_closed(i, msg); };

			wsc->on_error =
				[this](const std::error_code& code)
				{  websocket_error(code); };

			m_send_stream = std::make_shared<webpp::ws_client::SendStream>();

			asio::thread listener([this]()
				{   m_wsclient->start(); });
		}
		else
			fatalerror("Invalid TIPI connection address. Syntax: -conn rpi.<host>[:<port>]\n");
	}
	else
		LOGMASKED(LOG_CONFIG, "No Raspberry Pi connected\n");
}

void tipi_card_device::websocket_opened()
{
	LOGMASKED(LOG_RPI, "Opening connection\n");
	m_connected = true;
}

void tipi_card_device::websocket_incoming(std::shared_ptr<webpp::ws_client::Message> message)
{
	switch (message->fin_rsv_opcode & 0x0f)
	{
	case 1:
		// Text message is "RD=xxx" or "RC=xxx" with xxx=decimal number
		LOGMASKED(LOG_RPI, "Got text message: %s\n", message->string());
		if (message->string().find("RD=")==0)
		{
			m_rd = std::stoi(message->string().substr(3));
		}
		if (message->string().find("RC=")==0)
		{
			m_rc = std::stoi(message->string().substr(3));
		}
		break;

	case 2:
		LOGMASKED(LOG_RPI, "Got binary message, length %d\n", message->size());
		m_rpimessage = std::make_unique<u8[]>(message->size());
		for (int i=0; i < message->size(); i++)
			m_rpimessage[i] = (u8)message->get();

		m_msglength = message->size();
		m_msgindex = -2;
		process_message();
		break;

	default:
		LOGMASKED(LOG_RPI, "Got unknown message\n");
		break;
	}
}

void tipi_card_device::websocket_error(const std::error_code& code)
{
	if (code.value() == std::make_error_code(std::errc::connection_reset).value())
	{
		m_wsclient->stop();
		if (m_attempts > 0)
		{
			m_attempts--;
			LOGMASKED(LOG_CONFIG, "Error, connection reset. Retrying.\n");
			m_restart_timer->adjust(attotime::from_msec(1000));
		}
		else
		{
			LOGMASKED(LOG_CONFIG, "Connection is reset on setup several times; giving up\n");
			m_connected = false;
		}
	}
	else
	{
		LOGMASKED(LOG_RPI, "Got error: %s (%d)\n", code.message().c_str(), code.value());
	}
}

/*
    Restart the websocket connection
*/
void tipi_card_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	open_websocket();
}


void tipi_card_device::websocket_closed(int i, const std::string& msg)
{
	LOGMASKED(LOG_RPI, "Closing connection\n");
	// Open it again after 2 secs
	m_wsclient->stop();
	m_restart_timer->adjust(attotime::from_msec(3000));
}

void tipi_card_device::device_start()
{
	m_eprom = memregion(TI99_DSRROM)->base();
	save_item(NAME(m_address));
	save_item(NAME(m_dsr));
	save_item(NAME(m_portaccess));
	save_item(NAME(m_msgindex));
	save_item(NAME(m_msglength));
	save_item(NAME(m_attempts));
	save_item(NAME(m_connected));
	save_item(NAME(m_tc));
	save_item(NAME(m_td));
	save_item(NAME(m_rc));
	save_item(NAME(m_rd));

	m_restart_timer = timer_alloc(0);
	m_attempts = 5;
}

void tipi_card_device::device_reset()
{
	m_cru_base = (ioport("SW1")->read()) << 8;
	m_address = 0;
	m_dsr = false;
	m_portaccess = false;
	open_websocket();
	m_connected = false;
}

/*
    The CRU address base for the card.
*/
INPUT_PORTS_START( tipi )
	PORT_START("SW1")
	PORT_DIPNAME(0x1f, 0x10, "TIPI card CRU base")
		PORT_DIPSETTING(0x10, "1000") // Default setting
		PORT_DIPSETTING(0x11, "1100")
		PORT_DIPSETTING(0x12, "1200")
		PORT_DIPSETTING(0x13, "1300")
		PORT_DIPSETTING(0x14, "1400")
		PORT_DIPSETTING(0x15, "1500")
		PORT_DIPSETTING(0x16, "1600")
		PORT_DIPSETTING(0x17, "1700")
		PORT_DIPSETTING(0x18, "1800")
		PORT_DIPSETTING(0x19, "1900")
		PORT_DIPSETTING(0x1a, "1a00")
		PORT_DIPSETTING(0x1b, "1b00")
		PORT_DIPSETTING(0x1c, "1c00")
		PORT_DIPSETTING(0x1d, "1d00")
		PORT_DIPSETTING(0x1e, "1e00")
		PORT_DIPSETTING(0x1f, "1f00")
INPUT_PORTS_END

void tipi_card_device::device_add_mconfig(machine_config &config)
{
	TI99_TIPI_RPI(config, m_rpi, 0);
}

ROM_START( tipi )
	ROM_REGION(0x8000, TI99_DSRROM, 0)
	ROM_LOAD("tipidsr.u2", 0x0000, 0x8000, CRC(a54e65af) SHA1(f930e4b079a5b6b24dc20f262e9c599d2051b6be))
ROM_END


const tiny_rom_entry *tipi_card_device::device_rom_region() const
{
	return ROM_NAME( tipi );
}

ioport_constructor tipi_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tipi );
}

/****************************************************************************/

/* Connection to the external device, a Raspberry PI */

tipi_attached_device::tipi_attached_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TI99_TIPI_RPI, tag, owner, clock),
	device_image_interface(mconfig, *this)
{
}

/*
    Initialize connection
*/
image_init_result tipi_attached_device::call_load()
{
	return image_init_result::PASS;  // OK
}

void tipi_attached_device::call_unload()
{
}


} // end namespace bus::ti99::peb
