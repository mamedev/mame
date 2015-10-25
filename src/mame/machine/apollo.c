// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont
/*
 * apollo.c - Apollo DS3500 CPU Board
 *
 *  Created on: Jan 20, 2011
 *      Author: Hans Ostermeyer
 *
 * Contains:
 * - apollo_config.c - APOLLO DS3500 configuration
 * - apollo_csr.c - APOLLO DS3500 CPU Control and Status registers
 * - apollo_dma.c - APOLLO DS3500 DMA controllers
 * - apollo_pic.c - APOLLO DS3500 PIC8259 controllers
 * - apollo_ptm.c - APOLLO DS3500 Programmable Timer 6840
 * - apollo_rtc.c - APOLLO DS3500 RTC MC146818
 * - apollo_sio.c - APOLLO DS3500 SIO
 * - apollo_sio2.c - APOLLO DS3500 SIO2
 * - apollo_stdio.c - stdio terminal for mess
 * - apollo_3c505.h - Apollo 3C505 Ethernet controller
 *
 * see also:
 * - http://www.bitsavers.org/pdf/apollo/008778-03_DOMAIN_Series_3000_4000_Technical_Reference_Aug87.pdf
 * - http://www.freescale.com/files/32bit/doc/inactive/MC68681UM.pdf
 *
 *  SIO usage:
 *      SIO: ch A keyboard, ch B serial console
 *      SIO2: modem/printer?
 *
 */

#include "includes/apollo.h"

#include "bus/isa/omti8621.h"
#include "bus/isa/sc499.h"
#include "bus/isa/3c505.h"

#define APOLLO_IRQ_VECTOR 0xa0
#define APOLLO_IRQ_PTM 0
#define APOLLO_IRQ_SIO1 1
#define APOLLO_IRQ_PIC_SLAVE 3
#define APOLLO_IRQ_CTAPE 5
#define APOLLO_IRQ_FDC 6
#define APOLLO_IRQ_RTC 8 // DN3000 only
#define APOLLO_IRQ_SIO2 8 // DN3500
#define APOLLO_IRQ_ETH2 9
#define APOLLO_IRQ_ETH1 10
#define APOLLO_IRQ_DIAG 13
#define APOLLO_IRQ_WIN1 14

#define APOLLO_DMA_ETH1 6
#define APOLLO_DMA_ETH2 3

//##########################################################################
// machine/apollo_config.c - APOLLO DS3500 configuration
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

static UINT16 config = 0;

/***************************************************************************
 apollo_config - check configuration setting
 ***************************************************************************/

int apollo_config(int mask)
{
	return config & mask ? 1 : 0;
}

/***************************************************************************
 Input Ports
 ***************************************************************************/

INPUT_PORTS_START( apollo_config )
	PORT_START( "apollo_config" )
		PORT_CONFNAME(APOLLO_CONF_SERVICE_MODE, 0x00, "Normal/Service" )
		PORT_CONFSETTING(0x00, "Service" )
		PORT_CONFSETTING(APOLLO_CONF_SERVICE_MODE, "Normal " )

		PORT_CONFNAME(APOLLO_CONF_DISPLAY, APOLLO_CONF_8_PLANES, "Graphics Controller")
		PORT_CONFSETTING(APOLLO_CONF_8_PLANES, "8-Plane Color")
		PORT_CONFSETTING(APOLLO_CONF_4_PLANES, "4-Plane Color")
		PORT_CONFSETTING(APOLLO_CONF_MONO_15I, "15\" Monochrome")
//      PORT_CONFSETTING(APOLLO_CONF_MONO_19I, "19\" Monochrome")

		PORT_CONFNAME(APOLLO_CONF_GERMAN_KBD, 0x00, "German Keyboard")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_GERMAN_KBD, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_20_YEARS_AGO, APOLLO_CONF_20_YEARS_AGO, "20 Years Ago ...")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_20_YEARS_AGO, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_25_YEARS_AGO, APOLLO_CONF_25_YEARS_AGO, "25 Years Ago ...")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_25_YEARS_AGO, DEF_STR ( On ) )
#ifdef APOLLO_XXL
		PORT_CONFNAME(APOLLO_CONF_NODE_ID, APOLLO_CONF_NODE_ID, "Node ID from Disk")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_NODE_ID, DEF_STR ( On ) )
#endif
//      PORT_CONFNAME(APOLLO_CONF_IDLE_SLEEP, 0x00, "Idle Sleep")
//      PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
//      PORT_CONFSETTING(APOLLO_CONF_IDLE_SLEEP, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_TRAP_TRACE, 0x00, "Trap Trace")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_TRAP_TRACE, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_FPU_TRACE, 0x00, "FPU Trace")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_FPU_TRACE, DEF_STR ( On ) )
#ifdef APOLLO_XXL
		PORT_CONFNAME(APOLLO_CONF_DISK_TRACE, 0x00, "Disk Trace")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_DISK_TRACE, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_NET_TRACE, 0x00, "Network Trace")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_NET_TRACE, DEF_STR ( On ) )
#endif
INPUT_PORTS_END

class apollo_config_device : public device_t
{
public:
	apollo_config_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
};

extern const device_type APOLLO_CONF;

const device_type APOLLO_CONF = &device_creator<apollo_config_device>;

