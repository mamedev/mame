// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                    -= Seta, Sammy, Visco (SSV) System =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU          :      NEC V60

Sound Chip   :      Ensoniq ES5506 (OTTOR2)

Custom Chips :      ST-0004     (Video DAC)
                    ST-0005     (Parallel I/O)
                    ST-0006     (Video controller)
                    ST-0007     (System controller)

Others       :      Battery + MB3790 + LH5168D-10L (NVRAM)
                    DX-102              (I/O)
                    M62X42B             (RTC)
                    ST010
                    TA8210              (Audio AMP)
                    uPD71051/7001C      (UART)

-----------------------------------------------------------------------------------
Main Board  ROM Board   Year + Game                                 By
-----------------------------------------------------------------------------------
STA-0001    STS-0001    93  Super Real Mahjong PIV                  Seta
STA-0001    STS-0001    93  Dramatic Adventure Quiz Keith & Lucy    Visco
STA-0001    SAM-5127    93  Survival Arts                           Sammy
STA-0001    SAM-5127    93  Dyna Gears                              Sammy
STA-0001B   VISCO-001B  94  Drift Out '94                           Visco
STA-0001B   GOLF ROM    94  Eagle Shot Golf                         Sammy
STA-0001B   STS0003     94  Twin Eagle II - The Rescue Mission (1)  Seta
STA-0001B   VISCO       95  Mobil Suit Gundam Final Shooting (2)    Banpresto
STA-0001B   P1-102A     95  Mahjong Hyper Reaction                  Sammy
?           ?           95  Ultra X Weapons / Ultra Keibitai        Banpresto / Tsuburaya Prod.
STA-0001B   VISCO-JJ1   96  Lovely Pop Mahjong JangJang Shimasho    Visco
STA-0001B   VISCO-001B  96  Storm Blade                             Visco
STA-0001B   P1-105A     96? Meosis Magic                            Sammy
STA-0001B   ?           97  Joryuu Syougi Kyoushitsu (3)            Visco
STA-0001B   VISCO-JJ1   97  Koi Koi Shimasho 2                      Visco
STA-0001B   P1-112A     97  Mahjong Hyper Reaction 2                Sammy
STA-0001B   ?           97  Monster Slider                          Visco / Datt
STA-0001    ?           97  Super Real Mahjong P7                   Seta
STA-0001B   ?           98  Gourmet Battle Quiz Ryorioh CooKing     Visco
STA-0001B   P1-112C     98  Pachinko Sexy Reaction                  Sammy
STA-0001B   B1-001A     99  Pachinko Sexy Reaction 2                Sammy
STA-0001B   P1-112C     99  Change Air Blade                        Visco
STA-0001B   VISCO-JJ1   00  Lovely Pop Mahjong JangJang Shimasho 2  Visco
STA-0001B   SSV_SUB     00  Vasara                                  Visco
STA-0001B   SSV_SUB     01  Vasara 2                                Visco
-----------------------------------------------------------------------------------

(1) Uses ST010 MCU / Math chip
(2) Uses ST-0009 & ST-0020 & Light Gun
(3) Uses NEC V810 CPU as sub cpu for the AI (basically the same as the majinsen games on st0016)

SSV Hardware Overview
Sammy/Seta/Visco, 1993-2001

SSV was a joint venture between "S"ammy, "S"eta and "V"isco to produce a high quality 32-bit
RISC-based hardware platform.

The SSV main board contains all of the CPUs, RAM, custom ICs and sound chips. The lower PCB plugs
into 4x 64-pin connectors and contains the game software and any additional game-specific connectors
for controllers etc. The SSV games can be swapped by simply exchanging the lower (game) PCB onto any
SSV main board. There is an older main board STA-0001 which is not compatible with _some_ of the
newer games. The actual PCB will plug in, but the games will not boot up, instead displaying some
kind of debug screen full of numbers. This is known to occur on Vasara and Vasara 2 but may occur
with other games also. AFAIK, the actual PCBs look identical!

PCB Layouts
-----------

Main Board (common to all SSV-based games)

STA-0001B SYSTEM SSV
|---------------------------------------------------------|
| 2066   VOL                                              |
|TA8210                    |--------|      74ALS245 x6    |
|      TL072 |--------|    |        |                     |
|H           |ENSONIC |    | ST0007 |                 JP1 |
|      TL072 |ES5506  |    |        |   |--------|JP2     |
|            |OTTOR2  |    |--------|   |        |74ALS74 |
|S           |--------|                 |  V60   |        |
|      D6376                74ALS08     |        |74F32   |
|   74LS07                              |--------|        |
|          TC4050                                 JP3     |
|J TD62064                                                |
|A              74ALS245                                  |
|M      LM358      74ALS245                               |
|M        |------|     74ALS245                           |
|A        |ST0004|                 681000  681000         |
|         |      |                                        |
||------| |------| 43256  43256 43256       43256  43256  |
||ST0005|                         |--------|      JP4     |
||      |      48MHz              |        |       MB3771 |
||------|42.9545MHz   74AC245     | ST0006 | 74ALS273     |
|                         74F245  |        |       74F04  |
| DSW1(8)   DSW2(8)               |--------|              |
|---------------------------------------------------------|
(Note All IC's Shown)

Notes:
      H       - 3 pin connector for additional sound output
      S       - 5 pin connector for additional sound output
      D6376   - NEC D6376 2-channel 16-bit D/A converter (SOIC16)
      V60     - NEC D70615GD-16 V60 CPU, clock 16.000MHz [48/3] (QFP120)
      ES5506  - Ensonic ES5506 sound IC, clock 16.000MHz [48/3] (PLCC68)
      LM358   - National Semiconductor LM358 Low Power Dual Operational Amplifier (DIP8)
      2066    - New Japan Radio NJM2066 Dual Operational Amplifier (DIP16)
      TL072   - Texas Instruments TL072 Low Noise JFET-Input Operational Amplifier (DIP8)
      TA8210  - Toshiba TA8210 20W 2-channel Audio Power Amplifier
      43256   - Sharp LH52B256-70LL 32K x8 SRAM (DIP28)
                Note all sockets with 43256 can accept DIP32 128K x8 SRAM instead depending on
                game requirements
      681000  - Samsung KM681000ALP-7L 128K x8 SRAM (DIP32)
      JP1     - 3 pin jumper, hardwired on PCB to 2-3
      JP2     - 3 pin jumper, hardwired on PCB to 2-3
      JP3     - 3 pin jumper, hardwired on PCB to 1-3
      JP4     - 3 pin jumper, hardwired on PCB to 1-3

      SETA custom IC's -
                         ST-0004 Video DAC (QFP64)
                         ST-0005 Parallel I/O (QFP100)
                         ST-0006 Video controller (QFP208, covered with large heaksink)
                         ST-0007 System controller (QFP160)

To Do:

- hypreac2  :   communication with other units

- janjans2  :   Coin inputs don't register in the input test

- jsk       :   Coin inputs don't register in the input test

- mslider   :   Coin inputs don't register in the input test

- srmp4     :   Coin inputs don't register in the input test

- srmp7     :   Needs interrupts by the sound chip (not properly hooked up yet). Kludged to work.

Notes:

- gdfs      :   Hold 1P Start on startup to enter test mode

- jsk       :   Push 1P Start and 2P Start to advance to sound test


    code @ $e75cdc

     W:
            0x482000 - 0x482007 - values taken from obj table
            0x482040 - 0x482043 - write latch ?

     R:
            0x482022 - 0x482023 - result = direction, probably : 00 = down, 40 = left, 80 = up, c0 = right
            0x482042 - 0x482043 - protection status bits ?

***************************************************************************/

#include "emu.h"
#include "cpu/v810/v810.h"
#include "cpu/v60/v60.h"
#include "machine/nvram.h"
#include "includes/ssv.h"

/***************************************************************************


                                Interrupts


***************************************************************************/

/* Update the IRQ state based on all possible causes */
void ssv_state::update_irq_state()
{
	m_maincpu->set_input_line(0, (m_requested_int & m_irq_enable)? ASSERT_LINE : CLEAR_LINE);
}

IRQ_CALLBACK_MEMBER(ssv_state::irq_callback)
{
	int i;
	for ( i = 0; i <= 7; i++ )
	{
		if (m_requested_int & (1 << i))
		{
			UINT16 vector = m_irq_vectors[i * (16/2)] & 7;
			return vector;
		}
	}
	return 0;
}

WRITE16_MEMBER(ssv_state::irq_ack_w)
{
	int level = ((offset * 2) & 0x70) >> 4;

	m_requested_int &= ~(1 << level);

	update_irq_state();
}

/*
    IRQ Enable Register:

    drifto94:   c at the start
    hypreact:   ff at the start
    hypreac2:   ff at the start
    janjans1:   0,6c,60
    keithlcy:   c at the start
    meosism:    ff at the start
    mslider:    c at the start
    ryorioh:    0,c at the start
    srmp4:      8 at the start
    srmp7:      8 at the start, 28, 40 (seems related to 21000e writes)
    survarts:   0,8 at the start
    sxyreact:   ff at the start
    ultrax:     40,00 at the start then 42,4a
    twineag2:   40,00 at the start then 42,4a
*/
WRITE16_MEMBER(ssv_state::irq_enable_w)
{
	COMBINE_DATA(&m_irq_enable);
}

TIMER_DEVICE_CALLBACK_MEMBER(ssv_state::interrupt)
{
	int scanline = param;

	if (scanline == 0)
	{
		if (m_interrupt_ultrax)
		{
			m_requested_int |= 1 << 1;  // needed by ultrax to coin up, breaks cairblad
			update_irq_state();
		}
	}
	else if(scanline == 240)
	{
		m_requested_int |= 1 << 3;  // vblank
		update_irq_state();
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(ssv_state::gdfs_interrupt)
{
	int scanline = param;

	if ((scanline % 64) == 0)
	{
		m_requested_int |= 1 << 6;  // reads lightgun (4 times for 4 axis)
		update_irq_state();
	}
	else if(scanline == 240)
	{
		m_requested_int |= 1 << 3;  // vblank
		update_irq_state();
	}
}

/***************************************************************************


                            Coins Lockout / Counter


***************************************************************************/

/*
    drifto94:   c3
    janjans1:   c3
    keithlcy:   c3
    mslider:    c3, 83 in test mode
    ryorioh:    c3

    hypreac2:   80
    hypreact:   80
    meosism:    83
    srmp4:      83, c0 in test mode (where only tilemap sprites are used)
    srmp7:      80
    survarts:   83
    sxyreact:   80
*/
WRITE16_MEMBER(ssv_state::lockout_w)
{
//  popmessage("%02X",data & 0xff);
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_lockout_w(1,~data & 0x01);
		machine().bookkeeping().coin_lockout_w(0,~data & 0x02);
		machine().bookkeeping().coin_counter_w(1, data & 0x04);
		machine().bookkeeping().coin_counter_w(0, data & 0x08);
//                        data & 0x40?
		enable_video(data & 0x80);
	}
}

/* Same as above but with inverted lockout lines */
WRITE16_MEMBER(ssv_state::lockout_inv_w)
{
//  popmessage("%02X",data & 0xff);
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_lockout_w(1, data & 0x01);
		machine().bookkeeping().coin_lockout_w(0, data & 0x02);
		machine().bookkeeping().coin_counter_w(1, data & 0x04);
		machine().bookkeeping().coin_counter_w(0, data & 0x08);
//                        data & 0x40?
		enable_video(data & 0x80);
	}
}

void ssv_state::machine_reset()
{
	m_requested_int = 0;
}


/***************************************************************************


                                DSP


***************************************************************************/

static ADDRESS_MAP_START( dsp_prg_map, AS_PROGRAM, 32, ssv_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("dspprg", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp_data_map, AS_DATA, 16, ssv_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("dspdata", 0)
ADDRESS_MAP_END

READ16_MEMBER(ssv_state::dsp_dr_r)
{
	return m_dsp->snesdsp_read(true);
}

WRITE16_MEMBER(ssv_state::dsp_dr_w)
{
	m_dsp->snesdsp_write(true, data);
}

READ16_MEMBER(ssv_state::dsp_r)
{
	UINT16 temp = m_dsp->dataram_r(offset/2);
	UINT16 res;

	if (offset & 1)
	{
		res = temp>>8;
	}
	else
	{
		res = temp & 0xff;
	}

	return res;
}

WRITE16_MEMBER(ssv_state::dsp_w)
{
	UINT16 temp = m_dsp->dataram_r(offset/2);

	if (offset & 1)
	{
		temp &= 0xff;
		temp |= data<<8;
	}
	else
	{
		temp &= 0xff00;
		temp |= data;
	}

	m_dsp->dataram_w(offset/2, temp);
}

/***************************************************************************


                                Memory Maps


***************************************************************************/

#ifdef UNUSED_FUNCTION
READ16_MEMBER(ssv_state::fake_r){   return ssv_scroll[offset];  }
#endif

#define SSV_MAP( _ROM  )                                                                                            \
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_SHARE("mainram")                                     /*  RAM     */  \
	AM_RANGE(0x100000, 0x13ffff) AM_RAM AM_SHARE("spriteram")                                       /*  Sprites */  \
	AM_RANGE(0x140000, 0x15ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") /* Palette */\
	AM_RANGE(0x160000, 0x17ffff) AM_RAM                                                             /*          */  \
	AM_RANGE(0x1c0000, 0x1c0001) AM_READ(vblank_r           )                                   /*  Vblank? */  \
/**/AM_RANGE(0x1c0002, 0x1c007f) AM_READONLY                                    /*  Scroll  */  \
	AM_RANGE(0x1c0000, 0x1c007f) AM_WRITE(scroll_w) AM_SHARE("scroll")                  /*  Scroll  */  \
	AM_RANGE(0x210002, 0x210003) AM_READ_PORT("DSW1")                                                               \
	AM_RANGE(0x210004, 0x210005) AM_READ_PORT("DSW2")                                                               \
	AM_RANGE(0x210008, 0x210009) AM_READ_PORT("P1")                                                                 \
	AM_RANGE(0x21000a, 0x21000b) AM_READ_PORT("P2")                                                                 \
	AM_RANGE(0x21000c, 0x21000d) AM_READ_PORT("SYSTEM")                                                             \
	AM_RANGE(0x21000e, 0x21000f) AM_READNOP AM_WRITE(lockout_w)                             /*  Lockout */  \
	AM_RANGE(0x210010, 0x210011) AM_WRITENOP                                                                        \
	AM_RANGE(0x230000, 0x230071) AM_WRITEONLY AM_SHARE("irq_vectors")                       /*  IRQ Vec */  \
	AM_RANGE(0x240000, 0x240071) AM_WRITE(irq_ack_w )                                           /*  IRQ Ack */  \
	AM_RANGE(0x260000, 0x260001) AM_WRITE(irq_enable_w)                                         /*  IRQ En  */  \
	AM_RANGE(0x300000, 0x30007f) AM_DEVREADWRITE8("ensoniq", es5506_device, read, write, 0x00ff)    /*  Sound   */  \
	AM_RANGE(_ROM, 0xffffff) AM_ROM AM_REGION("maincpu", 0)                                         /*  ROM     */

/***************************************************************************
                                Drift Out '94
***************************************************************************/

READ16_MEMBER(ssv_state::drifto94_unknown_r)
{
	return machine().rand() & 0xffff;
}

static ADDRESS_MAP_START( drifto94_map, AS_PROGRAM, 16, ssv_state )
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP                                      // ? 1 at the start
	AM_RANGE(0x400000, 0x47ffff) AM_WRITEONLY                                       // ?
	AM_RANGE(0x480000, 0x480001) AM_READWRITE(dsp_dr_r, dsp_dr_w)
	AM_RANGE(0x482000, 0x482fff) AM_READWRITE(dsp_r, dsp_w)
	AM_RANGE(0x483000, 0x485fff) AM_WRITENOP                                        // ?
	AM_RANGE(0x500000, 0x500001) AM_WRITENOP                                        // ??
	AM_RANGE(0x510000, 0x510001) AM_READ(drifto94_unknown_r)                       // ??
	AM_RANGE(0x520000, 0x520001) AM_READ(drifto94_unknown_r)                       // ??
	AM_RANGE(0x580000, 0x5807ff) AM_RAM AM_SHARE("nvram")   // NVRAM
	SSV_MAP( 0xc00000 )
ADDRESS_MAP_END


/***************************************************************************
                     Mobil Suit Gundam Final Shooting
***************************************************************************/

READ16_MEMBER(ssv_state::gdfs_eeprom_r)
{
	ioport_port *gun[] = { m_io_gunx1, m_io_guny1, m_io_gunx2, m_io_guny2 };

	return (((m_gdfs_lightgun_select & 1) ? 0 : 0xff) ^ gun[m_gdfs_lightgun_select]->read()) | (m_eeprom->do_read() << 8);
}

WRITE16_MEMBER(ssv_state::gdfs_eeprom_w)
{
	if (data & ~0x7b00)
		logerror("%s - Unknown EEPROM bit written %04X\n",machine().describe_context(),data);

	if ( ACCESSING_BITS_8_15 )
	{
//      data & 0x8000 ? (near palette writes)
//      data & 0x0001 ?

		// latch the bit
		m_eeprom->di_write((data & 0x4000) >> 14);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x1000) ? ASSERT_LINE : CLEAR_LINE );

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x2000) ? ASSERT_LINE : CLEAR_LINE );

		if (!(m_gdfs_eeprom_old & 0x0800) && (data & 0x0800))   // rising clock
			m_gdfs_lightgun_select = (data & 0x0300) >> 8;
	}

	COMBINE_DATA(&m_gdfs_eeprom_old);
}


static ADDRESS_MAP_START( gdfs_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x400000, 0x41ffff) AM_RAM_WRITE(gdfs_tmapram_w) AM_SHARE("gdfs_tmapram")
	AM_RANGE(0x420000, 0x43ffff) AM_RAM
	AM_RANGE(0x440000, 0x44003f) AM_RAM AM_SHARE("gdfs_tmapscroll")
	AM_RANGE(0x500000, 0x500001) AM_WRITE(gdfs_eeprom_w)
	AM_RANGE(0x540000, 0x540001) AM_READ(gdfs_eeprom_r)
	AM_RANGE(0x600000, 0x600fff) AM_RAM
	AM_RANGE(0x800000, 0x87ffff) AM_DEVREADWRITE( "st0020_spr", st0020_device, st0020_sprram_r, st0020_sprram_w );
	AM_RANGE(0x8c0000, 0x8c00ff) AM_DEVREADWRITE( "st0020_spr", st0020_device, st0020_blitram_r, st0020_blitram_w );
	AM_RANGE(0x900000, 0x9fffff) AM_DEVREADWRITE( "st0020_spr", st0020_device, st0020_gfxram_r, st0020_gfxram_w );
	SSV_MAP( 0xc00000 )
ADDRESS_MAP_END


/***************************************************************************
                                Hyper Reaction
***************************************************************************/

/*
    The game prints "backup ram ok" and there is code to test some ram
    at 0x580000-0x5bffff. The test is skipped and this ram isn't used
    though. I guess it's either a left-over or there are different
    version with some battery backed RAM (which would indeed be on the
    rom-board, AFAIK)
*/

READ16_MEMBER(ssv_state::hypreact_input_r)
{
	UINT16 input_sel = *m_input_sel;

	if (input_sel & 0x0001) return m_io_key0->read();
	if (input_sel & 0x0002) return m_io_key1->read();
	if (input_sel & 0x0004) return m_io_key2->read();
	if (input_sel & 0x0008) return m_io_key3->read();
	logerror("CPU #0 PC %06X: unknown input read: %04X\n",space.device().safe_pc(),input_sel);
	return 0xffff;
}

static ADDRESS_MAP_START( hypreact_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x210000, 0x210001) AM_READ(watchdog_reset16_r)            // Watchdog
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP                      // ? 5 at the start
	AM_RANGE(0x21000e, 0x21000f) AM_WRITE(lockout_inv_w)            // Inverted lockout lines
//  AM_RANGE(0x280000, 0x280001) AM_READNOP                       // ? read at the start, value not used
	AM_RANGE(0xc00000, 0xc00001) AM_READ(hypreact_input_r)              // Inputs
	AM_RANGE(0xc00006, 0xc00007) AM_RAM AM_SHARE("input_sel")           //
	AM_RANGE(0xc00008, 0xc00009) AM_NOP                                 //
	SSV_MAP( 0xf00000 )
ADDRESS_MAP_END


/***************************************************************************
                                Hyper Reaction 2
***************************************************************************/

static ADDRESS_MAP_START( hypreac2_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x210000, 0x210001) AM_READ(watchdog_reset16_r)                // Watchdog
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP                          // ? 5 at the start
	AM_RANGE(0x21000e, 0x21000f) AM_WRITE(lockout_inv_w)                // Inverted lockout lines
//  AM_RANGE(0x280000, 0x280001) AM_READNOP                           // ? read at the start, value not used
	AM_RANGE(0x500000, 0x500001) AM_READ(hypreact_input_r)                  // Inputs
	AM_RANGE(0x500002, 0x500003) AM_READ(hypreact_input_r)                  // (again?)
	AM_RANGE(0x520000, 0x520001) AM_WRITEONLY AM_SHARE("input_sel") // Inputs
//  0x540000, 0x540003  communication with other units
	SSV_MAP( 0xe00000 )
ADDRESS_MAP_END


/***************************************************************************
                                Jan Jan Simasyo
***************************************************************************/



static ADDRESS_MAP_START( janjans1_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x210000, 0x210001) AM_WRITENOP                            // koikois2 but not janjans1
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP                          // ? 1 at the start
	AM_RANGE(0x210006, 0x210007) AM_READNOP
	AM_RANGE(0x800000, 0x800001) AM_WRITEONLY AM_SHARE("input_sel") // Inputs
	AM_RANGE(0x800002, 0x800003) AM_READ(srmp4_input_r)                     // Inputs
	SSV_MAP( 0xc00000 )
ADDRESS_MAP_END


/***************************************************************************
                                Keith & Lucy
***************************************************************************/

static ADDRESS_MAP_START( keithlcy_map, AS_PROGRAM, 16, ssv_state )
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP  // ? 1 at the start
	AM_RANGE(0x210010, 0x210011) AM_WRITENOP    //
	AM_RANGE(0x21000e, 0x21000f) AM_READNOP //
	AM_RANGE(0x400000, 0x47ffff) AM_WRITEONLY   // ?
	SSV_MAP( 0xe00000 )
ADDRESS_MAP_END


/***************************************************************************
                                Meosis Magic
***************************************************************************/

static ADDRESS_MAP_START( meosism_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x210000, 0x210001) AM_READ(watchdog_reset16_r )                           // Watchdog
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP                                      // ? 5 at the start
//  AM_RANGE(0x280000, 0x280001) AM_READNOP                                       // ? read once, value not used
//  AM_RANGE(0x500004, 0x500005) AM_WRITENOP                                      // ? 0,58,18
	AM_RANGE(0x580000, 0x58ffff) AM_RAM AM_SHARE("nvram")   // NVRAM
	SSV_MAP( 0xf00000 )
ADDRESS_MAP_END

/***************************************************************************
                                Monster Slider
***************************************************************************/

/* Monster Slider needs the RAM mirrored for the gameplay logic to work correctly */

READ16_MEMBER(ssv_state::mainram_r)
{
	return m_mainram[offset];
}

WRITE16_MEMBER(ssv_state::mainram_w)
{
	COMBINE_DATA(&m_mainram[offset]);
}

static ADDRESS_MAP_START( mslider_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x010000, 0x01ffff) AM_READWRITE(mainram_r, mainram_w) // RAM Mirror
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP                          // ? 1 at the start
	AM_RANGE(0x400000, 0x47ffff) AM_WRITEONLY                           // ?
//  AM_RANGE(0x500000, 0x500001) AM_WRITENOP                          // ? ff at the start
	SSV_MAP( 0xf00000 )
ADDRESS_MAP_END


/***************************************************************************
                    Gourmet Battle Quiz Ryohrioh CooKing
***************************************************************************/

static ADDRESS_MAP_START( ryorioh_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x210000, 0x210001) AM_WRITE(watchdog_reset16_w)   // Watchdog
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP              // ? 1 at the start
	SSV_MAP( 0xc00000 )
ADDRESS_MAP_END


/***************************************************************************
                            Super Real Mahjong PIV
***************************************************************************/

READ16_MEMBER(ssv_state::srmp4_input_r)
{
	UINT16 input_sel = *m_input_sel;

	if (input_sel & 0x0002) return m_io_key0->read();
	if (input_sel & 0x0004) return m_io_key1->read();
	if (input_sel & 0x0008) return m_io_key2->read();
	if (input_sel & 0x0010) return m_io_key3->read();
	logerror("CPU #0 PC %06X: unknown input read: %04X\n",space.device().safe_pc(),input_sel);
	return 0xffff;
}

static ADDRESS_MAP_START( srmp4_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x210000, 0x210001) AM_READ(watchdog_reset16_r)                // Watchdog
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP                          // ? 1,5 at the start
	AM_RANGE(0xc0000a, 0xc0000b) AM_READ(srmp4_input_r)                     // Inputs
	AM_RANGE(0xc0000e, 0xc0000f) AM_WRITEONLY AM_SHARE("input_sel") // Inputs
	AM_RANGE(0xc00010, 0xc00011) AM_WRITENOP                            //
	SSV_MAP( 0xf00000 )
ADDRESS_MAP_END


/***************************************************************************
                            Super Real Mahjong P7
***************************************************************************/

