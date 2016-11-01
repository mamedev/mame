// license:BSD-3-Clause
// copyright-holders:R. Belmont, Parduz
/*
    Ensoniq panel/display device
*/
#include "emu.h"
#include "esqpanel.h"
#include "asio.h"

#define ESQPANEL_EXTERNAL_TIMER_ID 47000

//**************************************************************************
// External panel support
//**************************************************************************

#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <list>
#include "asio.h"

#include "../machine/esqpanel_vfx_html.h"
#include "../machine/esqpanel_vfx_js.h"

#define ASIO_STANDALONE
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

typedef websocketpp::server<websocketpp::config::asio> ws_server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class external_panel;

using external_panel_ptr = std::shared_ptr<external_panel>;
typedef std::map<connection_hdl, external_panel_ptr, std::owner_less<connection_hdl>> connection_to_panel_map;

enum message_type {
	UNKNOWN = 0,
	ANALOG = 1 << 0,
	BUTTON = 1 << 1,
	CONTROL = 1 << 2,
	DISPLAY = 1 << 3,
	INFO = 1 << 4
};

class external_panel : public std::enable_shared_from_this<external_panel>
{
public:
	static int get_message_type(const char c)
	{
		switch(c)
		{
		case 'A':
			return message_type::ANALOG;
		case 'B':
			return message_type::BUTTON;
		case 'C':
			return message_type::CONTROL;
		case 'D':
			return message_type::DISPLAY;
		case 'I':
			return message_type::INFO;
		default:
			return message_type::UNKNOWN;
		}
	}

	external_panel() : m_send_message_types(0)
	{
		// printf("session: constructed\n");
	}

	int handle_control_message(const std::string &command)
	{
		int old = m_send_message_types;
		std::istringstream is(command);
		if (get_message_type(is.get()) != message_type::CONTROL)
		{
			return 0;
		}

		int n;
		while (!is.eof()) {
			char c = is.get();
			int message_type = external_panel::get_message_type(c);
			is >> n;
			int send = (n != 0);
			if (send)
			{
				m_send_message_types |= message_type;
			}
			else
			{
				m_send_message_types &= ~message_type;
			}
		}

		return m_send_message_types ^ old;
	}

	int send_message_types()
	{
		return m_send_message_types;
	}

	bool send_display_data()
	{
		return m_send_message_types & message_type::DISPLAY;
	}

	bool send_analog_values()
	{
		return m_send_message_types & message_type::ANALOG;
	}

	bool send_buttons()
	{
		return m_send_message_types & message_type::BUTTON;
	}

private:
	int m_send_message_types;
};

class esqpanel_external_panel_server
{
public:
	esqpanel_external_panel_server(short port) :
		m_port(port),
		m_active(false),
		m_index("index.html"),
		m_keyboard("unknown"),
		m_version(1)
	{
		m_server.init_asio();
		m_server.set_reuse_addr(true);

		m_server.set_access_channels(websocketpp::log::alevel::none);
		m_server.set_error_channels(websocketpp::log::elevel::all);

		m_server.set_open_handler(bind(&esqpanel_external_panel_server::on_open,this,::_1));
		m_server.set_close_handler(bind(&esqpanel_external_panel_server::on_close,this,::_1));
		m_server.set_message_handler(bind(&esqpanel_external_panel_server::on_message,this,::_1,::_2));
		m_server.set_http_handler(bind(&esqpanel_external_panel_server::on_http_request,this,::_1));
	}

	virtual ~esqpanel_external_panel_server()
	{
	}

	void start()
	{
		// printf("server: start()\n");
		m_working_thread = std::thread([](esqpanel_external_panel_server* self) { self->run(); }, this);
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		m_active = true;
	}

