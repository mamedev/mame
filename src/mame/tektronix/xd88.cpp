// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Tektronix XD88 systems.
 *
 * Sources:
 *  - https://wiki.unix-haters.org/doku.php?id=tektronix:xd88:resources
 *
 * TODO:
 *  - everything
 */

#include "emu.h"

#include "cpu/m88000/m88000.h"

#include "machine/mc88200.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class xd88_state : public driver_device
{
public:
	xd88_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_cmmu(*this, "cmmu%u", 0U)
		, m_boot(*this, "boot")
	{
	}

	void xd88_01(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void cpu_map(address_map &map);

private:
	required_device<mc88100_device> m_cpu;
	required_device_array<mc88200_device, 8> m_cmmu;

	memory_view m_boot;
};

void xd88_state::machine_start()
{
}

void xd88_state::machine_reset()
{
	m_boot.select(0);
}

void xd88_state::cpu_map(address_map &map)
{
	// TODO: RAM options (8M is standard)
	map(0x0000'0000, 0x007f'ffff).ram();

	map(0x0000'0000, 0x0003'ffff).view(m_boot);
	m_boot[0](0x0000'0000, 0x0003'ffff).rom().region("eprom", 0);

	map(0xfe00'0000, 0xfe03'ffff).rom().region("eprom", 0);
}

void xd88_state::xd88_01(machine_config &config)
{
	MC88100(config, m_cpu, 20'000'000);
	m_cpu->set_addrmap(AS_PROGRAM, &xd88_state::cpu_map);

	for (unsigned i = 0; i < std::size(m_cmmu); i++)
		MC88200(config, m_cmmu[i], 20'000'000, i).set_mbus(m_cpu, AS_PROGRAM);

	// TODO: multiple i&d cmmu's
	m_cpu->set_cmmu_d(m_cmmu[0]);
	m_cpu->set_cmmu_i(m_cmmu[4]);
}

ROM_START(xd88_01)
	ROM_REGION32_BE(0x40000, "eprom", 0)
	ROMX_LOAD("160-5796-04__u3930_v1.4.u3930", 0, 0x10000, CRC(db2ef744) SHA1(82ddb494fc1e2693da3d021c1cfeea49ef06ed3e), ROM_SKIP(3))
	ROMX_LOAD("160-5797-04__u4230_v1.4.u4230", 1, 0x10000, CRC(8fe40d66) SHA1(30cacd590c2598cb6db1933309d2d60e57f375eb), ROM_SKIP(3))
	ROMX_LOAD("160-5798-04__u4530_v1.4.u4530", 2, 0x10000, CRC(2aa2503c) SHA1(edc869a7ac9b6ce68603146884cf9a95ee22d39e), ROM_SKIP(3))
	ROMX_LOAD("160-5799-04__u4830_v1.4.u4830", 3, 0x10000, CRC(f7ecedae) SHA1(6063e616e4c1de5af048bd2cb61ad64a57c7f5a4), ROM_SKIP(3))
ROM_END

} // anonymous namespace

/*   YEAR  NAME     PARENT COMPAT MACHINE  INPUT CLASS       INIT        COMPANY      FULLNAME   FLAGS */
COMP(1990, xd88_01, 0,     0,     xd88_01, 0,    xd88_state, empty_init, "Tektronix", "XD88/01", MACHINE_IS_SKELETON)
