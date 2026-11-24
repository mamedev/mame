// license:BSD-3-Clause
// copyright-holders:Carl

// Analog Devices AD1848, main codec in Windows Sound System adapters
// TODO: Emulate pin-compatible Crystal Semiconductor CS4231 and its extra Mode 2 features

#include "emu.h"
#include "ad1848.h"

#include "speaker.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


DEFINE_DEVICE_TYPE(AD1848, ad1848_device, "ad1848", "AD1848 16-bit SoundPort Stereo Codec")

ad1848_device::ad1848_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AD1848, tag, owner, clock),
	device_memory_interface( mconfig, *this ),
	m_space_io_config("indirect_io", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(ad1848_device::io_map), this)),
	m_irq_cb(*this),
	m_drq_cb(*this),
	m_ldac(*this, "ldac"),
	m_rdac(*this, "rdac")
{
}

void ad1848_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();
	DAC_16BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 0); // unknown DAC
	DAC_16BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 1); // unknown DAC
}


void ad1848_device::device_start()
{
	m_timer = timer_alloc(FUNC(ad1848_device::update_tick), this);

	save_item(NAME(m_regs.idx));
	save_item(NAME(m_addr));
	save_item(NAME(m_stat));
	save_item(NAME(m_sam_cnt));
	save_item(NAME(m_calibration_cycles));
	save_item(NAME(m_samples));
	save_item(NAME(m_count));
	save_item(NAME(m_play));
	save_item(NAME(m_mce));
	save_item(NAME(m_trd));
	save_item(NAME(m_irq));
}

void ad1848_device::device_reset()
{
	memset(&m_regs.idx[0], '\0', sizeof(m_regs));
	m_addr = 0;
	m_stat = 0;
	m_sam_cnt = 0;
	m_calibration_cycles = 0;
	m_samples = 0;
	m_play = false;
	m_irq = false;
}

device_memory_interface::space_config_vector ad1848_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_space_io_config)
	};
}

void ad1848_device::io_map(address_map &map)
{
	// TODO: split up into individual handlers
	map(0x00, 0x0f).rw(FUNC(ad1848_device::ixa_r), FUNC(ad1848_device::ixa_w));
}

uint8_t ad1848_device::ixa_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOG("IXA%d: read\n", offset);
	// WSSXLAT fails if it doesn't find a suitable identifer
	if (offset == 12)
		return 0x0a;
	return m_regs.idx[offset];
}

void ad1848_device::ixa_w(offs_t offset, uint8_t data)
{
	// TODO: register #12 is r/o on regular AD1848K, but actually r/w on spinoffs
	if (offset == 12)
	{
		LOG("Warning: IXA12 misc register write %02x\n", data);
		return;
	}
	LOG("IXA%d: %02x\n", offset, data);
	m_regs.idx[m_addr] = data;
	switch(m_addr)
	{
		case 8:
			if(!m_mce)
				return;
			m_regs.dform &= 0x7f;
			break;
		case 9:
		{
			static constexpr int div_factor[] = {3072, 1536, 896, 768, 448, 384, 512, 2560};

			// auto-calibration
			if (m_mce && BIT(data, 3))
			{
				logerror("set auto-calibration\n");
				m_regs.init |= 0x20;
				m_calibration_cycles = 384;
			}

			m_play = (data & 1) ? true : false;
			// FIXME: provide external configuration for XTAL1 (24.576 MHz) and XTAL2 (16.9344 MHz) inputs
			attotime rate = (m_play || BIT(m_regs.init, 5)) ? attotime::from_hz(((m_regs.dform & 1) ? 16.9344_MHz_XTAL : 24.576_MHz_XTAL)
					/ div_factor[(m_regs.dform >> 1) & 7]) : attotime::never;
			m_timer->adjust(rate, 0 , rate);
			m_drq_cb(m_play ? ASSERT_LINE : CLEAR_LINE);
			// TODO: check if 1 -> 1 play transitions also reset the clock
			// sometimes multibyte sound drifts
			if (m_play)
				m_sam_cnt = 0;
			break;
		}
		case 14:
		case 15:
			m_count = (m_regs.ubase << 8) | m_regs.lbase;
			break;
	}
}

