// license:BSD-3-Clause
// copyright-holders:Felipe Corrêa da Silva Sanches
/*******************************************************************************

    PMD 32-SD disk unit

    RM-TEAM's replacement for the Tesla PMD 32 floppy unit: an ATmega32 that
    keeps disk images on an SD card and answers the PMD 32 protocol over the
    same bidirectional 8255 link (see pmd32.cpp), plus commands that pick the
    images.  The Consul 2717 603.513/32 disk driver was written for it.

    This is a high-level emulation of the protocol: each of the four drives
    A: to D: holds an image selected in MAME rather than through the unit's
    LCD and buttons.  Image types follow the unit's images.cfg:

        P32, DCZ   80 tracks x 36 sectors, 512-byte physical sectors
        DZ8       154 tracks x 26 sectors, 128-byte physical sectors

    Every command is a letter, its parameters and an XOR checksum; the unit
    answers 0x33 (ACK) or 0x99 (NAK), then an error code and any data with
    their own XOR checksum.  Between commands it offers the presentation
    byte 0xaa until the host answers with the same.

    TODO:
    - The SD card directory commands (H with a name, K, L, M, N) report an
      empty card.
    - S (write physical sector) and the unit memory commands U, C and J
      are not supported.

*******************************************************************************/

#include "emu.h"
#include "pmd32sd.h"

#include "softlist_dev.h"

#include "multibyte.h"

#include <algorithm>
#include <iterator>
#include <string_view>


namespace {

constexpr uint8_t PRESENTATION = 0xaa;
constexpr uint8_t ACK = 0x33;
constexpr uint8_t NAK = 0x99;

constexpr uint8_t ERR_OK = 0x00;
constexpr uint8_t ERR_WRITE_PROTECT = 0x01;
constexpr uint8_t ERR_READ = 0x03;
constexpr uint8_t ERR_WRITE = 0x04;
constexpr uint8_t ERR_BAD_DRIVE = 0x05;
constexpr uint8_t ERR_NOT_FOUND = 0x06;
constexpr uint8_t ERR_CREATE = 0x08;
constexpr uint8_t ERR_OTHER = 0x99;


//**************************************************************************
//  disk images
//**************************************************************************

class pmd32sd_image_device : public device_t, public device_image_interface
{
public:
	pmd32sd_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// geometry of the inserted image; sectors() is 0 when the drive is empty
	unsigned tracks() const { return m_tracks; }
	unsigned sectors() const { return m_sectors; }
	uint8_t sector_size() const { return m_sector_size; }

	bool read_sector(unsigned track, unsigned sector, uint8_t *buffer);
	bool write_sector(unsigned track, unsigned sector, uint8_t const *buffer);

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return "pmd32sd_disk"; }
	virtual const char *file_extensions() const noexcept override { return "p32,dcz,dz8"; }
	virtual const char *image_type_name() const noexcept override { return "floppydisk"; }
	virtual const char *image_brief_type_name() const noexcept override { return "flop"; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override;

private:
	uint16_t m_tracks;
	uint8_t m_sectors;
	uint8_t m_sector_size;
};

} // anonymous namespace

DEFINE_DEVICE_TYPE(PMD32SD_IMAGE, pmd32sd_image_device, "pmd32sd_image", "PMD 32-SD disk image")

namespace {

pmd32sd_image_device::pmd32sd_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PMD32SD_IMAGE, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_tracks(0)
	, m_sectors(0)
	, m_sector_size(0)
{
}

void pmd32sd_image_device::device_start()
{
}

const software_list_loader &pmd32sd_image_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}

std::pair<std::error_condition, std::string> pmd32sd_image_device::call_load()
{
	switch (length())
	{
	case 80 * 36 * 128:
		m_tracks = 80;
		m_sectors = 36;
		m_sector_size = 2;
		break;

	case 154 * 26 * 128:
		m_tracks = 154;
		m_sectors = 26;
		m_sector_size = 0;
		break;

	default:
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported image size (must be 368,640 or 512,512 bytes)");
	}

	return std::make_pair(std::error_condition(), std::string());
}

void pmd32sd_image_device::call_unload()
{
	m_tracks = 0;
	m_sectors = 0;
	m_sector_size = 0;
}

bool pmd32sd_image_device::read_sector(unsigned track, unsigned sector, uint8_t *buffer)
{
	if ((track >= m_tracks) || (sector >= m_sectors))
		return false;
	if (fseek((track * m_sectors + sector) * 128, SEEK_SET))
		return false;
	return fread(buffer, 128) == 128;
}

