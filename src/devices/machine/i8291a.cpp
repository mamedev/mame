// license:BSD-3-Clause
// copyright-holders: Sven Schnelle
/*********************************************************************

 i8291a.cpp

 Intel 8291A GPIB controller

 *********************************************************************/

#include "emu.h"


#include "i8291a.h"

#define LOG_STATE  (LOG_GENERAL << 1)
#define LOG_REG    (LOG_STATE << 1)
#define LOG_INT    (LOG_REG << 1)
#define LOG_CMD    (LOG_INT << 1)

#define VERBOSE 0

#include "logmacro.h"
DEFINE_DEVICE_TYPE(I8291A, i8291a_device, "i8291a", "Intel I8291A GPIB Talker/Listener")

ALLOW_SAVE_TYPE(i8291a_device::listener_state);
ALLOW_SAVE_TYPE(i8291a_device::talker_state);
ALLOW_SAVE_TYPE(i8291a_device::talker_primary_state);
ALLOW_SAVE_TYPE(i8291a_device::listener_primary_state);
ALLOW_SAVE_TYPE(i8291a_device::parallel_poll_state);
ALLOW_SAVE_TYPE(i8291a_device::device_trigger_state);
ALLOW_SAVE_TYPE(i8291a_device::device_clear_state);
ALLOW_SAVE_TYPE(i8291a_device::talker_serial_poll_state);
ALLOW_SAVE_TYPE(i8291a_device::serial_poll_state);
ALLOW_SAVE_TYPE(i8291a_device::remote_local_state);
ALLOW_SAVE_TYPE(i8291a_device::source_handshake_state);
ALLOW_SAVE_TYPE(i8291a_device::acceptor_handshake_state);

i8291a_device::i8291a_device(const machine_config &mconfig, const char *tag,
		device_t *owner, uint32_t clock) :
	device_t{mconfig, I8291A, tag, owner, clock},
	m_int_write_func{*this},
	m_dreq_write_func{*this},
	m_trig_write_func{*this},
	m_eoi_write_func{*this},
	m_dav_write_func{*this},
	m_nrfd_write_func{*this},
	m_ndac_write_func{*this},
	m_srq_write_func{*this},
	m_dio_write_func{*this},
	m_dio_read_func{*this, 0xff},
	m_din{0},
	m_dout{0},
	m_ints1{0},
	m_ints2{0},
	m_ie1{0},
	m_ie2{0},
	m_address0{0},
	m_address1{0},
	m_eos{0},
	m_spoll_mode{0},
	m_address_mode{0},
	m_address_status{0},
	m_cpt{0},
	m_auxa{0},
	m_auxb{0},
	m_atn{false},
	m_ren{false},
	m_nrfd{false},
	m_ndac{false},
	m_dav{false},
	m_srq{false},
	m_ifc{false},
	m_eoi{false},
	m_dio{0},
	m_nrfd_out{false},
	m_ndac_out{false},
	m_dav_out{false},
	m_srq_out{false},
	m_eoi_out{false},
	m_pon{false},
	m_rdy{false},
	m_lpe{false},
	m_ist{false},
	m_rtl{false},
	m_apt_flag{false},
	m_cpt_flag{false},
	m_din_flag{false},
	m_nba{false},
	m_pp_sense{false},
	m_pp_line{0},
	m_send_eoi{false},
	m_sh_state{source_handshake_state::SIDS},
	m_ah_state{acceptor_handshake_state::AIDS},
	m_t_state{talker_state::TIDS},
	m_tp_state{talker_primary_state::TPIS},
	m_tsp_state{talker_serial_poll_state::SPIS},
	m_l_state{listener_state::LIDS},
	m_lp_state{listener_primary_state::LPIS},
	m_rl_state{remote_local_state::LOCS},
	m_pp_state{parallel_poll_state::PPIS},
	m_dc_state{device_clear_state::DCIS},
	m_dt_state{device_trigger_state::DTIS},
	m_state_changed(false),
	m_ignore_ext_signals(false),
	m_intr_out(false),
	m_dreq_out(false)
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
	update_state(m_t_state, talker_state::TIDS);
	update_state(m_tp_state, talker_primary_state::TPIS);
	update_state(m_tsp_state, talker_serial_poll_state::SPIS);
	update_state(m_l_state, listener_state::LIDS);
	update_state(m_rl_state, remote_local_state::LOCS);
	update_state(m_pp_state, parallel_poll_state::PPIS);
	update_state(m_dc_state, device_clear_state::DCIS);
	update_state(m_dt_state, device_trigger_state::DTIS);
	update_state(m_sh_state, source_handshake_state::SIDS);
	update_state(m_ah_state, acceptor_handshake_state::AIDS);
	update_state(m_lp_state, listener_primary_state::LPIS);
}

