// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    none.cpp

    Dummy sound interface.

***************************************************************************/

#include "sound_module.h"

#include "modules/osdmodule.h"


namespace osd {

namespace {

class sound_none : public osd_module, public sound_module
{
public:
	sound_none() : osd_module(OSD_SOUND_PROVIDER, "none")
	{
	}
	virtual ~sound_none() { }

	virtual int init(osd_interface &osd, const osd_options &options) override { return 0; }
	virtual void exit() override { }

	virtual uint32_t get_generation() override { return 1; }
	virtual audio_info get_information() override
	{
		audio_info result;
		result.m_generation = 1;
		result.m_default_sink = 0;
		result.m_default_source = 0;
		return result;
	}

	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override { return 0; }
	virtual void stream_close(uint32_t id) override { }
	virtual void stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame) override { }
};

} // anonymous namespace

} // namespace osd

MODULE_DEFINITION(SOUND_NONE, osd::sound_none)
