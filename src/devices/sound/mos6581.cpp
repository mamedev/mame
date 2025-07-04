// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Curt Coder
/**********************************************************************

    MOS 6581/8580 Sound Interface Device emulation

**********************************************************************/

#include "emu.h"
#include "mos6581.h"
#include "sid.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MOS6581, mos6581_device, "mos6581", "MOS 6581 SID")
DEFINE_DEVICE_TYPE(MOS8580, mos8580_device, "mos8580", "MOS 8580 SID")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6581_device - constructor
//-------------------------------------------------

mos6581_device::mos6581_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_read_potx(*this, 0xff)
	, m_read_poty(*this, 0xff)
	, m_stream(nullptr)
	, m_variant(variant)

{
}

mos6581_device::mos6581_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6581_device(mconfig, MOS6581, tag, owner, clock, TYPE_6581)
{
}

mos6581_device::~mos6581_device()
{
}

//-------------------------------------------------
//  mos8580_device - constructor
//-------------------------------------------------

mos8580_device::mos8580_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6581_device(mconfig, MOS8580, tag, owner, clock, TYPE_8580)
{
}

//-------------------------------------------------
//  save_state - add save states
//-------------------------------------------------

void mos6581_device::save_state(SID6581_t *token)
{
	save_item(NAME(token->type));
	save_item(NAME(token->clock));

	save_item(NAME(token->PCMfreq));
	save_item(NAME(token->PCMsid));
	save_item(NAME(token->PCMsidNoise));

	save_item(NAME(token->reg));
	//save_item(NAME(token->sidKeysOn));
	//save_item(NAME(token->sidKeysOff));

	save_item(NAME(token->masterVolume));
	save_item(NAME(token->masterVolumeAmplIndex));

	save_item(NAME(token->filter.Enabled));
	save_item(NAME(token->filter.Type));
	save_item(NAME(token->filter.CurType));
	save_item(NAME(token->filter.Dy));
	save_item(NAME(token->filter.ResDy));
	save_item(NAME(token->filter.Value));

	for (int v = 0; v < m_token->max_voices; v++)
	{
		save_item(NAME(token->optr[v].reg), v);

		save_item(NAME(token->optr[v].SIDfreq), v);
		save_item(NAME(token->optr[v].SIDpulseWidth), v);
		save_item(NAME(token->optr[v].SIDctrl), v);
		save_item(NAME(token->optr[v].SIDAD), v);
		save_item(NAME(token->optr[v].SIDSR), v);

		save_item(NAME(token->optr[v].sync), v);

		save_item(NAME(token->optr[v].pulseIndex), v);
		save_item(NAME(token->optr[v].newPulseIndex), v);

		save_item(NAME(token->optr[v].curSIDfreq), v);
		save_item(NAME(token->optr[v].curNoiseFreq), v);

		save_item(NAME(token->optr[v].output), v);
		//save_item(NAME(token->optr[v].outputMask), v);

		save_item(NAME(token->optr[v].filtVoiceMask), v);
		save_item(NAME(token->optr[v].filtEnabled), v);
		save_item(NAME(token->optr[v].filtLow), v);
		save_item(NAME(token->optr[v].filtRef), v);
		save_item(NAME(token->optr[v].filtIO), v);

		save_item(NAME(token->optr[v].cycleLenCount), v);
#if defined(DIRECT_FIXPOINT)
		save_item(NAME(token->optr[v].cycleLen.l), v);
		save_item(NAME(token->optr[v].cycleAddLen.l), v);
#else
		save_item(NAME(token->optr[v].cycleAddLenPnt), v);
		save_item(NAME(token->optr[v].cycleLen), v);
		save_item(NAME(token->optr[v].cycleLenPnt), v);
#endif

#if defined(DIRECT_FIXPOINT)
		save_item(NAME(token->optr[v].waveStep.l), v);
		save_item(NAME(token->optr[v].waveStepAdd.l), v);
#else
		save_item(NAME(token->optr[v].waveStep), v);
		save_item(NAME(token->optr[v].waveStepAdd), v);
		save_item(NAME(token->optr[v].waveStepPnt), v);
		save_item(NAME(token->optr[v].waveStepAddPnt), v);
#endif
		save_item(NAME(token->optr[v].waveStepOld), v);
		for (int n = 0; n < 2; n++)
		{
			save_item(NAME(token->optr[v].wavePre[n].len), v | (n << 4));
#if defined(DIRECT_FIXPOINT)
			save_item(NAME(token->optr[v].wavePre[n].stp), v | (n << 4));
#else
			save_item(NAME(token->optr[v].wavePre[n].pnt), v | (n << 4));
			save_item(NAME(token->optr[v].wavePre[n].stp), v | (n << 4));
#endif
		}

#if defined(DIRECT_FIXPOINT)
		save_item(NAME(token->optr[v].noiseReg.l), v);
#else
		save_item(NAME(token->optr[v].noiseReg), v);
#endif
		save_item(NAME(token->optr[v].noiseStep), v);
		save_item(NAME(token->optr[v].noiseStepAdd), v);
		save_item(NAME(token->optr[v].noiseOutput), v);
		save_item(NAME(token->optr[v].noiseIsLocked), v);

		save_item(NAME(token->optr[v].ADSRctrl), v);
		//save_item(NAME(token->optr[v].gateOnCtrl), v);
		//save_item(NAME(token->optr[v].gateOffCtrl), v);

#ifdef SID_FPUENVE
		save_item(NAME(token->optr[v].fenveStep), v);
		save_item(NAME(token->optr[v].fenveStepAdd), v);
		save_item(NAME(token->optr[v].enveStep), v);
#elif defined(DIRECT_FIXPOINT)
		save_item(NAME(token->optr[v].enveStep.l), v);
		save_item(NAME(token->optr[v].enveStepAdd.l), v);
#else
		save_item(NAME(token->optr[v].enveStep), v);
		save_item(NAME(token->optr[v].enveStepAdd), v);
		save_item(NAME(token->optr[v].enveStepPnt), v);
		save_item(NAME(token->optr[v].enveStepAddPnt), v);
#endif
		save_item(NAME(token->optr[v].enveVol), v);
		save_item(NAME(token->optr[v].enveSusVol), v);
		save_item(NAME(token->optr[v].enveShortAttackCount), v);
	}

	save_item(NAME(token->optr3_outputmask));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6581_device::device_start()
{
	// create sound stream
	m_stream = stream_alloc(0, 1, machine().sample_rate());

	// initialize SID engine
	m_token = std::make_unique<SID6581_t>();
	m_token->device = this;
	m_token->mixer_channel = m_stream;
	m_token->PCMfreq = machine().sample_rate();
	m_token->clock = clock();
	m_token->type = m_variant;

	m_token->init();
	sidInitWaveformTables(m_variant);
	save_state(m_token.get());
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6581_device::device_reset()
{
	m_token->reset();
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void mos6581_device::device_post_load()
{
	m_token->postload();
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void mos6581_device::sound_stream_update(sound_stream &stream)
{
	m_token->fill_buffer(stream);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t mos6581_device::read(offs_t offset)
{
	uint8_t data;

	switch (offset & 0x1f)
	{
	case 0x19:
		data = m_read_potx(0);
		break;

	case 0x1a:
		data = m_read_poty(0);
		break;

	default:
		data = m_token->port_r(machine(), offset);
		break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos6581_device::write(offs_t offset, uint8_t data)
{
	m_token->port_w(offset, data);
}
