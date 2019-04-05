// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the Intel 82586 and 82596 Ethernet controller devices.
 *
 * This driver covers the following devices:
 *
 *   - 82586 - 16/24 data/address bus, 6/8/10 MHz
 *   - 82596SX - 16/32 data/address bus, 16/20 MHz
 *   - 82596DX - 32/32 data/address bus, 25/33 MHz
 *   - 82596CA - 32/32 data/address bus, 16/20/25/33 MHz
 *
 * This implementation should cover all of the above reasonably well, but
 * no testing of big endian mode in particular, and very limited testing
 * of the 82596 in non-linear modes has been done so far.
 *
 * Sources:
 *
 *   http://bitsavers.org/pdf/intel/_dataBooks/1991_Microcommunications.pdf
 *   http://bitsavers.org/pdf/intel/_dataBooks/1996_Networking.pdf
 *   https://www.intel.com/assets/pdf/general/82596ca.pdf
 *
 * TODO
 *   - testing for 82596 big endian and non-linear modes
 *   - more complete statistics capture
 *   - 82596 monitor mode
 *   - throttle timers and diagnostic command
 *   - special case handling for different 82596 steppings in big endian mode
 */

#include "emu.h"
#include "i82586.h"
#include "hashing.h"

#define LOG_GENERAL (1U << 0)
#define LOG_FRAMES  (1U << 1)
#define LOG_FILTER  (1U << 2)
#define LOG_CONFIG  (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_FRAMES | LOG_FILTER | LOG_CONFIG)

#include "logmacro.h"

ALLOW_SAVE_TYPE(i82586_base_device::cu_state);
ALLOW_SAVE_TYPE(i82586_base_device::ru_state);

DEFINE_DEVICE_TYPE(I82586, i82586_device, "i82586", "Intel 82586 IEEE 802.3 Ethernet LAN Coprocessor")
DEFINE_DEVICE_TYPE(I82596_LE16, i82596_le16_device, "i82596sx_le", "Intel 82596 SX High-Performance 32-Bit Local Area Network Coprocessor (little)")
DEFINE_DEVICE_TYPE(I82596_BE16, i82596_be16_device, "i82596sx_be", "Intel 82596 SX High-Performance 32-Bit Local Area Network Coprocessor (big)")
DEFINE_DEVICE_TYPE(I82596_LE32, i82596_le32_device, "i82596dx_le", "Intel 82596 DX/CA High-Performance 32-Bit Local Area Network Coprocessor (little)")
DEFINE_DEVICE_TYPE(I82596_BE32, i82596_be32_device, "i82596dx_be", "Intel 82596 DX/CA High-Performance 32-Bit Local Area Network Coprocessor (big)")

// Ethernet broadcast address
static const u8 ETH_BROADCAST[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

// configure parameter default values
static const u8 CFG_DEFAULTS[] = { 0x00, 0xc8, 0x40, 0x26, 0x00, 0x60, 0x00, 0xf2, 0x00, 0x00, 0x40, 0xff, 0x00, 0x3f };

// describes parameters and default values for logging
static const struct
{
	const char *const name, *const unit;
	const u8 dflt, byte, mask, shift;
	const bool ieee8023;
}
CFG_PARAMS[] =
{
	{ "address length",             "bytes",                               6,  3, 0x07, 0, true },
	{ "a/l field location",         "located in fd",                       0,  3, 0x08, 3, false },
	{ "auto retransmit",            "auto retransmit enable",              1, 11, 0x08, 3, false },
	{ "bitstuffing/eoc",            "eoc",                                 0,  8, 0x40, 6, false },
	{ "broadcast disable",          "broadcast reception enabled",         0,  8, 0x02, 1, false },
	{ "cdbsac",                     "disabled",                            1, 11, 0x10, 4, false },
	{ "cdt filter",                 "bit times",                           0,  9, 0x70, 4, false },
	{ "cdt src",                    "external collision detection",        0,  9, 0x80, 7, false },
	{ "crc in memory",              "crc not transferred to memory",       1, 11, 0x04, 2, false },
	{ "crc-16/crc-32",              "crc-32",                              0,  8, 0x20, 5, true },
	{ "crs filter",                 "bit times",                           0,  9, 0x07, 0, false },
	{ "crs src",                    "external crs",                        0,  9, 0x08, 3, false },
	{ "disbof",                     "backoff enabled",                     0, 13, 0x80, 7, false },
	{ "ext loopback",               "disabled",                            0,  3, 0x80, 7, false },
	{ "exponential priority",       "802.3 algorithm",                     0,  4, 0x70, 4, true },
	{ "exponential backoff method", "802.3 algorithm",                     0,  4, 0x80, 7, true },
	{ "full duplex (fdx)",          "csma/cd protocol (no fdx)",           0, 12, 0x40, 6, false },
	{ "fifo threshold",             "tx: 32 bytes, rx: 64 bytes",          8,  1, 0x0f, 0, false },
	{ "int loopback",               "disabled",                            0,  3, 0x40, 6, false },
	{ "interframe spacing",         "bit times",                          96,  5, 0xff, 0, true },
	{ "linear priority",            "802.3 algorithm",                     0,  4, 0x07, 0, true },
	{ "length field",               "padding disabled",                    1, 11, 0x02, 1, false },
	{ "min frame length",           "bytes",                              64, 10, 0xff, 0, true },
	{ "mc all",                     "disabled",                            1, 11, 0x20, 5, false },
	{ "monitor",                    "disabled",                            3, 11, 0xc0, 6, false },
	{ "manchester/nrz",             "nrz",                                 0,  8, 0x04, 2, false },
	{ "multi ia",                   "disabled",                            0, 14, 0x40, 6, false },
	{ "number of retries",          "maximum number of retries",          15,  7, 0xf0, 4, true },
	{ "no crc insertion",           "crc appended to frame",               0,  8, 0x10, 4, false },
	{ "prefetch bit in rbd",        "disabled (valid only in new modes)",  0,  0, 0x80, 7, false },
	{ "preamble length",            "bytes",                               7,  3, 0x30, 4, true },
	{ "preamble until crs",         "disabled",                            1, 11, 0x01, 0, false },
	{ "promiscuous mode",           "address filter on",                   0,  8, 0x01, 0, false },
	{ "padding",                    "no padding",                          0,  8, 0x80, 7, false },
	{ "resume rd",                  "do not reread next cb on resume (82596B stepping only)",
																		   0,  2, 0x02, 1, false },
	{ "slot time (lo)",             "bit times",                           0,  6, 0xff, 0, true },
	{ "slot time (hi)",             "bit times",                           2,  7, 0x07, 0, true },
	{ "save bad frame",             "discards bad frames",                 0,  2, 0x80, 7, false },
	{ "transmit on no crs",         "disabled",                            0,  8, 0x08, 3, false },
};

i82586_base_device::i82586_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endian, u8 datawidth, u8 addrwidth)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_network_interface(mconfig, *this, 10.0f)
	, m_space_config("shared", endian, datawidth, addrwidth)
	, m_out_irq(*this)
	, m_cx(false)
	, m_fr(false)
	, m_cna(false)
	, m_rnr(false)
	, m_initialised(false)
	, m_irq_assert(1)
	, m_cu_state(CU_IDLE)
	, m_ru_state(RU_IDLE)
	, m_scp_address(SCP_ADDRESS)
{
}

i82586_device::i82586_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82586_base_device(mconfig, I82586, tag, owner, clock, ENDIANNESS_LITTLE, 16, 24)
{
}

i82596_device::i82596_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endian, u8 datawidth)
	: i82586_base_device(mconfig, type, tag, owner, clock, endian, datawidth, 32)
{
}

i82596_le16_device::i82596_le16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82596_device(mconfig, I82596_LE16, tag, owner, clock, ENDIANNESS_LITTLE, 16)
{
}

i82596_be16_device::i82596_be16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82596_device(mconfig, I82596_BE16, tag, owner, clock, ENDIANNESS_BIG, 16)
{
}

