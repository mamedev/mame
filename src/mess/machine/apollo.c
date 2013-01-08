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
 */

#include "includes/apollo.h"
#include "machine/apollo_kbd.h"
#include "machine/omti8621.h"
#include "machine/sc499.h"
#include "machine/3c505.h"

#include "machine/6840ptm.h"
#include "machine/68681.h"
#include "machine/8237dma.h"
#include "machine/mc146818.h"
#include "machine/pic8259.h"

#include "machine/pc_fdc.h"
#include "formats/apollo_dsk.h"

#include "cpu/m68000/m68000.h"
//#include "cpu/m68000/m68kcpu.h"

#include "emuopts.h"

#if defined(APOLLO_FOR_LINUX)
#include <fcntl.h>
#include <unistd.h>
#endif

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

		PORT_CONFNAME(APOLLO_CONF_GERMAN_KBD, 0x00, "German Keyboard")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_GERMAN_KBD, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_DATE_1990, 0x00, "20 Years Ago ...")
		PORT_CONFSETTING(0x00, DEF_STR ( Off ) )
		PORT_CONFSETTING(APOLLO_CONF_DATE_1990, DEF_STR ( On ) )

		PORT_CONFNAME(APOLLO_CONF_NODE_ID, 0x00, "Node ID from Disk")
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

/*-------------------------------------------------
 device start callback
 -------------------------------------------------*/

static DEVICE_START(apollo_config)
{
	DLOG1(("start apollo_config"));
}

/*-------------------------------------------------
 device reset callback
 -------------------------------------------------*/

