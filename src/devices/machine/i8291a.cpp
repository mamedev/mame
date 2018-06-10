// license:BSD-3-Clause
// copyright-holders: Sven Schnelle
/*********************************************************************

 i8291a.cpp

 Intel 8291A GPIB controller

 *********************************************************************/

#include "emu.h"


#include "i8291a.h"

#include "logmacro.h"
#define LOG_STATE_MASK  (LOG_GENERAL << 1)
#define LOG_STATE(...)  LOGMASKED(LOG_STATE_MASK, __VA_ARGS__)
#define LOG_REG_MASK    (LOG_STATE_MASK << 1)
#define LOG_REG(...)    LOGMASKED(LOG_REG_MASK, __VA_ARGS__)
#define LOG_INT_MASK    (LOG_REG_MASK << 1)
#define LOG_INT(...)    LOGMASKED(LOG_INT_MASK, __VA_ARGS__)
#define LOG_CMD_MASK	(LOG_INT_MASK << 1)
#define LOG_CMD(...)	LOGMASKED(LOG_CMD_MASK, __VA_ARGS__)
#undef VERBOSE
#define VERBOSE (LOG_GENERAL)//|LOG_REG_MASK|LOG_CMD_MASK|LOG_STATE_MASK|LOG_STATE_MASK)

DEFINE_DEVICE_TYPE(I8291A, i8291a_device, "i8291a", "I8291A")

i8291a_device::i8291a_device(const machine_config &mconfig, const char *tag,
		device_t *owner, uint32_t clock) :
		device_t(mconfig, I8291A, tag, owner, clock),
		m_int_write_func(*this),
		m_dreq_write_func(*this),
		m_trig_write_func(*this),
		m_eoi_write_func{*this},
		m_dav_write_func{*this},
		m_nrfd_write_func{*this},
		m_ndac_write_func{*this},
		m_srq_write_func{*this},
		m_dio_write_func{*this},
		m_dio_read_func{*this}
				{
}

void i8291a_device::device_reset()
{
	m_pon = true;
	m_ints1 = 0;
	m_ints2 = 0;
	m_auxa = 0;
	m_auxb = 0;
	m_spoll_mode = 0;
	// TODO: clear parallel poll
	m_address_status &= ~REG_ADDRESS_STATUS_EOI;
	m_rdy = true;
	m_nba = false;
	m_send_eoi = false;
	m_ist = false;
	m_eoi = false;
	m_ifc = false;
	m_srq = false;
	m_dav = false;
	m_ndac = false;
	m_nrfd = false;
	m_atn = false;
	m_ren = false;
	update_state(m_t_state, TalkerState::TIDS);
	update_state(m_tp_state, TalkerPrimaryState::TPIS);
	update_state(m_tsp_state, TalkerSerialPollState::SPIS);
	update_state(m_l_state, ListenerState::LIDS);
	update_state(m_rl_state, RemoteLocalState::LOCS);
	update_state(m_pp_state, ParallelPollState::PPIS);
	update_state(m_dc_state, DeviceClearState::DCIS);
	update_state(m_dt_state, DeviceTriggerState::DTIS);
	update_state(m_sh_state, SourceHandshake::SIDS);
	update_state(m_ah_state, AcceptorHandshake::AIDS);
}

void i8291a_device::device_start()
{
	m_int_write_func.resolve_safe();
	m_dreq_write_func.resolve_safe();
	m_dav_write_func.resolve_safe();
	m_nrfd_write_func.resolve_safe();
	m_ndac_write_func.resolve_safe();
	m_eoi_write_func.resolve_safe();
	m_srq_write_func.resolve_safe();
	m_dio_read_func.resolve_safe(0xff);
	m_dio_write_func.resolve_safe();

}

READ8_MEMBER(i8291a_device::din_r)
{
	LOG_REG("%s: %02X\n", __FUNCTION__, m_din);
	m_din_flag = false;
	m_ints1 &= ~REG_INTS1_BI;
	update_int();
	run_fsm();
	return m_din;
}

