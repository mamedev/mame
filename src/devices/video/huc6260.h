// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6260 interface and definitions

**********************************************************************/

#ifndef MAME_VIDEO_HUC6260_H
#define MAME_VIDEO_HUC6260_H

#pragma once


#define MCFG_HUC6260_NEXT_PIXEL_DATA_CB(_devcb) \
	downcast<huc6260_device &>(*device).set_next_pixel_data_callback(DEVCB_##_devcb);

#define MCFG_HUC6260_TIME_TIL_NEXT_EVENT_CB(_devcb) \
	downcast<huc6260_device &>(*device).set_time_til_next_event_callback(DEVCB_##_devcb);

#define MCFG_HUC6260_VSYNC_CHANGED_CB(_devcb) \
	downcast<huc6260_device &>(*device).set_vsync_changed_callback(DEVCB_##_devcb);

#define MCFG_HUC6260_HSYNC_CHANGED_CB(_devcb) \
	downcast<huc6260_device &>(*device).set_hsync_changed_callback(DEVCB_##_devcb);


class huc6260_device :  public device_t,
						public device_palette_interface,
						public device_video_interface
{
public:
	static constexpr unsigned PALETTE_SIZE = 1024;

	/* Screen timing stuff */
	static constexpr unsigned WPF = 1365;   // width of a line in frame including blanking areas
	static constexpr unsigned LPF = 263;    // max number of lines in a single frame


	// construction/destruction
	huc6260_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_next_pixel_data_callback(Object &&cb) { return m_next_pixel_data_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_time_til_next_event_callback(Object &&cb) { return m_time_til_next_event_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_vsync_changed_callback(Object &&cb) { return m_vsync_changed_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_hsync_changed_callback(Object &&cb) { return m_hsync_changed_cb.set_callback(std::forward<Object>(cb)); }
	auto next_pixel_data() { return m_next_pixel_data_cb.bind(); }
	auto time_til_next_event() { return m_time_til_next_event_cb.bind(); }
	auto vsync_changed() { return m_vsync_changed_cb.bind(); }
	auto hsync_changed() { return m_hsync_changed_cb.bind(); }

	void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	READ8_MEMBER(palette_direct_read);
	WRITE8_MEMBER(palette_direct_write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual uint32_t palette_entries() const override { return PALETTE_SIZE; }

private:
	void palette_init();

	int     m_last_h;
	int     m_last_v;
	int     m_height;

	/* Callback function to retrieve pixel data */
	devcb_read16                    m_next_pixel_data_cb;

	/* TODO: Choose proper types */
	/* Callback function to get time until next event */
	devcb_read16                    m_time_til_next_event_cb;

	/* Callback function which gets called when vsync changes */
	devcb_write_line                m_vsync_changed_cb;

	/* Callback function which gets called when hsync changes */
	devcb_write_line                m_hsync_changed_cb;

	uint16_t  m_palette[512];
	uint16_t  m_address;
	uint16_t  m_greyscales;       /* Should the HuC6260 output grey or color graphics */
	uint8_t   m_blur;             /* Should the edges of graphics be blurred/Select screen height 0=262, 1=263 */
	uint8_t   m_pixels_per_clock; /* Number of pixels to output per colour clock */
	uint16_t  m_pixel_data;
	uint8_t   m_pixel_clock;

	emu_timer   *m_timer;
	std::unique_ptr<bitmap_ind16>   m_bmp;
};


DECLARE_DEVICE_TYPE(HUC6260, huc6260_device)

#endif // MAME_VIDEO_HUC6260_H
