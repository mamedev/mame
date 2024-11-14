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

    0x5ff9 (RC):  PI Control Signal (input from RPi)
    0x5ffb (RD):  PI Data (input)
    0x5ffd (TC):  TI Control Signal (output to RPi)
    0x5fff (TD):  TI Data (output)

    CRU map
    -------
    1x00: Activate the card and its ROM  (x=0...f)
    1x02: Send RESET to Raspberry Pi

    TI-TIPI protocol [2]
    --------------------

       Byte layer protocol
       ----------------
       Reset-sync:
          TI: rs -> TC            rs = f1
          PI: (TC==rs) rs -> RC
          PIemu: (TC==rs) rs -> RC; (mode==sync) send("SYNC")

       Write-byte(byte):
          TI: byte -> TD
              wb   -> TC          wb = 02 / 03 (alternating)
          PI: (TC==wb) send (TD); (TC) -> RC
          PIemu: (TC==wb) q.append(TD); (TC) -> RC


       byte=Read-byte:
          TI: rb -> TC            rb = 06 / 07 (alternating)
              (RC==rb) return (RD)
          PI: (TC==rb) byte -> RD; (TC) -> RC
          PIemu: (TC==rb) (q.length > 0) q.pop -> RD; (TC) -> RC

       Message layer protocol
       ----------------------
       Send-message(c1,c2,...,cn), n = (n1<<8)+n2:
          Write-byte(n1)
          Write-byte(n2)
          for i=1..n: Write-byte(c[i])

       (c1,...cn) = Receive-message:
          n1=Read-byte
          n2=Read-byte
          for i=1..(n1<<8)+n2: c[i] = Read-byte

       Communication layer protocol
       ----------------------------
       Communication:
          Reset-sync
          Send-message(ms)
          rm = Receive-message

       Emulation adaptation
       --------------------
       Send-messageEmu(c1,c2,...,cn), n = (n1<<8)+n2:
          Write-byte(n1)
          Write-byte(n2)
          for i=1..n: Write-byte(c[i])
          send(q.content)


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

    Logging
    -------
    Since we use a background thread for the websocket client, and logging
    is not thread-safe, log output may look broken at some times (in particular
    when setting the LOG_PORTS flag).


    Michael Zapf
    March 20222

    References

    [1] TIPI description on Github: https://github.com/jedimatt42/tipi
    [2] TIPI Protocol: https://github.com/jedimatt42/tipi/wiki/TIPI-Protocol

*****************************************************************************/

#include "emu.h"
#include "tipi.h"

#define LOG_WARN       (1U << 1)
#define LOG_EPROM      (1U << 2)
#define LOG_CRU        (1U << 3)
#define LOG_PORTS      (1U << 4)
#define LOG_RPI        (1U << 5)
#define LOG_RECV       (1U << 6)
#define LOG_SEND       (1U << 7)
#define LOG_PROT       (1U << 8)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
#define RASPI "rpi"

#define PAUSEBASE 1000

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
	m_portaccess(false),
	m_waitinit(false),
	m_syncmode(true),
	m_connected(false),
	m_rpiconn(false),
	m_pausetime(PAUSEBASE),
	m_pending_read(false),
	m_tc(0),
	m_rd(0)
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
		// Lines A13-A15 connected to PLD (currently not used)
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
				val = get_rd();
				LOGMASKED(LOG_PORTS, "RD -> %02x\n", m_rd);
			}
			else
			{
				val = m_rc;
				LOGMASKED(LOG_PORTS, "RC -> %02x\n", val);
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
				LOGMASKED(LOG_PORTS, "TD <- %02x\n", data);
				m_td = data;
			}
			else
			{
				set_tc(data);
			}
		}
	}
}

/*
    Debugger access.
*/
void tipi_card_device::debug_read(offs_t offset, uint8_t* value)
{
	int base = 0;
	if (in_dsr_space(offset, true) && m_selected)
	{
		if ((offset & 0x1ff8)!=0x1ff8)  // not in mapping area
		{
			uint8_t* rom = &m_eprom[base | (offset & 0x1fff)];
			*value = *rom;
		}
	}
}

