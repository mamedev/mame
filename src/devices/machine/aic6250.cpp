// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of the Adaptec AIC-6250 SCSI Protocol Chip.
 *
 * Used in:
 *
 *   MIPS Rx2030
 *   Data General AViiON (AV100, 200, 300, 400 and 4300)
 *   Microbotics HardFrame (SCSI controller for Amiga 2000)
 *   Radio Shack 250-4161 (16 bit ISA hard/floppy controller card)
 *   pc532
 *
 * Sources:
 *
 *   http://bitsavers.org/pdf/adaptec/asic/AIC-6250_1988.pdf
 *
 * Status: very WIP, enough to load RISC/os on MIPS Rx2030 driver, but many
 * unimplemented and incorrect behaviours.
 *
 * TODO
 *   - fix problems with ATN
 *   - 16 bit DMA odd address start
 *   - disconnect/reselect
 *   - phase checks
 */

#include "emu.h"
#include "aic6250.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REG     (1U << 1)
#define LOG_STATE   (1U << 2)
#define LOG_CONFIG  (1U << 3)
#define LOG_INT     (1U << 4)
#define LOG_SCSI    (1U << 5)
#define LOG_DMA     (1U << 6)

//#define VERBOSE (LOG_GENERAL|LOG_REG|LOG_STATE|LOG_CONFIG|LOG_INT|LOG_SCSI|LOG_DMA)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(AIC6250, aic6250_device, "aic6250", "Adaptec 6250 High-Performance SCSI Protocol Chip")

static char const *const nscsi_phase[] = { "DATA OUT", "DATA IN", "COMMAND", "STATUS", "*", "*", "MESSAGE OUT", "MESSAGE IN" };
static char const *const aic6250_phase[] = { "DATA OUT", "*", "DATA IN", "*", "COMMAND", "MESSAGE OUT", "STATUS", "MESSAGE IN" };

aic6250_device::aic6250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nscsi_device(mconfig, AIC6250, tag, owner, clock)
	, m_int_cb(*this)
	, m_breq_cb(*this)
	, m_port_a_r_cb(*this)
	, m_port_a_w_cb(*this)
	, m_port_b_r_cb(*this)
	, m_port_b_w_cb(*this)
{
}

void aic6250_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(aic6250_device::dma_count_l_r), FUNC(aic6250_device::dma_count_l_w));
	map(0x1, 0x1).rw(FUNC(aic6250_device::dma_count_m_r), FUNC(aic6250_device::dma_count_m_w));
	map(0x2, 0x2).rw(FUNC(aic6250_device::dma_count_h_r), FUNC(aic6250_device::dma_count_h_w));
	map(0x3, 0x3).w(FUNC(aic6250_device::int_msk_reg_0_w));
	map(0x4, 0x4).w(FUNC(aic6250_device::offset_cntrl_w));
	map(0x5, 0x5).rw(FUNC(aic6250_device::fifo_status_r), FUNC(aic6250_device::dma_cntrl_w));
	map(0x6, 0x6).rw(FUNC(aic6250_device::rev_cntrl_r), FUNC(aic6250_device::int_msk_reg_1_w));
	map(0x7, 0x7).rw(FUNC(aic6250_device::status_reg_0_r), FUNC(aic6250_device::control_reg_0_w));
	map(0x8, 0x8).rw(FUNC(aic6250_device::status_reg_1_r), FUNC(aic6250_device::control_reg_1_w));
	map(0x9, 0x9).rw(FUNC(aic6250_device::scsi_signal_reg_r), FUNC(aic6250_device::scsi_signal_reg_w));
	map(0xa, 0xa).rw(FUNC(aic6250_device::scsi_id_data_r), FUNC(aic6250_device::scsi_id_data_w));
	map(0xb, 0xb).r(FUNC(aic6250_device::source_dest_id_r));
	map(0xc, 0xc).rw(FUNC(aic6250_device::memory_data_r), FUNC(aic6250_device::memory_data_w));
	map(0xd, 0xd).rw(FUNC(aic6250_device::port_a_r), FUNC(aic6250_device::port_a_w));
	map(0xe, 0xe).rw(FUNC(aic6250_device::port_b_r), FUNC(aic6250_device::port_b_w));
	map(0xf, 0xf).rw(FUNC(aic6250_device::scsi_latch_data_r), FUNC(aic6250_device::scsi_bsy_rst_w));
}

READ8_MEMBER(aic6250_device::read)
{
	u8 data = space.unmap();

	if (offset)
	{
		switch (m_address_reg)
		{
		case 0x0: data = dma_count_l_r(); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x1: data = dma_count_m_r(); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x2: data = dma_count_h_r(); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x3: if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x4: if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x5: data = fifo_status_r(); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x6: data = rev_cntrl_r(); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x7: data = status_reg_0_r(); if (!machine().side_effects_disabled()) m_address_reg++; break;

		case 0x8: data = status_reg_1_r(); break;
		case 0x9: data = scsi_signal_reg_r(); break;
		case 0xa: data = scsi_id_data_r(); break;
		case 0xb: data = source_dest_id_r(); break;
		case 0xc: data = memory_data_r(); break;
		case 0xd: data = port_a_r(); break;
		case 0xe: data = port_b_r(); break;
		case 0xf: data = scsi_latch_data_r(); break;
		}
	}
	else
		// FIXME: not sure if possible to read address register
		data = m_address_reg;

	return data;
}

