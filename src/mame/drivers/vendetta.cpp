// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    Vendetta (GX081) (c) 1991 Konami

    Preliminary driver by:
    Ernesto Corvi
    someone@secureshell.com

    Notes:
    - collision detection is handled by a protection chip. Its emulation might
      not be 100% accurate.

********************************************************************************
   Game driver for "ESCAPE KIDS (TM)"  (KONAMI, 1991)
 --------------------------------------------------------------------------------

            This driver was made on the basis of 'src/drivers/vendetta.c' file.
                                         Driver by OHSAKI Masayuki (2002/08/13)

 ********************************************************************************


 ***** NOTES *****
      -------
  1) ESCAPE KIDS uses 053246's unknown function. (see video/konamiic.c)
                   (053246 register #5  UnKnown Bit #5, #3, #2 always set "1")


 ***** On the "error.log" *****
      --------------------
  1) "YM2151 Write 00 to undocumented register #xx" (xx=00-1f)
                Why???

  2) "xxxx: read from unknown 052109 address yyyy"
  3) "xxxx: write zz to unknown 052109 address yyyy"
                These are video/konamiic.c's message.
                "video/konamiic.c" checks 052109 RAM area access.
                If accessed over 0x1800 (0x3800), logged 2) or 3) messages.
                Escape Kids use 0x1800-0x19ff and 0x3800-0x39ff area.


 ***** UnEmulated *****
      ------------
  1) 0x3fc0-0x3fcf (052109 RAM area) access (053252 ???)
  2) 0x7c00 (Banked ROM area) access to data WRITE (???)
  3) 0x3fda (053248 RAM area) access to data WRITE (Watchdog ???)


 ***** ESCAPE KIDS PCB layout/ Need to dump *****
      --------------------------------------
   (Parts side view)
   +-------------------------------------------------------+
   |   R          ROM9                               [CN1] |  CN1:Player4 Input?
   |   O                                             [CN2] |           (Labeled '4P')
   |   M          ROM8                       ROM1    [SW1] |  CN2:Player3 Input?
   |   7                              [CUS1]             +-+           (Labeled '3P')
   |        [CUS7]   [CUS8]                              +-+  CN3:Stereo sound out
   | R                                       [CUS2]        |
   | O                                                   J |  SW1:Test Switch
   | M                                                   A |
   | 6    [CUS6]                                         M | ***  Custom Chips  ***
   |                                                     M |      CUS1: 053248
   | R                                                   A |      CUS2: 053252
   | O    [CUS5]                                        56P|      CUS3: 053260
   | M                                                     |      CUS4: 053246
   | 5                           ROM2  [ Z80 ]           +-+      CUS5: 053247
   |                                                     +-+      CUS6: 053251
   | R    [CUS4]                     [CUS3] [YM2151] [CN3] |      CUS7: 051962
   | O                                                     |      CUS8: 052109
   | M                                 ROM3                |
   | 4                                         [Sound AMP] |
   +-------------------------------------------------------+

  ***  Dump ROMs  ***
     1) ROM1 (17C)  32Pin 1Mbit UV-EPROM          -> save "975r01" file
     2) ROM2 ( 5F)  28Pin 512Kbit One-Time PROM   -> save "975f02" file
     3) ROM3 ( 1D)  40Pin 4Mbit MASK ROM          -> save "975c03" file
     4) ROM4 ( 3K)  42Pin 8Mbit MASK ROM          -> save "975c04" file
     5) ROM5 ( 8L)  42Pin 8Mbit MASK ROM          -> save "975c05" file
     6) ROM6 (12M)  42Pin 8Mbit MASK ROM          -> save "975c06" file
     7) ROM7 (16K)  42Pin 8Mbit MASK ROM          -> save "975c07" file
     8) ROM8 (16I)  40Pin 4Mbit MASK ROM          -> save "975c08" file
     9) ROM9 (18I)  40Pin 4Mbit MASK ROM          -> save "975c09" file
                                                        vvvvvvvvvvvv
                                                        esckidsj.zip

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

#include "cpu/m6809/konami.h" /* for the callback and the firq irq definition */
#include "machine/eepromser.h"
#include "sound/2151intf.h"
#include "sound/k053260.h"
#include "includes/konamipt.h"
#include "includes/vendetta.h"

/***************************************************************************

  EEPROM

***************************************************************************/

