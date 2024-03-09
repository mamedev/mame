// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    portmidi.cpp

    Midi implementation based on portmidi library

*******************************************************************c********/

#include "midi_module.h"

#include "modules/osdmodule.h"

#ifndef NO_USE_MIDI

#include "interface/midiport.h"
#include "osdcore.h" // osd_printf_*

#include <portmidi.h>

#include <cstdio>
#include <cstring>


namespace osd {

namespace {

class pm_module : public osd_module, public midi_module
{
public:

	pm_module() : osd_module(OSD_MIDI_PROVIDER, "pm"), midi_module()
	{
	}
	virtual ~pm_module() { }

	virtual int init(osd_interface &osd, const osd_options &options) override;
	virtual void exit() override;

	virtual std::unique_ptr<midi_input_port> create_input(std::string_view name) override;
	virtual std::unique_ptr<midi_output_port> create_output(std::string_view name) override;
	virtual port_info_vector list_midi_ports() override;
};


static constexpr unsigned RX_EVENT_BUF_SIZE = 512;

static constexpr uint8_t MIDI_SYSEX = 0xf0;
static constexpr uint8_t MIDI_EOX   = 0xf7;

class midi_input_pm : public midi_input_port
{
public:
	midi_input_pm(PortMidiStream *stream) : m_stream(stream), m_rx_sysex(false) { }
	virtual ~midi_input_pm();

	virtual bool poll() override;
	virtual int read(uint8_t *pOut) override;

private:
	PortMidiStream *const m_stream;
	PmEvent m_evbuf[RX_EVENT_BUF_SIZE];
	bool m_rx_sysex;
};

class midi_output_pm : public midi_output_port
{
public:
	midi_output_pm(PortMidiStream *stream) : m_stream(stream), m_xmit_cnt(0), m_last_status(0) { }
	virtual ~midi_output_pm();

