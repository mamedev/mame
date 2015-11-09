// license:BSD-3-Clause
// copyright-holders:Aaron Giles
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
        * NBA Jam Extreme
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
#include "dcs.h"


#define LOG_DCS_TRANSFERS           (0)
#define LOG_DCS_IO                  (0)
#define LOG_BUFFER_FILLING          (0)

#define ENABLE_HLE_TRANSFERS        (1)



/*************************************
 *
 *  Constants
 *
 *************************************/

#define LCTRL_OUTPUT_EMPTY          0x400
#define LCTRL_INPUT_EMPTY           0x800

#define IS_OUTPUT_EMPTY()           (m_latch_control & LCTRL_OUTPUT_EMPTY)
#define IS_OUTPUT_FULL()            (!(m_latch_control & LCTRL_OUTPUT_EMPTY))
#define SET_OUTPUT_EMPTY()          (m_latch_control |= LCTRL_OUTPUT_EMPTY)
#define SET_OUTPUT_FULL()           (m_latch_control &= ~LCTRL_OUTPUT_EMPTY)

#define IS_INPUT_EMPTY()            (m_latch_control & LCTRL_INPUT_EMPTY)
#define IS_INPUT_FULL()             (!(m_latch_control & LCTRL_INPUT_EMPTY))
#define SET_INPUT_EMPTY()           (m_latch_control |= LCTRL_INPUT_EMPTY)
#define SET_INPUT_FULL()            (m_latch_control &= ~LCTRL_INPUT_EMPTY)


/* These are some of the control registers. We don't use them all */
enum
{
	IDMA_CONTROL_REG = 0,   /* 3fe0 */
	BDMA_INT_ADDR_REG,      /* 3fe1 */
	BDMA_EXT_ADDR_REG,      /* 3fe2 */
	BDMA_CONTROL_REG,       /* 3fe3 */
	BDMA_WORD_COUNT_REG,    /* 3fe4 */
	PROG_FLAG_DATA_REG,     /* 3fe5 */
	PROG_FLAG_CONTROL_REG,  /* 3fe6 */

	S1_AUTOBUF_REG = 15,    /* 3fef */
	S1_RFSDIV_REG,          /* 3ff0 */
	S1_SCLKDIV_REG,         /* 3ff1 */
	S1_CONTROL_REG,         /* 3ff2 */
	S0_AUTOBUF_REG,         /* 3ff3 */
	S0_RFSDIV_REG,          /* 3ff4 */
	S0_SCLKDIV_REG,         /* 3ff5 */
	S0_CONTROL_REG,         /* 3ff6 */
	S0_MCTXLO_REG,          /* 3ff7 */
	S0_MCTXHI_REG,          /* 3ff8 */
	S0_MCRXLO_REG,          /* 3ff9 */
	S0_MCRXHI_REG,          /* 3ffa */
	TIMER_SCALE_REG,        /* 3ffb */
	TIMER_COUNT_REG,        /* 3ffc */
	TIMER_PERIOD_REG,       /* 3ffd */
	WAITSTATES_REG,         /* 3ffe */
	SYSCONTROL_REG          /* 3fff */
};


/* these macros are used to reference the SDRC ASIC */
#define SDRC_ROM_ST     ((m_sdrc.reg[0] >> 0) & 3)    /* 0=0000, 1=3000, 2=3400, 3=none */
#define SDRC_ROM_SZ     ((m_sdrc.reg[0] >> 4) & 1)    /* 0=4k, 1=1k */
#define SDRC_ROM_MS     ((m_sdrc.reg[0] >> 5) & 1)    /* 0=/BMS, 1=/DMS */
#define SDRC_ROM_PG     ((m_sdrc.reg[0] >> 7) & 7)
#define SDRC_SM_EN      ((m_sdrc.reg[0] >> 11) & 1)
#define SDRC_SM_BK      ((m_sdrc.reg[0] >> 12) & 1)
#define SDRC_SMODE      ((m_sdrc.reg[0] >> 13) & 7)

#define SDRC_DM_ST      ((m_sdrc.reg[1] >> 0) & 3)    /* 0=none, 1=0000, 2=3000, 3=3400 */
#define SDRC_DM_REF     ((m_sdrc.reg[1] >> 4) & 3)
#define SDRC_DM_3WS     ((m_sdrc.reg[1] >> 7) & 1)
#define SDRC_TFS_INV    ((m_sdrc.reg[1] >> 8) & 1)
#define SDRC_RES_TFS    ((m_sdrc.reg[1] >> 10) & 3)
#define SDRC_LED        ((m_sdrc.reg[1] >> 13) & 1)
#define SDRC_MUTE       ((m_sdrc.reg[1] >> 14) & 1)
#define SDRC_AREF_ACT   ((m_sdrc.reg[1] >> 15) & 1)

#define SDRC_DM_PG      ((m_sdrc.reg[2] >> 0) & 0x7ff)
#define SDRC_EPM_PG     ((m_sdrc.reg[2] >> 0) & 0x1fff)


/* these macros are used to reference the DSIO ASIC */
#define DSIO_EMPTY_FIFO ((m_dsio.reg[1] >> 0) & 1)
#define DSIO_CUR_OUTPUT ((m_dsio.reg[1] >> 4) & 1)
#define DSIO_RES_TFS    ((m_dsio.reg[1] >> 10) & 1)
#define DSIO_LED        ((m_dsio.reg[1] >> 13) & 1)
#define DSIO_MUTE       ((m_dsio.reg[1] >> 14) & 1)

#define DSIO_DM_PG      ((m_dsio.reg[2] >> 0) & 0x7ff)


/* these macros are used to reference the DENVER ASIC */
#define DENV_DSP_SPEED  ((m_dsio.reg[1] >> 2) & 3)    /* read only: 1=33.33MHz */
#define DENV_RES_TFS    ((m_dsio.reg[1] >> 10) & 1)
#define DENV_CHANNELS   ((m_dsio.reg[1] >> 11) & 3)   /* 0=2ch, 1=4ch, 2=6ch */
#define DENV_LED        ((m_dsio.reg[1] >> 13) & 1)
#define DENV_MUTE       ((m_dsio.reg[1] >> 14) & 1)

#define DENV_DM_PG      ((m_dsio.reg[2] >> 0) & 0x7ff)


/*************************************
 *
 *  Original DCS Memory Maps
 *
 *************************************/

/* DCS 2k memory map */
static ADDRESS_MAP_START( dcs_2k_program_map, AS_PROGRAM, 32, dcs_audio_device )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("dcsint")
	AM_RANGE(0x0800, 0x0fff) AM_RAM AM_SHARE("dcsext")
	AM_RANGE(0x1000, 0x17ff) AM_RAM AM_SHARE("dcsext")
	AM_RANGE(0x1800, 0x1fff) AM_RAM AM_SHARE("dcsext")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dcs_2k_data_map, AS_DATA, 16, dcs_audio_device )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x1800) AM_READWRITE(dcs_dataram_r, dcs_dataram_w)
	AM_RANGE(0x2000, 0x2fff) AM_ROMBANK("databank")
	AM_RANGE(0x3000, 0x33ff) AM_WRITE(dcs_data_bank_select_w)
	AM_RANGE(0x3400, 0x37ff) AM_READWRITE(input_latch_r, output_latch_w)
	AM_RANGE(0x3800, 0x39ff) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END


/* DCS 2k with UART memory map */
static ADDRESS_MAP_START( dcs_2k_uart_data_map, AS_DATA, 16, dcs_audio_device )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x1800) AM_READWRITE(dcs_dataram_r, dcs_dataram_w)
	AM_RANGE(0x2000, 0x2fff) AM_ROMBANK("databank")
	AM_RANGE(0x3000, 0x33ff) AM_WRITE(dcs_data_bank_select_w)
	AM_RANGE(0x3400, 0x3402) AM_NOP                             /* UART (ignored) */
	AM_RANGE(0x3403, 0x3403) AM_READWRITE(input_latch_r, output_latch_w)
	AM_RANGE(0x3404, 0x3405) AM_NOP                             /* UART (ignored) */
	AM_RANGE(0x3800, 0x39ff) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END


/* DCS 8k memory map */
static ADDRESS_MAP_START( dcs_8k_program_map, AS_PROGRAM, 32, dcs_audio_device )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("dcsint")
	AM_RANGE(0x0800, 0x1fff) AM_RAM AM_SHARE("dcsext")
	AM_RANGE(0x3000, 0x3003) AM_READWRITE(input_latch32_r, output_latch32_w) // why?
ADDRESS_MAP_END

static ADDRESS_MAP_START( dcs_8k_data_map, AS_DATA, 16, dcs_audio_device )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x1fff) AM_READWRITE(dcs_dataram_r, dcs_dataram_w)
	AM_RANGE(0x2000, 0x2fff) AM_ROMBANK("databank")
	AM_RANGE(0x3000, 0x3000) AM_WRITE(dcs_data_bank_select_w)
	AM_RANGE(0x3400, 0x3403) AM_READWRITE(input_latch_r, output_latch_w) // mk3 etc. need this
	AM_RANGE(0x3800, 0x39ff) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END

