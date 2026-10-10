// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Sega/VideoLogic 315-6289 "ELAN"

    NAOMI 2 transform & lighting processor.  Turns model data in its 32MB
    of SDRAM into tile accelerator parameters for both CLX2s.

    TODO:
    - Two volume polygons (only the first volume is used)
    - Bump mapping
    - Fog and attenuation light routing
    - Side clipping, backface culling
    - Timing, the interrupt output
    - The macro tiler most likely decides which CLX2 gets what

***************************************************************************/

#include "emu.h"
#include "315_6289.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>

#define LOG_REGS    (1U << 1)
#define LOG_COMMAND (1U << 2)
#define LOG_STATE   (1U << 3)
#define LOG_POLYGON (1U << 4)
#define LOG_DMA     (1U << 5)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(SEGA_315_6289, sega_315_6289_device, "sega_315_6289", "Sega 315-6289 ELAN T&L processor")

sega_315_6289_device::sega_315_6289_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SEGA_315_6289, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_cpu_space(*this, finder_base::DUMMY_TAG, -1)
	, m_ram(*this, finder_base::DUMMY_TAG)
	, m_pvr(*this, { finder_base::DUMMY_TAG, finder_base::DUMMY_TAG })
	, m_vram_base{ 0, 0 }
	, m_vram_size(0)
	, m_ram_mask(0)
	, m_dma_timer(nullptr)
	, m_sh4_ctrl(0)
	, m_tiler(0)
	, m_irq_status(0)
	, m_irq_mask(0)
	, m_regs{ }
	, m_fifo{ }
	, m_dma_pending(false)
	, m_pass_remaining(0)
	, m_pass_vertex_words(8)
	, m_user_clip(0)
	, m_projection{ }
	, m_matrix_valid(false)
	, m_matrix{ }
	, m_normal_matrix{ }
	, m_near(0.0f)
	, m_far(0.0f)
	, m_envmap_offset{ 0.0f, 0.0f }
	, m_gmp_valid(false)
	, m_gmp_select(0)
	, m_gmp_gloss(0.0f)
	, m_gmp_diffuse{ }
	, m_gmp_specular{ }
	, m_light_model_valid(false)
	, m_light_model_flags(0)
	, m_diffuse_mask(0)
	, m_specular_mask(0)
	, m_ambient_base{ }
	, m_ambient_offset{ }
	, m_lights_valid(0)
	, m_open_volume(false)
	, m_shadow_volume(false)
	, m_model_tsp(0)
	, m_textured(false)
	, m_envmap(false)
{
}

void sega_315_6289_device::device_start()
{
	m_ram_mask = m_ram.bytes() - 1;
	m_dma_timer = timer_alloc(FUNC(sega_315_6289_device::dma_done), this);

	save_item(NAME(m_sh4_ctrl));
	save_item(NAME(m_tiler));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_regs));
	save_item(NAME(m_fifo));
	save_item(NAME(m_dma_pending));
	save_item(NAME(m_pass_remaining));
	save_item(NAME(m_pass_vertex_words));
	save_item(NAME(m_user_clip));
	save_item(NAME(m_projection));
	save_item(NAME(m_matrix_valid));
	save_item(NAME(m_matrix));
	save_item(NAME(m_normal_matrix));
	save_item(NAME(m_near));
	save_item(NAME(m_far));
	save_item(NAME(m_envmap_offset));
	save_item(NAME(m_gmp_valid));
	save_item(NAME(m_gmp_select));
	save_item(NAME(m_gmp_gloss));
	save_item(NAME(m_gmp_diffuse));
	save_item(NAME(m_gmp_specular));
	save_item(NAME(m_light_model_valid));
	save_item(NAME(m_light_model_flags));
	save_item(NAME(m_diffuse_mask));
	save_item(NAME(m_specular_mask));
	save_item(NAME(m_ambient_base));
	save_item(NAME(m_ambient_offset));
	save_item(STRUCT_MEMBER(m_lights, parallel));
	save_item(STRUCT_MEMBER(m_lights, color));
	save_item(STRUCT_MEMBER(m_lights, direction));
	save_item(STRUCT_MEMBER(m_lights, position));
	save_item(STRUCT_MEMBER(m_lights, routing));
	save_item(STRUCT_MEMBER(m_lights, dmode));
	save_item(STRUCT_MEMBER(m_lights, smode));
	save_item(STRUCT_MEMBER(m_lights, dist_inverse));
	save_item(STRUCT_MEMBER(m_lights, dist_attenuation));
	save_item(STRUCT_MEMBER(m_lights, angle_attenuation));
	save_item(STRUCT_MEMBER(m_lights, dist_a));
	save_item(STRUCT_MEMBER(m_lights, dist_b));
	save_item(STRUCT_MEMBER(m_lights, angle_a));
	save_item(STRUCT_MEMBER(m_lights, angle_b));
	save_item(NAME(m_lights_valid));
}

void sega_315_6289_device::device_reset()
{
	m_sh4_ctrl = 0;
	m_tiler = 0x31;
	m_irq_status = 0;
	m_irq_mask = 0;
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_dma_pending = false;
	m_dma_timer->adjust(attotime::never);
	m_pass_remaining = 0;
	m_pass_vertex_words = 8;
	m_user_clip = 0;
	reset_state();
}