/*
    Interrupts aren't supported by the chip emulator yet
    (lev 5 in this case, I guess)
*/
READ16_MEMBER(ssv_state::srmp7_irqv_r)
{
	return 0x0080;
}

WRITE16_MEMBER(ssv_state::srmp7_sound_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		int bank = 0x400000/2 * (data & 1); // UINT16 address
		int voice;
		for (voice = 0; voice < 32; voice++)
			m_ensoniq->voice_bank_w(voice, bank);
	}
//  popmessage("%04X",data);
}

READ16_MEMBER(ssv_state::srmp7_input_r)
{
	UINT16 input_sel = *m_input_sel;

	if (input_sel & 0x0002) return m_io_key0->read();
	if (input_sel & 0x0004) return m_io_key1->read();
	if (input_sel & 0x0008) return m_io_key2->read();
	if (input_sel & 0x0010) return m_io_key3->read();
	logerror("CPU #0 PC %06X: unknown input read: %04X\n",space.device().safe_pc(),input_sel);
	return 0xffff;
}

static ADDRESS_MAP_START( srmp7_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x010000, 0x050faf) AM_RAM                                     // More RAM
	AM_RANGE(0x210000, 0x210001) AM_READ(watchdog_reset16_r)                // Watchdog
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP                          // ? 0,4 at the start
	AM_RANGE(0x21000e, 0x21000f) AM_WRITE(lockout_inv_w)                // Coin Counters / Lockouts
	AM_RANGE(0x300076, 0x300077) AM_READ(srmp7_irqv_r)                      // Sound
//  0x540000, 0x540003, related to lev 5 irq?
	AM_RANGE(0x580000, 0x580001) AM_WRITE(srmp7_sound_bank_w)               // Sound Bank
	AM_RANGE(0x600000, 0x600001) AM_READ(srmp7_input_r)                     // Inputs
	AM_RANGE(0x680000, 0x680001) AM_WRITEONLY AM_SHARE("input_sel") // Inputs
	SSV_MAP( 0xc00000 )
ADDRESS_MAP_END


/***************************************************************************
                                Survival Arts
***************************************************************************/

static ADDRESS_MAP_START( survarts_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x210000, 0x210001) AM_READ(watchdog_reset16_r)    // Watchdog
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP              // ? 0,4 at the start
//  AM_RANGE(0x290000, 0x290001) AM_READNOP               // ?
//  AM_RANGE(0x2a0000, 0x2a0001) AM_READNOP               // ?

	AM_RANGE(0x400000, 0x43ffff) AM_RAM                         // dyna

	AM_RANGE(0x500008, 0x500009) AM_READ_PORT("ADD_BUTTONS")    // Extra Buttons
	SSV_MAP( 0xf00000 )
ADDRESS_MAP_END


/***************************************************************************
                            Pachinko Sexy Reaction
***************************************************************************/


READ16_MEMBER(ssv_state::sxyreact_ballswitch_r)
{
	if ( m_io_service )
	{
		return m_io_service->read();
	}
	return 0;
}

READ16_MEMBER(ssv_state::sxyreact_dial_r)
{
	return ((m_sxyreact_serial >> 1) & 0x80);
}


WRITE16_MEMBER(ssv_state::sxyreact_dial_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 0x20)
			m_sxyreact_serial = ( m_io_paddle ? m_io_paddle->read() : 0 ) & 0xff;

		if ( (m_sxyreact_dial & 0x40) && !(data & 0x40) )   // $40 -> $00
			m_sxyreact_serial <<= 1;                        // shift 1 bit

		m_sxyreact_dial = data;
	}
}

WRITE16_MEMBER(ssv_state::sxyreact_motor_w)
{
//  popmessage("%04X",data);   // 8 = motor on; 0 = motor off
}

static ADDRESS_MAP_START( sxyreact_map, AS_PROGRAM, 16, ssv_state )
//  AM_RANGE(0x020000, 0x03ffff) AM_READWRITE(mainram_r, mainram_w)             // sxyreac2 reads / writes here, why?
	AM_RANGE(0x210000, 0x210001) AM_READ(watchdog_reset16_r)                            // Watchdog
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP                                      // ? 1 at the start
	AM_RANGE(0x21000e, 0x21000f) AM_WRITE(lockout_inv_w)                            // Inverted lockout lines
	AM_RANGE(0x500002, 0x500003) AM_READ(sxyreact_ballswitch_r)                         // ?
	AM_RANGE(0x500004, 0x500005) AM_READWRITE(sxyreact_dial_r, sxyreact_motor_w)        // Dial Value (serial)
	AM_RANGE(0x520000, 0x520001) AM_WRITE(sxyreact_dial_w)                              // Dial Value (advance 1 bit)
	AM_RANGE(0x580000, 0x58ffff) AM_RAM AM_SHARE("nvram")   // NVRAM
	SSV_MAP( 0xe00000 )
ADDRESS_MAP_END


/***************************************************************************
                                Twin Eagle II
***************************************************************************/

/* comes as either a standalone board or a standard SSV rom board (verified) */

static ADDRESS_MAP_START( twineag2_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x010000, 0x03ffff) AM_RAM                         // More RAM
	AM_RANGE(0x210000, 0x210001) AM_READ(watchdog_reset16_r)    // Watchdog (also value is cmp.b with mem 8)
	AM_RANGE(0x480000, 0x480001) AM_READWRITE(dsp_dr_r, dsp_dr_w)
	AM_RANGE(0x482000, 0x482fff) AM_READWRITE(dsp_r, dsp_w)
	SSV_MAP( 0xe00000 )
ADDRESS_MAP_END


/***************************************************************************
                                    Ultra X
***************************************************************************/

/* standalone board based on SSV hardware */

static ADDRESS_MAP_START( ultrax_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x010000, 0x03ffff) AM_RAM                         // More RAM
	AM_RANGE(0x210000, 0x210001) AM_READ(watchdog_reset16_r)    // Watchdog (also value is cmp.b with memory address 8)
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP              // ? 2,6 at the start
	SSV_MAP( 0xe00000 )
ADDRESS_MAP_END

/***************************************************************************
            Joryuu Syougi Kyoushitsu
***************************************************************************/

/* from st0016.c */

READ32_MEMBER(ssv_state::latch32_r)
{
	if(!offset)
		m_latches[2]&=~2;
	return m_latches[offset];
}

WRITE32_MEMBER(ssv_state::latch32_w)
{
	if(!offset)
		m_latches[2]|=1;
	COMBINE_DATA(&m_latches[offset]);
	machine().scheduler().synchronize();
}

READ16_MEMBER(ssv_state::latch16_r)
{
	if(!offset)
		m_latches[2]&=~1;
	return m_latches[offset];
}

WRITE16_MEMBER(ssv_state::latch16_w)
{
	if(!offset)
		m_latches[2]|=2;
	m_latches[offset]=data;
	machine().scheduler().synchronize();
}

static ADDRESS_MAP_START( jsk_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x050000, 0x05ffff) AM_READWRITE(mainram_r, mainram_w) // RAM Mirror?
	AM_RANGE(0x210000, 0x210001) AM_WRITE(watchdog_reset16_w)               // Watchdog
	AM_RANGE(0x400000, 0x47ffff) AM_RAM                                     // RAM?
	AM_RANGE(0x900000, 0x900007) AM_READWRITE(latch16_r, latch16_w)
	SSV_MAP( 0xf00000 )
ADDRESS_MAP_END


static ADDRESS_MAP_START( jsk_v810_mem, AS_PROGRAM, 32, ssv_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_RAM
	AM_RANGE(0x80000000, 0x8001ffff) AM_RAM
	AM_RANGE(0xc0000000, 0xc001ffff) AM_RAM
	AM_RANGE(0x40000000, 0x4000000f) AM_READWRITE(latch32_r, latch32_w)
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("sub", 0)
ADDRESS_MAP_END


/***************************************************************************
  Eagle Shot Golf
***************************************************************************/

WRITE16_MEMBER(ssv_state::eaglshot_gfxrom_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		membank("gfxrom")->set_entry(data < 6 ? data : 6);
	}
}

READ16_MEMBER(ssv_state::eaglshot_trackball_r)
{
	switch(m_trackball_select)
	{
		case 0x60:  return (m_io_trackx->read() >> 8) & 0xff;
		case 0x40:  return (m_io_trackx->read() >> 0) & 0xff;

		case 0x70:  return (m_io_tracky->read() >> 8) & 0xff;
		case 0x50:  return (m_io_tracky->read() >> 0) & 0xff;
	}
	return 0;
}

WRITE16_MEMBER(ssv_state::eaglshot_trackball_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_trackball_select = data;
	}
}



READ16_MEMBER(ssv_state::eaglshot_gfxram_r)
{
	return m_eaglshot_gfxram[offset + (m_scroll[0x76/2] & 0xf) * 0x40000/2];
}

WRITE16_MEMBER(ssv_state::eaglshot_gfxram_w)
{
	offset += (m_scroll[0x76/2] & 0xf) * 0x40000/2;
	COMBINE_DATA(&m_eaglshot_gfxram[offset]);
	m_gfxdecode->gfx(0)->mark_dirty(offset / (16*8/2));
	m_gfxdecode->gfx(1)->mark_dirty(offset / (16*8/2));
}


static ADDRESS_MAP_START( eaglshot_map, AS_PROGRAM, 16, ssv_state )
	AM_RANGE(0x180000, 0x1bffff) AM_READWRITE(eaglshot_gfxram_r, eaglshot_gfxram_w)
	AM_RANGE(0x210000, 0x210001) AM_READNOP /*AM_READ(watchdog_reset16_r)*/                 // Watchdog
//  AM_RANGE(0x210002, 0x210003) AM_WRITENOP                                      // ? 0,4 at the start
	AM_RANGE(0x21000e, 0x21000f) AM_WRITE(lockout_inv_w)                            // Inverted lockout lines
	AM_RANGE(0x800000, 0x800001) AM_WRITE(eaglshot_gfxrom_bank_w)
	AM_RANGE(0x900000, 0x900001) AM_WRITE(eaglshot_trackball_w)
	AM_RANGE(0xa00000, 0xbfffff) AM_ROMBANK("gfxrom")
	AM_RANGE(0xc00000, 0xc007ff) AM_RAM AM_SHARE("nvram")   // NVRAM
	AM_RANGE(0xd00000, 0xd00001) AM_READ(eaglshot_trackball_r)
	SSV_MAP( 0xf00000 )
ADDRESS_MAP_END



/***************************************************************************


                                Input Ports


***************************************************************************/


/***************************************************************************
                        Basic Coinage Settings
***************************************************************************/

#define SSV_COINAGE_BASIC( shift, default, name, diploc )                                           \
	PORT_DIPNAME( 0x0003 << (shift), (default) << (shift), (name) ) PORT_DIPLOCATION( (diploc) )    \
	PORT_DIPSETTING(                 0x0001 << (shift), DEF_STR( 2C_1C ) )                          \
	PORT_DIPSETTING(                 0x0003 << (shift), DEF_STR( 1C_1C ) )                          \
	PORT_DIPSETTING(                 0x0000 << (shift), DEF_STR( 2C_3C ) )                          \
	PORT_DIPSETTING(                 0x0002 << (shift), DEF_STR( 1C_2C ) )

#define SSV_COINAGE_STANDARD( shift, default, name, diploc )                                        \
	PORT_DIPNAME( 0x0007 << (shift), (default) << (shift), (name) ) PORT_DIPLOCATION( (diploc) )    \
	PORT_DIPSETTING(                 0x0005 << (shift), DEF_STR( 3C_1C ) )                          \
	PORT_DIPSETTING(                 0x0006 << (shift), DEF_STR( 2C_1C ) )                          \
	PORT_DIPSETTING(                 0x0007 << (shift), DEF_STR( 1C_1C ) )                          \
	PORT_DIPSETTING(                 0x0004 << (shift), DEF_STR( 1C_2C ) )                          \
	PORT_DIPSETTING(                 0x0003 << (shift), DEF_STR( 1C_3C ) )                          \
	PORT_DIPSETTING(                 0x0002 << (shift), DEF_STR( 1C_4C ) )                          \
	PORT_DIPSETTING(                 0x0001 << (shift), DEF_STR( 1C_5C ) )                          \
	PORT_DIPSETTING(                 0x0000 << (shift), DEF_STR( 1C_6C ) )

#define SSV_COINAGE_EXTENDED( shift, default, name, diploc )                                        \
	PORT_DIPNAME( 0x000f << (shift), (default) << (shift), (name) ) PORT_DIPLOCATION( (diploc) )    \
	PORT_DIPSETTING(                 0x0007 << (shift), DEF_STR( 4C_1C ) )                          \
	PORT_DIPSETTING(                 0x0008 << (shift), DEF_STR( 3C_1C ) )                          \
	PORT_DIPSETTING(                 0x0009 << (shift), DEF_STR( 2C_1C ) )                          \
	PORT_DIPSETTING(                 0x000f << (shift), DEF_STR( 1C_1C ) )                          \
	PORT_DIPSETTING(                 0x0006 << (shift), DEF_STR( 2C_3C ) )                          \
	PORT_DIPSETTING(                 0x000e << (shift), DEF_STR( 1C_2C ) )                          \
	PORT_DIPSETTING(                 0x000d << (shift), DEF_STR( 1C_3C ) )                          \
	PORT_DIPSETTING(                 0x000c << (shift), DEF_STR( 1C_4C ) )                          \
	PORT_DIPSETTING(                 0x000b << (shift), DEF_STR( 1C_5C ) )                          \
	PORT_DIPSETTING(                 0x000a << (shift), DEF_STR( 1C_6C ) )                          \
/* "** ADDED MULTIPLE COIN FEATURE **" */                                                           \
	PORT_DIPSETTING(                 0x0005 << (shift), "Multiple Coin Feature A" )                 \
/* 2c-1c, 4c-2c, 5c-3c & 6c-4c */                                                                   \
	PORT_DIPSETTING(                 0x0004 << (shift), "Multiple Coin Feature B" )                 \
/* 2c-1c, 4c-3c */                                                                                  \
	PORT_DIPSETTING(                 0x0003 << (shift), "Multiple Coin Feature C" )                 \
/* 1c-1c, 2c-2c, 3c-3c, 4c-4c, 5c-6c */                                                             \
	PORT_DIPSETTING(                 0x0002 << (shift), "Multiple Coin Feature D" )                 \
/* 1c-1c, 2c-2c, 3c-3c & 4c-5c */                                                                   \
	PORT_DIPSETTING(                 0x0001 << (shift), "Multiple Coin Feature E" )                 \
/* 1c-1c, 2c-3c */                                                                                  \
/* Meaning of all "ON" varies between games so it's not included here */


/***************************************************************************
                        Common Joystick Inputs
***************************************************************************/

static INPUT_PORTS_START( ssv_joystick )
	PORT_START("DSW1")  // IN0 - $210002
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )      /* Modified below */

	PORT_START("DSW2")  // IN1 - $210004
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )      /* Modified below */

	PORT_START("P1")    // IN2 - $210008
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("P2")    // IN3 - $21000a
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("SYSTEM")    // IN4 - $21000c
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(10)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNKNOWN  )
INPUT_PORTS_END


/***************************************************************************
                        Common Mahjong Inputs
***************************************************************************/

static INPUT_PORTS_START( ssv_mahjong )
	PORT_INCLUDE(ssv_joystick)

	PORT_START("KEY0")  // IN5 - $800002(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")  // IN6 - $800002(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")  // IN7 - $800002(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")  // IN8 - $800002(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                         Common Quiz Inputs
***************************************************************************/

static INPUT_PORTS_START( ssv_quiz )
	PORT_START("DSW1")  // IN0 - $210002
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "DSW1:1" ) /* Manual states this dip is "Unused" */
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "DSW1:2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "DSW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW1:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	SSV_COINAGE_BASIC( 4, 0x03, DEF_STR( Coin_A ), "DSW1:5,6" )
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* Modified below */

	PORT_START("DSW2")  // IN1 - $210004
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "DSW2:1,2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )     /* 15 sec */
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )   /* 12 sec */
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )     /* 10 sec */
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )  /* 8 sec */
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW, IPT_UNKNOWN )      /* Modified below */

	PORT_START("P1")    // IN2 - $210008
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("P2")    // IN3 - $21000a
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")    // IN4 - $21000c
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(10)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNKNOWN  )
INPUT_PORTS_END


/***************************************************************************
                           Change Air Blade
***************************************************************************/

