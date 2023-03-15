// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Hard Drivin' sound hardware

****************************************************************************/

#include "emu.h"
#include "harddriv.h"

#include "cpu/tms32010/tms32010.h"
#include "speaker.h"


#define BIO_FREQUENCY       (1000000 / 50)
#define CYCLES_PER_BIO      (5000000 / BIO_FREQUENCY)


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  harddriv_sound_board_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(HARDDRIV_SOUND_BOARD, harddriv_sound_board_device, "harddriv_sound", "Hard Drivin' Sound Board")

harddriv_sound_board_device::harddriv_sound_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HARDDRIV_SOUND_BOARD, tag, owner, clock),
	m_soundcpu(*this, "soundcpu"),
	m_latch(*this, "latch"),
	m_dac(*this, "dac"),
	m_sounddsp(*this, "sounddsp"),
	m_sounddsp_ram(*this, "sounddsp_ram"),
	m_sound_rom(*this, "serialroms"),
	m_soundflag(0),
	m_mainflag(0),
	m_sounddata(0),
	m_maindata(0),
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

uint16_t harddriv_sound_board_device::hd68k_snd_data_r()
{
	m_soundflag = 0;
	logerror("%s:main read from sound=%04X\n", machine().describe_context(), m_sounddata);
	return m_sounddata;
}


uint16_t harddriv_sound_board_device::hd68k_snd_status_r()
{
	return (m_mainflag << 15) | (m_soundflag << 14) | 0x1fff;
}


TIMER_CALLBACK_MEMBER( harddriv_sound_board_device::delayed_68k_w )
{
	m_maindata = param;
	m_mainflag = 1;
	update_68k_interrupts();
}


void harddriv_sound_board_device::hd68k_snd_data_w(uint16_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(harddriv_sound_board_device::delayed_68k_w), this), data);
	logerror("%s:main write to sound=%04X\n", machine().describe_context(), data);
}


void harddriv_sound_board_device::hd68k_snd_reset_w(uint16_t data)
{
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	m_mainflag = m_soundflag = 0;
	update_68k_interrupts();
	logerror("%s:Reset sound\n", machine().describe_context());
}



/*************************************
 *
 *  I/O from sound CPU side
 *
 *************************************/

uint16_t harddriv_sound_board_device::hdsnd68k_data_r()
{
	m_mainflag = 0;
	update_68k_interrupts();
	logerror("%s:sound read from main=%04X\n", machine().describe_context(), m_maindata);
	return m_maindata;
}


void harddriv_sound_board_device::hdsnd68k_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sounddata);
	m_soundflag = 1;
	logerror("%s:sound write to main=%04X\n", machine().describe_context(), data);
}



/*************************************
 *
 *  Misc. 68k inputs
 *
 *************************************/

uint16_t harddriv_sound_board_device::hdsnd68k_switches_r(offs_t offset)
{
	logerror("%s:hdsnd68k_switches_r(%04X)\n", machine().describe_context(), offset);
	return 0;
}


uint16_t harddriv_sound_board_device::hdsnd68k_320port_r(offs_t offset)
{
	logerror("%s:hdsnd68k_320port_r(%04X)\n", machine().describe_context(), offset);
	return 0;
}


uint16_t harddriv_sound_board_device::hdsnd68k_status_r()
{
//FFFF 3000 R   READSTAT    Read Status
//            D15 = 'Main Flag'
//            D14 = 'Sound Flag'
//            D13 = Test Switch
//            D12 = 5220 Ready Flag (0=Ready)
	//logerror("%s:hdsnd68k_status_r(%04X)\n", machine().describe_context(), offset);
	return (m_mainflag << 15) | (m_soundflag << 14) | 0x2000 | 0;//((ioport("IN0")->read() & 0x0020) << 8) | 0;
}



/*************************************
 *
 *  Misc. 68k outputs
 *
 *************************************/

void harddriv_sound_board_device::hdsnd68k_latches_w(offs_t offset, uint16_t data)
{
	// bit 3 selects the value; data is ignored
	// low 3 bits select the function
	m_latch->write_bit(offset & 7, (offset >> 3) & 1);
}


WRITE_LINE_MEMBER(harddriv_sound_board_device::speech_write_w)
{
	// data == 0 means high, 1 means low
	logerror("%06X:SPWR=%d\n", m_soundcpu->pcbase(), state);
}


