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

#include "emu.h"
#include "apollo.h"

#include "bus/isa/omti8621.h"
#include "bus/isa/sc499.h"
#include "bus/isa/3c505.h"

#include "softlist_dev.h"

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

static uint16_t config = 0;

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
		PORT_CONFSETTING(APOLLO_CONF_SERVICE_MODE, "Normal" )

		PORT_CONFNAME(APOLLO_CONF_DISPLAY, APOLLO_CONF_8_PLANES, "Graphics Controller")
		PORT_CONFSETTING(APOLLO_CONF_8_PLANES, "8-Plane Color")
		PORT_CONFSETTING(APOLLO_CONF_4_PLANES, "4-Plane Color")
		PORT_CONFSETTING(APOLLO_CONF_MONO_15I, "15\" Monochrome")
//      PORT_CONFSETTING(APOLLO_CONF_MONO_19I, "19\" Monochrome")

		PORT_CONFNAME(APOLLO_CONF_GERMAN_KBD, 0x00, "German Keyboard")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_GERMAN_KBD, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_30_YEARS_AGO, APOLLO_CONF_30_YEARS_AGO, "30 Years Ago ...")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_30_YEARS_AGO, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_25_YEARS_AGO, APOLLO_CONF_25_YEARS_AGO, "25 Years Ago ...")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_25_YEARS_AGO, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_NODE_ID, APOLLO_CONF_NODE_ID, "Node ID from Disk")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_NODE_ID, DEF_STR ( On ) )

//      PORT_CONFNAME(APOLLO_CONF_IDLE_SLEEP, 0x00, "Idle Sleep")
//      PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
//      PORT_CONFSETTING(APOLLO_CONF_IDLE_SLEEP, DEF_STR ( On ) )
#ifdef APOLLO_XXL
		PORT_CONFNAME(APOLLO_CONF_TRAP_TRACE, 0x00, "Trap Trace")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_TRAP_TRACE, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_FPU_TRACE, 0x00, "FPU Trace")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_FPU_TRACE, DEF_STR ( On ) )

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
	apollo_config_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	// internal state
};

DEFINE_DEVICE_TYPE(APOLLO_CONF, apollo_config_device, "apollo_config", "Apollo Configuration")

apollo_config_device::apollo_config_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, APOLLO_CONF, tag, owner, clock)
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

static uint16_t cpu_status_register = APOLLO_CSR_SR_BIT15 | APOLLO_CSR_SR_SERVICE;
static uint16_t cpu_control_register = 0x0000;

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

uint16_t apollo_csr_get_control_register(void)
{
	return cpu_control_register;
}

uint16_t apollo_csr_get_status_register(void)
{
	return cpu_status_register;
}

void apollo_csr_set_status_register(uint16_t mask, uint16_t data)
{
	uint16_t new_value = (cpu_status_register & ~mask) | (data & mask);

	if (new_value != cpu_status_register)
	{
		cpu_status_register = new_value;
		//LOG1(("#### setting CPU Status Register with data=%04x & %04x to %04x", data, mask, cpu_status_register));
	}
}

/*-------------------------------------------------
 DN3000/DN3500 CPU Status Register at 0x8000/0x10000
 -------------------------------------------------*/

void apollo_state::apollo_csr_status_register_w(offs_t offset, uint16_t data, uint16_t mem_mask){
	// To clear bus timeouts or parity conditions from status register,
	// write to the status register. This register is readonly.
	// in DN3500 bit 15 is always set (undocumented !?)
	cpu_status_register &= (APOLLO_CSR_SR_BIT15 | APOLLO_CSR_SR_FP_TRAP | APOLLO_CSR_SR_SERVICE);
	SLOG1(("writing CPU Status Register at offset %X = %04x & %04x (%04x)", offset, data, mem_mask, cpu_status_register));
}

uint16_t apollo_state::apollo_csr_status_register_r(offs_t offset, uint16_t mem_mask){
	SLOG2(("reading CPU Status Register at offset %X = %04x & %04x", offset, cpu_status_register, mem_mask));
	return cpu_status_register & mem_mask;
}

