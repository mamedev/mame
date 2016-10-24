// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System Hang On hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/gen_latch.h"
#include "machine/segaic16.h"
#include "video/segaic16.h"
#include "video/segaic16_road.h"
#include "video/sega16sp.h"


// ======================> segahang_state

class segahang_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segahang_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_subcpu(*this, "subcpu"),
			m_soundcpu(*this, "soundcpu"),
			m_mcu(*this, "mcu"),
			m_i8255_1(*this, "i8255_1"),
			m_i8255_2(*this, "i8255_2"),
			m_sprites(*this, "sprites"),
			m_segaic16vid(*this, "segaic16vid"),
			m_segaic16road(*this, "segaic16road"),
			m_soundlatch(*this, "soundlatch"),
			m_workram(*this, "workram"),
			m_sharrier_video(false),
			m_adc_select(0),
			m_adc_ports(*this, {"ADC0", "ADC1", "ADC2", "ADC3"}),
			m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	// PPI read/write callbacks
	void video_lamps_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tilemap_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sub_control_adc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t adc_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// main CPU read/write handlers
	uint16_t hangon_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hangon_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sharrier_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sharrier_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// Z80 sound CPU read/write handlers
	uint8_t sound_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// I8751-related VBLANK interrupt hanlders
	void i8751_main_cpu_vblank(device_t &device);

	// game-specific driver init
	void init_generic();
	void init_sharrier();
	void init_enduror();
	void init_endurobl();
	void init_endurob2();

	// video updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// internal types
	typedef delegate<void ()> i8751_sim_delegate;

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
	void sharrier_i8751_sim();

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<i8255_device> m_i8255_1;
	required_device<i8255_device> m_i8255_2;
	required_device<sega_16bit_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<segaic16_road_device> m_segaic16road;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory pointers
	required_shared_ptr<uint16_t> m_workram;

	// configuration
	bool                    m_sharrier_video;
	i8751_sim_delegate      m_i8751_vblank_hook;

	// internal state
	uint8_t                   m_adc_select;
	optional_ioport_array<4> m_adc_ports;
	bool                    m_shadow;
	optional_shared_ptr<uint16_t> m_decrypted_opcodes;
};
