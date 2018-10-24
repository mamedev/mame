// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI IOC2 I/O Controller emulation

**********************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "machine/ioc2.h"

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

	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(1000000);
	m_pit->set_clk<1>(1000000);
	m_pit->set_clk<2>(1000000);
	m_pit->out_handler<2>().set(m_kbdc, FUNC(kbdc8042_device::write_out2));
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
	, m_int3_local0_status_reg(0)
	, m_int3_local0_mask_reg(0)
	, m_int3_local1_status_reg(0)
	, m_int3_local1_mask_reg(0)
	, m_int3_map_status_reg(0)
	, m_int3_map_mask0_reg(0)
	, m_int3_map_mask1_reg(0)
	, m_int3_map_pol_reg(0)
	, m_int3_timer_clear_reg(0)
	, m_int3_err_status_reg(0)
	, m_par_read_cnt(0)
	, m_par_cntl(0)
{
}

void ioc2_device::device_start()
{
	m_front_panel_reg = FRONT_PANEL_POWER_STATE;
}

void ioc2_device::device_reset()
{
	m_par_read_cnt = 0;
	m_par_cntl = 0;

	m_gen_ctrl_select_reg = 0;
	m_gen_ctrl_reg = 0;
	m_front_panel_reg = FRONT_PANEL_POWER_STATE;

	m_read_reg = 0;
	m_dma_sel = 0;
	m_reset_reg = 0;
	m_write_reg = 0;

	m_int3_local0_status_reg = 0;
	m_int3_local0_mask_reg = 0;
	m_int3_local1_status_reg = 0;
	m_int3_local1_mask_reg = 0;
	m_int3_map_status_reg = 0;
	m_int3_map_mask0_reg = 0;
	m_int3_map_mask1_reg = 0;
	m_int3_map_pol_reg = 0;
	m_int3_timer_clear_reg = 0;
	m_int3_err_status_reg = 0;
}

void ioc2_device::raise_local0_irq(uint8_t source_mask)
{
	m_int3_local0_status_reg |= source_mask;
	m_maincpu->set_input_line(MIPS3_IRQ0, (m_int3_local0_mask_reg & m_int3_local0_status_reg) != 0 ? ASSERT_LINE : CLEAR_LINE);
}

void ioc2_device::lower_local0_irq(uint8_t source_mask)
{
	m_int3_local0_status_reg &= ~source_mask;
}

void ioc2_device::raise_local1_irq(uint8_t source_mask)
{
	m_int3_local1_status_reg |= source_mask;
	m_maincpu->set_input_line(MIPS3_IRQ1, (m_int3_local1_mask_reg & m_int3_local1_status_reg) != 0 ? ASSERT_LINE : CLEAR_LINE);
}

void ioc2_device::lower_local1_irq(uint8_t source_mask)
{
	m_int3_local1_status_reg &= ~source_mask;
}

