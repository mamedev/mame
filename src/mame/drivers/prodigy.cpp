// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/******************************************************************************

    ACI Prodigy chess computer driver.

    http://www.spacious-mind.com/html/destiny_prodigy.html
    Morphy software ELO rating: 1559

    TODO:
    - Sound
    - Row/column LEDs
    - Chess board sensors

    +-------------------------------------------------------------------------------------+
    |LEDS--------------------------------------------+        +-----------------+         |
    | | |              o o o o o o o o COLCN         |        |                 |         |
    | V |                                            |        | 4 char BCD LED  |         |
    | O | 8                                          |        +-----------------+         |
    |   |                                      ROWCN |        |||||||||||||||||||  ____   |
    | O | 7                                         o|+----------------------+    /    \  |
    |   |                                           o||  VIA                 |   ( beep ) |
    | O | 6                                         o|| R6522                |    \____/  |
    |   |                                           o|+----------------------+            |
    | O | 5                                         o|+----------------------+            |
    |   |                                           o||  CPU                 |            |
    | O | 4         8 x 8 button matrix             o|| R6502-13             |   +--+  __ |
    |   |                                           o|+----------------------+   |74| PWR||
    | O | 3                                         o|   o    o  +-------------+ |LS| SW >|
    |   |                                            | +=======+ |  ROM        | |14|  __||
    | O | 2                                          | | 2MHz  | | R2912       | +--+     |
    |   |                                            | |  XTAL | +-------------+     +--+ |
    | O | 1                                          |++-------+ +-------------+     |74| |
    |   |                                            || 74145N | |  RAM        |     |LS| |
    |   |   A     B    C    D    E    F     G    H   |+--------+ | M58725P     |     |00| |
    |   +--------------------------------------------+           +-------------+     +--+ |
    |LEDS-> O     O    O    O    O    O     O    O                OOOOOOOOOOOO KPDCN      |
    +-------------------------------------------------------------------------------------+

 Tracing the image shows that VIA Port A is used on the ROWCN and Port B on COLCN

 The VIA pins CB1 and CB2 together with PB0 and PB1 via the 74145 drives the BCD display.
 The BCD display is of common anode type and each anode a0-a3 selects which digit that
 will be lit selected by PB0 and PB1. The segments to be lit is selected by the 8 bits
 shifted into the 74164 by clock from VIA CB1 and data from CB2.

  Behind the BCD display we find the following supporting circuit

                                                               4x7 segment BCD display
                                      +---+       +-----+          +---+---+---+---+
+-----------------+            CB1    |74 |==/4/=>|2x   |==/8/====>| 0   1   2   3 |
|                 |       VIA  CB2    |164|==/4/=>|75491| segments |               |
| 4 char BCD LED  |  ===> 6522        +---+       +-----+          +---+---+---+---+
+-----------------+            PB1--->|74 |                          |   |   |   |
|||||||||||||||||||            PB2--->|145|=/4/=/R/=>b(4x    )c=/4/==============>
                                      +---+           (PN2907)e=+     anodes
                                                                |+5v

 The keypad is connected to the 12 pin KPDCN connector left to right KP1:

  Pin #: KP1 KP2 KP3 KP4 KP5 KP6 KP7 KP8 KP9 KP10 KP11 K12
  VIA  :     PB4 PB5 PA0 PA1 PA2 PA3 PA4 PA5 PA6       PA7
  74145:  Q8                                       Q9      - used to decode/ground one half of the KPAD at a time

 Q7 is suspected to be ground for the chessboard buttons

*******************************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "cpu/m6502/m6502.h"
#include "machine/74145.h"
#include "machine/netlist.h"
#include "machine/nl_prodigy.h"
#include "machine/6522via.h"

// TODO: Move all HTTPUI stuff global and generic
#define HTTPUI 1

#if HTTPUI
#include <zlib.h>
#include <iostream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#endif

// Generated artwork includes
#include "prodigy.lh"

