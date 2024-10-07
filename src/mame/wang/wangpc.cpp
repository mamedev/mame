// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Wang Professional Computer

    http://www.seasip.info/VintagePC/wangpc.html

    chdman -createblankhd q540.chd 512 8 17 512

*/

/*

    TODO:

    - with quantum perfect cpu gets stuck @ 49c3 mov ss,cs:[52ah]
    - hard disk

*/

#include "emu.h"
#include "softlist_dev.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "bus/wangpc/wangpc.h"
#include "cpu/i86/i86.h"
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/i8255.h"
#include "machine/im6402.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/scn_pci.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "wangpckb.h"


namespace {

#define I8086_TAG       "i8086"
#define AM9517A_TAG     "am9517a"
#define I8259A_TAG      "i8259"
#define I8255A_TAG      "i8255a"
#define I8253_TAG       "i8253"
#define IM6402_TAG      "im6402"
#define SCN2661_TAG     "scn2661"
#define UPD765_TAG      "upd765"
#define CENTRONICS_TAG  "centronics"
#define RS232_TAG       "rs232"
#define WANGPC_KEYBOARD_TAG "wangpckb"
#define LED_DIAGNOSTIC  "led0"

class wangpc_state : public driver_device
{
public:
	// constructor
	wangpc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I8086_TAG),
		m_dmac(*this, AM9517A_TAG),
		m_pic(*this, I8259A_TAG),
		m_ppi(*this, I8255A_TAG),
		m_pit(*this, I8253_TAG),
		m_uart(*this, IM6402_TAG),
		m_epci(*this, SCN2661_TAG),
		m_fdc(*this, UPD765_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy(*this, UPD765_TAG ":%u:525dd", 0U),
		m_centronics(*this, CENTRONICS_TAG),
		m_cent_data_in(*this, "cent_data_in"),
		m_cent_data_out(*this, "cent_data_out"),
		m_bus(*this, "wangpcbus"),
		m_sw(*this, "SW"),
		m_led_diagnostic(*this, LED_DIAGNOSTIC),
		m_timer2_irq(1),
		m_centronics_ack(1),
		m_dav(1),
		m_dma_eop(1),
		m_uart_dr(0),
		m_uart_tbre(0),
		m_fpu_irq(0),
		m_bus_irq2(0),
		m_enable_eop(0),
		m_disable_dreq2(0),
		m_fdc_drq(0),
		m_fdc_dd0(0),
		m_fdc_dd1(0),
		m_fdc_tc(0),
		m_ds1(false),
		m_ds2(false)
	{
	}

	void wangpc(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<i8255_device> m_ppi;
	required_device<pit8253_device> m_pit;
	required_device<im6402_device> m_uart;
	required_device<scn_pci_device> m_epci;
	required_device<upd765a_device> m_fdc;
	required_device<ram_device> m_ram;
	required_device_array<floppy_image_device, 2> m_floppy;
	required_device<centronics_device> m_centronics;
	required_device<input_buffer_device> m_cent_data_in;
	required_device<output_latch_device> m_cent_data_out;
	required_device<wangpcbus_device> m_bus;
	required_ioport m_sw;
	output_finder<> m_led_diagnostic;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void select_drive();
	void check_level1_interrupts();
	void check_level2_interrupts();
	void update_fdc_drq();
	void update_fdc_tc();

	void fdc_ctrl_w(uint8_t data);
	uint8_t deselect_drive1_r();
	void deselect_drive1_w(uint8_t data);
	uint8_t select_drive1_r();
	void select_drive1_w(uint8_t data);
	uint8_t deselect_drive2_r();
	void deselect_drive2_w(uint8_t data);
	uint8_t select_drive2_r();
	void select_drive2_w(uint8_t data);
	uint8_t motor1_on_r(offs_t offset);
	void motor1_on_w(offs_t offset, uint8_t data);
	uint8_t motor2_on_r(offs_t offset);
	void motor2_on_w(offs_t offset, uint8_t data);
	uint8_t fdc_reset_r();
	void fdc_reset_w(uint8_t data);
	uint8_t fdc_tc_r();
	void fdc_tc_w(uint8_t data);
	void dma_page_w(offs_t offset, uint8_t data);
	uint8_t status_r();
	void timer0_irq_clr_w(uint8_t data);
	uint8_t timer2_irq_clr_r();
	void nmi_mask_w(uint8_t data);
	uint8_t led_on_r();
	void fpu_mask_w(uint8_t data);
	uint8_t dma_eop_clr_r();
	void uart_tbre_clr_w(uint8_t data);
	uint8_t uart_r();
	void uart_w(uint8_t data);
	uint8_t centronics_r();
	void centronics_w(uint8_t data);
	uint8_t busy_clr_r();
	void acknlg_clr_w(uint8_t data);
	uint8_t led_off_r();
	void parity_nmi_clr_w(uint8_t data);
	uint8_t option_id_r();

	void hrq_w(int state);
	void eop_w(int state);
	uint8_t memr_r(offs_t offset);
	void memw_w(offs_t offset, uint8_t data);
	uint8_t ior2_r();
	void iow2_w(uint8_t data);
	void dack0_w(int state);
	void dack1_w(int state);
	void dack2_w(int state);
	void dack3_w(int state);
	uint8_t ppi_pa_r();
	uint8_t ppi_pb_r();
	uint8_t ppi_pc_r();
	void ppi_pc_w(uint8_t data);
	void pit0_w(int state);
	void pit2_w(int state);
	void uart_dr_w(int state);
	void uart_tbre_w(int state);
	void epci_irq_w(int state);
	void write_centronics_ack(int state);
	void write_centronics_busy(int state);
	void write_centronics_fault(int state);
	void write_centronics_perror(int state);
	void bus_irq2_w(int state);

	void fdc_irq(int state);
	void fdc_drq(int state);

	void on_disk0_load(floppy_image_device *image);
	void on_disk0_unload(floppy_image_device *image);
	void on_disk1_load(floppy_image_device *image);
	void on_disk1_unload(floppy_image_device *image);

	void wangpc_io(address_map &map) ATTR_COLD;
	void wangpc_mem(address_map &map) ATTR_COLD;

	uint8_t m_dma_page[4];
	int m_dack;

	int m_timer2_irq;
	int m_centronics_ack;
	int m_centronics_busy;
	int m_centronics_fault;
	int m_centronics_perror;
	int m_dav;
	int m_dma_eop;
	int m_uart_dr;
	int m_uart_tbre;
	int m_fpu_irq;
	int m_bus_irq2;

	int m_enable_eop;
	int m_disable_dreq2;
	int m_fdc_drq;
	int m_fdc_dd0;
	int m_fdc_dd1;
	int m_fdc_tc;
	int m_ds1;
	int m_ds2;

	int m_led[6];
};



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void wangpc_state::select_drive()
{
	floppy_image_device *floppy = nullptr;

	if (m_ds1) floppy = m_floppy[0];
	if (m_ds2) floppy = m_floppy[1];

	m_fdc->set_floppy(floppy);
}

void wangpc_state::fdc_ctrl_w(uint8_t data)
{
	/*

	    bit     description

	    0       Enable /EOP
	    1       Disable /DREQ2
	    2       Clear drive 1 door disturbed interrupt
	    3       Clear drive 2 door disturbed interrupt
	    4
	    5
	    6
	    7

	*/

	m_enable_eop = BIT(data, 0);
	m_disable_dreq2 = BIT(data, 1);

	if (BIT(data, 2)) m_fdc_dd0 = 0;
	if (BIT(data, 3)) m_fdc_dd1 = 0;

	if (LOG)
	{
		logerror("%s: Enable /EOP %u\n", machine().describe_context(), m_enable_eop);
		logerror("%s: Disable /DREQ2 %u\n", machine().describe_context(), m_disable_dreq2);
	}

	update_fdc_tc();
	update_fdc_drq();
}


uint8_t wangpc_state::deselect_drive1_r()
{
	m_ds1 = false;
	select_drive();

	return 0xff;
}

void wangpc_state::deselect_drive1_w(uint8_t data)
{
	deselect_drive1_r();
}

uint8_t wangpc_state::select_drive1_r()
{
	m_ds1 = true;
	select_drive();

	return 0xff;
}

void wangpc_state::select_drive1_w(uint8_t data)
{
	select_drive1_r();
}

uint8_t wangpc_state::deselect_drive2_r()
{
	m_ds2 = false;
	select_drive();

	return 0xff;
}

void wangpc_state::deselect_drive2_w(uint8_t data)
{
	deselect_drive2_r();
}

uint8_t wangpc_state::select_drive2_r()
{
	m_ds2 = true;
	select_drive();

	return 0xff;
}

void wangpc_state::select_drive2_w(uint8_t data)
{
	select_drive2_r();
}

uint8_t wangpc_state::motor1_on_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		motor1_on_w(offset, 0);

	return 0xff;
}

void wangpc_state::motor1_on_w(offs_t offset, uint8_t data)
{
	if (LOG) logerror("%s: Drive 1 motor %s\n", machine().describe_context(), offset ? "ON" : "OFF");

	m_floppy[0]->mon_w(!offset);
}

uint8_t wangpc_state::motor2_on_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		motor2_on_w(offset, 0);

