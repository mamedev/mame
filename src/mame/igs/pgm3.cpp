// license:BSD-3-Clause
// copyright-holders:Xing Xing, Peter Wilhelmsen

/* PGM 3 hardware.

  Games on this platform

  Knights of Valour 3 HD

  according to Xing Xing
  "The main cpu of PGM3 whiched coded as 'SOC38' is an ARM1176@800M designed by SOCLE(http://www.socle-tech.com/). Not much infomation is available on this asic"

  the card images seem to have encrypted data up to the C2000000 mark, then
  some text string about a non-bootable disk followed by mostly blank data

  Offset(h)  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F

0C2000000  EB 3C 90 6D 6B 64 6F 73 66 73 00 00 02 04 04 00  ë<.mkdosfs......
0C2000010  02 00 02 00 00 F8 00 01 3F 00 FF 00 00 00 00 00  .....ø..?.ÿ.....
0C2000020  00 00 04 00 00 00 29 4C 88 BA 7C 20 20 20 20 20  ......)L.º|
0C2000030  20 20 20 20 20 20 46 41 54 31 36 20 20 20 0E 1F        FAT16   ..
0C2000040  BE 5B 7C AC 22 C0 74 0B 56 B4 0E BB 07 00 CD 10  ¾[|¬"Àt.V´.»..Í.
0C2000050  5E EB F0 32 E4 CD 16 CD 19 EB FE 54 68 69 73 20  ^ëð2äÍ.Í.ëþThis
0C2000060  69 73 20 6E 6F 74 20 61 20 62 6F 6F 74 61 62 6C  is not a bootabl
0C2000070  65 20 64 69 73 6B 2E 20 20 50 6C 65 61 73 65 20  e disk.  Please
0C2000080  69 6E 73 65 72 74 20 61 20 62 6F 6F 74 61 62 6C  insert a bootabl
0C2000090  65 20 66 6C 6F 70 70 79 20 61 6E 64 0D 0A 70 72  e floppy and..pr
0C20000A0  65 73 73 20 61 6E 79 20 6B 65 79 20 74 6F 20 74  ess any key to t
0C20000B0  72 79 20 61 67 61 69 6E 20 2E 2E 2E 20 0D 0A 00  ry again ... ...


  DSW:
    1: OFF = Game mode / ON = Test mode
    2: OFF = JAMMA / ON = JVS
    3: OFF = 16/9 (1280x720) / ON = 4/3 (800x600)
    4: NO USE

  todo: add other hardware details?

*/

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "imagedev/harddriv.h"
#include "machine/nvram.h"
#include "machine/pl080.h"
#include "machine/s35390a.h"
#include "machine/vic_pl192.h"
#include "video/cat6613.h"
#include "video/hantro_g1.h"
#include "video/mali200.h"

#include "screen.h"
#include "speaker.h"

#include "aes128cbc.h"
#include "aes128ecb.h"
#include "aes192cbc.h"
#include "aes192ecb.h"
#include "aes256cbc.h"
#include "aes256ecb.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

// Enable individual transaction traces when debugging the corresponding bus.
#define LOG_SPI (1U << 1)
#define LOG_IO (1U << 2)
#define LOG_SDHC (1U << 3)
#define LOG_SPACC (1U << 4)
#define LOG_AUDIO (1U << 6)
#define VERBOSE (0)
#include "logmacro.h"

namespace {

#include "igs38_sdhc.hxx"
#include "igs38_timer.hxx"
#include "igs38_i2c.hxx"
#include "igs38_spi.hxx"
#include "pgm3_io.hxx"
#include "igs38_i2s.hxx"

// Minimal model of the older SPAcc core in IGS38. Register accesses are traced
// from IgsSpacc_crypt at mask ROM 0x3d08 (called at 0x1070).
// AES ECB/CBC, context 0 and polled DDT jobs are implemented.
class igs38_spacc_device : public device_t
{
public:
	igs38_spacc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_dma_space(T &&tag, int space) { m_dma.set_tag(std::forward<T>(tag), space); }
	auto dma_permissions_callback() { return m_dma_permissions.bind(); }

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum : u32
	{
		IRQ_EN = 0x00 / 4,
		IRQ_STAT = 0x04 / 4,
		IRQ_CTRL = 0x08 / 4,
		FIFO_STAT = 0x0c / 4,
		SRC_PTR = 0x20 / 4,
		DST_PTR = 0x24 / 4,
		OFFSET = 0x28 / 4,
		PRE_AAD_LEN = 0x2c / 4,
		POST_AAD_LEN = 0x30 / 4,
		PROC_LEN = 0x34 / 4,
		ICV_LEN = 0x38 / 4,
		ICV_OFFSET = 0x3c / 4,
		IV_OFFSET = 0x40 / 4,
		SW_CTRL = 0x44 / 4,
		AUX_INFO = 0x48 / 4,
		CTRL = 0x4c / 4,
		STAT_POP = 0x50 / 4,
		STATUS = 0x54 / 4,
		KEY_SZ = 0x100 / 4,
		ID = 0x180 / 4
	};

	enum : u8
	{
		OK = 0,
		MEMERR = 2,
		BLOCKERR = 3
	};

	// Model limits, not measured hardware capacities. The ROM uses one job
	// at a time, at most 0x8000 bytes, with twenty-entry descriptor workspaces.
	static constexpr u32 FIFO_DEPTH = 16;
	static constexpr u32 MAX_DDT = 20;
	static constexpr u32 MAX_PACKET = 0x8000;

	struct dma_segment
	{
		u32 address;
		u32 length;
	};

	u32 regs_r(offs_t offset);
	void regs_w(offs_t offset, u32 data, u32 mem_mask = ~0U);

	u32 context_r(offs_t offset) { return m_context[offset]; }
	void context_w(offs_t offset, u32 data, u32 mem_mask = ~0U) { COMBINE_DATA(&m_context[offset]); }

	bool dma_range(u32 address, u32 length, bool write);
	bool read_ddt(u32 address, u32 length, bool write, std::vector<dma_segment> &segments);
	template <typename Context, typename Init, typename Process, typename... Args>
	static void crypt_blocks(Init init, Process process, void (*cleanup)(Context *), std::vector<u8> const &input, std::vector<u8> &output, Args... args);
	u8 crypt();
	void submit();

	required_address_space m_dma;
	devcb_read8 m_dma_permissions;
	u32 m_regs[0x184 / 4];
	u32 m_context[16];
	u32 m_fifo[FIFO_DEPTH];
	u8 m_fifo_head;
	u8 m_fifo_count;
};

DEFINE_DEVICE_TYPE_PRIVATE(IGS38_SPACC, igs38_spacc_device, igs38_spacc_device, "igs38_spacc", "IGS38 SPAcc (AES boot subset)")

igs38_spacc_device::igs38_spacc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS38_SPACC, tag, owner, clock)
	, m_dma(*this, finder_base::DUMMY_TAG, -1)
	, m_dma_permissions(*this, 0)
{
}

void igs38_spacc_device::device_start()
{
	save_item(NAME(m_regs));
	save_item(NAME(m_context));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_count));
}

void igs38_spacc_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_context), std::end(m_context), 0);
	std::fill(std::begin(m_fifo), std::end(m_fifo), 0);
	m_fifo_head = m_fifo_count = 0;
}

