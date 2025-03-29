// license:BSD-3-Clause
// copyright-holders:Nadav Waisbrod
/***************************************************************************

    serial.cpp

    Serial output interface.

***************************************************************************/

#include "output_module.h"
#include "modules/osdmodule.h"
#include "osdcore.h"
#include "modules/lib/osdobj_common.h" // For osd_options definition
#include "serial/serial.h"
#include "util/strformat.h"

#include <memory>
#include <string>
#include <stdexcept>

namespace osd {

namespace {

class output_serial : public osd_module, public output_module
{
public:
	output_serial() : 
		osd_module(OSD_OUTPUT_PROVIDER, "serial"),
		m_serial(nullptr),
		m_port(""),
		m_baudrate(9600)
	{
	}
	
	virtual ~output_serial()
	{
		close_port();
	}

	virtual int init(osd_interface &osd, const osd_options &options) override
	{
		// Get port and baudrate from options
		m_port = options.serial_port();
		m_baudrate = options.serial_baudrate();
		
		if (m_port.empty())
		{
			osd_printf_warning("Serial output: No port specified, using COM1\n");
			m_port = "COM1";
		}
		
		// Try to open the port
		open_port();
		
		return 0;
	}
	
	virtual void exit() override
	{
		close_port();
	}

	// output_module
	virtual void notify(const char *outname, int32_t value) override
	{
		if (!m_serial || !m_serial->isOpen())
			return;
			
		try {
			// Format a message to send over serial
			std::string message = util::string_format("%s=%d\n", (outname == nullptr) ? "none" : outname, value);
			m_serial->write(message);
		}
		catch (const std::exception &e) {
			osd_printf_error("Serial output error: %s\n", e.what());
			close_port();
		}
	}

private:
	void open_port()
	{
		try {
			close_port(); // Close if already open
			m_serial = std::make_unique<serial::Serial>(m_port, m_baudrate);
			osd_printf_info("Serial output: Connected to %s at %d baud\n", m_port.c_str(), m_baudrate);
		}
		catch (const std::exception &e) {
			osd_printf_error("Serial output: Failed to open %s - %s\n", m_port.c_str(), e.what());
		}
	}
	
	void close_port()
	{
		if (m_serial && m_serial->isOpen())
		{
			try {
				m_serial->close();
				osd_printf_info("Serial output: Disconnected from %s\n", m_port.c_str());
			}
			catch (const std::exception &e) {
				osd_printf_error("Serial output: Error closing port - %s\n", e.what());
			}
		}
		m_serial.reset();
	}
	
	std::unique_ptr<serial::Serial> m_serial;
	std::string m_port;
	uint32_t m_baudrate;
};

} // anonymous namespace

} // namespace osd

MODULE_DEFINITION(OUTPUT_SERIAL, osd::output_serial) 