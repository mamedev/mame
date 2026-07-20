// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood
/*

Common code for the later Namco System 21 DSP board 5 TMS320C25 DSPs with custom
Namco programming (marked C67) in a 1x Master, 4x Slave configuration

used by Star Blade, Solvalou, Cybersled, Air Combat

TODO:
- add DSP idle loop speedup hacks?
- handle protection properly and with callbacks
- handle splitting of workload across slaves
- remove hacks!

*/

#include "emu.h"
#include "namcos21_dsp_c67.h"

#define ENABLE_LOGGING      0

DEFINE_DEVICE_TYPE(NAMCOS21_DSP_C67, namcos21_dsp_c67_device, "namcos21_dsp_c67_device", "Namco System 21 DSP Setup (5x C67 type)")

namcos21_dsp_c67_device::namcos21_dsp_c67_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NAMCOS21_DSP_C67, tag, owner, clock),
	m_renderer(*this, finder_base::DUMMY_TAG),
	m_c67master(*this, "dspmaster"),
	m_c67slave(*this, "dspslave%u", 0U),
	m_ptrom24(*this, "point24"),
	m_master_dsp_ram(*this, "master_dsp_ram"),
	m_gametype(0)
{
}

void namcos21_dsp_c67_device::device_start()
{
	assert(m_ptrom24.length() == 0x100000);

	m_dspram16 = make_unique_clear<u16 []>(0x10000/2); // 0x8000 16-bit words
	save_pointer(NAME(m_dspram16), 0x10000/2);

	assert((PTRAM_SIZE & (PTRAM_SIZE - 1)) == 0);
	m_pointram = make_unique_clear<u8[]>(PTRAM_SIZE);
	save_pointer(NAME(m_pointram), PTRAM_SIZE);

	memset(m_depthcue, 0, sizeof(m_depthcue));

	m_dsp_state = std::make_unique<dsp_state>();

	save_item(NAME(m_dsp_state->master_port_data));
	save_item(NAME(m_dsp_state->master_source_address));
	save_item(NAME(m_dsp_state->master_ddraw_buffer));
	save_item(NAME(m_dsp_state->master_ddraw_size));
	save_item(NAME(m_dsp_state->master_finished));

	save_item(NAME(m_dsp_state->slave_input_buffer));
	save_item(NAME(m_dsp_state->slave_bytes_available));
	save_item(NAME(m_dsp_state->slave_bytes_advertised));
	save_item(NAME(m_dsp_state->slave_input_start));
	save_item(NAME(m_dsp_state->slave_output_buffer));
	save_item(NAME(m_dsp_state->slave_output_size));
	save_item(NAME(m_dsp_state->slave_active));

	save_item(NAME(m_pointram_idx));
	save_item(NAME(m_pointram_control));
	save_item(NAME(m_pointrom_idx));
	save_item(NAME(m_point_data));
	save_item(NAME(m_point_data_available));
	save_item(NAME(m_depthcue));
	save_item(NAME(m_need_kickstart));
}

void namcos21_dsp_c67_device::device_reset()
{
	// Wipe the framebuffers
	m_renderer->swap_and_clear_poly_framebuffer();
	m_renderer->swap_and_clear_poly_framebuffer();

	m_dsp_state->master_source_address = 0;
	m_dsp_state->slave_bytes_available = 0;
	m_dsp_state->slave_bytes_advertised = 0;
	m_dsp_state->slave_input_start = 0;
	m_dsp_state->slave_output_size = 0;
	m_dsp_state->master_ddraw_size = 0;
	m_dsp_state->master_finished = 0;
	m_dsp_state->slave_active = 0;

	m_pointram_idx = 0;
	m_pointram_control = 0;
	m_pointrom_idx = 0;
	m_point_data = 0;
	m_point_data_available = 0;
	m_need_kickstart = true;
}

void namcos21_dsp_c67_device::reset_dsps(int state)
{
	// starblad/solvalou exiting service mode requires this otherwise 3d becomes non-functional
	if (state)
		this->reset();

	if (m_c67master)
		m_c67master->set_input_line(INPUT_LINE_RESET, state);

	if (m_c67slave[0])
		m_c67slave[0]->set_input_line(INPUT_LINE_RESET, state);
}


