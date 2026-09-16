// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
// thanks-to:Darren Izzard
/**********************************************************************

    Acorn ANV02 Music 500

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANV02_Music500.html

    Hybrid Music 5000 Synthesiser

    https://www.retro-kit.co.uk/page.cfm/content/Hybrid-Music-5000-Synthesiser/
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Hybrid_Music5000.html

    Hybrid Music 3000 Expander

    https://www.retro-kit.co.uk/page.cfm/content/Hybrid-Music-3000-Expander/

    Peartree Music 87 Synthesiser

    http://www.computinghistory.org.uk/det/4535/Peartree-Computers-Music-87-Synthesizer-(M500)/

    TODO:
    - convert to use DAC76 device
    - add suitably patched version of Ample ROM
    - add ROMs for Music 87

**********************************************************************/


#include "emu.h"
#include "m5000.h"
#include "speaker.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(HTMUSIC, htmusic_device, "htmusic", "Hybrid Technology Music System")

DEFINE_DEVICE_TYPE(BBC_M500,  bbc_m500_device,  "bbc_m500",  "Acorn Music 500");
DEFINE_DEVICE_TYPE(BBC_M5000, bbc_m5000_device, "bbc_m5000", "Hybrid Music 5000 Synthesiser");
DEFINE_DEVICE_TYPE(BBC_M3000, bbc_m3000_device, "bbc_m3000", "Hybrid Music 3000 Expander");
DEFINE_DEVICE_TYPE(BBC_M87,   bbc_m87_device,   "bbc_m87",   "Peartree Music 87 Synthesiser");


//-------------------------------------------------
//  ROM( m5000 )
//-------------------------------------------------

//ROM_START( m5000 )
	//ROM_REGION(0x4000, "exp_rom", 0)
	// Each AMPLE Nucleus ROM has a unique identification code (ID)
	//ROM_LOAD("ample_patch.rom", 0x0000, 0x4000, CRC(15bbd499) SHA1(ac90f403b3bde0cdaae68546799b94962bc00fcb))
//ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_m500_device::add_common_devices(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	HTMUSIC(config, m_hybrid, 12_MHz_XTAL / 2);
	m_hybrid->add_route(0, "speaker", 1.0, 0);
	m_hybrid->add_route(1, "speaker", 1.0, 1);

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, DERIVED_CLOCK(1, 1), bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_1mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
}

void bbc_m500_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);

	SOFTWARE_LIST(config, "flop_ls_hybrid").set_original("bbc_flop_hybrid").set_filter("M500");
}

void bbc_m5000_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);

	SOFTWARE_LIST(config, "flop_ls_hybrid").set_original("bbc_flop_hybrid").set_filter("M5000");
}

void bbc_m3000_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);
}

void bbc_m87_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);

	SOFTWARE_LIST(config, "flop_ls_hybrid").set_original("bbc_flop_hybrid").set_filter("M87");
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

//const tiny_rom_entry *bbc_m5000_device::device_rom_region() const
//{
	//return ROM_NAME( m5000 );
//}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_hybrid_device - constructor
//-------------------------------------------------

bbc_m500_device::bbc_m500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_1mhzbus(*this, "1mhzbus")
	, m_hybrid(*this, "hybrid")
	, m_page(0)
{
}

bbc_m500_device::bbc_m500_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_m500_device(mconfig, BBC_M500, tag, owner, clock)
{
}

bbc_m5000_device::bbc_m5000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_m500_device(mconfig, BBC_M5000, tag, owner, clock)
{
}

bbc_m3000_device::bbc_m3000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_m500_device(mconfig, BBC_M3000, tag, owner, clock)
{
}

