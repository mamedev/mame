// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/state.cpp

    Menus for saving and loading state

***************************************************************************/

#include "emu.h"
#include "ui/state.h"

#include "emuopts.h"
#include "inputdev.h"
#include "uiinput.h"

#include "path.h"


namespace ui {

/***************************************************************************
    ANONYMOUS NAMESPACE
***************************************************************************/

namespace {

//-------------------------------------------------
//  keyboard_input_item_name
//-------------------------------------------------

std::string keyboard_input_item_name(input_item_id id)
{
	if (id >= ITEM_ID_A && id <= ITEM_ID_Z)
		return std::string(1, char(id - ITEM_ID_A + 'a'));
	if (id >= ITEM_ID_0 && id <= ITEM_ID_9)
		return std::string(1, char(id - ITEM_ID_0 + '0'));

	// only supported for A-Z/0-9
	throw false;
}


//-------------------------------------------------
//  code_item_pair
//-------------------------------------------------

std::pair<std::string, std::string> code_item_pair(const running_machine &machine, input_item_id id)
{
	// only supported for A-Z|0-9
	assert((id >= ITEM_ID_A && id <= ITEM_ID_Z) || (id >= ITEM_ID_0 && id <= ITEM_ID_9));
	input_code const code = input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, id);

	return std::make_pair(keyboard_input_item_name(id), machine.input().code_name(code));
}

} // anonymous namespace


/***************************************************************************
    FILE ENTRY
***************************************************************************/

std::string menu_load_save_state_base::s_last_file_selected;


//-------------------------------------------------
//  file_entry ctor
//-------------------------------------------------

menu_load_save_state_base::file_entry::file_entry(std::string &&file_name, std::string &&visible_name, const std::chrono::system_clock::time_point &last_modified)
	: m_file_name(std::move(file_name))
	, m_visible_name(std::move(visible_name))
	, m_last_modified(last_modified)
{
}


