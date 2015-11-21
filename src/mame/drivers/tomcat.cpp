// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*

    Atari Tomcat prototype hardware

    Driver by Mariusz Wojcieszek

    Notes:
    - game has no sound, while sound hardware was developed, sound program was
      not prepared

    TODO:
    - add proper timing of interrupts and framerate (currently commented out,
      as they cause test mode to hang)
    - vector quality appears to be worse than original game (compared to original
      game videos)
    - verify controls
    - implement game linking (after MAME supports network)
    - current implementation of 68010 <-> tms32010 is a little bit hacky, after
      tms32010 is started by 68010, 68010 is suspended until tms32010 reads command
      and starts executing
    - hook up tms5220 - it is currently not used at all

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "cpu/m6502/m6502.h"
#include "video/vector.h"
#include "video/avgdvg.h"
#include "machine/timekpr.h"
#include "machine/nvram.h"
#include "machine/6532riot.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"
#include "sound/2151intf.h"



class tomcat_state : public driver_device
{
public:
	tomcat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tms(*this, "tms"),
		m_shared_ram(*this, "shared_ram"),
		m_maincpu(*this, "maincpu"),
		m_dsp(*this, "dsp") { }

	required_device<tms5220_device> m_tms;
	int m_control_num;
	required_shared_ptr<UINT16> m_shared_ram;
	UINT8 m_nvram[0x800];
	int m_dsp_BIO;
	int m_dsp_idle;
	DECLARE_WRITE16_MEMBER(tomcat_adcon_w);
	DECLARE_READ16_MEMBER(tomcat_adcread_r);
	DECLARE_READ16_MEMBER(tomcat_inputs_r);
	DECLARE_WRITE16_MEMBER(tomcat_led1on_w);
	DECLARE_WRITE16_MEMBER(tomcat_led2on_w);
	DECLARE_WRITE16_MEMBER(tomcat_led2off_w);
	DECLARE_WRITE16_MEMBER(tomcat_led1off_w);
	DECLARE_WRITE16_MEMBER(tomcat_lnkmodel_w);
	DECLARE_WRITE16_MEMBER(tomcat_errl_w);
	DECLARE_WRITE16_MEMBER(tomcat_errh_w);
	DECLARE_WRITE16_MEMBER(tomcat_ackl_w);
	DECLARE_WRITE16_MEMBER(tomcat_ackh_w);
	DECLARE_WRITE16_MEMBER(tomcat_lnkmodeh_w);
	DECLARE_WRITE16_MEMBER(tomcat_txbuffl_w);
	DECLARE_WRITE16_MEMBER(tomcat_txbuffh_w);
	DECLARE_WRITE16_MEMBER(tomcat_sndresl_w);
	DECLARE_WRITE16_MEMBER(tomcat_sndresh_w);
	DECLARE_WRITE16_MEMBER(tomcat_mresl_w);
	DECLARE_WRITE16_MEMBER(tomcat_mresh_w);
	DECLARE_WRITE16_MEMBER(tomcat_irqclr_w);
	DECLARE_READ16_MEMBER(tomcat_inputs2_r);
	DECLARE_READ16_MEMBER(tomcat_320bio_r);
	DECLARE_READ16_MEMBER(dsp_BIO_r);
	DECLARE_READ16_MEMBER(tomcat_shared_ram_r);
	DECLARE_WRITE16_MEMBER(tomcat_shared_ram_w);
	DECLARE_READ8_MEMBER(tomcat_nvram_r);
	DECLARE_WRITE8_MEMBER(tomcat_nvram_w);
	DECLARE_WRITE8_MEMBER(soundlatches_w);
	virtual void machine_start();
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_dsp;
};



WRITE16_MEMBER(tomcat_state::tomcat_adcon_w)
{
	m_control_num = data;
}

READ16_MEMBER(tomcat_state::tomcat_adcread_r)
{
	switch( m_control_num )
	{
	case 0: return ioport("STICKY")->read();
	case 1: return ioport("STICKX")->read();
	default: return 0x7f7f;
	}
}

READ16_MEMBER(tomcat_state::tomcat_inputs_r)
{
	UINT16 result = 0;
	if (ACCESSING_BITS_8_15)
		result |= ioport("IN0")->read() << 8;

	return result;
}

WRITE16_MEMBER(tomcat_state::tomcat_led1on_w)
{
	set_led_status(machine(), 1, 1);
}

WRITE16_MEMBER(tomcat_state::tomcat_led2on_w)
{
	set_led_status(machine(), 2, 1);
}

WRITE16_MEMBER(tomcat_state::tomcat_led2off_w)
{
	set_led_status(machine(), 2, 0);
}