bool pmd32sd_image_device::write_sector(unsigned track, unsigned sector, uint8_t const *buffer)
{
	if (is_readonly() || (track >= m_tracks) || (sector >= m_sectors))
		return false;
	if (fseek((track * m_sectors + sector) * 128, SEEK_SET))
		return false;
	return fwrite(buffer, 128) == 128;
}


//**************************************************************************
//  unit
//**************************************************************************

class pmd32sd_device : public device_t, public device_pmd85_gpio_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::DISK; }

	pmd32sd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_pmd85_gpio_interface implementation
	virtual uint8_t pa_r() override;
	virtual void pa_w(uint8_t data) override;
	virtual void pc_w(uint8_t data) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr unsigned IN_SIZE = 520;
	static constexpr unsigned OUT_SIZE = 140;

	required_device_array<pmd32sd_image_device, 4> m_drive;

	emu_timer *m_link_timer;
	emu_timer *m_byte_timer;
	emu_timer *m_idle_timer;

	uint8_t m_host_data;
	uint8_t m_out_data;
	bool m_host_obf;
	bool m_host_ibf;
	bool m_in_link;
	bool m_connected;
	bool m_byte_pending;
	bool m_rx_pending;
	bool m_tx_consumed;
	bool m_wp[4];
	uint8_t m_in[IN_SIZE];
	uint32_t m_in_len;
	uint8_t m_out[OUT_SIZE];
	uint8_t m_out_len;
	uint8_t m_out_pos;

	TIMER_CALLBACK_MEMBER(service_link);
	TIMER_CALLBACK_MEMBER(send_next);
	TIMER_CALLBACK_MEMBER(idle_timeout);

	void receive(uint8_t data);
	unsigned command_length() const;
	void execute();
	void send(uint8_t const *data, unsigned length);
	void put_byte(uint8_t data);
};


pmd32sd_device::pmd32sd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PMD32SD, tag, owner, clock)
	, device_pmd85_gpio_interface(mconfig, *this)
	, m_drive(*this, "drive%u", 0U)
	, m_link_timer(nullptr)
	, m_byte_timer(nullptr)
	, m_idle_timer(nullptr)
	, m_host_data(0xff)
	, m_out_data(0xff)
	, m_host_obf(true)
	, m_host_ibf(false)
	, m_in_link(false)
	, m_connected(false)
	, m_byte_pending(false)
	, m_rx_pending(false)
	, m_tx_consumed(false)
	, m_wp{ false, false, false, false }
	, m_in{ }
	, m_in_len(0)
	, m_out{ }
	, m_out_len(0)
	, m_out_pos(0)
{
}

void pmd32sd_device::device_add_mconfig(machine_config &config)
{
	for (auto &drive : m_drive)
		PMD32SD_IMAGE(config, drive);
}

void pmd32sd_device::device_start()
{
	m_link_timer = timer_alloc(FUNC(pmd32sd_device::service_link), this);
	m_byte_timer = timer_alloc(FUNC(pmd32sd_device::send_next), this);
	m_idle_timer = timer_alloc(FUNC(pmd32sd_device::idle_timeout), this);

	save_item(NAME(m_host_data));
	save_item(NAME(m_out_data));
	save_item(NAME(m_host_obf));
	save_item(NAME(m_host_ibf));
	save_item(NAME(m_connected));
	save_item(NAME(m_byte_pending));
	save_item(NAME(m_rx_pending));
	save_item(NAME(m_tx_consumed));
	save_item(NAME(m_wp));
	save_item(NAME(m_in));
	save_item(NAME(m_in_len));
	save_item(NAME(m_out));
	save_item(NAME(m_out_len));
	save_item(NAME(m_out_pos));
}

void pmd32sd_device::device_reset()
{
	m_connected = false;
	m_byte_pending = false;
	m_rx_pending = false;
	m_tx_consumed = false;
	m_in_len = 0;
	m_out_len = 0;
	m_out_pos = 0;
	m_idle_timer->adjust(attotime::never);
	m_byte_timer->adjust(attotime::from_msec(1));
}


// The unit takes each byte the host writes (/OBFA falling) by pulsing /ACKA,
// and strobes its own bytes in with /STBA whenever the host's input latch is
// empty; the host reading a byte (IBFA falling) lets the next one go.  The
// unit reacts a few microseconds after each change, never during the host's
// own access.

uint8_t pmd32sd_device::pa_r()
{
	return m_out_data;
}

void pmd32sd_device::pa_w(uint8_t data)
{
	m_host_data = data;
}

