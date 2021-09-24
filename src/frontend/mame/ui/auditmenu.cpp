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

void *const ITEMREF_START = reinterpret_cast<void *>(std::uintptr_t(1));

} // anonymous namespace


bool sorted_game_list(const game_driver *x, const game_driver *y)
{
	bool clonex = (x->parent[0] != '0') || x->parent[1];
	int cx = -1;
	if (clonex)
	{
		cx = driver_list::find(x->parent);
		if ((0 > cx) || (driver_list::driver(cx).flags & machine_flags::IS_BIOS_ROOT))
			clonex = false;
	}

	bool cloney = (y->parent[0] != '0') || y->parent[1];
	int cy = -1;
	if (cloney)
	{
		cy = driver_list::find(y->parent);
		if ((0 > cy) || (driver_list::driver(cy).flags & machine_flags::IS_BIOS_ROOT))
			cloney = false;
	}

	if (!clonex && !cloney)
	{
		return (core_stricmp(x->type.fullname(), y->type.fullname()) < 0);
	}
	else if (clonex && cloney)
	{
		if (!core_stricmp(x->parent, y->parent))
			return (core_stricmp(x->type.fullname(), y->type.fullname()) < 0);
		else
			return (core_stricmp(driver_list::driver(cx).type.fullname(), driver_list::driver(cy).type.fullname()) < 0);
	}
	else if (!clonex && cloney)
	{
		if (!core_stricmp(x->name, y->parent))
			return true;
		else
			return (core_stricmp(x->type.fullname(), driver_list::driver(cy).type.fullname()) < 0);
	}
	else
	{
		if (!core_stricmp(x->parent, y->name))
			return false;
		else
			return (core_stricmp(driver_list::driver(cx).type.fullname(), y->type.fullname()) < 0);
	}
}


menu_audit::menu_audit(mame_ui_manager &mui, render_container &container, std::vector<ui_system_info> &availablesorted, mode audit_mode)
	: menu(mui, container)
	, m_worker_thread()
	, m_audit_mode(audit_mode)
	, m_total((mode::FAST == audit_mode)
			? std::accumulate(availablesorted.begin(), availablesorted.end(), std::size_t(0), [] (std::size_t n, ui_system_info const &info) { return n + (info.available ? 0 : 1);  })
			: availablesorted.size())
	, m_availablesorted(availablesorted)
	, m_audited(0)
	, m_current(nullptr)
	, m_phase(phase::CONSENT)
{
	switch (m_audit_mode)
	{
	case mode::FAST:
		m_prompt[0] = util::string_format(_("Audit ROMs for %1$u machines marked unavailable?"), m_total);
		break;
	case mode::ALL:
		m_prompt[0] = util::string_format(_("Audit ROMs for all %1$u machines?"), m_total);
		break;
	}
	std::string filename(emulator_info::get_configname());
	filename += "_avail.ini";
	m_prompt[1] = util::string_format(_("(results will be saved to %1$s)"), filename);
}

menu_audit::~menu_audit()
{
}

void menu_audit::custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2)
{
	switch (m_phase)
	{
	case phase::CONSENT:
		draw_text_box(
				std::begin(m_prompt), std::end(m_prompt),
				x, x2, y - top, y - ui().box_tb_border(),
				ui::text_layout::CENTER, ui::text_layout::NEVER, false,
				ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
		break;

	case phase::AUDIT:
		{
			// there's a race here between the total audited being updated and the next driver pointer being loaded
			// it doesn't matter because we redraw on every frame anyway so it sorts itself out very quickly
			game_driver const *const driver(m_current.load());
			std::size_t const audited(m_audited.load());
			std::string const text(util::string_format(
						_("Auditing ROMs for machine %2$u of %3$u...\n%1$s"),
						driver ? driver->type.fullname() : "",
						audited + 1,
						m_total));
			ui().draw_text_box(container(), text, ui::text_layout::CENTER, 0.5f, 0.5f, UI_GREEN_COLOR);
		}
		break;
	}
}

void menu_audit::populate(float &customtop, float &custombottom)
{
	item_append(_("Start Audit"), 0, ITEMREF_START);
	customtop = (ui().get_line_height() * 2.0f) + (ui().box_tb_border() * 3.0f);
}

void menu_audit::handle()
{
	switch (m_phase)
	{
	case phase::CONSENT:
		{
			event const *const menu_event(process(0));
			if (menu_event && (ITEMREF_START == menu_event->itemref) && (IPT_UI_SELECT == menu_event->iptkey))
			{
				m_phase = phase::AUDIT;
				m_worker_thread = std::thread(
						[this] ()
						{
							switch (m_audit_mode)
							{
							case mode::FAST:
								audit_fast();
								return;
							case mode::ALL:
								audit_all();
								return;
							}
							throw false;
						});
			}
		}
		break;

	case phase::AUDIT:
		process(PROCESS_CUSTOM_ONLY | PROCESS_NOINPUT);

		if (m_audited.load() >= m_total)
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