WRITE8_MEMBER(aic6250_device::write)
{
	if (offset)
	{
		switch (m_address_reg)
		{
		case 0x0: dma_count_l_w(data); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x1: dma_count_m_w(data); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x2: dma_count_h_w(data); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x3: int_msk_reg_0_w(data); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x4: offset_cntrl_w(data); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x5: dma_cntrl_w(data); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x6: int_msk_reg_1_w(data); if (!machine().side_effects_disabled()) m_address_reg++; break;
		case 0x7: control_reg_0_w(data); if (!machine().side_effects_disabled()) m_address_reg++; break;

		case 0x8: control_reg_1_w(data); break;
		case 0x9: scsi_signal_reg_w(data); break;
		case 0xa: scsi_id_data_w(data); break;
		case 0xb: break;
		case 0xc: memory_data_w(data); break;
		case 0xd: port_a_w(data); break;
		case 0xe: port_b_w(data); break;
		case 0xf: scsi_bsy_rst_w(data); break;
		}
	}
	else
		m_address_reg = data & 0xf;
}

void aic6250_device::device_start()
{
	m_int_cb.resolve_safe();
	m_breq_cb.resolve_safe();

	m_port_a_r_cb.resolve_safe(0xff);
	m_port_a_w_cb.resolve_safe();
	m_port_b_r_cb.resolve_safe(0xff);
	m_port_b_w_cb.resolve_safe();

	save_item(NAME(m_dma_count));
	save_item(NAME(m_int_msk_reg_0));
	save_item(NAME(m_offset_cntrl));
	save_item(NAME(m_dma_cntrl));
	save_item(NAME(m_rev_cntrl));
	save_item(NAME(m_int_msk_reg_1));
	save_item(NAME(m_status_reg_0));
	save_item(NAME(m_control_reg_0));
	save_item(NAME(m_status_reg_1));
	save_item(NAME(m_control_reg_1));
	save_item(NAME(m_scsi_signal_reg));
	save_item(NAME(m_scsi_id_data));
	save_item(NAME(m_source_dest_id));
	save_item(NAME(m_memory_data));
	save_item(NAME(m_port_a_latch));
	save_item(NAME(m_port_b_latch));
	save_item(NAME(m_scsi_latch_data));

	m_rev_cntrl = 0x02;

	m_state_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(aic6250_device::state_loop), this));
	m_state = IDLE;

	m_int_asserted = false;

	// FIXME: for now, let's just look at everything
	scsi_bus->ctrl_wait(scsi_refid, S_ALL, S_ALL);
}

/*
 * Register Conditions On Reset
 *
 * The AIC-6250 essentially has two modes of reset; i.e., the Power On Reset
 * (/POR) and the SCSI Reset (SCSI /RST) which could be asserted through
 * software on the SCSI bus. Registers 00, 01, 02, 09, 0a, 0b, 0c and 0f are
 * not affected by any reset condition and their content is unknown on power
 * up. At other times it would depend on the activity which preceded the reset
 * action. Also, these registers are relevant only when some activity has been
 * initiated.
 *
 * Registers 04, 06, 0d and 0e are reset (to inactive zeroes) by the /POR only.
 *
 * Registers 05 and 09 are reset (to inactive zeros) by both the /POR or SCSI
 * /RST. On reset, the state of register 05(R) would be XX110000. The other
 * registers have multiple conditions as outlined below.
 *
 * Register 03 is reset to zero by the /POR; however, bit 6 (ARB/SEL Start)
 * would be reset by SCSI /RST also.
 *
 * Register 07(R) bits 0,1 are unaffected, while bits 2-7 are reset by /POR.
 * Bits 2,3,5 are also reset and bit 7 is set to 1 by the SCSI /RST condition.
 * Bit 4 will be set to 1 if EN BUS FREE DETECT INT (Reg 06, bit 2) is set to 1.
 *
 * Register 07(W), all bits except bit 6 are reset by /POR, while bits 3-5
 * would also be reset by SCSI /RST.
 *
 * Register 08(R), bits 0-5 are reset by /POR while bits 0-2 are reset by SCSI
 * /RST also. The state of bits 3-5 on reset will be 0. Bit 6 will be 1, bit 7
 * will normally be 1 as determined by /BACK * /BREQ.
 *
 * Register 08(W) bit 5 is a don't care. Bits 1-7 are reset by /POR. Bit 0 is
 * set to 1 while bit 7 is also reset by SCSI /RST.
 */