static INPUT_PORTS_START( cairblad )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0
	SSV_COINAGE_STANDARD( 0, 0x07, DEF_STR( Coin_A ), "DSW1:1,2,3" )
	SSV_COINAGE_STANDARD( 3, 0x07, DEF_STR( Coin_B ), "DSW1:4,5,6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "DSW1:7" ) /* Manual lists this dip as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "DSW1:8" ) /* Manual lists this dip as "Unused" */

	PORT_MODIFY("DSW2") // IN1
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "DSW2:1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW2:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "DSW2:3,4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Free_Play ) )    PORT_DIPLOCATION( "DSW2:5" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION( "DSW2:6,7" )
	PORT_DIPSETTING(      0x0040, "Every 2 Mil" )
	PORT_DIPSETTING(      0x0060, "2 Mil/6 Mil" )
	PORT_DIPSETTING(      0x0020, "4 Million" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "DSW2:8" )
INPUT_PORTS_END


/***************************************************************************
                                Drift Out '94
***************************************************************************/

static INPUT_PORTS_START( drifto94 )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "DSW1:1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0002, IP_ACTIVE_LOW, "DSW1:2" )
	PORT_DIPNAME( 0x0004, 0x0004, "Sound Test" )            PORT_DIPLOCATION( "DSW1:3" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW1:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	SSV_COINAGE_BASIC( 4, 0x03, DEF_STR( Coin_A ), "DSW1:5,6" )
	SSV_COINAGE_BASIC( 6, 0x03, DEF_STR( Coin_B ), "DSW1:7,8" )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "DSW2:1,2" )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "DSW2:3" ) /* Manual lists these dips as "Unused" */
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "DSW2:4" )
	PORT_DIPNAME( 0x0010, 0x0010, "Music Volume" )          PORT_DIPLOCATION( "DSW2:5" )
	PORT_DIPSETTING(      0x0000, "Quiet" )
	PORT_DIPSETTING(      0x0010, "Loud" )
	PORT_DIPNAME( 0x0020, 0x0020, "Sound Volume" )          PORT_DIPLOCATION( "DSW2:6" )
	PORT_DIPSETTING(      0x0000, "Quiet" )
	PORT_DIPSETTING(      0x0020, "Loud" )
	PORT_DIPNAME( 0x0040, 0x0040, "Save Best Time" )        PORT_DIPLOCATION( "DSW2:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "DSW2:8" ) /* Manual lists this dip as "Unused" */
INPUT_PORTS_END


/***************************************************************************
                Dyna Gears
***************************************************************************/

static INPUT_PORTS_START( dynagear )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	SSV_COINAGE_EXTENDED( 0, 0x0f, DEF_STR( Coin_A ), "DSW1:1,2,3,4" )
	//PORT_DIPSETTING(      0x0000, "???" ) - No values listed for all "ON"
	SSV_COINAGE_EXTENDED( 4, 0x0f, DEF_STR( Coin_B ), "DSW1:5,6,7,8" )
	//PORT_DIPSETTING(      0x0000, "???" ) - No values listed for all "ON"

	PORT_MODIFY("DSW2") // IN0 - $210004
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "DSW2:1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW2:2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "DSW2:3,4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) )        PORT_DIPLOCATION( "DSW2:5,6" )
	PORT_DIPSETTING(      0x0010, "1" )
	PORT_DIPSETTING(      0x0030, "2" )
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )    PORT_DIPLOCATION( "DSW2:7" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Health" )                PORT_DIPLOCATION( "DSW2:8" )
	PORT_DIPSETTING(      0x0000, "3 Hearts" )
	PORT_DIPSETTING(      0x0080, "4 Hearts" )

	PORT_START("ADD_BUTTONS")   // IN5 - $500008
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Eagle Shot Golf
***************************************************************************/

static INPUT_PORTS_START( eaglshot )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	SSV_COINAGE_EXTENDED( 0, 0x0f, DEF_STR( Coinage ), "DSW1:1,2,3,4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Credits To Start" )          PORT_DIPLOCATION( "DSW1:5" )
	PORT_DIPSETTING(      0x0010, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Controls ) )         PORT_DIPLOCATION( "DSW1:6" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Trackball ) )    // trackball dosn't work yet
	PORT_DIPSETTING(      0x0000, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Trackball Type" )            PORT_DIPLOCATION( "DSW1:7" )
	PORT_DIPSETTING(      0x0040, "24 Counts (USA)" )
	PORT_DIPSETTING(      0x0000, "12 Counts (Japan)" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "DSW1:8" )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0003, 0x0003, "Number Of Holes" )           PORT_DIPLOCATION( "DSW2:1,2" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION( "DSW2:3,4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	/*PORT_DIPSETTING(      0x0000, "???" ) - No listed value for ON & ON */
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION( "DSW2:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION( "DSW2:6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION( "DSW2:7" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "DSW2:8" )

	PORT_START("TRACKX")    // IN5 - trackball x ($d00000)
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_RESET PORT_PLAYER(1)

	PORT_START("TRACKY")    // IN6 - trackball y ($d00000)
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_RESET PORT_PLAYER(1)
INPUT_PORTS_END


/***************************************************************************
                     Mobil Suit Gundam Final Shooting
***************************************************************************/

static INPUT_PORTS_START( gdfs )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Controls ) ) PORT_DIPLOCATION( "DSW1:1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Light_Gun ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Light Gun Calibration" ) PORT_DIPLOCATION( "DSW1:2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Level_Select ) ) PORT_DIPLOCATION( "DSW1:3" ) /* Manual lists this dip as "Unused" */
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Coinage ) )  PORT_DIPLOCATION( "DSW1:4,5" )
//  PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) ) /* 2 Coins to Start, 1 Coin to Continue??? */
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Save Scores" )       PORT_DIPLOCATION( "DSW1:6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )       // Clear NVRAM on boot
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "DSW1:7" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "DSW1:8" )   /* Manual lists this dip as "Unused" */

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0001, 0x0001, "Invert X Axis" )     PORT_DIPLOCATION( "DSW2:1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "DSW2:2" )   /* Manual lists this dip as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "DSW2:3" )   /* Manual lists this dip as "Unused" */
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION( "DSW2:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW2:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Damage From Machine Gun" )   PORT_DIPLOCATION( "DSW2:6" )    // F76E34
	PORT_DIPSETTING(      0x0020, "Light" )
	PORT_DIPSETTING(      0x0000, "Heavy" )
	PORT_DIPNAME( 0x0040, 0x0040, "Damage From Beam Cannon" )   PORT_DIPLOCATION( "DSW2:7" )    // F77487
	PORT_DIPSETTING(      0x0040, "Light" )
	PORT_DIPSETTING(      0x0000, "Heavy" )
	PORT_DIPNAME( 0x0080, 0x0080, "Damage From Missle" )    PORT_DIPLOCATION( "DSW2:8" )    // F77255
	PORT_DIPSETTING(      0x0080, "Light" )
	PORT_DIPSETTING(      0x0000, "Heavy" )

	PORT_START("GUNX1") // IN5 - $540000(0)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNY1") // IN6 - $540000(1)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNX2") // IN7 - $540000(2)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNY2") // IN8 - $540000(3)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


/***************************************************************************
                                Hyper Reaction
***************************************************************************/

static INPUT_PORTS_START( hypreact )
	PORT_START("DSW1")  // IN0 - $210002
	SSV_COINAGE_STANDARD( 0, 0x07, DEF_STR( Coin_A ), "DSWA:1,2,3" )
	SSV_COINAGE_STANDARD( 3, 0x07, DEF_STR( Coin_B ), "DSWA:4,5,6" )
	PORT_DIPNAME( 0x0040, 0x0040, "Half Coins To Continue" )    PORT_DIPLOCATION( "DSWA:7" )
	PORT_DIPSETTING(      0x0040, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )        PORT_DIPLOCATION( "DSWA:8" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")  // IN1 - $210004
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION( "DSWB:1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION( "DSWB:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION( "DSWB:3,4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Controls ) )         PORT_DIPLOCATION( "DSWB:5" )
	PORT_DIPSETTING(      0x0010, "Keyboard" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Multiple coins" )            PORT_DIPLOCATION( "DSWB:6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Keep Status On Continue" )   PORT_DIPLOCATION( "DSWB:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "DSWB:8" )

	PORT_START("P1")    // IN2 - $210008 (used in joystick mode)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_NAME("P1 Mahjong Chi (Joy Mode)")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_NAME("P1 Mahjong Pon (Joy Mode)")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_NAME("P1 Mahjong Kan (Joy Mode)")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   )

	PORT_START("P2")    // IN3 - $21000a (used in joystick mode)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_NAME("P1 Mahjong Reach (Joy Mode)")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_NAME("P1 Mahjong Ron (Joy Mode)")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_NAME("Tsumo")// Is this correct?
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("SYSTEM")    // IN4 - $21000c
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(10)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service coin & bet
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("KEY0")  // IN5 - $c00000(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")  // IN6 - $c00000(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")  // IN7 - $c00000(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")  // IN8 - $c00000(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Hyper Reaction 2
***************************************************************************/

static INPUT_PORTS_START( hypreac2 )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	SSV_COINAGE_STANDARD( 0, 0x07, DEF_STR( Coin_A ), "DSWA:1,2,3" )
	SSV_COINAGE_STANDARD( 3, 0x07, DEF_STR( Coin_B ), "DSWA:4,5,6" )
	PORT_DIPNAME( 0x0040, 0x0040, "Half Coins To Continue" )    PORT_DIPLOCATION( "DSWA:7" )
	PORT_DIPSETTING(      0x0040, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )        PORT_DIPLOCATION( "DSWA:8" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION( "DSWB:1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION( "DSWB:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION( "DSWB:3,4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Controls ) )         PORT_DIPLOCATION( "DSWB:5" )
	PORT_DIPSETTING(      0x0010, "Keyboard" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Communication 1" )           PORT_DIPLOCATION( "DSWB:6" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Communication 2" )           PORT_DIPLOCATION( "DSWB:7" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "DSWB:8" )

	PORT_START("KEY0")  // IN5 - $500000(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")  // IN6 - $500000(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")  // IN7 - $500000(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")  // IN8 - $500000(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                JangJang Shimasho
***************************************************************************/

static INPUT_PORTS_START( janjans1 )
	PORT_INCLUDE(ssv_mahjong)

	PORT_MODIFY("DSW1") // IN0 - $210002
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "DSW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "DSW1:2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "DSW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW1:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	SSV_COINAGE_BASIC( 4, 0x03, DEF_STR( Coinage ), "DSW1:5,6" )
	PORT_DIPNAME( 0x0040, 0x0040, "Voice" )                 PORT_DIPLOCATION( "DSW1:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "DSW1:8" )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "DSW2:1,2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Nudity" )                PORT_DIPLOCATION( "DSW2:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Mini Game" )             PORT_DIPLOCATION( "DSW2:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, "Initial Score" )         PORT_DIPLOCATION( "DSW2:5,6" )
	PORT_DIPSETTING(      0x0020, "1000" )
	PORT_DIPSETTING(      0x0030, "1500" )
	PORT_DIPSETTING(      0x0010, "2000" )
	PORT_DIPSETTING(      0x0000, "3000" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Communication" )         PORT_DIPLOCATION( "DSW2:7,8" )
//  PORT_DIPSETTING(      0x0080, "unused" )
	PORT_DIPSETTING(      0x00c0, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0040, "Board 1 (Main)" )
	PORT_DIPSETTING(      0x0000, "Board 2 (Sub)" )
INPUT_PORTS_END


/***************************************************************************
                                JangJang Shimasho 2
***************************************************************************/

static INPUT_PORTS_START( janjans2 )
	PORT_INCLUDE(ssv_mahjong)

	PORT_MODIFY("DSW1") // IN0 - $210002
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "DSW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION( "DSW1:2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "DSW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION( "DSW1:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	SSV_COINAGE_BASIC( 4, 0x03, DEF_STR( Coinage ), "DSW1:5,6" )
	PORT_DIPNAME( 0x0040, 0x0040, "Nudity" )                    PORT_DIPLOCATION( "DSW1:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "DSW1:8" )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION( "DSW2:1,2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Initial Score (vs player)")  PORT_DIPLOCATION( "DSW2:3,4" )
	PORT_DIPSETTING(      0x0008, "10000" )
	PORT_DIPSETTING(      0x0004, "15000" )
	PORT_DIPSETTING(      0x000c, "20000" )
	PORT_DIPSETTING(      0x0000, "25000" )
	PORT_DIPNAME( 0x0030, 0x0030, "Initial Score (vs CPU)" )    PORT_DIPLOCATION( "DSW2:5,6" )
	PORT_DIPSETTING(      0x0020, "1000" )
	PORT_DIPSETTING(      0x0030, "1500" )
	PORT_DIPSETTING(      0x0010, "2000" )
	PORT_DIPSETTING(      0x0000, "3000" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Communication" )             PORT_DIPLOCATION( "DSW2:7,8" )
//  PORT_DIPSETTING(      0x0080, "unused" )
	PORT_DIPSETTING(      0x00c0, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0040, "Transmitter" )
	PORT_DIPSETTING(      0x0000, "Receiver" )
INPUT_PORTS_END


/***************************************************************************
                           Joryuu Syougi Kyoushitsu
***************************************************************************/

static INPUT_PORTS_START( jsk )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_SERVICE_DIPLOC( 0x0008, IP_ACTIVE_LOW, "DSW1:4" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Minutes" )               PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(      0x0080, "3" )
	PORT_DIPSETTING(      0x00c0, "4" )
	PORT_DIPSETTING(      0x0040, "5" )
	PORT_DIPSETTING(      0x0000, "6" )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0007, 0x0007, "Difficulty A" )          PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0000, "1 (Novice)" )        // 8 fixed levels
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0003, "4" )
	PORT_DIPSETTING(      0x0007, "5 (Medium)" )
	PORT_DIPSETTING(      0x0006, "6" )
	PORT_DIPSETTING(      0x0005, "7" )
	PORT_DIPSETTING(      0x0004, "8 (expert)"    )
	PORT_DIPNAME( 0x0008, 0x0008, "Difficulty Switch" )     PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0008, "A (8 Levels)" )
	PORT_DIPSETTING(      0x0000, "B (4 Levels)" )
	PORT_DIPNAME( 0x0030, 0x0030, "Difficulty B" )          PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Easy )    )  // 4 levels, and player can select 3 levels during game
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Campaign" )              PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0040, "Available" )
	PORT_DIPSETTING(      0x0000, "Finished" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************
                                Keith & Lucy
***************************************************************************/

static INPUT_PORTS_START( keithlcy )
	PORT_INCLUDE(ssv_quiz)

	PORT_MODIFY("DSW1") // IN0 - $210002
	SSV_COINAGE_BASIC( 6, 0x03, DEF_STR( Coin_B ), "DSW1:7,8" )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )        PORT_DIPLOCATION( "DSW2:3,4" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION( "DSW2:5,6" )
	PORT_DIPSETTING(      0x0030, "Every 100k" )        //100
	PORT_DIPSETTING(      0x0020, "Every 150k" )        //150
	PORT_DIPSETTING(      0x0010, "100k & Every 200K" ) //100
	PORT_DIPSETTING(      0x0000, "Every 200k" )        //200
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "DSW2:7" ) /* Manual lists these dips as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "DSW2:8" ) /* Manual lists these dips as "Unused" */
INPUT_PORTS_END


/***************************************************************************
                            Koi Koi Simasho 2
***************************************************************************/

static INPUT_PORTS_START( koikois2 )
	PORT_INCLUDE(ssv_mahjong)

	PORT_MODIFY("DSW1") // IN0 - $210002
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "DSW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "DSW1:2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "DSW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW1:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	SSV_COINAGE_BASIC( 4, 0x03, DEF_STR( Coinage ), "DSW1:5,6" )
	PORT_DIPNAME( 0x0040, 0x0040, "Voice" )                 PORT_DIPLOCATION( "DSW1:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Controls ) )     PORT_DIPLOCATION( "DSW1:8" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0000, "Keyboard" )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "DSW2:1,2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Nudity" )                PORT_DIPLOCATION( "DSW2:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "DSW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "DSW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "DSW2:6" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Communication" )         PORT_DIPLOCATION( "DSW2:7,8" )
//  PORT_DIPSETTING(      0x0080, "unused" )
	PORT_DIPSETTING(      0x00c0, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0040, "Board 1 (Main)" )
	PORT_DIPSETTING(      0x0000, "Board 2 (Sub)" )
INPUT_PORTS_END


/***************************************************************************
                                Meosis Magic
***************************************************************************/

static INPUT_PORTS_START( meosism )
	PORT_START("DSW1")  // IN0 - $210002
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coinage ) )      PORT_DIPLOCATION( "DSW1:1,2" )
	PORT_DIPSETTING(      0x0003, "1 Medal/1 Credit" )
	PORT_DIPSETTING(      0x0001, "1 Medal/5 Credits" )
	PORT_DIPSETTING(      0x0002, "1 Medal/10 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Medal/20 Credits" )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW1:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Attendant Pay" )         PORT_DIPLOCATION( "DSW1:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Medals Payout" )         PORT_DIPLOCATION( "DSW1:5" )
	PORT_DIPSETTING(      0x0010, "400" )
	PORT_DIPSETTING(      0x0000, "800" )
	PORT_DIPNAME( 0x0020, 0x0020, "Max Credits" )           PORT_DIPLOCATION( "DSW1:6" )
	PORT_DIPSETTING(      0x0020, "5000" )
	PORT_DIPSETTING(      0x0000, "9999" )
	PORT_DIPNAME( 0x0040, 0x0040, "Hopper" )                PORT_DIPLOCATION( "DSW1:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Reel Speed" )            PORT_DIPLOCATION( "DSW1:8" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )

	PORT_START("DSW2")  // IN1 - $210004
	PORT_DIPNAME( 0x0003, 0x0003, "Game Rate" )             PORT_DIPLOCATION( "DSW2:1,2" )
	PORT_DIPSETTING(      0x0000, "80%" )
	PORT_DIPSETTING(      0x0002, "85%" )
	PORT_DIPSETTING(      0x0003, "90%" )
	PORT_DIPSETTING(      0x0001, "95%" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "DSW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "DSW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "DSW2:5" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Controls ) )     PORT_DIPLOCATION( "DSW2:6" )
	PORT_DIPSETTING(      0x0020, "Simple" )
	PORT_DIPSETTING(      0x0000, "Complex" )
	PORT_DIPNAME( 0x0040, 0x0000, "Coin Sensor" )           PORT_DIPLOCATION( "DSW2:7" )
	PORT_DIPSETTING(      0x0040, "Active High" )
	PORT_DIPSETTING(      0x0000, "Active Low" )
	PORT_DIPNAME( 0x0080, 0x0080, "Hopper Sensor" )         PORT_DIPLOCATION( "DSW2:8" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1")    // IN2 - $210008
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4        )   //bet
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3        )   //stop/r
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2        )   //stop/c
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1        )   //stop/l
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )   //no
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )   //yes
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1         )   //start
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN        )   //-

	PORT_START("P2")    // IN3 - $21000a
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  ) //-
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN  ) //-
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Payout") //payout
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  ) //-
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_TILT     ) //reset
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  ) //-
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  ) //-

	PORT_START("SYSTEM")    // IN4 - $21000c
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
//  PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(10)  // Should work but doesn't
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) //service coin
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Analyzer") //analyzer
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5  ) //max bet
	PORT_BIT( 0x00e2, IP_ACTIVE_LOW, IPT_UNKNOWN  )
INPUT_PORTS_END


/***************************************************************************
                                Monster Slider
***************************************************************************/

static INPUT_PORTS_START( mslider )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	SSV_COINAGE_STANDARD( 0, 0x07, DEF_STR( Coin_A ), "DSW1:1,2,3" )
	SSV_COINAGE_STANDARD( 3, 0x07, DEF_STR( Coin_B ), "DSW1:4,5,6" )
	PORT_SERVICE_DIPLOC( 0x0040, IP_ACTIVE_LOW, "DSW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "DSW1:8" ) /* Manual lists this dip as "Unused" */

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "DSW2:1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW2:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "DSW2:3,4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0030, 0x0030, "Rounds (Vs Mode)" )      PORT_DIPLOCATION( "DSW2:5,6" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0030, "2" )
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "DSW2:7" ) /* Manual lists these dips as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "DSW2:8" )
INPUT_PORTS_END


/***************************************************************************
                    Gourmet Battle Quiz Ryohrioh CooKing
***************************************************************************/

static INPUT_PORTS_START( ryorioh )
	PORT_INCLUDE(ssv_quiz)

	PORT_MODIFY("DSW1") // IN0 - $210002
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "DSW1:7" ) /* Manual states this dip is "Unused" */
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "DSW1:8" )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "DSW2:3" ) /* Manual states dips 3-8 are "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "DSW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "DSW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "DSW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "DSW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "DSW2:8" )
INPUT_PORTS_END


/***************************************************************************
                            Super Real Mahjong PIV
***************************************************************************/

static INPUT_PORTS_START( srmp4 )
	PORT_INCLUDE(ssv_mahjong)

	PORT_MODIFY("DSW1") // IN0 - $210002
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )           PORT_DIPLOCATION( "DSW1:1,2,3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )           PORT_DIPLOCATION( "DSW1:4,5,6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )        PORT_DIPLOCATION( "DSW1:7" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "DSW1:8" )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) )       PORT_DIPLOCATION( "DSW2:1,2,3" )
	PORT_DIPSETTING(      0x0006, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION( "DSW2:4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION( "DSW2:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0020, IP_ACTIVE_LOW, "DSW2:6" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION( "DSW2:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "DSW2:8" )
INPUT_PORTS_END


/***************************************************************************
                            Super Real Mahjong P7
***************************************************************************/

static INPUT_PORTS_START( srmp7 )
	PORT_START("DSW1")  // IN0 - $210002
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )          PORT_DIPLOCATION( "DSW1:1,2,3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "DSW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "DSW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "DSW1:6" )
	PORT_DIPNAME( 0x0040, 0x0040, "Re-cloth" )                  PORT_DIPLOCATION( "DSW1:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Nudity" )                    PORT_DIPLOCATION( "DSW1:8" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("DSW2")  // IN1 - $210004
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) )       PORT_DIPLOCATION( "DSW2:1,2,3" )
	PORT_DIPSETTING(      0x0006, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Kuitan" )                    PORT_DIPLOCATION( "DSW2:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION( "DSW2:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION( "DSW2:6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION( "DSW2:7" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "DSW2:8" )

	PORT_START("P1")    // IN2 - $210008
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")    // IN3 - $21000a
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // IN4 - $21000c
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(10)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // tested
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("KEY0")  // IN6 - $600000(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")  // IN7 - $600000(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")  // IN8 - $600000(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")  // IN5 - $600000(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Storm Blade
***************************************************************************/

static INPUT_PORTS_START( stmblade )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	SSV_COINAGE_STANDARD( 0, 0x07, DEF_STR( Coin_A ), "DSW1:1,2,3" )
	SSV_COINAGE_STANDARD( 3, 0x07, DEF_STR( Coin_B ), "DSW1:4,5,6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "DSW1:7" )
	PORT_DIPNAME( 0x0080, 0x0080, "Rapid Fire" )            PORT_DIPLOCATION( "DSW1:8" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "DSW2:1" ) /* works opposite of expected, emulation issue?? */
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW2:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "DSW2:3,4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) )        PORT_DIPLOCATION( "DSW2:5,6" )
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION( "DSW2:7" )
	PORT_DIPSETTING(      0x0040, "600000" )
	PORT_DIPSETTING(      0x0000, "800000" )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "DSW2:8" )
INPUT_PORTS_END


/***************************************************************************
                                Survival Arts
***************************************************************************/

static INPUT_PORTS_START( survarts )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	SSV_COINAGE_EXTENDED( 0, 0x09, DEF_STR( Coin_A ), "DSW1:1,2,3,4" )
	PORT_DIPSETTING(      0x0000, "2 Credits Start, 1 to continue" )
	SSV_COINAGE_EXTENDED( 4, 0x09, DEF_STR( Coin_B ), "DSW1:5,6,7,8" )
	PORT_DIPSETTING(      0x0000, "2 Credits Start, 1 to continue" )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "DSW2:1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "DSW2:2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Timer Speed" )           PORT_DIPLOCATION( "DSW2:3" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_DIPNAME( 0x0008, 0x0008, "Damage Level" )          PORT_DIPLOCATION( "DSW2:4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "DSW2:5,6" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Fatal Damage" )          PORT_DIPLOCATION( "DSW2:7,8" )
	PORT_DIPSETTING(      0x0040, "Light" )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0080, "Heavy" )
	PORT_DIPSETTING(      0x0000, "Heaviest" )

	PORT_START("ADD_BUTTONS")   // IN5 - $500008
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Pachinko Sexy Reaction
***************************************************************************/

static INPUT_PORTS_START( sxyreact )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	SSV_COINAGE_STANDARD( 0, 0x07, DEF_STR( Coin_A ), "DSW1:1,2,3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "DSW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "DSW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "DSW1:6" )
	//SSV_COINAGE_STANDARD( 3, 0x07, DEF_STR( Coin_B ), "DSW1:4,5,6" )
	PORT_DIPNAME( 0x0040, 0x0040, "Credits To Play" )           PORT_DIPLOCATION( "DSW1:7" )
	PORT_DIPSETTING(      0x0040, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0080, 0x0080, "Buy Balls With Credits" )    PORT_DIPLOCATION( "DSW1:8" )    // press start
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION( "DSW2:1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION( "DSW2:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION( "DSW2:3,4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Controls ) )         PORT_DIPLOCATION( "DSW2:5" )
	PORT_DIPSETTING(      0x0010, "Dial" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Free_Play ) )        PORT_DIPLOCATION( "DSW2:6" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0040, IP_ACTIVE_LOW, "DSW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "DSW2:8" )

	PORT_START("SERVICE")   // IN5 - $500002
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE2 )    // ball switch on -> handle motor off

	PORT_START("PADDLE")    // IN6 - $500004
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_MINMAX(0,0xcf) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M)
INPUT_PORTS_END


/***************************************************************************
                                Twin Eagle II
***************************************************************************/

static INPUT_PORTS_START( twineag2 )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	SSV_COINAGE_EXTENDED( 0, 0x0f, DEF_STR( Coin_A ), "DSW1:1,2,3,4" )
	//PORT_DIPSETTING(      0x0000, "???" ) - No values listed for all "ON"
	SSV_COINAGE_EXTENDED( 4, 0x0f, DEF_STR( Coin_B ), "DSW1:5,6,7,8" )
	//PORT_DIPSETTING(      0x0000, "???" ) - No values listed for all "ON"

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "DSW2:1,2,3" )
	PORT_DIPSETTING(      0x0006, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Free_Play ) )    PORT_DIPLOCATION( "DSW2:4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Lives ) )        PORT_DIPLOCATION( "DSW2:5" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Pause ) )        PORT_DIPLOCATION( "DSW2:6" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "DSW2:7" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "DSW2:8" )

// Not shown in service mode I/O or show any function in game.  Valid?
//  PORT_MODIFY("SYSTEM")   // IN4 - $21000c
//  PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
//  PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE4 )
//  PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START3 )
//  PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START4 )

INPUT_PORTS_END


/***************************************************************************
                                Ultra X
***************************************************************************/

static INPUT_PORTS_START( ultrax )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("DSW1") // IN0 - $210002
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DSW1:1,2,3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DSW1:5,6,7,8")
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_6C ) )

	PORT_MODIFY("DSW2") // IN1 - $210004
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )     //$140
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )   //$190
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )     //$200
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )  //$300
	PORT_DIPNAME( 0x0014, 0x0004, DEF_STR( Region ) )               PORT_DIPLOCATION("DSW2:3,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( China ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( Japan ) )
//PORT_DIPSETTING(      0x0010, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( World ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	// country            0x0010
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "DSW2:8" )
INPUT_PORTS_END


/***************************************************************************
                               Vasara
***************************************************************************/

static INPUT_PORTS_START( vasara )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("SYSTEM")   // IN4 - $21000c
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1") // IN0 - $210002
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Free_Play )  )   PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "DSW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	SSV_COINAGE_BASIC( 4, 0x03, DEF_STR( Coin_A ), "DSW1:5,6" )
	SSV_COINAGE_BASIC( 6, 0x03, DEF_STR( Coin_B ), "DSW1:7,8" )

	PORT_MODIFY("DSW2") // IN1
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Bomber Stock" )          PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0004, "1" )
	PORT_DIPSETTING(      0x000c, "2" )
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ))         PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, "Game Voice" )            PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "English Subtitles" )     PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
                               Vasara 2
***************************************************************************/

static INPUT_PORTS_START( vasara2 )
	PORT_INCLUDE(ssv_joystick)

	PORT_MODIFY("SYSTEM")   // IN4 - $21000c
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1") // IN0 - $210002
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Free_Play )  )   PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "DSW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	SSV_COINAGE_BASIC( 4, 0x03, DEF_STR( Coin_A ), "DSW1:5,6" )
	SSV_COINAGE_BASIC( 6, 0x03, DEF_STR( Coin_B ), "DSW1:7,8" )

	PORT_MODIFY("DSW2") // IN1
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0008, "5" )
	PORT_DIPNAME( 0x0010, 0x0010, "Game Voice" )            PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Vasara Stock" )          PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0040, 0x0040, "English Subtitles" )     PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Secret Character" )      PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



/***************************************************************************


                            Graphics Layouts


***************************************************************************/

/*  16 x 8 tiles. Depth is 8 bits, but can be decreased to 6 (and maybe
    less) at runtime.   */

static const gfx_layout layout_16x8x8 =
{
	16,8,
	RGN_FRAC(1,4),
	8,
	{   RGN_FRAC(3,4)+8, RGN_FRAC(3,4)+0,
		RGN_FRAC(2,4)+8, RGN_FRAC(2,4)+0,
		RGN_FRAC(1,4)+8, RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8, RGN_FRAC(0,4)+0    },
	{   STEP8(0,1), STEP8(16,1) },
	{   STEP8(0,16*2)   },
	16*8*2
};

static const gfx_layout layout_16x8x6 =
{
	16,8,
	RGN_FRAC(1,4),
	6,
	{
		RGN_FRAC(2,4)+8, RGN_FRAC(2,4)+0,
		RGN_FRAC(1,4)+8, RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8, RGN_FRAC(0,4)+0    },
	{   STEP8(0,1), STEP8(16,1) },
	{   STEP8(0,16*2)   },
	16*8*2
};

static GFXDECODE_START( ssv )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x8x8, 0, 0x8000/64 ) // [0] Sprites (256 colors)
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x8x6, 0, 0x8000/64 ) // [1] Sprites (64 colors)
GFXDECODE_END

static const gfx_layout layout_16x8x8_ram =
{
	16,8,
	0x40000 * 16 / (16 * 8),
	8,
	{   STEP8(0,1)      },
	{   STEP16(0,8)     },
	{   STEP8(0,16*8)   },
	16*8*8
};

static const gfx_layout layout_16x8x6_ram =
{
	16,8,
	0x40000 * 16 / (16 * 8),
	6,
	{   2,3,4,5,6,7     },
	{   STEP16(0,8)     },
	{   STEP8(0,16*8)   },
	16*8*8
};

static GFXDECODE_START( eaglshot )
	GFXDECODE_ENTRY( nullptr, 0, layout_16x8x8_ram, 0, 0x8000/64 ) // [0] Sprites (256 colors, decoded from ram)
	GFXDECODE_ENTRY( nullptr, 0, layout_16x8x6_ram, 0, 0x8000/64 ) // [1] Sprites (64 colors, decoded from ram)
GFXDECODE_END

static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{   STEP8(0,1)      },
	{   STEP16(0,8)     },
	{   STEP16(0,16*8)  },
	16*16*8
};

static GFXDECODE_START( gdfs )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x8x8,   0, 0x8000/64  ) // [0] Sprites (256 colors)
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x8x6,   0, 0x8000/64  ) // [1] Sprites (64 colors)
	GFXDECODE_ENTRY( "gfx3", 0, layout_16x16x8,  0, 0x8000/256 ) // [3] Tilemap
GFXDECODE_END

/***************************************************************************


                                Machine Drivers


***************************************************************************/

/***************************************************************************

    Some games (e.g. hypreac2) oddly map the high bits of the tile code
    to the gfx roms: arranging the roms accordingly would waste tens of
    megabytes. So we use a look-up table.

    We also need to set up game specific offsets for sprites and layers
    (at least until the CRT controlled will be emulated).

***************************************************************************/

void ssv_state::init(int interrupt_ultrax)
{
	int i;
	for (i = 0; i < 16; i++)
		m_tile_code[i]   =   ( (i & 8) ? (1 << 16) : 0 ) +
								( (i & 4) ? (2 << 16) : 0 ) +
								( (i & 2) ? (4 << 16) : 0 ) +
								( (i & 1) ? (8 << 16) : 0 ) ;
	enable_video(1);
	m_interrupt_ultrax = interrupt_ultrax;

	save_item(NAME(m_requested_int));
	save_item(NAME(m_irq_enable));
}

void ssv_state::init_hypreac2_common()
{
	int i;

	for (i = 0; i < 16; i++)
		m_tile_code[i]   =   (i << 16);
}

void ssv_state::init_eaglshot_banking()
{
	init_hypreac2_common();
	membank("gfxrom")->configure_entries(0, 6+1, memregion("gfxdata")->base(), 0x200000);
}

// massages the data from the BPMicro-compatible dump to runnable form
void ssv_state::init_st010()
{
	UINT8 *dspsrc = (UINT8 *)memregion("st010")->base();
	UINT32 *dspprg = (UINT32 *)memregion("dspprg")->base();
	UINT16 *dspdata = (UINT16 *)memregion("dspdata")->base();

	// copy DSP program
	for (int i = 0; i < 0x10000; i+= 4)
	{
		*dspprg = dspsrc[0+i]<<24 | dspsrc[1+i]<<16 | dspsrc[2+i]<<8;
		dspprg++;
	}

	// copy DSP data
	for (int i = 0; i < 0x1000; i+= 2)
	{
		*dspdata++ = dspsrc[0x10000+i]<<8 | dspsrc[0x10001+i];
	}
}

