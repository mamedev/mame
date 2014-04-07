/* Sega CD / Mega CD */

#include "cpu/m68000/m68000.h"
#include "machine/lc89510.h"
#include "machine/megacdcd.h"

#define SEGACD_CLOCK      12500000

#define RAM_MODE_2MEG (0)
#define RAM_MODE_1MEG (2)

#define DMA_PCM  (0x0400)
#define DMA_PRG  (0x0500)
#define DMA_WRAM (0x0700)

// irq3 timer
#define CHECK_SCD_LV3_INTERRUPT \
	if (lc89510_temp->get_segacd_irq_mask() & 0x08) \
	{ \
		m_scdcpu->set_input_line(3, HOLD_LINE); \
	}
// from master
#define CHECK_SCD_LV2_INTERRUPT \
	if (lc89510_temp->get_segacd_irq_mask() & 0x04) \
	{ \
		m_scdcpu->set_input_line(2, HOLD_LINE); \
	}

// gfx convert
#define CHECK_SCD_LV1_INTERRUPT \
	if (lc89510_temp->get_segacd_irq_mask() & 0x02) \
	{ \
		m_scdcpu->set_input_line(1, HOLD_LINE); \
	}

#define SEGACD_IRQ3_TIMER_SPEED (attotime::from_nsec(segacd_irq3_timer_reg*30720))


class sega_segacd_device : public device_t
{
public:
	sega_segacd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	required_device<cpu_device> m_scdcpu;
	lc89510_temp_device *lc89510_temp;

	UINT16 *segacd_backupram;
	timer_device *stopwatch_timer;
	UINT8 segacd_font_color;
	UINT16* segacd_font_bits;
	UINT16 scd_rammode;
	UINT32 scd_mode_dmna_ret_flags ;

	timer_device *segacd_gfx_conversion_timer;
	timer_device *segacd_irq3_timer;
	//timer_device *segacd_hock_timer;

	UINT16* segacd_4meg_prgram;  // pointer to SubCPU PrgRAM
	UINT16* segacd_dataram;
	UINT16* segacd_dataram2;
	tilemap_t    *segacd_stampmap[4];


	UINT8 segacd_ram_writeprotect_bits;
	int segacd_4meg_prgbank;// = 0; // which bank the MainCPU can see of the SubCPU PrgRAM
	int segacd_memory_priority_mode;// = 0;
	int segacd_stampsize;

	UINT16 segacd_hint_register;
	UINT16 segacd_imagebuffer_vdot_size;
	UINT16 segacd_imagebuffer_vcell_size;
	UINT16 segacd_imagebuffer_hdot_size;

	int segacd_conversion_active;// = 0;
	UINT16 segacd_stampmap_base_address;
	UINT16 segacd_imagebuffer_start_address;
	UINT16 segacd_imagebuffer_offset;


	UINT16 segacd_comms_flags;// = 0x0000;
	UINT16 segacd_comms_part1[0x8];
	UINT16 segacd_comms_part2[0x8];

	int segacd_redled;// = 0;
	int segacd_greenled;// = 0;
	int segacd_ready;// = 1; // actually set 100ms after startup?
	UINT16 segacd_irq3_timer_reg;


	TIMER_DEVICE_CALLBACK_MEMBER( segacd_irq3_timer_callback );
	TIMER_DEVICE_CALLBACK_MEMBER( segacd_gfx_conversion_timer_callback );

	UINT16 handle_segacd_sub_int_callback(int irqline);

	inline void write_pixel(running_machine& machine, UINT8 pix, int pixeloffset );
	UINT16 segacd_1meg_mode_word_read(int offset, UINT16 mem_mask);
	void segacd_1meg_mode_word_write(int offset, UINT16 data, UINT16 mem_mask, int use_pm);

	DECLARE_READ16_MEMBER( segacd_dmaaddr_r );
	DECLARE_WRITE16_MEMBER( segacd_dmaaddr_w );
	UINT16 m_dmaaddr;

	UINT16 m_a12000_halt_reset_reg;

	int m_framerate;
	int m_base_total_scanlines;
	int m_total_scanlines;

	void segacd_mark_tiles_dirty(running_machine& machine, int offset);
	int segacd_get_active_stampmap_tilemap(void);

	// set some variables at start, depending on region (shall be moved to a device interface?)
	void set_framerate(int rate) { m_framerate = rate; }
	void set_total_scanlines(int total) { m_base_total_scanlines = total; }     // this gets set at start only
	void update_total_scanlines(bool mode3) { m_total_scanlines = mode3 ? (m_base_total_scanlines * 2) : m_base_total_scanlines; }  // this gets set at each EOF