#define LOG_SETUP   (1U <<  1)
#define LOG_READ    (1U <<  2)
#define LOG_BCD     (1U <<  3)
#define LOG_NETLIST (1U <<  4)
#define LOG_CLK     (1U <<  5)
#define LOG_KBD     (1U <<  6)
#define LOG_AW      (1U <<  7)
#define LOG_HTTP    (1U <<  8)
#define LOG_XML     (1U <<  9)

//#define VERBOSE (LOG_HTTP|LOG_AW) // (LOG_BCD|LOG_NETLIST|LOG_SETUP)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGBCD(...)   LOGMASKED(LOG_BCD,     __VA_ARGS__)
#define LOGNL(...)    LOGMASKED(LOG_NETLIST, __VA_ARGS__)
#define LOGCLK(...)   LOGMASKED(LOG_CLK,     __VA_ARGS__)
#define LOGKBD(...)   LOGMASKED(LOG_KBD,     __VA_ARGS__)
#define LOGAW(...)    LOGMASKED(LOG_AW,      __VA_ARGS__)
#define LOGHTTP(...)  LOGMASKED(LOG_HTTP,    __VA_ARGS__)
#define LOGXML(...)   LOGMASKED(LOG_XML,     __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define NETLIST_TAG "bcd"

class prodigy_state : public driver_device
{
public:
	prodigy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_74145(*this, "io_74145")
		, m_segments(0)
		, m_digit_cache{0xff, 0xff, 0xff, 0xff}
		, m_via(*this, "via")
		, m_bcd(*this, NETLIST_TAG)
		, m_cb1(*this, "bcd:cb1")
		, m_cb2(*this, "bcd:cb2")
		, m_digit(0.0)
		, m_io_line(*this, "LINE%u", 0)
		, m_digits(*this, "digit%u", 0U)
#if HTTPUI
		, m_connection(NULL)
		, m_web_line0(0)
		, m_web_line1(0)
		, m_web_line2(0)
		, m_web_line3(0)
		, m_web_line4(0)
#endif
	{ }

	void prodigy(machine_config &config);

private:
	NETDEV_LOGIC_CALLBACK_MEMBER(bcd_bit0_cb);
	NETDEV_LOGIC_CALLBACK_MEMBER(bcd_bit1_cb);
	NETDEV_LOGIC_CALLBACK_MEMBER(bcd_bit2_cb);
	NETDEV_LOGIC_CALLBACK_MEMBER(bcd_bit3_cb);
	NETDEV_LOGIC_CALLBACK_MEMBER(bcd_bit4_cb);
	NETDEV_LOGIC_CALLBACK_MEMBER(bcd_bit5_cb);
	NETDEV_LOGIC_CALLBACK_MEMBER(bcd_bit6_cb);
	NETDEV_LOGIC_CALLBACK_MEMBER(bcd_bit7_cb);

