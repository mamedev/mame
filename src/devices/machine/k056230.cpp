// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Ariane Fugmann
/**************************************************************************************************

Konami IC 056230 (LANC)

Device Notes:
-The custom IC itself
-64k shared ram
-LS161 4-bit binary counter
-PAL(056787) for racinfrc's sub board and plygonet.cpp
-PAL(056787A) for zr107.cpp, gticlub.cpp and thunderh's I/O board
-HYC2485S RS485 transceiver

TODO:
- nearly everything, currently a wrapper to make games happy and don't fail POSTs
- LANC part name for konami/viper.cpp

**************************************************************************************************/

#include "emu.h"
#include "k056230.h"
#include "emuopts.h"

#define LOG_REG_READS   (1U << 1)
#define LOG_REG_WRITES  (1U << 2)
#define LOG_RAM_READS   (1U << 3)
#define LOG_RAM_WRITES  (1U << 4)
#define LOG_UNKNOWNS    (1U << 5)
#define LOG_ALL (LOG_REG_READS | LOG_REG_WRITES | LOG_RAM_READS | LOG_RAM_WRITES | LOG_UNKNOWNS)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(K056230, k056230_device, "k056230", "K056230 LANC")
DEFINE_DEVICE_TYPE(K056230_VIPER, k056230_viper_device, "k056230_viper", "Konami Viper LANC")

k056230_device::k056230_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_ram(*this, "lanc_ram", 0x800U * 4, ENDIANNESS_BIG)
	, m_irq_cb(*this)
{
}

k056230_device::k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: k056230_device(mconfig, K056230, tag, owner, clock)
{
	// prepare "filenames"
	m_localhost = util::string_format("socket.%s:%s", mconfig.options().comm_localhost(), mconfig.options().comm_localport());
	m_remotehost = util::string_format("socket.%s:%s", mconfig.options().comm_remotehost(), mconfig.options().comm_remoteport());
}

void k056230_device::device_start()
{
	// state saving
	save_item(NAME(m_irq_state));
	save_item(NAME(m_ctrl_reg));
	save_item(NAME(m_status));

	save_item(NAME(m_linkenable));
	save_item(NAME(m_linkid));
	save_item(NAME(m_txmode));
}

void k056230_device::device_reset()
{
	m_irq_state = CLEAR_LINE;
	m_ctrl_reg = 0;
	m_status = 0;

	m_linkenable = 0;
	m_linkid = 0;
	m_txmode = 0;
}

void k056230_device::regs_map(address_map &map)
{
	map(0x00, 0x00).lrw8(
		NAME([this] (offs_t offset) {
			u8 data = 0x08 | m_status;
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_REG_READS, "%s: Status Register read %02x\n", machine().describe_context(), data);
			return data;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGMASKED(LOG_REG_WRITES, "%s: Mode Register write %02x\n", machine().describe_context(), data);
			set_mode(data);
		})
	),
	map(0x01, 0x01).lrw8(
		NAME([this] (offs_t offset) {
			const u8 res = 0x00;
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_REG_READS, "%s: CRC Error Register read %02x\n", machine().describe_context(), res);
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGMASKED(LOG_REG_WRITES, "%s: Control Register write %02x\n", machine().describe_context(), data);
			set_ctrl(data);
			// TODO: This is a literal translation of the previous device behaviour, and is incorrect.
			// Namely it can't possibly ping irq state on the fly, needs some transaction from the receiver.
			const int old_state = m_irq_state;
			if (BIT(data, 5))
			{
				LOGMASKED(LOG_REG_WRITES, "%s: regs_w: Asserting IRQ\n", machine().describe_context());
				m_irq_state = ASSERT_LINE;
			}
			if (!BIT(data, 0))
			{
				LOGMASKED(LOG_REG_WRITES, "%s: regs_w: Clearing IRQ\n", machine().describe_context());
				m_irq_state = CLEAR_LINE;
			}
			if (old_state != m_irq_state)
			{
				m_irq_cb(m_irq_state);
			}
		})
	);
	map(0x02, 0x02).lw8(
		NAME([this] (offs_t offset, u8 data) {
			LOGMASKED(LOG_REG_WRITES, "%s: regs_w: Sub ID Register = %02x\n", machine().describe_context(), data);
		})
	);
}

