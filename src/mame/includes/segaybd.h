// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega Y-Board hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/mb3773.h"
#include "machine/segaic16.h"
#include "video/segaic16.h"
#include "video/sega16sp.h"


// ======================> segaybd_state

class segaybd_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segaybd_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subx(*this, "subx")
		, m_suby(*this, "suby")
		, m_soundcpu(*this, "soundcpu")
		, m_linkcpu(*this, "linkcpu")
		, m_watchdog(*this, "watchdog")
		, m_bsprites(*this, "bsprites")
		, m_ysprites(*this, "ysprites")
		, m_segaic16vid(*this, "segaic16vid")
		, m_soundlatch(*this, "soundlatch")
		, m_adc_ports(*this, "ADC.%u", 0)
		, m_pdrift_bank(0)
		, m_scanline_timer(nullptr)
		, m_irq2_scanline(0)
		, m_timer_irq_state(0)
		, m_vblank_irq_state(0)
		, m_misc_io_data(0)
		, m_tmp_bitmap(512, 512)
	{
	}

	// main CPU read/write handlers
	DECLARE_WRITE8_MEMBER(output1_w);
	DECLARE_WRITE8_MEMBER(misc_output_w);
	DECLARE_WRITE8_MEMBER(output2_w);
	DECLARE_WRITE16_MEMBER(sound_data_w);

	// sound Z80 CPU read/write handlers
	DECLARE_READ8_MEMBER(sound_data_r);

	// linked cabinet specific handlers
	DECLARE_WRITE_LINE_MEMBER(mb8421_intl);
	DECLARE_WRITE_LINE_MEMBER(mb8421_intr);
	DECLARE_READ16_MEMBER(link_r);
	DECLARE_READ16_MEMBER(link2_r);
	DECLARE_WRITE16_MEMBER(link2_w);
//  DECLARE_READ8_MEMBER(link_portc0_r);

	// input helpers
	ioport_value analog_mux();

	// game-specific output handlers
	void gforce2_output_cb1(uint16_t data);
	void gforce2_output_cb2(uint16_t data);
	void gloc_output_cb1(uint16_t data);
	void gloc_output_cb2(uint16_t data);
	void r360_output_cb2(uint16_t data);
	void pdrift_output_cb1(uint16_t data);
	void pdrift_output_cb2(uint16_t data);
	void rchase_output_cb2(uint16_t data);

	// game-specific driver init
	DECLARE_DRIVER_INIT(generic);
	DECLARE_DRIVER_INIT(pdrift);
	DECLARE_DRIVER_INIT(r360);
	DECLARE_DRIVER_INIT(gforce2);
	DECLARE_DRIVER_INIT(rchase);
	DECLARE_DRIVER_INIT(gloc);

	// video updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void yboard_deluxe(machine_config &config);
	void yboard_link(machine_config &config);
	void yboard(machine_config &config);
	void link_map(address_map &map);
	void link_portmap(address_map &map);
	void main_map(address_map &map);
	void main_map_link(address_map &map);
	void motor_map(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
	void subx_map(address_map &map);
	void suby_map(address_map &map);
protected:
	// internal types
	typedef delegate<void (uint16_t)> output_delegate;

	// timer IDs
	enum
	{
		TID_IRQ2_GEN,
		TID_SOUND_WRITE
	};

	// device overrides
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// internal helpers
	void update_irqs();

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subx;
	required_device<m68000_device> m_suby;
	required_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_linkcpu;
	required_device<mb3773_device> m_watchdog;
	required_device<sega_sys16b_sprite_device> m_bsprites;
	required_device<sega_yboard_sprite_device> m_ysprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<generic_latch_8_device> m_soundlatch;

	// input ports
	optional_ioport_array<6> m_adc_ports;

	// configuration
	output_delegate m_output_cb1;
	output_delegate m_output_cb2;

	// internal state
	uint16_t          m_pdrift_bank;
	emu_timer *     m_scanline_timer;
	int             m_irq2_scanline;
	uint8_t           m_timer_irq_state;
	uint8_t           m_vblank_irq_state;
	uint8_t           m_misc_io_data;
	bitmap_ind16    m_tmp_bitmap;
};
