// license:BSD-3-Clause
// copyright-holders:Ville Linde

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
      CY7C199  - 32k x8 SRAM
      XC5204   - Xilinx XC5204 FPGA
      XC5210   - Xilink XC5210 FPGA
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

TODO:
- Add X76F041 device when dumps will be available and get rid of the work_ram hack
*/

#include "emu.h"
#include "konami_gn676_lan.h"


//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


DEFINE_DEVICE_TYPE(KONAMI_GN676_LAN, konami_gn676_lan_device, "konami_gn676_lan", "Konami GN676 Network PCB")

konami_gn676_lan_device::konami_gn676_lan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KONAMI_GN676_LAN, tag, owner, clock),
	m_fpga_uploaded(false),
	m_lanc2_ram_r(0),
	m_lanc2_ram_w(0),
	m_lanc2_ram(nullptr),
	m_work_ram(*this, finder_base::DUMMY_TAG)
{
	std::fill(std::begin(m_lanc2_reg), std::end(m_lanc2_reg), 0);
}


void konami_gn676_lan_device::device_start()
{
	m_lanc2_ram = std::make_unique<uint8_t[]>(0x8000);

	save_item(NAME(m_fpga_uploaded));
	save_item(NAME(m_lanc2_ram_r));
	save_item(NAME(m_lanc2_ram_w));
	save_item(NAME(m_lanc2_reg));
	save_pointer(NAME(m_lanc2_ram), 0x8000);
}


uint32_t konami_gn676_lan_device::lanc1_r(offs_t offset)
{
	switch (offset)
	{
		case 0x40/4:
		{
			uint32_t r = 0;

			r |= (m_fpga_uploaded) ? (1 << 6) : 0;
			r |= 1 << 5;

			return (r) << 24;
		}

		default:
		{
			LOG("lanc1_r: %08X at %08X\n", offset, machine().describe_context());
			return 0xffffffff;
		}
	}
}

void konami_gn676_lan_device::lanc1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("lanc1_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, machine().describe_context());
}

uint32_t konami_gn676_lan_device::lanc2_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t r = 0;

	if (offset == 0)
	{
		if (ACCESSING_BITS_0_7)
		{
			r |= m_lanc2_ram[m_lanc2_ram_r & 0x7fff];
			m_lanc2_ram_r++;
		}
		else
		{
			r |= 0xffffff00;
		}
	}
	else if (offset == 1)
	{		
		r |= 0x00005555;		// set all other machines as disconnected
	}
	else if (offset == 3)
	{		
		r |= 0xffffffff;
	}
	else if (offset == 4)
	{
		if (ACCESSING_BITS_24_31)
		{
			r |= 0x00000000;
		}
	}

	LOG("lanc2_r: %08X, %08X at %08X\n", offset, mem_mask, machine().describe_context());

	return r;
}

void konami_gn676_lan_device::lanc2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0)
	{
		if (ACCESSING_BITS_24_31)
		{
			uint8_t value = data >> 24;

			value = ((value >> 7) & 0x01) |
					((value >> 5) & 0x02) |
					((value >> 3) & 0x04) |
					((value >> 1) & 0x08) |
					((value << 1) & 0x10) |
					((value << 3) & 0x20) |
					((value << 5) & 0x40) |
					((value << 7) & 0x80);

			m_fpga_uploaded = true;
			m_lanc2_reg[0] = (uint8_t)(data >> 24);

			LOG("lanc2_fpga_w: %02X at %08X\n", value, machine().describe_context());
		}
		if (ACCESSING_BITS_8_15)
		{
			m_lanc2_ram_r = 0;
			m_lanc2_ram_w = 0;
			m_lanc2_reg[1] = (uint8_t)(data >> 8);

			if (data & 0x1000)
			{
				// send out frame for this machine
			}
			else
			{
				// read from other machines
				//int machine_id = (m_lanc2_reg[2] >> 4) & 7;
				//int self = m_lanc2_reg[2] & 1;

				for (auto j = 0; j < 0x110; j++)
				{
					m_lanc2_ram[j] = 0xff;
				}
			}
		}
		if (ACCESSING_BITS_16_23)
		{
			m_lanc2_reg[2] = (uint8_t)(data >> 16);
		}
		if (ACCESSING_BITS_0_7)
		{
			m_lanc2_ram[m_lanc2_ram_w & 0x7fff] = data & 0xff;
			m_lanc2_ram_w++;
		}
	}
	if (offset == 4) // only type B has the chip at 2G
	{
		// TODO: HACK! The data below would normally be present on the serialflash at 2G.

		if (strcmp(machine().system().name, "thrilld") == 0 ||
			strcmp(machine().system().name, "thrilldb") == 0 ||
			strcmp(machine().system().name, "thrilldbe") == 0)
		{
			m_work_ram[(0x3ffed0/4) + 0] = 0x472a3731;      // G*71
			m_work_ram[(0x3ffed0/4) + 1] = 0x33202020;      // 3
			m_work_ram[(0x3ffed0/4) + 2] = 0x2d2d2a2a;      // --**
			m_work_ram[(0x3ffed0/4) + 3] = 0x2a207878;      // *

			m_work_ram[(0x3fff40/4) + 0] = 0x47433731;      // GC71
			m_work_ram[(0x3fff40/4) + 1] = 0x33000000;      // 3
			m_work_ram[(0x3fff40/4) + 2] = 0x19994a41;      //   JA
			m_work_ram[(0x3fff40/4) + 3] = 0x4100a9b1;      // A
		}
		else if (strcmp(machine().system().name, "racingj2") == 0)
		{
			m_work_ram[(0x3ffc80/4) + 0] = 0x47453838;      // GE88
			m_work_ram[(0x3ffc80/4) + 1] = 0x38003030;      // 8 00
			m_work_ram[(0x3ffc80/4) + 2] = 0x39374541;      // 97EA
			m_work_ram[(0x3ffc80/4) + 3] = 0x410058da;      // A
		}
	}

	LOG("lanc2_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, machine().describe_context());
}