	void stop()
	{
		// printf("server: stop()\n");
		m_server.stop();
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			m_active = false;
			m_commands.clear();
		}
		m_working_thread.join();
	}

	void send_to_all(char c)
	{
		// printf("server: send_to_all(%02x)\n", ((unsigned int) c) & 0xff);
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		if (m_active)
		{
			// printf("server: sending '%02x' to all\n", ((unsigned int) c) & 0xff);
			m_to_send.str("");
			m_to_send.put('D');
			m_to_send.put(c);
			const std::string &s = m_to_send.str();

			for (auto iter: m_panels)
			{
				websocketpp::lib::error_code ec;
				external_panel_ptr panel = iter.second;
				if (panel->send_display_data())
				{
					send(iter.first, s);
				}
			}
		}
	}

	void on_open(connection_hdl connection)
	{
		std::cout << "Opening connection " << std::endl;

		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		m_panels[connection] = std::make_shared<external_panel>();
	}

	void on_close(connection_hdl connection)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		external_panel_ptr panel = external_panel_for_connection(connection);

		std::cout << "Closing connection " << std::endl;

		m_panels.erase(connection);
	}

	void on_message(connection_hdl connection, ws_server::message_ptr message)
	{
		external_panel_ptr panel = external_panel_for_connection(connection);
		const std::string &command = message->get_payload();

		int t = external_panel::get_message_type(command.front());

		if (t == message_type::CONTROL)
		{
			int changed = panel->handle_control_message(command);
			// printf("server: control message, changed = '%x'\n", changed);
			if ((changed & message_type::DISPLAY) && panel->send_display_data())
			{
				// printf("server: control message, sending contents\n");
				send_contents(connection);
			}

			if ((changed & message_type::ANALOG) && panel->send_analog_values())
			{
				// printf("server: control message, sending analog values\n");
				send_analog_values(connection);
			}

			if ((changed & message_type::BUTTON) && panel->send_buttons())
			{
				// printf("server: control message, sending button states\n");
				send_button_states(connection);
			}
		}
		else if (t == message_type::INFO)
		{
			std::ostringstream o;
			o << "I" << get_keyboard() << "," << get_version();
			send(connection, o.str());
		}
		else
		{
			{
				std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_commands.emplace_back(command);
			}

			// Echo the non-command message to any other connected panels that want it
			websocketpp::lib::error_code ec;
			for (auto iter: m_panels)
			{
				external_panel_ptr other_panel = iter.second;
				if (other_panel != panel && (t & other_panel->send_message_types()) != 0)
				{
					send(iter.first, command);
				}
			}
		}
	}

	void on_http_request(connection_hdl hdl)
	{
		ws_server::connection_ptr connection = m_server.get_con_from_hdl(hdl);
		const std::string &uri = connection->get_request().get_uri();

		if (uri == "/") // redirect from the root URL to the index.
		{
			// Set status to 301
			connection->set_status(websocketpp::http::status_code::moved_permanently);
			connection->append_header("Location", m_index);

			std::ostringstream body;
			body << "<html><body><a href=\"" << m_index << "\">See here</a></body></html>" << std::endl;
			connection->set_body(body.str());
		}
		else
		{
			auto i = m_http_handlers.find(uri);
			if (i != m_http_handlers.end())
			{
				std::ostringstream body;
				if ((i->second)(body, uri))
				{
					connection->set_status(websocketpp::http::status_code::ok);
					connection->set_body(body.str());
				}
				else
				{
					// hmmm ...
				}
			}
		}
	}

	external_panel_ptr external_panel_for_connection(connection_hdl connection)
	{
		auto it = m_panels.find(connection);

		if (it == m_panels.end()) {
			// this connection is not in the list. This really shouldn't happen
			// and probably means something else is wrong.
			throw std::invalid_argument("No panel avaliable for connection");
		}

		return it->second;
	}

	void run()
	{
		m_server.listen(m_port);
		m_server.start_accept();
		m_server.run();
	}

	bool has_commands()
	{
		// printf("server: has_commands()\n");
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		return !m_commands.empty();
	}

	std::string get_next_command()
	{
		// printf("server: get_next_command()\n");
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		std::string command = std::move(m_commands.front());
		m_commands.pop_front();
		return command;
	}

	void set_index(const std::string &index)
	{
		m_index = index;
	}

	void add_http_handler(const std::string &uri, std::function<bool(std::ostream&, const std::string&)> handler)
	{
		m_http_handlers[uri] = handler;
	}

	void set_content_provider(std::function<bool(std::ostream&)> provider)
	{
		m_content_provider = provider;
	}

	void set_keyboard(const std::string &keyboard)
	{
		m_keyboard = keyboard;
	}

	const std::string &get_keyboard() const
	{
		return m_keyboard;
	}

	const int get_version() const
	{
		return m_version;
	}