apollo_config_device::apollo_config_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, APOLLO_CONF, "Apollo Configuration", tag, owner, clock, "apollo_config", __FILE__)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void apollo_config_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apollo_config_device::device_start()
{
	MLOG1(("start apollo_config"));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apollo_config_device::device_reset()
{
	MLOG1(("reset apollo_config"));
	// load configuration
	config = machine().root_device().ioport("apollo_config")->read();
}



//##########################################################################
// machine/apollo_csr.c - APOLLO DS3500 CPU Control and Status registers
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

#define CPU_CONTROL_REGISTER_ADDRESS 0x010100

static UINT16 cpu_status_register = APOLLO_CSR_SR_BIT15 | APOLLO_CSR_SR_SERVICE;
static UINT16 cpu_control_register = 0x0000;

/*-------------------------------------------------
  apollo_csr_get/set_servicemode
 -------------------------------------------------*/

/*static int apollo_csr_get_servicemode()
{
    return cpu_status_register & APOLLO_CSR_SR_SERVICE ? 0 : 1;
}*/

static void apollo_csr_set_servicemode(int mode)
{
	apollo_csr_set_status_register(1, mode ? APOLLO_CSR_SR_SERVICE : 0);
}

UINT16 apollo_csr_get_control_register(void)
{
	return cpu_control_register;
}

UINT16 apollo_csr_get_status_register(void)
{
	return cpu_status_register;
}

void apollo_csr_set_status_register(UINT16 mask, UINT16 data)
{
	UINT16 new_value = (cpu_status_register & ~mask) | (data & mask);

	if (new_value != cpu_status_register)
	{
		cpu_status_register = new_value;
		LOG1(("#### setting CPU Status Register with data=%04x & %04x to %04x", data, mask, cpu_status_register));
	}
}

/*-------------------------------------------------
 DN3000/DN3500 CPU Status Register at 0x8000/0x10000
 -------------------------------------------------*/

WRITE16_MEMBER(apollo_state::apollo_csr_status_register_w){
	// To clear bus timeouts or parity conditions from status register,
	// write to the status register. This register is readonly.
	// in DN3500 bit 15 is always set (undocumented !?)
	cpu_status_register &= (APOLLO_CSR_SR_BIT15 | APOLLO_CSR_SR_FP_TRAP | APOLLO_CSR_SR_SERVICE);
	SLOG1(("writing CPU Status Register at offset %X = %04x & %04x (%04x)", offset, data, mem_mask, cpu_status_register));
}

READ16_MEMBER(apollo_state::apollo_csr_status_register_r){
	SLOG2(("reading CPU Status Register at offset %X = %04x & %04x", offset, cpu_status_register, mem_mask));
	return cpu_status_register & mem_mask;
}

/*-------------------------------------------------
 DN3000/DN3500 CPU Control Register at 0x8100/0x10100
 -------------------------------------------------*/

WRITE16_MEMBER(apollo_state::apollo_csr_control_register_w)
{
	int leds;

	if ((mem_mask & APOLLO_CSR_CR_FPU_TRAP_ENABLE) == 0)
	{
		// FPU Trap enable not involved
	}
	else if (((data ^ cpu_control_register) & APOLLO_CSR_CR_FPU_TRAP_ENABLE) == 0)
	{
		// FPU Trap enable remains unchanged
	}
	else if ((data & APOLLO_CSR_CR_FPU_TRAP_ENABLE) == 0)
	{
		// enable FPU (i.e. FPU opcodes in CPU)
		apollo_set_cpu_has_fpu(m_maincpu, 1);
	}
	else
	{
		// disable FPU (i.e. FPU opcodes in CPU)
		apollo_set_cpu_has_fpu(m_maincpu, 0);

		if (!apollo_is_dn3000())
		{
			// hack: set APOLLO_CSR_SR_FP_TRAP in cpu status register for /sau7/self_test
			// APOLLO_CSR_SR_FP_TRAP in status register should be set by next fmove instruction
			// cpu_status_register |= APOLLO_CSR_SR_FP_TRAP;
		}
	}

	cpu_control_register = (cpu_control_register & ~mem_mask) | (data & mem_mask);

	output_set_value("internal_led_1", (cpu_control_register >> 15) & 1);
	output_set_value("internal_led_2", (cpu_control_register >> 14) & 1);
	output_set_value("internal_led_3", (cpu_control_register >> 13) & 1);
	output_set_value("internal_led_4", (cpu_control_register >> 12) & 1);
	output_set_value("external_led_a", (cpu_control_register >> 11) & 1);
	output_set_value("external_led_b", (cpu_control_register >> 10) & 1);
	output_set_value("external_led_c", (cpu_control_register >> 9) & 1);
	output_set_value("external_led_d", (cpu_control_register >> 8) & 1);

	leds = ((cpu_control_register >> 8) & 0xff) ^ 0xff;

	SLOG1(("writing CPU Control Register at offset %X = %04x & %04x (%04x - %d%d%d%d %d%d%d%d)",
					offset, data, mem_mask, cpu_control_register,
					(leds >> 3) & 1,(leds >> 2) & 1, (leds >> 1) & 1, (leds >> 0) & 1,
					(leds >> 7) & 1,(leds >> 6) & 1, (leds >> 5) & 1, (leds >> 4) & 1 ));

	if (data & APOLLO_CSR_CR_RESET_DEVICES)
	{
		// FIXME: reset all devices (but not SIO lines!)
	}
}

READ16_MEMBER(apollo_state::apollo_csr_control_register_r)
{
	SLOG1(("reading CPU Control Register at offset %X = %04x & %04x", offset, cpu_control_register, mem_mask));
	return cpu_control_register & mem_mask;
}

//##########################################################################
// machine/apollo_dma.c - APOLLO DS3500 DMA
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

static UINT16 address_translation_map[0x400];

static UINT16 dma_page_register[16] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static const UINT8 channel2page_register[8] = { 7, 3, 1, 2, 0, 11, 9, 10};

static UINT8 dn3000_dma_channel1 = 1; // 1 = memory/ctape, 2 = floppy dma channel
static UINT8 dn3000_dma_channel2 = 5; // 5 = memory dma channel

/*-------------------------------------------------
 DN3000/DN3500 DMA Controller 1 at 0x9000/0x10c00
 -------------------------------------------------*/

WRITE8_MEMBER(apollo_state::apollo_dma_1_w){
	SLOG1(("apollo_dma_1_w: writing DMA Controller 1 at offset %02x = %02x", offset, data));
	m_dma8237_1->write(space, offset, data);
}

READ8_MEMBER(apollo_state::apollo_dma_1_r){
	UINT8 data = m_dma8237_1->read(space, offset);
	SLOG1(("apollo_dma_1_r: reading DMA Controller 1 at offset %02x = %02x", offset, data));
	return data;
}

/*-------------------------------------------------
 DN3000/DN3500 DMA Controller 2 at 0x9100/0x10d00
 -------------------------------------------------*/

WRITE8_MEMBER(apollo_state::apollo_dma_2_w){
	SLOG1(("apollo_dma_2_w: writing DMA Controller 2 at offset %02x = %02x", offset/2, data));
	m_dma8237_2->write(space, offset / 2, data);
}

READ8_MEMBER(apollo_state::apollo_dma_2_r){
	// Nasty hack (13-06-15 - ost):
	// MD self_test will test wrong DMA register and
	// mem-to-mem DMA in am9517a.c is often starting much too late (for MD self_test)
	// (8237dma.c was always fast enough to omit these problems)
	if (offset == 8)
	{
		switch (space.device().safe_pcbase())
		{
		case 0x00102e22: // DN3000
		case 0x01002f3c: // DN3500
		case 0x010029a6: // DN5500
			offset = 16;
			break;
		}
	}
	UINT8 data = m_dma8237_2->read(space, offset / 2);
	SLOG1(("apollo_dma_2_r: reading DMA Controller 2 at offset %02x = %02x", offset/2, data));
	return data;
}

/***************************************************************************
 DN3000 DMA Page Register at 0x9200
 ***************************************************************************/

WRITE8_MEMBER(apollo_state::apollo_dma_page_register_w){
	dma_page_register[offset & 0x0f] = data;
	SLOG1(("writing DMA Page Register at offset %02x = %02x", offset,  data));
}

READ8_MEMBER(apollo_state::apollo_dma_page_register_r){
	UINT8 data = dma_page_register[offset & 0x0f];
	SLOG1(("reading DMA Page Register at offset %02x = %02x", offset, data));
	return data;
}

/*-------------------------------------------------
 DN3500 Address Translation Map at 0x017000
 -------------------------------------------------*/

WRITE16_MEMBER(apollo_state::apollo_address_translation_map_w){
	address_translation_map[offset & 0x3ff] = data;
	SLOG2(("writing Address Translation Map at offset %02x = %04x", offset, data));
}

READ16_MEMBER(apollo_state::apollo_address_translation_map_r){
	UINT16 data = address_translation_map[offset & 0x3ff];
	SLOG2(("reading Address Translation Map at offset %02x = %04x", offset, data));
	return data;
}

READ8_MEMBER(apollo_state::apollo_dma_read_byte){
	UINT8 data;
	offs_t page_offset;

	if (apollo_is_dn3000()) {
		page_offset = dma_page_register[channel2page_register[dn3000_dma_channel1]] << 16;
	} else {
		// FIXME: address_translation_map[0x200] ?
		page_offset = (((offs_t) address_translation_map[0x200 + (offset >> 10)]) << 10) & 0x7FFFFC00;
		offset &= 0x3ff;
	}

	data = m_maincpu->space(AS_PROGRAM).read_byte(page_offset + offset);

	if (VERBOSE > 1 || offset < 4 || (offset & 0xff) == 0 || (offset & 0xff) == 0xff)
	{
		SLOG1(("dma read byte at offset %x+%03x = %02x", page_offset, offset, data));
	}
//  logerror(" %02x", data);
	return data;
}

WRITE8_MEMBER(apollo_state::apollo_dma_write_byte){
	offs_t page_offset;
	if (apollo_is_dn3000()) {
		page_offset = dma_page_register[channel2page_register[dn3000_dma_channel1]] << 16;
	} else {
		// FIXME: address_translation_map[0x200] ?
		page_offset = (((offs_t) address_translation_map[0x200 + (offset >> 10)]) << 10) & 0x7FFFFC00;
		offset &= 0x3ff;
	}
	// FIXME: MSB not available, writing only LSB
	m_maincpu->space(AS_PROGRAM).write_byte(page_offset + offset, data);

	if (VERBOSE > 1 || offset < 4 || (offset & 0xff) == 0 || (offset & 0xff) == 0xff)
	{
		SLOG1(("dma write byte at offset %x+%03x = %02x", page_offset, offset , data));
	}
//  logerror(" %02x", data);
}

READ8_MEMBER(apollo_state::apollo_dma_read_word){
	UINT16 data;
	offs_t page_offset;

	if (apollo_is_dn3000()) {
		page_offset = dma_page_register[channel2page_register[dn3000_dma_channel2]] << 16;
		page_offset &= 0xfffe0000;
		offset <<= 1;
	} else {
		// FIXME: address_translation_map[0x200] ?
		page_offset = (((offs_t) address_translation_map[0x200 + (offset >> 9)]) << 10) & 0x7FFFFC00;
		offset = (offset << 1) & 0x3ff;
	}

	data = m_maincpu->space(AS_PROGRAM).read_byte(page_offset + offset);

	SLOG1(("dma read word at offset %x+%03x = %04x", page_offset, offset , data));
	// FIXME: MSB will get lost
	return data;
}

WRITE8_MEMBER(apollo_state::apollo_dma_write_word){
	offs_t page_offset;

	SLOG1(("dma write word at offset %x = %02x", offset, data));

	if (apollo_is_dn3000()) {
		page_offset = dma_page_register[channel2page_register[dn3000_dma_channel2]] << 16;
		page_offset &= 0xfffe0000;
		offset <<= 1;
	} else {
		// FIXME: address_translation_map[0x200] ?
		page_offset
				= (((offs_t) address_translation_map[0x200 + (offset >> 9)])
						<< 10) & 0x7FFFFC00;
		offset = (offset << 1) & 0x3ff;
	}

	m_maincpu->space(AS_PROGRAM).write_byte(page_offset + offset, data);
	SLOG1(("dma write word at offset %x+%03x = %02x", page_offset, offset, data));
}

WRITE_LINE_MEMBER(apollo_state::apollo_dma8237_out_eop ) {
	CLOG1(("dma out eop state %02x", state));
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1)
		m_isa->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
}

