// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood

/*

Common code for the later Namco System 21 DSP board 5 TMS320C25 DSPs with custom Namco programming (marked C67) in a 1x Master, 4x Slave configuration

used by Star Blade, Cybersled

TODO: handle protection properly and with callbacks
      handle splitting of workload across slaves
      remove hacks!
      some of the list processing should probably be in the 3d device, split it out

*/

#include "emu.h"
#include "namcos21_dsp_c67.h"

DEFINE_DEVICE_TYPE(NAMCOS21_DSP_C67, namcos21_dsp_c67_device, "namcos21_dsp_c67_device", "Namco System 21 DSP Setup (5x C67 type)")

namcos21_dsp_c67_device::namcos21_dsp_c67_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCOS21_DSP_C67, tag, owner, clock),
	m_renderer(*this, finder_base::DUMMY_TAG),
	m_c67master(*this, "dspmaster"),
	m_c67slave(*this, "dspslave%u", 0U),
	m_ptrom24(*this,"point24"),
	m_master_dsp_ram(*this,"master_dsp_ram"),
	m_gametype(0),
	m_yield_hack_cb(*this),
	m_irq_enable(false)
{
}

void namcos21_dsp_c67_device::device_start()
{
	m_dspram16 = std::make_unique<uint16_t []>(0x10000/2); // 0x8000 16-bit words
	std::fill_n(m_dspram16.get(), 0x10000/2, 0x0000);

	m_pointram = std::make_unique<uint8_t[]>(PTRAM_SIZE);
	m_mpDspState = std::make_unique<dsp_state>();

	save_pointer(NAME(m_dspram16), 0x10000/2);

	save_item(NAME(m_mpDspState->masterSourceAddr));
	save_item(NAME(m_mpDspState->slaveInputBuffer));
	save_item(NAME(m_mpDspState->slaveBytesAvailable));
	save_item(NAME(m_mpDspState->slaveBytesAdvertised));
	save_item(NAME(m_mpDspState->slaveInputStart));
	save_item(NAME(m_mpDspState->slaveOutputBuffer));
	save_item(NAME(m_mpDspState->slaveOutputSize));
	save_item(NAME(m_mpDspState->masterDirectDrawBuffer));
	save_item(NAME(m_mpDspState->masterDirectDrawSize));
	save_item(NAME(m_mpDspState->masterFinished));
	save_item(NAME(m_mpDspState->slaveActive));

	save_pointer(NAME(m_pointram), PTRAM_SIZE);
	save_item(NAME(m_pointram_idx));
	save_item(NAME(m_pointram_control));
	save_item(NAME(m_pointrom_idx));
	save_item(NAME(m_mPointRomMSB));
	save_item(NAME(m_mbPointRomDataAvailable));
	save_item(NAME(m_depthcue));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_mbNeedsKickstart));
}

void namcos21_dsp_c67_device::device_reset()
{
	m_poly_frame_width = m_renderer->get_width();
	m_poly_frame_height = m_renderer->get_height();

	/* DSP startup hacks */
	m_mbNeedsKickstart = (m_gametype == NAMCOS21_CYBERSLED) ? 200 : 20;

	/* Wipe the framebuffers */
	m_renderer->swap_and_clear_poly_framebuffer();
	m_renderer->swap_and_clear_poly_framebuffer();

	//reset_dsps(ASSERT_LINE);

	m_mpDspState->masterSourceAddr = 0;
	m_mpDspState->slaveBytesAvailable = 0;
	m_mpDspState->slaveBytesAdvertised = 0;
	m_mpDspState->slaveInputStart = 0;
	m_mpDspState->slaveOutputSize = 0;
	m_mpDspState->masterDirectDrawSize = 0;
	m_mpDspState->masterFinished = 0;
	m_mpDspState->slaveActive = 0;

	m_pointram_idx = 0;
	m_pointram_control = 0;
	m_pointrom_idx = 0;
	m_mPointRomMSB = 0;
	m_mbPointRomDataAvailable = 0;
	m_irq_enable = 0;

	// clear these?
	//m_depthcue[2][0x400];
	//m_pointram
	//m_mpDspState->slaveInputBuffer[DSP_BUF_MAX];
	//m_mpDspState->slaveOutputBuffer[DSP_BUF_MAX];
	//m_mpDspState->masterDirectDrawBuffer[256];
}

