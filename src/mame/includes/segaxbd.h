// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega X-Board hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "machine/segaic16.h"
#include "video/segaic16.h"
#include "video/segaic16_road.h"
#include "video/sega16sp.h"
#include "video/resnet.h"
#include "screen.h"

// ======================> segaxbd_state


class segaxbd_state : public device_t
{
public:
	// construction/destruction
	segaxbd_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	segaxbd_state(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// compare/timer chip callbacks
	void timer_ack_callback();
	void sound_data_w(uint8_t data);

	// main CPU read/write handlers
	DECLARE_READ16_MEMBER( adc_r );
	DECLARE_WRITE16_MEMBER( adc_w );
	uint16_t iochip_r(int which, int port, int inputval);
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


	// video updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// palette helpers
	DECLARE_WRITE16_MEMBER( paletteram_w );

	void install_aburner2(void);
	void install_lastsurv(void);
	void install_loffire(void);
	void install_smgp(void);
	void install_gprider(void);

protected:
	// internal types
	typedef delegate<uint8_t (uint8_t)> ioread_delegate;
	typedef delegate<void (uint8_t)> iowrite_delegate;

	// timer IDs
	enum
	{
		TID_SCANLINE,
		TID_IRQ2_GEN,
		TID_SOUND_WRITE
	};

	// device overrides
//  virtual void machine_reset();
	virtual void video_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// internal helpers
	void update_main_irqs();
	DECLARE_WRITE_LINE_MEMBER(m68k_reset_callback);

	// custom I/O
	void generic_iochip0_lamps_w(uint8_t data);
	uint8_t aburner2_iochip0_motor_r(uint8_t data);
	void aburner2_iochip0_motor_w(uint8_t data);
	uint8_t smgp_iochip0_motor_r(uint8_t data);
	void smgp_iochip0_motor_w(uint8_t data);
	uint8_t lastsurv_iochip1_port_r(uint8_t data);
	void lastsurv_iochip0_muxer_w(uint8_t data);

	// devices
public:
	required_device<m68000_device> m_maincpu;
protected:
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_soundcpu2;
	optional_device<i8751_device> m_mcu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<sega_315_5250_compare_timer_device> m_cmptimer_1;
	required_device<sega_xboard_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<segaic16_road_device> m_segaic16road;
	required_device<generic_latch_8_device> m_soundlatch;
	required_shared_ptr<uint16_t> m_subram0;

	// configuration
	bool            m_adc_reverse[8];
	ioread_delegate m_iochip_custom_io_r[2][8];
	iowrite_delegate m_iochip_custom_io_w[2][8];
	uint8_t           m_road_priority;

	// internal state
	emu_timer *     m_scanline_timer;
	uint8_t           m_timer_irq_state;
	uint8_t           m_vblank_irq_state;
	uint8_t           m_iochip_regs[2][8];

	// game-specific state
	uint16_t *        m_loffire_sync;
	uint8_t           m_lastsurv_mux;
public: // -- stupid system16.c
	// memory pointers
	required_shared_ptr<uint16_t> m_paletteram;
	bool            m_gprider_hack;

protected:
	void palette_init();
	uint32_t      m_palette_entries;          // number of palette entries
	uint8_t       m_palette_normal[32];       // RGB translations for normal pixels
	uint8_t       m_palette_shadow[32];       // RGB translations for shadowed pixels
	uint8_t       m_palette_hilight[32];      // RGB translations for hilighted pixels
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_ioport_array<8> m_adc_ports;
	optional_ioport_array<4> m_mux_ports;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};


class segaxbd_regular_state :  public segaxbd_state
{
public:
	segaxbd_regular_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
//  virtual void device_start();
//  virtual void device_reset();
};



class segaxbd_fd1094_state :  public segaxbd_state
{
public:
	segaxbd_fd1094_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
//  virtual void device_start();
//  virtual void device_reset();
};

class segaxbd_lastsurv_fd1094_state :  public segaxbd_state
{
public:
	segaxbd_lastsurv_fd1094_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
//  virtual void device_start();
//  virtual void device_reset();
};

class segaxbd_lastsurv_state :  public segaxbd_state
{
public:
	segaxbd_lastsurv_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
//  virtual void device_start();
//  virtual void device_reset();
};


class segaxbd_smgp_fd1094_state :  public segaxbd_state
{
public:
	segaxbd_smgp_fd1094_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
//  virtual void device_start();
//  virtual void device_reset();
};


class segaxbd_smgp_state :  public segaxbd_state
{
public:
	segaxbd_smgp_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
//  virtual void device_start();
//  virtual void device_reset();
};


class segaxbd_rascot_state :  public segaxbd_state
{
public:
	segaxbd_rascot_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
//  virtual void device_start();
//  virtual void device_reset();
};
