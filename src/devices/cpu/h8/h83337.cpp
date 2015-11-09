// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h83337.h"

const device_type H83334 = &device_creator<h83334_device>;
const device_type H83336 = &device_creator<h83336_device>;
const device_type H83337 = &device_creator<h83337_device>;


h83337_device::h83337_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	h8_device(mconfig, type, name, tag, owner, clock, shortname, source, true, address_map_delegate(FUNC(h83337_device::map), this)),
	intc(*this, "intc"),
	adc(*this, "adc"),
	port1(*this, "port1"),
	port2(*this, "port2"),
	port3(*this, "port3"),
	port4(*this, "port4"),
	port5(*this, "port5"),
	port6(*this, "port6"),
	port7(*this, "port7"),
	port8(*this, "port8"),
	port9(*this, "port9"),
	timer8_0(*this, "timer8_0"),
	timer8_1(*this, "timer8_1"),
	timer16(*this, "timer16"),
	timer16_0(*this, "timer16:0"),
	sci0(*this, "sci0"),
	sci1(*this, "sci1")
{
}

h83337_device::h83337_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	h8_device(mconfig, H83337, "H8/3337", tag, owner, clock, "h83337", __FILE__, true, address_map_delegate(FUNC(h83337_device::map), this)),
	intc(*this, "intc"),
	adc(*this, "adc"),
	port1(*this, "port1"),
	port2(*this, "port2"),
	port3(*this, "port3"),
	port4(*this, "port4"),
	port5(*this, "port5"),
	port6(*this, "port6"),
	port7(*this, "port7"),
	port8(*this, "port8"),
	port9(*this, "port9"),
	timer8_0(*this, "timer8_0"),
	timer8_1(*this, "timer8_1"),
	timer16(*this, "timer16"),
	timer16_0(*this, "timer16:0"),
	sci0(*this, "sci0"),
	sci1(*this, "sci1")
{
	ram_start = 0xf780;
}

h83334_device::h83334_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	h83337_device(mconfig, H83334, "H8/3334", tag, owner, clock, "h83334", __FILE__)
{
	ram_start = 0xfb80;
}

h83336_device::h83336_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	h83337_device(mconfig, H83336, "H8/3336", tag, owner, clock, "h83336", __FILE__)
{
	ram_start = 0xf780;
}

static MACHINE_CONFIG_FRAGMENT(h83337)
	MCFG_H8_INTC_ADD("intc")
	MCFG_H8_ADC_3337_ADD("adc", "intc", 35)
	MCFG_H8_PORT_ADD("port1", h8_device::PORT_1, 0x00, 0x00)
	MCFG_H8_PORT_ADD("port2", h8_device::PORT_2, 0x00, 0x00)
	MCFG_H8_PORT_ADD("port3", h8_device::PORT_3, 0x00, 0x00)
	MCFG_H8_PORT_ADD("port4", h8_device::PORT_4, 0x00, 0x00)
	MCFG_H8_PORT_ADD("port5", h8_device::PORT_5, 0xf8, 0xf8)
	MCFG_H8_PORT_ADD("port6", h8_device::PORT_6, 0x00, 0x00)
	MCFG_H8_PORT_ADD("port7", h8_device::PORT_7, 0x00, 0x00)
	MCFG_H8_PORT_ADD("port8", h8_device::PORT_8, 0x80, 0x80)
	MCFG_H8_PORT_ADD("port9", h8_device::PORT_9, 0x00, 0x00)
	MCFG_H8_TIMER8_CHANNEL_ADD("timer8_0", "intc", 19, 20, 21, 8, 2, 64, 32, 1024, 256)
	MCFG_H8_TIMER8_CHANNEL_ADD("timer8_1", "intc", 22, 23, 24, 8, 2, 64, 128, 1024, 2048)
	MCFG_H8_TIMER16_ADD("timer16", 1, 0xff)
	MCFG_H8_TIMER16_CHANNEL_ADD("timer16:0", 4, 0, "intc", 32)
	MCFG_H8_SCI_ADD("sci0", "intc", 27, 28, 29, 30)
	MCFG_H8_SCI_ADD("sci1", "intc", 31, 32, 33, 34)
