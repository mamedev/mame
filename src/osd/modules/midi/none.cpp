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

class osd_midi_device_none : public osd_midi_device
{
public:
	virtual bool open_input(const char *devname) override { return false; }
	virtual bool open_output(const char *devname) override { return false; }
	virtual void close() override { }
	virtual bool poll() override { return false; }
	virtual int read(uint8_t *pOut) override { return 0; }
	virtual void write(uint8_t data) override { }
};


class none_module : public osd_module, public midi_module
{
public:
	none_module() : osd_module(OSD_MIDI_PROVIDER, "none"), midi_module() { }

	virtual int init(osd_interface &osd, const osd_options &options) override { return 0; }
	virtual void exit() override { }

	virtual std::unique_ptr<osd_midi_device> create_midi_device() override;
	virtual void list_midi_devices() override;
};

std::unique_ptr<osd_midi_device> none_module::create_midi_device()
{
	return std::make_unique<osd_midi_device_none>();
}

void none_module::list_midi_devices()
{
	osd_printf_warning("\nMIDI is not supported in this configuration\n");
}

} // anonymous namespace

} // namespace osd


MODULE_DEFINITION(MIDI_NONE, osd::none_module)