/*-------------------------------------------------
 DN3000/DN3500 CPU Control Register at 0x8100/0x10100
 -------------------------------------------------*/

void apollo_state::apollo_csr_control_register_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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

		if (!apollo_is_dn3000() && !m_maincpu->get_pmmu_enable())
		{
			// hack: set APOLLO_CSR_SR_FP_TRAP in cpu status register for /sau7/self_test
			// APOLLO_CSR_SR_FP_TRAP in status register should be set by next fmove instruction
			cpu_status_register |= APOLLO_CSR_SR_FP_TRAP;
		}
	}

	COMBINE_DATA(&cpu_control_register);

	for (int i = 0; i < 4; i++)
		m_internal_leds[i] = BIT(cpu_control_register, 15 - i);
	for (int i = 0; i < 4; i++)
		m_external_leds[i] = BIT(cpu_control_register, 11 - i);

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

uint16_t apollo_state::apollo_csr_control_register_r(offs_t offset, uint16_t mem_mask)
{
	SLOG1(("reading CPU Control Register at offset %X = %04x & %04x", offset, cpu_control_register, mem_mask));
	return cpu_control_register & mem_mask;
}

//##########################################################################
// machine/apollo_dma.c - APOLLO DS3500 DMA
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

static uint16_t address_translation_map[0x400];

static uint16_t dma_page_register[16] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static const uint8_t channel2page_register[8] = { 7, 3, 1, 2, 0, 11, 9, 10};

static uint8_t dn3000_dma_channel1 = 1; // 1 = memory/ctape, 2 = floppy dma channel
static uint8_t dn3000_dma_channel2 = 5; // 5 = memory dma channel

/*-------------------------------------------------
 DN3000/DN3500 DMA Controller 1 at 0x9000/0x10c00
 -------------------------------------------------*/

void apollo_state::apollo_dma_1_w(offs_t offset, uint8_t data){
	SLOG1(("apollo_dma_1_w: writing DMA Controller 1 at offset %02x = %02x", offset, data));
	m_dma8237_1->write(offset, data);
}

uint8_t apollo_state::apollo_dma_1_r(offs_t offset){
	uint8_t data = m_dma8237_1->read(offset);
	SLOG1(("apollo_dma_1_r: reading DMA Controller 1 at offset %02x = %02x", offset, data));
	return data;
}

/*-------------------------------------------------
 DN3000/DN3500 DMA Controller 2 at 0x9100/0x10d00
 -------------------------------------------------*/

void apollo_state::apollo_dma_2_w(offs_t offset, uint8_t data){
	SLOG1(("apollo_dma_2_w: writing DMA Controller 2 at offset %02x = %02x", offset/2, data));
	m_dma8237_2->write(offset / 2, data);
}

uint8_t apollo_state::apollo_dma_2_r(offs_t offset){
	// Nasty hack (13-06-15 - ost):
	// MD self_test will test wrong DMA register and
	// mem-to-mem DMA in am9517a.c is often starting much too late (for MD self_test)
	// (8237dma.c was always fast enough to omit these problems)
	if (offset == 8)
	{
		switch (m_maincpu->pcbase())
		{
		case 0x00102e22: // DN3000
		case 0x01002f3c: // DN3500
		case 0x010029a6: // DN5500
			offset = 16;
			break;
		}
	}
	uint8_t data = m_dma8237_2->read(offset / 2);
	SLOG1(("apollo_dma_2_r: reading DMA Controller 2 at offset %02x = %02x", offset/2, data));
	return data;
}

/***************************************************************************
 DN3000 DMA Page Register at 0x9200
 ***************************************************************************/

void apollo_state::apollo_dma_page_register_w(offs_t offset, uint8_t data){
	dma_page_register[offset & 0x0f] = data;
	SLOG1(("writing DMA Page Register at offset %02x = %02x", offset,  data));
}

uint8_t apollo_state::apollo_dma_page_register_r(offs_t offset){
	uint8_t data = dma_page_register[offset & 0x0f];
	SLOG1(("reading DMA Page Register at offset %02x = %02x", offset, data));
	return data;
}

