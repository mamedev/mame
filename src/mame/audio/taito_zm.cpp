// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
/***************************************************************************

    Taito Zoom ZSG-2 sound board
    Includes: MN10200 CPU, ZOOM ZSG-2 audio chip, TMS57002 DASP
    By Olivier Galibert.

----------------------------------------------------------------------------

Panasonic MN1020012A Sound CPU (QFP128), 12.5MHz pin 30 (OSCI)

Zoom Corp. ZSG-2 Sound PCM chip (QFP100), 25MHz pin 99

Texas Instruments TMS57002DPHA DSP (QFP80)
* 12.5MHz pin 11 [25/2] (CLKIN)
* 32.5525kHz pin 5 and 76 (LRCKO) (LRCKI)
* 1.5625MHz pin 75 and 2 [25/16] (BCKI) (BCKO)

Newer games have a Panasonic MN1020819DA,
and a Zoom Corp. ZFX-2 DSP instead of the TMS57002.


TODO:
- add DSP, sound is tinny without it

***************************************************************************/

#include "emu.h"
#include "taito_zm.h"
#include "machine/intelfsh.h"

/**************************************************************************/

DEFINE_DEVICE_TYPE(TAITO_ZOOM, taito_zoom_device, "taito_zoom", "Taito Zoom Sound System")

//-------------------------------------------------
//  taito_zoom_device - constructor
//-------------------------------------------------

taito_zoom_device::taito_zoom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TAITO_ZOOM, tag, owner, clock),
	m_soundcpu(*this, "mn10200"),
	m_zsg2(*this, "zsg2"),
	m_reg_address(0),
	m_tms_ctrl(0),
	m_use_flash(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void taito_zoom_device::device_start()
{
	m_snd_shared_ram = make_unique_clear<uint8_t[]>(0x100);

	// register for savestates
	save_item(NAME(m_reg_address));
	save_item(NAME(m_tms_ctrl));
	save_pointer(NAME(m_snd_shared_ram.get()), 0x100);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void taito_zoom_device::device_reset()
{
	m_reg_address = 0;

	m_zsg2->reset();
}


/***************************************************************************

  MN10200 I/O and Memory Map

***************************************************************************/

READ8_MEMBER(taito_zoom_device::shared_ram_r)
{
	return m_snd_shared_ram[offset];
}

WRITE8_MEMBER(taito_zoom_device::shared_ram_w)
{
	m_snd_shared_ram[offset] = data;
}


READ8_MEMBER(taito_zoom_device::tms_ctrl_r)
{
	return m_tms_ctrl;
}

WRITE8_MEMBER(taito_zoom_device::tms_ctrl_w)
{
#if 0
	tms57002_reset_w(data & 4);
	tms57002_cload_w(data & 2);
	tms57002_pload_w(data & 1);
#endif

	m_tms_ctrl = data;
}


void taito_zoom_device::taitozoom_mn_map(address_map &map)
{
	if(m_use_flash) {
		map(0x080000, 0x0fffff).r(":pgmflash", FUNC(intelfsh16_device::read));
	} else {
		map(0x080000, 0x0fffff).rom().region("mn10200", 0);
	}
	map(0x400000, 0x41ffff).ram();
	map(0x800000, 0x8007ff).rw("zsg2", FUNC(zsg2_device::read), FUNC(zsg2_device::write));
	map(0xc00000, 0xc00001).ram(); // TMS57002 comms
	map(0xe00000, 0xe000ff).rw(this, FUNC(taito_zoom_device::shared_ram_r), FUNC(taito_zoom_device::shared_ram_w)); // M66220FP for comms with maincpu
}


/***************************************************************************

  maincpu I/O

***************************************************************************/

WRITE16_MEMBER(taito_zoom_device::sound_irq_w)
{
	m_soundcpu->set_input_line(0, ASSERT_LINE);
	m_soundcpu->set_input_line(0, CLEAR_LINE);
}

READ16_MEMBER(taito_zoom_device::sound_irq_r)
{
	// reads this before writing irq, bit 0 = busy?
	return 0;
}

WRITE16_MEMBER(taito_zoom_device::reg_data_w)
{
	switch (m_reg_address)
	{
		case 0x04:
			// zsg2+dsp global volume left
			if (data & 0xc0c0)
				popmessage("ZOOM gain L %04X, contact MAMEdev", data);
			m_zsg2->set_output_gain(0, (data & 0x3f) / 63.0);
			break;

		case 0x05:
			// zsg2+dsp global volume right
			if (data & 0xc0c0)
				popmessage("ZOOM gain R %04X, contact MAMEdev", data);
			m_zsg2->set_output_gain(1, (data & 0x3f) / 63.0);
			break;

		default:
			break;
	}
}

WRITE16_MEMBER(taito_zoom_device::reg_address_w)
{
	m_reg_address = data & 0xff;
}


/***************************************************************************

  Machine Config

***************************************************************************/

MACHINE_CONFIG_START(taito_zoom_device::device_add_mconfig)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("mn10200", MN1020012A, XTAL(25'000'000)/2)
	MCFG_MN10200_READ_PORT_CB(1, READ8(DEVICE_SELF, taito_zoom_device, tms_ctrl_r))
	MCFG_MN10200_WRITE_PORT_CB(1, WRITE8(DEVICE_SELF, taito_zoom_device, tms_ctrl_w))
	MCFG_DEVICE_PROGRAM_MAP(taitozoom_mn_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60000))

	MCFG_ZSG2_ADD("zsg2", XTAL(25'000'000))

	// we assume the parent machine has created lspeaker/rspeaker
	MCFG_SOUND_ROUTE(0, "^lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "^rspeaker", 1.0)
MACHINE_CONFIG_END
