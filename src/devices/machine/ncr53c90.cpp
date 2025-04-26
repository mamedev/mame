// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

/*
 * TODO
 *   - 16 bit dma alignment and last byte handling
 *   - clean up variable naming and protection
 */

#include "emu.h"
#include "ncr53c90.h"

#define LOG_STATE   (1U << 1)
#define LOG_FIFO    (1U << 2)
#define LOG_COMMAND (1U << 3)

#define VERBOSE (0)
#include "logmacro.h"

#define DELAY_HACK

DEFINE_DEVICE_TYPE(NCR53C90, ncr53c90_device, "ncr53c90", "NCR 53C90 SCSI Controller")
DEFINE_DEVICE_TYPE(NCR53C90A, ncr53c90a_device, "ncr53c90a", "NCR 53C90A Advanced SCSI Controller")
DEFINE_DEVICE_TYPE(NCR53C94, ncr53c94_device, "ncr53c94", "NCR 53C94 Advanced SCSI Controller")
DEFINE_DEVICE_TYPE(NCR53C96, ncr53c96_device, "ncr53c96", "NCR 53C96 Advanced SCSI Controller")
DEFINE_DEVICE_TYPE(NCR53CF94, ncr53cf94_device, "ncr53cf94", "NCR 53CF94-2 Fast SCSI Controller") // TODO: differences not emulated
DEFINE_DEVICE_TYPE(NCR53CF96, ncr53cf96_device, "ncr53cf96", "NCR 53CF96-2 Fast SCSI Controller") // TODO: differences not emulated

void ncr53c90_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(ncr53c90_device::tcounter_lo_r), FUNC(ncr53c90_device::tcount_lo_w));
	map(0x1, 0x1).rw(FUNC(ncr53c90_device::tcounter_hi_r), FUNC(ncr53c90_device::tcount_hi_w));
	map(0x2, 0x2).rw(FUNC(ncr53c90_device::fifo_r), FUNC(ncr53c90_device::fifo_w));
	map(0x3, 0x3).rw(FUNC(ncr53c90_device::command_r), FUNC(ncr53c90_device::command_w));
	map(0x4, 0x4).rw(FUNC(ncr53c90_device::status_r), FUNC(ncr53c90_device::bus_id_w));
	map(0x5, 0x5).rw(FUNC(ncr53c90_device::istatus_r), FUNC(ncr53c90_device::timeout_w));
	map(0x6, 0x6).rw(FUNC(ncr53c90_device::seq_step_r), FUNC(ncr53c90_device::sync_period_w));
	map(0x7, 0x7).rw(FUNC(ncr53c90_device::fifo_flags_r), FUNC(ncr53c90_device::sync_offset_w));
	map(0x8, 0x8).rw(FUNC(ncr53c90_device::conf_r), FUNC(ncr53c90_device::conf_w));
	map(0xa, 0xa).w(FUNC(ncr53c90_device::test_w));
	map(0x9, 0x9).w(FUNC(ncr53c90_device::clock_w));
}

uint8_t ncr53c90_device::read(offs_t offset)
{
	switch (offset)
	{
		case 0:  return tcounter_lo_r();
		case 1:  return tcounter_hi_r();
		case 2:  return fifo_r();
		case 3:  return command_r();
		case 4:  return status_r();
		case 5:  return istatus_r();
		case 6:  return seq_step_r();
		case 7:  return fifo_flags_r();
		case 8:  return conf_r();
		default: return 0xff;
	}
}

void ncr53c90_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:  tcount_lo_w(data); break;
		case 1:  tcount_hi_w(data); break;
		case 2:  fifo_w(data); break;
		case 3:  command_w(data); break;
		case 4:  bus_id_w(data); break;
		case 5:  timeout_w(data); break;
		case 6:  sync_period_w(data); break;
		case 7:  sync_offset_w(data); break;
		case 8:  conf_w(data); break;
		case 9:  clock_w(data); break;
		case 10: test_w(data); break;
		default: break;
	}
}

void ncr53c90a_device::map(address_map &map)
{
	ncr53c90_device::map(map);

	map(0xb, 0xb).rw(FUNC(ncr53c90a_device::conf2_r), FUNC(ncr53c90a_device::conf2_w));
}

uint8_t ncr53c90a_device::read(offs_t offset)
{
	if (offset == 11)
		return conf2_r();
	return ncr53c90_device::read(offset);
}

void ncr53c90a_device::write(offs_t offset, uint8_t data)
{
	if (offset == 11)
		return conf2_w(data);
	ncr53c90_device::write(offset, data);
}

void ncr53c94_device::map(address_map &map)
{
	ncr53c90a_device::map(map);

	map(0xc, 0xc).rw(FUNC(ncr53c94_device::conf3_r), FUNC(ncr53c94_device::conf3_w));
	map(0xf, 0xf).w(FUNC(ncr53c94_device::fifo_align_w));
}

uint8_t ncr53c94_device::read(offs_t offset)
{
	if (offset == 12)
		return conf3_r();
	return ncr53c90a_device::read(offset);
}

void ncr53c94_device::write(offs_t offset, uint8_t data)
{
	if (offset == 12)
		conf3_w(data);
	else if (offset == 15)
		fifo_align_w(data);
	else
		ncr53c90a_device::write(offset, data);
}

void ncr53cf94_device::map(address_map &map)
{
	ncr53c94_device::map(map);

	map(0xd, 0xd).rw(FUNC(ncr53cf94_device::conf4_r), FUNC(ncr53cf94_device::conf4_w));
	map(0xe, 0xe).rw(FUNC(ncr53cf94_device::tcounter_hi2_r), FUNC(ncr53cf94_device::tcount_hi2_w));
}

