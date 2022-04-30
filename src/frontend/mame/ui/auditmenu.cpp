// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/*********************************************************************

    ui/auditmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/auditmenu.h"

#include "ui/systemlist.h"
#include "ui/ui.h"

#include "audit.h"

#include "drivenum.h"
#include "fileio.h"
#include "uiinput.h"

#include "util/corestr.h"

#include <numeric>
#include <sstream>
#include <thread>


extern const char UI_VERSION_TAG[];

namespace ui {

namespace {

void *const ITEMREF_START_FULL = reinterpret_cast<void *>(std::uintptr_t(1));
void *const ITEMREF_START_FAST = reinterpret_cast<void *>(std::uintptr_t(2));

} // anonymous namespace


menu_audit::menu_audit(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
	, m_availablesorted(system_list::instance().sorted_list())
	, m_unavailable(
			std::accumulate(
				m_availablesorted.begin(),
				m_availablesorted.end(),
				std::size_t(0),
				[] (std::size_t n, ui_system_info const &info) { return n + (info.available ? 0 : 1);  }))
	, m_future()
	, m_next(0)
	, m_audited(0)
	, m_current(nullptr)
	, m_cancel(false)
	, m_phase(phase::CONFIRMATION)
	, m_fast(true)
{
	std::string filename(emulator_info::get_configname());
	filename += "_avail.ini";
	m_prompt = util::string_format(_("Results will be saved to %1$s"), filename);
}

menu_audit::~menu_audit()
{
	m_cancel.store(true);
	for (auto &future : m_future)
	{
		if (future.valid())
			future.wait();
	}
}


void menu_audit::custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2)
{
	switch (m_phase)
	{
	case phase::CONFIRMATION:
		if ((ITEMREF_START_FAST == selectedref) || (ITEMREF_START_FULL == selectedref))
		{
			draw_text_box(
					&m_prompt, &m_prompt + 1,
					x, x2, y2 + ui().box_tb_border(), y2 + bottom,
					text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, false,
					ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
		}
		break;

	case phase::AUDIT:
		{
			// there's a race here between the total audited being updated and the next driver pointer being loaded
			// it doesn't matter because we redraw on every frame anyway so it sorts itself out very quickly
			ui_system_info const *const system(m_current.load());
			std::size_t const audited(m_audited.load());
			std::size_t const total(m_fast ? m_unavailable : m_availablesorted.size());
			std::ostringstream text;
			util::stream_format(text,
					_("Auditing media for machine %2$u of %3$u...\n%1$s"),
					system ? std::string_view(system->description) : std::string_view(),
					(std::min)(audited + 1, total),
					total);
			text << '\n' << m_prompt;
			ui().draw_text_box(
					container(),
					std::move(text).str(),
					text_layout::text_justify::CENTER,
					0.5f, 0.5f,
					ui().colors().background_color());
		}
		break;

	case phase::CANCELLATION:
		ui().draw_text_box(
				container(),
				util::string_format(
					_("Cancel audit?\n\nPress %1$s to cancel\nPress %2$s to continue"),
					ui().get_general_input_setting(IPT_UI_SELECT),
					ui().get_general_input_setting(IPT_UI_CANCEL)),
				text_layout::text_justify::CENTER,
				0.5f, 0.5f,
				UI_RED_COLOR);
		break;
	}
}


bool menu_audit::custom_ui_cancel()
{
	return m_phase != phase::CONFIRMATION;
}


void menu_audit::populate(float &customtop, float &custombottom)
{
	if (m_unavailable && (m_availablesorted.size() != m_unavailable))
		item_append(util::string_format(_("Audit media for %1$u machines marked unavailable"), m_unavailable), 0, ITEMREF_START_FAST);
	item_append(util::string_format(_("Audit media for all %1$u machines"), m_availablesorted.size()), 0, ITEMREF_START_FULL);
	item_append(menu_item_type::SEPARATOR, 0);
	custombottom = (ui().get_line_height() * 1.0f) + (ui().box_tb_border() * 3.0f);
}

void menu_audit::handle(event const *ev)
{
	switch (m_phase)
	{
	case phase::CONFIRMATION:
		if (ev && (IPT_UI_SELECT == ev->iptkey))
		{
			if ((ITEMREF_START_FULL == ev->itemref) || (ITEMREF_START_FAST == ev->itemref))
			{
				set_process_flags(PROCESS_CUSTOM_ONLY | PROCESS_NOINPUT);
				m_phase = phase::AUDIT;
				m_fast = ITEMREF_START_FAST == ev->itemref;
				m_prompt = util::string_format(_("Press %1$s to cancel\n"), ui().get_general_input_setting(IPT_UI_CANCEL));
				m_future.resize(std::thread::hardware_concurrency());
				for (auto &future : m_future)
					future = std::async(std::launch::async, [this] () { return do_audit(); });
			}
		}
		break;

	case phase::AUDIT:
	case phase::CANCELLATION:
		if ((m_next.load() >= m_availablesorted.size()) || m_cancel.load())
		{
			bool done(true);
			for (auto &future : m_future)
				done = future.get() && done;
			m_future.clear();
			if (done)
			{
				save_available_machines();
				reset_parent(reset_options::SELECT_FIRST);
			}
			stack_pop();
		}
		else if (machine().ui_input().pressed(IPT_UI_CANCEL))
		{
			if (phase::AUDIT == m_phase)
				m_phase = phase::CANCELLATION;
			else
				m_phase = phase::AUDIT;
		}
		else if ((phase::CANCELLATION == m_phase) && machine().ui_input().pressed(IPT_UI_SELECT))
		{
			m_cancel.store(true);
		}
		break;
	}
}

bool menu_audit::do_audit()
{
	while (true)
	{
		std::size_t const i(m_next.fetch_add(1));
		if (m_availablesorted.size() <= i)
			return true;

		ui_system_info &info(m_availablesorted[i]);
		if (!m_fast || !info.available)
		{
			if (m_cancel.load())
				return false;

			m_current.store(&info);
			driver_enumerator enumerator(machine().options(), info.driver->name);
			enumerator.next();
			media_auditor auditor(enumerator);
			media_auditor::summary const summary(auditor.audit_media(AUDIT_VALIDATE_FAST));
			info.available = (summary == media_auditor::CORRECT) || (summary == media_auditor::BEST_AVAILABLE) || (summary == media_auditor::NONE_NEEDED);

			// if everything looks good, include the driver
			info.available = (summary == media_auditor::CORRECT) || (summary == media_auditor::BEST_AVAILABLE) || (summary == media_auditor::NONE_NEEDED);
			++m_audited;
		}
	}
}

void menu_audit::save_available_machines()
{
	// attempt to open the output file
	emu_file file(ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (!file.open(std::string(emulator_info::get_configname()) + "_avail.ini"))
	{
		// generate header
		file.printf("#\n%s%s\n#\n\n", UI_VERSION_TAG, emulator_info::get_bare_build_version());

		// generate available list
		for (ui_system_info const &info : m_availablesorted)
		{
			if (info.available)
				file.printf("%s\n", info.driver->name);
		}

		file.close();
	}
}

} // namespace ui