/* Williams WPC DCS/Security Pinball */
static ADDRESS_MAP_START( dcs_wpc_program_map, AS_PROGRAM, 32, dcs_audio_device )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("dcsint")
	AM_RANGE(0x1000, 0x3fff) AM_RAM AM_SHARE("dcsext")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dcs_wpc_data_map, AS_DATA, 16, dcs_audio_wpc_device )
	AM_RANGE(0x0000, 0x07ff) AM_ROMBANK("databank")
	AM_RANGE(0x1000, 0x2fff) AM_READWRITE(dcs_dataram_r, dcs_dataram_w)
	AM_RANGE(0x3000, 0x3000) AM_WRITE(dcs_data_bank_select_w)
	AM_RANGE(0x3100, 0x3100) AM_WRITE(dcs_data_bank_select2_w)
	AM_RANGE(0x3300, 0x3303) AM_READWRITE(input_latch_r, output_latch_w)
	AM_RANGE(0x3800, 0x39ff) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END

/*************************************
 *
 *  DCS2 Memory Maps
 *
 *************************************/

static ADDRESS_MAP_START( dcs2_2115_program_map, AS_PROGRAM, 32, dcs_audio_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("dcsint")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dcs2_2104_program_map, AS_PROGRAM, 32, dcs_audio_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x01ff) AM_RAM AM_SHARE("dcsint")
ADDRESS_MAP_END


static ADDRESS_MAP_START( dcs2_2115_data_map, AS_DATA, 16, dcs_audio_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0400, 0x0400) AM_READWRITE(input_latch_r, input_latch_ack_w)
	AM_RANGE(0x0401, 0x0401) AM_WRITE(output_latch_w)
	AM_RANGE(0x0402, 0x0402) AM_READWRITE(output_control_r, output_control_w)
	AM_RANGE(0x0403, 0x0403) AM_READ(latch_status_r)
	AM_RANGE(0x0404, 0x0407) AM_READ(fifo_input_r)
	AM_RANGE(0x0480, 0x0483) AM_READWRITE(sdrc_r, sdrc_w)
	AM_RANGE(0x3800, 0x39ff) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dcs2_2104_data_map, AS_DATA, 16, dcs_audio_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0400, 0x0400) AM_READWRITE(input_latch_r, input_latch_ack_w)
	AM_RANGE(0x0401, 0x0401) AM_WRITE(output_latch_w)
	AM_RANGE(0x0402, 0x0402) AM_READWRITE(output_control_r, output_control_w)
	AM_RANGE(0x0403, 0x0403) AM_READ(latch_status_r)
	AM_RANGE(0x0404, 0x0407) AM_READ(fifo_input_r)
	AM_RANGE(0x0480, 0x0483) AM_READWRITE(sdrc_r, sdrc_w)
	AM_RANGE(0x3800, 0x38ff) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END



/*************************************
 *
 *  DSIO Memory Maps
 *
 *************************************/