void i8291a_device::update_int()
{
	bool dreq = ((m_ie2 & REG_IE2_DMAO) && (m_ints1 & REG_INTS1_BO)) |
			((m_ie2 & REG_IE2_DMAI)  && (m_ints1 & REG_INTS1_BI));

	if (m_dreq_out != dreq) {
		LOG_INT("%s: dreq %s\n", __FUNCTION__, dreq ? "true" : "false");
		m_dreq_write_func(dreq);
		m_dreq_out = dreq;
	}

	bool intr = false;

	if (m_ints1 & m_ie1)
		intr = true;

	if ((m_ints2 & m_ie2) & 0x0f)
		intr = true;

	if (intr) {
		m_ints2 |= REG_INTS2_INT;
		m_address0 |= REG_ADDRESS0_INT;
	} else {
		m_ints2 &= ~REG_INTS2_INT;
		m_address0 &= ~REG_ADDRESS0_INT;
	}

	if (m_intr_out != intr) {
		LOG_INT("%s: intr %s\n", __FUNCTION__, intr ? "true" : "false");
		m_int_write_func(intr);
		m_intr_out = intr;
	}



}

READ8_MEMBER(i8291a_device::ints1_r)
{
	uint8_t ret = m_ints1;
	LOG_REG("%s: %02X\n", __FUNCTION__, ret);
	m_ints1 = 0;
	update_int();
	return ret;
}

READ8_MEMBER(i8291a_device::ints2_r)
{
	uint8_t ret = m_ints2;
	LOG_REG("%s: %02X\n", __FUNCTION__, ret);
	m_ints2 = 0;
	update_int();
	return ret;
}

READ8_MEMBER(i8291a_device::spoll_stat_r)
{
	LOG_REG("%s\n", __FUNCTION__);
	return 0;
}

READ8_MEMBER(i8291a_device::addr_stat_r)
{
	LOG_REG("%s = %02X\n", __FUNCTION__, m_address_status);
	return m_address_status;
}

READ8_MEMBER(i8291a_device::cpt_r)
{
	LOG_REG("%s\n", __FUNCTION__);
	return m_cpt;
}

READ8_MEMBER(i8291a_device::addr0_r)
{
	//LOG_REG("%s = %02X\n", __FUNCTION__, m_address0);
	return m_address0;
}

READ8_MEMBER(i8291a_device::addr1_r)
{
	LOG_REG("%s = %02X\n", __FUNCTION__, m_address1);
	return m_address1;
}

WRITE8_MEMBER(i8291a_device::dout_w)
{
	LOG_REG("%s: %02X\n", __FUNCTION__, data);
	if (m_nba)
		return;
	m_dout = data;
	m_nba = true;
	run_fsm();
}

WRITE8_MEMBER(i8291a_device::ie1_w)
{
	LOG_REG("%s: %02X\n", __FUNCTION__, data);
	m_ie1 = data;
	run_fsm();
}

WRITE8_MEMBER(i8291a_device::ie2_w)
{
	LOG_REG("%s: %02X\n", __FUNCTION__, data);
	m_ie2 = data;
	run_fsm();
}

WRITE8_MEMBER(i8291a_device::spoll_mode_w)
{
	LOG_REG("%s: %02X\n", __FUNCTION__, data);
	m_spoll_mode = data;
	run_fsm();
}

WRITE8_MEMBER(i8291a_device::addr_mode_w)
{
	LOG_REG("%s: %02X\n", __FUNCTION__, data);
	m_address_mode = data & 3;
	run_fsm();
}