void sega_315_6289_device::map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(sega_315_6289_device::unknown_r), FUNC(sega_315_6289_device::unknown_w));
	map(0x00, 0x03).r(FUNC(sega_315_6289_device::id_r));
	map(0x04, 0x07).r(FUNC(sega_315_6289_device::revision_r));
	map(0x08, 0x0b).w(FUNC(sega_315_6289_device::reset_w));
	map(0x0c, 0x0f).r(FUNC(sega_315_6289_device::queue_r));
	map(0x10, 0x13).rw(FUNC(sega_315_6289_device::sh4_ctrl_r), FUNC(sega_315_6289_device::sh4_ctrl_w));
	map(0x14, 0x17).r(FUNC(sega_315_6289_device::sdram_refresh_r));
	map(0x1c, 0x1f).r(FUNC(sega_315_6289_device::sdram_config_r));
	map(0x30, 0x33).rw(FUNC(sega_315_6289_device::tiler_r), FUNC(sega_315_6289_device::tiler_w));
	map(0x74, 0x77).rw(FUNC(sega_315_6289_device::irq_status_r), FUNC(sega_315_6289_device::irq_status_w));
	map(0x78, 0x7b).rw(FUNC(sega_315_6289_device::irq_mask_r), FUNC(sega_315_6289_device::irq_mask_w));
}

uint32_t sega_315_6289_device::id_r()
{
	return 0xe1ad0000;
}

uint32_t sega_315_6289_device::revision_r()
{
	// Texture transfers are only used when this is 0x10 or greater
	return 0x10;
}

void sega_315_6289_device::reset_w(uint32_t data)
{
	LOGMASKED(LOG_REGS, "%s: reset %08x\n", machine().describe_context(), data);
	if (!data)
	{
		m_irq_status = 0;
	}
}

uint32_t sega_315_6289_device::queue_r()
{
	// queued commands, there has to be room for one more
	return 0;
}

uint32_t sega_315_6289_device::sh4_ctrl_r()
{
	return m_sh4_ctrl;
}

void sega_315_6289_device::sh4_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// ---- -x-- enable second CLX2
	// ---- --x- ELAN has channel 2
	// ---- ---x broadcast on CS1
	COMBINE_DATA(&m_sh4_ctrl);
	LOGMASKED(LOG_REGS, "%s: SH-4 interface control %08x\n", machine().describe_context(), m_sh4_ctrl);
}

uint32_t sega_315_6289_device::sdram_refresh_r()
{
	return 0x2029;
}

uint32_t sega_315_6289_device::sdram_config_r()
{
	return 0xa7320961;
}

uint32_t sega_315_6289_device::tiler_r()
{
	return m_tiler;
}

void sega_315_6289_device::tiler_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// --ll llll ---- ---- ---- ---- ---- ---- left tile
	// ---- ---- tttt ---- ---- ---- ---- ---- top tile
	// ---- ---- ---- --rr rrrr ---- ---- ---- right tile
	// ---- ---- ---- ---- ---- bbbb ---- ---- bottom tile
	// ---- ---- ---- ---- ---- ---- --x- ---- tile vertically
	// ---- ---- ---- ---- ---- ---- ---x ---- tile horizontally
	// ---- ---- ---- ---- ---- ---- ---- ---x enable
	COMBINE_DATA(&m_tiler);
	LOGMASKED(LOG_REGS, "%s: macro tiler %08x\n", machine().describe_context(), m_tiler);
}

uint32_t sega_315_6289_device::irq_status_r()
{
	return m_irq_status;
}

void sega_315_6289_device::irq_status_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_irq_status &= ~(data & mem_mask);
}

uint32_t sega_315_6289_device::irq_mask_r()
{
	return m_irq_mask;
}

void sega_315_6289_device::irq_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_irq_mask);
	LOGMASKED(LOG_REGS, "%s: interrupt mask %08x\n", machine().describe_context(), m_irq_mask);
}

uint32_t sega_315_6289_device::unknown_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_REGS, "%s: read %02x\n", machine().describe_context(), offset * 4);
	}
	return m_regs[offset];
}

void sega_315_6289_device::unknown_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_regs[offset]);
	LOGMASKED(LOG_REGS, "%s: write %02x = %08x\n", machine().describe_context(), offset * 4, m_regs[offset]);
}

void sega_315_6289_device::command_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const int index = offset & 7;

	COMBINE_DATA(&m_fifo[index]);
	if ((index == 7) && ACCESSING_BITS_24_31)
	{
		LOGMASKED(LOG_COMMAND, "%s: FIFO %08x %08x %08x %08x %08x %08x %08x %08x\n", machine().describe_context(),
				m_fifo[0], m_fifo[1], m_fifo[2], m_fifo[3], m_fifo[4], m_fifo[5], m_fifo[6], m_fifo[7]);

		if (execute(true, 0, 32, 0) && !m_dma_pending)
		{
			m_irq_status |= IRQ_COMMAND_DONE;
		}
	}
}

template <typename... Params>
void sega_315_6289_device::error(const char *format, Params &&... args)
{
	logerror(format, std::forward<Params>(args)...);
	m_irq_status |= IRQ_ERROR | IRQ_COMMAND_DONE;
	m_pass_remaining = 0;
}

void sega_315_6289_device::reset_state()
{
	m_matrix_valid = false;
	m_gmp_valid = false;
	m_light_model_valid = false;
	m_lights_valid = 0;
	m_open_volume = false;
	m_shadow_volume = false;
	m_model_tsp = 0;
}

