// license:BSD-3-Clause
// copyright-holders:Carl
// ATI Stereo F/X
//
// TODO: UART is connected to MIDI port

#include "emu.h"
#include "stereo_fx.h"

#include "sound/dac.h"
#include "speaker.h"


DEFINE_DEVICE_TYPE(ISA8_STEREO_FX, stereo_fx_device, "stereo_fx", "ATi Stereo F/X Audio Adapter")

uint8_t stereo_fx_device::dev_dsp_data_r()
{
	if (!machine().side_effects_disabled())
		m_data_in = false;
	return m_in_byte;
}

void stereo_fx_device::dev_dsp_data_w(uint8_t data)
{
	m_data_out = true;
	m_out_byte = data;
}

// port 1 is the left DAC but is written and read bitwise during capture
uint8_t stereo_fx_device::p1_r()
{
	return 0x80;
}

uint8_t stereo_fx_device::p3_r()
{
	uint8_t ret = 0;

	ret |= m_data_out << 2; // INT0
	ret |= m_data_in << 3;  // INT1
	ret |= m_t0 << 4; // T0
	ret |= m_t1 << 5; // T1
	return ret;
}

void stereo_fx_device::p3_w(uint8_t data)
{
	m_t1 = (data & 0x20) >> 5;
}

void stereo_fx_device::dev_host_irq_w(uint8_t data)
{
	m_isa->irq5_w(1);
}

void stereo_fx_device::raise_drq_w(uint8_t data)
{
	m_isa->drq1_w(1);
}

/* port 0x20 - in ROM (usually) stored in RAM 0x22
 * bit0 -
 * bit1 -
 * bit2 -
 * bit3 -
 * bit4 -
 * bit5 -
 * bit6 -
 * bit7 -
*/
void stereo_fx_device::port20_w(uint8_t data)
{
	m_port20 = data;
}

/* port 0x00 - in ROM (usually) stored in RAM 0x21
 * bit0 - bits 0-4 related to sample rate
 * bit1 - are set to 0x09-0x1e
 * bit2 -
 * bit3 -
 * bit4 -
 * bit5 -
 * bit6 -
 * bit7 -
*/
void stereo_fx_device::port00_w(uint8_t data)
{
	m_port00 = data;
}

ROM_START( stereo_fx )
	ROM_REGION( 0x8000, "stereo_fx_cpu", 0 )
	ROM_LOAD("ati_stereo_fx.bin", 0x0000, 0x8000, CRC(1bebffa6) SHA1(e66c2619a6c05199554b5702d67877ae3799d415))
ROM_END

void stereo_fx_device::stereo_fx_io(address_map &map)
{
	map(0xFF00, 0xFF00).w(FUNC(stereo_fx_device::port00_w));
	map(0xFF10, 0xFF10).w("rdac", FUNC(dac_byte_interface::data_w));
	map(0xFF20, 0xFF20).w(FUNC(stereo_fx_device::port20_w));
	//map(0xFF30, 0xFF30).w(FUNC(stereo_fx_device::));  //  used only on reset and undocumented cmd 0xc4
	map(0xFF40, 0xFF40).rw(FUNC(stereo_fx_device::dev_dsp_data_r), FUNC(stereo_fx_device::dev_dsp_data_w));
	map(0xFF50, 0xFF50).w(FUNC(stereo_fx_device::raise_drq_w));
	map(0xFF60, 0xFF60).w(FUNC(stereo_fx_device::dev_host_irq_w));
}

