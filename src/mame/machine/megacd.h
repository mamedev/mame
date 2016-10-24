// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega CD / Mega CD */

#include "cpu/m68000/m68000.h"
#include "machine/lc89510.h"
#include "machine/megacdcd.h"
#include "sound/rf5c68.h"

#define SEGACD_CLOCK      12500000

#define RAM_MODE_2MEG (0)
#define RAM_MODE_1MEG (2)

#define DMA_PCM  (0x0400)
#define DMA_PRG  (0x0500)
#define DMA_WRAM (0x0700)

// irq3 timer
#define CHECK_SCD_LV3_INTERRUPT \
	if (m_lc89510_temp->get_segacd_irq_mask() & 0x08) \
	{ \
		m_scdcpu->set_input_line(3, HOLD_LINE); \
	}
// from master
#define CHECK_SCD_LV2_INTERRUPT \
	if (m_lc89510_temp->get_segacd_irq_mask() & 0x04) \
	{ \
		m_scdcpu->set_input_line(2, HOLD_LINE); \
	}

// gfx convert
#define CHECK_SCD_LV1_INTERRUPT \
	if (m_lc89510_temp->get_segacd_irq_mask() & 0x02) \
	{ \
		m_scdcpu->set_input_line(1, HOLD_LINE); \
	}

#define SEGACD_IRQ3_TIMER_SPEED (attotime::from_nsec(m_irq3_timer_reg*30720))


class sega_segacd_device : public device_t, public device_gfx_interface
{
public:
	sega_segacd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	required_device<cpu_device> m_scdcpu;
	required_device<rf5c68_device> m_rfsnd;
	required_device<lc89510_temp_device> m_lc89510_temp;
	required_device<timer_device> m_stopwatch_timer;
	required_device<timer_device> m_stamp_timer;
	required_device<timer_device> m_irq3_timer;
	required_device<timer_device> m_dma_timer;
	//required_device<timer_device> m_hock_timer;

	required_shared_ptr<uint16_t> m_prgram;
	required_shared_ptr<uint16_t> m_dataram;
	required_shared_ptr<uint16_t> m_font_bits;

	// can't use a memshare because it's 8-bit RAM in a 16-bit address space
	std::vector<uint8_t> m_backupram;

	uint8_t m_font_color;

	uint16_t scd_rammode;
	uint32_t scd_mode_dmna_ret_flags ;

	tilemap_t    *segacd_stampmap[4];


	uint8_t segacd_ram_writeprotect_bits;
	int segacd_4meg_prgbank;// = 0; // which bank the MainCPU can see of the SubCPU PrgRAM
	int segacd_memory_priority_mode;// = 0;
	int segacd_stampsize;

	uint16_t segacd_hint_register;
	uint16_t segacd_imagebuffer_vdot_size;
	uint16_t segacd_imagebuffer_vcell_size;
	uint16_t segacd_imagebuffer_hdot_size;

	int segacd_conversion_active;// = 0;
	uint16_t segacd_stampmap_base_address;
	uint16_t segacd_imagebuffer_start_address;
	uint16_t segacd_imagebuffer_offset;


	uint16_t segacd_comms_flags;// = 0x0000;
	uint16_t segacd_comms_part1[0x8];
	uint16_t segacd_comms_part2[0x8];

	int segacd_redled;// = 0;
	int segacd_greenled;// = 0;
	int segacd_ready;// = 1; // actually set 100ms after startup?
	uint8_t m_irq3_timer_reg;


	void irq3_timer_callback(timer_device &timer, void *ptr, int32_t param);
	void stamp_timer_callback(timer_device &timer, void *ptr, int32_t param);

	inline void write_pixel(uint8_t pix, int pixeloffset);
	uint16_t segacd_1meg_mode_word_read(int offset, uint16_t mem_mask);
	void segacd_1meg_mode_word_write(int offset, uint16_t data, uint16_t mem_mask, int use_pm);

	uint16_t segacd_dmaaddr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void segacd_dmaaddr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t m_dmaaddr;

	uint16_t m_a12000_halt_reset_reg;

	int m_framerate;
	int m_base_total_scanlines;
	int m_total_scanlines;

	void segacd_mark_tiles_dirty(int offset);
	int segacd_get_active_stampmap_tilemap(void);

