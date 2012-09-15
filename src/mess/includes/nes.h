/*****************************************************************************
 *
 * includes/nes.h
 *
 * Nintendo Entertainment System (Famicom)
 *
 ****************************************************************************/

#ifndef NES_H_
#define NES_H_


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

struct chr_bank 
{
	int source;	//defines source of base pointer
	int origin; //defines offset of 0x400 byte segment at base pointer
	UINT8* access;	//source translated + origin -> valid pointer!
};

/*PPU nametable fast banking constants and structures */

#define CIRAM 0
#define ROM 1
#define EXRAM 2
#define MMC5FILL 3
#define CART_NTRAM 4

#define NES_BATTERY 0
#define NES_WRAM 1

struct name_table 
{
	int source;		/* defines source of base pointer */
	int origin;		/* defines offset of 0x400 byte segment at base pointer */
	int writable;	/* ExRAM, at least, can be write-protected AND used as nametable */
	UINT8* access;	/* direct access when possible */
};

typedef void (*nes_prg_callback)(running_machine &machine, int start, int bank);
typedef void (*nes_chr_callback)(running_machine &machine, int start, int bank, int source);

class nes_state : public driver_device
{
public:
	nes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* input_related - this part has to be cleaned up (e.g. in_2 and in_3 are not really necessary here...) */
	nes_input m_in_0, m_in_1, m_in_2, m_in_3;
	UINT8 m_fck_scan, m_fck_mode;

	int           m_prg_bank[5];
	chr_bank      m_chr_map[8];  //quick banking structure, because some of this changes multiple times per scanline!
	name_table    m_nt_page[4];  //quick banking structure for a maximum of 4K of RAM/ROM/ExRAM

	int m_chr_open_bus;
	int m_prgram_bank5_start, m_battery_bank5_start, m_empty_bank5_start;

	UINT8 m_ce_mask, m_ce_state;
	UINT8 m_vrc_ls_prg_a, m_vrc_ls_prg_b, m_vrc_ls_chr;

	int m_MMC5_floodtile;
	int m_MMC5_floodattr;
	int m_mmc5_vram_control;
	UINT8 m_mmc5_high_chr;
	UINT8 m_mmc5_split_scr;
	UINT8 *m_extended_ntram;

	UINT8 m_mmc5_last_chr_a;
	UINT16 m_mmc5_vrom_regA[8];
	UINT16 m_mmc5_vrom_regB[4];
	UINT8 m_mmc5_prg_regs[4];
	UINT8 m_mmc5_bank_security;
	UINT8 m_mmc5_prg_mode;
	UINT8 m_mmc5_chr_mode;
	UINT8 m_mmc5_chr_high;
	UINT8 m_mmc5_split_ctrl;
	UINT8 m_mmc5_split_yst;
	UINT8 m_mmc5_split_bank;

	/* video-related */
	int m_nes_vram_sprite[8]; /* Used only by mmc5 for now */
	int m_last_frame_flip;

	/* misc */
	write8_space_func   m_mmc_write_low;
	const char  		*m_mmc_write_low_name;
	write8_space_func   m_mmc_write_mid;
	const char  		*m_mmc_write_mid_name;
	write8_space_func   m_mmc_write;
	const char  		*m_mmc_write_name;
	read8_space_func    m_mmc_read_low;
	const char  		*m_mmc_read_low_name;
	read8_space_func    m_mmc_read_mid;
	const char  		*m_mmc_read_mid_name;
	read8_space_func    m_mmc_read;
	const char  		*m_mmc_read_name;
	emu_timer	        *m_irq_timer;

	nes_prg_callback    m_mmc3_prg_cb;	// these are used to simplify a lot emulation of some MMC3 pirate clones
	nes_chr_callback    m_mmc3_chr_cb;

	/* devices */
	cpu_device      *m_maincpu;
	ppu2c0x_device      *m_ppu;
	device_t      *m_sound;
	device_t      *m_cart;

	/* misc region to be allocated at init */
	// variables which don't change at run-time
	UINT8      *m_rom;
	UINT8      *m_prg;
	UINT8      *m_vrom;
	UINT8      *m_vram;
	UINT8      *m_wram;
	UINT8      *m_ciram; //PPU nametable RAM - external to PPU!
	UINT8      *m_battery_ram;
	UINT8      *m_mapper_ram;
	UINT8      *m_mapper_bram;

	/***** Mapper-related variables *****/