bool sega_315_6289_device::execute(bool fifo, offs_t address, uint32_t size, int depth)
{
	if (depth > MAX_DEPTH)
	{
		error("%s: commands nested too deep\n", machine().describe_context());
		return false;
	}

	int links = 0;
	while (size >= 32)
	{
		const uint32_t pcw = stream_r(fifo, address);
		uint32_t used = 32;

		if (m_pass_remaining || !BIT(pcw, 27))
		{
			used = pass_through(fifo, address, size & ~31);
		}
		else
		{
			switch (BIT(pcw, 8, 4))
			{
				case CMD_NULL:
					break;

				case CMD_PROJECTION:
					set_projection(fifo, address);
					break;

				case CMD_MATRIX_LIGHT:
					if ((stream_r(fifo, address + 4) == 0xf) && (stream_r(fifo, address + 8) == 0x7f))
					{
						used = 160;
						if (fifo || (size < used))
						{
							error("%s: incomplete matrix\n", machine().describe_context());
							return false;
						}
						set_matrix(fifo, address);
					}
					else if (BIT(stream_r(fifo, address + 4), 4))
					{
						set_light_model(fifo, address);
					}
					else
					{
						set_light(fifo, address);
					}
					break;

				case CMD_GMP:
					used = 64;
					if (fifo || (size < used))
					{
						error("%s: incomplete GMP\n", machine().describe_context());
						return false;
					}
					set_gmp(fifo, address);
					break;

				case CMD_ICH:
					if (fifo)
					{
						error("%s: polygon in the FIFO\n", machine().describe_context());
						return false;
					}
					used = polygon(fifo, address);
					break;

				case CMD_MODEL:
				{
					const uint32_t param = stream_r(fifo, address + 4);
					const uint32_t offset = stream_r(fifo, address + 16);
					const uint32_t length = stream_r(fifo, address + 24);

					LOGMASKED(LOG_COMMAND, "model at %08x size %x pcw %08x param %08x tsp %08x\n", offset, length, pcw, param, stream_r(fifo, address + 8));
					m_open_volume = BIT(param, 28);
					m_shadow_volume = BIT(pcw, 7);
					m_model_tsp = stream_r(fifo, address + 8);
					m_user_clip = BIT(pcw, 16, 2);
					if (!execute(false, offset & 0x1ffffff8, length, depth + 1))
					{
						return false;
					}
					m_open_volume = false;
					m_shadow_volume = false;
					m_model_tsp = 0;
					break;
				}

				case CMD_REGISTER_WAIT:
				{
					// waits for bits of a CLX2 register, i.e. the end of a list
					const uint32_t offset = stream_r(fifo, address + 4);
					const uint32_t mask = stream_r(fifo, address + 12);

					LOGMASKED(LOG_COMMAND, "register wait %08x %08x mask %08x\n", offset, stream_r(fifo, address + 8), mask);
					if ((offset != 0xffffffff) && mask)
					{
						reset_state();
					}
					break;
				}

				case CMD_LINK:
				{
					const uint32_t offset = stream_r(fifo, address + 4);
					const uint32_t destination = stream_r(fifo, address + 8);
					const uint32_t length = stream_r(fifo, address + 12);

					if (BIT(offset, 31))
					{
						texture_dma(offset, destination, length);
					}
					else if (BIT(offset, 29))
					{
						texture_copy(offset, destination, length);
					}
					else
					{
						LOGMASKED(LOG_COMMAND, "link to %08x size %x\n", offset, length);
						if ((size - used) < 32)
						{
							// a link at the end of a block continues the list rather than nesting
							if (++links > MAX_LINKS)
							{
								error("%s: endless display list\n", machine().describe_context());
								return false;
							}
							fifo = false;
							address = offset & m_ram_mask;
							size = length;
							continue;
						}
						if (!execute(false, offset & m_ram_mask, length, depth + 1))
						{
							return false;
						}
					}
					break;
				}

				default:
					error("%s: unknown command %08x at %08x\n", machine().describe_context(), pcw, address);
					return false;
			}
		}

		if (!used)
		{
			error("%s: stuck in the command stream\n", machine().describe_context());
			return false;
		}
		if (used > size)
		{
			break;
		}
		address += used;
		size -= used;
	}

	return true;
}

TIMER_CALLBACK_MEMBER(sega_315_6289_device::dma_done)
{
	m_dma_pending = false;
	m_irq_status |= IRQ_DMA_DONE;
}

void sega_315_6289_device::texture_dma(uint32_t source, uint32_t destination, uint32_t size)
{
	destination &= m_vram_size - 1;
	if (size > m_vram_size)
	{
		error("%s: texture too large (%x bytes)\n", machine().describe_context(), size);
		return;
	}

	// DMA channel 2 of the SH-4 provides the data
	sh4_ddt_dma ddt;
	ddt.source = 0;
	ddt.destination = m_vram_base[0] + destination;
	ddt.length = size;
	ddt.size = 0;
	ddt.buffer = nullptr;
	ddt.direction = 0;
	ddt.channel = 2;
	ddt.mode = 25;
	m_cpu->sh4_dma_ddt(&ddt);
	LOGMASKED(LOG_DMA, "%s: texture from SH-4 %08x to %08x size %x\n", machine().describe_context(), ddt.source - size, destination, size);

	// each CLX2 needs a copy
	for (uint32_t i = 0; i < size; i += 8)
	{
		m_cpu_space->write_qword(m_vram_base[1] + ((destination + i) & (m_vram_size - 1)), m_cpu_space->read_qword(m_vram_base[0] + ((destination + i) & (m_vram_size - 1))));
	}

	// TODO: 64 bits at 100MHz in theory
	m_dma_pending = true;
	m_dma_timer->adjust(m_cpu->cycles_to_attotime(512));
}

