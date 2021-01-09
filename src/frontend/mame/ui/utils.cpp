// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/***************************************************************************

    ui/utils.cpp

    Internal UI user interface.

***************************************************************************/

#include "emu.h"
#include "ui/utils.h"

#include "ui/inifile.h"
#include "ui/selector.h"

#include "language.h"
#include "mame.h"

#include "drivenum.h"
#include "rendfont.h"
#include "romload.h"
#include "softlist.h"

#include <atomic>
#include <bitset>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <unordered_set>


namespace ui {

namespace {

constexpr char const *SOFTWARE_REGIONS[] = {
		"arab", "arg", "asia", "aus", "aut",
		"bel", "blr", "bra",
		"can", "chi", "chn", "cze",
		"den",
		"ecu", "esp", "euro",
		"fin", "fra",
		"gbr", "ger", "gre",
		"hkg", "hun",
		"irl", "isr", "isv", "ita",
		"jpn",
		"kaz", "kor",
		"lat", "lux",
		"mex",
		"ned", "nld", "nor", "nzl",
		"pol",
		"rus",
		"slo", "spa", "sui", "swe",
		"tha", "tpe", "tw",
		"uk", "ukr", "usa" };

constexpr char const *MACHINE_FILTER_NAMES[machine_filter::COUNT] = {
		__("Unfiltered"),
		__("Available"),
		__("Unavailable"),
		__("Working"),
		__("Not Working"),
		__("Mechanical"),
		__("Not Mechanical"),
		__("Category"),
		__("Favorites"),
		__("BIOS"),
		__("Not BIOS"),
		__("Parents"),
		__("Clones"),
		__("Manufacturer"),
		__("Year"),
		__("Save Supported"),
		__("Save Unsupported"),
		__("CHD Required"),
		__("No CHD Required"),
		__("Vertical Screen"),
		__("Horizontal Screen"),
		__("Custom Filter") };

constexpr char const *SOFTWARE_FILTER_NAMES[software_filter::COUNT] = {
		__("Unfiltered"),
		__("Available"),
		__("Unavailable"),
		__("Favorites"),
		__("Parents"),
		__("Clones"),
		__("Year"),
		__("Publisher"),
		__("Supported"),
		__("Partially Supported"),
		__("Unsupported"),
		__("Release Region"),
		__("Device Type"),
		__("Software List"),
		__("Custom Filter") };



//-------------------------------------------------
//  base implementation for simple filters
//-------------------------------------------------

template <class Base, typename Base::type Type>
class simple_filter_impl_base : public Base
{
public:
	virtual char const *config_name() const override { return Base::config_name(Type); }
	virtual char const *display_name() const override { return Base::display_name(Type); }
	virtual char const *filter_text() const override { return nullptr; }

	virtual void show_ui(mame_ui_manager &mui, render_container &container, std::function<void (Base &)> &&handler) override
	{
		handler(*this);
	}

	virtual bool wants_adjuster() const override { return false; }
	virtual char const *adjust_text() const override { return filter_text(); }
	virtual uint32_t arrow_flags() const override { return 0; }
	virtual bool adjust_left() override { return false; }
	virtual bool adjust_right() override { return false; }

	virtual void save_ini(emu_file &file, unsigned indent) const override
	{
		file.puts(util::string_format("%2$*1$s%3$s = 1\n", 2 * indent, "", config_name()));
	}

	virtual typename Base::type get_type() const override { return Type; }

	virtual std::string adorned_display_name(typename Base::type n) const override
	{
		std::string result;
		if (Type == n)
			result = convert_command_glyph("_> ");
		result.append(Base::display_name(n));
		return result;
	}

	using Base::config_name;
	using Base::display_name;

protected:
	simple_filter_impl_base() { }
};



//-------------------------------------------------
//  base implementation for single-choice filters
//-------------------------------------------------

template <class Base, typename Base::type Type>
class choice_filter_impl_base : public simple_filter_impl_base<Base, Type>
{
public:
	virtual char const *filter_text() const override { return selection_valid() ? selection_text().c_str() : nullptr; }

	virtual void show_ui(mame_ui_manager &mui, render_container &container, std::function<void (Base &)> &&handler) override
	{
		if (m_choices.empty())
		{
			handler(*this);
		}
		else
		{
			menu::stack_push<menu_selector>(
					mui, container,
					std::vector<std::string>(m_choices), // ouch, a vector copy!
					m_selection,
					[this, cb = std::move(handler)] (int selection)
					{
						m_selection = selection;
						cb(*this);
					});
		}
	}

	virtual bool wants_adjuster() const override { return have_choices(); }

	virtual uint32_t arrow_flags() const override
	{
		return ((have_choices() && m_selection) ? menu::FLAG_LEFT_ARROW : 0) | ((m_choices.size() > (m_selection + 1)) ? menu::FLAG_RIGHT_ARROW : 0);
	}

	virtual bool adjust_left() override
	{
		if (!have_choices() || !m_selection)
			return false;
		m_selection = (std::min)(m_selection - 1, unsigned(m_choices.size() - 1));
		return true;
	}

	virtual bool adjust_right() override
	{
		if (m_choices.size() <= (m_selection + 1))
			return false;
		++m_selection;
		return true;
	}

	virtual void save_ini(emu_file &file, unsigned indent) const override
	{
		char const *const text(filter_text());
		file.puts(util::string_format("%2$*1$s%3$s = %4$s\n", 2 * indent, "", this->config_name(), text ? text : ""));
	}

protected:
	choice_filter_impl_base(std::vector<std::string> const &choices, char const *value)
		: m_choices(choices)
		, m_selection(0U)
	{
		if (value)
		{
			std::vector<std::string>::const_iterator const found(std::find(choices.begin(), choices.end(), value));
			if (choices.end() != found)
				m_selection = std::distance(choices.begin(), found);
		}
	}

	bool have_choices() const { return !m_choices.empty(); }
	bool selection_valid() const { return m_choices.size() > m_selection; }
	unsigned selection_index() const { return m_selection; }
	std::string const &selection_text() const { return m_choices[m_selection]; }

private:
	std::vector<std::string> const &m_choices;
	unsigned m_selection;
};



//-------------------------------------------------
//  base implementation for composite filters
//-------------------------------------------------

template <class Impl, class Base, typename Base::type Type>
class composite_filter_impl_base : public simple_filter_impl_base<Base, Type>
{
public:
	virtual void show_ui(mame_ui_manager &mui, render_container &container, std::function<void (Base &)> &&handler) override;

	virtual bool wants_adjuster() const override { return true; }
	virtual char const *adjust_text() const override { return _("<set up filters>"); }

	virtual void save_ini(emu_file &file, unsigned indent) const override
	{
		auto const tail(std::find_if(std::begin(m_filters), std::end(m_filters), [] (typename Base::ptr const &flt) { return !flt; }));
		file.puts(util::string_format("%2$*1$s%3$s = %4$d\n", 2 * indent, "", this->config_name(), std::distance(std::begin(m_filters), tail)));
		for (auto it = std::begin(m_filters); tail != it; ++it)
			(*it)->save_ini(file, indent + 1);
	}