i82596_le32_device::i82596_le32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82596_device(mconfig, I82596_LE32, tag, owner, clock, ENDIANNESS_LITTLE, 32)
{
}

i82596_be32_device::i82596_be32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82596_device(mconfig, I82596_BE32, tag, owner, clock, ENDIANNESS_BIG, 32)
{
}

// shared implementation
void i82586_base_device::device_start()
{
	m_space = &space(0);

	m_out_irq.resolve();

	m_cu_timer = timer_alloc(CU_TIMER);
	m_cu_timer->enable(false);

	save_item(NAME(m_cx));
	save_item(NAME(m_fr));
	save_item(NAME(m_cna));
	save_item(NAME(m_rnr));
	save_item(NAME(m_initialised));

	save_item(NAME(m_cu_state));
	save_item(NAME(m_ru_state));

	save_item(NAME(m_scp_address));
	save_item(NAME(m_scb_base));
	save_item(NAME(m_scb_address));
	save_item(NAME(m_scb_cs));
	save_item(NAME(m_cba));
	save_item(NAME(m_rfd));

	save_item(NAME(m_mac_multi));
}

void i82586_base_device::device_reset()
{
	m_cu_timer->enable(false);

	m_cx = false;
	m_fr = false;
	m_cna = false;
	m_rnr = false;
	m_initialised = false;

	m_cu_state = CU_IDLE;
	m_ru_state = RU_IDLE;

	m_scp_address = SCP_ADDRESS;
}

void i82586_base_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case CU_TIMER:
			cu_execute();
			break;
	}
}

device_memory_interface::space_config_vector i82586_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

WRITE_LINE_MEMBER(i82586_base_device::ca)
{
	LOG("channel attention %s (%s)\n", state ? "asserted" : "cleared", machine().describe_context());

	if (state)
	{
		// on first ca after reset, initialise
		if (!m_initialised)
			initialise();
		else
			process_scb();
	}
}

int i82586_base_device::recv_start_cb(u8 *buf, int length)
{
	// discard external packets in loopback mode
	if (cfg_loopback_mode())
		return 0;

	switch (m_ru_state)
	{
	case RU_IDLE:
	case RU_SUSPENDED:
		// discard all frames
		break;

	case RU_READY:
		return recv_start(buf, length);

	default:
		// no resources
		// TODO: accumulate statistics
		break;
	}

	return 0;
}

int i82586_base_device::recv_start(u8 *buf, int length)
{
	if (address_filter(buf))
	{
		LOG("recv_start receiving frame length %d\n", length);
		dump_bytes(buf, length);

		return ru_execute(buf, length);
	}

	return 0;
}

void i82586_base_device::recv_complete_cb(int result)
{
	ru_complete(result);

	update_scb();
}

void i82586_base_device::process_scb()
{
	// fetch current command and status
	m_scb_cs = m_space->read_dword(m_scb_address);

	// handle reset
	if (m_scb_cs & RESET)
	{
		LOG("process_scb reset\n");

		device_reset();
		return;
	}

	static const char *const CUC_NAME[] = { "NOP", "START", "RESUME", "SUSPEND", "ABORT", "THROTTLE_D", "THROTTLE_I", "reserved" };
	static const char *const RUC_NAME[] = { "NOP", "START", "RESUME", "SUSPEND", "ABORT", "reserved", "reserved", "reserved" };
	LOG("process_scb command/status 0x%08x (cuc %s, ruc %s)\n", m_scb_cs,
		CUC_NAME[(m_scb_cs & CUC) >> 24],
		RUC_NAME[(m_scb_cs & RUC) >> 20]);

	// clear interrupt flags when acknowledged
	if (m_scb_cs & ACK_CX)
		m_cx = false;
	if (m_scb_cs & ACK_FR)
		m_fr = false;
	if (m_scb_cs & ACK_CNA)
		m_cna = false;
	if (m_scb_cs & ACK_RNR)
		m_rnr = false;

	switch (m_scb_cs & CUC)
	{
	case CUC_NOP:
		break;

	case CUC_START:
		m_cba = address(m_scb_address, 4, 4);

		LOG("process_scb cuc start command block address 0x%08x\n", m_cba);

		m_cu_state = CU_ACTIVE;
		m_cu_timer->adjust(attotime::zero);
		break;

	case CUC_RESUME:
		m_cu_state = CU_ACTIVE;
		m_cu_timer->enable(true);
		break;

	case CUC_SUSPEND:
		m_cu_state = CU_SUSPENDED;
		m_cu_timer->enable(false);
		m_cna = true;
		break;

	case CUC_ABORT:
		m_cu_state = CU_IDLE;
		m_cu_timer->reset();
		m_cna = true;
		break;

	case CUC_THROTTLE_D:
	case CUC_THROTTLE_I:
		break;
	}

	switch (m_scb_cs & RUC)
	{
	case RUC_NOP:
		break;

	case RUC_START:
		m_rfd = address(m_scb_address, 6, 8);

		LOG("process_scb ruc start receive frame descriptor address 0x%08x\n", m_rfd);

		m_ru_state = RU_READY;
		break;

	case RUC_RESUME:
		m_ru_state = RU_READY;
		break;

	case RUC_SUSPEND:
		m_ru_state = RU_SUSPENDED;
		m_rnr = true;
		break;

	case RUC_ABORT:
		m_ru_state = RU_IDLE;
		m_rnr = true;
		break;
	}

	LOG("process_scb complete\n");
	update_scb();
}

void i82586_base_device::update_scb()
{
	// write the status word and clear the command word of the scb
	// TODO: T (throttle) status flag
	m_space->write_dword(m_scb_address,
		(m_cx ? CX : 0) |
		(m_fr ? FR : 0) |
		(m_cna ? CNA : 0) |
		(m_rnr ? RNR : 0) |
		(m_cu_state << 8) |
		(m_ru_state << 4));

	LOG("update_scb%s%s%s%s\n", m_cx ? " CX" : "", m_fr ? " FR" : "", m_cna ? " CNA" : "", m_rnr ? " RNR" : "");

	// update interrupt status
	set_irq(m_cx || m_fr || m_cna || m_rnr);
}

void i82586_base_device::cu_execute()
{
	// fetch the command block command/status
	const u32 cb_cs = m_space->read_dword(m_cba);
	u16 status = 0;

	// set busy status
	m_space->write_dword(m_cba, cb_cs | CB_B);

	static const char *const CMD_NAME[] = { "NOP", "INDIVIDUAL ADDRESS SETUP", "CONFIGURE", "MULTICAST SETUP", "TRANSMIT", "TIME DOMAIN REFLECTOMETER", "DUMP", "DIAGNOSE" };
	LOG("cu_execute command 0x%08x (%s)\n", cb_cs, CMD_NAME[(cb_cs & CB_CMD) >> 16]);

	if (m_cu_state != CU_IDLE)
	{
		// execute command logic
		switch (cb_cs & CB_CMD)
		{
		case CB_NOP:
			status |= CB_OK;
			break;

		case CB_IASETUP:
			if (cu_iasetup())
				status |= CB_OK;
			break;

		case CB_CONFIGURE:
			if (cu_configure())
				status |= CB_OK;
			break;

		case CB_MCSETUP:
			if (cu_mcsetup())
				status |= CB_OK;
			break;

		case CB_TRANSMIT:
			if (cu_transmit(cb_cs))
				return;
			break;

		case CB_TDREFLECT:
			if (cu_tdreflect())
				status |= CB_OK;
			break;

		case CB_DUMP:
			if (cu_dump())
				status |= CB_OK;
			break;

		case CB_DIAGNOSE:
			status |= CB_OK;
			break;
		}
	}
	else
		// abort status
		status |= CB_A;

	// complete command
	cu_complete(status);
}

