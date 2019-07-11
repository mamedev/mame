// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for CDR4210 SCSI CD-R drive by Creative Technology Ltd.

*******************************************************************************/

#include "emu.h"
#include "cpu/m37710/m37710.h"
#include "machine/ncr5390.h"
#include "machine/nscsi_bus.h"

class cdr4210_state : public driver_device
{
public:
	cdr4210_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void cdr4210(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	u8 mystery_data_r();
	void mystery_data_w(u8 data);
	void mystery_address_w(u8 data);

	void mem_map(address_map &map);

	required_device<m37710s4_device> m_maincpu;

	u8 m_mystery_address;
};

void cdr4210_state::machine_start()
{
	m_mystery_address = 0;

	save_item(NAME(m_mystery_address));
}

u8 cdr4210_state::mystery_data_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: Reading from mystery register #%02X\n", machine().describe_context(), m_mystery_address);

	return 0;
}

void cdr4210_state::mystery_data_w(u8 data)
{
	logerror("%s: Writing %02X to mystery register #%02X\n", machine().describe_context(), data, m_mystery_address);
}

void cdr4210_state::mystery_address_w(u8 data)
{
	m_mystery_address = data;
}

void cdr4210_state::mem_map(address_map &map)
{
	map(0x000880, 0x007fff).ram();
	map(0x008000, 0x03ffff).rom().region("flash", 0x08000);
	map(0x050000, 0x050000).w(FUNC(cdr4210_state::mystery_address_w));
	map(0x050001, 0x050001).rw(FUNC(cdr4210_state::mystery_data_r), FUNC(cdr4210_state::mystery_data_w));
	map(0x058000, 0x05800f).m("scsi:7:scsic", FUNC(ncr53cf94_device::map));
	map(0x060000, 0x060003).noprw(); // ?
}

static INPUT_PORTS_START(cdr4210)
INPUT_PORTS_END

static void scsi_devices(device_slot_interface &device)
{
	device.option_add_internal("scsic", NCR53CF94).clock(25'000'000); // type guessed
}

void cdr4210_state::cdr4210(machine_config &config)
{
	M37710S4(config, m_maincpu, 12'500'000); // type and clock are total guesses
	m_maincpu->set_addrmap(AS_PROGRAM, &cdr4210_state::mem_map);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", scsi_devices, "scsic", true);
}

ROM_START(cdr4210)
	ROM_REGION16_LE(0x40000, "flash", 0)
	ROM_LOAD("cr113.bin", 0x00000, 0x40000, CRC(fd2faff9) SHA1(6aafdedf12240ad347427287c0db289f90bd064d))
ROM_END

// this should probably be the parent
ROM_START(cw7501)
	ROM_REGION16_LE(0x40000, "flash", 0)
	ROM_LOAD("mk200.bin", 0x00000, 0x40000, CRC(12efd802) SHA1(2986ee5eedbe0cb662a9a7e7fa4e6ca7ccd8c539))
ROM_END

// another clone: Plasmon CDR 4240


SYST(1996, cdr4210, 0,       0, cdr4210, cdr4210, cdr4210_state, empty_init, "Creative Technology", "CD-R 4210 (v1.13)", MACHINE_IS_SKELETON)
SYST(1996, cw7501,  cdr4210, 0, cdr4210, cdr4210, cdr4210_state, empty_init, "Panasonic", "CW-7501 (v2.00)", MACHINE_IS_SKELETON)