	return 0xff;
}

void wangpc_state::motor2_on_w(offs_t offset, uint8_t data)
{
	if (LOG) logerror("%s: Drive 2 motor %s\n", machine().describe_context(), offset ? "ON" : "OFF");

	m_floppy[1]->mon_w(!offset);
}

uint8_t wangpc_state::fdc_reset_r()
{
	if (LOG) logerror("%s: FDC reset\n", machine().describe_context());

	m_fdc->reset();

	return 0xff;
}

void wangpc_state::fdc_reset_w(uint8_t data)
{
	fdc_reset_r();
}

uint8_t wangpc_state::fdc_tc_r()
{
	if (LOG) logerror("%s: FDC TC\n", machine().describe_context());

	m_fdc->tc_w(1);
	m_fdc->tc_w(0);

	return 0xff;
}

void wangpc_state::fdc_tc_w(uint8_t data)
{
	fdc_tc_r();
}


//-------------------------------------------------
//  dma_page_w -
//-------------------------------------------------

void wangpc_state::dma_page_w(offs_t offset, uint8_t data)
{
	if (LOG) logerror("%s: DMA page %u: %06x\n", machine().describe_context(), offset + 1, (data & 0x0f) << 16);

	m_dma_page[offset + 1] = data & 0x0f;
}