void i82586_base_device::cu_complete(const u16 status)
{
	// clear busy status and set completion status
	const u32 cb_cs = m_space->read_dword(m_cba);
	m_space->write_dword(m_cba, (cb_cs & ~0xffffU) | CB_C | status);

	// chain to next command
	if (!(cb_cs & CB_EL))
	{
		// check for suspend or abort
		if (m_cu_state == CU_ACTIVE)
		{
			// fetch link address
			m_cba = address(m_cba, 4, 4);

			// restart timer
			m_cu_timer->adjust(attotime::zero);
		}
	}
	else
	{
		// no more commands
		m_cu_state = CU_IDLE;
		m_cna = true;
	}

	// suspend on completion
	if (cb_cs & CB_S)
	{
		m_cu_state = CU_SUSPENDED;
		m_cu_timer->enable(false);
		m_cna = true;
	}

	static const char *const CU_STATE_NAME[] = { "IDLE", "SUSPENDED", "ACTIVE" };
	LOG("cu_execute complete state %s\n", CU_STATE_NAME[m_cu_state]);

	// set command executed status
	m_cx = (cb_cs & CB_I) && (status & CB_OK);

	update_scb();
}

bool i82586_base_device::address_filter(u8 *mac)
{
	if (cfg_address_length() != 6)
	{
		LOG("address_filter error: address length %d not supported\n", cfg_address_length());

		return false;
	}

	LOGMASKED(LOG_FILTER, "address_filter testing destination address %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	if (cfg_promiscuous_mode())
	{
		LOG("address_filter accepted: promiscuous mode enabled\n");

		return true;
	}

	// ethernet broadcast
	if (!cfg_broadcast_disable() && !memcmp(mac, ETH_BROADCAST, cfg_address_length()))
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted: broadcast\n");

		return true;
	}

	// individual address
	if (!memcmp(mac, get_mac(), cfg_address_length()))
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted: individual address match\n");

		return true;
	}

	// ethernet multicast
	if ((mac[0] & 0x1) && m_mac_multi)
		if (m_mac_multi & address_hash(mac, cfg_address_length()))
		{
			LOGMASKED(LOG_FILTER, "address_filter accepted: multicast filter match\n");

			return true;
		}

	return false;
}

// shared helpers
void i82586_base_device::set_irq(bool irq)
{
	if (irq)
	{
		LOG("irq asserted\n");

		m_out_irq(m_irq_assert);
		m_out_irq(!m_irq_assert);
	}
}

u32 i82586_base_device::compute_crc(u8 *buf, int length, bool crc16)
{
	// TODO: crc16 (not used by Ethernet)
	return util::crc32_creator::simple(buf, length);
}

u64 i82586_base_device::address_hash(u8 *buf, int length)
{
	// address hash is computed using bits 2-7 from crc of address
	u32 crc = compute_crc(buf, length, false);

	return 1U << ((crc >> 2) & 0x3f);
}

int i82586_base_device::fetch_bytes(u8 *buf, u32 src, int length)
{
	int offset = 0;

	switch (m_space->data_width())
	{
	case 16:
		// handle misaligned start address
		if (src & 1)
		{
			buf[offset] = m_space->read_byte(src + offset);
			offset++;
		}

		// fetch aligned words from the source
		while (offset + 1 < length)
		{
			*(u16 *)&buf[offset] = m_space->read_word(src + offset);
			offset += 2;
		}

		// handle misaligned end address
		if ((src + length) & 1)
		{
			buf[offset] = m_space->read_byte(src + offset);
			offset++;
		}
		break;

	case 32:
		// handle misaligned start address
		switch (src & 3)
		{
		case 1:
			buf[offset] = m_space->read_byte(src + offset);
			offset++;

			*(u16 *)&buf[offset] = m_space->read_word(src + offset);
			offset += 2;
			break;

		case 2:
			*(u16 *)&buf[offset] = m_space->read_word(src + offset);
			offset += 2;
			break;

		case 3:
			buf[offset] = m_space->read_byte(src + offset);
			offset++;
			break;
		}

		// fetch aligned dwords from the source
		while (offset + 3 < length)
		{
			*(u32 *)&buf[offset] = m_space->read_dword(src + offset);
			offset += 4;
		}

		// handle misaligned end address
		switch ((src + length) & 3)
		{
		case 1:
			buf[offset] = m_space->read_byte(src + offset);
			offset++;
			break;

		case 2:
			*(u16 *)&buf[offset] = m_space->read_word(src + offset);
			offset += 2;
			break;

		case 3:
			*(u16 *)&buf[offset] = m_space->read_word(src + offset);
			offset += 2;
			buf[offset] = m_space->read_byte(src + offset);
			offset++;
			break;
		}
		break;
	}

	return offset;
}

int i82586_base_device::store_bytes(u32 dst, u8 *buf, int length)
{
	int offset = 0;

	switch (m_space->data_width())
	{
	case 16:
		// handle misaligned start address
		if (dst & 1)
		{
			m_space->write_byte(dst + offset, buf[offset]);
			offset++;
		}

		// store aligned words to the destination
		while (offset + 1 < length)
		{
			m_space->write_word(dst + offset, *(u16 *)&buf[offset]);
			offset += 2;
		}

		// handle misaligned end address
		if ((dst + length) & 1)
		{
			m_space->write_byte(dst + offset, buf[offset]);
			offset++;
		}
		break;

	case 32:
		// handle misaligned start address
		switch (dst & 3)
		{
		case 1:
			m_space->write_byte(dst + offset, buf[offset]);
			offset++;
			m_space->write_word(dst + offset, *(u16 *)&buf[offset]);
			offset += 2;
			break;

		case 2:
			m_space->write_word(dst + offset, *(u16 *)&buf[offset]);
			offset += 2;
			break;

		case 3:
			m_space->write_byte(dst + offset, buf[offset]);
			offset++;
			break;
		}

		// store aligned dwords to the destination
		while (offset + 3 < length)
		{
			m_space->write_dword(dst + offset, *(u32 *)&buf[offset]);
			offset += 4;
		}

		// handle misaligned end address
		switch ((dst + length) & 3)
		{
		case 1:
			m_space->write_byte(dst + offset, buf[offset]);
			offset++;
			break;

		case 2:
			m_space->write_word(dst + offset, *(u16 *)&buf[offset]);
			offset += 2;
			break;

		case 3:
			m_space->write_word(dst + offset, *(u16 *)&buf[offset]);
			offset += 2;
			m_space->write_byte(dst + offset, buf[offset]);
			offset++;
			break;
		}
		break;
	}

	return offset;
}

void i82586_base_device::dump_bytes(u8 *buf, int length)
{
	if (VERBOSE & LOG_FRAMES)
	{
		// pad frame with zeros to 8-byte boundary
		for (int i = 0; i < 8 - (length % 8); i++)
			buf[length + i] = 0;

		// dump length / 8 (rounded up) groups of 8 bytes
		for (int i = 0; i < (length + 7) / 8; i++)
			LOGMASKED(LOG_FRAMES, "%02x %02x %02x %02x %02x %02x %02x %02x\n",
				buf[i * 8 + 0], buf[i * 8 + 1], buf[i * 8 + 2], buf[i * 8 + 3],
				buf[i * 8 + 4], buf[i * 8 + 5], buf[i * 8 + 6], buf[i * 8 + 7]);
	}
}

// 82586 implementation
void i82586_device::device_start()
{
	i82586_base_device::device_start();

	save_item(NAME(m_rbd_offset));
	save_item(NAME(m_cfg_bytes));
}

void i82586_device::device_reset()
{
	i82586_base_device::device_reset();

	// configure parameter defaults
	memcpy(m_cfg_bytes, CFG_DEFAULTS, CFG_SIZE);
}