	virtual std::string adorned_display_name(typename Base::type n) const override
	{
		std::string result;
		if (Type == n)
			result = convert_command_glyph("_> ");
		else
		{
			for (unsigned i = 0; (MAX > i) && m_filters[i]; ++i)
			{
				if (m_filters[i]->get_type() == n)
				{
					result = convert_command_glyph(util::string_format("@custom%u ", i + 1));
					break;
				}
			}
		}
		result.append(Base::display_name(n));
		return result;
	}

	virtual bool apply(typename Base::entry_type const &info) const override
	{
		std::bitset<Base::COUNT> inclusions, included;
		for (typename Base::ptr const &flt : m_filters)
		{
			if (!flt)
				break;

			typename Base::type const t(flt->get_type());
			if (Impl::is_inclusion(t))
			{
				if (!included.test(t))
				{
					inclusions.set(t);
					included.set(t, flt->apply(info));
				}
			}
			else if (!flt->apply(info))
			{
				return false;
			}
		}
		return inclusions == included;
	}

protected:
	composite_filter_impl_base() { }

	void populate(char const *value, emu_file *file, unsigned indent)
	{
		// try to load filters from a file
		if (value && file)
		{
			unsigned const cnt(unsigned((std::max)(std::min(int(MAX), std::atoi(value)), 0)));
			for (unsigned i = 0; cnt > i; ++i)
			{
				typename Base::ptr flt(static_cast<Impl &>(*this).create(*file, indent + 1));
				if (!flt || !check_type(i, flt->get_type()))
					break;
				m_filters[i] = std::move(flt);
			}
		}

		// instantiate first allowed filter type if we're still empty
		for (typename Base::type t = Base::FIRST; (Base::COUNT > t) && !m_filters[0]; ++t)
		{
			if (Impl::type_allowed(0, t))
				m_filters[0] = static_cast<Impl &>(*this).create(t);
		}
	}

private:
	static constexpr unsigned MAX = 8;

	class menu_configure : public menu
	{
	public:
		menu_configure(
				mame_ui_manager &mui,
				render_container &container,
				Impl &parent,
				std::function<void (Base &filter)> &&handler)
			: menu(mui, container)
			, m_parent(parent)
			, m_handler(std::move(handler))
			, m_added(false)
		{
		}

		virtual ~menu_configure() override { m_handler(m_parent); }

	protected:
		virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override
		{
			char const *const text[] = { _("Select custom filters:") };
			draw_text_box(
					std::begin(text), std::end(text),
					x, x2, y - top, y - ui().box_tb_border(),
					ui::text_layout::CENTER, ui::text_layout::NEVER, false,
					ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
		}

	private:
		enum : uintptr_t
		{
			FILTER_FIRST = 1,
			FILTER_LAST = FILTER_FIRST + MAX - 1,
			ADJUST_FIRST,
			ADJUST_LAST = ADJUST_FIRST + MAX - 1,
			REMOVE_FILTER,
			ADD_FILTER
		};

		virtual void populate(float &customtop, float &custombottom) override;
		virtual void handle() override;

		bool set_filter_type(unsigned pos, typename Base::type n)
		{
			if (!m_parent.m_filters[pos] || (m_parent.m_filters[pos]->get_type()))
			{
				save_filter(pos);
				retrieve_filter(pos, n);
				return true;
			}
			else
			{
				return false;
			}
		}

		bool append_filter()
		{
			unsigned pos = 0;
			while (m_parent.m_filters[pos])
			{
				if (MAX <= ++pos)
					return false;
			}
			for (typename Base::type candidate = Base::FIRST; Base::COUNT > candidate; ++candidate)
			{
				if (m_parent.check_type(pos, candidate))
				{
					set_filter_type(pos, candidate);
					return true;
				}
			}
			return false;
		}

		bool drop_last_filter()
		{
			for (unsigned i = 2; MAX >= i; ++i)
			{
				if ((MAX <= i) || !m_parent.m_filters[i])
				{
					save_filter(i - 1);
					m_parent.m_filters[i - 1].reset();
					return true;
				}
			}
			return false;
		}

		void save_filter(unsigned pos)
		{
			typename Base::ptr &flt(m_parent.m_filters[pos]);
			if (flt && flt->wants_adjuster())
				m_saved_filters[pos][flt->get_type()] = std::move(flt);
		}

		void retrieve_filter(unsigned pos, typename Base::type n)
		{
			typename Base::ptr &flt(m_parent.m_filters[pos]);
			auto const found(m_saved_filters[pos].find(n));
			if (m_saved_filters[pos].end() != found)
			{
				flt = std::move(found->second);
				m_saved_filters[pos].erase(found);
			}
			else
			{
				flt = m_parent.create(n);
			}
		}

		uint32_t get_arrow_flags(unsigned pos)
		{
			uint32_t result(0);
			typename Base::type const current(m_parent.m_filters[pos]->get_type());

			// look for a lower type that's allowed and isn't contradictory
			typename Base::type prev(current);
			while ((Base::FIRST < prev) && !(FLAG_LEFT_ARROW & result))
			{
				if (m_parent.check_type(pos, --prev))
					result |= FLAG_LEFT_ARROW;
			}

			// look for a higher type that's allowed and isn't contradictory
			typename Base::type next(current);
			while ((Base::LAST > next) && !(FLAG_RIGHT_ARROW & result))
			{
				if (m_parent.check_type(pos, ++next))
					result |= FLAG_RIGHT_ARROW;
			}

			return result;
		}

		Impl &m_parent;
		std::map<typename Base::type, typename Base::ptr> m_saved_filters[MAX];
		std::function<void (Base &)> m_handler;
		bool m_added;
	};

	bool check_type(unsigned pos, typename Base::type candidate)
	{
		if (!Impl::type_allowed(pos, candidate))
			return false;
		unsigned j = 0;
		while ((MAX > j) && m_filters[j] && ((pos == j) || !Impl::types_contradictory(m_filters[j]->get_type(), candidate)))
			++j;
		return (MAX <= j) || !m_filters[j];
	};