//-------------------------------------------------
//  status_r -
//-------------------------------------------------

uint8_t wangpc_state::status_r()
{
	/*

	    bit     description

	    0       Memory Parity Flag
	    1       I/O Error Flag
	    2       Unassigned
	    3       FDC Interrupt Flag
	    4       Door disturbed on drive 1
	    5       Door disturbed on drive 2
	    6       Door open on drive 1
	    7       Door open on drive 2

	*/

	uint8_t data = 0x03;

	// floppy interrupts
	data |= m_fdc->get_irq() << 3;
	data |= m_fdc_dd0 << 4;
	data |= m_fdc_dd1 << 5;
	data |= m_floppy[0]->exists() ? 0 : 0x40;
	data |= m_floppy[1]->exists() ? 0 : 0x80;

	return data;
}


//-------------------------------------------------
//  timer0_int_clr_w -
//-------------------------------------------------

void wangpc_state::timer0_irq_clr_w(uint8_t data)
{
	//if (LOG) logerror("%s: Timer 0 IRQ clear\n", machine().describe_context());

	m_pic->ir0_w(0);
}


//-------------------------------------------------
//  timer2_irq_clr_r -
//-------------------------------------------------

uint8_t wangpc_state::timer2_irq_clr_r()
{
	//if (LOG) logerror("%s: Timer 2 IRQ clear\n", machine().describe_context());

	m_timer2_irq = 1;
	check_level1_interrupts();

	return 0xff;
}


//-------------------------------------------------
//  nmi_mask_w -
//-------------------------------------------------