void sega_315_6289_device::texture_copy(uint32_t source, uint32_t destination, uint32_t size)
{
	source &= m_ram_mask;
	destination &= m_vram_size - 1;
	if (size > m_vram_size)
	{
		error("%s: texture too large (%x bytes)\n", machine().describe_context(), size);
		return;
	}

	LOGMASKED(LOG_DMA, "%s: texture from RAM %08x to %08x size %x\n", machine().describe_context(), source, destination, size);
	for (uint32_t i = 0; i < size; i += 8)
	{
		const uint64_t data = m_ram[((source + i) & m_ram_mask) >> 3];
		for (const offs_t base : m_vram_base)
		{
			m_cpu_space->write_qword(base + ((destination + i) & (m_vram_size - 1)), data);
		}
	}

	m_dma_pending = true;
	m_dma_timer->adjust(m_cpu->cycles_to_attotime(512));
}

void sega_315_6289_device::ta_w(const uint32_t *data, int count)
{
	for (int i = 0; i < count; i += 2)
	{
		const uint64_t both = data[i] | (uint64_t(data[i + 1]) << 32);
		for (powervr2_device *const pvr : m_pvr)
		{
			pvr->ta_fifo_poly_w(0, both);
		}
	}
}

uint32_t sega_315_6289_device::parameter_words(uint32_t pcw)
{
	const int list = m_pvr[0]->tafifo_listtype;

	switch (pcw >> 29)
	{
		case 4: // polygon or modifier volume
			if (((list < 0) ? BIT(pcw, 24, 3) : list) & 1)
			{
				m_pass_vertex_words = 16;
			}
			else
			{
				const int config = m_pvr[0]->pvr_parameterconfig[pcw & 0x3d];
				m_pass_vertex_words = powervr2_device::pvr_wordsvertex[config];
				return powervr2_device::pvr_wordspolygon[config];
			}
			break;

		case 5: // sprite
			m_pass_vertex_words = 16;
			break;

		case 7: // vertex
			return m_pass_vertex_words;
	}

	return 8;
}

uint32_t sega_315_6289_device::pass_through(bool fifo, offs_t address, uint32_t size)
{
	uint32_t used = 0;

	while (used < size)
	{
		if (!m_pass_remaining)
		{
			const uint32_t pcw = stream_r(fifo, address + used);
			if (BIT(pcw, 27))
			{
				break;
			}
			m_pass_remaining = parameter_words(pcw);
			LOGMASKED(LOG_COMMAND, "pass through %08x (%d words)\n", pcw, m_pass_remaining);
		}

		uint32_t data[8];
		for (int i = 0; i < 8; i++)
		{
			data[i] = stream_r(fifo, address + used + (i * 4));
		}
		ta_w(data, 8);
		used += 32;
		m_pass_remaining -= 8;
	}

	return used;
}

static inline void unpack_color(uint32_t color, float argb[4])
{
	argb[0] = float(BIT(color, 24, 8)) / 255.0f;
	argb[1] = float(BIT(color, 16, 8)) / 255.0f;
	argb[2] = float(BIT(color, 8, 8)) / 255.0f;
	argb[3] = float(BIT(color, 0, 8)) / 255.0f;
}

void sega_315_6289_device::set_projection(bool fifo, offs_t address)
{
	for (int i = 0; i < 4; i++)
	{
		m_projection[i] = stream_float_r(fifo, address + 8 + (i * 4));
	}
	LOGMASKED(LOG_STATE, "projection x %f %f y %f %f\n", m_projection[0], m_projection[1], m_projection[2], m_projection[3]);
}

void sega_315_6289_device::set_matrix(bool fifo, offs_t address)
{
	m_envmap_offset[0] = stream_float_r(fifo, address + 0x24);
	for (int i = 0; i < 9; i++)
	{
		m_normal_matrix[i / 3][i % 3] = stream_float_r(fifo, address + 0x28 + (i * 4));
	}
	m_envmap_offset[1] = stream_float_r(fifo, address + 0x4c);

	m_near = stream_float_r(fifo, address + 0x64);
	for (int i = 0; i < 12; i++)
	{
		m_matrix[i / 3][i % 3] = stream_float_r(fifo, address + 0x68 + (i * 4));
	}
	m_far = stream_float_r(fifo, address + 0x98);
	m_matrix_valid = true;

	LOGMASKED(LOG_STATE, "matrix %f %f %f %f / %f %f %f %f / %f %f %f %f near %f far %f\n",
			m_matrix[0][0], m_matrix[1][0], m_matrix[2][0], m_matrix[3][0],
			m_matrix[0][1], m_matrix[1][1], m_matrix[2][1], m_matrix[3][1],
			m_matrix[0][2], m_matrix[1][2], m_matrix[2][2], m_matrix[3][2],
			m_near, m_far);
}

void sega_315_6289_device::set_light_model(bool fifo, offs_t address)
{
	const uint32_t masks = stream_r(fifo, address + 8);
	float color[4];

	m_light_model_flags = stream_r(fifo, address + 4);
	m_diffuse_mask = masks & 0xffff;
	m_specular_mask = masks >> 16;
	unpack_color(stream_r(fifo, address + 12), color);
	std::copy_n(&color[1], 3, m_ambient_base);
	unpack_color(stream_r(fifo, address + 16), color);
	std::copy_n(&color[1], 3, m_ambient_offset);
	m_light_model_valid = true;

	LOGMASKED(LOG_STATE, "light model %08x diffuse %04x specular %04x ambient %08x %08x\n", m_light_model_flags, m_diffuse_mask, m_specular_mask,
			stream_r(fifo, address + 12), stream_r(fifo, address + 16));
}