	// set some variables at start, depending on region (shall be moved to a device interface?)
	void set_framerate(int rate) { m_framerate = rate; }
	void set_total_scanlines(int total) { m_base_total_scanlines = total; }     // this gets set at start only
	void update_total_scanlines(bool mode3) { m_total_scanlines = mode3 ? (m_base_total_scanlines * 2) : m_base_total_scanlines; }  // this gets set at each EOF

	void SCD_GET_TILE_INFO_16x16_1x1( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_32x32_1x1( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_16x16_16x16( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_32x32_16x16( int& tile_region, int& tileno, int tile_index );

	void get_stampmap_16x16_1x1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_stampmap_32x32_1x1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_stampmap_16x16_16x16_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_stampmap_32x32_16x16_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint8_t get_stampmap_16x16_1x1_tile_info_pixel(int xpos, int ypos);
	uint8_t get_stampmap_32x32_1x1_tile_info_pixel(int xpos, int ypos);
	uint8_t get_stampmap_16x16_16x16_tile_info_pixel(int xpos, int ypos);
	uint8_t get_stampmap_32x32_16x16_tile_info_pixel(int xpos, int ypos);

	void scd_a12000_halt_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t scd_a12000_halt_reset_r(address_space &space, offs_t offset, uint16_t mem_mask);
	uint16_t scd_a12002_memory_mode_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void scd_a12002_memory_mode_w_8_15(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void scd_a12002_memory_mode_w_0_7(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void scd_a12002_memory_mode_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_sub_memory_mode_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_sub_memory_mode_w_8_15(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void segacd_sub_memory_mode_w_0_7(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void segacd_sub_memory_mode_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t segacd_comms_flags_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_comms_flags_subcpu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	void segacd_comms_flags_maincpu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t scd_4m_prgbank_ram_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void scd_4m_prgbank_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_comms_main_part1_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_comms_main_part1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_comms_main_part2_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_comms_main_part2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_comms_sub_part1_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_comms_sub_part1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_comms_sub_part2_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_comms_sub_part2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);



	uint16_t segacd_main_dataram_part1_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_main_dataram_part1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t scd_hint_vector_r(address_space &space, offs_t offset, uint16_t mem_mask);
	uint16_t scd_a12006_hint_register_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void scd_a12006_hint_register_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);


	void segacd_stopwatch_timer_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_stopwatch_timer_r(address_space &space, offs_t offset, uint16_t mem_mask);
	uint16_t segacd_sub_led_ready_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_sub_led_ready_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_sub_dataram_part1_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_sub_dataram_part1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_sub_dataram_part2_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_sub_dataram_part2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);


	uint16_t segacd_stampsize_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_stampsize_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);

	uint8_t read_pixel_from_stampmap(bitmap_ind16* srcbitmap, int x, int y);

	void segacd_trace_vector_base_address_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_imagebuffer_vdot_size_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_imagebuffer_vdot_size_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_stampmap_base_address_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_stampmap_base_address_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_imagebuffer_start_address_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_imagebuffer_start_address_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_imagebuffer_offset_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_imagebuffer_offset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_imagebuffer_vcell_size_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_imagebuffer_vcell_size_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_imagebuffer_hdot_size_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_imagebuffer_hdot_size_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t segacd_irq3timer_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void segacd_irq3timer_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t backupram_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void backupram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t font_color_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void font_color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t font_converted_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void dma_timer_callback(timer_device &timer, void *ptr, int32_t param);
	int segacd_sub_int_callback(device_t &device, int irqline);

	void SegaCD_CDC_Do_DMA( int &dmacount, uint8_t *CDC_BUFFER, uint16_t &dma_addrc, uint16_t &destination );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
//  virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const override;
private:
//  virtual void device_config_complete();

};


class sega_segacd_us_device : public sega_segacd_device
{
	public:
		sega_segacd_us_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	protected:

};

class sega_segacd_japan_device : public sega_segacd_device
{
	public:
		sega_segacd_japan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	protected:
//      virtual machine_config_constructor device_mconfig_additions() const;
};

class sega_segacd_europe_device : public sega_segacd_device
{
	public:
		sega_segacd_europe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	protected:
//      virtual machine_config_constructor device_mconfig_additions() const;
};


extern const device_type SEGA_SEGACD_US;
extern const device_type SEGA_SEGACD_JAPAN;
extern const device_type SEGA_SEGACD_EUROPE;