void namcos21_dsp_c67_device::reset_dsps(int state)
{
	if (m_c67master)
		m_c67master->set_input_line(INPUT_LINE_RESET, state);

	if (m_c67slave[0])
		m_c67slave[0]->set_input_line(INPUT_LINE_RESET, state);
}


void namcos21_dsp_c67_device::reset_kickstart()
{
	//printf( "dspkick=0x%x\n", data );
	namcos21_kickstart_hacks(true);
}

void namcos21_dsp_c67_device::device_add_mconfig(machine_config &config)
{
	namco_c67_device& dspmaster(NAMCO_C67(config, m_c67master, 24'000'000)); /* 24 MHz? overclocked */
	dspmaster.set_addrmap(AS_PROGRAM, &namcos21_dsp_c67_device::master_dsp_program);
	dspmaster.set_addrmap(AS_DATA, &namcos21_dsp_c67_device::master_dsp_data);
	dspmaster.set_addrmap(AS_IO, &namcos21_dsp_c67_device::master_dsp_io);
	dspmaster.hold_in_cb().set_constant(0);
	dspmaster.hold_ack_out_cb().set_nop();
	dspmaster.xf_out_cb().set(FUNC(namcos21_dsp_c67_device::dsp_xf_w));

	for (int i = 0; i < 4; i++)
	{
		namco_c67_device& dspslave(NAMCO_C67(config, m_c67slave[i], 24'000'000)); /* 24 MHz? overclocked */
		dspslave.set_addrmap(AS_PROGRAM, &namcos21_dsp_c67_device::slave_dsp_program);
		dspslave.set_addrmap(AS_DATA, &namcos21_dsp_c67_device::slave_dsp_data);
		dspslave.set_addrmap(AS_IO, &namcos21_dsp_c67_device::slave_dsp_io);
		dspslave.hold_in_cb().set_constant(0);
		dspslave.hold_ack_out_cb().set_nop();
		dspslave.xf_out_cb().set(FUNC(namcos21_dsp_c67_device::slave_XF_output_w));

		// the emulation currently only uses one slave DSP clocked at 4x the normal rate instead of the master splitting the workload across the 4 slaves
		if (i != 0)
			dspslave.set_disable();
		else
			dspslave.set_clock(24'000'000*4);
	}
}


void namcos21_dsp_c67_device::dspcuskey_w(uint16_t data)
{ /* TODO: proper cuskey emulation */
}