uint8_t ncr53cf94_device::read(offs_t offset)
{
	if (offset == 13)
		return conf4_r();
	else if (offset == 14)
		return tcounter_hi2_r();
	return ncr53c94_device::read(offset);
}

void ncr53cf94_device::write(offs_t offset, uint8_t data)
{
	if (offset == 13)
		conf4_w(data);
	else if (offset == 14)
		tcount_hi2_w(data);
	else
		ncr53c94_device::write(offset, data);
}

ncr53c90_device::ncr53c90_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nscsi_device(mconfig, type, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, tm(nullptr), config(0), status(0), istatus(0), clock_conv(0), sync_offset(0), sync_period(0), bus_id(0)
	, select_timeout(0), seq(0), tcount(0), tcounter(0), tcounter_mask(0xffff), mode(0), fifo_pos(0), command_pos(0), state(0), xfr_phase(0), dma_dir(0), irq(false), drq(false), test_mode(false), stepping(0)
	, m_irq_handler(*this)
	, m_drq_handler(*this)
{
}

ncr53c90a_device::ncr53c90a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ncr53c90_device(mconfig, type, tag, owner, clock)
	, config2(0)
{
}

ncr53c90_device::ncr53c90_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ncr53c90_device(mconfig, NCR53C90, tag, owner, clock)
{
}

ncr53c90a_device::ncr53c90a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ncr53c90a_device(mconfig, NCR53C90A, tag, owner, clock)
{
}

ncr53c94_device::ncr53c94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ncr53c94_device(mconfig, NCR53C94, tag, owner, clock)
{
}

ncr53c94_device::ncr53c94_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ncr53c90a_device(mconfig, type, tag, owner, clock)
	, config3(0)
	, m_busmd(BUSMD_0)
{
}

ncr53c96_device::ncr53c96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ncr53c94_device(mconfig, NCR53C96, tag, owner, clock)
{
}

ncr53cf94_device::ncr53cf94_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ncr53c94_device(mconfig, type, tag, owner, clock)
	, config4(0)
	, family_id(0x04)
	, revision_level(0x02)
{
}

ncr53cf94_device::ncr53cf94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ncr53cf94_device(mconfig, NCR53CF94, tag, owner, clock)
{
}

ncr53cf96_device::ncr53cf96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ncr53cf94_device(mconfig, NCR53CF96, tag, owner, clock)
{
}

void ncr53c90_device::device_start()
{
	nscsi_device::device_start();

	save_item(NAME(command));
	save_item(NAME(config));
	save_item(NAME(status));
	save_item(NAME(istatus));
	save_item(NAME(clock_conv));
	save_item(NAME(sync_offset));
	save_item(NAME(sync_period));
	save_item(NAME(bus_id));
	save_item(NAME(select_timeout));
	save_item(NAME(seq));
	save_item(NAME(fifo));
	save_item(NAME(tcount));
	save_item(NAME(tcounter));
	save_item(NAME(tcounter_mask));
	save_item(NAME(mode));
	save_item(NAME(fifo_pos));
	save_item(NAME(command_pos));
	save_item(NAME(state));
	save_item(NAME(xfr_phase));
	save_item(NAME(dma_dir));
	save_item(NAME(irq));
	save_item(NAME(drq));
	save_item(NAME(test_mode));

	config = 0;
	bus_id = 0;
	select_timeout = 0;
	tm = timer_alloc(FUNC(ncr53c90_device::update_tick), this);
}

void ncr53c90_device::device_reset()
{
	fifo_pos = 0;
	memset(fifo, 0, sizeof(fifo));

	clock_conv = 2;
	sync_period = 5;
	sync_offset = 0;
	seq = 0;
	config &= 7;
	status = 0;
	istatus = 0;
	irq = false;
	m_irq_handler(irq);

	state = IDLE;
	scsi_bus->ctrl_wait(scsi_refid, S_SEL|S_BSY|S_RST, S_ALL);
	drq = false;
	test_mode = false;
	m_drq_handler(drq);

	scsi_bus->ctrl_w(scsi_refid, 0, S_RST);
	tcount = 0;
	tcounter = 0;
	tcounter_mask = 0xffff;

	reset_disconnect();
}

void ncr53c90_device::reset_disconnect()
{
	scsi_bus->ctrl_w(scsi_refid, 0, ~S_RST);

	command_pos = 0;
	memset(command, 0, sizeof(command));
	mode = MODE_D;
}

void ncr53c90_device::scsi_ctrl_changed()
{
	uint32_t ctrl = scsi_bus->ctrl_r();
	if(ctrl & S_RST) {
		LOG("scsi bus reset\n");
		return;
	}

	// disallow further recursion from here
	if(!stepping)
		step(false);
}

TIMER_CALLBACK_MEMBER(ncr53c90_device::update_tick)
{
	step(true);
}

void ncr53c90_device::step(bool timeout)
{
	uint32_t ctrl = scsi_bus->ctrl_r();
	uint32_t data = scsi_bus->data_r();
	uint8_t c     = command[0] & 0x7f;

	LOGMASKED(LOG_STATE, "state=%d.%d %s @ %s\n",
		state & STATE_MASK, (state & SUB_MASK) >> SUB_SHIFT,
		timeout ? "timeout" : "change", machine().time().to_string());

	stepping++;

	if(mode == MODE_I && !(ctrl & S_BSY)) {
		state = IDLE;
		istatus |= I_DISCONNECT;
		reset_disconnect();
		check_irq();
	}
	switch(state & SUB_MASK ? state & SUB_MASK : state & STATE_MASK) {
	case IDLE:
		break;

	case BUSRESET_WAIT_INT:
		state = IDLE;
		scsi_bus->ctrl_w(scsi_refid, 0, S_RST);
		reset_disconnect();

		if (!(config & 0x40)) {
			LOG("SCSI reset interrupt\n");
			istatus |= I_SCSI_RESET;
			check_irq();
		}
		break;

	case ARB_COMPLETE << SUB_SHIFT: {
		if(!timeout)
			break;

		int win;
		for(win=7; win>=0 && !(data & (1<<win)); win--) {};
		if(win != scsi_id) {
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
			fatalerror("ncr53c90_device::step need to wait for bus free\n");
		}
		state = (state & STATE_MASK) | (ARB_ASSERT_SEL << SUB_SHIFT);
		scsi_bus->ctrl_w(scsi_refid, S_SEL, S_SEL);
		delay(6);
		break;
	}

	case ARB_ASSERT_SEL << SUB_SHIFT:
		if(!timeout)
			break;

		scsi_bus->data_w(scsi_refid, (1<<scsi_id) | (1<<bus_id));
		state = (state & STATE_MASK) | (ARB_SET_DEST << SUB_SHIFT);
		delay_cycles(4);
		break;

	case ARB_SET_DEST << SUB_SHIFT:
		if(!timeout)
			break;

		state = (state & STATE_MASK) | (ARB_RELEASE_BUSY << SUB_SHIFT);
		scsi_bus->ctrl_w(scsi_refid, c == CD_SELECT_ATN || c == CD_SELECT_ATN_STOP ? S_ATN : 0, S_ATN|S_BSY);
		delay(2);
		break;

	case ARB_RELEASE_BUSY << SUB_SHIFT:
		if(!timeout)
			break;

		if(ctrl & S_BSY) {
			state = (state & STATE_MASK) | (ARB_DESKEW_WAIT << SUB_SHIFT);
			if(c == CD_RESELECT)
				scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
			delay_cycles(2);
		} else {
			state = (state & STATE_MASK) | (ARB_TIMEOUT_BUSY << SUB_SHIFT);
#ifdef DELAY_HACK
			delay(1);
#else
			delay(8192*select_timeout);
#endif
		}
		break;

	case ARB_DESKEW_WAIT << SUB_SHIFT:
		if(!timeout)
			break;

		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_SEL);

		if(c == CD_RESELECT) {
			LOG("mode switch to Target\n");
			mode = MODE_T;
		} else {
			LOG("mode switch to Initiator\n");
			mode = MODE_I;
		}
		state &= STATE_MASK;
		step(true);
		break;

	case ARB_TIMEOUT_BUSY << SUB_SHIFT:
		if(timeout) {
			scsi_bus->data_w(scsi_refid, 0);
			LOG("select timeout\n");
			state = (state & STATE_MASK) | (ARB_TIMEOUT_ABORT << SUB_SHIFT);
			delay(1000);
		} else if(ctrl & S_BSY) {
			state = (state & STATE_MASK) | (ARB_DESKEW_WAIT << SUB_SHIFT);
			if(c == CD_RESELECT)
				scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
			delay_cycles(2);
		}
		break;

	case ARB_TIMEOUT_ABORT << SUB_SHIFT:
		if(!timeout)
			break;

		if(ctrl & S_BSY) {
			state = (state & STATE_MASK) | (ARB_DESKEW_WAIT << SUB_SHIFT);
			if(c == CD_RESELECT)
				scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
			delay_cycles(2);
		} else {
			scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
			state = IDLE;
			istatus |= I_DISCONNECT;
			reset_disconnect();
			check_irq();
		}
		break;

	case SEND_WAIT_SETTLE << SUB_SHIFT:
		if(!timeout)
			break;

		state = (state & STATE_MASK) | (SEND_WAIT_REQ_0 << SUB_SHIFT);
		step(false);
		break;

	case SEND_WAIT_REQ_0 << SUB_SHIFT:
		if(ctrl & S_REQ)
			break;
		state = state & STATE_MASK;
		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		step(false);
		break;

	case RECV_WAIT_REQ_1 << SUB_SHIFT:
		if(!(ctrl & S_REQ))
			break;

		state = (state & STATE_MASK) | (RECV_WAIT_SETTLE << SUB_SHIFT);
		delay_cycles(sync_period);
		break;

	case RECV_WAIT_SETTLE << SUB_SHIFT:
		if(!timeout)
			break;

		if((state & STATE_MASK) != INIT_XFR_RECV_PAD)
		{
			fifo_push(scsi_bus->data_r());
			// in async mode data in phase in initiator mode, tcount is decremented on ACKO, not DACK
			if ((mode == MODE_I) && (sync_offset == 0) && ((ctrl & S_PHASE_MASK) == S_PHASE_DATA_IN))
			{
				LOGMASKED(LOG_FIFO, "decrement_tcounter data in async, phase %02x (tcounter=%d)\n", (ctrl & S_PHASE_MASK), tcounter);
				decrement_tcounter();
				check_drq();
			}
		}
		scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
		state = (state & STATE_MASK) | (RECV_WAIT_REQ_0 << SUB_SHIFT);
		step(false);
		break;

	case RECV_WAIT_REQ_0 << SUB_SHIFT:
		if(ctrl & S_REQ)
			break;
		state = state & STATE_MASK;
		step(false);
		break;

	case DISC_SEL_ARBITRATION_INIT:
		if(!timeout)
			break;

		state = DISC_SEL_ARBITRATION;
		step(false);
		break;

	case DISC_SEL_ARBITRATION:
		// wait until a command is in the fifo
		if (!fifo_pos) {
			// this sequence isn't documented for initiator selection, but
			// it makes macqd700 happy and may be consistent with target
			// selection sequences
			seq = 1;
			// dma starts after bus arbitration/selection is complete
			check_drq();
			break;
		}

		if(c == CD_SELECT) {
			state = DISC_SEL_WAIT_REQ;
		} else
			state = DISC_SEL_ATN_WAIT_REQ;

		scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
		step(false);
		break;

	case DISC_SEL_ATN_WAIT_REQ:
		if(!(ctrl & S_REQ))
			break;
		if((ctrl & S_PHASE_MASK) != S_PHASE_MSG_OUT) {
			function_complete();
			break;
		}
		if(c == CD_SELECT_ATN)
			scsi_bus->ctrl_w(scsi_refid, 0, S_ATN);
		state = DISC_SEL_ATN_SEND_BYTE;
		send_byte();
		break;

	case DISC_SEL_ATN_SEND_BYTE:
		if(c == CD_SELECT_ATN_STOP) {
			seq = 2;
			function_bus_complete();
		} else {
			state = DISC_SEL_WAIT_REQ;
			step(false);
		}
		break;

	case DISC_SEL_WAIT_REQ:
		if(!(ctrl & S_REQ))
			break;
		if((ctrl & S_PHASE_MASK) != S_PHASE_COMMAND) {
			if((!dma_command || (status & S_TC0)) && !fifo_pos)
				seq = 4;
			else
				seq = 2;
			scsi_bus->ctrl_wait(scsi_refid, 0, S_REQ);
			function_bus_complete();
			break;
		}
		if(!fifo_pos)
			break;
		if(seq < 3)
			seq = 3;
		state = DISC_SEL_SEND_BYTE;
		send_byte();
		break;

	case DISC_SEL_SEND_BYTE:
		if((!dma_command || (status & S_TC0)) && !fifo_pos)
			seq = 4;

		state = DISC_SEL_WAIT_REQ;
		step(false);
		break;

	case INIT_CPT_RECV_BYTE_ACK:
		state = INIT_CPT_RECV_WAIT_REQ;
		scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		step(false);
		break;

	case INIT_CPT_RECV_WAIT_REQ:
		if(!(ctrl & S_REQ))
			break;

		if((ctrl & S_PHASE_MASK) != S_PHASE_MSG_IN) {
			command_pos = 0;
			bus_complete();
		} else {
			state = INIT_CPT_RECV_BYTE_NACK;
			recv_byte();
		}
		break;

	case INIT_CPT_RECV_BYTE_NACK:
		function_complete();
		break;

	case INIT_MSG_WAIT_REQ:
		if((ctrl & (S_REQ|S_BSY)) == S_BSY)
			break;
		bus_complete();
		break;

	case INIT_XFR:
		switch(xfr_phase) {
		case S_PHASE_DATA_OUT:
		case S_PHASE_COMMAND:
		case S_PHASE_MSG_OUT: {
			state = INIT_XFR_SEND_BYTE;

			// can't send if the fifo is empty
			if (fifo_pos == 0)
				break;

			// determine remaining bytes to transfer, accounting for fifo level plus potential incoming DMA bytes
			int remaining_bytes = fifo_pos + (dma_command ? tcounter : 0);

			// if it's the last message byte, deassert ATN before sending
			if (xfr_phase == S_PHASE_MSG_OUT && remaining_bytes == 1)
				scsi_bus->ctrl_w(scsi_refid, 0, S_ATN);

			send_byte();
			break;
		}

		case S_PHASE_DATA_IN:
		case S_PHASE_STATUS:
		case S_PHASE_MSG_IN:
			// can't receive if the fifo is full
			if (fifo_pos == 16)
				break;

			// if it's the last message byte, ACK remains asserted, terminate with function_complete()
			state = (xfr_phase == S_PHASE_MSG_IN && (!dma_command || tcounter == 1)) ? INIT_XFR_RECV_BYTE_NACK : INIT_XFR_RECV_BYTE_ACK;

			recv_byte();
			break;

		default:
			LOG("xfer on phase %d\n", scsi_bus->ctrl_r() & S_PHASE_MASK);
			function_complete();
			break;
		}
		break;

	case INIT_XFR_WAIT_REQ:
		if(!(ctrl & S_REQ))
			break;

		// check for command complete
		if ((dma_command && (status & S_TC0) && (dma_dir == DMA_IN || fifo_pos == 0)) // dma in/out: transfer count == 0
		|| (!dma_command && (xfr_phase & S_INP) == 0 && fifo_pos == 0)      // non-dma out: fifo empty
		|| (!dma_command && (xfr_phase & S_INP) == S_INP && fifo_pos == 1)) // non-dma in: every byte
			state = INIT_XFR_BUS_COMPLETE;
		else
			// check for phase change
			if((ctrl & S_PHASE_MASK) != xfr_phase) {
				command_pos = 0;
				state = INIT_XFR_BUS_COMPLETE;
			} else {
				state = INIT_XFR;
			}
		step(false);
		break;

	case INIT_XFR_SEND_BYTE:
		state = INIT_XFR_WAIT_REQ;
		step(false);
		break;

	case INIT_XFR_RECV_BYTE_ACK:
		state = INIT_XFR_WAIT_REQ;
		scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		step(false);
		break;

	case INIT_XFR_RECV_BYTE_NACK:
		state = INIT_XFR_FUNCTION_COMPLETE;
		step(false);
		break;

	case INIT_XFR_FUNCTION_COMPLETE:
		// wait for dma transfer to complete and fifo to drain
		if (dma_command && (!(status & S_TC0) || fifo_pos))
			break;
		function_complete();
		break;

	case INIT_XFR_BUS_COMPLETE:
		// wait for dma transfer to complete and fifo to drain
		// (FIFO may still contain one residual byte if enabled for 16-bit DMA)
		if (dma_command && drq)
			break;
		bus_complete();
		break;

	case INIT_XFR_SEND_PAD_WAIT_REQ:
		if(!(ctrl & S_REQ))
			break;

		if((ctrl & S_PHASE_MASK) != xfr_phase) {
			command_pos = 0;
			bus_complete();
		} else {
			state = INIT_XFR_SEND_PAD;
			send_byte();
		}
		break;

	case INIT_XFR_SEND_PAD:
		decrement_tcounter();
		if(!(status & S_TC0)) {
			state = INIT_XFR_SEND_PAD_WAIT_REQ;
			step(false);
		} else
			function_complete();
		break;

	case INIT_XFR_RECV_PAD_WAIT_REQ:
		if(!(ctrl & S_REQ))
			break;

		if((ctrl & S_PHASE_MASK) != xfr_phase) {
			command_pos = 0;
			bus_complete();
		} else {
			state = INIT_XFR_RECV_PAD;
			recv_byte();
		}
		break;

	case INIT_XFR_RECV_PAD:
		decrement_tcounter();
		if(!(status & S_TC0)) {
			state = INIT_XFR_RECV_PAD_WAIT_REQ;
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
			step(false);
		} else
			function_complete();
		break;

	default:
		LOG("step() unexpected state %d.%d\n",
			state & STATE_MASK, (state & SUB_MASK) >> SUB_SHIFT);
		exit(0);
	}

	assert(stepping > 0);
	stepping--;
}