void namcos21_dsp_c67_device::reset_kickstart()
{
	if (!m_need_kickstart)
		return;
	m_need_kickstart = false;

	namcos21_kickstart();
}

void namcos21_dsp_c67_device::device_add_mconfig(machine_config &config)
{
	NAMCO_C67(config, m_c67master, 40_MHz_XTAL);
	m_c67master->set_addrmap(AS_PROGRAM, &namcos21_dsp_c67_device::master_dsp_program);
	m_c67master->set_addrmap(AS_DATA, &namcos21_dsp_c67_device::master_dsp_data);
	m_c67master->set_addrmap(AS_IO, &namcos21_dsp_c67_device::master_dsp_io);
	m_c67master->xf_out_cb().set(FUNC(namcos21_dsp_c67_device::dsp_xf_w));

	for (int i = 0; i < 4; i++)
	{
		NAMCO_C67(config, m_c67slave[i], 40_MHz_XTAL);
		m_c67slave[i]->set_addrmap(AS_PROGRAM, &namcos21_dsp_c67_device::slave_dsp_program);
		m_c67slave[i]->set_addrmap(AS_DATA, &namcos21_dsp_c67_device::slave_dsp_data);
		m_c67slave[i]->set_addrmap(AS_IO, &namcos21_dsp_c67_device::slave_dsp_io);
		m_c67slave[i]->hold_in_cb().set_constant(0);
		m_c67slave[i]->hold_ack_out_cb().set_nop();
		m_c67slave[i]->xf_out_cb().set(FUNC(namcos21_dsp_c67_device::slave_xf_output_w));

		// instead of the master splitting the workload across the 4 slaves, the emulation
		// currently only uses one slave DSP clocked at 4x the normal rate
		if (i != 0)
			m_c67slave[i]->set_disable();
		else
			m_c67slave[i]->set_clock(m_c67slave[i]->clock() * 4);
	}
}


void namcos21_dsp_c67_device::dspcuskey_w(u16 data)
{
	// TODO: proper cuskey emulation
}

u16 namcos21_dsp_c67_device::dspcuskey_r()
{
	u16 result = 0;
	if (m_gametype == NAMCOS21_SOLVALOU)
	{
		switch (m_c67master->pc())
		{
		case 0x805e: result = 0x0000; break;
		case 0x805f: result = 0xfeba; break;
		case 0x8067: result = 0xffff; break;
		case 0x806e: result = 0x0145; break;
		default:
			logerror("unk cuskey_r; pc=0x%x\n", m_c67master->pc());
			break;
		}
	}
	else if (m_gametype == NAMCOS21_CYBERSLED)
	{
		switch (m_c67master->pc())
		{
		case 0x8061: result = 0xfe95; break;
		case 0x8069: result = 0xffff; break;
		case 0x8070: result = 0x016a; break;
		default:
			break;
		}
	}
	else if (m_gametype == NAMCOS21_AIRCOMBAT)
	{
		switch (m_c67master->pc())
		{
		case 0x8062: result = 0xfeb9; break;
		case 0x806a: result = 0xffff; break;
		case 0x8071: result = 0x0146; break;
		default:
			break;
		}
	}
	return result;
}

void namcos21_dsp_c67_device::transmit_word_to_slave(u16 data)
{
	u32 offs = m_dsp_state->slave_input_start + m_dsp_state->slave_bytes_available++;
	m_dsp_state->slave_input_buffer[offs % DSP_BUF_MAX] = data;

	if (ENABLE_LOGGING) logerror("+%04x(#%04x)\n", data, m_dsp_state->slave_bytes_available);

	m_dsp_state->slave_active = 1;
	if (m_dsp_state->slave_bytes_available >= DSP_BUF_MAX)
	{
		fatalerror("IDC overflow\n");
	}
}