void igs38_spacc_device::map(address_map &map)
{
	map(0x0000, 0x0183).rw(FUNC(igs38_spacc_device::regs_r), FUNC(igs38_spacc_device::regs_w));
	map(0x4000, 0x403f).rw(FUNC(igs38_spacc_device::context_r), FUNC(igs38_spacc_device::context_w));
}

u32 igs38_spacc_device::regs_r(offs_t offset)
{
	if (offset == FIFO_STAT)
	{
		// This revision uses [22:16] for status count and bit 31 for empty.
		// Jobs complete synchronously, so there is no pending command count.
		return (u32(m_fifo_count) << 16) | (m_fifo_count ? 0 : 0x80000000U) | ((m_fifo_count == FIFO_DEPTH) ? 0x8000 : 0);
	}
	return m_regs[offset];
}

void igs38_spacc_device::regs_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset)
	{
	case IRQ_STAT:
		m_regs[offset] &= ~(data & mem_mask); // write-one-to-clear
		break;
	case FIFO_STAT:
	case STATUS:
	case ID:
		break; // read-only; actual silicon ID is not known
	case STAT_POP:
		// POP latches a result in STATUS. Reading STATUS must not pop again,
		// including debugger reads and reads after the FIFO becomes empty.
		if ((data & mem_mask & 1) && m_fifo_count)
		{
			m_regs[STATUS] = m_fifo[m_fifo_head];
			m_fifo_head = (m_fifo_head + 1) % FIFO_DEPTH;
			--m_fifo_count;
		}
		break;
	case SRC_PTR:
	case DST_PTR:
		COMBINE_DATA(&m_regs[offset]);
		m_regs[offset] &= ~7U; // eight-byte DDT entries; ROM probes low bits
		break;
	case SW_CTRL:
		// The mask ROM reads the next eight-bit job tag before submitting;
		// unlike newer SPAcc drivers, it never supplies a tag by writing here.
		COMBINE_DATA(&m_regs[offset]);
		m_regs[offset] &= 0xff;
		break;
	case CTRL:
		if (mem_mask == 0xffffffff)
		{
			m_regs[offset] = data;
			submit();
		}
		break;
	case IRQ_EN:
	case IRQ_CTRL:
	case OFFSET:
	case PRE_AAD_LEN:
	case POST_AAD_LEN:
	case PROC_LEN:
	case ICV_LEN:
	case ICV_OFFSET:
	case IV_OFFSET:
	case AUX_INFO:
	case KEY_SZ:
		COMBINE_DATA(&m_regs[offset]);
		break;
	default:
		if (data & mem_mask)
			logerror("unimplemented register write +%03x = %08x & %08x\n", offset * 4, data, mem_mask);
		break;
	}
}

bool igs38_spacc_device::dma_range(u32 address, u32 length, bool write)
{
	if (!length || (u64(address) + length > 0x1'0000'0000ULL))
		return false;
	// The board supplies RAM/ROM permissions so an invalid job cannot invoke
	// MMIO recursively. Do not use get_write_ptr: the CPU's exclusive-monitor
	// write tap (or a debugger watchpoint) can hide otherwise valid RAM.
	// Actual transfers still use the bus, preserving taps and memory views.
	for (u64 page = address & ~u64(0xfff); page < u64(address) + length; page += 0x1000)
	{
		if (!(m_dma_permissions(u32(page)) & (write ? 2 : 1)))
			return false;
	}
	return true;
}

bool igs38_spacc_device::read_ddt(u32 address, u32 length, bool write, std::vector<dma_segment> &segments)
{
	// A DDT is a flat array of little-endian (physical address, byte count)
	// pairs. Zero length terminates it; PROC_LEN bounds the actual transfer.
	for (u32 entry = 0; entry != MAX_DDT && length; ++entry)
	{
		const u64 descriptor = u64(address) + entry * 8;
		if (descriptor + 8 > 0x1'0000'0000ULL || !dma_range(u32(descriptor), 8, false))
			return false;
		const u32 buffer = m_dma->read_dword(u32(descriptor));
		const u32 bytes = std::min(length, m_dma->read_dword(u32(descriptor) + 4));
		if (!dma_range(buffer, bytes, write))
			return false;
		segments.push_back({ buffer, bytes });
		length -= bytes;
	}
	return !length;
}

template <typename Context, typename Init, typename Process, typename... Args>
void igs38_spacc_device::crypt_blocks(Init init, Process process, void (*cleanup)(Context *), std::vector<u8> const &input, std::vector<u8> &output, Args... args)
{
	Context context;
	init(&context, args...);
	if constexpr (std::is_invocable_v<Process, Context *, u8 const *, u32, u8 *>)
	{
		process(&context, input.data(), u32(input.size()), output.data());
	}
	else
	{
		for (u32 i = 0; i < input.size(); i += 16)
			process(&context, &input[i], &output[i]);
	}
	cleanup(&context);
}

u8 igs38_spacc_device::crypt()
{
	const u32 length = m_regs[PROC_LEN];
	// CTRL: cipher 2 (AES), mode 0/1 (ECB/CBC), encrypt bit 24,
	// context 0. KEY_EXP bit 29 is accepted; expand the key for each job.
	u32 const key_bytes = m_regs[KEY_SZ] & 0x7fffffff;
	bool const encrypt = BIT(m_regs[CTRL], 24), cbc = BIT(m_regs[CTRL], 8);
	if ((m_regs[CTRL] & ~0x21000100U) != 2 || !BIT(m_regs[KEY_SZ], 31) || (key_bytes != 16 && key_bytes != 24 && key_bytes != 32) ||
			m_regs[OFFSET] || m_regs[PRE_AAD_LEN] || m_regs[POST_AAD_LEN] || m_regs[ICV_LEN] || m_regs[ICV_OFFSET] || m_regs[IV_OFFSET] ||
			m_regs[AUX_INFO] || !length || (length & 15) || length > MAX_PACKET)
		return BLOCKERR;

	std::vector<dma_segment> source, destination;
	if (!read_ddt(m_regs[SRC_PTR], length, false, source) || !read_ddt(m_regs[DST_PTR], length, true, destination))
		return MEMERR;

	// Gather before writing, so in-place transfers and DDT boundaries within
	// AES blocks are safe. Invalid descriptors never produce partial output.
	std::vector<u8> input(length), output(length);
	u32 pos = 0;
	for (const auto &segment : source)
	{
		for (u32 i = 0; i != segment.length; ++i)
			input[pos++] = m_dma->read_byte(segment.address + i);
	}

	u8 key[32], iv[16];
	for (u32 i = 0; i != 32; ++i)
		key[i] = m_context[i / 4] >> (8 * (i & 3));
	for (u32 i = 0; i != 16; ++i)
		iv[i] = m_context[8 + i / 4] >> (8 * (i & 3));
	switch (key_bytes)
	{
	case 16:
		if (cbc)
		{
			crypt_blocks(encrypt ? aes128cbc::AES_EncryptInit : aes128cbc::AES_DecryptInit,
					encrypt ? aes128cbc::AES_Encrypt : aes128cbc::AES_Decrypt, aes128cbc::AES_CTX_Free, input, output, key, iv);
		}
		else
		{
			crypt_blocks(encrypt ? aes128ecb::AES_EncryptInit : aes128ecb::AES_DecryptInit,
					encrypt ? aes128ecb::AES_Encrypt : aes128ecb::AES_Decrypt, aes128ecb::AES_CTX_Free, input, output, key);
		}
		break;
	case 24:
		if (cbc)
		{
			crypt_blocks(encrypt ? aes192cbc::AES_EncryptInit : aes192cbc::AES_DecryptInit,
					encrypt ? aes192cbc::AES_Encrypt : aes192cbc::AES_Decrypt, aes192cbc::AES_CTX_Free, input, output, key, iv);
		}
		else
		{
			crypt_blocks(encrypt ? aes192ecb::AES_EncryptInit : aes192ecb::AES_DecryptInit,
					encrypt ? aes192ecb::AES_Encrypt : aes192ecb::AES_Decrypt, aes192ecb::AES_CTX_Free, input, output, key);
		}
		break;
	case 32:
		if (cbc)
		{
			crypt_blocks(encrypt ? aes256cbc::AES_EncryptInit : aes256cbc::AES_DecryptInit,
					encrypt ? aes256cbc::AES_Encrypt : aes256cbc::AES_Decrypt, aes256cbc::AES_CTX_Free, input, output, key, iv);
		}
		else
		{
			crypt_blocks(encrypt ? aes256ecb::AES_EncryptInit : aes256ecb::AES_DecryptInit,
					encrypt ? aes256ecb::AES_Encrypt : aes256ecb::AES_Decrypt, aes256ecb::AES_CTX_Free, input, output, key);
		}
		break;
	}

	pos = 0;
	for (const auto &segment : destination)
	{
		for (u32 i = 0; i != segment.length; ++i)
			m_dma->write_byte(segment.address + i, output[pos++]);
	}
	// No automatic context write-back yet. The ROM reloads the IV from the
	// last ciphertext block before each subsequent 0x8000-byte packet.
	return OK;
}

