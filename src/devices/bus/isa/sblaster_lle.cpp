// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************
    Sound Blaster LLE device
    - The "DSP" is an 8051.

    Models:
    - CT1310 (Sound Blaster 1.0)
        - In Creative Labs product literature but may not have been released
        - OPL2, 2x SAA1099, gameport, 22KHz playback/12KHz record

    - CT1320 (Sound Blaster 1.5)
        - DSP p/n CT1321, version 1.5
        - Cost-reduced CT1310
        - OPL2, 2xSAA1099 are now optional, gameport, 22KHz playback/12KHz record

    - CT1350 (Sound Blaster 2.0)
        - DSP p/n CT1351, version 2.0
        - OPL2, no SAA1099, 44KHz playback/22KHz record, gameport, auto-initializing DMA
        - If DSP 2.0x is placed in a CT1320, it becomes functionally equivalent to a CT1350.
***************************************************************************/

#include "emu.h"
#include "sblaster_lle.h"
#include "speaker.h"

//#define VERBOSE 1
#include "logmacro.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

static constexpr XTAL CLOCK_SAA1099 = XTAL(14'318'181/2);   // ISA OSC pin / 2
static constexpr XTAL CLOCK_YM3812  = XTAL(14'318'181/4);   // ISA OSC pin / 4
static constexpr XTAL XTAL_DSP      = XTAL(12'000'000);     // crystal X1

DEFINE_DEVICE_TYPE(ISA8_SOUND_BLASTER_2_0_LLE, isa8_sblaster_2_0_lle_device, "isa8_sblaster_2_0", "Sound Blaster 2.0 (CT1350, LLE)")

void isa8_sblaster_2_0_lle_device::device_add_mconfig(machine_config &config)
{
	I80C51(config, m_dsp, XTAL_DSP); // XTL1
	m_dsp->set_addrmap(AS_PROGRAM, &isa8_sblaster_2_0_lle_device::map_dsp_program);
	m_dsp->set_addrmap(AS_IO, &isa8_sblaster_2_0_lle_device::map_dsp_io);

	// No inputs on ports 0 or 1
	m_dsp->port_in_cb<2>().set(FUNC(isa8_sblaster_2_0_lle_device::dsp_port2_r));
	m_dsp->port_in_cb<3>().set(FUNC(isa8_sblaster_2_0_lle_device::dsp_port3_r));
	// No outputs on port 0
	m_dsp->port_out_cb<1>().set(m_dac, FUNC(dac_byte_interface::data_w));               // DSP port 1 is hooked right up to the DAC.
	m_dsp->port_out_cb<2>().set(FUNC(isa8_sblaster_2_0_lle_device::dsp_port2_w));
	m_dsp->port_out_cb<3>().set(FUNC(isa8_sblaster_2_0_lle_device::dsp_port3_w));

	PC_JOY(config, m_joy);

	SPEAKER(config, m_lspeaker).front_left();
	SPEAKER(config, m_rspeaker).front_right();

	MC1408(config, m_dac, 0);
	m_dac->add_route(ALL_OUTPUTS, m_lspeaker, 0.5);
	m_dac->add_route(ALL_OUTPUTS, m_rspeaker, 0.5);

	YM3812(config, m_ym3812, CLOCK_YM3812);
	m_ym3812->add_route(ALL_OUTPUTS, m_lspeaker, 1.0);
	m_ym3812->add_route(ALL_OUTPUTS, m_rspeaker, 1.0);

	// FM and DAC are mono, but CMS is stereo.
	SAA1099(config, m_saa1099_1, CLOCK_SAA1099);
	m_saa1099_1->add_route(0, m_lspeaker, 0.5);
	m_saa1099_1->add_route(1, m_rspeaker, 0.5);
	SAA1099(config, m_saa1099_2, CLOCK_SAA1099);
	m_saa1099_2->add_route(0, m_lspeaker, 0.5);
	m_saa1099_2->add_route(1, m_rspeaker, 0.5);
}

