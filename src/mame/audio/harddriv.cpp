// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Hard Drivin' sound hardware

****************************************************************************/

#include "emu.h"
#include "cpu/tms32010/tms32010.h"
#include "sound/dac.h"
#include "machine/atarigen.h"
#include "includes/harddriv.h"


#define BIO_FREQUENCY       (1000000 / 50)
#define CYCLES_PER_BIO      (5000000 / BIO_FREQUENCY)


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  harddriv_sound_board_device - constructor
//-------------------------------------------------

const device_type HARDDRIV_SOUND_BOARD_DEVICE = &device_creator<harddriv_sound_board_device>;

harddriv_sound_board_device::harddriv_sound_board_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, HARDDRIV_SOUND_BOARD_DEVICE, "Hard Drivin' Sound Board", tag, owner, clock, "harddriv_sound", __FILE__),
	m_soundcpu(*this, "soundcpu"),
	m_dac(*this, "dac"),
	m_sounddsp(*this, "sounddsp"),
	m_sounddsp_ram(*this, "sounddsp_ram"),
	m_sound_rom(*this, "serialroms"),
	m_soundflag(0),
	m_mainflag(0),
	m_sounddata(0),
	m_maindata(0),
	m_dacmute(0),
	m_cramen(0),
	m_irq68k(0),
	m_sound_rom_offs(0),
	m_last_bio_cycles(0)
{
	memset(m_comram, 0 , sizeof(m_comram));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void harddriv_sound_board_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void harddriv_sound_board_device::device_reset()
{
	m_last_bio_cycles = 0;
	m_sounddsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

/*************************************
 *
 *  Update flags
 *
 *************************************/

void harddriv_sound_board_device::update_68k_interrupts()
{
	m_soundcpu->set_input_line(1, m_mainflag ? ASSERT_LINE : CLEAR_LINE);
	m_soundcpu->set_input_line(3, m_irq68k   ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  I/O from main CPU side
 *
 *************************************/

READ16_MEMBER(harddriv_sound_board_device::hd68k_snd_data_r)
{
	m_soundflag = 0;
	logerror("%06X:main read from sound=%04X\n", space.device().safe_pcbase(), m_sounddata);
	return m_sounddata;
}


READ16_MEMBER(harddriv_sound_board_device::hd68k_snd_status_r)
{
	return (m_mainflag << 15) | (m_soundflag << 14) | 0x1fff;
}


TIMER_CALLBACK_MEMBER( harddriv_sound_board_device::delayed_68k_w )
{
	m_maindata = param;
	m_mainflag = 1;
	update_68k_interrupts();
}


WRITE16_MEMBER(harddriv_sound_board_device::hd68k_snd_data_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(harddriv_sound_board_device::delayed_68k_w), this), data);
	logerror("%06X:main write to sound=%04X\n", space.device().safe_pcbase(), data);
}


WRITE16_MEMBER(harddriv_sound_board_device::hd68k_snd_reset_w)
{
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	m_mainflag = m_soundflag = 0;
	update_68k_interrupts();
	logerror("%06X:Reset sound\n", space.device().safe_pcbase());
}



/*************************************
 *
 *  I/O from sound CPU side
 *
 *************************************/

READ16_MEMBER(harddriv_sound_board_device::hdsnd68k_data_r)
{
	m_mainflag = 0;
	update_68k_interrupts();
	logerror("%06X:sound read from main=%04X\n", space.device().safe_pcbase(), m_maindata);
	return m_maindata;
}


WRITE16_MEMBER(harddriv_sound_board_device::hdsnd68k_data_w)
{
	COMBINE_DATA(&m_sounddata);
	m_soundflag = 1;
	logerror("%06X:sound write to main=%04X\n", space.device().safe_pcbase(), data);
}



/*************************************
 *
 *  Misc. 68k inputs
 *
 *************************************/

READ16_MEMBER(harddriv_sound_board_device::hdsnd68k_switches_r)
{
	logerror("%06X:hdsnd68k_switches_r(%04X)\n", space.device().safe_pcbase(), offset);
	return 0;
}


READ16_MEMBER(harddriv_sound_board_device::hdsnd68k_320port_r)
{
	logerror("%06X:hdsnd68k_320port_r(%04X)\n", space.device().safe_pcbase(), offset);
	return 0;
}


READ16_MEMBER(harddriv_sound_board_device::hdsnd68k_status_r)
{
//FFFF 3000 R   READSTAT    Read Status
//            D15 = 'Main Flag'
//            D14 = 'Sound Flag'
//            D13 = Test Switch
//            D12 = 5220 Ready Flag (0=Ready)
	logerror("%06X:hdsnd68k_status_r(%04X)\n", space.device().safe_pcbase(), offset);
	return (m_mainflag << 15) | (m_soundflag << 14) | 0x2000 | 0;//((ioport("IN0")->read() & 0x0020) << 8) | 0;
}



/*************************************
 *
 *  Misc. 68k outputs
 *
 *************************************/

WRITE16_MEMBER(harddriv_sound_board_device::hdsnd68k_latches_w)
{
	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 0: /* SPWR - 5220 write strobe */
			/* data == 0 means high, 1 means low */
			logerror("%06X:SPWR=%d\n", space.device().safe_pcbase(), data);
			break;

		case 1: /* SPRES - 5220 hard reset */
			/* data == 0 means low, 1 means high */
			logerror("%06X:SPRES=%d\n", space.device().safe_pcbase(), data);
			break;

		case 2: /* SPRATE */
			/* data == 0 means 8kHz, 1 means 10kHz */
			logerror("%06X:SPRATE=%d\n", space.device().safe_pcbase(), data);
			break;

		case 3: /* CRAMEN */
			/* data == 0 means disable 68k access to COM320, 1 means enable */
			m_cramen = data;
			break;

		case 4: /* RES320 */
			logerror("%06X:RES320=%d\n", space.device().safe_pcbase(), data);
			m_sounddsp->set_input_line(INPUT_LINE_HALT, data ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 7: /* LED */
			break;
	}
}


WRITE16_MEMBER(harddriv_sound_board_device::hdsnd68k_speech_w)
{
	logerror("%06X:hdsnd68k_speech_w(%04X)=%04X\n", space.device().safe_pcbase(), offset, data);
}


WRITE16_MEMBER(harddriv_sound_board_device::hdsnd68k_irqclr_w)
{
	m_irq68k = 0;
	update_68k_interrupts();
}



/*************************************
 *
 *  TMS32010 access
 *
 *************************************/

READ16_MEMBER(harddriv_sound_board_device::hdsnd68k_320ram_r)
{
	return m_sounddsp_ram[offset & 0xfff];
}


WRITE16_MEMBER(harddriv_sound_board_device::hdsnd68k_320ram_w)
{
	COMBINE_DATA(&m_sounddsp_ram[offset & 0xfff]);
}


READ16_MEMBER(harddriv_sound_board_device::hdsnd68k_320ports_r)
{
	return m_sounddsp->space(AS_IO).read_word((offset & 7) << 1);
}


WRITE16_MEMBER(harddriv_sound_board_device::hdsnd68k_320ports_w)
{
	m_sounddsp->space(AS_IO).write_word((offset & 7) << 1, data);
}


READ16_MEMBER(harddriv_sound_board_device::hdsnd68k_320com_r)
{
	if (m_cramen)
		return m_comram[offset & 0x1ff];

	logerror("%06X:hdsnd68k_320com_r(%04X) -- not allowed\n", space.device().safe_pcbase(), offset);
	return 0xffff;
}


WRITE16_MEMBER(harddriv_sound_board_device::hdsnd68k_320com_w)
{
	if (m_cramen)
		COMBINE_DATA(&m_comram[offset & 0x1ff]);
	else
		logerror("%06X:hdsnd68k_320com_w(%04X)=%04X -- not allowed\n", space.device().safe_pcbase(), offset, data);
}



/*************************************
 *
 *  TMS32010 interrupts
 *
 *************************************/

READ16_MEMBER(harddriv_sound_board_device::hdsnddsp_get_bio)
{
	UINT64 cycles_since_last_bio = m_sounddsp->total_cycles() - m_last_bio_cycles;
	INT32 cycles_until_bio = CYCLES_PER_BIO - cycles_since_last_bio;

	/* if we're not at the next BIO yet, advance us there */
	if (cycles_until_bio > 0)
	{
		space.device().execute().adjust_icount(-cycles_until_bio);
		m_last_bio_cycles += CYCLES_PER_BIO;
	}
	else
		m_last_bio_cycles = m_sounddsp->total_cycles();
	return ASSERT_LINE;
}



/*************************************
 *
 *  TMS32010 ports
 *
 *************************************/

WRITE16_MEMBER(harddriv_sound_board_device::hdsnddsp_dac_w)
{
	/* DAC L */
	if (!m_dacmute)
		m_dac->write_signed16(data ^ 0x8000);
}


WRITE16_MEMBER(harddriv_sound_board_device::hdsnddsp_comport_w)
{
	/* COM port TD0-7 */
	logerror("%06X:hdsnddsp_comport_w=%d\n", space.device().safe_pcbase(), data);
}


WRITE16_MEMBER(harddriv_sound_board_device::hdsnddsp_mute_w)
{
	/* mute DAC audio, D0=1 */
/*  m_dacmute = data & 1;     -- NOT STUFFED */
	logerror("%06X:mute DAC=%d\n", space.device().safe_pcbase(), data);
}


WRITE16_MEMBER(harddriv_sound_board_device::hdsnddsp_gen68kirq_w)
{
	/* generate 68k IRQ */
	m_irq68k = 1;
	update_68k_interrupts();
}


WRITE16_MEMBER(harddriv_sound_board_device::hdsnddsp_soundaddr_w)
{
	if (offset == 0)
	{
		/* select sound ROM block */
		m_sound_rom_offs = (m_sound_rom_offs & 0xffff) | ((data & 15) << 16);
	}
	else
	{
		/* sound memory address */
		m_sound_rom_offs = (m_sound_rom_offs & ~0xffff) | (data & 0xffff);
	}
}


READ16_MEMBER(harddriv_sound_board_device::hdsnddsp_rom_r)
{
	if (m_sound_rom_offs < m_sound_rom.length())
		return m_sound_rom[m_sound_rom_offs++] << 7;
	m_sound_rom_offs++;
	return 0;
}


READ16_MEMBER(harddriv_sound_board_device::hdsnddsp_comram_r)
{
	return m_comram[m_sound_rom_offs++ & 0x1ff];
}


READ16_MEMBER(harddriv_sound_board_device::hdsnddsp_compare_r)
{
	logerror("%06X:hdsnddsp_compare_r(%04X)\n", space.device().safe_pcbase(), offset);
	return 0;
}

static ADDRESS_MAP_START( driversnd_68k_map, AS_PROGRAM, 16, harddriv_sound_board_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0xff0000, 0xff0fff) AM_READWRITE(hdsnd68k_data_r, hdsnd68k_data_w)
	AM_RANGE(0xff1000, 0xff1fff) AM_READWRITE(hdsnd68k_switches_r, hdsnd68k_latches_w)
	AM_RANGE(0xff2000, 0xff2fff) AM_READWRITE(hdsnd68k_320port_r, hdsnd68k_speech_w)
	AM_RANGE(0xff3000, 0xff3fff) AM_READWRITE(hdsnd68k_status_r, hdsnd68k_irqclr_w)
	AM_RANGE(0xff4000, 0xff5fff) AM_READWRITE(hdsnd68k_320ram_r, hdsnd68k_320ram_w)
	AM_RANGE(0xff6000, 0xff7fff) AM_READWRITE(hdsnd68k_320ports_r, hdsnd68k_320ports_w)
	AM_RANGE(0xff8000, 0xffbfff) AM_READWRITE(hdsnd68k_320com_r, hdsnd68k_320com_w)
	AM_RANGE(0xffc000, 0xffffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( driversnd_dsp_program_map, AS_PROGRAM, 16, harddriv_sound_board_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000, 0xfff) AM_RAM AM_SHARE("sounddsp_ram")
ADDRESS_MAP_END


/* $000 - 08F  TMS32010 Internal Data RAM in Data Address Space */

static ADDRESS_MAP_START( driversnd_dsp_io_map, AS_IO, 16, harddriv_sound_board_device )
	AM_RANGE(0, 0) AM_READWRITE(hdsnddsp_rom_r, hdsnddsp_dac_w)
	AM_RANGE(1, 1) AM_READ(hdsnddsp_comram_r)
	AM_RANGE(2, 2) AM_READ(hdsnddsp_compare_r)
	AM_RANGE(1, 2) AM_WRITENOP
	AM_RANGE(3, 3) AM_WRITE(hdsnddsp_comport_w)
	AM_RANGE(4, 4) AM_WRITE(hdsnddsp_mute_w)
	AM_RANGE(5, 5) AM_WRITE(hdsnddsp_gen68kirq_w)
	AM_RANGE(6, 7) AM_WRITE(hdsnddsp_soundaddr_w)
	AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(hdsnddsp_get_bio)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( harddriv_snd )

	/* basic machine hardware */
	MCFG_CPU_ADD("soundcpu", M68000, XTAL_16MHz/2)
	MCFG_CPU_PROGRAM_MAP(driversnd_68k_map)

	MCFG_CPU_ADD("sounddsp", TMS32010, XTAL_20MHz)
	MCFG_CPU_PROGRAM_MAP(driversnd_dsp_program_map)
	/* Data Map is internal to the CPU */
	MCFG_CPU_IO_MAP(driversnd_dsp_io_map)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor harddriv_sound_board_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( harddriv_snd );
}
