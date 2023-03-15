// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawnone.cpp - stub "nothing" drawer
//
//============================================================

#include "render_module.h"

#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"
#include "modules/osdwindow.h"

#include "main.h"
#include "render.h"

#include <memory>


namespace osd {

namespace {

class renderer_none : public osd_renderer
{
public:
	renderer_none(osd_window &window) : osd_renderer(window) { }

	virtual int create() override { return 0; }
	virtual render_primitive_list *get_primitives() override;
	virtual int draw(const int update) override { return 0; }
	virtual void save() override { }
	virtual void record() override { }
	virtual void toggle_fsfx() override { }
};

render_primitive_list *renderer_none::get_primitives()
{
	osd_dim const dimensions = window().get_size();
	if ((dimensions.width() <= 0) || (dimensions.height() <= 0))
		return nullptr;

	window().target()->set_bounds(dimensions.width(), dimensions.height(), window().pixel_aspect());
	return &window().target()->get_primitives();
}


class video_none : public osd_module, public render_module
{
public:
	video_none() : osd_module(OSD_RENDERER_PROVIDER, "none") { }

	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override { }

	virtual std::unique_ptr<osd_renderer> create(osd_window &window) override;

protected:
	virtual unsigned flags() const override { return 0; }
};

int video_none::init(osd_interface &osd, osd_options const &options)
{
	if (!emulator_info::standalone() && (options.seconds_to_run() == 0))
		osd_printf_warning("Warning: -video none doesn't make much sense without -seconds_to_run\n");

	return 0;
}

std::unique_ptr<osd_renderer> video_none::create(osd_window &window)
{
	return std::make_unique<renderer_none>(window);
}

} // anonymous namespace

} // namespace osd


MODULE_DEFINITION(RENDERER_NONE, osd::video_none)