u32 k056230_device::ram_r(offs_t offset, u32 mem_mask)
{
	const auto lanc_ram = util::big_endian_cast<const u32>(m_ram.target());
	u32 data = lanc_ram[offset & 0x7ff];
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_RAM_READS, "%s: Network RAM read [%04x (%03x)]: %08x & %08x\n", machine().describe_context(), offset << 2, (offset & 0x7ff) << 2, data, mem_mask);
	return data;
}

void k056230_device::ram_w(offs_t offset, u32 data, u32 mem_mask)
{
	const auto lanc_ram = util::big_endian_cast<u32>(m_ram.target());
	LOGMASKED(LOG_RAM_WRITES, "%s: Network RAM write [%04x (%03x)] = %08x & %08x\n", machine().describe_context(), offset << 2, (offset & 0x7ff) << 2, data, mem_mask);
	COMBINE_DATA(&lanc_ram[offset & 0x7ff]);
}


void k056230_device::set_mode(u8 data)
{
	switch (data & 0xf0)
	{
		case 0x00:
		case 0x20:
			m_linkid = data & 0x0f;
			break;

		default:
			logerror("k056230-set_mode: %02x\n", data);
			break;
	}
}

void k056230_device::set_ctrl(u8 data)
{

	m_linkenable = data && 0x10;

	switch (data)
	{
		case 0x10:
		case 0x18:
			// prepare for tx?
			break;

		case 0x14:
		case 0x1c:
			// prepare for rx? - clear bit5
			m_status = 0x00;
			break;

		case 0x36:
		case 0x3e:
			// plygonet, polynetw, gticlub = 36, midnrun 3e
			// tx mode?
			m_txmode = 0x01;
			comm_tick();
			break;

		case 0x37:
		case 0x3f:
			// rx mode? - set bit 5
			// plygonet, polynetw, gticlub = 37, midnrun 3f
			m_status = 0x20;
			m_txmode = 0x00;
			comm_tick();
			break;

		default:
			logerror("k056230-set_ctrl: %02x\n", data);
			break;
	}
}