READ32_MEMBER( ioc2_device::read )
{
	switch (offset)
	{
		case PI1_DATA_REG:
		case PI1_CTRL_REG:
		case PI1_STATUS_REG:
			return m_pi1->read(space, offset, 0xff);

		case PI1_DMA_CTRL_REG:
		case PI1_INT_STATUS_REG:
		case PI1_INT_MASK_REG:
		case PI1_TIMER1_REG:
		case PI1_TIMER2_REG:
		case PI1_TIMER3_REG:
		case PI1_TIMER4_REG:
			return 0;

		case SERIAL1_CMD_REG:
		case SERIAL1_DATA_REG:
		case SERIAL2_CMD_REG:
		case SERIAL2_DATA_REG:
			return m_scc->ba_cd_r(space, (offset - SERIAL1_CMD_REG) ^ 3);

		case KBD_MOUSE_REGS1:
		case KBD_MOUSE_REGS2:
			return m_kbdc->data_r(space, (offset - KBD_MOUSE_REGS1) * 4);

		case PANEL_REG:
			return m_front_panel_reg;

		case SYSID_REG:
			return get_system_id();

		case READ_REG:
			return m_read_reg;

		case DMA_SEL_REG:
			// Bits 2-0 not quite understood, seem to be copy/paste error in SGI's own documents:
			//
			// 2    RW      Parallel Port DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [this makes sense. -ed.]
			// 1    RW      ISDN Channel B DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [is this a copy/paste error? perhaps "Parallel Port" should be "ISDN Channel B"?]
			// 0    RW      [same text as above. Another copy/paste error, maybe? Should be channel A, with the bit selecting DMA channel 0/1 for ISDN channel A, the and the same for ISDN channel B in bit 1?]
			return m_dma_sel;

		case RESET_REG:
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
			return m_write_reg;

		case INT3_LOCAL0_STATUS_REG:
			return m_int3_local0_status_reg;

		case INT3_LOCAL0_MASK_REG:
			return m_int3_local0_mask_reg;

		case INT3_LOCAL1_STATUS_REG:
			return m_int3_local1_status_reg;

		case INT3_LOCAL1_MASK_REG:
			return m_int3_local1_mask_reg;

		case INT3_MAP_STATUS_REG:
			return m_int3_map_status_reg;

		case INT3_MAP_MASK0_REG:
			return m_int3_map_mask0_reg;

		case INT3_MAP_MASK1_REG:
			return m_int3_map_mask1_reg;

		case INT3_MAP_POLARITY_REG:
			return m_int3_map_pol_reg;

		case INT3_TIMER_CLEAR_REG:
			return m_int3_timer_clear_reg;

		case INT3_ERROR_STATUS_REG:
			return m_int3_err_status_reg;

		case TIMER_COUNT0_REG:
		case TIMER_COUNT1_REG:
		case TIMER_COUNT2_REG:
		case TIMER_CONTROL_REG:
			return m_pit->read(offset - TIMER_COUNT0_REG);
	}

	return 0;
}

