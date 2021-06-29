// license:LGPL-2.1+
// copyright-holders:Angelo Salese,Ryan Holtz,David Haywood

/*
    Super A'Can sound driver (UM6619)

    The UM6619 integrates:

    implemented here:

    6502 CPU
    DMA Controller
    Pad Interface for 2 controllers

    implemented in audio/supracan_um6619_audiosoc.cpp

    Custom audio hardware

    see audio/supracan_um6619_audiosoc.cpp for further notes

*/

#include "emu.h"

#include "audio/supracan_um6619_cpu.h"

#define LOG_UNKNOWNS    (1 << 0)
#define LOG_DMA         (1 << 1)
#define LOG_IRQS        (1 << 9)
#define LOG_SOUND       (1 << 10)
#define LOG_68K_SOUND   (1 << 12)
#define LOG_CONTROLS    (1 << 13)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SUPRACAN_UM6619_CPU, supracan_um6619_cpu_device, "umc6619_cpu", "UM6619 CPU + logic")

supracan_um6619_cpu_device::supracan_um6619_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: supracan_um6619_cpu_device(mconfig, SUPRACAN_UM6619_CPU, tag, owner, clock)
{
}

supracan_um6619_cpu_device::supracan_um6619_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: m6502_device(mconfig, type, tag, owner, clock)
	, m_soundram(*this, "soundram")
	, m_porta_r(*this)
	, m_portb_r(*this)
	, m_read_maincpu_space(*this)
	, m_write_maincpu_space(*this)
{
	program_config.m_internal_map = std::move(address_map_constructor(FUNC(supracan_um6619_cpu_device::supracan_sound_mem), this));
}

uint8_t supracan_um6619_cpu_device::_6502_soundmem_r(offs_t offset)
{
	uint8_t data = m_soundram[offset];

	switch (offset)
	{
	case 0x300: // Boot OK status
		break;
	case 0x402:
	case 0x403:
	{
		const uint8_t index = offset - 0x402;
		data = m_sound_cpu_shift_regs[index];
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_SOUND, "%s: 6502_soundmem_r: Shift register %d read: %02x\n", machine().describe_context(), index, data);
		}
		break;
	}
	case 0x410:
		data = m_soundcpu_irq_enable;
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_SOUND, "%s: 6502_soundmem_r: IRQ enable read: %02x\n", machine().describe_context(), data);
		}
		break;
	case 0x411:
		data = m_soundcpu_irq_source;
		m_soundcpu_irq_source = 0;
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_SOUND, "%s: %s: 6502_soundmem_r: Sound IRQ source read + clear: %02x\n", machine().describe_context(), machine().time().to_string(), data);
			set_input_line(0, CLEAR_LINE);
		}
		break;
	case 0x420:
		if (!machine().side_effects_disabled())
		{
			data = m_sound_status;
			LOGMASKED(LOG_SOUND, "%s: %s: 6502_soundmem_r: Sound hardware status read:       0420 = %02x\n", machine().describe_context(), machine().time().to_string(), m_sound_status);
		}
		break;
	case 0x422:
		if (!machine().side_effects_disabled())
		{
			data = sound_read(m_sound_reg_addr);
			LOGMASKED(LOG_SOUND, "%s: %s: 6502_soundmem_r: Sound hardware reg data read:     0422 = %02x\n", machine().describe_context(), machine().time().to_string(), data);
		}
		break;
	case 0x404:
	case 0x405:
	case 0x409:
	case 0x414:
	case 0x416:
		// Intentional fall-through
	default:
		if (offset >= 0x400 && offset < 0x500)
		{
			if (!machine().side_effects_disabled())
			{
				LOGMASKED(LOG_SOUND | LOG_UNKNOWNS, "%s: 6502_soundmem_r: Unknown register %04x (%02x)\n", machine().describe_context(), offset, data);
			}
		}
		break;
	}

	return data;
}


