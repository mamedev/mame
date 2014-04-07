#include "emu.h"
#include "sound/ay8910.h"
#include "audio/cclimber.h"


/* macro to convert 4-bit unsigned samples to 16-bit signed samples */
#define SAMPLE_CONV4(a) (0x1111*((a&0x0f))-0x8000)

#define SND_CLOCK 3072000   /* 3.072 MHz */

const ay8910_interface cclimber_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, cclimber_audio_device, sample_select_w),
	DEVCB_NULL
};

static INT16 *samplebuf;    /* buffer to decode samples at run time */

static SAMPLES_START( cclimber_sh_start )
{
	running_machine &machine = device.machine();
	samplebuf = 0;
	if (machine.root_device().memregion("samples")->base())
		samplebuf = auto_alloc_array(machine, INT16, 2 * machine.root_device().memregion("samples")->bytes());
}

const samples_interface cclimber_samples_interface =
{
	1,
	NULL,
	cclimber_sh_start
};

MACHINE_CONFIG_FRAGMENT( cclimber_audio )
	MCFG_SOUND_ADD("aysnd", AY8910, SND_CLOCK/2)
	MCFG_SOUND_CONFIG(cclimber_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, ":mono", 0.50)

	MCFG_SAMPLES_ADD("samples", cclimber_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, ":mono", 0.5)
MACHINE_CONFIG_END

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CCLIMBER_AUDIO = &device_creator<cclimber_audio_device>;


//**************************************************************************
//  JSA IIIS-SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  cclimber_audio_device: Constructor
//-------------------------------------------------

cclimber_audio_device::cclimber_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CCLIMBER_AUDIO, "cclimber Sound Board", tag, owner, clock, "cclimber_audio", __FILE__),
	m_sample_num(0),
	m_sample_freq(0),
	m_sample_volume(0),
	m_samples(*this, "samples")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cclimber_audio_device::device_start()
{
}

//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor cclimber_audio_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cclimber_audio );
}

WRITE8_MEMBER( cclimber_audio_device::sample_select_w )
{
	m_sample_num = data;
}

WRITE8_MEMBER( cclimber_audio_device::sample_rate_w )
{
	/* calculate the sampling frequency */
	m_sample_freq = SND_CLOCK / 4 / (256 - data);
}

WRITE8_MEMBER( cclimber_audio_device::sample_volume_w )
{
	m_sample_volume = data & 0x1f;    /* range 0-31 */
}

WRITE8_MEMBER( cclimber_audio_device::sample_trigger_w )
{
	if (data == 0)
		return;

	play_sample(32 * m_sample_num,m_sample_freq,m_sample_volume);
}


void cclimber_audio_device::play_sample(int start,int freq,int volume)
{
	int len;
	int romlen = machine().root_device().memregion("samples")->bytes();
	const UINT8 *rom = machine().root_device().memregion("samples")->base();

	if (!rom) return;

	/* decode the rom samples */
	len = 0;
	while (start + len < romlen && rom[start+len] != 0x70)
	{
		int sample;

		sample = (rom[start + len] & 0xf0) >> 4;
		samplebuf[2*len] = SAMPLE_CONV4(sample) * volume / 31;

		sample = rom[start + len] & 0x0f;
		samplebuf[2*len + 1] = SAMPLE_CONV4(sample) * volume / 31;

		len++;
	}

	m_samples->start_raw(0,samplebuf,2 * len,freq);
}