private:
	void send(connection_hdl connection, const std::string &s)
	{
		websocketpp::lib::error_code ec;
		m_server.send(connection, s, websocketpp::frame::opcode::binary, ec);
		if (ec)
		{
			std::cerr << "error sending to external panel, disconnecting it: " << ec << std::endl;
			m_panels.erase(connection);
		}
	}

	void send_contents(connection_hdl connection)
	{
		if (m_content_provider)
		{
			m_to_send.str("");
			m_to_send.put('D');
			if (m_content_provider(m_to_send))
			{
				send(connection, m_to_send.str());
			}
		}
	}

	void send_analog_values(connection_hdl hdl)
	{
		// TODO(cbrunschen): get the current analog values and send them
	}

	void send_button_states(connection_hdl hdl)
	{
		// TODO(cbrunschen): track current button states and send them
	}

	short m_port;
	bool m_active;

	ws_server m_server;
	std::recursive_mutex m_mutex;

	connection_to_panel_map m_panels;
	std::list<std::string> m_commands;
	std::thread m_working_thread;
	std::ostringstream m_to_send;

	std::string m_index;
	std::string m_keyboard;
	int m_version;
	std::map<const std::string, std::function<bool(std::ostream&, const std::string&)>> m_http_handlers;
	std::function<bool(std::ostream&)> m_content_provider;
};

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ESQPANEL1x22 = &device_creator<esqpanel1x22_device>;
const device_type ESQPANEL2x40 = &device_creator<esqpanel2x40_device>;
const device_type ESQPANEL2x40_VFX = &device_creator<esqpanel2x40_vfx_device>;
const device_type ESQPANEL2x16_SQ1 = &device_creator<esqpanel2x16_sq1_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  esqpanel_device - constructor
//-------------------------------------------------