WRITE_LINE_MEMBER(apollo_state::apollo_dma_1_hrq_changed ) {
	CLOG2(("dma 1 hrq changed state %02x", state));
	m_dma8237_1->dreq0_w(state);

	/* Assert HLDA */
	m_dma8237_1->hack_w(state);

	// cascade mode?
	// i8237_hlda_w(get_device_dma8237_2(device), state);
}

WRITE_LINE_MEMBER(apollo_state::apollo_dma_2_hrq_changed ) {
	CLOG2(("dma 2 hrq changed state %02x", state));
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_2->hack_w(state);
}

READ8_MEMBER( apollo_state::pc_dma8237_0_dack_r ) { return m_isa->dack_r(0); }
READ8_MEMBER( apollo_state::pc_dma8237_1_dack_r ) { return m_isa->dack_r(1); }
READ8_MEMBER( apollo_state::pc_dma8237_2_dack_r ) { return m_isa->dack_r(2); }
READ8_MEMBER( apollo_state::pc_dma8237_3_dack_r ) { return m_isa->dack_r(3); }
READ8_MEMBER( apollo_state::pc_dma8237_5_dack_r ) { return m_isa->dack_r(5); }
READ8_MEMBER( apollo_state::pc_dma8237_6_dack_r ) { return m_isa->dack_r(6); }
READ8_MEMBER( apollo_state::pc_dma8237_7_dack_r ) { return m_isa->dack_r(7); }