/*-------------------------------------------------
 DN3500 Address Translation Map at 0x017000
 -------------------------------------------------*/

void apollo_state::apollo_address_translation_map_w(offs_t offset, uint16_t data){
	address_translation_map[offset & 0x3ff] = data;
	SLOG2(("writing Address Translation Map at offset %02x = %04x", offset, data));
}

uint16_t apollo_state::apollo_address_translation_map_r(offs_t offset){
	uint16_t data = address_translation_map[offset & 0x3ff];
	SLOG2(("reading Address Translation Map at offset %02x = %04x", offset, data));
	return data;
}

uint8_t apollo_state::apollo_dma_read_byte(offs_t offset){
	uint8_t data;
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

void apollo_state::apollo_dma_write_byte(offs_t offset, uint8_t data){
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

uint8_t apollo_state::apollo_dma_read_word(offs_t offset){
	uint16_t data;
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

void apollo_state::apollo_dma_write_word(offs_t offset, uint8_t data){
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

void apollo_state::apollo_dma8237_out_eop(int state) {
	CLOG1(("dma out eop state %02x", state));
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1)
		m_isa->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
}

void apollo_state::apollo_dma_1_hrq_changed(int state) {
	CLOG2(("dma 1 hrq changed state %02x", state));
	m_dma8237_1->dreq0_w(state);

	/* Assert HLDA */
	m_dma8237_1->hack_w(state);

	// cascade mode?
	// i8237_hlda_w(get_device_dma8237_2(device), state);
}

void apollo_state::apollo_dma_2_hrq_changed(int state) {
	CLOG2(("dma 2 hrq changed state %02x", state));
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_2->hack_w(state);
}

uint8_t apollo_state::pc_dma8237_0_dack_r() { return m_isa->dack_r(0); }
uint8_t apollo_state::pc_dma8237_1_dack_r() { return m_isa->dack_r(1); }
uint8_t apollo_state::pc_dma8237_2_dack_r() { return m_isa->dack_r(2); }
uint8_t apollo_state::pc_dma8237_3_dack_r() { return m_isa->dack_r(3); }
uint8_t apollo_state::pc_dma8237_5_dack_r() { return m_isa->dack_r(5); }
uint8_t apollo_state::pc_dma8237_6_dack_r() { return m_isa->dack_r(6); }
uint8_t apollo_state::pc_dma8237_7_dack_r() { return m_isa->dack_r(7); }

void apollo_state::pc_dma8237_0_dack_w(uint8_t data){ m_isa->dack_w(0, data); }
void apollo_state::pc_dma8237_1_dack_w(uint8_t data){ m_isa->dack_w(1, data); }
void apollo_state::pc_dma8237_2_dack_w(uint8_t data){ m_isa->dack_w(2, data); }
void apollo_state::pc_dma8237_3_dack_w(uint8_t data){ m_isa->dack_w(3, data); }
void apollo_state::pc_dma8237_5_dack_w(uint8_t data){ m_isa->dack_w(5, data); }
void apollo_state::pc_dma8237_6_dack_w(uint8_t data){ m_isa->dack_w(6, data); }
void apollo_state::pc_dma8237_7_dack_w(uint8_t data){ m_isa->dack_w(7, data); }

void apollo_state::pc_dack0_w(int state) { select_dma_channel(0, state); }
void apollo_state::pc_dack1_w(int state) { select_dma_channel(1, state); }
void apollo_state::pc_dack2_w(int state) { select_dma_channel(2, state); }
void apollo_state::pc_dack3_w(int state) { select_dma_channel(3, state); }
void apollo_state::pc_dack4_w(int state) { m_dma8237_1->hack_w( state ? 0 : 1); } // it's inverted
void apollo_state::pc_dack5_w(int state) { select_dma_channel(5, state); }
void apollo_state::pc_dack6_w(int state) { select_dma_channel(6, state); }
void apollo_state::pc_dack7_w(int state) { select_dma_channel(7, state); }

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

u16 apollo_state::apollo_pic_get_vector()
{
	uint32_t vector = m_pic8259_master->acknowledge();
	if ((vector & 0x0f) == APOLLO_IRQ_PIC_SLAVE) {
		vector = m_pic8259_slave->acknowledge();
	}

	if (!machine().side_effects_disabled()) {
		// don't log ptm interrupts
		if (vector != APOLLO_IRQ_VECTOR+APOLLO_IRQ_PTM) {
			MLOG1(("apollo_pic_acknowledge: irq=%d vector=%x", vector & 0x0f, vector));
		}

		if (apollo_is_dn3000()) {
			apollo_csr_set_status_register(APOLLO_CSR_SR_INTERRUPT_PENDING, 0);
		} else {
			// clear bit Interrupt Pending in Cache Status Register
			apollo_set_cache_status_register(this,0x10, 0x00);
		}
	}
	return vector;
}

/*************************************************************
 * pic8259 configuration
 *************************************************************/

uint8_t apollo_state::apollo_pic8259_get_slave_ack(offs_t offset)
{
		MLOG1(("apollo_pic8259_get_slave_ack: offset=%x", offset));

		return offset == 3 ? m_pic8259_slave->acknowledge() : 0;
}

void apollo_state::apollo_pic8259_master_set_int_line(int state) {
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
		apollo_set_cache_status_register(this,0x10, state ? 0x10 : 0x00);
	}

	m_maincpu->set_input_line(M68K_IRQ_6,state ? ASSERT_LINE : CLEAR_LINE);
}

void apollo_state::apollo_pic8259_slave_set_int_line(int state) {
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

void apollo_state::apollo_ptm_timer_tick(int state)
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

void apollo_state::apollo_ptm_irq_function(int state)
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

void apollo_state::apollo_rtc_w(offs_t offset, uint8_t data)
{
	m_rtc->write_direct(offset, data);
	if (offset >= 0x0b && offset <= 0x0c)
	{
		SLOG2(("writing MC146818 at offset %02x = %02x", offset, data));
	}
}

uint8_t apollo_state::apollo_rtc_r(offs_t offset)
{
	uint8_t data;
	data = m_rtc->read_direct(offset);
	if (offset >= 0x0b && offset <= 0x0c)
	{
		SLOG2(("reading MC146818 at offset %02x = %02x", offset, data));
	}
	return data;
}

void apollo_state::apollo_rtc_irq_function(int state)
{
	apollo_pic_set_irq_line(APOLLO_IRQ_RTC, state);
}

//##########################################################################
// machine/apollo_sio.c - DN3000/DS3500 SIO at 0x8400/0x10400
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

apollo_sio::apollo_sio(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	duart_base_device(mconfig, APOLLO_SIO, tag, owner, clock),
	m_csrb(0)
{
}

void apollo_sio::device_reset()
{
	uint8_t input_data = apollo_get_ram_config_byte();
	ip0_w((input_data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
	ip1_w((input_data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
	ip2_w((input_data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
	ip3_w((input_data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	ip4_w((input_data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
	ip5_w((input_data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
	ip6_w((input_data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t apollo_sio::read(offs_t offset)
{
	static int last_read8_offset[2] = { -1, -1 };
	static int last_read8_value[2] = { -1, -1 };

	static const char * const duart68681_reg_read_names[0x10] = { "MRA", "SRA",
			"BRG Test", "RHRA", "IPCR", "ISR", "CTU", "CTL", "MRB", "SRB",
			"1X/16X Test", "RHRB", "IVR", "Input Ports", "Start Counter",
			"Stop Counter" };

	int data = duart_base_device::read(offset/2);

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
	}

	// omit logging if sio is being polled from the boot rom
	if ((offset != last_read8_offset[1] || data != last_read8_value[1]) &&
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

void apollo_sio::write(offs_t offset, uint8_t data)
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
	duart_base_device::write(offset/2, data);
}

// device type definition
DEFINE_DEVICE_TYPE(APOLLO_SIO, apollo_sio, "apollo_sio", "DN3000/DS3500 SIO (MC2681)")

void apollo_state::sio_irq_handler(int state)
{
	apollo_pic_set_irq_line(APOLLO_IRQ_SIO1, state);
}

void apollo_state::sio_output(uint8_t data)
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

void apollo_state::sio2_irq_handler(int state)
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
DEFINE_DEVICE_TYPE(APOLLO_NI, apollo_ni, "node_id", "Apollo Node ID")

//-------------------------------------------------
//  apollo_ni - constructor
//-------------------------------------------------

apollo_ni::apollo_ni(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APOLLO_NI, tag, owner, clock),
	device_image_interface(mconfig, *this),
	m_wdc(*this, ":isa1:wdc")
{
}

//-------------------------------------------------
//  apollo_ni - destructor
//-------------------------------------------------

apollo_ni::~apollo_ni()
{
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

void apollo_ni::set_node_id(uint32_t node_id)
{
	m_node_id = node_id;
	CLOG1(("apollo_ni::set_node_id: node ID is %x", node_id));
}

//-------------------------------------------------
//  read/write
//-------------------------------------------------

void apollo_ni::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	CLOG1(("Error: writing node id ROM at offset %02x = %04x & %04x", offset, data, mem_mask));
}

uint16_t apollo_ni::read(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;
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
std::pair<std::error_condition, std::string> apollo_ni::call_load()
{
	CLOG1(("apollo_ni::call_load: %s", filename()));

	uint64_t size = length();
		if (size != 32)
	{
		CLOG(("apollo_ni::call_load: %s has unexpected file size %d", filename(), size));
	}
	else
	{
		uint8_t data[32];
		fread(data, sizeof(data));

		uint8_t checksum = data[2] + data[4] + data[6];
		if (checksum != data[30])
		{
			CLOG(("apollo_ni::call_load: checksum is %02x - should be %02x", checksum, data[30]));
		}
		else
		{
			m_node_id = (((data[2] << 8) | data[4]) << 8) | (data[6]);
			CLOG1(("apollo_ni::call_load: node ID is %x", m_node_id));
			return std::make_pair(std::error_condition(), std::string());
		}
	}
	return std::make_pair(image_error::UNSPECIFIED, std::string());
}

/*-------------------------------------------------
 DEVICE_IMAGE_CREATE( rom )
 -------------------------------------------------*/

std::pair<std::error_condition, std::string> apollo_ni::call_create(int format_type, util::option_resolution *format_options)
{
	CLOG1(("apollo_ni::call_create:"));

	if (length() > 0)
	{
		CLOG(("apollo_ni::call_create: %s already exists", filename()));
	}
	else
	{
		uint32_t node_id = 0;
		sscanf(basename_noext(), "%x", &node_id);
		if (node_id == 0 || node_id > 0xfffff)
		{
			CLOG(("apollo_ni::call_create: filename %s is no valid node ID", basename()));
		}
		else
		{
			uint8_t data[32];
			memset(data, 0, sizeof(data));
			data[2] = node_id >> 16;
			data[4] = node_id >> 8;
			data[6] = node_id;
			data[30] = data[2] + data[4] + data[6];
			fwrite(data, sizeof(data));
			CLOG(("apollo_ni::call_create: created %s with node ID %x", filename(), node_id));
			set_node_id(node_id);
			return std::make_pair(std::error_condition(), std::string());
		}
	}
	return std::make_pair(image_error::UNSPECIFIED, std::string());
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
	uint8_t db[0x50];

	// check label of physical volume and get sector data of logical volume 1
	// Note: sector data starts with 32 byte block header
	// set node ID from UID of logical volume 1 of logical unit 0
	if (m_wdc
			&& m_wdc->get_sector(0, db, sizeof(db), 0) == sizeof(db)
			&& memcmp(db + 0x22, "APOLLO", 6) == 0)
	{
		uint16_t sector1 = apollo_is_dn5500() ? 4 : 1;

		if (m_wdc->get_sector(sector1, db, sizeof(db), 0) == sizeof(db))
		{
			// set node_id from UID of logical volume 1 of logical unit 0
			m_node_id = (((db[0x49] << 8) | db[0x4a]) << 8) | db[0x4b];
			CLOG1(("apollo_ni::set_node_id_from_disk: node ID is %x", m_node_id));
		}
	}
}

//##########################################################################
// machine/apollo.c - APOLLO DS3500 CPU Board
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

DEVICE_INPUT_DEFAULTS_START(3c505)
	DEVICE_INPUT_DEFAULTS("IO_BASE",  0x3f0, 0x300) // I/O address 0x300
	DEVICE_INPUT_DEFAULTS("IRQ_DRQ",  0x0f, 0x0a)   // IRQ 10
	DEVICE_INPUT_DEFAULTS("IRQ_DRQ",  0x70, 0x60)   // DRQ 6
	DEVICE_INPUT_DEFAULTS("ROM_OPTS", 0x01, 0x01)   // host ROM enabled
	DEVICE_INPUT_DEFAULTS("ROM_OPTS", 0xfe, 0x00)   // host ROM address 0x00000
DEVICE_INPUT_DEFAULTS_END

static void apollo_isa_cards(device_slot_interface &device)
{
	device.option_add("wdc", ISA16_OMTI8621_APOLLO);    // Combo ESDI/AT floppy controller
	device.option_add("ctape", ISA8_SC499);             // Archive SC499 cartridge tape
	device.option_add("3c505", ISA16_3C505).default_bios("apollo"); // 3Com 3C505 Ethernet card
	device.set_option_device_input_defaults("3c505", DEVICE_INPUT_DEFAULTS_NAME(3c505));
}

void apollo_state::common(machine_config &config)
{
	// configuration MUST be reset first !
	APOLLO_CONF(config, APOLLO_CONF_TAG, 0);

	AM9517A(config, m_dma8237_1, 14.318181_MHz_XTAL / 3);
	m_dma8237_1->out_hreq_callback().set(FUNC(apollo_state::apollo_dma_1_hrq_changed));
	m_dma8237_1->out_eop_callback().set(FUNC(apollo_state::apollo_dma8237_out_eop));
	m_dma8237_1->in_memr_callback().set(FUNC(apollo_state::apollo_dma_read_byte));
	m_dma8237_1->out_memw_callback().set(FUNC(apollo_state::apollo_dma_write_byte));
	m_dma8237_1->in_ior_callback<0>().set(FUNC(apollo_state::pc_dma8237_0_dack_r));
	m_dma8237_1->in_ior_callback<1>().set(FUNC(apollo_state::pc_dma8237_1_dack_r));
	m_dma8237_1->in_ior_callback<2>().set(FUNC(apollo_state::pc_dma8237_2_dack_r));
	m_dma8237_1->in_ior_callback<3>().set(FUNC(apollo_state::pc_dma8237_3_dack_r));
	m_dma8237_1->out_iow_callback<0>().set(FUNC(apollo_state::pc_dma8237_0_dack_w));
	m_dma8237_1->out_iow_callback<1>().set(FUNC(apollo_state::pc_dma8237_1_dack_w));
	m_dma8237_1->out_iow_callback<2>().set(FUNC(apollo_state::pc_dma8237_2_dack_w));
	m_dma8237_1->out_iow_callback<3>().set(FUNC(apollo_state::pc_dma8237_3_dack_w));
	m_dma8237_1->out_dack_callback<0>().set(FUNC(apollo_state::pc_dack0_w));
	m_dma8237_1->out_dack_callback<1>().set(FUNC(apollo_state::pc_dack1_w));
	m_dma8237_1->out_dack_callback<2>().set(FUNC(apollo_state::pc_dack2_w));
	m_dma8237_1->out_dack_callback<3>().set(FUNC(apollo_state::pc_dack3_w));

	AM9517A(config, m_dma8237_2, 14.318181_MHz_XTAL / 3);
	m_dma8237_2->out_hreq_callback().set(FUNC(apollo_state::apollo_dma_2_hrq_changed));
	m_dma8237_2->in_memr_callback().set(FUNC(apollo_state::apollo_dma_read_word));
	m_dma8237_2->out_memw_callback().set(FUNC(apollo_state::apollo_dma_write_word));
	m_dma8237_2->in_ior_callback<1>().set(FUNC(apollo_state::pc_dma8237_5_dack_r));
	m_dma8237_2->in_ior_callback<2>().set(FUNC(apollo_state::pc_dma8237_6_dack_r));
	m_dma8237_2->in_ior_callback<3>().set(FUNC(apollo_state::pc_dma8237_7_dack_r));
	m_dma8237_2->out_iow_callback<1>().set(FUNC(apollo_state::pc_dma8237_5_dack_w));
	m_dma8237_2->out_iow_callback<2>().set(FUNC(apollo_state::pc_dma8237_6_dack_w));
	m_dma8237_2->out_iow_callback<3>().set(FUNC(apollo_state::pc_dma8237_7_dack_w));
	m_dma8237_2->out_dack_callback<0>().set(FUNC(apollo_state::pc_dack4_w));
	m_dma8237_2->out_dack_callback<1>().set(FUNC(apollo_state::pc_dack5_w));
	m_dma8237_2->out_dack_callback<2>().set(FUNC(apollo_state::pc_dack6_w));
	m_dma8237_2->out_dack_callback<3>().set(FUNC(apollo_state::pc_dack7_w));

	PIC8259(config, m_pic8259_master, 0);
	m_pic8259_master->out_int_callback().set(FUNC(apollo_state::apollo_pic8259_master_set_int_line));
	m_pic8259_master->in_sp_callback().set_constant(1);
	m_pic8259_master->read_slave_ack_callback().set(FUNC(apollo_state::apollo_pic8259_get_slave_ack));

	PIC8259(config, m_pic8259_slave, 0);
	m_pic8259_slave->out_int_callback().set(FUNC(apollo_state::apollo_pic8259_slave_set_int_line));
	m_pic8259_slave->in_sp_callback().set_constant(0);

	PTM6840(config, m_ptm, 0);
	m_ptm->set_external_clocks(250000, 125000, 62500);
	m_ptm->irq_callback().set(FUNC(apollo_state::apollo_ptm_irq_function));

	clock_device &ptmclock(CLOCK(config, "ptmclock", 250000));
	ptmclock.signal_handler().set(FUNC(apollo_state::apollo_ptm_timer_tick));

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	// FIXME: is this interrupt really only connected on DN3000?
	//m_rtc->irq().set(FUNC(apollo_state::apollo_rtc_irq_function));
	m_rtc->set_use_utc(true);
	m_rtc->set_binary(false);
	m_rtc->set_24hrs(false);
	m_rtc->set_epoch(0);

	APOLLO_NI(config, m_node_id, 0);

	APOLLO_SIO(config, m_sio2, 3.6864_MHz_XTAL);
	m_sio2->irq_cb().set(FUNC(apollo_state::sio2_irq_handler));

	ISA16(config, m_isa, 0);
	m_isa->set_custom_spaces();
	m_isa->irq2_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir1_w)); // in place of irq 2 on at irq 9 is used
	m_isa->irq3_callback().set(m_pic8259_master, FUNC(pic8259_device::ir3_w));
	m_isa->irq4_callback().set(m_pic8259_master, FUNC(pic8259_device::ir4_w));
	m_isa->irq5_callback().set(m_pic8259_master, FUNC(pic8259_device::ir5_w));
	m_isa->irq6_callback().set(m_pic8259_master, FUNC(pic8259_device::ir6_w));
	m_isa->irq7_callback().set(m_pic8259_master, FUNC(pic8259_device::ir7_w));
	m_isa->irq10_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir2_w));
	m_isa->irq11_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir3_w));
	m_isa->irq12_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir4_w));
	m_isa->irq14_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir6_w));
	m_isa->irq15_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir7_w));
	m_isa->drq0_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq0_w));
	m_isa->drq1_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq1_w));
	m_isa->drq2_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq2_w));
	m_isa->drq3_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq3_w));
	m_isa->drq5_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq1_w));
	m_isa->drq6_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq2_w));
	m_isa->drq7_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq3_w));

	ISA16_SLOT(config, "isa1", 0, APOLLO_ISA_TAG, apollo_isa_cards, "wdc", false); // FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa2", 0, APOLLO_ISA_TAG, apollo_isa_cards, "ctape", false);
	ISA16_SLOT(config, "isa3", 0, APOLLO_ISA_TAG, apollo_isa_cards, "3c505", false);
	ISA16_SLOT(config, "isa4", 0, APOLLO_ISA_TAG, apollo_isa_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, APOLLO_ISA_TAG, apollo_isa_cards, nullptr, false);
	ISA16_SLOT(config, "isa6", 0, APOLLO_ISA_TAG, apollo_isa_cards, nullptr, false);
	ISA16_SLOT(config, "isa7", 0, APOLLO_ISA_TAG, apollo_isa_cards, nullptr, false);

	SOFTWARE_LIST(config, "ctape_list").set_original("apollo_ctape");
}