void supracan_um6619_cpu_device::mc68k_soundram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// this looks like a hack? it allows booting, but stops 8-bit writes from working properly for this region
	m_soundram[offset * 2 + 1] = data & 0xff;
	m_soundram[offset * 2] = data >> 8;

	if (offset * 2 < 0x500 && offset * 2 >= 0x400)
	{
		if (ACCESSING_BITS_8_15)
		{
			_6502_soundmem_w(offset * 2, data >> 8);
		}
		if (ACCESSING_BITS_0_7)
		{
			_6502_soundmem_w(offset * 2 + 1, data & 0xff);
		}
	}
	LOGMASKED(LOG_68K_SOUND, "%s: 68k sound RAM write: %04x & %04x = %04x\n", machine().describe_context(), offset << 1, mem_mask, (uint16_t)data);
}

uint16_t supracan_um6619_cpu_device::mc68k_soundram_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_soundram[offset * 2] << 8;
	data |= m_soundram[offset * 2 + 1];

	if (offset * 2 >= 0x400 && offset * 2 < 0x500)
	{
		data = 0;
		if (ACCESSING_BITS_8_15)
		{
			data |= _6502_soundmem_r(offset * 2) << 8;
		}
		if (ACCESSING_BITS_0_7)
		{
			data |= _6502_soundmem_r(offset * 2 + 1);
		}
	}
	//LOGMASKED(LOG_68K_SOUND, "%s: 68k sound RAM read: %04x & %04x (%04x)\n", machine().describe_context(), offset << 1, mem_mask, data);

	return data;
}



void supracan_um6619_cpu_device::_6502_soundmem_w(offs_t offset, uint8_t data)
{
	static attotime s_curr_time = attotime::zero;
	attotime now = machine().time();

	switch (offset)
	{
	case 0x407:
	{
		LOGMASKED(LOG_CONTROLS, "%s: 6502_soundmem_w: Shift register control: %02x\n", machine().describe_context(), data);
		const uint8_t old = m_sound_cpu_shift_ctrl;
		m_sound_cpu_shift_ctrl = data;
		const uint8_t lowered = old & ~m_sound_cpu_shift_ctrl;
		for (uint8_t pad = 0; pad < 2; pad++)
		{
			if (BIT(lowered, pad + 0))
			{
				m_latched_controls[pad] = (pad == 0) ? m_porta_r() : m_portb_r();
			}
			if (BIT(lowered, pad + 2))
			{
				m_sound_cpu_shift_regs[pad] <<= 1;
				m_sound_cpu_shift_regs[pad] |= BIT(m_latched_controls[pad], 15);
				m_latched_controls[pad] <<= 1;
			}
			if (BIT(lowered, pad + 4))
			{
				m_sound_cpu_shift_regs[pad] = 0;
			}
		}
		break;
	}
	case 0x410:
		m_soundcpu_irq_enable = data;
		LOGMASKED(LOG_SOUND | LOG_IRQS, "%s: 6502_soundmem_w: IRQ enable: %02x\n", machine().describe_context(), data);
		break;
	case 0x420:
		LOGMASKED(LOG_SOUND, "%s: %s: 6502_soundmem_w: Sound addr write:                 0420 = %02x\n", machine().describe_context(), now.to_string(), data);
		m_sound_reg_addr = data;
		break;
	case 0x422:
	{
		attotime delta = (s_curr_time == attotime::zero ? attotime::zero : (now - s_curr_time));
		s_curr_time = now;
		LOGMASKED(LOG_SOUND, "%s: %s: 6502_soundmem_w: Sound data write:                 0422 = %02x (delta %0.3f)\n", machine().describe_context(), now.to_string(), data, (float)delta.as_double());
		sound_write(m_sound_reg_addr, data);
		break;
	}
	default:
		if (offset >= 0x400 && offset < 0x500)
		{
			LOGMASKED(LOG_SOUND | LOG_UNKNOWNS, "%s: 6502_soundmem_w: Unknown register %04x = %02x\n", machine().describe_context(), offset, data);
		}
		m_soundram[offset] = data;
		break;
	}
}

void supracan_um6619_cpu_device::supracan_sound_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(supracan_um6619_cpu_device::_6502_soundmem_r), FUNC(supracan_um6619_cpu_device::_6502_soundmem_w)).share("soundram");
}