void namcos21_dsp_c67_device::transfer_dsp_data(bool first)
{
	u16 addr = m_dsp_state->master_source_address;
	bool const mode = BIT(addr, 15);
	addr &= 0x7fff;

	if (addr)
	{
		for (;;)
		{
			u16 const old = addr;
			u16 const code = m_dspram16[addr];
			addr = (addr + 1) & 0x7fff;

			if (!mode)
			{
				if (code == 0xffff)
				{
					m_dsp_state->master_source_address = 0;
					return;
				}

				// direct data transfer
				if (ENABLE_LOGGING) logerror("DATA TFR(0x%x)\n", code);
				transmit_word_to_slave(code);
				for (int i = 0; i < code; i++)
				{
					u16 const data = m_dspram16[addr];
					addr = (addr + 1) & 0x7fff;
					transmit_word_to_slave(data);
				}
			}
			else if (code == 0xffff)
			{
				addr = m_dspram16[addr];
				m_dsp_state->master_source_address = addr;
				if (ENABLE_LOGGING) logerror("GOTO:0x%04x\n", addr);
				addr &= 0x7fff;

				// return after goto self
				if (old == addr)
					return;
				else
					continue;
			}
			else if (first)
			{
				if (ENABLE_LOGGING) logerror("HEADER TFR(0x%x)\n", code);
				transmit_word_to_slave(code + 1);
				for (int i = 0; i < code; i++)
				{
					u16 const data = m_dspram16[addr];
					addr = (addr + 1) & 0x7fff;
					transmit_word_to_slave(data);
				}
			}
			else
			{
				if (ENABLE_LOGGING) logerror("OBJ TFR(0x%x)\n", code);
				s32 masterAddr = read_pointrom_data(code);
				u16 const len = m_dspram16[addr];
				addr = (addr + 1) & 0x7fff;

				for (;;)
				{
					int subAddr = read_pointrom_data(masterAddr++);
					if (subAddr == 0xffffff)
					{
						break;
					}
					else
					{
						int const primwords = (u16)read_pointrom_data(subAddr++);
						if (primwords > 2)
						{
							transmit_word_to_slave(0); // pad1
							transmit_word_to_slave(len + 1);
							for (int i = 0; i < len; i++)
							{
								transmit_word_to_slave(m_dspram16[(addr + i) & 0x7fff]);
							}
							transmit_word_to_slave(0); // pad2
							transmit_word_to_slave(primwords + 1);
							for (int i = 0; i < primwords; i++)
							{
								transmit_word_to_slave((u16)read_pointrom_data(subAddr + i));
							}
						}
						else
						{
							if (ENABLE_LOGGING) logerror("TFR NOP?\n");
						}
					}
				}
				addr += len;
			}

			first = false;
		}
	}
}



void namcos21_dsp_c67_device::namcos21_kickstart()
{
	m_renderer->swap_and_clear_poly_framebuffer();

	m_dsp_state->master_source_address = 0;
	m_dsp_state->slave_output_size = 0;
	m_dsp_state->master_finished = 0;
	m_dsp_state->slave_active = 0;

	m_c67master->set_input_line(0, HOLD_LINE);
	m_c67slave[0]->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

u16 namcos21_dsp_c67_device::read_word_from_slave_input()
{
	u16 data = 0;

	if (m_dsp_state->slave_bytes_available > 0)
	{
		data = m_dsp_state->slave_input_buffer[m_dsp_state->slave_input_start];

		if (!machine().side_effects_disabled())
		{
			m_dsp_state->slave_input_start++;
			m_dsp_state->slave_input_start %= DSP_BUF_MAX;
			m_dsp_state->slave_bytes_available--;
			if (m_dsp_state->slave_bytes_advertised > 0)
			{
				m_dsp_state->slave_bytes_advertised--;
			}
			if (ENABLE_LOGGING) logerror("%s:-%04x(0x%04x)\n", machine().describe_context(), data, m_dsp_state->slave_bytes_available);
		}
	}

	return data;
}

u16 namcos21_dsp_c67_device::get_input_bytes_advertised_for_slave()
{
	if (!machine().side_effects_disabled())
	{
		if (m_dsp_state->slave_bytes_advertised < m_dsp_state->slave_bytes_available)
		{
			m_dsp_state->slave_bytes_advertised++;
		}
		else if (m_dsp_state->slave_active && m_dsp_state->master_finished && m_dsp_state->master_source_address)
		{
			namcos21_kickstart();
		}
	}

	return m_dsp_state->slave_bytes_advertised;
}

u16 namcos21_dsp_c67_device::dspram16_r(offs_t offset)
{
	return m_dspram16[offset];
}

void namcos21_dsp_c67_device::dspram16_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dspram16[offset]);

	if (m_dsp_state->master_source_address && offset == 1 + (m_dsp_state->master_source_address & 0x7fff))
	{
		if (ENABLE_LOGGING) logerror("IDC-CONTINUE\n");
		transfer_dsp_data(false);
	}
}

