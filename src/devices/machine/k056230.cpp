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
	m_timeout_tx(m_ioctx),
	m_state_rx(0U),
	m_state_tx(0U)
	{
	}

	void start()
	{

		m_thread = std::thread(
				[this] ()
				{
					LOG("k056230: network thread started\n");
					try {
						m_ioctx.run();
					} catch (const std::exception& e) {
						LOG("k056230: Exception in network thread: %s\n", e.what());
					} catch (...) { // Catch any other unknown exceptions
						LOG("k056230: Unknown exception in network thread\n");
					}
					LOG("k056230: network thread completed\n");
				});
	}

	void reset(std::string localhost, std::string localport, std::string remotehost, std::string remoteport)
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

		m_ioctx.post(
				[this] ()
				{
					std::error_code err;
					if (m_acceptor.is_open())
						m_acceptor.close(err);
					if (m_sock_rx.is_open())
						m_sock_rx.close(err);
					if (m_sock_tx.is_open())
						m_sock_tx.close(err);
					m_timeout_tx.cancel();
					m_state_rx.store(0);
					m_state_tx.store(0);
					start_accept();
					start_connect();
				});
	}

	void stop()
	{
		m_ioctx.post(
				[this] ()
				{
					std::error_code err;
					if (m_acceptor.is_open())
						m_acceptor.close(err);
					if (m_sock_rx.is_open())
						m_sock_rx.close(err);
					if (m_sock_tx.is_open())
						m_sock_tx.close(err);
					m_timeout_tx.cancel();
					m_state_rx.store(0);
					m_state_tx.store(0);
					m_ioctx.stop();
				});
		m_work_guard.reset();
		if (m_thread.joinable()) {
			m_thread.join();
		}
	}

	void check_sockets()
	{
	}

	bool connected()
	{
		return m_state_rx.load() == 2 && m_state_tx.load() == 2;
	}

	unsigned receive(uint8_t *buffer, unsigned data_size)
	{
		if (m_state_rx.load() < 2)
			return UINT_MAX;

		if (data_size > m_fifo_rx.used())
			return 0;

		return m_fifo_rx.read(&buffer[0], data_size, false);
	}

	unsigned send(uint8_t *buffer, unsigned data_size)
	{
		if (m_state_tx.load() < 2)
			return UINT_MAX;

		if (data_size > m_fifo_tx.free())
		{
			LOG("k056230: TX buffer overflow\n");
			return UINT_MAX;
		}

		bool const sending = m_fifo_tx.used();
		m_fifo_tx.write(&buffer[0], data_size);
		if (!sending)
			m_ioctx.post(
					[this] ()
					{
						start_send_tx();
					});
		return data_size;
	}