void igs38_spacc_device::submit()
{
	if (m_fifo_count == FIFO_DEPTH)
	{
		logerror("command rejected: completion FIFO full\n");
		return;
	}
	const u8 tag = m_regs[SW_CTRL];
	const u8 result = crypt();
	m_fifo[(m_fifo_head + m_fifo_count) % FIFO_DEPTH] = (u32(result) << 24) | tag;
	++m_fifo_count;
	m_regs[SW_CTRL] = u8(tag + 1);
	m_regs[IRQ_STAT] |= 0x10; // status event; VIC delivery is not implemented
	if (result != OK)
		logerror("job %02x failed: result=%u\n", tag, result);
	LOGMASKED(LOG_SPACC, "job %02x ctrl=%08x key_sz=%08x src_ddt=%08x dst_ddt=%08x length=%08x result=%u\n", tag, m_regs[CTRL], m_regs[KEY_SZ],
			m_regs[SRC_PTR], m_regs[DST_PTR], m_regs[PROC_LEN], result);
}

// Synthetic monitor: 720p60 preferred, 800x600p60, 640x480p60 and stereo LPCM.
constexpr u8 MONITOR_EDID[256] =
{
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x34, 0x2d, 0x38, 0x03, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x20, 0x01, 0x03, 0x80, 0x20, 0x12, 0x78, 0x0e, 0xee, 0x91, 0xa3, 0x54, 0x4c, 0x99, 0x26,
	0x0f, 0x50, 0x54, 0x21, 0x00, 0x00, 0x45, 0x40, 0x81, 0xc0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x1d, 0x00, 0x72, 0x51, 0xd0, 0x1e, 0x20, 0x6e, 0x28,
	0x55, 0x00, 0x40, 0xb4, 0x10, 0x00, 0x00, 0x1e, 0xa0, 0x0f, 0x20, 0x00, 0x31, 0x58, 0x1c, 0x20,
	0x28, 0x80, 0x14, 0x00, 0x40, 0xb4, 0x10, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x4d,
	0x41, 0x4d, 0x45, 0x20, 0x50, 0x47, 0x4d, 0x33, 0x0a, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfd,
	0x00, 0x32, 0x4b, 0x1e, 0x50, 0x08, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x00,
	0x02, 0x03, 0x18, 0xc1, 0x42, 0x84, 0x01, 0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x65,
	0x03, 0x0c, 0x00, 0x10, 0x00, 0xe2, 0x00, 0x6a, 0x01, 0x1d, 0x00, 0x72, 0x51, 0xd0, 0x1e, 0x20,
	0x6e, 0x28, 0x55, 0x00, 0x40, 0xb4, 0x10, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd1,
};


class pgm3_state : public driver_device
{
public:
	pgm3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_spacc(*this, "spacc")
		, m_sdhc(*this, "sdhc%u", 0U)
		, m_vic(*this, "vic%u", 0U)
		, m_timers(*this, "timer%u", 0U)
		, m_i2c(*this, "i2c%u", 0U)
		, m_spi(*this, "spi%u", 0U)
		, m_io(*this, "io")
		, m_hdmi(*this, "hdmi")
		, m_rtc(*this, "rtc")
		, m_mali(*this, "mali")
		, m_hantro(*this, "hantro")
		, m_dma(*this, "dma%u", 0U)
		, m_i2s(*this, "i2s")
		, m_boot_view(*this, "boot")
	{
	}