uint16_t namcos21_dsp_c67_device::dspcuskey_r()
{
	uint16_t result = 0;

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
		case 0x8070: result = 0x016A; break;
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

void namcos21_dsp_c67_device::transmit_word_to_slave(uint16_t data)
{
	unsigned offs = m_mpDspState->slaveInputStart+m_mpDspState->slaveBytesAvailable++;

	m_mpDspState->slaveInputBuffer[offs%DSP_BUF_MAX] = data;

	if (ENABLE_LOGGING)
		logerror("+%04x(#%04x)\n", data, m_mpDspState->slaveBytesAvailable);

	m_mpDspState->slaveActive = 1;

	if (m_mpDspState->slaveBytesAvailable >= DSP_BUF_MAX)
		fatalerror("IDC overflow\n");
}

void namcos21_dsp_c67_device::transfer_dsp_data()
{
	uint16_t addr = m_mpDspState->masterSourceAddr;
	bool const mode = BIT(addr, 15);

	addr &= 0x7fff;
	if (!addr)
		return;

	while (true)
	{
		uint16_t const old = addr;
		uint16_t const code = m_dspram16[addr++];

		if (code == 0xffff)
		{
			if (mode)
			{
				addr = m_dspram16[addr];
				m_mpDspState->masterSourceAddr = addr;

				if (ENABLE_LOGGING)
					logerror("LOOP:0x%04x\n", addr);

				addr &= 0x7fff;
				if (old == addr)
					return;
			}
			else
			{
				m_mpDspState->masterSourceAddr = 0;
				return;
			}
		}
		else if (!mode)
		{
			// direct data transfer
			if (ENABLE_LOGGING)
				logerror("DATA TFR(0x%x)\n", code);

			transmit_word_to_slave(code);

			for (int i = 0; i < code; i++)
			{
				uint16_t const data = m_dspram16[addr++];
				transmit_word_to_slave(data);
			}
		}
		else if (code == 0x18 || code == 0x1a)
		{
			if (ENABLE_LOGGING)
				logerror("HEADER TFR(0x%x)\n", code);

			transmit_word_to_slave(code + 1);

			for (int i = 0; i < code; i++)
			{
				uint16_t const data = m_dspram16[addr++];
				transmit_word_to_slave(data);
			}
		}
		else
		{
			if (ENABLE_LOGGING)
				logerror("OBJ TFR(0x%x)\n", code);

			int32_t masterAddr = read_pointrom_data(code);
			uint16_t const len = m_dspram16[addr++];
			while (true)
			{
				int subAddr = read_pointrom_data(masterAddr++);
				if (subAddr == 0xffffff)
					break;

				uint16_t const primWords = (uint16_t)read_pointrom_data(subAddr++);
				// TODO: this function causes an IDC overflow in Solvalou, something else failed prior to that?
				// In Header TFR when bad parameters happens there's a suspicious 0x000f 0x0003 as first two words,
				// maybe it's supposed to have a different length there ...
				// cfr: object code 0x17 in service mode
				if (primWords > 2)
				{
					transmit_word_to_slave(0); // pad1
					transmit_word_to_slave(len + 1);

					for (int i = 0; i < len; i++)
					{ // transform
						transmit_word_to_slave(m_dspram16[addr + i]);
					}

					transmit_word_to_slave(0); // pad2
					transmit_word_to_slave(primWords + 1);
					
					for (int i = 0; i < primWords; i++)
					{
						transmit_word_to_slave((uint16_t)read_pointrom_data(subAddr + i));
					}
				}
				else if (ENABLE_LOGGING)
				{
					logerror("TFR NOP?\n");
				}
			}

			addr += len;
		}
	}
}



void namcos21_dsp_c67_device::namcos21_kickstart_hacks(bool internal)
{
	/* patch dsp watchdog */
	switch (m_gametype)
	{
	case namcos21_dsp_c67_device::NAMCOS21_AIRCOMBAT:
		m_master_dsp_ram[0x008e] = 0x808f;
		break;

	case namcos21_dsp_c67_device::NAMCOS21_SOLVALOU:
		m_master_dsp_ram[0x008b] = 0x808c;
		break;

	default:
		break;
	}

	if (internal)
	{
		if (m_mbNeedsKickstart == 0)
			return;

		m_mbNeedsKickstart--;
		
		if (m_mbNeedsKickstart)
			return;
	}

	m_renderer->swap_and_clear_poly_framebuffer();

	m_mpDspState->masterSourceAddr = 0;
	m_mpDspState->slaveOutputSize = 0;
	m_mpDspState->masterFinished = 0;
	m_mpDspState->slaveActive = 0;
	
	m_c67master->set_input_line(0, HOLD_LINE);
	m_c67slave[0]->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

uint16_t namcos21_dsp_c67_device::read_word_from_slave_input()
{
	uint16_t data = 0;

	if (m_mpDspState->slaveBytesAvailable > 0)
	{
		data = m_mpDspState->slaveInputBuffer[m_mpDspState->slaveInputStart++];

		m_mpDspState->slaveInputStart %= DSP_BUF_MAX;
		m_mpDspState->slaveBytesAvailable--;
		
		if (m_mpDspState->slaveBytesAdvertised > 0)
			m_mpDspState->slaveBytesAdvertised--;

		if (ENABLE_LOGGING)
			logerror("%s:-%04x(0x%04x)\n", machine().describe_context(), data, m_mpDspState->slaveBytesAvailable);
	}

	return data;
}

uint16_t namcos21_dsp_c67_device::get_input_bytes_advertised_for_slave()
{
	if (m_mpDspState->slaveBytesAdvertised < m_mpDspState->slaveBytesAvailable)
	{
		m_mpDspState->slaveBytesAdvertised++;
	}
	else if(m_mpDspState->slaveActive && m_mpDspState->masterFinished && m_mpDspState->masterSourceAddr)
	{
		namcos21_kickstart_hacks(false);
	}

	return m_mpDspState->slaveBytesAdvertised;
}

uint16_t namcos21_dsp_c67_device::dspram16_r(offs_t offset)
{
	return m_dspram16[offset];
}

void namcos21_dsp_c67_device::dspram16_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dspram16[offset]);

	if (m_mpDspState->masterSourceAddr && offset == 1 + (m_mpDspState->masterSourceAddr & 0x7fff))
	{
		if (ENABLE_LOGGING)
			logerror("IDC-CONTINUE\n");

		transfer_dsp_data();
	}
	else if (m_gametype == NAMCOS21_SOLVALOU && offset == 0x103)
	{
		// HACK: synchronization for solvalou - is this really needed?
		m_yield_hack_cb(1);
	}
}