void i8291a_device::device_start()
{
	save_item(NAME(m_din));
	save_item(NAME(m_dout));
	save_item(NAME(m_ints1));
	save_item(NAME(m_ints2));
	save_item(NAME(m_ie1));
	save_item(NAME(m_ie2));
	save_item(NAME(m_address0));
	save_item(NAME(m_address1));
	save_item(NAME(m_eos));
	save_item(NAME(m_spoll_mode));
	save_item(NAME(m_address_mode));
	save_item(NAME(m_address_status));
	save_item(NAME(m_cpt));
	save_item(NAME(m_auxa));
	save_item(NAME(m_auxb));
	save_item(NAME(m_atn));
	save_item(NAME(m_ren));
	save_item(NAME(m_nrfd));
	save_item(NAME(m_ndac));
	save_item(NAME(m_dav));
	save_item(NAME(m_srq));
	save_item(NAME(m_ifc));
	save_item(NAME(m_eoi));
	save_item(NAME(m_dio));
	save_item(NAME(m_nrfd_out));
	save_item(NAME(m_ndac_out));
	save_item(NAME(m_dav_out));
	save_item(NAME(m_srq_out));
	save_item(NAME(m_eoi_out));
	save_item(NAME(m_pon));
	save_item(NAME(m_rdy));
	save_item(NAME(m_lpe));
	save_item(NAME(m_ist));
	save_item(NAME(m_rtl));
	save_item(NAME(m_apt_flag));
	save_item(NAME(m_cpt_flag));
	save_item(NAME(m_din_flag));
	save_item(NAME(m_nba));
	save_item(NAME(m_pp_sense));
	save_item(NAME(m_pp_line));
	save_item(NAME(m_send_eoi));
	save_item(NAME(m_intr_out));
	save_item(NAME(m_dreq_out));
	save_item(NAME(m_sh_state));
	save_item(NAME(m_ah_state));
	save_item(NAME(m_t_state));
	save_item(NAME(m_tp_state));
	save_item(NAME(m_tsp_state));
	save_item(NAME(m_l_state));
	save_item(NAME(m_lp_state));
	save_item(NAME(m_rl_state));
	save_item(NAME(m_sp_state));
	save_item(NAME(m_pp_state));
	save_item(NAME(m_dc_state));
	save_item(NAME(m_dt_state));
}

uint8_t i8291a_device::din_r()
{
	LOGMASKED(LOG_REG, "%s: %02X\n", __FUNCTION__, m_din);
	if (!machine().side_effects_disabled()) {
		m_din_flag = false;
		m_ints1 &= ~REG_INTS1_BI;
		update_int();
		run_fsm();
	}
	return m_din;
}

void i8291a_device::update_int()
{
	bool dreq = ((m_ie2 & REG_IE2_DMAO) && (m_ints1 & REG_INTS1_BO)) |
			((m_ie2 & REG_IE2_DMAI)  && (m_ints1 & REG_INTS1_BI));

	if (m_dreq_out != dreq) {
		LOGMASKED(LOG_INT, "%s: dreq %s\n", __FUNCTION__, dreq ? "true" : "false");
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
		LOGMASKED(LOG_INT, "%s: intr %s\n", __FUNCTION__, intr ? "true" : "false");
		m_int_write_func(intr != bool(m_auxb & REG_AUXB_INT_ACTIVE_LOW));
		m_intr_out = intr;
	}
}

uint8_t i8291a_device::ints1_r()
{
	uint8_t ret = m_ints1;

	if (!machine().side_effects_disabled()) {
		LOGMASKED(LOG_REG, "%s: %02X\n", __FUNCTION__, ret);
		m_ints1 = 0;
		update_int();
	}
	return ret;
}

uint8_t i8291a_device::ints2_r()
{
	uint8_t ret = m_ints2;

	if (!machine().side_effects_disabled()) {
		LOGMASKED(LOG_REG, "%s: %02X\n", __FUNCTION__, ret);
		m_ints2 = 0;
		update_int();
	}
	return ret;
}