	void pgm3(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(hdmi_connected_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	struct scu_register
	{
		u32 reset;
		u32 writable;
	};

	// SCU reset values and writable bit masks.
	// Timer and I2C PCLK are derived below; other clocks and power domains
	// are still configuration latches rather than a complete clock/power model.
	static constexpr scu_register SCU_REGISTERS[] = {
		{ 0x49475338, 0x00000000 }, // 00 CID
		{ 0x01000031, 0x173f07ff }, // 04 MPLLCON
		{ 0x0100013a, 0x173f07ff }, // 08 UPLLCON
		{ 0x01010162, 0x173f07ff }, // 0c VPLLCON
		{ 0x0101002c, 0x173f07ff }, // 10 ENCPLLCON
		{ 0x0101002c, 0x173f07ff }, // 14 DECPLLCON
		{ 0x06400000, 0xffff7f55 }, // 18 CLKCFG1
		{ 0x00000000, 0xffffff5f }, // 1c CLKGATE1
		{ 0x00000000, 0x0bffffff }, // 20 CLKGATE2
		{ 0x00000000, 0x00770000 }, // 24 CLKDIV
		{ 0x00000000, 0x00000000 }, // 28 REMAP (command)
		{ 0x00000000, 0xffffffff }, // 2c PWRCON
		{ 0x00000000, 0x00000000 }, // 30 SWRESET (command)
		{ 0x00000000, 0x803ffbff }, // 34 CHIPCFG
		{ 0x00000000, 0x00007777 }, // 38 PWRDOMAIN
		{ 0x00000000, 0x00030301 }, // 3c IPCFG
		{ 0x00000000, 0xffffffff }, // 40 INFORMATIONA
		{ 0x00000000, 0xffffffff }, // 44 INFORMATIONB
		{ 0x00000000, 0xffffffff }, // 48 INFORMATIONC
		{ 0x00080808, 0xffffffff }, // 4c OTGCFG012
		{ 0x00000000, 0xffffffff }, // 50 INFORMATION1
		{ 0x00000000, 0xffffffff }, // 54 INFORMATION2
		{ 0x00000000, 0x00000000 }, // 58 STATUS
		{ 0x5555a6aa, 0xff3ff3ff }, // 5c PADDRV1 (reserved fields retain reset values)
		{ 0x00005555, 0x0000ffff }, // 60 PADDRV2
		{ 0x0101002c, 0x173f07ff }, // 64 CPUPLLCON
		{ 0x00000000, 0xffffffff }, // 68 INFORMATIOND
		{ 0x00000000, 0x0000001f }, // 6c CHIPCFG2
		{ 0x00000000, 0x000000ff }, // 70 CLKGATE3
		{ 0x00000000, 0xffffffff }, // 74 AUDIOCON (configuration latch)
	};

	u32 screen_update_pgm3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void lcd_map(address_map &map) ATTR_COLD;
	u32 lcd_status_r();
	void lcd_register_w(u32 reg, u32 data, u32 mem_mask);
	void lcd_configure_screen();
	void lcd_update_irq();
	void lcd_vblank(int state);

	void pgm3_map(address_map &map) ATTR_COLD;
	u32 scu_r(offs_t offset);
	void scu_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 scu_ext_r(offs_t offset);
	void peripheral_clocks();
	u8 spacc_dma_permissions(offs_t address);
	template <u32 Bank> void gpio_map(address_map &map) ATTR_COLD;
	u8 gpio_pins(u32 bank, u32 port);
	void gpio_update(u32 bank);
	u16 gpio_status(u32 bank);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<igs38_spacc_device> m_spacc;
	required_device_array<igs38_sdhc_device, 3> m_sdhc;
	required_device_array<vic_pl192_device, 2> m_vic;
	required_device_array<igs38_timer_device, 3> m_timers;
	required_device_array<igs38_i2c_device, 3> m_i2c;
	required_device_array<igs38_spi_device, 2> m_spi;
	required_device<pgm3_io_device> m_io;
	required_device<cat6613_device> m_hdmi;
	required_device<s35390a_device> m_rtc;
	required_device<mali200_device> m_mali;
	required_device<hantro_g1_device> m_hantro;
	required_device_array<pl080_device, 2> m_dma;
	required_device<igs38_i2s_device> m_i2s;
	memory_view m_boot_view;
	u32 m_scu[0x78 / 4];
	u8 m_boot_latched;
	bool m_scu_reset_pending;
	u8 m_gpio[7][0x78 / 4];
	u8 m_gpio_previous[7][2];
	u8 m_gpio_pending[7][2];
	u32 m_lcd[0x100 / 4];
	bool m_lcd_irq;
};


u32 pgm3_state::scu_r(offs_t offset)
{
	if (offset == 0x58 / 4)
	{
		u32 status = m_scu[offset];
		// Functional PLL model: locked unless held in reset. Lock delay and
		// clock-frequency changes still need modelling with the clock tree.
		for (u32 pll = 0; pll != 5; ++pll)
		{
			if (!BIT(m_scu[1 + pll], 28))
				status |= 1U << (2 + pll);
		}
		if (!BIT(m_scu[0x64 / 4], 28))
			status |= 0x80;
		return status;
	}
	return m_scu[offset];
}

void pgm3_state::scu_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 0x28 / 4)
	{
		// Word-wide commands, not an accumulating/readable configuration latch.
		// Reversible mapping model. Later production masks may disable
		// the BEEFBEEF command; that restriction is not modelled.
		if (mem_mask == 0xffffffff)
		{
			switch (data)
			{
			case 0xbeefbeef:
				m_boot_view.select((m_scu[0x58 / 4] & 1) ? 0 : 3);
				break;
			case 0xbeefdead:
				m_boot_view.select(1);
				break;
			case 0xdeadbeef:
				m_boot_view.select(2);
				break;
			}
		}
		return;
	}
	if (offset == 0x30 / 4)
	{
		if (mem_mask == 0xffffffff && data == 0x00ff00ff)
		{
			m_scu_reset_pending = true;
			machine().schedule_soft_reset();
		}
		return;
	}
	mem_mask &= SCU_REGISTERS[offset].writable;
	COMBINE_DATA(&m_scu[offset]);
	if (offset == 0x04 / 4 || offset == 0x20 / 4 || offset == 0x2c / 4)
		peripheral_clocks();
}

void pgm3_state::peripheral_clocks()
{
	// The PLL uses a 32 MHz reference. The PGM3 kernel derives
	// APB = MPLL/16 (56 MHz for 0101016f). CPU clock remains separate.
	u64 bus = 32'000'000;
	if (BIT(m_scu[0x2c / 4], 0))
	{
		u32 const pll = m_scu[0x04 / 4];
		bus = BIT(pll, 28) ? 0 : bus * ((pll & 255) + 1) / ((((pll >> 16) & 63) + 1) << ((pll >> 8) & 7));
	}
	u32 const pclk = u32(bus / 16);
	for (auto &timer : m_timers)
		timer->set_unscaled_clock(BIT(m_scu[0x20 / 4], 0) ? 0 : pclk);
	for (u32 i = 0; i < 3; ++i)
		m_i2c[i]->set_unscaled_clock(BIT(m_scu[0x20 / 4], 11 + i) ? 0 : pclk);
}

u32 pgm3_state::scu_ext_r(offs_t offset)
{
	// Firmware-derived subset of the SCU extension registers. The mask ROM
	// reads key slot [3:0] at +0c (PC 0xf04), and forced ROM/decode mode at
	// +10 bit 30 (PC 0xf5c).
	if (offset == 0x0c / 4)
		return m_boot_latched >> 4;
	if (offset == 0x10 / 4)
		return BIT(m_boot_latched, 1) ? 0x40000000 : 0;
	return 0;
}

u8 pgm3_state::gpio_pins(u32 bank, u32 port)
{
	const u8 *const regs = m_gpio[bank];
	const u8 direction = regs[port * 2 + 1];
	u8 input = (bank == 1 && port == 1) ? 0x08 : 0;
	const u8 test = regs[0x20 / 4];
	if ((test == 1 && port == 0) || (test == 3 && port == 1))
	{
		const u32 source = port ^ 1;
		const u8 source_dir = regs[source * 2 + 1];
		const u8 source_input = (bank == 1 && source == 1) ? 0x08 : 0;
		input = (regs[source * 2] & source_dir) | (source_input & ~source_dir);
	}
	return (regs[port * 2] & direction) | (input & ~direction);
}

void pgm3_state::gpio_update(u32 bank)
{
	for (u32 port = 0; port != 2; ++port)
	{
		const u8 pins = gpio_pins(bank, port);
		const u8 changed = pins ^ m_gpio_previous[bank][port];
		const u8 *const regs = m_gpio[bank];
		const u8 edge = ~regs[0x34 / 4 + port];
		const u8 trigger = regs[0x44 / 4 + port] | ~(pins ^ regs[0x54 / 4 + port]);
		// IEx disables individual triggers as well as the combined interrupt.
		m_gpio_pending[bank][port] |= changed & edge & trigger & regs[0x24 / 4 + port];
		m_gpio_previous[bank][port] = pins;
	}
	// TODO: connect the combined interrupt to VIC0/VIC1 when implemented.
}

u16 pgm3_state::gpio_status(u32 bank)
{
	u16 status = 0;
	for (u32 port = 0; port != 2; ++port)
	{
		const u8 *const regs = m_gpio[bank];
		const u8 sense = regs[0x34 / 4 + port];
		const u8 level = ~(gpio_pins(bank, port) ^ regs[0x54 / 4 + port]);
		const u8 active = ((m_gpio_pending[bank][port] & ~sense) | (level & sense)) & regs[0x24 / 4 + port];
		status |= u16(active) << (8 * port);
	}
	return status;
}

