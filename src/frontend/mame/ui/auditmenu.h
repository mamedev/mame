// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/***************************************************************************

    ui/auditmenu.h

    Internal UI user interface.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_AUDITMENU_H
#define MAME_FRONTEND_UI_AUDITMENU_H

#pragma once

#include "ui/menu.h"
#include "ui/utils.h"

#include <atomic>
#include <future>
#include <vector>


namespace ui {

class menu_audit : public menu
{
public:
	menu_audit(mame_ui_manager &mui, render_container &container);
	virtual ~menu_audit() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual bool custom_ui_back() override;

private:
	enum class phase { CONFIRMATION, AUDIT, CANCELLATION };

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	bool do_audit();
	void save_available_machines();

	std::string m_prompt;
	std::vector<std::reference_wrapper<ui_system_info> > const &m_availablesorted;
	std::size_t const m_unavailable;
	std::vector<std::future<bool> > m_future;
	std::atomic<std::size_t> m_next;
	std::atomic<std::size_t> m_audited;
	std::atomic<ui_system_info const *> m_current;
	std::atomic<bool> m_cancel;
	phase m_phase;
	bool m_fast;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_AUDITMENU_H