WRITE8_MEMBER(i8291a_device::aux_mode_w)
{
	LOG_REG("%s: %02X\n", __FUNCTION__, data);
	switch (data >> 5) {
	case 0:
		switch (data) {
		case AUXCMD_IMMEDIATE_EXEC_PON:
			LOG_REG("AUXCMD_IMMEDIATE_EXEC_PON\n");
			m_pon = 0;
			m_rdy = 1;
			break;

		case AUXCMD_CLEAR_PP:
			LOG_REG("AUXCMD_CLEAR_PP\n");
			m_ist = false;
			break;

		case AUXCMD_CHIP_RESET:
			LOG_REG("AUXCMD_CHIP_RESET\n");
			device_reset();
			break;

		case AUXCMD_FINISH_HANDSHAKE:
			LOG_REG("AUXCMD_FINISH_HANDSHAKE\n");
			break;

		case AUXCMD_TRIGGER:
			LOG_REG("AUXCMD_TRIGGER\n");
			update_state(m_dt_state, DeviceTriggerState::DTAS);
			break;

		case AUXCMD_CLEAR_RTL:
			LOG_REG("AUXCMD_CLEAR_RTL\n");
			m_rtl = false;
			break;

		case AUXCMD_SET_RTL:
			LOG_REG("AUXCMD_SET_RTL\n");
			m_rtl = true;
			break;

		case AUXCMD_SET_PP:
			LOG_REG("AUXCMD_SET_PP\n");
			m_ist = true;
			break;

		case AUXCMD_SEND_EOI:
			LOG_REG("AUXCMD_SEND_EOI\n");
			m_send_eoi = true;
			break;

		case AUXCMD_NON_VALID_SA:
			LOG_REG("AUXCMD_NON_VALID_SA\n");
			m_apt_flag = false;
			break;

		case AUXCMD_VALID_SA:
			LOG_REG("AUXCMD_VALID_SA: APT %d\n", m_apt_flag);
			if (m_tp_state == TalkerPrimaryState::TPAS)
				update_state(m_t_state, TalkerState::TADS);

			if (m_lp_state == ListenerPrimaryState::LPAS) {
				update_state(m_l_state, ListenerState::LADS);
				if ((m_address_mode & 3) != 1 && m_lp_state == ListenerPrimaryState::LPAS) {
					if (m_rl_state == RemoteLocalState::LOCS && !m_rtl)
						update_state(m_rl_state, RemoteLocalState::REMS);
					if (m_rl_state == RemoteLocalState::LWLS)
						update_state(m_rl_state, RemoteLocalState::RWLS);
				}
			}

			m_cpt_flag = false;
			m_apt_flag = false;
			break;

		case AUXCMD_PON:
			LOG_REG("AUXCMD_PON\n");
			device_reset();
			break;
		}
		case 1:
			/* preset internal clock counter */
			break;
		case 4:
			LOG_REG("AUXA = %02X\n", data & 0x1f);
			/* AUXA write */
			m_auxa = data;
			break;
		case 5:
			LOG_REG("AUXB = %02X\n", data & 0x1f);
			m_auxb = data;
			break;
		case 3:
			LOG_REG("PPOLL %s\n", (data & 0x10) ? "disable" : "enable");
			m_lpe = !!!(data & 0x10);
			m_pp_sense = data & 0x08;
			m_pp_line = data & 7;
			break;
		default:
			break;

	}
	run_fsm();
}

WRITE8_MEMBER(i8291a_device::addr01_w)
{
	LOG_REG("%s: %02X\n", __FUNCTION__, data);
	if (data & REG_ADDRESS01_ARS)
		m_address1 = data & ~REG_ADDRESS01_ARS;
	else
		m_address0 = data & ~REG_ADDRESS01_ARS;
	run_fsm();
}

WRITE8_MEMBER(i8291a_device::eos_w)
{
	LOG_REG("%s: %02X\n", __FUNCTION__, data);
	m_eos = data;
	run_fsm();
}

WRITE_LINE_MEMBER(i8291a_device::nrfd_w)
{
	m_nrfd = !state;
	run_fsm();
}

WRITE_LINE_MEMBER(i8291a_device::ndac_w)
{
	m_ndac = !state;
	run_fsm();
}

WRITE_LINE_MEMBER(i8291a_device::dav_w)
{
	m_dav = !state;
	run_fsm();
}

WRITE_LINE_MEMBER(i8291a_device::eoi_w)
{
	m_eoi = !state;
	run_fsm();
}

WRITE_LINE_MEMBER(i8291a_device::srq_w)
{
	m_srq = !state;
	run_fsm();
}

WRITE_LINE_MEMBER(i8291a_device::ifc_w)
{
	m_ifc = !state;
	run_fsm();
}

WRITE_LINE_MEMBER(i8291a_device::atn_w)
{
	m_atn = !state;
	run_fsm();
}

WRITE_LINE_MEMBER(i8291a_device::ren_w)
{
	m_ren = !state;
	run_fsm();
}

void i8291a_device::set_dav(bool state)
{
	if (m_dav_out == state) {
		m_dav_out = !state;
		m_dav_write_func(m_dav_out);
	}
}

void i8291a_device::set_nrfd(bool state)
{
	if (m_nrfd_out == state) {
		m_nrfd_out = !state;
		m_nrfd_write_func(m_nrfd_out);
	}
}

void i8291a_device::set_ndac(bool state)
{
	if (m_ndac_out == state) {
		m_ndac_out = !state;
		m_ndac_write_func(m_ndac_out);
	}
}

