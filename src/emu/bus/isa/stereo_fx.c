// license:BSD-3-Clause
// copyright-holders:Carl
// ATI Stereo F/X
//
// TODO: UART is connected to MIDI port

#include "stereo_fx.h"

const device_type ISA8_STEREO_FX = &device_creator<stereo_fx_device>;

READ8_MEMBER( stereo_fx_device::dev_dsp_data_r )
{
	m_data_in = false;
	return m_in_byte;
}

WRITE8_MEMBER( stereo_fx_device::dev_dsp_data_w )
{
	m_data_out = true;
	m_out_byte = data;
}

// port 1 is the left DAC but is written and read bitwise during capture
READ8_MEMBER( stereo_fx_device::p1_r )
{
	return 0x80;
}

READ8_MEMBER( stereo_fx_device::p3_r )
{
	UINT8 ret = 0;

	ret |= m_data_out << 2; // INT0
	ret |= m_data_in << 3;  // INT1
	ret |= m_t0 << 4; // T0
	ret |= m_t1 << 5; // T1
	return ret;
}

WRITE8_MEMBER( stereo_fx_device::p3_w )
{
	m_t1 = (data & 0x20) >> 5;
}

WRITE8_MEMBER( stereo_fx_device::dev_host_irq_w )
{
	m_isa->irq5_w(1);
}

WRITE8_MEMBER( stereo_fx_device::raise_drq_w )
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
WRITE8_MEMBER( stereo_fx_device::port20_w )
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
WRITE8_MEMBER( stereo_fx_device::port00_w )
{
	m_port00 = data;
}

ROM_START( stereo_fx )
	ROM_REGION( 0x8000, "stereo_fx_cpu", 0 )
	ROM_LOAD("ati_stereo_fx.bin", 0x0000, 0x8000, CRC(1bebffa6) SHA1(e66c2619a6c05199554b5702d67877ae3799d415))
ROM_END

static ADDRESS_MAP_START(stereo_fx_io, AS_IO, 8, stereo_fx_device)
	AM_RANGE(0xFF00, 0xFF00) AM_WRITE(port00_w)
	AM_RANGE(0xFF10, 0xFF10) AM_DEVWRITE("dacr", dac_device, write_unsigned8)
	AM_RANGE(0xFF20, 0xFF20) AM_WRITE(port20_w)
	//AM_RANGE(0xFF30, 0xFF30) AM_WRITE()  //  used only on reset and undocumented cmd 0xc4
	AM_RANGE(0xFF40, 0xFF40) AM_READWRITE(dev_dsp_data_r, dev_dsp_data_w)
	AM_RANGE(0xFF50, 0xFF50) AM_WRITE(raise_drq_w)
	AM_RANGE(0xFF60, 0xFF60) AM_WRITE(dev_host_irq_w)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READ(p1_r) AM_DEVWRITE("dacl", dac_device, write_unsigned8)
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READWRITE(p3_r, p3_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(stereo_fx_rom, AS_PROGRAM, 8, stereo_fx_device)
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( stereo_fx )
	MCFG_CPU_ADD("stereo_fx_cpu", I80C31, XTAL_30MHz)
	MCFG_CPU_IO_MAP(stereo_fx_io)
	MCFG_CPU_PROGRAM_MAP(stereo_fx_rom)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ym3812", YM3812, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)
	/* no CM/S support (empty sockets) */

	MCFG_SOUND_ADD("dacl", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MCFG_SOUND_ADD("dacr", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)

	MCFG_PC_JOY_ADD("pc_joy")
MACHINE_CONFIG_END

const rom_entry *stereo_fx_device::device_rom_region() const
{
	return ROM_NAME( stereo_fx );
}

machine_config_constructor stereo_fx_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( stereo_fx );
}

READ8_MEMBER( stereo_fx_device::dsp_data_r )
{
	m_data_out = false;
	return m_out_byte;
}

WRITE8_MEMBER( stereo_fx_device::dsp_cmd_w )
{
	m_data_in = true;
	m_in_byte = data;
}

UINT8 stereo_fx_device::dack_r(int line)
{
	m_data_out = false;
	m_isa->drq1_w(0);
	return m_out_byte;
}

void stereo_fx_device::dack_w(int line, UINT8 data)
{
	m_data_in = true;
	m_isa->drq1_w(0);
	m_in_byte = data;
}

WRITE8_MEMBER( stereo_fx_device::dsp_reset_w )
{
	device_reset();
	m_cpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
}

READ8_MEMBER( stereo_fx_device::dsp_wbuf_status_r )
{
	return m_data_in << 7;
}

READ8_MEMBER( stereo_fx_device::dsp_rbuf_status_r )
{
	m_isa->irq5_w(0);
	return m_data_out << 7;
}

WRITE8_MEMBER( stereo_fx_device::invalid_w )
{
	logerror("stereo fx: invalid port write\n");
}

READ8_MEMBER( stereo_fx_device::invalid_r )
{
	logerror("stereo fx: invalid port read\n");
	return 0xff;
}

stereo_fx_device::stereo_fx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ISA8_STEREO_FX, "ATI Stereo F/X Audio Adapter", tag, owner, clock, "stereo_fx", __FILE__),
	device_isa8_card_interface(mconfig, *this),
	m_dacl(*this, "dacl"),
	m_dacr(*this, "dacr"),
	m_joy(*this, "pc_joy"),
	m_cpu(*this, "stereo_fx_cpu")
{
	m_t1 = 0;
}

void stereo_fx_device::device_start()
{
	ym3812_device *ym3812 = subdevice<ym3812_device>("ym3812");
	set_isa_device();

	m_isa->install_device(0x0200, 0x0207, 0, 0, read8_delegate(FUNC(pc_joy_device::joy_port_r), subdevice<pc_joy_device>("pc_joy")), write8_delegate(FUNC(pc_joy_device::joy_port_w), subdevice<pc_joy_device>("pc_joy")));
	m_isa->install_device(0x0226, 0x0227, 0, 0, read8_delegate(FUNC(stereo_fx_device::invalid_r), this), write8_delegate(FUNC(stereo_fx_device::dsp_reset_w), this));
	m_isa->install_device(0x022a, 0x022b, 0, 0, read8_delegate(FUNC(stereo_fx_device::dsp_data_r), this), write8_delegate(FUNC(stereo_fx_device::invalid_w), this) );
	m_isa->install_device(0x022c, 0x022d, 0, 0, read8_delegate(FUNC(stereo_fx_device::dsp_wbuf_status_r), this), write8_delegate(FUNC(stereo_fx_device::dsp_cmd_w), this) );
	m_isa->install_device(0x022e, 0x022f, 0, 0, read8_delegate(FUNC(stereo_fx_device::dsp_rbuf_status_r), this), write8_delegate(FUNC(stereo_fx_device::invalid_w), this) );
	m_isa->install_device(0x0388, 0x0389, 0, 0, read8_delegate(FUNC(ym3812_device::read), ym3812), write8_delegate(FUNC(ym3812_device::write), ym3812));
	m_isa->install_device(0x0228, 0x0229, 0, 0, read8_delegate(FUNC(ym3812_device::read), ym3812), write8_delegate(FUNC(ym3812_device::write), ym3812));
	m_timer = timer_alloc();
	m_timer->adjust(attotime::from_hz(2000000), 0, attotime::from_hz(2000000));
	m_isa->set_dma_channel(1, this, FALSE);
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

void stereo_fx_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	m_t0 = !m_t0;
	m_cpu->set_input_line(MCS51_T0_LINE, m_t0);
}