	virtual void write(uint8_t data) override;

private:
	PortMidiStream *const m_stream;
	uint8_t m_xmit_in[4]; // Pm_Messages mean we can at most have 3 residue bytes
	int m_xmit_cnt;
	uint8_t m_last_status;
};


int pm_module::init(osd_interface &osd, const osd_options &options)
{
	// FIXME: check error return code
	Pm_Initialize();
	return 0;
}

void pm_module::exit()
{
	Pm_Terminate();
}

std::unique_ptr<midi_input_port> pm_module::create_input(std::string_view name)
{
	int found_dev = -1;
	if (name == "default")
	{
		found_dev = Pm_GetDefaultInputDeviceID();
	}
	else
	{
		int const num_devs = Pm_CountDevices();
		for (found_dev = 0; num_devs > found_dev; ++found_dev)
		{
			auto const info = Pm_GetDeviceInfo(found_dev);
			if (info->input && (name == info->name))
				break;
		}
		if (num_devs <= found_dev)
			found_dev = -1;
	}

	if (0 > found_dev)
	{
		osd_printf_warning("No MIDI input device named '%s' was found.\n", name);
		return nullptr;
	}

	PortMidiStream *stream = nullptr;
	PmError const err = Pm_OpenInput(&stream, found_dev, nullptr, RX_EVENT_BUF_SIZE, nullptr, nullptr);
	if (pmNoError != err)
	{
		osd_printf_error("Error opening PortMidi device '%s' for input.\n", name);
		return nullptr;
	}

	try
	{
		return std::make_unique<midi_input_pm>(stream);
	}
	catch (...)
	{
		Pm_Close(stream);
		return nullptr;
	}
}

std::unique_ptr<midi_output_port> pm_module::create_output(std::string_view name)
{
	int found_dev = -1;
	if (name == "default")
	{
		found_dev = Pm_GetDefaultOutputDeviceID();
	}
	else
	{
		int const num_devs = Pm_CountDevices();
		for (found_dev = 0; num_devs > found_dev; ++found_dev)
		{
			auto const info = Pm_GetDeviceInfo(found_dev);
			if (info->output && (name == info->name))
				break;
		}
		if (num_devs <= found_dev)
			found_dev = -1;
	}

	PortMidiStream *stream = nullptr;
	PmError const err = Pm_OpenOutput(&stream, found_dev, nullptr, 100, nullptr, nullptr, 0);
	if (pmNoError != err)
	{
		osd_printf_error("Error opening PortMidi device '%s' for output.\n", name);
		return nullptr;
	}

	try
	{
		return std::make_unique<midi_output_pm>(stream);
	}
	catch (...)
	{
		Pm_Close(stream);
		return nullptr;
	}
}

midi_module::port_info_vector pm_module::list_midi_ports()
{
	int const num_devs = Pm_CountDevices();
	int const def_input = Pm_GetDefaultInputDeviceID();
	int const def_output = Pm_GetDefaultOutputDeviceID();
	port_info_vector result;
	result.reserve(num_devs);
	for (int i = 0; num_devs > i; ++i)
	{
		auto const pm_info = Pm_GetDeviceInfo(i);
		result.emplace_back(port_info{
				pm_info->name,
				0 != pm_info->input,
				0 != pm_info->output,
				def_input == i,
				def_output == i });
	}
	return result;
}


midi_input_pm::~midi_input_pm()
{
	if (m_stream)
		Pm_Close(m_stream);
}

bool midi_input_pm::poll()
{
	PmError const chk = Pm_Poll(m_stream);
	return (chk == pmGotData) ? true : false;
}

int midi_input_pm::read(uint8_t *pOut)
{
	int msgsRead = Pm_Read(m_stream, m_evbuf, RX_EVENT_BUF_SIZE);
	int bytesOut = 0;

	if (msgsRead <= 0)
	{
		return 0;
	}

	for (int msg = 0; msg < msgsRead; msg++)
	{
		uint8_t status = Pm_MessageStatus(m_evbuf[msg].message);

		if (m_rx_sysex)
		{
			if (status & 0x80)  // sys real-time imposing on us?
			{
				if (status == 0xf2)
				{
					*pOut++ = status;
					*pOut++ = Pm_MessageData1(m_evbuf[msg].message);
					*pOut++ = Pm_MessageData2(m_evbuf[msg].message);
					bytesOut += 3;
				}
				else if (status == 0xf3)
				{
					*pOut++ = status;
					*pOut++ = Pm_MessageData1(m_evbuf[msg].message);
					bytesOut += 2;
				}
				else
				{
					*pOut++ = status;
					bytesOut++;
					if (status == MIDI_EOX)
					{
						m_rx_sysex = false;
					}
				}
			}
			else    // shift out the sysex bytes
			{
				for (int i = 0; i < 4; i++)
				{
					uint8_t byte = m_evbuf[msg].message & 0xff;
					*pOut++ = byte;
					bytesOut++;
					if (byte == MIDI_EOX)
					{
						m_rx_sysex = false;
						break;
					}
					m_evbuf[msg].message >>= 8;
				}
			}
		}
		else
		{
			switch ((status>>4) & 0xf)
			{
				case 0xc:   // 2-byte messages
				case 0xd:
					*pOut++ = status;
					*pOut++ = Pm_MessageData1(m_evbuf[msg].message);
					bytesOut += 2;
					break;

				case 0xf:   // system common
					switch (status & 0xf)
					{
						case 0: // System Exclusive
						{
							*pOut++ = status;   // this should be OK: the shortest legal sysex is F0 tt dd F7, I believe
							*pOut++ = (m_evbuf[msg].message>>8) & 0xff;
							*pOut++ = (m_evbuf[msg].message>>16) & 0xff;
							uint8_t last = *pOut++ = (m_evbuf[msg].message>>24) & 0xff;
							bytesOut += 4;
							m_rx_sysex = (last != MIDI_EOX);
							break;
						}

						case 7: // End of System Exclusive
							*pOut++ = status;
							bytesOut += 1;
							m_rx_sysex = false;
							break;

						case 2: // song pos
							*pOut++ = status;
							*pOut++ = Pm_MessageData1(m_evbuf[msg].message);
							*pOut++ = Pm_MessageData2(m_evbuf[msg].message);
							bytesOut += 3;
							break;

						case 3: // song select
							*pOut++ = status;
							*pOut++ = Pm_MessageData1(m_evbuf[msg].message);
							bytesOut += 2;
							break;

						default:    // all other defined Fx messages are 1 byte
							*pOut++ = status;
							bytesOut += 1;
							break;
					}
					break;

				default:
					*pOut++ = status;
					*pOut++ = Pm_MessageData1(m_evbuf[msg].message);
					*pOut++ = Pm_MessageData2(m_evbuf[msg].message);
					bytesOut += 3;
					break;
			}
		}
	}

	return bytesOut;
}


midi_output_pm::~midi_output_pm()
{
	if (m_stream)
		Pm_Close(m_stream);
}

void midi_output_pm::write(uint8_t data)
{
	int bytes_needed = 0;
	PmEvent ev;
	ev.timestamp = 0;   // use the current time

//  printf("write: %02x (%d)\n", data, m_xmit_cnt);

	// reject data bytes when no valid status exists
	if ((m_last_status == 0) && !(data & 0x80))
	{
		m_xmit_cnt = 0;
		return;
	}

	if (m_xmit_cnt >= 4)
	{
		printf("MIDI out: packet assembly overflow, contact MAMEdev!\n");
		return;
	}

	// handle sysex
	if (m_last_status == MIDI_SYSEX)
	{
//      printf("sysex: %02x (%d)\n", data, m_xmit_cnt);

		// if we get a status that isn't sysex, assume it's system common
		if ((data & 0x80) && (data != MIDI_EOX))
		{
//          printf("common during sysex!\n");
			ev.message = Pm_Message(data, 0, 0);
			Pm_Write(m_stream, &ev, 1);
			return;
		}

		m_xmit_in[m_xmit_cnt++] = data;

		// if EOX or 4 bytes filled, transmit 4 bytes
		if ((m_xmit_cnt == 4) || (data == MIDI_EOX))
		{
			ev.message = m_xmit_in[0] | (m_xmit_in[1]<<8) | (m_xmit_in[2]<<16) | (m_xmit_in[3]<<24);
			Pm_Write(m_stream, &ev, 1);
			m_xmit_in[0] = m_xmit_in[1] = m_xmit_in[2] = m_xmit_in[3] = 0;
			m_xmit_cnt = 0;

//          printf("SysEx packet: %08x\n", ev.message);

			// if this is EOX, kill the running status
			if (data == MIDI_EOX)
			{
				m_last_status = 0;
			}
		}

		return;
	}

	// handle running status.  don't allow system real-time messages to be considered as running status.
	if ((m_xmit_cnt == 0) && (data & 0x80) && (data < 0xf8))
	{
		m_last_status = data;
	}

	if ((m_xmit_cnt == 0) && !(data & 0x80))
	{
		m_xmit_in[m_xmit_cnt++] = m_last_status;
		m_xmit_in[m_xmit_cnt++] = data;
//      printf("\trunning status: [%d] = %02x, [%d] = %02x, m_last_status = %02x\n", m_xmit_cnt-2, m_last_status, m_xmit_cnt-1, data, m_last_status);
	}
	else
	{
		m_xmit_in[m_xmit_cnt++] = data;
//      printf("\tNRS: [%d] = %02x\n", m_xmit_cnt-1, data);
	}

	if ((m_xmit_cnt == 1) && (m_xmit_in[0] == MIDI_SYSEX))
	{
//      printf("Start SysEx!\n");
		m_last_status = MIDI_SYSEX;
		return;
	}

	// are we there yet?
//  printf("status check: %02x\n", m_xmit_in[0]);
	switch ((m_xmit_in[0]>>4) & 0xf)
	{
		case 0xc:   // 2-byte messages
		case 0xd:
			bytes_needed = 2;
			break;

		case 0xf:   // system common
			switch (m_xmit_in[0] & 0xf)
			{
				case 0: // System Exclusive is handled above
					break;

				case 7: // End of System Exclusive
					bytes_needed = 1;
					break;

				case 2: // song pos
				case 3: // song select
					bytes_needed = 3;
					break;

				default:    // all other defined Fx messages are 1 byte
					bytes_needed = 1;
					break;
			}
			break;

		default:
			bytes_needed = 3;
			break;
	}

	if (m_xmit_cnt == bytes_needed)
	{
		ev.message = Pm_Message(m_xmit_in[0], m_xmit_in[1], m_xmit_in[2]);
		Pm_Write(m_stream, &ev, 1);
		m_xmit_cnt = 0;
	}

}

} // anonymous namespace

} // namespace osd

#else

namespace osd { namespace { MODULE_NOT_SUPPORTED(pm_module, OSD_MIDI_PROVIDER, "pm") } }

#endif


MODULE_DEFINITION(MIDI_PM, osd::pm_module)