static ADDRESS_MAP_START( dsio_program_map, AS_PROGRAM, 32, dcs_audio_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("dcsint")
ADDRESS_MAP_END


static ADDRESS_MAP_START( dsio_data_map, AS_DATA, 16, dcs_audio_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_RAMBANK("databank")
	AM_RANGE(0x0400, 0x3fdf) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( dsio_io_map, AS_IO, 16, dcs_audio_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0400, 0x0400) AM_READWRITE(input_latch_r, input_latch_ack_w)
	AM_RANGE(0x0401, 0x0401) AM_WRITE(output_latch_w)
	AM_RANGE(0x0402, 0x0402) AM_READWRITE(output_control_r, output_control_w)
	AM_RANGE(0x0403, 0x0403) AM_READ(latch_status_r)
	AM_RANGE(0x0404, 0x0407) AM_READ(fifo_input_r)
	AM_RANGE(0x0480, 0x0483) AM_READWRITE(dsio_r, dsio_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Denver Memory Maps
 *
 *************************************/

static ADDRESS_MAP_START( denver_program_map, AS_PROGRAM, 32, dcs_audio_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("dcsint")
ADDRESS_MAP_END


static ADDRESS_MAP_START( denver_data_map, AS_DATA, 16, dcs_audio_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_RAMBANK("databank")
	AM_RANGE(0x0800, 0x3fdf) AM_RAM
	AM_RANGE(0x3fe0, 0x3fff) AM_READWRITE(adsp_control_r, adsp_control_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( denver_io_map, AS_IO, 16, dcs_audio_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0400, 0x0400) AM_READWRITE(input_latch_r, input_latch_ack_w)
	AM_RANGE(0x0401, 0x0401) AM_WRITE(output_latch_w)
	AM_RANGE(0x0402, 0x0402) AM_READWRITE(output_control_r, output_control_w)
	AM_RANGE(0x0403, 0x0403) AM_READ(latch_status_r)
	AM_RANGE(0x0404, 0x0407) AM_READ(fifo_input_r)
	AM_RANGE(0x0480, 0x0483) AM_READWRITE(denver_r, denver_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Original DCS Machine Drivers
 *
 *************************************/

/* Basic DCS system with ADSP-2105 and 2k of SRAM (T-unit, V-unit, Killer Instinct) */
MACHINE_CONFIG_FRAGMENT( dcs_audio_2k )
	MCFG_CPU_ADD("dcs", ADSP2105, XTAL_10MHz)
	MCFG_ADSP21XX_SPORT_TX_CB(WRITE32(dcs_audio_device, sound_tx_callback))      /* callback for serial transmit */
	MCFG_ADSP21XX_TIMER_FIRED_CB(WRITELINE(dcs_audio_device,timer_enable_callback))   /* callback for timer fired */
	MCFG_CPU_PROGRAM_MAP(dcs_2k_program_map)
	MCFG_CPU_DATA_MAP(dcs_2k_data_map)

	MCFG_TIMER_DEVICE_ADD("dcs_reg_timer", DEVICE_SELF, dcs_audio_device, dcs_irq)
	MCFG_TIMER_DEVICE_ADD("dcs_int_timer", DEVICE_SELF, dcs_audio_device, internal_timer_callback)

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

MACHINE_CONFIG_DERIVED( dcs_audio_wpc, dcs_audio_2k )
	MCFG_CPU_MODIFY("dcs")
	MCFG_CPU_PROGRAM_MAP(dcs_wpc_program_map)
	MCFG_CPU_DATA_MAP(dcs_wpc_data_map)
MACHINE_CONFIG_END


/*************************************
 *
 *  DCS2 Machine Drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( dcs2_audio_2115 )
	MCFG_CPU_ADD("dcs2", ADSP2115, XTAL_16MHz)
	MCFG_ADSP21XX_SPORT_TX_CB(WRITE32(dcs_audio_device, sound_tx_callback))      /* callback for serial transmit */
	MCFG_ADSP21XX_TIMER_FIRED_CB(WRITELINE(dcs_audio_device,timer_enable_callback))   /* callback for timer fired */
	MCFG_CPU_PROGRAM_MAP(dcs2_2115_program_map)
	MCFG_CPU_DATA_MAP(dcs2_2115_data_map)

	MCFG_TIMER_DEVICE_ADD("dcs_reg_timer", DEVICE_SELF, dcs_audio_device, dcs_irq)
	MCFG_TIMER_DEVICE_ADD("dcs_sport_timer", DEVICE_SELF, dcs_audio_device, sport0_irq)
	MCFG_TIMER_DEVICE_ADD("dcs_int_timer", DEVICE_SELF, dcs_audio_device, internal_timer_callback)
	MCFG_TIMER_DEVICE_ADD("dcs_hle_timer", DEVICE_SELF, dcs_audio_device, transfer_watchdog_callback)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( dcs2_audio_2104, dcs2_audio_2115 )
	MCFG_CPU_REPLACE("dcs2", ADSP2104, XTAL_16MHz)
	MCFG_ADSP21XX_SPORT_TX_CB(WRITE32(dcs_audio_device, sound_tx_callback))      /* callback for serial transmit */
	MCFG_ADSP21XX_TIMER_FIRED_CB(WRITELINE(dcs_audio_device,timer_enable_callback))   /* callback for timer fired */
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
	MCFG_ADSP21XX_SPORT_TX_CB(WRITE32(dcs_audio_device, sound_tx_callback))      /* callback for serial transmit */
	MCFG_ADSP21XX_TIMER_FIRED_CB(WRITELINE(dcs_audio_device,timer_enable_callback))   /* callback for timer fired */
	MCFG_CPU_PROGRAM_MAP(dsio_program_map)
	MCFG_CPU_DATA_MAP(dsio_data_map)
	MCFG_CPU_IO_MAP(dsio_io_map)

	MCFG_TIMER_DEVICE_ADD("dcs_reg_timer", DEVICE_SELF, dcs_audio_device, dcs_irq)
	MCFG_TIMER_DEVICE_ADD("dcs_int_timer", DEVICE_SELF, dcs_audio_device, internal_timer_callback)

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
	MCFG_ADSP21XX_SPORT_TX_CB(WRITE32(dcs_audio_device, sound_tx_callback))      /* callback for serial transmit */
	MCFG_ADSP21XX_TIMER_FIRED_CB(WRITELINE(dcs_audio_device,timer_enable_callback))   /* callback for timer fired */
	MCFG_CPU_PROGRAM_MAP(denver_program_map)
	MCFG_CPU_DATA_MAP(denver_data_map)
	MCFG_CPU_IO_MAP(denver_io_map)

	MCFG_TIMER_DEVICE_ADD("dcs_reg_timer", DEVICE_SELF, dcs_audio_device, dcs_irq)
	MCFG_TIMER_DEVICE_ADD("dcs_int_timer", DEVICE_SELF, dcs_audio_device, internal_timer_callback)

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

void dcs_audio_device::dcs_boot()
{
	UINT8 buffer[0x1000];
//  UINT32 max_banks;
	UINT16 *base;
	int i;

	switch (m_rev)
	{
		/* rev 1/1.5: use the last set data bank to boot from */
		case 1:
		case 15:

			/* determine the base */
//          max_banks = m_bootrom_words / 0x1000;
			base = m_bootrom + ((m_sounddata_bank * 0x1000) % m_bootrom_words);

			/* convert from 16-bit data to 8-bit data and boot */
			for (i = 0; i < 0x1000; i++)
				buffer[i] = base[i];
			m_cpu->load_boot_data(buffer, m_internal_program_ram);
			break;

		/* rev 2: use the ROM page in the SDRC to boot from */
		case 2:

			/* determine the base */
			if (m_bootrom == m_sounddata)
			{
				/* EPROM case: page is selected from the page register */
				base = m_bootrom + ((SDRC_EPM_PG * 0x1000) % m_bootrom_words);
			}
			else
			{
				/* DRAM case: page is selected from the ROM page register */
				base = m_bootrom + ((SDRC_ROM_PG * 0x1000) % m_bootrom_words);
			}

			/* convert from 16-bit data to 8-bit data and boot */
			for (i = 0; i < 0x1000; i++)
				buffer[i] = base[i];
			m_cpu->load_boot_data(buffer, m_internal_program_ram);
			break;

		/* rev 3/4: HALT the ADSP-2181 until program is downloaded via IDMA */
		case 3:
		case 4:
			m_cpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			m_dsio.start_on_next_write = 0;
			break;
	}
}



/*************************************
 *
 *  System reset
 *
 *************************************/

TIMER_CALLBACK_MEMBER( dcs_audio_device::dcs_reset )
{
	if (LOG_DCS_IO)
		logerror("dcs_reset\n");

	/* reset the memory banking */
	switch (m_rev)
	{
		/* rev 1/1.5: just reset the bank to 0 */
		case 1:
		case 15:
			m_sounddata_bank = 0;
			membank("databank")->set_entry(0);
			break;

		/* rev 2: reset the SDRC ASIC */
		case 2:
			sdrc_reset();
			break;

		/* rev 3: reset the DSIO ASIC */
		case 3:
			dsio_reset();
			break;

		/* rev 4: reset the Denver ASIC */
		case 4:
			denver_reset();
			break;
	}
	/* initialize our state structure and install the transmit callback */
	m_size = 0;
	m_incs = 0;
	m_ireg = 0;

	/* initialize the ADSP control regs */
	memset(m_control_regs, 0, sizeof(m_control_regs));
	/* clear all interrupts */
	m_cpu->set_input_line(ADSP2105_IRQ0, CLEAR_LINE);
	m_cpu->set_input_line(ADSP2105_IRQ1, CLEAR_LINE);
	m_cpu->set_input_line(ADSP2105_IRQ2, CLEAR_LINE);

	/* initialize the comm bits */
	SET_INPUT_EMPTY();
	SET_OUTPUT_EMPTY();
	if (!m_last_input_empty && !m_input_empty_cb.isnull())
		m_input_empty_cb(m_last_input_empty = 1);
	if (m_last_output_full && !m_output_full_cb.isnull())
		m_output_full_cb(m_last_output_full = 0);

	/* boot */
	dcs_boot();

	/* reset timers */
	m_timer_enable = 0;
	m_timer_scale = 1;
	m_internal_timer->reset();

	/* start the SPORT0 timer */
	if (m_sport_timer != NULL)
		m_sport_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));

	/* reset the HLE transfer states */
	m_transfer.dcs_state = m_transfer.state = 0;
}



/*************************************
 *
 *  System setup
 *
 *************************************/

void dcs_audio_device::dcs_register_state()
{
	save_item(NAME(m_sdrc.reg));
	save_item(NAME(m_sdrc.seed));

	save_item(NAME(m_dsio.reg));
	save_item(NAME(m_dsio.start_on_next_write));
	save_item(NAME(m_dsio.channelbits));

	save_item(NAME(m_channels));
	save_item(NAME(m_size));
	save_item(NAME(m_incs));
	save_item(NAME(m_ireg));
	save_item(NAME(m_ireg_base));
	save_item(NAME(m_control_regs));

	save_item(NAME(m_sounddata_bank));

	save_item(NAME(m_auto_ack));
	save_item(NAME(m_latch_control));
	save_item(NAME(m_input_data));
	save_item(NAME(m_output_data));
	save_item(NAME(m_output_control));
	save_item(NAME(m_output_control_cycles));
	save_item(NAME(m_last_output_full));
	save_item(NAME(m_last_input_empty));
	save_item(NAME(m_progflags));

	save_item(NAME(m_timer_enable));
	save_item(NAME(m_timer_ignore));
	save_item(NAME(m_timer_start_cycles));
	save_item(NAME(m_timer_start_count));
	save_item(NAME(m_timer_scale));
	save_item(NAME(m_timer_period));
	save_item(NAME(m_timers_fired));

	save_item(NAME(m_transfer.dcs_state));
	save_item(NAME(m_transfer.state));
	save_item(NAME(m_transfer.start));
	save_item(NAME(m_transfer.stop));
	save_item(NAME(m_transfer.type));
	save_item(NAME(m_transfer.temp));
	save_item(NAME(m_transfer.writes_left));
	save_item(NAME(m_transfer.sum));
	save_item(NAME(m_transfer.fifo_entries));

	if (m_sram != NULL)
		save_pointer(NAME(m_sram), 0x8000*4 / sizeof(m_sram[0]));

	if (m_rev == 2)
		machine().save().register_postload(save_prepost_delegate(FUNC(dcs_audio_device::sdrc_remap_memory), this));
}


//-------------------------------------------------
//  dcs_audio_device - constructor
//-------------------------------------------------

dcs_audio_device::dcs_audio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int rev) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_cpu(NULL),
	m_program(NULL),
	m_data(NULL),
	m_rev(rev),
	m_polling_offset(0),
	m_polling_count(0),
	m_channels(0),
	m_size(0),
	m_incs(0),
	m_reg_timer(NULL),
	m_sport_timer(NULL),
	m_internal_timer(NULL),
	m_ireg(0),
	m_ireg_base(0),
	m_bootrom(NULL),
	m_bootrom_words(0),
	m_sounddata(NULL),
	m_sounddata_words(0),
	m_sounddata_banks(0),
	m_sounddata_bank(0),
	m_auto_ack(0),
	m_latch_control(0),
	m_input_data(0),
	m_output_data(0),
	m_output_control(0),
	m_output_control_cycles(0),
	m_last_output_full(0),
	m_last_input_empty(0),
	m_progflags(0),
	m_timer_enable(0),
	m_timer_ignore(0),
	m_timer_start_cycles(0),
	m_timer_start_count(0),
	m_timer_scale(0),
	m_timer_period(0),
	m_timers_fired(0),
	m_sram(NULL),
	m_polling_base(NULL),
	m_internal_program_ram(NULL),
	m_external_program_ram(NULL),
	m_dram_in_mb(0)
{
	m_dmadac[0] = m_dmadac[1] = m_dmadac[2] = m_dmadac[3] = m_dmadac[4] = m_dmadac[5] = NULL;
	memset(m_control_regs, 0, sizeof(m_control_regs));
	memset(&m_sdrc, 0, sizeof(m_sdrc));
	memset(&m_dsio, 0, sizeof(m_dsio));
	memset(&m_transfer, 0, sizeof(m_transfer));
}

void dcs_audio_device::device_reset()
{
	dcs_reset(NULL, 0);
}

void dcs_audio_device::device_start()
{
	m_sram = NULL;

	m_internal_program_ram = (UINT32 *)memshare("dcsint")->ptr();
	m_external_program_ram = (UINT32 *)memshare("dcsext")->ptr();

	/* find the DCS CPU and the sound ROMs */
	m_cpu = subdevice<adsp21xx_device>("dcs");
	if (m_cpu != NULL && !m_cpu->started())
		throw device_missing_dependencies();

	m_program = &m_cpu->space(AS_PROGRAM);
	m_data = &m_cpu->space(AS_DATA);
	m_channels = 1;
	m_dmadac[0] = subdevice<dmadac_sound_device>("dac");

	/* configure boot and sound ROMs */
	m_bootrom = (UINT16 *)machine().root_device().memregion("dcs")->base();
	m_bootrom_words = machine().root_device().memregion("dcs")->bytes() / 2;
	m_sounddata = m_bootrom;
	m_sounddata_words = m_bootrom_words;
	if (m_rev == 1)
	{
		m_sounddata_banks = m_sounddata_words / 0x1000;
		membank("databank")->configure_entries(0, m_sounddata_banks, m_sounddata, 0x1000*2);
	}
	else
	{
		m_sounddata_banks = m_sounddata_words / 0x800;
		membank("databank")->configure_entries(0, m_sounddata_banks, m_sounddata, 0x800*2);
	}

	/* create the timers */
	m_internal_timer = subdevice<timer_device>("dcs_int_timer");
	m_reg_timer = subdevice<timer_device>("dcs_reg_timer");

	/* non-RAM based automatically acks */
	m_auto_ack = TRUE;
	/* register for save states */
	dcs_register_state();
	/* reset the system */
	dcs_reset(NULL, 0);
}


void dcs2_audio_device::device_start()
{
	int soundbank_words;

	m_internal_program_ram = (UINT32 *)memshare("dcsint")->ptr();
	m_external_program_ram = (UINT32 *)memshare("dcsext")->ptr();

	/* find the DCS CPU and the sound ROMs */
	m_cpu = subdevice<adsp21xx_device>("dcs2");
	m_rev = 2;
	soundbank_words = 0x1000;
	if (m_cpu == NULL)
	{
		m_cpu = subdevice<adsp21xx_device>("dsio");
		m_rev = 3;
		soundbank_words = 0x400;
	}
	if (m_cpu == NULL)
	{
		m_cpu = subdevice<adsp21xx_device>("denver");
		m_rev = 4;
		soundbank_words = 0x800;
	}
	if (m_cpu != NULL && !m_cpu->started())
		throw device_missing_dependencies();

	m_program = &m_cpu->space(AS_PROGRAM);
	m_data = &m_cpu->space(AS_DATA);
	m_channels = 2;
	m_dmadac[0] = subdevice<dmadac_sound_device>("dac1");
	m_dmadac[1] = subdevice<dmadac_sound_device>("dac2");

	/* always boot from the base of "dcs" */
	m_bootrom = (UINT16 *)machine().root_device().memregion("dcs")->base();
	m_bootrom_words = machine().root_device().memregion("dcs")->bytes() / 2;

	/* supports both RAM and ROM variants */
	if (m_dram_in_mb != 0)
	{
		m_sounddata = auto_alloc_array(machine(), UINT16, m_dram_in_mb << (20-1));
		save_pointer(NAME(m_sounddata), m_dram_in_mb << (20-1));
		m_sounddata_words = (m_dram_in_mb << 20) / 2;
	}
	else
	{
		m_sounddata = m_bootrom;
		m_sounddata_words = m_bootrom_words;
	}
	m_sounddata_banks = m_sounddata_words / soundbank_words;
	if (m_rev != 2)
		membank("databank")->configure_entries(0, m_sounddata_banks, m_sounddata, soundbank_words*2);

	/* allocate memory for the SRAM */
	m_sram = auto_alloc_array(machine(), UINT16, 0x8000*4/2);

	/* create the timers */
	m_internal_timer = subdevice<timer_device>("dcs_int_timer");
	m_reg_timer = subdevice<timer_device>("dcs_reg_timer");
	m_sport_timer = subdevice<timer_device>("dcs_sport_timer");

	/* we don't do auto-ack by default */
	m_auto_ack = FALSE;

	/* install the speedup handler */
	if (m_polling_offset)
		m_polling_base = m_cpu->space(AS_DATA).install_readwrite_handler(m_polling_offset, m_polling_offset, read16_delegate(FUNC(dcs_audio_device::dcs_polling_r),this), write16_delegate(FUNC(dcs_audio_device::dcs_polling_w),this));

	/* allocate a watchdog timer for HLE transfers */
	m_transfer.hle_enabled = (ENABLE_HLE_TRANSFERS && m_dram_in_mb != 0);
	if (m_transfer.hle_enabled)
		m_transfer.watchdog = subdevice<timer_device>("dcs_hle_timer");

	/* register for save states */
	dcs_register_state();

	/* reset the system */
	dcs_reset(NULL, 0);
}


void dcs_audio_device::set_auto_ack(int state)
{
	m_auto_ack = state;
}



/*************************************
 *
 *  Original DCS read/write handlers
 *
 *************************************/

READ16_MEMBER( dcs_audio_device::dcs_dataram_r )
{
	return m_external_program_ram[offset] >> 8;
}


WRITE16_MEMBER( dcs_audio_device::dcs_dataram_w )
{
	UINT16 val = m_external_program_ram[offset] >> 8;
	COMBINE_DATA(&val);
	m_external_program_ram[offset] = (val << 8) | (m_external_program_ram[offset] & 0x0000ff);
}


WRITE16_MEMBER( dcs_audio_device::dcs_data_bank_select_w )
{
	if (m_rev != 15)
		m_sounddata_bank = data & 0x7ff;
	else
		m_sounddata_bank = (m_sounddata_bank & 0xff00) | (data & 0xff);

	membank("databank")->set_entry(m_sounddata_bank % m_sounddata_banks);

	/* bit 11 = sound board led */
#if 0
	if (m_rev != 15)
		set_led_status(machine(), 2, data & 0x800);
#endif
}

WRITE16_MEMBER( dcs_audio_device::dcs_data_bank_select2_w )
{
	m_sounddata_bank = (m_sounddata_bank & 0x00ff) | ((data & 0x01) << 8) | ((data & 0xfc) << 7);

	membank("databank")->set_entry(m_sounddata_bank % m_sounddata_banks);
}

/*************************************
 *
 *  SDRC ASIC Memory handling
 *
 *************************************/

void dcs_audio_device::sdrc_update_bank_pointers()
{
	if (SDRC_SM_EN != 0)
	{
		int pagesize = (SDRC_ROM_SZ == 0 && SDRC_ROM_ST != 0) ? 4096 : 1024;

		/* update the bank pointer based on whether we are ROM-based or RAM-based */
		if (m_bootrom == m_sounddata)
		{
			/* ROM-based; use the memory page to select from ROM */
			if (SDRC_ROM_MS == 1 && SDRC_ROM_ST != 3)
				membank("rompage")->set_base(&m_sounddata[(SDRC_EPM_PG * pagesize) % m_sounddata_words]);
		}
		else
		{
			/* RAM-based; use the ROM page to select from ROM, and the memory page to select from RAM */
			if (SDRC_ROM_MS == 1 && SDRC_ROM_ST != 3)
				membank("rompage")->set_base(&m_bootrom[(SDRC_ROM_PG * 4096 /*pagesize*/) % m_bootrom_words]);
			if (SDRC_DM_ST != 0)
				membank("drampage")->set_base(&m_sounddata[(SDRC_DM_PG * 1024) % m_sounddata_words]);
		}
	}
}


void dcs_audio_device::sdrc_remap_memory()
{
	/* if SRAM disabled, clean it out */
	if (SDRC_SM_EN == 0)
	{
		m_program->unmap_readwrite(0x0800, 0x3fff);
		m_data->unmap_readwrite(0x0800, 0x37ff);
	}

	/* otherwise, map the SRAM */
	else
	{
		/* first start with a clean program map */
		m_program->install_ram(0x0800, 0x3fff, m_sram + 0x4800);

		/* set up the data map based on the SRAM banking */
		/* map 0: ram from 0800-37ff */
		if (SDRC_SM_BK == 0)
		{
			m_data->install_ram(0x0800, 0x17ff, m_sram + 0x0000);
			m_data->install_ram(0x1800, 0x27ff, m_sram + 0x1000);
			m_data->install_ram(0x2800, 0x37ff, m_sram + 0x2000);
		}

		/* map 1: nothing from 0800-17ff, alternate RAM at 1800-27ff, same RAM at 2800-37ff */
		else
		{
			m_data->unmap_readwrite(0x0800, 0x17ff);
			m_data->install_ram(0x1800, 0x27ff, m_sram + 0x3000);
			m_data->install_ram(0x2800, 0x37ff, m_sram + 0x2000);
		}
	}

	/* map the ROM page as bank 25 */
	if (SDRC_ROM_MS == 1 && SDRC_ROM_ST != 3)
	{
		int baseaddr = (SDRC_ROM_ST == 0) ? 0x0000 : (SDRC_ROM_ST == 1) ? 0x3000 : 0x3400;
		int pagesize = (SDRC_ROM_SZ == 0 && SDRC_ROM_ST != 0) ? 4096 : 1024;
		m_data->install_read_bank(baseaddr, baseaddr + pagesize - 1, "rompage");
	}

	/* map the DRAM page as bank 26 */
	if (SDRC_DM_ST != 0)
	{
		int baseaddr = (SDRC_DM_ST == 1) ? 0x0000 : (SDRC_DM_ST == 2) ? 0x3000 : 0x3400;
		m_data->install_readwrite_bank(baseaddr, baseaddr + 0x3ff, "drampage");
	}

	/* update the bank pointers */
	sdrc_update_bank_pointers();

	/* reinstall the polling hotspot */
	if (m_polling_offset)
		m_polling_base = m_cpu->space(AS_DATA).install_readwrite_handler(m_polling_offset, m_polling_offset, read16_delegate(FUNC(dcs_audio_device::dcs_polling_r),this), write16_delegate(FUNC(dcs_audio_device::dcs_polling_w),this));
}


void dcs_audio_device::sdrc_reset()
{
	memset(m_sdrc.reg, 0, sizeof(m_sdrc.reg));
	sdrc_remap_memory();
}



/*************************************
 *
 *  SDRC ASIC read/write
 *
 *************************************/

READ16_MEMBER( dcs_audio_device::sdrc_r )
{
	sdrc_state &sdrc = m_sdrc;
	UINT16 result = sdrc.reg[offset];

	/* offset 3 is for security */
	if (offset == 3)
	{
		switch (SDRC_SMODE)
		{
			default:
			case 0: /* no-op */
				result = 0x5a81;
				break;

			case 1: /* write seed */
				result = 0x5aa4;
				break;

			case 2: /* read data */
				result = 0x5a00 | ((sdrc.seed & 0x3f) << 1);
				break;

			case 3: /* shift left */
				result = 0x5ab9;
				break;

			case 4: /* add */
				result = 0x5a03;
				break;

			case 5: /* xor */
				result = 0x5a69;
				break;

			case 6: /* prg */
				result = 0x5a20;
				break;

			case 7: /* invert */
				result = 0x5aff;
				break;
		}
	}

	return result;
}


WRITE16_MEMBER( dcs_audio_device::sdrc_w )
{
	sdrc_state &sdrc = m_sdrc;
	UINT16 diff = sdrc.reg[offset] ^ data;

	switch (offset)
	{
		/* offset 0 controls ROM mapping */
		case 0:
			sdrc.reg[0] = data;
			if (diff & 0x1833)
				sdrc_remap_memory();
			if (diff & 0x0380)
				sdrc_update_bank_pointers();
			break;

		/* offset 1 controls RAM mapping */
		case 1:
			sdrc.reg[1] = data;
			//dmadac_enable(&m_dmadac[0], m_channels, SDRC_MUTE);
			if (diff & 0x0003)
				sdrc_remap_memory();
			break;

		/* offset 2 controls paging */
		case 2:
			sdrc.reg[2] = data;
			if (diff & 0x1fff)
				sdrc_update_bank_pointers();
			break;

		/* offset 3 controls security */
		case 3:
			switch (SDRC_SMODE)
			{
				case 0: /* no-op */
				case 2: /* read data */
					break;

				case 1: /* write seed */
					sdrc.seed = data & 0xff;
					break;

				case 3: /* shift left */
					sdrc.seed = (sdrc.seed << 1) | 1;
					break;

				case 4: /* add */
					sdrc.seed += sdrc.seed >> 1;
					break;

				case 5: /* xor */
					sdrc.seed ^= (sdrc.seed << 1) | 1;
					break;

				case 6: /* prg */
					sdrc.seed = (((sdrc.seed << 7) ^ (sdrc.seed << 5) ^ (sdrc.seed << 4) ^ (sdrc.seed << 3)) & 0x80) | (sdrc.seed >> 1);
					break;

				case 7: /* invert */
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

void dcs_audio_device::dsio_reset()
{
	memset(&m_dsio, 0, sizeof(m_dsio));
}


READ16_MEMBER( dcs_audio_device::dsio_r )
{
	dsio_state &dsio = m_dsio;
	UINT16 result = dsio.reg[offset];

	if (offset == 1)
	{
		/* bit 4 specifies which channel is being output */
		dsio.channelbits ^= 0x0010;
		result = (result & ~0x0010) | dsio.channelbits;
	}
	return result;
}


WRITE16_MEMBER( dcs_audio_device::dsio_w )
{
	dsio_state &dsio = m_dsio;

	switch (offset)
	{
		/* offset 1 controls I/O */
		case 1:
			dsio.reg[1] = data;

			/* determine /MUTE and number of channels */
			dmadac_enable(&m_dmadac[0], m_channels, DSIO_MUTE);

			/* bit 0 resets the FIFO */
			if (!m_fifo_reset_w.isnull())
				m_fifo_reset_w(DSIO_EMPTY_FIFO ^ 1);
			break;

		/* offset 2 controls RAM pages */
		case 2:
			dsio.reg[2] = data;
			membank("databank")->set_entry(DSIO_DM_PG % m_sounddata_banks);
			break;
	}
}



/*************************************
 *
 *  Denver ASIC read/write
 *
 *************************************/

void dcs_audio_device::denver_reset()
{
	memset(&m_dsio, 0, sizeof(m_dsio));
}


READ16_MEMBER( dcs_audio_device::denver_r )
{
	UINT16 result = m_dsio.reg[offset];

	if (offset == 3)
	{
		/* returns 1 for DRAM, 2 for EPROM-based */
		result = 0x0001;
	}
	return result;
}


WRITE16_MEMBER( dcs_audio_device::denver_w )
{
	dsio_state &dsio = m_dsio;
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
			if (channels != m_channels)
			{
				m_channels = channels;
				for (chan = 0; chan < m_channels; chan++)
				{
					char buffer[10];
					sprintf(buffer, "dac%d", chan + 1);
					m_dmadac[chan] = subdevice<dmadac_sound_device>(buffer);
				}
				dmadac_enable(&m_dmadac[0], m_channels, enable);
				if (m_channels < 6)
					dmadac_enable(&m_dmadac[m_channels], 6 - m_channels, FALSE);
				recompute_sample_rate();
			}
			break;

		/* offset 2 controls RAM pages */
		case 2:
			dsio.reg[2] = data;
			membank("databank")->set_entry(DENV_DM_PG % m_sounddata_banks);
			break;

		/* offset 3 controls FIFO reset */
		case 3:
			if (!m_fifo_reset_w.isnull())
				m_fifo_reset_w(1);
			break;
	}
}



/*************************************
 *
 *  DSIO/Denver IDMA access
 *
 *************************************/

WRITE32_MEMBER( dcs_audio_device::dsio_idma_addr_w )
{
	dsio_state &dsio = m_dsio;
	if (LOG_DCS_TRANSFERS)
		logerror("%08X:IDMA_addr = %04X\n", space.device().safe_pc(), data);
	downcast<adsp2181_device *>(m_cpu)->idma_addr_w(data);
	if (data == 0)
		dsio.start_on_next_write = 2;
}


WRITE32_MEMBER( dcs_audio_device::dsio_idma_data_w )
{
	dsio_state &dsio = m_dsio;
	UINT32 pc = space.device().safe_pc();
	if (ACCESSING_BITS_0_15)
	{
		if (LOG_DCS_TRANSFERS)
			logerror("%08X:IDMA_data_w(%04X) = %04X\n", pc, downcast<adsp2181_device *>(m_cpu)->idma_addr_r(), data & 0xffff);
		downcast<adsp2181_device *>(m_cpu)->idma_data_w(data & 0xffff);
	}
	if (ACCESSING_BITS_16_31)
	{
		if (LOG_DCS_TRANSFERS)
			logerror("%08X:IDMA_data_w(%04X) = %04X\n", pc, downcast<adsp2181_device *>(m_cpu)->idma_addr_r(), data >> 16);
		downcast<adsp2181_device *>(m_cpu)->idma_data_w(data >> 16);
	}
	if (dsio.start_on_next_write && --dsio.start_on_next_write == 0)
	{
		logerror("Starting DSIO CPU\n");
		m_cpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}
}


READ32_MEMBER( dcs_audio_device::dsio_idma_data_r )
{
	UINT32 result;
	result = downcast<adsp2181_device *>(m_cpu)->idma_data_r();
	if (LOG_DCS_TRANSFERS)
		logerror("%08X:IDMA_data_r(%04X) = %04X\n", space.device().safe_pc(), downcast<adsp2181_device *>(m_cpu)->idma_addr_r(), result);
	return result;
}



/***************************************************************************
    DCS COMMUNICATIONS
****************************************************************************/

void dcs_audio_device::set_io_callbacks(write_line_delegate output_full_cb, write_line_delegate input_empty_cb)
{
	m_input_empty_cb = input_empty_cb;
	m_output_full_cb = output_full_cb;
}


void dcs_audio_device::set_fifo_callbacks(read16_delegate fifo_data_r, read16_delegate fifo_status_r, write_line_delegate fifo_reset_w)
{
	m_fifo_data_r = fifo_data_r;
	m_fifo_status_r = fifo_status_r;
	m_fifo_reset_w = fifo_reset_w;
}


int dcs_audio_device::control_r()
{
	/* only boost for DCS2 boards */
	if (!m_auto_ack && !m_transfer.hle_enabled)
		machine().scheduler().boost_interleave(attotime::from_nsec(500), attotime::from_usec(5));
	if ( /* m_rev == 1 || */ m_rev == 15) // == 1 check breaks mk3
		return IS_OUTPUT_FULL() ? 0x80 : 0x00;
	return m_latch_control;
}


void dcs_audio_device::reset_w(int state)
{
	/* going high halts the CPU */
	if (state)
	{
		//      logerror("%s: DCS reset = %d\n", machine().describe_context(), state);

		/* just run through the init code again */
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(dcs_audio_device::dcs_reset),this));
		m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}

	/* going low resets and reactivates the CPU */
	else
		m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}


READ16_MEMBER( dcs_audio_device::latch_status_r )
{
	int result = 0;
	if (IS_INPUT_FULL())
		result |= 0x80;
	if (IS_OUTPUT_EMPTY())
		result |= 0x40;
	if (!m_fifo_status_r.isnull() && (!m_transfer.hle_enabled || m_transfer.state == 0))
		result |= m_fifo_status_r(space, 0, 0xffff) & 0x38;
	if (m_transfer.hle_enabled && m_transfer.state != 0)
		result |= 0x08;
	return result;
}


READ16_MEMBER( dcs_audio_device::fifo_input_r )
{
	if (!m_fifo_data_r.isnull())
		return m_fifo_data_r(space,0, 0xffff);
	else
		return 0xffff;
}



/***************************************************************************
    INPUT LATCH (data from host to DCS)
****************************************************************************/

void dcs_audio_device::dcs_delayed_data_w(UINT16 data)
{
	if (LOG_DCS_IO)
		logerror("%s:dcs_data_w(%04X)\n", machine().describe_context(), data);

	/* boost the interleave temporarily */
	machine().scheduler().boost_interleave(attotime::from_nsec(500), attotime::from_usec(5));

	/* set the IRQ line on the ADSP */
	m_cpu->set_input_line(ADSP2105_IRQ2, ASSERT_LINE);

	/* indicate we are no longer empty */
	if (m_last_input_empty && !m_input_empty_cb.isnull())
		m_input_empty_cb(m_last_input_empty = 0);
	SET_INPUT_FULL();

	/* set the data */
	m_input_data = data;
}


TIMER_CALLBACK_MEMBER( dcs_audio_device::dcs_delayed_data_w_callback )
{
	dcs_delayed_data_w(param);
}


void dcs_audio_device::data_w(UINT16 data)
{
	/* preprocess the write */
	if (preprocess_write(data))
		return;

	/* if we are DCS1, set a timer to latch the data */
	if (m_sport_timer == NULL)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(dcs_audio_device::dcs_delayed_data_w_callback),this), data);
	else
		dcs_delayed_data_w(data);
}


WRITE16_MEMBER( dcs_audio_device::input_latch_ack_w )
{
	if (!m_last_input_empty && !m_input_empty_cb.isnull())
		m_input_empty_cb(m_last_input_empty = 1);
	SET_INPUT_EMPTY();
	m_cpu->set_input_line(ADSP2105_IRQ2, CLEAR_LINE);
}


READ16_MEMBER( dcs_audio_device::input_latch_r )
{
	if (m_auto_ack)
		input_latch_ack_w(space,0,0,0xffff);
	if (LOG_DCS_IO)
		logerror("%08X:input_latch_r(%04X)\n", space.device().safe_pc(), m_input_data);
	return m_input_data;
}

READ32_MEMBER( dcs_audio_device::input_latch32_r )
{
	if (m_auto_ack)
		input_latch_ack_w(space,0,0,0xffff);
	if (LOG_DCS_IO)
		logerror("%08X:input_latch32_r(%04X)\n", space.device().safe_pc(), m_input_data);
	return m_input_data << 8;
}

/***************************************************************************
    OUTPUT LATCH (data from DCS to host)
****************************************************************************/

TIMER_CALLBACK_MEMBER( dcs_audio_device::latch_delayed_w )
{
	if (!m_last_output_full && !m_output_full_cb.isnull())
		m_output_full_cb(m_last_output_full = 1);
	SET_OUTPUT_FULL();
	m_output_data = m_pre_output_data;
}


WRITE16_MEMBER( dcs_audio_device::output_latch_w )
{
	m_pre_output_data = data;
	if (LOG_DCS_IO)
		logerror("%08X:output_latch_w(%04X) (empty=%d)\n", space.device().safe_pc(), data, IS_OUTPUT_EMPTY());

	machine().scheduler().synchronize(timer_expired_delegate(FUNC(dcs_audio_device::latch_delayed_w),this), data>>8);
}

WRITE32_MEMBER( dcs_audio_device::output_latch32_w )
{
	m_pre_output_data = data >> 8;
	if (LOG_DCS_IO)
		logerror("%08X:output_latch32_w(%04X) (empty=%d)\n", space.device().safe_pc(), data>>8, IS_OUTPUT_EMPTY());

	machine().scheduler().synchronize(timer_expired_delegate(FUNC(dcs_audio_device::latch_delayed_w),this), data>>8);
}


void dcs_audio_device::delayed_ack_w()
{
	SET_OUTPUT_EMPTY();
}


TIMER_CALLBACK_MEMBER( dcs_audio_device::delayed_ack_w_callback )
{
	delayed_ack_w();
}


void dcs_audio_device::ack_w()
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(dcs_audio_device::delayed_ack_w_callback),this));
}


