// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood
/*

Common code for the original Namco System 21 DSP board, with a single DSP
used by Winning Run series, Driver's Eyes

TODO:
- handle protection properly and with callbacks
- how does the real hw send data to the renderer without telling the number of words
  it's going to send?

*/

#include "emu.h"
#include "namcos21_dsp.h"

DEFINE_DEVICE_TYPE(NAMCOS21_DSP, namcos21_dsp_device, "namcos21_dsp_device", "Namco System 21 DSP Setup (1x TMS320C25 type)")

namcos21_dsp_device::namcos21_dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NAMCOS21_DSP, tag, owner, clock),
	m_dsp(*this, "dsp"),
	m_dspbios(*this, "dspbios"),
	m_polydata(*this, "polydata"),
	m_ptrom16(*this, "point16"),
	m_renderer(*this, finder_base::DUMMY_TAG)
{
}

void namcos21_dsp_device::device_start()
{
	m_suspend_timer = timer_alloc(FUNC(namcos21_dsp_device::suspend_callback), this);

	m_pointrom_mask = m_ptrom16.length() - 1;
	assert((m_pointrom_mask & (m_pointrom_mask + 1)) == 0);

	m_pointram_idx = 0;
	m_pointram_control = 0;
	m_poly_index = 0;
	m_poly_size = 0;
	m_pointrom_addr = 0;
	m_dsp_busy = 0;
	m_dsp_complete = 0;

	std::fill(std::begin(m_dspcomram_control), std::end(m_dspcomram_control), 0);
	std::fill(std::begin(m_poly_buf), std::end(m_poly_buf), 0xffff);

	m_dspcomram = make_unique_clear<u16[]>(0x800*2);
	save_pointer(NAME(m_dspcomram), 0x800*2);

	assert((PTRAM_SIZE & (PTRAM_SIZE - 1)) == 0);
	m_pointram = make_unique_clear<u8[]>(PTRAM_SIZE);
	save_pointer(NAME(m_pointram), PTRAM_SIZE);

	save_item(NAME(m_pointram_idx));
	save_item(NAME(m_pointram_control));

	save_item(NAME(m_dspcomram_control));
	save_item(NAME(m_poly_buf));
	save_item(NAME(m_poly_index));
	save_item(NAME(m_poly_size));
	save_item(NAME(m_pointrom_addr));
	save_item(NAME(m_dsp_busy));
	save_item(NAME(m_dsp_complete));
}

TIMER_CALLBACK_MEMBER(namcos21_dsp_device::suspend_callback)
{
	m_dsp->suspend(SUSPEND_REASON_HALT, true);
}

void namcos21_dsp_device::device_reset()
{
	// can't suspend directly from here, needs to be on a timer?
	m_suspend_timer->adjust(attotime::zero);
	m_poly_index = 0;
}


u16 namcos21_dsp_device::dspcomram_r(offs_t offset)
{
	int bank = 1 - (m_dspcomram_control[0x4/2] & 1);
	u16 *mem = &m_dspcomram[0x800 * bank];
	return mem[offset];
}

void namcos21_dsp_device::dspcomram_w(offs_t offset, u16 data, u16 mem_mask)
{
	int bank = 1 - (m_dspcomram_control[0x4/2] & 1);
	u16 *mem = &m_dspcomram[0x800 * bank];
	COMBINE_DATA(&mem[offset]);
}

u16 namcos21_dsp_device::cuskey_r()
{
	int pc = m_dsp->pc();
	switch (pc)
	{
	case 0x0064: // winrun91
		return 0xfebb;
	case 0x006c: // winrun91
		return 0xffff;
	case 0x0073: // winrun91
		return 0x0144;

	case 0x0075: // winrun
		return 0x24;

	default:
		break;
	}
	return 0;
}

void namcos21_dsp_device::cuskey_w(u16 data)
{
}

void namcos21_dsp_device::dsp_render_w(u16 data)
{
	if (m_poly_index >= MAX_POLY_PARAM)
	{
		fatalerror("POLY_OVERFLOW\n");
	}

	// unlike namcos21_dsp_c67_device, it doesn't publish the number of words it's going to write,
	// but we still need to know it before sending data to the renderer
	if (m_poly_index == 0)
		m_poly_size = (m_dsp->state_int(TMS320C2X_AR1) + 1) * 3 + 1;

	m_poly_buf[m_poly_index++] = data;

	// draw quads
	if (m_poly_index == m_poly_size)
	{
		const u16 *source = m_poly_buf;
		u16 color = *source++;
		int size = 0;

		if (color & 0x8000)
		{
			// direct-draw
			m_renderer->draw_direct_quad(source, color);
			size = 12;
		}
		else
		{
			size = m_renderer->draw_quads(source, m_pointram.get(), PTRAM_SIZE, color);
		}

		if (m_poly_size != size + 1)
			logerror("dsp_render_w size mismatch (%d, expected %d)\n", size + 1, m_poly_size);

		std::fill_n(m_poly_buf, m_poly_index, 0xffff);
		m_poly_index = 0;
	}
}

