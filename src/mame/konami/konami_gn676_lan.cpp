// license:BSD-3-Clause
// copyright-holders:Ville Linde, windyfairy

/*
Hardware info by Guru

Network PCB (Racing Jam and Thrill Drive (NWK-LC))
-----------
GN676-PWB(H)A
MADE IN JAPAN
(C)1998 KONAMI
|------------------------|
|  CY7C199       N676H1  |
|                        |
|CN3                     |
|  HYC2485S              |
|   XC5204        XC5210 |
|CN2                     |
|         CN1            |
|------------------------|
Notes:
      CN1      - Connector joining to CPU board CN4
      CN2/3    - RCA jacks for network cable
      HYC2485S - Hybrid ceramic module for RS485
      CY7C199  - 32k x8 SRAM (labeled 3C)
      XC5204   - Xilinx XC5204 FPGA (labeled 7C)
      XC5210   - Xilink XC5210 FPGA (labeled 6F)
      N676H1   - PALCE16V8Q-15 stamped 'N676H1'

Network PCB (Racing Jam 2 and Thrill Drive (NWK-TR))
-----------
GN676-PWB(H)B
MADE IN JAPAN
(C)1998 KONAMI
|------------------------|
|  CY7C199       N676H1  |
|                      2G|
|CN3                     |
|  HYC2485S              |
|   XC5204        XC5210 |
|CN2                     |
|         CN1            |
|------------------------|
This pcb is the same as the A version but with one added chip:
      2G       - XICOR X76F041 Secure SerialFlash (SOIC8)

                 Note: This chip is also present on *some* Hornet games on the GN715 CPU board at location
                 30C. The chip refreshes game and region specific serial data to the Timekeeper RAM if the
                 region ID matches the timekeeper. Because Racing Jam 2 and Thrill Drive came in a conversion
                 kit for Racing Jam, the two former games will first boot with a "backup data error" because
                 because of the timekeeper used for the incorrect game. Pressing the test switch then sets the
                 timekeeper back to factory settings for the new kitted game installed. If the region ID in
                 serialflash and timekeeper do not match, the game boots with a "hardware error" message.


FPGA Bitstreams
---------------
- Racing Jam (racingj)
    - Uses type A board
    - Firmware (CRC32 92fde8df, 29491 bytes)

- Racing Jam 2 (racingj2, racingj2j)
    - Uses type B board with x76 chip? (x76 isn't used?)
    - Firmware (CRC32 dfc74cc9, 29491 bytes)

- Thrill Drive (thrilld, thrilldb, thrilldbu)
    - Uses type B board with x76 chip (except thrilldbu which uses type A without the x76 chip)
    - Firmware #1 (CRC32 3760e3ce, 29490 bytes)
        - Used during initial device test (does not get uploaded with skip post)
        - Tests every register and expects to be able to read back the values it wrote for every register *except* 0x05, 0x06, and 0x09 on lanc2
        - Seems to be a stubbed version of the normal firmware with the logic for all registers stubbed except memory-related registers
    - Firmware #2 (CRC32 a8c97a75, 29490 bytes)
        - Uploaded after boot sequence (even with skip post)
        - Allows usage of x76 chip
    - Firmware #3 (CRC32 93b86e35, 29490 bytes)
        - Uploaded after security check (boot finishes, just as it starts the actual game)
        - x76 chip capability unknown (TODO: this should be tested on real hardware)

Racing Jam 1 and 2 are both programmed to send one extra 0xff at the end of the upload sequence. The Thrill Drive a8c97a75 firmware
and the Racing Jam 2 dfc74cc9 firmware are actually the same except for the final 0xff.
*/

#include "emu.h"
#include "konami_gn676_lan.h"


//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

#define DUMP_FIRMWARE 0


DEFINE_DEVICE_TYPE(KONAMI_GN676A_LAN, konami_gn676a_lan_device, "konami_gn676a_lan", "Konami GN676-PWB(H)A Network PCB")
DEFINE_DEVICE_TYPE(KONAMI_GN676B_LAN, konami_gn676b_lan_device, "konami_gn676b_lan", "Konami GN676-PWB(H)B Network PCB")

konami_gn676_lan_device::konami_gn676_lan_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	m_x76f041(*this, "eeprom"),
	m_lanc2_ram(nullptr)
{
}

konami_gn676a_lan_device::konami_gn676a_lan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: konami_gn676_lan_device(mconfig, KONAMI_GN676A_LAN, tag, owner, clock)
{
}

konami_gn676b_lan_device::konami_gn676b_lan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: konami_gn676_lan_device(mconfig, KONAMI_GN676B_LAN, tag, owner, clock)
{
}