	void SCD_GET_TILE_INFO_16x16_1x1( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_32x32_1x1( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_16x16_16x16( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_32x32_16x16( int& tile_region, int& tileno, int tile_index );

	TILE_GET_INFO_MEMBER( get_stampmap_16x16_1x1_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_32x32_1x1_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_16x16_16x16_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_32x32_16x16_tile_info );

	UINT8 get_stampmap_16x16_1x1_tile_info_pixel(running_machine& machine, int xpos, int ypos);
	UINT8 get_stampmap_32x32_1x1_tile_info_pixel(running_machine& machine, int xpos, int ypos);
	UINT8 get_stampmap_16x16_16x16_tile_info_pixel(running_machine& machine, int xpos, int ypos);
	UINT8 get_stampmap_32x32_16x16_tile_info_pixel(running_machine& machine, int xpos, int ypos);

	WRITE16_MEMBER( scd_a12000_halt_reset_w );
	READ16_MEMBER( scd_a12000_halt_reset_r );
	READ16_MEMBER( scd_a12002_memory_mode_r );
	WRITE8_MEMBER( scd_a12002_memory_mode_w_8_15 );
	WRITE8_MEMBER( scd_a12002_memory_mode_w_0_7 );
	WRITE16_MEMBER( scd_a12002_memory_mode_w );
	READ16_MEMBER( segacd_sub_memory_mode_r );
	WRITE8_MEMBER( segacd_sub_memory_mode_w_8_15 );
	WRITE8_MEMBER( segacd_sub_memory_mode_w_0_7 );
	WRITE16_MEMBER( segacd_sub_memory_mode_w );

	READ16_MEMBER( segacd_comms_flags_r );
	WRITE16_MEMBER( segacd_comms_flags_subcpu_w );
	WRITE16_MEMBER( segacd_comms_flags_maincpu_w );
	READ16_MEMBER( scd_4m_prgbank_ram_r );
	WRITE16_MEMBER( scd_4m_prgbank_ram_w );
	READ16_MEMBER( segacd_comms_main_part1_r );
	WRITE16_MEMBER( segacd_comms_main_part1_w );
	READ16_MEMBER( segacd_comms_main_part2_r );
	WRITE16_MEMBER( segacd_comms_main_part2_w );
	READ16_MEMBER( segacd_comms_sub_part1_r );
	WRITE16_MEMBER( segacd_comms_sub_part1_w );
	READ16_MEMBER( segacd_comms_sub_part2_r );
	WRITE16_MEMBER( segacd_comms_sub_part2_w );



	READ16_MEMBER( segacd_main_dataram_part1_r );
	WRITE16_MEMBER( segacd_main_dataram_part1_w );

	READ16_MEMBER( scd_hint_vector_r );
	READ16_MEMBER( scd_a12006_hint_register_r );
	WRITE16_MEMBER( scd_a12006_hint_register_w );


	WRITE16_MEMBER( segacd_stopwatch_timer_w );
	READ16_MEMBER( segacd_stopwatch_timer_r );
	READ16_MEMBER( segacd_sub_led_ready_r );
	WRITE16_MEMBER( segacd_sub_led_ready_w );
	READ16_MEMBER( segacd_sub_dataram_part1_r );
	WRITE16_MEMBER( segacd_sub_dataram_part1_w );
	READ16_MEMBER( segacd_sub_dataram_part2_r );
	WRITE16_MEMBER( segacd_sub_dataram_part2_w );


	READ16_MEMBER( segacd_stampsize_r );
	WRITE16_MEMBER( segacd_stampsize_w );

	UINT8 read_pixel_from_stampmap( running_machine& machine, bitmap_ind16* srcbitmap, int x, int y);

	WRITE16_MEMBER( segacd_trace_vector_base_address_w );
	READ16_MEMBER( segacd_imagebuffer_vdot_size_r );
	WRITE16_MEMBER( segacd_imagebuffer_vdot_size_w );
	READ16_MEMBER( segacd_stampmap_base_address_r );
	WRITE16_MEMBER( segacd_stampmap_base_address_w );
	READ16_MEMBER( segacd_imagebuffer_start_address_r );
	WRITE16_MEMBER( segacd_imagebuffer_start_address_w );
	READ16_MEMBER( segacd_imagebuffer_offset_r );
	WRITE16_MEMBER( segacd_imagebuffer_offset_w );
	READ16_MEMBER( segacd_imagebuffer_vcell_size_r );
	WRITE16_MEMBER( segacd_imagebuffer_vcell_size_w );
	READ16_MEMBER( segacd_imagebuffer_hdot_size_r );
	WRITE16_MEMBER( segacd_imagebuffer_hdot_size_w );
	READ16_MEMBER( segacd_irq3timer_r );
	WRITE16_MEMBER( segacd_irq3timer_w );
	READ16_MEMBER( segacd_backupram_r );
	WRITE16_MEMBER( segacd_backupram_w );
	READ16_MEMBER( segacd_font_color_r );
	WRITE16_MEMBER( segacd_font_color_w );
	READ16_MEMBER( segacd_font_converted_r );
	TIMER_DEVICE_CALLBACK_MEMBER( scd_dma_timer_callback );
	IRQ_CALLBACK_MEMBER(segacd_sub_int_callback);

	void SegaCD_CDC_Do_DMA( int &dmacount, UINT8 *CDC_BUFFER, UINT16 &dma_addrc, UINT16 &destination );
	timer_device* scd_dma_timer;
	required_device<gfxdecode_device> m_gfxdecode;

protected:
	virtual void device_start();
	virtual void device_reset();

	// optional information overrides
//  virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
private:
//  virtual void device_config_complete();

};


class sega_segacd_us_device : public sega_segacd_device
{
	public:
		sega_segacd_us_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	protected:

};

class sega_segacd_japan_device : public sega_segacd_device
{
	public:
		sega_segacd_japan_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	protected:
//      virtual machine_config_constructor device_mconfig_additions() const;
};

class sega_segacd_europe_device : public sega_segacd_device
{
	public:
		sega_segacd_europe_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	protected:
//      virtual machine_config_constructor device_mconfig_additions() const;
};


extern const device_type SEGA_SEGACD_US;
extern const device_type SEGA_SEGACD_JAPAN;
extern const device_type SEGA_SEGACD_EUROPE;
