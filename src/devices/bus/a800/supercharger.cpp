// license: BSD-3-Clause
// copyright-holders: Angelo Salese
// thanks-to: Jindroush, HiassofT
/**************************************************************************************************

"SuperCharger"

multiply/divide math unit (unknown 18-pins chip type, scratched on PCB)
used by "Assault Force" floppy disk.
Not to be confused with Starpath SuperCharger, which is for Atari VCS.

**************************************************************************************************/

#include "emu.h"
#include "supercharger.h"

// device type definition
DEFINE_DEVICE_TYPE(A800_SUPER_CHARGER, a800_supercharger_device, "a800_supercharger", "Atari 8-bit SuperCharger 3D math unit cart")

a800_supercharger_device::a800_supercharger_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, A800_SUPER_CHARGER, tag, owner, clock)
	, device_a800_cart_interface(mconfig, *this)
	, m_status(0)
{
}

void a800_supercharger_device::device_start()
{
	save_pointer(NAME(m_data), 3);
	save_item(NAME(m_status));
}

void a800_supercharger_device::device_reset()
{
	std::fill(std::begin(m_data), std::end(m_data), 0);
	m_status = 1;
}

void a800_supercharger_device::cctl_map(address_map &map)
{
	map(0x00, 0x02).mirror(0xfc).lrw8(
		NAME([this](offs_t offset) { return m_data[offset]; }),
		NAME([this](offs_t offset, u8 data) { m_data[offset] = data; })
	);
	map(0x03, 0x03).mirror(0xfc).rw(FUNC(a800_supercharger_device::status_r), FUNC(a800_supercharger_device::command_w));
}

/*
 * ---- ---x last command status
 *           (0) valid
 *           (1) error (division by zero, result == 0 or unsupported command)
 */
u8 a800_supercharger_device::status_r(offs_t offset)
{
	return m_status;
}

// TODO: not instant
// (program sets four NOPs after each call)
void a800_supercharger_device::command_w(offs_t offset, u8 data)
{
	switch (data)
	{
		// division
		case 1:
		{
			const u32 numerator = (m_data[1] << 8) | (m_data[2] & 0xff);
			const u32 denominator = m_data[0];

			if (m_data[0] < m_data[1] || denominator == 0)
				m_status = 1;
			else
			{
				m_data[1] = (u8)(numerator % denominator);
				m_data[2] = (u8)(numerator / denominator);
				m_status = 0;
			}
			break;
		}
		// multiplication
		case 2:
		{
			const u32 result = m_data[0] * m_data[2];

			m_data[1] = (u8)(result >> 8);
			m_data[2] = (u8)(result & 0xff);
			m_status = 0;
			break;
		}
		default:
			m_status = 1;
			break;
	}
}