esqpanel_device::esqpanel_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_serial_interface(mconfig, *this),
	m_light_states(0x3f), // maximum number of lights
	m_write_tx(*this),
	m_write_analog(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void esqpanel_device::device_start()
{
	// printf("esqpanel: device_start()\n");
	m_write_tx.resolve_safe();
	m_write_analog.resolve_safe();

	m_external_panel_server = new esqpanel_external_panel_server((short) 55555);
	m_external_panel_server->set_keyboard(owner()->shortname());
	m_external_panel_server->set_index("/FrontPanel.html");
	m_external_panel_server->add_http_handler("/FrontPanel.html", [this](std::ostream& o, const std::string& uri)
	{
		return write_front_panel_html(o);
	});
	m_external_panel_server->add_http_handler("/FrontPanel.js", [this](std::ostream& o, const std::string& uri)
	{
		return write_front_panel_js(o);
	});
	m_external_panel_server->set_content_provider([this](std::ostream& o)
	{
		return write_contents(o);
	});
	m_external_panel_server->start();

	m_external_timer = timer_alloc(ESQPANEL_EXTERNAL_TIMER_ID);
	m_external_timer->enable(false);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void esqpanel_device::device_reset()
{
	// printf("esqpanel: device_reset()\n");
	// panel comms is at 62500 baud (double the MIDI rate), 8N2
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
	set_rcv_rate(62500);
	set_tra_rate(62500);

	m_tx_busy = false;
	m_xmit_read = m_xmit_write = 0;
	m_bCalibSecondByte = false;
	m_bButtonLightSecondByte = false;

	attotime sample_time(0, ATTOSECONDS_PER_MILLISECOND);
	attotime initial_delay(0, ATTOSECONDS_PER_MILLISECOND);

	m_external_timer->adjust(initial_delay, 0, sample_time);
	m_external_timer->enable(true);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void esqpanel_device::device_stop()
{
	// printf("esqpanel: device_stop()\n");
	device_t::device_stop();

	m_external_panel_server->stop();
	delete m_external_panel_server;
	m_external_panel_server = nullptr;
}

void esqpanel_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (ESQPANEL_EXTERNAL_TIMER_ID == id)
	{
		check_external_panel_server();
	}
	else
	{
		device_serial_interface::device_timer(timer, id, param, ptr);
	}
}

void esqpanel_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	uint8_t data = get_received_char();

	//  if (data >= 0xe0) printf("Got %02x from motherboard (second %s)\n", data, m_bCalibSecondByte ? "yes" : "no");

	send_to_display(data);

	m_external_panel_server->send_to_all(data);

	if (m_bCalibSecondByte)
	{
		//      printf("second byte is %02x\n", data);
		if (data == 0xfd)   // calibration request
		{
			//          printf("let's send reply!\n");
			xmit_char(0xff);   // this is the correct response for "calibration OK"
		}
		m_bCalibSecondByte = false;
	}
	else if (m_bButtonLightSecondByte)
	{
		// Lights on the Buttons, on the VFX-SD:
		// Number   Button
		// 0        1-6
		// 1        8
		// 2        6
		// 3        4
		// 4        2
		// 5        Compare
		// 6        1
		// 7        Presets
		// 8        7-12
		// 9        9
		// a        7
		// b        5
		// c        3
		// d        Sounds
		// e        0
		// f        Cart
		int lightNumber = data & 0x3f;

		// Light states:
		// 0 = Off
		// 2 = On
		// 3 = Blinking
		m_light_states[lightNumber] = (data & 0xc0) >> 6;

		// TODO: do something with the button information!
		// printf("Setting light %d to %s\n", lightNumber, lightState == 3 ? "Blink" : lightState == 2 ? "On" : "Off");
		m_bButtonLightSecondByte = false;
	}
	else if (data == 0xfb)   // request calibration
	{
		m_bCalibSecondByte = true;
	}
	else if (data == 0xff)  // button light state command
	{
		m_bButtonLightSecondByte = true;
	}
	else
	{
		// EPS wants a throwaway reply byte for each byte sent to the KPC
		// VFX-SD and SD-1 definitely don't :)
		if (m_eps_mode)
		{
			if (data == 0xe7)
			{
				xmit_char(0x00);   // actual value of response is never checked
			}
			else if (data == 0x71)
			{
				xmit_char(0x00);   // actual value of response is never checked
			}
			else
			{
				xmit_char(data);   // actual value of response is never checked
			}
		}
	}
}

void esqpanel_device::tra_complete()    // Tx completed sending byte
{
	//  printf("panel Tx complete\n");
	// is there more waiting to send?
	if (m_xmit_read != m_xmit_write)
	{
		transmit_register_setup(m_xmitring[m_xmit_read++]);
		if (m_xmit_read >= XMIT_RING_SIZE)
		{
			m_xmit_read = 0;
		}
	}
	else
	{
		m_tx_busy = false;
	}
}

void esqpanel_device::tra_callback()    // Tx send bit
{
	m_write_tx(transmit_register_get_data_bit());
}

void esqpanel_device::xmit_char(uint8_t data)
{
	//  printf("Panel: xmit %02x\n", data);

	// if tx is busy it'll pick this up automatically when it completes
	if (!m_tx_busy)
	{
		m_tx_busy = true;
		transmit_register_setup(data);
	}
	else
	{
		// tx is busy, it'll pick this up next time
		m_xmitring[m_xmit_write++] = data;
		if (m_xmit_write >= XMIT_RING_SIZE)
		{
			m_xmit_write = 0;
		}
	}
}

void esqpanel_device::check_external_panel_server() {
	while (m_external_panel_server->has_commands())
	{
		std::string command = m_external_panel_server->get_next_command();
		int l = command.length();
		if (l > 0) {
			std::istringstream is(command);
			char c;
			is >> c;
			if (c == 'B') {
				// button
				char ud;
				is >> ud;
				int button;
				is >> button;
				bool down = ud == 'D';
				uint8_t sendme = (down ? 0x80 : 0) | (button & 0xff);
				// printf("button %d %s : sending char to mainboard: %02x\n", button, down ? "down" : "up", sendme);
				xmit_char(sendme);
				xmit_char(0x00);
			} else if (c == 'A') {
				// analog value from ES5505 OTIS: 10 bits, left-aligned within 16 bits.
				int channel, value;
				is >> channel;
				is >> value;
				uint16_t analog_value = (value << 6);
				// printf("analog: channel %d, value %d = %04x\n", channel, value, analog_value);
				set_analog_value(channel, analog_value);
			}
		}
	}
}

void esqpanel_device::set_analog_value(offs_t offset, uint16_t value)
{
	m_write_analog(offset, value);
}

/* panel with 1x22 VFD display used in the EPS-16 and EPS-16 Plus */

static MACHINE_CONFIG_FRAGMENT(esqpanel1x22)
	MCFG_ESQ1x22_ADD("vfd")
		MACHINE_CONFIG_END

			machine_config_constructor esqpanel1x22_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( esqpanel1x22 );
}

