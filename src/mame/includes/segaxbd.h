// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega X-Board hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/segaic16.h"
#include "video/segaic16.h"
#include "video/segaic16_road.h"
#include "video/sega16sp.h"


// ======================> segaxbd_state

class segaxbd_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segaxbd_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_subcpu(*this, "subcpu"),
			m_soundcpu(*this, "soundcpu"),
			m_soundcpu2(*this, "soundcpu2"),
			m_mcu(*this, "mcu"),
			m_cmptimer_1(*this, "cmptimer_main"),
			m_sprites(*this, "sprites"),
			m_segaic16vid(*this, "segaic16vid"),
			m_segaic16road(*this, "segaic16road"),
			m_gprider_hack(false),
			m_road_priority(1),
			m_scanline_timer(NULL),
			m_timer_irq_state(0),
			m_vblank_irq_state(0),
			m_loffire_sync(NULL),
			m_lastsurv_mux(0)
	{
		memset(m_adc_reverse, 0, sizeof(m_adc_reverse));
		memset(m_iochip_regs, 0, sizeof(m_iochip_regs));
	}

	// compare/timer chip callbacks
	void timer_ack_callback();
	void sound_data_w(UINT8 data);

	// YM2151 chip callbacks
	DECLARE_WRITE_LINE_MEMBER( sound_cpu_irq );

	// main CPU read/write handlers
	DECLARE_READ16_MEMBER( adc_r );
	DECLARE_WRITE16_MEMBER( adc_w );
	UINT16 iochip_r(int which, int port, int inputval);
	DECLARE_READ16_MEMBER( iochip_0_r );
	DECLARE_WRITE16_MEMBER( iochip_0_w );
	DECLARE_READ16_MEMBER( iochip_1_r );
	DECLARE_WRITE16_MEMBER( iochip_1_w );
	DECLARE_WRITE16_MEMBER( iocontrol_w );

	// game-specific main CPU read/write handlers
	DECLARE_WRITE16_MEMBER( loffire_sync0_w );
	DECLARE_READ16_MEMBER( rascot_excs_r );
	DECLARE_WRITE16_MEMBER( rascot_excs_w );
	DECLARE_READ16_MEMBER( smgp_excs_r );
	DECLARE_WRITE16_MEMBER( smgp_excs_w );

	// sound Z80 CPU read/write handlers
	DECLARE_READ8_MEMBER( sound_data_r );

	// game-specific driver init
	DECLARE_DRIVER_INIT(generic);
	DECLARE_DRIVER_INIT(aburner2);
	DECLARE_DRIVER_INIT(lastsurv);
	DECLARE_DRIVER_INIT(loffire);
	DECLARE_DRIVER_INIT(smgp);
	DECLARE_DRIVER_INIT(rascot);
	DECLARE_DRIVER_INIT(gprider);

	// video updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// internal types
	typedef delegate<UINT8 (UINT8)> ioread_delegate;
	typedef delegate<void (UINT8)> iowrite_delegate;

	// timer IDs
	enum
	{
		TID_SCANLINE,
		TID_IRQ2_GEN,
		TID_SOUND_WRITE
	};

	// device overrides
	virtual void machine_reset();
	virtual void video_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// internal helpers
	void update_main_irqs();
	static void m68k_reset_callback(device_t *device);

	// custom I/O
	void generic_iochip0_lamps_w(UINT8 data);
	UINT8 aburner2_iochip0_motor_r(UINT8 data);
	void aburner2_iochip0_motor_w(UINT8 data);
	UINT8 smgp_iochip0_motor_r(UINT8 data);
	void smgp_iochip0_motor_w(UINT8 data);
	UINT8 lastsurv_iochip1_port_r(UINT8 data);
	void lastsurv_iochip0_muxer_w(UINT8 data);

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_soundcpu2;
	optional_device<i8751_device> m_mcu;
	required_device<sega_315_5250_compare_timer_device> m_cmptimer_1;
	required_device<sega_xboard_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<segaic16_road_device> m_segaic16road;

	// configuration
	bool            m_gprider_hack;
	bool            m_adc_reverse[8];
	ioread_delegate m_iochip_custom_io_r[2][8];
	iowrite_delegate m_iochip_custom_io_w[2][8];
	UINT8           m_road_priority;

	// internal state
	emu_timer *     m_scanline_timer;
	UINT8           m_timer_irq_state;
	UINT8           m_vblank_irq_state;
	UINT8           m_iochip_regs[2][8];

	// game-specific state
	UINT16 *        m_loffire_sync;
	UINT8           m_lastsurv_mux;
};