/***************************************************************************
    BASE CLASS FOR LOAD AND SAVE
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_load_save_state_base::menu_load_save_state_base(
		mame_ui_manager &mui,
		render_container &container,
		std::string_view header,
		std::string_view footer,
		bool must_exist,
		bool one_shot)
	: autopause_menu<>(mui, container)
	, m_switch_poller(machine().input())
	, m_footer(footer)
	, m_confirm_delete(nullptr)
	, m_must_exist(must_exist)
	, m_keys_released(false)
	, m_slot_selected(INPUT_CODE_INVALID)
{
	set_one_shot(one_shot);
	set_needs_prev_menu_item(!one_shot);
	set_heading(header);
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_load_save_state_base::~menu_load_save_state_base()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_load_save_state_base::populate()
{
	// build the "filename to code" map, if we have not already (if it were not for the
	// possibility that the system keyboard can be changed at runtime, I would put this
	// into a static)
	if (m_filename_to_code_map.empty())
	{
		// loop through A-Z/0-9
		for (input_item_id id = ITEM_ID_A; id <= ITEM_ID_Z; id++)
			m_filename_to_code_map.emplace(code_item_pair(machine(), id));
		for (input_item_id id = ITEM_ID_0; id <= ITEM_ID_9; id++)
			m_filename_to_code_map.emplace(code_item_pair(machine(), id));

		// do joysticks
		input_class const &sticks = machine().input().device_class(DEVICE_CLASS_JOYSTICK);
		if (sticks.enabled())
		{
			for (int i = 0; sticks.maxindex() >= i; ++i)
			{
				input_device const *const stick = sticks.device(i);
				if (stick)
				{
					for (input_item_id j = ITEM_ID_BUTTON1; (ITEM_ID_BUTTON32 >= j) && (stick->maxitem() >= j); ++j)
					{
						input_device_item const *const item = stick->item(j);
						if (item && (item->itemclass() == ITEM_CLASS_SWITCH))
						{
							m_filename_to_code_map.emplace(
									util::string_format("joy%i-%i", i, j - ITEM_ID_BUTTON1 + 1),
									machine().input().code_name(item->code()));
						}
					}
				}
			}
		}
	}

	// open the state directory
	osd::directory::ptr dir = osd::directory::open(state_directory());

	// create a separate vector, so we can add sorted entries to the menu
	std::vector<const file_entry *> m_entries_vec;

	// populate all file entries
	m_file_entries.clear();
	if (dir)
	{
		const osd::directory::entry *entry;
		while ((entry = dir->read()) != nullptr)
		{
			if (core_filename_ends_with(entry->name, ".sta"))
			{
				// get the file name of the entry
				std::string file_name(core_filename_extract_base(entry->name, true));

				// try translating it
				std::string visible_name = get_visible_name(file_name);

				// and proceed
				file_entry fileent(std::string(file_name), std::move(visible_name), entry->last_modified);
				auto iter = m_file_entries.emplace(std::make_pair(std::move(file_name), std::move(fileent))).first;
				m_entries_vec.push_back(&iter->second);
			}
		}
	}

	// sort the vector; put recently modified state files at the top
	std::sort(
			m_entries_vec.begin(),
			m_entries_vec.end(),
			[] (const file_entry *a, const file_entry *b)
			{
				return a->last_modified() > b->last_modified();
			});

	// add the entries
	for (const file_entry *entry : m_entries_vec)
	{
		// get the time as a local time string
		char time_string[128];
		auto last_modified_time_t = std::chrono::system_clock::to_time_t(entry->last_modified());
		std::strftime(time_string, sizeof(time_string), "%c", std::localtime(&last_modified_time_t));

		// format the text
		std::string text = util::string_format("%s: %s",
				entry->visible_name(),
				time_string);

		// append the menu item
		void *const itemref = itemref_from_file_entry(*entry);
		item_append(std::move(text), 0, itemref);

		// is this item selected?
		if (entry->file_name() == s_last_file_selected)
			set_selection(itemref);
	}

	if (m_entries_vec.empty())
	{
		item_append(_("[no saved states found]"), FLAG_DISABLE, nullptr);
		set_selection(nullptr);
	}
	item_append(menu_item_type::SEPARATOR);
	if (is_one_shot())
		item_append(_("Cancel"), 0, nullptr);

	// get ready to poll inputs
	m_switch_poller.reset();
	m_keys_released = false;
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_load_save_state_base::handle(event const *ev)
{
	// process the event
	if (INPUT_CODE_INVALID != m_slot_selected)
	{
		if (!machine().input().code_pressed(m_slot_selected))
			stack_pop();
		return false;
	}
	else if (ev && (ev->iptkey == IPT_UI_SELECT))
	{
		if (ev->itemref)
		{
			// user selected one of the entries
			file_entry const &entry = file_entry_from_itemref(ev->itemref);
			slot_selected(std::string(entry.file_name()));
		}
		stack_pop();
		return false;
	}
	else if (ev && (ev->iptkey == IPT_UI_CLEAR))
	{
		if (ev->itemref)
		{
			// prompt to confirm delete
			m_confirm_delete = &file_entry_from_itemref(ev->itemref);
			m_confirm_prompt = util::string_format(
					_("Delete saved state %1$s?\nPress %2$s to delete\nPress %3$s to cancel"),
					m_confirm_delete->visible_name(),
					ui().get_general_input_setting(IPT_UI_SELECT),
					ui().get_general_input_setting(IPT_UI_BACK));
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (!m_confirm_delete)
	{
		// poll inputs
		input_code code;
		std::string name = poll_inputs(code);
		if (!name.empty() && try_select_slot(std::move(name)))
		{
			m_switch_poller.reset();
			m_slot_selected = code;
		}
		return false;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------
//  get_visible_name
//-------------------------------------------------

std::string menu_load_save_state_base::get_visible_name(const std::string &file_name)
{
	auto const iter = m_filename_to_code_map.find(file_name);
	if (iter != m_filename_to_code_map.end())
		return iter->second;

	// otherwise these are the same
	return file_name;
}


//-------------------------------------------------
//  poll_inputs
//-------------------------------------------------

std::string menu_load_save_state_base::poll_inputs(input_code &code)
{
	input_code const result = m_switch_poller.poll();
	if (INPUT_CODE_INVALID == result)
	{
		m_keys_released = true;
	}
	else if (m_keys_released)
	{
		input_item_id const id = result.item_id();

		// keyboard A-Z and 0-9
		if (((ITEM_ID_A <= id) && (ITEM_ID_Z >= id)) || ((ITEM_ID_0 <= id) && (ITEM_ID_9 >= id)))
		{
			code = result;
			return keyboard_input_item_name(id);
		}

		// joystick buttons
		if ((DEVICE_CLASS_JOYSTICK == result.device_class()) && (ITEM_CLASS_SWITCH == result.item_class()) && (ITEM_MODIFIER_NONE == result.item_modifier()) && (ITEM_ID_BUTTON1 <= id) && (ITEM_ID_BUTTON32 >= id))
		{
			code = result;
			return util::string_format("joy%i-%i", result.device_index(), id - ITEM_ID_BUTTON1 + 1);
		}
	}
	code = INPUT_CODE_INVALID;
	return "";
}


//-------------------------------------------------
//  try_select_slot
//-------------------------------------------------

bool menu_load_save_state_base::try_select_slot(std::string &&name)
{
	if (!m_must_exist || is_present(name))
	{
		slot_selected(std::move(name));
		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------
//  slot_selected
//-------------------------------------------------

void menu_load_save_state_base::slot_selected(std::string &&name)
{
	// handle it
	process_file(std::string(name));

	// record the last slot touched
	s_last_file_selected = std::move(name);
}


//-------------------------------------------------
//  handle_keys - override key handling
//-------------------------------------------------

bool menu_load_save_state_base::handle_keys(uint32_t flags, int &iptkey)
{
	if (m_confirm_delete)
	{
		bool updated(false);
		if (exclusive_input_pressed(iptkey, IPT_UI_SELECT, 0))
		{
			// try to remove the file
			std::string const filename(util::path_concat(
						machine().options().state_directory(),
						machine().get_statename(machine().options().state_name()),
						m_confirm_delete->file_name() + ".sta"));
			std::error_condition const err(osd_file::remove(filename));
			if (err)
			{
				osd_printf_error(
						"Error removing file %s for state %s (%s:%d %s)\n",
						filename,
						m_confirm_delete->visible_name(),
						err.category().name(),
						err.value(),
						err.message());
				machine().popmessage(_("Error removing saved state file %1$s"), filename);
			}

			// repopulate the menu
			// reset switch poller here to avoid bogus save/load if confirmed with joystick button
			m_switch_poller.reset();
			m_confirm_prompt.clear();
			m_confirm_delete = nullptr;
			m_keys_released = false;
			reset(reset_options::REMEMBER_POSITION);
		}
		else if (exclusive_input_pressed(iptkey, IPT_UI_BACK, 0))
		{
			// don't delete it - dismiss the prompt
			m_switch_poller.reset();
			m_confirm_prompt.clear();
			m_confirm_delete = nullptr;
			m_keys_released = false;
			updated = true;
		}
		iptkey = IPT_INVALID;
		return updated;
	}
	else if (INPUT_CODE_INVALID != m_slot_selected)
	{
		iptkey = IPT_INVALID;
		return false;
	}
	else
	{
		return autopause_menu<>::handle_keys(flags, iptkey);
	}
}


//-------------------------------------------------
//  custom_pointer_updated - override pointer
//  handling
//-------------------------------------------------

std::tuple<int, bool, bool> menu_load_save_state_base::custom_pointer_updated(bool changed, ui_event const &uievt)
{
	// suppress clicks on the menu while the delete prompt is visible
	if (m_confirm_delete && uievt.pointer_buttons)
		return std::make_tuple(IPT_INVALID, true, false);
	else
		return autopause_menu<>::custom_pointer_updated(changed, uievt);

}


//-------------------------------------------------
//  recompute_metrics - recompute metrics
//-------------------------------------------------

void menu_load_save_state_base::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	autopause_menu<>::recompute_metrics(width, height, aspect);

	// set up custom render proc
	set_custom_space(0.0F, (2.0F * line_height()) + (3.0F * tb_border()));
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void menu_load_save_state_base::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	std::string_view text[2];
	unsigned count(0U);

	// add fixed footer if supplied
	if (!m_footer.empty())
		text[count++] = m_footer;

	// provide a prompt to delete if a state is selected
	if (selected_item().ref())
	{
		if (m_delete_prompt.empty())
			m_delete_prompt = util::string_format(_("Press %1$s to delete"), ui().get_general_input_setting(IPT_UI_CLEAR));
		text[count++] = m_delete_prompt;
	}

	// draw the footer box if necessary
	if (count)
	{
		draw_text_box(
				std::begin(text), std::next(std::begin(text), count),
				origx1, origx2, origy2 + tb_border(), origy2 + (count * line_height()) + (3.0F * tb_border()),
				text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, false,
				ui().colors().text_color(), ui().colors().background_color());
	}

	// draw the confirmation prompt if necessary
	if (!m_confirm_prompt.empty())
		ui().draw_text_box(container(), m_confirm_prompt, text_layout::text_justify::CENTER, 0.5F, 0.5F, ui().colors().background_color());
}


//-------------------------------------------------
//  itemref_from_file_entry
//-------------------------------------------------

void *menu_load_save_state_base::itemref_from_file_entry(const menu_load_save_state_base::file_entry &entry)
{
	return (void *)&entry;
}


//-------------------------------------------------
//  file_entry_from_itemref
//-------------------------------------------------

const menu_load_save_state_base::file_entry &menu_load_save_state_base::file_entry_from_itemref(void *itemref)
{
	return *((const file_entry *)itemref);
}


//-------------------------------------------------
//  state_name
//-------------------------------------------------

std::string menu_load_save_state_base::state_directory() const
{
	return util::path_concat(
			machine().options().state_directory(),
			machine().get_statename(machine().options().state_name()));
}


//-------------------------------------------------
//  is_present
//-------------------------------------------------

bool menu_load_save_state_base::is_present(const std::string &name) const
{
	return m_file_entries.find(name) != m_file_entries.end();
}


/***************************************************************************
    LOAD STATE
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_load_state::menu_load_state(mame_ui_manager &mui, render_container &container, bool one_shot)
	: menu_load_save_state_base(mui, container, _("Load State"), _("Select state to load"), true, one_shot)
{
}


//-------------------------------------------------
//  process_file
//-------------------------------------------------

void menu_load_state::process_file(std::string &&file_name)
{
	machine().schedule_load(std::move(file_name));
}


/***************************************************************************
    SAVE STATE
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_save_state::menu_save_state(mame_ui_manager &mui, render_container &container, bool one_shot)
	: menu_load_save_state_base(mui, container, _("Save State"), _("Press a key or joystick button, or select state to overwrite"), false, one_shot)
{
}


//-------------------------------------------------
//  process_file
//-------------------------------------------------

void menu_save_state::process_file(std::string &&file_name)
{
	machine().schedule_save(std::move(file_name));
}


} // namespace ui