void i8291a_device::set_eoi(bool state)
{
	if (m_eoi_out == state) {
		m_eoi_out = !state;
		m_eoi_write_func(m_eoi_out);
	}
}

void i8291a_device::set_srq(bool state)
{
	if (m_srq_out == state) {
		m_srq_out = !state;
		m_srq_write_func(m_srq_out);
	}
}

void i8291a_device::run_sh_fsm()
{

	if (m_pon || m_atn || !(m_t_state == TalkerState::TACS || m_t_state == TalkerState::SPAS)) {
		update_state(m_sh_state, SourceHandshake::SIDS);
	}

	switch (m_sh_state) {
	case SourceHandshake::SIDS:
		m_dio_write_func(0xff);
		set_eoi(false);
		m_nba = false;
		if (m_t_state == TalkerState::TACS || m_t_state == TalkerState::SPAS) {
			update_state(m_sh_state, SourceHandshake::SGNS);
			m_ints1 |= REG_INTS1_BO;
		}
		break;

	case SourceHandshake::SGNS:
		set_dav(false);

		if (m_nba) {
			update_state(m_sh_state, SourceHandshake::SDYS);
			m_ints1 &= ~REG_INTS1_BO;
		}
		break;

	case SourceHandshake::SDYS:
		if (!m_nrfd)
			update_state(m_sh_state, SourceHandshake::STRS);
		break;

	case SourceHandshake::STRS:
		m_nba = false;
		m_dio_write_func(~m_dout);
		set_eoi(m_send_eoi);

		set_dav(true);
		if (!m_ndac) {
			m_send_eoi = false;
			m_ints1 |= REG_INTS1_BO;
			update_state(m_sh_state, SourceHandshake::SGNS);
		}
		break;
	}
}