uint8_t ad1848_device::read(offs_t offset)
{
	switch(offset)
	{
		case 0:
			return m_addr | (m_mce ? 0x40 : 0) | (m_trd ? 0x20 : 0);
		case 1:
			return space(0).read_byte(m_addr);
		case 2:
			return m_stat | (m_irq ? 1 : 0);
		case 3:
			break; // capture
	}
	return 0;
}

void ad1848_device::write(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:
			m_addr = data & 0xf;
			m_mce = data & 0x40 ? true : false;
			m_trd = data & 0x20 ? true : false;
			break;
		case 1:
			space(0).write_byte(m_addr, data);
			break;
		case 2:
			m_irq_cb(CLEAR_LINE);
			m_irq = false;
			if(m_regs.iface & 1)
			{
				m_play = true;
				m_sam_cnt = 0;
			}
			break;
		case 3:
			break; // playback
	}
}

uint8_t ad1848_device::dack_r()
{
	m_drq_cb(CLEAR_LINE);
	return 0; // not implemented
}

void ad1848_device::dack_w(uint8_t data)
{
	if(!m_play)
		return;
	m_sam_cnt++;
	// data is little endian
	// (lower) left ch. first -> (upper byte of 16-bit) -> (lower) right ch. -> (upper)
	// TODO: add status live updates (bits 2 and 3, PIO only or applies here as well?)
	switch(m_regs.dform >> 4)
	{
		case MODE_DAC8_MONO:
		case MODE_ULAW_MONO:
		case MODE_ALAW_MONO:
			m_samples = data;
			m_sam_cnt = 0;
			break;
		case MODE_DAC8_STEREO:
		case MODE_ULAW_STEREO:
		case MODE_DAC16_MONO:
		case MODE_ALAW_STEREO:
			m_samples = (data << 8) | ((m_samples >> 8) & 0xff);
			if(m_sam_cnt == 2)
				m_sam_cnt = 0;
			break;
		case MODE_DAC16_STEREO:
			m_samples = (data << 24) | ((m_samples >> 8) & 0xffffff);
			if(m_sam_cnt == 4)
				m_sam_cnt = 0;
			break;
	}
	if(!m_sam_cnt)
		m_drq_cb(CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(ad1848_device::update_tick)
{
	// auto-calibration
	if (BIT(m_regs.init, 5))
	{
		if (--m_calibration_cycles == 0)
		{
			logerror("calibration finished\n");
			m_regs.init &= ~0x20;
		}
	}

	if(!m_play)
		return;
	switch(m_regs.dform >> 4)
	{
		case MODE_DAC8_MONO: // 8bit mono
			m_ldac->write(m_samples << 8);
			m_rdac->write(m_samples << 8);
			break;
		case MODE_DAC8_STEREO: // 8bit stereo
			m_ldac->write(m_samples << 8);
			m_rdac->write(m_samples & 0xff00);
			break;
		case MODE_ULAW_MONO: // ulaw mono
		case MODE_ULAW_STEREO: // ulaw stereo
		case MODE_ALAW_MONO: // alaw mono
		case MODE_ALAW_STEREO: // alaw stereo
			break;
		case MODE_DAC16_MONO: // 16bit mono
			m_ldac->write(m_samples ^ 0x8000);
			m_rdac->write(m_samples ^ 0x8000);
			break;
		case MODE_DAC16_STEREO: // 16bit stereo
			m_ldac->write(m_samples ^ 0x8000);
			m_rdac->write((m_samples >> 16) ^ 0x8000);
			break;
	}
	if(!m_count)
	{
		if(m_regs.pinc & 2)
		{
			if(m_trd)
				m_play = false;
			m_irq_cb(ASSERT_LINE);
			m_irq = true;
		}
		m_count = (m_regs.ubase << 8) | m_regs.lbase;
	}
	else
		m_count--;
	m_drq_cb(ASSERT_LINE);
}