/*
    Debugger access.
*/
void tipi_card_device::debug_write(offs_t offset, uint8_t data)
{
	// No write allowed
}

/*
    CRU read access
*/
void tipi_card_device::crureadz(offs_t offset, uint8_t *value)
{
	// No CRU read
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
			if (data != 0)
			{
				LOGMASKED(LOG_RPI, "Sending RESET\n");
				send("RESET");
			}
			break;
		default:
			LOGMASKED(LOG_CRU, "Unknown CRU Bit %d\n", bit);
			break;
		}
	}
}

u8 tipi_card_device::get_rd()
{
	// Prevent a read access to RD while RD and RC are being
	// changed by the websocket client. It should suffice to guard RD.
	const std::lock_guard<std::mutex> lock(m_mutex);
	return m_rd;
}

void tipi_card_device::send(const char* message)
{
	if (m_rpiconn && m_connected)
	{
		*m_send_stream << message;
		m_wsclient->send(m_send_stream);
	}
}

void tipi_card_device::send(u8* message, int len)
{
	if (m_rpiconn && m_connected)
	{
		for (int i=0; i < len; i++)
		{
			LOGMASKED(LOG_SEND, "Sending byte %02x\n", message[i]);
			*m_send_stream << message[i];
		}
		m_wsclient->send(m_send_stream, nullptr, 130);  // binary (10000010)
	}
}

enum
{
	TSWB = 0x02,
	TSRB = 0x06,
	TSRSET = 0xf1
};

/*
    Process the command submitted to the TC register.
    The TSRSET only sends a sync to the Pi in sync mode; the current TIPI
    implementation normally uses the async mode.

    Apart from that, TSRSET resets the pointers in preparation of getting a
    received message or composing a message to be sent.

    TSWB: Write a byte. Instead of sending single bytes, the message is composed
          in an array and sent when it is complete.
    TSRB: Read a byte. This requires a message to be available in the input
          queue. If the queue is empty, the read operation is not acknowledged.
          This is done in websocket_incoming.
*/
void tipi_card_device::set_tc(u8 command)
{
	// Synchronize between websocket client and main thread
	const std::lock_guard<std::mutex> lock(m_mutex);

	LOGMASKED(LOG_PORTS, "TC <- %02x\n", command);

	// Only call once for the same value
	if (m_tc == command) return;

	m_tc = command;

	switch (m_tc)
	{
	case TSRSET:
		LOGMASKED(LOG_PROT, "TSRSET\n");
		m_rpimessage = nullptr;
		m_msgindex = -2;
		m_rc = m_tc;
		m_tc = 0;

		if (m_syncmode)
		{
			LOGMASKED(LOG_PROT, "Sending SYNC\n");
			send("SYNC");   // only for SYNC mode
		}
		break;

	case TSWB:
	case TSWB+1:
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
				if (m_rpimessage != nullptr)
					m_rpimessage[m_msgindex] = m_td;
		}

		m_msgindex++;

		if (m_msgindex == m_msglength)
		{
			// Message is complete - transmit it
			LOGMASKED(LOG_RPI, "Sending message, length %d\n", m_msglength);
			send(m_rpimessage.get(), m_msglength);
		}

		m_rc = m_tc;   // Auto-acknowledge
		break;

	case TSRB:
	case TSRB+1:
		if (!m_indqueue.empty())
		{
			m_rc = m_tc;   // Auto-acknowledge
			m_rd = m_indqueue.front();
			m_indqueue.pop();
		}
		else
		{
			// We have to wait for the TIPI to send more data
			LOGMASKED(LOG_PROT, "Pending read\n");
			m_pending_read = true;
		}
		break;
	}
}

