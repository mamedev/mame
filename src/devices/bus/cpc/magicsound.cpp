// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * magicsound.cpp
 *
 *  Magic Sound Board for the Aleste 520EX
 *
 */

#include "emu.h"
#include "magicsound.h"
#include "sound/volt_reg.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AL_MAGICSOUND, al_magicsound_device, "al_magicsound", "Aleste Magic Sound Board")


MACHINE_CONFIG_START(al_magicsound_device::device_add_mconfig)
	MCFG_DEVICE_ADD( "dmac", AM9517A, XTAL(4'000'000) )  // CLK from expansion port
	// According to the schematics, the TC pin (EOP on western chips) is connected to NMI on the expansion port.
	// NMIs seem to occur too quickly when this is active, so either EOP is not triggered at the correct time, or
	// the K1810WT37 is different to the i8237/AM9517A
	//MCFG_I8237_OUT_EOP_CB(WRITELINE("^", cpc_expansion_slot_device, nmi_w)) // MCFG_DEVCB_INVERT
	MCFG_I8237_OUT_HREQ_CB(WRITELINE("dmac", am9517a_device, hack_w))
	MCFG_I8237_IN_MEMR_CB(READ8(*this, al_magicsound_device, dma_read_byte))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(*this, al_magicsound_device, dma_write_byte))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(*this, al_magicsound_device, dma_write_byte))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(*this, al_magicsound_device, dma_write_byte))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(*this, al_magicsound_device, dma_write_byte))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(*this, al_magicsound_device, dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(*this, al_magicsound_device, dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(*this, al_magicsound_device, dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(*this, al_magicsound_device, dack3_w))

	// Timing does not seem to be correct.
	// According to the schematics, the clock is from the clock pin on the expansion port (4MHz), and
	// passes through an inverter to each CLK pin on both timers.  This seems to be too fast.
	// Timer outputs to SAM0/1/2/3 are sample clocks for each sound channel, D/A0 is the low bit of the channel select.
	MCFG_DEVICE_ADD("timer1", PIT8254, 0)
	MCFG_PIT8253_CLK0(XTAL(4'000'000))
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(*this, al_magicsound_device, sam0_w))
	MCFG_PIT8253_CLK1(XTAL(4'000'000))
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(*this, al_magicsound_device, sam1_w))
	MCFG_PIT8253_CLK2(XTAL(4'000'000))
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(*this, al_magicsound_device, sam2_w))

	MCFG_DEVICE_ADD("timer2", PIT8254, 0)
	MCFG_PIT8253_CLK0(XTAL(4'000'000))
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(*this, al_magicsound_device, sam3_w))
	MCFG_PIT8253_CLK1(XTAL(4'000'000))
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(*this, al_magicsound_device, da0_w))
	MCFG_PIT8253_CLK2(XTAL(4'000'000))

	SPEAKER(config, "speaker").front_center();
	MCFG_DEVICE_ADD("dac", DAC_8BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.5) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
	// no pass-through(?)
MACHINE_CONFIG_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

al_magicsound_device::al_magicsound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AL_MAGICSOUND, tag, owner, clock),
	device_cpc_expansion_card_interface(mconfig, *this), m_slot(nullptr),
	m_dac(*this,"dac"),
	m_dmac(*this,"dmac"),
	m_timer1(*this,"timer1"),
	m_timer2(*this,"timer2"), m_current_channel(0), m_ramptr(nullptr), m_current_output(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void al_magicsound_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_IO);
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());

	space.install_readwrite_handler(0xf8d0,0xf8df,read8_delegate(FUNC(al_magicsound_device::dmac_r),this),write8_delegate(FUNC(al_magicsound_device::dmac_w),this));
	space.install_write_handler(0xf9d0,0xf9df,write8_delegate(FUNC(al_magicsound_device::timer_w),this));
	space.install_write_handler(0xfad0,0xfadf,write8_delegate(FUNC(al_magicsound_device::volume_w),this));
	space.install_write_handler(0xfbd0,0xfbdf,write8_delegate(FUNC(al_magicsound_device::mapper_w),this));

	m_ramptr = machine().device<ram_device>(":" RAM_TAG);

	for(int x=0;x<4;x++)
	{
		save_item(NAME(m_output[x]),x);
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void al_magicsound_device::device_reset()
{
	m_current_channel = -1;
	m_current_output = 0;
	set_timer_gate(false);
}

READ8_MEMBER(al_magicsound_device::dmac_r)
{
	return m_dmac->read(space,offset);
}

WRITE8_MEMBER(al_magicsound_device::dmac_w)
{
	m_dmac->write(space,offset,data);
}

WRITE8_MEMBER(al_magicsound_device::timer_w)
{
	// can both PITs be selected at the same time?
	if(offset & 0x08)
		m_timer1->write(space,offset & 0x03,data);
	if(offset & 0x04)
		m_timer2->write(space,offset & 0x03,data);
}

WRITE8_MEMBER(al_magicsound_device::volume_w)
{
	m_volume[offset & 0x03] = data & 0x3f;
}

WRITE8_MEMBER(al_magicsound_device::mapper_w)
{
	uint8_t channel = (offset & 0x0c) >> 2;
	uint8_t page = offset & 0x03;
	m_page[channel][page] = (~(data) & 0x3f) * 0x4000;
	set_timer_gate(true);
}

WRITE_LINE_MEMBER(al_magicsound_device::da0_w)
{
	m_dac->write(m_output[m_current_output++]);
	if(m_current_output > 3)
		m_current_output = 0;
}

WRITE_LINE_MEMBER(al_magicsound_device::dack0_w) { m_dack[0] = state; }
WRITE_LINE_MEMBER(al_magicsound_device::dack1_w) { m_dack[1] = state; }
WRITE_LINE_MEMBER(al_magicsound_device::dack2_w) { m_dack[2] = state; }
WRITE_LINE_MEMBER(al_magicsound_device::dack3_w) { m_dack[3] = state; }

WRITE_LINE_MEMBER(al_magicsound_device::sam0_w) { m_current_channel = 0; if(m_dack[0] && state) m_dmac->dreq0_w(1); }
WRITE_LINE_MEMBER(al_magicsound_device::sam1_w) { m_current_channel = 1; if(m_dack[1] && state) m_dmac->dreq1_w(1); }
WRITE_LINE_MEMBER(al_magicsound_device::sam2_w) { m_current_channel = 2; if(m_dack[2] && state) m_dmac->dreq2_w(1); }
WRITE_LINE_MEMBER(al_magicsound_device::sam3_w) { m_current_channel = 3; if(m_dack[3] && state) m_dmac->dreq3_w(1); }

READ8_MEMBER(al_magicsound_device::dma_read_byte)
{
	uint8_t ret = 0xff;
	uint8_t page = (offset & 0xc000) >> 14;

	if(m_current_channel != -1)
		ret = m_ramptr->read(m_page[m_current_channel][page] + (offset & 0x3fff));
	return ret;
}

WRITE8_MEMBER(al_magicsound_device::dma_write_byte)
{
	m_output[m_current_channel] = data;
}

void al_magicsound_device::set_timer_gate(bool state)
{
	m_timer1->write_gate0(state);
	m_timer1->write_gate1(state);
	m_timer1->write_gate2(state);
	m_timer2->write_gate0(state);
	m_timer2->write_gate1(state);
	m_timer2->write_gate2(state);
}