WRITE8_MEMBER(vendetta_state::vendetta_eeprom_w)
{
	/* bit 0 - VOC0 - Video banking related */
	/* bit 1 - VOC1 - Video banking related */
	/* bit 2 - MSCHNG - Mono Sound select (Amp) */
	/* bit 3 - EEPCS - Eeprom CS */
	/* bit 4 - EEPCLK - Eeprom CLK */
	/* bit 5 - EEPDI - Eeprom data */
	/* bit 6 - IRQ enable */
	/* bit 7 - Unused */


	if (data == 0xff ) /* this is a bug in the eeprom write code */
		return;

	/* EEPROM */
	ioport("EEPROMOUT")->write(data, 0xff);

	m_irq_enabled = (data >> 6) & 1;

	vendetta_video_banking(data & 1);
}

/********************************************/

READ8_MEMBER(vendetta_state::vendetta_K052109_r)
{
	return m_k052109->read(space, offset + 0x2000);
}

WRITE8_MEMBER(vendetta_state::vendetta_K052109_w)
{
	// *************************************************************************************
	// *  Escape Kids uses 052109's mirrored Tilemap ROM bank selector, but only during    *
	// *  Tilemap MASK-ROM Test       (0x1d80<->0x3d80, 0x1e00<->0x3e00, 0x1f00<->0x3f00)  *
	// *************************************************************************************
	if ((offset == 0x1d80) || (offset == 0x1e00) || (offset == 0x1f00))
		m_k052109->write(space, offset, data);
	m_k052109->write(space, offset + 0x2000, data);
}


void vendetta_state::vendetta_video_banking( int select )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (select & 1)
	{
		space.install_read_bank(m_video_banking_base + 0x2000, m_video_banking_base + 0x2fff, "bank4" );
		space.install_write_handler(m_video_banking_base + 0x2000, m_video_banking_base + 0x2fff, write8_delegate(FUNC(palette_device::write), m_palette.target()) );
		space.install_readwrite_handler(m_video_banking_base + 0x0000, m_video_banking_base + 0x0fff, read8_delegate(FUNC(k053247_device::k053247_r), (k053247_device*)m_k053246), write8_delegate(FUNC(k053247_device::k053247_w), (k053247_device*)m_k053246) );
		membank("bank4")->set_base(&m_paletteram[0]);
	}
	else
	{
		space.install_readwrite_handler(m_video_banking_base + 0x2000, m_video_banking_base + 0x2fff, read8_delegate(FUNC(vendetta_state::vendetta_K052109_r),this), write8_delegate(FUNC(vendetta_state::vendetta_K052109_w),this) );
		space.install_readwrite_handler(m_video_banking_base + 0x0000, m_video_banking_base + 0x0fff, read8_delegate(FUNC(k052109_device::read), (k052109_device*)m_k052109), write8_delegate(FUNC(k052109_device::write), (k052109_device*)m_k052109));
	}
}

WRITE8_MEMBER(vendetta_state::vendetta_5fe0_w)
{
	/* bit 0,1 coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	/* bit 2 = BRAMBK ?? */

	/* bit 3 = enable char ROM reading through the video RAM */
	m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 4 = INIT ?? */

	/* bit 5 = enable sprite ROM reading */
	m_k053246->k053246_set_objcha_line((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
}

void vendetta_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_Z80_NMI:
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		break;
	default:
		assert_always(FALSE, "Unknown id in vendetta_state::device_timer");
	}
}

WRITE8_MEMBER(vendetta_state::z80_arm_nmi_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	timer_set(attotime::from_usec(25), TIMER_Z80_NMI);
}

WRITE8_MEMBER(vendetta_state::z80_irq_w)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}

READ8_MEMBER(vendetta_state::z80_irq_r)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
	return 0x00;
}