void ncr53c90_device::send_byte()
{
	state = (state & STATE_MASK) | (SEND_WAIT_SETTLE << SUB_SHIFT);
	if((state & STATE_MASK) != INIT_XFR_SEND_PAD) {
		if(!fifo_pos)
			fatalerror("ncr53c90_device::send_byte - !fifo_pos\n");
		scsi_bus->data_w(scsi_refid, fifo_pop());
	}
	else
		scsi_bus->data_w(scsi_refid, 0);

	scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
	scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
	delay_cycles(sync_period);
}

void ncr53c90_device::recv_byte()
{
	scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
	state = (state & STATE_MASK) | (RECV_WAIT_REQ_1 << SUB_SHIFT);
	step(false);
}

void ncr53c90_device::function_bus_complete()
{
	LOG("function_bus_complete\n");
	state = IDLE;
	istatus |= I_FUNCTION|I_BUS;
	dma_set(DMA_NONE);
	check_drq();
	check_irq();
}

void ncr53c90_device::function_complete()
{
	LOG("function_complete\n");
	state = IDLE;
	istatus |= I_FUNCTION;
	dma_set(DMA_NONE);
	check_drq();
	check_irq();
}

void ncr53c90_device::bus_complete()
{
	LOG("bus_complete\n");
	state = IDLE;
	istatus |= I_BUS;
	dma_set(DMA_NONE);
	check_drq();
	check_irq();
}