static INPUT_PORTS_START( sb_dsw )
	PORT_START("JP1")
	PORT_CONFNAME( 0x03, 0x01, "I/O Base Address")
	PORT_CONFSETTING(    0x01, "220h")
	PORT_CONFSETTING(    0x02, "240h")

	PORT_START("JP4")
	PORT_CONFNAME( 0x0f, 0x01, "IRQ Line")
	PORT_CONFSETTING(    0x08, "IRQ2/9" )
	PORT_CONFSETTING(    0x04, "IRQ3" )
	PORT_CONFSETTING(    0x02, "IRQ5")
	PORT_CONFSETTING(    0x01, "IRQ7")

	PORT_START("JP8")
	PORT_CONFNAME( 0x01, 0x01, "Game Port")
	PORT_CONFSETTING(    0x00, "Disabled")
	PORT_CONFSETTING(    0x01, "Enabled")

	PORT_START("JP9")
	PORT_CONFNAME( 0x01, 0x00, "Creative Music System Present?")
	PORT_CONFSETTING(    0x00, "No")
	PORT_CONFSETTING(    0x01, "Yes")
INPUT_PORTS_END

ioport_constructor isa8_sblaster_2_0_lle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sb_dsw );
}

isa8_sblaster_2_0_lle_device::isa8_sblaster_2_0_lle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_SOUND_BLASTER_2_0_LLE, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_dsp(*this, "dsp"),
	m_joy(*this, "pc_joy"),
	m_ym3812(*this, "ym3812"),
	m_dac(*this, "dac"),
	m_lspeaker(*this, "lspeaker"),
	m_rspeaker(*this, "rspeaker"),
	m_saa1099_1(*this, "saa1099.1"),
	m_saa1099_2(*this, "saa1099.2"),
	m_jp1(*this, "JP1"),
	m_jp4(*this, "JP4"),
	m_jp8(*this, "JP8"),
	m_jp9(*this, "JP9")
{
}

void isa8_sblaster_2_0_lle_device::device_start()
{
	m_installed_base_io = 0;
	m_selected_irq = 0;
	set_isa_device();
}

void isa8_sblaster_2_0_lle_device::device_reset()
{
	m_gameport_enabled = m_jp8->read() & 0x01;
	m_cms_present = m_jp9->read() & 0x01;

	if(m_selected_irq == 0)
	{
		switch(m_jp4->read())
		{
			case 1: m_selected_irq = 7; break;
			case 2: m_selected_irq = 5; break;
			case 4: m_selected_irq = 3; break;
			case 8: m_selected_irq = 2; break;
		}
	}

	install_io();

	lower_irq();
	lower_dma();

	// DMA is not configurable.
	m_isa->set_dma_channel(1, this, false);

	m_irq_in_flag = false;
	m_irequest = true;
	m_drequest = false;

	dsp_reset_w(0, 0);

	m_irq_raised = 0;
	m_dma_raised = false;
}

void isa8_sblaster_2_0_lle_device::install_io()
{
	if(m_installed_base_io == 0)
	{
		uint16_t base = (m_jp1->read() & 0x02) ? 0x240 : 0x220;

		m_isa->install_device(base + 0x6, base + 0x7,
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::dsp_reset_r)),
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::dsp_reset_w)));
		m_isa->install_device(base + 0x8, base + 0x9,
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::ym3812_16_r)),
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::ym3812_16_w)));
		m_isa->install_device(base + 0xa, base + 0xb,
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::dsp_data_r)),
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::dsp_data_w)));
		m_isa->install_device(base + 0xc, base + 0xd,
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::dsp_wbuf_status_r)),
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::dsp_cmd_w)));
		m_isa->install_device(base + 0xe, base + 0xf,
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::dsp_rbuf_status_r)),
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::dsp_rbuf_status_w)));
		m_isa->install_device(0x388, 0x389,
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::ym3812_16_r)),
			emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::ym3812_16_w)));

		if(m_gameport_enabled)
		{
			m_isa->install_device(0x200, 0x207,
				emu::rw_delegate(m_joy, FUNC(pc_joy_device::joy_port_r)),
				emu::rw_delegate(m_joy, FUNC(pc_joy_device::joy_port_w)));
		}

		if(m_cms_present)
		{
			m_isa->install_device(base + 0x0, base + 0x1,
				emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::saa1099_16_r)), // read is a NOP
				emu::rw_delegate(m_saa1099_1, FUNC(saa1099_device::write)));
			m_isa->install_device(base + 0x2, base + 0x3,
				emu::rw_delegate(*this, FUNC(isa8_sblaster_2_0_lle_device::saa1099_16_r)), // read is a NOP
				emu::rw_delegate(m_saa1099_2, FUNC(saa1099_device::write)));
		}

		m_installed_base_io = base;
	}
}