void aic6250_device::device_reset()
{
	// registers 04, 06, 0d, 0e
	m_offset_cntrl = 0;
	m_dma_cntrl = 0;
	m_port_a_latch = 0;
	m_port_a_latch = 0;

	// registers 05, 09
	m_offset_count_zero = true;
	m_fifo.clear();
	m_scsi_signal_reg = 0;

	// register 03
	m_int_msk_reg_0 = 0;

	// registers 07(R) and 07(W)
	m_status_reg_0 &= (R07R_SCSI_PHASE_CHG_ATN | R07R_DMA_BYTE_CNT_ZERO);
	m_control_reg_0 &= R07W_P_MEM_RW;

	// registers 08(R) and 08(W)
	m_status_reg_1 = R08R_RESERVED;
}

void aic6250_device::scsi_reset()
{
	// registers 04, 06, 0d, 0e
	m_offset_count_zero = true;
	m_fifo.clear();
	m_scsi_signal_reg = 0;

	// register 03
	m_int_msk_reg_0 &= ~R03W_ARB_SEL_START;

	// register 07(R)
	m_status_reg_0 &= ~(R07R_SCSI_REQ_ON | R07R_SCSI_PARITY_ERR | R07R_PHASE_MISMATCH_ERR);
	m_status_reg_0 |= R07R_SCSI_RST_OCCURRED;
	if (m_int_msk_reg_1 & R06W_EN_BUS_FREE_DETECT_INT)
		m_status_reg_0 |= R07R_BUS_FREE_DETECT;

	// register 07(W)
	m_control_reg_0 &= ~(R07W_SCSI_INTERFACE_MODE | R07W_EN_PORT_A_INP_OR_OUT | R07W_TARGET_MODE);

	// registers 08(R) and 08(W)
	m_status_reg_1 &= ~(R08R_SELECTED | R08R_RESELECTED | R08R_SEL_OUT);
	m_control_reg_1 &= ~R08W_AUTO_SCSI_PIO_REQ;
}

void aic6250_device::int_msk_reg_0_w(u8 data)
{
	LOGMASKED(LOG_REG, "int_msk_reg_0_w 0x%02x\n", data);

	/*
	 * Writing a zero to Bits 0 and 1 of this register will reset the selected
	 * or reselected interrupt status in Status Register 1 (Register 08),
	 * causing the interrupt status to be lost.
	 */
	if (!(data & R03W_EN_SELECT_INT))
		m_status_reg_1 &= ~R08R_SELECTED;

	if (!(data & R03W_EN_RESEL_INT))
		m_status_reg_1 &= ~R08R_RESELECTED;

	/*
	 * This bit will be reset to 0 when the Enable Command Done interrupt bit (Reg 03, Bit 3) is set to zero.
	 */
	if (!(data & R03W_EN_CMD_DONE_INT))
		m_status_reg_1 &= ~R08R_CMD_DONE;

	if ((m_int_msk_reg_0 ^ data) & R03W_ARB_SEL_START)
	{
		if (data & R03W_ARB_SEL_START)
		{
			if (m_state != IDLE)
				fatalerror("attempted to start selection while not idle\n");

			m_state = ARB_BUS_FREE;
			m_state_timer->adjust(attotime::zero);
		}
		else
		{
			if (m_state == IDLE)
				fatalerror("attempted to abort selection while idle\n");

			m_state = IDLE;
		}
	}

	m_int_msk_reg_0 = data;

	int_check();
}

void aic6250_device::offset_cntrl_w(u8 data)
{
	LOGMASKED(LOG_REG, "offset_cntrl_w 0x%02x\n", data);

	if (VERBOSE & LOG_CONFIG)
	{
		if (data & R04W_OFFSET)
		{
			double divisor = 4.0 + ((data & R04W_SYNC_XFER_RATE) >> 4);

			LOGMASKED(LOG_CONFIG, "synchronous offset %d speed %.3f\n",
				data & R04W_OFFSET, clock() / divisor);
		}
		else
			LOGMASKED(LOG_CONFIG, "asynchronous transfer mode\n");
	}

	m_offset_cntrl = data;
}

u8 aic6250_device::fifo_status_r()
{
	u8 const data =
		(m_offset_count_zero ? R05R_OFFSET_COUNT_ZERO : 0) |
		(m_fifo.empty() ? R05R_FIFO_EMPTY : 0) |
		(m_fifo.full() ? R05R_FIFO_FULL : 0) |
		(m_fifo.queue_length() & R05R_FIFO_COUNTER);

	LOGMASKED(LOG_REG, "fifo_status_r 0x%02x\n", data);

	return data;
}

void aic6250_device::dma_cntrl_w(u8 data)
{
	LOGMASKED(LOG_REG, "dma_cntrl_w 0x%02x\n", data);

	if (!(m_dma_cntrl & R05W_DMA_XFER_EN) && (data & R05W_DMA_XFER_EN))
	{
		LOGMASKED(LOG_DMA, "dma transfer %s memory, count %d\n",
			data & R05W_TRANSFER_DIR ? "from" : "to", m_dma_count);

		if (m_state != IDLE)
			fatalerror("attempt to start dma while not idle\n");

		// FIXME: should we trigger the state machine directly, or from the dma_w?
		if (data & R05W_TRANSFER_DIR)
		{
			m_state = DMA_OUT;
			m_breq_cb(1);
		}
		else
		{
			m_state = DMA_IN;
			m_state_timer->adjust(attotime::zero);
		}
	}

	m_dma_cntrl = data;
}