UINT16 dcs_audio_device::data_r()
{
	/* data is actually only 8 bit (read from d8-d15, which is d0-d7 from the data access instructions POV) on early dcs, but goes 16 on later (seattle) */
	if (m_last_output_full && !m_output_full_cb.isnull())
		m_output_full_cb(m_last_output_full = 0);
	if (m_auto_ack)
		delayed_ack_w();

	if (LOG_DCS_IO)
		logerror("%s:dcs_data_r(%04X)\n", machine().describe_context(), m_output_data);
	return m_output_data;
}



/***************************************************************************
    OUTPUT CONTROL BITS (has 3 additional lines to the host)
****************************************************************************/

TIMER_CALLBACK_MEMBER( dcs_audio_device::output_control_delayed_w )
{
	if (LOG_DCS_IO)
		logerror("output_control = %04X\n", param);
	m_output_control = param;
	m_output_control_cycles = 0;
}


WRITE16_MEMBER( dcs_audio_device::output_control_w )
{
	if (LOG_DCS_IO)
		logerror("%04X:output_control = %04X\n", space.device().safe_pc(), data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(dcs_audio_device::output_control_delayed_w),this), data);
}


READ16_MEMBER( dcs_audio_device::output_control_r )
{
	m_output_control_cycles = m_cpu->total_cycles();
	return m_output_control;
}