/*
    Syntax for connecting is
    -conn rpi.<host>[:<port>]

    Default for port is 9901.
*/
TIMER_CALLBACK_MEMBER(tipi_card_device::open_websocket)
{
	if (m_rpiconn)
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

			LOG("Trying to connect to Raspberry Pi at %s\n", shost.c_str());

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
}

void tipi_card_device::websocket_opened()
{
	LOG("Connection established\n");
	m_connected = true;
	m_rc = 0;
	m_pausetime = PAUSEBASE;
	if (m_waitinit) m_slot->set_ready(ASSERT_LINE);
}

void tipi_card_device::websocket_incoming(std::shared_ptr<webpp::ws_client::Message> message)
{
	// Synchronize between websocket client and main thread
	const std::lock_guard<std::mutex> lock(m_mutex);

	// Caution: Message is an istream, so the string method consumes the Message
	// Size is not changed, nor is fin_rsv_opcode
	std::string msg;

	switch (message->fin_rsv_opcode & 0x0f)
	{
	case 1:
		msg = message->string();
		// Text message is "RD=xxx" or "RC=xxx" with xxx=decimal number

		LOGMASKED(LOG_RPI, "Got text message: %s\n", msg);

		if (msg.find("RD=")==0)
		{
			m_indqueue.push(std::stoi(message->string().substr(3)));
		}
		if (msg.find("RC=")==0)
		{
			m_rc = std::stoi(message->string().substr(3));
		}
		if (msg.find("ASYNC")==0)
		{
			LOGMASKED(LOG_PROT, "TIPI server offers ASYNC communication\n");
			m_syncmode = false;
		}
		break;

	case 2:
		LOGMASKED(LOG_RPI, "Got binary message, length %d\n", message->size());
		// Enqueue the incoming message
		// as n1 n2 msg, where n1 and n2 are the length of the message,
		// and msg is the byte sequence of the message.
		{
			u8 len1 = (message->size()>>8) & 0xff;
			u8 len2 = message->size() & 0xff;
			m_indqueue.push(len1);
			LOGMASKED(LOG_RECV, "<- %02x (len1) [%d]\n", len1, m_indqueue.size());
			m_indqueue.push(len2);
			LOGMASKED(LOG_RECV, "<- %02x (len2) [%d]\n", len2, m_indqueue.size());
		}

		for (int i=0; i < message->size(); i++)
		{
			u8 msgbyte = (u8)message->get();
			m_indqueue.push(msgbyte);
			LOGMASKED(LOG_RECV, "<- %02x [%d]\n", msgbyte, m_indqueue.size());
		}

		LOGMASKED(LOG_PROT, "Queue length: %d\n", m_indqueue.size());

		if (m_pending_read)
		{
			LOGMASKED(LOG_PROT, "Serving pending read\n");
			m_rc = m_tc;   // Auto-acknowledge
			m_rd = m_indqueue.front();
			m_indqueue.pop();
			m_pending_read = false;
		}

		break;

	default:
		LOGMASKED(LOG_RPI, "Got unknown message\n");
		break;
	}
}

void tipi_card_device::websocket_error(const std::error_code& code)
{
	m_pausetime = m_pausetime*2;
	m_attempts--;

	if (m_attempts > 0)
	{
		if (code.value() == std::make_error_code(std::errc::connection_reset).value())
		{
			LOGMASKED(LOG_WARN, "Error, connection reset. Retrying after %d secs.\n", m_pausetime/1000);
		}
		else
		{
			// End-of-file (2) occurs when the Pi is rebooted
			LOGMASKED(LOG_WARN, "Got error: %s (%d); retrying after %d secs\n", code.message().c_str(), code.value(), m_pausetime/1000);
			// Reconnection is not implemented on the Pi side for emulations,
			// so this may cause the config program to wait forever
			// Needs to be documented
		}
		m_restart_timer->adjust(attotime::from_msec(m_pausetime));
	}
	else
	{
		LOGMASKED(LOG_WARN, "Unable to re-establish connection to TIPI; giving up\n");
		m_connected = false;
	}
}

