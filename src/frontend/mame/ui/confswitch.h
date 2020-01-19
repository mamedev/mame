// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Vas Crabb
/***************************************************************************

    ui/confswitch.h

    Configuration/DIP switches menu.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_CONFSWITCH_H
#define MAME_FRONTEND_UI_CONFSWITCH_H

#include "ui/menu.h"

#include <functional>
#include <vector>


namespace ui {

class menu_confswitch : public menu
{
public:
	virtual ~menu_confswitch() override;

protected:
	struct field_descriptor
	{
		field_descriptor(ioport_field &f) noexcept;

		std::reference_wrapper<ioport_field> field;
	};

	struct switch_group_descriptor
	{
		switch_group_descriptor(ioport_field const &f, ioport_diplocation const &loc) noexcept;

		bool matches(ioport_field const &f, ioport_diplocation const &loc) const noexcept;
		unsigned switch_count() const noexcept;

		char const *name;
		std::reference_wrapper<device_t> owner;
		uint32_t mask;
		uint32_t state;
	};

	using field_vector = std::vector<field_descriptor>;
	using switch_group_vector = std::vector<switch_group_descriptor>;

	menu_confswitch(mame_ui_manager &mui, render_container &container, uint32_t type);

	virtual void populate(float &customtop, float &custombottom) override;

	field_vector const &fields() { return m_fields; }
	switch_group_vector const &switch_groups() { return m_switch_groups; }
	unsigned active_switch_groups() const { return m_active_switch_groups; }

	int m_dipcount;

private:
	virtual void handle() override;

	void find_fields();

	field_vector m_fields;
	switch_group_vector m_switch_groups;
	unsigned m_active_switch_groups;
	int const m_type;
};


class menu_settings_dip_switches : public menu_confswitch
{
public:
	menu_settings_dip_switches(mame_ui_manager &mui, render_container &container);
	virtual ~menu_settings_dip_switches() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	virtual void populate(float &customtop, float &custombottom) override;

	unsigned m_visible_switch_groups;
};


class menu_settings_machine_config : public menu_confswitch
{
public:
	menu_settings_machine_config(mame_ui_manager &mui, render_container &container);
	virtual ~menu_settings_machine_config();
};

} // namespace ui

#endif // MAME_FRONTEND_UI_CONFSWITCH_H
