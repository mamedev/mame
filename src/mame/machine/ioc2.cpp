// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI IOC2 I/O Controller emulation

**********************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
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

DEFINE_DEVICE_TYPE(SGI_IOC2_GUINNESS,   ioc2_guinness_device,   "ioc2g", "SGI IOC2 (Guiness)")
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

	KBDC8042(config, m_kbdc);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_PS2);
	m_kbdc->system_reset_callback().set_inputline(m_maincpu, INPUT_LINE_RESET);
	m_kbdc->input_buffer_full_callback().set(FUNC(ioc2_device::kbdc_int_w));

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
	m_kbdc->write_out2(state);
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

READ32_MEMBER(ioc2_device::read)
{
	switch (offset)
	{
		case PI1_DATA_REG:
		{
			const uint8_t data = m_pi1->read(space, offset, 0xff);;
			LOGMASKED(LOG_PI1, "%s: Read PI1 Data Register: %02x\n", machine().describe_context(), data);
			return data;
		}
		case PI1_CTRL_REG:
		{
			const uint8_t data = m_pi1->read(space, offset, 0xff);
			LOGMASKED(LOG_PI1, "%s: Read PI1 Control Register: %02x\n", machine().describe_context(), data);
			return data;
		}
		case PI1_STATUS_REG:
		{
			const uint8_t data = m_pi1->read(space, offset, 0xff);
			LOGMASKED(LOG_PI1, "%s: Read PI1 Status Register: %02x\n", machine().describe_context(), data);
			return data;
		}

		case PI1_DMA_CTRL_REG:
			LOGMASKED(LOG_PI1, "%s: Read PI1 DMA Control Register: 00\n", machine().describe_context());
			return 0;
		case PI1_INT_STATUS_REG:
			LOGMASKED(LOG_PI1, "%s: Read PI1 Interrupt Status Register: 00\n", machine().describe_context());
			return 0;
		case PI1_INT_MASK_REG:
			LOGMASKED(LOG_PI1, "%s: Read PI1 Interrupt Mask Register: 00\n", machine().describe_context());
			return 0;
		case PI1_TIMER1_REG:
			LOGMASKED(LOG_PI1, "%s: Read PI1 Timer1 Register: 00\n", machine().describe_context());
			return 0;
		case PI1_TIMER2_REG:
			LOGMASKED(LOG_PI1, "%s: Read PI1 Timer2 Register: 00\n", machine().describe_context());
			return 0;
		case PI1_TIMER3_REG:
			LOGMASKED(LOG_PI1, "%s: Read PI1 Timer3 Register: 00\n", machine().describe_context());
			return 0;
		case PI1_TIMER4_REG:
			LOGMASKED(LOG_PI1, "%s: Read PI1 Timer4 Register: 00\n", machine().describe_context());
			return 0;

		case SERIAL1_CMD_REG:
		{
			const uint8_t data = m_scc->ba_cd_r(space, (offset - SERIAL1_CMD_REG) ^ 3);
			LOGMASKED(LOG_SERIAL, "%s: Read Serial 1 Command Register: %02x\n", machine().describe_context(), data);
			return data;
		}
		case SERIAL1_DATA_REG:
		{
			const uint8_t data = m_scc->ba_cd_r(space, (offset - SERIAL1_CMD_REG) ^ 3);
			LOGMASKED(LOG_SERIAL, "%s: Read Serial 1 Data Register: %02x\n", machine().describe_context(), data);
			return data;
		}
		case SERIAL2_CMD_REG:
		{
			const uint8_t data = m_scc->ba_cd_r(space, (offset - SERIAL1_CMD_REG) ^ 3);
			LOGMASKED(LOG_SERIAL, "%s: Read Serial 2 Command Register: %02x\n", machine().describe_context(), data);
			return data;
		}
		case SERIAL2_DATA_REG:
		{
			const uint8_t data = m_scc->ba_cd_r(space, (offset - SERIAL1_CMD_REG) ^ 3);
			LOGMASKED(LOG_SERIAL, "%s: Read Serial 2 Data Register: %02x\n", machine().describe_context(), data);
			return data;
		}

		case KBD_MOUSE_REGS1:
		{
			const uint8_t data = m_kbdc->data_r(space, 0);
			LOGMASKED(LOG_MOUSEKBD, "%s: Read Keyboard/Mouse Register 1: %02x\n", machine().describe_context(), data);
			return data;
		}
		case KBD_MOUSE_REGS2:
		{
			const uint8_t data = m_kbdc->data_r(space, 4);
			LOGMASKED(LOG_MOUSEKBD, "%s: Read Keyboard/Mouse Register 2: %02x\n", machine().describe_context(), data);
			return data;
		}

		case PANEL_REG:
			LOGMASKED(LOG_PANEL, "%s: Read Front Panel Register: %02x\n", machine().describe_context(), m_front_panel_reg);
			return m_front_panel_reg;

		case SYSID_REG:
		{
			const uint8_t data = get_system_id();
			LOGMASKED(LOG_SYSID, "%s: Read System ID Register: %02x\n", machine().describe_context(), data);
			return data;
		}

		case READ_REG:
			LOGMASKED(LOG_READ, "%s: Read Read Register: %02x\n", machine().describe_context(), m_read_reg);
			return m_read_reg;

		case DMA_SEL_REG:
			// Bits 2-0 not quite understood, seem to be copy/paste error in SGI's own documents:
			//
			// 2    RW      Parallel Port DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [this makes sense. -ed.]
			// 1    RW      ISDN Channel B DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [is this a copy/paste error? perhaps "Parallel Port" should be "ISDN Channel B"?]
			// 0    RW      [same text as above. Another copy/paste error, maybe? Should be channel A, with the bit selecting DMA channel 0/1 for ISDN channel A, the and the same for ISDN channel B in bit 1?]
			LOGMASKED(LOG_DMA_SEL, "%s: Read DMA Select Register: %02x\n", machine().describe_context(), m_dma_sel);
			return m_dma_sel;

		case RESET_REG:
			LOGMASKED(LOG_RESET, "%s: Read Reset Register: %02x\n", machine().describe_context(), m_reset_reg);
			return m_reset_reg;

		case WRITE_REG:
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

		case INT3_LOCAL0_STATUS_REG:
			LOGMASKED(LOG_INT3, "%s: Read Interrupt Local0 Status Register: %02x\n", machine().describe_context(), m_int3_local_status_reg[0]);
			return m_int3_local_status_reg[0];

		case INT3_LOCAL0_MASK_REG:
			LOGMASKED(LOG_INT3, "%s: Read Interrupt Local0 Mask Register: %02x\n", machine().describe_context(), m_int3_local_mask_reg[0]);
			return m_int3_local_mask_reg[0];

		case INT3_LOCAL1_STATUS_REG:
			LOGMASKED(LOG_INT3, "%s: Read Interrupt Local1 Status Register: %02x\n", machine().describe_context(), m_int3_local_status_reg[1]);
			return m_int3_local_status_reg[1];

		case INT3_LOCAL1_MASK_REG:
			LOGMASKED(LOG_INT3, "%s: Read Interrupt Local1 Mask Register: %02x\n", machine().describe_context(), m_int3_local_mask_reg[1]);
			return m_int3_local_mask_reg[1];

		case INT3_MAP_STATUS_REG:
			LOGMASKED(LOG_INT3, "%s: Read Interrupt Map Status Register: %02x\n", machine().describe_context(), m_int3_map_status_reg);
			return m_int3_map_status_reg;

		case INT3_MAP_MASK0_REG:
			LOGMASKED(LOG_INT3, "%s: Read Interrupt Map Mask0 Register: %02x\n", machine().describe_context(), m_int3_map_mask_reg[0]);
			return m_int3_map_mask_reg[0];

		case INT3_MAP_MASK1_REG:
			LOGMASKED(LOG_INT3, "%s: Read Interrupt Map Mask1 Register: %02x\n", machine().describe_context(), m_int3_map_mask_reg[1]);
			return m_int3_map_mask_reg[1];

		case INT3_MAP_POLARITY_REG:
			LOGMASKED(LOG_INT3, "%s: Read Interrupt Map Polarity Register: %02x\n", machine().describe_context(), m_int3_map_pol_reg);
			return m_int3_map_pol_reg;

		case INT3_TIMER_CLEAR_REG:
			LOGMASKED(LOG_INT3, "%s: Read Interrupt Timer Clear (ignored)\n", machine().describe_context());
			return 0;

		case INT3_ERROR_STATUS_REG:
			LOGMASKED(LOG_INT3, "%s: Read Interrupt Error Status Register: %02x\n", machine().describe_context(), m_int3_err_status_reg);
			return m_int3_err_status_reg;

		case TIMER_COUNT0_REG:
		{
			const uint8_t data = m_pit->read(0);
			LOGMASKED(LOG_PIT, "%s: Read Timer Count0 Register: %02x\n", machine().describe_context(), data);
			return data;
		}
		case TIMER_COUNT1_REG:
		{
			const uint8_t data = m_pit->read(1);
			LOGMASKED(LOG_PIT, "%s: Read Timer Count1 Register: %02x\n", machine().describe_context(), data);
			return data;
		}
		case TIMER_COUNT2_REG:
		{
			const uint8_t data = m_pit->read(2);
			LOGMASKED(LOG_PIT, "%s: Read Timer Count2 Register: %02x\n", machine().describe_context(), data);
			return data;
		}
		case TIMER_CONTROL_REG:
		{
			const uint8_t data = m_pit->read(3);
			LOGMASKED(LOG_PIT, "%s: Read Timer Control Register: %02x\n", machine().describe_context(), data);
			return data;
		}
	}

	return 0;
}

