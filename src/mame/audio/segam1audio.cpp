// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Sega Model 1 sound board (68000 + 2x 315-5560 "MultiPCM")

  used for Model 1 and early Model 2 games

***************************************************************************/

#include "emu.h"
#include "audio/segam1audio.h"

#include "machine/clock.h"
#include "speaker.h"

#define M68000_TAG      "sndcpu"
#define MULTIPCM_1_TAG  "pcm1"
#define MULTIPCM_2_TAG  "pcm2"
#define YM3438_TAG      "ymsnd"
#define UART_TAG        "uart"
#define MPCMBANK1_TAG   "m1pcm1_bank"
#define MPCMBANK2_TAG   "m1pcm2_bank"

void segam1audio_device::segam1audio_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x09ffff).rom().region(M68000_TAG, 0x20000); // mirror of upper ROM socket
	map(0xc20000, 0xc20003).rw(UART_TAG, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xc40000, 0xc40007).rw(MULTIPCM_1_TAG, FUNC(multipcm_device::read), FUNC(multipcm_device::write)).umask16(0x00ff);
	map(0xc40012, 0xc40013).nopw();
	map(0xc50000, 0xc50001).w(FUNC(segam1audio_device::m1_snd_mpcm_bnk1_w));
	map(0xc60000, 0xc60007).rw(MULTIPCM_2_TAG, FUNC(multipcm_device::read), FUNC(multipcm_device::write)).umask16(0x00ff);
	map(0xc70000, 0xc70001).w(FUNC(segam1audio_device::m1_snd_mpcm_bnk2_w));
	map(0xd00000, 0xd00007).rw(YM3438_TAG, FUNC(ym3438_device::read), FUNC(ym3438_device::write)).umask16(0x00ff);
	map(0xf00000, 0xf0ffff).ram();
}

void segam1audio_device::mpcm1_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x1fffff).bankr(MPCMBANK1_TAG);
}

void segam1audio_device::mpcm2_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x1fffff).bankr(MPCMBANK2_TAG);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGAM1AUDIO, segam1audio_device, "segam1audio", "Sega Model 1 Sound Board")

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(segam1audio_device::device_add_mconfig)
	MCFG_DEVICE_ADD(M68000_TAG, M68000, 10000000)  // verified on real h/w
	MCFG_DEVICE_PROGRAM_MAP(segam1audio_map)

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD(YM3438_TAG, YM3438, 8000000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.60)

	MCFG_DEVICE_ADD(MULTIPCM_1_TAG, MULTIPCM, 8000000)
	MCFG_DEVICE_ADDRESS_MAP(0, mpcm1_map)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_DEVICE_ADD(MULTIPCM_2_TAG, MULTIPCM, 8000000)
	MCFG_DEVICE_ADDRESS_MAP(0, mpcm2_map)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	I8251(config, m_uart, 8000000); // T82C51, clock unknown
	m_uart->rxrdy_handler().set_inputline(m_audiocpu, M68K_IRQ_2);
	m_uart->txd_handler().set(FUNC(segam1audio_device::output_txd));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 500000)); // 16 times 31.25MHz (standard Sega/MIDI sound data rate)
	uart_clock.signal_handler().set(m_uart, FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append(m_uart, FUNC(i8251_device::write_rxc));
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  segam1audio_device - constructor
//-------------------------------------------------

segam1audio_device::segam1audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGAM1AUDIO, tag, owner, clock),
	m_audiocpu(*this, M68000_TAG),
	m_multipcm_1(*this, MULTIPCM_1_TAG),
	m_multipcm_2(*this, MULTIPCM_2_TAG),
	m_ym(*this, YM3438_TAG),
	m_uart(*this, UART_TAG),
	m_multipcm1_region(*this, MULTIPCM_1_TAG),
	m_multipcm2_region(*this, MULTIPCM_2_TAG),
	m_mpcmbank1(*this, MPCMBANK1_TAG),
	m_mpcmbank2(*this, MPCMBANK2_TAG),
	m_rxd_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void segam1audio_device::device_start()
{
	m_rxd_handler.resolve_safe();
	m_mpcmbank1->configure_entries(0, 4, m_multipcm1_region->base(), 0x100000);
	m_mpcmbank2->configure_entries(0, 4, m_multipcm2_region->base(), 0x100000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void segam1audio_device::device_reset()
{
	m_uart->write_cts(0);
}

WRITE16_MEMBER(segam1audio_device::m1_snd_mpcm_bnk1_w)
{
	m_mpcmbank1->set_entry(data & 3);
}

WRITE16_MEMBER(segam1audio_device::m1_snd_mpcm_bnk2_w)
{
	m_mpcmbank2->set_entry(data & 3);
}

WRITE_LINE_MEMBER(segam1audio_device::write_txd)
{
	m_uart->write_rxd(state);
}

WRITE_LINE_MEMBER(segam1audio_device::output_txd)
{
	m_rxd_handler(state);
}
