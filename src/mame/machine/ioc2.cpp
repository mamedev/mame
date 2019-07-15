// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI IOC2 I/O Controller emulation

**********************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "machine/input_merger.h"
#include "machine/ioc2.h"

#define LOG_PI1         (1 << 0)
#define LOG_SERIAL      (1 << 1)
#define LOG_MOUSEKBD    (1 << 2)
#define LOG_PANEL       (1 << 3)
#define LOG_SYSID       (1 << 4)
#define LOG_READ        (1 << 5)
#define LOG_DMA_SEL     (1 << 6)
#define LOG_RESET       (1 << 7)
#define LOG_WRITE       (1 << 8)
#define LOG_INT3        (1 << 9)
#define LOG_PIT         (1 << 10)
#define LOG_IRQS        (1 << 11)
#define LOG_ALL         (LOG_PI1 | LOG_SERIAL | LOG_MOUSEKBD | LOG_PANEL | LOG_SYSID | LOG_READ | LOG_DMA_SEL | LOG_RESET | LOG_WRITE | LOG_INT3 | LOG_PIT | LOG_IRQS)
#define LOG_DEFAULT     (LOG_ALL & ~(LOG_SYSID))

#define VERBOSE         (0)
#include "logmacro.h"

/*static*/ char const *const ioc2_device::SCC_TAG = "scc";
/*static*/ char const *const ioc2_device::PI1_TAG = "pi1";
/*static*/ char const *const ioc2_device::KBDC_TAG = "kbdc";
/*static*/ char const *const ioc2_device::PIT_TAG = "pit";
/*static*/ char const *const ioc2_device::RS232A_TAG = "rs232a";
/*static*/ char const *const ioc2_device::RS232B_TAG = "rs232b";

/*static*/ const XTAL ioc2_device::SCC_PCLK = 10_MHz_XTAL;
/*static*/ const XTAL ioc2_device::SCC_RXA_CLK = 3.6864_MHz_XTAL; // Needs verification
/*static*/ const XTAL ioc2_device::SCC_TXA_CLK = XTAL(0);
/*static*/ const XTAL ioc2_device::SCC_RXB_CLK = 3.6864_MHz_XTAL; // Needs verification
/*static*/ const XTAL ioc2_device::SCC_TXB_CLK = XTAL(0);

DEFINE_DEVICE_TYPE(SGI_IOC2_GUINNESS,   ioc2_guinness_device,   "ioc2g", "SGI IOC2 (Guinness)")
DEFINE_DEVICE_TYPE(SGI_IOC2_FULL_HOUSE, ioc2_full_house_device, "ioc2f", "SGI IOC2 (Full House)")

ioc2_guinness_device::ioc2_guinness_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ioc2_device(mconfig, SGI_IOC2_GUINNESS, tag, owner, clock)
{
}

ioc2_full_house_device::ioc2_full_house_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ioc2_device(mconfig, SGI_IOC2_FULL_HOUSE, tag, owner, clock)
{
}

static INPUT_PORTS_START( front_panel )
	PORT_START("panel_buttons")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Power")        PORT_CHANGED_MEMBER(DEVICE_SELF, ioc2_device, power_button, 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Volume Down")  PORT_CHANGED_MEMBER(DEVICE_SELF, ioc2_device, volume_down, 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Volume Up")    PORT_CHANGED_MEMBER(DEVICE_SELF, ioc2_device, volume_up, 0)
INPUT_PORTS_END

ioport_constructor ioc2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(front_panel);
}