/***********************************************************/

s32 namcos21_dsp_c67_device::read_pointrom_data(u32 offset)
{
	return m_ptrom24[offset & 0xfffff];
}


u16 namcos21_dsp_c67_device::dsp_port0_r()
{
	m_point_data = read_pointrom_data(m_pointrom_idx);
	if (!machine().side_effects_disabled())
	{
		m_pointrom_idx++;
		m_point_data_available = 1;
	}

	return m_point_data & 0xffff;
}

void namcos21_dsp_c67_device::dsp_port0_w(u16 data)
{
	// unused?
	if (ENABLE_LOGGING) logerror("PTRAM_LO(0x%04x)\n", data);
}

u16 namcos21_dsp_c67_device::dsp_port1_r()
{
	if (m_point_data_available)
	{
		if (!machine().side_effects_disabled())
			m_point_data_available = 0;
		return m_point_data >> 16 & 0xff;
	}

	return 0x8000; // IDC ack?
}

void namcos21_dsp_c67_device::dsp_port1_w(u16 data)
{
	// unused?
	if (ENABLE_LOGGING) logerror("PTRAM_HI(0x%04x)\n", data);
}

u16 namcos21_dsp_c67_device::dsp_port2_r()
{
	// IDC TRANSMIT ENABLE?
	return 0;
}

void namcos21_dsp_c67_device::dsp_port2_w(u16 data)
{
	if (ENABLE_LOGGING) logerror("IDC ADDR INIT(0x%04x)\n", data);
	m_dsp_state->master_source_address = data;
	transfer_dsp_data(true);
}

u16 namcos21_dsp_c67_device::dsp_port3_idc_rcv_enable_r()
{
	// IDC RECEIVE ENABLE?
	return 0;
}

void namcos21_dsp_c67_device::dsp_port3_w(u16 data)
{
	m_pointrom_idx <<= 16;
	m_pointrom_idx |= data;
}

void namcos21_dsp_c67_device::dsp_port4_w(u16 data)
{
	// receives $0B<<4 prior to IDC setup
}

u16 namcos21_dsp_c67_device::dsp_port8_r()
{
	// SMU status
	return 1;
}

void namcos21_dsp_c67_device::dsp_port8_w(u16 data)
{
	if (ENABLE_LOGGING) logerror("port8_w(%d)\n", data);

	if (~m_dsp_state->master_port_data[8] & data & 1)
		m_dsp_state->master_finished = 1;

	m_dsp_state->master_port_data[8] = data;
}

u16 namcos21_dsp_c67_device::dsp_port9_r()
{
	// render-device-busy; used for direct-draw
	return 0;
}

u16 namcos21_dsp_c67_device::dsp_porta_r()
{
	// config
	return 0;
}

void namcos21_dsp_c67_device::dsp_porta_w(u16 data)
{
	// boot: 1
	// IRQ0 end: 0
	// INT2 begin: 1
	// direct-draw begin: 0
	// INT1 begin: 1
	//if (ENABLE_LOGGING) logerror("dsp_porta_w(0x%04x)\n", data);
}

u16 namcos21_dsp_c67_device::dsp_portb_r()
{
	// config
	return 1;
}

void namcos21_dsp_c67_device::dsp_portb_w(u16 data)
{
	// only 0->1 transition triggers
	if (~m_dsp_state->master_port_data[0xb] & data & 1)
	{
		if (m_dsp_state->master_ddraw_size == 13)
		{
			int color = m_dsp_state->master_ddraw_buffer[0];
			if (color & 0x8000)
			{
				m_renderer->draw_direct_quad(&m_dsp_state->master_ddraw_buffer[1], color);
			}
			else
			{
				logerror("indirection used w/ direct draw?\n");
			}
		}
		else if (m_dsp_state->master_ddraw_size)
		{
			logerror("unexpected master_ddraw_size=%d!\n", m_dsp_state->master_ddraw_size);
		}

		m_dsp_state->master_ddraw_size = 0;
	}

	m_dsp_state->master_port_data[0xb] = data;
}