void sega_315_6289_device::set_light(bool fifo, offs_t address)
{
	const uint32_t pcw = stream_r(fifo, address);
	const uint32_t color = stream_r(fifo, address + 4);
	const uint32_t param = stream_r(fifo, address + 8);
	const int id = color & 0xf;
	light &l = m_lights[id];

	l.parallel = BIT(pcw, 20);
	l.color[0] = float(BIT(color, 24, 8)) / 255.0f;
	l.color[1] = float(BIT(color, 16, 8)) / 255.0f;
	l.color[2] = float(BIT(color, 8, 8)) / 255.0f;

	// 12 bits each, low nibbles in the control word; points away from the light
	l.direction[0] = -float((int32_t(int8_t(BIT(param, 16, 8))) * 16) + int32_t(BIT(pcw, 16, 4))) / 2047.0f;
	l.direction[1] = -float((int32_t(int8_t(BIT(param, 8, 8))) * 16) + int32_t(BIT(pcw, 4, 4))) / 2047.0f;
	l.direction[2] = -float((int32_t(int8_t(BIT(param, 0, 8))) * 16) + int32_t(BIT(pcw, 0, 4))) / 2047.0f;
	l.routing = BIT(param, 24, 4);
	l.dist_attenuation = false;
	l.angle_attenuation = false;
	if (l.parallel)
	{
		l.dmode = BIT(param, 28, 2);
		l.smode = 0;
	}
	else
	{
		const uint32_t dist = stream_r(fifo, address + 24);
		const uint32_t angle = stream_r(fifo, address + 28);

		l.dmode = BIT(color, 5, 3);
		l.smode = BIT(param, 28, 2);
		for (int i = 0; i < 3; i++)
		{
			l.position[i] = stream_float_r(fifo, address + 12 + (i * 4));
		}
		if (!dist && !angle && (l.position[0] == 0.0f) && (l.position[1] == 0.0f) && (l.position[2] == 0.0f))
		{
			// nothing for a point or spot light to work with
			l.parallel = true;
		}
		else
		{
			// the upper halves of single precision numbers
			l.dist_inverse = !BIT(param, 31);
			l.dist_a = u2f(dist << 16);
			l.dist_b = u2f(dist & 0xffff0000);
			l.angle_a = u2f(angle << 16);
			l.angle_b = u2f(angle & 0xffff0000);
			l.dist_attenuation = (l.dist_a != 1.0f) || (l.dist_b != 0.0f);
			l.angle_attenuation = (l.angle_a != 1.0f) || (l.angle_b != 0.0f);
		}
	}

	// the cone of a spot light uses it as is
	const float length = std::sqrt((l.direction[0] * l.direction[0]) + (l.direction[1] * l.direction[1]) + (l.direction[2] * l.direction[2]));
	if ((length > 0.0f) && l.parallel)
	{
		for (float &d : l.direction)
		{
			d /= length;
		}
	}

	// black ones have nothing to add
	if ((l.color[0] != 0.0f) || (l.color[1] != 0.0f) || (l.color[2] != 0.0f))
	{
		m_lights_valid |= 1 << id;
	}
	else
	{
		m_lights_valid &= ~(1 << id);
	}

	LOGMASKED(LOG_STATE, "light %d %s routing %x dmode %d smode %d color %f %f %f direction %f %f %f\n", id, l.parallel ? "parallel" : "point",
			l.routing, l.dmode, l.smode, l.color[0], l.color[1], l.color[2], l.direction[0], l.direction[1], l.direction[2]);
}

void sega_315_6289_device::set_gmp(bool fifo, offs_t address)
{
	const uint32_t gloss = stream_r(fifo, address + 4);

	m_gmp_select = stream_r(fifo, address + 8);
	m_gmp_gloss = std::pow(2.0f, float(BIT(gloss, 5, 3)) - 1.0f) * (1.0f + (float(BIT(gloss, 0, 5)) / 32.0f));
	unpack_color(stream_r(fifo, address + 12), m_gmp_diffuse);
	unpack_color(stream_r(fifo, address + 16), m_gmp_specular);
	m_gmp_valid = true;

	LOGMASKED(LOG_STATE, "GMP select %08x gloss %f diffuse %08x specular %08x\n", m_gmp_select, m_gmp_gloss, stream_r(fifo, address + 12), stream_r(fifo, address + 16));
}

void sega_315_6289_device::fetch_position(bool fifo, offs_t address, float view[3]) const
{
	const float x = stream_float_r(fifo, address + 4);
	const float y = stream_float_r(fifo, address + 8);
	const float z = stream_float_r(fifo, address + 12);

	if (m_matrix_valid)
	{
		// the camera looks down the negative Z axis
		view[0] = -((m_matrix[0][0] * x) + (m_matrix[1][0] * y) + (m_matrix[2][0] * z) + m_matrix[3][0]);
		view[1] = (m_matrix[0][1] * x) + (m_matrix[1][1] * y) + (m_matrix[2][1] * z) + m_matrix[3][1];
		view[2] = -((m_matrix[0][2] * x) + (m_matrix[1][2] * y) + (m_matrix[2][2] * z) + m_matrix[3][2]);
	}
	else
	{
		view[0] = x;
		view[1] = y;
		view[2] = z;
	}
}