uint8_t i8291a_device::spoll_stat_r()
{
	LOGMASKED(LOG_REG, "%s\n", __FUNCTION__);
	return 0;
}

uint8_t i8291a_device::addr_stat_r()
{
	if (!(m_address_status & 0xfe))
			m_address_status = 0;
	LOGMASKED(LOG_REG, "[%s] %s = %02X\n", machine().describe_context().c_str(), __FUNCTION__, m_address_status);

	return m_address_status;
}

uint8_t i8291a_device::cpt_r()
{
	LOGMASKED(LOG_REG, "%s\n", __FUNCTION__);
	return m_cpt;
}

uint8_t i8291a_device::addr0_r()
{
	//LOGMASKED(LOG_REG, "%s = %02X\n", __FUNCTION__, m_address0);
	return m_address0;
}

uint8_t i8291a_device::addr1_r()
{
	LOGMASKED(LOG_REG, "%s = %02X\n", __FUNCTION__, m_address1);
	return m_address1;
}

void i8291a_device::dout_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "%s: %02X\n", __FUNCTION__, data);
	if (m_nba)
		return;
	m_dout = data;
	m_nba = true;
	run_fsm();
}

void i8291a_device::ie1_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "%s: %02X\n", __FUNCTION__, data);
	m_ie1 = data;
	run_fsm();
}

void i8291a_device::ie2_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "%s: %02X\n", __FUNCTION__, data);
	m_ie2 = data;
	run_fsm();
}

void i8291a_device::spoll_mode_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "%s: %02X\n", __FUNCTION__, data);
	m_spoll_mode = data;
	run_fsm();
}

void i8291a_device::addr_mode_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "%s: %02X\n", __FUNCTION__, data);
	m_address_mode = data & 3;
	run_fsm();
}

void i8291a_device::aux_mode_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "%s: %02X\n", __FUNCTION__, data);
	switch (data >> 5) {
	case 0:
		switch (data) {
		case AUXCMD_IMMEDIATE_EXEC_PON:
			LOGMASKED(LOG_REG, "AUXCMD_IMMEDIATE_EXEC_PON\n");
			m_pon = 0;
			m_rdy = 1;
			break;

		case AUXCMD_CLEAR_PP:
			LOGMASKED(LOG_REG, "AUXCMD_CLEAR_PP\n");
			m_ist = false;
			break;

		case AUXCMD_CHIP_RESET:
			LOGMASKED(LOG_REG, "AUXCMD_CHIP_RESET\n");
			device_reset();
			break;

		case AUXCMD_FINISH_HANDSHAKE:
			LOGMASKED(LOG_REG, "AUXCMD_FINISH_HANDSHAKE\n");
			break;

		case AUXCMD_TRIGGER:
			LOGMASKED(LOG_REG, "AUXCMD_TRIGGER\n");
			update_state(m_dt_state, device_trigger_state::DTAS);
			break;

		case AUXCMD_CLEAR_RTL:
			LOGMASKED(LOG_REG, "AUXCMD_CLEAR_RTL\n");
			m_rtl = false;
			break;

		case AUXCMD_SET_RTL:
			LOGMASKED(LOG_REG, "AUXCMD_SET_RTL\n");
			m_rtl = true;
			break;

		case AUXCMD_SET_PP:
			LOGMASKED(LOG_REG, "AUXCMD_SET_PP\n");
			m_ist = true;
			break;

		case AUXCMD_SEND_EOI:
			LOGMASKED(LOG_REG, "AUXCMD_SEND_EOI\n");
			m_send_eoi = true;
			break;

		case AUXCMD_NON_VALID_SA:
			LOGMASKED(LOG_REG, "AUXCMD_NON_VALID_SA\n");
			m_apt_flag = false;
			break;

		case AUXCMD_VALID_SA:
			LOGMASKED(LOG_REG, "AUXCMD_VALID_SA: APT %d\n", m_apt_flag);
			if (m_tp_state == talker_primary_state::TPAS)
				update_state(m_t_state, talker_state::TADS);

			if (m_lp_state == listener_primary_state::LPAS) {
				update_state(m_l_state, listener_state::LADS);
				if ((m_address_mode & 3) != 1 && m_lp_state == listener_primary_state::LPAS) {
					if (m_rl_state == remote_local_state::LOCS && !m_rtl)
						update_state(m_rl_state, remote_local_state::REMS);
					if (m_rl_state == remote_local_state::LWLS)
						update_state(m_rl_state, remote_local_state::RWLS);
				}
			}

			m_cpt_flag = false;
			m_apt_flag = false;
			break;

		case AUXCMD_PON:
			LOGMASKED(LOG_REG, "AUXCMD_PON\n");
			device_reset();
			break;
		}
		break;
	case 1:
		/* preset internal clock counter */
		break;
	case 4:
		LOGMASKED(LOG_REG, "AUXA = %02X\n", data & 0x1f);
		/* AUXA write */
		m_auxa = data;
		break;
	case 5:
		LOGMASKED(LOG_REG, "AUXB = %02X\n", data & 0x1f);
		// force interrupt line update if polarity is changed
		if ((data ^ m_auxb) & REG_AUXB_INT_ACTIVE_LOW)
			m_intr_out = !m_intr_out;
		m_auxb = data;
		break;
	case 3:
		LOGMASKED(LOG_REG, "PPOLL %s (line %d)\n", (data & 0x10) ? "disable" : "enable", data & 7);
		m_lpe = !!!(data & 0x10);
		m_pp_sense = data & 0x08;
		m_pp_line = data & 7;
		break;
	default:
		break;
	}
	run_fsm();
}