	DECLARE_READ8_MEMBER( via_pa_r );
	DECLARE_READ8_MEMBER( via_pb_r );
	DECLARE_WRITE8_MEMBER( via_pa_w );
	DECLARE_WRITE8_MEMBER( via_pb_w );
	DECLARE_WRITE_LINE_MEMBER(via_cb1_w);
	DECLARE_WRITE_LINE_MEMBER(via_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(irq_handler);

	void maincpu_map(address_map &map);

	required_device<m6502_device> m_maincpu;
	required_device<ttl74145_device> m_74145;
	uint8_t m_segments;
	uint8_t m_digit_cache[4];
	required_device<via6522_device> m_via;
	required_device<netlist_mame_device> m_bcd;
	required_device<netlist_mame_logic_input_device> m_cb1;
	required_device<netlist_mame_logic_input_device> m_cb2;
	uint8_t m_digit;
	void update_bcd();

	virtual void device_start() override;
	required_ioport_array<5> m_io_line;
	output_finder<4> m_digits;
	uint16_t m_line[5];

#if HTTPUI
	http_manager *m_server;
	void on_update(http_manager::http_request_ptr request, http_manager::http_response_ptr response);
	const std::string decompress_layout_data(const internal_layout *layout_data);
	void on_open(http_manager::websocket_connection_ptr connection);
	void on_message(http_manager::websocket_connection_ptr connection, const std::string &payload, int opcode);
	void on_close(http_manager::websocket_connection_ptr connection, int status, const std::string& reason);
	void on_error(http_manager::websocket_connection_ptr connection, const std::error_code& error_code);
	void update_web_bcd(uint8_t digit_nbr, uint8_t m_segments);

	http_manager::websocket_connection_ptr m_connection;
	uint16_t m_web_line0;
	uint16_t m_web_line1;
	uint16_t m_web_line2;
	uint16_t m_web_line3;
	uint16_t m_web_line4;
#endif
};

NETDEV_LOGIC_CALLBACK_MEMBER(prodigy_state::bcd_bit0_cb) { if (data != 0) m_digit |= 0x01; else m_digit &= ~(0x01); LOGBCD("%s: %d m_digit: %02x\n", FUNCNAME, data, m_digit); }
NETDEV_LOGIC_CALLBACK_MEMBER(prodigy_state::bcd_bit1_cb) { if (data != 0) m_digit |= 0x02; else m_digit &= ~(0x02); LOGBCD("%s: %d m_digit: %02x\n", FUNCNAME, data, m_digit); }
NETDEV_LOGIC_CALLBACK_MEMBER(prodigy_state::bcd_bit2_cb) { if (data != 0) m_digit |= 0x04; else m_digit &= ~(0x04); LOGBCD("%s: %d m_digit: %02x\n", FUNCNAME, data, m_digit); }
NETDEV_LOGIC_CALLBACK_MEMBER(prodigy_state::bcd_bit3_cb) { if (data != 0) m_digit |= 0x08; else m_digit &= ~(0x08); LOGBCD("%s: %d m_digit: %02x\n", FUNCNAME, data, m_digit); }
NETDEV_LOGIC_CALLBACK_MEMBER(prodigy_state::bcd_bit4_cb) { if (data != 0) m_digit |= 0x10; else m_digit &= ~(0x10); LOGBCD("%s: %d m_digit: %02x\n", FUNCNAME, data, m_digit); }
NETDEV_LOGIC_CALLBACK_MEMBER(prodigy_state::bcd_bit5_cb) { if (data != 0) m_digit |= 0x20; else m_digit &= ~(0x20); LOGBCD("%s: %d m_digit: %02x\n", FUNCNAME, data, m_digit); }
NETDEV_LOGIC_CALLBACK_MEMBER(prodigy_state::bcd_bit6_cb) { if (data != 0) m_digit |= 0x40; else m_digit &= ~(0x40); LOGBCD("%s: %d m_digit: %02x\n", FUNCNAME, data, m_digit); }
NETDEV_LOGIC_CALLBACK_MEMBER(prodigy_state::bcd_bit7_cb) { if (data != 0) m_digit |= 0x80; else m_digit &= ~(0x80); LOGBCD("%s: %d m_digit: %02x\n", FUNCNAME, data, m_digit); }

void prodigy_state::device_start()
{
	m_digits.resolve();
	memset(m_line, 0, sizeof(m_line));
#if HTTPUI
	m_server =  machine().manager().http();
	if (m_server->is_active())
	{
		using namespace std::placeholders;
		m_server->add_http_handler("/layout*", std::bind(&prodigy_state::on_update, this, _1, _2));
		m_server->add_endpoint("/socket",
					   std::bind(&prodigy_state::on_open,    this, _1),
					   std::bind(&prodigy_state::on_message, this, _1, _2, _3),
					   std::bind(&prodigy_state::on_close,   this, _1, _2, _3),
					   std::bind(&prodigy_state::on_error,   this, _1, _2)
					   );
	}
#endif
}

#if HTTPUI
const std::string prodigy_state::decompress_layout_data(const internal_layout *layout_data)
{
	// +1 to ensure data is terminated for XML parser
	std::unique_ptr<u8> tempout(new u8(layout_data->decompressed_size + 1));
	std::string fail("");
	z_stream stream;
	int zerr;

	/* initialize the stream */
	memset(&stream, 0, sizeof(stream));
	stream.next_out = tempout.get();
	stream.avail_out = layout_data->decompressed_size;

	zerr = inflateInit(&stream);
	if (zerr != Z_OK)
	{
		fatalerror("could not inflateInit");
		return fail; // return empty buffer
	}

	/* decompress this chunk */
	stream.next_in = (unsigned char*)layout_data->data;
	stream.avail_in = layout_data->compressed_size;
	zerr = inflate(&stream, Z_NO_FLUSH);

	/* stop at the end of the stream */
	if (zerr == Z_STREAM_END)
	{
		// OK
	}
	else if (zerr != Z_OK)
	{
		fatalerror("decompression error\n");
		return fail; // return empty buffer
	}

	/* clean up */
	zerr = inflateEnd(&stream);
	if (zerr != Z_OK)
	{
		fatalerror("inflateEnd error\n");
		return fail; // return empty buffer
	}

	return std::string((const char *)tempout.get(), layout_data->decompressed_size + 1);
}


void prodigy_state::on_update(http_manager::http_request_ptr request, http_manager::http_response_ptr response)
{
	LOG("%s\n", FUNCNAME);

	LOGHTTP("Full request: %s\n", request->get_resource());
	LOGHTTP("Path: %s\n", request->get_path());
	LOGHTTP("Query: %s\n", request->get_query());
	LOGHTTP("Artpath: %s\n", this->machine().options().art_path());
	LOGHTTP("System: %s\n", machine().system().name);

	response->set_status(200);
	response->set_content_type("text/xml");

	if (std::string("/layout/layout.xsl") == request->get_path())
	{
		std::string path;
		osd_get_full_path(path, ".");
		LOGHTTP("Serving XSL document %s\n", path + "/web/layout.xsl");
#if 0
		m_server->serve_document(request, response, "layout.xls"); // Doesn't work, how do I make the webserver serve a file?
#else
		std::ifstream xslfile (path + "/web/layout.xsl");
		if (xslfile.is_open())
		{
			std::string buf;
			std::string outstr;
			while ( std::getline (xslfile, buf) )
			{
				outstr += buf + "\n";
			}
			xslfile.close();
			response->set_body(outstr);
			LOGXML("%s", outstr);
		}
		else
		{
			response->set_status(500); // Or 404...?
			logerror("Unable to open file %s", path + "web/layout.xsl");
			LOGHTTP("Unable to open file %s", path + "web/layout.xsl");
		}
#endif
	}
	else
	{
		z_stream stream;
		int zerr;
		char buf[1024 * 10];
		std::string outstr;
		memset(&stream, 0, sizeof(stream));
		stream.next_in = (Bytef *)(layout_prodigy.data);
		stream.avail_in = layout_prodigy.compressed_size;
		if ((zerr = inflateInit(&stream)) == Z_OK)
		{
			do
			{
				stream.next_out = (Bytef*)(&buf[0]);
				stream.avail_out = sizeof(buf);
				zerr = inflate(&stream, 0);
				if (zerr == Z_OK || zerr == Z_STREAM_END)
				{
					outstr.append(&buf[0], stream.total_out - outstr.size());
					LOGXML("appending %s\n", buf);
				}
				else
				{
					LOGHTTP("Append Error %d\n", zerr);
				}
			}while (zerr == Z_OK);
			if ((zerr = inflateEnd(&stream)) != Z_OK)
			{
				response->set_status(500); // Or 404...?
				fatalerror("Zlib decompression error\n");
			}
			else
			{
				outstr.insert(std::string("<?xml version=\"1.0\"?>").size(), "<?xml-stylesheet type=\"text/xml\" href=\"layout.xsl\"?>");
				response->set_body(outstr);
			}
		}
		else
		{
			response->set_status(500); // Or 404...?
			LOGHTTP("Init Error %d\n", zerr);
		}
	}
}

void prodigy_state::on_open(http_manager::websocket_connection_ptr connection)
{
	logerror("websocket connection opened\n");
	LOGHTTP("%s\n", FUNCNAME);
	m_connection = connection;
	for (int i = 0; i < 4; i++)
	{
		update_web_bcd(i, m_digit_cache[i]);
	}
}

void prodigy_state::on_message(http_manager::websocket_connection_ptr connection, const std::string &payload, int opcode)
{
	LOGHTTP("%s: %d - %s\n", FUNCNAME, opcode, payload);
	using namespace rapidjson;
	Document document;
	document.Parse(payload.c_str());
	LOGHTTP("%s - %04x: %s\n", document["inputtag"].GetString(), document["inputmask"].GetInt(), document["event"].GetInt() == 0 ? "mouse Down" : "mouse Up");

	// TODO: Reduce number of LINE%u and match mouse up to last mouse down so that mouse clicks don't stick "forever" when gliding out of click down boundary
	if (std::string(document["inputtag"].GetString()).compare("LINE0") == 0)
	{
		if (document["event"].GetInt() == 0) m_web_line0 |= document["inputmask"].GetInt(); else m_web_line0 &= ~(document["inputmask"].GetInt());
		LOGAW("-WEBLINE0: %02x\n", m_web_line0);
	}
	else if (std::string(document["inputtag"].GetString()).compare("LINE1") == 0)
	{
		if (document["event"].GetInt() == 0) m_web_line1 |= document["inputmask"].GetInt(); else m_web_line1 &= ~(document["inputmask"].GetInt());
		LOGAW("-WEBLINE1: %02x\n", m_web_line1);
	}
	else if (std::string(document["inputtag"].GetString()).compare("LINE2") == 0)
	{
		if (document["event"].GetInt() == 0) m_web_line2 |= document["inputmask"].GetInt(); else m_web_line2 &= ~(document["inputmask"].GetInt());
		LOGAW("-WEBLINE2: %02x\n", m_web_line2);
	}
	else if (std::string(document["inputtag"].GetString()).compare("LINE3") == 0)
	{
		if (document["event"].GetInt() == 0) m_web_line3 |= document["inputmask"].GetInt(); else m_web_line3 &= ~(document["inputmask"].GetInt());
		LOGAW("-WEBLINE3: %02x\n", m_web_line3);
	}
	else if (std::string(document["inputtag"].GetString()).compare("LINE4") == 0)
	{
		if (document["event"].GetInt() == 0) m_web_line4 |= document["inputmask"].GetInt(); else m_web_line4 &= ~(document["inputmask"].GetInt());
		LOGAW("-WEBLINE4: %02x\n", m_web_line4);
	}
}

void prodigy_state::on_close(http_manager::websocket_connection_ptr connection, int status, const std::string& reason)
{
	logerror("websocket connection closed: %d - %s\n", status, reason);
	LOGHTTP("%s: %d - %s\n", FUNCNAME, status, reason);
}

void prodigy_state::on_error(http_manager::websocket_connection_ptr connection, const std::error_code& error_code)
{
	logerror("websocket connection error: %d\n", error_code);
	LOGHTTP("%s: %d\n", FUNCNAME, error_code);
}

#endif

WRITE_LINE_MEMBER(prodigy_state::via_cb1_w)
{
	LOGCLK("%s: %d\n", FUNCNAME, state);
	m_cb1->write(state);
}

WRITE_LINE_MEMBER(prodigy_state::via_cb2_w)
{
	LOGCLK("%s: %d\n", FUNCNAME, state);
	m_cb2->write(state);
}

WRITE_LINE_MEMBER(prodigy_state::irq_handler)
{
	LOGBCD("%s: %d\n", FUNCNAME, state);
	m_maincpu->set_input_line(M6502_IRQ_LINE, state);
	update_bcd();
}

/* Pulling the base of the PN2907 PNP transistor low by the output of the 74145 controlled by PB0 and PB1 will feed
   the coresponding anode enabling the correct digit and lit the segments currently held by the outputs of the 74164
   serial to 8 bit parallel converter fed by serial data using CB1 clock and CB2 data

   PB2 and PB3 is also connected to the 74145, usage to be traced....
*/

READ8_MEMBER( prodigy_state::via_pa_r )
{
	LOGKBD("%s: Port A <- %02x\n", FUNCNAME, 0);
	uint16_t ttl74145_data = m_74145->read();

	LOGKBD(" - 74145: %03x\n", ttl74145_data);
#if HTTPUI
	if (ttl74145_data & 0x100) return ((m_line[0] & ~m_web_line0) | (m_line[1] & ~m_web_line1));
	if (ttl74145_data & 0x200) return ((m_line[4] & ~m_web_line4) | (m_line[3] & ~m_web_line3));
#else
	if (ttl74145_data & 0x100) return (m_line[0] | m_line[1]);
	if (ttl74145_data & 0x200) return (m_line[4] | m_line[3]);
#endif
	return 0xff;
}

READ8_MEMBER( prodigy_state::via_pb_r )
{
	LOGKBD("%s: Port B <- %02x\n", FUNCNAME, 0);
	uint16_t ttl74145_data = m_74145->read();

#if HTTPUI
	if (ttl74145_data & 0x100) return ((((m_line[2] & ~m_web_line2) >>  8) & 3) << 4);
	if (ttl74145_data & 0x200) return ((((m_line[2] & ~m_web_line2) >> 10) & 3) << 4);
#else
	if (ttl74145_data & 0x100) return (((m_line[2] >>  8) & 3) << 4);
	if (ttl74145_data & 0x200) return (((m_line[2] >> 10) & 3) << 4);
#endif

	return 0xff;
}

WRITE8_MEMBER( prodigy_state::via_pa_w )
{
	LOGKBD("%s: Port A -> %02x\n", FUNCNAME, data);
}

WRITE8_MEMBER( prodigy_state::via_pb_w )
{
	LOGBCD("%s: %02x ANODE %c\n", FUNCNAME, data, (data & 0x0f) <= 3 ? ('0' + (data & 0x03)) : 'x');
	LOGKBD("%s: %02x KBD Q8:%c Q9:%c\n", FUNCNAME, data, (data & 0x0f) == 8 ? '0' : '1', (data & 0x0f) == 9 ? '0' : '1');
	// Write PB0-PB3 to the 74145
	// Q0-Q3 => BCD0-BCD3 (PB0-PB1, PB2=0 PB3=0)
	// Q8-Q9 => KPDCN (PB0:0=Q8 1=Q9, PB1=0 PB2=0 PB3=1)
	m_74145->write( data & 0x0f );

	// Read the artwork
	int i = 0;
	for (auto & elem : m_io_line)
	{
		m_line[i] = elem->read();
		LOGAW("-LINE%u: %02x\n", i, m_line[i]);
		i++;
	}
}

#if HTTPUI
void prodigy_state::update_web_bcd(uint8_t digit_nbr, uint8_t m_segments)
{
	using namespace rapidjson;
	Document d;
	d.SetObject();
	Document::AllocatorType& allocator = d.GetAllocator();
	// Add data to JSON document
	d.AddMember("name",     "digit",    allocator);
	d.AddMember("number",   digit_nbr,  allocator);
	d.AddMember("segments", m_segments, allocator);
	// Convert JSON document to string
	StringBuffer buf;
	Writer<StringBuffer> writer(buf);
	d.Accept(writer);
	if (m_connection != NULL)
	{
		m_connection->send_message(buf.GetString(), 1);// 1 == Text 2 == binary
	}
}
#endif

void prodigy_state::update_bcd()
{
	LOGBCD("%s\n", FUNCNAME);
	uint16_t ttl74145_data;
	uint8_t digit_nbr = 4;

	ttl74145_data = m_74145->read();
	LOGBCD(" - 74145: %03x\n", ttl74145_data);

	if ((ttl74145_data & 0x0f) != 0x00)
	{
		switch (ttl74145_data & 0x0f)
		{
		case 0x01: digit_nbr = 0; break;
		case 0x02: digit_nbr = 1; break;
		case 0x04: digit_nbr = 2; break;
		case 0x08: digit_nbr = 3; break;
		default: logerror("Wrong BCD digit, shouldn't happen, call the maintainer!\n");
		}

		LOGBCD(" - digit number: %d\n", digit_nbr);
		LOGBCD(" - segments: %02x -> ", m_digit);
		m_segments = m_digit;
		LOGBCD("%02x\n", m_segments);

		if (digit_nbr < 4)
		{
			if (m_digit_cache[digit_nbr] != m_segments)
			{
				m_digits[digit_nbr] = m_segments;
				m_digit_cache[digit_nbr] = m_segments;
#if HTTPUI
				update_web_bcd(digit_nbr, m_segments);
#endif
			}
		}
	}
}

void prodigy_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x200f).m("via", FUNC(via6522_device::map));
	map(0x6000, 0x7fff).rom().region("roms", 0).mirror(0x8000);
}