int dcs_audio_device::data2_r()
{
	return m_output_control;
}



/*************************************
 *
 *  Timer management
 *
 *************************************/

void dcs_audio_device::update_timer_count()
{
	UINT64 periods_since_start;
	UINT64 elapsed_cycles;
	UINT64 elapsed_clocks;

	/* if not enabled, skip */
	if (!m_timer_enable)
		return;

	/* count cycles */
	elapsed_cycles = m_cpu->total_cycles() - m_timer_start_cycles;
	elapsed_clocks = elapsed_cycles / m_timer_scale;

	/* if we haven't counted past the initial count yet, just do that */
	if (elapsed_clocks < m_timer_start_count + 1)
		m_control_regs[TIMER_COUNT_REG] = m_timer_start_count - elapsed_clocks;

	/* otherwise, count how many periods */
	else
	{
		elapsed_clocks -= m_timer_start_count + 1;
		periods_since_start = elapsed_clocks / (m_timer_period + 1);
		elapsed_clocks -= periods_since_start * (m_timer_period + 1);
		m_control_regs[TIMER_COUNT_REG] = m_timer_period - elapsed_clocks;
	}
}


TIMER_DEVICE_CALLBACK_MEMBER( dcs_audio_device::internal_timer_callback )
{
	INT64 target_cycles;

	/* compute the absolute cycle when the next one should fire */
	/* we do this to avoid drifting */
	m_timers_fired++;
	target_cycles = m_timer_start_cycles + m_timer_scale * (m_timer_start_count + 1 + m_timers_fired * (UINT64)(m_timer_period + 1));
	target_cycles -= m_cpu->total_cycles();

	/* set the next timer, but only if it's for a reasonable number */
	if (!m_timer_ignore && (m_timer_period > 10 || m_timer_scale > 1))
		timer.adjust(m_cpu->cycles_to_attotime(target_cycles));

	/* the IRQ line is edge triggered */
	m_cpu->set_input_line(ADSP2105_TIMER, ASSERT_LINE);
	m_cpu->set_input_line(ADSP2105_TIMER, CLEAR_LINE);
}