void ncr53c90_device::delay(int cycles)
{
	cycles *= clock_conv ? clock_conv : 8;
	tm->adjust(clocks_to_attotime(cycles));
}

void ncr53c90_device::delay_cycles(int cycles)
{
	tm->adjust(clocks_to_attotime(cycles));
}

uint8_t ncr53c90_device::tcounter_lo_r()
{
	LOG("tcounter_lo_r %02x (%s)\n", tcounter & 0xff, machine().describe_context());
	return tcounter;
}

void ncr53c90_device::tcount_lo_w(uint8_t data)
{
	tcount = (tcount & ~uint32_t(0xff)) | data;
	LOG("tcount_lo_w %02x (%s)\n", data, machine().describe_context());
}

uint8_t ncr53c90_device::tcounter_hi_r()
{
	LOG("tcounter_hi_r %02x (%s)\n", (tcounter >> 8) & 0xff, machine().describe_context());
	return tcounter >> 8;
}

void ncr53c90_device::tcount_hi_w(uint8_t data)
{
	tcount = (tcount & ~uint32_t(0xff00)) | (uint32_t(data) << 8);
	LOG("tcount_hi_w %02x (%s)\n", data, machine().describe_context());
}

uint8_t ncr53c90_device::fifo_pop()
{
	uint8_t r = fifo[0];
	if(fifo_pos) {
		fifo_pos--;
		memmove(fifo, fifo+1, fifo_pos);
	}
	check_drq();
	return r;
}