void sega_315_6289_device::fetch_vertex(bool fifo, offs_t address, uint32_t format, vertex &v) const
{
	const uint32_t header = stream_r(fifo, address);
	float normal[3];

	fetch_position(fifo, address, v.view);
	address += 16;

	if (format & VTX_NORMAL)
	{
		for (int i = 0; i < 3; i++)
		{
			normal[i] = stream_float_r(fifo, address + (i * 4));
		}
		address += 16;
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			normal[i] = float(int8_t(BIT(header, i * 8, 8))) / 127.0f;
		}
	}
	if (m_matrix_valid)
	{
		const float x = normal[0], y = normal[1], z = normal[2];
		for (int i = 0; i < 3; i++)
		{
			normal[i] = (m_normal_matrix[i][0] * x) + (m_normal_matrix[i][1] * y) + (m_normal_matrix[i][2] * z);
		}
	}
	const float length = std::sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));
	if (length > 0.0f)
	{
		for (float &n : normal)
		{
			n /= length;
		}
	}

	v.uv[0] = v.uv[1] = 0.0f;
	if (format & VTX_UV)
	{
		v.uv[0] = stream_float_r(fifo, address);
		v.uv[1] = stream_float_r(fifo, address + 4);
		address += 8;
	}
	if (m_envmap)
	{
		v.uv[0] = std::clamp(m_envmap_offset[0] + (normal[0] / 2.0f) + 0.5f, 0.0f, 1.0f);
		v.uv[1] = std::clamp(m_envmap_offset[1] + (normal[1] / 2.0f) + 0.5f, 0.0f, 1.0f);
	}

	v.base[0] = v.base[1] = v.base[2] = v.base[3] = 1.0f;
	v.offs[0] = v.offs[1] = v.offs[2] = v.offs[3] = 0.0f;
	if (format & VTX_COLOR)
	{
		unpack_color(stream_r(fifo, address), v.base);
	}
	if (m_gmp_valid)
	{
		if (BIT(m_gmp_select, 0))
		{
			std::copy_n(m_gmp_diffuse, 4, v.base);
		}
		if (BIT(m_gmp_select, 1))
		{
			std::copy_n(m_gmp_specular, 4, v.offs);
		}
	}

	light_vertex(v, normal);
	if (!m_textured)
	{
		for (int i = 0; i < 4; i++)
		{
			v.base[i] = std::min(v.base[i] + v.offs[i], 1.0f);
		}
	}
}

void sega_315_6289_device::light_vertex(vertex &v, const float normal[3]) const
{
	// GMP can ask for the colors to be left alone
	if (!m_light_model_valid || (m_gmp_valid && BIT(m_gmp_select, 9)))
	{
		return;
	}

	float diffuse[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float specular[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// what an eye at the origin sees reflected
	float reflection[3];
	const float distance = std::sqrt((v.view[0] * v.view[0]) + (v.view[1] * v.view[1]) + (v.view[2] * v.view[2]));
	const float scale = (distance > 0.0f) ? (1.0f / distance) : 0.0f;
	const float incidence = ((v.view[0] * normal[0]) + (v.view[1] * normal[1]) + (v.view[2] * normal[2])) * scale;
	for (int i = 0; i < 3; i++)
	{
		reflection[i] = (v.view[i] * scale) - (2.0f * incidence * normal[i]);
	}

	// black takes no light
	const bool has_base = (v.base[1] != 0.0f) || (v.base[2] != 0.0f) || (v.base[3] != 0.0f);
	const bool has_offs = (v.offs[1] != 0.0f) || (v.offs[2] != 0.0f) || (v.offs[3] != 0.0f);

	for (uint32_t lights = (m_diffuse_mask | m_specular_mask) & m_lights_valid; lights; lights &= lights - 1)
	{
		const int id = std::countr_zero(lights);
		const light &l = m_lights[id];
		const bool alpha = l.routing & ROUTE_ALPHA;
		const bool is_diffuse = BIT(m_diffuse_mask, id) && (alpha || has_base);
		const bool is_specular = BIT(m_specular_mask, id) && (alpha || has_offs);
		if (!is_diffuse && !is_specular)
		{
			continue;
		}

		float direction[3];
		float attenuation = 1.0f;
		if (l.parallel)
		{
			std::copy_n(l.direction, 3, direction);
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				direction[i] = l.position[i] - v.view[i];
			}
			const float length = std::sqrt((direction[0] * direction[0]) + (direction[1] * direction[1]) + (direction[2] * direction[2]));
			if (length > 0.0f)
			{
				for (float &d : direction)
				{
					d /= length;
				}
			}
			if (l.dist_attenuation)
			{
				attenuation *= std::clamp((l.dist_b * (l.dist_inverse ? (1.0f / length) : length)) + l.dist_a, 0.0f, 1.0f);
			}
			if (l.angle_attenuation)
			{
				const float spot = (direction[0] * l.direction[0]) + (direction[1] * l.direction[1]) + (direction[2] * l.direction[2]);
				attenuation *= std::clamp(((1.0f - std::max(spot, 0.0f)) * l.angle_b) + l.angle_a, 0.0f, 1.0f);
			}
		}

		const float sign = ((l.routing & ROUTE_SUBTRACT) ? -2.0f : 2.0f) * attenuation;
		if (is_diffuse)
		{
			const float facing = (normal[0] * direction[0]) + (normal[1] * direction[1]) + (normal[2] * direction[2]);
			float factor = sign;
			if (l.dmode == 0)
			{
				factor *= std::max(facing, 0.0f);
			}
			else if (l.dmode == 1)
			{
				factor *= std::abs(facing);
			}

			if (alpha)
			{
				diffuse[0] += l.color[0] * factor;
			}
			else if (factor != 0.0f)
			{
				float *const target = (l.routing & ROUTE_DIFFUSE_OFFSET) ? specular : diffuse;
				for (int i = 0; i < 3; i++)
				{
					target[i + 1] += l.color[i] * factor * v.base[i + 1];
				}
			}
		}
		if (is_specular)
		{
			float facing = (reflection[0] * direction[0]) + (reflection[1] * direction[1]) + (reflection[2] * direction[2]);
			float factor = sign;
			if (l.smode == 0)
			{
				facing = std::max(facing, 0.0f);
				factor *= (facing > 0.0f) ? std::min(std::pow(facing, m_gmp_gloss), 1.0f) : 0.0f;
			}
			else if (l.smode == 1)
			{
				facing = std::abs(facing);
				factor *= (facing > 0.0f) ? std::min(std::pow(facing, m_gmp_gloss), 1.0f) : 0.0f;
			}

			if (alpha)
			{
				specular[0] += l.color[0] * factor;
			}
			else if (factor != 0.0f)
			{
				float *const target = (l.routing & ROUTE_SPECULAR_OFFSET) ? specular : diffuse;
				for (int i = 0; i < 3; i++)
				{
					target[i + 1] += l.color[i] * factor * v.offs[i + 1];
				}
			}
		}
	}

	for (int i = 0; i < 3; i++)
	{
		diffuse[i + 1] += BIT(m_light_model_flags, 5) ? (m_ambient_base[i] * v.base[i + 1]) : m_ambient_base[i];
		specular[i + 1] += BIT(m_light_model_flags, 6) ? (m_ambient_offset[i] * v.offs[i + 1]) : m_ambient_offset[i];
	}
	diffuse[0] += v.base[0];
	specular[0] += v.offs[0];

	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_light_model_flags, 9))
		{
			// what doesn't fit in the base color goes to the offset color
			specular[i] += std::max(diffuse[i] - 1.0f, 0.0f);
		}
		v.base[i] = std::clamp(diffuse[i], 0.0f, 1.0f);
		v.offs[i] = std::clamp(specular[i], 0.0f, 1.0f);
	}
}