WRITE16_MEMBER(tomcat_state::tomcat_led1off_w)
{
	set_led_status(machine(), 1, 0);
}

WRITE16_MEMBER(tomcat_state::tomcat_lnkmodel_w)
{
	// Link Mode Low (address strobe)
	// Master does not respond to Interrupts
}

WRITE16_MEMBER(tomcat_state::tomcat_errl_w)
{
	// Link Error Flag Low (address strobe)
}

WRITE16_MEMBER(tomcat_state::tomcat_errh_w)
{
	// Link Error Flag High (address strobe)
}

WRITE16_MEMBER(tomcat_state::tomcat_ackl_w)
{
	// Link ACK Flag Low (address strobe)
}

WRITE16_MEMBER(tomcat_state::tomcat_ackh_w)
{
	// Link ACK Flag High (address strobe)
}

WRITE16_MEMBER(tomcat_state::tomcat_lnkmodeh_w)
{
	// Link Mode high (address strobe)
}

WRITE16_MEMBER(tomcat_state::tomcat_txbuffl_w)
{
	// Link Buffer Control (address strobe)
}

WRITE16_MEMBER(tomcat_state::tomcat_txbuffh_w)
{
	// Link Buffer Control high (address strobe)
	// Turn off TX (Link) Buffer
}

WRITE16_MEMBER(tomcat_state::tomcat_sndresl_w)
{
	// Sound Reset Low       (Address Strobe)
	// Reset Sound System
}

WRITE16_MEMBER(tomcat_state::tomcat_sndresh_w)
{
	// Sound Reset high      (Address Strobe)
	// Release reset of sound system
}