INPUT_CHANGED_MEMBER(pgm3_state::hdmi_connected_changed)
{
	m_hdmi->set_connected(newval != 0);
}

template <u32 Bank>
void pgm3_state::gpio_map(address_map &map)
{
	for (u32 port = 0; port < 2; ++port)
	{
		map(port * 8, port * 8).umask32(0x000000ff).lrw8(
				[this, port] () -> u8 { return gpio_pins(Bank, port); }, "gpio_data_r",
				[this, port] (u8 data)
				{
					u8 const mask = m_gpio[Bank][port * 2 + 1];
					m_gpio[Bank][port * 2] = (m_gpio[Bank][port * 2] & ~mask) | (data & mask);
					gpio_update(Bank);
				}, "gpio_data_w");
		map(0x64 + port * 4, 0x64 + port * 4).umask32(0x000000ff).lrw8(
				[] () -> u8 { return 0; }, "gpio_clear_r",
				[this, port] (u8 data) { m_gpio_pending[Bank][port] &= ~data; }, "gpio_clear_w");
	}

	static constexpr u8 REGISTERS[] = { 0x04, 0x0c, 0x20, 0x24, 0x28, 0x34, 0x38, 0x44, 0x48, 0x54, 0x58 };
	for (u8 const reg : REGISTERS)
	{
		map(reg, reg).umask32(0x000000ff).lrw8(
				[this, reg] () -> u8 { return m_gpio[Bank][reg / 4]; }, "gpio_register_r",
				[this, reg] (u8 data)
				{
					m_gpio[Bank][reg / 4] = data & ((reg == 0x20) ? 7 : 0xff);
					gpio_update(Bank);
				}, "gpio_register_w");
	}
	map(0x74, 0x75).umask32(0x0000ffff).lr16([this] () -> u16 { return gpio_status(Bank); }, "gpio_status_r");
}

u8 pgm3_state::spacc_dma_permissions(offs_t address)
{
	if (address < 0x1000'0000)
		return (m_boot_view.entry() == 1 || m_boot_view.entry() == 2) ? 3 : 1;
	if (address >= 0x1000'0000 && address <= 0x1007'ffff)
		return 1;
	if ((address >= 0x2800'0000 && address <= 0x2801'ffff) || address >= 0x8000'0000)
		return 3;
	return 0;
}

void pgm3_state::pgm3_map(address_map &map)
{
	// Low 256 MiB is a mirrored boot-source / DDR2 / SRAM window.
	// View 3 is the NOR-boot variant of Remap 0, not a fourth hardware mode.
	map(0x00000000, 0x0fffffff).view(m_boot_view);
	m_boot_view[0](0x00000000, 0x00007fff).mirror(0x0fff8000).rom().region("internal_mask", 0);
	m_boot_view[1](0x00000000, 0x0fffffff).ram().share("ddr");
	m_boot_view[2](0x00000000, 0x0001ffff).mirror(0x0ffe0000).ram().share("sram");
	m_boot_view[3](0x00000000, 0x0007ffff).mirror(0x0ff80000).rom().region("internal_flash", 0);

	map(0x10000000, 0x1007ffff).rom().region("internal_flash", 0);
	// Parallel 32 KiB FRAM on external chip select 1. Linux's /dev/iomem
	// maps this range; the game stores bookkeeping and its board-bound region
	// record here. A blank replacement is not a factory-programmed board dump.
	map(0x14000000, 0x14007fff).ram().share("fram");
	map(0x28000000, 0x2801ffff).ram().share("sram");
	map(0x60060000, 0x60060fff).m(m_dma[0], FUNC(pl080_device::map));
	map(0x600a0000, 0x600a0fff).m(m_dma[1], FUNC(pl080_device::map));
	map(0x600e0000, 0x600fffff).m(m_sdhc[0], FUNC(igs38_sdhc_device::map));
	map(0x60100000, 0x6011ffff).m(m_sdhc[1], FUNC(igs38_sdhc_device::map));
	map(0x60120000, 0x6013ffff).m(m_sdhc[2], FUNC(igs38_sdhc_device::map));
	map(0x60220000, 0x60220fff).m(m_vic[0], FUNC(vic_pl192_device::map));
	map(0x60240000, 0x60240fff).m(m_vic[1], FUNC(vic_pl192_device::map));
	map(0x60340000, 0x60347fff).m(m_spacc, FUNC(igs38_spacc_device::map));
	map(0x61000000, 0x6100000f).m(m_timers[0], FUNC(igs38_timer_device::map));
	map(0x61000010, 0x6100001f).m(m_timers[1], FUNC(igs38_timer_device::map));
	map(0x61000020, 0x6100002f).m(m_timers[2], FUNC(igs38_timer_device::map));
	map(0x61003000, 0x6100302b).m(m_i2c[0], FUNC(igs38_i2c_device::map));
	map(0x61004000, 0x6100402b).m(m_i2c[1], FUNC(igs38_i2c_device::map));
	map(0x61005000, 0x6100502b).m(m_i2c[2], FUNC(igs38_i2c_device::map));
	map(0x61006000, 0x61006047).m(m_spi[0], FUNC(igs38_spi_device::map));
	map(0x61007000, 0x61007047).m(m_spi[1], FUNC(igs38_spi_device::map));
	map(0x6100b000, 0x6100b013).r(FUNC(pgm3_state::scu_ext_r));
	map(0x6100d000, 0x6100d01f).m(m_i2s, FUNC(igs38_i2s_device::map));
	map(0x6100f000, 0x6100f077).rw(FUNC(pgm3_state::scu_r), FUNC(pgm3_state::scu_w));
	map(0x61011000, 0x61011077).m(FUNC(pgm3_state::gpio_map<0>));
	map(0x61012000, 0x61012077).m(FUNC(pgm3_state::gpio_map<1>));
	map(0x61013000, 0x61013077).m(FUNC(pgm3_state::gpio_map<2>));
	map(0x61014000, 0x61014077).m(FUNC(pgm3_state::gpio_map<3>));
	map(0x61015000, 0x61015077).m(FUNC(pgm3_state::gpio_map<4>));
	map(0x61016000, 0x61016077).m(FUNC(pgm3_state::gpio_map<5>));
	map(0x61017000, 0x610170ff).m(FUNC(pgm3_state::lcd_map));
	map(0x61018000, 0x61018077).m(FUNC(pgm3_state::gpio_map<6>));
	map(0x61020000, 0x61023fff).m(m_mali, FUNC(mali200_device::map));
	map(0x61040000, 0x610403ff).m(m_hantro, FUNC(hantro_g1_device::map));
	// The loader uses 0x90000000..0xa13fffff for card authentication scratch.
	// The low remap exposes the first 256 MiB.
	// DDR controller initialization/training is not yet modelled.
	map(0x80000000, 0x8fffffff).ram().share("ddr");
	// The Mali build settings specify 2048 MiB total memory. The shipped
	// Mali driver reserves 0xa4000000..0xffffffff and allocates its initial
	// MMU tables near 0xfffc0000, outside the Linux-managed low memory.
	map(0x90000000, 0xffffffff).ram();
}

static INPUT_PORTS_START( pgm3 )
	// Input packet layout recovered from the IGS SPI client. Physical
	// DIP labels and the MCU firmware revision still need hardware verification.
	PORT_START("IODSW")
	PORT_DIPNAME(0x10, 0x00, "Display resolution")
	PORT_DIPSETTING(0x00, "1280 x 720")
	PORT_DIPSETTING(0x10, "800 x 600")
	PORT_DIPNAME(0x02, 0x00, "I/O connection")
	PORT_DIPSETTING(0x00, "JAMMA")
	PORT_DIPSETTING(0x02, "JVS")
	PORT_BIT(0xed, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("IOBUTTONS")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_PLAYER(1)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_PLAYER(1)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_PLAYER(1)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_PLAYER(1)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_START1)
	PORT_SERVICE_NO_TOGGLE(0x00000800, IP_ACTIVE_HIGH)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_SERVICE1)
	PORT_BIT(0x0000e000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(2)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_PLAYER(2)
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_PLAYER(2)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_PLAYER(2)
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_START2)
	PORT_BIT(0xf8000000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IOCOINS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_CHANGED_MEMBER("io", FUNC(pgm3_io_device::coin_changed), 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_CHANGED_MEMBER("io", FUNC(pgm3_io_device::coin_changed), 1)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("HDMI")
	PORT_CONFNAME(0x01, 0x01, "HDMI Monitor") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pgm3_state::hdmi_connected_changed), 0)
	PORT_CONFSETTING(0x00, "Disconnected")
	PORT_CONFSETTING(0x01, "Connected")
INPUT_PORTS_END

// LCDC scanout subset: page 0, little-endian RGB565 or XRGB8888 in DDR.
// Sample the framebuffer once per frame; scanline register latching is not modelled.
// YUV and page switching are not implemented. Frame completion drives the
// page-read interrupt. FIFO fill and pixel-clock/porch timing are approximate.
u32 pgm3_state::lcd_status_r()
{
	u32 const value = m_lcd[0x40 / 4];
	// The Linux IRQ handler reads INTSTAS and then disables the
	// source; it never writes an acknowledge register. Status is read-to-clear.
	if (!machine().side_effects_disabled())
	{
		m_lcd[0x40 / 4] = 0;
		lcd_update_irq();
	}
	return value;
}

void pgm3_state::lcd_update_irq()
{
	bool const state = bool(m_lcd[0x40 / 4] & ~m_lcd[0x4c / 4] & 0x3d);
	if (m_lcd_irq != state)
	{
		m_lcd_irq = state;
		m_vic[1]->irq_w<1>(state);
	}
}

void pgm3_state::lcd_vblank(int state)
{
	if (!state || !BIT(m_lcd[0], 0))
		return;
	m_lcd[0x40 / 4] |= BIT(m_lcd[0], 1) ? 0x10 : 1;
	lcd_update_irq();
}

void pgm3_state::lcd_map(address_map &map)
{
	map(0x00, 0xff).lr32([this] (offs_t offset) -> u32 { return m_lcd[offset]; }, "lcd_register_r").nopw();
	map(0x40, 0x43).r(FUNC(pgm3_state::lcd_status_r));
	map(0x44, 0x47).lw32(
			[this] (offs_t offset, u32 data, u32 mem_mask)
			{
				m_lcd[0x4c / 4] &= ~(data & mem_mask & 0x3f);
				lcd_update_irq();
			}, "lcd_irq_enable_w");
	map(0x48, 0x4b).lw32(
			[this] (offs_t offset, u32 data, u32 mem_mask)
			{
				m_lcd[0x4c / 4] |= data & mem_mask & 0x3f;
				lcd_update_irq();
			}, "lcd_irq_disable_w");

	struct lcd_register
	{
		u8 address;
		u32 writable;
	};
	static constexpr lcd_register REGISTERS[] =
	{
		{ 0x00, 0x007fff9f },
		{ 0x08, 0x00000007 },
		{ 0x0c, 0x1fff1fff },
		{ 0x10, 0xffffffff },
		{ 0x14, 0xffffffff },
		{ 0x1c, 0xfffffff0 },
		{ 0x20, 0xfffffff0 },
		{ 0x24, 0xfffffff0 },
		{ 0x30, 0x00000001 },
		{ 0x4c, 0x0000003f },
		{ 0x50, 0xfffffff0 },
		{ 0x54, 0xfffffff0 },
		{ 0x58, 0xfffffff0 },
		{ 0x60, 0xfffffff0 },
		{ 0x64, 0xfffffff0 },
		{ 0x68, 0xfffffff0 },
		{ 0x70, 0x1fff1fff },
		{ 0x74, 0x1fff1fff },
		{ 0x78, 0x00ffffff }
	};
	for (auto const &reg : REGISTERS)
	{
		map(reg.address, reg.address + 3).lw32(
				[this, reg] (offs_t offset, u32 data, u32 mem_mask) { lcd_register_w(reg.address / 4, data, mem_mask & reg.writable); }, "lcd_register_w");
	}
}

void pgm3_state::lcd_register_w(u32 reg, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_lcd[reg]);
	if (reg == 0 && !BIT(m_lcd[0], 0))
		m_lcd[0x40 / 4] = 0;
	lcd_update_irq();
	if (reg == 0 || reg == 0x0c / 4)
	{
		lcd_configure_screen();
		m_hdmi->set_video_signal(BIT(m_lcd[0], 0) && (m_lcd[0x0c / 4] & 0x1fff) && BIT(m_lcd[0x0c / 4], 16, 13));
	}
}