private:
	class fifo
	{
	public:
		unsigned write(uint8_t *buffer, unsigned data_size)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			unsigned used = 0;
			if (m_wp >= m_rp)
			{
				used = std::min<unsigned>(m_buffer.size() - m_wp, data_size);
				std::copy_n(&buffer[0], used, &m_buffer[m_wp]);
				m_wp = (m_wp + used) % m_buffer.size();
			}
			unsigned const block = std::min<unsigned>(data_size - used, m_rp - m_wp);
			if (block)
			{
				std::copy_n(&buffer[used], block, &m_buffer[m_wp]);
				used += block;
				m_wp += block;
			}
			m_used += used;
			return used;
		}

		unsigned read(uint8_t *buffer, unsigned data_size, bool peek)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			unsigned rp = m_rp;
			unsigned used = 0;
			if (rp >= m_wp)
			{
				used = std::min<std::size_t>(m_buffer.size() - rp, data_size);
				std::copy_n(&m_buffer[rp], used, &buffer[0]);
				rp = (rp + used) % m_buffer.size();
			}
			unsigned const block = std::min<unsigned>(data_size - used, m_wp - rp);
			if (block)
			{
				std::copy_n(&m_buffer[rp], block, &buffer[used]);
				used += block;
				rp += block;
			}
			if (!peek)
			{
				m_rp = (m_rp + used) % m_buffer.size();
				m_used -= used;
			}
			return used;
		}

		void consume(unsigned data_size)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_rp = (m_rp + data_size) % m_buffer.size();
			m_used -= data_size;
		}

		unsigned used()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_used;
		}

		unsigned free()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_buffer.size() - m_used;
		}

		void clear()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_wp = m_rp = m_used = 0;
		}


	private:
		unsigned m_wp = 0;
		unsigned m_rp = 0;
		unsigned m_used = 0;
		std::array<uint8_t, 0x80000> m_buffer;
		std::mutex m_mutex;
	};

	void start_accept()
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
									m_state_rx.store(0);
									start_accept();
								}
								else
								{
									LOG("k056230: RX connection from %s\n", sock.remote_endpoint());
									std::error_code e;
									m_acceptor.close(e);
									m_sock_rx = std::move(sock);
									m_sock_rx.set_option(asio::socket_base::keep_alive(true));
									m_state_rx.store(2);
									start_receive_rx();
								}
							});
					m_state_rx.store(1);
				}
			}
		}
		if (err)
		{
			LOG("k056230: RX failed - %d %s\n", err.value(), err.message());
		}
	}

	void start_connect()
	{
		std::error_code err;
		if (m_sock_tx.is_open())
			m_sock_tx.close(err);
		m_sock_tx.open(m_remoteaddr->protocol(), err);
		if (!err)
		{
			m_sock_tx.set_option(asio::ip::tcp::no_delay(true));
			m_sock_tx.set_option(asio::socket_base::keep_alive(true));
			osd_printf_verbose("k056230: TX connecting to %s\n", *m_remoteaddr);
			m_timeout_tx.expires_after(std::chrono::seconds(10));
			m_timeout_tx.async_wait(
					[this] (std::error_code const &err)
					{
						if (!err && m_state_tx.load() == 1)
						{
							osd_printf_verbose("k056230: TX connect timed out\n");
							std::error_code e;
							m_sock_tx.close(e);
							m_state_tx.store(0);
							start_connect();
						}
					});
			m_sock_tx.async_connect(
					*m_remoteaddr,
					[this] (std::error_code const &err)
					{
						m_timeout_tx.cancel();
						if (err)
						{
							osd_printf_verbose("k056230: TX connect error - %d %s\n", err.value(), err.message());
							std::error_code e;
							m_sock_tx.close(e);
							m_state_tx.store(0);
							start_connect();
						}
						else
						{
							LOG("k056230: TX connection established\n");
							m_state_tx.store(2);
						}
					});
			m_state_tx.store(1);
		}
	}

	void start_send_tx()
	{
		unsigned used = m_fifo_tx.read(&m_buffer_tx[0], std::min<unsigned>(m_fifo_tx.used(), m_buffer_tx.size()), true);
		m_sock_tx.async_write_some(
				asio::buffer(&m_buffer_tx[0], used),
				[this] (std::error_code const &err, std::size_t length)
				{
					m_fifo_tx.consume(length);
					if (err)
					{
						LOG("k056230: TX connection error: %s\n", err.message().c_str());
						m_sock_tx.close();
						m_state_tx.store(0);
						m_fifo_tx.clear();
						start_connect();
					}
					else if (m_fifo_tx.used())
					{
						start_send_tx();
					}
				});
	}

	void start_receive_rx()
	{
		m_sock_rx.async_read_some(
				asio::buffer(m_buffer_rx),
				[this] (std::error_code const &err, std::size_t length)
				{
					if (err || !length)
					{
						if (err)
							LOG("k056230: RX connection error: %s\n", err.message());
						else
							LOG("k056230: RX connection lost\n");
						m_sock_rx.close();
						m_state_rx.store(0);
						m_fifo_rx.clear();
						start_accept();
					}
					else
					{
						if (UINT_MAX == m_fifo_rx.write(&m_buffer_rx[0], length))
						{
							LOG("k056230: RX buffer overflow\n");
							m_sock_rx.close();
							m_state_rx.store(0);
							m_fifo_rx.clear();
						}
						start_receive_rx();
					}
				});
	}

	template <typename Format, typename... Params>
	void logerror(Format &&fmt, Params &&... args) const
	{
		util::stream_format(
				std::cerr,
				"%s",
				util::string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

	std::thread m_thread;
	asio::io_context m_ioctx;
	asio::executor_work_guard<asio::io_context::executor_type> m_work_guard{m_ioctx.get_executor()};
	std::optional<asio::ip::tcp::endpoint> m_localaddr;
	std::optional<asio::ip::tcp::endpoint> m_remoteaddr;
	asio::ip::tcp::acceptor m_acceptor;
	asio::ip::tcp::socket m_sock_rx;
	asio::ip::tcp::socket m_sock_tx;
	asio::steady_timer m_timeout_tx;
	std::atomic_uint m_state_rx;
	std::atomic_uint m_state_tx;
	fifo m_fifo_rx;
	fifo m_fifo_tx;
	std::array<uint8_t, 0x400> m_buffer_rx;
	std::array<uint8_t, 0x400> m_buffer_tx;
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
	m_tick_timer = timer_alloc(FUNC(k056230_device::tick_timer_callback), this);
	m_tick_timer->adjust(attotime::never);

	auto ctx = std::make_unique<context>();
	m_context = std::move(ctx);
	m_context->start();

	// state saving
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_status));

	save_item(NAME(m_linkenable));
	save_item(NAME(m_linkmaster));
	save_item(NAME(m_linkid));
	save_item(NAME(m_linkidm));
	save_item(NAME(m_linksize));
	save_item(NAME(m_txmode));
}