/********************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, vendetta_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK("bank1")
	AM_RANGE(0x2000, 0x3fff) AM_RAM
	AM_RANGE(0x5f80, 0x5f9f) AM_DEVREADWRITE("k054000", k054000_device, read, write)
	AM_RANGE(0x5fa0, 0x5faf) AM_DEVWRITE("k053251", k053251_device, write)
	AM_RANGE(0x5fb0, 0x5fb7) AM_DEVWRITE("k053246", k053247_device, k053246_w)
	AM_RANGE(0x5fc0, 0x5fc0) AM_READ_PORT("P1")
	AM_RANGE(0x5fc1, 0x5fc1) AM_READ_PORT("P2")
	AM_RANGE(0x5fc2, 0x5fc2) AM_READ_PORT("P3")
	AM_RANGE(0x5fc3, 0x5fc3) AM_READ_PORT("P4")
	AM_RANGE(0x5fd0, 0x5fd0) AM_READ_PORT("EEPROM")
	AM_RANGE(0x5fd1, 0x5fd1) AM_READ_PORT("SERVICE")
	AM_RANGE(0x5fe0, 0x5fe0) AM_WRITE(vendetta_5fe0_w)
	AM_RANGE(0x5fe2, 0x5fe2) AM_WRITE(vendetta_eeprom_w)
	AM_RANGE(0x5fe4, 0x5fe4) AM_READWRITE(z80_irq_r, z80_irq_w)
	AM_RANGE(0x5fe6, 0x5fe7) AM_DEVREADWRITE("k053260", k053260_device, main_read, main_write)
	AM_RANGE(0x5fe8, 0x5fe9) AM_DEVREAD("k053246", k053247_device, k053246_r)
	AM_RANGE(0x5fea, 0x5fea) AM_READ(watchdog_reset_r)
	/* what is the desired effect of overlapping these memory regions anyway? */
	AM_RANGE(0x4000, 0x4fff) AM_RAMBANK("bank3")
	AM_RANGE(0x6000, 0x6fff) AM_RAMBANK("bank2")
	AM_RANGE(0x4000, 0x7fff) AM_DEVREADWRITE("k052109", k052109_device, read, write)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( esckids_map, AS_PROGRAM, 8, vendetta_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM                         // 053248 64K SRAM
	AM_RANGE(0x3f80, 0x3f80) AM_READ_PORT("P1")
	AM_RANGE(0x3f81, 0x3f81) AM_READ_PORT("P2")
	AM_RANGE(0x3f82, 0x3f82) AM_READ_PORT("P3")             // ???  (But not used)
	AM_RANGE(0x3f83, 0x3f83) AM_READ_PORT("P4")             // ???  (But not used)
	AM_RANGE(0x3f92, 0x3f92) AM_READ_PORT("EEPROM")
	AM_RANGE(0x3f93, 0x3f93) AM_READ_PORT("SERVICE")
	AM_RANGE(0x3fa0, 0x3fa7) AM_DEVWRITE("k053246", k053247_device, k053246_w)           // 053246 (Sprite)
	AM_RANGE(0x3fb0, 0x3fbf) AM_DEVWRITE("k053251", k053251_device, write)           // 053251 (Priority Encoder)
	AM_RANGE(0x3fc0, 0x3fcf) AM_DEVREADWRITE("k053252", k053252_device, read, write)              // Not Emulated (053252 ???)
	AM_RANGE(0x3fd0, 0x3fd0) AM_WRITE(vendetta_5fe0_w)      // Coin Counter, 052109 RMRD, 053246 OBJCHA
	AM_RANGE(0x3fd2, 0x3fd2) AM_WRITE(vendetta_eeprom_w)    // EEPROM, Video banking
	AM_RANGE(0x3fd4, 0x3fd4) AM_READWRITE(z80_irq_r, z80_irq_w)            // Sound
	AM_RANGE(0x3fd6, 0x3fd7) AM_DEVREADWRITE("k053260", k053260_device, main_read, main_write) // Sound
	AM_RANGE(0x3fd8, 0x3fd9) AM_DEVREAD("k053246", k053247_device, k053246_r)                // 053246 (Sprite)
	AM_RANGE(0x3fda, 0x3fda) AM_WRITENOP                // Not Emulated (Watchdog ???)
	/* what is the desired effect of overlapping these memory regions anyway? */
	AM_RANGE(0x2000, 0x2fff) AM_RAMBANK("bank3")                    // 052109 (Tilemap) 0x0000-0x0fff
	AM_RANGE(0x4000, 0x4fff) AM_RAMBANK("bank2")                    // 052109 (Tilemap) 0x2000-0x3fff, Tilemap MASK-ROM bank selector (MASK-ROM Test)
	AM_RANGE(0x2000, 0x5fff) AM_DEVREADWRITE("k052109", k052109_device, read, write)            // 052109 (Tilemap)
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")                    // 053248 '975r01' 1M ROM (Banked)
	AM_RANGE(0x8000, 0xffff) AM_ROM                         // 053248 '975r01' 1M ROM (0x18000-0x1ffff)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, vendetta_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf801) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(z80_arm_nmi_w)
	AM_RANGE(0xfc00, 0xfc2f) AM_DEVREADWRITE("k053260", k053260_device, read, write)
ADDRESS_MAP_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( vendet4p )
	PORT_START("P1")
	KONAMI8_RL_B12_COIN(1)

	PORT_START("P2")
	KONAMI8_RL_B12_COIN(2)

	PORT_START("P3")
	KONAMI8_RL_B12_COIN(3)

	PORT_START("P4")
	KONAMI8_RL_B12_COIN(4)

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, do_read)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, ready_read)
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen") /* not really vblank, object related. Its timed, otherwise sprites flicker */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, cs_write)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, clk_write)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, di_write)
INPUT_PORTS_END