void namcos21_dsp_c67_device::dspram16_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dspram16[offset]);

	if (m_mpDspState->masterSourceAddr && offset == 1 + (m_mpDspState->masterSourceAddr & 0x7fff))
	{
		if (ENABLE_LOGGING)
			logerror("IDC-CONTINUE\n");

		transfer_dsp_data();
	}
}

/***********************************************************/

int32_t namcos21_dsp_c67_device::read_pointrom_data(unsigned offset)
{
	return m_ptrom24[offset & 0xfffff];
}


uint16_t namcos21_dsp_c67_device::dsp_port0_r()
{
	int32_t data = read_pointrom_data(m_pointrom_idx++);

	m_mPointRomMSB = (uint8_t)(data>>16);
	m_mbPointRomDataAvailable = 1;

	return (uint16_t)data;
}

void namcos21_dsp_c67_device::dsp_port0_w(uint16_t data)
{ /* unused? */
	if (ENABLE_LOGGING)
		logerror("PTRAM_LO(0x%04x)\n", data);
}

uint16_t namcos21_dsp_c67_device::dsp_port1_r()
{
	if (m_mbPointRomDataAvailable)
	{
		m_mbPointRomDataAvailable = 0;
		return m_mPointRomMSB;
	}

	return 0x8000; /* IDC ack? */
}

void namcos21_dsp_c67_device::dsp_port1_w(uint16_t data)
{ /* unused? */
	if (ENABLE_LOGGING)
		logerror("PTRAM_HI(0x%04x)\n", data);
}

uint16_t namcos21_dsp_c67_device::dsp_port2_r()
{ /* IDC TRANSMIT ENABLE? */
	return 0;
}

void namcos21_dsp_c67_device::dsp_port2_w(uint16_t data)
{
	if (ENABLE_LOGGING)
		logerror("IDC ADDR INIT(0x%04x)\n", data);

	m_mpDspState->masterSourceAddr = data;
	transfer_dsp_data();
}

uint16_t namcos21_dsp_c67_device::dsp_port3_idc_rcv_enable_r()
{ /* IDC RECEIVE ENABLE? */
	return 0;
}

void namcos21_dsp_c67_device::dsp_port3_w(uint16_t data)
{
	m_pointrom_idx <<= 16;
	m_pointrom_idx |= data;
}

void namcos21_dsp_c67_device::dsp_port4_w(uint16_t data)
{ /* receives $0B<<4 prior to IDC setup */
}

uint16_t namcos21_dsp_c67_device::dsp_port8_r()
{ /* SMU status */
	return 1;
}


void namcos21_dsp_c67_device::dsp_port8_w(uint16_t data)
{
	if (ENABLE_LOGGING)
		logerror("port8_w(%d)\n", data);

	if (data)
		m_mpDspState->masterFinished = 1;

	m_irq_enable = data;
}

uint16_t namcos21_dsp_c67_device::dsp_port9_r()
{ /* render-device-busy; used for direct-draw */
	return 0;
}

uint16_t namcos21_dsp_c67_device::dsp_porta_r()
{ /* config */
	return 0;
}