MACHINE_CONFIG_END

DEVICE_ADDRESS_MAP_START(map, 16, h83337_device)
	AM_RANGE(ram_start, 0xff7f) AM_RAM

	AM_RANGE(0xff88, 0xff89) AM_DEVREADWRITE8("sci1",      h8_sci_device,             smr_r,   smr_w,   0xff00)
	AM_RANGE(0xff88, 0xff89) AM_DEVREADWRITE8("sci1",      h8_sci_device,             brr_r,   brr_w,   0x00ff)
	AM_RANGE(0xff8a, 0xff8b) AM_DEVREADWRITE8("sci1",      h8_sci_device,             scr_r,   scr_w,   0xff00)
	AM_RANGE(0xff8a, 0xff8b) AM_DEVREADWRITE8("sci1",      h8_sci_device,             tdr_r,   tdr_w,   0x00ff)
	AM_RANGE(0xff8c, 0xff8d) AM_DEVREADWRITE8("sci1",      h8_sci_device,             ssr_r,   ssr_w,   0xff00)
	AM_RANGE(0xff8c, 0xff8d) AM_DEVREAD8(     "sci1",      h8_sci_device,             rdr_r,            0x00ff)
	AM_RANGE(0xff90, 0xff91) AM_DEVREADWRITE8("timer16:0", h8_timer16_channel_device, tier_r,  tier_w,  0xff00)
	AM_RANGE(0xff90, 0xff91) AM_DEVREADWRITE8("timer16:0", h8_timer16_channel_device, tsr_r,   tsr_w,   0x00ff)
	AM_RANGE(0xff92, 0xff93) AM_DEVREADWRITE( "timer16:0", h8_timer16_channel_device, tcnt_r,  tcnt_w         )
//  AM_RANGE(0xff94, 0xff95) AM_DEVREADWRITE( "timer16:0", h8_timer16_channel_device, ocr_r,   ocr_w          )
	AM_RANGE(0xff96, 0xff97) AM_DEVREADWRITE8("timer16:0", h8_timer16_channel_device, tcr_r,   tcr_w,   0xff00)