WRITE32_MEMBER( ioc2_device::write )
{
	switch (offset)
	{
		case PI1_DATA_REG:
		case PI1_CTRL_REG:
		case PI1_STATUS_REG:
			m_pi1->write(space, offset, data & 0xff, 0xff);
			return;

		case PI1_DMA_CTRL_REG:
		case PI1_INT_STATUS_REG:
		case PI1_INT_MASK_REG:
		case PI1_TIMER1_REG:
		case PI1_TIMER2_REG:
		case PI1_TIMER3_REG:
		case PI1_TIMER4_REG:
			return;

		case SERIAL1_CMD_REG:
		case SERIAL1_DATA_REG:
		case SERIAL2_CMD_REG:
		case SERIAL2_DATA_REG:
			m_scc->ba_cd_w(space, (offset - SERIAL1_CMD_REG) ^ 3, data & 0xff);
			return;

		case KBD_MOUSE_REGS1:
		case KBD_MOUSE_REGS2:
			m_kbdc->data_w(space, (offset - KBD_MOUSE_REGS1) * 4, data & 0xff);
			return;

		case PANEL_REG:
			m_front_panel_reg &= ~(data & (FRONT_PANEL_VOL_UP_INT | FRONT_PANEL_VOL_DOWN_INT | FRONT_PANEL_POWER_BUTTON_INT));
			return;

		case DMA_SEL_REG:
		{
			// Bits 2-0 not quite understood, seem to be copy/paste error in SGI's own documents:
			//
			// 5:4  RW      Serial Port Clock Select: 00 selects a 10MHz internal clock (default), 01 selects a 6.67MHz internal clock, and 02 or 03 selects the external clock input.
			// 2    RW      Parallel Port DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [this makes sense. -ed.]
			// 1    RW      ISDN Channel B DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [is this a copy/paste error? perhaps "Parallel Port" should be "ISDN Channel B"?]
			// 0    RW      [same text as above. Another copy/paste error, maybe? Should be channel A, with the bit selecting DMA channel 0/1 for ISDN channel A, the and the same for ISDN channel B in bit 1?]
			uint8_t old = m_dma_sel;
			m_dma_sel = data;
			uint8_t diff = old ^ m_dma_sel;
			if (diff & DMA_SEL_CLOCK_SEL_MASK)
			{
				if (diff & DMA_SEL_CLOCK_SEL_EXT)
				{
					logerror("%s: External clock select %sselected\n", machine().describe_context(), (old & DMA_SEL_CLOCK_SEL_EXT) != 0 ? "de" : "");
					// TODO: verify the external Rx/Tx clock, is it fixed or programmable?
				}
			}
			// TODO: Currently we always assume a 10MHz clock as PCLK
			return;
		}

		case RESET_REG:
			handle_reset_reg_write(data);
			return;

		case WRITE_REG:
			m_write_reg = data;
			return;

		case INT3_LOCAL0_STATUS_REG:
		case INT3_LOCAL1_STATUS_REG:
		case INT3_MAP_STATUS_REG:
		case INT3_ERROR_STATUS_REG:
			// Read-only registers
			return;

		case INT3_LOCAL0_MASK_REG:
		{
			uint8_t old = m_int3_local0_mask_reg;
			m_int3_local0_mask_reg = data;
			bool old_line = (old & m_int3_local0_status_reg) != 0;
			bool new_line = (m_int3_local0_mask_reg & m_int3_local0_status_reg) != 0;
			if (old_line != new_line)
			{
				const uint32_t int_bits = (m_int3_local1_mask_reg & m_int3_local1_status_reg) | (m_int3_local0_mask_reg & m_int3_local0_status_reg);
				m_maincpu->set_input_line(MIPS3_IRQ0, int_bits != 0 ? ASSERT_LINE : CLEAR_LINE);
			}
			return;
		}

		case INT3_LOCAL1_MASK_REG:
		{
			uint8_t old = m_int3_local1_mask_reg;
			m_int3_local1_mask_reg = data;
			bool old_line = (old & m_int3_local1_status_reg) != 0;
			bool new_line = (m_int3_local1_mask_reg & m_int3_local1_status_reg) != 0;
			if (old_line != new_line)
			{
				const uint32_t int_bits = (m_int3_local1_mask_reg & m_int3_local1_status_reg) | (m_int3_local0_mask_reg & m_int3_local0_status_reg);
				m_maincpu->set_input_line(MIPS3_IRQ0, int_bits != 0 ? ASSERT_LINE : CLEAR_LINE);
			}
			return;
		}

		case INT3_MAP_MASK0_REG:
			// TODO: Implement mappable interrupts
			m_int3_map_mask0_reg = data;
			return;

		case INT3_MAP_MASK1_REG:
			// TODO: Implement mappable interrupts
			m_int3_map_mask1_reg = data;
			return;

		case INT3_MAP_POLARITY_REG:
			// TODO: Mappable interrupt polarity select
			m_int3_map_pol_reg = data;
			return;

		case TIMER_COUNT0_REG:
		case TIMER_COUNT1_REG:
		case TIMER_COUNT2_REG:
		case TIMER_CONTROL_REG:
			m_pit->write(offset - TIMER_COUNT0_REG, data & 0xff);
			return;
	}
}

void ioc2_device::handle_reset_reg_write(uint8_t data)
{
	// guinness/fullhouse-specific implementations can handle bit 3 being used for ISDN reset on Indy only and bit 2 for EISA reset on Indigo 2 only, but for now we do nothing with it
	m_reset_reg = data;
}

INPUT_CHANGED_MEMBER( ioc2_device::power_button )
{
	if (!newval)
	{
		m_front_panel_reg |= FRONT_PANEL_POWER_BUTTON_INT;
	}
}

INPUT_CHANGED_MEMBER( ioc2_device::volume_up )
{
	if (!newval)
	{
		m_front_panel_reg |= FRONT_PANEL_VOL_UP_INT;
		m_front_panel_reg |= FRONT_PANEL_VOL_UP_HOLD;
	}
	else
	{
		m_front_panel_reg &= ~FRONT_PANEL_VOL_UP_HOLD;
	}
}

INPUT_CHANGED_MEMBER( ioc2_device::volume_down )
{
	if (!newval)
	{
		m_front_panel_reg |= FRONT_PANEL_VOL_DOWN_INT;
		m_front_panel_reg |= FRONT_PANEL_VOL_DOWN_HOLD;
	}
	else
	{
		m_front_panel_reg &= ~FRONT_PANEL_VOL_DOWN_HOLD;
	}
}