void pmd32sd_device::pc_w(uint8_t data)
{
	bool const obf = BIT(data, 7);
	bool const ibf = BIT(data, 5);
	if (m_host_obf && !obf)
		m_rx_pending = true;
	if (m_host_ibf && !ibf && m_byte_pending)
		m_tx_consumed = true;
	m_host_obf = obf;
	m_host_ibf = ibf;

	if (!m_in_link)
		m_link_timer->adjust(attotime::from_usec(5));
}

TIMER_CALLBACK_MEMBER(pmd32sd_device::service_link)
{
	m_in_link = true;

	if (m_tx_consumed)
	{
		m_tx_consumed = false;
		m_byte_pending = false;
		if (m_out_pos < m_out_len)
		{
			m_byte_timer->adjust(attotime::from_usec((m_out_pos == 1) ? 500 : 20));
		}
		else
		{
			m_out_len = m_out_pos = 0;
			m_idle_timer->adjust(attotime::from_msec(25));
			if (!m_connected)
				m_byte_timer->adjust(attotime::from_msec(3));
		}
	}

	if (m_rx_pending)
	{
		m_rx_pending = false;
		uint8_t const byte = m_host_data;
		acknowledge_host();
		receive(byte);
	}

	// offer again a byte the host was not ready to take
	if (m_byte_pending && !m_host_ibf)
	{
		strobe_host();
	}

	m_in_link = false;
}

void pmd32sd_device::put_byte(uint8_t data)
{
	m_out_data = data;
	m_byte_pending = true;
	if (!m_host_ibf)
	{
		bool const in_link = m_in_link;
		m_in_link = true;
		strobe_host();
		m_in_link = in_link;
	}
}

TIMER_CALLBACK_MEMBER(pmd32sd_device::send_next)
{
	if (m_byte_pending)
		return;
	if (m_out_pos < m_out_len)
		put_byte(m_out[m_out_pos++]);
	else if (!m_connected)
		put_byte(PRESENTATION);
}

TIMER_CALLBACK_MEMBER(pmd32sd_device::idle_timeout)
{
	// no command for 25 ms: go back to offering the presentation byte
	m_connected = false;
	m_in_len = 0;
	if (!m_byte_pending && (m_out_pos >= m_out_len))
		m_byte_timer->adjust(attotime::zero);
}

void pmd32sd_device::send(uint8_t const *data, unsigned length)
{
	length = std::min<unsigned>(length, OUT_SIZE);
	std::copy_n(data, length, m_out);
	m_out_len = length;
	m_out_pos = 0;
	m_byte_timer->adjust(attotime::from_usec(20));
}


void pmd32sd_device::receive(uint8_t data)
{
	m_idle_timer->adjust(attotime::from_msec(25));

	if (!m_connected)
	{
		if (data == PRESENTATION)
		{
			m_connected = true;
			m_in_len = 0;
		}
		return;
	}

	if (m_in_len < IN_SIZE)
		m_in[m_in_len] = data;
	m_in_len++;

	if (m_in_len >= command_length())
	{
		execute();
		m_in_len = 0;
	}
}

unsigned pmd32sd_device::command_length() const
{
	switch (m_in[0])
	{
	case 'B': case 'K': case '*': case '@':
		return 2;
	case 'I': case 'G': case 'L': case 'P':
		return 3;
	case 'Q': case 'R': case 'F': case 'J':
		return 4;
	case 'C':
		return 6;
	case 'T': case 'W':
		return 4 + 128;
	case 'S':
		return 4 + 513;
	case 'H':
		if (m_in_len < 4)
			return 4;
		return ((m_in[3] == 0x00) || (m_in[3] == 0xff)) ? 5 : (5 + m_in[3]);
	case 'M': case 'N':
		if (m_in_len < 2)
			return 2;
		return 3 + m_in[1];
	case 'U':
		if (m_in_len < 5)
			return 5;
		return 6 + get_u16be(&m_in[3]);
	default:
		return 1;
	}
}