void namcos21_dsp_c67_device::dsp_porta_w(uint16_t data)
{
	/* boot: 1 */
	/* IRQ0 end: 0 */
	/* INT2 begin: 1 */
	/* direct-draw begin: 0 */
	/* INT1 begin: 1 */
//  if (ENABLE_LOGGING) logerror( "dsp_porta_w(0x%04x)\n", data );
}

uint16_t namcos21_dsp_c67_device::dsp_portb_r()
{ /* config */
	return 1;
}

void namcos21_dsp_c67_device::dsp_portb_w(uint16_t data)
{
	if (data==0)
	{ /* only 0->1 transition triggers */
		return;
	}

	if (m_mpDspState->masterDirectDrawSize == 13)
	{
		int sx[4], sy[4], zcode[4];
		uint16_t color  = m_mpDspState->masterDirectDrawBuffer[0];

		for (int i=0; i<4; i++)
		{
			sx[i] = m_poly_frame_width/2 + (int16_t)m_mpDspState->masterDirectDrawBuffer[i*3+1];
			sy[i] = m_poly_frame_height/2 + (int16_t)m_mpDspState->masterDirectDrawBuffer[i*3+2];
			zcode[i] = m_mpDspState->masterDirectDrawBuffer[i*3+3];
		}

		if (BIT(color, 15))
			m_renderer->draw_quad(sx, sy, zcode, color);
		else
			logerror("indirection used w/ direct draw?\n");
	}
	else if (m_mpDspState->masterDirectDrawSize)
	{
		logerror("unexpected masterDirectDrawSize=%d!\n", m_mpDspState->masterDirectDrawSize);
	}

	m_mpDspState->masterDirectDrawSize = 0;
}

void namcos21_dsp_c67_device::dsp_portc_w(uint16_t data)
{
	if (m_mpDspState->masterDirectDrawSize < DSP_BUF_MAX)
	{
		m_mpDspState->masterDirectDrawBuffer[m_mpDspState->masterDirectDrawSize++] = data;
	}
	else
	{
		logerror("portc overflow\n");
	}
}

uint16_t namcos21_dsp_c67_device::dsp_portf_r()
{ /* informs BIOS that this is Master DSP */
	return 0;
}

void namcos21_dsp_c67_device::dsp_xf_w(uint16_t data)
{
	if (ENABLE_LOGGING)
		logerror("xf(%d)\n",data);
}

void namcos21_dsp_c67_device::master_dsp_program(address_map &map)
{
	map(0x8000, 0xbfff).ram().share("master_dsp_ram");
}