DRIVER_INIT_MEMBER(ssv_state,drifto94)     {    init(0); init_st010();  }
DRIVER_INIT_MEMBER(ssv_state,eaglshot)     {    init(0); init_eaglshot_banking(); save_item(NAME(m_trackball_select)); }
DRIVER_INIT_MEMBER(ssv_state,gdfs)         {    init(0); save_item(NAME(m_gdfs_lightgun_select)); save_item(NAME(m_gdfs_eeprom_old)); }
DRIVER_INIT_MEMBER(ssv_state,hypreact)     {    init(0); }
DRIVER_INIT_MEMBER(ssv_state,hypreac2)     {    init(0); init_hypreac2_common();    }
DRIVER_INIT_MEMBER(ssv_state,janjans1)     {    init(0); }
DRIVER_INIT_MEMBER(ssv_state,keithlcy)     {    init(0); }
DRIVER_INIT_MEMBER(ssv_state,meosism)       {   init(0); }
DRIVER_INIT_MEMBER(ssv_state,mslider)       {   init(0); }
DRIVER_INIT_MEMBER(ssv_state,ryorioh)       {   init(0); }
DRIVER_INIT_MEMBER(ssv_state,srmp4)        {    init(0);
//  ((UINT16 *)memregion("maincpu")->base())[0x2b38/2] = 0x037a;   /* patch to see gal test mode */
}
DRIVER_INIT_MEMBER(ssv_state,srmp7)        {    init(0); }
DRIVER_INIT_MEMBER(ssv_state,stmblade)     {    init(0); init_st010(); }
DRIVER_INIT_MEMBER(ssv_state,survarts)     {    init(0); }
DRIVER_INIT_MEMBER(ssv_state,dynagear)     {    init(0); }
DRIVER_INIT_MEMBER(ssv_state,sxyreact)     {    init(0); init_hypreac2_common();  save_item(NAME(m_sxyreact_serial)); save_item(NAME(m_sxyreact_dial)); }
DRIVER_INIT_MEMBER(ssv_state,cairblad)     {    init(0); init_hypreac2_common();    }
DRIVER_INIT_MEMBER(ssv_state,sxyreac2)     {    init(0); init_hypreac2_common();  save_item(NAME(m_sxyreact_serial)); save_item(NAME(m_sxyreact_dial)); }
DRIVER_INIT_MEMBER(ssv_state,twineag2)     {    init(1); init_st010();  }
DRIVER_INIT_MEMBER(ssv_state,ultrax)        {   init(1); }
DRIVER_INIT_MEMBER(ssv_state,vasara)        {   init(0); }
DRIVER_INIT_MEMBER(ssv_state,jsk)          {    init(0); save_item(NAME(m_latches)); }

#define SSV_MASTER_CLOCK XTAL_48MHz/3

#define SSV_PIXEL_CLOCK XTAL_42_9545MHz/6
#define SSV_HTOTAL 0x1c6
#define SSV_HBEND 0
#define SSV_HBSTART 0x150
#define SSV_VTOTAL 0x106
#define SSV_VBEND 0
#define SSV_VBSTART 0xf0

static MACHINE_CONFIG_START( ssv, ssv_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V60, SSV_MASTER_CLOCK) /* Based on STA-0001 & STA-0001B System boards */
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(ssv_state,irq_callback)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", ssv_state, interrupt, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(SSV_PIXEL_CLOCK,SSV_HTOTAL,SSV_HBEND,SSV_HBSTART,SSV_VTOTAL,SSV_VBEND,SSV_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(ssv_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ssv)
	MCFG_PALETTE_ADD("palette", 0x8000)
	MCFG_PALETTE_FORMAT(XRGB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ensoniq", ES5506, SSV_MASTER_CLOCK)
	MCFG_ES5506_REGION0("ensoniq.0")
	MCFG_ES5506_REGION1("ensoniq.1")
	MCFG_ES5506_REGION2("ensoniq.2")
	MCFG_ES5506_REGION3("ensoniq.3")
	MCFG_ES5506_CHANNELS(1)               /* channels */
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( drifto94, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(drifto94_map)

	MCFG_CPU_ADD("dsp", UPD96050, 10000000) /* TODO: correct? */
	MCFG_CPU_PROGRAM_MAP(dsp_prg_map)
	MCFG_CPU_DATA_MAP(dsp_data_map)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xcd-0x25)*2-1, 0, (0x101-0x13)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( gdfs, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(gdfs_map)
	MCFG_TIMER_MODIFY("scantimer")
	MCFG_TIMER_DRIVER_CALLBACK(ssv_state, gdfs_interrupt)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xd5-0x2c)*2-1, 0, (0x102-0x12)-1)
	MCFG_SCREEN_UPDATE_DRIVER(ssv_state, screen_update_gdfs)

	MCFG_DEVICE_ADD("st0020_spr", ST0020_SPRITES, 0)
	MCFG_ST0020_SPRITES_PALETTE("palette")

	MCFG_GFXDECODE_MODIFY("gfxdecode", gdfs)
	MCFG_VIDEO_START_OVERRIDE(ssv_state,gdfs)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( hypreact, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hypreact_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xcb-0x22)*2-1, 0, (0xfe - 0x0e)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( hypreac2, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hypreac2_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xcb-0x22)*2-1, 0, (0xfe - 0x0e)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( janjans1, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(janjans1_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xcb-0x23)*2-1, 0, (0xfe - 0x0e)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( keithlcy, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(keithlcy_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xcd-0x25)*2-1, 0, (0x101 - 0x13)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( meosism, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(meosism_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xd5-0x2c)*2-1, 0, (0xfe - 0x12)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mslider, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mslider_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xd6-0x26)*2-1, 0, (0xfe - 0x0e)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ryorioh, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ryorioh_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xcb-0x23)*2-1, 0, (0xfe - 0x0e)-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( vasara, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ryorioh_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xcc-0x24)*2-1, 0,(0xfe - 0x0e)-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( srmp4, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(srmp4_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xd4-0x2c)*2-1, 0, (0x102 - 0x12)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( srmp7, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(srmp7_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xd4-0x2c)*2-1, 0, (0xfd - 0x0e)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( stmblade, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(drifto94_map)

	MCFG_CPU_ADD("dsp", UPD96050, 10000000)
	MCFG_CPU_PROGRAM_MAP(dsp_prg_map)
	MCFG_CPU_DATA_MAP(dsp_data_map)

	/* don't need this, game just does a simple check at boot then the DSP stalls into a tight loop. */
//  MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_NVRAM_ADD_0FILL("nvram")
	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xd6-0x26)*2-1, 0, (0xfe - 0x0e)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( survarts, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(survarts_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xd4-0x2c)*2-1, 0, (0x102 - 0x12)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( dynagear, survarts )

	/* basic machine hardware */
	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xd4-0x2c)*2-1, 0, (0x102 - 0x12)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( eaglshot, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(eaglshot_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xca - 0x2a)*2-1, 0, (0xf6 - 0x16)-1)
	MCFG_SCREEN_UPDATE_DRIVER(ssv_state, screen_update_eaglshot)

	MCFG_GFXDECODE_MODIFY("gfxdecode", eaglshot)
	MCFG_VIDEO_START_OVERRIDE(ssv_state,eaglshot)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sxyreact, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sxyreact_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xcb - 0x22)*2-1, 0, (0xfe - 0x0e)-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sxyreac2, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sxyreact_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xcb - 0x23)*2-1, 0, (0xfe - 0x0e)-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cairblad, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sxyreact_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xcb - 0x22)*2-1, 0, (0xfe - 0x0e)-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( twineag2, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(twineag2_map)

	MCFG_CPU_ADD("dsp", UPD96050, 10000000)
	MCFG_CPU_PROGRAM_MAP(dsp_prg_map)
	MCFG_CPU_DATA_MAP(dsp_data_map)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xd4 - 0x2c)*2-1, 0, (0x102 - 0x12)-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ultrax, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ultrax_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xd4 - 0x2c)*2-1, 0, (0x102 - 0x12)-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( jsk, ssv )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(jsk_map)

	MCFG_CPU_ADD("sub", V810,25000000)
	MCFG_CPU_PROGRAM_MAP(jsk_v810_mem)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, (0xca - 0x22)*2-1, 0, (0xfe - 0x0e)-1)
MACHINE_CONFIG_END


/***************************************************************************


                                ROMs Loading


***************************************************************************/


/***************************************************************************

                        Change Air Blade (Japan)

Change Air Blade
Sammy, 1999

ROM board for use with System SSV Main Board
PCB No: P1-112C

Fairly sparsely populated board containing not much except....

RAM   : 6262 (x1)
OTHER : 3.6V Ni-Cd Battery
PALs  : (x1, labelled AC412G00)

ROMs  : (Filename  = ROM Label)
        (Extension = PCB Location)
------------------------------
AC1801M01.U6    32M Mask
AC1802M01.U9    32M Mask

AC1805M01.U8    32M Mask
AC1806M01.U11   32M Mask

AC1803M01.U7    32M Mask
AC1804M01.U10   32M Mask

AC1807M01.U41   32M Mask
AC1810E01.U32   27C160


***************************************************************************/

ROM_START( cairblad )
	ROM_REGION( 0x200000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "ac1810e0.u32",  0x000000, 0x200000, CRC(13a0b4c2) SHA1(3498303e9b186ab329ee761cee9d4cb8ed552455) ) // AC1810E01.U32    27C160

	ROM_REGION( 0x2000000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "ac1801m0.u6",  0x0000000, 0x400000, CRC(1b2b6943) SHA1(95c5dc0ed1d533b2285452c8546346d96a90d097) ) // AC1801M01.U6    32M Mask
	ROM_LOAD( "ac1802m0.u9",  0x0400000, 0x400000, CRC(e053b087) SHA1(9569e79c6363e8f97c27aacaa29d25cf32c4b4c1) ) // AC1802M01.U9    32M Mask

	ROM_LOAD( "ac1803m0.u7",  0x0800000, 0x400000, CRC(45484866) SHA1(5e2f06743906be298202eafc233b76762d60d8aa) ) // AC1803M01.U7    32M Mask
	ROM_LOAD( "ac1804m0.u10", 0x0c00000, 0x400000, CRC(5e0b2285) SHA1(b3b8f249c1b1b2e9438ebc3a669f3ebfb5aa5feb) ) // AC1804M01.U10   32M Mask

	ROM_LOAD( "ac1805m0.u8",  0x1000000, 0x400000, CRC(19771f43) SHA1(d6a05392c58d3f60d666e08b3a82f06fa2c8e3a3) ) // AC1805M01.U8    32M Mask
	ROM_LOAD( "ac1806m0.u11", 0x1400000, 0x400000, CRC(816b97dc) SHA1(3737cb37a4db720901661fa9b4e30c44181efb94) ) // AC1806M01.U11   32M Mask

	ROM_FILL(                 0x1800000, 0x800000, 0x000000)

	ROM_REGION16_BE( 0x400000, "ensoniq.0", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "ac1410m0.u41", 0x000000, 0x400000, CRC(ecf1f255) SHA1(984b1529b8f0c7d94ea713c85d71df00f54eba79) ) // AC1807M01.U41   32M Mask
ROM_END


/***************************************************************************

                        Drift Out '94 - The hard order

----------------------
System SSV (STA-0001B)
----------------------
CPU  : NEC D70615GD-16-S (V60)
Sound: Ensoniq ES5506 (OTTOR2)
OSC  : 42.9545MHz(X2) 48.0000MHz(X3)

Custom chips:
ST-0004 (Video DAC?)
ST-0005 (Parallel I/O?)
ST-0006 (Video controller)
ST-0007 (System controller)

Program Work RAM  : 256Kbitx2 (expandable to 1Mx2)
Object Work RAM   : 1Mbitx2
Color Palette RAM : 256Kbitx3 (expandable to 1Mx3)

-------------------------
SSV Subboard (VISCO-001B)
-------------------------
ROMs:
visco-33.bin - Main programs (27c4000)
visco-37.bin /

vg003-19.u26 - Data? (mask, read as 27c160)

vg003-17.u22 - Samples (mask, read as 27c160)
vg003-18.u15 /

vg003-01.a0 - Graphics (mask, read as 27c160)
vg003-05.a1 |
vg003-09.a2 |
vg009-13.a3 |
vg009-02.b0 |
vg003-06.b1 |
vg003-10.b2 |
vg003-14.b3 |
vg003-03.c0 |
vg003-07.c1 |
vg003-11.c2 |
vg003-15.c3 |
vg003-04.d0 |
vg003-08.d1 |
vg003-12.d2 |
vg003-16.d3 /

VISCO-001B
|-----------------------|CN1|-|
|                             |
|              VG003-16.D3  L |
|              VG003-12.D2  o |
| VG003-18.U15 VG003-08.D1  g |
| VG003-17.U22 VG003-04.D0  i |
|              VG003-15.C3  c |
|              VG003-11.C2    |
|              VG003-07.C1  c |
| uPD4701AC    VG003-03.C0  h |
| ADC0809      VG003-14.B3  i |
| VG003-19.U26 VG003-10.B2  p |
| VISCO-33.U33 VG003-06.B1  s |
| VISCO-37.U37 VG003-02.B0    |
|6264 VG003-22 VG003-13.A3  x |
|    ST010     VG003-09.A2    |
|              VG003-05.A1  1 |
|3790 BT2* BT1 VG003-01.A0  7 |
|                             |
|-----------------------------|

  VG003-22: GAL16V8B (undumped) at U29
SETA ST010: Custom programmed uPD96050 MCU used for math caculations
ADC0809CCN: 8bit microprocessor compatible A/D converters with 8-Channel Multiplexer
 uPD4701AC: X, Y 2-axis Incremental Encoder Counter
       CN1: 5-pin male header/connector
      3790: MB3790 Bi-CMOS Battery Backup IC
      6264: Sharp LH5168D-10L 8K x 8bit CMOS SRAM
       BT1: CR2032 3v battery

***************************************************************************/

ROM_START( drifto94 )
	ROM_REGION( 0x400000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "vg003-19.u26", 0x000000, 0x200000, CRC(238e5e2b) SHA1(fe58f571857804263642d7d089df962327a007b6) ) // "SoundDriverV1.1a"
	ROM_LOAD16_BYTE( "visco-37.u37", 0x200000, 0x080000, CRC(78fa3ccb) SHA1(0c79ff1aa31e7ca1eeb14fbef7774278fa83ba44) )
	ROM_RELOAD(                      0x300000, 0x080000)
	ROM_LOAD16_BYTE( "visco-33.u33", 0x200001, 0x080000, CRC(88351146) SHA1(1decce44b5d244b57676177f417e4937d7088124) )
	ROM_RELOAD(                      0x300001, 0x080000)

	ROM_REGION( 0x2000000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "vg003-01.a0", 0x0000000, 0x200000, CRC(2812aa1a) SHA1(5046fe51a4ea50051a19cfeeb091c87f0f217fb8) )
	ROM_LOAD( "vg003-05.a1", 0x0200000, 0x200000, CRC(1a1dd910) SHA1(f2252e4cd1b6269036ed02cec9d5a224736c1bce) )
	ROM_LOAD( "vg003-09.a2", 0x0400000, 0x200000, CRC(198f1c06) SHA1(7df5d51aa62f0b609cd1d296a3cfeeb38fbcd9d0) )
	ROM_LOAD( "vg003-13.a3", 0x0600000, 0x200000, CRC(b45b2267) SHA1(66828efcca2050bc1cdca6bbf2e8cf015ff937a8) )

	ROM_LOAD( "vg003-02.b0", 0x0800000, 0x200000, CRC(d7402027) SHA1(32af6d611ea277a860ee10e98f4eee5c4458ef7a) )
	ROM_LOAD( "vg003-06.b1", 0x0a00000, 0x200000, CRC(518c509f) SHA1(e4bcbe1d8644490a58670add40d2908c7acdf989) )
	ROM_LOAD( "vg003-10.b2", 0x0c00000, 0x200000, CRC(c1ee9d8b) SHA1(7425cf92225cd1c8d764aa47db6219a6d8b090a5) )
	ROM_LOAD( "vg003-14.b3", 0x0e00000, 0x200000, CRC(645b672b) SHA1(26dfde289679cd780bc65f4f6783a6a0f8b87818) )

	ROM_LOAD( "vg003-03.c0", 0x1000000, 0x200000, CRC(1ca7163d) SHA1(d8c5fd0054a1bc1fbad5866216f6d83c42436ecd) )
	ROM_LOAD( "vg003-07.c1", 0x1200000, 0x200000, CRC(2ff113bb) SHA1(a482ddd3c86633a79d18a03193d70fc8f0f157d8) )
	ROM_LOAD( "vg003-11.c2", 0x1400000, 0x200000, CRC(f924b105) SHA1(079ad0fc3b34c31a67dd88a442088237a2c03fdd) )
	ROM_LOAD( "vg003-15.c3", 0x1600000, 0x200000, CRC(83623b01) SHA1(026654303fb58958bc7f7be86aeb4fdd08e8be7b) )

	ROM_LOAD( "vg003-04.d0", 0x1800000, 0x200000, CRC(6be9bc62) SHA1(c0b49a558786b50f04c1cd87a11e111ad31b85d9) )
	ROM_LOAD( "vg003-08.d1", 0x1a00000, 0x200000, CRC(a7113cdb) SHA1(74d8bfee7e816d53e60d4e54a2584643562a1ee5) )
	ROM_LOAD( "vg003-12.d2", 0x1c00000, 0x200000, CRC(ac0fd855) SHA1(992ae0d02bcefaa2fad7462b211a49fbd1338b62) )
	ROM_LOAD( "vg003-16.d3", 0x1e00000, 0x200000, CRC(1a5fd312) SHA1(1e67ffa51408de107be75c9c63df6fd1bb6ce6b1) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "vg003-17.u22", 0x000000, 0x200000, CRC(6f9294ce) SHA1(b097defd95eb1d8f00e107d7669f9d33148e75c1) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "vg003-18.u15", 0x000000, 0x200000, CRC(511b3e93) SHA1(09eda175c8f1b21c18645519cc6e89c6ca1fc5de) )

	ROM_REGION( 0x11000, "st010", 0)
	ROM_LOAD( "st010.bin",    0x000000, 0x011000, CRC(aa11ee2d) SHA1(cc1984e989cb94e3dcbb5f99e085b5414e18a017) )
	ROM_REGION( 0x10000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x1000, "dspdata", ROMREGION_ERASEFF)
ROM_END


/***************************************************************************

Eagle Shot Golf
Sammy, 1994

GOLF ROM
|----------------------------------------|
| 514400                    GAL SI3-11   |
|                           GAL SI3-12   |
| 514400                                 |
|          SI003-06.u31    SI003-09.u18  |
| 514400                                 |
|                                        |
| 514400   SI003-05.u30    SI003-10.u20  |
|                                        |
| 514400                                 |
|          SI003-04.u10                  |
| 514400                                 |
|                                        |
| 514400   SI003-03.u11                  |
|                                        |
| 514400                                 |
|          SI003-02.u12                  |
|                                        |
|                                        |
|          SI003-01.u13    SI003-07.U23  |
|                                        |
|                                        |
|                          SI003-08.U24  |
|                                        |
|                             uPD4701    |
|                                        |
|                             mc14584b   |
|                                        |
|                                 P1     |
|-------------------------------||||||---|

 mc14584b - Motorola HEX Schmitt Trigger
  uPD4701 - Mouse/Trackball interface by NEC
   514400 - Hatachi HM514400BS7 1M x 4 DRAM (4 Megs of RAM)
      GAL - GAL16V8B
       P1 - 6 Pin trackball connector (same pinout as Golden Tee 9x series)

Lower PCB
PCB Number: GOLF ROM PCB
RAM       : HM514400 (x8)
PALs      : GAL16V8 (x2) labelled SI3-11 & SI3-12
OTHER     : NEC D4701AC
            Controls probably trackball, has 6 pin connector hooked up to a
            mc14584b Logic IC. Joystick appears to be used also for selecting
            stance, club and direction.

ROMs      : U18 & U20 are used for main program.
            All rest are 16M Mask
            U23 & U24 are sound related, all others for GFX.

Loc  ROMs           Use & eprom type
-----------------------------------------
U18  si003-09.prl - V60 Program (27C4001)
U20  si003-10.prh /

U23  si003-07.s0 - Samples (16M-Mask)
U24  si003-08.s1 /

U13  si003-01.d0 - Graphics (16M-Mask)
U12  si003-02.d1 |
U11  si003-03.d2 |
U10  si003-04.d3 |
U30  si003-05.d4 |
U31  si003-06.d5 /

NOTE: The "s" and "d" designations above are silk-screened on the rom PCB.

Chips of note:  mc14584b - Motorola HEX Schmitt Trigger

This chip is used for the trackball trigger / reading / converting values

***************************************************************************/

ROM_START( eaglshot )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "si003-09.u18",  0x000000, 0x080000, CRC(219c71ce) SHA1(4f8996b4c5b267a90073d67857358147732f8c0d) )
	ROM_LOAD16_BYTE( "si003-10.u20",  0x000001, 0x080000, CRC(c8872e48) SHA1(c8e1e712d5fa380f8fc1447502f21d2ae592811a) )

	ROM_REGION16_LE( 0xe00000, "gfxdata", ROMREGION_ERASEFF ) /* Sprites - Read by the CPU */
	ROM_LOAD( "si003-01.u13", 0x0000000, 0x200000, CRC(d7df0d52) SHA1(d7b79a186f4272334c2297666c52f32c05787c29) )
	ROM_LOAD( "si003-02.u12", 0x0200000, 0x200000, CRC(92b4d50d) SHA1(9dc2f2961b088824d8370ac83dff796345fe4158) )
	ROM_LOAD( "si003-03.u11", 0x0400000, 0x200000, CRC(6ede4012) SHA1(6663990c6ee8e500cb8c51ad2102761ee0b3351d) )
	ROM_LOAD( "si003-04.u10", 0x0600000, 0x200000, CRC(4c65d1a1) SHA1(165f16d08813d2c989ddce4bb23b3a3652003bd5) )
	ROM_LOAD( "si003-05.u30", 0x0800000, 0x200000, CRC(daf52d56) SHA1(108419ef7d3716a3890b0d8bcbfddc1585daaae8) )
	ROM_LOAD( "si003-06.u31", 0x0a00000, 0x200000, CRC(449f9ae5) SHA1(b3e664eb88d14d1e25a0cfc8dcccc8270ca778c9) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "si003-07.u23", 0x000000, 0x200000, CRC(81679fd6) SHA1(ca3b07a87781278b5c7c85951728bbe5dfcbe042) )
	ROM_LOAD16_WORD_SWAP( "si003-08.u24", 0x200000, 0x200000, CRC(d0122ba2) SHA1(96230fb690cf144cd873f7d51c0304736a698316) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )
ROM_END


/***************************************************************************

                    (Mahjong) Hyper Reaction (Japan)

(c)1995 Sammy, SSV system

P1-102A (ROM board)

***************************************************************************/

ROM_START( hypreact )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "s14-1-02.u2", 0x000000, 0x080000, CRC(d90a383c) SHA1(9945f60ce6e1f50c24c2ae3c2c5d0df9ec3b8926) )
	ROM_LOAD16_BYTE( "s14-1-01.u1", 0x000001, 0x080000, CRC(80481401) SHA1(4b1b7050893b6659762297d0f6496c7193ea6c4e) )

	ROM_REGION( 0x1800000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "s14-1-07.u7",  0x0000000, 0x200000, CRC(6c429fd0) SHA1(de1bbcd4a20410328d88a3b246afa8e1a6a6f232) )
	ROM_LOAD( "s14-1-05.u13", 0x0200000, 0x200000, CRC(2ff72f98) SHA1(92bd5042e19e1dae1252305413684f9cff4bd0ac) )
	ROM_LOAD( "s14-1-06.u10", 0x0400000, 0x200000, CRC(f470ec42) SHA1(f31e9c3f3daa212226b9eea14aa1d01367fa348f) )

	ROM_LOAD( "s14-1-10.u6",  0x0600000, 0x200000, CRC(fdd706ba) SHA1(893ead529c1ef62002dcff97092ff9fa51ced938) )
	ROM_LOAD( "s14-1-08.u12", 0x0800000, 0x200000, CRC(5bb9bb0d) SHA1(1874375cbe79663ff1b5181a1c16fa597a6b55f7) )
	ROM_LOAD( "s14-1-09.u9",  0x0a00000, 0x200000, CRC(d1dda65f) SHA1(b4bbd5c9da08b4d4fedb48cfe2dea4f27895c2fd) )

	ROM_LOAD( "s14-1-13.u8",  0x0c00000, 0x200000, CRC(971caf11) SHA1(7cfc8bed4431467da53e19056402aa2409be5d88) )
	ROM_LOAD( "s14-1-11.u14", 0x0e00000, 0x200000, CRC(6d8e7bae) SHA1(93258663ceb6174917560bb66d72a42ba0f96c0e) )
	ROM_LOAD( "s14-1-12.u11", 0x1000000, 0x200000, CRC(233a8e23) SHA1(0c813ec80ac63aa342c8ea57d9e38cada74456d9) )

	ROM_FILL(                 0x1200000, 0x600000, 0x000000          )

//  The chip seems to use REGION1 too, but produces no sound from there.

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "s14-1-04.u4", 0x000000, 0x200000, CRC(a5955336) SHA1(1ac0f5d27224e93acfe449d8ca5c3ab3b7f5dd8c) )
	ROM_LOAD16_WORD_SWAP( "s14-1-03.u5", 0x200000, 0x200000, CRC(283a6ec2) SHA1(766c685384ea8d801c53a2ae36b4980318aff06b) )
