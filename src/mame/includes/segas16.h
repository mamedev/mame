/***************************************************************************

    Sega System 16A/16B/18/Outrun/Hang On/X-Board/Y-Board hardware

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "machine/nvram.h"
#include "machine/segaic16.h"
#include "sound/dac.h"
#include "sound/2151intf.h"
#include "sound/2413intf.h"
#include "sound/upd7759.h"
#include "video/segaic16.h"


// ======================> segahang_state

class segahang_state : public driver_device
{
public:
	// construction/destruction
	segahang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_subcpu(*this, "subcpu"),
		  m_soundcpu(*this, "soundcpu"),
		  m_mcu(*this, "mcu"),
		  m_ppi8255_1(*this, "ppi8255_1"),
		  m_ppi8255_2(*this, "ppi8255_2"),
		  m_i8751_vblank_hook(NULL),
		  m_adc_select(0)
	{ }

//protected:
	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<ppi8255_device> m_ppi8255_1;
	required_device<ppi8255_device> m_ppi8255_2;
	
	// configuration
	void (*m_i8751_vblank_hook)(running_machine &machine);

	// internal state
	UINT8 		m_adc_select;
};


// ======================> segas16a_state

class segas16a_state : public driver_device
{
public:
	// construction/destruction
	segas16a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_soundcpu(*this, "soundcpu"),
		  m_mcu(*this, "mcu"),
		  m_ppi8255(*this, "ppi8255"),
		  m_ymsnd(*this, "ymsnd"),
		  m_n7751(*this, "n7751")
	{ }

//protected:
	// devices
	required_device<m68000_device> m_maincpu;
	required_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<ppi8255_device> m_ppi8255;
	required_device<ym2151_device> m_ymsnd;
	optional_device<n7751_device> m_n7751;

	// configuration
	read16_space_func	m_custom_io_r;
	write16_space_func	m_custom_io_w;
	void (*m_i8751_vblank_hook)(running_machine &machine);
	void (*m_lamp_changed_w)(running_machine &machine, UINT8 changed, UINT8 newval);

	// internal state
	UINT8 				m_video_control;
	UINT8 				m_mcu_control;
	UINT8 				m_n7751_command;
	UINT32 				m_n7751_rom_address;
	UINT8 				m_last_buttons1;
	UINT8 				m_last_buttons2;
	int 				m_read_port;
	UINT8 				m_mj_input_num;
};


// ======================> segas16b_state

class segas16b_state : public driver_device
{
public:
	// construction/destruction
	segas16b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_mapper(*this, "mapper"),
		  m_maincpu(*this, "maincpu"),
		  m_soundcpu(*this, "soundcpu"),
		  m_mcu(*this, "mcu"),
		  m_ym2151(*this, "ym2151"),
		  m_ym2413(*this, "ym2413"),
		  m_upd7759(*this, "upd"),
		  m_315_5248_1(*this, "315_5248"),
		  m_315_5250_1(*this, "315_5250_1"),
		  m_315_5250_2(*this, "315_5250_2"),
		  m_nvram(*this, "nvram"),
		  m_workram(*this, "workram"),
		  m_romboard(ROM_BOARD_INVALID),
		  m_tilemap_type(SEGAIC16_TILEMAP_16B),
		  m_disable_screen_blanking(false),
		  m_i8751_initial_config(NULL),
		  m_atomicp_sound_divisor(0),
		  m_atomicp_sound_count(0),
		  m_hwc_input_value(0),
		  m_mj_input_num(0),
		  m_mj_last_val(0)
	{ }

	// memory mapping
	void memory_mapper(sega_315_5195_mapper_device &mapper, UINT8 index);
	UINT8 mapper_sound_r();
	void mapper_sound_w(UINT8 data);

	// main CPU read/write handlers
	DECLARE_WRITE16_MEMBER( rom_5704_bank_w );
	DECLARE_READ16_MEMBER( rom_5797_bank_math_r );
	DECLARE_WRITE16_MEMBER( rom_5797_bank_math_w );
	DECLARE_READ16_MEMBER( unknown_rgn2_r );
	DECLARE_WRITE16_MEMBER( unknown_rgn2_w );
	DECLARE_READ16_MEMBER( standard_io_r );
	DECLARE_WRITE16_MEMBER( standard_io_w );
	DECLARE_WRITE16_MEMBER( atomicp_sound_w );
	
	// sound CPU read/write handlers
	DECLARE_WRITE8_MEMBER( upd7759_control_w );
	DECLARE_READ8_MEMBER( upd7759_status_r );

	// other callbacks
	static void upd7759_generate_nmi(device_t *device, int state);
	INTERRUPT_GEN_MEMBER( i8751_main_cpu_vblank );

	// ROM board-specific driver init
	void init_generic_5358_small();
	void init_generic_5358();
	void init_generic_5521();
	void init_generic_5704();
	void init_generic_5797();
	void init_generic_korean();
	
	// game-specific driver init
	void init_aceattac_5358();
	void init_aliensy3_5358_small();
	void init_altbeast_5521();
	void init_altbeasj_5521();
	void init_altbeas5_5521();
	void init_altbeas4_5521();
	void init_aurail1_5704();
	void init_aurailj_5704();
	void init_ddux_5704();
	void init_dunkshot_5358_small();
	void init_exctleag_5358();
	void init_goldnaxe_5704();
	void init_goldnaxe_5797();
	void init_hwchamp_5521();
	void init_passshtj_5358();
	void init_sdi_5358_small();
	void init_defense_5358_small();
	void init_shinobi4_5521();
	void init_shinobi3_5358();
	void init_sjryuko_5358_small();
	void init_timescan_5358_small();
	void init_tturf_5704();
	void init_wb3_5704();
	void init_snapper();
	
	// video updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// wrappers for legacy functions (to be removed)
	template<write16_space_func _Legacy>
	WRITE16_MEMBER( legacy_wrapper ) { _Legacy(&space, offset, data, mem_mask); }

protected:
	// internal types
	typedef delegate<void ()> i8751_sim_delegate;

	// timer IDs
	enum
	{
		TID_INIT_I8751,
		TID_ATOMICP_SOUND_IRQ
	};

	// rom board types
	enum segas16b_rom_board
	{
		ROM_BOARD_INVALID,
		ROM_BOARD_171_5358_SMALL,		// 171-5358 with smaller ROMs
		ROM_BOARD_171_5358,				// 171-5358
		ROM_BOARD_171_5521,				// 171-5521
		ROM_BOARD_171_5704,				// 171-5704 - don't know any diff between this and 171-5521
		ROM_BOARD_171_5797,				// 171-5797
		ROM_BOARD_KOREAN				// (custom Korean)
	};
	
	// device overrides
	virtual void video_start();
	virtual void machine_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// internal helpers	
	void init_generic(segas16b_rom_board rom_board);

	// i8751 simulations
	void altbeast_common_i8751_sim(offs_t soundoffs, offs_t inputoffs);
	void altbeasj_i8751_sim();
	void altbeas5_i8751_sim();
	void altbeast_i8751_sim();
	void ddux_i8751_sim();
	void goldnaxe_i8751_sim();
	void tturf_i8751_sim();
	void wb3_i8751_sim();

	// custom I/O
	DECLARE_READ16_MEMBER( aceattac_custom_io_r );
	DECLARE_READ16_MEMBER( dunkshot_custom_io_r );
	DECLARE_READ16_MEMBER( hwchamp_custom_io_r );
	DECLARE_WRITE16_MEMBER( hwchamp_custom_io_w );
	DECLARE_READ16_MEMBER( passshtj_custom_io_r );
	DECLARE_READ16_MEMBER( sdi_custom_io_r );
	DECLARE_READ16_MEMBER( sjryuko_custom_io_r );
	DECLARE_WRITE16_MEMBER( sjryuko_custom_io_w );
	
	// devices
	required_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	optional_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	optional_device<ym2151_device> m_ym2151;
	optional_device<ym2413_device> m_ym2413;
	optional_device<upd7759_device> m_upd7759;
	optional_device<ic_315_5248_device> m_315_5248_1;
	optional_device<ic_315_5250_device> m_315_5250_1;
	optional_device<ic_315_5250_device> m_315_5250_2;
	required_device<nvram_device> m_nvram;

	// memory pointers	
	required_shared_ptr<UINT16> m_workram;

	// configuration
	segas16b_rom_board	m_romboard;
	int					m_tilemap_type;
	read16_delegate	 	m_custom_io_r;
	write16_delegate 	m_custom_io_w;
	bool				m_disable_screen_blanking;
	const UINT8 *		m_i8751_initial_config;
	i8751_sim_delegate	m_i8751_vblank_hook;
	UINT8 				m_atomicp_sound_divisor;

	// game-specific state
	UINT8 				m_atomicp_sound_count;
	UINT8 				m_hwc_input_value;
	UINT8 				m_mj_input_num;
	UINT8 				m_mj_last_val;
};


// ======================> isgsm_state

class isgsm_state : public segas16b_state
{
public:
	// construction/destruction
	isgsm_state(const machine_config &mconfig, device_type type, const char *tag)
		: segas16b_state(mconfig, type, tag),
		  m_read_xor(0),
		  m_cart_addrlatch(0),
		  m_cart_addr(0),
		  m_data_type(0),
		  m_data_addr(0),
		  m_data_mode(0),
		  m_addr_latch(0),
		  m_security_value(0),
		  m_security_latch(0),
		  m_rle_control_position(8),
		  m_rle_control_byte(0),
		  m_rle_latched(false),
		  m_rle_byte(0)
	{ }

	// driver init
	void init_isgsm();
	void init_shinfz();
	void init_tetrbx();

	// read/write handlers
	DECLARE_WRITE16_MEMBER( sound_w16 );
	DECLARE_WRITE16_MEMBER( cart_addr_high_w );
	DECLARE_WRITE16_MEMBER( cart_addr_low_w );
	DECLARE_READ16_MEMBER( cart_data_r );
	DECLARE_WRITE16_MEMBER( data_w );
	DECLARE_WRITE16_MEMBER( datatype_w );
	DECLARE_WRITE16_MEMBER( addr_high_w );
	DECLARE_WRITE16_MEMBER( addr_low_w );
	DECLARE_WRITE16_MEMBER( cart_security_high_w );
	DECLARE_WRITE16_MEMBER( cart_security_low_w );
	DECLARE_READ16_MEMBER( cart_security_low_r );
	DECLARE_READ16_MEMBER( cart_security_high_r );
	DECLARE_WRITE16_MEMBER( sound_reset_w );
	DECLARE_WRITE16_MEMBER( main_bank_change_w );

	// security callbacks
	UINT32 shinfz_security(UINT32 input);
	UINT32 tetrbx_security(UINT32 input);

//protected:
	// driver overrides
	virtual void machine_reset();

	// configuration
	UINT8 			m_read_xor;
	typedef delegate<UINT32 (UINT32)> security_callback_delegate;
	security_callback_delegate m_security_callback;

	// internal state
	UINT16 			m_cart_addrlatch;
	UINT32 			m_cart_addr;
	UINT8 			m_data_type;
	UINT32 			m_data_addr;
	UINT8  			m_data_mode;
	UINT16 			m_addr_latch;
	UINT32 			m_security_value;
	UINT16 			m_security_latch;
	UINT8 			m_rle_control_position;
	UINT8 			m_rle_control_byte;
	bool 			m_rle_latched;
	UINT8 			m_rle_byte;
};


// ======================> segas18_state

class segas18_state : public driver_device
{
public:
	// construction/destruction
	segas18_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_mapper(*this, "mapper"),
		  m_maincpu(*this, "maincpu"),
		  m_soundcpu(*this, "soundcpu"),
		  m_mcu(*this, "mcu"),
		  m_nvram(*this, "nvram"),
		  m_workram(*this, "workram"),
		  m_romboard(ROM_BOARD_INVALID),
		  m_has_guns(false),
		  m_grayscale_enable(false),
		  m_vdp_enable(false),
		  m_vdp_mixing(0),
		  m_mcu_data(0),
		  m_lghost_value(0),
		  m_lghost_select(0)
	{
		memset(m_misc_io_data, 0, sizeof(m_misc_io_data));
		memset(m_wwally_last_x, 0, sizeof(m_wwally_last_x));
		memset(m_wwally_last_y, 0, sizeof(m_wwally_last_y));
	}
	
	// driver init
	void init_generic_shad();
	void init_generic_5874();
	void init_generic_5987();
	void init_ddcrew();
	void init_lghost();
	void init_wwally();

	// memory mapping
	void memory_mapper(sega_315_5195_mapper_device &mapper, UINT8 index);
	UINT8 mapper_sound_r();
	void mapper_sound_w(UINT8 data);

	// read/write handlers
	DECLARE_WRITE16_MEMBER( rom_5987_bank_w );
	DECLARE_READ16_MEMBER( io_chip_r );
	DECLARE_WRITE16_MEMBER( io_chip_w );
	DECLARE_READ16_MEMBER( misc_io_r );
	DECLARE_WRITE16_MEMBER( misc_io_w );
	DECLARE_WRITE8_MEMBER( soundbank_w );
	DECLARE_WRITE8_MEMBER( mcu_data_w );

	// custom I/O
	DECLARE_READ16_MEMBER( ddcrew_custom_io_r );
	DECLARE_READ16_MEMBER( lghost_custom_io_r );
	DECLARE_WRITE16_MEMBER( lghost_custom_io_w );
	DECLARE_READ16_MEMBER( wwally_custom_io_r );
	DECLARE_WRITE16_MEMBER( wwally_custom_io_w );

	// video rendering
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// wrappers for legacy functions (to be removed)
	template<read16_space_func _Legacy>
	READ16_MEMBER( legacy_wrapper_r ) { return _Legacy(&space, offset, mem_mask); }
	template<write16_space_func _Legacy>
	WRITE16_MEMBER( legacy_wrapper ) { _Legacy(&space, offset, data, mem_mask); }

protected:
	// timer IDs
	enum
	{
		TID_INITIAL_BOOST
	};

	// rom board types
	enum segas18_rom_board
	{
		ROM_BOARD_INVALID,
		ROM_BOARD_171_SHADOW,	// 171-???? -- used by shadow dancer
		ROM_BOARD_171_5874,		// 171-5874
		ROM_BOARD_171_5987		// 171-5987
	};
	
	// device overrides
	virtual void machine_reset();
	virtual void video_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// internal helpers	
	void init_generic(segas18_rom_board rom_board);
	void set_grayscale(bool enable);
	void set_vdp_enable(bool enable);
	void set_vdp_mixing(UINT8 mixing);
	void draw_vdp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);

	// devices
	required_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	required_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<nvram_device> m_nvram;

	// memory pointers	
	required_shared_ptr<UINT16> m_workram;
	
	// configuration
	segas18_rom_board	m_romboard;
	read16_delegate	 	m_custom_io_r;
	write16_delegate 	m_custom_io_w;
	bool 				m_has_guns;

	// internal state
	bool 				m_grayscale_enable;
	bool 				m_vdp_enable;
	UINT8 				m_vdp_mixing;
	bitmap_ind16		m_temp_bitmap;
	UINT8 				m_mcu_data;
	UINT8 				m_misc_io_data[0x10];
	
	// game-specific state
	UINT8 				m_wwally_last_x[3];
	UINT8 				m_wwally_last_y[3];
	UINT8 				m_lghost_value;
	UINT8 				m_lghost_select;
};


// ======================> segaorun_state

class segaorun_state : public driver_device
{
public:
	// construction/destruction
	segaorun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_mapper(*this, "mapper"),
		  m_maincpu(*this, "maincpu"),
		  m_subcpu(*this, "subcpu"),
		  m_soundcpu(*this, "soundcpu"),
		  m_ppi8255(*this, "ppi8255"),
		  m_nvram(*this, "nvram"),
		  m_workram(*this, "workram"),
		  m_custom_map(NULL),
		  m_is_shangon(false),
		  m_scanline_timer(NULL),
		  m_irq2_state(0),
		  m_adc_select(0),
		  m_vblank_irq_state(0)
	{ }

	// driver init
	void init_outrun();
	void init_outrunb();
	void init_shangon();
	void init_shangon3();

	// memory mapping
	void memory_mapper(sega_315_5195_mapper_device &mapper, UINT8 index);
	UINT8 mapper_sound_r();
	void mapper_sound_w(UINT8 data);

	// read/write handlers
	DECLARE_READ16_MEMBER( misc_io_r );
	DECLARE_WRITE16_MEMBER( misc_io_w );
	DECLARE_WRITE16_MEMBER( nop_w );
	DECLARE_READ8_MEMBER( unknown_porta_r );
	DECLARE_READ8_MEMBER( unknown_portb_r );
	DECLARE_READ8_MEMBER( unknown_portc_r );
	DECLARE_WRITE8_MEMBER( unknown_porta_w );
	DECLARE_WRITE8_MEMBER( unknown_portb_w );
	DECLARE_WRITE8_MEMBER( video_control_w );
	DECLARE_READ8_MEMBER( sound_data_r );

	// custom I/O
	DECLARE_READ16_MEMBER( outrun_custom_io_r );
	DECLARE_WRITE16_MEMBER( outrun_custom_io_w );
	DECLARE_READ16_MEMBER( shangon_custom_io_r );
	DECLARE_WRITE16_MEMBER( shangon_custom_io_w );

	// video rendering
	UINT32 screen_update_outrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_shangon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// wrappers for legacy functions (to be removed)
	template<read16_space_func _Legacy>
	READ16_MEMBER( legacy_wrapper_r ) { return _Legacy(&space, offset, mem_mask); }
	template<write16_space_func _Legacy>
	WRITE16_MEMBER( legacy_wrapper ) { _Legacy(&space, offset, data, mem_mask); }

protected:
	// timer IDs
	enum
	{
		TID_SCANLINE,
		TID_IRQ2_GEN,
		TID_SOUND_WRITE
	};
	
	// device overrides
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// internal helpers
	void init_generic();
	void update_main_irqs();
	static void m68k_reset_callback(device_t *device);
	
	// devices
	required_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	required_device<ppi8255_device> m_ppi8255;
	optional_device<nvram_device> m_nvram;

	// memory
	required_shared_ptr<UINT16> m_workram;
	
	// configuration
	read16_delegate	 	m_custom_io_r;
	write16_delegate 	m_custom_io_w;
	const UINT8 *		m_custom_map;
	bool				m_is_shangon;

	// internal state
	emu_timer *			m_scanline_timer;
	UINT8 				m_irq2_state;
	UINT8 				m_adc_select;
	UINT8 				m_vblank_irq_state;
};


// ======================> segaxbd_state

class segaxbd_state : public driver_device
{
public:
	// construction/destruction
	segaxbd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_subcpu(*this, "subcpu"),
		  m_soundcpu(*this, "soundcpu"),
		  m_soundcpu2(*this, "soundcpu2"),
		  m_mcu(*this, "mcu"),
		  m_315_5250_1(*this, "5250_main"),
		  m_gprider_hack(false),
		  m_timer_irq_state(0),
		  m_vblank_irq_state(0),
		  m_road_priority(0),
		  m_loffire_sync(NULL),
		  m_lastsurv_mux(0)
	{
		memset(m_adc_reverse, 0, sizeof(m_adc_reverse));
		memset(m_iochip_custom_io_r, 0, sizeof(m_iochip_custom_io_r));
		memset(m_iochip_custom_io_w, 0, sizeof(m_iochip_custom_io_w));
		memset(m_iochip_regs, 0, sizeof(m_iochip_regs));
	}

//protected:
	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_soundcpu2;
	optional_device<i8751_device> m_mcu;
	required_device<ic_315_5250_device> m_315_5250_1;

	// configuration
	bool			m_gprider_hack;
	bool 			m_adc_reverse[8];
	UINT8 (*m_iochip_custom_io_r[2][8])(running_machine &machine, UINT8 data);
	void (*m_iochip_custom_io_w[2][8])(running_machine &machine, UINT8 data);
	
	// internal state
	emu_timer *		m_scanline_timer;
	UINT8 			m_timer_irq_state;
	UINT8 			m_vblank_irq_state;
	UINT8 			m_iochip_regs[2][8];
	UINT8 			m_road_priority;

	// game-specific state
	UINT16 *		m_loffire_sync;
	UINT8			m_lastsurv_mux;
};


// ======================> segaybd_state

class segaybd_state : public driver_device
{
public:
	// construction/destruction
	segaybd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_subx(*this, "subx"),
		  m_suby(*this, "suby"),
		  m_soundcpu(*this, "soundcpu")
	{
		memset(m_analog_data, 0, sizeof(m_analog_data));
		memset(m_misc_io_data, 0, sizeof(m_misc_io_data));
	}

//protected:
	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subx;
	required_device<m68000_device> m_suby;
	required_device<z80_device> m_soundcpu;

	// internal state
	emu_timer *		m_scanline_timer;
	UINT8 			m_analog_data[4];
	int 			m_irq2_scanline;
	UINT8 			m_timer_irq_state;
	UINT8 			m_vblank_irq_state;
	UINT8 			m_misc_io_data[0x10];
	bitmap_ind16 *	m_tmp_bitmap;
};


/*----------- defined in video/segahang.c -----------*/

VIDEO_START( hangon );
VIDEO_START( sharrier );
SCREEN_UPDATE_IND16( hangon );

/*----------- defined in video/segas16a.c -----------*/

VIDEO_START( system16a );
SCREEN_UPDATE_IND16( system16a );

/*----------- defined in video/segaxbd.c -----------*/

VIDEO_START( xboard );
SCREEN_UPDATE_IND16( xboard );

/*----------- defined in video/segaybd.c -----------*/

VIDEO_START( yboard );
SCREEN_UPDATE_IND16( yboard );
