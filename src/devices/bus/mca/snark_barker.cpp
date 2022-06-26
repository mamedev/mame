// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    Snark Barker MCA
	MCA ID @5085
	Using Sound Blaster DSP code 2.02

	https://github.com/schlae/sb-firmware/blob/master/sbv202.asm

	MCU code breakpoints:
	0x446: received a command
	0x44D: A <- command number

	0x4D7: cmdg_setup_e
	0x764: cmd_dac_dma

***************************************************************************/

#include "emu.h"
#include "snark_barker.h"

//#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

#define ym3812_StdClock XTAL(3'579'545)
#define ymf262_StdClock XTAL(14'318'181)
#define XTAL_DSP XTAL(12'000'000)	// XTL1

DEFINE_DEVICE_TYPE(MCA16_SNARK_BARKER, mca16_snark_barker_device, "mca_snark_barker", "TubeTime Snark Barker MCA (@5085)")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_snark_barker_device::device_add_mconfig(machine_config &config)
{
	I80C51(config, m_dsp, XTAL_DSP); // XTL1
	m_dsp->set_addrmap(AS_PROGRAM, &mca16_snark_barker_device::map_dsp_program);
	m_dsp->set_addrmap(AS_IO, &mca16_snark_barker_device::map_dsp_io);

	m_dsp->port_in_cb<0>().set(FUNC(mca16_snark_barker_device::dsp_port0_r));
	// No inputs on port 1
	m_dsp->port_in_cb<2>().set(FUNC(mca16_snark_barker_device::dsp_port2_r));
	m_dsp->port_in_cb<3>().set(FUNC(mca16_snark_barker_device::dsp_port3_r));
	m_dsp->port_out_cb<0>().set(FUNC(mca16_snark_barker_device::dsp_port0_w));
	m_dsp->port_out_cb<1>().set(m_dac, FUNC(dac_byte_interface::data_w));				// DSP port 1 is hooked right up to the DAC.
	m_dsp->port_out_cb<2>().set(FUNC(mca16_snark_barker_device::dsp_port2_w));
	m_dsp->port_out_cb<3>().set(FUNC(mca16_snark_barker_device::dsp_port3_w));

	PC_JOY(config, m_joy);

	SPEAKER(config, m_speaker).front_center();
	MC1408(config, m_dac, 0).add_route(ALL_OUTPUTS, m_speaker, 0.5);

	YM3812(config, m_ym3812, ym3812_StdClock);
	m_ym3812->add_route(ALL_OUTPUTS, m_speaker, 1.0);
	/* no CM/S support (empty sockets) */
}

//-------------------------------------------------
//  mca16_planar_lpt_device - constructor
//-------------------------------------------------

mca16_snark_barker_device::mca16_snark_barker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_snark_barker_device(mconfig, MCA16_SNARK_BARKER, tag, owner, clock)
{
}