void i82586_device::initialise()
{
	// read iscp address from scp
	u32 iscp_address = m_space->read_dword(m_scp_address + 8);
	LOG("initialise iscp address 0x%08x\n", iscp_address);

	u16 scb_offset = m_space->read_word(iscp_address + 2);

	m_scb_base = m_space->read_dword(iscp_address + 4);
	m_scb_address = m_scb_base + scb_offset;
	LOG("initialise scb base address 0x%06x offset 0x%04x address 0x%08x\n", m_scb_base, scb_offset, m_scb_address);

	// clear iscp busy byte
	m_space->write_byte(iscp_address, 0);

	m_cx = true;
	m_cna = true;

	m_initialised = true;
	LOG("initialise complete\n");

	// update scb
	update_scb();
}

bool i82586_device::cu_iasetup()
{
	int len = cfg_address_length();
	char mac[6];
	u32 data;

	if (len != 6)
	{
		LOG("cu_iasetup unexpected individual address length %d != 6\n", len);

		return false;
	}

	data = m_space->read_dword(m_cba + 4);
	mac[0] = (data >> 16) & 0xff;
	mac[1] = (data >> 24) & 0xff;

	data = m_space->read_dword(m_cba + 8);
	mac[2] = (data >> 0) & 0xff;
	mac[3] = (data >> 8) & 0xff;
	mac[4] = (data >> 16) & 0xff;
	mac[5] = (data >> 24) & 0xff;

	LOG("cu_iasetup individual address %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	set_mac(mac);

	return true;
}

bool i82586_device::cu_configure()
{
	int count;

	// first two bytes
	u16 data = m_space->read_word(m_cba + 6);
	cfg_set(0, (data >> 0) & 0xff);
	cfg_set(1, (data >> 8) & 0xff);

	// extract byte count (4 <= count <= 12)
	count = cfg_get(0) & 0xf;
	count = count < 4 ? 4 : (count > CFG_SIZE ? CFG_SIZE : count);

	// read remaining bytes one word at a time
	for (int i = 2; i < count; i++)
	{
		if ((i & 1) == 0)
		{
			data = m_space->read_word(m_cba + 6 + i);
			cfg_set(i, (data >> 0) & 0xff);
		}
		else
			cfg_set(i, (data >> 8) & 0xff);
	}

	if (VERBOSE & LOG_CONFIG)
	{
		LOGMASKED(LOG_CONFIG, "%-30s %3s %3s %3s %s\n", "parameter", "def", "cur", "chg", "default value interpretation");
		for (auto param : CFG_PARAMS)
		{
			if (param.byte < (CFG_SIZE - 1))
			{
				u8 value = (m_cfg_bytes[param.byte] & param.mask) >> param.shift;

				LOGMASKED(LOG_CONFIG, "%-30s %3d %3d  %c  %s%s\n",
					param.name, param.dflt, value, value == param.dflt ? ' ' : '*', param.unit,
					param.ieee8023 ? (value == param.dflt ? "" : " (current value not 802.3 compatible)") : "");
			}
		}
	}

	return true;
}

bool i82586_device::cu_mcsetup()
{
	int addr_len = cfg_address_length();
	u16 mc_count;
	u8 data[6];

	if (addr_len != 6)
	{
		LOG("cu_mcsetup unexpected address length %d != 6\n", addr_len);
		return false;
	}

	// read the address count
	mc_count = m_space->read_word(m_cba + 6, TB_COUNT);

	// reset current list
	LOG("mc_setup configuring %d addresses\n", mc_count);
	m_mac_multi = 0;

	// read and process the addresses
	for (int i = 0; i < mc_count; i++)
	{
		*(u16 *)&data[0] = m_space->read_word(m_cba + 8 + i * 6 + 0);
		*(u16 *)&data[1] = m_space->read_word(m_cba + 8 + i * 6 + 2);
		*(u16 *)&data[2] = m_space->read_word(m_cba + 8 + i * 6 + 4);

		// add a hash of this address to the table
		m_mac_multi |= address_hash(data, cfg_address_length());

		LOG("mc_setup inserting address %02x:%02x:%02x:%02x:%02x:%02x\n",
			data[0], data[1], data[2], data[3], data[4], data[5]);
	}

	return true;
}

bool i82586_device::cu_transmit(u32 command)
{
	u16 tbd_count;

	// ethernet frame buffer
	u8 buf[MAX_FRAME_SIZE];
	u16 length = 0;

	u16 tbd_offset = m_space->read_word(m_cba + 6);

	// optionally insert source, destination address and length (14 bytes)
	if (!cfg_no_src_add_ins())
	{
		const char *mac = get_mac();
		u32 data;

		// insert destination address (6 bytes)
		data = m_space->read_dword(m_cba + 8);
		buf[length++] = (data >> 0) & 0xff;
		buf[length++] = (data >> 8) & 0xff;
		buf[length++] = (data >> 16) & 0xff;
		buf[length++] = (data >> 24) & 0xff;

		data = m_space->read_dword(m_cba + 12);
		buf[length++] = (data >> 0) & 0xff;
		buf[length++] = (data >> 8) & 0xff;

		// insert source address (6 bytes)
		LOG("cu_transmit inserting source address %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		for (int i = 0; i < 6; i++)
			buf[length++] = mac[i];

		// insert length (2 bytes)
		LOG("cu_transmit frame length 0x%04x\n", ((data >> 24) & 0xff) | ((data >> 16) & 0xff00));
		buf[length++] = (data >> 16) & 0xff;
		buf[length++] = (data >> 24) & 0xff;
	}

	// check if there is no tbd
	tbd_count = (tbd_offset == TBD_EMPTY) ? TB_EOF : 0;

	// insert payload from tbd
	while (!(tbd_count & TB_EOF))
	{
		u32 tbd_address = m_scb_base + tbd_offset;
		u32 tb_address, data;

		// get the size and address of this buffer, and address of the next descriptor
		data = m_space->read_dword(tbd_address + 0);
		tbd_count = (data >> 0) & 0xffff;
		tbd_offset = (data >> 16) & 0xffff;

		tb_address = m_space->read_dword(tbd_address + 4);

		LOG("cu_transmit inserting %d bytes from transmit buffer address 0x%08x\n", tbd_count & TB_COUNT, tb_address);
		length += fetch_bytes(&buf[length], tb_address, tbd_count & TB_COUNT);
	}

	// optionally compute/insert ethernet frame check sequence (4 bytes)
	if (!cfg_no_crc_insertion() && !cfg_loopback_mode())
	{
		LOG("cu_transmit inserting frame check sequence\n");

		u32 crc = compute_crc(buf, length, cfg_crc16());

		// insert the fcs
		buf[length++] = (crc >> 0) & 0xff;
		buf[length++] = (crc >> 8) & 0xff;
		buf[length++] = (crc >> 16) & 0xff;
		buf[length++] = (crc >> 24) & 0xff;
	}

	if (cfg_loopback_mode())
	{
		LOG("cu_transmit loopback frame length %d\n", length);

		int status = recv_start(buf, length);
		if (status)
			ru_complete(status);

		cu_complete(CB_OK);

		return true;
	}
	else
	{
		LOG("cu_transmit sending frame length %d\n", length);
		dump_bytes(buf, length);

		return send(buf, length) == length;
	}
}

void i82586_base_device::send_complete_cb(int result)
{
	// always turn on the heartbeat indicator status after a successful transmit; not
	// strictly correct, but allows one InterPro 2000 diagnostic to pass
	cu_complete(result ? (CB_OK | CB_S6) : CB_S9);
}

bool i82586_device::cu_tdreflect()
{
	m_space->write_word(m_cba + 6, TDR_LNK_OK | TDR_TIME);

	return true;
}

bool i82586_device::cu_dump()
{
	int length = DUMP_SIZE;
	u8 buf[DUMP_SIZE];
	u32 dump_address;

	// clear dump buffer
	memset(buf, 0, length);

	// populate dump buffer
	// configure bytes
	memcpy(&buf[0x00], &m_cfg_bytes[0], CFG_SIZE);

	// individual address
	memcpy(&buf[0x0c], get_mac(), 6);

	// hash register
	*(u64 *)&buf[0x24] = m_mac_multi;

	// store dump buffer
	dump_address = m_scb_base + m_space->read_word(m_cba + 6);

	LOG("cu_dump storing %d bytes address 0x%08x\n", length, dump_address);
	store_bytes(dump_address, buf, length);

	return true;
}

bool i82586_device::address_filter(u8 *mac)
{
	if (i82586_base_device::address_filter(mac))
		return true;

	LOGMASKED(LOG_FILTER, "address_filter rejected\n");

	return false;
}

u16 i82586_device::ru_execute(u8 *buf, int length)
{
	// fetch receive frame descriptor command/status
	u32 rfd_cs = m_space->read_dword(m_rfd);
	u16 status = 0;

	// current buffer position and bytes remaining
	int position = 0, remaining = length;

	// set busy status
	m_space->write_dword(m_rfd, rfd_cs | RFD_B);

	LOG("ru_execute receiving %d bytes into rfd 0x%08x\n", length, m_rfd);

	// set short frame status
	if (length < cfg_min_frame_length())
		status |= RFD_S_SHORT;

	// set crc status
	if (!cfg_loopback_mode() && ~compute_crc(buf, length, cfg_crc16()) != FCS_RESIDUE)
	{
		LOGMASKED(LOG_FRAMES, "ru_execute crc error computed 0x%08x stored 0x%08x\n",
			compute_crc(buf, length - 4, cfg_crc16()), *(u32 *)&buf[length - 4]);

		// increment crc error count
		m_space->write_word(m_scb_address + 8, m_space->read_word(m_scb_address + 8) + 1);

		status |= RFD_S_CRC;
	}

	// TODO: alignment error (crc in misaligned frame), status bit 10
	// TODO: increment alignment error counter

	// fetch initial rbd offset from rfd
	m_rbd_offset = m_space->read_word(m_rfd + 6);

	if (!cfg_no_src_add_ins())
	{
		// compute stored length (from 2 * addresses + length field)
		int actual = cfg_address_length() * 2 + 2;

		LOG("ru_execute storing %d bytes into rfd\n", actual);

		// store data in rfd
		store_bytes(m_rfd + 8, buf, actual);
		position += actual;
		remaining -= actual;
	}

	// store remaining bytes in receive buffers
	while (remaining && m_rbd_offset != RBD_EMPTY)
	{
		// fetch the count and address for this buffer
		u32 rb_address = m_space->read_dword(m_scb_base + m_rbd_offset + 4);
		u16 rbd_size = m_space->read_word(m_scb_base + m_rbd_offset + 8);

		// compute number of bytes to store in buffer
		int actual = remaining > (rbd_size & RB_SIZE) ? (rbd_size & RB_SIZE) : remaining;

		LOG("ru_execute storing %d bytes into receive buffer 0x%08x size %d\n", actual, rb_address, rbd_size & RB_SIZE);

		// store data in buffer
		store_bytes(rb_address, &buf[position], actual);
		position += actual;
		remaining -= actual;

		// store actual count
		m_space->write_word(m_scb_base + m_rbd_offset + 0, actual | RB_F | (remaining ? 0 : RB_EOF));

		// check if buffers exhausted
		if ((rbd_size & RB_EL))
		{
			m_rbd_offset = RBD_EMPTY;

			if (remaining)
			{
				// set buffers exhausted status
				status |= RFD_S_BUFFER;

				m_ru_state = RU_NR;
				m_rnr = true;
			}
		}
		else
			// fetch next rbd offset
			m_rbd_offset = m_space->read_word(m_scb_base + m_rbd_offset + 2);
	}

	if (remaining == 0 || cfg_save_bad_frames())
		// set frame received status
		status |= RFD_C;

	// frame received without errors
	if (!(status & RFD_ERROR_82586))
		status |= RFD_OK;

	return status;
}

void i82586_device::ru_complete(const u16 status)
{
	if (status & RFD_OK)
		LOG("ru_complete frame received without error\n");
	else
		LOG("ru_complete frame received with errors status 0x%04x\n", status);

	// update receive frame descriptor status
	u32 rfd_cs = m_space->read_dword(m_rfd);
	m_space->write_dword(m_rfd, (rfd_cs & ~0xffffU) | status);

	// if we received without error, or we're saving bad frames, advance to the next rfd
	if ((status & RFD_OK) || cfg_save_bad_frames())
	{
		if (!(rfd_cs & RFD_EL))
		{
			// advance to next rfd
			m_rfd = m_scb_base + m_space->read_word(m_rfd + 4);

			// store next free rbd address into rfd
			if (m_rbd_offset != RBD_EMPTY)
				m_space->write_word(m_rfd + 6, m_rbd_offset);
		}
		else
		{
			m_ru_state = RU_NR;
			m_rnr = true;
		}

		// set frame received status
		m_fr = true;
	}

	// suspend on completion
	if (rfd_cs & RFD_S)
	{
		m_ru_state = RU_SUSPENDED;
		m_rnr = true;
	}

	static const char *const RU_STATE_NAME[] = { "IDLE", "SUSPENDED", "NO RESOURCES", nullptr, "READY" };
	LOG("ru_complete complete state %s\n", RU_STATE_NAME[m_ru_state]);
}

u32 i82586_device::address(u32 base, int offset, int address, u16 empty)
{
	u16 data = m_space->read_word(base + offset);

	return (data == empty) ? empty : m_scb_base + data;
}

// 82596 implementation
void i82596_device::device_start()
{
	i82586_base_device::device_start();

	save_item(NAME(m_cfg_bytes));

	save_item(NAME(m_sysbus));

	save_item(NAME(m_irq_assert));
	save_item(NAME(m_rbd_address));
	save_item(NAME(m_mac_multi_ia));
}

void i82596_device::device_reset()
{
	i82586_base_device::device_reset();

	// configure parameter defaults
	memcpy(m_cfg_bytes, CFG_DEFAULTS, CFG_SIZE);
}

void i82596_device::port(u32 data)
{
	switch (data & 0xf)
	{
	case 0:
		// execute a software reset
		LOG("port reset\n");
		reset();
		break;

	case 1:
		// execute a self-test
		LOG("port self-test\n");
		break;

	case 2:
		// write an alterantive system configuration pointer address
		if (!m_initialised)
		{
			m_scp_address = data & ~0xf;
			LOG("port scp address 0x%08x\n", data);
		}
		break;

	case 3:
		// write an alternative dump area pointer and perform dump
		LOG("port dump\n");
		break;
	}
}

void i82596_device::initialise()
{
	// read iscp address and sysbus from scp
	u32 iscp_address = m_space->read_dword(m_scp_address + 8);
	m_sysbus = m_space->read_byte(m_scp_address + 2);

	LOG("initialise sysbus 0x%02x mode %s, %s triggering of bus throttle timers, lock function %s, interrupt active %s, 32-bit address pointers in linear mode per %s stepping)\n",
		m_sysbus,
		mode() == MODE_82586 ? "82586" : (mode() == MODE_32SEGMENTED ? "32-bit segmented mode" : (mode() == MODE_LINEAR ? "linear" : "reserved")),
		m_sysbus & SYSBUS_TRG ? "external" : "internal",
		m_sysbus & SYSBUS_LOCK ? "disabled" : "enabled",
		m_sysbus & SYSBUS_INT ? "low" : "high",
		m_sysbus & SYSBUS_BE ? "B" : "A1");
	LOG("initialise iscp address 0x%08x\n", iscp_address);

	switch (mode())
	{
	case MODE_82586:
	case MODE_32SEGMENTED:
	{
		u16 scb_offset = m_space->read_word(iscp_address + 2);

		m_scb_base = m_space->read_dword(iscp_address + 4);
		m_scb_address = m_scb_base + scb_offset;
		LOG("initialise scb base address 0x%08x offset 0x%04x address 0x%08x\n", m_scb_base, scb_offset, m_scb_address);
	}
		break;

	case MODE_LINEAR:
		m_scb_address = m_space->read_dword(iscp_address + 4);
		LOG("initialise scb address 0x%08x\n", m_scb_address);
		break;
	}

	// configure interrupt polarity
	m_irq_assert = (m_sysbus & SYSBUS_INT) ? 0 : 1;

	// clear iscp busy byte
	m_space->write_byte(iscp_address, 0);

	m_cx = true;
	m_cna = true;

	m_initialised = true;
	LOG("initialise complete\n");

	// update scb
	update_scb();
}

bool i82596_device::cu_iasetup()
{
	int len = cfg_address_length();
	u32 data;
	char mac[6];

	if (len != 6)
	{
		LOG("cu_iasetup unexpected individual address length %d != 6\n", len);

		return false;
	}

	switch (mode())
	{
	case MODE_82586:
	case MODE_32SEGMENTED:
		data = m_space->read_dword(m_cba + 4);
		mac[0] = (data >> 16) & 0xff;
		mac[1] = (data >> 24) & 0xff;

		data = m_space->read_dword(m_cba + 8);
		mac[2] = (data >> 0) & 0xff;
		mac[3] = (data >> 8) & 0xff;
		mac[4] = (data >> 16) & 0xff;
		mac[5] = (data >> 24) & 0xff;
		break;

	case MODE_LINEAR:
		data = m_space->read_dword(m_cba + 8);
		mac[0] = (data >> 0) & 0xff;
		mac[1] = (data >> 8) & 0xff;
		mac[2] = (data >> 16) & 0xff;
		mac[3] = (data >> 24) & 0xff;

		data = m_space->read_dword(m_cba + 12);
		mac[4] = (data >> 0) & 0xff;
		mac[5] = (data >> 8) & 0xff;
		break;
	}

	LOG("cu_iasetup individual address %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	set_mac(mac);

	return true;
}

bool i82596_device::cu_configure()
{
	u32 data32 = 0;
	u16 data16;
	int count;

	switch (mode())
	{
	case MODE_82586:
		// first two bytes are word aligned
		data16 = m_space->read_word(m_cba + 6);

		cfg_set(0, (data16 >> 0) & 0xff);
		cfg_set(1, (data16 >> 8) & 0xff);

		// extract byte count (datasheet does not state minimum count)
		count = cfg_get(0) & 0xf;
		count = count < 4 ? 4 : (count > i82586_device::CFG_SIZE ? i82586_device::CFG_SIZE : count);

		// read remaining bytes one dword at a time
		for (int i = 2; i < count; i++)
		{
			switch (i & 3)
			{
			case 2:
				data32 = m_space->read_dword(m_cba + 6 + i);
				cfg_set(i, (data32 >> 0) & 0xff);
				break;
			case 3: cfg_set(i, (data32 >> 8) & 0xff); break;
			case 0: cfg_set(i, (data32 >> 16) & 0xff); break;
			case 1: cfg_set(i, (data32 >> 24) & 0xff); break;
			}
		}
	break;

	case MODE_32SEGMENTED:
		// first two bytes are word aligned
		data16 = m_space->read_word(m_cba + 6);

		cfg_set(0, (data16 >> 0) & 0xff);
		cfg_set(1, (data16 >> 8) & 0xff);

		// extract byte count (datasheet does not state minimum count)
		count = cfg_get(0) & 0xf;
		count = count < 4 ? 4 : (count > CFG_SIZE ? CFG_SIZE : count);

		// read remaining bytes one dword at a time
		for (int i = 2; i < count; i++)
		{
			switch (i & 3)
			{
			case 2:
				data32 = m_space->read_dword(m_cba + 6 + i);
				cfg_set(i, (data32 >> 0) & 0xff);
				break;
			case 3: cfg_set(i, (data32 >> 8) & 0xff); break;
			case 0: cfg_set(i, (data32 >> 16) & 0xff); break;
			case 1: cfg_set(i, (data32 >> 24) & 0xff); break;
			}
		}
		break;

	case MODE_LINEAR:
		// bytes are all dword aligned
		data32 = m_space->read_dword(m_cba + 8);

		cfg_set(0, (data32 >> 0) & 0xff);
		cfg_set(1, (data32 >> 8) & 0xff);
		cfg_set(2, (data32 >> 16) & 0xff);
		cfg_set(3, (data32 >> 24) & 0xff);

		// extract byte count (datasheet does not state minimum count)
		count = cfg_get(0) & 0xf;
		count = count < 4 ? 4 : (count > CFG_SIZE ? CFG_SIZE : count);

		// read remaining bytes one dword at a time
		for (int i = 4; i < count; i++)
		{
			switch (i & 3)
			{
			case 0:
				data32 = m_space->read_dword(m_cba + 8 + i);
				cfg_set(i, (data32 >> 0) & 0xff);
				break;

			case 1: cfg_set(i, (data32 >> 8) & 0xff); break;
			case 2: cfg_set(i, (data32 >> 16) & 0xff); break;
			case 3: cfg_set(i, (data32 >> 24) & 0xff); break;
			}
		}
		break;
	}

	if (VERBOSE & LOG_CONFIG)
	{
		LOGMASKED(LOG_CONFIG, "%-30s %3s %3s %3s %s\n", "parameter", "def", "cur", "chg", "default value interpretation");
		for (auto param : CFG_PARAMS)
		{
			u8 value = (m_cfg_bytes[param.byte] & param.mask) >> param.shift;

			LOGMASKED(LOG_CONFIG, "%-30s %3d %3d  %c  %s%s\n",
				param.name, param.dflt, value, value == param.dflt ? ' ' : '*', param.unit,
				param.ieee8023 ? (value == param.dflt ? "" : " (current value not 802.3 compatible)") : "");
		}
	}

	return true;
}

bool i82596_device::cu_mcsetup()
{
	int addr_len = cfg_address_length();
	u16 mc_count = 0;

	int offset = 0;
	u8 data[20];
	bool multi_ia;

	if (addr_len != 6)
	{
		LOG("cu_mcsetup unexpected address length %d != 6\n", addr_len);
		return false;
	}

	switch (mode())
	{
	case MODE_82586:
	case MODE_32SEGMENTED:
		mc_count = m_space->read_word(m_cba + 6, TB_COUNT);
		break;

	case MODE_LINEAR:
		mc_count = m_space->read_word(m_cba + 8, TB_COUNT);
		offset = 2;
		break;
	}

	// if count is zero, release multicast list and finish
	if (mc_count == 0)
	{
		LOG("mc_setup multicast filter disabled\n");
		m_mac_multi = 0;

		return true;
	}

	// fetch the first word
	*(u32 *)&data[0] = m_space->read_dword(m_cba + 8);

	// multi ia when configured and lsb of first address is clear
	multi_ia = cfg_multi_ia() && !BIT(data[offset], 0);

	// clear existing list
	LOG("mc_setup configuring %d %s addresses\n", mc_count, multi_ia ? "multi-ia" : "multicast");
	(multi_ia ? m_mac_multi_ia : m_mac_multi) = 0;

	for (int i = 0; i < mc_count; i++)
	{
		// compute offset of address in 18 byte buffer
		int n = (i % 3) * 6;

		// read the next dword
		*(u32 *)&data[n + 6] = m_space->read_dword(m_cba + 8 + i * 4 + 4);

		// unaligned case needs special handling
		if (n == 12 && offset == 2)
			*(u16 *)&data[18] = *(u16 *)&data[0];

		// add a hash of this address to the table
		(multi_ia ? m_mac_multi_ia : m_mac_multi) |= address_hash(&data[n + offset], cfg_address_length());

		LOG("mc_setup inserting address %02x:%02x:%02x:%02x:%02x:%02x\n",
			data[n + offset + 0], data[n + offset + 1], data[n + offset + 2], data[n + offset + 3], data[n + offset + 4], data[n + offset + 5]);
	}

	return true;
}

bool i82596_device::cu_transmit(u32 command)
{
	u32 tbd_address;
	u16 tcb_count, tbd_count;

	// ethernet frame buffer
	u8 buf[MAX_FRAME_SIZE];
	u16 length = 0;

	// need offset into tcb for linear mode
	int offset = mode() == MODE_LINEAR ? 4 : 0;

	// fetch tbd address
	if (mode() != MODE_LINEAR)
	{
		u16 tbd_offset = m_space->read_word(m_cba + 6);

		tbd_address = (tbd_offset == TBD_EMPTY) ? tbd_offset : m_scb_base + tbd_offset;
	}
	else
		tbd_address = m_space->read_dword(m_cba + 8);

	// fetch the tcb count
	tcb_count = (mode() == MODE_82586) ? 0 : m_space->read_word(m_cba + 8 + offset);

	LOG("cu_transmit %s mode, crc insertion %s, tcb count %d, %s tbd\n",
		command & CB_SF ? "flexible" : "simplified", command & CB_NC ? "disabled" : "enabled", tcb_count & TB_COUNT, (tbd_address == TBD_EMPTY) ? "no" : "valid");

	if ((command & CB_SF) && !(tcb_count & TB_EOF))
		LOG("cu_transmit error: tcb eof not set in simplified mode\n");

	// insert payload from tcb when in simplified mode, or when flexible mode and tcb_count > 0
	if ((command & CB_SF) || (!(command & CB_SF) && (tcb_count & TB_COUNT)))
	{
		// optionally insert destination, source and length (14 bytes)
		if (!cfg_no_src_add_ins())
		{
			const char *mac = get_mac();
			u32 data;

			// insert destination address (6 bytes)
			data = m_space->read_dword(m_cba + 12 + offset);
			buf[length++] = (data >> 0) & 0xff;
			buf[length++] = (data >> 8) & 0xff;
			buf[length++] = (data >> 16) & 0xff;
			buf[length++] = (data >> 24) & 0xff;

			data = m_space->read_dword(m_cba + 16 + offset);
			buf[length++] = (data >> 0) & 0xff;
			buf[length++] = (data >> 8) & 0xff;

			// insert source address (6 bytes)
			LOG("cu_transmit inserting source address %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			for (int i = 0; i < 6; i++)
				buf[length++] = mac[i];

			// insert length from tcb (2 bytes)
			LOG("cu_transmit frame length 0x%04x\n", ((data >> 24) & 0xff) | ((data >> 16) & 0xff00));
			buf[length++] = (data >> 16) & 0xff;
			buf[length++] = (data >> 24) & 0xff;

			// insert payload from tcb
			LOG("cu_transmit inserting %d bytes from transmit command block\n", (tcb_count & TB_COUNT) - 8);
			length += fetch_bytes(&buf[length], m_cba + 20 + offset, (tcb_count & TB_COUNT) - 8);
		}
		else
		{
			// insert entire payload from tcb
			LOG("cu_transmit inserting %d bytes from transmit command block\n", tcb_count & TB_COUNT);
			length += fetch_bytes(&buf[length], m_cba + 12 + offset, tcb_count & TB_COUNT);
		}
	}
	else if (!cfg_no_src_add_ins())
		LOG("cu_transmit error: don't know how to insert source address in flexible mode without tcb payload\n");

	// check for no tbd
	tbd_count = ((tcb_count & TB_EOF) || (tbd_address == TBD_EMPTY)) ? TB_EOF : 0;

	// insert payload from tbd
	while (!(tbd_count & TB_EOF))
	{
		u32 data, tb_address = 0;

		// fetch the count and address for this buffer, and address of the next descriptor
		switch (mode())
		{
		case MODE_82586:
		case MODE_32SEGMENTED:
			data = m_space->read_dword(tbd_address + 0);
			tbd_count = data;
			tb_address = m_space->read_dword(tbd_address + 4);

			tbd_address = m_scb_base + (data >> 16);
			break;

		case MODE_LINEAR:
			tbd_count = m_space->read_word(tbd_address + 0);
			tb_address = m_space->read_dword(tbd_address + 8);

			tbd_address = m_space->read_dword(tbd_address + 4);
			break;
		}

		// fetch and insert the buffer bytes into our transmit buffer
		LOG("cu_transmit inserting %d bytes from transmit buffer address 0x%08x\n", tbd_count & TB_COUNT, tb_address);
		length += fetch_bytes(&buf[length], tb_address, tbd_count & TB_COUNT);
	}

	// optionally compute/insert ethernet frame check sequence (4 bytes)
	if (!cfg_no_crc_insertion() && !(command & CB_NC) && !cfg_loopback_mode())
	{
		LOG("cu_transmit inserting frame check sequence\n");

		u32 crc = compute_crc(buf, length, cfg_crc16());

		// append the fcs
		buf[length++] = (crc >> 0) & 0xff;
		buf[length++] = (crc >> 8) & 0xff;
		buf[length++] = (crc >> 16) & 0xff;
		buf[length++] = (crc >> 24) & 0xff;
	}

	if (cfg_loopback_mode())
	{
		LOG("cu_transmit loopback frame length %d\n", length);

		int status = recv_start(buf, length);
		if (status)
			ru_complete(status);

		cu_complete(CB_OK);

		return true;
	}
	else
	{
		LOG("cu_transmit sending frame length %d\n", length);
		dump_bytes(buf, length);

		return send(buf, length) == length;
	}
}

bool i82596_device::cu_tdreflect()
{
	switch (mode())
	{
	case MODE_82586:
	case MODE_32SEGMENTED:
		m_space->write_word(m_cba + 6, TDR_LNK_OK | TDR_TIME);
		break;

	case MODE_LINEAR:
		m_space->write_word(m_cba + 8, TDR_LNK_OK | TDR_TIME);
		break;
	}

	return true;
}

bool i82596_device::cu_dump()
{
	int length = mode() == MODE_82586 ? i82586_device::DUMP_SIZE : DUMP_SIZE;
	u8 buf[DUMP_SIZE];
	u32 dump_address;

	// clear dump buffer
	memset(buf, 0, length);

	// populate dump buffer
	if (mode() == MODE_82586)
	{
		// configure bytes 2-10
		memcpy(&buf[0x02], &m_cfg_bytes[2], 9);

		// individual address
		memcpy(&buf[0x0c], get_mac(), 6);

		// hash register
		*(u64 *)&buf[0x24] = m_mac_multi;
	}
	else
	{
		// configure bytes 2-13
		memcpy(&buf[0x00], &m_cfg_bytes[2], 12);

		// individual address
		memcpy(&buf[0x0e], get_mac(), 6);

		// hash register
		*(u64 *)&buf[0x26] = m_mac_multi;
	}

	// store dump buffer
	dump_address = address(m_cba, 6, 8);
	LOG("cu_dump storing %d bytes address 0x%08x\n", length, dump_address);
	store_bytes(dump_address, buf, length);

	return true;
}

bool i82596_device::address_filter(u8 *mac)
{
	if (i82586_base_device::address_filter(mac))
		return true;

	// check for accept all multicast
	if ((mac[0] & 0x1) && !cfg_mc_all())
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted: multicast and configured to accept all multicast\n");

		return true;
	}

	// not ethernet multicast, check multi-ia
	if (!(mac[0] & 0x1) && cfg_multi_ia() && m_mac_multi_ia)
	{
		if (m_mac_multi_ia & address_hash(mac, cfg_address_length()))
		{
			LOGMASKED(LOG_FILTER, "address_filter accepted: multi-ia filter match");

			return true;
		}
	}

	LOGMASKED(LOG_FILTER, "address_filter rejected\n");

	return false;
}

u16 i82596_device::ru_execute(u8 *buf, int length)
{
	// fetch receive frame descriptor command/status
	const u32 rfd_cs = m_space->read_dword(m_rfd);
	u16 status = 0;

	// offset into rfd/rbd for linear mode
	int linear_offset = mode() == MODE_LINEAR ? 4 : 0;

	// current buffer position and bytes remaining
	int position = 0, remaining = length;

	if (cfg_crc_in_memory())
		remaining -= 4;

	// set busy status
	m_space->write_dword(m_rfd, rfd_cs | RFD_B);

	LOG("ru_execute receiving %d bytes using %s mode into rfd 0x%08x\n",
		remaining, (mode() == MODE_82586 ? "82586" : ((rfd_cs & RFD_SF) ? "flexible" : "simplified")), m_rfd);

	// TODO: check length if configured, status bit 12

	// set short frame status
	if (length < cfg_min_frame_length())
	{
		LOGMASKED(LOG_FRAMES, "ru_execute frame length %d less than minimum %d\n", length, cfg_min_frame_length());

		// increment short frame count
		if (mode() != MODE_82586)
			m_space->write_dword(m_scb_address + 28 + linear_offset, m_space->read_dword(m_scb_address + 28 + linear_offset) + 1);

		status |= RFD_S_SHORT;
	}

	// set crc status
	if (!cfg_loopback_mode() && ~compute_crc(buf, length, cfg_crc16()) != FCS_RESIDUE)
	{
		LOGMASKED(LOG_FRAMES, "ru_execute crc error computed 0x%08x stored 0x%08x\n",
			compute_crc(buf, length - 4, cfg_crc16()), *(u32 *)&buf[length - 4]);

		// increment crc error count
		if (mode() == MODE_82586)
			m_space->write_word(m_scb_address + 8, m_space->read_word(m_scb_address + 8) + 1);
		else
			m_space->write_dword(m_scb_address + 8 + linear_offset, m_space->read_dword(m_scb_address + 8 + linear_offset) + 1);

		status |= RFD_S_CRC;
	}

	// TODO: alignment error (crc in misaligned frame), status bit 10
	// TODO: increment alignment error counter

	// set multicast status
	if (mode() != MODE_82586 && memcmp(buf, get_mac(), cfg_address_length()))
		status |= RFD_S_MULTICAST;

	// fetch initial rbd address from rfd
	m_rbd_address = address(m_rfd, 6, 8, RBD_EMPTY);

	// check for simplified mode
	if (mode() != MODE_82586 && !(rfd_cs & RFD_SF))
	{
		// fetch size word
		u16 rfd_size = m_space->read_word(m_rfd + 10 + linear_offset, RB_SIZE);

		// increment "no resources" counter
		if (rfd_size < remaining)
			m_space->write_dword(m_scb_address + 16 + linear_offset, m_space->read_dword(m_scb_address + 16 + linear_offset) + 1);

		// truncate/capture the frame
		if (remaining <= rfd_size || cfg_save_bad_frames())
		{
			// compute stored length
			int actual = (rfd_size < remaining) ? rfd_size : remaining;

			LOG("ru_execute storing %d bytes into rfd size %d\n", actual, rfd_size);

			// store data in rfd
			store_bytes(m_rfd + 12 + linear_offset, buf, actual);

			// store actual count, f and eof
			m_space->write_word(m_rfd + 8 + linear_offset, actual | RB_F | RB_EOF);

			// set frame received and truncated frame status
			status |= RFD_C | (actual < remaining ? RFD_S_TRUNCATED : 0);

			position += actual;
			remaining -= actual;
		}
		else
			LOG("ru_execute discarding %d byte frame exceeding rfd size %d\n", remaining, rfd_size);
	}
	else
	{
		// flexible mode, store leading data into rfd
		if (mode() != MODE_82586)
		{
			// fetch size word
			u16 rfd_size = m_space->read_word(m_rfd + 10 + linear_offset, RB_SIZE);

			// compute stored length (from rfd_size)
			int actual = (rfd_size < remaining) ? rfd_size : remaining;

			LOG("ru_execute storing %d bytes into rfd size %d\n", actual, rfd_size);

			// store data in rfd
			store_bytes(m_rfd + 12 + linear_offset, buf, actual);
			position += actual;
			remaining -= actual;

			// store actual count, f and eof
			m_space->write_word(m_rfd + 8 + linear_offset, actual | RB_F | (remaining ? 0 : RB_EOF));
		}
		else if (!cfg_no_src_add_ins())
		{
			// compute stored length (from 2 * addresses + length field)
			int actual = cfg_address_length() * 2 + 2;

			LOG("ru_execute storing %d bytes into rfd\n", actual);

			// store data in rfd
			store_bytes(m_rfd + 8, buf, actual);
			position += actual;
			remaining -= actual;
		}

		// store remaining bytes in receive buffers
		while (remaining && m_rbd_address != RBD_EMPTY)
		{
			// fetch the count and address for this buffer
			u32 rb_address = m_space->read_dword(m_rbd_address + 4 + linear_offset);
			u16 rbd_size = m_space->read_word(m_rbd_address + 8 + linear_offset);

			// compute number of bytes to store in buffer
			int actual = remaining > (rbd_size & RB_SIZE) ? (rbd_size & RB_SIZE) : remaining;

			LOG("ru_execute storing %d bytes into receive buffer 0x%08x size %d\n", actual, rb_address, rbd_size & RB_SIZE);

			// store data in buffer
			store_bytes(rb_address, &buf[position], actual);
			position += actual;
			remaining -= actual;

			// store actual count
			m_space->write_word(m_rbd_address + 0, actual | RB_F | (remaining ? 0 : RB_EOF));

			// check if buffers exhausted
			if ((rbd_size & RB_EL))
			{
				m_rbd_address = RBD_EMPTY;

				if (remaining)
				{
					// set buffers exhausted status
					status |= RFD_S_BUFFER;

					m_ru_state = mode() == MODE_82586 ? RU_NR : RU_NR_RBD;
					m_rnr = true;
				}
			}
			else
				// fetch next rbd address
				m_rbd_address = address(m_rbd_address, 2, 4);
		}

		if (remaining == 0 || cfg_save_bad_frames())
			// set frame received status
			status |= RFD_C;
	}

	if (!(status & (mode() == MODE_82586 ? RFD_ERROR_82586 : RFD_ERROR)))
		status |= RFD_OK;

	return status;
}

void i82596_device::ru_complete(const u16 status)
{
	if (status & RFD_OK)
		LOG("ru_complete frame received without error\n");
	else
		LOG("ru_complete frame received with errors status 0x%04x\n", status);

	// store status
	const u32 rfd_cs = m_space->read_dword(m_rfd);
	m_space->write_dword(m_rfd, (rfd_cs & ~0xffffU) | status);

	// if we received without error, or we're saving bad frames, advance to the next rfd
	if ((rfd_cs & RFD_OK) || cfg_save_bad_frames())
	{
		if (!(rfd_cs & RFD_EL))
		{
			// advance to next rfd
			m_rfd = address(m_rfd, 4, 4);

			// store next free rbd address into rfd
			if (m_rbd_address != RBD_EMPTY)
			{
				if (mode() == MODE_LINEAR)
					m_space->write_dword(m_rfd + 8, m_rbd_address);
				else
					m_space->write_word(m_rfd + 6, m_rbd_address - m_scb_base);
			}
		}
		else
		{
			m_ru_state = mode() == MODE_82586 ? RU_NR : RU_NR_RFD;
			m_rnr = true;
		}

		// set frame received status
		m_fr = true;
	}

	// suspend on completion
	if (rfd_cs & RFD_S)
	{
		m_ru_state = RU_SUSPENDED;
		m_rnr = true;
	}

	static const char *const RU_STATE_NAME[] = { "IDLE", "SUSPENDED", "NO RESOURCES", nullptr, "READY", nullptr, nullptr, nullptr, nullptr, nullptr, "NO RESOURCES (RFD)", nullptr, "NO RESOURCES (RBD)" };
	LOG("ru_complete complete state %s\n", RU_STATE_NAME[m_ru_state]);
}

u32 i82596_device::address(u32 base, int offset, int address, u16 empty)
{
	if (mode() != MODE_LINEAR)
	{
		u16 data = m_space->read_word(base + offset);

		return (data == empty) ? empty : m_scb_base + data;
	}
	else
		return m_space->read_dword(base + address);
}