void i8291a_device::addr01_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "%s: %02X\n", __FUNCTION__, data);
	if (data & REG_ADDRESS01_ARS)
		m_address1 = data & ~REG_ADDRESS01_ARS;
	else
		m_address0 = data & ~REG_ADDRESS01_ARS;
	run_fsm();
}

void i8291a_device::eos_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "%s: %02X\n", __FUNCTION__, data);
	m_eos = data;
	run_fsm();
}

void i8291a_device::nrfd_w(int state)
{
	m_nrfd = !state;
	run_fsm();
}

void i8291a_device::ndac_w(int state)
{
	m_ndac = !state;
	run_fsm();
}

void i8291a_device::dav_w(int state)
{
	m_dav = !state;
	run_fsm();
}

void i8291a_device::eoi_w(int state)
{
	m_eoi = !state;
	run_fsm();
}

void i8291a_device::srq_w(int state)
{
	m_srq = !state;
	run_fsm();
}

void i8291a_device::ifc_w(int state)
{
	m_ifc = !state;
	run_fsm();
}

void i8291a_device::atn_w(int state)
{
	m_atn = !state;
	run_fsm();
}

void i8291a_device::ren_w(int state)
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

	if (m_pon || m_atn || !(m_t_state == talker_state::TACS || m_t_state == talker_state::SPAS)) {
		update_state(m_sh_state, source_handshake_state::SIDS);
	}

	switch (m_sh_state) {
	case source_handshake_state::SIDS:
		m_dio_write_func(0xff);
		set_eoi(false);
		m_nba = false;
		if (m_t_state == talker_state::TACS || m_t_state == talker_state::SPAS) {
			update_state(m_sh_state, source_handshake_state::SGNS);

		}
		break;

	case source_handshake_state::SGNS:
		set_dav(false);

		if (m_nba) {
			update_state(m_sh_state, source_handshake_state::SDYS);
			m_ints1 &= ~REG_INTS1_BO;
		} else {
			if (!m_nrfd)
				m_ints1 |= REG_INTS1_BO;
			else
				m_ints1 &= ~REG_INTS1_BO;
		}

		break;

	case source_handshake_state::SDYS:
		if (!m_nrfd)
			update_state(m_sh_state, source_handshake_state::STRS);
		break;

	case source_handshake_state::STRS:
		m_nba = false;
		m_dio_write_func(~m_dout);
		set_eoi(m_send_eoi);

		set_dav(true);
		if (!m_ndac) {
			m_send_eoi = false;
			m_ints1 |= REG_INTS1_BO;
			update_state(m_sh_state, source_handshake_state::SGNS);
		}
		break;
	}
}