ROM_END


/***************************************************************************

                    (Mahjong) Hyper Reaction 2 (Japan)

(c)1997 Sammy,SSV system

P1-112A (ROM board)

***************************************************************************/

ROM_START( hypreac2 )
	ROM_REGION( 0x200000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "u2.bin",  0x000000, 0x080000, CRC(05c93266) SHA1(0833e80f67ccb4ac17e771fa04dc6f433554a34f) )
	ROM_LOAD16_BYTE( "u1.bin",  0x000001, 0x080000, CRC(80cf9e59) SHA1(7025321539891e1a3354ca233255f5395d716933) )
	ROM_LOAD16_BYTE( "u47.bin", 0x100000, 0x080000, CRC(a3e9bfee) SHA1(1e897646bafd07ab48eda2883926506c6bedab87) )
	ROM_LOAD16_BYTE( "u46.bin", 0x100001, 0x080000, CRC(68c41235) SHA1(6ec32aa6ab6074a8db63a76a3d1a0ec2dc8f8aae) )

	ROM_REGION( 0x2800000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "s16-1-16.u6",  0x0000000, 0x400000, CRC(b308ac34) SHA1(409652bc5a537650cab1f3709a2c2be206f72a78) )
	ROM_LOAD( "s16-1-15.u9",  0x0400000, 0x400000, CRC(2c8e381e) SHA1(a8681620809d3d9dc62b443232b6e4c4c4209248) )
	ROM_LOAD( "s16-1-14.u12", 0x0800000, 0x200000, CRC(afe9d187) SHA1(802df8b1bbb94e4451a6b97c852fa555a6cf5837) )

	ROM_LOAD( "s16-1-10.u7",  0x0a00000, 0x400000, CRC(86a10cbd) SHA1(7c15da7c3ffebff058e78439c64f6c0386e4d55b) )
	ROM_LOAD( "s16-1-09.u10", 0x0e00000, 0x400000, CRC(6b8e4d92) SHA1(a58c02d3fe595ab654b267cebcb1c6e8ec0b20c4) )
	ROM_LOAD( "s16-1-08.u13", 0x1200000, 0x200000, CRC(b355f45d) SHA1(7e0fe81825745555b9627716bfdf1132f20e88f0) )

	ROM_LOAD( "s16-1-13.u8",  0x1400000, 0x400000, CRC(89869af2) SHA1(46a8eec18327b515a33c6e01d35fb9b947fcab1f) )
	ROM_LOAD( "s16-1-12.u11", 0x1800000, 0x400000, CRC(87d9c748) SHA1(1332db901e50e2fd25d3323920f99e0ef0b0533d) )
	ROM_LOAD( "s16-1-11.u14", 0x1c00000, 0x200000, CRC(70b3c0a0) SHA1(009e2f2f292ed6f10a9d54557861294156664e72) )

	ROM_FILL(                 0x1e00000,0x0a00000, 0x000000)

	ROM_REGION16_BE( 0x400000, "ensoniq.0", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "s16-1-06.u41", 0x000000, 0x400000, CRC(626e8a81) SHA1(45ef5b630aed575acd160ede1413e0370f4f9761) )

	ROM_REGION16_BE( 0x600000, "ensoniq.1", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "s16-1-07.u42", 0x200000, 0x400000, CRC(42bcb41b) SHA1(060312b19bd52770410cec1f77e5d8d6478d80eb) )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "s16-1-07.u42", 0x000000, 0x400000, CRC(42bcb41b) SHA1(060312b19bd52770410cec1f77e5d8d6478d80eb) )
ROM_END


/***************************************************************************

             Lovely Pop Mahjong Jangjang Shimasho (Japan)

(c)1996 Visco, SSV System

***************************************************************************/

ROM_START( janjans1 )
	ROM_REGION( 0x400000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "jj1-data.bin", 0x000000, 0x200000, CRC(6734537e) SHA1(a40f84479141a6f33ce465e66ba9313b54915002) )
	ROM_LOAD16_BYTE( "jj1-prol.bin", 0x200000, 0x080000, CRC(4231d928) SHA1(820d1233cd1a8d0c4ece15b94bd9be976b383fe2) )
	ROM_RELOAD(                      0x300000, 0x080000)
	ROM_LOAD16_BYTE( "jj1-proh.bin", 0x200001, 0x080000, CRC(651383c6) SHA1(8291f86b230eee3a2ebcc926a8370777ee21ec47) )
	ROM_RELOAD(                      0x300001, 0x080000)

	ROM_REGION( 0x2800000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "jj1-a0.bin", 0x0000000, 0x400000, CRC(39bbbc46) SHA1(77c6b5e9d4315671ea79ec838baa7ae043bcd8c4) )
	ROM_LOAD( "jj1-a1.bin", 0x0400000, 0x400000, CRC(26020133) SHA1(32c834655d885431d466f25a729aee2d589ade1b) )
	ROM_LOAD( "jj1-a2.bin", 0x0800000, 0x200000, CRC(e993251e) SHA1(6cea12bbfc170ad4ecdc09c1728f88ec7534270a) )

	ROM_LOAD( "jj1-b0.bin", 0x0a00000, 0x400000, CRC(8ee66b0a) SHA1(f5a641d54a3040b67d9b6e9533c4e1ed3dbc9e12) )
	ROM_LOAD( "jj1-b1.bin", 0x0e00000, 0x400000, CRC(048719b3) SHA1(b81198d58afbc7ef2f7dc71cfef11d269bc1608f) )
	ROM_LOAD( "jj1-b2.bin", 0x1200000, 0x200000, CRC(6e95af3f) SHA1(c4336a3f169143d5ab828ea527c08dcac27654c3) )

	ROM_LOAD( "jj1-c0.bin", 0x1400000, 0x400000, CRC(9df28afc) SHA1(98ee75b028257614c3354d5ec7b3d4b27be75595) )
	ROM_LOAD( "jj1-c1.bin", 0x1800000, 0x400000, CRC(eb470ed3) SHA1(ac0601eb57283c3ebb2daf20d07a2c350804b8e6) )
	ROM_LOAD( "jj1-c2.bin", 0x1c00000, 0x200000, CRC(aaf72c2d) SHA1(774e713bbc4c2ed7ff7c9fb49a06246d97c33bad) )

	ROM_LOAD( "jj1-d0.bin", 0x1e00000, 0x400000, CRC(2b3bd591) SHA1(0619b2779bd4bc19a5259040ccce0fdbefecf1d0) )
	ROM_LOAD( "jj1-d1.bin", 0x2200000, 0x400000, CRC(f24c0d36) SHA1(212969b456bfd7cc00081f65c03c1e167186891a) )
	ROM_LOAD( "jj1-d2.bin", 0x2600000, 0x200000, CRC(481b3be8) SHA1(cd1bcaca8c236cebba72d315e759b2e9d243aca8) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "jj1-snd0.bin", 0x000000, 0x200000, CRC(4f7d620a) SHA1(edded130ce7bb0f37e1f59b2771ae6a10a061f9e) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "jj1-snd1.bin", 0x000000, 0x200000, CRC(9b3a7ae5) SHA1(193743fcce779c4a8a73a44c54b5391d08116331) )
ROM_END

/***************************************************************************

            Lovely Pop Mahjong JangJang Shimasho 2 (Japan)

(c)1996-2000 Visco

This game runs on SSV hardware. The game PCB is VISCO-JJ1
The game PCB contains mostly just ROMs and logic, plus....
NEC D71051 (DIP28)
8MHz OSC TI 75175 (DIP8)
2x PALs (near program ROMs)
2x 8-pin connectors for extra controls

***************************************************************************/

ROM_START( janjans2 )
	ROM_REGION( 0x400000, "maincpu", 0 )        /* V60 Code */
	ROM_LOAD16_WORD( "jan2-dat.u28",  0x000000, 0x200000, CRC(0c9c62bf) SHA1(17c6eea7cec05860c238cc22706fec1a8e3d9263) )
	ROM_LOAD16_BYTE( "jan2-prol.u26", 0x200000, 0x080000, CRC(758a7249) SHA1(1126e8527bad000bdfbd59da46d72ed256cb0fa9) )
	ROM_RELOAD(                       0x300000, 0x080000)
	ROM_LOAD16_BYTE( "jan2-proh.u27", 0x200001, 0x080000, CRC(fcd5da62) SHA1(e0243e41e4ec25e82b0316f1189ed069c369e7b1) )
	ROM_RELOAD(                       0x300001, 0x080000)

	ROM_REGION( 0x2000000, "gfx1", 0 )    /* Sprites */
	ROM_LOAD( "jan2-a0.u13", 0x0000000, 0x400000, CRC(37869bea) SHA1(6259e8584775ca702ef4e9e460c6d874980ffecb) )
	ROM_LOAD( "jan2-a1.u14", 0x0400000, 0x400000, CRC(8189e74f) SHA1(ea083a7ef0858dac59e14a77f10a9900b20447f3) )

	ROM_LOAD( "jan2-b0.u16", 0x0800000, 0x400000, CRC(19877c5c) SHA1(5faaf3b862ca544589b46cf52d5fdb73287ceb6f) )
	ROM_LOAD( "jan2-b1.u17", 0x0c00000, 0x400000, CRC(8d0f7190) SHA1(d06b53d627f7629bda4de7e130eae2be14c18a5a) )

	ROM_LOAD( "jan2-c0.u21", 0x1000000, 0x400000, CRC(8bdff3d5) SHA1(be3d2f72abae620f8855d5443dae62880f58f7c6) )
	ROM_LOAD( "jan2-c1.u22", 0x1400000, 0x400000, CRC(f7ea5934) SHA1(1767a0206fc35dc2800c9b5b061f704828571452) )

	ROM_LOAD( "jan2-d0.u34", 0x1800000, 0x400000, CRC(479fdb54) SHA1(667d89518877a3b501a87c9c765b85b9a0b23517) )
	ROM_LOAD( "jan2-d1.u35", 0x1c00000, 0x400000, CRC(c0148895) SHA1(f89482a6ef475ca44d570332d05201b34887afbb) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 )    /* Samples */
	ROM_LOAD16_BYTE( "jan2-snd0.u29", 0x000000, 0x200000, CRC(22cc054e) SHA1(4926dd9f8f85880d6c1d14f93d68f330898b473a) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE | 0 )    /* Samples */
	ROM_LOAD16_BYTE( "jan2-snd1.u33", 0x000000, 0x200000, CRC(cbcac4a6) SHA1(f0c57fa6784e910bdb94f046d09e58e26921773b) )
ROM_END


/***************************************************************************

Joryuu Syougi Kyoushitsu
(c)1997 Visco, System SSV ROM board

CPU : NEC JAPAN D70732GD-25 (C)NEC1991 V810 9651MK007
OSC : 48.0000MHz

ROMs:
JSK-U4 .BIN [ec22fb41] - (27c1001)
JSK-U24.BIN [1fa6e156]  |
JSK-U38.BIN [8e5c0de3]  |
JSK-U52.BIN [b11aef0c] /

JSK-U71.BIN [f6774fba] - (27c040)
JSK-U72.BIN [db6b2554] /

JSK-S0.U65  [8d1a9aeb] - (16M mask)

JSK-A0.BIN  [4bac3196] - (16M mask)
JSK-B0.BIN  [40664e5a]  |
JSK-C0.BIN  [2a230e64]  |
JSK-D0.BIN  [911e53a6] /

GAL (not dumped):
U53.BIN     [--------] - GAL16V8B
U70.BIN     [--------] /

dumped by sayu

Note, in the old bad dump u52 had a few different bytes:

ROM_LOAD32_BYTE( "jsk-u52.bin", 0x00000, 0x20000, CRC(19cc585f) SHA1(b53138e93d40c0cf03aee838d7653f5665d9cf35) )

addr   old  this
1FF8F: FF   00
1FF97: FF   00
1FFB7: FF   00
1FFCF: FF   00
1FFD7: FF   00
1FFF7: FF   00

***************************************************************************/

ROM_START( jsk )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jsk-u72.bin", 0x00000, 0x80000, CRC(db6b2554) SHA1(c4c6617461e1d3f8660a2b97fd2c38ef245f0d4a) )
	ROM_LOAD16_BYTE( "jsk-u71.bin", 0x00001, 0x80000, CRC(f6774fba) SHA1(3a74e5091d9d72e4f92c7c637cfe5c0dcc60bbe1) )

	ROM_REGION( 0x20000*4, "sub", 0 )
	ROM_LOAD32_BYTE( "jsk-u52.bin", 0x00000, 0x20000, CRC(b11aef0c) SHA1(37c0fedd6454a05b647513600f1b0998c572c7a5) )
	ROM_LOAD32_BYTE( "jsk-u38.bin", 0x00001, 0x20000, CRC(8e5c0de3) SHA1(54c5dfd858086b0eb7ffa82c19fb1dfd7752d50e) )
	ROM_LOAD32_BYTE( "jsk-u24.bin", 0x00002, 0x20000, CRC(1fa6e156) SHA1(4daedf660d89c185c945d4a526312f6528fe7b17) )
	ROM_LOAD32_BYTE( "jsk-u4.bin",  0x00003, 0x20000, CRC(ec22fb41) SHA1(c0d6b0a92075214a91da78be52d273771cb9f646) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "jsk-a0.bin", 0x0000000, 0x400000, CRC(18981a19) SHA1(b4bf93f38099963350b9e5e64890ce7adc1bc983) )
	ROM_LOAD( "jsk-b0.bin", 0x0400000, 0x400000, CRC(f6df0ff9) SHA1(d7736e4ae6e099aef320a59668d7f17590c346b9) )
	ROM_LOAD( "jsk-c0.bin", 0x0800000, 0x400000, CRC(b8282939) SHA1(d041fb013e5011bf6b9d9bc2c816b2f3969723b7) )
	ROM_LOAD( "jsk-d0.bin", 0x0c00000, 0x400000, CRC(fc733e0c) SHA1(951060f6600b8b677ad2f41f59071c375ea9d4cf) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE )   /* Samples */
	ROM_LOAD16_BYTE( "jsk-s0.u65", 0x000000, 0x200000, CRC(8d1a9aeb) SHA1(37316bd3e8cbe2a84239e1a11a56d4fe4723ae1a) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )
ROM_END


/***************************************************************************

                Dramatic Adventure Quiz Keith & Lucy (Japan)

(c)1993 Visco, SSV system

STS-0001 (ROM board)

***************************************************************************/

ROM_START( keithlcy )
	ROM_REGION( 0x200000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "vg002-07.u28", 0x000000, 0x100000, CRC(57f80ff5) SHA1(9dcc35a79d3799407190d113e0f1b57864d6c56a) ) // "SETA SoundDriver"
	ROM_LOAD16_BYTE( "kl-p0l.u26",   0x100000, 0x080000, CRC(d7b177fb) SHA1(2a3533b952a7b2404720916662743c144e870c0b) )
	ROM_LOAD16_BYTE( "kl-p0h.u27",   0x100001, 0x080000, CRC(9de7add4) SHA1(16f4405b12734cb6a83cff8be21d03bb3c2e2266) )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "vg002-01.u13", 0x000000, 0x200000, CRC(b44d85b2) SHA1(cf78d46f9f2594a23af08a898afbf5dd609abcec) )
	ROM_LOAD( "vg002-02.u16", 0x200000, 0x200000, CRC(aa05fd14) SHA1(9144e9668788fcd45bd6c8464f9b4f865397f783) )
	ROM_LOAD( "vg002-03.u21", 0x400000, 0x200000, CRC(299a8a7d) SHA1(b24d8ffba01d345f48f47f92e58e9b2a9ec62526) )
	ROM_LOAD( "vg002-04.u34", 0x600000, 0x200000, CRC(d3633f9b) SHA1(250a25b75a4810a676a02c390bb597b6f1cd7494) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "vg002-05.u29", 0x000000, 0x200000, CRC(66aecd79) SHA1(7735034b8fb35ad5e7916acd0c2e224a7c62e195) )
	ROM_LOAD16_WORD_SWAP( "vg002-06.u33", 0x200000, 0x200000, CRC(75d8c8ea) SHA1(545768ac6d8953cd3044680953476276337a94b9) )
ROM_END


/***************************************************************************

                            Koi Koi Shimasho 2 (Japan)

(c)1997 Visco, SSV System

VISCO-JJ1 (same board as janjans1)

OSC  :8.00000MHz

KK2_A0.BIN   [0c89a9ae] GFX (32M mask)
KK2_A1.BIN   [3f85ff19]  |
KK2_B0.BIN   [4d028972]  |
KK2_B1.BIN   [16e085f7]  |
KK2_C0.BIN   [34b699d9]  |
KK2_C1.BIN   [896fbb6f]  |
KK2_D0.BIN   [0e3005a4]  |
KK2_D1.BIN   [0116a5fb] /

U26.BIN      [4be937a1] Programs (27c4001) (PROL)
U27.BIN      [25f39d93] /                  (PROH)

KK2_SND0.BIN [b27eaa94] Sound (16M mask)
KK2_SND1.BIN [e5a963e1] /

(socket for DATA ROM is empty)

***************************************************************************/

ROM_START( koikois2 )
	ROM_REGION( 0x400000, "maincpu", 0 )     /* V60 Code */
//  socket for DATA ROM is empty
	ROM_LOAD16_BYTE( "u26.bin", 0x200000, 0x080000, CRC(4be937a1) SHA1(b2c22ec12fc110984bd1914f8e3e16a8cb866816) )
	ROM_RELOAD(                 0x300000, 0x080000)
	ROM_LOAD16_BYTE( "u27.bin", 0x200001, 0x080000, CRC(25f39d93) SHA1(a36bc2fe5657f6ceada724fd42843e19408b39b8) )
	ROM_RELOAD(                 0x300001, 0x080000)

	ROM_REGION( 0x2000000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "kk2-a0.bin", 0x0000000, 0x400000, CRC(b94b76c2) SHA1(07ce3e3946669c1bd2f022da9861164625be9c1b) )
	ROM_LOAD( "kk2-a1.bin", 0x0400000, 0x200000, CRC(a7c99f56) SHA1(de341e99f76446fab4d7f09c2d8a6f18554b5d2f) )

	ROM_LOAD( "kk2-b0.bin", 0x0800000, 0x400000, CRC(4d028972) SHA1(732c874d3511c7bce006436d557ec24e54df0166) )
	ROM_LOAD( "kk2-b1.bin", 0x0c00000, 0x200000, CRC(778ec9fb) SHA1(5983f0292e274e3da098b461355e2c001f4881b3) )

	ROM_LOAD( "kk2-c0.bin", 0x1000000, 0x400000, CRC(34b699d9) SHA1(b5208d5f70f21725e54c9dc59de73f1a5646a72c) )
	ROM_LOAD( "kk2-c1.bin", 0x1400000, 0x200000, CRC(ab451e88) SHA1(0c4d6c0c758f2ab4210c201605dd573661b6c553) )

	ROM_LOAD( "kk2-d0.bin", 0x1800000, 0x400000, CRC(0e3005a4) SHA1(fa8da58308d58bb6b2e8beb8ee8f7ea08b18f4d9) )
	ROM_LOAD( "kk2-d1.bin", 0x1c00000, 0x200000, CRC(17a02252) SHA1(c7aa61e27f197b3c497a65a9369e3a6a20c9f82a) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "kk2_snd0.bin", 0x000000, 0x200000, CRC(b27eaa94) SHA1(05baaef683a1fcd9eb8a7cfd5b280c05108e832f) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "kk2_snd1.bin", 0x000000, 0x200000, CRC(e5a963e1) SHA1(464ffd53ac2e6db62225b18d12bfea93160771ec) )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", 0 ) /* Samples */
	ROM_COPY( "ensoniq.1", 0x000000, 0x000000, 0x400000 )
ROM_END


/***************************************************************************

                        Meosis Magic (Japan, BET?)

(c)1996 Sammy, SSV System

P1-105A

Custom:     DX-102 (I/O)
Others:     M62X42B (RTC?)
            64k SRAM (Back up)
            Ni-Cd Battery

***************************************************************************/

ROM_START( meosism )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "s15-2-2.u47", 0x000000, 0x080000, CRC(2ab0373f) SHA1(826aec3b9698ec5db5d7a72c3a24b1ef779fb227) )
	ROM_LOAD16_BYTE( "s15-2-1.u46", 0x000001, 0x080000, CRC(a4bce148) SHA1(17ec4d91e215bd38258329b1a71e7f135c5733ad) )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "s15-1-7.u7", 0x000000, 0x200000, CRC(ec5023cb) SHA1(3406f5143a40c8dcd2d45b44ea91c737810ab05b) )
	ROM_LOAD( "s15-1-8.u6", 0x200000, 0x200000, CRC(f04b0836) SHA1(83678427cd0ed0d68ff770baa2693226b391f6c8) )
	ROM_LOAD( "s15-1-5.u9", 0x400000, 0x200000, CRC(c0414b97) SHA1(3ca8423e04f606981d158065e38431f2509e1daa) )
	ROM_LOAD( "s15-1-6.u8", 0x600000, 0x200000, CRC(d721aeb6) SHA1(3bef7e027a0e14fbf589aee32a6d9cab779da7d4) )

//  The chip seems to use REGION1 too, but produces no sound from there.

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "s15-1-4.u45", 0x000000, 0x200000, CRC(0c6738a7) SHA1(acf9056bb052db7a11cf903d77ab16425d813835) )
	ROM_LOAD16_WORD_SWAP( "s15-1-3.u43", 0x200000, 0x200000, CRC(d7e83178) SHA1(74e5c09f6d3b2c8e1c1cc2b0eab0490b5bbc9099) )
ROM_END


/***************************************************************************

                            Monster Slider (Japan)

(c)1997 Visco/PATT, System SSV

ms-pl.bin - V60 main program (27c4000, low)
ms-ph.bin - V60 main program (27c4000, high)

ms-snd0.bin \
            |- sound data (read as 27c160)
ms-snd1.bin /

ms-a0.bin \
ms-b0.bin |- Graphics (read as 27c160)
ms-c0.bin /

ms-a1.bin \
ms-b1.bin |- Graphics (27c4100)
ms-c1.bin /

vg001-14 \
         |- (GAL16V8. not dumped)
vg001-15 /

Other parts:    uPD71051
                OSC 8.0000MHz

***************************************************************************/

ROM_START( mslider )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "ms-pl.bin", 0x000000, 0x080000, CRC(70b2a05d) SHA1(387cf67e3e505c4cc1b5cd0b6c9fb3bc27d07e24) )
	ROM_LOAD16_BYTE( "ms-ph.bin", 0x000001, 0x080000, CRC(34a64e9f) SHA1(acf3d8490f3ec99b6171e71328a991fcc9c5a8b1) )

	ROM_REGION( 0xa00000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "ms-a0.bin", 0x000000, 0x200000, CRC(7ed38ccc) SHA1(9c584a5f6b3aad8646d155a56e4070cfed4af540) )
	ROM_LOAD( "ms-a1.bin", 0x200000, 0x080000, CRC(83f5995f) SHA1(33ae99a96702d4aba422eaf454b86c96aaf88426) )

	ROM_LOAD( "ms-b0.bin", 0x280000, 0x200000, CRC(faa076e1) SHA1(cca583c617e5d4ab995605dd16280931893991c7) )
	ROM_LOAD( "ms-b1.bin", 0x480000, 0x080000, CRC(ef9748db) SHA1(34ab4524ec81b81ae2540f7d69e0f8254fd1b8f4) )

	ROM_LOAD( "ms-c0.bin", 0x500000, 0x200000, CRC(f9d3e052) SHA1(4cdde756b24ee980f3c79a35a1fe071861fdeef9) )
	ROM_LOAD( "ms-c1.bin", 0x700000, 0x080000, CRC(7f910c5a) SHA1(23ea13b6c07d3d31a25c21704d6a3e506578b199) )

	ROM_FILL(              0x780000, 0x280000, 0x000000)

	ROM_REGION16_BE( 0x400000, "ensoniq.0", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "ms-snd0.bin", 0x000000, 0x200000, CRC(cda6e3a5) SHA1(28ad8f34bc4f907654582f3522b377b97234eba8) )
	ROM_LOAD16_WORD_SWAP( "ms-snd1.bin", 0x200000, 0x200000, CRC(8f484b35) SHA1(cbf3ee7ec6337915f9d90a5b43d2de1eaa5537d0) )
ROM_END


/***************************************************************************

                    Gourmet Battle Quiz Ryohrioh CooKing (Japan)

(c)1998 Visco, SSV System

***************************************************************************/

