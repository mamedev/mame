/***************************************************************************

    Midway DCS Audio Board

****************************************************************************

    There are several variations of this board, which was in use by
    Midway and eventually Atari for almost 10 years.

    DCS ROM-based mono:
        * ADSP-2105 @ 10MHz
        * single channel output
        * 2k external shared program/data RAM
        * ROM-based, up to 8MB total
        * used in:
            Mortal Kombat 2 (1993)
            Cruisin' USA (1994)
            Revolution X (1994)
            Killer Instinct (1994)
            Killer Instinct 2 (1995)
            Cruisin' World (1996)
            Offroad Challenge (1997)

        * 8k external shared program/data RAM
        * used in:
            Mortal Kombat 3 (1994)
            Ultimate Mortal Kombat 3 (1994)
            2 On 2 Open Ice Challenge (1995)
            WWF Wrestlemania (1995)
            NBA Hangtime (1996)
            NBA Maximum Hangtime (1996)
            Rampage World Tour (1997)

    DCS2 RAM-based stereo (Seattle):
        * ADSP-2115 @ 16MHz
        * dual channel output (stereo)
        * SDRC ASIC for RAM/ROM access
        * RAM-based, 2MB total
        * used in:
            War Gods (1995)
            Wayne Gretzky's 3D Hockey (1996)
            Mace: The Dark Age (1996)
            Biofreaks (1997)
            NFL Blitz (1997)
            California Speed (1998)
            Vapor TRX (1998)
            NFL Blitz '99 (1998)
            CarnEvil (1998)
            Hyperdrive (1998)
            NFL Blitz 2000 Gold (1999)

    DCS2 ROM-based stereo (Zeus):
        * ADSP-2104 @ 16MHz
        * dual channel output (stereo)
        * SDRC ASIC for RAM/ROM access
        * ROM-based, up to 16MB total
        * used in:
            Mortal Kombat 4 (1997)
            Invasion (1999)
            Cruisin' Exotica (1999)
            The Grid (2001)

    DCS2 RAM-based stereo (Vegas):
        * ADSP-2104 @ 16MHz
        * dual channel output (stereo)
        * SDRC ASIC for RAM/ROM access
        * RAM-based, 4MB total
        * used in:
            Gauntlet Legends (1998)
            Tenth Degree (1998)
            Gauntlet Dark Legacy (1999)
            War: The Final Assault (1999)

    DCS2 RAM-based stereo (DSIO):
        * ADSP-2181 @ 16.667MHz
        * dual channel output (stereo)
        * custom ASIC for RAM/ROM access
        * RAM-based, 4MB total
        * used in:
            Road Burners (1999)

    DCS2 RAM-based multi-channel (Denver):
        * ADSP-2181 @ 16.667MHz
        * 2-6 channel output
        * custom ASIC for RAM/ROM access
        * RAM-based, 4MB total
        * used in:
            San Francisco Rush: 2049 (1998)

    Unknown other DCS boards:
        * NBA Showtime
        * NBA Showtime / NFL Blitz 2000 Gold
        * Cart Fury

*****************************************************************************

    SDRC (Sound DRAM Control) ASIC
        * Boot ROM = 32k x 8
        * Data ROM = Up to 16MB ROM (4 chip selects)
        * SRAM = 32k x 24 or 8k x 24
            * common map:
                 PGM 0800-0fff -> RAM 4800-4fff
                 PGM 1000-1fff -> RAM 5000-5fff
                 PGM 2000-2fff -> RAM 6000-6fff
                 PGM 3000-3fff -> RAM 7000-7fff
            * bank = 0:
                DATA 0800-0fff -> RAM 0800-0fff
                DATA 1000-17ff -> RAM 0000-07ff
                DATA 1800-1fff -> RAM 1800-1fff
                DATA 2000-27ff -> RAM 1000-17ff
                DATA 2800-2fff -> RAM 2800-2fff
                DATA 3000-37ff -> RAM 2000-27ff
            * bank = 1:
                DATA 0800-0fff -> unmapped
                DATA 1000-17ff -> unmapped
                DATA 1800-1fff -> RAM 3800-3fff
                DATA 2000-27ff -> RAM 3000-37ff
                DATA 2800-2fff -> RAM 2800-2fff
                DATA 3000-37ff -> RAM 2000-27ff

    0480 (reset = XXX0 0X00 0X00 XX00)
        15:13 = SMODE (write only)
          12  = SM_BK (SRAM bank: 0 or 1)
          11  = SM_EN (SRAM enable: 0=disabled, 1=enabled)
         9:7  = ROM_PG (ROM page select: 0-7)
          5   = ROM_MS (ROM memory select: 0=boot memory, 1=data memory)
          4   = ROM_SZ (ROM area size: 0=4k words, 1=1k words)
         1:0  = ROM_ST (ROM memory start: 0=0000, 1=3000, 2=3400, 3=none)

    0481 (reset = 000X 00X0 0X00 XX00)
          15  = AREF_ACT (read only, 1=DRAM auto refresh in progress)
          14  = /MUTE (mute output)
          13  = /LED (LED output)
        11:10 = /RES_TFS (Reset TFS outputs: low bit = channel 1&2, high = channel 3&4)
          8   = TFS_INV (TFS output polarity: 0=same, 1=inverted)
          7   = DM_3WS (DRAM wait states: 0=2, 1=3)
         5:4  = DM_REF (DRAM refresh: 0=disabled, 1=manual, 2=auto, 3=auto 2x)
         1:0  = DM_ST (DRAM memory start: 0=none, 1=0000, 2=3000, 3=3400)

    0482 (reset = XXX0 0000 0000 0000)
        10:0  = DM_PG[10..0] (DRAM page)
        12:0  = EPM_PG[12..0] (EPROM page [low 10 bits used for 4k pages])

    0483 (reset = 1010 0000 1000 0001)
        15:8  = SDRC_ID[7..0] (revision: 5A = ASIC version, A0 = FPGA version)
          7   = SEC_D7
          6   = SEC_D[6..1]
          0   = SEC_D0

****************************************************************************/

#include "emu.h"
#include "cpu/adsp2100/adsp2100.h"
#include "dcs.h"
#include "sound/dmadac.h"
#include "machine/midwayic.h"


#define LOG_DCS_TRANSFERS			(0)
#define LOG_DCS_IO					(0)
#define LOG_BUFFER_FILLING			(0)

#define ENABLE_HLE_TRANSFERS		(1)



/*************************************
 *
 *  Constants
 *
 *************************************/

#define LCTRL_OUTPUT_EMPTY			0x400
#define LCTRL_INPUT_EMPTY			0x800

#define IS_OUTPUT_EMPTY()			(dcs.latch_control & LCTRL_OUTPUT_EMPTY)
#define IS_OUTPUT_FULL()			(!(dcs.latch_control & LCTRL_OUTPUT_EMPTY))
#define SET_OUTPUT_EMPTY()			(dcs.latch_control |= LCTRL_OUTPUT_EMPTY)
#define SET_OUTPUT_FULL()			(dcs.latch_control &= ~LCTRL_OUTPUT_EMPTY)

#define IS_INPUT_EMPTY()			(dcs.latch_control & LCTRL_INPUT_EMPTY)
#define IS_INPUT_FULL()				(!(dcs.latch_control & LCTRL_INPUT_EMPTY))
#define SET_INPUT_EMPTY()			(dcs.latch_control |= LCTRL_INPUT_EMPTY)
#define SET_INPUT_FULL()			(dcs.latch_control &= ~LCTRL_INPUT_EMPTY)


/* These are the some of the control register, we dont use them all */
enum
{
	IDMA_CONTROL_REG = 0,	/* 3fe0 */
	BDMA_INT_ADDR_REG,		/* 3fe1 */
	BDMA_EXT_ADDR_REG,		/* 3fe2 */
	BDMA_CONTROL_REG,		/* 3fe3 */
	BDMA_WORD_COUNT_REG,	/* 3fe4 */
	PROG_FLAG_DATA_REG,		/* 3fe5 */
	PROG_FLAG_CONTROL_REG,	/* 3fe6 */

	S1_AUTOBUF_REG = 15,	/* 3fef */
	S1_RFSDIV_REG,			/* 3ff0 */
	S1_SCLKDIV_REG,			/* 3ff1 */
	S1_CONTROL_REG,			/* 3ff2 */
	S0_AUTOBUF_REG,			/* 3ff3 */
	S0_RFSDIV_REG,			/* 3ff4 */
	S0_SCLKDIV_REG,			/* 3ff5 */
	S0_CONTROL_REG,			/* 3ff6 */
	S0_MCTXLO_REG,			/* 3ff7 */
	S0_MCTXHI_REG,			/* 3ff8 */
	S0_MCRXLO_REG,			/* 3ff9 */
	S0_MCRXHI_REG,			/* 3ffa */
	TIMER_SCALE_REG,		/* 3ffb */
	TIMER_COUNT_REG,		/* 3ffc */
	TIMER_PERIOD_REG,		/* 3ffd */
	WAITSTATES_REG,			/* 3ffe */
	SYSCONTROL_REG			/* 3fff */
};


/* these macros are used to reference the SDRC ASIC */
#define SDRC_ROM_ST		((dcs.sdrc.reg[0] >> 0) & 3)	/* 0=0000, 1=3000, 2=3400, 3=none */
#define SDRC_ROM_SZ		((dcs.sdrc.reg[0] >> 4) & 1)	/* 0=4k, 1=1k */
#define SDRC_ROM_MS		((dcs.sdrc.reg[0] >> 5) & 1)	/* 0=/BMS, 1=/DMS */
#define SDRC_ROM_PG		((dcs.sdrc.reg[0] >> 7) & 7)
#define SDRC_SM_EN		((dcs.sdrc.reg[0] >> 11) & 1)
#define SDRC_SM_BK		((dcs.sdrc.reg[0] >> 12) & 1)
#define SDRC_SMODE		((dcs.sdrc.reg[0] >> 13) & 7)

