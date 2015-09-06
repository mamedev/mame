// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Incredible Technologies/Strata system
    (8-bit blitter variant)

**************************************************************************/

#include "machine/ticket.h"
#include "video/tlc34076.h"
#include "video/tms34061.h"

#define YBUFFER_COUNT   15

class itech8_state : public driver_device
{
public:
	itech8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_subcpu(*this, "sub"),
		m_tms34061(*this, "tms34061"),
		m_tlc34076(*this, "tlc34076"),
		m_screen(*this, "screen"),
		m_ticket(*this, "ticket"),
		m_an(*this, analog_inputs),
		m_fakex(*this, "FAKEX"),
		m_fakey(*this, "FAKEY"),
		m_visarea(0, 0, 0, 0) { }

	enum
	{
		TIMER_IRQ_OFF,
		TIMER_BEHIND_BEAM_UPDATE,
		TIMER_DELAYED_SOUND_DATA,
		TIMER_BLITTER_DONE,
		TIMER_DELAYED_Z80_CONTROL
	};

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_subcpu;
	required_device<tms34061_device> m_tms34061;
	required_device<tlc34076_device> m_tlc34076;
	required_device<screen_device> m_screen;
	required_device<ticket_dispenser_device> m_ticket;
	DECLARE_IOPORT_ARRAY(analog_inputs);
	optional_ioport_array<4> m_an;
	optional_ioport m_fakex;
	optional_ioport m_fakey;

	rectangle m_visarea;

	UINT8 m_grom_bank;
	UINT8 m_blitter_int;
	UINT8 m_tms34061_int;
	UINT8 m_periodic_int;
	UINT8 m_sound_data;
	UINT8 m_pia_porta_data;
	UINT8 m_pia_portb_data;
	UINT8 m_z80_ctrl;
	UINT8 m_z80_port_val;
	UINT8 m_z80_clear_to_send;
	UINT16 m_sensor0;
	UINT16 m_sensor1;
	UINT16 m_sensor2;
	UINT16 m_sensor3;
	UINT8 m_curvx;
	UINT8 m_curvy;
	UINT8 m_curx;
	INT8 m_xbuffer[YBUFFER_COUNT];
	INT8 m_ybuffer[YBUFFER_COUNT];
	int m_ybuffer_next;
	int m_curxpos;
	int m_last_ytotal;
	UINT8 m_crosshair_vis;
	UINT8 m_blitter_data[16];
	UINT8 m_blit_in_progress;
	UINT8 m_page_select;
	offs_t m_fetch_offset;
	UINT8 m_fetch_rle_count;
	UINT8 m_fetch_rle_value;
	UINT8 m_fetch_rle_literal;
	UINT8 *m_grom_base;
	UINT32 m_grom_size;
	UINT8 m_grmatch_palcontrol;
	UINT8 m_grmatch_xscroll;
	rgb_t m_grmatch_palette[2][16];
	emu_timer *m_irq_off_timer;
	emu_timer *m_behind_beam_update_timer;
	emu_timer *m_delayed_sound_data_timer;
	emu_timer *m_blitter_done_timer;
	emu_timer *m_delayed_z80_control_timer;

	// common
	DECLARE_WRITE_LINE_MEMBER(generate_tms34061_interrupt);
	DECLARE_WRITE8_MEMBER(nmi_ack_w);
	DECLARE_WRITE8_MEMBER(blitter_bank_w);
	DECLARE_WRITE8_MEMBER(rimrockn_bank_w);
	DECLARE_WRITE8_MEMBER(pia_portb_out);
	DECLARE_WRITE8_MEMBER(sound_data_w);
	DECLARE_WRITE8_MEMBER(gtg2_sound_data_w);
	DECLARE_READ8_MEMBER(sound_data_r);
	DECLARE_WRITE8_MEMBER(grom_bank_w);
	DECLARE_WRITE16_MEMBER(grom_bank16_w);
	DECLARE_WRITE16_MEMBER(display_page16_w);
	DECLARE_WRITE16_MEMBER(palette16_w);
	DECLARE_WRITE8_MEMBER(palette_w);
	DECLARE_WRITE8_MEMBER(page_w);
	DECLARE_READ8_MEMBER(blitter_r);
	DECLARE_WRITE8_MEMBER(blitter_w);
	DECLARE_WRITE8_MEMBER(tms34061_w);
	DECLARE_READ8_MEMBER(tms34061_r);
	DECLARE_WRITE8_MEMBER(grmatch_palette_w);
	DECLARE_WRITE8_MEMBER(grmatch_xscroll_w);
	DECLARE_WRITE8_MEMBER(pia_porta_out);
	DECLARE_WRITE8_MEMBER(ym2203_portb_out);

	DECLARE_CUSTOM_INPUT_MEMBER(special_r);
	DECLARE_CUSTOM_INPUT_MEMBER(gtg_mux);

	DECLARE_DRIVER_INIT(rimrockn);
	DECLARE_DRIVER_INIT(grmatch);
	DECLARE_DRIVER_INIT(peggle);
	DECLARE_DRIVER_INIT(slikshot);
	DECLARE_DRIVER_INIT(neckneck);
	DECLARE_DRIVER_INIT(arligntn);
	DECLARE_DRIVER_INIT(hstennis);
	DECLARE_DRIVER_INIT(sstrike);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_VIDEO_START(slikshot);
	DECLARE_MACHINE_START(sstrike);

	UINT32 screen_update_2layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_grmatch(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_slikshot(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_2page(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_2page_large(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(generate_nmi);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_CALLBACK_MEMBER(behind_the_beam_update);
	TIMER_CALLBACK_MEMBER(delayed_sound_data_w);
	TIMER_CALLBACK_MEMBER(blitter_done);
	TIMER_DEVICE_CALLBACK_MEMBER(grmatch_palette_update);

	inline UINT8 fetch_next_raw();
	inline void consume_raw(int count);
	inline UINT8 fetch_next_rle();
	inline void consume_rle(int count);
	void perform_blit(address_space &space);
	void update_interrupts(int periodic, int tms34061, int blitter);

	/*----------- defined in machine/slikshot.c -----------*/

	DECLARE_READ8_MEMBER( slikz80_port_r );
	DECLARE_WRITE8_MEMBER( slikz80_port_w );

	DECLARE_READ8_MEMBER( slikshot_z80_r );
	DECLARE_READ8_MEMBER( slikshot_z80_control_r );
	DECLARE_WRITE8_MEMBER( slikshot_z80_control_w );

	void inters_to_vels(UINT16 inter1, UINT16 inter2, UINT16 inter3, UINT8 beams,
							UINT8 *xres, UINT8 *vxres, UINT8 *vyres);
	void vels_to_inters(UINT8 x, UINT8 vx, UINT8 vy,
							UINT16 *inter1, UINT16 *inter2, UINT16 *inter3, UINT8 *beams);
	void inters_to_words(UINT16 inter1, UINT16 inter2, UINT16 inter3, UINT8 *beams,
							UINT16 *word1, UINT16 *word2, UINT16 *word3);

	void words_to_sensors(UINT16 word1, UINT16 word2, UINT16 word3, UINT8 beams,
							UINT16 *sens0, UINT16 *sens1, UINT16 *sens2, UINT16 *sens3);
	void compute_sensors();
	TIMER_CALLBACK_MEMBER( delayed_z80_control_w );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};