void k056230_device::comm_tick()
{
	if (m_linkenable == 0x01)
	{
		// check rx socket
		if (!m_line_rx)
		{
			osd_printf_verbose("k056230: rx listen on %s\n", m_localhost);
			u64 filesize; // unused
			std::error_condition filerr = osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
			if (filerr.value() != 0)
			{
				osd_printf_verbose("k056230: rx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
				m_line_rx.reset();
			}
		}

		// check tx socket
		if ((!m_line_tx) && (m_txmode == 0x01))
		{
			osd_printf_verbose("k056230: tx connect to %s\n", m_remotehost);
			u64 filesize; // unused
			std::error_condition filerr = osd_file::open(m_remotehost, 0, m_line_tx, filesize);
			if (filerr.value() != 0)
			{
				osd_printf_verbose("k056230: tx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
				m_line_tx.reset();
			}
		}

		// if both sockets are there check ring
		if (m_line_rx && m_line_tx)
		{
			int data_size = 0x201;

			if (m_txmode == 0x00)
			{
				// try to read one message
				int recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if valid id
					u8 idx = m_buffer0[0];
					if (idx <= 0x0e)
					{
						if (idx != m_linkid)
						{
							// save message to ram
							int frame_start = (idx & 0x07) * 0x0100;
							int frame_size = (idx & 0x08) ? 0x0200 : 0x0100;
							int frame_offset = 0;
							u32 frame_data = 0;

							for (int j = 0x00 ; j < frame_size ; j += 4)
							{
								frame_offset = (frame_start + j) / 4;
								frame_data = m_buffer0[4 + j] |  m_buffer0[3 + j] << 8 |  m_buffer0[2 + j] << 16 |  m_buffer0[1 + j] << 24;
								ram_w(frame_offset, frame_data, 0xffffffff);
							}

							// forward message to other nodes
							send_frame(data_size);
						}
					}

					// try to read another message
					recv = read_frame(data_size);
				}
			}
			else if (m_txmode == 0x01)
			{
				// send local data to network (once)
				int frame_start = (m_linkid & 0x07) * 0x100;
				int frame_size = (m_linkid & 0x08) ? 0x200 : 0x100;

				int frame_offset = 0;
				u32 frame_data = 0;

				m_buffer0[0] = m_linkid;
				for (int i = 0x00 ; i < frame_size ; i += 4)
				{
					frame_offset = (frame_start + i) / 4;
					frame_data = ram_r(frame_offset, 0xffffffff);
					m_buffer0[4 + i] = frame_data & 0xff;
					m_buffer0[3 + i] = (frame_data >> 8) & 0xff;
					m_buffer0[2 + i] = (frame_data >> 16) & 0xff;
					m_buffer0[1 + i] = (frame_data >> 24) & 0xff;
				}

				send_frame(data_size);
				m_txmode = 0x02;
			}
		}
	}
}

int k056230_device::read_frame(int data_size)
{
	if (!m_line_rx)
		return 0;

	// try to read a message (handling partial reads)
	u32 bytes_total = 0;
	u32 bytes_read = 0;
	while (bytes_total < data_size)
	{
		std::error_condition filerr = m_line_rx->read(&m_buffer0[bytes_total], 0, data_size - bytes_total, bytes_read);
		if (filerr)
		{
			bytes_read = 0;
			// special case if first read returned "no data"
			if (bytes_total == 0 && std::errc::operation_would_block == filerr)
				return 0;
		}
		if ((!filerr && bytes_read == 0) || (filerr && std::errc::operation_would_block != filerr))
		{
			osd_printf_verbose("k056230: rx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
			m_line_rx.reset();
			return 0;
		}
		bytes_total += bytes_read;
	}
	return data_size;
}

void k056230_device::send_frame(int data_size)
{
	if (!m_line_tx)
		return;

	// try to send a message (handling partial writes)
	u32 bytes_total = 0;
	u32 bytes_sent = 0;
	while (bytes_total < data_size)
	{
		std::error_condition filerr = m_line_tx->write(&m_buffer0[bytes_total], 0, data_size - bytes_total, bytes_sent);
		if (filerr)
		{
			osd_printf_verbose("k056230: tx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
			m_line_tx.reset();
			return;
		}
		bytes_total += bytes_sent;
	}
}


/****************************************
 *
 * konami/viper.cpp superset overrides
 *
 ***************************************/

k056230_viper_device::k056230_viper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: k056230_device(mconfig, K056230_VIPER, tag, owner, clock)
{
}

void k056230_viper_device::device_reset()
{
	k056230_device::device_reset();
	m_control = 0;
	m_irq_enable = false;
	m_unk[0] = m_unk[1] = 0;
}

void k056230_viper_device::regs_map(address_map &map)
{
	map(0x00, 0x00).lw8(
		NAME([this] (offs_t offset, u8 data) {
			LOGMASKED(LOG_REG_WRITES, "%s: network_id_w = %02x\n", machine().describe_context(), data);
		})
	);
	map(0x01, 0x01).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// HACK: more irq bad juju
			if (!BIT(data, 0))
			{
				m_status = 0;
				m_irq_state = CLEAR_LINE;
			}
			else
			{
				if(m_irq_enable)
					m_irq_state = ASSERT_LINE;
			}

			if (BIT(data, 3))
				m_status = 0x10;

			m_irq_cb(m_irq_state);

			// TODO: is this readback-able?
			m_control = data;
			LOGMASKED(LOG_REG_WRITES, "%s: control_w: %02x\n", machine().describe_context(), m_control);
		})
	);
	map(0x02, 0x02).lr8(
		NAME([this] (offs_t offset) {
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_REG_READS, "%s: status_r: %02x\n", machine().describe_context(), m_status);
			return m_status;
		})
	);
	// TODO: unknown regs
	map(0x03, 0x04).lrw8(
		NAME([this] (offs_t offset) {
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_REG_READS, "%s: unk%d_r\n", machine().describe_context(), offset + 3, m_unk[offset]);
			return m_unk[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGMASKED(LOG_REG_WRITES, "%s: unk%d_w %02x\n", machine().describe_context(), offset + 3, m_unk[offset], data);
			m_unk[offset] = data;
		})
	);
	map(0x05, 0x05).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// TODO: should sync if an irq is already there
			m_irq_enable = bool(BIT(data, 0));
			LOGMASKED(LOG_REG_WRITES, "%s: irq enable: %02x\n", machine().describe_context(), data);
		})
	);
}