void namcos21_dsp_c67_device::master_dsp_data(address_map &map)
{
	map(0x2000, 0x200f).rw(FUNC(namcos21_dsp_c67_device::dspcuskey_r), FUNC(namcos21_dsp_c67_device::dspcuskey_w));
	map(0x8000, 0xffff).rw(FUNC(namcos21_dsp_c67_device::dspram16_r), FUNC(namcos21_dsp_c67_device::dspram16_w)); /* 0x8000 words */
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

void namcos21_dsp_c67_device::render_slave_output(uint16_t data)
{
	if (m_mpDspState->slaveOutputSize >= 4096)
		fatalerror("SLAVE OVERFLOW (0x%x)\n", m_mpDspState->slaveOutputBuffer[0]);

	/* append word to slave output buffer */
	m_mpDspState->slaveOutputBuffer[m_mpDspState->slaveOutputSize++] = data;

	uint16_t *pSource = m_mpDspState->slaveOutputBuffer;
	uint16_t count = *pSource++;

	if (count && m_mpDspState->slaveOutputSize > count)
	{
		uint16_t color = *pSource++;
		int sx[4], sy[4], zcode[4];

		if (BIT(color, 15))
		{
			if (count != 13) 
				logerror("?!direct-draw(%d)\n", count);

			for(int j=0; j<4; j++)
			{
				sx[j] = m_poly_frame_width/2 + (int16_t)pSource[j*3 + 0];
				sy[j] = m_poly_frame_height/2 + (int16_t)pSource[j*3 + 1];
				zcode[j] = pSource[j*3 + 2];
			}

			m_renderer->draw_quad(sx, sy, zcode, color & 0x7fff);
		}
		else
		{
			uint8_t code;
			int quad_idx = color * 6;

			do
			{
				code = m_pointram[quad_idx++];
				color = m_pointram[quad_idx++] | (code << 8);

				for(int j=0; j<4; j++)
				{
					uint8_t vi = m_pointram[quad_idx++];
					sx[j] = m_poly_frame_width/2  + (int16_t)pSource[vi*3 + 0];
					sy[j] = m_poly_frame_height/2 + (int16_t)pSource[vi*3 + 1];
					zcode[j] = pSource[vi*3 + 2];
				}

				m_renderer->draw_quad(sx, sy, zcode, color & 0x7fff);
			} while (!BIT(code, 7)); //Reached end-of-quadlist marker?
		}

		m_mpDspState->slaveOutputSize = 0;
	}
	else if (count == 0)
	{
		fatalerror("RenderSlaveOutput\n");
	}
}

uint16_t namcos21_dsp_c67_device::slave_port0_r()
{
	return read_word_from_slave_input();
}

void namcos21_dsp_c67_device::slave_port0_w(uint16_t data)
{
	render_slave_output(data);
}

uint16_t namcos21_dsp_c67_device::slave_port2_r()
{
	return get_input_bytes_advertised_for_slave();
}

uint16_t namcos21_dsp_c67_device::slave_port3_r()
{ /* render-device queue size */
	/* up to 0x1fe bytes?
	 * slave blocks until free &space exists
	 */
	return 0;
}

void namcos21_dsp_c67_device::slave_port3_w(uint16_t data)
{ /* 0=busy, 1=ready? */
}

void namcos21_dsp_c67_device::slave_XF_output_w(uint16_t data)
{
	if (ENABLE_LOGGING)
		logerror("%s :slaveXF(%d)\n", machine().describe_context(), data);
}

uint16_t namcos21_dsp_c67_device::slave_portf_r()
{ /* informs BIOS that this is Slave DSP */
	return 1;
}

void namcos21_dsp_c67_device::slave_dsp_program(address_map &map)
{
	map(0x8000, 0x8fff).ram();
}

void namcos21_dsp_c67_device::slave_dsp_data(address_map &map)
{
	/* no external data memory */
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
void namcos21_dsp_c67_device::pointram_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
//  uint16_t prev = m_pointram_control;
	COMBINE_DATA(&m_pointram_control);

	/* m_pointram_control&0x20 : bank for depthcue data */
#if 0
	logerror( "%s dsp_control_w:[%x]:=%04x ",
		machine().describe_context(),
		offset,
		m_pointram_control );

	uint16_t delta = (prev^m_pointram_control)&m_pointram_control;
	if( delta&0x10 )
	{
		logerror( " [reset]" );
	}
	if( delta&2 )
	{
		logerror( " send(A)%x", m_pointram_control&1 );
	}
	if( delta&4 )
	{
		logerror( " send(B)%x", m_pointram_control&1 );
	}
	if( delta&8 )
	{
		logerror( " send(C)%x", m_pointram_control&1 );
	}
	logerror( "\n" );
#endif
	m_pointram_idx = 0; /* HACK */
}

uint16_t namcos21_dsp_c67_device::pointram_data_r()
{
	return m_pointram[m_pointram_idx];
}

void namcos21_dsp_c67_device::pointram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
//      if( (m_pointram_idx%6)==0 ) logerror("\n" );
//      logerror( " %02x", data );
		m_pointram[m_pointram_idx++] = data;
		m_pointram_idx &= (PTRAM_SIZE - 1);
	}
}


uint16_t namcos21_dsp_c67_device::namcos21_depthcue_r(offs_t offset)
{
	unsigned bank = BIT(m_pointram_control, 5);
	return m_depthcue[bank][offset];
}
void namcos21_dsp_c67_device::namcos21_depthcue_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		unsigned bank = BIT(m_pointram_control, 5);
		m_depthcue[bank][offset] = data;
//      if( (offset&0xf)==0 ) logerror( "\n depthcue: " );
//      logerror( " %02x", data );
	}
}