WRITE32_MEMBER( ioc2_device::write )
{
	switch (offset)
	{
		case PI1_DATA_REG:
			LOGMASKED(LOG_PI1, "%s: Write PI1 Data Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_pi1->write(space, offset, (uint8_t)data);
			return;
		case PI1_CTRL_REG:
			LOGMASKED(LOG_PI1, "%s: Write PI1 Control Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_pi1->write(space, offset, (uint8_t)data);
			return;
		case PI1_STATUS_REG:
			LOGMASKED(LOG_PI1, "%s: Write PI1 Status Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_pi1->write(space, offset, (uint8_t)data);
			return;

		case PI1_DMA_CTRL_REG:
			LOGMASKED(LOG_PI1, "%s: Write PI1 DMA Control Register: %02x\n", machine().describe_context(), (uint8_t)data);
			return;
		case PI1_INT_STATUS_REG:
			LOGMASKED(LOG_PI1, "%s: Write PI1 Interrupt Status Register: %02x\n", machine().describe_context(), (uint8_t)data);
			return;
		case PI1_INT_MASK_REG:
			LOGMASKED(LOG_PI1, "%s: Write PI1 Interrupt Mask Register: %02x\n", machine().describe_context(), (uint8_t)data);
			return;
		case PI1_TIMER1_REG:
			LOGMASKED(LOG_PI1, "%s: Write PI1 Timer1 Register: %02x\n", machine().describe_context(), (uint8_t)data);
			return;
		case PI1_TIMER2_REG:
			LOGMASKED(LOG_PI1, "%s: Write PI1 Timer2 Register: %02x\n", machine().describe_context(), (uint8_t)data);
			return;
		case PI1_TIMER3_REG:
			LOGMASKED(LOG_PI1, "%s: Write PI1 Timer3 Register: %02x\n", machine().describe_context(), (uint8_t)data);
			return;
		case PI1_TIMER4_REG:
			LOGMASKED(LOG_PI1, "%s: Write PI1 Timer4 Register: %02x\n", machine().describe_context(), (uint8_t)data);
			return;

		case SERIAL1_CMD_REG:
			LOGMASKED(LOG_SERIAL, "%s: Write Serial 1 Command Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_scc->ba_cd_w(space, 3, data & 0xff);
			return;
		case SERIAL1_DATA_REG:
			LOGMASKED(LOG_SERIAL, "%s: Write Serial 1 Data Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_scc->ba_cd_w(space, 2, data & 0xff);
			return;
		case SERIAL2_CMD_REG:
			LOGMASKED(LOG_SERIAL, "%s: Write Serial 2 Command Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_scc->ba_cd_w(space, 1, data & 0xff);
			return;
		case SERIAL2_DATA_REG:
			LOGMASKED(LOG_SERIAL, "%s: Write Serial 2 Data Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_scc->ba_cd_w(space, 0, data & 0xff);
			return;

		case KBD_MOUSE_REGS1:
			LOGMASKED(LOG_MOUSEKBD, "%s: Write Keyboard/Mouse Register 1: %02x\n", machine().describe_context(), (uint8_t)data);
			m_kbdc->data_w(space, 0, data & 0xff);
			return;
		case KBD_MOUSE_REGS2:
			LOGMASKED(LOG_MOUSEKBD, "%s: Write Keyboard/Mouse Register 2: %02x\n", machine().describe_context(), (uint8_t)data);
			m_kbdc->data_w(space, 4, data & 0xff);
			return;

		case PANEL_REG:
			LOGMASKED(LOG_PANEL, "%s: Write Front Panel Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_front_panel_reg &= ~(data & (FRONT_PANEL_VOL_UP_INT | FRONT_PANEL_VOL_DOWN_INT | FRONT_PANEL_POWER_BUTTON_INT));
			if (!(m_front_panel_reg & FRONT_PANEL_INT_MASK))
				lower_local_irq(1, INT3_LOCAL1_PANEL);
			return;

		case DMA_SEL_REG:
		{
			// Bits 2-0 not quite understood, seem to be copy/paste error in SGI's own documents:
			//
			// 5:4  RW      Serial Port Clock Select: 00 selects a 10MHz internal clock (default), 01 selects a 6.67MHz internal clock, and 02 or 03 selects the external clock input.
			// 2    RW      Parallel Port DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [this makes sense. -ed.]
			// 1    RW      ISDN Channel B DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [is this a copy/paste error? perhaps "Parallel Port" should be "ISDN Channel B"?]
			// 0    RW      [same text as above. Another copy/paste error, maybe? Should be channel A, with the bit selecting DMA channel 0/1 for ISDN channel A, the and the same for ISDN channel B in bit 1?]
			LOGMASKED(LOG_DMA_SEL, "%s: Write DMA Select Register: %02x\n", machine().describe_context(), (uint8_t)data);
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
			return;
		}

		case RESET_REG:
			LOGMASKED(LOG_RESET, "%s: Write Reset Register: %02x\n", machine().describe_context(), (uint8_t)data);
			handle_reset_reg_write(data);
			return;

		case WRITE_REG:
			LOGMASKED(LOG_WRITE, "%s: Write Write Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_write_reg = data;
			return;

		case INT3_LOCAL0_STATUS_REG:
		case INT3_LOCAL1_STATUS_REG:
		case INT3_MAP_STATUS_REG:
		case INT3_ERROR_STATUS_REG:
			// Read-only registers
			return;

		case INT3_LOCAL0_MASK_REG:
			LOGMASKED(LOG_INT3, "%s: Write Interrupt Local0 Mask Register: %02x\n", machine().describe_context(), (uint8_t)data);
			set_local_int_mask(0, data);
			return;
		case INT3_LOCAL1_MASK_REG:
			LOGMASKED(LOG_INT3, "%s: Write Interrupt Local1 Mask Register: %02x\n", machine().describe_context(), (uint8_t)data);
			set_local_int_mask(1, data);
			return;

		case INT3_MAP_MASK0_REG:
			LOGMASKED(LOG_INT3, "%s: Write Interrupt Map Mask0 Register: %02x\n", machine().describe_context(), (uint8_t)data);
			set_map_int_mask(0, data);
			return;
		case INT3_MAP_MASK1_REG:
			LOGMASKED(LOG_INT3, "%s: Write Interrupt Map Mask1 Register: %02x\n", machine().describe_context(), (uint8_t)data);
			set_map_int_mask(1, data);
			return;
		case INT3_MAP_POLARITY_REG:
			// TODO: Mappable interrupt polarity select
			LOGMASKED(LOG_INT3, "%s: Write Interrupt Map Polarity Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_int3_map_pol_reg = data;
			return;
		case INT3_TIMER_CLEAR_REG:
			LOGMASKED(LOG_INT3, "%s: Write Interrupt Timer Clear Register: %02x\n", machine().describe_context(), (uint8_t)data);
			set_timer_int_clear(data);
			return;

		case TIMER_COUNT0_REG:
			LOGMASKED(LOG_PIT, "%s: Write Timer Count0 Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_pit->write(0, (uint8_t)data);
			return;
		case TIMER_COUNT1_REG:
			LOGMASKED(LOG_PIT, "%s: Write Timer Count1 Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_pit->write(1, (uint8_t)data);
			return;
		case TIMER_COUNT2_REG:
			LOGMASKED(LOG_PIT, "%s: Write Timer Count2 Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_pit->write(2, (uint8_t)data);
			return;
		case TIMER_CONTROL_REG:
			LOGMASKED(LOG_PIT, "%s: Write Timer Control Register: %02x\n", machine().describe_context(), (uint8_t)data);
			m_pit->write(3, (uint8_t)data);
			return;
	}
}

void ioc2_device::set_local_int_mask(int channel, uint32_t mask)
{
	uint8_t old = m_int3_local_mask_reg[channel];
	m_int3_local_mask_reg[channel] = (uint8_t)mask;
	bool old_line = (old & m_int3_local_status_reg[channel]) != 0;
	bool new_line = (m_int3_local_mask_reg[channel] & m_int3_local_status_reg[channel]) != 0;
	if (old_line != new_line)
	{
		const uint32_t int_bits = (m_int3_local_mask_reg[channel] & m_int3_local_status_reg[channel]);
		m_maincpu->set_input_line(channel, int_bits != 0 ? ASSERT_LINE : CLEAR_LINE);
	}
}

void ioc2_device::set_map_int_mask(int channel, uint32_t mask)
{
	m_int3_map_mask_reg[channel] = (uint8_t)mask;
	check_mappable_interrupt(channel);
}

void ioc2_device::set_timer_int_clear(uint32_t data)
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
	if (BIT(data, 1))
	{
		m_kbdc->reset();
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