void konami_gn676b_lan_device::device_add_mconfig(machine_config &config)
{
	X76F041(config, m_x76f041);
}

void konami_gn676_lan_device::device_start()
{
	m_lanc2_ram = std::make_unique<uint8_t[]>(0x8000);

	save_item(NAME(m_lanc2_ram_r));
	save_item(NAME(m_lanc2_ram_w));
	save_item(NAME(m_lanc2_reg));
	save_item(NAME(m_network_buffer_max_size));
	save_item(NAME(m_network_id));
	save_item(NAME(m_fpga_reset_state));
	save_item(NAME(m_fpga_uploaded));
	save_item(NAME(m_fpga_waiting_firmware));
	save_item(NAME(m_fpga_receiving));
	save_item(NAME(m_fpga_is_stubbed));
	save_item(NAME(m_fpga_firmware_size));
	save_item(NAME(m_fpga_firmware_crc));
	save_item(NAME(m_x76f041_enabled));
	save_item(NAME(m_x76f041_read_enabled));
	save_item(NAME(m_x76f041_rst_triggered));

	save_pointer(NAME(m_lanc2_ram), 0x8000);
}
void konami_gn676_lan_device::device_reset()
{
	std::fill_n(m_lanc2_ram.get(), 0x8000, 0);

	m_fpga_reset_state = true;
	reset_fpga_state(false);
}

uint8_t konami_gn676_lan_device::lanc1_r(offs_t offset)
{
	uint8_t r = 0xff;

	if (offset == 0x40)
	{
		r = (m_fpga_receiving << 6) | (m_fpga_waiting_firmware << 5);

		// racingj/racingj2 polls expecting to see bit 5 set and bit 6 unset before
		// it'll start uploading the firmware, so only set it after being polled.
		m_fpga_receiving = true;
	}

	LOG("%s: lanc1_r: read %02X from %08X\n", tag(), r, offset, machine().describe_context());

	return r;
}

void konami_gn676_lan_device::lanc1_w(offs_t offset, uint8_t data)
{
	LOG("%s: lanc1_w: wrote %02X to %08X\n", machine().describe_context(), data, offset);

	if (offset == 0x20)
	{
		if (BIT(data, 7))
		{
			m_fpga_uploaded = true;
			m_fpga_waiting_firmware = false;
			m_fpga_receiving = false;
			m_fpga_firmware_crc = util::crc32_creator::simple(m_lanc2_ram.get(), m_fpga_firmware_size);

			LOG("%s: lanc1_fpga: Found firmware with hash of %08x\n", tag(), m_fpga_firmware_crc, machine().describe_context());

			m_x76f041_enabled = m_fpga_firmware_crc == 0xa8c97a75 || m_fpga_firmware_crc == 0xdfc74cc9 || m_fpga_firmware_crc == 0x93b86e35;
			m_fpga_is_stubbed = m_fpga_firmware_crc == 0x3760e3ce;

			if (DUMP_FIRMWARE)
			{
				// DEBUG: Dump firmware for quick comparison
				FILE *f = fopen(util::string_format("firmware_%08x.bin", m_fpga_firmware_crc).c_str(), "wb");
				if (f)
				{
					fwrite(m_lanc2_ram.get(), 1, m_fpga_firmware_size, f);
					fclose(f);
				}
			}

			std::fill_n(m_lanc2_ram.get(), 0x8000, 0);
		}
	}
}

uint8_t konami_gn676_lan_device::lanc2_r(offs_t offset)
{
	if (m_fpga_waiting_firmware)
	{
		if (offset == 0)
			return 0x80; // Required to be set for firmware to be written
		return 0;
	}

	uint8_t r = m_lanc2_reg[offset];
	switch (offset)
	{
		case 1:
			r = BIT(m_lanc2_ram_r, 8, 8);
			break;

		case 2:
			r = BIT(m_lanc2_ram_r, 0, 8);
			break;

		case 3:
			r = m_lanc2_ram[m_lanc2_ram_r];
			m_lanc2_ram_r = (m_lanc2_ram_r + 1) & 0x7fff;

			if (!m_fpga_is_stubbed)
				r = 0xff; // HACK: network traffic isn't implemented so give dummy data
			break;

		case 6:
		case 7:
			if (m_fpga_is_stubbed)
			{
				// Thrill Drive's boot test checks for this specific logic
				r = (m_lanc2_reg[offset] + 3) & 0x7f;
				m_lanc2_reg[offset]++;
			}
			else
				r = 0x55; // HACK: force clients to show as disconnected
			break;

		case 9:
		{
			// Thrill Drive's boot test checks for this specific logic
			uint8_t a = m_lanc2_reg[offset] + 6;
			uint8_t b = BIT(a, 4, 2) & BIT(a, 5, 2) & BIT(a, 6, 2);
			r = (b << 4) | (a & 0x0f);

			if (m_fpga_is_stubbed)
				m_lanc2_reg[offset]++;
			break;
		}

		case 0x0c:
		case 0x0d:
			// Used with reg 6/7 to determine info about clients
			break;

		case 0x10:
			if (m_x76f041 && m_x76f041_enabled && m_x76f041_read_enabled)
				r = m_x76f041->read_sda();
			break;
	}

	LOG("%s: lanc2_r: read %02X from %08X\n", tag(), r, offset, machine().describe_context());

	return r;
}