	typename Base::ptr m_filters[MAX];
};

template <class Impl, class Base, typename Base::type Type>
void composite_filter_impl_base<Impl, Base, Type>::show_ui(
		mame_ui_manager &mui,
		render_container &container,
		std::function<void (Base &filter)> &&handler)
{
	menu::stack_push<menu_configure>(mui, container, static_cast<Impl &>(*this), std::move(handler));
}


template <class Impl, class Base, typename Base::type Type>
void composite_filter_impl_base<Impl, Base, Type>::menu_configure::populate(float &customtop, float &custombottom)
{
	// add items for each active filter
	unsigned i = 0;
	for (i = 0; (MAX > i) && m_parent.m_filters[i]; ++i)
	{
		item_append(util::string_format(_("Filter %1$u"), i + 1), m_parent.m_filters[i]->display_name(), get_arrow_flags(i), (void *)(FILTER_FIRST + i));
		if (m_added)
			set_selected_index(item_count() - 2);
		if (m_parent.m_filters[i]->wants_adjuster())
		{
			std::string name(convert_command_glyph("^!"));
			item_append(name, m_parent.m_filters[i]->adjust_text(), m_parent.m_filters[i]->arrow_flags(), (void *)(ADJUST_FIRST + i));
		}
		item_append(menu_item_type::SEPARATOR);
	}
	m_added = false;

	// add remove/add handlers
	if (1 < i)
		item_append(_("Remove last filter"), 0, (void *)REMOVE_FILTER);
	if (MAX > i)
		item_append(_("Add filter"), 0, (void *)ADD_FILTER);
	item_append(menu_item_type::SEPARATOR);

	// leave space for heading
	customtop = ui().get_line_height() + 3.0f * ui().box_tb_border();
}

template <class Impl, class Base, typename Base::type Type>
void composite_filter_impl_base<Impl, Base, Type>::menu_configure::handle()
{
	const event *menu_event = process(PROCESS_LR_REPEAT);
	if (menu_event && menu_event->itemref)
	{
		m_added = false;
		bool changed(false);
		uintptr_t const ref(reinterpret_cast<uintptr_t>(menu_event->itemref));
		switch (menu_event->iptkey)
		{
		case IPT_UI_LEFT:
		case IPT_UI_RIGHT:
			if ((FILTER_FIRST <= ref) && (FILTER_LAST >= ref))
			{
				// change filter type
				unsigned const pos(ref - FILTER_FIRST);
				typename Base::type const current(m_parent.m_filters[pos]->get_type());
				if (IPT_UI_LEFT == menu_event->iptkey)
				{
					typename Base::type n(current);
					while ((Base::FIRST < n) && !changed)
					{
						if (m_parent.check_type(pos, --n))
							changed = set_filter_type(pos, n);
					}
				}
				else
				{
					typename Base::type n(current);
					while ((Base::LAST > n) && !changed)
					{
						if (m_parent.check_type(pos, ++n))
							changed = set_filter_type(pos, n);
					}
				}
			}
			else if ((ADJUST_FIRST <= ref) && (ADJUST_LAST >= ref))
			{
				// change filter value
				Base &pos(*m_parent.m_filters[ref - ADJUST_FIRST]);
				changed = (IPT_UI_LEFT == menu_event->iptkey) ? pos.adjust_left() : pos.adjust_right();
			}
			break;

		case IPT_UI_SELECT:
			if ((FILTER_FIRST <= ref) && (FILTER_LAST >= ref))
			{
				// show selector with non-contradictory types
				std::vector<typename Base::type> types;
				std::vector<std::string> names;
				types.reserve(Base::COUNT);
				names.reserve(Base::COUNT);
				int sel(-1);
				unsigned const pos(ref - FILTER_FIRST);
				typename Base::type const current(m_parent.m_filters[pos]->get_type());
				for (typename Base::type candidate = Base::FIRST; Base::COUNT > candidate; ++candidate)
				{
					if (Impl::type_allowed(pos, candidate))
					{
						if (current == candidate)
							sel = types.size();
						unsigned i = 0;
						while ((MAX > i) && m_parent.m_filters[i] && ((pos == i) || !Impl::types_contradictory(m_parent.m_filters[i]->get_type(), candidate)))
							++i;
						if ((MAX <= i) || !m_parent.m_filters[i])
						{
							types.emplace_back(candidate);
							names.emplace_back(Base::display_name(candidate));
						}
					}
				}
				menu::stack_push<menu_selector>(
						ui(),
						container(),
						std::move(names),
						sel,
						[this, pos, t = std::move(types)] (int selection)
						{
							if (set_filter_type(pos, t[selection]))
								reset(reset_options::REMEMBER_REF);
						});
			}
			else if ((ADJUST_FIRST <= ref) && (ADJUST_LAST >= ref))
			{
				// show selected filter's UI
				m_parent.m_filters[ref - ADJUST_FIRST]->show_ui(ui(), container(), [this] (Base &filter) { reset(reset_options::REMEMBER_REF); });
			}
			else if (REMOVE_FILTER == ref)
			{
				changed = drop_last_filter();
			}
			else if (ADD_FILTER == ref)
			{
				m_added = append_filter();
			}
			break;
		}

		// rebuild if anything changed
		if (changed)
			reset(reset_options::REMEMBER_REF);
		else if (m_added)
			reset(reset_options::SELECT_FIRST);
	}
}



//-------------------------------------------------
//  invertable machine filters
//-------------------------------------------------

template <machine_filter::type Type = machine_filter::AVAILABLE>
class available_machine_filter_impl : public simple_filter_impl_base<machine_filter, Type>
{
public:
	available_machine_filter_impl(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_system_info const &system) const override { return system.available; }
};


template <machine_filter::type Type = machine_filter::WORKING>
class working_machine_filter_impl : public simple_filter_impl_base<machine_filter, Type>
{
public:
	working_machine_filter_impl(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_system_info const &system) const override { return !(system.driver->flags & machine_flags::NOT_WORKING); }
};


template <machine_filter::type Type = machine_filter::MECHANICAL>
class mechanical_machine_filter_impl : public simple_filter_impl_base<machine_filter, Type>
{
public:
	mechanical_machine_filter_impl(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_system_info const &system) const override { return system.driver->flags & machine_flags::MECHANICAL; }
};


template <machine_filter::type Type = machine_filter::BIOS>
class bios_machine_filter_impl : public simple_filter_impl_base<machine_filter, Type>
{
public:
	bios_machine_filter_impl(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_system_info const &system) const override { return system.driver->flags & machine_flags::IS_BIOS_ROOT; }
};


template <machine_filter::type Type = machine_filter::PARENTS>
class parents_machine_filter_impl : public simple_filter_impl_base<machine_filter, Type>
{
public:
	parents_machine_filter_impl(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_system_info const &system) const override
	{
		bool const have_parent(strcmp(system.driver->parent, "0"));
		auto const parent_idx(have_parent ? driver_list::find(system.driver->parent) : -1);
		return !have_parent || (0 > parent_idx) || (driver_list::driver(parent_idx).flags & machine_flags::IS_BIOS_ROOT);
	}
};


template <machine_filter::type Type = machine_filter::CHD>
class chd_machine_filter_impl : public simple_filter_impl_base<machine_filter, Type>
{
public:
	chd_machine_filter_impl(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_system_info const &system) const override
	{
		for (tiny_rom_entry const *rom = system.driver->rom; !ROMENTRY_ISEND(rom); ++rom)
		{
			if (ROMENTRY_ISREGION(rom) && ROMREGION_ISDISKDATA(rom))
				return true;
		}
		return false;
	}
};


template <machine_filter::type Type = machine_filter::SAVE>
class save_machine_filter_impl : public simple_filter_impl_base<machine_filter, Type>
{
public:
	save_machine_filter_impl(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_system_info const &system) const override { return system.driver->flags & machine_flags::SUPPORTS_SAVE; }
};


template <machine_filter::type Type = machine_filter::VERTICAL>
class vertical_machine_filter_impl : public simple_filter_impl_base<machine_filter, Type>
{
public:
	vertical_machine_filter_impl(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_system_info const &system) const override { return system.driver->flags & machine_flags::SWAP_XY; }
};



//-------------------------------------------------
//  concrete machine filters
//-------------------------------------------------

class manufacturer_machine_filter : public choice_filter_impl_base<machine_filter, machine_filter::MANUFACTURER>
{
public:
	manufacturer_machine_filter(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: choice_filter_impl_base<machine_filter, machine_filter::MANUFACTURER>(data.manufacturers(), value)
	{
	}

	virtual bool apply(ui_system_info const &system) const override
	{
		if (!have_choices())
			return true;
		else if (!selection_valid())
			return false;

		std::string const name(machine_filter_data::extract_manufacturer(system.driver->manufacturer));
		return !name.empty() && (selection_text() == name);
	}
};


class year_machine_filter : public choice_filter_impl_base<machine_filter, machine_filter::YEAR>
{
public:
	year_machine_filter(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: choice_filter_impl_base<machine_filter, machine_filter::YEAR>(data.years(), value)
	{
	}

	virtual bool apply(ui_system_info const &system) const override { return !have_choices() || (selection_valid() && (selection_text() == system.driver->year)); }
};



//-------------------------------------------------
//  complementary machine filters
//-------------------------------------------------

template <template <machine_filter::type T> class Base, machine_filter::type Type>
class inverted_machine_filter : public Base<Type>
{
public:
	inverted_machine_filter(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: Base<Type>(data, value, file, indent)
	{
	}

	virtual bool apply(ui_system_info const &system) const override { return !Base<Type>::apply(system); }
};

using available_machine_filter      = available_machine_filter_impl<>;
using working_machine_filter        = working_machine_filter_impl<>;
using mechanical_machine_filter     = mechanical_machine_filter_impl<>;
using bios_machine_filter           = bios_machine_filter_impl<>;
using parents_machine_filter        = parents_machine_filter_impl<>;
using save_machine_filter           = save_machine_filter_impl<>;
using chd_machine_filter            = chd_machine_filter_impl<>;
using vertical_machine_filter       = vertical_machine_filter_impl<>;

using unavailable_machine_filter    = inverted_machine_filter<available_machine_filter_impl, machine_filter::UNAVAILABLE>;
using not_working_machine_filter    = inverted_machine_filter<working_machine_filter_impl, machine_filter::NOT_WORKING>;
using not_mechanical_machine_filter = inverted_machine_filter<mechanical_machine_filter_impl, machine_filter::NOT_MECHANICAL>;
using not_bios_machine_filter       = inverted_machine_filter<bios_machine_filter_impl, machine_filter::NOT_BIOS>;
using clones_machine_filter         = inverted_machine_filter<parents_machine_filter_impl, machine_filter::CLONES>;
using nosave_machine_filter         = inverted_machine_filter<save_machine_filter_impl, machine_filter::NOSAVE>;
using nochd_machine_filter          = inverted_machine_filter<chd_machine_filter_impl, machine_filter::NOCHD>;
using horizontal_machine_filter     = inverted_machine_filter<vertical_machine_filter_impl, machine_filter::HORIZONTAL>;



//-------------------------------------------------
//  dummy machine filters (special-cased in menu)
//-------------------------------------------------

template <machine_filter::type Type>
class inclusive_machine_filter_impl : public simple_filter_impl_base<machine_filter, Type>
{
public:
	inclusive_machine_filter_impl(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_system_info const &system) const override { return true; }
};

using all_machine_filter            = inclusive_machine_filter_impl<machine_filter::ALL>;
using favorite_machine_filter       = inclusive_machine_filter_impl<machine_filter::FAVORITE>;



//-------------------------------------------------
//  category machine filter
//-------------------------------------------------

class category_machine_filter : public simple_filter_impl_base<machine_filter, machine_filter::CATEGORY>
{
public:
	category_machine_filter(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: m_ini(0)
		, m_group(0)
		, m_include_clones(false)
		, m_adjust_text()
		, m_cache()
		, m_cache_valid(false)
	{
		inifile_manager const &mgr(mame_machine_manager::instance()->inifile());
		if (value)
		{
			std::string_view const s(value);
			std::string_view::size_type const split(s.find('/'));
			std::string_view const ini(s.substr(0, split));

			for (unsigned i = 0; mgr.get_file_count() > i; ++i)
			{
				if (mgr.get_file_name(i) == ini)
				{
					m_ini = i;
					if (std::string_view::npos != split)
					{
						std::string_view const group(s.substr(split + 1));
						for (unsigned j = 0; mgr.get_category_count(i) > j; ++j)
						{
							if (mgr.get_category_name(i, j) == group)
							{
								m_group = j;
								break;
							}
						}
					}
					break;
				}
			}
		}

		if (mgr.get_file_count() > m_ini)
			m_include_clones = include_clones_default(mgr.get_file_name(m_ini));

		set_adjust_text();
	}

	virtual char const *filter_text() const override
	{
		inifile_manager const &mgr(mame_machine_manager::instance()->inifile());
		return ((mgr.get_file_count() > m_ini) && (mgr.get_category_count(m_ini) > m_group)) ? m_adjust_text.c_str() : nullptr;
	}

	virtual void show_ui(mame_ui_manager &mui, render_container &container, std::function<void (machine_filter &)> &&handler) override;

	virtual bool wants_adjuster() const override { return mame_machine_manager::instance()->inifile().get_file_count(); }
	virtual char const *adjust_text() const override { return m_adjust_text.c_str(); }

	virtual void save_ini(emu_file &file, unsigned indent) const override
	{
		char const *const text(filter_text());
		file.puts(util::string_format("%2$*1$s%3$s = %4$s\n", 2 * indent, "", this->config_name(), text ? text : ""));
	}

	virtual bool apply(ui_system_info const &system) const override
	{
		inifile_manager const &mgr(mame_machine_manager::instance()->inifile());
		if (!mgr.get_file_count())
			return true;
		else if ((mgr.get_file_count() <= m_ini) || (mgr.get_category_count(m_ini) <= m_group))
			return false;

		if (!m_cache_valid)
			mame_machine_manager::instance()->inifile().load_ini_category(m_ini, m_group, m_cache);
		m_cache_valid = true;

		if (m_cache.end() != m_cache.find(system.driver))
			return true;

		if (m_include_clones)
		{
			int const found(driver_list::find(system.driver->parent));
			return m_cache.end() != m_cache.find(&driver_list::driver(found));
		}

		return false;
	}

private:
	class menu_configure : public menu
	{
	public:
		menu_configure(
				mame_ui_manager &mui,
				render_container &container,
				category_machine_filter &parent,
				std::function<void (machine_filter &filter)> &&handler)
			: menu(mui, container)
			, m_parent(parent)
			, m_handler(std::move(handler))
			, m_state(std::make_unique<std::pair<unsigned, bool> []>(mame_machine_manager::instance()->inifile().get_file_count()))
			, m_ini(parent.m_ini)
		{
			inifile_manager const &mgr(mame_machine_manager::instance()->inifile());
			for (size_t i = 0; mgr.get_file_count() > i; ++i)
			{
				m_state[i].first = (m_ini == i) ? m_parent.m_group : 0U;
				m_state[i].second = (m_ini == i) ? m_parent.m_include_clones : include_clones_default(mgr.get_file_name(i));
			}
		}

		virtual ~menu_configure() override
		{
			bool const valid(mame_machine_manager::instance()->inifile().get_file_count() > m_ini);
			unsigned const group(valid ? m_state[m_ini].first : 0);
			if ((m_ini != m_parent.m_ini) || (group != m_parent.m_group))
			{
				m_parent.m_cache.clear();
				m_parent.m_cache_valid = false;
			}
			m_parent.m_ini = m_ini;
			m_parent.m_group = group;
			m_parent.m_include_clones = valid ? m_state[m_ini].second : false;
			m_parent.set_adjust_text();
			m_handler(m_parent);
		}

	protected:
		virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override
		{
			char const *const text[] = { _("Select category:") };
			draw_text_box(
					std::begin(text), std::end(text),
					x, x2, y - top, y - ui().box_tb_border(),
					ui::text_layout::CENTER, ui::text_layout::NEVER, false,
					ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
		}

	private:
		enum : uintptr_t
		{
			INI_FILE = 1,
			SYSTEM_GROUP,
			INCLUDE_CLONES
		};

		virtual void populate(float &customtop, float &custombottom) override;
		virtual void handle() override;

		category_machine_filter &m_parent;
		std::function<void (machine_filter &)> m_handler;
		std::unique_ptr<std::pair<unsigned, bool> []> const m_state;
		unsigned m_ini;
	};

	void set_adjust_text()
	{
		inifile_manager const &mgr(mame_machine_manager::instance()->inifile());
		unsigned const filecnt(mgr.get_file_count());
		if (!filecnt)
		{
			m_adjust_text = _("[no category INI files]");
		}
		else
		{
			m_ini = std::min(m_ini, filecnt - 1);
			unsigned const groupcnt(mgr.get_category_count(m_ini));
			if (!groupcnt)
			{
				m_adjust_text = _("[no groups in INI file]");
			}
			else
			{
				m_group = std::min(m_group, groupcnt - 1);
				m_adjust_text = util::string_format("%s/%s", mgr.get_file_name(m_ini), mgr.get_category_name(m_ini, m_group));
			}
		}
	}

	static bool include_clones_default(std::string const &name)
	{
		return !core_stricmp(name.c_str(), "category.ini") || !core_stricmp(name.c_str(), "alltime.ini");
	}

	unsigned m_ini, m_group;
	bool m_include_clones;
	std::string m_adjust_text;
	mutable std::unordered_set<game_driver const *> m_cache;
	mutable bool m_cache_valid;
};

void category_machine_filter::show_ui(mame_ui_manager &mui, render_container &container, std::function<void (machine_filter &)> &&handler)
{
	menu::stack_push<menu_configure>(mui, container, *this, std::move(handler));
}


void category_machine_filter::menu_configure::populate(float &customtop, float &custombottom)
{
	inifile_manager const &mgr(mame_machine_manager::instance()->inifile());
	unsigned const filecnt(mgr.get_file_count());
	if (!filecnt)
	{
		item_append(_("No category INI files found"), FLAG_DISABLE, nullptr);
	}
	else
	{
		m_ini = std::min(m_ini, filecnt - 1);
		item_append(_("File"), mgr.get_file_name(m_ini), get_arrow_flags(0U, filecnt - 1, m_ini), reinterpret_cast<void *>(INI_FILE));
		unsigned const groupcnt(mgr.get_category_count(m_ini));
		if (!groupcnt)
		{
			item_append(_("No groups found in category file"), FLAG_DISABLE, nullptr);
		}
		else
		{
			m_state[m_ini].first = std::min(m_state[m_ini].first, groupcnt - 1);
			item_append(_("Group"), mgr.get_category_name(m_ini, m_state[m_ini].first), get_arrow_flags(0U, groupcnt - 1, m_state[m_ini].first), reinterpret_cast<void *>(SYSTEM_GROUP));
			item_append(_("Include clones"), m_state[m_ini].second ? _("Yes") : _("No"), m_state[m_ini].second ? FLAG_LEFT_ARROW : FLAG_RIGHT_ARROW, reinterpret_cast<void *>(INCLUDE_CLONES));
		}
	}
	item_append(menu_item_type::SEPARATOR);
	customtop = ui().get_line_height() + 3.0f * ui().box_tb_border();
}

void category_machine_filter::menu_configure::handle()
{
	const event *menu_event = process(PROCESS_LR_REPEAT);
	if (menu_event && menu_event->itemref)
	{
		bool changed(false);
		uintptr_t const ref(reinterpret_cast<uintptr_t>(menu_event->itemref));
		inifile_manager const &mgr(mame_machine_manager::instance()->inifile());
		switch (menu_event->iptkey)
		{
		case IPT_UI_LEFT:
			if ((INI_FILE == ref) && m_ini)
			{
				--m_ini;
				changed = true;
			}
			else if ((SYSTEM_GROUP == ref) && m_state[m_ini].first)
			{
				--m_state[m_ini].first;
				changed = true;
			}
			else if ((INCLUDE_CLONES == ref) && m_state[m_ini].second)
			{
				m_state[m_ini].second = false;
				changed = true;
			}
			break;
		case IPT_UI_RIGHT:
			if ((INI_FILE == ref) && (mgr.get_file_count() > (m_ini + 1)))
			{
				++m_ini;
				changed = true;
			}
			else if ((SYSTEM_GROUP == ref) && (mgr.get_category_count(m_ini) > (m_state[m_ini].first + 1)))
			{
				++m_state[m_ini].first;
				changed = true;
			}
			else if ((INCLUDE_CLONES == ref) && !m_state[m_ini].second)
			{
				m_state[m_ini].second = true;
				changed = true;
			}
			break;

		case IPT_UI_SELECT:
			if (INI_FILE == ref)
			{
				std::vector<std::string> choices;
				choices.reserve(mgr.get_file_count());
				for (size_t i = 0; mgr.get_file_count() > i; ++i)
					choices.emplace_back(mgr.get_file_name(i));
				menu::stack_push<menu_selector>(
						ui(),
						container(),
						std::move(choices),
						m_ini,
						[this] (int selection)
						{
							if (selection != m_ini)
							{
								m_ini = selection;
								reset(reset_options::REMEMBER_REF);
							}
						});
			}
			else if (SYSTEM_GROUP == ref)
			{
				std::vector<std::string> choices;
				choices.reserve(mgr.get_category_count(m_ini));
				for (size_t i = 0; mgr.get_category_count(m_ini) > i; ++i)
					choices.emplace_back(mgr.get_category_name(m_ini, i));
				menu::stack_push<menu_selector>(
						ui(),
						container(),
						std::move(choices),
						m_state[m_ini].first,
						[this] (int selection)
						{
							if (selection != m_state[m_ini].first)
							{
								m_state[m_ini].first = selection;
								reset(reset_options::REMEMBER_REF);
							}
						});
			}
			else if (INCLUDE_CLONES == ref)
			{
				m_state[m_ini].second = !m_state[m_ini].second;
				reset(reset_options::REMEMBER_REF);
			}
			break;
		}

		// rebuild if anything changed
		if (changed)
			reset(reset_options::REMEMBER_REF);
	}
}



//-------------------------------------------------
//  composite machine filter
//-------------------------------------------------

class custom_machine_filter : public composite_filter_impl_base<custom_machine_filter, machine_filter, machine_filter::CUSTOM>
{
public:
	custom_machine_filter(machine_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: composite_filter_impl_base<custom_machine_filter, machine_filter, machine_filter::CUSTOM>()
		, m_data(data)
	{
		populate(value, file, indent);
	}

	ptr create(type n) const { return machine_filter::create(n, m_data); }
	ptr create(emu_file &file, unsigned indent) const { return machine_filter::create(file, m_data, indent); }

	static bool type_allowed(unsigned pos, type n)
	{
		return (FIRST <= n) && (LAST >= n) && (ALL != n) && (FAVORITE != n) && (CUSTOM != n);
	}

	static bool types_contradictory(type n, type m)
	{
		switch (n)
		{
		case AVAILABLE:         return UNAVAILABLE == m;
		case UNAVAILABLE:       return AVAILABLE == m;
		case WORKING:           return NOT_WORKING == m;
		case NOT_WORKING:       return WORKING == m;
		case MECHANICAL:        return NOT_MECHANICAL == m;
		case NOT_MECHANICAL:    return MECHANICAL == m;
		case BIOS:              return NOT_BIOS == m;
		case NOT_BIOS:          return BIOS == m;
		case PARENTS:           return CLONES == m;
		case CLONES:            return PARENTS == m;
		case SAVE:              return NOSAVE == m;
		case NOSAVE:            return SAVE == m;
		case CHD:               return NOCHD == m;
		case NOCHD:             return CHD == m;
		case VERTICAL:          return HORIZONTAL == m;
		case HORIZONTAL:        return VERTICAL == m;

		case ALL:
		case CATEGORY:
		case FAVORITE:
		case MANUFACTURER:
		case YEAR:
		case CUSTOM:
		case COUNT:
			break;
		}
		return false;
	}

	static bool is_inclusion(type n)
	{
		return (CATEGORY == n) || (MANUFACTURER == n) || (YEAR == n);
	}

private:
	machine_filter_data const &m_data;
};



//-------------------------------------------------
//  concrete software filters
//-------------------------------------------------

class all_software_filter : public simple_filter_impl_base<software_filter, software_filter::ALL>
{
public:
	all_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_software_info const &info) const override { return true; }
};


class available_software_filter : public simple_filter_impl_base<software_filter, software_filter::AVAILABLE>
{
public:
	available_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_software_info const &info) const override { return info.available; }
};


class unavailable_software_filter : public simple_filter_impl_base<software_filter, software_filter::UNAVAILABLE>
{
public:
	unavailable_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_software_info const &info) const override { return !info.available; }
};


class favorite_software_filter : public simple_filter_impl_base<software_filter, software_filter::FAVORITE>
{
public:
	favorite_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: m_manager(mame_machine_manager::instance()->favorite())
	{
	}