void dcs_audio_device::reset_timer()
{
	/* if not enabled, skip */
	if (!m_timer_enable)
		return;

	/* compute the time until the first firing */
	m_timer_start_cycles = m_cpu->total_cycles();
	m_timers_fired = 0;

	/* if this is the first timer, check the IRQ routine for the DRAM refresh stub */
	/* if that's all the timer does, we don't really need to fire */
	if (!m_timer_ignore)
	{
		/* Road Burners: @ 28: JMP $0032  18032F, same code at $32 */

		if (m_program->read_dword(0x18*4) == 0x0c0030 &&      /* ENA SEC_REG */
			m_program->read_dword(0x19*4) == 0x804828 &&      /* SI = DM($0482) */
			m_program->read_dword(0x1a*4) == 0x904828 &&      /* DM($0482) = SI */
			m_program->read_dword(0x1b*4) == 0x0C0020 &&      /* DIS SEC_REG */
			m_program->read_dword(0x1c*4) == 0x0A001F)            /* RTI */
		{
			m_timer_ignore = TRUE;
		}
	}

	/* adjust the timer if not optimized */
	if (!m_timer_ignore)
		m_internal_timer->adjust(m_cpu->cycles_to_attotime(m_timer_scale * (m_timer_start_count + 1)));
}