void i8291a_device::handle_command()
{
	if (m_din <= 0x1f) {
		switch (m_din) {
		case IFCMD_DCL:
			LOG_CMD("DCL\n");
			update_state(m_dc_state, DeviceClearState::DCAS);
			break;
		case IFCMD_GTL:
			LOG_CMD("GTL\n");
			if (m_l_state == ListenerState::LADS)
				update_state(m_rl_state, RemoteLocalState::LWLS);
			if (m_rl_state == RemoteLocalState::REMS &&
					m_l_state == ListenerState::LADS && m_rtl)
				update_state(m_rl_state, RemoteLocalState::LOCS);
			break;
		case IFCMD_SDC:
			LOG_CMD("SDC\n");
			if (m_l_state == ListenerState::LADS)
				update_state(m_dc_state, DeviceClearState::DCAS);
			break;
		case IFCMD_GET:
			LOG_CMD("GET\n");
			break;
		case IFCMD_LLO:
			LOG_CMD("LLO\n");
			if (m_rl_state == RemoteLocalState::LOCS)
				update_state(m_rl_state, RemoteLocalState::LWLS);
			if (m_rl_state == RemoteLocalState::REMS)
				update_state(m_rl_state, RemoteLocalState::RWLS);
			break;
		case IFCMD_SPE:
			LOG_CMD("SPE\n");
			update_state(m_tsp_state, TalkerSerialPollState::SPMS);
			break;
		case IFCMD_SPD:
			LOG_CMD("SPD\n");
			update_state(m_tsp_state, TalkerSerialPollState::SPIS);
			break;
		case 0:
			break;
		default:
			LOG("unhandled message: %02X\n", m_din);
			if (m_auxb & REG_AUXB_CPT_ENABLE) {
				m_cpt_flag = m_din;
				m_ints1 |= REG_INTS1_CPT;
			}
			break;
		}
	} else if (m_din >= 0x20 && m_din <= 0x3f) {
		bool addr_matched = false;
		LOG_CMD("MLA%d - mode %d, my addr0 %s%s%02x addr1 %s%s%02x\n", m_din & 0x1f,
				m_address_mode & 3,
				(m_address0 & REG_ADDRESS_DT) ? "DT " : "",
				(m_address0 & REG_ADDRESS_DL) ? "DL " : "",
				m_address0 & 0x1f,
				(m_address1 & REG_ADDRESS_DT) ? "DT " : "",
				(m_address1 & REG_ADDRESS_DL) ? "DL " : "",
						m_address1 & 0x1f);

		if ((m_din & 0x1f) == (m_address0 & 0x1f) && !(m_address0 & REG_ADDRESS_DL)) {
			LOG_CMD("Address 0 matched (mode %02X)\n", m_address_mode);
			m_address_status &= ~REG_ADDRESS_STATUS_MJMN;
			addr_matched = true;
		}

		if ((m_address_mode & 3) != 2 && (m_din & 0x1f) == (m_address1 & 0x1f) && !(m_address1 & REG_ADDRESS_DL)) {
			LOG_CMD("Address 1 matched (mode %02X)\n", m_address_mode);
			m_address_status |= REG_ADDRESS_STATUS_MJMN;
			addr_matched = true;
		}

		if (addr_matched) {
			update_state(m_lp_state, ListenerPrimaryState::LPAS);
			if (addr_matched && (m_address_mode & 3) == 1 && m_l_state == ListenerState::LIDS)
				update_state(m_l_state, ListenerState::LADS);
		} else {
			update_state(m_lp_state, ListenerPrimaryState::LPIS);
			update_state(m_l_state, ListenerState::LIDS);
			m_address_status &= ~(REG_ADDRESS_STATUS_MJMN);
		}

		if (addr_matched && (m_address_mode & 3) == 1 && m_rl_state == RemoteLocalState::LWLS)
			update_state(m_rl_state, RemoteLocalState::RWLS);
	} else if (m_din >= 0x40 && m_din <= 0x5f) {
		bool addr_matched = false;

		LOG_CMD("MTA%d\n", m_din & 0x1f);

		if ((m_din & 0x1f) == (m_address0 & 0x1f) && !(m_address0 & REG_ADDRESS_DT)) {
			LOG_STATE("Address 0 matched (mode %02X)\n", m_address_mode);
			m_address_status &= ~REG_ADDRESS_STATUS_MJMN;
			addr_matched = true;
		}

		if ((m_address_mode & 3) != 2 && (m_din & 0x1f) == (m_address1 & 0x1f) && !(m_address1 & REG_ADDRESS_DT)) {
			LOG_STATE("Address 1 matched (mode %02X)\n", m_address_mode);
			m_address_status |= REG_ADDRESS_STATUS_MJMN;
			addr_matched = true;
		}

		if(addr_matched) {
			update_state(m_tp_state, TalkerPrimaryState::TPAS);
			if ((m_address_mode & 3) == 1 && m_t_state == TalkerState::TIDS)
				update_state(m_t_state, TalkerState::TADS);
		} else {
			update_state(m_tp_state, TalkerPrimaryState::TPIS);
			update_state(m_t_state, TalkerState::TIDS);
		}

	} else if (m_din >= 0x60 && m_din <= 0x7f) {
		LOG_CMD("MSA%d\n", m_din & 0x1f);
		if ((m_address_mode & 3) == 2 && m_t_state == TalkerState::TIDS)
			update_state(m_t_state, TalkerState::TADS);

		if ((m_address_mode & 3) != 1 && m_rl_state == RemoteLocalState::LWLS)
			update_state(m_rl_state, RemoteLocalState::RWLS);

		if ((m_address_mode & 3) == 3) {
			/* In address mode 3, MSA is passed to host for verification */
			m_ints1 |= REG_INTS1_APT;
			m_cpt = m_din;
			m_apt_flag = true;
		}
	}
}