ROM_START( ryorioh )
	ROM_REGION( 0x400000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD( "ryorioh.dat",      0x000000, 0x200000, CRC(d1335a6a) SHA1(a5670ab3c399736232baaabc59573bdb3bf762da) )
	ROM_LOAD16_BYTE( "ryorioh.l", 0x200000, 0x080000, CRC(9ad60e7d) SHA1(572b84bab08eb8293d93e03182d9871d8973b7dd) )
	ROM_RELOAD(                   0x300000, 0x080000)
	ROM_LOAD16_BYTE( "ryorioh.h", 0x200001, 0x080000, CRC(0655fcff) SHA1(2c088e42323f87e01b65f9f523e258f881d4e773) )
	ROM_RELOAD(                   0x300001, 0x080000)

	ROM_REGION( 0x2000000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "ryorioh.a0", 0x0000000, 0x400000, CRC(f76ee003) SHA1(04022238dcfd5cf0e4f97c3c3b24df574ec6b609) )
	ROM_LOAD( "ryorioh.a1", 0x0400000, 0x400000, CRC(ca44d66d) SHA1(d5ed2bbc9831182b212533bd67bb3831f655110a) )

	ROM_LOAD( "ryorioh.b0", 0x0800000, 0x400000, CRC(daa134f4) SHA1(c3dce66b2c67061ae980374f7559b2bb8ef2502d) )
	ROM_LOAD( "ryorioh.b1", 0x0c00000, 0x400000, CRC(7611697c) SHA1(febb0bc5f3bc8766be4377092c8443a489379bca) )

	ROM_LOAD( "ryorioh.c0", 0x1000000, 0x400000, CRC(20eb49cf) SHA1(13c201e28be17cdfc7e6266a6d1fb41cfbe04b53) )
	ROM_LOAD( "ryorioh.c1", 0x1400000, 0x400000, CRC(1370c75e) SHA1(30dfe37f3fab0e3e94df4a6d45f1291ad41e0147) )

	ROM_LOAD( "ryorioh.d0", 0x1800000, 0x400000, CRC(ffa14ef1) SHA1(22a6992f6217d8ef2140e72063290fa34cb45683) )
	ROM_LOAD( "ryorioh.d1", 0x1c00000, 0x400000, CRC(ae6055e8) SHA1(ee20a7b3c4f899404ca259991509728d3a0f96b9) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "ryorioh.snd", 0x000000, 0x200000, CRC(7bd38b76) SHA1(d8490b4af839ef0802b8b2a47277fcd4091e4d37) )
ROM_END


/***************************************************************************

                            Super Real Mahjong PIV

(c)SETA 1993, System SSV

CPU        : V60 (12MHz)
Sound      : Ensoniq OTTO
Work RAM   : 256Kbit (expandable to 1Mbitx2. SRMP7 requires this)
Object RAM : 1Mbitx2
Palette RAM: 256Kbitx3 (expandable to 1Mbitx3)

sx001-01.a0 \
sx001-02.b0 |
sx001-03.c0 |
sx001-04.a1 |
sx001-05.b1 |- Graphics (16M Mask)
sx001-06.c1 |
sx001-07.a2 |
sx001-08.b2 |
sx001-09.c2 /

sx001-10.sd0 - Sound - 16M Mask

sx001-11.prl - Main program (low)  - 27c040
sx001-12.prh - Main program (high) - 27c040

Custom chips
ST-0004 (Video DAC)
ST-0005 (Parallel I/O)
ST-0006 (Video controller - 32768 palettes from 24bit color)
ST-0007 (System controller)

***************************************************************************/

ROM_START( srmp4 )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "sx001-14.prl", 0x000000, 0x080000, CRC(19aaf46e) SHA1(0c0f5acc1880971c56e7e2c2e3ad7c2932b82d4b) )
	ROM_LOAD16_BYTE( "sx001-15.prh", 0x000001, 0x080000, CRC(dbd31399) SHA1(a77dc85f481454b10223d7f4e0395e07d2f8d4f3) )

	ROM_REGION( 0x1800000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "sx001-01.a0", 0x0000000, 0x200000, CRC(94ee9203) SHA1(a0e944a375f94e9dd668b06f15580384902d0fe1) )
	ROM_LOAD( "sx001-04.a1", 0x0200000, 0x200000, CRC(38c9c49a) SHA1(c392d1cf5d16a348bdaa7222f2420a61a831a50a) )
	ROM_LOAD( "sx001-07.a2", 0x0400000, 0x200000, CRC(ee66021e) SHA1(f4df2bdf8100a3bd39bb61f9bb4807ca9e13537a) )

	ROM_LOAD( "sx001-02.b0", 0x0600000, 0x200000, CRC(adffb598) SHA1(fab372aebfbb12feaf7a7716a780cf2e5cc60731) )
	ROM_LOAD( "sx001-05.b1", 0x0800000, 0x200000, CRC(4c400a38) SHA1(42623d6134fb6d8ce9059f7774c6bf4d2ea5d2d9) )
	ROM_LOAD( "sx001-08.b2", 0x0a00000, 0x200000, CRC(36efd52c) SHA1(f51c6d9ceff02b9ee3f8e4ffa17f6c00ee9de905) )

	ROM_LOAD( "sx001-03.c0", 0x0c00000, 0x200000, CRC(4336b037) SHA1(f42c5622e141e384efb52955f7f6a58a8ba8fc2c) )
	ROM_LOAD( "sx001-06.c1", 0x0e00000, 0x200000, CRC(6fe7229e) SHA1(e1432aa500460f79b5b78ee4b249d8fc9f566ce1) )
	ROM_LOAD( "sx001-09.c2", 0x1000000, 0x200000, CRC(91dd8218) SHA1(a500dca9eefbf93187b1dfde7ddff1d22b886d44) )

	ROM_FILL(                0x1200000, 0x600000, 0x000000)

	ROM_REGION16_BE( 0x400000, "ensoniq.0", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "sx001-10.sd0", 0x000000, 0x200000, CRC(45409ef1) SHA1(327d0a63deac6f0f8b9a408a321c03dd4e965569) )
	ROM_RELOAD(                           0x200000, 0x200000)
ROM_END

ROM_START( srmp4o )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "sx001-11.prl", 0x000000, 0x080000, CRC(dede3e64) SHA1(6fe998babfd2ad8f268c59bd365115a2d7cfc8f9) )
	ROM_LOAD16_BYTE( "sx001-12.prh", 0x000001, 0x080000, CRC(739c53c3) SHA1(68f12cf42177df208ff6499ccc7ccc1423e3ad5f) )

	ROM_REGION( 0x1800000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "sx001-01.a0", 0x0000000, 0x200000, CRC(94ee9203) SHA1(a0e944a375f94e9dd668b06f15580384902d0fe1) )
	ROM_LOAD( "sx001-04.a1", 0x0200000, 0x200000, CRC(38c9c49a) SHA1(c392d1cf5d16a348bdaa7222f2420a61a831a50a) )
	ROM_LOAD( "sx001-07.a2", 0x0400000, 0x200000, CRC(ee66021e) SHA1(f4df2bdf8100a3bd39bb61f9bb4807ca9e13537a) )

	ROM_LOAD( "sx001-02.b0", 0x0600000, 0x200000, CRC(adffb598) SHA1(fab372aebfbb12feaf7a7716a780cf2e5cc60731) )
	ROM_LOAD( "sx001-05.b1", 0x0800000, 0x200000, CRC(4c400a38) SHA1(42623d6134fb6d8ce9059f7774c6bf4d2ea5d2d9) )
	ROM_LOAD( "sx001-08.b2", 0x0a00000, 0x200000, CRC(36efd52c) SHA1(f51c6d9ceff02b9ee3f8e4ffa17f6c00ee9de905) )

	ROM_LOAD( "sx001-03.c0", 0x0c00000, 0x200000, CRC(4336b037) SHA1(f42c5622e141e384efb52955f7f6a58a8ba8fc2c) )
	ROM_LOAD( "sx001-06.c1", 0x0e00000, 0x200000, CRC(6fe7229e) SHA1(e1432aa500460f79b5b78ee4b249d8fc9f566ce1) )
	ROM_LOAD( "sx001-09.c2", 0x1000000, 0x200000, CRC(91dd8218) SHA1(a500dca9eefbf93187b1dfde7ddff1d22b886d44) )

	ROM_FILL(                0x1200000, 0x600000, 0x000000)

	ROM_REGION16_BE( 0x400000, "ensoniq.0", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "sx001-10.sd0", 0x000000, 0x200000, CRC(45409ef1) SHA1(327d0a63deac6f0f8b9a408a321c03dd4e965569) )
	ROM_RELOAD(                           0x200000, 0x200000)
ROM_END


/***************************************************************************

                            Super Real Mahjong P7 (Japan)

(c)1997 Seta, SSV system

***************************************************************************/

ROM_START( srmp7 )
	ROM_REGION( 0x400000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "sx015-10.dat", 0x000000, 0x200000, CRC(fad3ac6a) SHA1(9a4695c06bc74ca4de0c1a83bdf38f6651c0e2a1) )
	ROM_LOAD16_BYTE( "sx015-07.pr0", 0x200000, 0x080000, CRC(08d7f841) SHA1(67567acff0ce278576290a896005de0397605eef) )
	ROM_RELOAD(                      0x300000, 0x080000)
	ROM_LOAD16_BYTE( "sx015-08.pr1", 0x200001, 0x080000, CRC(90307825) SHA1(13b3f82c8854808684bd41deb0bbd442efe7b685) )
	ROM_RELOAD(                      0x300001, 0x080000)

	ROM_REGION( 0x4000000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "sx015-26.a0", 0x0000000, 0x400000, CRC(a997be9d) SHA1(37470af24531557113f953f727f6b8cab602a7d3) )
	ROM_LOAD( "sx015-25.a1", 0x0400000, 0x400000, CRC(29ac4211) SHA1(32edf3982b0e27077cc17cd38b67a27d36dc3ad8) )
	ROM_LOAD( "sx015-24.a2", 0x0800000, 0x400000, CRC(b8fea3da) SHA1(9c3a53348f72f39d84d078068c62b10920854cd0) )
	ROM_LOAD( "sx015-23.a3", 0x0c00000, 0x400000, CRC(9ec0b81e) SHA1(fe9550592852db8a0fc38f8af444c4c137b803eb) )

	ROM_LOAD( "sx015-22.b0", 0x1000000, 0x400000, CRC(62c3df07) SHA1(2c2b7ccc53d0ccc78f599d0789d38296935c0316) )
	ROM_LOAD( "sx015-21.b1", 0x1400000, 0x400000, CRC(55b8a431) SHA1(ccdc70b27c7fc9efe2c20df23f01b96f3b542d72) )
	ROM_LOAD( "sx015-20.b2", 0x1800000, 0x400000, CRC(e84a64d7) SHA1(af7a04cebb1ccbbd76812f7b6f7bb79023aff291) )
	ROM_LOAD( "sx015-19.b3", 0x1c00000, 0x400000, CRC(994b5063) SHA1(0c44e94773160e75ef03f7ceb95ab1b123ae3ecf) )

	ROM_LOAD( "sx015-18.c0", 0x2000000, 0x400000, CRC(72d43fd4) SHA1(96582adae0bf1cc8359dd1ecc0d00a42d306c565) )
	ROM_LOAD( "sx015-17.c1", 0x2400000, 0x400000, CRC(fdfd82f1) SHA1(005b60fd7bf9f61ecd16daa6e6bb213ed6a9875b) )
	ROM_LOAD( "sx015-16.c2", 0x2800000, 0x400000, CRC(86aa314b) SHA1(5cd238785f683d3a33f36c5a326d350805ef21ff) )
	ROM_LOAD( "sx015-15.c3", 0x2c00000, 0x400000, CRC(11f50e16) SHA1(4e26aa84bea8b7e73056b0cc70661332fa7d9473) )

	ROM_LOAD( "sx015-14.d0", 0x3000000, 0x400000, CRC(186f83fa) SHA1(6f03056b766c223fef639627706c2476f9af378d) )
	ROM_LOAD( "sx015-13.d1", 0x3400000, 0x400000, CRC(ea6e5329) SHA1(614dd8d36d94a4c8b2b0c30ec96c6d183065561e) )
	ROM_LOAD( "sx015-12.d2", 0x3800000, 0x400000, CRC(80336523) SHA1(ec66e21fe1401fdb438e03657542a7b6b0cbc5ce) )
	ROM_LOAD( "sx015-11.d3", 0x3c00000, 0x400000, CRC(134c8e28) SHA1(669118b58f27d5e2e08052debe904f95d9ab32a3) )

	ROM_REGION16_BE( 0x800000, "ensoniq.0", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "sx015-06.s0", 0x000000, 0x200000, CRC(0d5a206c) SHA1(2fdaf2a56b6608f20a788eb79a8426102ff33e14) )
	ROM_RELOAD(                     0x400000, 0x200000)

	ROM_REGION16_BE( 0x800000, "ensoniq.1", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "sx015-05.s1", 0x000000, 0x200000, CRC(bb8cebe2) SHA1(3691e5fb4e963f69c1fe01cb5d968433029c4833) )
	ROM_RELOAD(                     0x400000, 0x200000)

	ROM_REGION16_BE( 0x800000, "ensoniq.2", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "sx015-04.s2", 0x000000, 0x200000, CRC(f6e933df) SHA1(7cb69515a0ffc62fbac2be3a5fb322538560bd38) )
	ROM_LOAD16_BYTE( "sx015-02.s4", 0x400000, 0x200000, CRC(6567bc3e) SHA1(e902f22f1499edc6a0e2c8b6cc26460d66a3bdbe) )

	ROM_REGION16_BE( 0x800000, "ensoniq.3", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "sx015-03.s3", 0x000000, 0x200000, CRC(5b51ab21) SHA1(cf3e86e41f7984208984d6486b04cec117dadc18) )
	ROM_LOAD16_BYTE( "sx015-01.s5", 0x400000, 0x200000, CRC(481b00ed) SHA1(2c3d158dd5be9af0ee57fd5dd94d2ec75e28b182) )
ROM_END


/***************************************************************************

         Survival Arts

Sammy USA, 1994

This game runs on SSV hardware.

Game PCB Layout
---------------

SAM-5127
|----------------------------------------|
| SI001-13.U6               4P     3P    |
|                                        |
| SI001-12.U7                            |
|                                        |
| SI001-11.U8                            |
|                                        |
| SI001-10.U9                            |
|                                        |
|                                        |
|                                        |
|                                        |
|                                        |
|                                        |
|                           SI001-08.U21 |
|                                        |
|                           SI001-05.U22 |
|                                        |
|                           SI001-02.U23 |
|                                        |
|                                        |
|USA-PR-H.U3                             |
|                                        |
|USA-PR-L.U4  SI001-09.U15  SI001-07.U25 |
|                                        |
|             SI001-06.U16  SI001-04.U26 |
|PAL                                     |
|(SI001-14)   SI001-03.U17  SI001-01.U27 |
|                                        |
|----------------------------------------|

 Manufacturer: Sammy USA
 System Type: System SSV

 ----------------------
 System SSV (STA-0001)
 ----------------------
 CPU  : NEC D70615GD-16 (V60)
 Sound: Ensoniq ES5506 (OTTOR2)
 OSC  : 42.9545MHz(X2) 48.0000MHz(X3)

 Custom chips:
 ST-0004 (Video DAC)
 ST-0005 (Parallel I/O)
 ST-0006 (Video controller)
 ST-0007 (System controller)

 Program Work RAM  : 256Kbitx2 (expandable to 1Mx2)
 Object Work RAM   : 1Mbitx2
 Color Palette RAM : 256Kbitx3 (expandable to 1Mx3)

 -------------------------
 SSV Subboard (SAM-5127)
 -------------------------
 ROMs:
 USA-PR-H.u3 - V60 Program (27C4001)
 USA-PR-L.u4 /

 si001-10.s0 - Samples (16M-Mask)
 si001-12.s2 /

 si001-11.s1 - Samples (8M-Mask)
 si001-13.s3 /

 si001-01.a0 - Graphics (16M-Mask)
 si001-04.a1 |
 si001-05.a2 |
 si001-02.b0 |
 si001-05.b1 |
 si001-07.b2 |
 si001-03.c0 |
 si001-06.c1 |
 si001-09.c2 /

 Empty Sockets:
 DATA --- 16M-Mask
 A3     |
 B3     |
 C3     |
 D0-D3 /

 GAL:
 si003-14.u5 (16V8B)

 MISC:
 P3 10 Pin Header for Buttons 4-6 (Kick) for each player
 P4 10 Pin Header (unused)

***************************************************************************/

ROM_START( survarts )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "prl-r6.u4", 0x000000, 0x080000, CRC(ef5f6e17) SHA1(1857beb15d2214c7ecb60b59e696ba24b2791734) )
	ROM_LOAD16_BYTE( "prh-r5.u3", 0x000001, 0x080000, CRC(d446f010) SHA1(fb6c349edb2e6d1fcf8ed360dbe82be6d74f91d2) )

	ROM_REGION( 0x1800000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "si001-01.u27", 0x0000000, 0x200000, CRC(8b38fbab) SHA1(c4a67b24b33d4eef7b0f885bd69cae6c67bd3981) ) /* A0 */
	ROM_LOAD( "si001-04.u26", 0x0200000, 0x200000, CRC(34248b54) SHA1(077198f8de1622b71c580e34d5ad1b6bf3229fe9) ) /* A1 */
	ROM_LOAD( "si001-07.u25", 0x0400000, 0x200000, CRC(497d6151) SHA1(a9860c75943c0fd2991660ce2a9505edc6c2fa46) ) /* A2 */

	ROM_LOAD( "si001-02.u23", 0x0600000, 0x200000, CRC(cb4a2dbd) SHA1(26cdd1b54a3fa1dc3c3a8945d1a3562e9c62ace6) ) /* B0 */
	ROM_LOAD( "si001-05.u22", 0x0800000, 0x200000, CRC(8f092381) SHA1(6c49f1f5b3c31bd7c6a93ba0450d9f64fd512633) ) /* B1 */
	ROM_LOAD( "si001-08.u21", 0x0a00000, 0x200000, CRC(182b88c4) SHA1(a5b6a3e1fd67f036b1255385e81b6a3eb69f9f3f) ) /* B2 */

	ROM_LOAD( "si001-03.u17", 0x0c00000, 0x200000, CRC(92fdf652) SHA1(cf7aeb3a1e8ffe34cf24cb919a0ab3cc90202fa9) ) /* C0 */
	ROM_LOAD( "si001-06.u16", 0x0e00000, 0x200000, CRC(9a62f532) SHA1(7e7ba1224e52b33a9bd14058230efc871178c4f8) ) /* C1 */
	ROM_LOAD( "si001-09.u15", 0x1000000, 0x200000, CRC(0955e393) SHA1(0be9134190706eaee49177034b0536b05c4bc7ac) ) /* C2 */

	ROM_FILL(                0x1200000, 0x600000, 0x000000)

//  The chip seems to use REGION1 too, but produces no sound from there.

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "si001-10.u9", 0x000000, 0x100000, CRC(5642b333) SHA1(84936af8b3882e116b279e422075f35aabdd232f) ) /* S0 */
	ROM_LOAD16_WORD_SWAP( "si001-11.u8", 0x100000, 0x100000, CRC(a81e6ea6) SHA1(499f070500895ed7b6785b42fb6bbf973fc6dc04) ) /* S1 */
	ROM_LOAD16_WORD_SWAP( "si001-12.u7", 0x200000, 0x100000, CRC(e9b2b45b) SHA1(17fd27cdb8a0b9932cb1e71e0547c0d9d6fc7d06) ) /* S2 */
	ROM_LOAD16_WORD_SWAP( "si001-13.u6", 0x300000, 0x100000, CRC(d66a7e26) SHA1(57b659daef00421b6742963f792bd5e020f625c9) ) /* S3 */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8b.u5",   0x0000, 0x0117, CRC(378ce368) SHA1(cc068c9b63e6f91165f9caec581645ad45b2d3d0) )
ROM_END

ROM_START( survartsu )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "usa-pr-l.u4", 0x000000, 0x080000, CRC(fa328673) SHA1(f7217eaa2a8d3fb7f706fa1aecaaa5b1b8d5e32c) )
	ROM_LOAD16_BYTE( "usa-pr-h.u3", 0x000001, 0x080000, CRC(6bee2635) SHA1(a2d0517bf599331ef47beb8a902589039e4502e0) )

	ROM_REGION( 0x1800000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "si001-01.u27", 0x0000000, 0x200000, CRC(8b38fbab) SHA1(c4a67b24b33d4eef7b0f885bd69cae6c67bd3981) ) /* A0 */
	ROM_LOAD( "si001-04.u26", 0x0200000, 0x200000, CRC(34248b54) SHA1(077198f8de1622b71c580e34d5ad1b6bf3229fe9) ) /* A1 */
	ROM_LOAD( "si001-07.u25", 0x0400000, 0x200000, CRC(497d6151) SHA1(a9860c75943c0fd2991660ce2a9505edc6c2fa46) ) /* A2 */

	ROM_LOAD( "si001-02.u23", 0x0600000, 0x200000, CRC(cb4a2dbd) SHA1(26cdd1b54a3fa1dc3c3a8945d1a3562e9c62ace6) ) /* B0 */
	ROM_LOAD( "si001-05.u22", 0x0800000, 0x200000, CRC(8f092381) SHA1(6c49f1f5b3c31bd7c6a93ba0450d9f64fd512633) ) /* B1 */
	ROM_LOAD( "si001-08.u21", 0x0a00000, 0x200000, CRC(182b88c4) SHA1(a5b6a3e1fd67f036b1255385e81b6a3eb69f9f3f) ) /* B2 */

	ROM_LOAD( "si001-03.u17", 0x0c00000, 0x200000, CRC(92fdf652) SHA1(cf7aeb3a1e8ffe34cf24cb919a0ab3cc90202fa9) ) /* C0 */
	ROM_LOAD( "si001-06.u16", 0x0e00000, 0x200000, CRC(9a62f532) SHA1(7e7ba1224e52b33a9bd14058230efc871178c4f8) ) /* C1 */
	ROM_LOAD( "si001-09.u15", 0x1000000, 0x200000, CRC(0955e393) SHA1(0be9134190706eaee49177034b0536b05c4bc7ac) ) /* C2 */

	ROM_FILL(                0x1200000, 0x600000, 0x000000)

//  The chip seems to use REGION1 too, but produces no sound from there.

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "si001-10.u9", 0x000000, 0x100000, CRC(5642b333) SHA1(84936af8b3882e116b279e422075f35aabdd232f) ) /* S0 */
	ROM_LOAD16_WORD_SWAP( "si001-11.u8", 0x100000, 0x100000, CRC(a81e6ea6) SHA1(499f070500895ed7b6785b42fb6bbf973fc6dc04) ) /* S1 */
	ROM_LOAD16_WORD_SWAP( "si001-12.u7", 0x200000, 0x100000, CRC(e9b2b45b) SHA1(17fd27cdb8a0b9932cb1e71e0547c0d9d6fc7d06) ) /* S2 */
	ROM_LOAD16_WORD_SWAP( "si001-13.u6", 0x300000, 0x100000, CRC(d66a7e26) SHA1(57b659daef00421b6742963f792bd5e020f625c9) ) /* S3 */
ROM_END

ROM_START( survartsj )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "jpn-pr-l.u4", 0x000000, 0x080000, CRC(e5a52e8c) SHA1(0a51c16d23d99c3e6a12f8a96c62fe8c72179a22) )
	ROM_LOAD16_BYTE( "jan-pr-h.u3", 0x000001, 0x080000, CRC(051c9bca) SHA1(b8a7c5e4cb12cb0f05b5ba15394bd1fcf0476bf0) )  // jan typo on sticker

	ROM_REGION( 0x1800000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "si001-01.u27", 0x0000000, 0x200000, CRC(8b38fbab) SHA1(c4a67b24b33d4eef7b0f885bd69cae6c67bd3981) ) /* A0 */
	ROM_LOAD( "si001-04.u26", 0x0200000, 0x200000, CRC(34248b54) SHA1(077198f8de1622b71c580e34d5ad1b6bf3229fe9) ) /* A1 */
	ROM_LOAD( "si001-07.u25", 0x0400000, 0x200000, CRC(497d6151) SHA1(a9860c75943c0fd2991660ce2a9505edc6c2fa46) ) /* A2 */

	ROM_LOAD( "si001-02.u23", 0x0600000, 0x200000, CRC(cb4a2dbd) SHA1(26cdd1b54a3fa1dc3c3a8945d1a3562e9c62ace6) ) /* B0 */
	ROM_LOAD( "si001-05.u22", 0x0800000, 0x200000, CRC(8f092381) SHA1(6c49f1f5b3c31bd7c6a93ba0450d9f64fd512633) ) /* B1 */
	ROM_LOAD( "si001-08.u21", 0x0a00000, 0x200000, CRC(182b88c4) SHA1(a5b6a3e1fd67f036b1255385e81b6a3eb69f9f3f) ) /* B2 */

	ROM_LOAD( "si001-03.u17", 0x0c00000, 0x200000, CRC(92fdf652) SHA1(cf7aeb3a1e8ffe34cf24cb919a0ab3cc90202fa9) ) /* C0 */
	ROM_LOAD( "si001-06.u16", 0x0e00000, 0x200000, CRC(9a62f532) SHA1(7e7ba1224e52b33a9bd14058230efc871178c4f8) ) /* C1 */
	ROM_LOAD( "si001-09.u15", 0x1000000, 0x200000, CRC(0955e393) SHA1(0be9134190706eaee49177034b0536b05c4bc7ac) ) /* C2 */

	ROM_FILL(                0x1200000, 0x600000, 0x000000)

//  The chip seems to use REGION1 too, but produces no sound from there.

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "si001-10.u9", 0x000000, 0x100000, CRC(5642b333) SHA1(84936af8b3882e116b279e422075f35aabdd232f) ) /* S0 */
	ROM_LOAD16_WORD_SWAP( "si001-11.u8", 0x100000, 0x100000, CRC(a81e6ea6) SHA1(499f070500895ed7b6785b42fb6bbf973fc6dc04) ) /* S1 */
	ROM_LOAD16_WORD_SWAP( "si001-12.u7", 0x200000, 0x100000, CRC(e9b2b45b) SHA1(17fd27cdb8a0b9932cb1e71e0547c0d9d6fc7d06) ) /* S2 */
	ROM_LOAD16_WORD_SWAP( "si001-13.u6", 0x300000, 0x100000, CRC(d66a7e26) SHA1(57b659daef00421b6742963f792bd5e020f625c9) ) /* S3 */