void aic6250_device::int_msk_reg_1_w(u8 data)
{
	LOGMASKED(LOG_REG, "int_msk_reg_1_w 0x%02x\n", data);

	if (!(data & R06W_EN_BUS_FREE_DETECT_INT))
		m_status_reg_0 &= ~R07R_BUS_FREE_DETECT;

	m_int_msk_reg_1 = data;

	int_check();
}

u8 aic6250_device::status_reg_0_r()
{
	u8 const dma_count_zero = !m_dma_count ? R07R_DMA_BYTE_CNT_ZERO : 0;

	return (m_status_reg_0 & ~R07R_DMA_BYTE_CNT_ZERO) | dma_count_zero;
}

void aic6250_device::control_reg_0_w(u8 data)
{
	LOGMASKED(LOG_REG, "control_reg_0_w 0x%02x\n", data);

	LOGMASKED(LOG_CONFIG, "scsi id %d, %s, port A %d, %s\n",
		data & R07W_SCSI_ID,
		data & R07W_SCSI_INTERFACE_MODE ? "differential" : "single-ended",
		data & R07W_EN_PORT_A_INP_OR_OUT ? "output" : "input",
		data & R07W_TARGET_MODE ? "target" : "initiator");

	if (data & R07W_P_MEM_CYCLE_REQ)
		logerror("processor memory %s request not supported\n", data & R07W_P_MEM_RW ? "write" : "read");

	m_control_reg_0 = data;
}

u8 aic6250_device::status_reg_1_r()
{
	if (m_status_reg_0 & R07R_ERROR_MASK)
		return m_status_reg_1 | R08R_ERROR;
	else
		return m_status_reg_1;
}

void aic6250_device::control_reg_1_w(u8 data)
{
	LOGMASKED(LOG_REG, "control_reg_1_w 0x%02x\n", data);

	if (data & R08W_CHIP_SW_RESET)
	{
		LOG("chip software reset\n");

		m_state = IDLE;

		scsi_reset();
		device_reset();

		m_control_reg_1 = R08W_CHIP_SW_RESET;

		set_int_state(false);
	}
	else
	{
		LOGMASKED(LOG_CONFIG, "%s frequency, port B input/output %s, %d-bit memory bus%s\n",
			data & R08W_CLK_FREQ_MODE ? "high" : "low",
			data & R08W_EN_PORT_B_INP_OR_OUT ? "enabled" : "disabled",
			data & R08W_EN_16_BIT_MEM_BUS ? 16 : 8,
			data & R08W_AUTO_SCSI_PIO_REQ ? ", automatic PIO" : "");

		if (!(m_control_reg_1 & R08W_AUTO_SCSI_PIO_REQ) && (data & R08W_AUTO_SCSI_PIO_REQ))
		{
			if (m_state != IDLE)
				fatalerror("attempted to start auto pio while not idle\n");

			m_state = AUTO_PIO;
			m_state_timer->adjust(attotime::zero);
		}

		m_control_reg_1 = data;
	}
}

u8 aic6250_device::scsi_signal_reg_r()
{
	u32 const ctrl = scsi_bus->ctrl_r();

	u8 const data =
		((ctrl & S_ACK) ? R09R_SCSI_ACK_IN : 0) |
		((ctrl & S_REQ) ? R09R_SCSI_REQ_IN : 0) |
		((ctrl & S_BSY) ? R09R_SCSI_BSY_IN : 0) |
		((ctrl & S_SEL) ? R09R_SCSI_SEL_IN : 0) |
		((ctrl & S_ATN) ? R09R_SCSI_ATN_IN : 0) |
		((ctrl & S_MSG) ? R09R_SCSI_MSG_IN : 0) |
		((ctrl & S_INP) ? R09R_SCSI_IO_IN : 0) |
		((ctrl & S_CTL) ? R09R_SCSI_CD_IN : 0);

	LOGMASKED(LOG_REG, "scsi_signal_reg_r 0x%02x\n", data);

	return data;
}

