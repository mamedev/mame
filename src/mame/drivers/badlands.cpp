// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Bad Lands hardware

    driver by Aaron Giles

    Games supported:
        * Bad Lands (1989)

    Known bugs:
        * none at this time

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    MAIN CPU
    ========================================================================
    000000-03FFFF   R     xxxxxxxx xxxxxxxx   Program ROM
    FC0000          R     -------x --------   Sound command buffer full
    FC0000            W   -------- --------   Sound CPU reset
    FD0000-FD1FFF   R/W   -------- xxxxxxxx   EEPROM
    FE0000            W   -------- --------   Watchdog reset
    FE2000            W   -------- --------   VBLANK IRQ acknowledge
    FE4000          R     -------- xxxx----   Switch inputs
                    R     -------- x-------      (Self test)
                    R     -------- -x------      (VBLANK)
                    R     -------- --x-----      (Player 2 button)
                    R     -------- ---x----      (Player 1 button)
    FE6000          R     -------- xxxxxxxx   Player 1 steering
    FE6002          R     -------- xxxxxxxx   Player 2 steering
    FE6004          R     -------- xxxxxxxx   Player 1 pedal
    FE6006          R     -------- xxxxxxxx   Player 2 pedal
    FE8000            W   xxxxxxxx --------   Sound command write
    FEA000          R     xxxxxxxx --------   Sound response read
    FEC000            W   -------- -------x   Playfield tile bank select
    FEE000            W   -------- --------   EEPROM enable
    FFC000-FFC0FF   R/W   xxxxxxxx xxxxxxxx   Playfield palette RAM (128 entries)
                    R/W   x------- --------      (RGB 1 LSB)
                    R/W   -xxxxx-- --------      (Red 5 MSB)
                    R/W   ------xx xxx-----      (Green 5 MSB)
                    R/W   -------- ---xxxxx      (Blue 5 MSB)
    FFC100-FFC1FF   R/W   xxxxxxxx xxxxxxxx   Motion object palette RAM (128 entries)
    FFC200-FFC3FF   R/W   xxxxxxxx xxxxxxxx   Extra palette RAM (256 entries)
    FFE000-FFEFFF   R/W   xxxxxxxx xxxxxxxx   Playfield RAM (64x32 tiles)
                    R/W   xxx----- --------      (Palette select)
                    R/W   ---x---- --------      (Tile bank select)
                    R/W   ----xxxx xxxxxxxx      (Tile index)
    FFF000-FFFFFF   R/W   xxxxxxxx xxxxxxxx   Motion object RAM (32 entries x 4 words)
                    R/W   ----xxxx xxxxxxxx      (0: Tile index)
                    R/W   xxxxxxxx x-------      (1: Y position)
                    R/W   -------- ----xxxx      (1: Number of Y tiles - 1)
                    R/W   xxxxxxxx x-------      (3: X position)
                    R/W   -------- ----x---      (3: Priority)
                    R/W   -------- -----xxx      (3: Palette select)
    ========================================================================
    Interrupts:
        IRQ1 = VBLANK
        IRQ2 = sound CPU communications
    ========================================================================


    ========================================================================
    SOUND CPU (based on JSA II, but implemented onboard)
    ========================================================================
    0000-1FFF   R/W   xxxxxxxx   Program RAM
    2000-2001   R/W   xxxxxxxx   YM2151 communications
    2802        R     xxxxxxxx   Sound command read
    2804        R     xxxx--xx   Status input
                R     x-------      (Self test)
                R     -x------      (Sound command buffer full)
                R     --x-----      (Sound response buffer full)
                R     ---x----      (Self test)
                R     ------xx      (Coin inputs)
    2806        R/W   --------   IRQ acknowledge
    2A02          W   xxxxxxxx   Sound response write
    2A04          W   xxxx---x   Sound control
                  W   xx------      (ROM bank select)
                  W   --xx----      (Coin counters)
                  W   -------x      (YM2151 reset)
    3000-3FFF   R     xxxxxxxx   Banked ROM
    4000-FFFF   R     xxxxxxxx   Program ROM
    ========================================================================
    Interrupts:
        IRQ = timed interrupt ORed with YM2151 interrupt
        NMI = latch on sound command
    ========================================================================