WRITE_LINE_MEMBER(harddriv_sound_board_device::speech_reset_w)
{
	// data == 0 means low, 1 means high
	logerror("%06X:SPRES=%d\n", m_soundcpu->pcbase(), state);
}


WRITE_LINE_MEMBER(harddriv_sound_board_device::speech_rate_w)
{
	// data == 0 means 8kHz, 1 means 10kHz
	logerror("%06X:SPRATE=%d\n", m_soundcpu->pcbase(), state);
}


WRITE_LINE_MEMBER(harddriv_sound_board_device::cram_enable_w)
{
	// data == 0 means disable 68k access to COM320, 1 means enable
	m_cramen = state;
}


WRITE_LINE_MEMBER(harddriv_sound_board_device::led_w)
{
}


void harddriv_sound_board_device::hdsnd68k_speech_w(offs_t offset, uint16_t data)
{
	logerror("%s:hdsnd68k_speech_w(%04X)=%04X\n", machine().describe_context(), offset, data);
}


void harddriv_sound_board_device::hdsnd68k_irqclr_w(uint16_t data)
{
	m_irq68k = 0;
	update_68k_interrupts();
}



/*************************************
 *
 *  TMS32010 access
 *
 *************************************/

uint16_t harddriv_sound_board_device::hdsnd68k_320ram_r(offs_t offset)
{
	return m_sounddsp_ram[offset & 0xfff];
}


void harddriv_sound_board_device::hdsnd68k_320ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sounddsp_ram[offset & 0xfff]);
}


uint16_t harddriv_sound_board_device::hdsnd68k_320ports_r(offs_t offset)
{
	return m_sounddsp->space(AS_IO).read_word(offset & 7);
}


void harddriv_sound_board_device::hdsnd68k_320ports_w(offs_t offset, uint16_t data)
{
	m_sounddsp->space(AS_IO).write_word(offset & 7, data);
}


uint16_t harddriv_sound_board_device::hdsnd68k_320com_r(offs_t offset)
{
	if (m_cramen)
		return m_comram[offset & 0x1ff];

	logerror("%s:hdsnd68k_320com_r(%04X) -- not allowed\n", machine().describe_context(), offset);
	return 0xffff;
}


void harddriv_sound_board_device::hdsnd68k_320com_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_cramen)
		COMBINE_DATA(&m_comram[offset & 0x1ff]);
	else
		logerror("%s:hdsnd68k_320com_w(%04X)=%04X -- not allowed\n", machine().describe_context(), offset, data);
}



/*************************************
 *
 *  TMS32010 interrupts
 *
 *************************************/

READ_LINE_MEMBER(harddriv_sound_board_device::hdsnddsp_get_bio)
{
	uint64_t cycles_since_last_bio = m_sounddsp->total_cycles() - m_last_bio_cycles;
	int32_t cycles_until_bio = CYCLES_PER_BIO - cycles_since_last_bio;

	/* if we're not at the next BIO yet, advance us there */
	if (cycles_until_bio > 0)
	{
		m_sounddsp->adjust_icount(-cycles_until_bio);
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

void harddriv_sound_board_device::hdsnddsp_dac_w(uint16_t data)
{
	/* /DACL */
	m_dac->write((data >> 4) ^ 0x800); // schematics show d0-3 are ignored & the msb is inverted
}


void harddriv_sound_board_device::hdsnddsp_comport_w(uint16_t data)
{
	/* COM port TD0-7 */
	logerror("%s:hdsnddsp_comport_w=%d\n", machine().describe_context(), data);
}


void harddriv_sound_board_device::hdsnddsp_mute_w(uint16_t data)
{
	/* mute DAC audio, D0=1 */
	logerror("%s:mute DAC=%d\n", machine().describe_context(), data);
}


void harddriv_sound_board_device::hdsnddsp_gen68kirq_w(uint16_t data)
{
	/* generate 68k IRQ */
	m_irq68k = 1;
	update_68k_interrupts();
}


void harddriv_sound_board_device::hdsnddsp_soundaddr_w(offs_t offset, uint16_t data)
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


uint16_t harddriv_sound_board_device::hdsnddsp_rom_r()
{
	if (m_sound_rom_offs < m_sound_rom.length())
		return m_sound_rom[m_sound_rom_offs++] << 7;
	m_sound_rom_offs++;
	return 0;
}


uint16_t harddriv_sound_board_device::hdsnddsp_comram_r()
{
	return m_comram[m_sound_rom_offs++ & 0x1ff];
}


uint16_t harddriv_sound_board_device::hdsnddsp_compare_r(offs_t offset)
{
	logerror("%s:hdsnddsp_compare_r(%04X)\n", machine().describe_context(), offset);
	return 0;
}

void harddriv_sound_board_device::driversnd_68k_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x01ffff).rom();
	map(0xff0000, 0xff0fff).rw(FUNC(harddriv_sound_board_device::hdsnd68k_data_r), FUNC(harddriv_sound_board_device::hdsnd68k_data_w));
	map(0xff1000, 0xff1fff).rw(FUNC(harddriv_sound_board_device::hdsnd68k_switches_r), FUNC(harddriv_sound_board_device::hdsnd68k_latches_w));
	map(0xff2000, 0xff2fff).rw(FUNC(harddriv_sound_board_device::hdsnd68k_320port_r), FUNC(harddriv_sound_board_device::hdsnd68k_speech_w));
	map(0xff3000, 0xff3fff).rw(FUNC(harddriv_sound_board_device::hdsnd68k_status_r), FUNC(harddriv_sound_board_device::hdsnd68k_irqclr_w));
	map(0xff4000, 0xff5fff).rw(FUNC(harddriv_sound_board_device::hdsnd68k_320ram_r), FUNC(harddriv_sound_board_device::hdsnd68k_320ram_w));
	map(0xff6000, 0xff7fff).rw(FUNC(harddriv_sound_board_device::hdsnd68k_320ports_r), FUNC(harddriv_sound_board_device::hdsnd68k_320ports_w));
	map(0xff8000, 0xffbfff).rw(FUNC(harddriv_sound_board_device::hdsnd68k_320com_r), FUNC(harddriv_sound_board_device::hdsnd68k_320com_w));
	map(0xffc000, 0xffffff).ram();
}


