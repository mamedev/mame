/*****************************************************************************
 *
 * includes/sms.h
 *
 ****************************************************************************/

#ifndef SMS_H_
#define SMS_H_

#define LOG_REG
#define LOG_PAGING
#define LOG_COLOR

#define NVRAM_SIZE             (0x08000)
#define CPU_ADDRESSABLE_SIZE   (0x10000)

#define MAX_CARTRIDGES        16

class sms_state : public driver_device
{
public:
	void map_cart_16k( UINT16 address, UINT16 bank );
	void map_cart_8k( UINT16 address, UINT16 bank );
	void map_bios_16k( UINT16 address, UINT16 bank );
	void map_bios_8k( UINT16 address, UINT16 bank );

public:
	sms_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag) { }

	// device_ts
	device_t *m_main_cpu;
	device_t *m_control_cpu;
	sega315_5124_device *m_vdp;
	eeprom_device *m_eeprom;
	device_t *m_ym;
	device_t *m_main_scr;
	device_t *m_left_lcd;
	device_t *m_right_lcd;
	address_space *m_space;

	UINT8 m_bios_page_count;
	UINT8 m_fm_detect;
	UINT8 m_ctrl_reg;
	int m_paused;
	UINT8 m_bios_port;
	UINT8 *m_BIOS;
	UINT8 *m_mapper_ram;
	UINT8 m_mapper[4];
	// we are going to use 1-6, same as bank numbers. Notice, though, that most mappers
	// only work on 16K banks and, hence, banks 4-6 are not always directly set
	// (they often use bank3 + 0x2000 and bank5 + 0x2000)
	UINT8 *m_banking_bios[8];
	UINT8 *m_banking_cart[8];
	UINT8 *m_banking_none;
	UINT8 m_gg_sio[5];
	UINT8 m_store_control;
	UINT8 m_input_port0;
	UINT8 m_input_port1;

	// for gamegear LCD persistence hack
	bitmap_rgb32 m_prev_bitmap;

	// for 3D glass binocular hack
	bitmap_rgb32 m_prevleft_bitmap;
	bitmap_rgb32 m_prevright_bitmap;

	/* Model identifiers */
	UINT8 m_is_gamegear;
	UINT8 m_is_region_japan;
	UINT8 m_has_bios_0400;
	UINT8 m_has_bios_2000;
	UINT8 m_has_bios_full;
	UINT8 m_has_bios;
	UINT8 m_has_fm;

	/* Data needed for Rapid Fire Unit support */
	emu_timer *m_rapid_fire_timer;
	UINT8 m_rapid_fire_state_1;
	UINT8 m_rapid_fire_state_2;

	/* Data needed for Paddle Control controller */
	UINT32 m_last_paddle_read_time;
	UINT8 m_paddle_read_state;

	/* Data needed for Sports Pad controller */
	UINT32 m_last_sports_pad_time_1;
	UINT32 m_last_sports_pad_time_2;
	UINT8 m_sports_pad_state_1;
	UINT8 m_sports_pad_state_2;
	UINT8 m_sports_pad_last_data_1;
	UINT8 m_sports_pad_last_data_2;
	UINT8 m_sports_pad_1_x;
	UINT8 m_sports_pad_1_y;
	UINT8 m_sports_pad_2_x;
	UINT8 m_sports_pad_2_y;

	/* Data needed for Light Phaser */
	emu_timer *m_lphaser_1_timer;
	emu_timer *m_lphaser_2_timer;
	UINT8 m_lphaser_1_latch;
	UINT8 m_lphaser_2_latch;
	int m_lphaser_x_offs;	/* Needed to 'calibrate' lphaser; set at cart loading */

	/* Data needed for SegaScope (3D glasses) */
	UINT8 m_sscope_state;

	/* Cartridge slot info */
	UINT8 m_current_cartridge;
	struct
	{
		UINT8 *ROM;        /* Pointer to ROM image data */
		UINT32 size;       /* Size of the ROM image */
		UINT32 features;   /* on-cartridge special hardware */
		UINT8 *cartSRAM;   /* on-cartridge SRAM */
		UINT8 sram_save;   /* should be the contents of the on-cartridge SRAM be saved */
		UINT8 *cartRAM;    /* additional on-cartridge RAM (64KB for Ernie Els Golf) */
		UINT32 ram_size;   /* size of the on-cartridge RAM */
		UINT8 ram_page;    /* currently swapped in cartridge RAM */

		/* Data needed for Terebi Oekaki (TV Draw) */
		UINT8 m_tvdraw_data;

		/* Data needed for 4pak mapper */
		UINT8 m_4pak_page0;
		UINT8 m_4pak_page1;
		UINT8 m_4pak_page2;

		/* Data needed for 93c46 */
		bool m_93c46_enabled;
		UINT8 m_93c46_lines;
	} m_cartridge[MAX_CARTRIDGES];
	DECLARE_WRITE8_MEMBER(sms_input_write);
	DECLARE_WRITE8_MEMBER(sms_fm_detect_w);
	DECLARE_READ8_MEMBER(sms_fm_detect_r);
	DECLARE_WRITE8_MEMBER(sms_io_control_w);
	DECLARE_READ8_MEMBER(sms_count_r);
	DECLARE_READ8_MEMBER(sms_input_port_0_r);
	DECLARE_READ8_MEMBER(sms_input_port_1_r);
	DECLARE_WRITE8_MEMBER(sms_ym2413_register_port_0_w);
	DECLARE_WRITE8_MEMBER(sms_ym2413_data_port_0_w);
	DECLARE_READ8_MEMBER(gg_input_port_2_r);
	DECLARE_READ8_MEMBER(sms_sscope_r);
	DECLARE_WRITE8_MEMBER(sms_sscope_w);
	DECLARE_READ8_MEMBER(sms_mapper_r);
	DECLARE_WRITE8_MEMBER(sms_tvdraw_axis_w);
	DECLARE_READ8_MEMBER(sms_tvdraw_status_r);
	DECLARE_READ8_MEMBER(sms_tvdraw_data_r);
	DECLARE_WRITE8_MEMBER(sms_93c46_w);
	DECLARE_READ8_MEMBER(sms_93c46_r);
	DECLARE_WRITE8_MEMBER(sms_mapper_w);
	DECLARE_WRITE8_MEMBER(sms_korean_zemina_banksw_w);
	DECLARE_WRITE8_MEMBER(sms_codemasters_page0_w);
	DECLARE_WRITE8_MEMBER(sms_codemasters_page1_w);
	DECLARE_WRITE8_MEMBER(sms_4pak_page0_w);
	DECLARE_WRITE8_MEMBER(sms_4pak_page1_w);
	DECLARE_WRITE8_MEMBER(sms_4pak_page2_w);
	DECLARE_WRITE8_MEMBER(sms_janggun_bank0_w);
	DECLARE_WRITE8_MEMBER(sms_janggun_bank1_w);
	DECLARE_WRITE8_MEMBER(sms_janggun_bank2_w);
	DECLARE_WRITE8_MEMBER(sms_janggun_bank3_w);
	DECLARE_WRITE8_MEMBER(sms_bios_w);
	DECLARE_WRITE8_MEMBER(sms_cartram2_w);
	DECLARE_WRITE8_MEMBER(sms_cartram_w);
	DECLARE_WRITE8_MEMBER(gg_sio_w);
	DECLARE_READ8_MEMBER(gg_sio_r);
	DECLARE_READ8_MEMBER(sms_store_cart_select_r);
	DECLARE_WRITE8_MEMBER(sms_store_cart_select_w);
	DECLARE_READ8_MEMBER(sms_store_select1);
	DECLARE_READ8_MEMBER(sms_store_select2);
	DECLARE_READ8_MEMBER(sms_store_control_r);
	DECLARE_WRITE8_MEMBER(sms_store_control_w);
	DECLARE_DRIVER_INIT(sg1000m3);
	DECLARE_DRIVER_INIT(gamegear);
	DECLARE_DRIVER_INIT(gamegeaj);
	DECLARE_DRIVER_INIT(sms2kr);
	DECLARE_DRIVER_INIT(smsj);
	DECLARE_DRIVER_INIT(sms1);
	DECLARE_DRIVER_INIT(smssdisp);
};


