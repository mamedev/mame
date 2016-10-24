// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega pre-System 16 & System 16A hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/i8243.h"
#include "machine/nvram.h"
#include "machine/segaic16.h"
#include "machine/watchdog.h"
#include "sound/ym2151.h"
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
			m_watchdog(*this, "watchdog"),
			m_segaic16vid(*this, "segaic16vid"),
			m_soundlatch(*this, "soundlatch"),
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
			m_mj_input_num(0),
			m_mj_inputs(*this, {"MJ0", "MJ1", "MJ2", "MJ3", "MJ4", "MJ5"})
	{ }

	// PPI read/write callbacks
	void misc_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tilemap_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// main CPU read/write handlers
	uint16_t standard_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void standard_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t misc_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void misc_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// Z80 sound CPU read/write handlers
	uint8_t sound_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void n7751_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void n7751_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void n7751_rom_offset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// N7751 sound generator CPU read/write handlers
	uint8_t n7751_rom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t n7751_p2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void n7751_p2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t n7751_t1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// I8751 MCU read/write handlers
	void mcu_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// I8751-related VBLANK interrupt handlers
	void mcu_irq_assert(device_t &device);
	void i8751_main_cpu_vblank(device_t &device);

	// game-specific driver init
	void init_generic();
	void init_dumpmtmt();
	void init_quartet();
	void init_fantzonep();
	void init_sjryukoa();
	void init_aceattaca();
	void init_passsht16a();
	void init_mjleague();
	void init_sdi();

	// video updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// internal types
	typedef delegate<void ()> i8751_sim_delegate;
	typedef delegate<void (uint8_t, uint8_t)> lamp_changed_delegate;

	// timer IDs
	enum
	{
		TID_INIT_I8751,
		TID_PPI_WRITE
	};

	// driver overrides
	virtual void video_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// I8751 simulations
	void dumpmtmt_i8751_sim();
	void quartet_i8751_sim();

	// custom I/O handlers
	uint16_t aceattaca_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mjleague_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t passsht16a_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sdi_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sjryuko_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sjryuko_lamp_changed_w(uint8_t changed, uint8_t newval);

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<i8255_device> m_i8255;
	required_device<ym2151_device> m_ymsnd;
	optional_device<n7751_device> m_n7751;
	optional_device<i8243_device> m_n7751_i8243;
	required_device<nvram_device> m_nvram;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<sega_sys16a_sprite_device> m_sprites;

	// memory pointers
	required_shared_ptr<uint16_t> m_workram;
	optional_shared_ptr<uint8_t> m_sound_decrypted_opcodes;

	// configuration
	read16_delegate         m_custom_io_r;
	write16_delegate        m_custom_io_w;
	i8751_sim_delegate      m_i8751_vblank_hook;
	lamp_changed_delegate   m_lamp_changed_w;

	// internal state
	uint8_t                   m_video_control;
	uint8_t                   m_mcu_control;
	uint8_t                   m_n7751_command;
	uint32_t                  m_n7751_rom_address;
	uint8_t                   m_last_buttons1;
	uint8_t                   m_last_buttons2;
	uint8_t                   m_read_port;
	uint8_t                   m_mj_input_num;
	optional_ioport_array<6> m_mj_inputs;
};