bbc_m87_device::bbc_m87_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_m500_device(mconfig, BBC_M87, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_m500_device::device_start()
{
	// register for save states
	save_item(NAME(m_page));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_m500_device::fred_r(offs_t offset)
{
	return m_1mhzbus->fred_r(offset);
}

void bbc_m500_device::fred_w(offs_t offset, uint8_t data)
{
	if (offset == 0xff)
	{
		m_page = data;
	}

	m_1mhzbus->fred_w(offset, data);
}

uint8_t bbc_m500_device::jim_r(offs_t offset)
{
	return m_1mhzbus->jim_r(offset);
}

void bbc_m500_device::jim_w(offs_t offset, uint8_t data)
{
	if ((m_page & 0xf0) == 0x30)
	{
		uint8_t page = (m_page >> 1) & 7;

		m_hybrid->ram_w((page << 8) | offset, data);
	}

	m_1mhzbus->jim_w(offset, data);
}


void bbc_m3000_device::jim_w(offs_t offset, uint8_t data)
{
	if ((m_page & 0xf0) == 0x50)
	{
		uint8_t page = (m_page >> 1) & 7;

		m_hybrid->ram_w((page << 8) | offset, data);
	}

	m_1mhzbus->jim_w(offset, data);
}


//**************************************************************************
//  HYBRID MUSIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  htmusic_device - constructor
//-------------------------------------------------

htmusic_device::htmusic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HTMUSIC, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_counter(0)
	, m_c4d(0)
	, m_sign(0)
	, m_sam(0)
	, m_disable(false)
	, m_modulate(false)
	, m_dsp_timer(nullptr)
	, m_stream(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void htmusic_device::device_start()
{
	// 12-bit antilog as per Am6070 datasheet
	for (int i = 0; i < 128; i++)
		m_antilog[i] = (uint16_t)(2 * (pow(2.0, i >> 4) * ((i & 15) + 16.5) - 16.5));

	// create the stream
	m_stream = stream_alloc(0, 2, clock() / 128);

	// allocate timer
	m_dsp_timer = timer_alloc(FUNC(htmusic_device::dsp_tick), this);

	// register for save states
	save_item(NAME(m_wave_ram));
	save_item(NAME(m_phase_ram));
	save_item(NAME(m_counter));
	save_item(NAME(m_c4d));
	save_item(NAME(m_sign));
	save_item(NAME(m_disable));
	save_item(NAME(m_modulate));
	save_item(NAME(m_sam));
	save_item(NAME(m_sam_l));
	save_item(NAME(m_sam_r));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void htmusic_device::device_reset()
{
	// clear ram
	memset(m_wave_ram, 0, sizeof(m_wave_ram));
	memset(m_phase_ram, 0, sizeof(m_phase_ram));

	memset(m_sam_l, 0, sizeof(m_sam_l));
	memset(m_sam_r, 0, sizeof(m_sam_r));

	// reset counter
	m_counter = 0;
	m_disable = false;
	m_modulate = false;

	// start timer
	m_dsp_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));
}


//-------------------------------------------------
//  digital signal processor
//-------------------------------------------------

#define I_CHAN(c) ((((c) & 0x1e) >> 1) + (((c) & 0x01) << 7) + 0x700)
#define I_FREQ(c) (I_CHAN(c))
#define I_WAVESEL(c) (I_CHAN(c) + 0x50)
#define I_AMP(c) (I_CHAN(c) + 0x60)
#define I_CTL(c) (I_CHAN(c) + 0x70)
#define FREQ(c) ((m_wave_ram[I_FREQ(c) + 0x20] << 16) | (m_wave_ram[I_FREQ(c) + 0x10] << 8) | (m_wave_ram[I_FREQ(c)] & 0x7e))
#define DISABLE(c) (BIT(m_wave_ram[I_FREQ(c)], 0))
#define AMP(c) (m_wave_ram[I_AMP(c)])
#define WAVESEL(c) (m_wave_ram[I_WAVESEL(c)] >> 4)
#define CTL(c) (m_wave_ram[I_CTL(c)])
#define MODULATE(c) (BIT(CTL(c), 5))
#define INVERT(c) (BIT(CTL(c), 4))
#define PAN(c) (CTL(c) & 0x0f)

TIMER_CALLBACK_MEMBER(htmusic_device::dsp_tick)
{
	// 4-bit channel select
	uint8_t channel = (m_counter >> 3) & 0x0f;
	uint8_t c = (channel << 1) + (m_modulate ? 1 : 0);

	// 3-bit program counter
	switch (m_counter & 0x07)
	{
	case 0:
		m_stream->update();

		m_disable = DISABLE(c);
		break;

	case 1:
		break;

	case 2:
		// In the real hardware the disable bit works by forcing the
		// phase accumulator to zero.
		if (m_disable)
		{
			m_phase_ram[channel] = 0;
			m_c4d = 0;
		}
		else
		{
			uint32_t sum = m_phase_ram[channel] + FREQ(c);
			m_phase_ram[channel] = sum & 0xffffff;
			// c4d is used for "Synchronization" e.g. the "Wha" instrument
			m_c4d = sum >> 24;
		}
		break;

	case 3:
		break;

	case 4:
		break;

	case 5:
		m_sam = m_wave_ram[(WAVESEL(c) << 7) + (m_phase_ram[channel] >> 17)];
		break;

	case 6:
		// The amplitude operates in the log domain
		// - m_sam holds the wave table output which is 1 bit sign and 7 bit magnitude
		// - amp holds the amplitude which is 1 bit sign and 8 bit magnitude (0x00 being quite, 0x7f being loud)
		// The real hardware combines these in a single 8 bit adder, as we do here.
		//
		// Consider a positive wav value (sign bit = 1)
		//       wav: (0x80 -> 0xFF) + amp: (0x00 -> 0x7F) => (0x80 -> 0x7E)
		// values in the range 0x80...0xff are very small and clamped to zero
		//
		// Consider a negative wav value (sign bit = 0)
		//       wav: (0x00 -> 0x7F) + amp: (0x00 -> 0x7F) => (0x00 -> 0xFE)
		// values in the range 0x00...0x7f are very small and clamped to zero
		//
		// In both cases:
		// - zero clamping happens when the sign bit stays the same
		// - the 7-bit result is in bits 0..6
		//
		m_sign = m_sam & 0x80;
		m_sam += AMP(c);
		if ((m_sign ^ m_sam) & 0x80)
			m_sam &= 0x7f; // sign bits being different is the normal case
		else
			m_sam = 0x00;  // sign bits being the same indicates underflow so clamp to zero
		break;

	case 7:
	{
		m_modulate = MODULATE(c) && (!!(m_sign) || !!(m_c4d));

		// in the real hardware, inversion does not affect modulation
		if (INVERT(c))
			m_sign ^= 0x80;

		// m_sam is an 8-bit log value
		if (m_sign)
			m_sam = m_antilog[m_sam];  // sign being 1 is positive
		else
			m_sam = -m_antilog[m_sam]; // sign being 0 is negative

		// m_sam is a 12-bit linear sample
		uint8_t pan;
		switch (PAN(c))
		{
		case 8: case 9: case 10: pan = 6; break;
		case 11: pan = 5; break;
		case 12: pan = 4; break;
		case 13: pan = 3; break;
		case 14: pan = 2; break;
		case 15: pan = 1; break;
		default: pan = 0; break;
		}

		// Apply panning. In the real hardware, a disabled channel is not actually
		// forced to zero, but this seems harmless so leave in for now.
		m_sam_l[channel] = m_disable ? 0 : ((m_sam * pan) / 6);
		m_sam_r[channel] = m_disable ? 0 : ((m_sam * (6 - pan)) / 6);
		break;
	}
	}

	m_counter++;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void htmusic_device::sound_stream_update(sound_stream &stream)
{
	// iterate over channels and accumulate sample data
	for (int channel = 0; channel < 16; channel++)
	{
		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
		{
			stream.add_int(0, sampindex, m_sam_l[channel], 8031 * 16);
			stream.add_int(1, sampindex, m_sam_r[channel], 8031 * 16);
		}
	}
}
