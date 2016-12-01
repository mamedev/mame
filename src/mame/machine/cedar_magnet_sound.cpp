// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
   how should the ctcs hook up properly?

   irq vectors

   0xe6 - from ctc0? (would need vector of e0?) probably used to drive MSM5205
   0xee - from ctc0? (would need vector of e8?) ^^
   0xf6 - drive AY (once per frame?) triggered by ctc1 channel 3? (sets vector to f0, f6 = channel3?)
   0xff - read sound latch (triggered by write from master board)

   any attempts made to hook up the ctcs end up resulting in it taking an interrupt
   with vector 0xf0, which points to 0x0000 and resets the cpu?!

   does the device here also need to add daisychain functions in order for the 0xff vector to be used
   with the soundlatch writes?

*/

#include "emu.h"
#include "cedar_magnet_sound.h"


extern const device_type CEDAR_MAGNET_SOUND = &device_creator<cedar_magnet_sound_device>;


cedar_magnet_sound_device::cedar_magnet_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cedar_magnet_board_device(mconfig, CEDAR_MAGNET_SOUND, "Cedar Sound", tag, owner, clock, "cedmag_sound", __FILE__),
	m_ctc0(*this, "ctc0"),
	m_ctc1(*this, "ctc1")
{
}


READ8_MEMBER(cedar_magnet_sound_device::soundlatch_r)
{
	return m_command;
}

void cedar_magnet_sound_device::write_command(uint8_t data)
{
	m_command = data;
	// this interrupt causes it to read the soundlatch at 0x14
	m_cpu->set_input_line_and_vector(0, HOLD_LINE,0xff);
}



static ADDRESS_MAP_START( cedar_magnet_sound_map, AS_PROGRAM, 8, cedar_magnet_sound_device )
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cedar_magnet_sound_io, AS_IO, 8, cedar_magnet_sound_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)

	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ctc0", z80ctc_device, read, write)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ctc1", z80ctc_device, read, write)

	AM_RANGE(0x08, 0x08) AM_WRITE(adpcm_latch_w)

	AM_RANGE(0x0c, 0x0c) AM_DEVWRITE("aysnd0", ay8910_device, address_w)
	AM_RANGE(0x0d, 0x0d) AM_DEVWRITE("aysnd0", ay8910_device, data_w)

	AM_RANGE(0x10, 0x10) AM_DEVWRITE("aysnd1", ay8910_device, address_w)
	AM_RANGE(0x11, 0x11) AM_DEVWRITE("aysnd1", ay8910_device, data_w)

	AM_RANGE(0x14, 0x14) AM_READ(soundlatch_r)

ADDRESS_MAP_END

WRITE8_MEMBER(cedar_magnet_sound_device::adpcm_latch_w)
{
	// it writes 8-bits of sample data here, to be fed to the msm 4-bits at a time
	// probably via other triggers
	m_adpcm_data = data;
}

WRITE8_MEMBER(cedar_magnet_sound_device::ay1_porta_w)
{
	// unknown but used
}

WRITE_LINE_MEMBER(cedar_magnet_sound_device::ctc0_z0_w)
{
//  printf("USED ctc0_z0_w %d\n", state);
}

WRITE_LINE_MEMBER(cedar_magnet_sound_device::ctc0_z1_w)
{
//  printf("USED  ctc0_z1_w %d\n", state);
}


// I don't think any of the below are used

WRITE_LINE_MEMBER(cedar_magnet_sound_device::ctc1_z0_w)
{
	printf("ctc1_z0_w %d\n", state);
}

WRITE_LINE_MEMBER(cedar_magnet_sound_device::ctc1_z1_w)
{
	printf("ctc1_z1_w %d\n", state);
}

WRITE_LINE_MEMBER(cedar_magnet_sound_device::ctc1_z2_w)
{
	printf("ctc1_z2_w %d\n", state);
}

WRITE_LINE_MEMBER(cedar_magnet_sound_device::ctc0_z2_w)
{
	printf("ctc0_z2_w %d\n", state);
}

WRITE_LINE_MEMBER(cedar_magnet_sound_device::ctc0_int_w)
{
	//printf("ctc0_int_w %d\n", state);
}

WRITE_LINE_MEMBER(cedar_magnet_sound_device::ctc1_int_w)
{

}

#if 0
static const z80_daisy_config daisy_chain[] =
{
	{ "ctc1" },
	{ "ctc0" },
// soundlatch from main CPU needs to be able to generate a vector too?
	{ nullptr }
};
#endif

TIMER_CALLBACK_MEMBER(cedar_magnet_sound_device::reset_assert_callback)
{
	cedar_magnet_board_device::reset_assert_callback(ptr,param);
	// reset lines go to the ctc as well?
	m_ctc0->reset();
	m_ctc1->reset();
}



INTERRUPT_GEN_MEMBER(cedar_magnet_sound_device::fake_irq)
{
	// these should be coming from the CTC...
//  if (m_fake_counter==0) m_cpu->set_input_line_and_vector(0, HOLD_LINE,0xe6);
//  if (m_fake_counter==1) m_cpu->set_input_line_and_vector(0, HOLD_LINE,0xee);
	if (m_fake_counter==2) m_cpu->set_input_line_and_vector(0, HOLD_LINE,0xf6); // drives the AY, should be from ctc1 4th counter?

	m_fake_counter++;

	if (m_fake_counter == 4) m_fake_counter = 0;
}

static MACHINE_CONFIG_FRAGMENT( cedar_magnet_sound )
	MCFG_CPU_ADD("topcpu", Z80,4000000)
	MCFG_CPU_PROGRAM_MAP(cedar_magnet_sound_map)
	MCFG_CPU_IO_MAP(cedar_magnet_sound_io)
//  MCFG_Z80_DAISY_CHAIN(daisy_chain)
	MCFG_CPU_PERIODIC_INT_DRIVER(cedar_magnet_sound_device, fake_irq, 4*60)

	MCFG_DEVICE_ADD("ctc0", Z80CTC, 4000000/8 )
//  MCFG_Z80CTC_INTR_CB(INPUTLINE("topcpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_INTR_CB(WRITELINE(cedar_magnet_sound_device, ctc0_int_w))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(cedar_magnet_sound_device, ctc0_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(cedar_magnet_sound_device, ctc0_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(cedar_magnet_sound_device, ctc0_z2_w))

	MCFG_DEVICE_ADD("ctc1", Z80CTC, 4000000/8 )
//  MCFG_Z80CTC_INTR_CB(INPUTLINE("topcpu", INPUT_LINE_IRQ0))
//  MCFG_Z80CTC_INTR_CB(DEVWRITELINE("ctc0", z80ctc_device, trg0))
	MCFG_Z80CTC_INTR_CB(WRITELINE(cedar_magnet_sound_device, ctc1_int_w))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(cedar_magnet_sound_device, ctc1_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(cedar_magnet_sound_device, ctc1_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(cedar_magnet_sound_device, ctc1_z2_w))

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd0", AY8910, 4000000/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("aysnd1", AY8910, 4000000/2)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(cedar_magnet_sound_device, ay1_porta_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("adpcm", MSM5205, 4000000/16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_CONFIG_END

machine_config_constructor cedar_magnet_sound_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cedar_magnet_sound );
}

void cedar_magnet_sound_device::device_start()
{
	m_cpu = subdevice<z80_device>("topcpu");
	m_ram = (uint8_t*)memshare("ram")->ptr();
}

void cedar_magnet_sound_device::device_reset()
{
	m_command = 0;
	m_fake_counter = 0;
	cedar_magnet_board_device::device_reset();
}
