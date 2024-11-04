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
		struct toggle
		{
			ioport_field *field;
			ioport_value mask;
		};

		switch_group_descriptor(ioport_field const &f, ioport_diplocation const &loc) noexcept;

		bool matches(ioport_field const &f, ioport_diplocation const &loc) const noexcept;
		unsigned switch_count() const noexcept;

		char const *name;
		std::reference_wrapper<device_t> owner;
		toggle toggles[32];
		uint32_t mask;
		uint32_t state;
	};

	using field_vector = std::vector<field_descriptor>;
	using switch_group_vector = std::vector<switch_group_descriptor>;

	menu_confswitch(mame_ui_manager &mui, render_container &container, uint32_t type);

	virtual void menu_activated() override;
	virtual void populate() override;

	field_vector const &fields() { return m_fields; }
	switch_group_vector const &switch_groups() { return m_switch_groups; }
	unsigned active_switch_groups() const { return m_active_switch_groups; }

private:
	virtual bool handle(event const *ev) override;

	void find_fields();

	field_vector m_fields;
	switch_group_vector m_switch_groups;
	unsigned m_active_switch_groups;
	int const m_type;
	bool m_changed;
};


class menu_settings_dip_switches : public menu_confswitch
{
public:
	menu_settings_dip_switches(mame_ui_manager &mui, render_container &container);
	virtual ~menu_settings_dip_switches() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual std::tuple<int, bool, bool> custom_pointer_updated(bool changed, ui_event const &uievt) override;

private:
	virtual void populate() override;

	std::vector<float> m_switch_group_y;
	unsigned m_visible_switch_groups;
	float m_single_width;
	float m_nub_width;
	float m_first_nub;
	float m_clickable_height;
};


class menu_settings_machine_config : public menu_confswitch
{
public:
	menu_settings_machine_config(mame_ui_manager &mui, render_container &container);
	virtual ~menu_settings_machine_config();
};

} // namespace ui

#endif // MAME_FRONTEND_UI_CONFSWITCH_H