void aic6250_device::scsi_signal_reg_w(u8 data)
{
	LOGMASKED(LOG_REG, "scsi_signal_reg_w 0x%02x\n", data);

	if (m_control_reg_0 & R07W_TARGET_MODE)
		scsi_bus->ctrl_w(scsi_refid,
			((data & R09W_SCSI_REQ_OUT) ? S_REQ : 0) |
			((data & R09W_SCSI_BSY_OUT) ? S_BSY : 0) |
			((data & R09W_SCSI_SEL_OUT) ? S_SEL : 0) |
			((data & R09W_SCSI_MSG_OUT) ? S_MSG : 0) |
			((data & R09W_SCSI_IO_OUT) ? S_INP : 0) |
			((data & R09W_SCSI_CD_OUT) ? S_CTL : 0), S_REQ | S_BSY | S_SEL | S_MSG | S_INP | S_CTL);
	else
		scsi_bus->ctrl_w(scsi_refid,
			((data & R09W_SCSI_ACK_OUT) ? S_ACK : 0) |
			((data & R09W_SCSI_BSY_OUT) ? S_BSY : 0) |
			((data & R09W_SCSI_SEL_OUT) ? S_SEL : 0) |
			((data & R09W_SCSI_ATN_OUT) ? S_ATN : 0), S_ACK | S_BSY | S_SEL | S_ATN);

	if ((data ^ m_scsi_signal_reg) & R09R_PHASE_MASK)
		LOGMASKED(LOG_SCSI, "expecting phase %s\n", aic6250_phase[data >> 5]);

	if (!(m_control_reg_0 & R07W_TARGET_MODE) && phase_match(data, scsi_bus->ctrl_r()))
		m_status_reg_0 &= ~R07R_PHASE_MISMATCH_ERR;

	if (!(m_control_reg_0 & R07W_TARGET_MODE) && (data & R09W_SCSI_ACK_OUT))
		m_status_reg_0 &= ~R07R_SCSI_REQ_ON;

	m_scsi_signal_reg = data;

	// HACK: trigger check for phase match
	scsi_ctrl_changed();
}

u8 aic6250_device::scsi_id_data_r()
{
	// TODO: selection/reselection phase
	u8 const data = scsi_bus->data_r();

	LOGMASKED(LOG_REG, "scsi_id_data_r 0x%02x\n", data);

	return data;
}

void aic6250_device::scsi_id_data_w(u8 data)
{
	LOGMASKED(LOG_REG, "scsi_id_data_w 0x%02x\n", data);

	scsi_bus->data_w(scsi_refid, data);

	m_scsi_id_data = data;
}

u8 aic6250_device::memory_data_r()
{
	LOGMASKED(LOG_REG, "memory_data_r 0x%02x\n", m_memory_data);

	return m_memory_data;
}

void aic6250_device::memory_data_w(u8 data)
{
	LOGMASKED(LOG_REG, "memory_data_w 0x%02x\n", data);

	m_memory_data = data;
}

u8 aic6250_device::port_a_r()
{
	// FIXME: not sure if port A can be read in differential mode
	u8 const data = (m_control_reg_0 & R07W_SCSI_INTERFACE_MODE)
		|| (m_control_reg_0 & R07W_EN_PORT_A_INP_OR_OUT) ? m_port_a_latch : m_port_a_r_cb();

	LOGMASKED(LOG_REG, "port_a_r 0x%02x\n", data);

	return data;
}

void aic6250_device::port_a_w(u8 data)
{
	LOGMASKED(LOG_REG, "port_a_w 0x%02x\n", data);

	if (!(m_control_reg_0 & R07W_SCSI_INTERFACE_MODE) && (m_control_reg_0 & R07W_EN_PORT_A_INP_OR_OUT))
		m_port_a_w_cb(data);

	m_port_a_latch = data;
}

u8 aic6250_device::port_b_r()
{
	u8 const data = ((m_control_reg_1 & R08W_EN_16_BIT_MEM_BUS)
		|| (m_control_reg_1 & R08W_EN_PORT_B_INP_OR_OUT)) ? m_port_b_latch : m_port_b_r_cb();

	LOGMASKED(LOG_REG, "port_b_r 0x%02x\n", data);

	return data;
}

void aic6250_device::port_b_w(u8 data)
{
	LOGMASKED(LOG_REG, "port_b_w 0x%02x\n", data);

	if (!(m_control_reg_1 & R08W_EN_16_BIT_MEM_BUS) && (m_control_reg_1 & R08W_EN_PORT_B_INP_OR_OUT))
		m_port_b_w_cb(data);

	m_port_b_latch = data;
}