static INPUT_PORTS_START( vendetta )
	PORT_INCLUDE( vendet4p )

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( esckids )
	PORT_START("P1")
	KONAMI8_RL_B12_COIN(1)      // Player 1 Control

	PORT_START("P2")
	KONAMI8_RL_B12_COIN(2)      // Player 2 Control

	PORT_START("P3")
	KONAMI8_RL_B12_COIN(3)      // Player 3 Control ???  (Not used)

	PORT_START("P4")
	KONAMI8_RL_B12_COIN(4)      // Player 4 Control ???  (Not used)

	PORT_START("SERVICE")       // Start, Service
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, do_read)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, ready_read)
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen") /* not really vblank, object related. Its timed, otherwise sprites flicker */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, cs_write)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, clk_write)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, di_write)
INPUT_PORTS_END

static INPUT_PORTS_START( esckidsj )
	PORT_INCLUDE( esckids )

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/***************************************************************************

    Machine Driver

***************************************************************************/

INTERRUPT_GEN_MEMBER(vendetta_state::vendetta_irq)
{
	if (m_irq_enabled)
		device.execute().set_input_line(KONAMI_IRQ_LINE, HOLD_LINE);
}

void vendetta_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 28, &ROM[0x10000], 0x2000);
	membank("bank1")->set_entry(0);

	m_paletteram.resize(0x1000);
	m_palette->basemem().set(m_paletteram, ENDIANNESS_BIG, 2);

	save_item(NAME(m_paletteram));
	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layerpri));
}

void vendetta_state::machine_reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_layerpri[i] = 0;
		m_layer_colorbase[i] = 0;
	}

	m_sprite_colorbase = 0;
	m_irq_enabled = 0;

	/* init banks */
	vendetta_video_banking(0);
}

WRITE8_MEMBER( vendetta_state::banking_callback )
{
	if (data >= 0x1c)
		logerror("PC = %04x : Unknown bank selected %02x\n", machine().device("maincpu")->safe_pc(), data);
	else
		membank("bank1")->set_entry(data);
}

static MACHINE_CONFIG_START( vendetta, vendetta_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KONAMI, XTAL_24MHz/8)   /* 052001 (verified on pcb) */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vendetta_state,  vendetta_irq)
	MCFG_KONAMICPU_LINE_CB(WRITE8(vendetta_state, banking_callback))

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz) /* verified with PCB */
	MCFG_CPU_PROGRAM_MAP(sound_map)
							/* interrupts are triggered by the main CPU */

	MCFG_EEPROM_SERIAL_ER5911_8BIT_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.17) /* measured on PCB */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(13*8, (64-13)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(vendetta_state, screen_update_vendetta)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("k052109", K052109, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K052109_CB(vendetta_state, vendetta_tile_callback)

	MCFG_DEVICE_ADD("k053246", K053246, 0)
	MCFG_K053246_CB(vendetta_state, sprite_callback)
	MCFG_K053246_CONFIG("gfx2", NORMAL_PLANE_ORDER, 53, 6)
	MCFG_K053246_PALETTE("palette")

	MCFG_K053251_ADD("k053251")

	MCFG_K054000_ADD("k054000")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz)  /* verified with PCB */
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_K053260_ADD("k053260", XTAL_3_579545MHz)    /* verified with PCB */
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.75)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( esckids, vendetta )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(esckids_map)

	MCFG_SCREEN_MODIFY("screen")