WRITE8_MEMBER( apollo_state::pc_dma8237_0_dack_w ){ m_isa->dack_w(0, data); }
WRITE8_MEMBER( apollo_state::pc_dma8237_1_dack_w ){ m_isa->dack_w(1, data); }
WRITE8_MEMBER( apollo_state::pc_dma8237_2_dack_w ){ m_isa->dack_w(2, data); }
WRITE8_MEMBER( apollo_state::pc_dma8237_3_dack_w ){ m_isa->dack_w(3, data); }
WRITE8_MEMBER( apollo_state::pc_dma8237_5_dack_w ){ m_isa->dack_w(5, data); }
WRITE8_MEMBER( apollo_state::pc_dma8237_6_dack_w ){ m_isa->dack_w(6, data); }
WRITE8_MEMBER( apollo_state::pc_dma8237_7_dack_w ){ m_isa->dack_w(7, data); }

WRITE_LINE_MEMBER( apollo_state::pc_dack0_w ) { select_dma_channel(0, state); }
WRITE_LINE_MEMBER( apollo_state::pc_dack1_w ) { select_dma_channel(1, state); }
WRITE_LINE_MEMBER( apollo_state::pc_dack2_w ) { select_dma_channel(2, state); }
WRITE_LINE_MEMBER( apollo_state::pc_dack3_w ) { select_dma_channel(3, state); }
WRITE_LINE_MEMBER( apollo_state::pc_dack4_w ) { m_dma8237_1->hack_w( state ? 0 : 1); } // it's inverted
WRITE_LINE_MEMBER( apollo_state::pc_dack5_w ) { select_dma_channel(5, state); }
WRITE_LINE_MEMBER( apollo_state::pc_dack6_w ) { select_dma_channel(6, state); }
WRITE_LINE_MEMBER( apollo_state::pc_dack7_w ) { select_dma_channel(7, state); }

void apollo_state::select_dma_channel(int channel, bool state)
{
	if(!state) {
		m_dma_channel = channel;
		if(m_cur_eop)
			m_isa->eop_w(channel, ASSERT_LINE );

	} else if(m_dma_channel == channel) {
		m_dma_channel = -1;
		if(m_cur_eop)
			m_isa->eop_w(channel, CLEAR_LINE );
	}
}

//##########################################################################
// machine/apollo_pic.c - APOLLO DS3500 PIC 8259 controllers
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

void apollo_state::apollo_pic_set_irq_line(int irq, int state)
{
	switch (irq) {
	case 0: m_pic8259_master->ir0_w(state); break;
	case 1: m_pic8259_master->ir1_w(state); break;
	case 2: m_pic8259_master->ir2_w(state); break;
	case 3: m_pic8259_master->ir3_w(state); break;
	case 4: m_pic8259_master->ir4_w(state); break;
	case 5: m_pic8259_master->ir5_w(state); break;
	case 6: m_pic8259_master->ir6_w(state); break;
	case 7: m_pic8259_master->ir7_w(state); break;

	case 8:  m_pic8259_slave->ir0_w(state); break;
	case 9:  m_pic8259_slave->ir1_w(state); break;
	case 10: m_pic8259_slave->ir2_w(state); break;
	case 11: m_pic8259_slave->ir3_w(state); break;
	case 12: m_pic8259_slave->ir4_w(state); break;
	case 13: m_pic8259_slave->ir5_w(state); break;
	case 14: m_pic8259_slave->ir6_w(state); break;
	case 15: m_pic8259_slave->ir7_w(state); break;
	}
}

IRQ_CALLBACK_MEMBER(apollo_state::apollo_pic_acknowledge)
{
	UINT32 vector = m_pic8259_master->acknowledge();
	if ((vector & 0x0f) == APOLLO_IRQ_PIC_SLAVE) {
		vector = m_pic8259_slave->acknowledge();
	}

	// don't log ptm interrupts
	if (vector != APOLLO_IRQ_VECTOR+APOLLO_IRQ_PTM) {
		MLOG1(("apollo_pic_acknowledge: irq=%d vector=%x", vector & 0x0f, vector));
	}

	if (apollo_is_dn3000()) {
		apollo_csr_set_status_register(APOLLO_CSR_SR_INTERRUPT_PENDING, 0);
	} else {
		// clear bit Interrupt Pending in Cache Status Register
		apollo_set_cache_status_register(0x10, 0x00);
	}
	return vector;
}

/*************************************************************
 * pic8259 configuration
 *************************************************************/

READ8_MEMBER( apollo_state::apollo_pic8259_get_slave_ack )
{
		MLOG1(("apollo_pic8259_get_slave_ack: offset=%x", offset));

		return offset == 3 ? m_pic8259_slave->acknowledge() : 0;
}

WRITE_LINE_MEMBER( apollo_state::apollo_pic8259_master_set_int_line ) {
	static int interrupt_line = -1;
	if (state != interrupt_line) {
		device_t *device = m_pic8259_master;
		DLOG1(("apollo_pic8259_master_set_int_line: %x", state));
	}
	interrupt_line = state;

	if (apollo_is_dn3000()) {
		apollo_csr_set_status_register(APOLLO_CSR_SR_INTERRUPT_PENDING,
				state ? APOLLO_CSR_SR_INTERRUPT_PENDING : 0);
	} else {
		// set bit Interrupt Pending in Cache Status Register
		apollo_set_cache_status_register(0x10, state ? 0x10 : 0x00);
	}

	m_maincpu->set_input_line_and_vector(M68K_IRQ_6,state ? ASSERT_LINE : CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR);
}

WRITE_LINE_MEMBER( apollo_state::apollo_pic8259_slave_set_int_line ) {
	static int interrupt_line = -1;
	if (state != interrupt_line) {
		device_t *device = m_pic8259_slave;
		DLOG1(("apollo_pic8259_slave_set_int_line: %x", state));
		interrupt_line = state;
		apollo_pic_set_irq_line(3, state);
	}
}

//##########################################################################
// machine/apollo_ptm.c - APOLLO DS3500 Programmable Timer 6840
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

WRITE_LINE_MEMBER(apollo_state::apollo_ptm_timer_tick)
{
	if ((state) && (m_ptm->started()))
	{
		ptm_counter++;
		m_ptm->set_c1( 1);
		m_ptm->set_c1( 0);
		m_ptm->set_c2(ptm_counter & 1);

		if ((ptm_counter & 1) == 0)
		{
			m_ptm->set_c3((ptm_counter >> 1) & 1);
		}
	}
}