void namcos21_dsp_c67_device::dsp_portc_w(u16 data)
{
	if (m_dsp_state->master_ddraw_size < DSP_BUF_MAX)
	{
		m_dsp_state->master_ddraw_buffer[m_dsp_state->master_ddraw_size++] = data;
	}
	else
	{
		logerror("portc overflow\n");
	}
}

u16 namcos21_dsp_c67_device::dsp_portf_r()
{
	// informs BIOS that this is Master DSP
	return 0;
}

void namcos21_dsp_c67_device::dsp_xf_w(u16 data)
{
	if (ENABLE_LOGGING) logerror("xf(%d)\n",data);
}

void namcos21_dsp_c67_device::master_dsp_program(address_map &map)
{
	map(0x8000, 0xbfff).ram().share("master_dsp_ram");
}

void namcos21_dsp_c67_device::master_dsp_data(address_map &map)
{
	map(0x2000, 0x200f).rw(FUNC(namcos21_dsp_c67_device::dspcuskey_r), FUNC(namcos21_dsp_c67_device::dspcuskey_w));
	map(0x8000, 0xffff).rw(FUNC(namcos21_dsp_c67_device::dspram16_r), FUNC(namcos21_dsp_c67_device::dspram16_w)); // 0x8000 words
}

void namcos21_dsp_c67_device::master_dsp_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(namcos21_dsp_c67_device::dsp_port0_r), FUNC(namcos21_dsp_c67_device::dsp_port0_w));
	map(0x01, 0x01).rw(FUNC(namcos21_dsp_c67_device::dsp_port1_r), FUNC(namcos21_dsp_c67_device::dsp_port1_w));
	map(0x02, 0x02).rw(FUNC(namcos21_dsp_c67_device::dsp_port2_r), FUNC(namcos21_dsp_c67_device::dsp_port2_w));
	map(0x03, 0x03).rw(FUNC(namcos21_dsp_c67_device::dsp_port3_idc_rcv_enable_r), FUNC(namcos21_dsp_c67_device::dsp_port3_w));
	map(0x04, 0x04).w(FUNC(namcos21_dsp_c67_device::dsp_port4_w));
	map(0x08, 0x08).rw(FUNC(namcos21_dsp_c67_device::dsp_port8_r), FUNC(namcos21_dsp_c67_device::dsp_port8_w));
	map(0x09, 0x09).r(FUNC(namcos21_dsp_c67_device::dsp_port9_r));
	map(0x0a, 0x0a).rw(FUNC(namcos21_dsp_c67_device::dsp_porta_r), FUNC(namcos21_dsp_c67_device::dsp_porta_w));
	map(0x0b, 0x0b).rw(FUNC(namcos21_dsp_c67_device::dsp_portb_r), FUNC(namcos21_dsp_c67_device::dsp_portb_w));
	map(0x0c, 0x0c).w(FUNC(namcos21_dsp_c67_device::dsp_portc_w));
	map(0x0f, 0x0f).r(FUNC(namcos21_dsp_c67_device::dsp_portf_r));
}

/************************************************************************************/

void namcos21_dsp_c67_device::render_slave_output(u16 data)
{
	if (m_dsp_state->slave_output_size >= 4096)
	{
		fatalerror("SLAVE OVERFLOW (0x%x)\n", m_dsp_state->slave_output_buffer[0]);
	}

	// append word to slave output buffer
	m_dsp_state->slave_output_buffer[m_dsp_state->slave_output_size++] = data;

	// draw quads
	u16 *source = m_dsp_state->slave_output_buffer;
	u16 count = *source++;

	if (count && m_dsp_state->slave_output_size > count)
	{
		u16 color = *source++;
		int size = 0;

		if (color & 0x8000)
		{
			m_renderer->draw_direct_quad(source, color);
			size = 12;
		}
		else
		{
			size = m_renderer->draw_quads(source, m_pointram.get(), PTRAM_SIZE, color);
		}

		if (count != size + 1)
			logerror("render_slave_output size mismatch (%d, expected %d)\n", size + 1, count);

		std::fill_n(m_dsp_state->slave_output_buffer, m_dsp_state->slave_output_size, 0xffff);
		m_dsp_state->slave_output_size = 0;
	}
	else if (count == 0)
	{
		fatalerror("render_slave_output\n");
	}
}

