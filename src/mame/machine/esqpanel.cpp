// license:BSD-3-Clause
// copyright-holders:R. Belmont, Parduz
/*
    Ensoniq panel/display device
*/
#include "emu.h"
#include "esqpanel.h"

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

namespace esqpanel {

	class external_panel;

	using external_panel_ptr = std::shared_ptr<external_panel>;
	typedef std::map<http_manager::websocket_connection_ptr, external_panel_ptr, std::owner_less<http_manager::websocket_connection_ptr>> connection_to_panel_map;

	enum message_type {
		UNKNOWN = 0,
		ANALOG = 1 << 0,
		BUTTON = 1 << 1,
		CONTROL = 1 << 2,
		DISPLAY = 1 << 3,
		INFO = 1 << 4
	};

	class external_panel
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

	class external_panel_server
	{
	public:
		enum websocket_opcode {
			text = 1,
			binary = 2
		};
		external_panel_server(http_manager *webserver) :
			m_server(webserver),
			m_keyboard("unknown"),
			m_version("1")
		{
			using namespace std::placeholders;
			if (m_server->is_active()) {
				m_server->add_endpoint("/esqpanel/socket",
						std::bind(&external_panel_server::on_open, this, _1),
						std::bind(&external_panel_server::on_message, this, _1, _2, _3),
						std::bind(&external_panel_server::on_close, this, _1, _2, _3),
						std::bind(&external_panel_server::on_error, this, _1, _2)
						);
			}
		}

		virtual ~external_panel_server()
		{
			if (m_server->is_active()) {
				m_server->remove_endpoint("/esqpanel/socket");
			}
		}

		void send_to_all(char c)
		{
			// printf("server: send_to_all(%02x)\n", ((unsigned int) c) & 0xff);
			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			// printf("server: sending '%02x' to all\n", ((unsigned int) c) & 0xff);
			m_to_send.str("");
			m_to_send.put('D');
			m_to_send.put(c);
			const std::string &s = m_to_send.str();

			for (auto iter: m_panels)
			{
				external_panel_ptr panel = iter.second;
				if (panel->send_display_data())
				{
					send(iter.first, s);
				}
			}
		}

		void on_open(http_manager::websocket_connection_ptr connection)
		{
			using namespace std::placeholders;

			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			m_panels[connection] = std::make_shared<external_panel>();
		}

		void on_message(http_manager::websocket_connection_ptr connection, const std::string &payload, int opcode)
		{
			external_panel_ptr panel = external_panel_for_connection(connection);
			const std::string &command = payload;

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

		void on_close(http_manager::websocket_connection_ptr connection, int status, const std::string& reason)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			m_panels.erase(connection);
		}

		void on_error(http_manager::websocket_connection_ptr connection, const std::error_code& error_code)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			m_panels.erase(connection);
		}

		void on_document_request(http_manager::http_request_ptr request, http_manager::http_response_ptr response, const std::string &filename)
		{
			m_server->serve_document(request, response, filename);
		}

		void on_template_request(http_manager::http_request_ptr request, http_manager::http_response_ptr response, const std::string &filename)
		{
			using namespace std::placeholders;
			m_server->serve_template(request, response, filename, std::bind(&external_panel_server::get_template_value, this, _1), '$', '$');
		}