	virtual bool apply(ui_software_info const &info) const override { return m_manager.is_favorite_software(info); }

private:
	favorite_manager const &m_manager;
};


class parents_software_filter : public simple_filter_impl_base<software_filter, software_filter::PARENTS>
{
public:
	parents_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_software_info const &info) const override { return info.parentname.empty(); }
};


class clones_software_filter : public simple_filter_impl_base<software_filter, software_filter::CLONES>
{
public:
	clones_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_software_info const &info) const override { return !info.parentname.empty(); }
};


class years_software_filter : public choice_filter_impl_base<software_filter, software_filter::YEAR>
{
public:
	years_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: choice_filter_impl_base<software_filter, software_filter::YEAR>(data.years(), value)
	{
	}

	virtual bool apply(ui_software_info const &info) const override { return !have_choices() || (selection_valid() && (selection_text() == info.year)); }
};


class publishers_software_filter : public choice_filter_impl_base<software_filter, software_filter::PUBLISHERS>
{
public:
	publishers_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: choice_filter_impl_base<software_filter, software_filter::PUBLISHERS>(data.publishers(), value)
	{
	}

	virtual bool apply(ui_software_info const &info) const override
	{
		if (!have_choices())
			return true;
		else if (!selection_valid())
			return false;

		std::string const name(software_filter_data::extract_publisher(info.publisher));
		return !name.empty() && (selection_text() == name);
	}
};


class supported_software_filter : public simple_filter_impl_base<software_filter, software_filter::SUPPORTED>
{
public:
	supported_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_software_info const &info) const override { return SOFTWARE_SUPPORTED_YES == info.supported; }
};



class partial_supported_software_filter : public simple_filter_impl_base<software_filter, software_filter::PARTIAL_SUPPORTED>
{
public:
	partial_supported_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_software_info const &info) const override { return SOFTWARE_SUPPORTED_PARTIAL == info.supported; }
};


class unsupported_software_filter : public simple_filter_impl_base<software_filter, software_filter::UNSUPPORTED>
{
public:
	unsupported_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent) { }

