// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega pre-System 16 & System 16A hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/i8243.h"
#include "machine/nvram.h"
#include "machine/segaic16.h"
#include "sound/2151intf.h"
#include "video/segaic16.h"
#include "video/sega16sp.h"


// ======================> segas16a_state

class segas16a_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segas16a_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_soundcpu(*this, "soundcpu"),
			m_mcu(*this, "mcu"),
			m_i8255(*this, "i8255"),
			m_ymsnd(*this, "ymsnd"),
			m_n7751(*this, "n7751"),
			m_n7751_i8243(*this, "n7751_8243"),
			m_nvram(*this, "nvram"),
			m_segaic16vid(*this, "segaic16vid"),
			m_sprites(*this, "sprites"),
			m_workram(*this, "nvram"),
			m_sound_decrypted_opcodes(*this, "sound_decrypted_opcodes"),
			m_video_control(0),
			m_mcu_control(0),
			m_n7751_command(0),
			m_n7751_rom_address(0),
			m_last_buttons1(0),
			m_last_buttons2(0),
			m_read_port(0),
			m_mj_input_num(0)
	{ }

	// PPI read/write callbacks
	DECLARE_WRITE8_MEMBER( misc_control_w );
	DECLARE_WRITE8_MEMBER( tilemap_sound_w );

	// main CPU read/write handlers
	DECLARE_READ16_MEMBER( standard_io_r );
	DECLARE_WRITE16_MEMBER( standard_io_w );
	DECLARE_READ16_MEMBER( misc_io_r );
	DECLARE_WRITE16_MEMBER( misc_io_w );

	// Z80 sound CPU read/write handlers
	DECLARE_READ8_MEMBER( sound_data_r );
	DECLARE_WRITE8_MEMBER( n7751_command_w );
	DECLARE_WRITE8_MEMBER( n7751_control_w );
	DECLARE_WRITE8_MEMBER( n7751_rom_offset_w );

	// N7751 sound generator CPU read/write handlers
	DECLARE_READ8_MEMBER( n7751_rom_r );
	DECLARE_READ8_MEMBER( n7751_p2_r );
	DECLARE_WRITE8_MEMBER( n7751_p2_w );
	DECLARE_READ8_MEMBER( n7751_t1_r );

	// I8751 MCU read/write handlers
	DECLARE_WRITE8_MEMBER( mcu_control_w );
	DECLARE_WRITE8_MEMBER( mcu_io_w );
	DECLARE_READ8_MEMBER( mcu_io_r );

	// I8751-related VBLANK interrupt handlers
	INTERRUPT_GEN_MEMBER( mcu_irq_assert );
	INTERRUPT_GEN_MEMBER( i8751_main_cpu_vblank );

	// game-specific driver init
	DECLARE_DRIVER_INIT(generic);
	DECLARE_DRIVER_INIT(dumpmtmt);
	DECLARE_DRIVER_INIT(quartet);
	DECLARE_DRIVER_INIT(fantzonep);
	DECLARE_DRIVER_INIT(sjryukoa);
	DECLARE_DRIVER_INIT(aceattaca);
	DECLARE_DRIVER_INIT(passsht16a);
	DECLARE_DRIVER_INIT(mjleague);
	DECLARE_DRIVER_INIT(sdi);

	// video updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// internal types
	typedef delegate<void ()> i8751_sim_delegate;
	typedef delegate<void (UINT8, UINT8)> lamp_changed_delegate;

	// timer IDs
	enum
	{
		TID_INIT_I8751,
		TID_PPI_WRITE
	};

	// driver overrides
	virtual void video_start();
	virtual void machine_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// I8751 simulations
	void dumpmtmt_i8751_sim();
	void quartet_i8751_sim();

	// custom I/O handlers
	DECLARE_READ16_MEMBER( aceattaca_custom_io_r );
	DECLARE_READ16_MEMBER( mjleague_custom_io_r );
	DECLARE_READ16_MEMBER( passsht16a_custom_io_r );
	DECLARE_READ16_MEMBER( sdi_custom_io_r );
	DECLARE_READ16_MEMBER( sjryuko_custom_io_r );
	void sjryuko_lamp_changed_w(UINT8 changed, UINT8 newval);

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<i8255_device> m_i8255;
	required_device<ym2151_device> m_ymsnd;
	optional_device<n7751_device> m_n7751;
	optional_device<i8243_device> m_n7751_i8243;
	required_device<nvram_device> m_nvram;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<sega_sys16a_sprite_device> m_sprites;

	// memory pointers
	required_shared_ptr<UINT16> m_workram;
	optional_shared_ptr<UINT8> m_sound_decrypted_opcodes;

	// configuration
	read16_delegate         m_custom_io_r;
	write16_delegate        m_custom_io_w;
	i8751_sim_delegate      m_i8751_vblank_hook;
	lamp_changed_delegate   m_lamp_changed_w;

	// internal state
	UINT8                   m_video_control;
	UINT8                   m_mcu_control;
	UINT8                   m_n7751_command;
	UINT32                  m_n7751_rom_address;
	UINT8                   m_last_buttons1;
	UINT8                   m_last_buttons2;
	UINT8                   m_read_port;
	UINT8                   m_mj_input_num;
};