ROM_END

/***************************************************************************

                        Dynagears
Sammy, 1993

This game runs on SSV hardware.

Game PCB Layout
---------------

SAM-5127
|----------------------------------------|
| SI002-10.U6                            |
|                                        |
| SI002-09.U7                            |
|                                        |
| SI002-08.U8                            |
|                                        |
| SI002-07.U9                            |
|                                        |
|                                        |
|                                        |
|                                        |
|                                        |
|                                        |
|                                        |
|                                        |
|                           SI002-05.U22 |
|                                        |
|                           SI002-02.U23 |
|                                        |
|                                        |
|SI002-PRH.U3                            |
|                                        |
|SI002-PRL.U4                            |
|                                        |
|             SI002-06.U16  SI002-04.U26 |
|PAL                                     |
|(SI002-14)   SI002-03.U17  SI002-01.U27 |
|                                        |
|----------------------------------------|

***************************************************************************/

ROM_START( dynagear )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "si002-prl.u4", 0x000000, 0x080000, CRC(71ba29c6) SHA1(ef43ab665daa4fc9ee01996d03f2f0b4c74c8435) )
	ROM_LOAD16_BYTE( "si002-prh.u3", 0x000001, 0x080000, CRC(d0947a12) SHA1(95b54ed9dc51c952ad123103b8633a821cde05e9) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "si002-01.u27", 0x0000000, 0x200000, CRC(0060a521) SHA1(10cdb967e6cb4fc7c23c1ac40b24e35262060f5c) )
	ROM_LOAD( "si002-04.u26", 0x0200000, 0x200000, CRC(6140f47d) SHA1(49dcebe724990acdac76746886efe88b68ce956f) )

	ROM_LOAD( "si002-02.u23", 0x0400000, 0x200000, CRC(c22f2a41) SHA1(969affc8bac9a6024e7e5103384a40a6a2acf653) )
	ROM_LOAD( "si002-05.u22", 0x0600000, 0x200000, CRC(482412fd) SHA1(dfb896631b6999ce8ac6aeef84ff44150d67739a) )

	ROM_LOAD( "si002-03.u17", 0x0800000, 0x200000, CRC(4261a6b8) SHA1(df163faa84a86f126d5d405aef316ff9dd3c05eb) )
	ROM_LOAD( "si002-06.u16", 0x0a00000, 0x200000, CRC(0e1f23f6) SHA1(ea35c75776b75131ef9133a16a36d95132dc6776) )

	ROM_FILL(                0xc00000, 0x400000, 0x000000)

//  The chip seems to use REGION1 too, but produces no sound from there.

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "si002-07.u9", 0x000000, 0x100000, CRC(30d2bf11) SHA1(263e9a4e6a77aa451daf6d1225071cc1147a6541) )
	ROM_LOAD16_WORD_SWAP( "si002-08.u8", 0x100000, 0x100000, CRC(253704ee) SHA1(887ebca2af497fc59b274838cdf284223cc92c97) )
	ROM_LOAD16_WORD_SWAP( "si002-09.u7", 0x200000, 0x100000, CRC(1ea86db7) SHA1(e887ea5be99f753e73355a45e37dfddb2a1d6cf6) )
	ROM_LOAD16_WORD_SWAP( "si002-10.u6", 0x300000, 0x100000, CRC(e369c177) SHA1(646aad00a8f9eda847e9a51fb0a511bf49eb9fe2) )
ROM_END


/***************************************************************************

                        Pachinko Sexy Reaction (Japan)

(c)1998 Sammy, SSV system

P1-112C (ROM board)

Chips:  DX-102 x2
        uPD7001C (ADC?)
        64k NVRAM

***************************************************************************/

ROM_START( sxyreact )
	ROM_REGION( 0x200000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_BYTE( "ac414e00.u2",  0x000000, 0x080000, CRC(d5dd7593) SHA1(ad1c7c2f27e0423ab346172a5c91316c9c0b3620) )
	ROM_LOAD16_BYTE( "ac413e00.u1",  0x000001, 0x080000, CRC(f46aee4a) SHA1(8336304797987321903977373dec027cfca2e211) )
	ROM_LOAD16_BYTE( "ac416e00.u47", 0x100000, 0x080000, CRC(e0f7bba9) SHA1(5eafd72c9fa4588f18fa02113a93abdcaf8d8693) )
	ROM_LOAD16_BYTE( "ac415e00.u46", 0x100001, 0x080000, CRC(92de1b5f) SHA1(69e30ffc0c59e7dafe3f9c76bfee782028dab042) )

	ROM_REGION( 0x2800000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "ac1401m0.u6",  0x0000000, 0x400000, CRC(0b7b693c) SHA1(1e65c3f55cf3aa63d4229d30b5894c89b83cdf3e) )
	ROM_LOAD( "ac1402m0.u9",  0x0400000, 0x400000, CRC(9d593303) SHA1(c02037fabe8a74f01a25357ffdd3ce01b930008b) )
	ROM_LOAD( "ac1403m0.u12", 0x0800000, 0x200000, CRC(af433eca) SHA1(dfd83eba390171d93bc6888cc1d24a9a38d900bd) )

	ROM_LOAD( "ac1404m0.u7",  0x0a00000, 0x400000, CRC(cdda2ccb) SHA1(c22ff59e1cf621e0288537be567e0b42bf8e9bcf) )
	ROM_LOAD( "ac1405m0.u10", 0x0e00000, 0x400000, CRC(e5e7a5df) SHA1(9ab32f2a1ef055825b6cd3f643af1bd62c53f46a) )
	ROM_LOAD( "ac1406m0.u13", 0x1200000, 0x200000, CRC(c7053409) SHA1(468527a24ea592dbd884431ca57d43790cbc2456) )

	ROM_LOAD( "ac1407m0.u8",  0x1400000, 0x400000, CRC(28c83d5e) SHA1(abf4f0c1e2caa5cfa5a18fc95c025d73d6a8bc95) )
	ROM_LOAD( "ac1408m0.u11", 0x1800000, 0x400000, CRC(c45bab47) SHA1(d00802005e091088eabeb672a6428417db43cb66) )
	ROM_LOAD( "ac1409m0.u14", 0x1c00000, 0x200000, CRC(be1c66c2) SHA1(6d7b60d3b4286a768eac122c3d163e6e5287adc3) )

	ROM_FILL(                 0x1e00000, 0xa00000, 0x000000)

	ROM_REGION16_BE( 0x400000, "ensoniq.0", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "ac1410m0.u41", 0x000000, 0x400000, CRC(2a880afc) SHA1(193235bccde28a7d693a1a1f0159260a3a63a7d5) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "ac1411m0.u42", 0x200000, 0x200000, CRC(2ba4ca43) SHA1(9cddf57094e68d3840a37f44fbdf2f43f539ba11) )
	ROM_CONTINUE( 0x000000, 0x200000 )  // this will go in region 3

	// a few sparse samples are played from here
	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_COPY( "ensoniq.1", 0x000000,    0x200000, 0x200000 )
ROM_END

/***************************************************************************

                        Pachinko Sexy Reaction 2 (Japan)

Pachinko Sexy Reaction 2
Sammy, 1999

SAMMY B1-001A
|---------------------------------------------------------|
|                                                         |
|                                                         |
|                                                         |
|                                                         |
|               U12   AC1702T00.U9                        |
|                         AC1701T00.U6                    |
|                                                         |
|                                                         |
|                                                         |
|CN8                  AC1704T00.U10       AC1706T00.U11   |
|               U13      AC1703T00.U7   U14   AC1705T00.U8|
|                                                         |
|                                                         |
|                                                         |
|                                                         |
|         AC1708T00.U42                                   |
|CN3             AC1707T00.U41                            |
|                                                  BATT   |
|                      AC1714E00.U32            U55       |
|        CN5                   U47  U46  U2 U1  U35       |
|     D7001C                                8464  GAL16V8 |
|                                                         |
|---------------------------------------------------------|
Notes:
      6264     - Fujitsu MB8464A-10LL-SK 8K x8 SRAM (DIP28)
      D7001C   - NEC D7001C UART (DIP16)
      BATT     - 3.6V NICD Battery
      CN5      - 3 pin connector
      CN3      - 10 pin connector
      CN8      - 9 pin connector
      U12,U13, \
      U14      / Locations for SOP44 surface mounted ROMs, unpopulated
      U47,U46, \
      U2,U1    / Locations for DIP32 ROMs, unpopulated
      U35      - Location for NEC D71051GU, unpopulated
      U55      - Location for MSM62X42B Real Time Clock, unpopulated
      GAL16V8  - Lattice GAL16V8D stamped 'AC1709G00' (DIP20)
      U41,U42, \
      U7,U8,   \
      U10,U11  / OKI M27C3252CZ 32MBit SOP44 surface mounted maskROMs

      U32      - ST Microelectronics M27C160 16MBit EPROM (DIP42)

***************************************************************************/

ROM_START( sxyreac2 )
	ROM_REGION( 0x200000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "ac1714e00.u32",  0x000000, 0x200000, CRC(78075d70) SHA1(05c84bb32c6f97fceb5436d192c14cac79d9ab07) )

	ROM_REGION( 0x2000000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "ac1701t00.u6",  0x0000000, 0x400000, CRC(e14611c2) SHA1(0eaf28b27b879b6ce99bea03b286717a2d6f60f4) )
	ROM_LOAD( "ac1702t00.u9",  0x0400000, 0x400000, CRC(2c8b07f8) SHA1(e4128075c207d03206085f58b5aa8ebd28d3c2a9) )

	ROM_LOAD( "ac1703t00.u7",  0x0800000, 0x400000, CRC(d6c7e861) SHA1(b4c17829222f5b9430e96183f77fa49ec040061e) )
	ROM_LOAD( "ac1704t00.u10", 0x0c00000, 0x400000, CRC(5fa7ccf0) SHA1(9972e3e689024505739eeaefcc12670918e2cbd8) )

	ROM_LOAD( "ac1705t00.u8",  0x1000000, 0x400000, CRC(2dff0652) SHA1(3c68ec3b233f248208ea80e4799a9504318b4e7c) )
	ROM_LOAD( "ac1706t00.u11", 0x1400000, 0x400000, CRC(e7a168e0) SHA1(b4e19cc3a1fd0f18db7476ebe7cbb397c60e01b3) )

	ROM_FILL(                 0x1800000, 0x800000, 0x000000)

	ROM_REGION16_BE( 0x400000, "ensoniq.0", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "ac1707t00.u41", 0x000000, 0x400000, CRC(28999bc4) SHA1(4cddaa4a155cc03d456e6edb20dd207f7ff3d9c4) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", 0 ) /* Samples */
	ROM_LOAD16_WORD_SWAP( "ac1708t00.u42", 0x200000, 0x200000, CRC(7001eec0) SHA1(cc568ef90ec7201a73e9dc217d72cfbc3860e6b8) )
	ROM_CONTINUE( 0x000000, 0x200000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_COPY( "ensoniq.1", 0x000000,    0x200000, 0x200000 )
ROM_END


/***************************************************************************

                                Storm Blade

----------------------
System SSV (STA-0001B)
----------------------
CPU  : NEC D70615GD-16-S (V60)
Sound: Ensoniq ES5506 (OTTOR2)
OSC  : 42.9545MHz(X2) 48.0000MHz(X3)

Custom chips:
ST-0004 (Video DAC?)
ST-0005 (Parallel I/O?)
ST-0006 (Video controller)
ST-0007 (System controller)

Program Work RAM  : 256Kbitx2 (expandable to 1Mx2)
Object Work RAM   : 1Mbitx2
Color Palette RAM : 256Kbitx3 (expandable to 1Mx3)

-------------------------
SSV Subboard (VISCO-001B)
-------------------------

VISCO-001B
|-----------------------|CN1|-|
|                             |
|                 D3.U2*   L  |
|                 D2.U5*   o  |
|         U15*    D1.U8*   g  |
| SB-SND0.U22     D0.U12*  i  |
|                 C3.U1*   c  |
|              SB-C2.U4       |
|              SB-C1.U7    c  |
| uPD4701AC    SB-C0.U11   h  |
| ADC0809         B3.U14*  i  |
| SB-PD0.U26   SB-B2.U18   p  |
| U33J.U33     SB-B1.U21   s  |
| U37J.U37     SB-B0.U25      |
|6264 VG003-22    A3.U28*  x  |
|    ST010     SB-A2.U32      |
|              SB-A1.U35   1  |
|3790 BT2* BT1 SB-A0.U41   7  |
|                             |
|-----------------------------|

  VG003-22: GAL16V8B (undumped) at U29 (same GAL as Drift Out '94 - The Hard Order)
SETA ST010: Custom programmed uPD96050 MCU used for math caculations
ADC0809CCN: 8bit microprocessor compatible A/D converters with 8-Channel Multiplexer
 uPD4701AC: X, Y 2-axis Incremental Encoder Counter
       CN1: 5-pin male header/connector
      3790: MB3790 Bi-CMOS Battery Backup IC
      6264: Sharp LH5168D-10L 8K x 8bit CMOS SRAM
       BT1: CR2032 3v battery

* Denotes unpopulated component

U37, U33 = 27c040
U22, U41, U35, U25, U21, U11, U7  = 16 MEG MASK ROMS
U32, U18, U4 = 4 MEG MASK ROMS
U26 = 8 MEG MASK ROM

***************************************************************************/

ROM_START( stmblade )
	ROM_REGION( 0x400000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "sb-pd0.u26",  0x000000, 0x100000, CRC(91c4fbf7) SHA1(68e57ea2a9756a95a81c6688905352d631e9f2de) )
	ROM_LOAD16_BYTE( "s-blade.u37", 0x200000, 0x080000, CRC(a6a42cc7) SHA1(4bff79ff03b81a7ed96d3ad285242580146976be) )
	ROM_RELOAD(                     0x300000, 0x080000)
	ROM_LOAD16_BYTE( "s-blade.u33", 0x200001, 0x080000, CRC(16104ca6) SHA1(63835051c358dce33d92974d1de911b98835a3d9) )
	ROM_RELOAD(                     0x300001, 0x080000)

	ROM_REGION( 0x1800000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "sb-a0.u41", 0x0000000, 0x200000, CRC(2a327b51) SHA1(fb1e92b7f740a80cb0c977e106d0c4bfee092dad) )
	ROM_LOAD( "sb-a1.u35", 0x0200000, 0x200000, CRC(246f6f28) SHA1(09171f04452fbcf9e3333c135288fd6e5b8244f7) )
	ROM_LOAD( "sb-a2.u32", 0x0400000, 0x080000, CRC(2049acf3) SHA1(3982b4650921da0563336060887767627f8679ab) )
	ROM_LOAD( "sb-b0.u25", 0x0600000, 0x200000, CRC(b3aa3e68) SHA1(990be5925b6c8c0d0e83ca9064425d93853fe206) )
	ROM_LOAD( "sb-b1.u21", 0x0800000, 0x200000, CRC(e95b38e7) SHA1(9256f027e4c496e3bf96ecb65c0f3e69791e2755) )
	ROM_LOAD( "sb-b2.u18", 0x0a00000, 0x080000, CRC(d080e620) SHA1(a262b42214c09fccb8f4878d8566e2acd87dbd23) )
	ROM_LOAD( "sb-c0.u11", 0x0c00000, 0x200000, CRC(825dd8f1) SHA1(39d32f54c97e21f92598442f05fd91ae2403a0d2) )
	ROM_LOAD( "sb-c1.u7",  0x0e00000, 0x200000, CRC(744afcd7) SHA1(db716a1a2ad5864ebdb4865430cb637fb94ed34f) )
	ROM_LOAD( "sb-c2.u4",  0x1000000, 0x080000, CRC(fd1d2a92) SHA1(957a8a52b79e252c7f1a4b6383107ae609dce5ef) )
	ROM_FILL(              0x1200000, 0x600000, 0x000000)

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "sb-snd0.u22", 0x000000, 0x200000, CRC(4efd605b) SHA1(9c97be105c923c7db847d9b9aea37025edb685a0) )

	ROM_REGION( 0x11000, "st010", 0)
	ROM_LOAD( "st010.bin",    0x000000, 0x011000, CRC(aa11ee2d) SHA1(cc1984e989cb94e3dcbb5f99e085b5414e18a017) )
	ROM_REGION( 0x10000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x1000, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( stmbladej )
	ROM_REGION( 0x400000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "sb-pd0.u26",  0x000000, 0x100000, CRC(91c4fbf7) SHA1(68e57ea2a9756a95a81c6688905352d631e9f2de) )
	ROM_LOAD16_BYTE( "u37j.u37", 0x200000, 0x080000, CRC(dce20df8) SHA1(d589bf7bebbf6b3c76ddb4b1f8d0c7d6bee34561) )
	ROM_RELOAD(                  0x300000, 0x080000)
	ROM_LOAD16_BYTE( "u33j.u33", 0x200001, 0x080000, CRC(12f68940) SHA1(c50caee87cdcbb3a4af3a139234ed12942cfab72) )
	ROM_RELOAD(                  0x300001, 0x080000)

	ROM_REGION( 0x1800000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "sb-a0.u41", 0x0000000, 0x200000, CRC(2a327b51) SHA1(fb1e92b7f740a80cb0c977e106d0c4bfee092dad) )
	ROM_LOAD( "sb-a1.u35", 0x0200000, 0x200000, CRC(246f6f28) SHA1(09171f04452fbcf9e3333c135288fd6e5b8244f7) )
	ROM_LOAD( "sb-a2.u32", 0x0400000, 0x080000, CRC(2049acf3) SHA1(3982b4650921da0563336060887767627f8679ab) )
	ROM_LOAD( "sb-b0.u25", 0x0600000, 0x200000, CRC(b3aa3e68) SHA1(990be5925b6c8c0d0e83ca9064425d93853fe206) )
	ROM_LOAD( "sb-b1.u21", 0x0800000, 0x200000, CRC(e95b38e7) SHA1(9256f027e4c496e3bf96ecb65c0f3e69791e2755) )
	ROM_LOAD( "sb-b2.u18", 0x0a00000, 0x080000, CRC(d080e620) SHA1(a262b42214c09fccb8f4878d8566e2acd87dbd23) )
	ROM_LOAD( "sb-c0.u11", 0x0c00000, 0x200000, CRC(825dd8f1) SHA1(39d32f54c97e21f92598442f05fd91ae2403a0d2) )
	ROM_LOAD( "sb-c1.u7",  0x0e00000, 0x200000, CRC(744afcd7) SHA1(db716a1a2ad5864ebdb4865430cb637fb94ed34f) )
	ROM_LOAD( "sb-c2.u4",  0x1000000, 0x080000, CRC(fd1d2a92) SHA1(957a8a52b79e252c7f1a4b6383107ae609dce5ef) )
	ROM_FILL(              0x1200000, 0x600000, 0x000000)

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "sb-snd0.u22", 0x000000, 0x200000, CRC(4efd605b) SHA1(9c97be105c923c7db847d9b9aea37025edb685a0) )

	ROM_REGION( 0x11000, "st010", 0)
	ROM_LOAD( "st010.bin",    0x000000, 0x011000, CRC(aa11ee2d) SHA1(cc1984e989cb94e3dcbb5f99e085b5414e18a017) )
	ROM_REGION( 0x10000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x1000, "dspdata", ROMREGION_ERASEFF)
ROM_END


/***************************************************************************

                                Twin Eagle II
SETA, 1994

STS0003 TWIN EAGLE
|-------------------------------------|
|                                     |
|                                     |
|                 SX002-09.u2         |
|                                     |
|                 SX002-08.u3         |
|                                     |
| SX002-11.u7     SX002-07.u6         |
|                                     |
|                                     |
| SX002-10.u14    SX002-06.u13        |
|                                     |
|                 SX002-05.u16        |
|                                     |
| SX002-12.u22    SX002-04.u19        |
|                                     |
| SX002-13                            |
|                 SX002-03.u22        |
|                                     |
|                 SX002-02.u28        |
|                                     |
|                 SX002-01.u32        |
| SETA ST010                          |
|-------------------------------------|

  SX002-13: GAL16V8B (undumped)
SETA ST010: Custom programmed uPD96050 MCU used for math caculations
All roms are 16M Mask roms

***************************************************************************/

ROM_START( twineag2 )
	ROM_REGION( 0x200000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "sx002-12.u22", 0x000000, 0x200000, CRC(846044dc) SHA1(c1c85de1c466fb7c3580824baa1571cd0fed6ec6) )

	ROM_REGION( 0x1800000, "gfx1", 0 )  /* Sprites */
	ROM_LOAD( "sx002-01.u32", 0x0000000, 0x200000, CRC(6d6896b5) SHA1(e8efd29b9f951bff6664e47cb5fd67f1d8f40608) ) /* A0 */
	ROM_LOAD( "sx002-02.u28", 0x0200000, 0x200000, CRC(3f47e97a) SHA1(5b0fdc762cf704c8bd92c4a4a42dba4a127b3d49) ) /* A1 */
	ROM_LOAD( "sx002-03.u25", 0x0400000, 0x200000, CRC(544f18bf) SHA1(539e6df1ded4e9ac8974c697215cc1e5c5a40cda) ) /* A2, A3 is unpopulated */

	ROM_LOAD( "sx002-04.u19", 0x0600000, 0x200000, CRC(58c270e2) SHA1(7629ba978b18252f375bdc16ed62388d64a35ca1) ) /* B0 */
	ROM_LOAD( "sx002-05.u16", 0x0800000, 0x200000, CRC(3c310229) SHA1(9a8b81d5f17ce3078627a697aaf07f1b3ba6e08c) ) /* B1 */
	ROM_LOAD( "sx002-06.u13", 0x0a00000, 0x200000, CRC(46d5b1f3) SHA1(3ec03eddb159eb391ccdce5a0a867a54b3350150) ) /* B2, B3 is unpopulated */

	ROM_LOAD( "sx002-07.u6",  0x0c00000, 0x200000, CRC(c30fa397) SHA1(d4575868c1b63f9e94bf24539a3fd8a85df93d0b) ) /* C0 */
	ROM_LOAD( "sx002-08.u3",  0x0e00000, 0x200000, CRC(64edcefa) SHA1(55a71afe87da93e35c5ba291e970bdcd91b52a7a) ) /* C1 */
	ROM_LOAD( "sx002-09.u2",  0x1000000, 0x200000, CRC(51527c56) SHA1(378155a585e5b847bd8ae1f17cb651138d844e33) ) /* C2, C3 is unpopulated */

	ROM_FILL(             0x1200000, 0x600000, 0x000000          )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 ) /* Samples */
	ROM_LOAD16_BYTE( "sx002-10.u14", 0x000000, 0x200000, CRC(b0669dfa) SHA1(ff805f59864ac4ccee3e249c06804d844d3df59c) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE | 0 ) /* Samples */
	ROM_LOAD16_BYTE( "sx002-11.u7",  0x000000, 0x200000, CRC(b8dd621a) SHA1(f9b43e018f2bb121e4f4e9554419cd32b870556b) )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", 0 ) /* Samples */
	ROM_COPY( "ensoniq.1", 0x000000, 0x000000, 0x400000 )

	ROM_REGION( 0x11000, "st010", 0)
	ROM_LOAD( "st010.bin",    0x000000, 0x011000, CRC(aa11ee2d) SHA1(cc1984e989cb94e3dcbb5f99e085b5414e18a017) )
	ROM_REGION( 0x10000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x1000, "dspdata", ROMREGION_ERASEFF)
ROM_END


/***************************************************************************

                Ultra Keibitai / Ultra X Weapon

(c)1995 Banpresto (developed by Seta)
Hardware is almost identical to SSV system

****************************************************************************/

ROM_START( ultrax )
	ROM_REGION( 0x200000, "maincpu", 0 )  /* V60 Code */
	ROM_LOAD16_BYTE( "71047-11.u64", 0x000000, 0x080000, CRC(593b2678) SHA1(3b24b59a21386a4688502c5f0a2dd4eb0ec92544) )
	ROM_LOAD16_BYTE( "71047-09.u65", 0x000001, 0x080000, CRC(08ea8d91) SHA1(5d2672f6c96fbbe9d80bd6539c1400b62745892a) )
	ROM_LOAD16_BYTE( "71047-12.u62", 0x100000, 0x080000, CRC(76a77ab2) SHA1(0cf2f293defc23c807556ff92ea99f963fafed40) )
	ROM_LOAD16_BYTE( "71047-10.u63", 0x100001, 0x080000, CRC(7c79faf9) SHA1(40c1420eeae355efa628bbcfd69e0dd18d343fd9) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "71047-01.u73", 0x0000000, 0x200000, CRC(66662b08) SHA1(0cb683e5f85ffe21bd3367af4d3e48a484dbd4c3) )
	ROM_LOAD( "71047-02.u74", 0x0200000, 0x100000, CRC(6b00dc0c) SHA1(6af8ceed72d13f9979175c0d907a4a8c127ca1ad) )
	ROM_LOAD( "71047-03.u76", 0x0300000, 0x200000, CRC(00fcd6c2) SHA1(61d13cbafbc0fd6ff62cd08aa88591ed0fd0b182) )
	ROM_LOAD( "71047-04.u77", 0x0500000, 0x100000, CRC(d9e710d1) SHA1(063459a247f9ff81cb558802e9943b3ea8a2ea3a) )
	ROM_LOAD( "71047-05.u75", 0x0600000, 0x200000, CRC(10848193) SHA1(40b7ebb6011dc703bbf620cd22cd678c10ec67a4) )
	ROM_LOAD( "71047-06.u88", 0x0800000, 0x100000, CRC(b8ac2942) SHA1(3e85e8f5669d469dd3114455248546d32a642315) )
	ROM_FILL(                 0x0900000, 0x300000, 0x000000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 ) /* Samples */
	ROM_LOAD16_BYTE( "71047-07.u59", 0x000000, 0x200000, CRC(d9828b62) SHA1(f66a388d7a00b3a45d386671c061f5b840453451) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE | 0 ) /* Samples */
	ROM_LOAD16_BYTE( "71047-08.u60", 0x000000, 0x200000, CRC(30ebff6d) SHA1(53824c1fc37e22b545fd68b59722f7968f0ca1e2) )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", 0 ) /* Samples */
	ROM_COPY( "ensoniq.1", 0x000000, 0x000000, 0x400000 )
ROM_END


ROM_START( ultraxg )
	ROM_REGION( 0x200000, "maincpu", 0 )  /* V60 Code */
	ROM_LOAD16_BYTE( "sx010-11.5h", 0x000000, 0x080000, CRC(58554bdd) SHA1(4b35fbaca0cb458e3b21c43e4b4dfd708a7b9397) ) /* hand written label with date 2/27 & checksum "44B4" */
	ROM_LOAD16_BYTE( "sx010-09.5c", 0x000001, 0x080000, CRC(153e79b2) SHA1(e0278267e345ebd81f66fbb9d262cbd78df2d01b) ) /* hand written label with date 2/27 & checksum "449B" */
	ROM_LOAD16_BYTE( "sx010-12.5k", 0x100000, 0x080000, CRC(14ad58c9) SHA1(d6be82044a22eb21cac8f7e961a198e369743962) ) /* hand written label with date 2/27 & checksum "3CCF" */
	ROM_LOAD16_BYTE( "sx010-10.5d", 0x100001, 0x080000, CRC(7e64473e) SHA1(2444ac4d95563b18c8aec077e742b0f84fd38f43) ) /* hand written label with date 2/27 & checksum "0CC5" */

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "71047-01.u73", 0x0000000, 0x200000, CRC(66662b08) SHA1(0cb683e5f85ffe21bd3367af4d3e48a484dbd4c3) )
	ROM_LOAD( "71047-02.u74", 0x0200000, 0x100000, CRC(6b00dc0c) SHA1(6af8ceed72d13f9979175c0d907a4a8c127ca1ad) )
	ROM_LOAD( "71047-03.u76", 0x0300000, 0x200000, CRC(00fcd6c2) SHA1(61d13cbafbc0fd6ff62cd08aa88591ed0fd0b182) )
	ROM_LOAD( "71047-04.u77", 0x0500000, 0x100000, CRC(d9e710d1) SHA1(063459a247f9ff81cb558802e9943b3ea8a2ea3a) )
	ROM_LOAD( "71047-05.u75", 0x0600000, 0x200000, CRC(10848193) SHA1(40b7ebb6011dc703bbf620cd22cd678c10ec67a4) )
	ROM_LOAD( "71047-06.u88", 0x0800000, 0x100000, CRC(b8ac2942) SHA1(3e85e8f5669d469dd3114455248546d32a642315) )
	ROM_FILL(                 0x0900000, 0x300000, 0x000000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 ) /* Samples */
	ROM_LOAD16_BYTE( "71047-07.u59", 0x000000, 0x200000, CRC(d9828b62) SHA1(f66a388d7a00b3a45d386671c061f5b840453451) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE | 0 ) /* Samples */
	ROM_LOAD16_BYTE( "71047-08.u60", 0x000000, 0x200000, CRC(30ebff6d) SHA1(53824c1fc37e22b545fd68b59722f7968f0ca1e2) )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", 0 ) /* Samples */
	ROM_COPY( "ensoniq.1", 0x000000, 0x000000, 0x400000 )