void i8291a_device::run_ah_fsm()
{
	bool f2 = m_atn || m_l_state == ListenerState::LACS || m_l_state == ListenerState::LADS;

	if ((m_pon || !f2) && m_ah_state != AcceptorHandshake::AIDS) {
		LOG_STATE("reset AH: pon %d f2 %d\n", m_pon, f2);
		update_state(m_ah_state, AcceptorHandshake::AIDS);
		set_nrfd(false);
		set_ndac(false);
		return;
	}

	switch (m_ah_state) {
	case AcceptorHandshake::AIDS:
		if (f2 && !m_pon)
			update_state(m_ah_state, AcceptorHandshake::ANRS);
		break;
	case AcceptorHandshake::ANRS:
		set_nrfd(true);
		set_ndac(true);
		m_cpt_flag = false;
		m_apt_flag = false;
		//LOG("m_rdy: %d m_cpt_flag: %d m_apt_flag: %d, m_din_flag %d\n", m_rdy, m_cpt_flag, m_apt_flag, m_din_flag);
		if (m_atn || m_rdy)
			update_state(m_ah_state, AcceptorHandshake::ACRS);
		break;

	case AcceptorHandshake::ACRS:
		set_nrfd(false);
		if (m_dav)
			update_state(m_ah_state, AcceptorHandshake::ADYS);
		break;
	case AcceptorHandshake::ADYS:
		if (!m_dav) {
			update_state(m_ah_state, AcceptorHandshake::ACRS);
		} else {
			update_state(m_ah_state, AcceptorHandshake::ACDS);
		}
		break;
	case AcceptorHandshake::ACDS:

		if (!m_rdy) {
			update_state(m_ah_state, AcceptorHandshake::AWNS);
			break;
		}
		m_din = ~m_dio_read_func();
		set_nrfd(true);

		if (m_atn) {
			/* Received command */
			m_din &= ~0x80; // TODO: parity
			handle_command();
			if (!m_cpt_flag && !m_apt_flag)
				update_state(m_ah_state, AcceptorHandshake::AWNS);
		} else {
			/* Received data */
			m_din_flag = true;
			m_ints1 |= REG_INTS1_BI;
			if (m_eoi)
				m_ints1 |= REG_INTS1_END;
			// TODO: END on EOS
		}
		break;
	case AcceptorHandshake::AWNS:
		set_ndac(false);
		// TODO: T3 delay, rdy holdoff

		if (!m_dav)
			update_state(m_ah_state, AcceptorHandshake::ANRS);
		break;
	}
}

void i8291a_device::run_t_fsm()
{
	switch (m_t_state) {
	case TalkerState::TIDS:
		m_address_status &= ~REG_ADDRESS_STATUS_TA;
		break;

	case TalkerState::TADS:
		m_address_status |= REG_ADDRESS_STATUS_TA;
		if(!m_atn) {
			if (m_tsp_state == TalkerSerialPollState::SPMS)
				update_state(m_t_state, TalkerState::SPAS);
			else
				update_state(m_t_state, TalkerState::TACS);
		}

		if (m_tp_state == TalkerPrimaryState::TPIS)
			update_state(m_t_state, TalkerState::TIDS);
		break;
		// TODO: F4 function
	case TalkerState::TACS:
		m_address_status |= REG_ADDRESS_STATUS_TA;
		if (m_atn)
			update_state(m_t_state, TalkerState::TADS); // TODO: T2 delay
		break;

	case TalkerState::SPAS:
		if (m_atn)
			update_state(m_t_state, TalkerState::TADS); // TODO: T2 delay
		break;
	}
}

void i8291a_device::run_tp_fsm()
{
	switch (m_tp_state) {
	case TalkerPrimaryState::TPIS:
		m_address_status &= ~REG_ADDRESS_STATUS_TPAS;
		break;
	case TalkerPrimaryState::TPAS:
		m_address_status |= REG_ADDRESS_STATUS_TPAS;
		break;
	}
}

void i8291a_device::run_tsp_fsm()
{
	if (m_pon || m_ifc)
		update_state(m_tsp_state, TalkerSerialPollState::SPIS);

	switch(m_tsp_state) {
	case TalkerSerialPollState::SPIS:
		break;
	case TalkerSerialPollState::SPMS:
		break;
	}
}

void i8291a_device::run_l_fsm()
{
	switch (m_l_state) {
	case ListenerState::LIDS:
		m_address_status &= ~REG_ADDRESS_STATUS_LA;
		break;
	case ListenerState::LADS:
		m_address_status |= REG_ADDRESS_STATUS_LA;
		if (m_lp_state == ListenerPrimaryState::LPIS)
			update_state(m_l_state, ListenerState::LIDS);
		break;
	case ListenerState::LACS:
		break;
	}
}

void i8291a_device::run_dc_fsm()
{
	switch (m_dc_state) {
	case DeviceClearState::DCIS:
		break;
	case DeviceClearState::DCAS:
		m_ints1 |= REG_INTS1_DEC;
		update_state(m_dc_state, DeviceClearState::DCIS);
		break;
	}
}

void i8291a_device::run_dt_fsm()
{
	switch (m_dt_state) {
	case DeviceTriggerState::DTIS:
		break;
	case DeviceTriggerState::DTAS:
		m_trig_write_func(true);
		m_trig_write_func(false);
		update_state(m_dt_state, DeviceTriggerState::DTIS);
		break;
	}
}