void wangpc_state::nmi_mask_w(uint8_t data)
{
	if (LOG) logerror("%s: NMI mask %02x\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  led_on_r -
//-------------------------------------------------

uint8_t wangpc_state::led_on_r()
{
	if (LOG) logerror("%s: Diagnostic LED on\n", machine().describe_context());

	m_led_diagnostic = 1;

	return 0xff;
}


//-------------------------------------------------
//  fpu_mask_w -
//-------------------------------------------------

void wangpc_state::fpu_mask_w(uint8_t data)
{
	if (LOG) logerror("%s: FPU mask %02x\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  dma_eop_clr_r -
//-------------------------------------------------

uint8_t wangpc_state::dma_eop_clr_r()
{
	if (LOG) logerror("%s: EOP clear\n", machine().describe_context());

	m_dma_eop = 1;

	check_level2_interrupts();

	return 0xff;
}


//-------------------------------------------------
//  uart_tbre_clr_w -
//-------------------------------------------------

void wangpc_state::uart_tbre_clr_w(uint8_t data)
{
	if (LOG) logerror("%s: TBRE clear\n", machine().describe_context());

	m_uart_tbre = 0;

	check_level2_interrupts();
}


//-------------------------------------------------
//  uart_r -
//-------------------------------------------------

uint8_t wangpc_state::uart_r()
{
	m_uart_dr = 0;

	check_level2_interrupts();

	uint8_t data = m_uart->read();

	if (LOG) logerror("%s: UART read %02x\n", machine().describe_context(), data);

	return data;
}


//-------------------------------------------------
//  uart_w -
//-------------------------------------------------

void wangpc_state::uart_w(uint8_t data)
{
	if (LOG) logerror("%s: UART write %02x\n", machine().describe_context(), data);

	switch (data)
	{
	case 0x10: m_led[0] = 1; break;
	case 0x11: m_led[0] = 0; break;
	case 0x12: m_led[1] = 1; break;
	case 0x13: m_led[1] = 0; break;
	case 0x14: m_led[2] = 1; break;
	case 0x15: m_led[2] = 0; break;
	case 0x16: m_led[3] = 1; break;
	case 0x17: m_led[3] = 0; break;
	case 0x18: m_led[4] = 1; break;
	case 0x19: m_led[4] = 0; break;
	case 0x1a: m_led[5] = 1; break;
	case 0x1b: m_led[5] = 0; break;
	case 0x1c: m_led[0] = m_led[1] = m_led[2] = m_led[3] = m_led[4] = m_led[5] = 1; break;
	case 0x1d: m_led[0] = m_led[1] = m_led[2] = m_led[3] = m_led[4] = m_led[5] = 0; break;
	}

	if (LOG) popmessage("%u%u%u%u%u%u", m_led[0], m_led[1], m_led[2], m_led[3], m_led[4], m_led[5]);

	m_uart_tbre = 0;
	check_level2_interrupts();

	m_uart->write(data);
}


//-------------------------------------------------
//  centronics_r -
//-------------------------------------------------

uint8_t wangpc_state::centronics_r()
{
	m_dav = 1;
	check_level1_interrupts();

	return m_cent_data_in->read();
}


//-------------------------------------------------
//  centronics_w -
//-------------------------------------------------

void wangpc_state::centronics_w(uint8_t data)
{
	m_centronics_ack = 1;
	check_level1_interrupts();

	m_cent_data_out->write(data);

	m_centronics->write_strobe(0);
	m_centronics->write_strobe(1);
}


//-------------------------------------------------
//  busy_clr_r -
//-------------------------------------------------

uint8_t wangpc_state::busy_clr_r()
{
	if (LOG) logerror("%s: BUSY clear\n", machine().describe_context());

	m_centronics_busy = 0;
	check_level1_interrupts();

	return 0xff;
}


//-------------------------------------------------
//  acknlg_clr_w -
//-------------------------------------------------

void wangpc_state::acknlg_clr_w(uint8_t data)
{
	if (LOG) logerror("%s: ACKNLG clear\n", machine().describe_context());

	m_centronics_ack = 1;
	check_level1_interrupts();
}


//-------------------------------------------------
//  led_off_r -
//-------------------------------------------------

uint8_t wangpc_state::led_off_r()
{
	if (LOG) logerror("%s: Diagnostic LED off\n", machine().describe_context());

	m_led_diagnostic = 0;

	return 0xff;
}


//-------------------------------------------------
//  parity_nmi_clr_w -
//-------------------------------------------------

void wangpc_state::parity_nmi_clr_w(uint8_t data)
{
	if (LOG) logerror("%s: Parity NMI clear\n", machine().describe_context());
}


//-------------------------------------------------
//  option_id_r -
//-------------------------------------------------

uint8_t wangpc_state::option_id_r()
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7       FDC Interrupt Flag

	*/

	uint8_t data = 0;

	// FDC interrupt
	data |= (m_fdc_dd0 || m_fdc_dd1 || (int) m_fdc->get_irq()) << 7;

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( wangpc_mem )
//-------------------------------------------------

void wangpc_state::wangpc_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x1ffff).ram();
	map(0x40000, 0xf3fff).rw(m_bus, FUNC(wangpcbus_device::mrdc_r), FUNC(wangpcbus_device::amwc_w));
	map(0xfc000, 0xfffff).rom().region(I8086_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( wangpc_io )
//-------------------------------------------------

void wangpc_state::wangpc_io(address_map &map)
{
	map.unmap_value_high();
	map(0x1000, 0x1000).w(FUNC(wangpc_state::fdc_ctrl_w));
	map(0x1004, 0x1004).rw(FUNC(wangpc_state::deselect_drive1_r), FUNC(wangpc_state::deselect_drive1_w));
	map(0x1006, 0x1006).rw(FUNC(wangpc_state::select_drive1_r), FUNC(wangpc_state::select_drive1_w));
	map(0x1008, 0x1008).rw(FUNC(wangpc_state::deselect_drive2_r), FUNC(wangpc_state::deselect_drive2_w));
	map(0x100a, 0x100a).rw(FUNC(wangpc_state::select_drive2_r), FUNC(wangpc_state::select_drive2_w));
	map(0x100c, 0x100f).rw(FUNC(wangpc_state::motor1_on_r), FUNC(wangpc_state::motor1_on_w)).umask16(0x00ff);
	map(0x1010, 0x1013).rw(FUNC(wangpc_state::motor2_on_r), FUNC(wangpc_state::motor2_on_w)).umask16(0x00ff);
	map(0x1014, 0x1017).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0x1018, 0x1018).mirror(0x0002).rw(FUNC(wangpc_state::fdc_reset_r), FUNC(wangpc_state::fdc_reset_w));
	map(0x101c, 0x101c).mirror(0x0002).rw(FUNC(wangpc_state::fdc_tc_r), FUNC(wangpc_state::fdc_tc_w));
	map(0x1020, 0x1027).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x1028, 0x1029); //.w(FUNC(wangpc_state::)); (?)
	map(0x1040, 0x1047).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x1060, 0x1063).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x1080, 0x1087).r(m_epci, FUNC(scn_pci_device::read)).umask16(0x00ff);
	map(0x1088, 0x108f).w(m_epci, FUNC(scn_pci_device::write)).umask16(0x00ff);
	map(0x10a0, 0x10bf).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0x00ff);
	map(0x10c2, 0x10c7).w(FUNC(wangpc_state::dma_page_w)).umask16(0x00ff);
	map(0x10e0, 0x10e0).rw(FUNC(wangpc_state::status_r), FUNC(wangpc_state::timer0_irq_clr_w));
	map(0x10e2, 0x10e2).rw(FUNC(wangpc_state::timer2_irq_clr_r), FUNC(wangpc_state::nmi_mask_w));
	map(0x10e4, 0x10e4).rw(FUNC(wangpc_state::led_on_r), FUNC(wangpc_state::fpu_mask_w));
	map(0x10e6, 0x10e6).rw(FUNC(wangpc_state::dma_eop_clr_r), FUNC(wangpc_state::uart_tbre_clr_w));
	map(0x10e8, 0x10e8).rw(FUNC(wangpc_state::uart_r), FUNC(wangpc_state::uart_w));
	map(0x10ea, 0x10ea).rw(FUNC(wangpc_state::centronics_r), FUNC(wangpc_state::centronics_w));
	map(0x10ec, 0x10ec).rw(FUNC(wangpc_state::busy_clr_r), FUNC(wangpc_state::acknlg_clr_w));
	map(0x10ee, 0x10ee).rw(FUNC(wangpc_state::led_off_r), FUNC(wangpc_state::parity_nmi_clr_w));
	map(0x10fe, 0x10fe).r(FUNC(wangpc_state::option_id_r));
	map(0x1100, 0x1fff).rw(m_bus, FUNC(wangpcbus_device::sad_r), FUNC(wangpcbus_device::sad_w));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( wangpc )