/*----------- defined in machine/sms.c -----------*/

/* Function prototypes */

INPUT_CHANGED( lgun1_changed );
INPUT_CHANGED( lgun2_changed );

WRITE_LINE_DEVICE_HANDLER( sms_pause_callback );
WRITE_LINE_DEVICE_HANDLER( sms_store_int_callback );

DEVICE_START( sms_cart );
DEVICE_IMAGE_LOAD( sms_cart );

MACHINE_START( sms );
MACHINE_RESET( sms );


#define IO_EXPANSION    (0x80)	/* Expansion slot enable (1= disabled, 0= enabled) */
#define IO_CARTRIDGE    (0x40)	/* Cartridge slot enable (1= disabled, 0= enabled) */
#define IO_CARD         (0x20)	/* Card slot disabled (1= disabled, 0= enabled) */
#define IO_WORK_RAM     (0x10)	/* Work RAM disabled (1= disabled, 0= enabled) */
#define IO_BIOS_ROM     (0x08)	/* BIOS ROM disabled (1= disabled, 0= enabled) */
#define IO_CHIP         (0x04)	/* I/O chip disabled (1= disabled, 0= enabled) */



VIDEO_START( sms1 );
VIDEO_START( gamegear );
SCREEN_UPDATE_RGB32( sms1 );
SCREEN_UPDATE_RGB32( sms );
SCREEN_UPDATE_RGB32( gamegear );

#endif /* SMS_H_ */