void aic6250_device::scsi_ctrl_changed()
{
	u32 const control = scsi_bus->ctrl_r();

	if ((control & S_BSY) && !(control & S_SEL))
		LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%08x phase %s%s%s\n", control, nscsi_phase[control & S_PHASE_MASK],
			control & S_REQ ? " REQ" : "", control & S_ACK ? " ACK" : "");
	else if (control & S_BSY)
		LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%08x arbitration/selection\n", control);
	else
		LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%08x BUS FREE\n", control);

	// phase change/atn
	if (!(m_control_reg_0 & R07W_TARGET_MODE))
	{
		if ((control ^ m_scsi_ctrl_state) & S_PHASE_MASK)
		{
			if ((m_control_reg_1 & R08W_PHASE_CHANGE_MODE) || (control & S_REQ))
			{
				LOGMASKED(LOG_SCSI, "bus phase change\n");

				m_status_reg_0 |= R07R_SCSI_PHASE_CHG_ATN;
			}

			if ((control & S_REQ) && !phase_match(m_scsi_signal_reg, control))
			{
				LOGMASKED(LOG_SCSI, "bus phase mismatch expect %s found %s\n", aic6250_phase[m_scsi_signal_reg >> 5], nscsi_phase[control & S_PHASE_MASK]);

				m_status_reg_0 |= R07R_PHASE_MISMATCH_ERR;
			}
		}
	}
	else
		if (!(m_scsi_ctrl_state & S_ATN) && (control & S_ATN))
		{
			LOGMASKED(LOG_SCSI, "bus atn asserted\n");

			m_status_reg_0 |= R07R_SCSI_PHASE_CHG_ATN;
		}

	// scsi req on
	if (!(m_control_reg_0 & R07W_TARGET_MODE) && !(m_scsi_ctrl_state & S_REQ) && (control & S_REQ))
	{
		LOGMASKED(LOG_SCSI, "bus req asserted\n");

		m_status_reg_0 |= R07R_SCSI_REQ_ON;
	}

	// bus free
	if ((m_scsi_ctrl_state & (S_SEL | S_BSY | S_RST)) && !(control & (S_SEL | S_BSY | S_RST)))
	{
		LOGMASKED(LOG_SCSI, "bus free\n");

		m_status_reg_0 |= R07R_BUS_FREE_DETECT;
	}

	if (!(m_scsi_ctrl_state & S_RST) && (control & S_RST))
	{
		LOGMASKED(LOG_SCSI, "bus reset asserted\n");

		m_status_reg_0 |= R07R_SCSI_RST_OCCURRED;

		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
	}

	// record new state
	m_scsi_ctrl_state = control;

	int_check();

	// TODO: in future, probably schedule scsi engine, not just interrupt checks
	//m_state_timer->adjust(attotime::zero);
}


TIMER_CALLBACK_MEMBER(aic6250_device::state_loop)
{
	// step state machine until delay, idle state or interrupt
	int delay = state_step();

	// check for interrupts
	bool const interrupt = int_check();

	if (delay < 0)
		return;

	/*
	 * All clock cycles are referred to assuming the high-frequency mode, set
	 * by the Clock Frequency mode bit in Control Register 1 (Bit 2, Register
	 * 08). If the low-frequency mode is being used, the number of clock cycles
	 * must be divided by two.
	 */
	if (!(m_control_reg_1 & R08W_CLK_FREQ_MODE))
		delay >>= 1;

	if (m_state != IDLE && !interrupt)
		m_state_timer->adjust(attotime::from_ticks(delay, clock()));
}