//  AM_RANGE(0xff96, 0xff97) AM_DEVREADWRITE8("timer16:0", h8_timer16_channel_device, tocr_r,  tocr_w,  0x00ff)
	AM_RANGE(0xff98, 0xff9f) AM_DEVREAD(      "timer16:0", h8_timer16_channel_device, tgr_r                   )

	AM_RANGE(0xffac, 0xffad) AM_DEVREADWRITE8("port1",     h8_port_device,            pcr_r,   pcr_w,   0xff00)
	AM_RANGE(0xffac, 0xffad) AM_DEVREADWRITE8("port2",     h8_port_device,            pcr_r,   pcr_w,   0x00ff)
	AM_RANGE(0xffae, 0xffaf) AM_DEVREADWRITE8("port3",     h8_port_device,            pcr_r,   pcr_w,   0xff00)
	AM_RANGE(0xffb0, 0xffb1) AM_DEVWRITE8(    "port1",     h8_port_device,                     ddr_w,   0xff00)
	AM_RANGE(0xffb0, 0xffb1) AM_DEVWRITE8(    "port2",     h8_port_device,                     ddr_w,   0x00ff)
	AM_RANGE(0xffb2, 0xffb3) AM_DEVREADWRITE8("port1",     h8_port_device,            port_r,  dr_w,    0xff00)
	AM_RANGE(0xffb2, 0xffb3) AM_DEVREADWRITE8("port2",     h8_port_device,            port_r,  dr_w,    0x00ff)
	AM_RANGE(0xffb4, 0xffb5) AM_DEVWRITE8(    "port3",     h8_port_device,                     ddr_w,   0xff00)
	AM_RANGE(0xffb4, 0xffb5) AM_DEVWRITE8(    "port4",     h8_port_device,                     ddr_w,   0x00ff)
	AM_RANGE(0xffb6, 0xffb7) AM_DEVREADWRITE8("port3",     h8_port_device,            port_r,  dr_w,    0xff00)
	AM_RANGE(0xffb6, 0xffb7) AM_DEVREADWRITE8("port4",     h8_port_device,            port_r,  dr_w,    0x00ff)
	AM_RANGE(0xffb8, 0xffb9) AM_DEVWRITE8(    "port5",     h8_port_device,                     ddr_w,   0xff00)
	AM_RANGE(0xffb8, 0xffb9) AM_DEVWRITE8(    "port6",     h8_port_device,                     ddr_w,   0x00ff)
	AM_RANGE(0xffba, 0xffbb) AM_DEVREADWRITE8("port5",     h8_port_device,            port_r,  dr_w,    0xff00)
	AM_RANGE(0xffba, 0xffbb) AM_DEVREADWRITE8("port6",     h8_port_device,            port_r,  dr_w,    0x00ff)
	AM_RANGE(0xffbc, 0xffbd) AM_DEVWRITE8(    "port8",     h8_port_device,                     ddr_w,   0x00ff)
	AM_RANGE(0xffbe, 0xffbf) AM_DEVREADWRITE8("port7",     h8_port_device,            port_r,  dr_w,    0xff00)
	AM_RANGE(0xffbe, 0xffbf) AM_DEVREADWRITE8("port8",     h8_port_device,            port_r,  dr_w,    0x00ff)
	AM_RANGE(0xffc0, 0xffc1) AM_DEVWRITE8(    "port9",     h8_port_device,                     ddr_w,   0xff00)
	AM_RANGE(0xffc0, 0xffc1) AM_DEVREADWRITE8("port9",     h8_port_device,            port_r,  dr_w,    0x00ff)
	AM_RANGE(0xffc2, 0xffc3) AM_READWRITE8(                                           wscr_r,  wscr_w,  0xff00)
	AM_RANGE(0xffc2, 0xffc3) AM_READWRITE8(                                           stcr_r,  stcr_w,  0x00ff)
	AM_RANGE(0xffc4, 0xffc5) AM_READWRITE8(                                           syscr_r, syscr_w, 0xff00)
	AM_RANGE(0xffc4, 0xffc5) AM_READWRITE8(                                           mdcr_r,  mdcr_w,  0x00ff)
	AM_RANGE(0xffc6, 0xffc7) AM_DEVREADWRITE8("intc",      h8_intc_device,            iscr_r,  iscr_w,  0xff00)
	AM_RANGE(0xffc6, 0xffc7) AM_DEVREADWRITE8("intc",      h8_intc_device,            ier_r,   ier_w,   0x00ff)
	AM_RANGE(0xffc8, 0xffc9) AM_DEVREADWRITE8("timer8_0",  h8_timer8_channel_device,  tcr_r,   tcr_w,   0xff00)
	AM_RANGE(0xffc8, 0xffc9) AM_DEVREADWRITE8("timer8_0",  h8_timer8_channel_device,  tcsr_r,  tcsr_w,  0x00ff)
	AM_RANGE(0xffca, 0xffcb) AM_DEVREADWRITE8("timer8_0",  h8_timer8_channel_device,  tcor_r,  tcor_w,  0xffff)
	AM_RANGE(0xffcc, 0xffcd) AM_DEVREADWRITE8("timer8_0",  h8_timer8_channel_device,  tcnt_r,  tcnt_w,  0xff00)
	AM_RANGE(0xffd0, 0xffd1) AM_DEVREADWRITE8("timer8_1",  h8_timer8_channel_device,  tcr_r,   tcr_w,   0xff00)
	AM_RANGE(0xffd0, 0xffd1) AM_DEVREADWRITE8("timer8_1",  h8_timer8_channel_device,  tcsr_r,  tcsr_w,  0x00ff)
	AM_RANGE(0xffd2, 0xffd3) AM_DEVREADWRITE8("timer8_1",  h8_timer8_channel_device,  tcor_r,  tcor_w,  0xffff)
	AM_RANGE(0xffd4, 0xffd5) AM_DEVREADWRITE8("timer8_1",  h8_timer8_channel_device,  tcnt_r,  tcnt_w,  0xff00)
	AM_RANGE(0xffd8, 0xffd9) AM_DEVREADWRITE8("sci0",      h8_sci_device,             smr_r,   smr_w,   0xff00)
	AM_RANGE(0xffd8, 0xffd9) AM_DEVREADWRITE8("sci0",      h8_sci_device,             brr_r,   brr_w,   0x00ff)
	AM_RANGE(0xffda, 0xffdb) AM_DEVREADWRITE8("sci0",      h8_sci_device,             scr_r,   scr_w,   0xff00)
	AM_RANGE(0xffda, 0xffdb) AM_DEVREADWRITE8("sci0",      h8_sci_device,             tdr_r,   tdr_w,   0x00ff)
	AM_RANGE(0xffdc, 0xffdd) AM_DEVREADWRITE8("sci0",      h8_sci_device,             ssr_r,   ssr_w,   0xff00)
	AM_RANGE(0xffdc, 0xffdd) AM_DEVREAD8(     "sci0",      h8_sci_device,             rdr_r,            0x00ff)

	AM_RANGE(0xffe0, 0xffe7) AM_DEVREAD8(     "adc",       h8_adc_device,             addr8_r,          0xffff)
	AM_RANGE(0xffe8, 0xffe9) AM_DEVREADWRITE8("adc",       h8_adc_device,             adcsr_r, adcsr_w, 0xff00)
	AM_RANGE(0xffe8, 0xffe9) AM_DEVREADWRITE8("adc",       h8_adc_device,             adcr_r,  adcr_w,  0x00ff)

	AM_RANGE(0xfff2, 0xfff3) AM_DEVREADWRITE8("port6",     h8_port_device,            pcr_r,   pcr_w,   0xff00)