void supracan_um6619_cpu_device::set_sound_irq(uint8_t bit, uint8_t state)
{
	const uint8_t old = m_soundcpu_irq_source;
	if (state)
		m_soundcpu_irq_source |= 1 << bit;
	else
		m_soundcpu_irq_source &= ~(1 << bit);
	const uint8_t changed = old ^ m_soundcpu_irq_source;
	if (changed)
	{
		set_input_line(0, (m_soundcpu_irq_enable & m_soundcpu_irq_source) ? ASSERT_LINE : CLEAR_LINE);
	}
}

uint16_t supracan_um6619_cpu_device::sound_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0x04/2:
		data = (m_soundram[0x40c] << 8) | m_soundram[0x40d];
		LOGMASKED(LOG_SOUND, "%s: sound_r: DMA Request address from 6502, %08x: %04x & %04x\n", machine().describe_context(), 0xe90000 + (offset << 1), data, mem_mask);
		break;

	case 0x0c/2:
		data = m_soundram[0x40a];
		LOGMASKED(LOG_SOUND, "%s: sound_r: DMA Request flag from 6502, %08x: %04x & %04x\n", machine().describe_context(), 0xe90000 + (offset << 1), data, mem_mask);
		machine().debug_break();
		break;

	default:
		LOGMASKED(LOG_SOUND | LOG_UNKNOWNS, "%s: sound_r: Unknown register: %08x & %04x\n", machine().describe_context(), 0xe90000 + (offset << 1), mem_mask);
		break;
	}

	return data;
}

void supracan_um6619_cpu_device::sound_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
	case 0x000a/2:  /* Sound cpu IRQ request. */
		LOGMASKED(LOG_SOUND, "%s: Sound CPU IRQ request: %04x\n", machine().describe_context(), data);
		set_sound_irq(5, 1);
		//set_input_line(0, ASSERT_LINE);
		break;
	case 0x001c/2:  /* Sound cpu control. Bit 0 tied to sound cpu RESET line */
	{
		const uint16_t old = m_sound_cpu_ctrl;
		m_sound_cpu_ctrl = data;
		const uint16_t changed = old ^ m_sound_cpu_ctrl;
		if (BIT(changed, 0))
		{
			if (BIT(m_sound_cpu_ctrl, 0))
			{
				/* Reset and enable the sound cpu */
				set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				m6502_device::device_reset();
			}
			else
			{
				/* Halt the sound cpu */
				set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			}
		}
		LOGMASKED(LOG_SOUND, "%s: Sound CPU ctrl write: %04x\n", machine().describe_context(), data);
		break;
	}
	default:
		LOGMASKED(LOG_SOUND | LOG_UNKNOWNS, "%s: sound_w: Unknown register: %08x = %04x & %04x\n", machine().describe_context(), 0xe90000 + (offset << 1), data, mem_mask);
		break;
	}
}