//-------------------------------------------------

static INPUT_PORTS_START( wangpc )
	// keyboard defined in machine/wangpckb.c

	PORT_START("SW")
	PORT_DIPNAME( 0x0f, 0x0f, "CPU Baud Rate" ) PORT_DIPLOCATION("SW:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "19200" )
	PORT_DIPSETTING(    0x0e, "9600" )
	PORT_DIPSETTING(    0x0d, "7200" )
	PORT_DIPSETTING(    0x0c, "4800" )
	PORT_DIPSETTING(    0x0b, "3600" )
	PORT_DIPSETTING(    0x0a, "2400" )
	PORT_DIPSETTING(    0x09, "2000" )
	PORT_DIPSETTING(    0x08, "1800" )
	PORT_DIPSETTING(    0x07, "1200" )
	PORT_DIPSETTING(    0x06, "600" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "150" )
	PORT_DIPSETTING(    0x03, "134.5" )
	PORT_DIPSETTING(    0x02, "110" )
	PORT_DIPSETTING(    0x01, "75" )
	PORT_DIPSETTING(    0x00, "50 (Loop on Power-Up)" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I8237
//-------------------------------------------------

void wangpc_state::update_fdc_tc()
{
	if (m_enable_eop)
		m_fdc->tc_w(m_fdc_tc);
	else
		m_fdc->tc_w(0);
}

void wangpc_state::hrq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	m_dmac->hack_w(state);
}

void wangpc_state::eop_w(int state)
{
	if (m_dack == 2)
	{
		m_fdc_tc = state;
		update_fdc_tc();
	}

	if (state)
	{
		if (LOG) logerror("EOP set\n");

		m_dma_eop = 0;
		check_level2_interrupts();
	}

	m_bus->tc_w(state);
}

uint8_t wangpc_state::memr_r(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_page[m_dack] << 16) | offset;

	return program.read_byte(addr);
}

void wangpc_state::memw_w(offs_t offset, uint8_t data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_page[m_dack] << 16) | offset;

	program.write_byte(addr, data);
}

uint8_t wangpc_state::ior2_r()
{
	if (m_disable_dreq2)
		return m_bus->dack_r(2);
	else
		return m_fdc->dma_r();
}

void wangpc_state::iow2_w(uint8_t data)
{
	if (m_disable_dreq2)
		m_bus->dack_w(2, data);
	else
		m_fdc->dma_w(data);
}

void wangpc_state::dack0_w(int state)
{
	if (!state) m_dack = 0;
}

void wangpc_state::dack1_w(int state)
{
	if (!state) m_dack = 1;
}

void wangpc_state::dack2_w(int state)
{
	if (!state) m_dack = 2;
}

void wangpc_state::dack3_w(int state)
{
	if (!state) m_dack = 3;
}

//-------------------------------------------------
//  pic8259_interface pic_intf
//-------------------------------------------------

void wangpc_state::check_level1_interrupts()
{
	int state = !m_timer2_irq || !m_epci->rxrdy_r() || !m_epci->txemt_dschg_r() || !m_centronics_ack || !m_dav || m_centronics_busy;

	m_pic->ir1_w(state);
}

void wangpc_state::check_level2_interrupts()
{
	int state = !m_dma_eop || m_uart_dr || m_uart_tbre || m_fdc_dd0 || m_fdc_dd1 || m_fdc->get_irq() || m_fpu_irq || m_bus_irq2;

	m_pic->ir2_w(state);
}