void ncr53c90_device::fifo_push(uint8_t val)
{
	LOGMASKED(LOG_FIFO, "Push %02x to FIFO at position %d\n", val, fifo_pos);
	if(fifo_pos != 16)
		fifo[fifo_pos++] = val;
	check_drq();
}

uint8_t ncr53c90_device::fifo_r()
{
	uint8_t r;
	if(fifo_pos) {
		r = fifo[0];
		fifo_pos--;
		memmove(fifo, fifo+1, fifo_pos);
	} else
		r = 0;

	check_drq();
	LOGMASKED(LOG_FIFO, "fifo_r 0x%02x fifo_pos %d (%s)\n", r, fifo_pos, machine().describe_context());
	return r;
}

void ncr53c90_device::fifo_w(uint8_t data)
{
	LOGMASKED(LOG_FIFO, "fifo_w 0x%02x fifo_pos %d (%s)\n", data, fifo_pos, machine().describe_context());
	if(fifo_pos != 16)
		fifo[fifo_pos++] = data;

	check_drq();
	step(false);
}

uint8_t ncr53c90_device::command_r()
{
	LOG("command_r (%s)\n", machine().describe_context());
	return command[0];
}

void ncr53c90_device::command_w(uint8_t data)
{
	LOG("command_w %02x command_pos %d (%s)\n", data, command_pos, machine().describe_context());
	if(command_pos == 2) {
		status |= S_GROSS_ERROR;
		check_irq();
		return;
	}
	/*
	 * Note the RESET chip and RESET SCSI Bus commands execute as soon as they are loaded into
	 * the top of the Command Register.
	 */
	if((data & 0x7f) == CM_RESET || (data & 0x7f) == CM_RESET_BUS)
		command_pos = 0;

	command[command_pos++] = data;
	if(command_pos == 1)
		start_command();
}