void konami_gn676_lan_device::lanc2_w(offs_t offset, uint8_t data)
{
	if (m_fpga_waiting_firmware)
	{
		if (offset == 0)
			m_lanc2_ram[m_fpga_firmware_size++] = data;
		return;
	}

	LOG("%s: lanc2_w: wrote %02X to %08X\n", tag(), data, offset, machine().describe_context());

	m_lanc2_reg[offset] = data;

	switch (offset)
	{
		case 0:
			// This shouldn't actually be used in practice but Thrill Drive's boot test explicitly writes
			// (addr >> 16) & 1 to this register which would put it out of the available RAM space.
			break;

		case 1:
			m_lanc2_ram_r = (m_lanc2_ram_r & 0x100ff) | (data << 8);
			m_lanc2_ram_w = (m_lanc2_ram_w & 0x100ff) | (data << 8);
			break;

		case 2:
			m_lanc2_ram_r = (m_lanc2_ram_r & 0x1ff00) | data;
			m_lanc2_ram_w = (m_lanc2_ram_w & 0x1ff00) | data;
			break;

		case 3:
			m_lanc2_ram[m_lanc2_ram_w] = data;
			m_lanc2_ram_w = (m_lanc2_ram_w + 1) & 0x7fff;
			break;

		case 4:
			// Network enabled flag?
			// Set to 0 after firmware is uploaded, set to 1 when setting self network ID and similar info
			break;

		case 5:
			m_network_id = data;
			break;

		case 0x10:
			if (m_x76f041 && m_x76f041_enabled)
			{
				/*
				    0x01 = x76 SDA
				    0x02 = x76 RST
				    0x04 = x76 CS???
				    0x08 = x76 SCL
				    0x10 = Controls direction of x76 SDA
				*/

				if (BIT(data, 1))
					m_x76f041_rst_triggered = true;

				m_x76f041_read_enabled = BIT(data, 4);
				m_x76f041->write_rst(BIT(data, 1));
				m_x76f041->write_scl(BIT(data, 3));

				if (!m_x76f041_read_enabled)
				{
					if (m_x76f041_rst_triggered && BIT(data, 1) == 0)
					{
						// HACK: RST was triggered previously and now we're exiting read mode, so reset
						// the x76 state so we're not stuck in a loop reading the reset response.
						// My guess is bit 2 is used in some way for this but the usage doesn't make
						// sense (if used as CS then it'll reset the chip state entirely during ACK
						// polling in normal usage). FPGA magic?
						m_x76f041->write_cs(1);
						m_x76f041->write_cs(0);
						m_x76f041_rst_triggered = false;
					}

					m_x76f041->write_sda(BIT(data, 0));
				}
			}
			break;

		case 0x1c:
			m_network_buffer_max_size = (data << 8) | (m_network_buffer_max_size & 0xff);
			break;

		case 0x1d:
			m_network_buffer_max_size = (m_network_buffer_max_size & 0xff00) | data;
			break;
	}
}

void konami_gn676_lan_device::reset_fpga_state(bool state)
{
	if (state == m_fpga_reset_state)
		return;

	// Reset state of FPGA so that it's waiting for new firmware until
	// bit 7 of lanc1_w addr 0x20 is set.
	m_fpga_reset_state = state;

	m_fpga_waiting_firmware = true;
	m_fpga_uploaded = false;
	m_fpga_receiving = false;
	m_fpga_is_stubbed = false;
	m_fpga_firmware_size = 0;
	m_fpga_firmware_crc = 0;

	m_x76f041_enabled = false;
	m_x76f041_read_enabled = false;
	m_x76f041_rst_triggered = false;

	m_lanc2_ram_r = m_lanc2_ram_w = 0;

	m_network_buffer_max_size = 0;
	m_network_id = 0;

	std::fill(std::begin(m_lanc2_reg), std::end(m_lanc2_reg), 0);
}
