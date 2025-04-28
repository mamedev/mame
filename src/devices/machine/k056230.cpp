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
- interupts (racinfrc lan check shows debug info while holding F2)
- "manual" TX/RX mode
- LANC part name for konami/viper.cpp

**************************************************************************************************/

#include "emu.h"
#include "k056230.h"

#include "emuopts.h"
#include "multibyte.h"

#include "asio.h"

#include <iostream>

#define LOG_REG_READS   (1U << 1)
#define LOG_REG_WRITES  (1U << 2)
#define LOG_RAM_READS   (1U << 3)
#define LOG_RAM_WRITES  (1U << 4)
#define LOG_UNKNOWNS    (1U << 5)
#define LOG_ALL (LOG_REG_READS | LOG_REG_WRITES | LOG_RAM_READS | LOG_RAM_WRITES | LOG_UNKNOWNS)

#define VERBOSE (0)
#include "logmacro.h"

class k056230_device::context
{
public:
	context() :
	m_acceptor(m_ioctx),
	m_sock_rx(m_ioctx),
	m_sock_tx(m_ioctx),
	m_tx_timeout(m_ioctx)
	{
	}

	void start(std::string localhost, std::string localport, std::string remotehost, std::string remoteport)
	{
		std::error_code err;
		asio::ip::tcp::resolver resolver(m_ioctx);

		for (auto &&resolveIte : resolver.resolve(localhost, localport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_localaddr = resolveIte.endpoint();
			LOG("k056230: localhost = %s\n", *m_localaddr);
		}
		if (err)
		{
			LOG("k056230: localhost resolve error: %s\n", err.message());
		}

		for (auto &&resolveIte : resolver.resolve(remotehost, remoteport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_remoteaddr = resolveIte.endpoint();
			LOG("k056230: remotehost = %s\n", *m_remoteaddr);
		}
		if (err)
		{
			LOG("k056230: remotehost resolve error: %s\n", err.message());
		}
	}

	void stop()
	{
		std::error_code err;
		if (m_acceptor.is_open())
			m_acceptor.close(err);
		if (m_sock_rx.is_open())
			m_sock_rx.close(err);
		if (m_sock_tx.is_open())
			m_sock_tx.close(err);
		m_tx_timeout.cancel();
		m_tx_state = 0;
		m_rx_state = 0;
	}

	void check_sockets()
	{
		// if async operation in progress, poll context
		if ((m_rx_state == 1) || (m_tx_state == 1))
			m_ioctx.poll();

		// start acceptor if needed
		if (m_localaddr && m_rx_state == 0)
		{
			std::error_code err;
			m_acceptor.open(m_localaddr->protocol(), err);
			m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
			if (!err)
			{
				m_acceptor.bind(*m_localaddr, err);
				if (!err)
				{
					m_acceptor.listen(1, err);
					if (!err)
					{
						osd_printf_verbose("k056230: RX listen on %s\n", *m_localaddr);
						m_acceptor.async_accept(
								[this] (std::error_code const &err, asio::ip::tcp::socket sock)
								{
									if (err)
									{
										LOG("k056230: RX error accepting - %d %s\n", err.value(), err.message());
										std::error_code e;
										m_acceptor.close(e);
										m_rx_state = 0;
									}
									else
									{
										LOG("k056230: RX connection from %s\n", sock.remote_endpoint());
										std::error_code e;
										m_acceptor.close(e);
										m_sock_rx = std::move(sock);
										m_sock_rx.non_blocking(true);
										m_sock_rx.set_option(asio::socket_base::receive_buffer_size(524288));
										m_sock_rx.set_option(asio::socket_base::keep_alive(true));
										m_rx_state = 2;
									}
								});
						m_rx_state = 1;
					}
				}
			}
			if (err)
			{
				LOG("k056230: RX failed - %d %s\n", err.value(), err.message());
			}
		}

		// connect socket if needed
		if (m_remoteaddr && m_tx_state == 0)
		{
			std::error_code err;
			if (m_sock_tx.is_open())
				m_sock_tx.close(err);
			m_sock_tx.open(m_remoteaddr->protocol(), err);
			if (!err)
			{
				m_sock_tx.non_blocking(true);
				m_sock_tx.set_option(asio::ip::tcp::no_delay(true));
				m_sock_tx.set_option(asio::socket_base::send_buffer_size(65536));
				m_sock_tx.set_option(asio::socket_base::keep_alive(true));
				osd_printf_verbose("k056230: TX connecting to %s\n", *m_remoteaddr);
				m_tx_timeout.expires_after(std::chrono::seconds(10));
				m_tx_timeout.async_wait(
						[this] (std::error_code const &err)
						{
							if (!err && m_tx_state == 1)
							{
								osd_printf_verbose("k056230: TX connect timed out\n");
								std::error_code e;
								m_sock_tx.close(e);
								m_tx_state = 0;
							}
						});
				m_sock_tx.async_connect(
						*m_remoteaddr,
						[this] (std::error_code const &err)
						{
							m_tx_timeout.cancel();
							if (err)
							{
								osd_printf_verbose("k056230: TX connect error - %d %s\n", err.value(), err.message());
								std::error_code e;
								m_sock_tx.close(e);
								m_tx_state = 0;
							}
							else
							{
								LOG("k056230: TX connection established\n");
								m_tx_state = 2;
							}
						});
				m_tx_state = 1;
			}
		}
	}

	bool connected()
	{
		return m_rx_state == 2 && m_tx_state == 2;
	}

	unsigned receive(uint8_t *buffer, unsigned data_size)
	{
		if (m_rx_state != 2)
			return 0;

		std::error_code err;
		std::size_t bytes_read = m_sock_rx.receive(asio::buffer(&buffer[0], data_size), asio::socket_base::message_peek, err);
		if (err == asio::error::would_block)
			return 0;

		if (bytes_read == data_size)
			bytes_read = m_sock_rx.receive(asio::buffer(&buffer[0], data_size), 0, err);

		if (err)
		{
			osd_printf_verbose("k056230: RX error receiving - %d %s\n", err.value(), err.message());
			m_sock_rx.close(err);
			m_rx_state = 0;
			return UINT_MAX;
		}

		if (bytes_read == data_size)
			return bytes_read;
		return 0;
	}

	unsigned send(uint8_t *buffer, unsigned data_size)
	{
		if (m_tx_state != 2)
			return 0;

		std::error_code err;
		std::size_t bytes_sent = m_sock_tx.send(asio::buffer(&buffer[0], data_size), 0, err);
		if (err || bytes_sent != data_size)
		{
			osd_printf_verbose("k056230: TX error sending - %d %s\n", err.value(), err.message());
			m_sock_tx.close(err);
			m_tx_state = 0;
			return UINT_MAX;
		}
		return data_size;
	}

private:
	template <typename Format, typename... Params>
	void logerror(Format &&fmt, Params &&... args) const
	{
		util::stream_format(
				std::cerr,
				"%s",
				util::string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

	asio::io_context m_ioctx;
	std::optional<asio::ip::tcp::endpoint> m_localaddr;
	std::optional<asio::ip::tcp::endpoint> m_remoteaddr;
	asio::ip::tcp::acceptor m_acceptor;
	asio::ip::tcp::socket m_sock_rx;
	asio::ip::tcp::socket m_sock_tx;
	asio::steady_timer m_tx_timeout;
	uint8_t m_rx_state;
	uint8_t m_tx_state;
};

DEFINE_DEVICE_TYPE(K056230, k056230_device, "k056230", "K056230 LANC")
DEFINE_DEVICE_TYPE(K056230_VIPER, k056230_viper_device, "k056230_viper", "Konami Viper LANC")

k056230_device::k056230_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock),
	m_ram(*this, "lanc_ram", 0x800U * 4, ENDIANNESS_BIG),
	m_irq_cb(*this)
{
}

k056230_device::k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: k056230_device(mconfig, K056230, tag, owner, clock)
{
}

void k056230_device::device_start()
{
	auto ctx = std::make_unique<context>();
	m_context = std::move(ctx);

	// state saving
	save_item(NAME(m_irq_state));
	save_item(NAME(m_status));

	save_item(NAME(m_linkenable));
	save_item(NAME(m_linkid));
	save_item(NAME(m_linksize));
	save_item(NAME(m_txmode));
}

void k056230_device::device_reset()
{
	m_irq_enable = false;
	m_irq_state = CLEAR_LINE;
	m_status = 0;

	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);

	m_context->stop();

	auto const &opts = mconfig().options();
	m_context->start(opts.comm_localhost(), opts.comm_localport(), opts.comm_remotehost(), opts.comm_remoteport());

	m_linkenable = 0;
	m_linkid = 0;
	m_linksize = 0x0100;
	m_txmode = 0;
}

void k056230_device::device_stop()
{
	m_context->stop();
	m_context.reset();
}

void k056230_device::regs_map(address_map &map)
{
	map(0x00, 0x00).lrw8(
		NAME([this] (offs_t offset) {
			/*
			status register bits:
				2,1,0 = TID, transmit status
				3     = RXD
				4     = INTST-, 0 when INT
				5     = RUNST, 1 when comms active
				7,6   = unused
			*/
			u8 data = 0x08 | m_status;
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_REG_READS, "%s: Status Register read %02x\n", machine().describe_context(), data);
			return data;
		}),
		NAME([this] (offs_t offset, u8 data) {
			/*
			mode register bits:
				2,1,0 = ID  - 0 to 7
				4,3   = IDM - 0, 1, 3 valid, 2 invalid
				5     = M/S - 1 means MASTER
				6     = TXD
				7     = TXM - 1 means TX line will be whatever TXD is set to. (not implemented)
			*/
			LOGMASKED(LOG_REG_WRITES, "%s: Mode Register write %02x\n", machine().describe_context(), data);
			set_mode(data);
		})
	),
	map(0x01, 0x01).lrw8(
		NAME([this] (offs_t offset) {
			/*
			crc error register bits:
				each bit signals a receive error from the corrosponding main-id (bit 0 = id 0 etc.)
			*/
			const u8 res = 0x00;
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_REG_READS, "%s: CRC Error Register read %02x\n", machine().describe_context(), res);
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			/*
			control register bits:
				0   = START
				1   = INTPR - 1 enables int
				2   = CRCR - 1 enables crc error register
				4,3 = SIZE - 0 32byte, 1 64 byte, 2 128 byte, 3, 256 byte
				5   = SLEEP- - 0 means disabled, 1 means enabled?
				6   = H/S - comm speed - 0 = clk / 16, 1 = clk / 8
				7   = unused
			*/
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
			/*
			sub-id register bits:
				1,0     = SID, sub-id
				3,2     = MSID, master sub-id
				7,6,5,4 = unused
			*/
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
	m_linkid = data & 0x07;
	switch ((data & 0x18) >> 3)
	{
		case 0:
			// 8 cabinet mode
			m_linksize = 0x0100;
			break;
		case 1:
			// 4 cabinet mode
			m_linksize = 0x0200;
			break;
		case 3:
			// 2 cabinet mode
			m_linksize = 0x0400;
			break;
		case 2:
			// invalid
		default:
			m_linksize = 0x0100;
			logerror("set_mode: %02x invalid IDM selected\n", data);
			break;
	}
	if (data & 0x80)
	{
		logerror("set_mode: %02x manual tx/rx mode NOT implemented\n", data);
	}
}

void k056230_device::set_ctrl(u8 data)
{
	m_linkenable = (data & 0x20) ? 0x01 : 0x00;
	switch (data)
	{
		case 0x10:
		case 0x18:
			// prepare for rx?
			break;

		case 0x14:
		case 0x1c:
			// prepare for tx? - clear bit5
			m_status = 0x00;
			break;

		case 0x36:
		case 0x3e:
			// plygonet, polynetw, gticlub = 36, midnrun 3e
			// rx mode?
			m_txmode = 0x00;
			comm_tick();
			break;

		case 0x37:
		case 0x3f:
			// tx mode? - set bit 5
			// plygonet, polynetw, gticlub = 37, midnrun 3f
			m_status = 0x20;
			m_txmode = 0x01;
			comm_tick();
			break;

		default:
			logerror("k056230-set_ctrl: %02x\n", data);
			break;
	}
}

void k056230_device::comm_tick()
{
	m_context->check_sockets();

	if (m_linkenable == 0x01)
	{
		// if both sockets are there check ring
		if (m_context->connected())
		{
			unsigned data_size = m_linksize + 1;

			if (m_txmode == 0x00)
			{
				// try to read one message
				unsigned recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if valid id
					u8 idx = m_buffer[0];
					if (idx <= 0x0e)
					{
						if (idx != m_linkid)
						{
							// save message to ram
							unsigned frame_start = idx * 0x0100;
							unsigned frame_size = m_linksize;
							unsigned frame_offset = 0;
							u32 frame_data = 0;

							for (unsigned i = 0; i < frame_size; i += 4)
							{
								frame_offset = (frame_start + i) / 4;
								frame_data = get_u32be(&m_buffer[i + 1]);
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
				unsigned frame_start = m_linkid * 0x100;
				unsigned frame_size = m_linksize;

				unsigned frame_offset = 0;
				u32 frame_data = 0;

				m_buffer[0] = m_linkid;
				for (unsigned i = 0; i < frame_size; i += 4)
				{
					frame_offset = (frame_start + i) / 4;
					frame_data = ram_r(frame_offset, 0xffffffff);
					put_u32be(&m_buffer[i + 1], frame_data);
				}

				send_frame(data_size);
				m_txmode = 0x02;
			}
		}
	}
}

unsigned k056230_device::read_frame(unsigned data_size)
{
	unsigned bytes_read = m_context->receive(&m_buffer[0], data_size);
	if (bytes_read == UINT_MAX)
	{
		// error case, do nothing
		return 0;
	}
	return bytes_read;
}

void k056230_device::send_frame(unsigned data_size)
{
	unsigned bytes_sent = m_context->send(&m_buffer[0], data_size);
	if (bytes_sent == UINT_MAX)
	{
		// error case, do nothing
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
				if (m_irq_enable)
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