ADDRESS_MAP_END

machine_config_constructor h83337_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(h83337);
}

void h83337_device::execute_set_input(int inputnum, int state)
{
	intc->set_input(inputnum, state);
}

void h83337_device::irq_setup()
{
	CCR |= F_I;
}

void h83337_device::update_irq_filter()
{
	if(CCR & F_I)
		intc->set_filter(2, -1);
	else
		intc->set_filter(0, -1);
}

void h83337_device::interrupt_taken()
{
	standard_irq_callback(intc->interrupt_taken(taken_irq_vector));
}

void h83337_device::internal_update(UINT64 current_time)
{
	UINT64 event_time = 0;

	add_event(event_time, adc->internal_update(current_time));
	add_event(event_time, sci0->internal_update(current_time));
	add_event(event_time, sci1->internal_update(current_time));
	add_event(event_time, timer8_0->internal_update(current_time));
	add_event(event_time, timer8_1->internal_update(current_time));
	add_event(event_time, timer16_0->internal_update(current_time));

	recompute_bcount(event_time);
}

void h83337_device::device_start()
{
	h8_device::device_start();
}

void h83337_device::device_reset()
{
	h8_device::device_reset();
	syscr = 0x09;
}

READ8_MEMBER(h83337_device::syscr_r)
{
	return syscr;
}

WRITE8_MEMBER(h83337_device::syscr_w)
{
	syscr = data;
	logerror("%s: syscr = %02x\n", tag(), data);
}

READ8_MEMBER(h83337_device::wscr_r)
{
	return 0x00;
}

WRITE8_MEMBER(h83337_device::wscr_w)
{
	logerror("%s: wscr = %02x\n", tag(), data);
}

READ8_MEMBER(h83337_device::stcr_r)
{
	return 0x00;
}

WRITE8_MEMBER(h83337_device::stcr_w)
{
	logerror("%s: stcr = %02x\n", tag(), data);
	timer8_0->set_extra_clock_bit(data & 0x01);
	timer8_1->set_extra_clock_bit(data & 0x02);
}

READ8_MEMBER(h83337_device::mdcr_r)
{
	return 0x00;
}

WRITE8_MEMBER(h83337_device::mdcr_w)
{
	logerror("%s: mdcr = %02x\n", tag(), data);
}