//MCFG_SCREEN_VISIBLE_AREA(13*8, (64-13)*8-1, 2*8, 30*8-1 )    /* black areas on the edges */
	MCFG_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )

	MCFG_DEVICE_REMOVE("k054000")
	MCFG_DEVICE_REMOVE("k052109")

	MCFG_DEVICE_ADD("k052109", K052109, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K052109_CB(vendetta_state, esckids_tile_callback)

	MCFG_DEVICE_MODIFY("k053246")
	MCFG_K053246_CONFIG("gfx2", NORMAL_PLANE_ORDER, 101, 6)

	MCFG_DEVICE_ADD("k053252", K053252, 6000000)
	MCFG_K053252_OFFSETS(12*8, 1*8)
MACHINE_CONFIG_END



/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( vendetta )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "081t01", 0x10000, 0x38000, CRC(e76267f5) SHA1(efef6c2edb4c181374661f358dad09123741b63d) )
	ROM_CONTINUE(       0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them ) */
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) /* sprites */
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) /* sprites */
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) /* sprites */
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) /* sprites */

	ROM_REGION( 0x100000, "k053260", 0 ) /* 053260 samples */
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )
ROM_END

ROM_START( vendettar )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "081r01", 0x10000, 0x38000, CRC(84796281) SHA1(e4330c6eaa17adda5b4bd3eb824388c89fb07918) )
	ROM_CONTINUE(       0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them ) */
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) /* sprites */
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) /* sprites */
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) /* sprites */
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) /* sprites */

	ROM_REGION( 0x100000, "k053260", 0 ) /* 053260 samples */
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "vendettar.nv", 0x0000, 0x080, CRC(ec3f0449) SHA1(da35b98cd10bfabe9df3ede05462fabeb0e01ca9) )
ROM_END

ROM_START( vendettaz )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "081z01.bin", 0x10000, 0x38000, CRC(4d225a8d) SHA1(fe8f6e63d033cf04c9a287d870db244fddb81f03) )
	ROM_CONTINUE(       0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them ) */
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) /* sprites */
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) /* sprites */
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) /* sprites */
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) /* sprites */

	ROM_REGION( 0x100000, "k053260", 0 ) /* 053260 samples */
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )
ROM_END

ROM_START( vendetta2p )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "081w01", 0x10000, 0x38000, CRC(cee57132) SHA1(8b6413877e127511daa76278910c2ee3247d613a) )
	ROM_CONTINUE(       0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them ) */
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) /* sprites */
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) /* sprites */
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) /* sprites */
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) /* sprites */

	ROM_REGION( 0x100000, "k053260", 0 ) /* 053260 samples */
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )
ROM_END


ROM_START( vendetta2peba )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "081-eb-a01.17c", 0x10000, 0x38000, CRC(8430bb52) SHA1(54e896510fa44e76b0640b17150210fbf6b3b5bc)) // Label was unclear apart from EB stamp on the middle line.  Bottom line looked like 401, but probably A01
	ROM_CONTINUE(       0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them ) */
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) /* sprites */
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) /* sprites */
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) /* sprites */
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) /* sprites */

	ROM_REGION( 0x100000, "k053260", 0 ) /* 053260 samples */
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )
ROM_END

ROM_START( vendetta2pu )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "081u01", 0x10000, 0x38000, CRC(b4d9ade5) SHA1(fbd543738cb0b68c80ff05eed7849b608de03395) )
	ROM_CONTINUE(       0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them ) */
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) /* sprites */
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) /* sprites */
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) /* sprites */
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) /* sprites */

	ROM_REGION( 0x100000, "k053260", 0 ) /* 053260 samples */
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )
ROM_END

ROM_START( vendetta2pd )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "081d01", 0x10000, 0x38000, CRC(335da495) SHA1(ea74680eb898aeecf9f1eec95f151bcf66e6b6cb) )
	ROM_CONTINUE(       0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them ) */
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) /* sprites */
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) /* sprites */
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) /* sprites */
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) /* sprites */

	ROM_REGION( 0x100000, "k053260", 0 ) /* 053260 samples */
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "vendetta.nv", 0x0000, 0x080, CRC(fbac4e30) SHA1(d3ff3a392550d9b06400b9292a44bdac7ba5c801) )
ROM_END