void i8291a_device::run_pp_fsm()
{
	if (m_pon)
		update_state(m_pp_state, ParallelPollState::PPIS);

	switch (m_pp_state) {
	case ParallelPollState::PPIS:
		if (m_lpe && !m_pon)
			update_state(m_pp_state, ParallelPollState::PPSS);
		break;

	case ParallelPollState::PPSS:
		if (!m_lpe) {
			update_state(m_pp_state, ParallelPollState::PPIS);
			break;
		}

		if (m_atn && m_eoi)
			update_state(m_pp_state, ParallelPollState::PPAS);
		break;

	case ParallelPollState::PPAS:
		if (!m_atn || !m_eoi) {
			m_dio_write_func(0xff);
			update_state(m_pp_state, ParallelPollState::PPSS);
			break;
		}

		if (m_ist == m_pp_sense)
			m_dio_write_func(0x7f); // XXX ~(1 << (7 - m_pp_line)));
		else
			m_dio_write_func(0xff);
		break;
	}
}

void i8291a_device::run_sp_fsm()
{
	switch (m_sp_state) {
	case SerialPollState::APRS:
		break;
	case SerialPollState::NPRS:
		break;
	case SerialPollState::SRQS:
		break;
	}
}

void i8291a_device::run_rl_fsm()
{
	if (m_pon || !m_ren)
		update_state(m_rl_state, RemoteLocalState::LOCS);

	switch (m_rl_state) {
	case RemoteLocalState::LOCS:
		break;
	case RemoteLocalState::LWLS:
		break;
	case RemoteLocalState::REMS:
		break;
	case RemoteLocalState::RWLS:
		break;
	}
}

void i8291a_device::run_lp_fsm()
{
	switch(m_lp_state) {
	case ListenerPrimaryState::LPIS:
		m_address_status &= ~REG_ADDRESS_STATUS_LPAS;
		break;
	case ListenerPrimaryState::LPAS:
		m_address_status |= REG_ADDRESS_STATUS_LPAS;
		break;
	}
}

void i8291a_device::run_fsm()
{
	if (m_ignore_ext_signals)
		return;
	m_ignore_ext_signals = true;
	do {
		m_state_changed = false;

		auto m_l_state_old = m_l_state;
		auto m_t_state_old = m_t_state;
		auto m_rl_state_old = m_rl_state;
		run_sh_fsm();
		run_ah_fsm();
		run_t_fsm();
		run_tp_fsm();
		run_tsp_fsm();
		run_l_fsm();
		run_lp_fsm();
		run_dc_fsm();
		run_dt_fsm();
		run_sp_fsm();
		run_pp_fsm();
		run_rl_fsm();

		if ((m_t_state_old == TalkerState::TIDS && m_t_state == TalkerState::TADS) ||
				(m_t_state_old == TalkerState::TADS && m_t_state == TalkerState::TIDS) ||
				(m_l_state_old == ListenerState::LIDS && m_l_state == ListenerState::LADS) ||
				(m_l_state_old == ListenerState::LADS && m_l_state == ListenerState::LIDS))
			m_ints2 |= REG_INTS2_ADSC;

		if ((m_rl_state_old == RemoteLocalState::LOCS && m_rl_state == RemoteLocalState::REMS) ||
				(m_rl_state_old == RemoteLocalState::REMS && m_rl_state == RemoteLocalState::LOCS) ||
				(m_rl_state_old == RemoteLocalState::LWLS && m_rl_state == RemoteLocalState::RWLS) ||
				(m_rl_state_old == RemoteLocalState::RWLS && m_rl_state == RemoteLocalState::LWLS))
			m_ints2 |= REG_INTS2_REMC;

		if ((m_rl_state_old == RemoteLocalState::LOCS && m_rl_state == RemoteLocalState::LWLS) ||
				(m_rl_state_old == RemoteLocalState::REMS && m_rl_state == RemoteLocalState::RWLS) ||
				(m_rl_state_old == RemoteLocalState::LWLS && m_rl_state == RemoteLocalState::LOCS) ||
				(m_rl_state_old == RemoteLocalState::RWLS && m_rl_state == RemoteLocalState::REMS))
			m_ints2 |= REG_INTS2_REMC;

		m_rdy = !m_apt_flag && !m_cpt_flag && !m_din_flag;
		update_int();
	} while (m_state_changed);
	m_ignore_ext_signals = false;
}