void ncr53c90_device::command_pop_and_chain()
{
	if(command_pos) {
		command_pos--;
		if(command_pos) {
			command[0] = command[1];
			start_command();
		}
	}
}

void ncr53c90_device::load_tcounter()
{
	LOGMASKED(LOG_COMMAND, "DMA command: tcounter reloaded to %d\n", tcount & tcounter_mask);
	tcounter = tcount & tcounter_mask;

	// clear transfer count zero flag when counter is reloaded
	status &= ~S_TC0;
}

void ncr53c90_device::start_command()
{
	uint8_t c = command[0] & 0x7f;
	if(!check_valid_command(c)) {
		LOG("invalid command %02x\n", command[0]);
		istatus |= I_ILLEGAL;
		check_irq();
		return;
	}

	// for dma commands, reload transfer counter
	dma_command = command[0] & 0x80;
	if (dma_command)
	{
		load_tcounter();
	}
	else
	{
		tcounter = 0;
	}

	switch(c) {
	case CM_NOP:
		LOGMASKED(LOG_COMMAND, "NOP\n");
		command_pop_and_chain();
		break;

	case CM_FLUSH_FIFO:
		LOGMASKED(LOG_COMMAND, "Flush FIFO\n");
		fifo_pos = 0;
		command_pop_and_chain();
		break;

	case CM_RESET:
		LOGMASKED(LOG_COMMAND, "Reset chip\n");
		device_reset();
		break;

	case CM_RESET_BUS:
		LOGMASKED(LOG_COMMAND, "Reset SCSI bus\n");
		state = BUSRESET_WAIT_INT;
		scsi_bus->ctrl_w(scsi_refid, S_RST, S_RST);
		delay(130);
		break;

	case CD_RESELECT:
		LOGMASKED(LOG_COMMAND, "Reselect sequence\n");
		state = DISC_REC_ARBITRATION;
		arbitrate();
		break;

	case CD_SELECT:
	case CD_SELECT_ATN:
	case CD_SELECT_ATN_STOP:
		LOGMASKED(LOG_COMMAND,
			(c == CD_SELECT) ? "Select without ATN sequence\n" :
			(c == CD_SELECT_ATN) ? "Select with ATN sequence\n" :
			"Select with ATN and stop sequence\n");
		seq = 0;
		state = DISC_SEL_ARBITRATION_INIT;
		dma_set(dma_command ? DMA_OUT : DMA_NONE);
		arbitrate();
		break;

	case CD_ENABLE_SEL:
		LOGMASKED(LOG_COMMAND, "Enable selection/reselection\n");
		command_pop_and_chain();
		break;

	case CD_DISABLE_SEL:
		LOGMASKED(LOG_COMMAND, "Disable selection/reselection\n");
		function_complete();
		command_pop_and_chain();
		break;

	case CI_XFER:
		LOGMASKED(LOG_COMMAND, "Transfer information\n");
		state = INIT_XFR;
		xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;
		dma_set(dma_command ? ((xfr_phase & S_INP) ? DMA_IN : DMA_OUT) : DMA_NONE);
		check_drq();
		step(false);
		break;

	case CI_COMPLETE:
		LOGMASKED(LOG_COMMAND, "Initiator command complete sequence\n");
		state = INIT_CPT_RECV_BYTE_ACK;
		dma_set(dma_command ? DMA_IN : DMA_NONE);
		recv_byte();
		break;

	case CI_MSG_ACCEPT:
		LOGMASKED(LOG_COMMAND, "Message accepted\n");
		state = INIT_MSG_WAIT_REQ;
		// It's undocumented what the sequence register should contain after a message accept
		// command, but the InterPro boot code expects it to be non-zero; setting it to an
		// arbirary 1 here makes InterPro happy. Also in the InterPro case (perhaps typical),
		// after ACK is asserted the device disconnects and the INIT_MSG_WAIT_REQ state is never
		// entered, meaning we end up with I_DISCONNECT instead of I_BUS interrupt status.
		seq = 2;
		scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		step(false);
		break;

	case CI_PAD:
		LOGMASKED(LOG_COMMAND, "Transfer pad\n");
		xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;
		if(xfr_phase & S_INP)
			state = INIT_XFR_RECV_PAD_WAIT_REQ;
		else
			state = INIT_XFR_SEND_PAD_WAIT_REQ;
		scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		step(false);
		break;

	case CI_SET_ATN:
		LOGMASKED(LOG_COMMAND, "Set ATN\n");
		scsi_bus->ctrl_w(scsi_refid, S_ATN, S_ATN);
		command_pop_and_chain();
		break;

	case CI_RESET_ATN:
		LOGMASKED(LOG_COMMAND, "Reset ATN\n");
		scsi_bus->ctrl_w(scsi_refid, 0, S_ATN);
		command_pop_and_chain();
		break;

	default:
		fatalerror("ncr53c90_device::start_command unimplemented command %02x\n", c);
	}
}