void i8291a_device::handle_command()
{
	if (m_din <= 0x1f) {
		switch (m_din) {
		case IFCMD_DCL:
			LOGMASKED(LOG_CMD, "DCL\n");
			update_state(m_dc_state, device_clear_state::DCAS);
			break;
		case IFCMD_GTL:
			LOGMASKED(LOG_CMD, "GTL\n");
			if (m_l_state == listener_state::LADS)
				update_state(m_rl_state, remote_local_state::LWLS);
			if (m_rl_state == remote_local_state::REMS &&
					m_l_state == listener_state::LADS && m_rtl)
				update_state(m_rl_state, remote_local_state::LOCS);
			break;
		case IFCMD_SDC:
			LOGMASKED(LOG_CMD, "SDC\n");
			if (m_l_state == listener_state::LADS) {
				update_state(m_dc_state, device_clear_state::DCAS);
			}
			break;
		case IFCMD_GET:
			LOGMASKED(LOG_CMD, "GET\n");
			break;
		case IFCMD_LLO:
			LOGMASKED(LOG_CMD, "LLO\n");
			if (m_rl_state == remote_local_state::LOCS)
				update_state(m_rl_state, remote_local_state::LWLS);
			if (m_rl_state == remote_local_state::REMS)
				update_state(m_rl_state, remote_local_state::RWLS);
			break;
		case IFCMD_SPE:
			LOGMASKED(LOG_CMD, "SPE\n");
			update_state(m_tsp_state, talker_serial_poll_state::SPMS);
			break;
		case IFCMD_SPD:
			LOGMASKED(LOG_CMD, "SPD\n");
			update_state(m_tsp_state, talker_serial_poll_state::SPIS);
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
		LOGMASKED(LOG_CMD, "MLA%d - mode %d, my addr0 %s%s%02x addr1 %s%s%02x\n", m_din & 0x1f,
				m_address_mode & 3,
				(m_address0 & REG_ADDRESS_DT) ? "DT " : "",
				(m_address0 & REG_ADDRESS_DL) ? "DL " : "",
				m_address0 & 0x1f,
				(m_address1 & REG_ADDRESS_DT) ? "DT " : "",
				(m_address1 & REG_ADDRESS_DL) ? "DL " : "",
						m_address1 & 0x1f);

		if ((m_din & 0x1f) == (m_address0 & 0x1f) && !(m_address0 & REG_ADDRESS_DL)) {
			LOGMASKED(LOG_CMD, "Address 0 matched (mode %02X)\n", m_address_mode);
			m_address_status &= ~REG_ADDRESS_STATUS_MJMN;
			addr_matched = true;
		}

		if ((m_address_mode & 3) != 2 && (m_din & 0x1f) == (m_address1 & 0x1f) && !(m_address1 & REG_ADDRESS_DL)) {
			LOGMASKED(LOG_CMD, "Address 1 matched (mode %02X)\n", m_address_mode);
			m_address_status |= REG_ADDRESS_STATUS_MJMN;
			addr_matched = true;
		}

		if (addr_matched) {
			update_state(m_lp_state, listener_primary_state::LPAS);
			if (addr_matched && (m_address_mode & 3) == 1 && m_l_state == listener_state::LIDS)
				update_state(m_l_state, listener_state::LADS);
		} else {
			update_state(m_lp_state, listener_primary_state::LPIS);
			update_state(m_l_state, listener_state::LIDS);
			m_address_status &= ~(REG_ADDRESS_STATUS_MJMN);
		}

		if (addr_matched && (m_address_mode & 3) == 1 && m_rl_state == remote_local_state::LWLS)
			update_state(m_rl_state, remote_local_state::RWLS);
	} else if (m_din >= 0x40 && m_din <= 0x5f) {
		bool addr_matched = false;

		LOGMASKED(LOG_CMD, "MTA%d\n", m_din & 0x1f);

		if ((m_din & 0x1f) == (m_address0 & 0x1f) && !(m_address0 & REG_ADDRESS_DT)) {
			LOGMASKED(LOG_STATE, "Address 0 matched (mode %02X)\n", m_address_mode);
			m_address_status &= ~REG_ADDRESS_STATUS_MJMN;
			addr_matched = true;
		}

		if ((m_address_mode & 3) != 2 && (m_din & 0x1f) == (m_address1 & 0x1f) && !(m_address1 & REG_ADDRESS_DT)) {
			LOGMASKED(LOG_STATE, "Address 1 matched (mode %02X)\n", m_address_mode);
			m_address_status |= REG_ADDRESS_STATUS_MJMN;
			addr_matched = true;
		}

		if (addr_matched) {
			update_state(m_tp_state, talker_primary_state::TPAS);
			if ((m_address_mode & 3) == 1 && m_t_state == talker_state::TIDS)
				update_state(m_t_state, talker_state::TADS);
			if ((m_address_mode & 3) == 3)
				update_state(m_t_state, talker_state::TIDS);
		} else {
			update_state(m_tp_state, talker_primary_state::TPIS);
			update_state(m_t_state, talker_state::TIDS);
		}

	} else if (m_din >= 0x60 && m_din <= 0x7f) {
		LOGMASKED(LOG_CMD, "MSA%d\n", m_din & 0x1f);
		if ((m_address_mode & 3) == 2 && m_t_state == talker_state::TIDS)
			update_state(m_t_state, talker_state::TADS);

		if ((m_address_mode & 3) != 1 && m_rl_state == remote_local_state::LWLS)
			update_state(m_rl_state, remote_local_state::RWLS);

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
	bool f2 = m_atn || m_l_state == listener_state::LACS || m_l_state == listener_state::LADS;

	if ((m_pon || !f2) && m_ah_state != acceptor_handshake_state::AIDS) {
		LOGMASKED(LOG_STATE, "reset AH: pon %d f2 %d\n", m_pon, f2);
		update_state(m_ah_state, acceptor_handshake_state::AIDS);
		set_nrfd(false);
		set_ndac(false);
		return;
	}

	switch (m_ah_state) {
	case acceptor_handshake_state::AIDS:
		if (f2 && !m_pon)
			update_state(m_ah_state, acceptor_handshake_state::ANRS);
		break;
	case acceptor_handshake_state::ANRS:
		set_nrfd(true);
		set_ndac(true);
		m_cpt_flag = false;
		m_apt_flag = false;
		//LOG("m_rdy: %d m_cpt_flag: %d m_apt_flag: %d, m_din_flag %d\n", m_rdy, m_cpt_flag, m_apt_flag, m_din_flag);
		if (m_atn || m_rdy)
			update_state(m_ah_state, acceptor_handshake_state::ACRS);
		break;

	case acceptor_handshake_state::ACRS:
		set_nrfd(false);
		if (m_dav)
			update_state(m_ah_state, acceptor_handshake_state::ADYS);
		break;
	case acceptor_handshake_state::ADYS:
		if (!m_dav) {
			update_state(m_ah_state, acceptor_handshake_state::ACRS);
		} else {
			update_state(m_ah_state, acceptor_handshake_state::ACDS);
		}
		break;
	case acceptor_handshake_state::ACDS:

		if (!m_rdy) {
			update_state(m_ah_state, acceptor_handshake_state::AWNS);
			break;
		}
		m_din = ~m_dio_read_func();
		set_nrfd(true);

		if (m_atn) {
			/* Received command */
			m_din &= ~0x80; // TODO: parity
			handle_command();
			if (!m_cpt_flag && !m_apt_flag)
				update_state(m_ah_state, acceptor_handshake_state::AWNS);
		} else {
			/* Received data */
			m_din_flag = true;
			m_ints1 |= REG_INTS1_BI;
			if (m_eoi)
				m_ints1 |= REG_INTS1_END;
			// TODO: END on EOS
		}
		break;
	case acceptor_handshake_state::AWNS:
		set_ndac(false);
		// TODO: T3 delay, rdy holdoff

		if (!m_dav)
			update_state(m_ah_state, acceptor_handshake_state::ANRS);
		break;
	}
}

void i8291a_device::run_t_fsm()
{
	switch (m_t_state) {
	case talker_state::TIDS:
		m_send_eoi = false;
		m_address_status &= ~REG_ADDRESS_STATUS_TA;
		break;

	case talker_state::TADS:
		m_address_status |= REG_ADDRESS_STATUS_TA;
		if (!m_atn) {
			if (m_tsp_state == talker_serial_poll_state::SPMS)
				update_state(m_t_state, talker_state::SPAS);
			else
				update_state(m_t_state, talker_state::TACS);
		}

		if (m_tp_state == talker_primary_state::TPIS)
			update_state(m_t_state, talker_state::TIDS);
		break;
		// TODO: F4 function
	case talker_state::TACS:
		m_address_status |= REG_ADDRESS_STATUS_TA;
		if (m_atn)
			update_state(m_t_state, talker_state::TADS); // TODO: T2 delay
		break;

	case talker_state::SPAS:
		if (m_atn)
			update_state(m_t_state, talker_state::TADS); // TODO: T2 delay
		break;
	}
}

void i8291a_device::run_tp_fsm()
{
	switch (m_tp_state) {
	case talker_primary_state::TPIS:
		m_address_status &= ~REG_ADDRESS_STATUS_TPAS;
		break;
	case talker_primary_state::TPAS:
		m_address_status |= REG_ADDRESS_STATUS_TPAS;
		break;
	}
}

void i8291a_device::run_tsp_fsm()
{
	if (m_pon || m_ifc)
		update_state(m_tsp_state, talker_serial_poll_state::SPIS);

	switch (m_tsp_state) {
	case talker_serial_poll_state::SPIS:
		break;
	case talker_serial_poll_state::SPMS:
		break;
	}
}

void i8291a_device::run_l_fsm()
{
	switch (m_l_state) {
	case listener_state::LIDS:
		m_address_status &= ~REG_ADDRESS_STATUS_LA;
		break;
	case listener_state::LADS:
		m_address_status |= REG_ADDRESS_STATUS_LA;
		if (m_lp_state == listener_primary_state::LPIS)
			update_state(m_l_state, listener_state::LIDS);
		break;
	case listener_state::LACS:
		break;
	}
}

void i8291a_device::run_dc_fsm()
{
	switch (m_dc_state) {
	case device_clear_state::DCIS:
		break;
	case device_clear_state::DCAS:

		m_ints1 |= REG_INTS1_DEC;
		update_int();
		update_state(m_dc_state, device_clear_state::DCIS);
		break;
	}
}

void i8291a_device::run_dt_fsm()
{
	switch (m_dt_state) {
	case device_trigger_state::DTIS:
		break;
	case device_trigger_state::DTAS:
		m_trig_write_func(true);
		m_trig_write_func(false);
		update_state(m_dt_state, device_trigger_state::DTIS);
		break;
	}
}

void i8291a_device::run_pp_fsm()
{
	if (m_pon)
		update_state(m_pp_state, parallel_poll_state::PPIS);

	switch (m_pp_state) {
	case parallel_poll_state::PPIS:
		if (m_lpe && !m_pon)
			update_state(m_pp_state, parallel_poll_state::PPSS);
		break;

	case parallel_poll_state::PPSS:
		if (!m_lpe) {
			update_state(m_pp_state, parallel_poll_state::PPIS);
			break;
		}

		if (m_atn && m_eoi)
			update_state(m_pp_state, parallel_poll_state::PPAS);
		break;

	case parallel_poll_state::PPAS:
		if (!m_atn || !m_eoi) {
			m_dio_write_func(0xff);
			update_state(m_pp_state, parallel_poll_state::PPSS);
			break;
		}
		if (m_ist == m_pp_sense)
			m_dio_write_func(~(1 << m_pp_line));
		else
			m_dio_write_func(0xff);
		break;
	}
}

void i8291a_device::run_sp_fsm()
{
	switch (m_sp_state) {
	case serial_poll_state::APRS:
		break;
	case serial_poll_state::NPRS:
		break;
	case serial_poll_state::SRQS:
		break;
	}
}

void i8291a_device::run_rl_fsm()
{
	if (m_pon || !m_ren)
		update_state(m_rl_state, remote_local_state::LOCS);

	switch (m_rl_state) {
	case remote_local_state::LOCS:
		break;
	case remote_local_state::LWLS:
		break;
	case remote_local_state::REMS:
		break;
	case remote_local_state::RWLS:
		break;
	}
}

void i8291a_device::run_lp_fsm()
{
	switch (m_lp_state) {
	case listener_primary_state::LPIS:
		m_address_status &= ~REG_ADDRESS_STATUS_LPAS;
		break;
	case listener_primary_state::LPAS:
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

		if ((m_t_state_old == talker_state::TIDS && m_t_state == talker_state::TADS) ||
				(m_t_state_old == talker_state::TADS && m_t_state == talker_state::TIDS) ||
				(m_l_state_old == listener_state::LIDS && m_l_state == listener_state::LADS) ||
				(m_l_state_old == listener_state::LADS && m_l_state == listener_state::LIDS))
			m_ints2 |= REG_INTS2_ADSC;

		if ((m_rl_state_old == remote_local_state::LOCS && m_rl_state == remote_local_state::REMS) ||
				(m_rl_state_old == remote_local_state::REMS && m_rl_state == remote_local_state::LOCS) ||
				(m_rl_state_old == remote_local_state::LWLS && m_rl_state == remote_local_state::RWLS) ||
				(m_rl_state_old == remote_local_state::RWLS && m_rl_state == remote_local_state::LWLS))
			m_ints2 |= REG_INTS2_REMC;

		if ((m_rl_state_old == remote_local_state::LOCS && m_rl_state == remote_local_state::LWLS) ||
				(m_rl_state_old == remote_local_state::REMS && m_rl_state == remote_local_state::RWLS) ||
				(m_rl_state_old == remote_local_state::LWLS && m_rl_state == remote_local_state::LOCS) ||
				(m_rl_state_old == remote_local_state::RWLS && m_rl_state == remote_local_state::REMS))
			m_ints2 |= REG_INTS2_REMC;

		m_rdy = !m_apt_flag && !m_cpt_flag && !m_din_flag;
		update_int();
	} while (m_state_changed);
	m_ignore_ext_signals = false;
}


const char *i8291a_device::get_state_name(device_clear_state state)
{
	switch (state) {
	case device_clear_state::DCIS:
		return "DICS";
	case device_clear_state::DCAS:
		return "DCAS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(device_trigger_state state)
{
	switch (state) {
	case device_trigger_state::DTIS:
		return "DTIS";
	case device_trigger_state::DTAS:
		return "DTAS";
	}
	return "Unknown";
}


const char *i8291a_device::get_state_name(parallel_poll_state state)
{
	switch (state){
	case parallel_poll_state::PPIS:
		return "PPIS";
	case parallel_poll_state::PPSS:
		return "PPSS";
	case parallel_poll_state::PPAS:
		return "PPAS";
	}
	return "Unknown";
}


const char *i8291a_device::get_state_name(serial_poll_state state)
{
	switch (state) {
	case serial_poll_state::APRS:
		return "APRS";
	case serial_poll_state::NPRS:
		return "NPRS";
	case serial_poll_state::SRQS:
		return "SRQS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(remote_local_state state)
{
	switch (state) {
	case remote_local_state::LOCS:
		return "LOCS";
	case remote_local_state::LWLS:
		return "LWLS";
	case remote_local_state::REMS:
		return "REMS";
	case remote_local_state::RWLS:
		return "RWLS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(listener_primary_state state)
{
	switch (state) {
	case listener_primary_state::LPIS:
		return "LPIS";
	case listener_primary_state::LPAS:
		return "LPAS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(source_handshake_state state)
{
	switch (state) {
	case source_handshake_state::SIDS:
		return "SIDS";
	case source_handshake_state::SGNS:
		return "SGNS";
	case source_handshake_state::SDYS:
		return "SDYS";
	case source_handshake_state::STRS:
		return "STRS";
	}
	return "Unknown";
}


const char *i8291a_device::get_state_name(acceptor_handshake_state state)
{
	switch (state) {
	case acceptor_handshake_state::AIDS:
		return "AIDS";
	case acceptor_handshake_state::ANRS:
		return "ANRS";
	case acceptor_handshake_state::ACRS:
		return "ACRS";
	case acceptor_handshake_state::ADYS:
		return "ADYS";
	case acceptor_handshake_state::ACDS:
		return "ACDS";
	case acceptor_handshake_state::AWNS:
		return "AWNS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(talker_state state)
{
	switch (state) {
	case talker_state::TIDS:
		return "TIDS";
	case talker_state::TADS:
		return "TADS";
	case talker_state::TACS:
		return "TACS";
	case talker_state::SPAS:
		return "SPAS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(talker_primary_state state)
{
	switch (state) {
	case talker_primary_state::TPIS:
		return "TPIS";
	case talker_primary_state::TPAS:
		return "TPAS";
	}
	return "Unknown";
}


const char *i8291a_device::get_state_name(listener_state state)
{
	switch (state) {
	case listener_state::LIDS:
		return "LIDS";
	case listener_state::LADS:
		return "LADS";
	case listener_state::LACS:
		return "LACS";
	}
	return "Unknown";
}

const char *i8291a_device::get_state_name(talker_serial_poll_state state)
{
	switch (state) {
	case talker_serial_poll_state::SPIS:
		return "SPIS";
	case talker_serial_poll_state::SPMS:
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

template<typename T> void i8291a_device::update_state(T &name, T state)
{
		if (name != state) {
			m_state_changed = true;
			LOGMASKED(LOG_STATE, "%s: %s -> %s\n", __FUNCTION__,
					get_state_name(name),
					get_state_name(state));
			name = state;
		}
}