// ISA DMA handling
uint8_t isa8_sblaster_2_0_lle_device::dack_r(int line)
{
	// Shouldn't happen.
	return dsp_data_r(0);
}

void isa8_sblaster_2_0_lle_device::dack_w(int line, uint8_t data)
{
	// Incoming DMA data!
	lower_dma();            // Acknowledge clears the DMA request.
	dsp_cmd_w(0, data);     // Write the byte to the data latch.
}

void isa8_sblaster_2_0_lle_device::raise_dma()
{
	// Not configurable.
	LOG("SB raising DMA %d\n", 1);
	m_isa->drq1_w(ASSERT_LINE);
}

void isa8_sblaster_2_0_lle_device::lower_dma()
{
	// Not configurable.
	LOG("SB lowering DMA %d\n", 1);
	m_isa->drq1_w(CLEAR_LINE);
}

void isa8_sblaster_2_0_lle_device::raise_irq()
{
	if(!m_irq_raised)
	{
		LOG("SB raising IRQ %d\n", m_selected_irq);
		switch(m_selected_irq)
		{
			case 7: m_isa->irq7_w(ASSERT_LINE); break;
			case 5: m_isa->irq5_w(ASSERT_LINE); break;
			case 3: m_isa->irq3_w(ASSERT_LINE); break;
			case 2: m_isa->irq2_w(ASSERT_LINE); break;
		}
		m_irq_raised = m_selected_irq;
	}
}

void isa8_sblaster_2_0_lle_device::lower_irq()
{
	if(m_irq_raised != 0)
	{
		LOG("SB lowering IRQ %d\n", m_irq_raised);
		switch(m_irq_raised)
		{
			case 2: m_isa->irq2_w(CLEAR_LINE); break;
			case 3: m_isa->irq3_w(CLEAR_LINE); break;
			case 5: m_isa->irq5_w(CLEAR_LINE); break;
			case 7: m_isa->irq7_w(CLEAR_LINE); break;
			default: break;
		}
		m_irq_raised = 0;
	}
}

// YM3812
uint8_t isa8_sblaster_2_0_lle_device::ym3812_16_r(offs_t offset)
{
	uint8_t retVal = 0xff;
	switch(offset)
	{
		case 0 : retVal = m_ym3812->status_r(); break;
	}
	return retVal;
}

void isa8_sblaster_2_0_lle_device::ym3812_16_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0 : m_ym3812->address_w(data); break;
		case 1 : m_ym3812->data_w(data); break;
	}
}

// 80C51
/*
    DSP port 2
    Bit 0: w OUTPUT_EN: 1 = Mute, 0 = Not Mute
    Bit 5: r MIC_COMP:  1 = Mic <- DAC. 0 = Mic -> DAC.
    Bit 6: r DAV_PC:    1 = Data waiting in buffer for host PC to read it.
    Bit 7: r DAV_DSP:   1 = Data waiting in buffer for DSP to read it.

*/
uint8_t isa8_sblaster_2_0_lle_device::dsp_port2_r()
{
	uint8_t data = 0;

	data |= (m_dav_dsp ? (1 << 7) : 0);
	data |= (m_dav_pc  ? (1 << 6) : 0);

	return data;
}
void isa8_sblaster_2_0_lle_device::dsp_port2_w(uint8_t data)
{
	// Bit 0 is the only output, which mutes audio output when pulled high.
	m_lspeaker->set_output_gain(ALL_OUTPUTS, !BIT(data, 0));
	m_rspeaker->set_output_gain(ALL_OUTPUTS, !BIT(data, 0));
}

/*
    DSP port 3

    Bit 0: r MIDI RX    MIDI receive data in.
    Bit 1: w MIDI TX    MIDI transmit data out.
    Bit 2: w IREQUEST   Strobe low to send IRQ to host PC.
    Bit 3: w DSP_BUSY   Set high to tell PC we are busy.
    Bit 4: w DMA_EN#    Set low to enable DMA requests.
    Bit 5: w DREQUEST   Strobe high to set DRQ.
    Bit 6: w DSP_WRITE# Strobe low to write a byte to the PC.
    Bit 7: w DSP_READ#  Strobe low to read a byte from the PC.
*/
uint8_t isa8_sblaster_2_0_lle_device::dsp_port3_r()
{
	uint8_t data = 0;

	if(m_dma_en) data |= (1 << 4);
	if(m_drequest) data |= (1 << 5);

	return 0 | data; // latched bits
}
void isa8_sblaster_2_0_lle_device::dsp_port3_w(uint8_t data)
{
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
}