ROM_START( vendettaj )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "081p01", 0x10000, 0x38000, CRC(5fe30242) SHA1(2ea98e66637fa2ad60044b1a2b0dd158a82403a2) )
	ROM_CONTINUE(       0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "081b02", 0x000000, 0x10000, CRC(4c604d9b) SHA1(22d979f5dbde7912dd927bf5538fdbfc5b82905e) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "081a09", 0x000000, 0x080000, CRC(b4c777a9) SHA1(cc2b1dff4404ecd72b604e25d00fffdf7f0f8b52) )
	ROM_LOAD32_WORD( "081a08", 0x000002, 0x080000, CRC(272ac8d9) SHA1(2da12fe4c13921bf0d4ebffec326f8d207ec4fad) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them ) */
	ROM_LOAD64_WORD( "081a04", 0x000000, 0x100000, CRC(464b9aa4) SHA1(28066ff0a07c3e56e7192918a882778c1b316b37) ) /* sprites */
	ROM_LOAD64_WORD( "081a05", 0x000002, 0x100000, CRC(4e173759) SHA1(ce803f2aca7d7dedad00ab30e112443848747bd2) ) /* sprites */
	ROM_LOAD64_WORD( "081a06", 0x000004, 0x100000, CRC(e9fe6d80) SHA1(2b7fc9d7fe43cd85dc8b975fe639c273cb0d9256) ) /* sprites */
	ROM_LOAD64_WORD( "081a07", 0x000006, 0x100000, CRC(8a22b29a) SHA1(be539f21518e13038ab1d4cc2b2a901dd3e621f4) ) /* sprites */

	ROM_REGION( 0x100000, "k053260", 0 ) /* 053260 samples */
	ROM_LOAD( "081a03", 0x000000, 0x100000, CRC(14b6baea) SHA1(fe15ee57f19f5acaad6c1642d51f390046a7468a) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "vendettaj.nv", 0x0000, 0x080, CRC(3550a54e) SHA1(370cd40a12c471b3b6690ecbdde9c7979bc2a652) )
ROM_END

ROM_START( esckids )
	ROM_REGION( 0x048000, "maincpu", 0 )        // Main CPU (053248) Code & Banked (1M x 1)
	ROM_LOAD( "17c.bin", 0x010000, 0x018000, CRC(9dfba99c) SHA1(dbcb89aad5a9addaf7200b2524be999877313a6e) )
	ROM_CONTINUE(       0x008000, 0x008000 )

	ROM_REGION( 0x010000, "audiocpu", 0 )       // Sound CPU (Z80) Code (512K x 1)
	ROM_LOAD( "975f02", 0x000000, 0x010000, CRC(994fb229) SHA1(bf194ae91240225b8edb647b1a62cd83abfa215e) )

	ROM_REGION( 0x100000, "k052109", 0 )       // Tilemap MASK-ROM (4M x 2)
	ROM_LOAD32_WORD( "975c09", 0x000000, 0x080000, CRC(bc52210e) SHA1(301a3892d250495c2e849d67fea5f01fb0196bed) )
	ROM_LOAD32_WORD( "975c08", 0x000002, 0x080000, CRC(fcff9256) SHA1(b60d29f4d04f074120d4bb7f2a71b9e9bf252d33) )

	ROM_REGION( 0x400000, "gfx2", 0 )       // Sprite MASK-ROM (8M x 4)
	ROM_LOAD64_WORD( "975c04", 0x000000, 0x100000, CRC(15688a6f) SHA1(a445237a11e5f98f0f9b2573a7ef0583366a137e) )
	ROM_LOAD64_WORD( "975c05", 0x000002, 0x100000, CRC(1ff33bb7) SHA1(eb17da33ba2769ea02f91fece27de2e61705e75a) )
	ROM_LOAD64_WORD( "975c06", 0x000004, 0x100000, CRC(36d410f9) SHA1(2b1fd93c11839480aa05a8bf27feef7591704f3d) )
	ROM_LOAD64_WORD( "975c07", 0x000006, 0x100000, CRC(97ec541e) SHA1(d1aa186b17cfe6e505f5b305703319299fa54518) )

	ROM_REGION( 0x100000, "k053260", 0 )    // Samples MASK-ROM (4M x 1)
	ROM_LOAD( "975c03", 0x000000, 0x080000, CRC(dc4a1707) SHA1(f252d08483fd664f8fc03bf8f174efd452b4cdc5) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "esckids.nv", 0x0000, 0x080, CRC(a8522e1f) SHA1(43f82fce3c3b854bc8898c63dffc7c01b288c8aa) )
ROM_END