void stereo_fx_device::stereo_fx_rom(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

const tiny_rom_entry *stereo_fx_device::device_rom_region() const
{
	return ROM_NAME( stereo_fx );
}

void stereo_fx_device::device_add_mconfig(machine_config &config)
{
	I80C31(config, m_cpu, XTAL(30'000'000));
	m_cpu->set_addrmap(AS_PROGRAM, &stereo_fx_device::stereo_fx_rom);
	m_cpu->set_addrmap(AS_IO, &stereo_fx_device::stereo_fx_io);
	m_cpu->port_in_cb<1>().set(FUNC(stereo_fx_device::p1_r));
	m_cpu->port_out_cb<1>().set("ldac", FUNC(dac_byte_interface::data_w));
	m_cpu->port_in_cb<3>().set(FUNC(stereo_fx_device::p3_r));
	m_cpu->port_out_cb<3>().set(FUNC(stereo_fx_device::p3_w));

	SPEAKER(config, "speaker", 2).front();
	ym3812_device &ym3812(YM3812(config, "ym3812", XTAL(3'579'545)));
	ym3812.add_route(ALL_OUTPUTS, "speaker", 1.00, 0);
	ym3812.add_route(ALL_OUTPUTS, "speaker", 1.00, 1);
	/* no CM/S support (empty sockets) */

	DAC_8BIT_R2R(config, "ldac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 0); // unknown DAC
	DAC_8BIT_R2R(config, "rdac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 1); // unknown DAC

	PC_JOY(config, m_joy);
}

uint8_t stereo_fx_device::dsp_data_r()
{
	if (!machine().side_effects_disabled())
		m_data_out = false;
	return m_out_byte;
}

void stereo_fx_device::dsp_cmd_w(uint8_t data)
{
	m_data_in = true;
	m_in_byte = data;
}

uint8_t stereo_fx_device::dack_r(int line)
{
	m_data_out = false;
	m_isa->drq1_w(0);
	return m_out_byte;
}

void stereo_fx_device::dack_w(int line, uint8_t data)
{
	m_data_in = true;
	m_isa->drq1_w(0);
	m_in_byte = data;
}

void stereo_fx_device::dsp_reset_w(uint8_t data)
{
	device_reset();
	m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

uint8_t stereo_fx_device::dsp_wbuf_status_r()
{
	return m_data_in << 7;
}

uint8_t stereo_fx_device::dsp_rbuf_status_r()
{
	if (!machine().side_effects_disabled())
		m_isa->irq5_w(0);
	return m_data_out << 7;
}

void stereo_fx_device::invalid_w(uint8_t data)
{
	logerror("stereo fx: invalid port write\n");
}

uint8_t stereo_fx_device::invalid_r()
{
	if (!machine().side_effects_disabled())
		logerror("stereo fx: invalid port read\n");
	return 0xff;
}

stereo_fx_device::stereo_fx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_STEREO_FX, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_joy(*this, "pc_joy"),
	m_cpu(*this, "stereo_fx_cpu"), m_data_in(false), m_in_byte(0), m_data_out(false), m_out_byte(0), m_port20(0), m_port00(0), m_timer(nullptr), m_t0(0)
{
	m_t1 = 0;
}

void stereo_fx_device::device_start()
{
	ym3812_device &ym3812 = *subdevice<ym3812_device>("ym3812");
	set_isa_device();

	m_isa->install_device(0x0200, 0x0207, read8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_r)), write8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_w)));
	m_isa->install_device(0x0226, 0x0227, read8smo_delegate(*this, FUNC(stereo_fx_device::invalid_r)), write8smo_delegate(*this, FUNC(stereo_fx_device::dsp_reset_w)));
	m_isa->install_device(0x022a, 0x022b, read8smo_delegate(*this, FUNC(stereo_fx_device::dsp_data_r)), write8smo_delegate(*this, FUNC(stereo_fx_device::invalid_w)));
	m_isa->install_device(0x022c, 0x022d, read8smo_delegate(*this, FUNC(stereo_fx_device::dsp_wbuf_status_r)), write8smo_delegate(*this, FUNC(stereo_fx_device::dsp_cmd_w)));
	m_isa->install_device(0x022e, 0x022f, read8smo_delegate(*this, FUNC(stereo_fx_device::dsp_rbuf_status_r)), write8smo_delegate(*this, FUNC(stereo_fx_device::invalid_w)));
	m_isa->install_device(0x0388, 0x0389, read8sm_delegate(ym3812, FUNC(ym3812_device::read)), write8sm_delegate(ym3812, FUNC(ym3812_device::write)));
	m_isa->install_device(0x0228, 0x0229, read8sm_delegate(ym3812, FUNC(ym3812_device::read)), write8sm_delegate(ym3812, FUNC(ym3812_device::write)));
	m_timer = timer_alloc(FUNC(stereo_fx_device::clock_tick), this);
	m_timer->adjust(attotime::from_hz(2000000), 0, attotime::from_hz(2000000));
	m_isa->set_dma_channel(1, this, false);
}


void stereo_fx_device::device_reset()
{
	m_isa->drq1_w(0);
	m_isa->irq5_w(0);
	m_data_out = false;
	m_data_in = false;
	m_port20 = 0;
	m_port00 = 0;
	m_t0 = CLEAR_LINE;
}

TIMER_CALLBACK_MEMBER(stereo_fx_device::clock_tick)
{
	m_t0 = !m_t0;
	m_cpu->set_input_line(MCS51_T0_LINE, m_t0);
}
