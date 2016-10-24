// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega Outrun hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/segaic16.h"
#include "machine/watchdog.h"
#include "video/segaic16.h"
#include "video/segaic16_road.h"
#include "video/sega16sp.h"


// ======================> segaorun_state

class segaorun_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segaorun_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag),
		m_mapper(*this, "mapper"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_soundcpu(*this, "soundcpu"),
		m_i8255(*this, "i8255"),
		m_nvram(*this, "nvram"),
		m_watchdog(*this, "watchdog"),
		m_sprites(*this, "sprites"),
		m_segaic16vid(*this, "segaic16vid"),
		m_segaic16road(*this, "segaic16road"),
		m_soundlatch(*this, "soundlatch"),
		m_bankmotor_timer(*this, "bankmotor"),
		m_digital_ports(*this, { { "SERVICE", "UNKNOWN", "COINAGE", "DSW" } }),
		m_adc_ports(*this, "ADC.%u", 0),
		m_workram(*this, "workram"),
		m_custom_map(nullptr),
		m_shangon_video(false),
		m_scanline_timer(nullptr),
		m_irq2_state(0),
		m_adc_select(0),
		m_vblank_irq_state(0),
		m_bankmotor_pos(0x8000),
		m_bankmotor_delta(0)
	{ }

	// PPI read/write handlers
	uint8_t unknown_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t unknown_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t unknown_portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void unknown_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void unknown_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bankmotor_limit_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bankmotor_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// memory mapping
	void memory_mapper(sega_315_5195_mapper_device &mapper, uint8_t index);
	uint8_t mapper_sound_r();
	void mapper_sound_w(uint8_t data);

	// main CPU read/write handlers
	uint16_t misc_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void misc_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nop_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// Z80 sound CPU read/write handlers
	uint8_t sound_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// game-specific driver init
	void init_generic();
	void init_outrun();
	void init_outrunb();
	void init_shangon();

	// video updates
	uint32_t screen_update_outrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_shangon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_segaic16vid->tileram_w(space,offset,data,mem_mask); };
	void textram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_segaic16vid->textram_w(space,offset,data,mem_mask); };
	uint16_t sega_road_control_0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_segaic16road->segaic16_road_control_0_r(space,offset,mem_mask); };
	void sega_road_control_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_segaic16road->segaic16_road_control_0_w(space,offset,data,mem_mask); };

	ioport_value bankmotor_pos_r(ioport_field &field, void *param);
	void bankmotor_update(timer_device &timer, void *ptr, int32_t param);

protected:
	// timer IDs
	enum
	{
		TID_SCANLINE,
		TID_IRQ2_GEN,
		TID_SOUND_WRITE
	};

	// device overrides
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// internal helpers
	void update_main_irqs();
	void m68k_reset_callback(int state);

	// custom I/O
	uint16_t outrun_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void outrun_custom_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t shangon_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void shangon_custom_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// devices
	required_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	required_device<i8255_device> m_i8255;
	optional_device<nvram_device> m_nvram;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<sega_16bit_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<segaic16_road_device> m_segaic16road;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_device<timer_device> m_bankmotor_timer;

	// input ports
	required_ioport_array<4> m_digital_ports;
	optional_ioport_array<8> m_adc_ports;

	// memory
	required_shared_ptr<uint16_t> m_workram;

	// configuration
	read16_delegate     m_custom_io_r;
	write16_delegate    m_custom_io_w;
	const uint8_t *       m_custom_map;
	bool                m_shangon_video;

	// internal state
	emu_timer *         m_scanline_timer;
	uint8_t               m_irq2_state;
	uint8_t               m_adc_select;
	uint8_t               m_vblank_irq_state;
	int                 m_bankmotor_pos;
	int                 m_bankmotor_delta;
};