void pgm3_state::lcd_configure_screen()
{
	u32 const width = m_lcd[0x0c / 4] & 0x1fff;
	u32 const height = (m_lcd[0x0c / 4] >> 16) & 0x1fff;
	u32 const alignment = BIT(m_lcd[0], 7) ? 8 : 16;
	if (width && width <= 1920 && !(width & (alignment - 1)) && height && height <= 1080)
		m_screen->configure(width, height, rectangle(0, width - 1, 0, height - 1), attotime::from_hz(60));
}

u32 pgm3_state::screen_update_pgm3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);
	// 24-bit output fetches one 0x00RRGGBB word per pixel;
	// it is Linux's 32-bpp framebuffer format, not packed three-byte RGB.
	u32 const ctrl = m_lcd[0];
	bool const rgb888 = BIT(ctrl, 7);
	u32 const width = m_lcd[0x0c / 4] & 0x1fff;
	u32 const height = (m_lcd[0x0c / 4] >> 16) & 0x1fff;
	if ((ctrl & 3) != 1 || (!rgb888 && (ctrl & 0x00300000) != 0x00200000) || BIT(m_lcd[0x08 / 4], 0) || m_lcd[0x30 / 4] ||
			m_lcd[0x70 / 4] || m_lcd[0x74 / 4] || !width || width > 1920 || (width & (rgb888 ? 7 : 15)) || !height || height > 1080)
		return 0;

	// Only fetch from a single contiguous physical DDR allocation. This also
	// prevents an invalid DMA address from causing MMIO reads during scanout.
	u32 const base = m_lcd[0x20 / 4];
	u64 const end = u64(base) + u64(width) * height * (rgb888 ? 4 : 2);
	if (base < 0x8000'0000 || end > ((base < 0x9000'0000) ? 0x9000'0000ULL : 0x1'0000'0000ULL))
		return 0;
	u32 const *const source = reinterpret_cast<u32 const *>(m_maincpu->space(AS_PROGRAM).get_read_ptr(base));
	if (!source)
		return 0;

	rectangle area(0, width - 1, 0, height - 1);
	area &= cliprect;
	for (int y = area.top(); y <= area.bottom(); ++y)
	{
		u32 *const row = &bitmap.pix(y);
		for (int x = area.left(); x <= area.right(); ++x)
		{
			u32 const pixel = y * width + x;
			if (rgb888)
			{
				row[x] = rgb_t(0xff000000 | (source[pixel] & 0xffffff));
				continue;
			}
			u16 const color = source[pixel / 2] >> ((pixel & 1) * 16);
			row[x] = rgb_t(pal5bit(color >> 11), pal6bit(color >> 5), pal5bit(color));
		}
	}
	return 0;
}

