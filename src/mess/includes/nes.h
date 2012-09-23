/*****************************************************************************
 *
 * includes/nes.h
 *
 * Nintendo Entertainment System (Famicom)
 *
 ****************************************************************************/

#ifndef NES_H_
#define NES_H_

#include "includes/nes_mmc.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define NTSC_CLOCK           N2A03_DEFAULTCLOCK     /* 1.789772 MHz */
#define PAL_CLOCK	           (26601712.0/16)        /* 1.662607 MHz */

#define NES_BATTERY_SIZE 0x2000


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct nes_input
{
	UINT32 shift;
	UINT32 i0, i1, i2;
};

/*PPU fast banking constants and structures */

#define CHRROM 0
#define CHRRAM 1


/*PPU nametable fast banking constants and structures */

#define CIRAM 0
#define ROM 1
#define EXRAM 2
#define MMC5FILL 3
#define CART_NTRAM 4

#define NES_BATTERY 0
#define NES_WRAM 1


class nes_state : public nes_carts_state
{
public:
	nes_state(const machine_config &mconfig, device_type type, const char *tag)
	: nes_carts_state(mconfig, type, tag) { }

	/* input_related - this part has to be cleaned up (e.g. in_2 and in_3 are not really necessary here...) */
	nes_input m_in_0, m_in_1, m_in_2, m_in_3;
	UINT8 m_fck_scan, m_fck_mode;

	/* video-related */
	int m_nes_vram_sprite[8]; /* Used only by mmc5 for now */
	int m_last_frame_flip;

	void init_nes_core();
	void pcb_handlers_setup();
	int pcb_initialize(int idx);

	DECLARE_WRITE8_MEMBER(nes_chr_w);
	DECLARE_READ8_MEMBER(nes_chr_r);
	DECLARE_WRITE8_MEMBER(nes_nt_w);
	DECLARE_READ8_MEMBER(nes_nt_r);
	DECLARE_WRITE8_MEMBER(nes_low_mapper_w);
	DECLARE_READ8_MEMBER(nes_low_mapper_r);

	/* misc */
	write8_delegate   m_mmc_write_low;
	write8_delegate   m_mmc_write_mid;
	write8_delegate   m_mmc_write;
	read8_delegate    m_mmc_read_low;
	read8_delegate    m_mmc_read_mid;
	read8_delegate    m_mmc_read;

	/* devices */
//  cpu_device        *m_maincpu;
//  ppu2c0x_device    *m_ppu;
//  device_t          *m_sound;
	device_t          *m_cart;
//  emu_timer         *m_irq_timer;

	/***** FDS-floppy related *****/

	int     m_disk_expansion;

	UINT8   m_fds_sides;
	UINT8   *m_fds_data;	// here, we store a copy of the disk
	UINT8   *m_fds_ram;	// here, we emulate the RAM adapter

	/* Variables which can change */
	UINT8   m_fds_motor_on;
	UINT8   m_fds_door_closed;
	UINT8   m_fds_current_side;
	UINT32  m_fds_head_position;
	UINT8   m_fds_status0;
	UINT8   m_fds_read_mode;
	UINT8   m_fds_write_reg;

	/* these are used in the mapper 20 handlers */
	int     m_fds_last_side;
	int     m_fds_count;
	DECLARE_READ8_MEMBER(nes_IN0_r);
	DECLARE_READ8_MEMBER(nes_IN1_r);
	DECLARE_WRITE8_MEMBER(nes_IN0_w);
	DECLARE_WRITE8_MEMBER(nes_IN1_w);
	DECLARE_READ8_MEMBER(nes_fds_r);
	DECLARE_WRITE8_MEMBER(nes_fds_w);
	DECLARE_WRITE8_MEMBER(nes_vh_sprite_dma_w);
	DECLARE_DRIVER_INIT(famicom);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_nes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/*----------- defined in machine/nes.c -----------*/


/* protos */

DEVICE_IMAGE_LOAD(nes_cart);
DEVICE_START(nes_disk);
DEVICE_IMAGE_LOAD(nes_disk);
DEVICE_IMAGE_UNLOAD(nes_disk);

int nes_ppu_vidaccess( device_t *device, int address, int data );

void nes_partialhash(hash_collection &dest, const unsigned char *data, unsigned long length, const char *functions);


#endif /* NES_H_ */