esqpanel1x22_device::esqpanel1x22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL1x22, "Ensoniq front panel with 1x22 VFD", tag, owner, clock, "esqpanel122", __FILE__),
m_vfd(*this, "vfd")
{
	m_eps_mode = true;
}

/* panel with 2x40 VFD display used in the ESQ-1, VFX-SD, SD-1, and others */

static MACHINE_CONFIG_FRAGMENT(esqpanel2x40)
	MCFG_ESQ2x40_ADD("vfd")
		MACHINE_CONFIG_END

			machine_config_constructor esqpanel2x40_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( esqpanel2x40 );
}

esqpanel2x40_device::esqpanel2x40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2x40, "Ensoniq front panel with 2x40 VFD", tag, owner, clock, "esqpanel240", __FILE__),
m_vfd(*this, "vfd")
{
	m_eps_mode = false;
}

/* panel with 2x40 VFD display used in the VFX, VFX-SD, SD-1 series */

static MACHINE_CONFIG_FRAGMENT(esqpanel2x40_vfx)
	MCFG_ESQ2x40_ADD("vfd")
		MACHINE_CONFIG_END

			machine_config_constructor esqpanel2x40_vfx_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( esqpanel2x40_vfx );
}

esqpanel2x40_vfx_device::esqpanel2x40_vfx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2x40, "Ensoniq VFX series front panel with 2x40 VFD", tag, owner, clock, "esqpanel2x40_vfx", __FILE__),
m_vfd(*this, "vfd")
{
	m_eps_mode = false;
}

bool esqpanel2x40_vfx_device::write_front_panel_html(std::ostream &o) const
{
	int len = strlen(esqpanel2x40_vfx_device::html) + 128;
	char *buf = new char[len];
	sprintf(buf, esqpanel2x40_vfx_device::html, m_external_panel_server->get_keyboard().c_str(), m_external_panel_server->get_version());
	o << buf;
	delete[] buf;
	return true;
}

bool esqpanel2x40_vfx_device::write_front_panel_js(std::ostream &o) const
{
	o << esqpanel2x40_vfx_device::js;
	return true;
}

bool esqpanel2x40_vfx_device::write_contents(std::ostream &o)
{
	m_vfd->write_contents(o);
	for (int i = 0; i < m_light_states.size(); i++)
	{
		o.put(0xff);
		o.put((m_light_states[i] << 6) | i);
	}
	return true;
}

// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------
static MACHINE_CONFIG_FRAGMENT(esqpanel2x16_sq1)
	MCFG_ESQ2x16_SQ1_ADD("vfd")
		MACHINE_CONFIG_END
			// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------
			machine_config_constructor esqpanel2x16_sq1_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( esqpanel2x16_sq1 );
}
// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------
esqpanel2x16_sq1_device::esqpanel2x16_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2x16_SQ1, "Ensoniq front panel with 2x16 LCD", tag, owner, clock, "esqpanel216_sq1", __FILE__),
m_vfd(*this, "vfd")
{
	m_eps_mode = false;
}