void ioc2_device::device_add_mconfig(machine_config &config)
{
	SCC85230(config, m_scc, SCC_PCLK);
	m_scc->configure_channels(SCC_RXA_CLK.value(), SCC_TXA_CLK.value(), SCC_RXB_CLK.value(), SCC_TXB_CLK.value());
	m_scc->out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc->out_dtra_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsa_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));
	m_scc->out_dtrb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_rts));
	m_scc->out_int_callback().set(FUNC(ioc2_device::duart_int_w));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.cts_handler().set(m_scc, FUNC(scc85230_device::ctsa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(scc85230_device::dcda_w));
	rs232a.rxd_handler().set(m_scc, FUNC(scc85230_device::rxa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.cts_handler().set(m_scc, FUNC(scc85230_device::ctsb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(scc85230_device::dcdb_w));
	rs232b.rxd_handler().set(m_scc, FUNC(scc85230_device::rxb_w));

	PC_LPT(config, m_pi1);

	// keyboard connector
	pc_kbdc_device &kbd_con(PC_KBDC(config, "kbd_con", 0));
	kbd_con.out_clock_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::kbd_clk_w));
	kbd_con.out_data_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::kbd_data_w));

	// keyboard port
	pc_kbdc_slot_device &kbd(PC_KBDC_SLOT(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	kbd.set_pc_kbdc_slot(&kbd_con);

	// auxiliary connector
	pc_kbdc_device &aux_con(PC_KBDC(config, "aux_con", 0));
	aux_con.out_clock_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::aux_clk_w));
	aux_con.out_data_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::aux_data_w));

	// auxiliary port
	pc_kbdc_slot_device &aux(PC_KBDC_SLOT(config, "aux", ps2_mice, STR_HLE_PS2_MOUSE));
	aux.set_pc_kbdc_slot(&aux_con);

	// keyboard controller
	PS2_KEYBOARD_CONTROLLER(config, m_kbdc, 12_MHz_XTAL);
	m_kbdc->kbd_clk().set(kbd_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_kbdc->kbd_data().set(kbd_con, FUNC(pc_kbdc_device::data_write_from_mb));
	m_kbdc->aux_clk().set(aux_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_kbdc->aux_data().set(aux_con, FUNC(pc_kbdc_device::data_write_from_mb));

	input_merger_device &kbdc_irq(INPUT_MERGER_ANY_HIGH(config, "kbdc_irq"));
	kbdc_irq.output_handler().set(FUNC(ioc2_device::kbdc_int_w));
	m_kbdc->kbd_irq().set(kbdc_irq, FUNC(input_merger_device::in_w<0>));
	m_kbdc->aux_irq().set(kbdc_irq, FUNC(input_merger_device::in_w<1>));

	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(0);
	m_pit->set_clk<1>(0);
	m_pit->set_clk<2>(1000000);
	m_pit->out_handler<0>().set(FUNC(ioc2_device::timer0_int));
	m_pit->out_handler<1>().set(FUNC(ioc2_device::timer1_int));
	m_pit->out_handler<2>().set(FUNC(ioc2_device::pit_clock2_out));
}


ioc2_device::ioc2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_scc(*this, SCC_TAG)
	, m_pi1(*this, PI1_TAG)
	, m_kbdc(*this, KBDC_TAG)
	, m_pit(*this, PIT_TAG)
	, m_gen_ctrl_select_reg(0)
	, m_gen_ctrl_reg(0)
	, m_front_panel_reg(0)
	, m_read_reg(0)
	, m_dma_sel(0)
	, m_reset_reg(0)
	, m_write_reg(0)
	, m_int3_map_status_reg(0)
	, m_int3_map_pol_reg(0)
	, m_int3_err_status_reg(0)
	, m_par_read_cnt(0)
	, m_par_cntl(0)
{
}

void ioc2_device::device_start()
{
	save_item(NAME(m_par_read_cnt));
	save_item(NAME(m_par_cntl));

	save_item(NAME(m_gen_ctrl_select_reg));
	save_item(NAME(m_gen_ctrl_reg));
	save_item(NAME(m_front_panel_reg));

	save_item(NAME(m_read_reg));
	save_item(NAME(m_dma_sel));
	save_item(NAME(m_reset_reg));
	save_item(NAME(m_write_reg));

	save_item(NAME(m_int3_local_status_reg));
	save_item(NAME(m_int3_local_mask_reg));
	save_item(NAME(m_int3_map_status_reg));
	save_item(NAME(m_int3_map_mask_reg));
	save_item(NAME(m_int3_map_pol_reg));
	save_item(NAME(m_int3_err_status_reg));
}

void ioc2_device::device_reset()
{
	m_par_read_cnt = 0;
	m_par_cntl = 0;

	m_gen_ctrl_select_reg = 0;
	m_gen_ctrl_reg = 0;
	m_front_panel_reg = FRONT_PANEL_VOL_UP_HOLD | FRONT_PANEL_VOL_DOWN_HOLD | FRONT_PANEL_POWER_STATE;

	m_read_reg = 0;
	m_dma_sel = 0;
	m_reset_reg = 0;
	m_write_reg = 0;

	memset(m_int3_local_status_reg, 0, sizeof(uint8_t) * 2);
	memset(m_int3_local_mask_reg, 0, sizeof(uint8_t) * 2);
	m_int3_map_status_reg = 0;
	memset(m_int3_map_mask_reg, 0, sizeof(uint8_t) * 2);
	m_int3_map_pol_reg = 0;
	m_int3_err_status_reg = 0;
}

void ioc2_device::raise_local_irq(int channel, uint8_t mask)
{
	const uint8_t old = m_int3_local_status_reg[channel];
	m_int3_local_status_reg[channel] |= mask;
	LOGMASKED(LOG_IRQS, "Raising Local%d interrupt mask %02x, interrupt status was %02x, now %02x, %sing interrupt\n", channel, mask, old,
		m_int3_local_status_reg[channel], (m_int3_local_mask_reg[channel] & m_int3_local_status_reg[channel]) ? "rais" : "lower");
	m_maincpu->set_input_line(channel, (m_int3_local_mask_reg[channel] & m_int3_local_status_reg[channel]) ? ASSERT_LINE : CLEAR_LINE);
}

void ioc2_device::lower_local_irq(int channel, uint8_t mask)
{
	const uint8_t old = m_int3_local_status_reg[channel];
	m_int3_local_status_reg[channel] &= ~mask;
	LOGMASKED(LOG_IRQS, "Lowering Local%d interrupt mask %02x, interrupt status was %02x, now %02x, %sing interrupt\n", channel, mask, old,
		m_int3_local_status_reg[channel], (m_int3_local_mask_reg[channel] & m_int3_local_status_reg[channel]) ? "rais" : "lower");
	m_maincpu->set_input_line(channel, (m_int3_local_mask_reg[channel] & m_int3_local_status_reg[channel]) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(ioc2_device::timer0_int)
{
	if (state)
	{
		LOGMASKED(LOG_IRQS, "Raising IRQ line 2\n");
		m_maincpu->set_input_line(2, ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER(ioc2_device::timer1_int)
{
	if (state)
	{
		LOGMASKED(LOG_IRQS, "Raising IRQ line 3\n");
		m_maincpu->set_input_line(3, ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER(ioc2_device::pit_clock2_out)
{
	m_pit->write_clk0(state);
	m_pit->write_clk1(state);
}

WRITE_LINE_MEMBER(ioc2_device::kbdc_int_w)
{
	set_mappable_int(0x10, state);
}

WRITE_LINE_MEMBER(ioc2_device::duart_int_w)
{
	set_mappable_int(0x20, state);
}

void ioc2_device::set_mappable_int(uint8_t mask, bool state)
{
	const uint8_t old = m_int3_map_status_reg;
	const uint8_t old0 = m_int3_map_mask_reg[0] & old;
	const uint8_t old1 = m_int3_map_mask_reg[1] & old;

	if (state)
		m_int3_map_status_reg |= mask;
	else
		m_int3_map_status_reg &= ~mask;

	const uint8_t new0 = m_int3_map_mask_reg[0] & m_int3_map_status_reg;
	if (old0 ^ new0)
	{
		check_mappable_interrupt(0);
	}

	const uint8_t new1 = m_int3_map_mask_reg[1] & m_int3_map_status_reg;
	if (old1 ^ new1)
	{
		check_mappable_interrupt(1);
	}
}

void ioc2_device::check_mappable_interrupt(int channel)
{
	if (channel == 0)
	{
		if (m_int3_map_mask_reg[channel] & m_int3_map_status_reg)
			raise_local_irq(channel, INT3_LOCAL0_MAPPABLE0);
		else
			lower_local_irq(channel, INT3_LOCAL0_MAPPABLE0);
	}
	else
	{
		if (m_int3_map_mask_reg[channel] & m_int3_map_status_reg)
			raise_local_irq(channel, INT3_LOCAL1_MAPPABLE1);
		else
			lower_local_irq(channel, INT3_LOCAL1_MAPPABLE1);
	}
}

u8 ioc2_device::pi1_dma_ctrl_r()
{
	LOGMASKED(LOG_PI1, "%s: Read PI1 DMA Control Register: 00\n", machine().describe_context());
	return 0;
}

void ioc2_device::pi1_dma_ctrl_w(u8 data)
{
	LOGMASKED(LOG_PI1, "%s: Write PI1 DMA Control Register: %02x\n", machine().describe_context(), data);
}

u8 ioc2_device::pi1_int_status_r()
{
	LOGMASKED(LOG_PI1, "%s: Read PI1 Interrupt Status Register: 00\n", machine().describe_context());
	return 0;
}

void ioc2_device::pi1_int_status_w(u8 data)
{
	LOGMASKED(LOG_PI1, "%s: Write PI1 Interrupt Status Register: %02x\n", machine().describe_context(), data);
}

u8 ioc2_device::pi1_int_mask_r()
{
	LOGMASKED(LOG_PI1, "%s: Read PI1 Interrupt Mask Register: 00\n", machine().describe_context());
	return 0;
}

void ioc2_device::pi1_int_mask_w(u8 data)
{
	LOGMASKED(LOG_PI1, "%s: Write PI1 Interrupt Mask Register: %02x\n", machine().describe_context(), data);
}

u8 ioc2_device::pi1_timer1_r()
{
	LOGMASKED(LOG_PI1, "%s: Read PI1 Timer1 Register: 00\n", machine().describe_context());
	return 0;
}

u8 ioc2_device::pi1_timer2_r()
{
	LOGMASKED(LOG_PI1, "%s: Read PI1 Timer2 Register: 00\n", machine().describe_context());
	return 0;
}

u8 ioc2_device::pi1_timer3_r()
{
	LOGMASKED(LOG_PI1, "%s: Read PI1 Timer3 Register: 00\n", machine().describe_context());
	return 0;
}

u8 ioc2_device::pi1_timer4_r()
{
	LOGMASKED(LOG_PI1, "%s: Read PI1 Timer4 Register: 00\n", machine().describe_context());
	return 0;
}

void ioc2_device::pi1_timer1_w(u8 data)
{
	LOGMASKED(LOG_PI1, "%s: Write PI1 Timer1 Register: %02x\n", machine().describe_context(), data);
}

void ioc2_device::pi1_timer2_w(u8 data)
{
	LOGMASKED(LOG_PI1, "%s: Write PI1 Timer2 Register: %02x\n", machine().describe_context(), data);
}

void ioc2_device::pi1_timer3_w(u8 data)
{
	LOGMASKED(LOG_PI1, "%s: Write PI1 Timer3 Register: %02x\n", machine().describe_context(), data);
}

void ioc2_device::pi1_timer4_w(u8 data)
{
	LOGMASKED(LOG_PI1, "%s: Write PI1 Timer4 Register: %02x\n", machine().describe_context(), data);
}

u8 ioc2_device::gc_select_r()
{
	LOGMASKED(LOG_READ, "%s: Read GC Select Register: 00\n", machine().describe_context());
	return 0;
}

void ioc2_device::gc_select_w(u8 data)
{
	LOGMASKED(LOG_WRITE, "%s: Write GC Select Register: %02x\n", machine().describe_context(), data);
}

u8 ioc2_device::gen_cntl_r()
{
	LOGMASKED(LOG_READ, "%s: Read General Control Register: 00\n", machine().describe_context());
	return 0;
}

void ioc2_device::gen_cntl_w(u8 data)
{
	LOGMASKED(LOG_WRITE, "%s: Write General Control Register: %02x\n", machine().describe_context(), data);
}

u8 ioc2_device::front_panel_r()
{
	LOGMASKED(LOG_PANEL, "%s: Read Front Panel Register: %02x\n", machine().describe_context(), m_front_panel_reg);
	return m_front_panel_reg;
}

void ioc2_device::front_panel_w(u8 data)
{
	LOGMASKED(LOG_PANEL, "%s: Write Front Panel Register: %02x\n", machine().describe_context(), data);
	m_front_panel_reg &= ~(data & (FRONT_PANEL_VOL_UP_INT | FRONT_PANEL_VOL_DOWN_INT | FRONT_PANEL_POWER_BUTTON_INT));
	if (!(m_front_panel_reg & FRONT_PANEL_INT_MASK))
		lower_local_irq(1, INT3_LOCAL1_PANEL);
}

u8 ioc2_device::system_id_r()
{
	const uint8_t data = get_system_id();
	LOGMASKED(LOG_SYSID, "%s: Read System ID Register: %02x\n", machine().describe_context(), data);
	return data;
}

u8 ioc2_device::read_r()
{
	LOGMASKED(LOG_READ, "%s: Read Read Register: %02x\n", machine().describe_context(), m_read_reg);
	return m_read_reg;
}

u8 ioc2_device::dma_sel_r()
{
	// Bits 2-0 not quite understood, seem to be copy/paste error in SGI's own documents:
	//
	// 2    RW      Parallel Port DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [this makes sense. -ed.]
	// 1    RW      ISDN Channel B DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [is this a copy/paste error? perhaps "Parallel Port" should be "ISDN Channel B"?]
	// 0    RW      [same text as above. Another copy/paste error, maybe? Should be channel A, with the bit selecting DMA channel 0/1 for ISDN channel A, the and the same for ISDN channel B in bit 1?]
	LOGMASKED(LOG_DMA_SEL, "%s: Read DMA Select Register: %02x\n", machine().describe_context(), m_dma_sel);
	return m_dma_sel;
}

void ioc2_device::dma_sel_w(u8 data)
{
	// Bits 2-0 not quite understood, seem to be copy/paste error in SGI's own documents:
	//
	// 5:4  RW      Serial Port Clock Select: 00 selects a 10MHz internal clock (default), 01 selects a 6.67MHz internal clock, and 02 or 03 selects the external clock input.
	// 2    RW      Parallel Port DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [this makes sense. -ed.]
	// 1    RW      ISDN Channel B DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [is this a copy/paste error? perhaps "Parallel Port" should be "ISDN Channel B"?]
	// 0    RW      [same text as above. Another copy/paste error, maybe? Should be channel A, with the bit selecting DMA channel 0/1 for ISDN channel A, the and the same for ISDN channel B in bit 1?]
	LOGMASKED(LOG_DMA_SEL, "%s: Write DMA Select Register: %02x\n", machine().describe_context(), data);
	uint8_t old = m_dma_sel;
	m_dma_sel = data;
	uint8_t changed = old ^ m_dma_sel;
	if (changed & DMA_SEL_CLOCK_SEL_MASK)
	{
		if (changed & DMA_SEL_CLOCK_SEL_EXT)
		{
			LOGMASKED(LOG_DMA_SEL, "%s: External clock select %sselected\n", machine().describe_context(), (old & DMA_SEL_CLOCK_SEL_EXT) != 0 ? "de" : "");
			// TODO: verify the external Rx/Tx clock, is it fixed or programmable?
		}
	}
	// TODO: Currently we always assume a 10MHz clock as PCLK
}

u8 ioc2_device::reset_r()
{
	LOGMASKED(LOG_RESET, "%s: Read Reset Register: %02x\n", machine().describe_context(), m_reset_reg);
	return m_reset_reg;
}

void ioc2_device::reset_w(u8 data)
{
	LOGMASKED(LOG_RESET, "%s: Write Reset Register: %02x\n", machine().describe_context(), data);
	handle_reset_reg_write(data);
}

u8 ioc2_device::write_r()
{
	// Not yet implemented, some bits unnecessary:
	//
	// Bit  Oper    Description
	// 7    RW      Margin High. Set low for normal +5V operation, high to step supply up to +5.5V. Cleared at reset.
	// 6    RW      Margin Low. Set lowf or normal +5V operation, high to step supply down to +4.5V. Cleared at reset.
	// 5    RW      UART1 PC Mode. Set low to configure Port1 for RS422 Mac mode, high to select RS232 PC mode. Cleared at reset.
	// 4    RW      UART2 PC Mode. Set low to configure Port2 for RS422 Mac mode, high to select RS232 PC mode. Cleared at reset.
	// 3    RW      Ethernet Auto Select (active high). Set low for manual mode, high to have LXT901 automatically select TP or AUI based on link integrity. Cleared at reset.
	// 2    RW      Ethernet Port Select. Set low for TP, high for AUI. This setting is only used when Auto Select is in manual mode. Cleared at reset.
	// 1    RW      Ethernet UTP/STP select. Set low to select 150 ohm termination fro shielded TP (default), set high to select 100 ohm termination for unshielded TP. Cleared at reset.
	// 0    RW      Ethernet Normal Threshold (NTH) select. Set low to select the normal TP squelch threshold (default), high to reduce threshold by 4.5 dB (set low when reset).
	LOGMASKED(LOG_WRITE, "%s: Read Write Register: %02x\n", machine().describe_context(), m_write_reg);
	return m_write_reg;
}

void ioc2_device::write_w(u8 data)
{
	LOGMASKED(LOG_WRITE, "%s: Write Write Register: %02x\n", machine().describe_context(), data);
	m_write_reg = data;
}

template <int N>
u8 ioc2_device::local_status_r()
{
	LOGMASKED(LOG_INT3, "%s: Read Interrupt Local%d Status Register: %02x\n", machine().describe_context(), N, m_int3_local_status_reg[N]);
	return m_int3_local_status_reg[N];
}

template <int N>
u8 ioc2_device::local_mask_r()
{
	LOGMASKED(LOG_INT3, "%s: Read Interrupt Local%d Mask Register: %02x\n", machine().describe_context(), N, m_int3_local_mask_reg[N]);
	return m_int3_local_mask_reg[N];
}

template <int N>
void ioc2_device::local_mask_w(u8 data)
{
	LOGMASKED(LOG_INT3, "%s: Write Interrupt Local%d Mask Register: %02x\n", machine().describe_context(), N, data);
	set_local_int_mask(N, data);
}

u8 ioc2_device::map_status_r()
{
	LOGMASKED(LOG_INT3, "%s: Read Interrupt Map Status Register: %02x\n", machine().describe_context(), m_int3_map_status_reg);
	return m_int3_map_status_reg;
}

template <int N>
u8 ioc2_device::map_mask_r()
{
	LOGMASKED(LOG_INT3, "%s: Read Interrupt Map Mask%d Register: %02x\n", machine().describe_context(), N, m_int3_map_mask_reg[N]);
	return m_int3_map_mask_reg[N];
}

template <int N>
void ioc2_device::map_mask_w(u8 data)
{
	LOGMASKED(LOG_INT3, "%s: Write Interrupt Map Mask%d Register: %02x\n", machine().describe_context(), N, data);
	set_map_int_mask(N, data);
}

u8 ioc2_device::map_pol_r()
{
	LOGMASKED(LOG_INT3, "%s: Read Interrupt Map Polarity Register: %02x\n", machine().describe_context(), m_int3_map_pol_reg);
	return m_int3_map_pol_reg;
}

void ioc2_device::map_pol_w(u8 data)
{
	// TODO: Mappable interrupt polarity select
	LOGMASKED(LOG_INT3, "%s: Write Interrupt Map Polarity Register: %02x\n", machine().describe_context(), data);
	m_int3_map_pol_reg = data;
}

void ioc2_device::timer_int_clear_w(u8 data)
{
	LOGMASKED(LOG_INT3, "%s: Write Interrupt Timer Clear Register: %02x\n", machine().describe_context(), data);
	set_timer_int_clear(data);
}

u8 ioc2_device::error_status_r()
{
	LOGMASKED(LOG_INT3, "%s: Read Interrupt Error Status Register: %02x\n", machine().describe_context(), m_int3_err_status_reg);
	return m_int3_err_status_reg;
}

void ioc2_device::base_map(address_map &map)
{
	map(0x00, 0x02).rw(m_pi1, FUNC(pc_lpt_device::read), FUNC(pc_lpt_device::write));
	map(0x03, 0x03).rw(FUNC(ioc2_device::pi1_dma_ctrl_r), FUNC(ioc2_device::pi1_dma_ctrl_w));
	map(0x04, 0x04).rw(FUNC(ioc2_device::pi1_int_status_r), FUNC(ioc2_device::pi1_int_status_w));
	map(0x05, 0x05).rw(FUNC(ioc2_device::pi1_int_mask_r), FUNC(ioc2_device::pi1_int_mask_w));
	map(0x06, 0x06).rw(FUNC(ioc2_device::pi1_timer1_r), FUNC(ioc2_device::pi1_timer1_w));
	map(0x07, 0x07).rw(FUNC(ioc2_device::pi1_timer2_r), FUNC(ioc2_device::pi1_timer2_w));
	map(0x08, 0x08).rw(FUNC(ioc2_device::pi1_timer3_r), FUNC(ioc2_device::pi1_timer3_w));
	map(0x09, 0x09).rw(FUNC(ioc2_device::pi1_timer4_r), FUNC(ioc2_device::pi1_timer4_w));
	map(0x0c, 0x0f).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));
	map(0x10, 0x10).rw(m_kbdc, FUNC(ps2_keyboard_controller_device::data_r), FUNC(ps2_keyboard_controller_device::data_w));
	map(0x11, 0x11).rw(m_kbdc, FUNC(ps2_keyboard_controller_device::status_r), FUNC(ps2_keyboard_controller_device::command_w));
	map(0x12, 0x12).rw(FUNC(ioc2_device::gc_select_r), FUNC(ioc2_device::gc_select_w));
	map(0x13, 0x13).rw(FUNC(ioc2_device::gen_cntl_r), FUNC(ioc2_device::gen_cntl_w));
	map(0x14, 0x14).rw(FUNC(ioc2_device::front_panel_r), FUNC(ioc2_device::front_panel_w));
	map(0x16, 0x16).r(FUNC(ioc2_device::system_id_r));
	map(0x18, 0x18).r(FUNC(ioc2_device::read_r));
	map(0x1a, 0x1a).rw(FUNC(ioc2_device::dma_sel_r), FUNC(ioc2_device::dma_sel_w));
	map(0x1c, 0x1c).rw(FUNC(ioc2_device::reset_r), FUNC(ioc2_device::reset_w));
	map(0x1e, 0x1e).rw(FUNC(ioc2_device::write_r), FUNC(ioc2_device::write_w));
}

void ioc2_guinness_device::map(address_map &map)
{
	base_map(map);
	map(0x20, 0x20).r(FUNC(ioc2_guinness_device::local_status_r<0>));
	map(0x21, 0x21).rw(FUNC(ioc2_guinness_device::local_mask_r<0>), FUNC(ioc2_guinness_device::local_mask_w<0>));
	map(0x22, 0x22).r(FUNC(ioc2_guinness_device::local_status_r<1>));
	map(0x23, 0x23).rw(FUNC(ioc2_guinness_device::local_mask_r<1>), FUNC(ioc2_guinness_device::local_mask_w<1>));
	map(0x24, 0x24).r(FUNC(ioc2_guinness_device::map_status_r));
	map(0x25, 0x25).rw(FUNC(ioc2_guinness_device::map_mask_r<0>), FUNC(ioc2_guinness_device::map_mask_w<0>));
	map(0x26, 0x26).rw(FUNC(ioc2_guinness_device::map_mask_r<1>), FUNC(ioc2_guinness_device::map_mask_w<1>));
	map(0x27, 0x27).rw(FUNC(ioc2_guinness_device::map_pol_r), FUNC(ioc2_guinness_device::map_pol_w));
	map(0x28, 0x28).w(FUNC(ioc2_guinness_device::timer_int_clear_w));
	map(0x29, 0x29).r(FUNC(ioc2_guinness_device::error_status_r));
	map(0x2c, 0x2f).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
}

void ioc2_full_house_device::map(address_map &map)
{
	base_map(map);
}

void ioc2_full_house_device::int2_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(ioc2_full_house_device::local_status_r<0>));
	map(0x01, 0x01).rw(FUNC(ioc2_full_house_device::local_mask_r<0>), FUNC(ioc2_full_house_device::local_mask_w<0>));
	map(0x02, 0x02).r(FUNC(ioc2_full_house_device::local_status_r<1>));
	map(0x03, 0x03).rw(FUNC(ioc2_full_house_device::local_mask_r<1>), FUNC(ioc2_full_house_device::local_mask_w<1>));
	map(0x04, 0x04).r(FUNC(ioc2_full_house_device::map_status_r));
	map(0x05, 0x05).rw(FUNC(ioc2_full_house_device::map_mask_r<0>), FUNC(ioc2_full_house_device::map_mask_w<0>));
	map(0x06, 0x06).rw(FUNC(ioc2_full_house_device::map_mask_r<1>), FUNC(ioc2_full_house_device::map_mask_w<1>));
	map(0x08, 0x08).w(FUNC(ioc2_full_house_device::timer_int_clear_w));
	map(0x0c, 0x0f).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
}

void ioc2_device::set_local_int_mask(int channel, uint8_t mask)
{
	uint8_t old = m_int3_local_mask_reg[channel];
	m_int3_local_mask_reg[channel] = mask;
	bool old_line = (old & m_int3_local_status_reg[channel]) != 0;
	bool new_line = (m_int3_local_mask_reg[channel] & m_int3_local_status_reg[channel]) != 0;
	if (old_line != new_line)
	{
		const uint8_t int_bits = (m_int3_local_mask_reg[channel] & m_int3_local_status_reg[channel]);
		m_maincpu->set_input_line(channel, int_bits != 0 ? ASSERT_LINE : CLEAR_LINE);
	}
}

void ioc2_device::set_map_int_mask(int channel, uint8_t mask)
{
	m_int3_map_mask_reg[channel] = mask;
	check_mappable_interrupt(channel);
}

void ioc2_device::set_timer_int_clear(uint8_t data)
{
	if (BIT(data, 0))
	{
		LOGMASKED(LOG_IRQS, "Lowering IRQ line 2\n");
		m_maincpu->set_input_line(2, CLEAR_LINE);
	}
	if (BIT(data, 1))
	{
		LOGMASKED(LOG_IRQS, "Lowering IRQ line 3\n");
		m_maincpu->set_input_line(3, CLEAR_LINE);
	}
}

void ioc2_device::handle_reset_reg_write(uint8_t data)
{
	// guinness/fullhouse-specific implementations can handle bit 3 being used for ISDN reset on Indy only and bit 2 for EISA reset on Indigo 2 only, but for now we do nothing with it
	if (!BIT(data, 1))
	{
		//m_kbdc->reset();
	}
	m_reset_reg = 0;
}

INPUT_CHANGED_MEMBER( ioc2_device::power_button )
{
	if (!newval)
		m_front_panel_reg |= FRONT_PANEL_POWER_BUTTON_INT;
	else
		m_front_panel_reg &= ~FRONT_PANEL_POWER_BUTTON_INT;

	if (m_front_panel_reg & FRONT_PANEL_INT_MASK)
		raise_local_irq(1, INT3_LOCAL1_PANEL);
}

INPUT_CHANGED_MEMBER( ioc2_device::volume_up )
{
	if (!newval)
	{
		m_front_panel_reg |= FRONT_PANEL_VOL_UP_INT;
		m_front_panel_reg &= ~FRONT_PANEL_VOL_UP_HOLD;
	}
	else
	{
		m_front_panel_reg |= FRONT_PANEL_VOL_UP_HOLD;
	}

	if (m_front_panel_reg & FRONT_PANEL_INT_MASK)
		raise_local_irq(1, INT3_LOCAL1_PANEL);
}

INPUT_CHANGED_MEMBER( ioc2_device::volume_down )
{
	if (!newval)
	{
		m_front_panel_reg |= FRONT_PANEL_VOL_DOWN_INT;
		m_front_panel_reg &= ~FRONT_PANEL_VOL_DOWN_HOLD;
	}
	else
	{
		m_front_panel_reg |= FRONT_PANEL_VOL_DOWN_HOLD;
	}

	if (m_front_panel_reg & FRONT_PANEL_INT_MASK)
		raise_local_irq(1, INT3_LOCAL1_PANEL);
}

WRITE_LINE_MEMBER(ioc2_device::gio_int0_w)
{
	if (state == ASSERT_LINE)
		raise_local_irq(0, ioc2_device::INT3_LOCAL0_FIFO);
	else
		lower_local_irq(0, ioc2_device::INT3_LOCAL0_FIFO);
}

WRITE_LINE_MEMBER(ioc2_device::gio_int1_w)
{
	if (state == ASSERT_LINE)
		raise_local_irq(0, ioc2_device::INT3_LOCAL0_GRAPHICS);
	else
		lower_local_irq(0, ioc2_device::INT3_LOCAL0_GRAPHICS);
}

WRITE_LINE_MEMBER(ioc2_device::gio_int2_w)
{
	if (state == ASSERT_LINE)
		raise_local_irq(1, ioc2_device::INT3_LOCAL1_RETRACE);
	else
		lower_local_irq(1, ioc2_device::INT3_LOCAL1_RETRACE);
}

WRITE_LINE_MEMBER(ioc2_device::hpc_dma_done_w)
{
	if (state)
		raise_local_irq(1, ioc2_device::INT3_LOCAL1_HPC_DMA);
	else
		lower_local_irq(1, ioc2_device::INT3_LOCAL1_HPC_DMA);
}

WRITE_LINE_MEMBER(ioc2_device::mc_dma_done_w)
{
	if (state == ASSERT_LINE)
		raise_local_irq(0, ioc2_device::INT3_LOCAL0_MC_DMA);
	else
		lower_local_irq(0, ioc2_device::INT3_LOCAL0_MC_DMA);
}

WRITE_LINE_MEMBER(ioc2_device::scsi0_int_w)
{
	if (state)
		raise_local_irq(0, ioc2_device::INT3_LOCAL0_SCSI0);
	else
		lower_local_irq(0, ioc2_device::INT3_LOCAL0_SCSI0);
}

WRITE_LINE_MEMBER(ioc2_device::scsi1_int_w)
{
	if (state)
		raise_local_irq(0, ioc2_device::INT3_LOCAL0_SCSI1);
	else
		lower_local_irq(0, ioc2_device::INT3_LOCAL0_SCSI1);
}

WRITE_LINE_MEMBER(ioc2_device::enet_int_w)
{
	if (state)
		raise_local_irq(0, ioc2_device::INT3_LOCAL0_ETHERNET);
	else
		lower_local_irq(0, ioc2_device::INT3_LOCAL0_ETHERNET);
}