//-------------------------------------------------
//  I8255A INTERFACE
//-------------------------------------------------

uint8_t wangpc_state::ppi_pa_r()
{
	/*

	    bit     description

	    0       /POWER ON
	    1       /SMART
	    2       /DATA AVAILABLE
	    3       SLCT
	    4       BUSY
	    5       /FAULT
	    6       PE
	    7       ACKNOWLEDGE

	*/

	uint8_t data = 0x08 | 0x02 | 0x01;

	data |= m_dav << 2;
	data |= m_centronics_busy << 4;
	data |= m_centronics_fault << 5;
	data |= m_centronics_perror << 6;
	data |= m_centronics_ack << 7;

	return data;
}

uint8_t wangpc_state::ppi_pb_r()
{
	/*

	    bit     description

	    0       /TIMER 2 INTERRUPT
	    1       /SERIAL INTERRUPT
	    2       /PARALLEL PORT INTERRUPT
	    3       /DMA INTERRUPT
	    4       KBD INTERRUPT TRANSMIT
	    5       KBD INTERRUPT RECEIVE
	    6       FLOPPY DISK INTERRUPT
	    7       8087 INTERRUPT

	*/

	uint8_t data = 0;

	// timer 2 interrupt
	data |= m_timer2_irq;

	// serial interrupt
	data |= (m_epci->rxrdy_r() & m_epci->txemt_dschg_r()) << 1;

	// parallel port interrupt
	data |= m_centronics_ack << 2;

	// DMA interrupt
	data |= m_dma_eop << 3;

	// keyboard interrupt
	data |= m_uart_tbre << 4;
	data |= m_uart_dr << 5;

	// FDC interrupt
	data |= m_fdc->get_irq() << 6;

	// 8087 interrupt
	data |= m_fpu_irq << 7;

	return data;
}

uint8_t wangpc_state::ppi_pc_r()
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       SW1
	    5       SW2
	    6       SW3
	    7       SW4

	*/

	return m_sw->read() << 4;
}

void wangpc_state::ppi_pc_w(uint8_t data)
{
	/*

	    bit     description

	    0       /USR0 (pin 14)
	    1       /USR1 (pin 36)
	    2       /RESET (pin 31)
	    3       Unassigned
	    4
	    5
	    6
	    7

	*/

	m_centronics->write_autofd(BIT(data, 0));
	m_centronics->write_select_in(BIT(data, 1));
	m_centronics->write_init(BIT(data, 2));
}

void wangpc_state::pit0_w(int state)
{
	if (state)
		m_pic->ir0_w(1);
}

void wangpc_state::pit2_w(int state)
{
	if (state)
	{
		m_timer2_irq = 0;
		check_level1_interrupts();
	}
}

//-------------------------------------------------
//  IM6402_INTERFACE( uart_intf )
//-------------------------------------------------

void wangpc_state::uart_dr_w(int state)
{
	if (state)
	{
		if (LOG) logerror("DR set\n");

		m_uart_dr = 1;
		check_level2_interrupts();
	}
}

void wangpc_state::uart_tbre_w(int state)
{
	if (state)
	{
		if (LOG) logerror("TBRE set\n");

		m_uart_tbre = 1;
		check_level2_interrupts();
	}
}


//-------------------------------------------------
//  SCN2661_INTERFACE( epci_intf )
//-------------------------------------------------

void wangpc_state::epci_irq_w(int state)
{
	check_level1_interrupts();
}


//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

static void wangpc_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void wangpc_state::fdc_irq(int state)
{
	if (LOG) logerror("FDC INT %u\n", state);

	check_level2_interrupts();
}

void wangpc_state::fdc_drq(int state)
{
	if (LOG) logerror("FDC DRQ %u\n", state);

	m_fdc_drq = state;
	update_fdc_drq();
}

void wangpc_state::update_fdc_drq()
{
	if (m_disable_dreq2)
		m_dmac->dreq2_w(1);
	else
		m_dmac->dreq2_w(!m_fdc_drq);
}


//-------------------------------------------------
//  centronics_interface centronics_intf
//-------------------------------------------------

void wangpc_state::write_centronics_ack(int state)
{
	if (LOG) logerror("ACKNLG %u\n", state);

	m_centronics_ack = state;

	check_level1_interrupts();
}

void wangpc_state::write_centronics_busy(int state)
{
	if (LOG) logerror("BUSY %u\n", state);

	m_centronics_busy = state;

	check_level1_interrupts();
}

void wangpc_state::write_centronics_fault(int state)
{
	m_centronics_fault = state;
}

void wangpc_state::write_centronics_perror(int state)
{
	m_centronics_perror = state;
}

//-------------------------------------------------
//  WANGPC_BUS_INTERFACE( kb_intf )
//-------------------------------------------------