bool ncr53c90_device::check_valid_command(uint8_t cmd)
{
	int subcmd = cmd & 15;
	switch((cmd >> 4) & 7) {
	case 0: return subcmd <= 3;
	case 4: return mode == MODE_D && subcmd <= 5;
	case 2: return mode == MODE_T && subcmd <= 11 && subcmd != 6;
	case 1: return mode == MODE_I && (subcmd <= 2 || subcmd == 8 || subcmd == 10);
	}
	return false;
}

void ncr53c90_device::arbitrate()
{
	state = (state & STATE_MASK) | (ARB_COMPLETE << SUB_SHIFT);
	scsi_bus->data_w(scsi_refid, 1 << scsi_id);
	scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
	delay(11);
}

void ncr53c90_device::check_irq()
{
	bool oldirq = irq;
	irq = istatus != 0;
	if(irq != oldirq)
		m_irq_handler(irq);

}

uint8_t ncr53c90_device::status_r()
{
	uint32_t ctrl = scsi_bus->ctrl_r();
	uint8_t res = status | (ctrl & S_MSG ? 4 : 0) | (ctrl & S_CTL ? 2 : 0) | (ctrl & S_INP ? 1 : 0);
	//LOG("status_r %02x (%s)\n", res, machine().describe_context());

	return res;
}

void ncr53c90_device::bus_id_w(uint8_t data)
{
	bus_id = data & 7;
	LOG("bus_id=%d\n", bus_id);
}

uint8_t ncr53c90_device::istatus_r()
{
	uint8_t res = istatus;

	if (!machine().side_effects_disabled())
	{
		if (irq)
		{
			status &= ~(S_GROSS_ERROR | S_PARITY | S_TCC);
			istatus = 0;
			seq = 0;
		}
		check_irq();
		if(res)
			command_pop_and_chain();

		LOG("istatus_r %02x (%s)\n", res, machine().describe_context());
	}
	return res;
}

void ncr53c90_device::timeout_w(uint8_t data)
{
	LOG("timeout_w 0x%02x\n", data);
	select_timeout = data;
}

uint8_t ncr53c90_device::seq_step_r()
{
	LOG("seq_step_r %d (%s)\n", seq, machine().describe_context());
	return seq;
}

void ncr53c90_device::sync_period_w(uint8_t data)
{
	sync_period = data & 0x1f;
}

uint8_t ncr53c90_device::fifo_flags_r()
{
	return fifo_pos;
}

void ncr53c90_device::sync_offset_w(uint8_t data)
{
	sync_offset = data & 0x0f;
}

uint8_t ncr53c90_device::conf_r()
{
	return config;
}

void ncr53c90_device::conf_w(uint8_t data)
{
	config = data;
	scsi_id = data & 7;

	// test mode can only be cleared by hard/soft reset
	if (data & 0x8)
		test_mode = true;
}

void ncr53c90_device::test_w(uint8_t data)
{
	if (test_mode)
		logerror("test_w %d (%s) - test mode not implemented\n", data, machine().describe_context());
}

void ncr53c90_device::clock_w(uint8_t data)
{
	clock_conv = data & 0x07;
}

void ncr53c90_device::dma_set(int dir)
{
	dma_dir = dir;
}

void ncr53c90_device::dma_w(uint8_t val)
{
	LOGMASKED(LOG_FIFO, "dma_w 0x%02x fifo_pos %d tcounter %d (%s)\n", val, fifo_pos, tcounter, machine().describe_context());
	fifo_push(val);
	decrement_tcounter();
	check_drq();
	step(false);
}

uint8_t ncr53c90_device::dma_r()
{
	if (machine().side_effects_disabled())
		return fifo[0];

	uint8_t r = fifo_pop();

	if ((sync_offset != 0) || ((scsi_bus->ctrl_r() & S_PHASE_MASK) != S_PHASE_DATA_IN))
	{
		decrement_tcounter();
	}
	check_drq();
	step(false);
	return r;
}

void ncr53c90_device::check_drq()
{
	bool drq_state = drq;

	switch (dma_dir) {
	case DMA_NONE:
		drq_state = false;
		break;

	case DMA_IN: // device to memory
		if (sync_offset == 0)
			drq_state = (fifo_pos > 0);
		else
			drq_state = !(status & S_TC0) && fifo_pos;
		break;

	case DMA_OUT: // memory to device
		drq_state = !(status & S_TC0) && fifo_pos < 16;
		break;
	}

	if (drq_state != drq) {
		drq = drq_state;
		m_drq_handler(drq);
	}
}

void ncr53c90_device::decrement_tcounter(int count)
{
	if (!dma_command)
		return;

	// If tcounter is 0 but TC0 is not set yet then it should mean tcount is also 0.
	// A tcount of 0 specifies the maximum length count (65536) so this should wrap
	// from 0 to 65535 only once.
	if (!(status & S_TC0))
		tcounter = (tcounter - count) & tcounter_mask;
	else
		tcounter = 0;

	if (tcounter == 0)
		status |= S_TC0;

	check_drq();
}