WRITE_LINE_MEMBER(dcs_audio_device::timer_enable_callback)
{
	m_timer_enable = state;
	m_timer_ignore = 0;
	if (state)
	{
		//osd_printf_debug("Timer enabled @ %d cycles/int, or %f Hz\n", m_timer_scale * (m_timer_period + 1), 1.0 / m_cpu->cycles_to_attotime(m_timer_scale * (m_timer_period + 1)));
		reset_timer();
	}
	else
	{
		//osd_printf_debug("Timer disabled\n");
		m_internal_timer->reset();
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

READ16_MEMBER( dcs_audio_device::adsp_control_r )
{
	UINT16 result = 0xffff;

	switch (offset)
	{
		case PROG_FLAG_DATA_REG:
			/* Denver waits for this & 0x000e == 0x0000 */
			/* Denver waits for this & 0x000e == 0x0006 */
			result = m_progflags ^= 0x0006;
			break;

		case IDMA_CONTROL_REG:
			if (m_rev == 3 || m_rev == 4)
				result = downcast<adsp2181_device *>(m_cpu)->idma_addr_r();
			break;

		case TIMER_COUNT_REG:
			update_timer_count();
			result = m_control_regs[offset];
			break;

		default:
			result = m_control_regs[offset];
			break;
	}
	return result;
}


WRITE16_MEMBER(dcs_audio_device:: adsp_control_w )
{
	m_control_regs[offset] = data;

	switch (offset)
	{
		case SYSCONTROL_REG:
			/* bit 9 forces a reset */
			if (data & 0x0200)
			{
				logerror("%04X:Rebooting DCS due to SYSCONTROL write\n", space.device().safe_pc());
				m_cpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
				dcs_boot();
				m_control_regs[SYSCONTROL_REG] = 0;
			}

			/* see if SPORT1 got disabled */
			if ((data & 0x0800) == 0)
			{
				dmadac_enable(&m_dmadac[0], m_channels, 0);
				m_reg_timer->reset();
			}
			break;

		case S1_AUTOBUF_REG:
			/* autobuffer off: nuke the timer, and disable the DAC */
			if ((data & 0x0002) == 0)
			{
				dmadac_enable(&m_dmadac[0], m_channels, 0);
				m_reg_timer->reset();
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
			if (data != m_timer_scale)
			{
				update_timer_count();
				m_timer_scale = data;
				reset_timer();
			}
			break;

		case TIMER_COUNT_REG:
			m_timer_start_count = data;
			reset_timer();
			break;

		case TIMER_PERIOD_REG:
			if (data != m_timer_period)
			{
				update_timer_count();
				m_timer_period = data;
				reset_timer();
			}
			break;

		case IDMA_CONTROL_REG:
			if (m_rev == 3 || m_rev == 4)
				downcast<adsp2181_device *>(m_cpu)->idma_addr_w(data);
			break;
	}
}


/***************************************************************************
    DCS IRQ GENERATION CALLBACKS
****************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER( dcs_audio_device::dcs_irq )
{
	/* get the index register */
	int reg = m_cpu->state_int(ADSP2100_I0 + m_ireg);

	/* copy the current data into the buffer */
	{
		int count = m_size / (2*(m_incs ? m_incs : 1));
		INT16 buffer[0x400];
		int i;

		for (i = 0; i < count; i++)
		{
			buffer[i] = m_data->read_word(reg * 2);
			reg += m_incs;
		}

		if (m_channels)
			dmadac_transfer(&m_dmadac[0], m_channels, 1, m_channels, count / m_channels, buffer);
	}

	/* check for wrapping */
	if (reg >= m_ireg_base + m_size)
	{
		/* reset the base pointer */
		reg = m_ireg_base;

		/* generate the (internal, thats why the pulse) irq */
		m_cpu->machine().driver_data()->generic_pulse_irq_line(*m_cpu,  ADSP2105_IRQ1, 1);
	}

	/* store it */
	m_cpu->set_state_int(ADSP2100_I0 + m_ireg, reg);
}


TIMER_DEVICE_CALLBACK_MEMBER( dcs_audio_device::sport0_irq )
{
	/* this latches internally, so we just pulse */
	/* note that there is non-interrupt code that reads/modifies/writes the output_control */
	/* register; if we don't interlock it, we will eventually lose sound (see CarnEvil) */
	/* so we skip the SPORT interrupt if we read with output_control within the last 5 cycles */
	if ((m_cpu->total_cycles() - m_output_control_cycles) > 5)
	{
		m_cpu->set_input_line(ADSP2115_SPORT0_RX, ASSERT_LINE);
		m_cpu->set_input_line(ADSP2115_SPORT0_RX, CLEAR_LINE);
	}
}


void dcs_audio_device::recompute_sample_rate()
{
	/* calculate how long until we generate an interrupt */

	/* frequency the time per each bit sent */
	attotime sample_period = attotime::from_hz(m_cpu->unscaled_clock()) * (2 * (m_control_regs[S1_SCLKDIV_REG] + 1));

	/* now put it down to samples, so we know what the channel frequency has to be */
	sample_period = sample_period * (16 * m_channels);
	dmadac_set_frequency(&m_dmadac[0], m_channels, ATTOSECONDS_TO_HZ(sample_period.attoseconds()));
	dmadac_enable(&m_dmadac[0], m_channels, 1);

	/* fire off a timer which will hit every half-buffer */
	if (m_incs)
	{
		attotime period = (sample_period * m_size) / (2 * m_channels * m_incs);
		m_reg_timer->adjust(period, 0, period);
	}
}

WRITE32_MEMBER(dcs_audio_device::sound_tx_callback)
{
	/* check if it's for SPORT1 */
	if (offset != 1)
		return;

	/* check if SPORT1 is enabled */
	if (m_control_regs[SYSCONTROL_REG] & 0x0800) /* bit 11 */
	{
		/* we only support autobuffer here (wich is what this thing uses), bail if not enabled */
		if (m_control_regs[S1_AUTOBUF_REG] & 0x0002) /* bit 1 */
		{
			/* get the autobuffer registers */
			int     mreg, lreg;
			UINT16  source;

			m_ireg = (m_control_regs[S1_AUTOBUF_REG] >> 9) & 7;
			mreg = (m_control_regs[S1_AUTOBUF_REG] >> 7) & 3;
			mreg |= m_ireg & 0x04; /* msb comes from ireg */
			lreg = m_ireg;

			/* now get the register contents in a more legible format */
			/* we depend on register indexes to be continuous (which is the case in our core) */
			source = m_cpu->state_int(ADSP2100_I0 + m_ireg);
			m_incs = m_cpu->state_int(ADSP2100_M0 + mreg);
			m_size = m_cpu->state_int(ADSP2100_L0 + lreg);

			/* get the base value, since we need to keep it around for wrapping */
			source -= m_incs;

			/* make it go back one so we dont lose the first sample */
			m_cpu->set_state_int(ADSP2100_I0 + m_ireg, source);

			/* save it as it is now */
			m_ireg_base = source;

			/* recompute the sample rate and timer */
			recompute_sample_rate();
			return;
		}
		else
			logerror( "ADSP SPORT1: trying to transmit and autobuffer not enabled!\n" );
	}

	/* if we get there, something went wrong. Disable playing */
	dmadac_enable(&m_dmadac[0], m_channels, 0);

	/* remove timer */
	m_reg_timer->reset();
}



/***************************************************************************
    VERY BASIC & SAFE OPTIMIZATIONS
****************************************************************************/

READ16_MEMBER( dcs_audio_device::dcs_polling_r )
{
	if (m_polling_count++ > 5)
		space.device().execute().eat_cycles(10000);
	return *m_polling_base;
}


WRITE16_MEMBER( dcs_audio_device::dcs_polling_w )
{
	m_polling_count = 0;
	COMBINE_DATA(m_polling_base);
}



/***************************************************************************
    DATA TRANSFER HLE MECHANISM
****************************************************************************/

void dcs_audio_device::fifo_notify(int count, int max)
{
	hle_transfer_state &transfer = m_transfer;

	/* skip if not in mid-transfer */
	if (!transfer.hle_enabled || transfer.state == 0 || m_fifo_data_r.isnull())
	{
		transfer.fifo_entries = 0;
		return;
	}

	/* preprocess a word */
	transfer.fifo_entries = count;
	if (transfer.state != 5 || transfer.fifo_entries == transfer.writes_left || transfer.fifo_entries >= 256)
	{
		for ( ; transfer.fifo_entries; transfer.fifo_entries--)
			preprocess_write(m_fifo_data_r(machine().driver_data()->generic_space(),0, 0xffff));
	}
}


TIMER_DEVICE_CALLBACK_MEMBER( dcs_audio_device::transfer_watchdog_callback )
{
	hle_transfer_state &transfer = m_transfer;
	int starting_writes_left = param;

	if (transfer.fifo_entries && starting_writes_left == transfer.writes_left)
	{
		for ( ; transfer.fifo_entries; transfer.fifo_entries--)
			preprocess_write(m_fifo_data_r(machine().driver_data()->generic_space(),0, 0xffff));
	}
	if (transfer.watchdog != NULL)
		transfer.watchdog->adjust(attotime::from_msec(1), transfer.writes_left);
}


TIMER_CALLBACK_MEMBER( dcs_audio_device::s1_ack_callback2 )
{
	/* if the output is full, stall for a usec */
	if (IS_OUTPUT_FULL())
	{
		machine().scheduler().timer_set(attotime::from_usec(1), timer_expired_delegate(FUNC(dcs_audio_device::s1_ack_callback2),this), param);
		return;
	}
	output_latch_w(m_cpu->space(AS_PROGRAM), 0, 0x000a, 0xffff);
}


TIMER_CALLBACK_MEMBER( dcs_audio_device::s1_ack_callback1 )
{
	/* if the output is full, stall for a usec */
	if (IS_OUTPUT_FULL())
	{
		machine().scheduler().timer_set(attotime::from_usec(1), timer_expired_delegate(FUNC(dcs_audio_device::s1_ack_callback1),this), param);
		return;
	}
	output_latch_w(m_cpu->space(AS_PROGRAM), 0, param, 0xffff);

	/* chain to the next word we need to write back */
	machine().scheduler().timer_set(attotime::from_usec(1), timer_expired_delegate(FUNC(dcs_audio_device::s1_ack_callback2),this));
}


int dcs_audio_device::preprocess_stage_1(UINT16 data)
{
	hle_transfer_state &transfer = m_transfer;

	switch (transfer.state)
	{
		case 0:
			/* look for command 0x001a to transfer chunks of data */
			if (data == 0x001a)
			{
				if (LOG_DCS_TRANSFERS)
					logerror("%s:DCS Transfer command %04X\n", machine().describe_context(), data);
				transfer.state++;
				if (transfer.hle_enabled)
					return 1;
			}

			/* look for command 0x002a to start booting the uploaded program */
			else if (data == 0x002a)
			{
				if (LOG_DCS_TRANSFERS)
					logerror("%s:DCS State change %04X\n", machine().describe_context(), data);
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
					m_sdrc.reg[0] &= ~0x1000;
					sdrc_remap_memory();
				}
				if (transfer.type == 2 && SDRC_SM_BK == 0)
				{
					m_sdrc.reg[0] |= 0x1000;
					sdrc_remap_memory();
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
						m_program->write_dword(transfer.start++ * 4, (transfer.temp << 8) | (data & 0xff));
				}
				else
					m_data->write_word(transfer.start++ * 2, data);

				/* if we're done, start a timer to send the response words */
				if (transfer.state == 0)
					machine().scheduler().timer_set(attotime::from_usec(1), timer_expired_delegate(FUNC(dcs_audio_device::s1_ack_callback1),this), transfer.sum);
				return 1;
			}
			break;
	}
	return 0;
}


TIMER_CALLBACK_MEMBER( dcs_audio_device::s2_ack_callback )
{
	address_space &space = m_cpu->space(AS_PROGRAM);

	/* if the output is full, stall for a usec */
	if (IS_OUTPUT_FULL())
	{
		machine().scheduler().timer_set(attotime::from_usec(1), timer_expired_delegate(FUNC(dcs_audio_device::s2_ack_callback),this), param);
		return;
	}
	output_latch_w(space, 0, param, 0xffff);
	output_control_w(space, 0, (m_output_control & ~0xff00) | 0x0300, 0xffff);
}


int dcs_audio_device::preprocess_stage_2(UINT16 data)
{
	hle_transfer_state &transfer = m_transfer;

	switch (transfer.state)
	{
		case 0:
			/* look for command 0x55d0 or 0x55d1 to transfer chunks of data */
			if (data == 0x55d0 || data == 0x55d1)
			{
				if (LOG_DCS_TRANSFERS)
					logerror("%s:DCS Transfer command %04X\n", machine().describe_context(), data);
				transfer.state++;
				if (transfer.hle_enabled)
					return 1;
			}

			/* anything else is ignored */
			else
			{
				if (LOG_DCS_TRANSFERS)
					logerror("%s:Command: %04X\n", machine().describe_context(), data);
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
				m_sounddata[transfer.start++] = data;

				/* if we're done, start a timer to send the response words */
				if (transfer.state == 0)
				{
					machine().scheduler().timer_set(attotime::from_usec(1), timer_expired_delegate(FUNC(dcs_audio_device::s2_ack_callback),this), transfer.sum);
					transfer.watchdog->reset();
				}
				return 1;
			}
			break;
	}
	return 0;
}


int dcs_audio_device::preprocess_write(UINT16 data)
{
	hle_transfer_state &transfer = m_transfer;
	int result;

	/* if we're not DCS2, skip */
	if (m_sport_timer == NULL)
		return 0;

	/* state 0 - initialization phase */
	if (transfer.dcs_state == 0)
		result = preprocess_stage_1(data);
	else
		result = preprocess_stage_2(data);

	/* if we did the write, toggle the full/not full state so interrupts are generated */
	if (result && !m_input_empty_cb.isnull())
	{
		if (m_last_input_empty)
			m_input_empty_cb(m_last_input_empty = 0);
		if (!m_last_input_empty)
			m_input_empty_cb(m_last_input_empty = 1);
	}
	return result;
}

const device_type DCS_AUDIO_2K = &device_creator<dcs_audio_2k_device>;

//-------------------------------------------------
//  dcs_audio_2k_device - constructor
//-------------------------------------------------

dcs_audio_2k_device::dcs_audio_2k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	dcs_audio_device(mconfig, DCS_AUDIO_2K, "DCS Audio 2K", tag, owner, clock, "dcs_audio_2k", __FILE__)
{
}

machine_config_constructor dcs_audio_2k_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dcs_audio_2k );
}

