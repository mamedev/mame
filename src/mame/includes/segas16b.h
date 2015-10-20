// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System 16B hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/segaic16.h"
#include "sound/2151intf.h"
#include "sound/2413intf.h"
#include "sound/upd7759.h"
#include "video/segaic16.h"
#include "video/sega16sp.h"


// ======================> segas16b_state

class segas16b_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segas16b_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag),
			m_mapper(*this, "mapper"),
			m_maincpu(*this, "maincpu"),
			m_soundcpu(*this, "soundcpu"),
			m_mcu(*this, "mcu"),
			m_ym2151(*this, "ym2151"),
			m_ym2413(*this, "ym2413"),
			m_upd7759(*this, "upd"),
			m_multiplier(*this, "multiplier"),
			m_cmptimer_1(*this, "cmptimer_1"),
			m_cmptimer_2(*this, "cmptimer_2"),
			m_nvram(*this, "nvram"),
			m_sprites(*this, "sprites"),
			m_segaic16vid(*this, "segaic16vid"),
			m_workram(*this, "workram"),
			m_romboard(ROM_BOARD_INVALID),
			m_tilemap_type(SEGAIC16_TILEMAP_16B),
			m_disable_screen_blanking(false),
			m_i8751_initial_config(NULL),
			m_atomicp_sound_divisor(0),
			m_atomicp_sound_count(0),
			m_hwc_input_value(0),
			m_mj_input_num(0),
			m_mj_last_val(0),
			m_gfxdecode(*this, "gfxdecode"),
			m_sound_decrypted_opcodes(*this, "sound_decrypted_opcodes"),
			m_decrypted_opcodes(*this, "decrypted_opcodes")
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

	DECLARE_READ16_MEMBER( bootleg_custom_io_r );
	DECLARE_WRITE16_MEMBER( bootleg_custom_io_w );

	// sound CPU read/write handlers
	DECLARE_WRITE8_MEMBER( upd7759_control_w );
	DECLARE_READ8_MEMBER( upd7759_status_r );
	DECLARE_WRITE16_MEMBER( sound_w16 );

	// other callbacks
	DECLARE_WRITE_LINE_MEMBER(upd7759_generate_nmi);
	INTERRUPT_GEN_MEMBER( i8751_main_cpu_vblank );

	// ROM board-specific driver init
	DECLARE_DRIVER_INIT(generic_5521);
	DECLARE_DRIVER_INIT(generic_5358);
	DECLARE_DRIVER_INIT(generic_5704);
	DECLARE_DRIVER_INIT(generic_5358_small);
	DECLARE_DRIVER_INIT(generic_5797);
	DECLARE_DRIVER_INIT(generic_korean);

	// game-specific driver init
	DECLARE_DRIVER_INIT(isgsm);
	DECLARE_DRIVER_INIT(tturf_5704);
	DECLARE_DRIVER_INIT(wb3_5704);
	DECLARE_DRIVER_INIT(hwchamp_5521);
	DECLARE_DRIVER_INIT(altbeas5_5521);
	DECLARE_DRIVER_INIT(sdi_5358_small);
	DECLARE_DRIVER_INIT(altbeasj_5521);
	DECLARE_DRIVER_INIT(ddux_5704);
	DECLARE_DRIVER_INIT(snapper);
	DECLARE_DRIVER_INIT(shinobi4_5521);
	DECLARE_DRIVER_INIT(goldnaxe_5704);
	DECLARE_DRIVER_INIT(defense_5358_small);
	DECLARE_DRIVER_INIT(sjryuko_5358_small);
	DECLARE_DRIVER_INIT(altbeast_5521);
	DECLARE_DRIVER_INIT(exctleag_5358);
	DECLARE_DRIVER_INIT(tetrbx);
	DECLARE_DRIVER_INIT(aceattac_5358);
	DECLARE_DRIVER_INIT(passshtj_5358);
	DECLARE_DRIVER_INIT(cencourt_5358);
	DECLARE_DRIVER_INIT(shinfz);
	DECLARE_DRIVER_INIT(dunkshot_5358_small);
	DECLARE_DRIVER_INIT(timescan_5358_small);
	DECLARE_DRIVER_INIT(shinobi3_5358);
	DECLARE_DRIVER_INIT(goldnaxe_5797);
	DECLARE_DRIVER_INIT(altbeas4_5521);
	DECLARE_DRIVER_INIT(aliensyn7_5358_small);

	// video updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE16_MEMBER( tileram_w ) { m_segaic16vid->tileram_w(space,offset,data,mem_mask); };
	DECLARE_WRITE16_MEMBER( textram_w ) { m_segaic16vid->textram_w(space,offset,data,mem_mask); };

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
		ROM_BOARD_171_5358_SMALL,       // 171-5358 with smaller ROMs
		ROM_BOARD_171_5358,             // 171-5358
		ROM_BOARD_171_5521,             // 171-5521
		ROM_BOARD_171_5704,             // 171-5704 - don't know any diff between this and 171-5521
		ROM_BOARD_171_5797,             // 171-5797
		ROM_BOARD_KOREAN                // (custom Korean)
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

	// custom I/O handlers
	DECLARE_READ16_MEMBER( aceattac_custom_io_r );
	DECLARE_READ16_MEMBER( dunkshot_custom_io_r );
	DECLARE_READ16_MEMBER( hwchamp_custom_io_r );
	DECLARE_WRITE16_MEMBER( hwchamp_custom_io_w );
	DECLARE_READ16_MEMBER( passshtj_custom_io_r );
	DECLARE_READ16_MEMBER( sdi_custom_io_r );
	DECLARE_READ16_MEMBER( sjryuko_custom_io_r );
	DECLARE_WRITE16_MEMBER( sjryuko_custom_io_w );

	protected:
	// devices
	optional_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	optional_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	optional_device<ym2151_device> m_ym2151;
	optional_device<ym2413_device> m_ym2413;
	optional_device<upd7759_device> m_upd7759;
	optional_device<sega_315_5248_multiplier_device> m_multiplier;
	optional_device<sega_315_5250_compare_timer_device> m_cmptimer_1;
	optional_device<sega_315_5250_compare_timer_device> m_cmptimer_2;
	required_device<nvram_device> m_nvram;
	required_device<sega_sys16b_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;

	// memory pointers
	required_shared_ptr<UINT16> m_workram;

	// configuration
	segas16b_rom_board  m_romboard;
	int                 m_tilemap_type;
	read16_delegate     m_custom_io_r;
	write16_delegate    m_custom_io_w;
	bool                m_disable_screen_blanking;
	const UINT8 *       m_i8751_initial_config;
	i8751_sim_delegate  m_i8751_vblank_hook;
	UINT8               m_atomicp_sound_divisor;

	// game-specific state
	UINT8               m_atomicp_sound_count;
	UINT8               m_hwc_input_value;
	UINT8               m_mj_input_num;
	UINT8               m_mj_last_val;

	required_device<gfxdecode_device> m_gfxdecode;
	optional_shared_ptr<UINT8> m_sound_decrypted_opcodes;
	optional_shared_ptr<UINT16> m_decrypted_opcodes;
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
	UINT8           m_read_xor;
	typedef delegate<UINT32 (UINT32)> security_callback_delegate;
	security_callback_delegate m_security_callback;

	// internal state
	UINT16          m_cart_addrlatch;
	UINT32          m_cart_addr;
	UINT8           m_data_type;
	UINT32          m_data_addr;
	UINT8           m_data_mode;
	UINT16          m_addr_latch;
	UINT32          m_security_value;
	UINT16          m_security_latch;
	UINT8           m_rle_control_position;
	UINT8           m_rle_control_byte;
	bool            m_rle_latched;
	UINT8           m_rle_byte;
};
