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


class floppy_image_format_t;

namespace ui {

// ======================> menu_confirm_save_as

class menu_confirm_save_as : public menu
{
public:
	menu_confirm_save_as(mame_ui_manager &mui, render_container &container, bool &yes);
	virtual ~menu_confirm_save_as() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	bool &m_yes;
};


// ======================> menu_file_create

class menu_file_create : public menu
{
public:
	menu_file_create(mame_ui_manager &mui, render_container &container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool &ok);
	virtual ~menu_file_create() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual bool custom_ui_back() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	bool &                          m_ok;
	device_image_interface *        m_image;
	std::string &                   m_current_directory;
	std::string &                   m_current_file;
	const image_device_format *     m_current_format;
	std::string                     m_filename;
};

// ======================> menu_select_format

class menu_select_format : public menu
{
public:
	menu_select_format(mame_ui_manager &mui, render_container &container,
					   const std::vector<const floppy_image_format_t *> &formats, int ext_match, const floppy_image_format_t **result);
	virtual ~menu_select_format() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	// internal state
	std::vector<const floppy_image_format_t *> m_formats;
	int                                        m_ext_match;
	const floppy_image_format_t *             *m_result;
};

// ======================> menu_select_floppy_init

class menu_select_floppy_init : public menu
{
public:
	menu_select_floppy_init(mame_ui_manager &mui, render_container &container,
		std::vector<std::reference_wrapper<const floppy_image_device::fs_info>> &&fs, int *result);
	virtual ~menu_select_floppy_init() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	// internal state
	std::vector<std::reference_wrapper<const floppy_image_device::fs_info>> m_fs;
	int *                                                                   m_result;
};


} // namespace ui

#endif // MAME_FRONTEND_UI_FILECREATE_H