	// common ones
	int        m_IRQ_enable, m_IRQ_enable_latch;
	UINT16     m_IRQ_count, m_IRQ_count_latch;
	UINT8      m_IRQ_toggle;
	UINT8      m_IRQ_reset;
	UINT8      m_IRQ_status;
	UINT8      m_IRQ_mode;
	UINT8      m_IRQ_clear;
	int        m_mult1, m_mult2;

	UINT8 m_mmc_chr_source;			// This is set at init to CHRROM or CHRRAM. a few mappers can swap between
									// the two (this is done in the specific handlers).

	UINT8 m_mmc_cmd1, m_mmc_cmd2;		// These represent registers where the mapper writes important values
	UINT8 m_mmc_count;				// This is used as counter in mappers like 1 and 45

	int m_mmc_prg_base, m_mmc_prg_mask;	// MMC3 based multigame carts select a block of banks by using these (and then act like normal MMC3),
	int m_mmc_chr_base, m_mmc_chr_mask;	// while MMC3 and clones (mapper 118 & 119) simply set them as 0 and 0xff resp.

	UINT8 m_mmc_prg_bank[6];				// Many mappers writes only some bits of the selected bank (for both PRG and CHR),
	UINT8 m_mmc_vrom_bank[16];			// hence these are handy to latch bank values.

	UINT16 m_MMC5_vrom_bank[12];			// MMC5 has 10bit wide VROM regs!
	UINT8 m_mmc_extra_bank[16];			// some MMC3 clone have 2 series of PRG/CHR banks...
										// we collect them all here: first 4 elements PRG banks, then 6/8 CHR banks

	UINT8 m_mmc_latch1, m_mmc_latch2;
	UINT8 m_mmc_reg[16];

	UINT8 m_mmc_dipsetting;

	// misc mapper related variables which should be merged with the above one, where possible
	int m_mmc1_reg_write_enable;
	int m_mmc1_latch;
	int m_mmc1_count;

	int m_mmc3_latch;
	int m_mmc3_wram_protect;
	int m_mmc3_alt_irq;

	int m_MMC5_rom_bank_mode;
	int m_MMC5_vrom_bank_mode;
	int m_MMC5_vram_protect;
	int m_MMC5_scanline;
	int m_vrom_page_a;
	int m_vrom_page_b;
	// int vrom_next[4];

	UINT8 m_mmc6_reg;

	// these might be unified in single mmc_reg[] array, together with state->m_mmc_cmd1 & state->m_mmc_cmd2
	// but be careful that MMC3 clones often use state->m_mmc_cmd1/state->m_mmc_cmd2 (from base MMC3) AND additional regs below!
	UINT8 m_mapper83_reg[10];
	UINT8 m_mapper83_low_reg[4];
	UINT8 m_txc_reg[4];	// used by mappers 132, 172 & 173
	UINT8 m_subor_reg[4];	// used by mappers 166 & 167
	UINT8 m_sachen_reg[8];	// used by mappers 137, 138, 139 & 141
	UINT8 m_map52_reg_written;
	UINT8 m_map114_reg, m_map114_reg_enabled;

	/***** NES-cart related *****/

	/* load-time cart variables which remain constant */
	UINT16 m_prg_chunks;		// iNES 2.0 allows for more chunks (a recently dumped multigame cart has 256 chunks of both PRG & CHR!)
	UINT16 m_chr_chunks;
	UINT16 m_vram_chunks;
	UINT8 m_trainer;
	UINT8 m_battery;			// if there is PRG RAM with battery backup
	UINT32 m_battery_size;
	UINT8 m_prg_ram;			// if there is PRG RAM with no backup
	UINT32 m_wram_size;
	UINT32 m_mapper_ram_size;
	UINT32 m_mapper_bram_size;

	/* system variables which don't change at run-time */
	UINT16 m_mapper;		// for iNES
	UINT16 m_pcb_id;		// for UNIF & xml
	UINT8 m_four_screen_vram;
	UINT8 m_hard_mirroring;
	UINT8 m_crc_hack;	// this is needed to detect different boards sharing the same Mappers (shame on .nes format)
	UINT8 m_ines20;

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
};


/*----------- defined in machine/nes.c -----------*/


/* protos */

DEVICE_IMAGE_LOAD(nes_cart);
DEVICE_START(nes_disk);
DEVICE_IMAGE_LOAD(nes_disk);
DEVICE_IMAGE_UNLOAD(nes_disk);






int nes_ppu_vidaccess( device_t *device, int address, int data );

void nes_partialhash(hash_collection &dest, const unsigned char *data, unsigned long length, const char *functions);

/*----------- defined in video/nes.c -----------*/



SCREEN_UPDATE_IND16( nes );


#endif /* NES_H_ */