#define SDRC_DM_ST		((dcs.sdrc.reg[1] >> 0) & 3)	/* 0=none, 1=0000, 2=3000, 3=3400 */
#define SDRC_DM_REF		((dcs.sdrc.reg[1] >> 4) & 3)
#define SDRC_DM_3WS		((dcs.sdrc.reg[1] >> 7) & 1)
#define SDRC_TFS_INV	((dcs.sdrc.reg[1] >> 8) & 1)
#define SDRC_RES_TFS	((dcs.sdrc.reg[1] >> 10) & 3)
#define SDRC_LED		((dcs.sdrc.reg[1] >> 13) & 1)
#define SDRC_MUTE		((dcs.sdrc.reg[1] >> 14) & 1)
#define SDRC_AREF_ACT	((dcs.sdrc.reg[1] >> 15) & 1)

#define SDRC_DM_PG		((dcs.sdrc.reg[2] >> 0) & 0x7ff)
#define SDRC_EPM_PG		((dcs.sdrc.reg[2] >> 0) & 0x1fff)


/* these macros are used to reference the DSIO ASIC */
#define DSIO_EMPTY_FIFO	((dcs.dsio.reg[1] >> 0) & 1)
#define DSIO_CUR_OUTPUT	((dcs.dsio.reg[1] >> 4) & 1)
#define DSIO_RES_TFS	((dcs.dsio.reg[1] >> 10) & 1)
#define DSIO_LED		((dcs.dsio.reg[1] >> 13) & 1)
#define DSIO_MUTE		((dcs.dsio.reg[1] >> 14) & 1)

#define DSIO_DM_PG		((dcs.dsio.reg[2] >> 0) & 0x7ff)


/* these macros are used to reference the DENVER ASIC */
#define DENV_DSP_SPEED	((dcs.dsio.reg[1] >> 2) & 3)	/* read only: 1=33.33MHz */
#define DENV_RES_TFS	((dcs.dsio.reg[1] >> 10) & 1)
#define DENV_CHANNELS	((dcs.dsio.reg[1] >> 11) & 3)	/* 0=2ch, 1=4ch, 2=6ch */
#define DENV_LED		((dcs.dsio.reg[1] >> 13) & 1)
#define DENV_MUTE		((dcs.dsio.reg[1] >> 14) & 1)

#define DENV_DM_PG		((dcs.dsio.reg[2] >> 0) & 0x7ff)



/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef struct _sdrc_state sdrc_state;
struct _sdrc_state
{
	UINT16		reg[4];
	UINT8		seed;
};


typedef struct _dsio_denver_state dsio_state;
struct _dsio_denver_state
{
	UINT16		reg[4];
	UINT8		start_on_next_write;
	UINT16		channelbits;
};


typedef struct _hle_transfer_state hle_transfer_state;
struct _hle_transfer_state
{
	UINT8		hle_enabled;
	INT32		dcs_state;
	INT32		state;
	INT32		start;
	INT32		stop;
	INT32		type;
	INT32		temp;
	INT32		writes_left;
	UINT16		sum;
	INT32		fifo_entries;
	timer_device *watchdog;
};


typedef struct _dcs_state dcs_state;
struct _dcs_state
{
	adsp21xx_device *cpu;
	address_space *program;
	address_space *data;
	UINT8		rev;
	offs_t		polling_offset;
	UINT32		polling_count;

	/* sound output */
	UINT8		channels;
	UINT16		size;
	UINT16		incs;
	dmadac_sound_device *dmadac[6];
	timer_device *reg_timer;
	timer_device *sport_timer;
	timer_device *internal_timer;
	INT32		ireg;
	UINT16		ireg_base;
	UINT16		control_regs[32];

	/* memory access/booting */
	UINT16 *	bootrom;
	UINT32		bootrom_words;
	UINT16 *	sounddata;
	UINT32		sounddata_words;
	UINT32		sounddata_banks;
	UINT16		sounddata_bank;

	/* I/O with the host */
	UINT8		auto_ack;
	UINT16		latch_control;
	UINT16		input_data;
	UINT16		output_data;
	UINT16		output_control;
	UINT64		output_control_cycles;
	UINT8		last_output_full;
	UINT8		last_input_empty;
	UINT16		progflags;
	void		(*output_full_cb)(running_machine &, int);
	void		(*input_empty_cb)(running_machine &, int);
	UINT16		(*fifo_data_r)(device_t *device);
	UINT16		(*fifo_status_r)(device_t *device);

	/* timers */
	UINT8		timer_enable;
	UINT8		timer_ignore;
	UINT64		timer_start_cycles;
	UINT32		timer_start_count;
	UINT32		timer_scale;
	UINT32		timer_period;
	UINT32		timers_fired;

	UINT16 *sram;
	UINT16 *polling_base;
	UINT32 *internal_program_ram;
	UINT32 *external_program_ram;

	sdrc_state sdrc;
	dsio_state dsio;
	hle_transfer_state transfer;
};



/*************************************
 *
 *  Statics
 *
 *************************************/

static dcs_state dcs;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static READ16_HANDLER( dcs_dataram_r );
static WRITE16_HANDLER( dcs_dataram_w );
static WRITE16_HANDLER( dcs_data_bank_select_w );

static void sdrc_reset(running_machine &machine);
static READ16_HANDLER( sdrc_r );
static WRITE16_HANDLER( sdrc_w );

static void dsio_reset(running_machine &machine);
static READ16_HANDLER( dsio_r );
static WRITE16_HANDLER( dsio_w );

static void denver_reset(running_machine &machine);
static READ16_HANDLER( denver_r );
static WRITE16_HANDLER( denver_w );

static READ16_HANDLER( adsp_control_r );
static WRITE16_HANDLER( adsp_control_w );

static READ16_HANDLER( latch_status_r );
static READ16_HANDLER( fifo_input_r );
static READ16_HANDLER( input_latch_r );
static WRITE16_HANDLER( input_latch_ack_w );
static WRITE16_HANDLER( output_latch_w );
static READ16_HANDLER( output_control_r );
static WRITE16_HANDLER( output_control_w );

static void timer_enable_callback(adsp21xx_device &device, int enable);
static TIMER_DEVICE_CALLBACK( internal_timer_callback );
static TIMER_DEVICE_CALLBACK( dcs_irq );
static TIMER_DEVICE_CALLBACK( sport0_irq );
static void recompute_sample_rate(running_machine &machine);
static void sound_tx_callback(adsp21xx_device &device, int port, INT32 data);

static READ16_HANDLER( dcs_polling_r );
static WRITE16_HANDLER( dcs_polling_w );

static TIMER_DEVICE_CALLBACK( transfer_watchdog_callback );
static int preprocess_write(running_machine &machine, UINT16 data);

static void sdrc_remap_memory(running_machine &machine);



/*************************************
 *
 *  Original DCS Memory Maps
 *
 *************************************/