ROM_START( esckidsj )
	ROM_REGION( 0x048000, "maincpu", 0 )        // Main CPU (053248) Code & Banked (1M x 1)
	ROM_LOAD( "975r01", 0x010000, 0x018000, CRC(7b5c5572) SHA1(b94b58c010539926d112c2dfd80bcbad76acc986) )
	ROM_CONTINUE(       0x008000, 0x008000 )

	ROM_REGION( 0x010000, "audiocpu", 0 )       // Sound CPU (Z80) Code (512K x 1)
	ROM_LOAD( "975f02", 0x000000, 0x010000, CRC(994fb229) SHA1(bf194ae91240225b8edb647b1a62cd83abfa215e) )

	ROM_REGION( 0x100000, "k052109", 0 )       // Tilemap MASK-ROM (4M x 2)
	ROM_LOAD32_WORD( "975c09", 0x000000, 0x080000, CRC(bc52210e) SHA1(301a3892d250495c2e849d67fea5f01fb0196bed) )
	ROM_LOAD32_WORD( "975c08", 0x000002, 0x080000, CRC(fcff9256) SHA1(b60d29f4d04f074120d4bb7f2a71b9e9bf252d33) )

	ROM_REGION( 0x400000, "gfx2", 0 )       // Sprite MASK-ROM (8M x 4)
	ROM_LOAD64_WORD( "975c04", 0x000000, 0x100000, CRC(15688a6f) SHA1(a445237a11e5f98f0f9b2573a7ef0583366a137e) )
	ROM_LOAD64_WORD( "975c05", 0x000002, 0x100000, CRC(1ff33bb7) SHA1(eb17da33ba2769ea02f91fece27de2e61705e75a) )
	ROM_LOAD64_WORD( "975c06", 0x000004, 0x100000, CRC(36d410f9) SHA1(2b1fd93c11839480aa05a8bf27feef7591704f3d) )
	ROM_LOAD64_WORD( "975c07", 0x000006, 0x100000, CRC(97ec541e) SHA1(d1aa186b17cfe6e505f5b305703319299fa54518) )

	ROM_REGION( 0x100000, "k053260", 0 )    // Samples MASK-ROM (4M x 1)
	ROM_LOAD( "975c03", 0x000000, 0x080000, CRC(dc4a1707) SHA1(f252d08483fd664f8fc03bf8f174efd452b4cdc5) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "esckidsj.nv", 0x0000, 0x080, CRC(985e2a2d) SHA1(afd9e5fc014d593d0a384326f32caf2a73fba867) )
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

DRIVER_INIT_MEMBER(vendetta_state,vendetta)
{
	m_video_banking_base = 0x4000;
}

DRIVER_INIT_MEMBER(vendetta_state,esckids)
{
	m_video_banking_base = 0x2000;
}



GAME( 1991, vendetta,    0,        vendetta, vendet4p, vendetta_state, vendetta, ROT0, "Konami", "Vendetta (World, 4 Players, ver. T)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendettar,   vendetta, vendetta, vendet4p, vendetta_state, vendetta, ROT0, "Konami", "Vendetta (US, 4 Players, ver. R)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendettaz,   vendetta, vendetta, vendet4p, vendetta_state, vendetta, ROT0, "Konami", "Vendetta (Asia, 4 Players, ver. Z)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendetta2p,  vendetta, vendetta, vendetta, vendetta_state, vendetta, ROT0, "Konami", "Vendetta (World, 2 Players, ver. W)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendetta2peba,vendetta,vendetta, vendetta, vendetta_state, vendetta, ROT0, "Konami", "Vendetta (World, 2 Players, ver. EB-A?)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendetta2pu, vendetta, vendetta, vendetta, vendetta_state, vendetta, ROT0, "Konami", "Vendetta (Asia, 2 Players, ver. U)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendetta2pd, vendetta, vendetta, vendetta, vendetta_state, vendetta, ROT0, "Konami", "Vendetta (Asia, 2 Players, ver. D)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, vendettaj,   vendetta, vendetta, vendetta, vendetta_state, vendetta, ROT0, "Konami", "Crime Fighters 2 (Japan, 2 Players, ver. P)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, esckids,     0,        esckids,  esckids, vendetta_state,  esckids,  ROT0, "Konami", "Escape Kids (Asia, 4 Players)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, esckidsj,    esckids,  esckids,  esckidsj, vendetta_state, esckids,  ROT0, "Konami", "Escape Kids (Japan, 2 Players)", MACHINE_SUPPORTS_SAVE )