	virtual bool apply(ui_software_info const &info) const override { return SOFTWARE_SUPPORTED_NO == info.supported; }
};


class region_software_filter : public choice_filter_impl_base<software_filter, software_filter::REGION>
{
public:
	region_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: choice_filter_impl_base<software_filter, software_filter::REGION>(data.regions(), value)
	{
	}

	virtual bool apply(ui_software_info const &info) const override
	{
		if (!have_choices())
			return true;
		else if (!selection_valid())
			return false;

		std::string const name(software_filter_data::extract_region(info.longname));
		return !name.empty() && (selection_text() == name);
	}
};


class device_type_software_filter : public choice_filter_impl_base<software_filter, software_filter::DEVICE_TYPE>
{
public:
	device_type_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: choice_filter_impl_base<software_filter, software_filter::DEVICE_TYPE>(data.device_types(), value)
	{
	}

	virtual bool apply(ui_software_info const &info) const override { return !have_choices() || (selection_valid() && (selection_text() == info.devicetype)); }
};


class list_software_filter : public choice_filter_impl_base<software_filter, software_filter::LIST>
{
public:
	list_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: choice_filter_impl_base<software_filter, software_filter::LIST>(data.list_descriptions(), value)
		, m_data(data)
	{
	}

	virtual bool apply(ui_software_info const &info) const override
	{
		return !have_choices() || (selection_valid() && (m_data.list_names()[selection_index()] == info.listname));
	}

private:
	software_filter_data const &m_data;
};



//-------------------------------------------------
//  composite software filter
//-------------------------------------------------

class custom_software_filter : public composite_filter_impl_base<custom_software_filter, software_filter, software_filter::CUSTOM>
{
public:
	custom_software_filter(software_filter_data const &data, char const *value, emu_file *file, unsigned indent)
		: composite_filter_impl_base<custom_software_filter, software_filter, software_filter::CUSTOM>()
		, m_data(data)
	{
		populate(value, file, indent);
	}