u16 namcos21_dsp_c67_device::slave_port0_r()
{
	return read_word_from_slave_input();
}

void namcos21_dsp_c67_device::slave_port0_w(u16 data)
{
	render_slave_output(data);
}

u16 namcos21_dsp_c67_device::slave_port2_r()
{
	return get_input_bytes_advertised_for_slave();
}

u16 namcos21_dsp_c67_device::slave_port3_r()
{
	// render-device queue size
	// up to 0x1fe bytes? slave blocks until free &space exists
	return 0;
}

void namcos21_dsp_c67_device::slave_port3_w(u16 data)
{
	// 0=busy, 1=ready?
}

void namcos21_dsp_c67_device::slave_xf_output_w(u16 data)
{
	if (ENABLE_LOGGING) logerror("%s :slave_xf(%d)\n", machine().describe_context(), data);
}

u16 namcos21_dsp_c67_device::slave_portf_r()
{
	// informs BIOS that this is Slave DSP
	return 1;
}

void namcos21_dsp_c67_device::slave_dsp_program(address_map &map)
{
	map(0x8000, 0x8fff).ram();
}

void namcos21_dsp_c67_device::slave_dsp_data(address_map &map)
{
	// no external data memory
}

void namcos21_dsp_c67_device::slave_dsp_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(namcos21_dsp_c67_device::slave_port0_r), FUNC(namcos21_dsp_c67_device::slave_port0_w));
	map(0x02, 0x02).r(FUNC(namcos21_dsp_c67_device::slave_port2_r));
	map(0x03, 0x03).rw(FUNC(namcos21_dsp_c67_device::slave_port3_r), FUNC(namcos21_dsp_c67_device::slave_port3_w));
	map(0x0f, 0x0f).r(FUNC(namcos21_dsp_c67_device::slave_portf_r));
}

/************************************************************************************/

/**
 * 801f->800f : prepare for master access to point ram
 * 801f       : done
 *
 * #bits  data  line
 *   8     1a0    4
 *   7     0f8    4
 *   7     0ff    4
 *   1     001    4
 *   7     00a    2
 *   a     0fe    8
 *
 * line   #bits  data
 * 0003   000A   000004FE
 * 0001   0007   0000000A
 * 0002   001A   03FFF1A0
 */
void namcos21_dsp_c67_device::pointram_control_w(offs_t offset, u16 data, u16 mem_mask)
{
	//u16 prev = m_pointram_control;
	COMBINE_DATA(&m_pointram_control);

	// m_pointram_control&0x20 : bank for depthcue data
#if 0
	logerror("%s dsp_control_w:[%x]:=%04x ",
		machine().describe_context(),
		offset,
		m_pointram_control);

	u16 delta = (prev^m_pointram_control)&m_pointram_control;
	if (delta & 0x10)
	{
		logerror(" [reset]");
	}
	if (delta & 2)
	{
		logerror(" send(A)%x", m_pointram_control & 1);
	}
	if (delta & 4)
	{
		logerror(" send(B)%x", m_pointram_control & 1);
	}
	if (delta & 8)
	{
		logerror(" send(C)%x", m_pointram_control & 1);
	}
	logerror("\n");
#endif
	m_pointram_idx = 0; // HACK
}

u16 namcos21_dsp_c67_device::pointram_data_r()
{
	return m_pointram[m_pointram_idx];
}

void namcos21_dsp_c67_device::pointram_data_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		//if ((m_pointram_idx%6) == 0) logerror("\n");
		//logerror(" %02x", data);
		m_pointram[m_pointram_idx] = data;
		m_pointram_idx = (m_pointram_idx + 1) & (PTRAM_SIZE - 1);
	}
}


u16 namcos21_dsp_c67_device::namcos21_depthcue_r(offs_t offset)
{
	int bank = (m_pointram_control & 0x20) ? 1 : 0;
	return m_depthcue[bank][offset];
}

void namcos21_dsp_c67_device::namcos21_depthcue_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		int bank = (m_pointram_control & 0x20) ? 1 : 0;
		m_depthcue[bank][offset] = data;
		//if ((offset & 0xf) == 0) logerror("\n depthcue: ");
		//logerror(" %02x", data);
	}
}
