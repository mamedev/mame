// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef _osdmac_h_
#define _osdmac_h_

#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"
#include "modules/font/font_module.h"

//============================================================
//  Defines
//============================================================

#define MACOPTION_INIPATH               "inipath"

#define MACOPTVAL_OPENGL                "opengl"
#define MACOPTVAL_BGFX                  "bgfx"
#define MACOPTVAL_METAL                 "metal"

#define MACOPTVAL_GLLIB                 "/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib"

//============================================================
//  TYPE DEFINITIONS
//============================================================

class mac_options : public osd_options
{
public:
	// construction/destruction
	mac_options();

private:
	static const options_entry s_option_entries[];
};


class mac_osd_interface : public osd_common_t
{
public:
	// construction/destruction
	mac_osd_interface(mac_options &options);
	virtual ~mac_osd_interface();

	// general overridables
	virtual void init(running_machine &machine) override;
	virtual void update(bool skip_redraw) override;
	virtual void input_update() override;

	// input overridables
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist) override;

	virtual void video_register() override;

	virtual bool video_init() override;
	virtual bool window_init() override;

	virtual void video_exit() override;
	virtual void window_exit() override;

	// sdl specific
	void poll_inputs(running_machine &machine);
	void release_keys();
	bool should_hide_mouse();
	void process_events_buf();

	virtual mac_options &options() override { return m_options; }

protected:
	virtual void build_slider_list() override;
	virtual void update_slider_list() override;

private:
	virtual void osd_exit() override;

	void extract_video_config();
	void output_oslog(const char *buffer);

	mac_options &m_options;
};

//============================================================
//  macwork.cpp
//============================================================

extern int osd_num_processors;

#endif