void harddriv_sound_board_device::driversnd_dsp_program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0xfff).ram().share("sounddsp_ram");
}


/* $000 - 08F  TMS32010 Internal Data RAM in Data Address Space */

void harddriv_sound_board_device::driversnd_dsp_io_map(address_map &map)
{
	map(0, 0).r(FUNC(harddriv_sound_board_device::hdsnddsp_rom_r)).w(FUNC(harddriv_sound_board_device::hdsnddsp_dac_w));
	map(1, 1).r(FUNC(harddriv_sound_board_device::hdsnddsp_comram_r));
	map(2, 2).r(FUNC(harddriv_sound_board_device::hdsnddsp_compare_r));
	map(1, 2).nopw();
	map(3, 3).w(FUNC(harddriv_sound_board_device::hdsnddsp_comport_w));
	map(4, 4).w(FUNC(harddriv_sound_board_device::hdsnddsp_mute_w));
	map(5, 5).w(FUNC(harddriv_sound_board_device::hdsnddsp_gen68kirq_w));
	map(6, 7).w(FUNC(harddriv_sound_board_device::hdsnddsp_soundaddr_w));
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void harddriv_sound_board_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_soundcpu, 16_MHz_XTAL/2);
	m_soundcpu->set_addrmap(AS_PROGRAM, &harddriv_sound_board_device::driversnd_68k_map);

	LS259(config, m_latch, 0); // 80R
	m_latch->q_out_cb<0>().set(FUNC(harddriv_sound_board_device::speech_write_w)); // SPWR - 5220 write strobe
	m_latch->q_out_cb<1>().set(FUNC(harddriv_sound_board_device::speech_reset_w)); // SPRES - 5220 hard reset
	m_latch->q_out_cb<2>().set(FUNC(harddriv_sound_board_device::speech_rate_w)); // SPRATE
	m_latch->q_out_cb<3>().set(FUNC(harddriv_sound_board_device::cram_enable_w)); // CRAMEN
	m_latch->q_out_cb<4>().set_inputline(m_sounddsp, INPUT_LINE_HALT).invert(); // RES320
	m_latch->q_out_cb<7>().set(FUNC(harddriv_sound_board_device::led_w));

	TMS32010(config, m_sounddsp, XTAL(20'000'000));
	m_sounddsp->set_addrmap(AS_PROGRAM, &harddriv_sound_board_device::driversnd_dsp_program_map);
	/* Data Map is internal to the CPU */
	m_sounddsp->set_addrmap(AS_IO, &harddriv_sound_board_device::driversnd_dsp_io_map);
	m_sounddsp->bio().set(FUNC(harddriv_sound_board_device::hdsnddsp_get_bio));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	AM6012(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // ls374d.75e + ls374d.90e + am6012
}