void wangpc_state::bus_irq2_w(int state)
{
	if (LOG) logerror("Bus IRQ2 %u\n", state);

	m_bus_irq2 = state;

	check_level2_interrupts();
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( wangpc )
//-------------------------------------------------

void wangpc_state::machine_start()
{
	// connect floppy callbacks
	m_floppy[0]->setup_load_cb(floppy_image_device::load_cb(&wangpc_state::on_disk0_load, this));
	m_floppy[0]->setup_unload_cb(floppy_image_device::unload_cb(&wangpc_state::on_disk0_unload, this));
	m_floppy[1]->setup_load_cb(floppy_image_device::load_cb(&wangpc_state::on_disk1_load, this));
	m_floppy[1]->setup_unload_cb(floppy_image_device::unload_cb(&wangpc_state::on_disk1_unload, this));

	m_led_diagnostic.resolve();

	// state saving
	save_item(NAME(m_dma_page));
	save_item(NAME(m_dack));
	save_item(NAME(m_timer2_irq));
	save_item(NAME(m_centronics_ack));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_fault));
	save_item(NAME(m_centronics_perror));
	save_item(NAME(m_dav));
	save_item(NAME(m_dma_eop));
	save_item(NAME(m_uart_dr));
	save_item(NAME(m_uart_tbre));
	save_item(NAME(m_fpu_irq));
	save_item(NAME(m_bus_irq2));
	save_item(NAME(m_enable_eop));
	save_item(NAME(m_disable_dreq2));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_ds1));
	save_item(NAME(m_ds2));
}


void wangpc_state::machine_reset()
{
	// initialize UART
	m_uart->cls1_w(1);
	m_uart->cls2_w(1);
	m_uart->pi_w(1);
	m_uart->sbs_w(1);
	m_uart->crl_w(1);
}


//-------------------------------------------------
//  on_disk0_change -
//-------------------------------------------------

void wangpc_state::on_disk0_load(floppy_image_device *image)
{
	on_disk0_unload(image);
}

void wangpc_state::on_disk0_unload(floppy_image_device *image)
{
	if (LOG) logerror("Door 1 disturbed\n");

	m_fdc_dd0 = 1;
	check_level2_interrupts();
}


//-------------------------------------------------
//  on_disk1_change -
//-------------------------------------------------

void wangpc_state::on_disk1_load(floppy_image_device *image)
{
	on_disk1_unload(image);
}

