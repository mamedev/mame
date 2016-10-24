// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega Y-Board hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
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
		, m_digital_ports(*this, { { "P1", "GENERAL", "LIMITSW", "PORTD", "PORTE", "DSW", "COINAGE", "PORTH" } })
		, m_adc_ports(*this, "ADC.%u", 0)
		, m_pdrift_bank(0)
		, m_scanline_timer(nullptr)
		, m_irq2_scanline(0)
		, m_timer_irq_state(0)
		, m_vblank_irq_state(0)
		, m_tmp_bitmap(512, 512)
	{
		memset(m_analog_data, 0, sizeof(m_analog_data));
		memset(m_misc_io_data, 0, sizeof(m_misc_io_data));
	}

	// main CPU read/write handlers
	uint16_t analog_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void analog_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t io_chip_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void io_chip_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// sound Z80 CPU read/write handlers
	uint8_t sound_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// linked cabinet specific handlers
	void mb8421_intl(int state);
	void mb8421_intr(int state);
	uint16_t link_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t link2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void link2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
//  uint8_t link_portc0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

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
	void init_generic();
	void init_pdrift();
	void init_r360();
	void init_gforce2();
	void init_rchase();
	void init_gloc();

	// video updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

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
	required_device<watchdog_timer_device> m_watchdog;
	required_device<sega_sys16b_sprite_device> m_bsprites;
	required_device<sega_yboard_sprite_device> m_ysprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<generic_latch_8_device> m_soundlatch;

	// input ports
	required_ioport_array<8> m_digital_ports;
	optional_ioport_array<6> m_adc_ports;

	// configuration
	output_delegate m_output_cb1;
	output_delegate m_output_cb2;

	// internal state
	uint16_t          m_pdrift_bank;
	emu_timer *     m_scanline_timer;
	uint8_t           m_analog_data[4];
	int             m_irq2_scanline;
	uint8_t           m_timer_irq_state;
	uint8_t           m_vblank_irq_state;
	uint8_t           m_misc_io_data[0x10];
	bitmap_ind16    m_tmp_bitmap;
};