void supracan_um6619_cpu_device::dma_w(int offset, uint16_t data, uint16_t mem_mask, int ch)
{
	switch (offset)
	{
	case 0x00/2: // Source address MSW
		LOGMASKED(LOG_DMA, "dma_w: source msw %d: %04x\n", ch, data);
		m_dma_regs.source[ch] &= 0x0000ffff;
		m_dma_regs.source[ch] |= data << 16;
		break;
	case 0x02/2: // Source address LSW
		LOGMASKED(LOG_DMA, "dma_w: source lsw %d: %04x\n", ch, data);
		m_dma_regs.source[ch] &= 0xffff0000;
		m_dma_regs.source[ch] |= data;
		break;
	case 0x04/2: // Destination address MSW
		LOGMASKED(LOG_DMA, "dma_w: dest msw %d: %04x\n", ch, data);
		m_dma_regs.dest[ch] &= 0x0000ffff;
		m_dma_regs.dest[ch] |= data << 16;
		break;
	case 0x06/2: // Destination address LSW
		LOGMASKED(LOG_DMA, "dma_w: dest lsw %d: %04x\n", ch, data);
		m_dma_regs.dest[ch] &= 0xffff0000;
		m_dma_regs.dest[ch] |= data;
		break;
	case 0x08/2: // Byte count
		LOGMASKED(LOG_DMA, "dma_w: count %d: %04x\n", ch, data);
		m_dma_regs.count[ch] = data;
		break;
	case 0x0a/2: // Control
		LOGMASKED(LOG_DMA, "dma_w: control %d: %04x\n", ch, data);
		if (data & 0x8800)
		{
			//if (data & 0x2000)
			//  m_dma_regs.source-=2;
			LOGMASKED(LOG_DMA, "dma_w: Kicking off a DMA from %08x to %08x, %d bytes (%04x)\n", m_dma_regs.source[ch], m_dma_regs.dest[ch], m_dma_regs.count[ch] + 1, data);

			for (int i = 0; i <= m_dma_regs.count[ch]; i++)
			{
				if (data & 0x1000)
				{
					m_write_maincpu_space(m_dma_regs.dest[ch], m_read_maincpu_space(m_dma_regs.source[ch]));

					m_dma_regs.dest[ch] += 2;
					m_dma_regs.source[ch] += 2;
					if (data & 0x0100)
						if ((m_dma_regs.dest[ch] & 0xf) == 0)
							m_dma_regs.dest[ch] -= 0x10;
				}
				else
				{
					uint16_t data = m_read_maincpu_space(m_dma_regs.source[ch], (m_dma_regs.dest[ch] & 1) ? 0x00ff : 0xff00) >> ((m_dma_regs.dest[ch] & 1) ? 0 : 8);
					m_write_maincpu_space(m_dma_regs.dest[ch], data << ((m_dma_regs.dest[ch] & 1) ? 0 : 8), (m_dma_regs.dest[ch] & 1) ? 0x00ff : 0xff00);

					m_dma_regs.dest[ch]++;
					m_dma_regs.source[ch]++;
				}
			}
		}
		else if (data != 0x0000) // fake DMA, used by C.U.G.
		{
			LOGMASKED(LOG_UNKNOWNS | LOG_DMA, "dma_w: Unknown DMA kickoff value of %04x (other regs %08x, %08x, %d)\n", data, m_dma_regs.source[ch], m_dma_regs.dest[ch], m_dma_regs.count[ch] + 1);
			fatalerror("dma_w: Unknown DMA kickoff value of %04x (other regs %08x, %08x, %d)\n",data, m_dma_regs.source[ch], m_dma_regs.dest[ch], m_dma_regs.count[ch] + 1);
		}
		break;
	default:
		LOGMASKED(LOG_UNKNOWNS | LOG_DMA, "dma_w: Unknown register: %08x = %04x & %04x\n", 0xe90020 + (offset << 1), data, mem_mask);
		break;
	}
}

void supracan_um6619_cpu_device::dma_channel0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	dma_w(offset, data, mem_mask, 0);
}

void supracan_um6619_cpu_device::dma_channel1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	dma_w(offset, data, mem_mask, 1);
}



void supracan_um6619_cpu_device::device_start()
{
	m6502_device::device_start();

	m_porta_r.resolve();
	m_portb_r.resolve();
	m_read_maincpu_space.resolve();
	m_write_maincpu_space.resolve_safe();

	save_item(NAME(m_soundcpu_irq_enable));
	save_item(NAME(m_soundcpu_irq_source));
	save_item(NAME(m_sound_cpu_ctrl));
	save_item(NAME(m_sound_cpu_shift_ctrl));
	save_item(NAME(m_sound_cpu_shift_regs));
	save_item(NAME(m_sound_status));
	save_item(NAME(m_sound_reg_addr));
	save_item(NAME(m_latched_controls));

	save_item(NAME(m_dma_regs.source));
	save_item(NAME(m_dma_regs.dest));
	save_item(NAME(m_dma_regs.count));
	save_item(NAME(m_dma_regs.control));
}

void supracan_um6619_cpu_device::device_reset()
{
	m6502_device::device_reset();

	m_soundcpu_irq_enable = 0;
	m_soundcpu_irq_source = 0;
	m_sound_cpu_ctrl = 0;

	m_sound_cpu_shift_ctrl = 0;
	std::fill(std::begin(m_sound_cpu_shift_regs), std::end(m_sound_cpu_shift_regs), 0);

	m_sound_status = 0;
	m_sound_reg_addr = 0;

	std::fill(std::begin(m_latched_controls), std::end(m_latched_controls), 0);

	set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}