	ptr create(type n) const { return software_filter::create(n, m_data); }
	ptr create(emu_file &file, unsigned indent) const { return software_filter::create(file, m_data, indent); }

	static bool type_allowed(unsigned pos, type n)
	{
		return (FIRST <= n) && (LAST >= n) && (ALL != n) && (CUSTOM != n);
	}

	static bool types_contradictory(type n, type m)
	{
		switch (n)
		{
		case AVAILABLE:         return UNAVAILABLE == m;
		case UNAVAILABLE:       return AVAILABLE == m;
		case PARENTS:           return CLONES == m;
		case CLONES:            return PARENTS == m;
		case SUPPORTED:         return (PARTIAL_SUPPORTED == m) || (UNSUPPORTED == m);
		case PARTIAL_SUPPORTED: return (SUPPORTED == m) || (UNSUPPORTED == m);
		case UNSUPPORTED:       return (SUPPORTED == m) || (PARTIAL_SUPPORTED == m);

		case ALL:
		case FAVORITE:
		case YEAR:
		case PUBLISHERS:
		case REGION:
		case DEVICE_TYPE:
		case LIST:
		case CUSTOM:
		case COUNT:
			break;
		}
		return false;
	}

	static bool is_inclusion(type n)
	{
		return (YEAR == n) || (PUBLISHERS == n) || (REGION == n) || (DEVICE_TYPE == n) || (LIST == n);
	}

private:
	software_filter_data const &m_data;
};

} // anonymous namespace



//-------------------------------------------------
//  static data for machine filters
//-------------------------------------------------

