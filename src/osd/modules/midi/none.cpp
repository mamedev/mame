// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    none.c

    Empty shim for systems not supporting midi / portmidi

***************************************************************************/

#include "midi_module.h"

#include "modules/osdmodule.h"
#include "osdcore.h"

#include <memory>


namespace osd {

namespace {

class none_module : public osd_module, public midi_module
{
public:
	none_module() : osd_module(OSD_MIDI_PROVIDER, "none"), midi_module() { }

	virtual int init(osd_interface &osd, const osd_options &options) override { return 0; }
	virtual void exit() override { }

	virtual std::unique_ptr<midi_input_port> create_input(std::string_view name) override { return nullptr; }
	virtual std::unique_ptr<midi_output_port> create_output(std::string_view name) override { return nullptr; }
	virtual std::vector<osd::midi_port_info> list_midi_ports() override { return std::vector<osd::midi_port_info>(); }
};

} // anonymous namespace

} // namespace osd


MODULE_DEFINITION(MIDI_NONE, osd::none_module)