const device_type DCS_AUDIO_2K_UART = &device_creator<dcs_audio_2k_uart_device>;

//-------------------------------------------------
//  dcs_audio_2k_uart_device - constructor
//-------------------------------------------------

dcs_audio_2k_uart_device::dcs_audio_2k_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	dcs_audio_device(mconfig, DCS_AUDIO_2K_UART, "DCS Audio 2K UART", tag, owner, clock, "dcs_audio_2k_uart", __FILE__)
{
}

machine_config_constructor dcs_audio_2k_uart_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dcs_audio_2k_uart );
}

const device_type DCS_AUDIO_8K = &device_creator<dcs_audio_8k_device>;

//-------------------------------------------------
//  dcs_audio_8k_device - constructor
//-------------------------------------------------

dcs_audio_8k_device::dcs_audio_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	dcs_audio_device(mconfig, DCS_AUDIO_8K, "DCS Audio 8K", tag, owner, clock, "dcs_audio_8k", __FILE__)
{
}

machine_config_constructor dcs_audio_8k_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dcs_audio_8k );
}

const device_type DCS_AUDIO_WPC = &device_creator<dcs_audio_wpc_device>;

//-------------------------------------------------
//  dcs_audio_wpc_device - constructor
//-------------------------------------------------

dcs_audio_wpc_device::dcs_audio_wpc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	dcs_audio_device(mconfig, DCS_AUDIO_WPC, "DCS Audio WPC", tag, owner, clock, "dcs_audio_wpc", __FILE__, 15)
{
}

machine_config_constructor dcs_audio_wpc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dcs_audio_wpc );
}



//-------------------------------------------------
//  dcs2_audio_device - constructor
//-------------------------------------------------

dcs2_audio_device::dcs2_audio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	dcs_audio_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}


const device_type DCS2_AUDIO_2115 = &device_creator<dcs2_audio_2115_device>;

//-------------------------------------------------
//  dcs2_audio_2115_device - constructor
//-------------------------------------------------

dcs2_audio_2115_device::dcs2_audio_2115_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	dcs2_audio_device(mconfig, DCS2_AUDIO_2115, "DCS2 Audio 2115", tag, owner, clock, "dcs2_audio_2115", __FILE__)
{
}

machine_config_constructor dcs2_audio_2115_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dcs2_audio_2115 );
}

const device_type DCS2_AUDIO_2104 = &device_creator<dcs2_audio_2104_device>;

//-------------------------------------------------
//  dcs2_audio_2104_device - constructor
//-------------------------------------------------

dcs2_audio_2104_device::dcs2_audio_2104_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	dcs2_audio_device(mconfig, DCS2_AUDIO_2104, "DCS2 Audio 2104", tag, owner, clock, "dcs2_audio_2104", __FILE__)
{
}

machine_config_constructor dcs2_audio_2104_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dcs2_audio_2104 );
}


const device_type DCS2_AUDIO_DSIO = &device_creator<dcs2_audio_dsio_device>;

//-------------------------------------------------
//  dcs2_audio_dsio_device - constructor
//-------------------------------------------------

dcs2_audio_dsio_device::dcs2_audio_dsio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	dcs2_audio_device(mconfig, DCS2_AUDIO_DSIO, "DCS2 Audio DSIO", tag, owner, clock, "dcs2_audio_dsio", __FILE__)
{
}

machine_config_constructor dcs2_audio_dsio_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dcs2_audio_dsio );
}

const device_type DCS2_AUDIO_DENVER = &device_creator<dcs2_audio_denver_device>;

//-------------------------------------------------
//  dcs2_audio_denver_device - constructor
//-------------------------------------------------

dcs2_audio_denver_device::dcs2_audio_denver_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	dcs2_audio_device(mconfig, DCS2_AUDIO_DENVER, "DCS2 Audio Denver", tag, owner, clock, "dcs2_audio_denver", __FILE__)
{
}

machine_config_constructor dcs2_audio_denver_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dcs2_audio_denver );
}