/*
 * According to the NCR 53C90A, 53C90B data book (http://bitsavers.org/pdf/ncr/scsi/NCR53C90ab.pdf),
 * the following are the differences from the 53C90:
 *
 *   - Supports three-byte message exchange SCSI-2 tagged queueing
 *   - Added select with ATN3 command
 *   - Added target DMA abort command
 *   - Added interrupt polling bit
 *   - Added second configuration register
 *   - Improved immunity to cable impedance mismatches and improper termination
 *   - Tri-state DMA request output
 *   - Cut leakage current on SCSI input pins when powered off
 *   - Relaxed register timings
 *   - Relaxed DMA timings
 *   - Relaxed CLK duty cycle
 *   - Lengthened read data access time
 *   - NOP required less often
 */

void ncr53c90a_device::device_start()
{
	save_item(NAME(config2));

	config2 = 0;

	ncr53c90_device::device_start();
}

void ncr53c90a_device::device_reset()
{
	config2 = 0;

	ncr53c90_device::device_reset();
}

uint8_t ncr53c90a_device::status_r()
{
	uint32_t ctrl = scsi_bus->ctrl_r();
	uint8_t res = (irq ? S_INTERRUPT : 0) | status | (ctrl & S_MSG ? 4 : 0) | (ctrl & S_CTL ? 2 : 0) | (ctrl & S_INP ? 1 : 0);
	//LOG("status_r %02x (%s)\n", res, machine().describe_context());
	if (irq && !machine().side_effects_disabled())
		status &= ~(S_GROSS_ERROR | S_PARITY | S_TCC);
	return res;
}

bool ncr53c90a_device::check_valid_command(uint8_t cmd)
{
	int subcmd = cmd & 15;
	switch ((cmd >> 4) & 7) {
	case 0: return subcmd <= 3 || (mode == MODE_T && subcmd == 4);
	case 4: return mode == MODE_D && subcmd <= 6;
	case 2: return mode == MODE_T && subcmd <= 11 && subcmd != 6;
	case 1: return mode == MODE_I && (subcmd <= 2 || subcmd == 8 || subcmd == 10 || subcmd == 11);
	}
	return false;
}

void ncr53c94_device::device_start()
{
	save_item(NAME(config3));

	config3 = 0;

	ncr53c90a_device::device_start();
}

void ncr53c94_device::device_reset()
{
	config3 = 0;

	ncr53c90a_device::device_reset();
}

u16 ncr53c94_device::dma16_r()
{
	// check fifo underflow
	if (fifo_pos < 2)
		return dma_r() | 0xff00;

	// pop two bytes from fifo
	u16 const data = fifo[0] | (fifo[1] << 8);
	if (!machine().side_effects_disabled())
	{
		fifo_pos -= 2;
		memmove(fifo, fifo + 2, fifo_pos);

		// update drq
		if ((sync_offset != 0) || ((scsi_bus->ctrl_r() & S_PHASE_MASK) != S_PHASE_DATA_IN))
		{
			decrement_tcounter(2);
		}
		check_drq();

		step(false);
	}

	return data;
}

void ncr53c94_device::dma16_w(u16 data)
{
	// check fifo overflow
	if (fifo_pos > 14 || tcounter == 1)
	{
		dma_w(data & 0x00ff);
		return;
	}

	LOGMASKED(LOG_FIFO, "dma16_w 0x%04x fifo_pos %d tcounter %d (%s)\n", data, fifo_pos, tcounter, machine().describe_context());

	// push two bytes into fifo
	fifo[fifo_pos++] = data;
	fifo[fifo_pos++] = data >> 8;

	// update drq
	decrement_tcounter(2);
	check_drq();

	step(false);
}

void ncr53c94_device::check_drq()
{
	if (m_busmd != BUSMD_0)
	{
		bool drq_state = drq;

		switch (dma_dir) {
		case DMA_NONE:
			drq_state = false;
			break;

		case DMA_IN: // device to memory (optionally save last remaining byte for processor)
			if (sync_offset == 0)
				drq_state = fifo_pos > (BIT(config3, 2) || !(status & S_TC0) ? 1 : 0);
			else
				drq_state = !(status & S_TC0) && fifo_pos > 1;
			break;

		case DMA_OUT: // memory to device
			drq_state = !(status & S_TC0) && fifo_pos < 15;
			break;
		}

		if (drq_state != drq) {
			drq = drq_state;
			m_drq_handler(drq);
		}
	}
	else
		ncr53c90_device::check_drq();
}

void ncr53cf94_device::device_start()
{
	save_item(NAME(config4));

	config4 = 0;

	ncr53c94_device::device_start();
}

void ncr53cf94_device::device_reset()
{
	config4 = 0;

	ncr53c94_device::device_reset();
}

void ncr53cf94_device::load_tcounter()
{
	ncr53c94_device::load_tcounter();

	// ID may be read by executing DMA NOP command twice, first with the features bit clear and then with it set
	if ((config2 & S2FE) == 0)
		tcount = (1 << 23) | (family_id << 19) | (revision_level << 16) | (tcount & 0xffff);
}

void ncr53cf94_device::conf2_w(uint8_t data)
{
	tcounter_mask = (data & S2FE) ? 0xffffff : 0xffff;
	config2 = data;
}

uint8_t ncr53cf94_device::tcounter_hi2_r()
{
	LOG("tcounter_hi2_r %02x (%s)\n", (tcounter >> 16) & 0xff, machine().describe_context());
	return tcounter >> 16;
}

void ncr53cf94_device::tcount_hi2_w(uint8_t data)
{
	tcount = (tcount & ~uint32_t(0xff0000)) | (uint32_t(data) << 16);
	LOG("tcount_hi2_w %02x (%s)\n", data, machine().describe_context());
}
