// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    none.c

    Empty shim for systems not supporting midi / portmidi

*******************************************************************c********/

#include "osdcore.h"
#include "modules/osdmodule.h"
#include "midi_module.h"

class none_module : public osd_module, public midi_module
{
public:

	none_module() : osd_module(OSD_MIDI_PROVIDER, "pm"), midi_module()
	{
	}
	virtual ~none_module() { }

	virtual int init(const osd_options &options) override;
	virtual void exit() override;

	virtual std::unique_ptr<osd_midi_device> create_midi_device() override;
	virtual void list_midi_devices() override;
};


class osd_midi_device_none : public osd_midi_device
{
public:
	virtual ~osd_midi_device_none() { }
	virtual bool open_input(const char *devname) override;
	virtual bool open_output(const char *devname) override;
	virtual void close() override;
	virtual bool poll() override;
	virtual int read(uint8_t *pOut) override;
	virtual void write(uint8_t data) override;
};

std::unique_ptr<osd_midi_device> none_module::create_midi_device()
{
	return std::make_unique<osd_midi_device_none>();
}


int none_module::init(const osd_options &options)
{
	return 0;
}

void none_module::exit()
{
}

void none_module::list_midi_devices()
{
	osd_printf_warning("\nMIDI is not supported in this build\n");
}

bool osd_midi_device_none::open_input(const char *devname)
{
	return false;
}

bool osd_midi_device_none::open_output(const char *devname)
{
	return false;
}

void osd_midi_device_none::close()
{
}

bool osd_midi_device_none::poll()
{
	return false;
}

int osd_midi_device_none::read(uint8_t *pOut)
{
	return 0;
}

void osd_midi_device_none::write(uint8_t data)
{
}

MODULE_DEFINITION(MIDI_NONE, none_module)