WRITE16_MEMBER(tomcat_state::tomcat_mresl_w)
{
	// 320 Reset Low         (Address Strobe)
	// Reset TMS320
	m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

WRITE16_MEMBER(tomcat_state::tomcat_mresh_w)
{
	// 320 Reset high        (Address Strobe)
	// Release reset of TMS320
	m_dsp_BIO = 0;
	m_dsp->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

WRITE16_MEMBER(tomcat_state::tomcat_irqclr_w)
{
	// Clear IRQ Latch          (Address Strobe)
	m_maincpu->set_input_line(1, CLEAR_LINE);
}

READ16_MEMBER(tomcat_state::tomcat_inputs2_r)
{
/*
*       D15 LNKFLAG     (Game Link)
*       D14 PC3        "    "
*       D13 PC2        "    "
*       D12 PC0        "    "
*       D11 MAINFLAG    (Sound System)
*       D10 SOUNDFLAG      "    "
*       D9  /IDLE*      (TMS320 System)
*       D8
*/
	return m_dsp_idle ? 0 : (1 << 9);
}

READ16_MEMBER(tomcat_state::tomcat_320bio_r)
{
	m_dsp_BIO = 1;
	m_maincpu->suspend(SUSPEND_REASON_SPIN, 1);
	return 0;
}

READ16_MEMBER(tomcat_state::dsp_BIO_r)
{
	if ( space.device().safe_pc() == 0x0001 )
	{
		if ( m_dsp_idle == 0 )
		{
			m_dsp_idle = 1;
			m_dsp_BIO = 0;
		}
		return !m_dsp_BIO;
	}
	else if ( space.device().safe_pc() == 0x0003 )
	{
		if ( m_dsp_BIO == 1 )
		{
			m_dsp_idle = 0;
			m_dsp_BIO = 0;
			m_maincpu->resume(SUSPEND_REASON_SPIN );
			return 0;
		}
		else
		{
			assert(0);
			return 0;
		}
	}
	else
	{
		return !m_dsp_BIO;
	}
}

READ16_MEMBER(tomcat_state::tomcat_shared_ram_r)
{
	return m_shared_ram[offset];
}

WRITE16_MEMBER(tomcat_state::tomcat_shared_ram_w)
{
	COMBINE_DATA(&m_shared_ram[offset]);
}

READ8_MEMBER(tomcat_state::tomcat_nvram_r)
{
	return m_nvram[offset];
}

WRITE8_MEMBER(tomcat_state::tomcat_nvram_w)
{
	m_nvram[offset] = data;
}

static ADDRESS_MAP_START( tomcat_map, AS_PROGRAM, 16, tomcat_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x402000, 0x402001) AM_READ(tomcat_adcread_r) AM_WRITE(tomcat_adcon_w)
	AM_RANGE(0x404000, 0x404001) AM_READ(tomcat_inputs_r) AM_DEVWRITE("avg", avg_tomcat_device, go_word_w)
	AM_RANGE(0x406000, 0x406001) AM_DEVWRITE("avg", avg_tomcat_device, reset_word_w)
	AM_RANGE(0x408000, 0x408001) AM_READWRITE(tomcat_inputs2_r, watchdog_reset16_w)
	AM_RANGE(0x40a000, 0x40a001) AM_READWRITE(tomcat_320bio_r, tomcat_irqclr_w)
	AM_RANGE(0x40e000, 0x40e001) AM_WRITE(tomcat_led1on_w)
	AM_RANGE(0x40e002, 0x40e003) AM_WRITE(tomcat_led2on_w)
	AM_RANGE(0x40e004, 0x40e005) AM_WRITE(tomcat_mresl_w)
	AM_RANGE(0x40e006, 0x40e007) AM_WRITE(tomcat_sndresl_w)
	AM_RANGE(0x40e008, 0x40e009) AM_WRITE(tomcat_lnkmodel_w)
	AM_RANGE(0x40e00a, 0x40e00b) AM_WRITE(tomcat_errl_w)
	AM_RANGE(0x40e00c, 0x40e00d) AM_WRITE(tomcat_ackl_w)
	AM_RANGE(0x40e00e, 0x40e00f) AM_WRITE(tomcat_txbuffl_w)
	AM_RANGE(0x40e010, 0x40e011) AM_WRITE(tomcat_led1off_w)
	AM_RANGE(0x40e012, 0x40e013) AM_WRITE(tomcat_led2off_w)
	AM_RANGE(0x40e014, 0x40e015) AM_WRITE(tomcat_mresh_w)
	AM_RANGE(0x40e016, 0x40e017) AM_WRITE(tomcat_sndresh_w)
	AM_RANGE(0x40e018, 0x40e019) AM_WRITE(tomcat_lnkmodeh_w)
	AM_RANGE(0x40e01a, 0x40e01b) AM_WRITE(tomcat_errh_w)
	AM_RANGE(0x40e01c, 0x40e01d) AM_WRITE(tomcat_ackh_w)
	AM_RANGE(0x40e01e, 0x40e01f) AM_WRITE(tomcat_txbuffh_w)
	AM_RANGE(0x800000, 0x803fff) AM_RAM AM_SHARE("vectorram")
	AM_RANGE(0xffa000, 0xffbfff) AM_READWRITE(tomcat_shared_ram_r, tomcat_shared_ram_w)
	AM_RANGE(0xffc000, 0xffcfff) AM_RAM
	AM_RANGE(0xffd000, 0xffdfff) AM_DEVREADWRITE8("m48t02", timekeeper_device, read, write, 0xff00)
	AM_RANGE(0xffd000, 0xffdfff) AM_READWRITE8(tomcat_nvram_r, tomcat_nvram_w, 0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp_map, AS_PROGRAM, 16, tomcat_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("shared_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp_io_map, AS_IO, 16, tomcat_state )
	AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(dsp_BIO_r)
ADDRESS_MAP_END

WRITE8_MEMBER(tomcat_state::soundlatches_w)
{
	switch(offset)
	{
		case 0x00: break; // XLOAD 0    Write the Sequential ROM counter Low Byte
		case 0x20: break; // XLOAD 1    Write the Sequential ROM counter High Byte
		case 0x40: break; // SOUNDWR    Write to Sound Interface Latch (read by Main)
		case 0x60: break; // unused
		case 0x80: break; // XREAD      Read the Sequential ROM (64K bytes) and increment the counter
		case 0xa0: break; // unused
		case 0xc0: break; // SOUNDREAD  Read the Sound Interface Latch (written by Main)
	}
}

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, tomcat_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x3000, 0x30df) AM_WRITE(soundlatches_w)
	AM_RANGE(0x30e0, 0x30e0) AM_NOP // COINRD Inputs: D7 = Coin L, D6 = Coin R, D5 = SOUNDFLAG
	AM_RANGE(0x5000, 0x507f) AM_RAM // 6532 ram
	AM_RANGE(0x5080, 0x509f) AM_DEVREADWRITE("riot", riot6532_device, read, write)
	AM_RANGE(0x6000, 0x601f) AM_DEVREADWRITE("pokey1", pokey_device, read, write)
	AM_RANGE(0x7000, 0x701f) AM_DEVREADWRITE("pokey2", pokey_device, read, write)
	AM_RANGE(0x8000, 0xffff) AM_NOP // main sound program rom
ADDRESS_MAP_END

static INPUT_PORTS_START( tomcat )
	PORT_START("IN0")   /* INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER("avg", avg_tomcat_device, done_r, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNUSED ) // SPARE
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_BUTTON5 ) // DIAGNOSTIC
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) // R FIRE
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 ) // L FIRE
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON4 ) // R THUMB
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON3 ) // L THUMB

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
INPUT_PORTS_END

void tomcat_state::machine_start()
{
	((UINT16*)m_shared_ram)[0x0000] = 0xf600;
	((UINT16*)m_shared_ram)[0x0001] = 0x0000;
	((UINT16*)m_shared_ram)[0x0002] = 0xf600;
	((UINT16*)m_shared_ram)[0x0003] = 0x0000;

	machine().device<nvram_device>("nvram")->set_base(m_nvram, 0x800);

	save_item(NAME(m_nvram));
	save_item(NAME(m_control_num));
	save_item(NAME(m_dsp_BIO));
	save_item(NAME(m_dsp_idle));

	m_dsp_BIO = 0;
}

static MACHINE_CONFIG_START( tomcat, tomcat_state )
	MCFG_CPU_ADD("maincpu", M68010, XTAL_12MHz / 2)
	MCFG_CPU_PROGRAM_MAP(tomcat_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(tomcat_state, irq1_line_assert,  5*60)
	//MCFG_CPU_PERIODIC_INT_DRIVER(tomcat_state, irq1_line_assert,  (double)XTAL_12MHz / 16 / 16 / 16 / 12)

	MCFG_CPU_ADD("dsp", TMS32010, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP( dsp_map)
	MCFG_CPU_IO_MAP( dsp_io_map)

	MCFG_CPU_ADD("soundcpu", M6502, XTAL_14_31818MHz / 8 )
	MCFG_DEVICE_DISABLE()
	MCFG_CPU_PROGRAM_MAP( sound_map)

	MCFG_DEVICE_ADD("riot", RIOT6532, XTAL_14_31818MHz / 8)
	/*
	 PA0 = /WS   OUTPUT  (TMS-5220 WRITE STROBE)
	 PA1 = /RS   OUTPUT  (TMS-5220 READ STROBE)
	 PA2 = /READY    INPUT   (TMS-5220 READY FLAG)
	 PA3 = FSEL  OUTPUT  Select TMS5220 clock;
	 0 = 325 KHz (8 KHz sampling)
	 1 = 398 KHz (10 KHz sampling)
	 PA4 = /CC1  OUTPUT  Coin Counter 1
	 PA5 = /CC2  OUTPUT  Coin Counter 2
	 PA6 = /MUSRES   OUTPUT  (Reset the Yamaha)
	 PA7 = MAINFLAG  INPUT
	 */
	// OUTB PB0 - PB7   OUTPUT  Speech Data
	// IRQ CB connected to IRQ line of 6502

	MCFG_QUANTUM_TIME(attotime::from_hz(4000))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_M48T02_ADD( "m48t02" )

	MCFG_VECTOR_ADD("vector")
	MCFG_SCREEN_ADD("screen", VECTOR)
	MCFG_SCREEN_REFRESH_RATE(40)
	//MCFG_SCREEN_REFRESH_RATE((double)XTAL_12MHz / 16 / 16 / 16 / 12  / 5 )
	MCFG_SCREEN_SIZE(400, 300)
	MCFG_SCREEN_VISIBLE_AREA(0, 280, 0, 250)
	MCFG_SCREEN_UPDATE_DEVICE("vector", vector_device, screen_update)

	MCFG_DEVICE_ADD("avg", AVG_TOMCAT, XTAL_12MHz)
	MCFG_AVGDVG_VECTOR("vector")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("pokey1", POKEY, XTAL_14_31818MHz / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.20)

	MCFG_SOUND_ADD("pokey2", POKEY, XTAL_14_31818MHz / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.20)

	MCFG_SOUND_ADD("tms", TMS5220, 325000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_YM2151_ADD("ymsnd", XTAL_14_31818MHz / 4)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.60)
MACHINE_CONFIG_END

ROM_START( tomcat )
	ROM_REGION( 0x10000, "maincpu", 0)
	ROM_LOAD16_BYTE( "rom1k.bin", 0x00001, 0x8000, CRC(5535a1ff) SHA1(b9807c749a8e6b5ddec3ff494130abda09f0baab) )
	ROM_LOAD16_BYTE( "rom2k.bin", 0x00000, 0x8000, CRC(021a01d2) SHA1(01d99aab54ad57a664e8aaa91296bb879fc6e422) )

	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136021-105.1l",   0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */
ROM_END

GAME( 1985, tomcat, 0, tomcat, tomcat, driver_device, 0, ROT0, "Atari", "TomCat (prototype)", MACHINE_SUPPORTS_SAVE )