void namcos21_dsp_device::dsp_pointrom_addr_w(offs_t offset, u16 data)
{
	if (offset == 0)
	{
		// port 8
		m_pointrom_addr = (m_pointrom_addr & ~0xffff) | data;
	}
	else
	{
		// port 9
		m_pointrom_addr = (m_pointrom_addr & 0xffff) | (data << 16);
	}
}

u16 namcos21_dsp_device::dsp_pointrom_data_r()
{
	u16 data = m_ptrom16[m_pointrom_addr & m_pointrom_mask];

	if (!machine().side_effects_disabled())
		m_pointrom_addr++;

	return data;
}

void namcos21_dsp_device::dsp_busy_w(u16 data)
{
	m_dsp_busy = data;
}

void namcos21_dsp_device::dsp_complete_w(u16 data)
{
	if (~m_dsp_complete & data & 1)
	{
		m_dsp->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		m_poly_index = 0;
	}

	m_dsp_complete = data;
}

u16 namcos21_dsp_device::table_r(offs_t offset)
{
	return m_polydata[offset];
}

void namcos21_dsp_device::dspbios_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dspbios[offset]);

	if (offset == 0xfff) // is this the real trigger?
		m_dsp->resume(SUSPEND_REASON_HALT);
}

u16 namcos21_dsp_device::m68k_dspcomram_r(offs_t offset)
{
	int bank = m_dspcomram_control[0x4/2] & 1;
	u16 *mem = &m_dspcomram[0x800 * bank];
	return mem[offset];
}

void namcos21_dsp_device::m68k_dspcomram_w(offs_t offset, u16 data, u16 mem_mask)
{
	int bank = m_dspcomram_control[0x4/2] & 1;
	u16 *mem = &m_dspcomram[0x800 * bank];
	COMBINE_DATA(&mem[offset]);
}

// 380000: read: dsp initialising? - 1 = busy
// 380000: write(0x01): done before dsp comram init
// 380004: write(bit 0): dspcomram bank, as seen by 68k
// 380008: read: dsp rendering status - 1 = busy

u16 namcos21_dsp_device::dspcomram_control_r(offs_t offset)
{
	if (offset == 0x8/2)
		return m_dsp_busy;

	return m_dspcomram_control[offset];
}

void namcos21_dsp_device::dspcomram_control_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 prev = m_dspcomram_control[offset];
	COMBINE_DATA(&m_dspcomram_control[offset]);

	// also swap poly framebuffer on bankswitch
	if ((prev ^ m_dspcomram_control[0x4/2]) & 1)
		m_renderer->swap_and_clear_poly_framebuffer();
}


void namcos21_dsp_device::dsp_program(address_map &map)
{
	// MCU is used in external program mode, program is uploaded to shared RAM by the 68k
	map(0x0000, 0x0fff).ram().share("dspbios");
}

void namcos21_dsp_device::dsp_data(address_map &map)
{
	map(0x2000, 0x200f).rw(FUNC(namcos21_dsp_device::cuskey_r), FUNC(namcos21_dsp_device::cuskey_w));
	map(0x4000, 0x4fff).rw(FUNC(namcos21_dsp_device::dspcomram_r), FUNC(namcos21_dsp_device::dspcomram_w));
	map(0x8000, 0xffff).r(FUNC(namcos21_dsp_device::table_r));
}

void namcos21_dsp_device::dsp_io(address_map &map)
{
	map(0x08, 0x09).rw(FUNC(namcos21_dsp_device::dsp_pointrom_data_r), FUNC(namcos21_dsp_device::dsp_pointrom_addr_w));
	map(0x0a, 0x0a).w(FUNC(namcos21_dsp_device::dsp_render_w));
	map(0x0b, 0x0b).w(FUNC(namcos21_dsp_device::dsp_busy_w));
	map(0x0c, 0x0c).w(FUNC(namcos21_dsp_device::dsp_complete_w));
}


void namcos21_dsp_device::device_add_mconfig(machine_config &config)
{
	TMS320C25(config, m_dsp, 40_MHz_XTAL); // 40 MHz oscillator on DSP board
	m_dsp->set_addrmap(AS_PROGRAM, &namcos21_dsp_device::dsp_program);
	m_dsp->set_addrmap(AS_DATA, &namcos21_dsp_device::dsp_data);
	m_dsp->set_addrmap(AS_IO, &namcos21_dsp_device::dsp_io);
	m_dsp->bio_in_cb().set_constant(0);
	m_dsp->xf_out_cb().set_nop();
}

void namcos21_dsp_device::pointram_control_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pointram_control);
	m_pointram_idx = 0; // HACK
}

u16 namcos21_dsp_device::pointram_data_r()
{
	return m_pointram[m_pointram_idx];
}

void namcos21_dsp_device::pointram_data_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_pointram[m_pointram_idx] = data;
		m_pointram_idx = (m_pointram_idx + 1) & (PTRAM_SIZE - 1);
	}
}