void k056230_device::device_reset()
{
	m_irq_enable = false;
	m_irq_state = CLEAR_LINE;
	m_status = 0;

	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);

	auto const &opts = mconfig().options();
	m_context->reset(opts.comm_localhost(), opts.comm_localport(), opts.comm_remotehost(), opts.comm_remoteport());

	m_linkenable = false;
	m_linkmaster = false;
	m_linkid = 0;
	m_linkidm = 0;
	m_linksize = 0;
	m_txmode = 0;

	m_tick_timer->adjust(attotime::from_hz(800), 0, attotime::from_hz(800));
}


void k056230_device::device_stop()
{
	m_tick_timer->adjust(attotime::never);

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

void k056230_device::set_irq(int state)
{
	if (state != m_irq_state)
	{
		if (state == CLEAR_LINE)
			m_status |= 0x10;
		else
			m_status &= 0xef;

		m_irq_state = state;

		if (m_irq_enable)
			m_irq_cb(m_irq_state);
	}
}

void k056230_device::set_mode(u8 data)
{
	m_linkid = data & 0x07;
	m_linkidm = (data & 0x18) >> 3;
	if (m_linkidm == 2)
		logerror("set_mode: %02x invalid IDM selected\n", data);
	m_linkmaster = bool(BIT(data, 5));
	// ignore bit 6 (TXD) for now
	if (data & 0x80)
		logerror("set_mode: %02x manual tx/rx mode NOT implemented\n", data);
}

void k056230_device::set_ctrl(u8 data)
{
	bool start = bool(BIT(data, 0));
	if (!bool(BIT(data, 1)))
	{
		set_irq(CLEAR_LINE);
	}
	m_irq_enable = bool(BIT(data, 1));
	// ignore bit 2 (CRCR) for now
	m_linksize = (data & 0x18) >> 3;
	m_linkenable = bool(BIT(data, 5));
	// ignore bit 6 (H/S) for now
	// ignore bit 7 (unused)

	if (m_linkenable)
	{
		if (m_linkid == 0x00)
		{
			if (start)
			{
				// master (start)
				m_txmode = 0x01;
				m_status |= 0x20;
			}
			else
			{
				// master (stop)
				m_txmode = 0x02;
				m_status &= 0xdf;
			}
		}
		else
		{
			// slave
			m_txmode = 0x03;
			m_status |= 0x20;
		}
	} else {
		// (disabled)
		m_status &= 0xdf;
		m_txmode = 0x00;
	}
}

TIMER_CALLBACK_MEMBER(k056230_device::tick_timer_callback)
{
	comm_tick();
}

void k056230_device::comm_tick()
{
	m_context->check_sockets();

	if (m_linkenable)
	{
		// if both sockets are there check ring
		if (m_context->connected())
		{
			unsigned frame_size = 1 << (5 + m_linksize);
			unsigned data_size = frame_size + 1;

			bool raise_irq = (m_irq_state == ASSERT_LINE);

			if (m_txmode >= 0x00)
			{
				// try to read one message
				unsigned recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if valid id
					u8 idx = m_buffer[0];

					if (idx <= 0x07)
					{
						// if not own message
						if ((idx | m_linkidm) != (m_linkid | m_linkidm))
						{
							// save message to ram
							unsigned frame_start = idx * 0x0100;
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

						if (idx == 0)
							m_txmode &= 0xfd;
					}

					// try to read another message
					recv = read_frame(data_size);
				}
			}

			if (m_txmode == 0x01)
			{
				// send local data to network
				for (unsigned j = 0; j <= m_linkidm; j++)
				{
					comm_send(m_linkid + j, frame_size, data_size);
				}

				// disable transmission
				m_txmode = 0x02;

				raise_irq = true;
			}

			// raise irq
			if (raise_irq)
			{
				set_irq(ASSERT_LINE);
			}
		}
	}
}

void k056230_device::comm_send(u8 idx, unsigned frame_size, unsigned data_size)
{
	unsigned frame_start = idx * 0x100;

	unsigned frame_offset = 0;
	u32 frame_data = 0;

	m_buffer[0] = idx;
	for (unsigned i = 0; i < frame_size; i += 4)
	{
		frame_offset = (frame_start + i) / 4;
		frame_data = ram_r(frame_offset, 0xffffffff);
		put_u32be(&m_buffer[i + 1], frame_data);
	}

	send_frame(data_size);
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