WRITE_LINE_MEMBER(apollo_state::apollo_ptm_irq_function)
{
	apollo_pic_set_irq_line(APOLLO_IRQ_PTM, state);
}

//  Timer 1's input is a 250-kHz (4-microsecond period) signal.
//  Timer 2's input is a 125-kHz (8-microsecond period) signal.
//  Timer 3's input is a 62.5-kHz (16-microsecond period) signal.
//  The Timer 3 input may be prescaled to make the effective input signal have a 128-microsecond period.


//##########################################################################
// machine/apollo_rtc.c - APOLLO DS3500 RTC MC146818
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

/***************************************************************************
 DN3000/DN3500 Realtime Calendar MC146818 at 0x8900/0x10900
 ***************************************************************************/

WRITE8_MEMBER(apollo_state::apollo_rtc_w)
{
	m_rtc->write(space, 0, offset);
	m_rtc->write(space, 1, data);
	if (offset >= 0x0b && offset <= 0x0c)
	{
		SLOG2(("writing MC146818 at offset %02x = %02x", offset, data));
	}
}

READ8_MEMBER(apollo_state::apollo_rtc_r)
{
	UINT8 data;
	m_rtc->write(space, 0, offset);
	data = m_rtc->read(space, 1);
	if (offset >= 0x0b && offset <= 0x0c)
	{
		SLOG2(("reading MC146818 at offset %02x = %02x", offset, data));
	}
	return data;
}

// TODO: this is covering for missing mc146818 functionality
TIMER_CALLBACK_MEMBER( apollo_state::apollo_rtc_timer )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	// FIXME: reading register 0x0c will clear all interrupt flags
	if ((apollo_rtc_r(space, 0x0c) & 0x80))
	{
		//SLOG2(("apollo_rtc_timer - set_irq_line %d", APOLLO_IRQ_RTC));
		apollo_pic_set_irq_line(APOLLO_IRQ_RTC, 1);
	}
}

//##########################################################################
// machine/apollo_sio.c - DN3000/DS3500 SIO at 0x8400/0x10400
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

apollo_sio::apollo_sio(const machine_config &mconfig, const char *tag,
		device_t *owner, UINT32 clock) :
	mc68681_device(mconfig, tag, owner, clock),
	m_csrb(0),
	m_ip6(0)
{
}

