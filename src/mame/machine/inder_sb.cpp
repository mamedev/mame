// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Inder / Dinamic Sound Board */


#include "emu.h"
#include "machine/inder_sb.h"



extern const device_type INDER_AUDIO = &device_creator<inder_sb_device>;


inder_sb_device::inder_sb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, INDER_AUDIO, "Inder 4xDAC Sound Board", tag, owner, clock, "indersb", __FILE__),
		device_mixer_interface(mconfig, *this, 2),
		m_audiocpu(*this, "audiocpu"),
		m_ctc(*this, "ctc"),
		m_dac0(*this, "dac0" ),
		m_dac1(*this, "dac1" ),
		m_dac2(*this, "dac2" ),
		m_dac3(*this, "dac3" )
{
}



// hacks for test purposes, these are installed over the program rom so we know when irqs are actually taken
READ8_MEMBER(inder_sb_device::megaphx_02cc_hack_r)  { /*logerror("%04x audicpu IRQ hack 0x02cc\n", machine().device("audiocpu")->safe_pc());*/  int bank = m_soundbank[0] & 7;  membank("snddata")->set_entry(bank); return memregion("audiocpu")->base()[0x02cc]; }
READ8_MEMBER(inder_sb_device::megaphx_02e6_hack_r)  { /*logerror("%04x audicpu IRQ hack 0x02e6\n", machine().device("audiocpu")->safe_pc());*/  int bank = m_soundbank[1] & 7;  membank("snddata")->set_entry(bank); return memregion("audiocpu")->base()[0x02e6]; }
READ8_MEMBER(inder_sb_device::megaphx_0309_hack_r)  { /*logerror("%04x audicpu IRQ hack 0x0309\n", machine().device("audiocpu")->safe_pc());*/  int bank = m_soundbank[2] & 7;  membank("snddata")->set_entry(bank); return memregion("audiocpu")->base()[0x0309]; }
READ8_MEMBER(inder_sb_device::megaphx_0323_hack_r)  { /*logerror("%04x audicpu IRQ hack 0x0323\n", machine().device("audiocpu")->safe_pc());*/  int bank = m_soundbank[3] & 7;  membank("snddata")->set_entry(bank); return memregion("audiocpu")->base()[0x0323]; }



READ16_MEMBER(inder_sb_device::megaphx_0x050002_r)
{
	space.machine().scheduler().synchronize();
//  int pc = machine().device("maincpu")->safe_pc();
	int ret = m_soundback;
	m_soundback = 0;
	//logerror("(%06x) megaphx_0x050002_r (from z80?) %04x\n", pc, mem_mask);
	return ret;
}

WRITE16_MEMBER(inder_sb_device::megaphx_0x050000_w)
{
//  int pc = machine().device("maincpu")->safe_pc();
	space.machine().scheduler().synchronize();

	//logerror("(%06x) megaphx_0x050000_w (to z80?) %04x %04x\n", pc, data, mem_mask);
	m_soundsent = 0xff;
	m_sounddata = data;

}

void inder_sb_device::install_sound_hacks(void)
{
	address_space &space = m_audiocpu->space(AS_PROGRAM);
	space.install_read_handler(0x02cc, 0x02cc, read8_delegate(FUNC(inder_sb_device::megaphx_02cc_hack_r), this));
	space.install_read_handler(0x02e6, 0x02e6, read8_delegate(FUNC(inder_sb_device::megaphx_02e6_hack_r), this));
	space.install_read_handler(0x0309, 0x0309, read8_delegate(FUNC(inder_sb_device::megaphx_0309_hack_r), this));
	space.install_read_handler(0x0323, 0x0323, read8_delegate(FUNC(inder_sb_device::megaphx_0323_hack_r), this));
}

void inder_sb_device::update_sound_irqs(void)
{
	if (m_soundirq) m_audiocpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	else m_audiocpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

WRITE_LINE_MEMBER(inder_sb_device::z80ctc_ch0)
{
//  int bank = m_soundbank[0] & 7;  membank("snddata")->set_entry(bank);
//  m_audiocpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
	if (state) m_soundirq |= 0x1;
	else m_soundirq &= ~0x1;

	update_sound_irqs();
}


WRITE_LINE_MEMBER(inder_sb_device::z80ctc_ch1)
{
//  int bank = m_soundbank[1] & 7;  membank("snddata")->set_entry(bank);
//  m_audiocpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
	if (state) m_soundirq |= 0x2;
	else m_soundirq &= ~0x2;

	update_sound_irqs();
}


WRITE_LINE_MEMBER(inder_sb_device::z80ctc_ch2)
{
//  int bank = m_soundbank[2] & 7;  membank("snddata")->set_entry(bank);
//  m_audiocpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
	if (state) m_soundirq |= 0x4;
	else m_soundirq &= ~0x4;

	update_sound_irqs();
}




WRITE_LINE_MEMBER(inder_sb_device::z80ctc_ch3)
{
//  int bank = m_soundbank[3] & 7;  membank("snddata")->set_entry(bank);
//  m_audiocpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
	if (state) m_soundirq |= 0x8;
	else m_soundirq &= ~0x8;

	update_sound_irqs();
}




static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, inder_sb_device )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("snddata")
ADDRESS_MAP_END

READ8_MEMBER(inder_sb_device::megaphx_sound_cmd_r)
{
	space.machine().scheduler().synchronize();
	return m_sounddata;
}

