// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Takahiro Nogi
/***************************************************************************

    Nichibutsu sound HW

    Shared component between niyanpai.cpp and csplayh5.cpp

    Uses a TMPZ84C011 with YM3812 and two DACs

    TODO:
    - DVD sound routing in here

***************************************************************************/

#include "emu.h"
#include "nichisnd.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(NICHISND, nichisnd_device, "nichisnd", "Nichibutsu Sound Device")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nichisnd_device - constructor
//-------------------------------------------------

nichisnd_device::nichisnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NICHISND, tag, owner, clock),
	m_soundlatch(*this, "soundlatch"),
	m_sound_rom(*this, "audiorom")
{
}

void nichisnd_device::nichisnd_map(address_map &map)
{
	map(0x0000, 0x77ff).rom().region("audiorom", 0);
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr("soundbank");
}

void nichisnd_device::nichisnd_io_map(address_map &map)
{
	map(0x80, 0x81).mirror(0xff00).w("ymsnd", FUNC(ym3812_device::write));
}


WRITE8_MEMBER(nichisnd_device::soundbank_w)
{
	membank("soundbank")->set_entry(data & 0x03);
}

WRITE8_MEMBER(nichisnd_device::soundlatch_clear_w)
{
	if (!(data & 0x01)) m_soundlatch->clear_w(space, 0, 0);
}


//-------------------------------------------------
//  add_device_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

static const z80_daisy_config daisy_chain[] =
{
	TMPZ84C011_DAISY_INTERNAL,
	{ nullptr }
};



void nichisnd_device::device_add_mconfig(machine_config &config)
{
	tmpz84c011_device& audiocpu(TMPZ84C011(config, "audiocpu", 8000000)); /* TMPZ84C011, 8.00 MHz */
	audiocpu.set_daisy_config(daisy_chain);
	audiocpu.set_addrmap(AS_PROGRAM, &nichisnd_device::nichisnd_map);
	audiocpu.set_addrmap(AS_IO, &nichisnd_device::nichisnd_io_map);
	audiocpu.in_pd_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	audiocpu.out_pa_callback().set(FUNC(nichisnd_device::soundbank_w));
	audiocpu.out_pb_callback().set("dac1", FUNC(dac_byte_interface::data_w));
	audiocpu.out_pc_callback().set("dac2", FUNC(dac_byte_interface::data_w));
	audiocpu.out_pe_callback().set(FUNC(nichisnd_device::soundlatch_clear_w));
	audiocpu.zc0_callback().set("audiocpu", FUNC(tmpz84c011_device::trg3));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM3812(config, "ymsnd", 4000000).add_route(ALL_OUTPUTS, "speaker", 1.0);

	DAC_8BIT_R2R(config, "dac1", 0).add_route(ALL_OUTPUTS, "speaker", 0.37); // unknown DAC
	DAC_8BIT_R2R(config, "dac2", 0).add_route(ALL_OUTPUTS, "speaker", 0.37); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac1", 1.0, DAC_VREF_POS_INPUT).add_route(0, "dac1", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "dac2", 1.0, DAC_VREF_POS_INPUT).add_route(0, "dac2", -1.0, DAC_VREF_NEG_INPUT);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nichisnd_device::device_start()
{
	uint8_t *SNDROM = m_sound_rom;

	// sound program patch
	SNDROM[0x0213] = 0x00;          // DI -> NOP

	// initialize sound rom bank
	membank("soundbank")->configure_entries(0, 3, m_sound_rom + 0x8000, 0x8000);
	membank("soundbank")->set_entry(0);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nichisnd_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

// use this to connect to the sound board
WRITE8_MEMBER(nichisnd_device::sound_host_command_w)
{
	m_soundlatch->write(space,0,data);
}
