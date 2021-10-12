// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/*********************************************************************

    ui/auditmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/auditmenu.h"

#include "ui/ui.h"

#include "audit.h"
#include "corestr.h"
#include "drivenum.h"

#include <numeric>


extern const char UI_VERSION_TAG[];

namespace ui {

namespace {

void *const ITEMREF_START_FULL = reinterpret_cast<void *>(std::uintptr_t(1));
void *const ITEMREF_START_FAST = reinterpret_cast<void *>(std::uintptr_t(2));

} // anonymous namespace


menu_audit::menu_audit(mame_ui_manager &mui, render_container &container, std::vector<ui_system_info> &availablesorted)
	: menu(mui, container)
	, m_worker_thread()
	, m_unavailable(
			std::accumulate(
				availablesorted.begin(),
				availablesorted.end(),
				std::size_t(0),
				[] (std::size_t n, ui_system_info const &info) { return n + (info.available ? 0 : 1);  }))
	, m_availablesorted(availablesorted)
	, m_audited(0)
	, m_current(nullptr)
	, m_phase(phase::CONSENT)
{
	std::string filename(emulator_info::get_configname());
	filename += "_avail.ini";
	m_prompt = util::string_format(_("Results will be saved to %1$s"), filename);
}

menu_audit::~menu_audit()
{
}

void menu_audit::custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2)
{
	switch (m_phase)
	{
	case phase::CONSENT:
		if ((ITEMREF_START_FAST == selectedref) || (ITEMREF_START_FULL == selectedref))
		{
			draw_text_box(
					&m_prompt, &m_prompt + 1,
					x, x2, y2 + ui().box_tb_border(), y2 + bottom,
					ui::text_layout::CENTER, ui::text_layout::NEVER, false,
					ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
		}
		break;

	case phase::AUDIT:
		{
			// there's a race here between the total audited being updated and the next driver pointer being loaded
			// it doesn't matter because we redraw on every frame anyway so it sorts itself out very quickly
			game_driver const *const driver(m_current.load());
			std::size_t const audited(m_audited.load());
			std::size_t const total(m_fast ? m_unavailable : m_availablesorted.size());
			std::string const text(
					util::string_format(
						_("Auditing ROMs for machine %2$u of %3$u...\n%1$s"),
						driver ? driver->type.fullname() : "",
						(std::min)(audited + 1, total),
						total));
			ui().draw_text_box(container(), text, ui::text_layout::CENTER, 0.5f, 0.5f, UI_GREEN_COLOR);
		}
		break;
	}
}

void menu_audit::populate(float &customtop, float &custombottom)
{
	if (m_unavailable && (m_availablesorted.size() != m_unavailable))
		item_append(util::string_format(_("Audit ROMs for %1$u machines marked unavailable"), m_unavailable), 0, ITEMREF_START_FAST);
	item_append(util::string_format(_("Audit ROMs for all %1$u machines"), m_availablesorted.size()), 0, ITEMREF_START_FULL);
	item_append(menu_item_type::SEPARATOR, 0);
	custombottom = (ui().get_line_height() * 1.0f) + (ui().box_tb_border() * 3.0f);
}

void menu_audit::handle()
{
	switch (m_phase)
	{
	case phase::CONSENT:
		{
			event const *const menu_event(process(0));
			if (menu_event && (IPT_UI_SELECT == menu_event->iptkey))
			{
				if ((ITEMREF_START_FULL == menu_event->itemref) || (ITEMREF_START_FAST == menu_event->itemref))
				{
					m_phase = phase::AUDIT;
					m_fast = ITEMREF_START_FAST == menu_event->itemref;
					m_worker_thread = std::thread(
							[this] ()
							{
								if (m_fast)
									audit_fast();
								else
									audit_all();
							});
				}
			}
		}
		break;

	case phase::AUDIT:
		process(PROCESS_CUSTOM_ONLY | PROCESS_NOINPUT);

		if (m_audited.load() >= (m_fast ? m_unavailable : m_availablesorted.size()))
		{
			m_worker_thread.join();
			save_available_machines();
			reset_parent(reset_options::SELECT_FIRST);
			stack_pop();
		}
		break;
	}
}

void menu_audit::audit_fast()
{
	for (ui_system_info &info : m_availablesorted)
	{
		if (!info.available)
		{
			m_current.store(info.driver);
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

void menu_audit::audit_all()
{
	driver_enumerator enumerator(machine().options());
	media_auditor auditor(enumerator);
	std::vector<bool> available(driver_list::total(), false);
	while (enumerator.next())
	{
		m_current.store(&enumerator.driver());
		media_auditor::summary const summary(auditor.audit_media(AUDIT_VALIDATE_FAST));

		// if everything looks good, include the driver
		available[enumerator.current()] = (summary == media_auditor::CORRECT) || (summary == media_auditor::BEST_AVAILABLE) || (summary == media_auditor::NONE_NEEDED);
		++m_audited;
	}

	for (ui_system_info &info : m_availablesorted)
		info.available = available[info.index];
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