const char *i8291a_device::get_state_name(DeviceClearState state)
{
	switch(state) {
		case DeviceClearState::DCIS:
			return "DICS";
		case DeviceClearState::DCAS:
			return "DCAS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(DeviceTriggerState state)
{
	switch(state) {
	case DeviceTriggerState::DTIS:
		return "DTIS";
	case DeviceTriggerState::DTAS:
		return "DTAS";
	}
	return "Unknown";
}


const char *i8291a_device::get_state_name(ParallelPollState state)
{
	switch(state){
	case ParallelPollState::PPIS:
		return "PPIS";
	case ParallelPollState::PPSS:
		return "PPSS";
	case ParallelPollState::PPAS:
		return "PPAS";
	}
	return "Unknown";
}


const char *i8291a_device::get_state_name(SerialPollState state)
{
	switch(state) {
	case SerialPollState::APRS:
		return "APRS";
	case SerialPollState::NPRS:
		return "NPRS";
	case SerialPollState::SRQS:
		return "SRQS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(RemoteLocalState state)
{
	switch(state) {
	case RemoteLocalState::LOCS:
		return "LOCS";
	case RemoteLocalState::LWLS:
		return "LWLS";
	case RemoteLocalState::REMS:
		return "REMS";
	case RemoteLocalState::RWLS:
		return "RWLS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(ListenerPrimaryState state)
{
	switch(state) {
	case ListenerPrimaryState::LPIS:
		return "LPIS";
	case ListenerPrimaryState::LPAS:
		return "LPAS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(SourceHandshake state)
{
	switch(state) {
	case SourceHandshake::SIDS:
		return "SIDS";
	case SourceHandshake::SGNS:
		return "SGNS";
	case SourceHandshake::SDYS:
		return "SDYS";
	case SourceHandshake::STRS:
		return "STRS";
	}
	return "Unknown";
}


const char *i8291a_device::get_state_name(AcceptorHandshake state)
{
	switch(state) {
	case AcceptorHandshake::AIDS:
		return "AIDS";
	case AcceptorHandshake::ANRS:
		return "ANRS";
	case AcceptorHandshake::ACRS:
		return "ACRS";
	case AcceptorHandshake::ADYS:
		return "ADYS";
	case AcceptorHandshake::ACDS:
		return "ACDS";
	case AcceptorHandshake::AWNS:
		return "AWNS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(TalkerState state)
{
	switch(state) {
	case TalkerState::TIDS:
		return "TIDS";
	case TalkerState::TADS:
		return "TADS";
	case TalkerState::TACS:
		return "TACS";
	case TalkerState::SPAS:
		return "SPAS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(TalkerPrimaryState state)
{
	switch(state) {
	case TalkerPrimaryState::TPIS:
		return "TPIS";
	case TalkerPrimaryState::TPAS:
		return "TPAS";
	}
	return "Unknown";
}


const char *i8291a_device::get_state_name(ListenerState state)
{
	switch(state) {
	case ListenerState::LIDS:
		return "LIDS";
	case ListenerState::LADS:
		return "LADS";
	case ListenerState::LACS:
		return "LACS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(TalkerSerialPollState state)
{
	switch(state) {
	case TalkerSerialPollState::SPIS:
		return "SPIS";
	case TalkerSerialPollState::SPMS:
		return "SPMS";
	}
	return "Unknown";
}

void i8291a_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(i8291a_device::din_r), FUNC(i8291a_device::dout_w));
	map(0x01, 0x01).rw(FUNC(i8291a_device::ints1_r), FUNC(i8291a_device::ie1_w));
	map(0x02, 0x02).rw(FUNC(i8291a_device::ints2_r), FUNC(i8291a_device::ie2_w));
	map(0x03, 0x03).rw(FUNC(i8291a_device::spoll_stat_r), FUNC(i8291a_device::spoll_mode_w));
	map(0x04, 0x04).rw(FUNC(i8291a_device::addr_stat_r), FUNC(i8291a_device::addr_mode_w));
	map(0x05, 0x05).rw(FUNC(i8291a_device::cpt_r), FUNC(i8291a_device::aux_mode_w));
	map(0x06, 0x06).rw(FUNC(i8291a_device::addr0_r), FUNC(i8291a_device::addr01_w));
	map(0x07, 0x07).rw(FUNC(i8291a_device::addr1_r), FUNC(i8291a_device::eos_w));

}