void tipi_card_device::websocket_closed(int i, const std::string& msg)
{
	LOG("Closing connection; retrying after %d secs\n", m_pausetime/1000);
	if (m_waitinit) m_slot->set_ready(CLEAR_LINE);
	// Open it again
	m_restart_timer->adjust(attotime::from_msec(m_pausetime));
	m_attempts = 8;
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
	save_item(NAME(m_lasttc));

	m_restart_timer = timer_alloc(FUNC(tipi_card_device::open_websocket), this);
	m_rpiconn = (m_rpi->exists());
	m_connected = false;
}

void tipi_card_device::device_reset()
{
	m_cru_base = (ioport("SW1")->read()) << 8;
	m_waitinit = (ioport("WAIT")->read()!=0);
	m_syncmode = true;
	m_address = 0;
	m_dsr = false;
	m_portaccess = false;
	m_pausetime = PAUSEBASE;
	if (!m_rpiconn) LOGMASKED(LOG_WARN, "No Raspberry Pi connected\n");

	// READY=0 is ignored at this point because the CPU is being reset at this time
	m_lasttc = 0;
	m_attempts = 8;
	if (!m_connected) m_restart_timer->adjust(attotime::from_msec(m_pausetime));
}

void tipi_card_device::device_stop()
{
	// MZ: Without this I'm getting segfaults/list corruption
	// when leaving the emulation
	m_wsclient->stop();
	LOG("Stopping TIPI\n");
}

/*
    The CRU address base for the card.
    For the Geneve OS version 7.30, CRU base 1800 is mandatory.
*/
INPUT_PORTS_START( tipi )
	PORT_START("SW1")
	PORT_DIPNAME(0x1f, 0x18, "CRU base")
		PORT_DIPSETTING(0x10, "1000")
		PORT_DIPSETTING(0x11, "1100")
		PORT_DIPSETTING(0x12, "1200")
		PORT_DIPSETTING(0x13, "1300")
		PORT_DIPSETTING(0x14, "1400")
		PORT_DIPSETTING(0x15, "1500")
		PORT_DIPSETTING(0x16, "1600")
		PORT_DIPSETTING(0x17, "1700")
		PORT_DIPSETTING(0x18, "1800") // Default setting
		PORT_DIPSETTING(0x19, "1900")
		PORT_DIPSETTING(0x1a, "1a00")
		PORT_DIPSETTING(0x1b, "1b00")
		PORT_DIPSETTING(0x1c, "1c00")
		PORT_DIPSETTING(0x1d, "1d00")
		PORT_DIPSETTING(0x1e, "1e00")
		PORT_DIPSETTING(0x1f, "1f00")
	PORT_START("WAIT")
	PORT_CONFNAME(0x01, 0x00, "Wait for initialization")
		PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
		PORT_CONFSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END

void tipi_card_device::device_add_mconfig(machine_config &config)
{
	TI99_TIPI_RPI(config, m_rpi, 0);
}

ROM_START( tipi )
	ROM_REGION(0x8000, TI99_DSRROM, 0)
	ROM_DEFAULT_BIOS("2023")
	ROM_SYSTEM_BIOS(0, "2021", "TIPI DSR (2021)" )
	ROMX_LOAD("tipidsr.u2", 0x0000, 0x8000, CRC(a54e65af) SHA1(f930e4b079a5b6b24dc20f262e9c599d2051b6be), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "2023", "TIPI DSR (2023)" )
	ROMX_LOAD("tipidsr_2023.u2", 0x0000, 0x8000, CRC(666f0b63) SHA1(70950e647d5a5102695580064b0bf8d2703586d3), ROM_BIOS(1))
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
std::pair<std::error_condition, std::string> tipi_attached_device::call_load()
{
	return std::make_pair(std::error_condition(), std::string());  // OK
}

void tipi_attached_device::call_unload()
{
}


} // end namespace bus::ti99::peb