/*
 * The keypad was modelled after the physical appearance but altered after finding out how it was working so
 * LINE0 to LINE4 has no correlation to the actual keypad anymore, which is connected like this:
 *
 * con  KP1/KP11
 *-----------------------
 * KP1  GND/HIZ
 * KP2  GO/BLACK
 * KP3  A1/B2
 * KP4  D4/E5
 * KP5  G7/E8
 * KP6  RESTORE/HALT&HINT
 * KP7  CE/AUDIO
 * KP8  LEVEL/TIME&NUMBER
 * KP9  CHANGE BOARD/F6
 * KP10 VERIFY/C3
 * KP11 HIZ/GND
 * KP12 ENTER/WHITE
 *-----------------------
 * KP1 and KP11 alternates as GND enabling 10 pads at a time which are read on VIA port A and B.
 * TODO: Refactor as two 10 bit LINEs rather then matrix in order to match circuit
 *
*/
static INPUT_PORTS_START( prodigy )
	PORT_START("LINE0") /* KEY ROW 0 */
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D_4")     PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('4')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G_7")     PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('7')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RESTORE") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CE")      PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0xc00, 0x00, IPT_UNUSED )

	PORT_START("LINE1") /* KEY ROW 1 */
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEVEL")        PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CHANGE_BOARD") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("VERIFY")       PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER")        PORT_CODE(KEYCODE_S)  PORT_CHAR('S')
	PORT_BIT(0xc00, 0x00, IPT_UNUSED )

	PORT_START("LINE2") /* KEY ROW 2 */
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GO")    PORT_CODE(KEYCODE_O)  PORT_CHAR('O')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A_1")   PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('1')
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BLACK") PORT_CODE(KEYCODE_P)  PORT_CHAR('P')
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B_2")   PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('2')
	PORT_BIT(0x000, 0x00, IPT_UNUSED )

	PORT_START("LINE3") /* KEY ROW 3 */
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E_5")       PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('5')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H_8")       PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('8')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HALT_HINT") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("AUDIO")     PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0xc00, 0x00, IPT_UNUSED )

	PORT_START("LINE4") /* KEY ROW 4 */
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TIME_NUMBER") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F_6")         PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('6')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C_3")         PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('3')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("WHITE")       PORT_CODE(KEYCODE_W)  PORT_CHAR('W')
	PORT_BIT(0xc00, 0x00, IPT_UNUSED )