void isa8_sblaster_2_0_lle_device::dsp_reset_w(offs_t offset, uint8_t data)
{
	if(BIT(data, 0))
	{
		// Reset DSP.
		m_dsp->reset();

		// Clear all flags.
		m_irq_in_flag = false;
		m_dav_dsp = false;
		m_dav_pc = false;
		m_irequest = true;  // active low
		m_drequest = false; // active high

		lower_irq();
		lower_dma();
	}
}

uint8_t isa8_sblaster_2_0_lle_device::dsp_rbuf_status_r(offs_t offset)
{
	// 0x22E
	if(!machine().side_effects_disabled())
	{
		lower_irq();                    // Reading this bit clears the IRQ.
	}

	return (m_dav_pc ? 0x80 : 0x00);    // Return the data waiting flag in bit 7.
}

uint8_t isa8_sblaster_2_0_lle_device::dsp_wbuf_status_r(offs_t offset)
{
	// 0x22C
	return (m_dav_dsp ? 0x80 : 0x00);
}

// From the perspective of the ISA bus.
uint8_t isa8_sblaster_2_0_lle_device::dsp_data_r(offs_t offset)
{
	uint8_t data = m_dsp_to_host_latch;     // PC retrieves the latched byte.

	if(!machine().side_effects_disabled())
	{
		LOG("PC reading DSP data latch: D:%02X\n", data);
		m_dav_pc = false;                       // Clear the PC has data flag.
		m_dsp_to_host_latch = 0;                // Clear the latch.
	}

	return data;
}

void isa8_sblaster_2_0_lle_device::dsp_cmd_w(offs_t offset, uint8_t data)
{
	LOG("DSP received command byte D:%02X\n", data);

	m_host_to_dsp_latch = data;
	m_dav_dsp = true;   // DSP has a byte waiting.
}

// From the perspective of the DSP.
uint8_t isa8_sblaster_2_0_lle_device::dsp_latch_r(offs_t offset)
{
	uint8_t data = m_host_to_dsp_latch;                 // DSP retrieves the latched byte.

	if(!machine().side_effects_disabled())
	{
		m_host_to_dsp_latch = 0x00;                         // Clear the latch.
		LOG("DSP reading a byte from PC: D:%02X\n", data);
		m_dav_dsp = false;                                  // Clear the DSP has data flag.
	}

	return data;
}

void isa8_sblaster_2_0_lle_device::dsp_latch_w(offs_t offset, uint8_t data)
{
	// Offset doesn't matter, the address bus isn't hooked up. It just writes a byte to a latch.
	LOG("DSP writing to PC: D:%02X\n", data);
	m_dsp_to_host_latch = data;
	m_dav_pc = true;    // PC has a byte waiting.
}

void isa8_sblaster_2_0_lle_device::map_dsp_program(address_map &map)
{
	map(0x000, 0xfff).rom().region("dsp", 0);
}

void isa8_sblaster_2_0_lle_device::map_dsp_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(isa8_sblaster_2_0_lle_device::dsp_latch_r), FUNC(isa8_sblaster_2_0_lle_device::dsp_latch_w));
}

uint8_t isa8_sblaster_2_0_lle_device::saa1099_16_r(offs_t offset)
{
	return 0xff;
}

ROM_START(isa8_sblaster_2_0)
	ROM_REGION(0x1000, "dsp", 0)
	ROM_SYSTEM_BIOS(0, "dsp_v202", "DSP V2.02 (Creative Labs CT1351)")
	ROMX_LOAD("ct1351v202.bin", 0x0000, 0x1000, CRC(bb2dd936) SHA1(2b5b75f2c9e923f0b44a454bdb06d9612ff4a8bb), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "anchor_v201", "DSP V2.01 (Anchor Electronics clone)")
	ROMX_LOAD("anchorv201.bin", 0x0000, 0x1000, CRC(bb2dd936) SHA1(2b5b75f2c9e923f0b44a454bdb06d9612ff4a8bb), ROM_BIOS(1))
ROM_END

const tiny_rom_entry *isa8_sblaster_2_0_lle_device::device_rom_region() const
{
	return ROM_NAME(isa8_sblaster_2_0);
}