void sega_315_6289_device::project(const float view[3], float screen[3]) const
{
	const float w = -view[2];

	screen[0] = ((-m_projection[0] * view[0]) - (m_projection[1] * view[2])) / w;
	screen[1] = ((m_projection[2] * view[1]) - (m_projection[3] * view[2])) / w;
	screen[2] = 1.0f / w;
}

void sega_315_6289_device::emit_vertex(const vertex &v, bool last)
{
	uint32_t data[16] = { };
	float screen[3];

	project(v.view, screen);
	data[0] = last ? 0xf0000000 : 0xe0000000;
	for (int i = 0; i < 3; i++)
	{
		data[i + 1] = f2u(screen[i]);
	}

	// floating point colors keep all of the precision
	if (m_textured)
	{
		data[4] = f2u(v.uv[0]);
		data[5] = f2u(v.uv[1]);
		for (int i = 0; i < 4; i++)
		{
			data[i + 8] = f2u(v.base[i]);
			data[i + 12] = f2u(v.offs[i]);
		}
		ta_w(data, 16);
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			data[i + 4] = f2u(v.base[i]);
		}
		ta_w(data, 8);
	}
}

int sega_315_6289_device::clip_triangle(const vertex &a, const vertex &b, const vertex &c, vertex result[4]) const
{
	const vertex *const in[3] = { &a, &b, &c };
	float dist[3];
	int count = 0;

	for (int i = 0; i < 3; i++)
	{
		dist[i] = -in[i]->view[2] - m_near;
	}
	for (int i = 0; i < 3; i++)
	{
		const int next = (i + 1) % 3;
		if (dist[i] >= 0.0f)
		{
			result[count++] = *in[i];
		}
		if ((dist[i] >= 0.0f) != (dist[next] >= 0.0f))
		{
			// the edge crosses the near plane
			const float t = dist[i] / (dist[i] - dist[next]);
			vertex &v = result[count++];
			for (int j = 0; j < 3; j++)
			{
				v.view[j] = in[i]->view[j] + ((in[next]->view[j] - in[i]->view[j]) * t);
			}
			for (int j = 0; j < 2; j++)
			{
				v.uv[j] = in[i]->uv[j] + ((in[next]->uv[j] - in[i]->uv[j]) * t);
			}
			for (int j = 0; j < 4; j++)
			{
				v.base[j] = in[i]->base[j] + ((in[next]->base[j] - in[i]->base[j]) * t);
				v.offs[j] = in[i]->offs[j] + ((in[next]->offs[j] - in[i]->offs[j]) * t);
			}
		}
	}

	return count;
}

void sega_315_6289_device::emit_triangle(const vertex &a, const vertex &b, const vertex &c)
{
	vertex clipped[4];
	const int count = clip_triangle(a, b, c, clipped);

	if (count >= 3)
	{
		// a strip goes around a quad in a Z
		emit_vertex(clipped[0], false);
		emit_vertex(clipped[1], false);
		if (count == 4)
		{
			emit_vertex(clipped[3], false);
		}
		emit_vertex(clipped[2], true);
	}
}

void sega_315_6289_device::flush_strip()
{
	const int count = m_strip.size();

	if (count >= 3)
	{
		int behind = 0;
		for (const vertex &v : m_strip)
		{
			if ((-v.view[2] - m_near) < 0.0f)
			{
				behind++;
			}
		}

		if (!behind)
		{
			for (int i = 0; i < count; i++)
			{
				emit_vertex(m_strip[i], i == (count - 1));
			}
		}
		else if (behind < count)
		{
			// every other triangle of a strip goes around the other way
			for (int i = 0; i < (count - 2); i++)
			{
				if (i & 1)
				{
					emit_triangle(m_strip[i + 1], m_strip[i], m_strip[i + 2]);
				}
				else
				{
					emit_triangle(m_strip[i], m_strip[i + 1], m_strip[i + 2]);
				}
			}
		}
	}
	m_strip.clear();
}

