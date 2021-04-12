// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvsave.h

    Save debugger view.

***************************************************************************/

#ifndef MAME_EMU_DEBUG_DVSAVE_H
#define MAME_EMU_DEBUG_DVSAVE_H

#pragma once

#include "debugvw.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// debug view for state
class debug_view_save : public debug_view
{
	friend class debug_view_manager;

	// construction/destruction
	debug_view_save(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_save();

protected:
	// view overrides
	virtual void view_update() override;
	virtual void view_notify(debug_view_notification type) override;
	virtual void view_click(const int button, const debug_view_xy& pos) override;

private:
	void build_list_recursive(save_registered_item &item, uintptr_t objbase, int depth, int count = 0);

	class save_item
	{
	public:
		save_item(save_registered_item &item, uintptr_t objbase, int depth, int count = 0) :
			m_item(item),
			m_objbase(objbase),
			m_depth(depth),
			m_count(count),
			m_collapsed(count <= 1)
		{
		}

		bool collapsible() const
		{
			return (m_item.type() == save_registered_item::TYPE_CONTAINER ||
					m_item.type() == save_registered_item::TYPE_STRUCT ||
					m_item.type() < save_registered_item::TYPE_ARRAY);
		}
		char const *name() const { return m_item.name(); }
		std::string value();
		save_registered_item::save_type type() const { return m_item.type(); }
		int depth() const { return m_depth; }
		int count() const { return m_count; }
		bool collapsed() const { return m_collapsed; }
		bool changed() const;
		void update_value();

		void set_collapse(bool collapse) { m_collapsed = collapse; }

	private:
		std::string value(save_registered_item &item, int count, uintptr_t objbase, bool collapsed);

		template<int N, typename... Args>
		bool catprintf(char (&buf)[N], int &pos, char *format, Args &&... args)
		{
			if (N - pos >= 1)
			{
				int result = snprintf(&buf[pos], N - 1 - pos, format, std::forward<Args>(args)...);
				if (result >= 0)
					pos += result;
				buf[N - 1] = 0;
			}
			return (N - pos >= 1);
		}

		save_registered_item &m_item;
		uintptr_t m_objbase;
		uint8_t m_depth;
		uint8_t m_count;
		bool m_collapsed;
		uint8_t m_prev_value[8];
	};

	// internal helpers
	void reset();
	void recompute();

	// internal state
	int                     m_divider;
	std::vector<save_item>  m_save_list;            // list of items
};


#endif // MAME_EMU_DEBUG_DVSAVE_H