mca16_snark_barker_device::mca16_snark_barker_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0x5085),
	m_dsp(*this, "dsp"),
	m_joy(*this, "pc_joy"),
	m_ym3812(*this, "ym3812"),
	m_dac(*this, "dac"),
	m_speaker(*this, "speaker")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_snark_barker_device::device_start()
{
    m_is_mapped = 0;
	set_mca_device();

	m_mca->install_device(0x0200, 0x0207, read8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_r)), write8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_w)));
	m_mca->install_device(0x0226, 0x0227, read8sm_delegate(*this, FUNC(mca16_snark_barker_device::dsp_reset_r)), write8sm_delegate(*this, FUNC(mca16_snark_barker_device::dsp_reset_w)));
	m_mca->install_device(0x022a, 0x022b, read8sm_delegate(*this, FUNC(mca16_snark_barker_device::dsp_data_r)), write8sm_delegate(*this, FUNC(mca16_snark_barker_device::dsp_data_w)));
	m_mca->install_device(0x022c, 0x022d, read8sm_delegate(*this, FUNC(mca16_snark_barker_device::dsp_wbuf_status_r)), write8sm_delegate(*this, FUNC(mca16_snark_barker_device::dsp_cmd_w)));
	m_mca->install_device(0x022e, 0x022f, read8sm_delegate(*this, FUNC(mca16_snark_barker_device::dsp_rbuf_status_r)), write8sm_delegate(*this, FUNC(mca16_snark_barker_device::dsp_rbuf_status_w)));

	m_mca->install_device(0x0388, 0x0389, read8sm_delegate(*this, FUNC(mca16_snark_barker_device::ym3812_16_r)), write8sm_delegate(*this, FUNC(mca16_snark_barker_device::ym3812_16_w)));
	m_mca->install_device(0x0228, 0x0229, read8sm_delegate(*this, FUNC(mca16_snark_barker_device::ym3812_16_r)), write8sm_delegate(*this, FUNC(mca16_snark_barker_device::ym3812_16_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_snark_barker_device::device_reset()
{
	if(m_is_mapped) unmap();
	m_is_mapped = 0;

	m_irq_in_flag = false;
	m_irequest = true;
	m_drequest = false;

	m_cur_dma_line = -1;
	m_cur_irq_line = -1;

	dsp_reset_w(0, 0);

	m_irq_raised = false;
	m_dma_raised = false;
}

uint8_t mca16_snark_barker_device::io8_r(offs_t offset)
{
    assert_card_feedback();
    LOG("%s: O:%02X\n", FUNCNAME, offset);
    return 0x00;
}

void mca16_snark_barker_device::io8_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    LOG("%s: O:%02X D:%02X\n", FUNCNAME, offset, data);
}

void mca16_snark_barker_device::pos_w(offs_t offset, uint8_t data)
{
	LOG("%s\n", FUNCNAME);

	switch(offset)
	{
		case 0:
			// Adapter Identification b0-b7
		case 1:
			// Adapter Identification b8-b15
			break;
		case 2:
			// Option Select Data 1
			break;
		case 3:
			update_pos(data);
			// Option Select Data 2
			break;
		case 4:
			// Option Select Data 3
			break;
		case 5:
			// Option Select Data 4
			break;
		case 6:
			// Subaddress Extension Low
			break;
		case 7:
			// Subaddress Extension High
			break;
		default:
			break;
	}
}

uint8_t mca16_snark_barker_device::get_pos_irq()
{
	uint8_t pos_irq = (m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] & 0b00011000) >> 3;

	switch(pos_irq)
	{
		case 0: pos_irq = 2; break;
		case 1:	pos_irq = 3; break;
		case 2: pos_irq = 5; break;
		case 3: pos_irq = 7; break;
		default: pos_irq = -1; break;
	}

	return pos_irq;
}

uint8_t mca16_snark_barker_device::get_pos_dma()
{
	uint8_t pos_dma = (m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] & 0b01100000) >> 5;

	switch(pos_dma)
	{
		case 0: pos_dma = 0; break;
		case 1: pos_dma = 1; break;
		case 2:	pos_dma = 3; break;
		default: pos_dma = -1; break;
	}

	return pos_dma;
}

void mca16_snark_barker_device::remap()
{
	// TODO: remap I/O addresses. IRQ line is handled separately.
	m_cur_dma_line = get_pos_dma();
	m_cur_irq_line = get_pos_irq();

	if(m_cur_dma_line != -1) m_mca->set_dma_channel(m_cur_dma_line, this, true);

	m_is_mapped = true;
}

void mca16_snark_barker_device::unmap()
{
	m_is_mapped = false;
	//m_mca->unset_dma_channel(m_cur_dma_line);
}

bool mca16_snark_barker_device::map_has_changed()
{
	uint8_t pos_irq = get_pos_irq();
	uint8_t pos_dma = get_pos_dma();

	return (pos_irq != m_cur_irq_line || pos_dma != m_cur_dma_line);
}

void mca16_snark_barker_device::update_pos(uint8_t data)
{
    // Only uses one POS byte.
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] = data;
	if(map_has_changed())
	{
		unmap();
		remap();
	}
}

uint8_t mca16_snark_barker_device::ym3812_16_r(offs_t offset)
{
	uint8_t retVal = 0xff;
	switch(offset)
	{
		case 0 : retVal = m_ym3812->status_r(); break;
	}
	return retVal;
}

void mca16_snark_barker_device::ym3812_16_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0 : m_ym3812->address_w(data); break;
		case 1 : m_ym3812->data_w(data); break;
	}
}

uint8_t mca16_snark_barker_device::dack_r(int line)
{
	// Shouldn't happen.

	LOG("*** %s: line %d\n", FUNCNAME, line);
	return dsp_data_r(0);
}