// for machines with the keyboard and a graphics head
void apollo_state::apollo(machine_config &config)
{
	common(config);
	APOLLO_SIO(config, m_sio, 3.6864_MHz_XTAL);
	m_sio->irq_cb().set(FUNC(apollo_state::sio_irq_handler));
	m_sio->outport_cb().set(FUNC(apollo_state::sio_output));
	m_sio->a_tx_cb().set(m_keyboard, FUNC(apollo_kbd_device::rx_w));

#ifdef APOLLO_XXL
	m_sio->b_tx_cb().set(APOLLO_STDIO_TAG, FUNC(apollo_stdio_device::rx_w));
#endif
}

static DEVICE_INPUT_DEFAULTS_START( apollo_terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

// for headless machines using a serial console
void apollo_state::apollo_terminal(machine_config &config)
{
	common(config);
	APOLLO_SIO(config, m_sio, 3.6864_MHz_XTAL);
	m_sio->irq_cb().set(FUNC(apollo_state::sio_irq_handler));
	m_sio->outport_cb().set(FUNC(apollo_state::sio_output));
	m_sio->b_tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_sio, FUNC(apollo_sio::rx_b_w));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(apollo_terminal));
}

void apollo_state::init_apollo()
{
	MLOG1(("driver_init_apollo"));
}