		external_panel_ptr external_panel_for_connection(http_manager::websocket_connection_ptr connection)
		{
			auto it = m_panels.find(connection);

			if (it == m_panels.end()) {
				// this connection is not in the list. This really shouldn't happen
				// and probably means something else is wrong.
				throw std::invalid_argument("No panel avaliable for connection");
			}

			return it->second;
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

		void add_http_document(const std::string &path, const std::string &filename)
		{
			m_server->remove_http_handler(path);
			if (filename != "")
			{
				using namespace std::placeholders;
				m_server->add_http_handler(path, std::bind(&external_panel_server::on_document_request, this, _1, _2, filename));
			}
		}

		void add_http_template(const std::string &path, const std::string &filename)
		{
			m_server->remove_http_handler(path);
			if (filename != "")
			{
				using namespace std::placeholders;
				m_server->add_http_handler(path, std::bind(&external_panel_server::on_template_request, this, _1, _2, filename));
			}
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

		const std::string &get_version() const
		{
			return m_version;
		}

		bool get_template_value(std::string &s)
		{
			if (s == "keyboard")
			{
				s = m_keyboard;
				return true;
			}
			else if (s == "version")
			{
				s = m_version;
				return true;
			}
			else
			{
				return false;
			}
		}

	private:
		void send(http_manager::websocket_connection_ptr connection, const std::string &s)
		{
			connection->send_message(s, websocket_opcode::binary);
		}

		void send_contents(http_manager::websocket_connection_ptr connection)
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

		void send_analog_values(http_manager::websocket_connection_ptr connection)
		{
			// TODO(cbrunschen): get the current analog values and send them
		}

		void send_button_states(http_manager::websocket_connection_ptr connection)
		{
			// TODO(cbrunschen): track current button states and send them
		}

		http_manager *m_server;
		std::recursive_mutex m_mutex;

		connection_to_panel_map m_panels;
		std::list<std::string> m_commands;
		std::thread m_working_thread;
		std::ostringstream m_to_send;

		std::string m_index;
		std::string m_keyboard;
		std::string m_version;
		std::function<bool(std::ostream&)> m_content_provider;
		std::map<const std::string, const std::string> m_template_values;
	};

}  // namespace esqpanel

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ESQPANEL1X22,     esqpanel1x22_device,     "esqpanel122",     "Ensoniq front panel with 1x22 VFD")
DEFINE_DEVICE_TYPE(ESQPANEL2X40,     esqpanel2x40_device,     "esqpanel240",     "Ensoniq front panel with 2x40 VFD")
DEFINE_DEVICE_TYPE(ESQPANEL2X40_VFX, esqpanel2x40_vfx_device, "esqpanel240_vfx", "Ensoniq front panel with 2x40 VFD for VFX family")
DEFINE_DEVICE_TYPE(ESQPANEL2X16_SQ1, esqpanel2x16_sq1_device, "esqpanel216_sq1", "Ensoniq front panel with 2x16 LCD")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  esqpanel_device - constructor
//-------------------------------------------------

esqpanel_device::esqpanel_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
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
	m_write_tx.resolve_safe();
	m_write_analog.resolve_safe();

	m_external_panel_server = new esqpanel::external_panel_server(machine().manager().http());
	if (machine().manager().http()->is_active()) {
		m_external_panel_server->set_keyboard(owner()->shortname());
		m_external_panel_server->set_index("/esqpanel/FrontPanel.html");
		m_external_panel_server->add_http_template("/esqpanel/FrontPanel.html", get_front_panel_html_file());
		m_external_panel_server->add_http_document("/esqpanel/FrontPanel.js", get_front_panel_js_file());
		m_external_panel_server->set_content_provider([this](std::ostream& o)
		{
			return write_contents(o);
		});

		m_external_timer = timer_alloc(ESQPANEL_EXTERNAL_TIMER_ID);
		m_external_timer->enable(false);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void esqpanel_device::device_reset()
{
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

	if (m_external_timer) {
		m_external_timer->adjust(initial_delay, 0, sample_time);
		m_external_timer->enable(true);
	}
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void esqpanel_device::device_stop()
{
	device_t::device_stop();

	delete m_external_panel_server;
	m_external_panel_server = nullptr;
}

void esqpanel_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (ESQPANEL_EXTERNAL_TIMER_ID == id)
	{
		check_external_panel_server();
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

void esqpanel1x22_device::device_add_mconfig(machine_config &config)
{
	ESQ1X22(config, m_vfd, 60);
}


esqpanel1x22_device::esqpanel1x22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL1X22, tag, owner, clock),
	m_vfd(*this, "vfd")
{
	m_eps_mode = true;
}

/* panel with 2x40 VFD display used in the ESQ-1, SQ-80 */

void esqpanel2x40_device::device_add_mconfig(machine_config &config)
{
	ESQ2X40(config, m_vfd, 60);
}


esqpanel2x40_device::esqpanel2x40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2X40, tag, owner, clock),
	m_vfd(*this, "vfd")
{
	m_eps_mode = false;
}

/* panel with 2x40 VFD display used in the VFX, VFX-SD, SD-1 series */

void esqpanel2x40_vfx_device::device_add_mconfig(machine_config &config)
{
	ESQ2X40(config, m_vfd, 60);
}

esqpanel2x40_vfx_device::esqpanel2x40_vfx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2X40_VFX, tag, owner, clock),
	m_vfd(*this, "vfd")
{
	m_eps_mode = false;
}

bool esqpanel2x40_vfx_device::write_contents(std::ostream &o)
{
	m_vfd->write_contents(o);
	for (int i = 0; i < m_light_states.size(); i++)
	{
		o.put(char(0xff));
		o.put((m_light_states[i] << 6) | i);
	}
	return true;
}



// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------
void esqpanel2x16_sq1_device::device_add_mconfig(machine_config &config)
{
	ESQ2X16_SQ1(config, m_vfd, 60);
}

// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------
esqpanel2x16_sq1_device::esqpanel2x16_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2X16_SQ1, tag, owner, clock),
	m_vfd(*this, "vfd")
{
	m_eps_mode = false;
}
