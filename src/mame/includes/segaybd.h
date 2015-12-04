// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega Y-Board hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/segaic16.h"
#include "video/segaic16.h"
#include "video/sega16sp.h"


// ======================> segaybd_state

class segaybd_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segaybd_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_subx(*this, "subx"),
			m_suby(*this, "suby"),
			m_soundcpu(*this, "soundcpu"),
			m_linkcpu(*this, "linkcpu"),
			m_bsprites(*this, "bsprites"),
			m_ysprites(*this, "ysprites"),
			m_segaic16vid(*this, "segaic16vid"),
			m_digital_ports(*this, digital_ports),
			m_adc_ports(*this, "ADC"),
			m_pdrift_bank(0),
			m_scanline_timer(nullptr),
			m_irq2_scanline(0),
			m_timer_irq_state(0),
			m_vblank_irq_state(0),
			m_tmp_bitmap(512, 512)
	{
		memset(m_analog_data, 0, sizeof(m_analog_data));
		memset(m_misc_io_data, 0, sizeof(m_misc_io_data));
	}

	// YM2151 chip callbacks
	DECLARE_WRITE_LINE_MEMBER( sound_cpu_irq );

	// main CPU read/write handlers
	DECLARE_READ16_MEMBER( analog_r );
	DECLARE_WRITE16_MEMBER( analog_w );
	DECLARE_READ16_MEMBER( io_chip_r );
	DECLARE_WRITE16_MEMBER( io_chip_w );
	DECLARE_WRITE16_MEMBER( sound_data_w );

	// sound Z80 CPU read/write handlers
	DECLARE_READ8_MEMBER( sound_data_r );

	// linked cabinet specific handlers
	DECLARE_WRITE_LINE_MEMBER( mb8421_intl );
	DECLARE_WRITE_LINE_MEMBER( mb8421_intr );
	DECLARE_READ16_MEMBER( link_r );
	DECLARE_READ16_MEMBER( link2_r );
	DECLARE_WRITE16_MEMBER( link2_w );
	DECLARE_READ8_MEMBER( link_portc0_r );

	// game-specific output handlers
	void gforce2_output_cb1(UINT16 data);
	void gforce2_output_cb2(UINT16 data);
	void gloc_output_cb1(UINT16 data);
	void gloc_output_cb2(UINT16 data);
	void r360_output_cb2(UINT16 data);
	void pdrift_output_cb1(UINT16 data);
	void pdrift_output_cb2(UINT16 data);
	void rchase_output_cb2(UINT16 data);

	// game-specific driver init
	DECLARE_DRIVER_INIT(generic);
	DECLARE_DRIVER_INIT(pdrift);
	DECLARE_DRIVER_INIT(r360);
	DECLARE_DRIVER_INIT(gforce2);
	DECLARE_DRIVER_INIT(rchase);
	DECLARE_DRIVER_INIT(gloc);

	// video updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// internal types
	typedef delegate<void (UINT16)> output_delegate;

	// timer IDs
	enum
	{
		TID_IRQ2_GEN,
		TID_SOUND_WRITE
	};

	// device overrides
	virtual void machine_reset();
	virtual void video_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// internal helpers
	void update_irqs();

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subx;
	required_device<m68000_device> m_suby;
	required_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_linkcpu;
	required_device<sega_sys16b_sprite_device> m_bsprites;
	required_device<sega_yboard_sprite_device> m_ysprites;
	required_device<segaic16_video_device> m_segaic16vid;

	// input ports
	DECLARE_IOPORT_ARRAY(digital_ports);
	required_ioport_array<8> m_digital_ports;
	optional_ioport_array<6> m_adc_ports;

	// configuration
	output_delegate m_output_cb1;
	output_delegate m_output_cb2;

	// internal state
	UINT16          m_pdrift_bank;
	emu_timer *     m_scanline_timer;
	UINT8           m_analog_data[4];
	int             m_irq2_scanline;
	UINT8           m_timer_irq_state;
	UINT8           m_vblank_irq_state;
	UINT8           m_misc_io_data[0x10];
	bitmap_ind16    m_tmp_bitmap;
};