ROM_END

/***************************************************************************

                            Vasara / Vasara 2

Visco Games, 2000 / 2001
(info from the vasara 2 board but vasara should be the same)

This is a sub board that fits any standard SSV main board.

There's very little on the ROM board. Just 2x 27C040 EPROMs,
4x 64Mbit SOP44 MASK ROMs, 3x 16Mbit SOP44 MASK ROMs,
some logic and 2 PALs near the PROG & DATA ROMs.

The actual ROM PCB has the capability to accept SOP44 and
TSOP48 type 1 SMT ROMs though many of the positions are unpopulated.
It's likely this same ROM board could be used for other Visco games.

ROM PCB (PCB Number: SSV_SUB)

SSV-SUB
|-------------------------------------------|
|                                           |
|            logic chips (x14)              |
|                                           |
|                                           |
|                                           |
|  U4   U8   U3   U7   U2   U6   U1   U5    |
|                                           |
|                                           |
|                                           |
|                                           |
|   U12    U11   logic (x3)   U10     U9    |
|                                           |
|                                           |
|                                           |
|                                           |
|                                           |
|                                           |
|             U38             U34           |
| U37  U36         U30  U31        PAL PAL  |
|             U39             U35           |
|                                           |
|                                           |
|-------------------------------------------|
Notes:
LOC      Vasara                  Vasara 2           Device
----------------------------------------------------------------------
U4       VASARA-1-D0             VASARA-2-D0        64M SOP44 MaskROM
U3       VASARA-1-C0             VASARA-2-C0        64M SOP44 MaskROM
U2       VASARA-1-B0             VASARA-2-B0        64M SOP44 MaskROM
U1       VASARA-1-A0             VASARA-2-A0        64M SOP44 MaskROM
U37      VASARA-1 SND1           VASARA-1 SND1      Macronix MX29F1610 SOP44 MaskROM
U36      VASARA-1 SND0           VASARA-1 SND0      Macronix MX29F1610 SOP44 MaskROM
U30      PRG-L U30               PRG-L U30          TMS 27C040 EPROM
U31      PRG-H U31               PRG-H U31          TMS 27C040 EPROM
U34      VASARA-1 DAT            VASARA-1 DAT       Macronix MX29F1610 MaskROM

Locations U5, U6, U7 &U8 wired for Macronix MX29F1610 SOP44 but not populated.
Locations U9, U10, U11, U12, U35, U38 & U39 wired for Macronix MX29F1610 TSOP48 but not populated.


File Name     Labeled as        Loc. Printed*      ROM Type
-----------------------------------------------------------------------
prg-h.u31     PRG-H U31         U31  PRG H       | 27C040
prg-l.u30     PRG-L U20         U30  PRG L      /
s1.u37        C DAT VASARA-1    U37  S1         \
s0.u36        B DAT VASARA-1    U36  S0          | Surface Mounted 16Mbit SOP44 MASK ROMs
data.u34      A SND 1 VASARA-1  U34  DATA ROM   /
d0.u4         VASARA-2-D0       U4   D0.D1      \
c0.u3         VASARA-2-C0       U3   C0.C1       | Surface Mounted 64Mbit SOP44 MASK ROMs
b0.u2         VASARA-2-B0       U2   B0.B1       |
a0.u1         VASARA-2-A0       U1   A0.A1      /

2x GAL16V8D

Printed = Info silk-screened/printed on the actual PCB

Vasara 2 has a secret character code like the Raizing games:

    Insert Coin, UP UP UP DOWN DOWN DOWN UP UP UP UP UP UP UP  (Up x3, Down x3, Up x7)

    Using this code will add a fifth selectable character called Saiga Magoichi
    To enable this character full time, set dip 8 of switch 2

****************************************************************************/

ROM_START( vasara )
	ROM_REGION( 0x400000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "data.u34",  0x000000, 0x200000, CRC(7704cc7e) SHA1(62bb018b7f0c7ee67fee37de17bb22a73bb9e420) )
	ROM_LOAD16_BYTE( "prg-l.u30", 0x200000, 0x080000, CRC(f0547886) SHA1(6a3717f8b89575d3cb4c7d56dd9df5052faa3c7f) )
	ROM_RELOAD(                   0x300000, 0x080000)
	ROM_LOAD16_BYTE( "prg-h.u31", 0x200001, 0x080000, CRC(6a39bba9) SHA1(05ede167150307d7bf59037f264b1d140f6646da) )
	ROM_RELOAD(                   0x300001, 0x080000)

	ROM_REGION( 0x2000000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "a0.u1", 0x0000000, 0x800000, CRC(673230a6) SHA1(a9d1a108c0737b709854bae199499577f5ae359e) )
	ROM_LOAD( "b0.u2", 0x0800000, 0x800000, CRC(31a2da7f) SHA1(5efec60affb2ed2b73a6694ac794d41375220609) )
	ROM_LOAD( "c0.u3", 0x1000000, 0x800000, CRC(d110dacf) SHA1(6f33bf6ce8c06f0b823b5478a56dc95095385181) )
	ROM_LOAD( "d0.u4", 0x1800000, 0x800000, CRC(82d0ca55) SHA1(5ac07df713504329fbc8e8b5374c04e53745230e) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "s0.u36", 0x000000, 0x200000, CRC(754fca02) SHA1(5b2810a36183e0d4f42f0fb4a09be033ad0db40d) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "s1.u37", 0x000000, 0x200000, CRC(5f303698) SHA1(bd6495f912aa9d761d245ef0a1566d9d7bdbb2ad) )
ROM_END

ROM_START( vasara2 )
	ROM_REGION( 0x400000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "data.u34",  0x000000, 0x200000, CRC(493d0103) SHA1(fda68fb089328cabb3bbd52f8703b445a9509bf1) )
	ROM_LOAD16_BYTE( "prg-l.u30", 0x200000, 0x080000, CRC(40e6f5f6) SHA1(05fee4535ffe8403e86ba92a58e5f2d040489c8e) )
	ROM_RELOAD(                   0x300000, 0x080000)
	ROM_LOAD16_BYTE( "prg-h.u31", 0x200001, 0x080000, CRC(c958e146) SHA1(568878526cef76ac0ce4feeaa46e7039291e5f77) )
	ROM_RELOAD(                   0x300001, 0x080000)

	ROM_REGION( 0x2000000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "a0.u1", 0x0000000, 0x800000, CRC(a6306c75) SHA1(bad715e53426a295d3571c025e0539d5f81ce5ab) )
	ROM_LOAD( "b0.u2", 0x0800000, 0x800000, CRC(227cbd9f) SHA1(a02787943b659508ce1589cdc7a372cc02826a10) )
	ROM_LOAD( "c0.u3", 0x1000000, 0x800000, CRC(54ede017) SHA1(4a7ff7ff8ec5843837016f35a588983b5ace06ff) )
	ROM_LOAD( "d0.u4", 0x1800000, 0x800000, CRC(4be8479d) SHA1(cbb5943dfae86f4d571459263199a63399dedc20) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "s0.u36", 0x000000, 0x200000, CRC(2b381b33) SHA1(b9dd13651e4b8d0b9e3bc4c592022f31ea634d19) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "s1.u37", 0x000000, 0x200000, CRC(11cd7098) SHA1(f75288b5c89df039dfb41d66bd275cda8605e75a) )
ROM_END

ROM_START( vasara2a )
	ROM_REGION( 0x400000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "data.u34",     0x000000, 0x200000, CRC(493d0103) SHA1(fda68fb089328cabb3bbd52f8703b445a9509bf1) )
	ROM_LOAD16_BYTE( "basara-l.u30", 0x200000, 0x080000, CRC(fd88b068) SHA1(a86e3ffc870e6f6f7f18273428b24d938d6b9c3d) )
	ROM_RELOAD(                      0x300000, 0x080000)
	ROM_LOAD16_BYTE( "basara-h.u31", 0x200001, 0x080000, CRC(91d641e6) SHA1(4987d1771a90c9f1ce45c2dd2de5b2922d5d19c5) )
	ROM_RELOAD(                      0x300001, 0x080000)

	ROM_REGION( 0x2000000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "a0.u1", 0x0000000, 0x800000, CRC(a6306c75) SHA1(bad715e53426a295d3571c025e0539d5f81ce5ab) )
	ROM_LOAD( "b0.u2", 0x0800000, 0x800000, CRC(227cbd9f) SHA1(a02787943b659508ce1589cdc7a372cc02826a10) )
	ROM_LOAD( "c0.u3", 0x1000000, 0x800000, CRC(54ede017) SHA1(4a7ff7ff8ec5843837016f35a588983b5ace06ff) )
	ROM_LOAD( "d0.u4", 0x1800000, 0x800000, CRC(4be8479d) SHA1(cbb5943dfae86f4d571459263199a63399dedc20) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "s0.u36", 0x000000, 0x200000, CRC(2b381b33) SHA1(b9dd13651e4b8d0b9e3bc4c592022f31ea634d19) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE | 0 )   /* Samples */
	ROM_LOAD16_BYTE( "s1.u37", 0x000000, 0x200000, CRC(11cd7098) SHA1(f75288b5c89df039dfb41d66bd275cda8605e75a) )
ROM_END

/***************************************************************************

Mobile Suit Gundam Final Shooting
Banpresto, 1995

Uses main board STA-0001B SYSTEM SSV

Game Board Layout
-----------------

VISCO (no other PCB numbers)
|---------------------------------------------------------|
|                 GAL16V8(2)                              |
|   VG004-01.U33            VG004-07.U39                  |
|      VG004-02.U34           VG004-08.U40                |
|        VG004-03.U35           VG004-10.U45      TC514400|
|          VG004-04.U36           VG004-09.U43            |
|            VG004-05.U37           VG004-11.U48  TC514400|
|D1            VG004-06.U38                               |
|                                       100MHz    TC514400|
|                                                         |
|                                |--------------| TC514400|
|      TC551001                  |              |         |
|P           |-------| GAL20V8   |              | TC514400|
| S2914      | SETA  | GAL20V8   |     SETA     |         |
|            |ST-0009| GAL20V8   |              | TC514400|
|            |       |           |   ST-0020    |         |
|            |-------|           |              | TC514400|
|      TC551001                  |              |         |
|           SSVV7.U16            |--------------| TC514400|
|               VG004-12.U4                               |
|C1                 VG004-13.U5  TC514260      VG004-14.U3|
| ADC0809             TC55257    GAL16V8(1) SSV2SET0.U1   |
|                     TC55257           SSVSET1.U2        |
|---------------------------------------------------------|
Notes:
      D1         - 4 pin connector for power
      P          - 10 pin connector
      C1         - 10 pin connector for analog controls
      ADC0809    - National Semiconductor ADC0809 8-Bit Microprocessor Compatible A/D Converter with 8-Channel Multiplexer (DIP28)
      S2914      - Seiko S2914 EEPROM (DIP8)
      TC551001   - Toshiba TC551001BFL-70L 128K x8 SRAM (SOP32)
      TC55257    - Toshiba TC55257N-70L 32K x8 SRAM (SOP28)
      TC514260   - Toshiba TC514260BJ-70 256K x16 DRAM (SOJ40)
      GAL16V8(1) - Lattice GAL16V8B stamped 'VG004-18' (DIP20)
      GAL16V8(2) - Lattice GAL16V8B stamped 'VG004-19' (DIP20)
      GAL20V8    - Lattice GAL20V8B all 3 stamped 'VG004-20' and have identical contents (DIP24)
      U16,U2,U1  - 4M MaskROM (DIP32)
      All other ROMs are 8M/16M MaskROM (DIP42)
      Custom Seta ICs -
                       ST-0009 (QFP176)
                       ST-0020 (QFP304, heatsinked)

****************************************************************************/

ROM_START( gdfs )
	ROM_REGION( 0x400000, "maincpu", 0 )     /* V60 Code */
	ROM_LOAD16_WORD( "vg004-14.u3",   0x000000, 0x100000, CRC(d88254df) SHA1(ccdfd42e4ce3941018f83e300da8bf7a5950f65c) )
	ROM_RELOAD(                       0x100000, 0x100000)
	ROM_LOAD16_BYTE( "ssv2set0.u1",   0x200000, 0x080000, CRC(c23b9e2c) SHA1(9026e065252981fb403255ddc5782359c0088e8a) )
	ROM_RELOAD(                       0x300000, 0x80000)
	ROM_LOAD16_BYTE( "ssv2set1.u2",   0x200001, 0x080000, CRC(d7d52570) SHA1(12e7531519a0a4331e409991265908fb518286ef) )
	ROM_RELOAD(                       0x300001, 0x80000)

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "vg004-09.u43", 0x000000, 0x200000, CRC(b7382cfa) SHA1(df735470181c16f8aac0e3be76e1ed53a32dbb9c) )
	ROM_LOAD( "vg004-10.u45", 0x200000, 0x200000, CRC(b3c6b1cb) SHA1(c601213e35d8dfd1244921da5c093f82145706d2) )
	ROM_LOAD( "vg004-11.u48", 0x400000, 0x200000, CRC(1491def1) SHA1(344043302c81b4118cac4f692375b8af7ea68570) )

	ROM_REGION( 0x1000000, "st0020_spr", /*0*/0 )   // Zooming Sprites, read by a blitter
	ROM_LOAD( "vg004-01.u33", 0x0000000, 0x200000, CRC(aa9a81c2) SHA1(a7d005f9be199e317aa4c6aed8a2ab322fe82119) )
	ROM_LOAD( "vg004-02.u34", 0x0200000, 0x200000, CRC(fa40ecb4) SHA1(0513f3b6879dc7d207646d949d6ddb7251f77bcc) )
	ROM_LOAD( "vg004-03.u35", 0x0400000, 0x200000, CRC(90004023) SHA1(041edb77b34e6677ac5b85ce542d87a9bb1baf31) )
	ROM_LOAD( "vg004-04.u36", 0x0600000, 0x200000, CRC(fdafd289) SHA1(3ff1969a176d13bfa68a48c9ed582f5789b1047f) )
	ROM_LOAD( "vg004-06.u38", 0x0a00000, 0x200000, CRC(3402325f) SHA1(7ea169c1f8b01a37bd7dbb4d486d38bdac62be5b) )
	ROM_LOAD( "vg004-05.u37", 0x0800000, 0x200000, CRC(9ae488b0) SHA1(7823cc689c588f3dbcafe9bdc94c094d6e9cd605) )
	ROM_LOAD( "vg004-07.u39", 0x0c00000, 0x200000, CRC(5e89fcf9) SHA1(db727ec8117e84c98037c756715e28fd5e39972a) )
	ROM_LOAD( "vg004-08.u40", 0x0e00000, 0x200000, CRC(6b1746dc) SHA1(35e5ee02975474985a4a611dcc439fc3050b7f94) )

	ROM_REGION( 0x80000, "gfx3", 0 )    // Tilemap
	ROM_LOAD( "ssvv7.u16",    0x0000000, 0x080000, CRC(f1c3ab6f) SHA1(b7f54f7ae60650fee7570aa4dd4266c629149673) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", 0 ) /* Samples */
	ROM_LOAD16_BYTE( "vg004-12.u4", 0x000000, 0x200000, CRC(eb41a4ef) SHA1(f4d0844a3c00cf90faa59ae982744b7f0bcbe218) )
	ROM_LOAD16_BYTE( "vg004-13.u5", 0x000001, 0x200000, CRC(a4ed3977) SHA1(5843d56f69789e70ce0201a693ffae322b628459) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", 0 ) /* Samples */
	ROM_COPY( "ensoniq.0", 0x000000, 0x000000, 0x400000 )
ROM_END

/***************************************************************************


                                Game Drivers


***************************************************************************/

//     year   rom       clone     machine   inputs    init      monitor manufacturer          title                                               flags

GAME( 1993,  dynagear,  0,        dynagear, dynagear, ssv_state, dynagear, ROT0,   "Sammy",              "Dyna Gear",                                                              MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1993,  keithlcy,  0,        keithlcy, keithlcy, ssv_state, keithlcy, ROT0,   "Visco",              "Dramatic Adventure Quiz Keith & Lucy (Japan)",                           MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1993,  srmp4,     0,        srmp4,    srmp4,    ssv_state, srmp4,    ROT0,   "Seta",               "Super Real Mahjong PIV (Japan)",                                         MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993,  srmp4o,    srmp4,    srmp4,    srmp4,    ssv_state, srmp4,    ROT0,   "Seta",               "Super Real Mahjong PIV (Japan, older set)",                              MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // by the numbering of the program roms this should be older

GAME( 1993,  survarts,  0,        survarts, survarts, ssv_state, survarts, ROT0,   "Sammy",              "Survival Arts (World)",                                                  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993,  survartsu, survarts, survarts, survarts, ssv_state, survarts, ROT0,   "American Sammy",     "Survival Arts (USA)",                                                    MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993,  survartsj, survarts, survarts, survarts, ssv_state, survarts, ROT0,   "Sammy",              "Survival Arts (Japan)",                                                  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1994,  drifto94,  0,        drifto94, drifto94, ssv_state, drifto94, ROT0,   "Visco",              "Drift Out '94 - The Hard Order (Japan)",                                 MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1994,  eaglshot,  0,        eaglshot, eaglshot, ssv_state, eaglshot, ROT0,   "Sammy",              "Eagle Shot Golf",                                                        MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1995,  hypreact,  0,        hypreact, hypreact, ssv_state, hypreact, ROT0,   "Sammy",              "Mahjong Hyper Reaction (Japan)",                                         MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1994,  twineag2,  0,        twineag2, twineag2, ssv_state, twineag2, ROT270, "Seta",               "Twin Eagle II - The Rescue Mission",                                     MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1995,  gdfs,      0,        gdfs,     gdfs,     ssv_state, gdfs,     ROT0,   "Banpresto",          "Mobil Suit Gundam Final Shooting (Japan)",                               MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1995,  ultrax,    0,        ultrax,   ultrax,   ssv_state, ultrax,   ROT270, "Banpresto / Tsuburaya Productions", "Ultra X Weapons / Ultra Keibitai",                        MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // 95-01-30 13:27:15 on startup
GAME( 1995,  ultraxg,   ultrax,   ultrax,   ultrax,   ssv_state, ultrax,   ROT270, "Banpresto / Tsuburaya Productions", "Ultra X Weapons / Ultra Keibitai (GAMEST review build)",  MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // 95-02-16 15:30:24 on startup (newer, but could have pause functionality due to being a review build so left as clone)

GAME( 1996,  janjans1,  0,        janjans1, janjans1, ssv_state, janjans1, ROT0,   "Visco",              "Lovely Pop Mahjong JangJang Shimasho (Japan)",                           MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1996?, meosism,   0,        meosism,  meosism,  ssv_state, meosism,  ROT0,   "Sammy",              "Meosis Magic (Japan)",                                                   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1996,  stmblade,  0,        stmblade, stmblade, ssv_state, stmblade, ROT270, "Visco",              "Storm Blade (US)",                                                       MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996,  stmbladej, stmblade, stmblade, stmblade, ssv_state, stmblade, ROT270, "Visco",              "Storm Blade (Japan)",                                                    MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1997,  hypreac2,  0,        hypreac2, hypreac2, ssv_state, hypreac2, ROT0,   "Sammy",              "Mahjong Hyper Reaction 2 (Japan)",                                       MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1997,  jsk,       0,        jsk,      jsk,      ssv_state, jsk,      ROT0,   "Visco",              "Joryuu Syougi Kyoushitsu (Japan)",                                       MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1997,  koikois2,  0,        janjans1, koikois2, ssv_state, janjans1, ROT0,   "Visco",              "Koi Koi Shimasho 2 - Super Real Hanafuda (Japan)",                       MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1997,  mslider,   0,        mslider,  mslider,  ssv_state, mslider,  ROT0,   "Visco / Datt Japan", "Monster Slider (Japan)",                                                 MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1997,  srmp7,     0,        srmp7,    srmp7,    ssv_state, srmp7,    ROT0,   "Seta",               "Super Real Mahjong P7 (Japan)",                                          MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1998,  ryorioh,   0,        ryorioh,  ryorioh,  ssv_state, ryorioh,  ROT0,   "Visco",              "Gourmet Battle Quiz Ryohrioh CooKing (Japan)",                           MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1998,  sxyreact,  0,        sxyreact, sxyreact, ssv_state, sxyreact, ROT0,   "Sammy",              "Pachinko Sexy Reaction (Japan)",                                         MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1999,  sxyreac2,  0,        sxyreac2, sxyreact, ssv_state, sxyreac2, ROT0,   "Sammy",              "Pachinko Sexy Reaction 2 (Japan)",                                       MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1999,  cairblad,  0,        cairblad, cairblad, ssv_state, cairblad, ROT270, "Sammy",              "Change Air Blade (Japan)",                                               MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 2000,  janjans2,  0,        janjans1, janjans2, ssv_state, janjans1, ROT0,   "Visco",              "Lovely Pop Mahjong JangJang Shimasho 2 (Japan)",                         MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 2000,  vasara,    0,        vasara,   vasara,   ssv_state, vasara,   ROT270, "Visco",              "Vasara",                                                                 MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 2001,  vasara2,   0,        vasara,   vasara2,  ssv_state, vasara,   ROT270, "Visco",              "Vasara 2 (set 1)",                                                       MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 2001,  vasara2a,  vasara2,  vasara,   vasara2,  ssv_state, vasara,   ROT270, "Visco",              "Vasara 2 (set 2)",                                                       MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