void machine_filter_data::add_manufacturer(std::string const &manufacturer)
{
	std::string name(extract_manufacturer(manufacturer));
	std::vector<std::string>::iterator const pos(std::lower_bound(m_manufacturers.begin(), m_manufacturers.end(), name));
	if ((m_manufacturers.end() == pos) || (*pos != name))
		m_manufacturers.emplace(pos, std::move(name));
}

void machine_filter_data::add_year(std::string const &year)
{
	std::vector<std::string>::iterator const pos(std::lower_bound(m_years.begin(), m_years.end(), year));
	if ((m_years.end() == pos) || (*pos != year))
		m_years.emplace(pos, year);
}

void machine_filter_data::finalise()
{
	std::stable_sort(m_manufacturers.begin(), m_manufacturers.end());
	std::stable_sort(m_years.begin(), m_years.end());
}

std::string machine_filter_data::extract_manufacturer(std::string const &manufacturer)
{
	size_t const found(manufacturer.find('('));
	if ((found != std::string::npos) && (found > 0))
		return manufacturer.substr(0, found - 1);
	else
		return manufacturer;
}

void machine_filter_data::set_filter(machine_filter::ptr &&filter)
{
	m_filters[filter->get_type()] = std::move(filter);
}

machine_filter &machine_filter_data::get_filter(machine_filter::type type)
{
	auto it(m_filters.find(type));
	if (m_filters.end() == it)
		it = m_filters.emplace(type, machine_filter::create(type, *this)).first;

	assert(it->second);
	return *it->second;
}

std::string machine_filter_data::get_config_string() const
{
	auto const active_filter(m_filters.find(m_current_filter));
	if (m_filters.end() != active_filter)
	{
		char const *const val(active_filter->second->filter_text());
		return val ? util::string_format("%s,%s", active_filter->second->config_name(), val) : active_filter->second->config_name();
	}
	else
	{
		return machine_filter::config_name(m_current_filter);
	}
}