void mca16_snark_barker_device::dack_w(int line, uint8_t data)
{
	// Incoming DMA data!
	LOG("%s: line %d data %d\n", FUNCNAME, line, data);
	
	lower_dma();			// Acknowledge clears the DMA request.
	dsp_cmd_w(0, data);		// Write the byte to the data latch.
}

/* The DSP */

uint8_t mca16_snark_barker_device::dsp_port0_r()
{
	LOG("%s\n", FUNCNAME);
	return 0;
}
void mca16_snark_barker_device::dsp_port0_w(uint8_t data)
{
	LOG("%s D:%02X\n", FUNCNAME, data);
}

uint8_t mca16_snark_barker_device::dsp_port2_r()
{
	/*
		Bit 5: MIC_COMP:	1 = Mic <- DAC. 0 = Mic -> DAC.
		Bit 6: DAV_PC:		1 = Data waiting in buffer for host PC to read it.
		Bit 7: DAV_DSP: 	1 = Data waiting in buffer for DSP to read it.
	 */

	uint8_t data = 0;

	data |= (m_dav_dsp ? (1 << 7) : 0);
	data |= (m_dav_pc  ? (1 << 6) : 0);

	//LOG("%s: D:%02X\n", FUNCNAME, data);

	return data;
}
void mca16_snark_barker_device::dsp_port2_w(uint8_t data)
{
	// Bit 0 is the only output, which mutes audio output when pulled high.
	LOG("%s D:%02X\n", FUNCNAME, data);
}

uint8_t mca16_snark_barker_device::dsp_port3_r()
{
	/*
		Bit 0: MIDI RX:		MIDI receive data in.
	 */

	LOG("%s\n", FUNCNAME);

	uint8_t data = 0;

	if(m_dma_en) data |= (1 << 4);
	if(m_drequest) data |= (1 << 5);

	return 0 | data; // latched bits
}
void mca16_snark_barker_device::dsp_port3_w(uint8_t data)
{
	/*
		Bit 1: MIDI TX		MIDI transmit data out.
		Bit 2: IREQUEST		Strobe low to send IRQ to host PC.
		Bit 3: DSP_BUSY		Set high to tell PC we are busy.
		Bit 4: DMA_EN#		Set low to enable DMA requests.
		Bit 5: DREQUEST		Strobe high to set DRQ.
		Bit 6: DSP_WRITE#	Strobe low to write a byte to the PC.
		Bit 7: DSP_READ#	Strobe low to read a byte from the PC.
	 */

	m_dma_en = BIT(data, 4); // active low

	if(!BIT(data, 2) && m_irequest) // Strobed high -> low
	{
		raise_irq();
	}

	if(BIT(data, 5))
	{
		if(m_dma_en == false && m_drequest == false)
		{
			raise_dma();
		} 
	}

	if(!BIT(data, 6))
	{
		m_dav_pc = true;
	}

	if(!BIT(data, 7))
	{
		m_dav_dsp = true;
	}

	m_irequest = BIT(data, 2);
	m_drequest = BIT(data, 5);

	LOG("%s D:%02X\n", FUNCNAME, data);
}

uint8_t mca16_snark_barker_device::dsp_reset_r(offs_t offset)
{
	// NOP
	LOG("%s\n", FUNCNAME);
	return 0;
}

void mca16_snark_barker_device::dsp_reset_w(offs_t offset, uint8_t data)
{
	LOG("%s D:%02X\n", FUNCNAME, data);

	// Reset DSP.
	m_dsp->reset();

	// Clear all flags.
	m_irq_in_flag = false;
	m_dav_dsp = false;
	m_dav_pc = false;
	m_irequest = true; 	// active low
	m_drequest = false;	// active high

	lower_irq();
	lower_dma();
}

uint8_t mca16_snark_barker_device::dsp_rbuf_status_r(offs_t offset)
{
	// 0x22E
	lower_irq();						// Reading this bit clears the IRQ.
	return (m_dav_pc ? 0x80 : 0x00);	// Return the data waiting flag in bit 7.
}

void mca16_snark_barker_device::dsp_rbuf_status_w(offs_t offset, uint8_t data)
{
	// NOP
}

uint8_t mca16_snark_barker_device::dsp_wbuf_status_r(offs_t offset)
{
	// 0x22C
	return (m_dav_dsp ? 0x80 : 0x00);
}

