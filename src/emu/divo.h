// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    divo.h

    Video output, aka screens

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DIH
#define MAME_EMU_DIH

class render_container;
class device_palette_interface;

/// Forward declaration for the enumerator typedef
template<typename T> class device_interface_enumerator;

class device_video_output_interface : public device_interface
{
public:
	static constexpr int DEFAULT_FRAME_RATE = 60;

	device_video_output_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_video_output_interface();

	// Should the debugger show a beam position in the interface?
	virtual bool has_beam() const = 0;

	// Is this a vector screen (they're rather special, rendering-wise)?
	virtual bool is_vector() const;

	// What name to use for the screen type (used for information and .ini loading)
	virtual const char *output_type_name() const = 0;

	// Beam position, when available
	virtual int hpos() const;
	virtual int vpos() const;

	// Rendering
	void set_container(render_container &container) { m_container = &container; }
	render_container &container() const { assert(m_container != nullptr); return *m_container; }
	virtual bool video_output_update() = 0;

	// Orientation
	void set_orientation(int orientation) { assert(!device().configured()); m_orientation = orientation; }
	int orientation() const { assert(device().configured()); return m_orientation; }

	// Timing
	virtual void override_frame_period(attotime period) = 0;
	virtual attotime frame_period() const = 0;

	// Palette access
	virtual bool has_palette() const = 0;
	virtual device_palette_interface &palette() const = 0;

	// Geometry
	virtual rectangle visible_area() const = 0;
	virtual std::pair<unsigned, unsigned> physical_aspect() const = 0;

protected:
	render_container *m_container;
	bool              m_is_primary_screen;
	int               m_orientation;              // orientation flags combined with system flags
	u32               m_unique_id;                // unique id for this device

	virtual void interface_config_complete() override;
	virtual void interface_pre_start() override;

	void update_if_primary();


	// static data
	static u32          m_id_counter;   // incremented for each constructed video output device,
										// used as a unique identifier during runtime
};

// iterator
typedef device_interface_enumerator<device_video_output_interface> video_output_interface_enumerator;


#endif  /* MAME_EMU_DIGFX_H */