void wangpc_state::on_disk1_unload(floppy_image_device *image)
{
	if (LOG) logerror("Door 2 disturbed\n");

	m_fdc_dd1 = 1;
	check_level2_interrupts();
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( wangpc )
//-------------------------------------------------

void wangpc_state::wangpc(machine_config &config)
{
	I8086(config, m_maincpu, 24_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &wangpc_state::wangpc_mem);
	m_maincpu->set_addrmap(AS_IO, &wangpc_state::wangpc_io);
	m_maincpu->set_irq_acknowledge_callback(I8259A_TAG, FUNC(pic8259_device::inta_cb));
	//config.m_perfect_cpu_quantum = subtag(I8086_TAG);

	// devices
	AM9517A(config, m_dmac, 24_MHz_XTAL / 6);
	m_dmac->dreq_active_low();
	m_dmac->out_hreq_callback().set(FUNC(wangpc_state::hrq_w));
	m_dmac->out_eop_callback().set(FUNC(wangpc_state::eop_w));
	m_dmac->in_memr_callback().set(FUNC(wangpc_state::memr_r));
	m_dmac->out_memw_callback().set(FUNC(wangpc_state::memw_w));
	m_dmac->in_ior_callback<1>().set(m_bus, FUNC(wangpcbus_device::dack1_r));
	m_dmac->in_ior_callback<2>().set(FUNC(wangpc_state::ior2_r));
	m_dmac->in_ior_callback<3>().set(m_bus, FUNC(wangpcbus_device::dack3_r));
	m_dmac->out_iow_callback<1>().set(m_bus, FUNC(wangpcbus_device::dack1_w));
	m_dmac->out_iow_callback<2>().set(FUNC(wangpc_state::iow2_w));
	m_dmac->out_iow_callback<3>().set(m_bus, FUNC(wangpcbus_device::dack3_w));
	m_dmac->out_dack_callback<0>().set(FUNC(wangpc_state::dack0_w));
	m_dmac->out_dack_callback<1>().set(FUNC(wangpc_state::dack1_w));
	m_dmac->out_dack_callback<2>().set(FUNC(wangpc_state::dack2_w));
	m_dmac->out_dack_callback<3>().set(FUNC(wangpc_state::dack3_w));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	I8255A(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(wangpc_state::ppi_pa_r));
	m_ppi->in_pb_callback().set(FUNC(wangpc_state::ppi_pb_r));
	m_ppi->in_pc_callback().set(FUNC(wangpc_state::ppi_pc_r));
	m_ppi->out_pc_callback().set(FUNC(wangpc_state::ppi_pc_w));

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(24_MHz_XTAL / 48);
	m_pit->out_handler<0>().set(FUNC(wangpc_state::pit0_w));
	m_pit->set_clk<1>(24_MHz_XTAL / 12);
	m_pit->set_clk<2>(24_MHz_XTAL / 48);
	m_pit->out_handler<2>().set(FUNC(wangpc_state::pit2_w));

	constexpr XTAL clk1mhz = 24_MHz_XTAL / 24;
	IM6402(config, m_uart, clk1mhz.value(), clk1mhz.value());
	m_uart->tro_callback().set("wangpckb", FUNC(wangpc_keyboard_device::write_rxd));
	m_uart->dr_callback().set(FUNC(wangpc_state::uart_dr_w));
	m_uart->tbre_callback().set(FUNC(wangpc_state::uart_tbre_w));

	SCN2661C(config, m_epci, 5.0688_MHz_XTAL);
	m_epci->txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	m_epci->rxrdy_handler().set(FUNC(wangpc_state::epci_irq_w));
	m_epci->rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));
	m_epci->dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	m_epci->txemt_dschg_handler().set(FUNC(wangpc_state::epci_irq_w));

	UPD765A(config, m_fdc, 24_MHz_XTAL / 6, false, false);
	m_fdc->intrq_wr_callback().set(FUNC(wangpc_state::fdc_irq));
	m_fdc->drq_wr_callback().set(FUNC(wangpc_state::fdc_drq));
	FLOPPY_CONNECTOR(config, UPD765_TAG ":0", wangpc_floppies, "525dd", floppy_image_device::default_pc_floppy_formats);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":1", wangpc_floppies, "525dd", floppy_image_device::default_pc_floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_data_input_buffer(m_cent_data_in);
	m_centronics->ack_handler().set(FUNC(wangpc_state::write_centronics_ack));
	m_centronics->busy_handler().set(FUNC(wangpc_state::write_centronics_busy));
	m_centronics->fault_handler().set(FUNC(wangpc_state::write_centronics_fault));
	m_centronics->perror_handler().set(FUNC(wangpc_state::write_centronics_perror));

	INPUT_BUFFER(config, m_cent_data_in);

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_epci, FUNC(scn_pci_device::rxd_w));
	rs232.cts_handler().set(m_epci, FUNC(scn_pci_device::cts_w));
	rs232.dsr_handler().set(m_epci, FUNC(scn_pci_device::dsr_w));
	rs232.dcd_handler().set(m_epci, FUNC(scn_pci_device::dcd_w));

	WANGPC_KEYBOARD(config, "wangpckb").txd_handler().set(m_uart, FUNC(im6402_device::rri_w));

	// bus
	WANGPC_BUS(config, m_bus, 24_MHz_XTAL / 3);
	m_bus->irq2_wr_callback().set(FUNC(wangpc_state::bus_irq2_w));
	m_bus->irq3_wr_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	m_bus->irq4_wr_callback().set(m_pic, FUNC(pic8259_device::ir4_w));
	m_bus->irq5_wr_callback().set(m_pic, FUNC(pic8259_device::ir5_w));
	m_bus->irq6_wr_callback().set(m_pic, FUNC(pic8259_device::ir6_w));
	m_bus->irq7_wr_callback().set(m_pic, FUNC(pic8259_device::ir7_w));
	m_bus->drq1_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq1_w));
	m_bus->drq2_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq2_w));
	m_bus->drq3_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq3_w));
	m_bus->ioerror_wr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	WANGPC_BUS_SLOT(config, "slot1", m_bus, wangpc_cards, nullptr, 1);
	WANGPC_BUS_SLOT(config, "slot2", m_bus, wangpc_cards, "mvc", 2);
	WANGPC_BUS_SLOT(config, "slot3", m_bus, wangpc_cards, nullptr, 3);
	WANGPC_BUS_SLOT(config, "slot4", m_bus, wangpc_cards, nullptr, 4);
	WANGPC_BUS_SLOT(config, "slot5", m_bus, wangpc_cards, nullptr, 5);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("128K");

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("wangpc");
}



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

//-------------------------------------------------
//  ROM( wangpc )
//-------------------------------------------------

ROM_START( wangpc )
	ROM_REGION16_LE( 0x4000, I8086_TAG, 0)
	ROM_LOAD16_BYTE( "0001 r2.l94", 0x0001, 0x2000, CRC(f9f41304) SHA1(1815295809ef11573d724ede47446f9ac7aee713) )
	ROM_LOAD16_BYTE( "379-0000 r2.l115", 0x0000, 0x2000, CRC(67b37684) SHA1(70d9f68eb88cc2bc9f53f949cc77411c09a4266e) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

COMP( 1985, wangpc, 0, 0, wangpc, wangpc, wangpc_state, empty_init, "Wang Laboratories", "Wang Professional Computer", MACHINE_SUPPORTS_SAVE )