/* DCS 2k memory map */
static ADDRESS_MAP_START( dcs_2k_program_map, AS_PROGRAM, 32, driver_device )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("dcsint")
	AM_RANGE(0x0800, 0x0fff) AM_RAM AM_SHARE("dcsext")
	AM_RANGE(0x1000, 0x17ff) AM_RAM AM_SHARE("dcsext")
	AM_RANGE(0x1800, 0x1fff) AM_RAM AM_SHARE("dcsext")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dcs_2k_data_map, AS_DATA, 16, driver_device )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x1800) AM_READWRITE_LEGACY(dcs_dataram_r, dcs_dataram_w)
	AM_RANGE(0x2000, 0x2fff) AM_ROMBANK("databank")
	AM_RANGE(0x3000, 0x33ff) AM_WRITE_LEGACY(dcs_data_bank_select_w)
	AM_RANGE(0x3400, 0x37ff) AM_READWRITE_LEGACY(input_latch_r, output_latch_w)
	AM_RANGE(0x3800, 0x39ff) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE_LEGACY(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END


/* DCS 2k with UART memory map */
static ADDRESS_MAP_START( dcs_2k_uart_data_map, AS_DATA, 16, driver_device )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x1800) AM_READWRITE_LEGACY(dcs_dataram_r, dcs_dataram_w)
	AM_RANGE(0x2000, 0x2fff) AM_ROMBANK("databank")
	AM_RANGE(0x3000, 0x33ff) AM_WRITE_LEGACY(dcs_data_bank_select_w)
	AM_RANGE(0x3400, 0x3402) AM_NOP								/* UART (ignored) */
	AM_RANGE(0x3403, 0x3403) AM_READWRITE_LEGACY(input_latch_r, output_latch_w)
	AM_RANGE(0x3404, 0x3405) AM_NOP								/* UART (ignored) */
	AM_RANGE(0x3800, 0x39ff) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE_LEGACY(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END


/* DCS 8k memory map */
static ADDRESS_MAP_START( dcs_8k_program_map, AS_PROGRAM, 32, driver_device )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("dcsint")
	AM_RANGE(0x0800, 0x1fff) AM_RAM AM_SHARE("dcsext")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dcs_8k_data_map, AS_DATA, 16, driver_device )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x1fff) AM_READWRITE_LEGACY(dcs_dataram_r, dcs_dataram_w)
	AM_RANGE(0x2000, 0x2fff) AM_ROMBANK("databank")
	AM_RANGE(0x3000, 0x33ff) AM_WRITE_LEGACY(dcs_data_bank_select_w)
	AM_RANGE(0x3400, 0x37ff) AM_READWRITE_LEGACY(input_latch_r, output_latch_w)
	AM_RANGE(0x3800, 0x39ff) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE_LEGACY(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END



/*************************************
 *
 *  DCS2 Memory Maps
 *
 *************************************/

static ADDRESS_MAP_START( dcs2_2115_program_map, AS_PROGRAM, 32, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_RAM	AM_SHARE("dcsint")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dcs2_2104_program_map, AS_PROGRAM, 32, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x01ff) AM_RAM	AM_SHARE("dcsint")
ADDRESS_MAP_END


static ADDRESS_MAP_START( dcs2_2115_data_map, AS_DATA, 16, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0400, 0x0400) AM_READWRITE_LEGACY(input_latch_r, input_latch_ack_w)
	AM_RANGE(0x0401, 0x0401) AM_WRITE_LEGACY(output_latch_w)
	AM_RANGE(0x0402, 0x0402) AM_READWRITE_LEGACY(output_control_r, output_control_w)
	AM_RANGE(0x0403, 0x0403) AM_READ_LEGACY(latch_status_r)
	AM_RANGE(0x0404, 0x0407) AM_READ_LEGACY(fifo_input_r)
	AM_RANGE(0x0480, 0x0483) AM_READWRITE_LEGACY(sdrc_r, sdrc_w)
	AM_RANGE(0x3800, 0x39ff) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE_LEGACY(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dcs2_2104_data_map, AS_DATA, 16, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0400, 0x0400) AM_READWRITE_LEGACY(input_latch_r, input_latch_ack_w)
	AM_RANGE(0x0401, 0x0401) AM_WRITE_LEGACY(output_latch_w)
	AM_RANGE(0x0402, 0x0402) AM_READWRITE_LEGACY(output_control_r, output_control_w)
	AM_RANGE(0x0403, 0x0403) AM_READ_LEGACY(latch_status_r)
	AM_RANGE(0x0404, 0x0407) AM_READ_LEGACY(fifo_input_r)
	AM_RANGE(0x0480, 0x0483) AM_READWRITE_LEGACY(sdrc_r, sdrc_w)
	AM_RANGE(0x3800, 0x38ff) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE_LEGACY(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END



/*************************************
 *
 *  DSIO Memory Maps
 *
 *************************************/

static ADDRESS_MAP_START( dsio_program_map, AS_PROGRAM, 32, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_RAM	AM_SHARE("dcsint")
ADDRESS_MAP_END


static ADDRESS_MAP_START( dsio_data_map, AS_DATA, 16, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_RAMBANK("databank")
	AM_RANGE(0x0400, 0x3fdf) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE_LEGACY(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( dsio_io_map, AS_IO, 16, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0400, 0x0400) AM_READWRITE_LEGACY(input_latch_r, input_latch_ack_w)
	AM_RANGE(0x0401, 0x0401) AM_WRITE_LEGACY(output_latch_w)
	AM_RANGE(0x0402, 0x0402) AM_READWRITE_LEGACY(output_control_r, output_control_w)
	AM_RANGE(0x0403, 0x0403) AM_READ_LEGACY(latch_status_r)
	AM_RANGE(0x0404, 0x0407) AM_READ_LEGACY(fifo_input_r)
	AM_RANGE(0x0480, 0x0483) AM_READWRITE_LEGACY(dsio_r, dsio_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Denver Memory Maps
 *
 *************************************/

static ADDRESS_MAP_START( denver_program_map, AS_PROGRAM, 32, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_RAM	AM_SHARE("dcsint")
ADDRESS_MAP_END


static ADDRESS_MAP_START( denver_data_map, AS_DATA, 16, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_RAMBANK("databank")
	AM_RANGE(0x0800, 0x3fdf) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE_LEGACY(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( denver_io_map, AS_IO, 16, driver_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0400, 0x0400) AM_READWRITE_LEGACY(input_latch_r, input_latch_ack_w)
	AM_RANGE(0x0401, 0x0401) AM_WRITE_LEGACY(output_latch_w)
	AM_RANGE(0x0402, 0x0402) AM_READWRITE_LEGACY(output_control_r, output_control_w)
	AM_RANGE(0x0403, 0x0403) AM_READ_LEGACY(latch_status_r)
	AM_RANGE(0x0404, 0x0407) AM_READ_LEGACY(fifo_input_r)
	AM_RANGE(0x0480, 0x0483) AM_READWRITE_LEGACY(denver_r, denver_w)
ADDRESS_MAP_END



/*************************************
 *
 *  CPU configuration
 *
 *************************************/

static const adsp21xx_config adsp_config =
{
	NULL,					/* callback for serial receive */
	sound_tx_callback,		/* callback for serial transmit */
	timer_enable_callback	/* callback for timer fired */
};



/*************************************
 *
 *  Original DCS Machine Drivers
 *
 *************************************/

/* Basic DCS system with ADSP-2105 and 2k of SRAM (T-unit, V-unit, Killer Instinct) */
MACHINE_CONFIG_FRAGMENT( dcs_audio_2k )
	MCFG_CPU_ADD("dcs", ADSP2105, XTAL_10MHz)
	MCFG_ADSP21XX_CONFIG(adsp_config)
	MCFG_CPU_PROGRAM_MAP(dcs_2k_program_map)
	MCFG_CPU_DATA_MAP(dcs_2k_data_map)

	MCFG_TIMER_ADD("dcs_reg_timer", dcs_irq)
	MCFG_TIMER_ADD("dcs_int_timer", internal_timer_callback)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/* Basic DCS system with ADSP-2105 and 2k of SRAM, using a UART for communications (X-unit) */
MACHINE_CONFIG_DERIVED( dcs_audio_2k_uart, dcs_audio_2k )

	MCFG_CPU_MODIFY("dcs")
	MCFG_CPU_DATA_MAP(dcs_2k_uart_data_map)
MACHINE_CONFIG_END


/* Basic DCS system with ADSP-2105 and 8k of SRAM (Wolf-unit) */
MACHINE_CONFIG_DERIVED( dcs_audio_8k, dcs_audio_2k )

	MCFG_CPU_MODIFY("dcs")
	MCFG_CPU_PROGRAM_MAP(dcs_8k_program_map)
	MCFG_CPU_DATA_MAP(dcs_8k_data_map)
MACHINE_CONFIG_END



/*************************************
 *
 *  DCS2 Machine Drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( dcs2_audio_2115 )
	MCFG_CPU_ADD("dcs2", ADSP2115, XTAL_16MHz)
	MCFG_ADSP21XX_CONFIG(adsp_config)
	MCFG_CPU_PROGRAM_MAP(dcs2_2115_program_map)
	MCFG_CPU_DATA_MAP(dcs2_2115_data_map)

	MCFG_TIMER_ADD("dcs_reg_timer", dcs_irq)
	MCFG_TIMER_ADD("dcs_sport_timer", sport0_irq)
	MCFG_TIMER_ADD("dcs_int_timer", internal_timer_callback)
	MCFG_TIMER_ADD("dcs_hle_timer", transfer_watchdog_callback)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( dcs2_audio_2104, dcs2_audio_2115 )
	MCFG_CPU_REPLACE("dcs2", ADSP2104, XTAL_16MHz)
	MCFG_ADSP21XX_CONFIG(adsp_config)
	MCFG_CPU_PROGRAM_MAP(dcs2_2104_program_map)
	MCFG_CPU_DATA_MAP(dcs2_2104_data_map)
MACHINE_CONFIG_END



/*************************************
 *
 *  DSIO Machine Drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( dcs2_audio_dsio )
	MCFG_CPU_ADD("dsio", ADSP2181, XTAL_32MHz)
	MCFG_ADSP21XX_CONFIG(adsp_config)
	MCFG_CPU_PROGRAM_MAP(dsio_program_map)
	MCFG_CPU_DATA_MAP(dsio_data_map)
	MCFG_CPU_IO_MAP(dsio_io_map)

	MCFG_TIMER_ADD("dcs_reg_timer", dcs_irq)
	MCFG_TIMER_ADD("dcs_int_timer", internal_timer_callback)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  Denver Machine Drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( dcs2_audio_denver )
	MCFG_CPU_ADD("denver", ADSP2181, XTAL_33_333MHz)
	MCFG_ADSP21XX_CONFIG(adsp_config)
	MCFG_CPU_PROGRAM_MAP(denver_program_map)
	MCFG_CPU_DATA_MAP(denver_data_map)
	MCFG_CPU_IO_MAP(denver_io_map)

	MCFG_TIMER_ADD("dcs_reg_timer", dcs_irq)
	MCFG_TIMER_ADD("dcs_int_timer", internal_timer_callback)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_SOUND_ADD("dac3", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_SOUND_ADD("dac4", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_SOUND_ADD("dac5", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_SOUND_ADD("dac6", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ADSP booting
 *
 *************************************/

static void dcs_boot(running_machine &machine)
{
	UINT8 buffer[0x1000];
//  UINT32 max_banks;
	UINT16 *base;
	int i;

	switch (dcs.rev)
	{
		/* rev 1: use the last set data bank to boot from */
		case 1:

			/* determine the base */
//          max_banks = dcs.bootrom_words / 0x1000;
			base = dcs.bootrom + ((dcs.sounddata_bank * 0x1000) % dcs.bootrom_words);

			/* convert from 16-bit data to 8-bit data and boot */
			for (i = 0; i < 0x1000; i++)
				buffer[i] = base[i];
			dcs.cpu->load_boot_data(buffer, dcs.internal_program_ram);
			break;

		/* rev 2: use the ROM page in the SDRC to boot from */
		case 2:

			/* determine the base */
			if (dcs.bootrom == dcs.sounddata)
			{
				/* EPROM case: page is selected from the page register */
				base = dcs.bootrom + ((SDRC_EPM_PG * 0x1000) % dcs.bootrom_words);
			}
			else
			{
				/* DRAM case: page is selected from the ROM page register */
				base = dcs.bootrom + ((SDRC_ROM_PG * 0x1000) % dcs.bootrom_words);
			}

			/* convert from 16-bit data to 8-bit data and boot */
			for (i = 0; i < 0x1000; i++)
				buffer[i] = base[i];
			dcs.cpu->load_boot_data(buffer, dcs.internal_program_ram);
			break;

		/* rev 3/4: HALT the ADSP-2181 until program is downloaded via IDMA */
		case 3:
		case 4:
			device_set_input_line(dcs.cpu, INPUT_LINE_HALT, ASSERT_LINE);
			dcs.dsio.start_on_next_write = 0;
			break;
	}
}



/*************************************
 *
 *  System reset
 *
 *************************************/

static TIMER_CALLBACK( dcs_reset )
{
	if (LOG_DCS_IO)
		logerror("dcs_reset\n");

	/* reset the memory banking */
	switch (dcs.rev)
	{
		/* rev 1: just reset the bank to 0 */
		case 1:
			dcs.sounddata_bank = 0;
			memory_set_bank(machine, "databank", 0);
			break;

		/* rev 2: reset the SDRC ASIC */
		case 2:
			sdrc_reset(machine);
			break;

		/* rev 3: reset the DSIO ASIC */
		case 3:
			dsio_reset(machine);
			break;

		/* rev 4: reset the Denver ASIC */
		case 4:
			denver_reset(machine);
			break;
	}

	/* initialize our state structure and install the transmit callback */
	dcs.size = 0;
	dcs.incs = 0;
	dcs.ireg = 0;

	/* initialize the ADSP control regs */
	memset(dcs.control_regs, 0, sizeof(dcs.control_regs));

	/* clear all interrupts */
	device_set_input_line(dcs.cpu, ADSP2105_IRQ0, CLEAR_LINE);
	device_set_input_line(dcs.cpu, ADSP2105_IRQ1, CLEAR_LINE);
	device_set_input_line(dcs.cpu, ADSP2105_IRQ2, CLEAR_LINE);

	/* initialize the comm bits */
	SET_INPUT_EMPTY();
	SET_OUTPUT_EMPTY();
	if (!dcs.last_input_empty && dcs.input_empty_cb)
		(*dcs.input_empty_cb)(machine, dcs.last_input_empty = 1);
	if (dcs.last_output_full && dcs.output_full_cb)
		(*dcs.output_full_cb)(machine, dcs.last_output_full = 0);

	/* boot */
	dcs_boot(machine);

	/* reset timers */
	dcs.timer_enable = 0;
	dcs.timer_scale = 1;
	dcs.internal_timer->reset();

	/* start the SPORT0 timer */
	if (dcs.sport_timer != NULL)
		dcs.sport_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));

	/* reset the HLE transfer states */
	dcs.transfer.dcs_state = dcs.transfer.state = 0;
}



/*************************************
 *
 *  System setup
 *
 *************************************/

static void dcs_register_state(running_machine &machine)
{
	state_save_register_global_array(machine, dcs.sdrc.reg);
	state_save_register_global(machine, dcs.sdrc.seed);

	state_save_register_global_array(machine, dcs.dsio.reg);
	state_save_register_global(machine, dcs.dsio.start_on_next_write);
	state_save_register_global(machine, dcs.dsio.channelbits);

	state_save_register_global(machine, dcs.channels);
	state_save_register_global(machine, dcs.size);
	state_save_register_global(machine, dcs.incs);
	state_save_register_global(machine, dcs.ireg);
	state_save_register_global(machine, dcs.ireg_base);
	state_save_register_global_array(machine, dcs.control_regs);

	state_save_register_global(machine, dcs.sounddata_bank);

	state_save_register_global(machine, dcs.auto_ack);
	state_save_register_global(machine, dcs.latch_control);
	state_save_register_global(machine, dcs.input_data);
	state_save_register_global(machine, dcs.output_data);
	state_save_register_global(machine, dcs.output_control);
	state_save_register_global(machine, dcs.output_control_cycles);
	state_save_register_global(machine, dcs.last_output_full);
	state_save_register_global(machine, dcs.last_input_empty);
	state_save_register_global(machine, dcs.progflags);

	state_save_register_global(machine, dcs.timer_enable);
	state_save_register_global(machine, dcs.timer_ignore);
	state_save_register_global(machine, dcs.timer_start_cycles);
	state_save_register_global(machine, dcs.timer_start_count);
	state_save_register_global(machine, dcs.timer_scale);
	state_save_register_global(machine, dcs.timer_period);
	state_save_register_global(machine, dcs.timers_fired);

	state_save_register_global(machine, dcs.transfer.dcs_state);
	state_save_register_global(machine, dcs.transfer.state);
	state_save_register_global(machine, dcs.transfer.start);
	state_save_register_global(machine, dcs.transfer.stop);
	state_save_register_global(machine, dcs.transfer.type);
	state_save_register_global(machine, dcs.transfer.temp);
	state_save_register_global(machine, dcs.transfer.writes_left);
	state_save_register_global(machine, dcs.transfer.sum);
	state_save_register_global(machine, dcs.transfer.fifo_entries);

	if (dcs.sram != NULL)
		state_save_register_global_pointer(machine, dcs.sram, 0x8000*4 / sizeof(dcs.sram[0]));

	if (dcs.rev == 2)
		machine.save().register_postload(save_prepost_delegate(FUNC(sdrc_remap_memory), &machine));
}

void dcs_init(running_machine &machine)
{
	memset(&dcs, 0, sizeof(dcs));
	dcs.sram = NULL;

	dcs.internal_program_ram = (UINT32 *)machine.memory().shared("dcsint")->ptr();
	dcs.external_program_ram = (UINT32 *)machine.memory().shared("dcsext")->ptr();

	/* find the DCS CPU and the sound ROMs */
	dcs.cpu = machine.device<adsp21xx_device>("dcs");
	dcs.program = dcs.cpu->space(AS_PROGRAM);
	dcs.data = dcs.cpu->space(AS_DATA);
	dcs.rev = 1;
	dcs.channels = 1;
	dcs.dmadac[0] = machine.device<dmadac_sound_device>("dac");

	/* configure boot and sound ROMs */
	dcs.bootrom = (UINT16 *)machine.region("dcs")->base();
	dcs.bootrom_words = machine.region("dcs")->bytes() / 2;
	dcs.sounddata = dcs.bootrom;
	dcs.sounddata_words = dcs.bootrom_words;
	dcs.sounddata_banks = dcs.sounddata_words / 0x1000;
	memory_configure_bank(machine, "databank", 0, dcs.sounddata_banks, dcs.sounddata, 0x1000*2);

	/* create the timers */
	dcs.internal_timer = machine.device<timer_device>("dcs_int_timer");
	dcs.reg_timer = machine.device<timer_device>("dcs_reg_timer");

	/* non-RAM based automatically acks */
	dcs.auto_ack = TRUE;

	/* register for save states */
	dcs_register_state(machine);

	/* reset the system */
	dcs_reset(machine, NULL, 0);
}


void dcs2_init(running_machine &machine, int dram_in_mb, offs_t polling_offset)
{
	int soundbank_words;

	memset(&dcs, 0, sizeof(dcs));
	dcs.internal_program_ram = (UINT32 *)machine.memory().shared("dcsint")->ptr();
	dcs.external_program_ram = (UINT32 *)machine.memory().shared("dcsext")->ptr();

	/* find the DCS CPU and the sound ROMs */
	dcs.cpu = machine.device<adsp21xx_device>("dcs2");
	dcs.rev = 2;
	soundbank_words = 0x1000;
	if (dcs.cpu == NULL)
	{
		dcs.cpu = machine.device<adsp21xx_device>("dsio");
		dcs.rev = 3;
		soundbank_words = 0x400;
	}
	if (dcs.cpu == NULL)
	{
		dcs.cpu = machine.device<adsp21xx_device>("denver");
		dcs.rev = 4;
		soundbank_words = 0x800;
	}
	dcs.program = dcs.cpu->space(AS_PROGRAM);
	dcs.data = dcs.cpu->space(AS_DATA);
	dcs.channels = 2;
	dcs.dmadac[0] = machine.device<dmadac_sound_device>("dac1");
	dcs.dmadac[1] = machine.device<dmadac_sound_device>("dac2");

	/* always boot from the base of "dcs" */
	dcs.bootrom = (UINT16 *)machine.region("dcs")->base();
	dcs.bootrom_words = machine.region("dcs")->bytes() / 2;

	/* supports both RAM and ROM variants */
	if (dram_in_mb != 0)
	{
		dcs.sounddata = auto_alloc_array(machine, UINT16, dram_in_mb << (20-1));
		dcs.sounddata_words = (dram_in_mb << 20) / 2;
	}
	else
	{
		dcs.sounddata = dcs.bootrom;
		dcs.sounddata_words = dcs.bootrom_words;
	}
	dcs.sounddata_banks = dcs.sounddata_words / soundbank_words;
	if (dcs.rev != 2)
		memory_configure_bank(machine, "databank", 0, dcs.sounddata_banks, dcs.sounddata, soundbank_words*2);

	/* allocate memory for the SRAM */
	dcs.sram = auto_alloc_array(machine, UINT16, 0x8000*4/2);

	/* create the timers */
	dcs.internal_timer = machine.device<timer_device>("dcs_int_timer");
	dcs.reg_timer = machine.device<timer_device>("dcs_reg_timer");
	dcs.sport_timer = machine.device<timer_device>("dcs_sport_timer");

	/* we don't do auto-ack by default */
	dcs.auto_ack = FALSE;

	/* install the speedup handler */
	dcs.polling_offset = polling_offset;
	if (polling_offset)
		dcs.polling_base = dcs.cpu->space(AS_DATA)->install_legacy_readwrite_handler(dcs.polling_offset, dcs.polling_offset, FUNC(dcs_polling_r), FUNC(dcs_polling_w));

	/* allocate a watchdog timer for HLE transfers */
	dcs.transfer.hle_enabled = (ENABLE_HLE_TRANSFERS && dram_in_mb != 0);
	if (dcs.transfer.hle_enabled)
		dcs.transfer.watchdog = machine.device<timer_device>("dcs_hle_timer");

	/* register for save states */
	dcs_register_state(machine);

	/* reset the system */
	dcs_reset(machine, NULL, 0);
}


void dcs_set_auto_ack(running_machine &machine, int state)
{
	dcs.auto_ack = state;
}



/*************************************
 *
 *  Original DCS read/write handlers
 *
 *************************************/

static READ16_HANDLER( dcs_dataram_r )
{
	return dcs.external_program_ram[offset] >> 8;
}


static WRITE16_HANDLER( dcs_dataram_w )
{
	UINT16 newdata = dcs.external_program_ram[offset] >> 8;
	COMBINE_DATA(&newdata);
	dcs.external_program_ram[offset] = (newdata << 8) | (dcs.external_program_ram[offset] & 0xff);
}


static WRITE16_HANDLER( dcs_data_bank_select_w )
{
	dcs.sounddata_bank = data & 0x7ff;
	memory_set_bank(space->machine(), "databank", dcs.sounddata_bank % dcs.sounddata_banks);

	/* bit 11 = sound board led */
#if 0
	set_led_status(space->machine(), 2, data & 0x800);
#endif
}



/*************************************
 *
 *  SDRC ASIC Memory handling
 *
 *************************************/

INLINE void sdrc_update_bank_pointers(running_machine &machine)
{
	if (SDRC_SM_EN != 0)
	{
		int pagesize = (SDRC_ROM_SZ == 0 && SDRC_ROM_ST != 0) ? 4096 : 1024;

		/* update the bank pointer based on whether we are ROM-based or RAM-based */
		if (dcs.bootrom == dcs.sounddata)
		{
			/* ROM-based; use the memory page to select from ROM */
			if (SDRC_ROM_MS == 1 && SDRC_ROM_ST != 3)
				memory_set_bankptr(machine, "rompage", &dcs.sounddata[(SDRC_EPM_PG * pagesize) % dcs.sounddata_words]);
		}
		else
		{
			/* RAM-based; use the ROM page to select from ROM, and the memory page to select from RAM */
			if (SDRC_ROM_MS == 1 && SDRC_ROM_ST != 3)
				memory_set_bankptr(machine, "rompage", &dcs.bootrom[(SDRC_ROM_PG * 4096 /*pagesize*/) % dcs.bootrom_words]);
			if (SDRC_DM_ST != 0)
				memory_set_bankptr(machine, "drampage", &dcs.sounddata[(SDRC_DM_PG * 1024) % dcs.sounddata_words]);
		}
	}
}


static void sdrc_remap_memory(running_machine &machine)
{
	/* if SRAM disabled, clean it out */
	if (SDRC_SM_EN == 0)
	{
		dcs.program->unmap_readwrite(0x0800, 0x3fff);
		dcs.data->unmap_readwrite(0x0800, 0x37ff);
	}

	/* otherwise, map the SRAM */
	else
	{
		/* first start with a clean program map */
		dcs.program->install_ram(0x0800, 0x3fff, dcs.sram + 0x4800);

		/* set up the data map based on the SRAM banking */
		/* map 0: ram from 0800-37ff */
		if (SDRC_SM_BK == 0)
		{
			dcs.data->install_ram(0x0800, 0x17ff, dcs.sram + 0x0000);
			dcs.data->install_ram(0x1800, 0x27ff, dcs.sram + 0x1000);
			dcs.data->install_ram(0x2800, 0x37ff, dcs.sram + 0x2000);
		}

		/* map 1: nothing from 0800-17ff, alternate RAM at 1800-27ff, same RAM at 2800-37ff */
		else
		{
			dcs.data->unmap_readwrite(0x0800, 0x17ff);
			dcs.data->install_ram(0x1800, 0x27ff, dcs.sram + 0x3000);
			dcs.data->install_ram(0x2800, 0x37ff, dcs.sram + 0x2000);
		}
	}

	/* map the ROM page as bank 25 */
	if (SDRC_ROM_MS == 1 && SDRC_ROM_ST != 3)
	{
		int baseaddr = (SDRC_ROM_ST == 0) ? 0x0000 : (SDRC_ROM_ST == 1) ? 0x3000 : 0x3400;
		int pagesize = (SDRC_ROM_SZ == 0 && SDRC_ROM_ST != 0) ? 4096 : 1024;
		dcs.data->install_read_bank(baseaddr, baseaddr + pagesize - 1, "rompage");
	}

	/* map the DRAM page as bank 26 */
	if (SDRC_DM_ST != 0)
	{
		int baseaddr = (SDRC_DM_ST == 1) ? 0x0000 : (SDRC_DM_ST == 2) ? 0x3000 : 0x3400;
		dcs.data->install_readwrite_bank(baseaddr, baseaddr + 0x3ff, "drampage");
	}

	/* update the bank pointers */
	sdrc_update_bank_pointers(machine);

	/* reinstall the polling hotspot */
	if (dcs.polling_offset)
		dcs.polling_base = dcs.cpu->space(AS_DATA)->install_legacy_readwrite_handler(dcs.polling_offset, dcs.polling_offset, FUNC(dcs_polling_r), FUNC(dcs_polling_w));
}


static void sdrc_reset(running_machine &machine)
{
	memset(dcs.sdrc.reg, 0, sizeof(dcs.sdrc.reg));
	sdrc_remap_memory(machine);
}



/*************************************
 *
 *  SDRC ASIC read/write
 *
 *************************************/

static READ16_HANDLER( sdrc_r )
{
	sdrc_state &sdrc = dcs.sdrc;
	UINT16 result = sdrc.reg[offset];

	/* offset 3 is for security */
	if (offset == 3)
	{
		switch (SDRC_SMODE)
		{
			default:
			case 0:	/* no-op */
				result = 0x5a81;
				break;

			case 1:	/* write seed */
				result = 0x5aa4;
				break;

			case 2:	/* read data */
				result = 0x5a00 | ((sdrc.seed & 0x3f) << 1);
				break;

			case 3:	/* shift left */
				result = 0x5ab9;
				break;

			case 4:	/* add */
				result = 0x5a03;
				break;

			case 5:	/* xor */
				result = 0x5a69;
				break;

			case 6:	/* prg */
				result = 0x5a20;
				break;

			case 7:	/* invert */
				result = 0x5aff;
				break;
		}
	}

	return result;
}


static WRITE16_HANDLER( sdrc_w )
{
	sdrc_state &sdrc = dcs.sdrc;
	UINT16 diff = sdrc.reg[offset] ^ data;

	switch (offset)
	{
		/* offset 0 controls ROM mapping */
		case 0:
			sdrc.reg[0] = data;
			if (diff & 0x1833)
				sdrc_remap_memory(space->machine());
			if (diff & 0x0380)
				sdrc_update_bank_pointers(space->machine());
			break;

		/* offset 1 controls RAM mapping */
		case 1:
			sdrc.reg[1] = data;
			//dmadac_enable(&dcs.dmadac[0], dcs.channels, SDRC_MUTE);
			if (diff & 0x0003)
				sdrc_remap_memory(space->machine());
			break;

		/* offset 2 controls paging */
		case 2:
			sdrc.reg[2] = data;
			if (diff & 0x1fff)
				sdrc_update_bank_pointers(space->machine());
			break;

		/* offset 3 controls security */
		case 3:
			switch (SDRC_SMODE)
			{
				case 0:	/* no-op */
				case 2:	/* read data */
					break;

				case 1:	/* write seed */
					sdrc.seed = data & 0xff;
					break;

				case 3:	/* shift left */
					sdrc.seed = (sdrc.seed << 1) | 1;
					break;

				case 4:	/* add */
					sdrc.seed += sdrc.seed >> 1;
					break;

				case 5:	/* xor */
					sdrc.seed ^= (sdrc.seed << 1) | 1;
					break;

				case 6:	/* prg */
					sdrc.seed = (((sdrc.seed << 7) ^ (sdrc.seed << 5) ^ (sdrc.seed << 4) ^ (sdrc.seed << 3)) & 0x80) | (sdrc.seed >> 1);
					break;

				case 7:	/* invert */
					sdrc.seed = ~sdrc.seed;
					break;
			}
			break;
	}
}



/*************************************
 *
 *  DSIO ASIC read/write
 *
 *************************************/

static void dsio_reset(running_machine &machine)
{
	memset(&dcs.dsio, 0, sizeof(dcs.dsio));
}


static READ16_HANDLER( dsio_r )
{
	dsio_state &dsio = dcs.dsio;
	UINT16 result = dsio.reg[offset];

	if (offset == 1)
	{
		/* bit 4 specifies which channel is being output */
		dsio.channelbits ^= 0x0010;
		result = (result & ~0x0010) | dsio.channelbits;
	}
	return result;
}


static WRITE16_HANDLER( dsio_w )
{
	dsio_state &dsio = dcs.dsio;

	switch (offset)
	{
		/* offset 1 controls I/O */
		case 1:
			dsio.reg[1] = data;

			/* determine /MUTE and number of channels */
			dmadac_enable(&dcs.dmadac[0], dcs.channels, DSIO_MUTE);

			/* bit 0 resets the FIFO */
			midway_ioasic_fifo_reset_w(space->machine(), DSIO_EMPTY_FIFO ^ 1);
			break;

		/* offset 2 controls RAM pages */
		case 2:
			dsio.reg[2] = data;
			memory_set_bank(space->machine(), "databank", DSIO_DM_PG % dcs.sounddata_banks);
			break;
	}
}



/*************************************
 *
 *  Denver ASIC read/write
 *
 *************************************/

static void denver_reset(running_machine &machine)
{
	memset(&dcs.dsio, 0, sizeof(dcs.dsio));
}


static READ16_HANDLER( denver_r )
{
	UINT16 result = dcs.dsio.reg[offset];

	if (offset == 3)
	{
		/* returns 1 for DRAM, 2 for EPROM-based */
		result = 0x0001;
	}
	return result;
}


static WRITE16_HANDLER( denver_w )
{
	dsio_state &dsio = dcs.dsio;
	int enable, channels, chan;

	switch (offset)
	{
		/* offset 1 controls I/O */
		case 1:
			dsio.reg[1] = data;

			/* determine /MUTE and number of channels */
			enable = DENV_MUTE;
			channels = 2 + 2 * DENV_CHANNELS;

			/* if the number of channels has changed, adjust */
			if (channels != dcs.channels)
			{
				dcs.channels = channels;
				for (chan = 0; chan < dcs.channels; chan++)
				{
					char buffer[10];
					sprintf(buffer, "dac%d", chan + 1);
					dcs.dmadac[chan] = space->machine().device<dmadac_sound_device>(buffer);
				}
				dmadac_enable(&dcs.dmadac[0], dcs.channels, enable);
				if (dcs.channels < 6)
					dmadac_enable(&dcs.dmadac[dcs.channels], 6 - dcs.channels, FALSE);
				recompute_sample_rate(space->machine());
			}
			break;

		/* offset 2 controls RAM pages */
		case 2:
			dsio.reg[2] = data;
			memory_set_bank(space->machine(), "databank", DENV_DM_PG % dcs.sounddata_bank);
			break;

		/* offset 3 controls FIFO reset */
		case 3:
			midway_ioasic_fifo_reset_w(space->machine(), 1);
			break;
	}
}



/*************************************
 *
 *  DSIO/Denver IDMA access
 *
 *************************************/

WRITE32_HANDLER( dsio_idma_addr_w )
{
	dsio_state &dsio = dcs.dsio;
	if (LOG_DCS_TRANSFERS)
		logerror("%08X:IDMA_addr = %04X\n", cpu_get_pc(&space->device()), data);
	downcast<adsp2181_device *>(dcs.cpu)->idma_addr_w(data);
	if (data == 0)
		dsio.start_on_next_write = 2;
}


WRITE32_HANDLER( dsio_idma_data_w )
{
	dsio_state &dsio = dcs.dsio;
	UINT32 pc = cpu_get_pc(&space->device());
	if (ACCESSING_BITS_0_15)
	{
		if (LOG_DCS_TRANSFERS)
			logerror("%08X:IDMA_data_w(%04X) = %04X\n", pc, downcast<adsp2181_device *>(dcs.cpu)->idma_addr_r(), data & 0xffff);
		downcast<adsp2181_device *>(dcs.cpu)->idma_data_w(data & 0xffff);
	}
	if (ACCESSING_BITS_16_31)
	{
		if (LOG_DCS_TRANSFERS)
			logerror("%08X:IDMA_data_w(%04X) = %04X\n", pc, downcast<adsp2181_device *>(dcs.cpu)->idma_addr_r(), data >> 16);
		downcast<adsp2181_device *>(dcs.cpu)->idma_data_w(data >> 16);
	}
	if (dsio.start_on_next_write && --dsio.start_on_next_write == 0)
	{
		logerror("Starting DSIO CPU\n");
		device_set_input_line(dcs.cpu, INPUT_LINE_HALT, CLEAR_LINE);
	}
}


READ32_HANDLER( dsio_idma_data_r )
{
	UINT32 result;
	result = downcast<adsp2181_device *>(dcs.cpu)->idma_data_r();
	if (LOG_DCS_TRANSFERS)
		logerror("%08X:IDMA_data_r(%04X) = %04X\n", cpu_get_pc(&space->device()), downcast<adsp2181_device *>(dcs.cpu)->idma_addr_r(), result);
	return result;
}



/***************************************************************************
    DCS COMMUNICATIONS
****************************************************************************/

void dcs_set_io_callbacks(void (*output_full_cb)(running_machine &, int), void (*input_empty_cb)(running_machine &, int))
{
	dcs.input_empty_cb = input_empty_cb;
	dcs.output_full_cb = output_full_cb;
}


void dcs_set_fifo_callbacks(UINT16 (*fifo_data_r)(device_t *device), UINT16 (*fifo_status_r)(device_t *device))
{
	dcs.fifo_data_r = fifo_data_r;
	dcs.fifo_status_r = fifo_status_r;
}


int dcs_control_r(running_machine &machine)
{
	/* only boost for DCS2 boards */
	if (!dcs.auto_ack && !dcs.transfer.hle_enabled)
		machine.scheduler().boost_interleave(attotime::from_nsec(500), attotime::from_usec(5));
	return dcs.latch_control;
}


void dcs_reset_w(running_machine &machine, int state)
{
	/* going high halts the CPU */
	if (state)
	{
		logerror("%s: DCS reset = %d\n", machine.describe_context(), state);

		/* just run through the init code again */
		machine.scheduler().synchronize(FUNC(dcs_reset));
		device_set_input_line(dcs.cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}

	/* going low resets and reactivates the CPU */
	else
		device_set_input_line(dcs.cpu, INPUT_LINE_RESET, CLEAR_LINE);
}


static READ16_HANDLER( latch_status_r )
{
	int result = 0;
	if (IS_INPUT_FULL())
		result |= 0x80;
	if (IS_OUTPUT_EMPTY())
		result |= 0x40;
	if (dcs.fifo_status_r != NULL && (!dcs.transfer.hle_enabled || dcs.transfer.state == 0))
		result |= (*dcs.fifo_status_r)(dcs.cpu) & 0x38;
	if (dcs.transfer.hle_enabled && dcs.transfer.state != 0)
		result |= 0x08;
	return result;
}


static READ16_HANDLER( fifo_input_r )
{
	if (dcs.fifo_data_r)
		return (*dcs.fifo_data_r)(dcs.cpu);
	else
		return 0xffff;
}



/***************************************************************************
    INPUT LATCH (data from host to DCS)
****************************************************************************/

static void dcs_delayed_data_w(running_machine &machine, int data)
{
	if (LOG_DCS_IO)
		logerror("%s:dcs_data_w(%04X)\n", machine.describe_context(), data);

	/* boost the interleave temporarily */
	machine.scheduler().boost_interleave(attotime::from_nsec(500), attotime::from_usec(5));

	/* set the IRQ line on the ADSP */
	device_set_input_line(dcs.cpu, ADSP2105_IRQ2, ASSERT_LINE);

	/* indicate we are no longer empty */
	if (dcs.last_input_empty && dcs.input_empty_cb)
		(*dcs.input_empty_cb)(machine, dcs.last_input_empty = 0);
	SET_INPUT_FULL();

	/* set the data */
	dcs.input_data = data;
}


static TIMER_CALLBACK( dcs_delayed_data_w_callback )
{
	dcs_delayed_data_w(machine, param);
}


void dcs_data_w(running_machine &machine, int data)
{
	/* preprocess the write */
	if (preprocess_write(machine, data))
		return;

	/* if we are DCS1, set a timer to latch the data */
	if (dcs.sport_timer == NULL)
		machine.scheduler().synchronize(FUNC(dcs_delayed_data_w_callback), data);
	else
		dcs_delayed_data_w(machine, data);
}


static WRITE16_HANDLER( input_latch_ack_w )
{
	if (!dcs.last_input_empty && dcs.input_empty_cb)
		(*dcs.input_empty_cb)(space->machine(), dcs.last_input_empty = 1);
	SET_INPUT_EMPTY();
	device_set_input_line(dcs.cpu, ADSP2105_IRQ2, CLEAR_LINE);
}


static READ16_HANDLER( input_latch_r )
{
	if (dcs.auto_ack)
		input_latch_ack_w(space,0,0,0xffff);
	if (LOG_DCS_IO)
		logerror("%08X:input_latch_r(%04X)\n", cpu_get_pc(&space->device()), dcs.input_data);
	return dcs.input_data;
}



/***************************************************************************
    OUTPUT LATCH (data from DCS to host)
****************************************************************************/

static TIMER_CALLBACK( latch_delayed_w )
{
	if (!dcs.last_output_full && dcs.output_full_cb)
		(*dcs.output_full_cb)(machine, dcs.last_output_full = 1);
	SET_OUTPUT_FULL();
	dcs.output_data = param;
}


static WRITE16_HANDLER( output_latch_w )
{
	if (LOG_DCS_IO)
		logerror("%08X:output_latch_w(%04X) (empty=%d)\n", cpu_get_pc(&space->device()), data, IS_OUTPUT_EMPTY());
	space->machine().scheduler().synchronize(FUNC(latch_delayed_w), data);
}


static void delayed_ack_w(running_machine &machine)
{
	SET_OUTPUT_EMPTY();
}


static TIMER_CALLBACK( delayed_ack_w_callback )
{
	delayed_ack_w(machine);
}


void dcs_ack_w(running_machine &machine)
{
	machine.scheduler().synchronize(FUNC(delayed_ack_w_callback));
}


int dcs_data_r(running_machine &machine)
{
	/* data is actually only 8 bit (read from d8-d15) */
	if (dcs.last_output_full && dcs.output_full_cb)
		(*dcs.output_full_cb)(machine, dcs.last_output_full = 0);
	if (dcs.auto_ack)
		delayed_ack_w(machine);

	if (LOG_DCS_IO)
		logerror("%s:dcs_data_r(%04X)\n", machine.describe_context(), dcs.output_data);
	return dcs.output_data;
}



/***************************************************************************
    OUTPUT CONTROL BITS (has 3 additional lines to the host)
****************************************************************************/

static TIMER_CALLBACK( output_control_delayed_w )
{
	if (LOG_DCS_IO)
		logerror("output_control = %04X\n", param);
	dcs.output_control = param;
	dcs.output_control_cycles = 0;
}


static WRITE16_HANDLER( output_control_w )
{
	if (LOG_DCS_IO)
		logerror("%04X:output_control = %04X\n", cpu_get_pc(&space->device()), data);
	space->machine().scheduler().synchronize(FUNC(output_control_delayed_w), data);
}


static READ16_HANDLER( output_control_r )
{
	dcs.output_control_cycles = dcs.cpu->total_cycles();
	return dcs.output_control;
}


int dcs_data2_r(running_machine &machine)
{
	return dcs.output_control;
}



/*************************************
 *
 *  Timer management
 *
 *************************************/

static void update_timer_count(running_machine &machine)
{
	UINT64 periods_since_start;
	UINT64 elapsed_cycles;
	UINT64 elapsed_clocks;

	/* if not enabled, skip */
	if (!dcs.timer_enable)
		return;

	/* count cycles */
	elapsed_cycles = dcs.cpu->total_cycles() - dcs.timer_start_cycles;
	elapsed_clocks = elapsed_cycles / dcs.timer_scale;

	/* if we haven't counted past the initial count yet, just do that */
	if (elapsed_clocks < dcs.timer_start_count + 1)
		dcs.control_regs[TIMER_COUNT_REG] = dcs.timer_start_count - elapsed_clocks;

	/* otherwise, count how many periods */
	else
	{
		elapsed_clocks -= dcs.timer_start_count + 1;
		periods_since_start = elapsed_clocks / (dcs.timer_period + 1);
		elapsed_clocks -= periods_since_start * (dcs.timer_period + 1);
		dcs.control_regs[TIMER_COUNT_REG] = dcs.timer_period - elapsed_clocks;
	}
}


static TIMER_DEVICE_CALLBACK( internal_timer_callback )
{
	INT64 target_cycles;

	/* compute the absolute cycle when the next one should fire */
	/* we do this to avoid drifting */
	dcs.timers_fired++;
	target_cycles = dcs.timer_start_cycles + dcs.timer_scale * (dcs.timer_start_count + 1 + dcs.timers_fired * (UINT64)(dcs.timer_period + 1));
	target_cycles -= dcs.cpu->total_cycles();

	/* set the next timer, but only if it's for a reasonable number */
	if (!dcs.timer_ignore && (dcs.timer_period > 10 || dcs.timer_scale > 1))
		timer.adjust(dcs.cpu->cycles_to_attotime(target_cycles));

	/* the IRQ line is edge triggered */
	device_set_input_line(dcs.cpu, ADSP2105_TIMER, ASSERT_LINE);
	device_set_input_line(dcs.cpu, ADSP2105_TIMER, CLEAR_LINE);
}


static void reset_timer(running_machine &machine)
{
	/* if not enabled, skip */
	if (!dcs.timer_enable)
		return;

	/* compute the time until the first firing */
	dcs.timer_start_cycles = dcs.cpu->total_cycles();
	dcs.timers_fired = 0;

	/* if this is the first timer, check the IRQ routine for the DRAM refresh stub */
	/* if that's all the timer does, we don't really need to fire */
	if (!dcs.timer_ignore)
	{
		/* Road Burners: @ 28: JMP $0032  18032F, same code at $32 */

		if (dcs.program->read_dword(0x18*4) == 0x0c0030 &&		/* ENA SEC_REG */
			dcs.program->read_dword(0x19*4) == 0x804828 &&		/* SI = DM($0482) */
			dcs.program->read_dword(0x1a*4) == 0x904828 &&		/* DM($0482) = SI */
			dcs.program->read_dword(0x1b*4) == 0x0C0020 &&		/* DIS SEC_REG */
			dcs.program->read_dword(0x1c*4) == 0x0A001F)			/* RTI */
		{
			dcs.timer_ignore = TRUE;
		}
	}

	/* adjust the timer if not optimized */
	if (!dcs.timer_ignore)
		dcs.internal_timer->adjust(dcs.cpu->cycles_to_attotime(dcs.timer_scale * (dcs.timer_start_count + 1)));
}


static void timer_enable_callback(adsp21xx_device &device, int enable)
{
	dcs.timer_enable = enable;
	dcs.timer_ignore = 0;
	if (enable)
	{
		//mame_printf_debug("Timer enabled @ %d cycles/int, or %f Hz\n", dcs.timer_scale * (dcs.timer_period + 1), 1.0 / dcs.cpu->cycles_to_attotime(dcs.timer_scale * (dcs.timer_period + 1)));
		reset_timer(device.machine());
	}
	else
	{
		//mame_printf_debug("Timer disabled\n");
		dcs.internal_timer->reset();
	}
}



/***************************************************************************
    ADSP CONTROL & TRANSMIT CALLBACK
****************************************************************************/

/*
    The ADSP2105 memory map when in boot rom mode is as follows:

    Program Memory:
    0x0000-0x03ff = Internal Program Ram (contents of boot rom gets copied here)
    0x0400-0x07ff = Reserved
    0x0800-0x3fff = External Program Ram

    Data Memory:
    0x0000-0x03ff = External Data - 0 Waitstates
    0x0400-0x07ff = External Data - 1 Waitstates
    0x0800-0x2fff = External Data - 2 Waitstates
    0x3000-0x33ff = External Data - 3 Waitstates
    0x3400-0x37ff = External Data - 4 Waitstates
    0x3800-0x39ff = Internal Data Ram
    0x3a00-0x3bff = Reserved (extra internal ram space on ADSP2101, etc)
    0x3c00-0x3fff = Memory Mapped control registers & reserved.
*/

static READ16_HANDLER( adsp_control_r )
{
	UINT16 result = 0xffff;

	switch (offset)
	{
		case PROG_FLAG_DATA_REG:
			/* Denver waits for this & 0x000e == 0x0000 */
			/* Denver waits for this & 0x000e == 0x0006 */
			result = dcs.progflags ^= 0x0006;
			break;

		case IDMA_CONTROL_REG:
			result = downcast<adsp2181_device *>(dcs.cpu)->idma_addr_r();
			break;

		case TIMER_COUNT_REG:
			update_timer_count(space->machine());
			result = dcs.control_regs[offset];
			break;

		default:
			result = dcs.control_regs[offset];
			break;
	}
	return result;
}


static WRITE16_HANDLER( adsp_control_w )
{
	dcs.control_regs[offset] = data;

	switch (offset)
	{
		case SYSCONTROL_REG:
			/* bit 9 forces a reset */
			if (data & 0x0200)
			{
				logerror("%04X:Rebooting DCS due to SYSCONTROL write\n", cpu_get_pc(&space->device()));
				device_set_input_line(dcs.cpu, INPUT_LINE_RESET, PULSE_LINE);
				dcs_boot(space->machine());
				dcs.control_regs[SYSCONTROL_REG] = 0;
			}

			/* see if SPORT1 got disabled */
			if ((data & 0x0800) == 0)
			{
				dmadac_enable(&dcs.dmadac[0], dcs.channels, 0);
				dcs.reg_timer->reset();
			}
			break;

		case S1_AUTOBUF_REG:
			/* autobuffer off: nuke the timer, and disable the DAC */
			if ((data & 0x0002) == 0)
			{
				dmadac_enable(&dcs.dmadac[0], dcs.channels, 0);
				dcs.reg_timer->reset();
			}
			break;

		case S1_CONTROL_REG:
			if (((data >> 4) & 3) == 2)
				logerror("DCS: Oh no!, the data is compresed with u-law encoding\n");
			if (((data >> 4) & 3) == 3)
				logerror("DCS: Oh no!, the data is compresed with A-law encoding\n");
			break;

		case TIMER_SCALE_REG:
			data = (data & 0xff) + 1;
			if (data != dcs.timer_scale)
			{
				update_timer_count(space->machine());
				dcs.timer_scale = data;
				reset_timer(space->machine());
			}
			break;

		case TIMER_COUNT_REG:
			dcs.timer_start_count = data;
			reset_timer(space->machine());
			break;

		case TIMER_PERIOD_REG:
			if (data != dcs.timer_period)
			{
				update_timer_count(space->machine());
				dcs.timer_period = data;
				reset_timer(space->machine());
			}
			break;

		case IDMA_CONTROL_REG:
			downcast<adsp2181_device *>(dcs.cpu)->idma_addr_w(data);
			break;
	}
}


/***************************************************************************
    DCS IRQ GENERATION CALLBACKS
****************************************************************************/

static TIMER_DEVICE_CALLBACK( dcs_irq )
{
	/* get the index register */
	int reg = cpu_get_reg(dcs.cpu, ADSP2100_I0 + dcs.ireg);

	/* copy the current data into the buffer */
	{
		int count = dcs.size / 2;
		INT16 buffer[0x400];
		int i;

		for (i = 0; i < count; i++)
		{
			buffer[i] = dcs.data->read_word(reg * 2);
			reg += dcs.incs;
		}

		if (dcs.channels)
			dmadac_transfer(&dcs.dmadac[0], dcs.channels, 1, dcs.channels, (dcs.size / 2) / dcs.channels, buffer);
	}

	/* check for wrapping */
	if (reg >= dcs.ireg_base + dcs.size)
	{
		/* reset the base pointer */
		reg = dcs.ireg_base;

		/* generate the (internal, thats why the pulse) irq */
		generic_pulse_irq_line(dcs.cpu,  ADSP2105_IRQ1, 1);
	}

	/* store it */
	dcs.cpu->set_state(ADSP2100_I0 + dcs.ireg, reg);
}


static TIMER_DEVICE_CALLBACK( sport0_irq )
{
	/* this latches internally, so we just pulse */
	/* note that there is non-interrupt code that reads/modifies/writes the output_control */
	/* register; if we don't interlock it, we will eventually lose sound (see CarnEvil) */
	/* so we skip the SPORT interrupt if we read with output_control within the last 5 cycles */
	if ((dcs.cpu->total_cycles() - dcs.output_control_cycles) > 5)
	{
		device_set_input_line(dcs.cpu, ADSP2115_SPORT0_RX, ASSERT_LINE);
		device_set_input_line(dcs.cpu, ADSP2115_SPORT0_RX, CLEAR_LINE);
	}
}


static void recompute_sample_rate(running_machine &machine)
{
	/* calculate how long until we generate an interrupt */

	/* frequency the time per each bit sent */
	attotime sample_period = attotime::from_hz(dcs.cpu->unscaled_clock()) * (2 * (dcs.control_regs[S1_SCLKDIV_REG] + 1));

	/* now put it down to samples, so we know what the channel frequency has to be */
	sample_period = sample_period * (16 * dcs.channels);
	dmadac_set_frequency(&dcs.dmadac[0], dcs.channels, ATTOSECONDS_TO_HZ(sample_period.attoseconds));
	dmadac_enable(&dcs.dmadac[0], dcs.channels, 1);

	/* fire off a timer wich will hit every half-buffer */
	if (dcs.incs)
	{
		attotime period = (sample_period * dcs.size) / (2 * dcs.channels * dcs.incs);
		dcs.reg_timer->adjust(period, 0, period);
	}
}


static void sound_tx_callback(adsp21xx_device &device, int port, INT32 data)
{
	/* check if it's for SPORT1 */
	if (port != 1)
		return;

	/* check if SPORT1 is enabled */
	if (dcs.control_regs[SYSCONTROL_REG] & 0x0800) /* bit 11 */
	{
		/* we only support autobuffer here (wich is what this thing uses), bail if not enabled */
		if (dcs.control_regs[S1_AUTOBUF_REG] & 0x0002) /* bit 1 */
		{
			/* get the autobuffer registers */
			int		mreg, lreg;
			UINT16	source;

			dcs.ireg = (dcs.control_regs[S1_AUTOBUF_REG] >> 9) & 7;
			mreg = (dcs.control_regs[S1_AUTOBUF_REG] >> 7) & 3;
			mreg |= dcs.ireg & 0x04; /* msb comes from ireg */
			lreg = dcs.ireg;

			/* now get the register contents in a more legible format */
			/* we depend on register indexes to be continuous (wich is the case in our core) */
			source = device.state(ADSP2100_I0 + dcs.ireg);
			dcs.incs = device.state(ADSP2100_M0 + mreg);
			dcs.size = device.state(ADSP2100_L0 + lreg);

			/* get the base value, since we need to keep it around for wrapping */
			source -= dcs.incs;

			/* make it go back one so we dont lose the first sample */
			device.set_state(ADSP2100_I0 + dcs.ireg, source);

			/* save it as it is now */
			dcs.ireg_base = source;

			/* recompute the sample rate and timer */
			recompute_sample_rate(device.machine());
			return;
		}
		else
			logerror( "ADSP SPORT1: trying to transmit and autobuffer not enabled!\n" );
	}

	/* if we get there, something went wrong. Disable playing */
	dmadac_enable(&dcs.dmadac[0], dcs.channels, 0);

	/* remove timer */
	dcs.reg_timer->reset();
}



/***************************************************************************
    VERY BASIC & SAFE OPTIMIZATIONS
****************************************************************************/

static READ16_HANDLER( dcs_polling_r )
{
	if (dcs.polling_count++ > 5)
		device_eat_cycles(&space->device(), 10000);
	return *dcs.polling_base;
}


static WRITE16_HANDLER( dcs_polling_w )
{
	dcs.polling_count = 0;
	COMBINE_DATA(dcs.polling_base);
}



/***************************************************************************
    DATA TRANSFER HLE MECHANISM
****************************************************************************/

void dcs_fifo_notify(running_machine &machine, int count, int max)
{
	hle_transfer_state &transfer = dcs.transfer;

	/* skip if not in mid-transfer */
	if (!transfer.hle_enabled || transfer.state == 0 || !dcs.fifo_data_r)
	{
		transfer.fifo_entries = 0;
		return;
	}

	/* preprocess a word */
	transfer.fifo_entries = count;
	if (transfer.state != 5 || transfer.fifo_entries == transfer.writes_left || transfer.fifo_entries >= 256)
	{
		for ( ; transfer.fifo_entries; transfer.fifo_entries--)
			preprocess_write(machine, (*dcs.fifo_data_r)(dcs.cpu));
	}
}


static TIMER_DEVICE_CALLBACK( transfer_watchdog_callback )
{
	hle_transfer_state &transfer = dcs.transfer;
	int starting_writes_left = param;

	if (transfer.fifo_entries && starting_writes_left == transfer.writes_left)
	{
		for ( ; transfer.fifo_entries; transfer.fifo_entries--)
			preprocess_write(timer.machine(), (*dcs.fifo_data_r)(dcs.cpu));
	}
	if (transfer.watchdog != NULL)
		transfer.watchdog->adjust(attotime::from_msec(1), transfer.writes_left);
}


static TIMER_CALLBACK( s1_ack_callback2 )
{
	/* if the output is full, stall for a usec */
	if (IS_OUTPUT_FULL())
	{
		machine.scheduler().timer_set(attotime::from_usec(1), FUNC(s1_ack_callback2), param);
		return;
	}
	output_latch_w(dcs.cpu->memory().space(AS_PROGRAM), 0, 0x000a, 0xffff);
}


static TIMER_CALLBACK( s1_ack_callback1 )
{
	/* if the output is full, stall for a usec */
	if (IS_OUTPUT_FULL())
	{
		machine.scheduler().timer_set(attotime::from_usec(1), FUNC(s1_ack_callback1), param);
		return;
	}
	output_latch_w(dcs.cpu->memory().space(AS_PROGRAM), 0, param, 0xffff);

	/* chain to the next word we need to write back */
	machine.scheduler().timer_set(attotime::from_usec(1), FUNC(s1_ack_callback2));
}


static int preprocess_stage_1(running_machine &machine, UINT16 data)
{
	hle_transfer_state &transfer = dcs.transfer;

	switch (transfer.state)
	{
		case 0:
			/* look for command 0x001a to transfer chunks of data */
			if (data == 0x001a)
			{
				if (LOG_DCS_TRANSFERS)
					logerror("%s:DCS Transfer command %04X\n", machine.describe_context(), data);
				transfer.state++;
				if (transfer.hle_enabled)
					return 1;
			}

			/* look for command 0x002a to start booting the uploaded program */
			else if (data == 0x002a)
			{
				if (LOG_DCS_TRANSFERS)
					logerror("%s:DCS State change %04X\n", machine.describe_context(), data);
				transfer.dcs_state = 1;
			}

			/* anything else is ignored */
			else
			{
				if (LOG_DCS_TRANSFERS)
					logerror("Command: %04X\n", data);
			}
			break;

		case 1:
			/* first word is the start address */
			transfer.start = data;
			transfer.state++;
			if (LOG_DCS_TRANSFERS)
				logerror("Start address = %04X\n", transfer.start);
			if (transfer.hle_enabled)
				return 1;
			break;

		case 2:
			/* second word is the stop address */
			transfer.stop = data;
			transfer.state++;
			if (LOG_DCS_TRANSFERS)
				logerror("Stop address = %04X\n", transfer.stop);
			if (transfer.hle_enabled)
				return 1;
			break;

		case 3:
			/* third word is the transfer type */
			/* transfer type 0 = program memory */
			/* transfer type 1 = SRAM bank 0 */
			/* transfer type 2 = SRAM bank 1 */
			transfer.type = data;
			transfer.state++;
			if (LOG_DCS_TRANSFERS) logerror("Transfer type = %04X\n", transfer.type);

			/* at this point, we can compute how many words to expect for the transfer */
			transfer.writes_left = transfer.stop - transfer.start + 1;
			if (transfer.type == 0)
				transfer.writes_left *= 2;

			/* reset the checksum */
			transfer.sum = 0;

			/* handle the HLE case */
			if (transfer.hle_enabled)
			{
				if (transfer.type == 1 && SDRC_SM_BK == 1)
				{
					dcs.sdrc.reg[0] &= ~0x1000;
					sdrc_remap_memory(machine);
				}
				if (transfer.type == 2 && SDRC_SM_BK == 0)
				{
					dcs.sdrc.reg[0] |= 0x1000;
					sdrc_remap_memory(machine);
				}
				return 1;
			}
			break;

		case 4:
			/* accumulate the sum over all data */
			transfer.sum += data;

			/* if we're out, stop the transfer */
			if (--transfer.writes_left == 0)
			{
				if (LOG_DCS_TRANSFERS) logerror("Transfer done, sum = %04X\n", transfer.sum);
				transfer.state = 0;
			}

			/* handle the HLE case */
			if (transfer.hle_enabled)
			{
				/* write the new data to memory */
				if (transfer.type == 0)
				{
					if (transfer.writes_left & 1)
						transfer.temp = data;
					else
						dcs.program->write_dword(transfer.start++ * 4, (transfer.temp << 8) | (data & 0xff));
				}
				else
					dcs.data->write_word(transfer.start++ * 2, data);

				/* if we're done, start a timer to send the response words */
				if (transfer.state == 0)
					machine.scheduler().timer_set(attotime::from_usec(1), FUNC(s1_ack_callback1), transfer.sum);
				return 1;
			}
			break;
	}
	return 0;
}


static TIMER_CALLBACK( s2_ack_callback )
{
	address_space *space = dcs.cpu->memory().space(AS_PROGRAM);

	/* if the output is full, stall for a usec */
	if (IS_OUTPUT_FULL())
	{
		machine.scheduler().timer_set(attotime::from_usec(1), FUNC(s2_ack_callback), param);
		return;
	}
	output_latch_w(space, 0, param, 0xffff);
	output_control_w(space, 0, (dcs.output_control & ~0xff00) | 0x0300, 0xffff);
}


static int preprocess_stage_2(running_machine &machine, UINT16 data)
{
	hle_transfer_state &transfer = dcs.transfer;

	switch (transfer.state)
	{
		case 0:
			/* look for command 0x55d0 or 0x55d1 to transfer chunks of data */
			if (data == 0x55d0 || data == 0x55d1)
			{
				if (LOG_DCS_TRANSFERS)
					logerror("%s:DCS Transfer command %04X\n", machine.describe_context(), data);
				transfer.state++;
				if (transfer.hle_enabled)
					return 1;
			}

			/* anything else is ignored */
			else
			{
				if (LOG_DCS_TRANSFERS)
					logerror("%s:Command: %04X\n", machine.describe_context(), data);
			}
			break;

		case 1:
			/* first word is the upper bits of the start address */
			transfer.start = data << 16;
			transfer.state++;
			if (transfer.hle_enabled)
				return 1;
			break;

		case 2:
			/* second word is the lower bits of the start address */
			transfer.start |= data;
			transfer.state++;
			if (LOG_DCS_TRANSFERS)
				logerror("Start address = %08X\n", transfer.start);
			if (transfer.hle_enabled)
				return 1;
			break;

		case 3:
			/* third word is the upper bits of the stop address */
			transfer.stop = data << 16;
			transfer.state++;
			if (transfer.hle_enabled)
				return 1;
			break;

		case 4:
			/* fourth word is the lower bits of the stop address */
			transfer.stop |= data;
			transfer.state++;
			if (LOG_DCS_TRANSFERS)
				logerror("Stop address = %08X\n", transfer.stop);

			/* at this point, we can compute how many words to expect for the transfer */
			transfer.writes_left = transfer.stop - transfer.start + 1;

			/* reset the checksum */
			transfer.sum = 0;
			if (transfer.hle_enabled)
			{
				transfer.watchdog->adjust(attotime::from_msec(1), transfer.writes_left);
				return 1;
			}
			break;

		case 5:
			/* accumulate the sum over all data */
			transfer.sum += data;

			/* if we're out, stop the transfer */
			if (--transfer.writes_left == 0)
			{
				if (LOG_DCS_TRANSFERS)
					logerror("Transfer done, sum = %04X\n", transfer.sum);
				transfer.state = 0;
			}

			/* handle the HLE case */
			if (transfer.hle_enabled)
			{
				/* write the new data to memory */
				dcs.sounddata[transfer.start++] = data;

				/* if we're done, start a timer to send the response words */
				if (transfer.state == 0)
				{
					machine.scheduler().timer_set(attotime::from_usec(1), FUNC(s2_ack_callback), transfer.sum);
					transfer.watchdog->reset();
				}
				return 1;
			}
			break;
	}
	return 0;
}


static int preprocess_write(running_machine &machine, UINT16 data)
{
	hle_transfer_state &transfer = dcs.transfer;
	int result;

	/* if we're not DCS2, skip */
	if (dcs.sport_timer == NULL)
		return 0;

	/* state 0 - initialization phase */
	if (transfer.dcs_state == 0)
		result = preprocess_stage_1(machine, data);
	else
		result = preprocess_stage_2(machine, data);

	/* if we did the write, toggle the full/not full state so interrupts are generated */
	if (result && dcs.input_empty_cb)
	{
		if (dcs.last_input_empty)
			(*dcs.input_empty_cb)(machine, dcs.last_input_empty = 0);
		if (!dcs.last_input_empty)
			(*dcs.input_empty_cb)(machine, dcs.last_input_empty = 1);
	}
	return result;
}