READ8_MEMBER(inder_sb_device::megaphx_sound_sent_r)
{
	space.machine().scheduler().synchronize();
	int ret = m_soundsent;
	m_soundsent = 0;
	return ret;
}

WRITE8_MEMBER(inder_sb_device::megaphx_sound_to_68k_w)
{
//  int pc = machine().device("audiocpu")->safe_pc();
	space.machine().scheduler().synchronize();
	//logerror("(%04x) megaphx_sound_to_68k_w (to 68k?) %02x\n", pc, data);

	m_soundback = data;
}

WRITE8_MEMBER(inder_sb_device::dac0_value_write)
{
//  printf("dac0_data_write %02x\n", data);
	m_dac0->write_unsigned8(data);
}

WRITE8_MEMBER(inder_sb_device::dac0_gain_write)
{
//  printf("dac0_gain_write %02x\n", data);
	dac_gain[0] = data;
}

WRITE8_MEMBER(inder_sb_device::dac1_value_write)
{
//  printf("dac1_data_write %02x\n", data);
	m_dac1->write_unsigned8(data);
}

WRITE8_MEMBER(inder_sb_device::dac1_gain_write)
{
//  printf("dac1_gain_write %02x\n", data);
	dac_gain[1] = data;
}

WRITE8_MEMBER(inder_sb_device::dac2_value_write)
{
//  printf("dac2_data_write %02x\n", data);
	m_dac2->write_unsigned8(data);
}

WRITE8_MEMBER(inder_sb_device::dac2_gain_write)
{
//  printf("dac2_gain_write %02x\n", data);
	dac_gain[2] = data;
}

WRITE8_MEMBER(inder_sb_device::dac3_value_write)
{
//  printf("dac3_data_write %02x\n", data);
	m_dac3->write_unsigned8(data);
}

WRITE8_MEMBER(inder_sb_device::dac3_gain_write)
{
//  printf("dac3_gain_write %02x\n", data);
	dac_gain[3] = data;
}

WRITE8_MEMBER(inder_sb_device::dac0_rombank_write)
{
	m_soundbank[0] = data;

//  printf("dac0_rombank_write %02x", data);
}

WRITE8_MEMBER(inder_sb_device::dac1_rombank_write)
{
	m_soundbank[1] = data;
//  printf("dac1_rombank_write %02x", data);

}

WRITE8_MEMBER(inder_sb_device::dac2_rombank_write)
{
	m_soundbank[2] = data;
//  printf("dac2_rombank_write %02x", data);
}

WRITE8_MEMBER(inder_sb_device::dac3_rombank_write)
{
	m_soundbank[3] = data;
//  printf("dac3_rombank_write %02x", data);

}


static ADDRESS_MAP_START( sound_io, AS_IO, 8, inder_sb_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(dac0_value_write)
	AM_RANGE(0x01, 0x01) AM_WRITE(dac0_gain_write)
	AM_RANGE(0x02, 0x02) AM_WRITE(dac1_value_write)
	AM_RANGE(0x03, 0x03) AM_WRITE(dac1_gain_write)
	AM_RANGE(0x04, 0x04) AM_WRITE(dac2_value_write)
	AM_RANGE(0x05, 0x05) AM_WRITE(dac2_gain_write)
	AM_RANGE(0x06, 0x06) AM_WRITE(dac3_value_write)
	AM_RANGE(0x07, 0x07) AM_WRITE(dac3_gain_write)

	// not 100% sure how rom banking works.. but each channel can specify a different bank for the 0x8000 range.  Maybe the bank happens when the interrupt triggers so each channel reads the correct data? (so we'd need to put the actual functions in the CTC callbacks)
	AM_RANGE(0x10, 0x10) AM_WRITE(dac0_rombank_write)
	AM_RANGE(0x11, 0x11) AM_WRITE(dac1_rombank_write)
	AM_RANGE(0x12, 0x12) AM_WRITE(dac2_rombank_write)
	AM_RANGE(0x13, 0x13) AM_WRITE(dac3_rombank_write)




	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)

	AM_RANGE(0x30, 0x30) AM_READWRITE(megaphx_sound_cmd_r, megaphx_sound_to_68k_w)
	AM_RANGE(0x31, 0x31) AM_READ(megaphx_sound_sent_r)
ADDRESS_MAP_END



static MACHINE_CONFIG_FRAGMENT( inder_sb )
	MCFG_CPU_ADD("audiocpu", Z80, 8000000) // unk freq
	MCFG_CPU_CONFIG(daisy_chain)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io)

	MCFG_DEVICE_ADD("ctc", Z80CTC, 4000000) // unk freq
	// runs in IM2 , vector set to 0x20 , values there are 0xCC, 0x02, 0xE6, 0x02, 0x09, 0x03, 0x23, 0x03  (so 02cc, 02e6, 0309, 0323, all of which are valid irq handlers)
	MCFG_Z80CTC_INTR_CB(WRITELINE(inder_sb_device, z80ctc_ch0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(inder_sb_device, z80ctc_ch1))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(inder_sb_device, z80ctc_ch2))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(inder_sb_device, z80ctc_ch3))
	// was this correct?!?

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac0")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_DAC_ADD("dac3")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

machine_config_constructor inder_sb_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( inder_sb );
}

void inder_sb_device::device_start()
{
	membank("snddata")->configure_entries(0, 8, memregion("user2")->base(), 0x8000);
	membank("snddata")->set_entry(0);

	install_sound_hacks();
}

void inder_sb_device::device_reset()
{
	m_soundirq = 0;
}