MACHINE_START_MEMBER(apollo_state,apollo)
{
	MLOG1(("machine_start_apollo"));

	m_dma_channel = -1;
	m_cur_eop = false;

	m_internal_leds.resolve();
	m_external_leds.resolve();
}

MACHINE_RESET_MEMBER(apollo_state,apollo)
{
	uint8_t year = apollo_rtc_r(9);

	MLOG1(("machine_reset_apollo"));

	// set configuration
	apollo_csr_set_servicemode(apollo_config(APOLLO_CONF_SERVICE_MODE));

	// change year according to configuration settings
	if (year < 25 && apollo_config(APOLLO_CONF_25_YEARS_AGO))
	{
		year += 75;
		apollo_rtc_w(9, year);
	}
	else if (year < 30 && apollo_config(APOLLO_CONF_30_YEARS_AGO))
	{
		year += 70;
		apollo_rtc_w(9, year);
	}
	else if (year >= 70 && !apollo_config(APOLLO_CONF_30_YEARS_AGO)
			&& !apollo_config(APOLLO_CONF_25_YEARS_AGO))
	{
		year -= 70;
		apollo_rtc_w(9, year);
	}

	ptm_counter = 0;
	sio_output_data = 0xff;
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
DEFINE_DEVICE_TYPE(APOLLO_STDIO, apollo_stdio_device, "apollo_stdio", "Apollo STDIO")

//-------------------------------------------------
// apollo_stdio_device - constructor
//-------------------------------------------------

apollo_stdio_device::apollo_stdio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APOLLO_STDIO, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_tx_w(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apollo_stdio_device::device_start()
{
	CLOG1(("device_start"));

	m_tx_w.resolve();

	m_poll_timer = timer_alloc(FUNC(apollo_stdio_device::poll_timer), this);
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

void apollo_stdio_device::rcv_complete() // Rx completed receiving byte
{
	receive_register_extract();
	uint8_t data = get_received_char();

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
	uint8_t data;
	while (::read(STDIN_FILENO, &data, 1) == 1)
	{
		xmit_char(data == '\n' ? '\r' : data);
	}
#endif
}

void apollo_stdio_device::xmit_char(uint8_t data)
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