INPUT_PORTS_END

void prodigy_state::prodigy(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(2'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &prodigy_state::maincpu_map);
	config.set_default_layout(layout_prodigy);

	TTL74145(config, m_74145, 0);

	VIA6522(config, m_via, XTAL(2'000'000));
	m_via->irq_handler().set(FUNC(prodigy_state::irq_handler));
	m_via->writepa_handler().set(FUNC(prodigy_state::via_pa_w));
	m_via->writepb_handler().set(FUNC(prodigy_state::via_pb_w));
	m_via->readpa_handler().set(FUNC(prodigy_state::via_pa_r));
	m_via->readpb_handler().set(FUNC(prodigy_state::via_pb_r));
	m_via->cb1_handler().set(FUNC(prodigy_state::via_cb1_w));
	m_via->cb2_handler().set(FUNC(prodigy_state::via_cb2_w));

	NETLIST_CPU(config, m_bcd, XTAL(2'000'000) * 30)
		.set_source(netlist_prodigy);

	NETLIST_LOGIC_INPUT(config, m_cb1); m_cb1->set_params("cb1.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_cb2); m_cb2->set_params("cb2.IN", 0);

	NETLIST_LOGIC_OUTPUT(config, "bcd:bcd_bit0").set_params("bcd_bit0", FUNC(prodigy_state::bcd_bit0_cb));
	NETLIST_LOGIC_OUTPUT(config, "bcd:bcd_bit1").set_params("bcd_bit1", FUNC(prodigy_state::bcd_bit1_cb));
	NETLIST_LOGIC_OUTPUT(config, "bcd:bcd_bit2").set_params("bcd_bit2", FUNC(prodigy_state::bcd_bit2_cb));
	NETLIST_LOGIC_OUTPUT(config, "bcd:bcd_bit3").set_params("bcd_bit3", FUNC(prodigy_state::bcd_bit3_cb));
	NETLIST_LOGIC_OUTPUT(config, "bcd:bcd_bit4").set_params("bcd_bit4", FUNC(prodigy_state::bcd_bit4_cb));
	NETLIST_LOGIC_OUTPUT(config, "bcd:bcd_bit5").set_params("bcd_bit5", FUNC(prodigy_state::bcd_bit5_cb));
	NETLIST_LOGIC_OUTPUT(config, "bcd:bcd_bit6").set_params("bcd_bit6", FUNC(prodigy_state::bcd_bit6_cb));
	NETLIST_LOGIC_OUTPUT(config, "bcd:bcd_bit7").set_params("bcd_bit7", FUNC(prodigy_state::bcd_bit7_cb));
}

/*
 * 6522 VIA init sequence
 * :via Reg 03 <- 00 - DDRA - Data Direction A, pin A0-A7 programmed as inputs
 * :via Reg 02 <- 8f - DDRB - Data Direction B, pin B4-B6 programmed as inputs, other pins as outputs
 * :via Reg 0c <- e0 - PCR - Peripheral Control Register, CB2 out pos out, CB1 ints on neg edg, CA2 inp neg act edg, CA1 ints on neg edg
 * :via Reg 0b <- 58 - ACR - Auxilary Control Register, con ints, B7 dis, timed ints, shift out on 0/2, PA and PB latches disabled
 * :via Reg 09 <- 58 - T2C-H - Timer 2 High Order Counter value
 * :via Reg 04 <- ff - T1C-L - Timer 1 Low Order Latches
 * :via Reg 0e <- a0 - IER - Interrupt Enable Register, Timer 2 Interrupts enabled
 *
 * ISR VIA accesses
 * Reg 08 -> e1 - T2C-L - T2 low order counter
 * Reg 08 <- 6e - T2C-L - T2 low order latches
 * Reg 09 <- 20 - T2C-H - T2 High Order Counter
 * Reg 0a <- 00 - SR - Shift Register
 * Reg 00 <- 09
 * Reg 01 -> ff
 * Reg 00 -> 09
 * Reg 00 <- 08
 * Reg 01 -> ff
 * Reg 00 -> 08
 * Reg 00 <- 07
 * Reg 01 -> ff
 * Reg 00 <- 06
 * Reg 01 -> ff
 * Reg 00 <- 05
 * Reg 01 -> ff
 * Reg 00 <- 04
 * Reg 01 -> ff
 * Reg 00 <- 03
 * Reg 01 -> ff
 * Reg 00 <- 02
 * Reg 01 -> ff
 * Reg 00 <- 01
 * Reg 01 -> ff
 * Reg 00 <- 00
 * Reg 01 -> ff
 * Reg 0a <- 00
 * Reg 00 <- 05
 * Reg 08 -> e2
*/
ROM_START(prodigy)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("0x2000.bin",  0x0000, 0x02000, CRC(8d60345a) SHA1(fff18ff12e1b1be91f8eac1178605a682564eff2))
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    STATE          INIT        COMPANY             FULLNAME           FLAGS
CONS( 1981, prodigy, 0,      0,      prodigy, prodigy, prodigy_state, empty_init, "Applied Concepts", "Destiny Prodigy", MACHINE_NO_SOUND )
