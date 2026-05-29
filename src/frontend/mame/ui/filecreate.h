// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/filecreate.h

    MESS's clunky built-in file manager

***************************************************************************/

#ifndef MAME_FRONTEND_UI_FILECREATE_H
#define MAME_FRONTEND_UI_FILECREATE_H

#pragma once

#include "ui/menu.h"

#include "imagedev/floppy.h"

#include <functional>
#include <string>
#include <string_view>
#include <vector>


class floppy_image_format_t;

namespace ui {

// ======================> menu_confirm_save_as

class menu_confirm_save_as : public menu
{
public:
	using handler_function = std::function<void ()>;

	menu_confirm_save_as(mame_ui_manager &mui, render_target &target, handler_function &&handler);
	virtual ~menu_confirm_save_as() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	handler_function m_handler;
};


// ======================> menu_file_create

class menu_file_create : public menu
{
public:
	using handler_function = std::function<void (std::string const &)>;

	menu_file_create(
			mame_ui_manager &mui,
			render_target &target,
			device_image_interface &image,
			std::string_view current_directory,
			std::string &&starting_name,
			handler_function &&handler);
	virtual ~menu_file_create() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual bool custom_ui_back() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	handler_function                m_handler;
	device_image_interface &        m_image;
	std::string_view const          m_current_directory;
	image_device_format const *     m_current_format;
	std::string                     m_filename;
};

// ======================> menu_select_format

class menu_select_format : public menu
{
public:
	using handler_function = std::function<void (floppy_image_format_t const &)>;

	menu_select_format(
			mame_ui_manager &mui,
			render_target &target,
			floppy_image_device &fd,
			std::string_view name,
			handler_function &&handler);

	virtual ~menu_select_format() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	// internal state
	handler_function const                      m_handler;
	std::vector<floppy_image_format_t const *>  m_formats;
	size_t                                      m_ext_match;
};

// ======================> menu_select_floppy_init

class menu_select_floppy_init : public menu
{
public:
	using handler_function = std::function<void (floppy_image_device::fs_info const &)>;

	menu_select_floppy_init(
			mame_ui_manager &mui,
			render_target &target,
			std::vector<std::reference_wrapper<floppy_image_device::fs_info const> > &&fs,
			handler_function &&handler);
	virtual ~menu_select_floppy_init() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	// internal state
	handler_function const                                                      m_handler;
	std::vector<std::reference_wrapper<floppy_image_device::fs_info const > >   m_fs;
};


} // namespace ui

#endif // MAME_FRONTEND_UI_FILECREATE_H