static DEVICE_RESET(apollo_config)
{
	DLOG1(("reset apollo_config"));
	// load configuration
	config = device->machine().root_device().ioport("apollo_config")->read();
}

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
	: device_t(mconfig, APOLLO_CONF, "Apollo Configuration", tag, owner, clock)
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
	DEVICE_START_NAME( apollo_config )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apollo_config_device::device_reset()
{
	DEVICE_RESET_NAME( apollo_config )(this);
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

static int apollo_csr_get_servicemode()
{

	return cpu_status_register & APOLLO_CSR_SR_SERVICE ? 0 : 1;
}

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
        apollo_set_cpu_has_fpu(&space.device(), 1);
	}
	else
	{
		// disable FPU (i.e. FPU opcodes in CPU)
        apollo_set_cpu_has_fpu(&space.device(), 0);

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

INLINE device_t *get_device_dma8237_1(device_t *device) {
	return device->machine().driver_data<apollo_state>()->dma8237_1;
}

INLINE device_t *get_device_dma8237_2(device_t *device) {
	return device->machine().driver_data<apollo_state>()->dma8237_2;
}

static void apollo_dma_fdc_drq(device_t *device, int state) {
	DLOG2(("apollo_dma_fdc_drq: state=%x", state));
	i8237_dreq2_w(get_device_dma8237_1(device), state);
}

static void apollo_dma_ctape_drq(device_t *device, int state) {
	DLOG1(("apollo_dma_ctape_drq: state=%x", state));
	i8237_dreq1_w(get_device_dma8237_1(device), state);
}

/*-------------------------------------------------
 DN3000/DN3500 DMA Controller 1 at 0x9000/0x10c00
 -------------------------------------------------*/

WRITE8_MEMBER(apollo_state::apollo_dma_1_w){
	SLOG1(("apollo_dma_1_w: writing DMA Controller 1 at offset %02x = %02x", offset, data));
	i8237_w(get_device_dma8237_1(&space.device()), space, offset, data);
}

READ8_MEMBER(apollo_state::apollo_dma_1_r){
	UINT8 data = i8237_r(get_device_dma8237_1(&space.device()), space, offset);
	SLOG1(("apollo_dma_1_r: reading DMA Controller 1 at offset %02x = %02x", offset, data));
	return data;
}

/*-------------------------------------------------
 DN3000/DN3500 DMA Controller 2 at 0x9100/0x10d00
 -------------------------------------------------*/

WRITE8_MEMBER(apollo_state::apollo_dma_2_w){
	SLOG1(("apollo_dma_2_w: writing DMA Controller 2 at offset %02x = %02x", offset/2, data));
	i8237_w(get_device_dma8237_2(&space.device()), space, offset / 2, data);
}

READ8_MEMBER(apollo_state::apollo_dma_2_r){
	UINT8 data = i8237_r(get_device_dma8237_2(&space.device()), space, offset / 2);
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

	data = space.read_byte(page_offset + offset);

	if (VERBOSE > 1 || offset < 4 || (offset & 0xff) == 0 || (offset & 0xff) == 0xff)
	{
		SLOG1(("dma read byte at offset %x+%03x = %02x", page_offset, offset, data));
	}

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
	space.write_byte(page_offset + offset, data);

	if (VERBOSE > 1 || offset < 4 || (offset & 0xff) == 0 || (offset & 0xff) == 0xff)
	{
		SLOG1(("dma write byte at offset %x+%03x = %02x", page_offset, offset , data));
	}
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

	data = space.read_byte(page_offset + offset);

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

	space.write_byte(page_offset + offset, data);
	SLOG1(("dma write word at offset %x+%03x = %02x", page_offset, offset, data));
}

static READ8_DEVICE_HANDLER( apollo_dma8237_ctape_dack_r ) {

	UINT8 data = sc499_dack_r(&space.machine());
	DLOG2(("dma ctape dack read %02x",data));

	// hack for DN3000: select appropriate DMA channel No.
	dn3000_dma_channel1 = 1; // 1 = ctape, 2 = floppy dma channel

	return data;
}

static WRITE8_DEVICE_HANDLER( apollo_dma8237_ctape_dack_w ) {
	DLOG2(("dma ctape dack write %02x", data));
	sc499_dack_w(&space.machine(), data);

	// hack for DN3000: select appropriate DMA channel No.
	// Note: too late for this byte, but next bytes will be ok
	dn3000_dma_channel1 = 1; // 1 = ctape, 2 = floppy dma channel
}

static READ8_DEVICE_HANDLER( apollo_dma8237_fdc_dack_r ) {
	pc_fdc_at_device *fdc = space.machine().device<pc_fdc_at_device>(APOLLO_FDC_TAG);
	UINT8 data = fdc->dma_r();
	//  DLOG2(("dma fdc dack read %02x",data));

	// hack for DN3000: select appropriate DMA channel No.
	dn3000_dma_channel1 = 2; // 1 = ctape, 2 = floppy dma channel

	return data;
}

static WRITE8_DEVICE_HANDLER( apollo_dma8237_fdc_dack_w ) {
	pc_fdc_at_device *fdc = space.machine().device<pc_fdc_at_device>(APOLLO_FDC_TAG);
	// DLOG2(("dma fdc dack write %02x", data));
	fdc->dma_w(data);

	// hack for DN3000: select appropriate DMA channel No.
	// Note: too late for this byte, but next bytes will be ok
	dn3000_dma_channel1 = 2; // 1 = ctape, 2 = floppy dma channel
}

static READ8_DEVICE_HANDLER( apollo_dma8237_wdc_dack_r ) {
	UINT8 data = 0xff; // omti8621_dack_r(device->machine);
	DLOG1(("dma wdc dack read %02x (not used, not emulated!)",data));
	return data;
}

static WRITE8_DEVICE_HANDLER( apollo_dma8237_wdc_dack_w ) {
	DLOG1(("dma wdc dack write %02x (not used, not emulated!)", data));
//  omti8621_dack_w(device->machine, data);
}

static WRITE_LINE_DEVICE_HANDLER( apollo_dma8237_out_eop ) {
	pc_fdc_at_device *fdc = device->machine().device<pc_fdc_at_device>(APOLLO_FDC_TAG);
	DLOG1(("dma out eop state %02x", state));
	fdc->tc_w(!state);
	sc499_set_tc_state(&device->machine(), state);
}

static WRITE_LINE_DEVICE_HANDLER( apollo_dma_1_hrq_changed ) {
	// DLOG2(("dma 1 hrq changed state %02x", state));
	i8237_dreq0_w(get_device_dma8237_2(device), state);

	/* Assert HLDA */
	i8237_hlda_w(device, state);

	// cascade mode?
	// i8237_hlda_w(get_device_dma8237_2(device), state);
}

static WRITE_LINE_DEVICE_HANDLER( apollo_dma_2_hrq_changed ) {
	// DLOG2(("dma 2 hrq changed state %02x", state));
	device->machine().device(MAINCPU)->execute().set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w(device, state);
}

static I8237_INTERFACE( apollo_dma8237_1_config )
{
	DEVCB_LINE(apollo_dma_1_hrq_changed),
	DEVCB_LINE(apollo_dma8237_out_eop),
	DEVCB_DRIVER_MEMBER(apollo_state, apollo_dma_read_byte),
	DEVCB_DRIVER_MEMBER(apollo_state, apollo_dma_write_byte),
	{	DEVCB_NULL, DEVCB_HANDLER(apollo_dma8237_ctape_dack_r), DEVCB_HANDLER(apollo_dma8237_fdc_dack_r), DEVCB_NULL},
	{	DEVCB_NULL, DEVCB_HANDLER(apollo_dma8237_ctape_dack_w), DEVCB_HANDLER(apollo_dma8237_fdc_dack_w), DEVCB_NULL},
	{	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL}
};

static I8237_INTERFACE( apollo_dma8237_2_config )
{
	DEVCB_LINE(apollo_dma_2_hrq_changed),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(apollo_state, apollo_dma_read_word),
	DEVCB_DRIVER_MEMBER(apollo_state, apollo_dma_write_word),
	{	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_HANDLER(apollo_dma8237_wdc_dack_r)},
	{	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_HANDLER(apollo_dma8237_wdc_dack_w)},
	{	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL}
};

//##########################################################################
// machine/apollo_pic.c - APOLLO DS3500 PIC 8259 controllers
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

INLINE device_t *get_pic8259_master(device_t *device) {
	return device->machine().driver_data<apollo_state>()->pic8259_master;
}

INLINE device_t *get_pic8259_slave(device_t *device) {
	return device->machine().driver_data<apollo_state>()->pic8259_slave;
}

/*-------------------------------------------------
 Interrupt Controller 8259 PIC #1 at 0x9400/0x11000
 -------------------------------------------------*/

WRITE8_DEVICE_HANDLER(apollo_pic8259_master_w ) {
	DLOG1(("writing %s at offset %X = %02x", device->tag(), offset, data));
	pic8259_w(device, space, offset, data);
}

READ8_DEVICE_HANDLER( apollo_pic8259_master_r ) {
	UINT8 data = pic8259_r(device, space, offset);
	DLOG1(("reading %s at offset %X = %02x", device->tag(), offset, data));
	return data;
}

/*-------------------------------------------------
 Interrupt Controller 8259 PIC #2 at 0x9500/0x11100
 -------------------------------------------------*/

WRITE8_DEVICE_HANDLER(apollo_pic8259_slave_w ) {
	DLOG1(("writing %s at offset %X = %02x", device->tag(), offset, data));
	pic8259_w(device, space, offset, data);
}

READ8_DEVICE_HANDLER( apollo_pic8259_slave_r ) {
	UINT8 data = pic8259_r(device, space, offset);
	DLOG1(("reading %s at offset %X = %02x", device->tag(), offset, data));
	return data;
}

static void apollo_pic_set_irq_line(device_t *device, int irq, int state) {
	// don't log PTM interrupts
	if (irq != APOLLO_IRQ_PTM) {
		DLOG1(("apollo_pic_set_irq_line: irq=%d state=%d", irq, state));
	}

	switch (irq) {
	case 0: pic8259_ir0_w(get_pic8259_master(device), state); break;
	case 1: pic8259_ir1_w(get_pic8259_master(device), state); break;
	case 2: pic8259_ir2_w(get_pic8259_master(device), state); break;
	case 3: pic8259_ir3_w(get_pic8259_master(device), state); break;
	case 4: pic8259_ir4_w(get_pic8259_master(device), state); break;
	case 5: pic8259_ir5_w(get_pic8259_master(device), state); break;
	case 6: pic8259_ir6_w(get_pic8259_master(device), state); break;
	case 7: pic8259_ir7_w(get_pic8259_master(device), state); break;

	case 8: pic8259_ir0_w(get_pic8259_slave(device), state); break;
	case 9: pic8259_ir1_w(get_pic8259_slave(device), state); break;
	case 10: pic8259_ir2_w(get_pic8259_slave(device), state); break;
	case 11: pic8259_ir3_w(get_pic8259_slave(device), state); break;
	case 12: pic8259_ir4_w(get_pic8259_slave(device), state); break;
	case 13: pic8259_ir5_w(get_pic8259_slave(device), state); break;
	case 14: pic8259_ir6_w(get_pic8259_slave(device), state); break;
	case 15: pic8259_ir7_w(get_pic8259_slave(device), state); break;
	}
}

IRQ_CALLBACK(apollo_pic_acknowledge) {
	UINT32 vector = pic8259_acknowledge(get_pic8259_master(device));
	if ((vector & 0x0f) == APOLLO_IRQ_PIC_SLAVE) {
		vector = pic8259_acknowledge(get_pic8259_slave(device));
	}

	// don't log ptm interrupts
	if (vector != APOLLO_IRQ_VECTOR+APOLLO_IRQ_PTM) {
		DLOG1(("apollo_pic_acknowledge: irq=%d vector=%x", vector & 0x0f, vector));
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

static WRITE_LINE_DEVICE_HANDLER( apollo_pic8259_master_set_int_line ) {
	static int interrupt_line = -1;
	if (state != interrupt_line) {
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

	device->machine().device(MAINCPU)->execute().set_input_line_and_vector(M68K_IRQ_6,state ? ASSERT_LINE : CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR);
}

static WRITE_LINE_DEVICE_HANDLER( apollo_pic8259_slave_set_int_line ) {
	static int interrupt_line = -1;
	if (state != interrupt_line) {
		DLOG1(("apollo_pic8259_slave_set_int_line: %x", state));
		interrupt_line = state;
		apollo_pic_set_irq_line(device, 3, state);
	}
}

static const struct pic8259_interface apollo_pic8259_master_config = {
		DEVCB_LINE(apollo_pic8259_master_set_int_line) };

static const struct pic8259_interface apollo_pic8259_slave_config = {
		DEVCB_LINE(apollo_pic8259_slave_set_int_line) };

//##########################################################################
// machine/apollo_ptm.c - APOLLO DS3500 Programmable Timer 6840
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

static emu_timer* ptm_timer;
static UINT32 ptm_counter = 0;

static TIMER_CALLBACK(apollo_ptm_timer_callback)
{
	ptm6840_device *device = downcast<ptm6840_device *>((device_t *) ptr);

	ptm_counter++;
	device->set_c1( 1);
	device->set_c1( 0);
	device->set_c2(ptm_counter & 1);

	if ((ptm_counter & 1) == 0)
	{
		device->set_c3((ptm_counter >> 1) & 1);
	}

	if (ptm_counter % 250000 == 0)
	{
		DLOG2(("apollo_ptm_timer_callback: %d", ptm_counter / 250000));
	}
}

static WRITE_LINE_DEVICE_HANDLER( apollo_ptm_irq_function ) {
	DLOG1(("apollo_ptm_irq_function: state=%d", state ));

//  ptm6840_device *ptm = device->machine().device<ptm6840_device>(APOLLO_PTM_TAG);

	apollo_pic_set_irq_line(device, APOLLO_IRQ_PTM, state);
}

//  Timer 1's input is a 250-kHz (4-microsecond period) signal.
//  Timer 2's input is a 125-kHz (8-microsecond period) signal.
//  Timer 3's input is a 62.5-kHz (16-microsecond period) signal.
//  The Timer 3 input may be prescaled to make the effective input signal have a 128-microsecond period.

static const ptm6840_interface apollo_ptm_config = {
		0,
		{ 250000, 125000, 62500 },
		{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
		DEVCB_LINE(apollo_ptm_irq_function)
};

static DEVICE_START( apollo_ptm )
{
	DLOG1(("start apollo_ptm"));
	// allocate and start ptm timer
	ptm_timer = device->machine().scheduler().timer_alloc(FUNC(apollo_ptm_timer_callback), device);
	ptm_timer->adjust( attotime::zero, 0, attotime::from_usec(4));
}

static DEVICE_RESET( apollo_ptm )
{
	DLOG1(("reset apollo_ptm"));
	device->reset();
}

READ8_DEVICE_HANDLER(apollo_ptm_r) {
	UINT8 data =downcast<ptm6840_device *>((device_t *) device)->read(offset / 2);

	// prevent excessive logging
	static UINT8 previous = 255;
	if (offset / 2 != 1 || data != previous) {
		DLOG1(("apollo_ptm_read reg %x returned %02x", offset/2, data ));
		if (offset / 2 == 1) {
			previous = data;
		}
	}
	return data;
}

WRITE8_DEVICE_HANDLER(apollo_ptm_w) {
	DLOG1(("apollo_ptm_write reg %x with %02x", offset/2, data ));
	downcast<ptm6840_device *>((device_t *) device)->write(offset / 2, data);
}

//##########################################################################
// machine/apollo_rtc.c - APOLLO DS3500 RTC MC146818
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

/***************************************************************************
 DN3000/DN3500 Realtime Calendar MC146818 at 0x8900/0x10900
 ***************************************************************************/

static DEVICE_RESET( apollo_rtc ) {
	address_space &space = device->machine().device(MAINCPU)->memory().space(AS_PROGRAM);
	apollo_state *state = device->machine().driver_data<apollo_state>();
	UINT8 year = state->apollo_rtc_r(space, 9);

	// change year according to configuration settings
	if (year < 20 && apollo_config(APOLLO_CONF_DATE_1990))
	{
		year+=80;
		state->apollo_rtc_w(space, 9, year);
	}
	else if (year >= 80 && !apollo_config(APOLLO_CONF_DATE_1990))
	{
		year -=80;
		state->apollo_rtc_w(space, 9, year);
	}

	//SLOG1(("reset apollo_rtc year=%d", year));
}

WRITE8_MEMBER(apollo_state::apollo_rtc_w)
{
	mc146818_device *rtc = machine().device<mc146818_device> (APOLLO_RTC_TAG);
	rtc->write(space, 0, offset);
	rtc->write(space, 1, data);
	if (offset >= 0x0b && offset <= 0x0c)
	{
		SLOG2(("writing MC146818 at offset %02x = %02x", offset, data));
	}
}

READ8_MEMBER(apollo_state::apollo_rtc_r)
{
	UINT8 data;
	mc146818_device *rtc = machine().device<mc146818_device> (APOLLO_RTC_TAG);
	rtc->write(space, 0, offset);
	data = rtc->read(space, 1);
	if (offset >= 0x0b && offset <= 0x0c)
	{
		SLOG2(("reading MC146818 at offset %02x = %02x", offset, data));
	}
	return data;
}

static TIMER_CALLBACK( apollo_rtc_timer )
{
	apollo_state *state = machine.driver_data<apollo_state>();
	address_space &space = machine.device(MAINCPU)->memory().space(AS_PROGRAM);

	// FIXME: reading register 0x0c will clear all interrupt flags
	if ((state->apollo_rtc_r(space, 0x0c) & 0x80))
	{
		//SLOG2(("apollo_rtc_timer - set_irq_line %d", APOLLO_IRQ_RTC));
		apollo_pic_set_irq_line(&space.device(), APOLLO_IRQ_RTC, 1);
	}
}

//##########################################################################
// machine/apollo_sio.c - APOLLO DS3500 SIO
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

#define SIO_SLEEP_DELAY_TIME 30000 // ms

static int isInitialized = 0;
static int input_from_stdin = 0;

static UINT8 sio_input_data = 0xff;
static UINT8 sio_output_data = 0xff;

static emu_timer *kbd_timer;
static int sleep_time = 0;

static int sio_irq_line = 0;

static UINT8 sio_csrb = 0;

/*-------------------------------------------------
 sio_sleep - sleep to reduce the CPU usage
 -------------------------------------------------*/

// we reduce the CPU usage, if SRB is being polled for input
// but only as long as the transmitter is empty and ready
// and the initial delay time has passed w/o IO

static void sio_sleep(int delay) {
	if (!apollo_config(APOLLO_CONF_IDLE_SLEEP)) {
		// nothing to do; sleeping is not enabled
	} else if (delay <= 0) {
		//reset the sleep delay time
		if (sleep_time > 0) {
			LOG2(("sio_sleep: sleeping stopped"))
			sleep_time = 0;
		}
	} else if (sleep_time < delay) {
		// sleep delay pending (i.e. don't sleep)
		sleep_time++;
	} else {
		if (sleep_time == delay) {
			LOG2(("sio_sleep: sleeping started after %d ms",sleep_time));
			sleep_time++;
		}
		// Note: ticks_per_second/100 will sleep for 6 ms (= 4-10 ms)
		osd_sleep(osd_ticks_per_second() / 50);
	}
}

/*-------------------------------------------------
 apollo_sio_rx_data - get character from keyboard/stdin
 -------------------------------------------------*/

void apollo_sio_rx_data(device_t* device, int ch, UINT8 data) {
	// omit logging for channel 1
	if (ch == 0) {
		DLOG1(("apollo_sio_rx_data ch=%d <- data=%02x", ch, data ));
	}

	if (ch == 1 && !isInitialized && data == '\r' && apollo_csr_get_servicemode()) {
		// force baudrate recognition
		data = 0xff;
		isInitialized = 1;
	}

	duart68681_rx_data(device, ch, data);
}

/*-------------------------------------------------
 apollo_sio_tx_data - put character to display/stdout
 -------------------------------------------------*/

static void apollo_sio_tx_data(device_t *device, int channel, UINT8 data) {
	if (channel == 0) {
		DLOG1(("apollo_sio_tx_data ch=%d -> data=%02x", channel, data ));
		device_t *keyboard = device->machine().device( APOLLO_KBD_TAG );
		if (keyboard != NULL) {
			apollo_kbd_getchar(keyboard, data);
		}
	} else if (channel == 1) {
		DLOG2(("apollo_sio_tx_data ch=%d -> data=%02x", channel, data ));

		if (data != '\r') {
			// output data to stdout
			putchar(data);
			fflush(stdout);

			if (apollo_is_dsp3x00()) {
				// output data to terminal emulator
				apollo_terminal_write(data);
			}
		}
	}
}

/*-------------------------------------------------
 sio configuration
 -------------------------------------------------*/

static void sio_irq_handler(device_t *device, int state, UINT8 vector) {
	DLOG2(("sio_irq_handler: vector=%02x", vector ));
	apollo_pic_set_irq_line(device, APOLLO_IRQ_SIO1, state);
	sio_irq_line = 1;
}

static UINT8 sio_input(device_t *device) {
	// necessary for DN3000?
	// sio_input_data = sio_input_data ? 0 : 0x0f;
	DLOG2(("reading 2681 input: %02x", sio_input_data ));
	return sio_input_data;
}

static void sio_output(device_t *device, UINT8 data) {
	DLOG1(("writing 2681 output: %02x", data ));

	if ((data & 0x80) != (sio_output_data & 0x80)) {
		apollo_pic_set_irq_line(device, APOLLO_IRQ_DIAG, (data & 0x80) ? 1 : 0);
		sio_output_data = data;
	}
}

const duart68681_config apollo_sio_config = {
		sio_irq_handler,
		apollo_sio_tx_data,
		sio_input,
		sio_output
};

/*-------------------------------------------------
 DN3000/DS3500 SIO at 0x8400/0x10400
 -------------------------------------------------*/

READ8_DEVICE_HANDLER(apollo_sio_r) {
	static int last_read8_offset[2] = { -1, -1 };
	static int last_read8_value[2] = { -1, -1 };

	static const char * const duart68681_reg_read_names[0x10] = { "MRA", "SRA",
			"BRG Test", "RHRA", "IPCR", "ISR", "CTU", "CTL", "MRB", "SRB",
			"1X/16X Test", "RHRB", "IVR", "Input Ports", "Start Counter",
			"Stop Counter" };

	int data = duart68681_r(device, space, offset / 2);

	if (sio_irq_line) {
		apollo_pic_set_irq_line(device, APOLLO_IRQ_SIO1, 0);
		sio_irq_line = 0;
	}

	switch (offset / 2) {
	case 0x0b: /* RHRB */
		if (data == 0x0d && sio_csrb == 0x77) {
			// special for MD command SK (Select keyboard) with baudrate set to 2000
			data = 0xff;
		}
		break;
	}

	// omit logging if sio is being polled from the boot rom
	if ((offset != last_read8_offset[1] || data != last_read8_value[1])
			&& (offset != last_read8_offset[0] || data != last_read8_value[0])) {
		last_read8_offset[0] = last_read8_offset[1];
		last_read8_value[0] = last_read8_value[1];
		last_read8_offset[1] = offset;
		last_read8_value[1] = data;
		DLOG2(("reading 2681 reg %x (%s) returned %02x",
				offset, duart68681_reg_read_names[offset/2], data ));
	}

	return data;
}

WRITE8_DEVICE_HANDLER(apollo_sio_w)
{
	static const char * const duart68681_reg_write_names[0x10] = { "MRA",
			"CSRA", "CRA", "THRA", "ACR", "IMR", "CRUR", "CTLR", "MRB", "CSRB",
			"CRB", "THRB", "IVR", "OPCR", "Set OP Bits", "Reset OP Bits" };

	if (sio_irq_line) {
		apollo_pic_set_irq_line(device, APOLLO_IRQ_SIO1, 0);
		sio_irq_line = 0;
	}

	// don't log THRB
	if (offset != 0x17) {
		DLOG2(("writing 2681 reg %x (%s) with %02x", offset, duart68681_reg_write_names[(offset/2) & 15], data ));
	}

	switch (offset / 2) {
	case 0x09: /* CSRB */
		// remember CSRB to handle MD command SK on DSP3x00
		sio_csrb = data;
		break;
	case 0x0b: /* THRB */
		// stop sleeping
		sio_sleep(0);
		break;
	case 0x0d: /* OPCR */
		if ((data & 0x0c) == 0x04) {
			// Unhandled OPCR value; used for RAM refresh circuit
			// ignore value; omit error message
			data &= ~0x0c;
		}
		break;
	}
	duart68681_w(device, space, offset / 2, data);
}

/*-------------------------------------------------
 kbd tty timer callback
 -------------------------------------------------*/

static TIMER_CALLBACK(kbd_timer_callback)
{

#if defined(APOLLO_FOR_LINUX)
	device_t *device = (device_t *) ptr;
	UINT8 data;

#define SRA 0x01
#define SRB 0x09

	if (!(duart68681_r(device, space, SRB) & 0x02))
	{
		// Channel B FIFO not yet full (STATUS_FIFO_FULL)
		if (read(STDIN_FILENO, &data, 1) == 1)
		{
			apollo_sio_rx_data(device, 1, data == '\n' ? '\r' : data);
			input_from_stdin = 1;
			// stop sleeping to reduce CPU usage
			sio_sleep(0);
		}
		else if (input_from_stdin && (duart68681_r(device, space, SRB) & 0x0c) == 0x0c)
		{
			// we reduce the CPU usage, if SRB is being polled for input
			// but only as long as the transmitter is empty and ready
			// and the initial delay time has has passed
			sio_sleep(SIO_SLEEP_DELAY_TIME);
		}
	}
#endif

	// The counter/timer on the SIO chip is used for the refresh count.
	// This is set up in the timer mode to  produce a square wave output on output OP3.
	// The period of the output is 15 microseconds.

	// toggle memory refresh counter
	sio_input_data ^= 0x01;
}

/*-------------------------------------------------
 device start callback
 -------------------------------------------------*/

static DEVICE_START(apollo_sio)
{
	kbd_timer = device->machine().scheduler().timer_alloc(FUNC(kbd_timer_callback), device);
}

/*-------------------------------------------------
 device reset callback
 -------------------------------------------------*/

static DEVICE_RESET(apollo_sio)
{
	DLOG1(("reset apollo_sio"));

	isInitialized = 0;
	input_from_stdin = 0;
	sleep_time = 0;
	sio_input_data = apollo_get_ram_config_byte();
	sio_output_data = 0xff;

#if defined(APOLLO_FOR_LINUX)
	// FIXME: unavailable in mingw
	// set stdin to nonblocking to allow polling in sio_poll_rxb
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
#endif

	// start the keyboard timer
	kbd_timer->adjust( attotime::zero, 0, attotime::from_msec(1));
}

//##########################################################################
// machine/apollo_sio2.c - APOLLO DS3500 SIO2
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

/*-------------------------------------------------
 sio2 configuration (DN3500 only)
 -------------------------------------------------*/

static void sio2_irq_handler(device_t *device, int state, UINT8 vector)
{
	DLOG1(("sio2_irq_handler: vector=%02x", vector ));
	apollo_pic_set_irq_line(device, APOLLO_IRQ_SIO2, state);
}

static void sio2_tx_data(device_t *device, int channel, UINT8 data)
{
	DLOG1(("apollo_sio2_tx_data ch=%d -> data=%02x", channel, data ));
}

static UINT8 sio2_input(device_t *device)
{
	UINT8 data = 0x00;
	DLOG1(("reading 2681 input: %02x", data ));
	return data;
}

static void sio2_output(device_t *device, UINT8 data)
{
	DLOG1(("writing 2681 output: %02x", data ));
}

const duart68681_config apollo_sio2_config = {
		sio2_irq_handler,
		sio2_tx_data,
		sio2_input,
		sio2_output
};

/*-------------------------------------------------
 DN3500 SIO2 at 0x10500
 -------------------------------------------------*/

READ8_DEVICE_HANDLER(apollo_sio2_r)
{
	static const char * const duart68681_reg_read_names[0x10] = { "MRA", "SRA",
			"BRG Test", "RHRA", "IPCR", "ISR", "CTU", "CTL", "MRB", "SRB",
			"1X/16X Test", "RHRB", "IVR", "Input Ports", "Start Counter",
			"Stop Counter" };

	apollo_pic_set_irq_line(device, APOLLO_IRQ_SIO2, 0);

	int data = duart68681_r(device, space, offset / 2);

	DLOG2(("reading 2681 reg %x (%s) returned %02x",
				offset, duart68681_reg_read_names[offset/2], data ));
	return data;
}

WRITE8_DEVICE_HANDLER(apollo_sio2_w)
{
	static const char * const duart68681_reg_write_names[0x10] = { "MRA",
			"CSRA", "CRA", "THRA", "ACR", "IMR", "CRUR", "CTLR", "MRB", "CSRB",
			"CRB", "THRB", "IVR", "OPCR", "Set OP Bits", "Reset OP Bits" };

	DLOG2(("writing 2681 reg %x (%s) with %02x", offset, duart68681_reg_write_names[(offset/2) & 15], data ));

	apollo_pic_set_irq_line(device, APOLLO_IRQ_SIO2, 0);

	switch (offset / 2) {
	case 0x04: /* ACR */
		if (data == 0x80) {
			// FIXME: unhandled ACR value
			// data = 0xe0;
		}
		break;
	}

	duart68681_w(device, space, offset / 2, data);
}

/*-------------------------------------------------
 device start callback
 -------------------------------------------------*/

static DEVICE_START(apollo_sio2)
{
	DLOG1(("start apollo_sio2"));
}

/*-------------------------------------------------
 device reset callback
 -------------------------------------------------*/

static DEVICE_RESET(apollo_sio2)
{
	DLOG1(("reset apollo_sio2"));
}

//##########################################################################
// machine/apollo_fdc.c - APOLLO DS3500 Floppy disk controller
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

FLOPPY_FORMATS_MEMBER( apollo_state::floppy_formats )
	FLOPPY_APOLLO_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( apollo_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END


void apollo_state::fdc_interrupt(bool state) {
	apollo_pic_set_irq_line( machine().firstcpu, APOLLO_IRQ_FDC, state ? ASSERT_LINE : CLEAR_LINE);
}

void apollo_state::fdc_dma_drq(bool state) {
	apollo_dma_fdc_drq(machine().firstcpu, state);
}

/***************************************************************************
 DN3500 3c505 DEVICE Configuration
 ***************************************************************************/

static void apollo_3c505_set_irq(device_t *device, int state) {
	// DLOG2(("apollo_3c505_interrupt: state=%x", state ));
	apollo_pic_set_irq_line(device->machine().firstcpu, APOLLO_IRQ_ETH1, state);
}

static int apollo_3c505_tx_data(device_t *device,
		const UINT8 tx_data_buffer[], int tx_data_length) {

	// transmit all transmitted packets to the apollo_netserver
	apollo_netserver_receive(device, tx_data_buffer, tx_data_length);

	// transmit all transmitted packets to the host ethernet (ignore any errors)
	return apollo_eth_transmit(device, tx_data_buffer, tx_data_length);
}

static int apollo_3c505_setfilter(device_t *device, int node_id)
{
	return apollo_eth_setfilter(device, node_id);
}

static int apollo_3c505_rx_data(device_t *device,
		const UINT8 rx_data_buffer[], int rx_data_length) {

	// transmit all received packets to the threecom3c505 receiver
	return threecom3c505_receive(device, rx_data_buffer, rx_data_length);
}

static void apollo_3c505_tx_init(device_t *device) {

	apollo_eth_init(device, apollo_3c505_rx_data);

	// setup to receive all packets from the apollo_netserver
	apollo_netserver_init(device->machine().options().media_path(), apollo_3c505_rx_data);
}

static THREECOM3C505_INTERFACE(apollo_3c505_config) = {
	apollo_3c505_set_irq,
	apollo_3c505_tx_init,
	apollo_3c505_tx_data,
	apollo_3c505_setfilter
};

/***************************************************************************
 DN3500 OMTI 8621 DEVICE Configuration
 ***************************************************************************/

static void apollo_wdc_set_irq(const running_machine *machine, int state) {
//  FIXME:
//  MLOG2(("apollo_wdc_set_irq: state=%x", state ));
	apollo_pic_set_irq_line(machine->firstcpu, APOLLO_IRQ_WIN1, state);
}

static const omti8621_config apollo_wdc_config = {
		apollo_wdc_set_irq
};

/***************************************************************************
 DN3500 Cartridge Tape DEVICE Configuration
 ***************************************************************************/

static void apollo_ctape_set_irq(const device_t *device, int state) {
	DLOG2(("apollo_ctape_set_irq: state=%x", state ));
	apollo_pic_set_irq_line(device->machine().firstcpu, APOLLO_IRQ_CTAPE, state);
}


static void apollo_ctape_dma_drq(const device_t *device, int state) {
	DLOG2(("apollo_ctape_dma_drq: state=%x", state ));
	apollo_dma_ctape_drq(device->machine().firstcpu, state);
}

static const sc499_interface apollo_ctape_config = {
	apollo_ctape_set_irq,
	apollo_ctape_dma_drq,
};

//##########################################################################
// machine/apollo.c - APOLLO DS3500 CPU Board
//##########################################################################

#undef VERBOSE
#define VERBOSE 0

MACHINE_CONFIG_FRAGMENT( apollo )
	// configuration MUST be reset first !
	MCFG_DEVICE_ADD(APOLLO_CONF_TAG, APOLLO_CONF, 0)

	MCFG_I8237_ADD( APOLLO_DMA1_TAG, XTAL_14_31818MHz/3, apollo_dma8237_1_config )
	MCFG_I8237_ADD( APOLLO_DMA2_TAG, XTAL_14_31818MHz/3, apollo_dma8237_2_config )
	MCFG_PIC8259_ADD( APOLLO_PIC1_TAG, apollo_pic8259_master_config )
	MCFG_PIC8259_ADD( APOLLO_PIC2_TAG, apollo_pic8259_slave_config )
	MCFG_PTM6840_ADD(APOLLO_PTM_TAG, apollo_ptm_config)
	MCFG_MC146818_ADD( APOLLO_RTC_TAG, MC146818_UTC )
    MCFG_DUART68681_ADD( APOLLO_SIO_TAG, XTAL_3_6864MHz, apollo_sio_config )
    MCFG_DUART68681_ADD( APOLLO_SIO2_TAG, XTAL_3_6864MHz, apollo_sio2_config )

	MCFG_PC_FDC_AT_ADD(APOLLO_FDC_TAG)
	MCFG_FLOPPY_DRIVE_ADD(APOLLO_FDC_TAG ":0", apollo_floppies, "525hd", 0, apollo_state::floppy_formats)

	MCFG_OMTI8621_ADD(APOLLO_WDC_TAG, apollo_wdc_config)
	MCFG_SC499_ADD(APOLLO_CTAPE_TAG, apollo_ctape_config)
	MCFG_THREECOM3C505_ADD(APOLLO_ETH_TAG, apollo_3c505_config)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(apollo_state,apollo)
{
	//MLOG1(("driver_init_apollo"));
}

MACHINE_START_MEMBER(apollo_state,apollo)
{
	//MLOG1(("machine_start_apollo"));

	pc_fdc_at_device *fdc = machine().device<pc_fdc_at_device>(APOLLO_FDC_TAG);
	fdc->setup_intrq_cb(pc_fdc_at_device::line_cb(FUNC(apollo_state::fdc_interrupt), this));
	fdc->setup_drq_cb(pc_fdc_at_device::line_cb(FUNC(apollo_state::fdc_dma_drq), this));

	device_start_apollo_ptm (machine().device(APOLLO_PTM_TAG) );
	device_start_apollo_sio(machine().device(APOLLO_SIO_TAG));
	device_start_apollo_sio2(machine().device(APOLLO_SIO2_TAG));

	if (apollo_is_dn3000())
	{
		//MLOG1(("faking mc146818 interrupts (DN3000 only)"));
		// fake mc146818 interrupts (DN3000 only)
		machine().scheduler().timer_pulse(attotime::from_hz(2), FUNC(apollo_rtc_timer));
	}
}

MACHINE_RESET_MEMBER(apollo_state,apollo)
{
	//MLOG1(("machine_reset_apollo"));

	dma8237_1 = machine().device(APOLLO_DMA1_TAG);
	dma8237_2 = machine().device(APOLLO_DMA2_TAG);
	pic8259_master = machine().device(APOLLO_PIC1_TAG);
	pic8259_slave = machine().device(APOLLO_PIC2_TAG);

	// set configuration
	apollo_csr_set_servicemode(apollo_config(APOLLO_CONF_SERVICE_MODE));

	device_reset_apollo_ptm(machine().device(APOLLO_PTM_TAG));
	device_reset_apollo_rtc(machine().device(APOLLO_RTC_TAG));
	device_reset_apollo_sio(machine().device(APOLLO_SIO_TAG));
	device_reset_apollo_sio2(machine().device(APOLLO_SIO2_TAG));
}