int aic6250_device::state_step()
{
	u8 const scsi_id = 1 << (m_control_reg_0 & R07W_SCSI_ID);
	int delay = 0;

	switch (m_state)
	{
	case IDLE:
		break;

	case ARB_BUS_FREE:
		LOGMASKED(LOG_STATE, "arbitration: waiting for bus free\n");
		if (!(scsi_bus->ctrl_r() & (S_SEL | S_BSY | S_RST)))
		{
			m_state = ARB_START;
			delay = 16; // 800ns
		}
		break;

	case ARB_START:
		LOGMASKED(LOG_STATE, "arbitration: started\n");
		m_state = ARB_EVALUATE;

		// drive our SCSI ID and assert BSY
		scsi_bus->data_w(scsi_refid, scsi_id);
		scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);

		delay = 56; // 2800ns
		break;

	case ARB_EVALUATE:
		// check if SEL asserted, or if there's a higher ID on the bus
		if ((scsi_bus->ctrl_r() & S_SEL) || (scsi_bus->data_r() & ~((scsi_id - 1) | scsi_id)))
		{
			LOGMASKED(LOG_STATE, "arbitration: lost\n");
			m_state = ARB_BUS_FREE;
			scsi_bus->ctrl_w(scsi_refid, 0, S_BSY);
			break;
		}

		LOGMASKED(LOG_STATE, "arbitration: won\n");
		m_state = SEL_START;
		delay = 24; // 1200ns
		break;

	case SEL_START:
		LOGMASKED(LOG_STATE, "selection: SEL asserted\n");

		m_status_reg_1 |= R08R_SEL_OUT;

		m_state = SEL_DELAY;
		delay = 2; // 100ns

		// drive both SCSI IDs and assert SEL
		scsi_bus->data_w(scsi_refid, m_scsi_id_data);
		scsi_bus->ctrl_w(scsi_refid, S_SEL, S_SEL);
		break;

	case SEL_DELAY:
		LOGMASKED(LOG_STATE, "selection: BSY cleared\n");

		m_state = SEL_WAIT_BSY;
		delay = 12; // 600ns

		// clear BSY, optionally assert ATN
		if (m_int_msk_reg_0 & R03W_EN_AUTO_ATN)
			scsi_bus->ctrl_w(scsi_refid, S_ATN, S_BSY | S_ATN);
		else
			scsi_bus->ctrl_w(scsi_refid, 0, S_BSY);
		break;

	case SEL_WAIT_BSY:
		if (scsi_bus->ctrl_r() & S_BSY)
		{
			LOGMASKED(LOG_STATE, "selection: BSY asserted by target\n");

			m_state = SEL_COMPLETE;
			delay = 1; // 50ns
		}
		else
			delay = -1;
		break;

	case SEL_COMPLETE:
		LOGMASKED(LOG_STATE, "selection: complete\n");

		m_int_msk_reg_0 &= ~R03W_ARB_SEL_START;
		m_status_reg_1 &= ~R08R_SEL_OUT;
		m_status_reg_1 |= R08R_CMD_DONE;
		m_source_dest_id = m_scsi_id_data;

		m_state = IDLE;

		// clear data and SEL
		// FIXME: should not clear ATN
		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_SEL | S_ATN);
		break;

	case DMA_IN:
		// FIXME: assert ack when: req asserted && phase match && count not zero && fifo not full
		if (!m_fifo.full())
		{
			u8 const data = scsi_bus->data_r();
			LOGMASKED(LOG_STATE, "dma in 0x%02x\n", data);

			m_status_reg_0 &= ~R07R_SCSI_REQ_ON;
			m_dma_count--;
			m_fifo.enqueue(data);

			m_state = DMA_IN_NEXT;

			scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
		}
		else
		{
			delay = -1;
			m_breq_cb(1);
		}
		break;

	case DMA_IN_NEXT:
		if (!(scsi_bus->ctrl_r() & S_REQ))
		{
			LOGMASKED(LOG_STATE, "dma in next count %d\n", m_dma_count);
			m_state = m_dma_count ? DMA_IN_REQ : DMA_IN_DRAIN;

			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		}
		break;

	case DMA_IN_REQ:
		if (scsi_bus->ctrl_r() & S_REQ)
			m_state = DMA_IN;
		break;

	case DMA_IN_DRAIN:
		if (!m_fifo.empty())
		{
			m_breq_cb(1);
			delay = -1;
		}
		else
			m_state = DMA_IN_DONE;
		break;

	case DMA_IN_DONE:
		LOGMASKED(LOG_STATE, "dma in done\n");
		m_status_reg_1 |= R08R_CMD_DONE;
		m_dma_cntrl &= ~R05W_DMA_XFER_EN;
		m_state = IDLE;
		break;

	case DMA_OUT:
		// FIXME: assert ack when: req asserted && phase match && count not zero && fifo not empty
		if (!m_fifo.empty())
		{
			u8 const data = m_fifo.dequeue();
			LOGMASKED(LOG_STATE, "dma out 0x%02x\n", data);

			m_status_reg_0 &= ~R07R_SCSI_REQ_ON;
			m_dma_count--;

			m_state = DMA_OUT_NEXT;

			// drive data, assert ACK
			scsi_bus->data_w(scsi_refid, data);
			scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
		}
		else
		{
			delay = -1;
			m_breq_cb(1);
		}
		break;

	case DMA_OUT_NEXT:
		if (!(scsi_bus->ctrl_r() & S_REQ))
		{
			LOGMASKED(LOG_STATE, "dma out next count %d\n", m_dma_count);
			m_state = m_dma_count ? DMA_OUT_REQ : DMA_OUT_DONE;

			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		}
		break;

	case DMA_OUT_REQ:
		if (scsi_bus->ctrl_r() & S_REQ)
			m_state = DMA_OUT;
		break;

	case DMA_OUT_DONE:
		LOGMASKED(LOG_STATE, "dma out done\n");
		m_status_reg_1 |= R08R_CMD_DONE;
		m_dma_cntrl &= ~R05W_DMA_XFER_EN;

		m_state = IDLE;
		break;

	case AUTO_PIO:
		// TODO: test expected phase
		// out: wait for req, check phase match, ack
		if (scsi_bus->ctrl_r() & S_REQ)
		{
			LOGMASKED(LOG_STATE, "auto pio\n");

			m_state = (m_dma_cntrl & R05W_TRANSFER_DIR) ? AUTO_PIO_OUT : AUTO_PIO_IN;
		}
		break;

	case AUTO_PIO_IN:
		m_state = AUTO_PIO_DONE;

		m_status_reg_0 &= ~R07R_SCSI_REQ_ON;
		m_scsi_latch_data = scsi_bus->data_r();

		LOGMASKED(LOG_STATE, "auto pio in 0x%02x\n", m_scsi_latch_data);
		scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
		break;

	case AUTO_PIO_OUT:
		LOGMASKED(LOG_STATE, "auto pio out 0x%02x\n", m_scsi_id_data);
		m_status_reg_0 &= ~R07R_SCSI_REQ_ON;

		m_state = AUTO_PIO_DONE;

		scsi_bus->data_w(scsi_refid, m_scsi_id_data);
		scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
		break;

	case AUTO_PIO_DONE:
		if (!(scsi_bus->ctrl_r() & S_REQ))
		{
			LOGMASKED(LOG_STATE, "auto pio done\n");

			m_status_reg_1 |= R08R_CMD_DONE;
			m_control_reg_1 &= ~R08W_AUTO_SCSI_PIO_REQ;
			m_state = IDLE;

			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		}
		break;
	}

	return delay;
}

