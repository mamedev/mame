/*
 * apollo.c - Apollo DS3500 CPU Board
 *
 *  Created on: Jan 20, 2011
 *      Author: Hans Ostermeyer
 *
 *  Released for general non-commercial use under the MAME license
 *  Visit http://mamedev.org for licensing and usage restrictions.
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
 * - apollo_fdc.c - APOLLO DS3500 Floppy disk controller
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

		PORT_CONFNAME(APOLLO_CONF_DATE_1990, APOLLO_CONF_DATE_1990, "20 Years Ago ...")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_DATE_1990, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_NODE_ID, APOLLO_CONF_NODE_ID, "Node ID from Disk")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_NODE_ID, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_IDLE_SLEEP, 0x00, "Idle Sleep")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_IDLE_SLEEP, DEF_STR ( On ) )

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

static I8237_INTERFACE( apollo_dma8237_1_config )
{
	DEVCB_DRIVER_LINE_MEMBER(apollo_state, apollo_dma_1_hrq_changed),
	DEVCB_DRIVER_LINE_MEMBER(apollo_state, apollo_dma8237_out_eop),
	DEVCB_DRIVER_MEMBER(apollo_state, apollo_dma_read_byte),
	DEVCB_DRIVER_MEMBER(apollo_state, apollo_dma_write_byte),
	{ DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_0_dack_r), DEVCB_DRIVER_MEMBER(apollo_state, pc_dma8237_1_dack_r), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_2_dack_r), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_3_dack_r)},
	{ DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_0_dack_w), DEVCB_DRIVER_MEMBER(apollo_state, pc_dma8237_1_dack_w), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_2_dack_w), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_3_dack_w)},
	{ DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dack0_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dack1_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dack2_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dack3_w) }
};

static I8237_INTERFACE( apollo_dma8237_2_config )
{
	DEVCB_DRIVER_LINE_MEMBER(apollo_state, apollo_dma_2_hrq_changed),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(apollo_state, apollo_dma_read_word),
	DEVCB_DRIVER_MEMBER(apollo_state, apollo_dma_write_word),
	{ DEVCB_NULL, DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_5_dack_r), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_6_dack_r), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_7_dack_r) },
	{ DEVCB_NULL, DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_5_dack_w), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_6_dack_w), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dma8237_7_dack_w) },
	{ DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dack4_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dack5_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dack6_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, apollo_state, pc_dack7_w) }
};

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

		return offset == 3 ? m_pic8259_slave->inta_r() : 0;
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

static const ptm6840_interface apollo_ptm_config = {
		0,
		{ 250000, 125000, 62500 },
		{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
		DEVCB_DRIVER_LINE_MEMBER(apollo_state, apollo_ptm_irq_function)
};

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
// machine/apollo_sio.c - APOLLO DS3500 SIO
//##########################################################################

WRITE_LINE_MEMBER(apollo_state::sio_irq_handler)
{
	apollo_pic_set_irq_line(APOLLO_IRQ_SIO1, state);
}

WRITE8_MEMBER(apollo_state::sio_output)
{
	if ((data & 0x80) != (sio_output_data & 0x80))
	{
		apollo_pic_set_irq_line(APOLLO_IRQ_DIAG, (data & 0x80) ? 1 : 0);
		sio_output_data = data;
	}
}
	// The counter/timer on the SIO chip is used for the refresh count.
	// This is set up in the timer mode to  produce a square wave output on output OP3.
	// The period of the output is 15 microseconds.

	// toggle memory refresh counter
//  sio_input_data ^= 0x01;

//##########################################################################
// machine/apollo_sio2.c - APOLLO DS3500 SIO2
//##########################################################################

WRITE_LINE_MEMBER(apollo_state::sio2_irq_handler)
{
	apollo_pic_set_irq_line(APOLLO_IRQ_SIO2, state);
}

//##########################################################################
// machine/apollo.c - APOLLO DS3500 CPU Board
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

static const isa16bus_interface isabus_intf =
{
	// interrupts
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_PIC2_TAG, pic8259_device, ir2_w), // in place of irq 2 on at irq 9 is used
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_PIC1_TAG, pic8259_device, ir3_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_PIC1_TAG, pic8259_device, ir4_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_PIC1_TAG, pic8259_device, ir5_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_PIC1_TAG, pic8259_device, ir6_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_PIC1_TAG, pic8259_device, ir7_w),

	DEVCB_DEVICE_LINE_MEMBER(APOLLO_PIC2_TAG, pic8259_device, ir3_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_PIC2_TAG, pic8259_device, ir4_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_PIC2_TAG, pic8259_device, ir5_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_PIC2_TAG, pic8259_device, ir6_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_PIC2_TAG, pic8259_device, ir7_w),

	// dma request
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_DMA1_TAG, am9517a_device, dreq0_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_DMA1_TAG, am9517a_device, dreq1_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_DMA1_TAG, am9517a_device, dreq2_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_DMA1_TAG, am9517a_device, dreq3_w),

	DEVCB_DEVICE_LINE_MEMBER(APOLLO_DMA2_TAG, am9517a_device, dreq1_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_DMA2_TAG, am9517a_device, dreq2_w),
	DEVCB_DEVICE_LINE_MEMBER(APOLLO_DMA2_TAG, am9517a_device, dreq3_w),
};

static SLOT_INTERFACE_START(apollo_isa_cards)
	SLOT_INTERFACE("wdc", ISA16_OMTI8621)   // Combo ESDI/AT floppy controller
	SLOT_INTERFACE("ctape", ISA8_SC499)     // Archive SC499 cartridge tape
	SLOT_INTERFACE("3c505", ISA16_3C505)    // 3Com 3C505 Ethernet card
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( common )
	// configuration MUST be reset first !
	MCFG_DEVICE_ADD(APOLLO_CONF_TAG, APOLLO_CONF, 0)

	MCFG_I8237_ADD( APOLLO_DMA1_TAG, XTAL_14_31818MHz/3, apollo_dma8237_1_config )
	MCFG_I8237_ADD( APOLLO_DMA2_TAG, XTAL_14_31818MHz/3, apollo_dma8237_2_config )
	MCFG_PIC8259_ADD( APOLLO_PIC1_TAG, WRITELINE(apollo_state,apollo_pic8259_master_set_int_line), VCC, READ8(apollo_state, apollo_pic8259_get_slave_ack))
	MCFG_PIC8259_ADD( APOLLO_PIC2_TAG, WRITELINE(apollo_state,apollo_pic8259_slave_set_int_line), GND, NULL)

	MCFG_PTM6840_ADD(APOLLO_PTM_TAG, apollo_ptm_config)
	MCFG_DEVICE_ADD("ptmclock", CLOCK, 250000)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(apollo_state, apollo_ptm_timer_tick))

	MCFG_MC146818_ADD( APOLLO_RTC_TAG, XTAL_32_768kHz )
	MCFG_MC146818_UTC( true )

	MCFG_MC68681_ADD( APOLLO_SIO2_TAG, XTAL_3_6864MHz )
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(apollo_state, sio2_irq_handler))

	MCFG_ISA16_BUS_ADD(APOLLO_ISA_TAG, ":"MAINCPU, isabus_intf)
	MCFG_ISA16_BUS_CUSTOM_SPACES()
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa1", apollo_isa_cards, "wdc", false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa2", apollo_isa_cards, "ctape", false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa3", apollo_isa_cards, "3c505", false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa4", apollo_isa_cards, NULL, false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa5", apollo_isa_cards, NULL, false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa6", apollo_isa_cards, NULL, false)
	MCFG_ISA16_SLOT_ADD(APOLLO_ISA_TAG, "isa7", apollo_isa_cards, NULL, false)
MACHINE_CONFIG_END

// for machines with the keyboard and a graphics head
MACHINE_CONFIG_FRAGMENT( apollo )
	MCFG_FRAGMENT_ADD(common)

	MCFG_MC68681_ADD( APOLLO_SIO_TAG, XTAL_3_6864MHz )
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(apollo_state, sio_irq_handler))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(apollo_state, sio_output))
	MCFG_MC68681_A_TX_CALLBACK(DEVWRITELINE(APOLLO_KBD_TAG, apollo_kbd_device, rx_w))
MACHINE_CONFIG_END

static DEVICE_INPUT_DEFAULTS_START( apollo_terminal )
	DEVICE_INPUT_DEFAULTS( "TERM_TXBAUD", 0xff, 0x06 ) // 9600
	DEVICE_INPUT_DEFAULTS( "TERM_RXBAUD", 0xff, 0x06 ) // 9600
	DEVICE_INPUT_DEFAULTS( "TERM_STARTBITS", 0xff, 0x01 ) // 1
	DEVICE_INPUT_DEFAULTS( "TERM_DATABITS", 0xff, 0x03 ) // 8
	DEVICE_INPUT_DEFAULTS( "TERM_PARITY", 0xff, 0x00 ) // N
	DEVICE_INPUT_DEFAULTS( "TERM_STOPBITS", 0xff, 0x01 ) // 1
DEVICE_INPUT_DEFAULTS_END

// for headless machines using a serial console
MACHINE_CONFIG_FRAGMENT( apollo_terminal )
	MCFG_FRAGMENT_ADD(common)

	MCFG_MC68681_ADD( APOLLO_SIO_TAG, XTAL_3_6864MHz )
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(apollo_state, sio_irq_handler))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(apollo_state, sio_output))
	MCFG_MC68681_B_TX_CALLBACK(DEVWRITELINE("rs232", rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "serial_terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(APOLLO_SIO_TAG, mc68681_device, rx_b_w))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("serial_terminal", apollo_terminal)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(apollo_state,apollo)
{
	//MLOG1(("driver_init_apollo"));
}

MACHINE_START_MEMBER(apollo_state,apollo)
{
	//MLOG1(("machine_start_apollo"));

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

	//MLOG1(("machine_reset_apollo"));

	// set configuration
	apollo_csr_set_servicemode(apollo_config(APOLLO_CONF_SERVICE_MODE));

	// change year according to configuration settings
	if (year < 20 && apollo_config(APOLLO_CONF_DATE_1990))
	{
		year+=80;
		apollo_rtc_w(space, 9, year);
	}
	else if (year >= 80 && !apollo_config(APOLLO_CONF_DATE_1990))
	{
		year -=80;
		apollo_rtc_w(space, 9, year);
	}

	ptm_counter = 0;
	sio_output_data = 0xff;

	if (apollo_is_dn3000())
	{
		m_dn3000_timer->adjust(attotime::from_hz(2), 0, attotime::from_hz(2));
	}
}