void pgm3_state::machine_start()
{
	m_scu_reset_pending = false;
	m_lcd_irq = false;

	save_item(NAME(m_scu));
	save_item(NAME(m_boot_latched));
	save_item(NAME(m_scu_reset_pending));
	save_item(NAME(m_gpio));
	save_item(NAME(m_gpio_previous));
	save_item(NAME(m_gpio_pending));
	save_item(NAME(m_lcd));
	save_item(NAME(m_lcd_irq));
}

void pgm3_state::machine_reset()
{
	if (m_scu_reset_pending)
	{
		// SCU-generated reset excludes SCU and PMU, so retain
		// the configuration, boot source and current memory view.
		m_scu[0x58 / 4] |= 0x00040000;
	}
	else
	{
		// Treat the emulator reset as an external POR, resampling boot straps.
		// Mask-ROM boot, key slot 1; eFuse 62 is clear on KOV3HD boards.
		m_boot_latched = 0x11;
		for (u32 reg = 0; reg != std::size(m_scu); ++reg)
			m_scu[reg] = SCU_REGISTERS[reg].reset;
		m_scu[0x58 / 4] = (m_boot_latched & 3) ? 1 : 0;
		m_boot_view.select((m_boot_latched & 3) ? 0 : 3);
	}
	m_scu_reset_pending = false;
	peripheral_clocks();
	std::fill(std::begin(m_lcd), std::end(m_lcd), 0);
	m_lcd[0x00 / 4] = 0x00000800;
	m_lcd[0x4c / 4] = 0x0000003f;
	m_lcd[0xfc / 4] = 0x00000b02;
	lcd_update_irq();
	m_hdmi->set_video_signal(0);
	for (u32 bank = 0; bank != 7; ++bank)
	{
		std::fill(std::begin(m_gpio[bank]), std::end(m_gpio[bank]), 0);
		for (u32 port = 0; port != 2; ++port)
		{
			m_gpio_pending[bank][port] = 0;
			m_gpio_previous[bank][port] = gpio_pins(bank, port);
		}
	}
}

void pgm3_state::pgm3(machine_config &config)
{
	/* basic machine hardware */
	ARM1176JZF_S(config, m_maincpu, 800'000'000); // SOC38 / IGS038 - ARM1176JZ based SoC
	m_maincpu->set_addrmap(AS_PROGRAM, &pgm3_state::pgm3_map);

	NVRAM(config, "fram", nvram_device::DEFAULT_ALL_0);

	IGS38_SPACC(config, m_spacc);
	m_spacc->set_dma_space(m_maincpu, AS_PROGRAM);
	m_spacc->dma_permissions_callback().set(FUNC(pgm3_state::spacc_dma_permissions));

	for (u32 i = 0; i < 2; ++i)
		PL192_VIC(config, m_vic[i]);
	m_vic[0]->out_irq_cb().set_inputline(m_maincpu, arm7_cpu_device::ARM7_IRQ_LINE);
	m_vic[0]->out_fiq_cb().set_inputline(m_maincpu, arm7_cpu_device::ARM7_FIRQ_LINE);
	m_vic[1]->out_irq_cb().set(m_vic[0], FUNC(vic_pl192_device::daisy_irq_w));
	m_vic[1]->out_fiq_cb().set(m_vic[0], FUNC(vic_pl192_device::daisy_fiq_w));
	m_vic[0]->daisy_vector_callback().set(m_vic[1], FUNC(vic_pl192_device::vector_r));

	for (u32 i = 0; i < 3; ++i)
	{
		IGS38_SDHC(config, m_sdhc[i]);
		m_sdhc[i]->set_dma_space(m_maincpu, AS_PROGRAM);
		m_sdhc[i]->dma_permissions_callback().set(FUNC(pgm3_state::spacc_dma_permissions));
	}

	for (u32 i = 0; i < 3; ++i)
		IGS38_TIMER(config, m_timers[i], 0);
	m_timers[0]->irq_callback().set(m_vic[0], FUNC(vic_pl192_device::irq_w<4>));
	m_timers[1]->irq_callback().set(m_vic[0], FUNC(vic_pl192_device::irq_w<5>));
	m_timers[2]->irq_callback().set(m_vic[0], FUNC(vic_pl192_device::irq_w<6>));

	for (u32 i = 0; i < 3; ++i)
		IGS38_I2C(config, m_i2c[i], 0);
	m_i2c[0]->irq_callback().set(m_vic[0], FUNC(vic_pl192_device::irq_w<16>));
	m_i2c[1]->irq_callback().set(m_vic[0], FUNC(vic_pl192_device::irq_w<17>));
	m_i2c[2]->irq_callback().set(m_vic[0], FUNC(vic_pl192_device::irq_w<18>));

	for (u32 i = 0; i < 2; ++i)
		IGS38_SPI(config, m_spi[i]);
	m_spi[0]->irq_callback().set(m_vic[0], FUNC(vic_pl192_device::irq_w<13>));
	m_spi[1]->irq_callback().set(m_vic[0], FUNC(vic_pl192_device::irq_w<14>));

	PGM3_IO(config, m_io);
	m_io->dips_callback().set_ioport("IODSW");
	m_io->buttons_callback().set_ioport("IOBUTTONS");
	m_io->set_key_region("iokey");
	m_spi[0]->select_callback<0>().set(m_io, FUNC(pgm3_io_device::select_w));
	m_spi[0]->transmit_callback().set(m_io, FUNC(pgm3_io_device::data_w));
	m_spi[0]->receive_callback().set(m_io, FUNC(pgm3_io_device::data_r));

	CAT6613(config, m_hdmi);
	m_hdmi->set_edid(MONITOR_EDID);
	m_hdmi->hpd_callback().set_ioport("HDMI");
	// Linux's CAT6613 client uses IRQ45: VIC1 input 13, external interrupt 0.
	m_hdmi->irq_callback().set(m_vic[1], FUNC(vic_pl192_device::irq_w<13>));
	m_i2c[0]->transmit_callback().set(m_hdmi, FUNC(cat6613_device::transmit));
	m_i2c[0]->receive_callback().set(m_hdmi, FUNC(cat6613_device::receive));
	m_i2c[0]->stop_callback().set(m_hdmi, FUNC(cat6613_device::stop));
	m_i2c[0]->acknowledge_callback().set(m_hdmi, FUNC(cat6613_device::acknowledge));

	S35390A(config, m_rtc, 32.768_kHz_XTAL);
	m_i2c[1]->transmit_callback().set(m_rtc, FUNC(s35390a_device::transmit));
	m_i2c[1]->receive_callback().set(m_rtc, FUNC(s35390a_device::receive));
	m_i2c[1]->stop_callback().set(m_rtc, FUNC(s35390a_device::stop));
	m_i2c[1]->acknowledge_callback().set(m_rtc, FUNC(s35390a_device::acknowledge));
	m_sdhc[0]->irq_callback().set(m_vic[1], FUNC(vic_pl192_device::irq_w<2>));
	m_sdhc[1]->irq_callback().set(m_vic[1], FUNC(vic_pl192_device::irq_w<3>));
	m_sdhc[2]->irq_callback().set(m_vic[1], FUNC(vic_pl192_device::irq_w<4>));

	MALI200(config, m_mali, 200'000'000);
	m_mali->set_dma_space(m_maincpu, AS_PROGRAM);
	// All physical access boundaries and remap views are 4 KiB aligned.
	m_mali->dma_page_permissions_callback().set(FUNC(pgm3_state::spacc_dma_permissions));
	m_mali->mmu_irq_callback().set(m_vic[1], FUNC(vic_pl192_device::irq_w<8>));
	m_mali->gp_irq_callback().set(m_vic[1], FUNC(vic_pl192_device::irq_w<9>));
	m_mali->pp_irq_callback().set(m_vic[1], FUNC(vic_pl192_device::irq_w<10>));

	HANTRO_G1(config, m_hantro, 200'000'000); // functional timing; clock unverified
	m_hantro->set_dma_space(m_maincpu, AS_PROGRAM);
	m_hantro->irq_callback().set(m_vic[0], FUNC(vic_pl192_device::irq_w<26>));

	for (u32 i = 0; i < 2; ++i)
	{
		PL080(config, m_dma[i], 100'000'000); // AHB arbitration timing is approximate.
		m_dma[i]->set_dma_space(m_maincpu, AS_PROGRAM);
	}
	m_dma[0]->irq_callback().set(m_vic[1], FUNC(vic_pl192_device::irq_w<5>));
	m_dma[1]->irq_callback().set(m_vic[1], FUNC(vic_pl192_device::irq_w<6>));

	IGS38_I2S(config, m_i2s, 11'289'600); // MCLK supplied by the board's Linux platform data.
	m_i2s->irq_callback().set(m_vic[0], FUNC(vic_pl192_device::irq_w<19>));
	m_i2s->dma_callback<0>().set(m_dma[0], FUNC(pl080_device::request_w<0>));
	m_i2s->dma_callback<1>().set(m_dma[0], FUNC(pl080_device::request_w<1>));

	SPEAKER(config, "speaker", 2).front();
	m_i2s->add_route(0, "speaker", 1.0, 0);
	m_i2s->add_route(1, "speaker", 1.0, 1);

	screen_device &screen(SCREEN(config, m_screen));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(1280, 720);
	screen.set_visarea(0, 1280 - 1, 0, 720 - 1);
	screen.set_physical_aspect(16, 9);
	screen.set_screen_update(FUNC(pgm3_state::screen_update_pgm3));
	screen.screen_vblank().set(FUNC(pgm3_state::lcd_vblank));
}