bool aic6250_device::int_check()
{
	bool int_asserted = false;

	// status interrupts
	if (m_int_msk_reg_0 & m_status_reg_1 & R03W_INT_MASK)
		int_asserted = true;

	// error interrupts
	if ((m_int_msk_reg_0 & R03W_EN_ERROR_INT) && (m_int_msk_reg_1 & R06W_ERROR_INT_MASK))
	{
		// phase change/atn
		if ((m_int_msk_reg_1 & R06W_EN_PHASE_CHANGE_INT_INIT) && (m_status_reg_0 & R07R_SCSI_PHASE_CHG_ATN))
			int_asserted = true;

		// scsi parity
		if ((m_int_msk_reg_1 & R06W_EN_SCSI_PARITY_ERR_INT) && (m_status_reg_0 & R07R_SCSI_PARITY_ERR))
			int_asserted = true;

		// bus free
		if ((m_int_msk_reg_1 & R06W_EN_BUS_FREE_DETECT_INT) && (m_status_reg_0 & R07R_BUS_FREE_DETECT))
			int_asserted = true;

		// phase mismatch (initiator only)
		if ((m_int_msk_reg_1 & R06W_EN_PHASE_MISMATCH_INT) && (m_status_reg_0 & R07R_PHASE_MISMATCH_ERR))
			int_asserted = true;

		// memory parity
		if ((m_int_msk_reg_1 & R06W_EN_MEM_PARITY_ERROR_INT) && (m_status_reg_0 & R07R_MEMORY_PARITY_ERR))
			int_asserted = true;

		// scsi reset
		if ((m_int_msk_reg_1 & R06W_EN_SCSI_RST_INT) && (m_status_reg_0 & R07R_SCSI_RST_OCCURRED))
			int_asserted = true;

		// scsi req on (initiator only)
		if ((m_int_msk_reg_1 & R06W_EN_SCSI_REQ_ON_INT) && (m_status_reg_0 & R07R_SCSI_REQ_ON))
			int_asserted = true;
	}

	if (int_asserted)
		LOGMASKED(LOG_INT, "sr0 0x%02x sr1 0x%02x\n", m_status_reg_0, m_status_reg_1);

	// update int line state
	set_int_state(int_asserted);

	return int_asserted;
}

void aic6250_device::set_int_state(bool asserted)
{
	if (m_int_asserted != asserted)
	{
		LOGMASKED(LOG_INT, "set_int_state interrupt %s\n", asserted ? "asserted" : "cleared");

		m_int_asserted = asserted;

		// line is active low
		m_int_cb(asserted ? 0 : 1);
	}
}

/*
 * This implementation has a simplistic DMA approach. DMA transfers to memory
 * start with the SCSI interface filling up the FIFO. When it's full (or the
 * count exhausted), B̅R̅E̅Q̅ is asserted and cleared in a loop until the FIFO is
 * empty, after which the SCSI interface is scheduled again.
 *
 * Transfers from memory start with B̅R̅E̅Q̅ being asserted and cleared until the
 * FIFO is full, after which the SCSI interface is scheduled. If the SCSI
 * interface requires more data, B̅R̅E̅Q̅ is asserted and the cycle repeats. When
 * the DMA transfer count falls below 8, data is transferred via individual
 * cycles on demand rather than prefetched.
 */
WRITE_LINE_MEMBER(aic6250_device::back_w)
{
	LOGMASKED(LOG_DMA, "back_w %d\n", state);

	m_breq_cb(0);

	if (m_dma_cntrl & R05W_TRANSFER_DIR)
		if (m_fifo.full() || m_dma_count < 8)
			m_state_timer->adjust(attotime::zero);
		else
			m_breq_cb(1);
	else
		if (m_fifo.empty())
			m_state_timer->adjust(attotime::zero);
		else
			m_breq_cb(1);
}

u8 aic6250_device::dma_r()
{
	u8 const data = m_fifo.dequeue();

	LOGMASKED(LOG_DMA, "dma_r 0x%02x\n", data);

	return data;
}

u16 aic6250_device::dma16_r()
{
	u16 data = m_fifo.dequeue();

	data |= u16(m_fifo.dequeue()) << 8;

	LOGMASKED(LOG_DMA, "dma16_r 0x%04x\n", data);

	return data;
}

void aic6250_device::dma_w(u8 data)
{
	LOGMASKED(LOG_DMA, "dma_w 0x%02x\n", data);

	m_fifo.enqueue(data);
}

void aic6250_device::dma16_w(u16 data)
{
	LOGMASKED(LOG_DMA, "dma16_w 0x%04x\n", data);

	m_fifo.enqueue(data);
	m_fifo.enqueue(data >> 8);
}