Badlands
Atari Games, 1989

PCB Layout
----------

A047193-01 BADLANDS
|-------------------------------------------------|
|TDA2030 LM324                ROM.2N ROM.2S ROM.2U|
|VOL     LM324                ROM.4N ROM.4S ROM.4U|
|6264    YM3012                                   |
|      YM2151          6116     PAL3              |
|   ROM.9C   PAL2                 ROM.10R  10T    |
|J    6502                        ROM.14R  ROM.14T|
|A                            MB87316             |
|M                     14.31818MHz    PAL4  VMA   |
|M                                                |
|A    VGC7205      ROM.17F          PAL5          |
|  TEST_SW   68000 ROM.20F      SOS               |
| NE556 PAL1       ROM.24F          PAL6     6264 |
|LED    DIP20      ROM.27F 2816                   |
|-------------------------------------------------|
Notes:
      68000  - clock 7.15909MHz [14.31818/2]
      6502   - clock 1.7897725 [14.31818/8]
      YM2151 - clock 3.579545MHz [14.31818/4]
      VGC7205- VLSI VGC7205A0672 ATARI 137304-2002
      2816   - 2k x8 EEPROM
      DIP20  - socket not populated
      SOS    - Motorola (rebadged) SOS-137550-001 ZKZKAA8946
      VMA    - Motorola (rebadged) VMA-137552-001 ZKZKAB8948
      MB87316- Fujitsu MB87316 137536-001 with clock input 7.15909MHz
               Listed in Jed Margolin's Atari Approved Vendor List
               (http://www.jmargolin.com/schem/atariavl.htm) as.....
               137536-001 IC,Line_Buffer(LB),CMOS,672B_RAM,500_Gate,40pin FUJITSU MB87316

      ROMs   -
               location  label
               ------------------------
               9C        136074-1018 E1FF
               17F       136074-1009 86DF
               20F       136074-1008 87FF
               24F       136074-1007 F2DE
               27F       136074-1006 98FE
               2N        136074-1013 A5A0
               2S        136074-1014 567B
               2U        136074-1017 BB88
               4N        136074-1012 6B70
               4S        136074-1014 4475
               4U        136074-1016 42A5
               10R       136074-1011 041C
               10T       not populated
               14R       136074-1010 5E67
               14T       136074-1019 D4C4

      PALs   -
               PAL1 - Lattice GAL16V8A-25LP labelled '136074-1001'
               PAL2 - Lattice GAL16V8A-25LP labelled '136074-1005'
               PAL3 - Lattice GAL16V8A-25LP labelled '136074-1004'
               PAL4 - Lattice GAL16V8A-25LP labelled '136074-1003'
               PAL5 - Lattice GAL16V8A-25LP labelled '136074-1002'
               PAL6 - Lattice GAL16V8A-25LP labelled '136074-2000'

Measurements -
              X1    - 14.31995MHz
              VSync - 59.9310Hz
              HSync - 15.4611kHz

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "sound/2151intf.h"
#include "includes/badlands.h"



/*************************************
 *
 *  Initialization
 *
 *************************************/

void badlands_state::update_interrupts()
{
	m_maincpu->set_input_line(1, m_video_int_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(2, m_sound_int_state ? ASSERT_LINE : CLEAR_LINE);
}


void badlands_state::scanline_update(screen_device &screen, int scanline)
{
	if (m_audiocpu != nullptr)
	{
		address_space &space = m_audiocpu->space(AS_PROGRAM);

		/* sound IRQ is on 32V */
		if (scanline & 32)
			m_soundcomm->sound_irq_ack_r(space, 0);
		else if (!(ioport("FE4000")->read() & 0x40))
			m_soundcomm->sound_irq_gen(m_audiocpu);
	}
	else
		return;
}


MACHINE_START_MEMBER(badlands_state,badlands)
{
	atarigen_state::machine_start();

	save_item(NAME(m_pedal_value));
}


MACHINE_RESET_MEMBER(badlands_state,badlands)
{
	m_pedal_value[0] = m_pedal_value[1] = 0x80;

	atarigen_state::machine_reset();
	scanline_timer_reset(*m_screen, 32);

	memcpy(m_bank_base, &m_bank_source_data[0x0000], 0x1000);
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

INTERRUPT_GEN_MEMBER(badlands_state::vblank_int)
{
	int pedal_state = ioport("PEDALS")->read();
	int i;

	/* update the pedals once per frame */
	for (i = 0; i < 2; i++)
	{
		m_pedal_value[i]--;
		if (pedal_state & (1 << i))
			m_pedal_value[i]++;
	}

	video_int_gen(device);
}



/*************************************
 *
 *  I/O read dispatch
 *
 *************************************/

READ16_MEMBER(badlands_state::sound_busy_r)
{
	int temp = 0xfeff;
	if (m_soundcomm->main_to_sound_ready()) temp ^= 0x0100;
	return temp;
}


READ16_MEMBER(badlands_state::pedal_0_r)
{
	return m_pedal_value[0];
}


READ16_MEMBER(badlands_state::pedal_1_r)
{
	return m_pedal_value[1];
}



/*************************************
 *
 *  Audio I/O handlers
 *
 *************************************/

READ8_MEMBER(badlands_state::audio_io_r)
{
	int result = 0xff;

	switch (offset & 0x206)
	{
		case 0x000:     /* n/c */
			logerror("audio_io_r: Unknown read at %04X\n", offset & 0x206);
			break;

		case 0x002:     /* /RDP */
			result = m_soundcomm->sound_command_r(space, offset);
			break;

		case 0x004:     /* /RDIO */
			/*
			    0x80 = self test
			    0x40 = NMI line state (active low)
			    0x20 = sound output full
			    0x10 = self test
			    0x08 = +5V
			    0x04 = +5V
			    0x02 = coin 2
			    0x01 = coin 1
			*/
			result = ioport("AUDIO")->read();
			if (!(ioport("FE4000")->read() & 0x0080)) result ^= 0x90;
			result ^= 0x10;
			break;

		case 0x006:     /* /IRQACK */
			m_soundcomm->sound_irq_ack_r(space, 0);
			break;

		case 0x200:     /* /VOICE */
		case 0x202:     /* /WRP */
		case 0x204:     /* /WRIO */
		case 0x206:     /* /MIX */
			logerror("audio_io_r: Unknown read at %04X\n", offset & 0x206);
			break;
	}

	return result;
}


WRITE8_MEMBER(badlands_state::audio_io_w)
{
	switch (offset & 0x206)
	{
		case 0x000:     /* n/c */
		case 0x002:     /* /RDP */
		case 0x004:     /* /RDIO */
			logerror("audio_io_w: Unknown write (%02X) at %04X\n", data & 0xff, offset & 0x206);
			break;

		case 0x006:     /* /IRQACK */
			m_soundcomm->sound_irq_ack_r(space, 0);
			break;

		case 0x200:     /* n/c */
		case 0x206:     /* n/c */
			break;

		case 0x202:     /* /WRP */
			m_soundcomm->sound_response_w(space, offset, data);
			break;

		case 0x204:     /* WRIO */
			/*
			    0xc0 = bank address
			    0x20 = coin counter 2
			    0x10 = coin counter 1
			    0x08 = n/c
			    0x04 = n/c
			    0x02 = n/c
			    0x01 = YM2151 reset (active low)
			*/

			/* update the bank */
			memcpy(m_bank_base, &m_bank_source_data[0x1000 * ((data >> 6) & 3)], 0x1000);
			break;
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, badlands_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0xfc0000, 0xfc1fff) AM_READ(sound_busy_r) AM_DEVWRITE("soundcomm", atari_sound_comm_device, sound_reset_w)
	AM_RANGE(0xfd0000, 0xfd1fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0x00ff)
	AM_RANGE(0xfe0000, 0xfe1fff) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0xfe2000, 0xfe3fff) AM_WRITE(video_int_ack_w)
	AM_RANGE(0xfe4000, 0xfe5fff) AM_READ_PORT("FE4000")
	AM_RANGE(0xfe6000, 0xfe6001) AM_READ_PORT("FE6000")
	AM_RANGE(0xfe6002, 0xfe6003) AM_READ_PORT("FE6002")
	AM_RANGE(0xfe6004, 0xfe6005) AM_READ(pedal_0_r)
	AM_RANGE(0xfe6006, 0xfe6007) AM_READ(pedal_1_r)
	AM_RANGE(0xfe8000, 0xfe9fff) AM_DEVWRITE8("soundcomm", atari_sound_comm_device, main_command_w, 0xff00)
	AM_RANGE(0xfea000, 0xfebfff) AM_DEVREAD8("soundcomm", atari_sound_comm_device, main_response_r, 0xff00)
	AM_RANGE(0xfec000, 0xfedfff) AM_WRITE(badlands_pf_bank_w)
	AM_RANGE(0xfee000, 0xfeffff) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0xffc000, 0xffc3ff) AM_DEVREADWRITE8("palette", palette_device, read, write, 0xff00) AM_SHARE("palette")
	AM_RANGE(0xffe000, 0xffefff) AM_RAM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0xfff000, 0xfff1ff) AM_RAM AM_SHARE("mob")
	AM_RANGE(0xfff200, 0xffffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, badlands_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x2800, 0x2bff) AM_READWRITE(audio_io_r, audio_io_w)
	AM_RANGE(0x3000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( badlands )
	PORT_START("FE4000")    /* fe4000 */
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("FE6000")    /* fe6000 */
	PORT_BIT( 0x00ff, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("FE6002")    /* fe6002 */
	PORT_BIT( 0x00ff, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AUDIO")     /* audio port */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* self test */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_SOUND_TO_MAIN_READY("soundcomm")   /* response buffer full */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")    /* command buffer full */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* self test */

	PORT_START("PEDALS")    /* fake for pedals */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xfffc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout pflayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8
};


static const gfx_layout molayout =
{
	16,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60 },
	{ 0*8, 8*8, 16*8, 24*8, 32*8, 40*8, 48*8, 56*8 },
	64*8
};


static GFXDECODE_START( badlands )
	GFXDECODE_ENTRY( "gfx1", 0, pflayout,    0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, molayout,  128, 8 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( badlands, badlands_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", badlands_state,  vblank_int)

	MCFG_CPU_ADD("audiocpu", M6502, ATARI_CLOCK_14MHz/8)
	MCFG_CPU_PROGRAM_MAP(audio_map)

	MCFG_MACHINE_START_OVERRIDE(badlands_state,badlands)
	MCFG_MACHINE_RESET_OVERRIDE(badlands_state,badlands)

	MCFG_ATARI_EEPROM_2816_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", badlands)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)
	MCFG_PALETTE_MEMBITS(8)

	MCFG_TILEMAP_ADD_STANDARD("playfield", "gfxdecode", 2, badlands_state, get_playfield_tile_info, 8,8, SCAN_ROWS, 64,32)
	MCFG_ATARI_MOTION_OBJECTS_ADD("mob", "screen", badlands_state::s_mob_config)
	MCFG_ATARI_MOTION_OBJECTS_GFXDECODE("gfxdecode")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses an SOS-2 chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(badlands_state, screen_update_badlands)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(badlands_state,badlands)

	/* sound hardware */
	MCFG_ATARI_SOUND_COMM_ADD("soundcomm", "audiocpu", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", ATARI_CLOCK_14MHz/4)
	MCFG_SOUND_ROUTE(0, "mono", 0.30)
	MCFG_SOUND_ROUTE(1, "mono", 0.30)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( badlands )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136074-1008.20f",  0x00000, 0x10000, CRC(a3da5774) SHA1(5ab1eb61d25594b2d7c40400cb57e7f47a717598) )
	ROM_LOAD16_BYTE( "136074-1006.27f",  0x00001, 0x10000, CRC(aa03b4f3) SHA1(5eda60c715ffcefd4ad34bdb90579e8671dc384a) )
	ROM_LOAD16_BYTE( "136074-1009.17f",  0x20000, 0x10000, CRC(0e2e807f) SHA1(5b61de066dca12c44335aa68a13c821845657866) )
	ROM_LOAD16_BYTE( "136074-1007.24f",  0x20001, 0x10000, CRC(99a20c2c) SHA1(9b0a5a5dafb8816e72330d302c60339b600b49a8) )

	ROM_REGION( 0x14000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136074-1018.9c", 0x10000, 0x4000, CRC(a05fd146) SHA1(d97abbcf7897ca720cc18ff3a323f41cd3b23c34) )
	ROM_CONTINUE(               0x04000, 0xc000 )

	ROM_REGION( 0x60000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136074-1012.4n",  0x000000, 0x10000, CRC(5d124c6c) SHA1(afebaaf90b3751f5e873fc4c45f1d5385ef86a6e) )  /* playfield */
	ROM_LOAD( "136074-1013.2n",  0x010000, 0x10000, CRC(b1ec90d6) SHA1(8d4c7db8e1bf9c050f5869eb38fa573867fdc12b) )
	ROM_LOAD( "136074-1014.4s",  0x020000, 0x10000, CRC(248a6845) SHA1(086ef0840b889e790ce3fcd09f98589aae932456) )
	ROM_LOAD( "136074-1015.2s",  0x030000, 0x10000, CRC(792296d8) SHA1(833cdb968064151ca77bb3dbe416ff7127a12de4) )
	ROM_LOAD( "136074-1016.4u",  0x040000, 0x10000, CRC(878f7c66) SHA1(31159bea5d6aac8100fca8f3860220b97d63e72e) )
	ROM_LOAD( "136074-1017.2u",  0x050000, 0x10000, CRC(ad0071a3) SHA1(472b197e5d320b3424d8a8d8c051b1023a07ae08) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "136074-1010.14r", 0x000000, 0x10000, CRC(c15f629e) SHA1(944e3479dce6e420cf9a3f4c1438c5ca66e5cb97) )  /* mo */
	ROM_LOAD( "136074-1011.10r", 0x010000, 0x10000, CRC(fb0b6717) SHA1(694ab0f04d673682831a24027757d4b3c40a4e0e) )
	ROM_LOAD( "136074-1019.14t", 0x020000, 0x10000, CRC(0e26bff6) SHA1(ee018dd37a27c7e7c16a57ea0d32aeb9cdf26bb4) )

	ROM_REGION( 0x0C00, "plds", 0 ) /* GAL16V8A-25LP */
	ROM_LOAD( "136074-1001.26c",  0x0000, 0x0117, CRC(04c3be6a) SHA1(f027834e652f3ff778b09c3754294b303f9ed826) )
	ROM_LOAD( "136074-1002.21r",  0x0200, 0x0117, CRC(f68bf41d) SHA1(72edd6d0f5d55d39c0020f384149de7ac964f273) )
	ROM_LOAD( "136074-1003.16s",  0x0400, 0x0117, CRC(a288bbd0) SHA1(62f5900ac88ffb335257f58d892492f370805498) )
	ROM_LOAD( "136074-1004.9n",   0x0600, 0x0117, CRC(5ffbdaad) SHA1(f7f802dfb7c9b404305a36b8354f91151e61c502) )
	ROM_LOAD( "136074-1005.12e",  0x0800, 0x0117, CRC(9df77c79) SHA1(52c1c190b80db9b9bc43ce6eefd5f37ac16e590c) )
	ROM_LOAD( "136074-2000.26r",  0x0A00, 0x0117, CRC(fb8fb3d0) SHA1(361b8f7984695ff26156afe79eaa2d85a150a978) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(badlands_state,badlands)
{
	/* initialize the audio system */
	m_bank_base = &memregion("audiocpu")->base()[0x03000];
	m_bank_source_data = &memregion("audiocpu")->base()[0x10000];
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1989, badlands, 0, badlands, badlands, badlands_state, badlands, ROT0, "Atari Games", "Bad Lands", 0 )

/* Badlands - Playmark Bootleg support - split this into its own file?

 Year: 1989
 Producer: Playmark

 cpu: 68000
 sound cpu:  Z80
 sound ics: YM2151 + 3012

 other ics: 28c16 2kx8 eeprom.Used to store bookeeping,settings etc. like original pcb.

 Osc: 20 Mhz, 28 Mhz

 ROMs:

 blb21, blb22, blb27, blb28 main program
 blb26 sound program
 blb29 to blb40 graphics

 All eproms are 27c512

 Note

 This romset comes from a bootleg pcb produced by Playmark.This pcb was been modified to use as control standard joysticks instead of steering wheels.Game differences are: Copyright string removed.

*/

READ16_MEMBER(badlands_state::badlandsb_unk_r)
{
	return 0xffff;
}

static ADDRESS_MAP_START( bootleg_map, AS_PROGRAM, 16, badlands_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM


	AM_RANGE(0x400008, 0x400009) AM_READ(badlandsb_unk_r )
	AM_RANGE(0x4000fe, 0x4000ff) AM_READ(badlandsb_unk_r )

	AM_RANGE(0xfc0000, 0xfc0001) AM_READ(badlandsb_unk_r )

	AM_RANGE(0xfe4000, 0xfe4001) AM_READ(badlandsb_unk_r )
	AM_RANGE(0xfe4004, 0xfe4005) AM_READ(badlandsb_unk_r )
	AM_RANGE(0xfe4006, 0xfe4007) AM_READ(badlandsb_unk_r )


	AM_RANGE(0xfd0000, 0xfd1fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0x00ff)
	//AM_RANGE(0xfe0000, 0xfe1fff) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0xfe2000, 0xfe3fff) AM_WRITE(video_int_ack_w)

	AM_RANGE(0xfec000, 0xfedfff) AM_WRITE(badlands_pf_bank_w)
	AM_RANGE(0xfee000, 0xfeffff) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0xffc000, 0xffc3ff) AM_DEVREADWRITE8("palette", palette_device, read, write, 0xff00) AM_SHARE("palette")
	AM_RANGE(0xffe000, 0xffefff) AM_RAM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0xfff000, 0xfff1ff) AM_RAM AM_SHARE("mob")
	AM_RANGE(0xfff200, 0xffffff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( badlandsb )

	PORT_INCLUDE( badlands )

	PORT_MODIFY("AUDIO") /* audio port */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END


static const gfx_layout pflayout_bootleg =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( badlandsb )
	GFXDECODE_ENTRY( "gfx1", 0, pflayout_bootleg,    0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, molayout,  128, 8 )
GFXDECODE_END


MACHINE_RESET_MEMBER(badlands_state,badlandsb)
{
//  m_pedal_value[0] = m_pedal_value[1] = 0x80;

	atarigen_state::machine_reset();
	scanline_timer_reset(*m_screen, 32);

//  memcpy(m_bank_base, &m_bank_source_data[0x0000], 0x1000);
}

static MACHINE_CONFIG_START( badlandsb, badlands_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_28MHz/4)   /* Divisor estimated */
	MCFG_CPU_PROGRAM_MAP(bootleg_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", badlands_state,  vblank_int)

//  MCFG_CPU_ADD("audiocpu", Z80, XTAL_20MHz/12)    /* Divisor estimated */
//  MCFG_CPU_PROGRAM_MAP(bootleg_soundmap)

	MCFG_MACHINE_START_OVERRIDE(badlands_state,badlands)
	MCFG_MACHINE_RESET_OVERRIDE(badlands_state,badlandsb)

	MCFG_ATARI_EEPROM_2816_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", badlandsb)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)
	MCFG_PALETTE_MEMBITS(8)

	MCFG_TILEMAP_ADD_STANDARD("playfield", "gfxdecode", 2, badlands_state, get_playfield_tile_info, 8,8, SCAN_ROWS, 64,32)
	MCFG_ATARI_MOTION_OBJECTS_ADD("mob", "screen", badlands_state::s_mob_config)
	MCFG_ATARI_MOTION_OBJECTS_GFXDECODE("gfxdecode")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses an SOS-2 chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(badlands_state, screen_update_badlands)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(badlands_state,badlands)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", XTAL_20MHz/8)  /* Divisor estimated */
	MCFG_SOUND_ROUTE(0, "mono", 0.30)
	MCFG_SOUND_ROUTE(1, "mono", 0.30)
MACHINE_CONFIG_END



/* bootleg by Playmark, uses Joystick controls */
ROM_START( badlandsb )
	/* bootleg 68k Program */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "blb28.ic21",  0x00000, 0x10000, CRC(dffb025d) SHA1(f2c17607acbbeee7d5d3f3dd2e8dc768b755e991) )
	ROM_LOAD16_BYTE( "blb22.ic22",  0x00001, 0x10000, CRC(ca3015c4) SHA1(72e1451498143d920239487704f4b4a8a71410e0) )
	ROM_LOAD16_BYTE( "blb27.ic19",  0x20000, 0x10000, CRC(0e2e807f) SHA1(5b61de066dca12c44335aa68a13c821845657866) )
	ROM_LOAD16_BYTE( "blb21.ic20",  0x20001, 0x10000, CRC(99a20c2c) SHA1(9b0a5a5dafb8816e72330d302c60339b600b49a8) )

	/* Z80 on the bootleg! */
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "blb26.ic27", 0x00000, 0x10000, CRC(59503ab4) SHA1(ea5686ee28f6125c1394d687cc35c6322c8f900c) )

	/* the 2nd half of 122,123,124 and 125 is identical to the first half and not used */
	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "13.ic123",     0x000000, 0x10000, CRC(55fac198) SHA1(055938f38cb7fc02ecaf35446b2598f7808baa1c) ) // alt set replaced bad rom
	ROM_LOAD( "blb37.ic92",   0x008000, 0x10000, CRC(9188db9f) SHA1(8f7dc2c4c0dec9a80b6214a2efaa0de0858de84c) )
	ROM_LOAD( "blb38.ic125",  0x020000, 0x10000, CRC(4839dd54) SHA1(031efbc144e5e088be0f3576aa514c7c2b775f6d) )
	ROM_LOAD( "8.ic91",       0x028000, 0x10000, CRC(4d85a509) SHA1(f8f13fe16706d01f0ee87cd1188c3115d6cdbf46) ) // alt set replaced bad rom
	ROM_LOAD( "blb30.ic122",  0x040000, 0x10000, CRC(61a1bcec) SHA1(fd38bbf8f6c8d1e0e936740db757f9fa85753503) )
	ROM_LOAD( "blb35.ic90",   0x048000, 0x10000, CRC(649c17f0) SHA1(bed3b7fc2c0516fe309bb81b65d8925ecf3065e4) )
	ROM_LOAD( "blb32.ic124",  0x060000, 0x10000, CRC(a67c61ba) SHA1(d701eb7f4520b57be54a7113d39f81d52800ee7e) )
	ROM_LOAD( "blb29.ic88",   0x068000, 0x10000, CRC(a9f280e5) SHA1(daff021d14f17da8c4469270a1e50e5a01d05d49) )

	/* the 1st half of 67 & 68 are empty and not used */
	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "blb33.ic67", 0x10001, 0x10000, CRC(aebf9938) SHA1(3778aacbde07e5a5d010e41ab62d5b0db8632ad8) )
	ROM_LOAD16_BYTE( "blb34.ic34", 0x00001, 0x10000, CRC(3eac30a5) SHA1(deefc668185bf30ad3eeba73853f97ce12b85293) )
	ROM_LOAD16_BYTE( "blb39.ic68", 0x10000, 0x10000, CRC(f398f2d7) SHA1(1eef64680101888425490eb4d5b86072e59753cf) )
	ROM_LOAD16_BYTE( "blb40.ic35", 0x00000, 0x10000, CRC(b47679ee) SHA1(0bd7d40dad214c54021c2014efbd374a7e4c7a3f) )
ROM_END


ROM_START( badlandsb2 )
	/* bootleg 68k Program */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "5.ic21",  0x00000, 0x10000, CRC(dffb025d) SHA1(f2c17607acbbeee7d5d3f3dd2e8dc768b755e991) )
	ROM_LOAD16_BYTE( "2.ic22",  0x00001, 0x10000, CRC(ca3015c4) SHA1(72e1451498143d920239487704f4b4a8a71410e0) )
	ROM_LOAD16_BYTE( "4.ic19",  0x20000, 0x10000, CRC(0e2e807f) SHA1(5b61de066dca12c44335aa68a13c821845657866) )
	ROM_LOAD16_BYTE( "1.ic20",  0x20001, 0x10000, CRC(99a20c2c) SHA1(9b0a5a5dafb8816e72330d302c60339b600b49a8) )

	/* Z80 on the bootleg! */
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "3.ic27", 0x00000, 0x10000, CRC(08850eb5) SHA1(be169e8ccee275b72bcfca66cd126cc27af7a1d6) )  // only rom that differs from badlandsb

	/* the 2nd half of 122,123,124 and 125 is identical to the first half and not used */
	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "13.ic123",  0x000000, 0x10000, CRC(55fac198) SHA1(055938f38cb7fc02ecaf35446b2598f7808baa1c) )
	ROM_LOAD( "14.ic92",   0x008000, 0x10000, CRC(9188db9f) SHA1(8f7dc2c4c0dec9a80b6214a2efaa0de0858de84c) )
	ROM_LOAD( "15.ic125",  0x020000, 0x10000, CRC(4839dd54) SHA1(031efbc144e5e088be0f3576aa514c7c2b775f6d) )
	ROM_LOAD( "8.ic91",    0x028000, 0x10000, CRC(4d85a509) SHA1(f8f13fe16706d01f0ee87cd1188c3115d6cdbf46) )
	ROM_LOAD( "7.ic122",   0x040000, 0x10000, CRC(61a1bcec) SHA1(fd38bbf8f6c8d1e0e936740db757f9fa85753503) )
	ROM_LOAD( "12.ic90",   0x048000, 0x10000, CRC(649c17f0) SHA1(bed3b7fc2c0516fe309bb81b65d8925ecf3065e4) )
	ROM_LOAD( "9.ic124",   0x060000, 0x10000, CRC(a67c61ba) SHA1(d701eb7f4520b57be54a7113d39f81d52800ee7e) )
	ROM_LOAD( "6.ic88",    0x068000, 0x10000, CRC(a9f280e5) SHA1(daff021d14f17da8c4469270a1e50e5a01d05d49) )

	/* the 1st half of 67 & 68 are empty and not used */
	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "10.ic67", 0x10001, 0x10000, CRC(aebf9938) SHA1(3778aacbde07e5a5d010e41ab62d5b0db8632ad8) )
	ROM_LOAD16_BYTE( "11.ic34", 0x00001, 0x10000, CRC(3eac30a5) SHA1(deefc668185bf30ad3eeba73853f97ce12b85293) )
	ROM_LOAD16_BYTE( "16.ic68", 0x10000, 0x10000, CRC(f398f2d7) SHA1(1eef64680101888425490eb4d5b86072e59753cf) )
	ROM_LOAD16_BYTE( "17.ic35", 0x00000, 0x10000, CRC(b47679ee) SHA1(0bd7d40dad214c54021c2014efbd374a7e4c7a3f) )
ROM_END



GAME( 1989, badlandsb, badlands, badlandsb, badlandsb, driver_device, 0, ROT0, "bootleg (Playmark)", "Bad Lands (bootleg)", MACHINE_NOT_WORKING )
GAME( 1989, badlandsb2,badlands, badlandsb, badlandsb, driver_device, 0, ROT0, "bootleg (Playmark)", "Bad Lands (bootleg, alternate)", MACHINE_NOT_WORKING )