void sega_315_6289_device::flush_volume()
{
	const int count = m_strip.size();

	for (int i = 0; i < (count - 2); i++)
	{
		const vertex &a = m_strip[(i & 1) ? (i + 1) : i];
		const vertex &b = m_strip[(i & 1) ? i : (i + 1)];
		const vertex &c = m_strip[i + 2];
		vertex clipped[4];
		int edges = clip_triangle(a, b, c, clipped);

		if (!edges)
		{
			// the ones behind the near plane end up on it
			clipped[0] = a;
			clipped[1] = b;
			clipped[2] = c;
			for (int j = 0; j < 3; j++)
			{
				clipped[j].view[2] = -m_near;
			}
			edges = 3;
		}

		for (int j = 0; j < (edges - 2); j++)
		{
			uint32_t data[16] = { };
			float screen[3];

			data[0] = 0xe0000000;
			project(clipped[0].view, screen);
			std::transform(std::begin(screen), std::end(screen), &data[1], f2u);
			project(clipped[j + 1].view, screen);
			std::transform(std::begin(screen), std::end(screen), &data[4], f2u);
			project(clipped[j + 2].view, screen);
			std::transform(std::begin(screen), std::end(screen), &data[7], f2u);
			ta_w(data, 16);
		}
	}
	m_strip.clear();
}

uint32_t sega_315_6289_device::polygon(bool fifo, offs_t address)
{
	const uint32_t pcw = stream_r(fifo, address);
	const uint32_t isp = stream_r(fifo, address + 4);
	const uint32_t tsp = stream_r(fifo, address + 8) ^ m_model_tsp;
	const uint32_t tcw = stream_r(fifo, address + 12);
	const uint32_t format = stream_r(fifo, address + 24);
	const uint32_t count = stream_r(fifo, address + 28);
	const uint32_t stride = 16 + ((format & VTX_NORMAL) ? 16 : 0) + ((format & VTX_UV) ? 8 : 0) + ((format & VTX_COLOR) ? 8 : 0) + ((format & VTX_BUMP) ? 16 : 0);
	const uint32_t used = 32 + (stride * count);

	int list = m_pvr[0]->tafifo_listtype;
	if (list < 0)
	{
		list = BIT(pcw, 24, 3);
	}

	LOGMASKED(LOG_POLYGON, "polygon at %08x pcw %08x isp %08x tsp %08x tcw %08x format %03x, %d vertices, list %d\n", address, pcw, isp, tsp, tcw, format, count, list);
	if ((format & ~(VTX_NORMAL | VTX_UV | VTX_COLOR | VTX_BUMP)) != 0x002)
	{
		logerror("%s: unknown vertex format %08x\n", machine().describe_context(), format);
		return used;
	}
	if (!count || (count > 0x10000))
	{
		return used;
	}

	float nearest = -std::numeric_limits<float>::max();
	float farthest = std::numeric_limits<float>::max();
	for (uint32_t i = 0; i < count; i++)
	{
		float view[3];

		fetch_position(fifo, address + 32 + (i * stride), view);
		nearest = std::max(nearest, view[2]);
		farthest = std::min(farthest, view[2]);
	}
	const bool volume = list & 1;
	if (!volume && ((farthest > -m_near) || (nearest < -m_far) || std::isnan(nearest) || std::isnan(farthest)))
	{
		return used;
	}

	uint32_t data[8] = { };
	if (volume)
	{
		data[0] = 0x80000000 | (list << 24) | (pcw & 0x40);
		data[1] = isp & (m_open_volume ? 0xffffffff : 0xe7ffffff);
		ta_w(data, 8);
		m_textured = false;
		m_envmap = false;
	}
	else
	{
		m_envmap = m_gmp_valid && BIT(m_gmp_select, 11);
		m_textured = ((format & VTX_UV) && BIT(pcw, 3)) || m_envmap || (format & VTX_BUMP);

		const bool offset = m_textured && !m_envmap && (BIT(pcw, 2) || (format & VTX_BUMP));
		data[0] = 0x80000010 | (list << 24) | (m_user_clip << 16) | ((pcw ^ (m_shadow_volume ? 0x80 : 0x00)) & 0x80) | (m_textured ? 0x08 : 0x00) | (offset ? 0x04 : 0x00) | (pcw & 0x02);
		// TODO: culling is ours to do
		data[1] = isp & 0xe7ffffff;
		data[2] = m_envmap ? ((tsp | 0x00100000) & ~0x00080000) : tsp;
		data[3] = tcw;
		ta_w(data, 8);
	}

	vertex center, previous;
	bool start = true;

	m_strip.clear();
	for (uint32_t i = 0; i < count; i++)
	{
		const offs_t where = address + 32 + (i * stride);
		const uint32_t header = stream_r(fifo, where);
		vertex v = { };

		if (volume)
		{
			fetch_position(fifo, where, v.view);
		}
		else
		{
			fetch_vertex(fifo, where, format, v);
		}

		if (start)
		{
			center = v;
			start = false;
		}
		else if (!volume && (BIT(header, 29, 2) == 2))
		{
			// one more triangle of a fan
			flush_strip();
			m_strip.push_back(center);
			m_strip.push_back(previous);
		}
		m_strip.push_back(v);
		previous = v;

		if (BIT(header, 31))
		{
			if (volume)
			{
				flush_volume();
			}
			else
			{
				flush_strip();
			}
			start = true;
		}
	}
	if (volume)
	{
		flush_volume();
	}
	else
	{
		flush_strip();
	}

	return used;
}