void pmd32sd_device::execute()
{
	unsigned const length = command_length();
	uint8_t crc = 0;
	for (unsigned i = 0; i < std::min<unsigned>(length, IN_SIZE); i++)
		crc ^= m_in[i];

	uint8_t reply[OUT_SIZE];
	unsigned count = 0;
	auto const result = [&reply, &count] (uint8_t err) { reply[count++] = ACK; reply[count++] = err; };
	auto const data = [&reply, &count] (uint8_t const *buf, unsigned len)
	{
		uint8_t sum = 0;
		for (unsigned i = 0; i < len; i++)
			sum ^= reply[count++] = buf[i];
		reply[count++] = sum;
	};

	// disk and sector byte: bits 7-6 select A: (00), B: (10), C: (01) or D: (11)
	unsigned const drive = bitswap<2>(m_in[1], 6, 7);
	unsigned const sector = m_in[1] & 0x3f;
	unsigned const track = m_in[2];

	if (crc || (length > IN_SIZE))
	{
		reply[count++] = NAK;
		send(reply, count);
		return;
	}

	switch (m_in[0])
	{
	case 'B':
	case 'Q':
	case 'R':
		{
			uint8_t buffer[128];
			bool const boot = m_in[0] == 'B';
			if (!m_drive[boot ? 0 : drive]->read_sector(boot ? 0 : track, boot ? 0 : sector, buffer))
			{
				result(ERR_READ);
			}
			else
			{
				result(ERR_OK);
				data(buffer, 128);
			}
		}
		break;

	case 'T':
	case 'W':
		if (m_wp[drive] || m_drive[drive]->is_readonly())
			result(ERR_WRITE_PROTECT);
		else
			result(m_drive[drive]->write_sector(track, sector, &m_in[3]) ? ERR_OK : ERR_WRITE);
		break;

	case 'F':
		if (m_wp[drive] || m_drive[drive]->is_readonly())
		{
			result(ERR_WRITE_PROTECT);
		}
		else
		{
			uint8_t buffer[128];
			std::fill(std::begin(buffer), std::end(buffer), 0xe5);
			bool ok = m_drive[drive]->sectors() != 0;
			for (unsigned s = 0; ok && (s < m_drive[drive]->sectors()); s++)
				ok = m_drive[drive]->write_sector(track, s, buffer);
			result(ok ? ERR_OK : ERR_WRITE);
		}
		break;

	case 'I':
		result((m_in[1] < 4) ? ERR_OK : ERR_BAD_DRIVE);
		break;

	case 'G':
		if (m_in[1] >= 4)
		{
			result(ERR_BAD_DRIVE);
		}
		else
		{
			pmd32sd_image_device &image = *m_drive[m_in[1]];
			char const *const basename = image.exists() ? image.basename() : nullptr;
			std::string_view const name = basename ? basename : "";
			uint8_t buffer[65];
			unsigned const len = std::min<size_t>(name.length(), 63);
			buffer[0] = (m_wp[m_in[1]] || image.is_readonly()) ? 0x01 : 0x00;
			buffer[1] = len;
			std::copy_n(name.begin(), len, &buffer[2]);
			result(ERR_OK);
			data(buffer, 2 + len);
		}
		break;

	case 'H':
		if (m_in[1] >= 4)
		{
			result(ERR_BAD_DRIVE);
		}
		else if (m_in[3] == 0x00)
		{
			m_drive[m_in[1]]->unload();
			result(ERR_OK);
		}
		else if (m_in[3] == 0xff)
		{
			m_wp[m_in[1]] = m_in[2] != 0;
			result(ERR_OK);
		}
		else
		{
			result(ERR_NOT_FOUND);
		}
		break;

	case 'K':
		{
			static constexpr uint8_t ROOT[] = { 1, '/' };
			reply[count++] = ACK;
			data(ROOT, 2);
		}
		break;

	case 'L':
		{
			static constexpr uint8_t EMPTY[] = { 0 };
			reply[count++] = ACK;
			data(EMPTY, 1);
		}
		break;

	case 'M':
		result(ERR_NOT_FOUND);
		break;

	case 'N':
		result(ERR_CREATE);
		break;

	case 'P':
		if (m_in[1] >= 4)
		{
			result(ERR_BAD_DRIVE);
		}
		else
		{
			pmd32sd_image_device const &image = *m_drive[m_in[1]];
			uint8_t const buffer[] = { uint8_t(image.tracks()), uint8_t(image.sectors()), image.sector_size() };
			result(ERR_OK);
			data(buffer, 3);
		}
		break;

	case '*':
	case '@':
		result(ERR_OK);
		break;

	case 'S':
		logerror("unsupported command %c\n", m_in[0]);
		result(ERR_OTHER);
		break;

	default:
		logerror("unsupported command %02x\n", m_in[0]);
		reply[count++] = NAK;
		break;
	}

	send(reply, count);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(PMD32SD, device_pmd85_gpio_interface, pmd32sd_device, "pmd32sd", "RM-TEAM PMD 32-SD disk unit")