void apollo_sio::device_reset()
{
	UINT8 input_data = apollo_get_ram_config_byte();
	ip0_w((input_data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
	ip1_w((input_data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
	ip2_w((input_data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
	ip3_w((input_data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	ip4_w((input_data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
	ip5_w((input_data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
//  ip6_w((input_data & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	// MC2681 has IP[6] (instead of /IACK on MC68681)
	m_ip6 = (input_data & 0x40) ? ASSERT_LINE : CLEAR_LINE;
}

READ8_MEMBER( apollo_sio::read )
{
	static int last_read8_offset[2] = { -1, -1 };
	static int last_read8_value[2] = { -1, -1 };

	static const char * const duart68681_reg_read_names[0x10] = { "MRA", "SRA",
			"BRG Test", "RHRA", "IPCR", "ISR", "CTU", "CTL", "MRB", "SRB",
			"1X/16X Test", "RHRB", "IVR", "Input Ports", "Start Counter",
			"Stop Counter" };

	int data = mc68681_device::read(space, offset/2, mem_mask);

	switch (offset / 2)
	{
	case 0x0b: /* RHRB */
		if (m_csrb == 0x77 && data == 0xfe)
		{
			// special fix for the MD ROM baudrate recognition
			// fix data only if CR is entered while baudrate is set to 2000 Baud

			// Receive and transmit clock in diserial.c are not precise enough
			// to support the baudrate recognition done in the Apollo MD ROM
			// use 0xff instead of 0xfe to set the baudrate recognition for 9600 Bd
			// (to prevent that the MD selftest or SK command will hang in Service mode)
			data = 0xff;
		}
		break;
	case 0x0d: /* IP */
		// MC2681 has IP[6] (instead of /IACK on MC68681)
		data = (data & ~0x40) | (m_ip6 ? 0x40 : 0);
		break;
	}

	// omit logging if sio is being polled from the boot rom
	if ((offset != last_read8_offset[1] || data != last_read8_value[1]) && \
		(offset != last_read8_offset[0] || data != last_read8_value[0]))
	{
		last_read8_offset[0] = last_read8_offset[1];
		last_read8_value[0] = last_read8_value[1];
		last_read8_offset[1] = offset;
		last_read8_value[1] = data;
		CLOG2(("reading MC2681 reg %02x (%s) returned %02x",
				offset, duart68681_reg_read_names[(offset/2) & 15], data));
	}

	return data;
}

WRITE8_MEMBER( apollo_sio::write )
{
	static const char * const duart68681_reg_write_names[0x10] = { "MRA",
			"CSRA", "CRA", "THRA", "ACR", "IMR", "CRUR", "CTLR", "MRB", "CSRB",
			"CRB", "THRB", "IVR", "OPCR", "Set OP Bits", "Reset OP Bits" };

	CLOG2(("writing MC2681 reg %02x (%s) with %02x",
			offset, duart68681_reg_write_names[(offset/2) & 15], data));

	switch (offset / 2) {
	case 0x09: /* CSRB */
		// remember CSRB to handle MD selftest or SK command
		m_csrb = data;
		break;
#if 1
	case 0x0b: /* THRB */
		// tee output of SIO1 to stdout
		// sad: ceterm will get confused from '\r'
		if (apollo_is_dsp3x00() && data != '\r') ::putchar(data);
		break;
#endif
	}
	mc68681_device::write(space, offset/2, data, mem_mask);
}

// device type definition
const device_type APOLLO_SIO = &device_creator<apollo_sio>;

WRITE_LINE_MEMBER(apollo_state::sio_irq_handler)
{
	apollo_pic_set_irq_line(APOLLO_IRQ_SIO1, state);
}

WRITE8_MEMBER(apollo_state::sio_output)
{
//  CLOG2(("apollo_sio - sio_output %02x", data));

	if ((data & 0x80) != (sio_output_data & 0x80))
	{
		apollo_pic_set_irq_line(APOLLO_IRQ_DIAG, (data & 0x80) ? 1 : 0);
	}

	// The counter/timer on the SIO chip is used for the RAM refresh count.
	// This is set up in the timer mode to produce a square wave output on output OP3.
	// The period of the output is 15 microseconds.

	if ((data & 0x08) != (sio_output_data & 0x08))
	{
		m_sio->ip0_w((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}

	sio_output_data = data;
}

//##########################################################################
// machine/apollo_sio2.c - APOLLO DS3500 SIO2
//##########################################################################

WRITE_LINE_MEMBER(apollo_state::sio2_irq_handler)
{
	apollo_pic_set_irq_line(APOLLO_IRQ_SIO2, state);
}

//##########################################################################
// machine/apollo_ni.c - APOLLO DS3500 node ID
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

#define DEFAULT_NODE_ID 0x12345

/***************************************************************************
 IMPLEMENTATION
 ***************************************************************************/

/*** Apollo Node ID device ***/

// device type definition
const device_type APOLLO_NI = &device_creator<apollo_ni> ;

//-------------------------------------------------
//  apollo_ni - constructor
//-------------------------------------------------

apollo_ni::apollo_ni(const machine_config &mconfig, const char *tag,
		device_t *owner, UINT32 clock) :
	device_t(mconfig, APOLLO_NI, "Node ID", tag, owner, clock, "node ID",
			__FILE__), device_image_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  apollo_ni - destructor
//-------------------------------------------------

apollo_ni::~apollo_ni()
{
}

void apollo_ni::device_config_complete()
{
	update_names(APOLLO_NI, "node_id", "ni");
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apollo_ni::device_start()
{
	CLOG1(("apollo_ni::device_start"));
	set_node_id(DEFAULT_NODE_ID);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apollo_ni::device_reset()
{
	CLOG1(("apollo_ni::device_reset"));
}

//-------------------------------------------------
//  set node ID
//-------------------------------------------------

void apollo_ni::set_node_id(UINT32 node_id)
{
	m_node_id = node_id;
	CLOG1(("apollo_ni::set_node_id: node ID is %x", node_id));
}

//-------------------------------------------------
//  read/write
//-------------------------------------------------

WRITE16_MEMBER(apollo_ni::write)
{
	CLOG1(("Error: writing node id ROM at offset %02x = %04x & %04x", offset, data, mem_mask));
}

READ16_MEMBER(apollo_ni::read)
{
	UINT16 data = 0;
	switch (offset & 0x0f)
	{
	case 1: // msb
		data = (m_node_id >> 16) & 0xff;
		break;
	case 2:
		data = (m_node_id >> 8) & 0xff;
		break;
	case 3: // lsb
		data = m_node_id & 0xff;
		break;
	case 15: // checksum
		data = ((m_node_id >> 16) + (m_node_id >> 8) + m_node_id) & 0xff;
		break;
	default:
		data = 0;
		break;
	}
	data <<= 8;
	CLOG2(("reading node id ROM at offset %02x = %04x & %04x", offset, data, mem_mask));
	return data;
}

/*-------------------------------------------------
 DEVICE_IMAGE_LOAD( rom )
 -------------------------------------------------*/
bool apollo_ni::call_load()
{
	CLOG1(("apollo_ni::call_load: %s", filename()));

	UINT64 size = length();
	 if (size != 32)
	{
		CLOG(("apollo_ni::call_load: %s has unexpected file size %" I64FMT "d", filename(), size));
	}
	else
	{
		UINT8 data[32];
		fread(data, sizeof(data));

		UINT8 checksum = data[2] + data[4] + data[6];
		if (checksum != data[30])
		{
			CLOG(("apollo_ni::call_load: checksum is %02x - should be %02x", checksum, data[30]));
		}
		else
		{
			m_node_id = (((data[2] << 8) | data[4]) << 8) | (data[6]);
			CLOG1(("apollo_ni::call_load: node ID is %x", m_node_id));
			return IMAGE_INIT_PASS;
		}
	}
	return IMAGE_INIT_FAIL;
}

/*-------------------------------------------------
 DEVICE_IMAGE_CREATE( rom )
 -------------------------------------------------*/

bool apollo_ni::call_create(int format_type, option_resolution *format_options)
{
	CLOG1(("apollo_ni::call_create:"));

	if (length() > 0)
	{
		CLOG(("apollo_ni::call_create: %s already exists", filename()));
	}
	else
	{
		UINT32 node_id = 0;
		sscanf(basename_noext(), "%x", &node_id);
		if (node_id == 0 || node_id > 0xfffff)
		{
			CLOG(("apollo_ni::call_create: filename %s is no valid node ID", basename()));
		}
		else
		{
			UINT8 data[32];
			memset(data, 0, sizeof(data));
			data[2] = node_id >> 16;
			data[4] = node_id >> 8;
			data[6] = node_id;
			data[30] = data[2] + data[4] + data[6];
			fwrite(data, sizeof(data));
			CLOG(("apollo_ni::call_create: created %s with node ID %x", filename(), node_id));
			set_node_id(node_id);
			return IMAGE_INIT_PASS;
		}
	}
	return IMAGE_INIT_FAIL;
}

/*-------------------------------------------------
 DEVICE_IMAGE_UNLOAD( rom )
 -------------------------------------------------*/
void apollo_ni::call_unload()
{
	CLOG1(("apollo_ni::call_unload:"));
}

//-------------------------------------------------
//  set node ID from disk
//-------------------------------------------------

void apollo_ni::set_node_id_from_disk()
{
#ifdef APOLLO_XXL
	// set node ID from UID of logical volume 1 of logical unit 0
	UINT8 db[0x50];

	// check label of physical volume and get sector data of logical volume 1
	// Note: sector data starts with 32 byte block header
	if (omti8621_device::get_sector(0, db, sizeof(db), 0) == sizeof(db)
			&& memcmp(db + 0x22, "APOLLO", 6) == 0)
	{
		UINT16 sector1 = apollo_is_dn5500() ? 4 : 1;

		if (omti8621_device::get_sector(sector1, db, sizeof(db), 0) == sizeof(db))
		{
			// set node_id from UID of logical volume 1 of logical unit 0
			m_node_id = (((db[0x49] << 8) | db[0x4a]) << 8) | db[0x4b];
			CLOG1(("apollo_ni::set_node_id_from_disk: node ID is %x", m_node_id));
		}
	}
#endif
}

//##########################################################################
// machine/apollo.c - APOLLO DS3500 CPU Board
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

static SLOT_INTERFACE_START(apollo_isa_cards)
	SLOT_INTERFACE("wdc", ISA16_OMTI8621)   // Combo ESDI/AT floppy controller
	SLOT_INTERFACE("ctape", ISA8_SC499)     // Archive SC499 cartridge tape
	SLOT_INTERFACE("3c505", ISA16_3C505)   // 3Com 3C505 Ethernet card
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( common )
	// configuration MUST be reset first !
	MCFG_DEVICE_ADD(APOLLO_CONF_TAG, APOLLO_CONF, 0)

	MCFG_DEVICE_ADD( APOLLO_DMA1_TAG, AM9517A, XTAL_14_31818MHz/3 )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(apollo_state, apollo_dma_1_hrq_changed))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(apollo_state, apollo_dma8237_out_eop))
	MCFG_I8237_IN_MEMR_CB(READ8(apollo_state, apollo_dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(apollo_state, apollo_dma_write_byte))
	MCFG_I8237_IN_IOR_0_CB(READ8(apollo_state, pc_dma8237_0_dack_r))
	MCFG_I8237_IN_IOR_1_CB(READ8(apollo_state, pc_dma8237_1_dack_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(apollo_state, pc_dma8237_2_dack_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(apollo_state, pc_dma8237_3_dack_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(apollo_state, pc_dma8237_0_dack_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(apollo_state, pc_dma8237_1_dack_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(apollo_state, pc_dma8237_2_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(apollo_state, pc_dma8237_3_dack_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(apollo_state, pc_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(apollo_state, pc_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(apollo_state, pc_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(apollo_state, pc_dack3_w))
	MCFG_DEVICE_ADD( APOLLO_DMA2_TAG, AM9517A, XTAL_14_31818MHz/3 )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(apollo_state, apollo_dma_2_hrq_changed))
	MCFG_I8237_IN_MEMR_CB(READ8(apollo_state, apollo_dma_read_word))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(apollo_state, apollo_dma_write_word))
	MCFG_I8237_IN_IOR_1_CB(READ8(apollo_state, pc_dma8237_5_dack_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(apollo_state, pc_dma8237_6_dack_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(apollo_state, pc_dma8237_7_dack_r))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(apollo_state, pc_dma8237_5_dack_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(apollo_state, pc_dma8237_6_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(apollo_state, pc_dma8237_7_dack_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(apollo_state, pc_dack4_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(apollo_state, pc_dack5_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(apollo_state, pc_dack6_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(apollo_state, pc_dack7_w))
	MCFG_PIC8259_ADD( APOLLO_PIC1_TAG, WRITELINE(apollo_state,apollo_pic8259_master_set_int_line), VCC, READ8(apollo_state, apollo_pic8259_get_slave_ack))
	MCFG_PIC8259_ADD( APOLLO_PIC2_TAG, WRITELINE(apollo_state,apollo_pic8259_slave_set_int_line), GND, NULL)

	MCFG_DEVICE_ADD(APOLLO_PTM_TAG, PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(0)
	MCFG_PTM6840_EXTERNAL_CLOCKS(250000, 125000, 62500)
	MCFG_PTM6840_IRQ_CB(WRITELINE(apollo_state, apollo_ptm_irq_function))
	MCFG_DEVICE_ADD("ptmclock", CLOCK, 250000)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(apollo_state, apollo_ptm_timer_tick))

	MCFG_MC146818_ADD( APOLLO_RTC_TAG, XTAL_32_768kHz )
	MCFG_MC146818_UTC( true )
	MCFG_MC146818_BINARY( false )
	MCFG_MC146818_24_12( false )
	MCFG_MC146818_EPOCH( 0 )

	MCFG_APOLLO_NI_ADD( APOLLO_NI_TAG, 0 )

	MCFG_APOLLO_SIO_ADD( APOLLO_SIO2_TAG, XTAL_3_6864MHz )
	MCFG_APOLLO_SIO_IRQ_CALLBACK(WRITELINE(apollo_state, sio2_irq_handler))

	MCFG_DEVICE_ADD(APOLLO_ISA_TAG, ISA16, 0)
	MCFG_ISA16_CPU(":" MAINCPU)
	MCFG_ISA16_BUS_CUSTOM_SPACES()
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE(APOLLO_PIC2_TAG, pic8259_device, ir2_w)) // in place of irq 2 on at irq 9 is used
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE(APOLLO_PIC1_TAG, pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE(APOLLO_PIC1_TAG, pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE(APOLLO_PIC1_TAG, pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE(APOLLO_PIC1_TAG, pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE(APOLLO_PIC1_TAG, pic8259_device, ir7_w))
	MCFG_ISA_OUT_IRQ10_CB(DEVWRITELINE(APOLLO_PIC2_TAG, pic8259_device, ir2_w))
	MCFG_ISA_OUT_IRQ11_CB(DEVWRITELINE(APOLLO_PIC2_TAG, pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ12_CB(DEVWRITELINE(APOLLO_PIC2_TAG, pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ14_CB(DEVWRITELINE(APOLLO_PIC2_TAG, pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ15_CB(DEVWRITELINE(APOLLO_PIC2_TAG, pic8259_device, ir7_w))
	MCFG_ISA_OUT_DRQ0_CB(DEVWRITELINE(APOLLO_DMA1_TAG, am9517a_device, dreq0_w))
	MCFG_ISA_OUT_DRQ1_CB(DEVWRITELINE(APOLLO_DMA1_TAG, am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(DEVWRITELINE(APOLLO_DMA1_TAG, am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(DEVWRITELINE(APOLLO_DMA1_TAG, am9517a_device, dreq3_w))
	MCFG_ISA_OUT_DRQ5_CB(DEVWRITELINE(APOLLO_DMA2_TAG, am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ6_CB(DEVWRITELINE(APOLLO_DMA2_TAG, am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ7_CB(DEVWRITELINE(APOLLO_DMA2_TAG, am9517a_device, dreq3_w))
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa1", apollo_isa_cards, "wdc", false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa2", apollo_isa_cards, "ctape", false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa3", apollo_isa_cards, "3c505", false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa4", apollo_isa_cards, NULL, false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa5", apollo_isa_cards, NULL, false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa6", apollo_isa_cards, NULL, false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa7", apollo_isa_cards, NULL, false)

	MCFG_SOFTWARE_LIST_ADD("ctape_list", "apollo_ctape")
MACHINE_CONFIG_END

// for machines with the keyboard and a graphics head
MACHINE_CONFIG_FRAGMENT( apollo )
	MCFG_FRAGMENT_ADD(common)
	MCFG_APOLLO_SIO_ADD( APOLLO_SIO_TAG, XTAL_3_6864MHz )
	MCFG_APOLLO_SIO_IRQ_CALLBACK(WRITELINE(apollo_state, sio_irq_handler))
	MCFG_APOLLO_SIO_OUTPORT_CALLBACK(WRITE8(apollo_state, sio_output))
	MCFG_APOLLO_SIO_A_TX_CALLBACK(DEVWRITELINE(APOLLO_KBD_TAG, apollo_kbd_device, rx_w))

#ifdef APOLLO_XXL
	MCFG_APOLLO_SIO_B_TX_CALLBACK(DEVWRITELINE(APOLLO_STDIO_TAG, apollo_stdio_device, rx_w))
#endif
MACHINE_CONFIG_END

static DEVICE_INPUT_DEFAULTS_START( apollo_terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

// for headless machines using a serial console
MACHINE_CONFIG_FRAGMENT( apollo_terminal )
	MCFG_FRAGMENT_ADD(common)
	MCFG_APOLLO_SIO_ADD( APOLLO_SIO_TAG, XTAL_3_6864MHz )
	MCFG_APOLLO_SIO_IRQ_CALLBACK(WRITELINE(apollo_state, sio_irq_handler))
	MCFG_APOLLO_SIO_OUTPORT_CALLBACK(WRITE8(apollo_state, sio_output))
	MCFG_APOLLO_SIO_B_TX_CALLBACK(DEVWRITELINE("rs232", rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(APOLLO_SIO_TAG, apollo_sio, rx_b_w))

	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", apollo_terminal)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(apollo_state,apollo)
{
	MLOG1(("driver_init_apollo"));
}

MACHINE_START_MEMBER(apollo_state,apollo)
{
	MLOG1(("machine_start_apollo"));

	if (apollo_is_dn3000())
	{
		//MLOG1(("faking mc146818 interrupts (DN3000 only)"));
		// fake mc146818 interrupts (DN3000 only)
		m_dn3000_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(apollo_state::apollo_rtc_timer),this));
	}
}

MACHINE_RESET_MEMBER(apollo_state,apollo)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 year = apollo_rtc_r(space, 9);

	m_dma_channel = -1;
	m_cur_eop = false;

	MLOG1(("machine_reset_apollo"));

	// set configuration
	apollo_csr_set_servicemode(apollo_config(APOLLO_CONF_SERVICE_MODE));

	// change year according to configuration settings
	if (year < 25 && apollo_config(APOLLO_CONF_25_YEARS_AGO))
	{
		year += 75;
		apollo_rtc_w(space, 9, year);
	}
	else if (year < 20 && apollo_config(APOLLO_CONF_20_YEARS_AGO))
	{
		year += 80;
		apollo_rtc_w(space, 9, year);
	}
	else if (year >= 80 && !apollo_config(APOLLO_CONF_20_YEARS_AGO)
			&& !apollo_config(APOLLO_CONF_25_YEARS_AGO))
	{
		year -= 80;
		apollo_rtc_w(space, 9, year);
	}

	ptm_counter = 0;
	sio_output_data = 0xff;

	if (apollo_is_dn3000())
	{
		m_dn3000_timer->adjust(attotime::from_hz(2), 0, attotime::from_hz(2));
	}
}

#ifdef APOLLO_XXL

//##########################################################################
// machine/apollo_stdio.c - stdio terminal for mess
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#endif

/***************************************************************************
 IMPLEMENTATION
 ***************************************************************************/

// device type definition
const device_type APOLLO_STDIO = &device_creator<apollo_stdio_device> ;

//-------------------------------------------------
// apollo_stdio_device - constructor
//-------------------------------------------------

apollo_stdio_device::apollo_stdio_device(const machine_config &mconfig,
		const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, APOLLO_STDIO, "Apollo STDIO", tag, owner, clock,
			"apollo_stdio", __FILE__), device_serial_interface(mconfig, *this),
			m_tx_w(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apollo_stdio_device::device_start()
{
	CLOG1(("device_start"));

	m_tx_w.resolve_safe();

	m_poll_timer = machine().scheduler().timer_alloc(timer_expired_delegate(
			FUNC(apollo_stdio_device::poll_timer), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apollo_stdio_device::device_reset()
{
	CLOG1(("device_reset"));

	// comms is at 8N1, 9600 baud
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(9600);
	set_tra_rate(9600);

	m_tx_busy = false;
	m_xmit_read = m_xmit_write = 0;

#if defined(__linux__)
	// FIXME: unavailable in mingw
	// set stdin to nonblocking to allow polling
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
#endif

	// start timer
	m_poll_timer->adjust(attotime::zero, 0, attotime::from_msec(1)); // every 1ms
}

void apollo_stdio_device::device_timer(emu_timer &timer, device_timer_id id,
		int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}

void apollo_stdio_device::rcv_complete() // Rx completed receiving byte
{
	receive_register_extract();
	UINT8 data = get_received_char();

	// output data to stdout (FIXME: '\r' may confuse ceterm)
	if (data != '\r')
	{
		::putchar(data);
		::fflush(stdout);
	}
	CLOG1(("rcv_complete %02x - %c", data, data));
}

void apollo_stdio_device::tra_complete() // Tx completed sending byte
{
	// is there more waiting to send?
	if (m_xmit_read != m_xmit_write)
	{
		transmit_register_setup(m_xmitring[m_xmit_read++]);
		if (m_xmit_read >= XMIT_RING_SIZE)
		{
			m_xmit_read = 0;
		}
	}
	else
	{
		m_tx_busy = false;
	}
}

void apollo_stdio_device::tra_callback() // Tx send bit
{
	int bit = transmit_register_get_data_bit();
	m_tx_w(bit);

	CLOG2(("tra_callback %02x", bit));
}

TIMER_CALLBACK_MEMBER(apollo_stdio_device::poll_timer)
{
#if defined(__linux__)
	UINT8 data;
	while (::read(STDIN_FILENO, &data, 1) == 1)
	{
		xmit_char(data == '\n' ? '\r' : data);
	}
#endif
}

void apollo_stdio_device::xmit_char(UINT8 data)
{
	CLOG1(("xmit_char %02x - %c", data, data));

	// if tx is busy it'll pick this up automatically when it completes
	if (!m_tx_busy)
	{
		m_tx_busy = true;
		transmit_register_setup(data);
	}
	else
	{
		// tx is busy, it'll pick this up next time
		m_xmitring[m_xmit_write++] = data;
		if (m_xmit_write >= XMIT_RING_SIZE)
		{
			m_xmit_write = 0;
		}
	}
}
#endif