// From the perspective of the MCA bus.
uint8_t mca16_snark_barker_device::dsp_data_r(offs_t offset)
{
	uint8_t data = m_dsp_to_host_latch;		// PC retrieves the latched byte.
	LOG("%s D:%02X\n", FUNCNAME, data);
	m_dav_pc = false;						// Clear the PC has data flag.
	m_dsp_to_host_latch = 0; 				// Clear the latch.

	return data;
}

void mca16_snark_barker_device::dsp_data_w(offs_t offset, uint8_t data)
{
	LOG("%s: This port does not exist\n", FUNCNAME);
}

void mca16_snark_barker_device::dsp_cmd_w(offs_t offset, uint8_t data)
{
	LOG("%s O:%02X D:%02X\n", FUNCNAME, offset, data);
	
	m_host_to_dsp_latch = data;
	m_dav_dsp = true; 	// DSP has a byte waiting.
}

// From the perspective of the DSP.
uint8_t mca16_snark_barker_device::dsp_latch_r(offs_t offset)
{
	uint8_t data = m_host_to_dsp_latch;					// DSP retrieves the latched byte.
	m_host_to_dsp_latch = 0x00;							// Clear the latch.
	LOG("%s O:%02X D:%02X\n", FUNCNAME, offset, data);
	m_dav_dsp = false;									// Clear the DSP has data flag.
	return data;
}

void mca16_snark_barker_device::dsp_latch_w(offs_t offset, uint8_t data)
{
	LOG("%s O:%02X D:%02X\n", FUNCNAME, offset, data);
	m_dsp_to_host_latch = data;
	m_dav_pc = true;	// PC has a byte waiting.
}

void mca16_snark_barker_device::map_dsp_program(address_map &map)
{
	map(0x000, 0xfff).rom().region("dsp", 0);
}

void mca16_snark_barker_device::map_dsp_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(mca16_snark_barker_device::dsp_latch_r), FUNC(mca16_snark_barker_device::dsp_latch_w));
}

ROM_START(mca_snark_barker)
	ROM_REGION(0x1000, "dsp", 0)
	ROM_LOAD("ct1351v202.bin", 0x0000, 0x1000, CRC(bb2dd936) SHA1(2b5b75f2c9e923f0b44a454bdb06d9612ff4a8bb))
ROM_END

const tiny_rom_entry *mca16_snark_barker_device::device_rom_region() const
{
	return ROM_NAME(mca_snark_barker);
}

void mca16_snark_barker_device::raise_dma()
{
	LOG("SB raising DMA %d\n", m_cur_dma_line);

	switch(m_cur_dma_line)
	{
		case 0: m_mca->dreq_w<0>(ASSERT_LINE); break;
		case 1: m_mca->dreq_w<1>(ASSERT_LINE); break;
		case 3: m_mca->dreq_w<3>(ASSERT_LINE); break;
	}
}

void mca16_snark_barker_device::lower_dma()
{
	LOG("SB lowering DMA %d\n", m_cur_dma_line);

	switch(m_cur_dma_line)
	{
		case 0: m_mca->dreq_w<0>(CLEAR_LINE); break;
		case 1: m_mca->dreq_w<1>(CLEAR_LINE); break;
		case 3: m_mca->dreq_w<3>(CLEAR_LINE); break;
	}
}

void mca16_snark_barker_device::raise_irq()
{
	if(!m_irq_raised)
	{
		LOG("SB raising IRQ %d\n", m_cur_irq_line);

		switch(m_cur_irq_line)
		{
			case 2: m_mca->ireq_w<2>(ASSERT_LINE); break;
			case 3: m_mca->ireq_w<3>(ASSERT_LINE); break;
			case 5: m_mca->ireq_w<5>(ASSERT_LINE); break;
			case 7: m_mca->ireq_w<7>(ASSERT_LINE); break;
		}
		m_irq_raised = true;
	}
	else
	{
		LOG("SB already raising IRQ %d\n", m_cur_irq_line);
	}

}

void mca16_snark_barker_device::lower_irq()
{
	if(m_irq_raised)
	{
		LOG("SB lowering IRQ %d\n", m_cur_irq_line);

		switch(m_cur_irq_line)
		{
			case 2: m_mca->ireq_w<2>(CLEAR_LINE); break;
			case 3: m_mca->ireq_w<3>(CLEAR_LINE); break;
			case 5: m_mca->ireq_w<5>(CLEAR_LINE); break;
			case 7: m_mca->ireq_w<7>(CLEAR_LINE); break;
		}
		m_irq_raised = false;
	}	
}