#define KOV3HD_SOC_ROMS \
	/* The mask part of the SOC38, this gets hidden by the system after use */ \
	/* Same ROM is likely used by other games as it contains a number of unused keys, with the key to use being selected by a HW register */ \
	/* Reconstructed from decapped production-sample silicon. */ \
	/* kov3hd uses the AES keys at 0x42b8 and 0x44a8 in this ROM */ \
	/* the only purpose of this bootloader is to decrypt the flash part of the internal ROM below, copying it into RAM */ \
	ROM_REGION32_LE( 0x8000, "internal_mask", ROMREGION_ERASE00 ) \
	ROM_LOAD( "internal_boot.bin", 0x0000, 0x8000, CRC(f6877f92) SHA1(4431d73cc7e5bbb11cd53449284fff1435a6ea32) ) \
	/* internal flash is for KOV3HD, and gets decrypted using hardware AES decryption and a key from internal_mask */ \
	ROM_REGION32_LE( 0x80000, "internal_flash", ROMREGION_ERASE00 ) \
	ROM_LOAD( "internal_flash.bin", 0x0000, 0x80000, CRC(5925187d) SHA1(3acc29891142d47a7bf3c73016a06bc436977b40) ) \
	/* Initial FRAM from the same board as the flash: region China, MAC 00:12:d8:00:01:09; factory reset in the service menu */ \
	ROM_REGION( 0x8000, "fram", 0 ) \
	ROM_LOAD( "fram_clear.bin", 0x0000, 0x8000, CRC(df72d4e1) SHA1(dc47175176732b02c8f1918c8c64395dd4f7905e) ) \
	ROM_REGION( 0x10, "iokey", 0 ) \
	ROM_LOAD( "iomcu.key", 0x00, 0x10, CRC(736d4d54) SHA1(d8fe8e3219bce63843a716b2ebe7a0b35490e59c) ) \


ROM_START( kov3hd )
	KOV3HD_SOC_ROMS

	DISK_REGION( "sdhc0" )
	DISK_IMAGE( "kov3hd_m105", 0, SHA1(81af30aa6e1a34b2a8fab8c5c23a313a7164767c) )
	// The earlier kov3hd_v105 image (SHA1 c185888c59880805bb76b5c0a42b05c614dcff37) fails on hardware.
ROM_END

ROM_START( kov3hd104 )
	KOV3HD_SOC_ROMS

	DISK_REGION( "sdhc0" )
	DISK_IMAGE( "kov3hd_m104", 0, SHA1(899b3b81825e6f23ae8f39aa67ad5b019f387cf9) )
ROM_END

ROM_START( kov3hd103 )
	KOV3HD_SOC_ROMS

	DISK_REGION( "sdhc0" )
	DISK_IMAGE( "kov3hd_m103", 0, SHA1(0d4fd981f477cd5ed62609b875f4ddec939a2bb0) )
ROM_END

ROM_START( kov3hd102 )
	KOV3HD_SOC_ROMS

	DISK_REGION( "sdhc0" )
	DISK_IMAGE( "kov3hd_m102", 0, SHA1(a5a872f9add5527b94019ec77ff1cd0f167f040f) )
ROM_END

ROM_START( kov3hd101 )
	KOV3HD_SOC_ROMS

	DISK_REGION( "sdhc0" )
	DISK_IMAGE( "kov3hd_m101", 0, SHA1(086d6f1b8b2c01a8670fd6480da44b9c507f6e08) )
ROM_END

} // anonymous namespace


// The board's FRAM region record selects China and is paired to its flash MAC.
GAME( 2013, kov3hd,     0,      pgm3,    pgm3, pgm3_state, empty_init, ROT0, "IGS", "Knights of Valour 3 HD (M-105CN 13-07-04 18:54:01)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2013, kov3hd104,  kov3hd, pgm3,    pgm3, pgm3_state, empty_init, ROT0, "IGS", "Knights of Valour 3 HD (M-104CN 13-05-16 14:48:16)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2013, kov3hd103,  kov3hd, pgm3,    pgm3, pgm3_state, empty_init, ROT0, "IGS", "Knights of Valour 3 HD (M-103CN 13-03-06 10:27:05)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2013, kov3hd102,  kov3hd, pgm3,    pgm3, pgm3_state, empty_init, ROT0, "IGS", "Knights of Valour 3 HD (M-102CN 13-01-11 10:35:28)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2012, kov3hd101,  kov3hd, pgm3,    pgm3, pgm3_state, empty_init, ROT0, "IGS", "Knights of Valour 3 HD (M-101CN 12-12-14 16:01:38)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