bool machine_filter_data::load_ini(emu_file &file)
{
	machine_filter::ptr flt(machine_filter::create(file, *this));
	if (flt)
	{
		// TODO: it should possibly replace an existing item here, but it may be relying on that not happening because it never clears the first start flag
		m_current_filter = flt->get_type();
		m_filters.emplace(m_current_filter, std::move(flt));
		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------
//  static data for software filters
//-------------------------------------------------

void software_filter_data::add_region(std::string const &longname)
{
	std::string name(extract_region(longname));
	std::vector<std::string>::iterator const pos(std::lower_bound(m_regions.begin(), m_regions.end(), name));
	if ((m_regions.end() == pos) || (*pos != name))
		m_regions.emplace(pos, std::move(name));
}

void software_filter_data::add_publisher(std::string const &publisher)
{
	std::string name(extract_publisher(publisher));
	std::vector<std::string>::iterator const pos(std::lower_bound(m_publishers.begin(), m_publishers.end(), name));
	if ((m_publishers.end() == pos) || (*pos != name))
		m_publishers.emplace(pos, std::move(name));
}

void software_filter_data::add_year(std::string const &year)
{
	std::vector<std::string>::iterator const pos(std::lower_bound(m_years.begin(), m_years.end(), year));
	if ((m_years.end() == pos) || (*pos != year))
		m_years.emplace(pos, year);
}

void software_filter_data::add_device_type(std::string const &device_type)
{
	std::vector<std::string>::iterator const pos(std::lower_bound(m_device_types.begin(), m_device_types.end(), device_type));
	if ((m_device_types.end() == pos) || (*pos != device_type))
		m_device_types.emplace(pos, device_type);
}

void software_filter_data::add_list(std::string const &name, std::string const &description)
{
	m_list_names.emplace_back(name);
	m_list_descriptions.emplace_back(description);
}

void software_filter_data::finalise()
{
	std::stable_sort(m_regions.begin(), m_regions.end());
	std::stable_sort(m_publishers.begin(), m_publishers.end());
	std::stable_sort(m_years.begin(), m_years.end());
	std::stable_sort(m_device_types.begin(), m_device_types.end());
}

std::string software_filter_data::extract_region(std::string const &longname)
{
	std::string fullname(longname);
	strmakelower(fullname);
	std::string::size_type const found(fullname.find('('));
	if (found != std::string::npos)
	{
		std::string::size_type const ends(fullname.find_first_not_of("abcdefghijklmnopqrstuvwxyz", found + 1));
		std::string_view const temp(std::string_view(fullname).substr(found + 1, ends - found - 1));
		auto const match(std::find_if(
				std::begin(SOFTWARE_REGIONS),
				std::end(SOFTWARE_REGIONS),
				[&temp] (char const *elem) { return temp == elem; }));
		if (std::end(SOFTWARE_REGIONS) != match)
			return longname.substr(found + 1, (std::string::npos != ends) ? (ends - found - 1) : ends);
	}
	return "<none>";
}

std::string software_filter_data::extract_publisher(std::string const &publisher)
{
	std::string::size_type const found(publisher.find('('));
	return publisher.substr(0, found - ((found && (std::string::npos != found)) ? 1 : 0));
}



//-------------------------------------------------
//  public machine filter interface
//-------------------------------------------------

machine_filter::ptr machine_filter::create(type n, machine_filter_data const &data, char const *value, emu_file *file, unsigned indent)
{
	assert(COUNT > n);
	switch (n)
	{
	case ALL:
		return std::make_unique<all_machine_filter>(data, value, file, indent);
	case AVAILABLE:
		return std::make_unique<available_machine_filter>(data, value, file, indent);
	case UNAVAILABLE:
		return std::make_unique<unavailable_machine_filter>(data, value, file, indent);
	case WORKING:
		return std::make_unique<working_machine_filter>(data, value, file, indent);
	case NOT_WORKING:
		return std::make_unique<not_working_machine_filter>(data, value, file, indent);
	case MECHANICAL:
		return std::make_unique<mechanical_machine_filter>(data, value, file, indent);
	case NOT_MECHANICAL:
		return std::make_unique<not_mechanical_machine_filter>(data, value, file, indent);
	case CATEGORY:
		return std::make_unique<category_machine_filter>(data, value, file, indent);
	case FAVORITE:
		return std::make_unique<favorite_machine_filter>(data, value, file, indent);
	case BIOS:
		return std::make_unique<bios_machine_filter>(data, value, file, indent);
	case NOT_BIOS:
		return std::make_unique<not_bios_machine_filter>(data, value, file, indent);
	case PARENTS:
		return std::make_unique<parents_machine_filter>(data, value, file, indent);
	case CLONES:
		return std::make_unique<clones_machine_filter>(data, value, file, indent);
	case MANUFACTURER:
		return std::make_unique<manufacturer_machine_filter>(data, value, file, indent);
	case YEAR:
		return std::make_unique<year_machine_filter>(data, value, file, indent);
	case SAVE:
		return std::make_unique<save_machine_filter>(data, value, file, indent);
	case NOSAVE:
		return std::make_unique<nosave_machine_filter>(data, value, file, indent);
	case CHD:
		return std::make_unique<chd_machine_filter>(data, value, file, indent);
	case NOCHD:
		return std::make_unique<nochd_machine_filter>(data, value, file, indent);
	case VERTICAL:
		return std::make_unique<vertical_machine_filter>(data, value, file, indent);
	case HORIZONTAL:
		return std::make_unique<horizontal_machine_filter>(data, value, file, indent);
	case CUSTOM:
		return std::make_unique<custom_machine_filter>(data, value, file, indent);
	case COUNT: // not valid, but needed to suppress warnings
		break;
	}
	return nullptr;
}

machine_filter::ptr machine_filter::create(emu_file &file, machine_filter_data const &data, unsigned indent)
{
	char buffer[MAX_CHAR_INFO];
	if (!file.gets(buffer, ARRAY_LENGTH(buffer)))
		return nullptr;

	// split it into a key/value or bail
	std::string_view key(buffer);
	for (std::string_view::size_type i = 0; (2 * indent) > i; ++i)
	{
		if ((key.length() <= i) || (' ' != key[i]))
			return nullptr;
	}
	key = key.substr(2 * indent);
	std::string_view::size_type const split(key.find(" = "));
	if (std::string_view::npos == split)
		return nullptr;
	std::string_view::size_type const nl(key.find_first_of("\r\n", split));
	std::string const value(key.substr(split + 3, (std::string_view::npos == nl) ? nl : (nl - split - 3)));
	key = key.substr(0, split);

	// look for a filter type that matches
	for (type n = FIRST; COUNT > n; ++n)
	{
		if (key == config_name(n))
			return create(n, data, value.c_str(), &file, indent);
	}
	return nullptr;
}

char const *machine_filter::config_name(type n)
{
	assert(COUNT > n);
	return MACHINE_FILTER_NAMES[n];
}

char const *machine_filter::display_name(type n)
{
	assert(COUNT > n);
	return _(MACHINE_FILTER_NAMES[n]);
}

machine_filter::machine_filter()
{
}


//-------------------------------------------------
//  public software filter interface
//-------------------------------------------------

char const *software_filter::config_name(type n)
{
	assert(COUNT > n);
	return SOFTWARE_FILTER_NAMES[n];
}

char const *software_filter::display_name(type n)
{
	assert(COUNT > n);
	return _(SOFTWARE_FILTER_NAMES[n]);
}

software_filter::software_filter()
{
}

software_filter::ptr software_filter::create(type n, software_filter_data const &data, char const *value, emu_file *file, unsigned indent)
{
	assert(COUNT > n);
	switch (n)
	{
	case ALL:
		return std::make_unique<all_software_filter>(data, value, file, indent);
	case AVAILABLE:
		return std::make_unique<available_software_filter>(data, value, file, indent);
	case UNAVAILABLE:
		return std::make_unique<unavailable_software_filter>(data, value, file, indent);
	case FAVORITE:
		return std::make_unique<favorite_software_filter>(data, value, file, indent);
	case PARENTS:
		return std::make_unique<parents_software_filter>(data, value, file, indent);
	case CLONES:
		return std::make_unique<clones_software_filter>(data, value, file, indent);
	case YEAR:
		return std::make_unique<years_software_filter>(data, value, file, indent);
	case PUBLISHERS:
		return std::make_unique<publishers_software_filter>(data, value, file, indent);
	case SUPPORTED:
		return std::make_unique<supported_software_filter>(data, value, file, indent);
	case PARTIAL_SUPPORTED:
		return std::make_unique<partial_supported_software_filter>(data, value, file, indent);
	case UNSUPPORTED:
		return std::make_unique<unsupported_software_filter>(data, value, file, indent);
	case REGION:
		return std::make_unique<region_software_filter>(data, value, file, indent);
	case DEVICE_TYPE:
		return std::make_unique<device_type_software_filter>(data, value, file, indent);
	case LIST:
		return std::make_unique<list_software_filter>(data, value, file, indent);
	case CUSTOM:
		return std::make_unique<custom_software_filter>(data, value, file, indent);
	case COUNT: // not valid, but needed to suppress warnings
		break;
	}
	return nullptr;
}

software_filter::ptr software_filter::create(emu_file &file, software_filter_data const &data, unsigned indent)
{
	char buffer[MAX_CHAR_INFO];
	if (!file.gets(buffer, ARRAY_LENGTH(buffer)))
		return nullptr;

	// split it into a key/value or bail
	std::string_view key(buffer);
	for (std::string_view::size_type i = 0; (2 * indent) > i; ++i)
	{
		if ((key.length() <= i) || (' ' != key[i]))
			return nullptr;
	}
	key = key.substr(2 * indent);
	std::string_view::size_type const split(key.find(" = "));
	if (std::string_view::npos == split)
		return nullptr;
	std::string_view::size_type const nl(key.find_first_of("\r\n", split));
	std::string const value(key.substr(split + 3, (std::string_view::npos == nl) ? nl : (nl - split - 3)));
	key = key.substr(0, split);

	// look for a filter type that matches
	for (type n = FIRST; COUNT > n; ++n)
	{
		if (key == config_name(n))
			return create(n, data, value.c_str(), &file, indent);
	}
	return nullptr;
}

} // namesapce ui


extern const char UI_VERSION_TAG[];
const char UI_VERSION_TAG[] = "# UI INFO ";

// Globals
uint8_t ui_globals::rpanel = 0;
uint8_t ui_globals::curdats_view = 0;
uint8_t ui_globals::cur_sw_dats_total = 0;
uint8_t ui_globals::curdats_total = 0;
uint8_t ui_globals::cur_sw_dats_view = 0;
bool ui_globals::reset = false;
int ui_globals::visible_main_lines = 0;
int ui_globals::visible_sw_lines = 0;
uint16_t ui_globals::panels_status = 0;

char* chartrimcarriage(char str[])
{
	char *pstr = strrchr(str, '\n');
	if (pstr)
		str[pstr - str] = '\0';
	pstr = strrchr(str, '\r');
	if (pstr)
		str[pstr - str] = '\0';
	return str;
}

const char* strensure(const char* s)
{
	return s == nullptr ? "" : s;
}

int getprecisionchr(const char* s)
{
	int precision = 1;
	char *dp = const_cast<char *>(strchr(s, '.'));
	if (dp != nullptr)
		precision = strlen(s) - (dp - s) - 1;
	return precision;
}

std::vector<std::string> tokenize(const std::string &text, char sep)
{
	std::vector<std::string> tokens;
	tokens.reserve(64);
	std::size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos)
	{
		std::string temp = text.substr(start, end - start);
		if (!temp.empty()) tokens.push_back(temp);
		start = end + 1;
	}
	std::string temp = text.substr(start);
	if (!temp.empty()) tokens.push_back(temp);
	return tokens;
}


ui_software_info::ui_software_info(
		software_info const &info,
		software_part const &p,
		game_driver const &d,
		std::string const &li,
		std::string const &is,
		std::string const &de)
	: shortname(info.shortname()), longname(info.longname()), parentname(info.parentname())
	, year(info.year()), publisher(info.publisher())
	, supported(info.supported())
	, part(p.name())
	, driver(&d)
	, listname(li), interface(p.interface()), instance(is)
	, startempty(0)
	, parentlongname()
	, usage()
	, devicetype(de)
	, available(false)
{
	for (feature_list_item const &feature : info.other_info())
	{
		if (feature.name() == "usage")
		{
			usage = feature.value();
			break;
		}
	}
}

// info for starting empty
ui_software_info::ui_software_info(game_driver const &d)
	: shortname(d.name), longname(d.type.fullname()), driver(&d), startempty(1), available(true)
{
}
