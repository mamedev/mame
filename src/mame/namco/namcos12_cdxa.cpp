// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

    Namco System 12 CDXA PCB

***************************************************************************/

#include "emu.h"
#include "namcos12_cdxa.h"

#include "bus/ata/atapicdr.h"

// Any drive as long as the ident name starts with "TOSHIB" will do, but this is the one that's used with CDXA games specifically
DECLARE_DEVICE_TYPE(TOSHIBA_XM6402B_CDROM, toshiba_xm6402b_cdrom_device)

class toshiba_xm6402b_cdrom_device : public atapi_fixed_cdrom_device
{
public:
	toshiba_xm6402b_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: atapi_fixed_cdrom_device(mconfig, TOSHIBA_XM6402B_CDROM, tag, owner, clock)
	{
	}

	virtual void device_start() override
	{
		atapi_fixed_cdrom_device::device_start();

		std::fill_n(&m_identify_buffer[27], ' ', 47 - 27);

		const char cdrom_ident_name[] = "TOSHIBA CD-ROM XM-6402B ";
		for (int i = 0; i < strlen(cdrom_ident_name) / 2; i++)
			m_identify_buffer[27 + i] = (cdrom_ident_name[i * 2] << 8) | cdrom_ident_name[i * 2 + 1];
	}
};

DEFINE_DEVICE_TYPE(TOSHIBA_XM6402B_CDROM, toshiba_xm6402b_cdrom_device, "toshiba_xm6402b_cdrom", "Toshiba XM-6402B CD-ROM")

///

DEFINE_DEVICE_TYPE(NAMCOS12_CDXA, namcos12_cdxa_device, "namcos12_cdxa", "Namco System 12 CDXA PCB")


namcos12_cdxa_device::namcos12_cdxa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCOS12_CDXA, tag, owner, clock)
	, m_maincpu(*this, "cdxa_cpu")
	, m_cram(*this, "cram")
	, m_sram(*this, "sram")
	, m_ata(*this, "ata")
	, m_lc78836m(*this, "lc78836m")
	, m_mb87078(*this, "mb87078")
	, m_icd2061a(*this, "icd2061a")
	, m_psx_int10_cb(*this)
{
}

void namcos12_cdxa_device::device_start()
{
	save_item(NAME(m_ide_sh2_enabled));
	save_item(NAME(m_ide_ps1_enabled));
	save_item(NAME(m_sram_enabled));
	save_item(NAME(m_psx_int10_busy));
	save_item(NAME(m_audio_cur_bit));
	save_item(NAME(m_volume_write_counter));
	save_item(NAME(m_audio_lrck));

	m_lc78836m->cksl1_w(1); // configure for 448 fs for 37800hz. setting to 0 gives 384 fs to make 44100hz
	m_lc78836m->cksl2_w(0); // these are all grounded
	m_lc78836m->fs1_w(0);
	m_lc78836m->fs2_w(0);
	m_lc78836m->emp_w(0);
}

void namcos12_cdxa_device::device_reset()
{
	m_ide_sh2_enabled = m_ide_ps1_enabled = false;
	m_sram_enabled = false;
	m_psx_int10_busy = false;
	m_audio_cur_bit = 0;
	m_volume_write_counter = 0;
	m_audio_lrck = 1; // start with left channel first

	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void namcos12_cdxa_device::device_add_mconfig(machine_config &config)
{
	SH2_SH7014(config, m_maincpu, XTAL(14'745'600));
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos12_cdxa_device::sh7014_map);

	ATA_INTERFACE(config, m_ata).options([] (device_slot_interface &device) { device.option_add("cdrom", TOSHIBA_XM6402B_CDROM); }, "cdrom", nullptr, true);

	LC78836M(config, m_lc78836m, XTAL(16'934'400));

	MB87078(config, m_mb87078);
	m_mb87078->gain_changed().set(FUNC(namcos12_cdxa_device::mb87078_gain_changed));

	// fudge the input clock for clockgen slightly so the sample rate becomes an even 37800hz instead of 37806hz, otherwise audio has a slight crackle
	ICD2061A(config, m_icd2061a, 14'742'890);
	m_icd2061a->vclkout_changed().set([this] (uint32_t clock) {
		m_maincpu->sci_set_external_clock_period<0>(attotime::from_hz(clock * 16 / 448));
		m_lc78836m->set_clock(clock / 2);
	});

	m_maincpu->sci_set_send_full_data_transmit_on_sync_hack<0>(true);
	m_maincpu->sci_tx_w<0>().set(FUNC(namcos12_cdxa_device::audio_dac_w));
	m_maincpu->write_portb().set(FUNC(namcos12_cdxa_device::portb_w));
}

void namcos12_cdxa_device::sh7014_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).ram().share("cram"); // 128k x8 SRAM (x2) connected directly to both the SH2 and C448
	map(0x00400000, 0x0040ffff).ram().share("sram"); // 32k x8 SRAM (x2) connected via the C448 chip
	map(0x0041601c, 0x0041601f).w(FUNC(namcos12_cdxa_device::trigger_psx_int10_w));
	// 417000 ?
	map(0x0041c000, 0x0041c003).nopw().r(FUNC(namcos12_cdxa_device::cdrom_status_flag_r));
	map(0x00c00000, 0x00c1ffff).rw(FUNC(namcos12_cdxa_device::sh2_cdrom_cs0_r), FUNC(namcos12_cdxa_device::sh2_cdrom_cs0_w));

	// Definitely is cs1 but hooking it up breaks the CD drive state seemingly due to timing issues.
	// The code sets cs1 control reg to 0xe, causing it go into the software reset routine immediately.
	// The game's code wants to immediately send a command for IDE_COMMAND_CHECK_POWER_MODE after setting cs1, but MAME rejects
	// all commands immediately after SRST is set so it gets stuck in an infinite loop waiting for the status register to become
	// 8 (IDE_STATUS_DRDY) so it can finish sending the rest of the check power mode command.
	// Things seem to work fine without it being hooked up so this is for documentation purposes.
	// map(0x00c20000, 0x00c3ffff).rw(FUNC(namcos12_cdxa_device::sh2_cdrom_cs1_r), FUNC(namcos12_cdxa_device::sh2_cdrom_cs1_w));
}

void namcos12_cdxa_device::reset_sh2_w(uint16_t data)
{
	if (data == 1)
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

uint32_t namcos12_cdxa_device::sh2_ram_r(offs_t offset)
{
	auto r = m_sram_enabled ? m_sram : m_cram;
	return rotl_32(r[offset], 16);
}

void namcos12_cdxa_device::sh2_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	mem_mask = rotl_32(mem_mask, 16);
	data = rotl_32(data, 16);

	if (!m_sram_enabled)
		COMBINE_DATA(&m_cram[offset]);

	COMBINE_DATA(&m_sram[offset]);
}

void namcos12_cdxa_device::sram_enabled_w(uint16_t data)
{
	m_sram_enabled = data != 0;
}

void namcos12_cdxa_device::ide_sh2_enabled_w(uint16_t data)
{
	// The SH2 is allowed to use the IDE device
	// In the code it's one or the other so this and cdxa_ide_ps1_enabled_w might be mutually exclusive
	m_ide_sh2_enabled = data != 0;
}

void namcos12_cdxa_device::ide_ps1_enabled_w(uint16_t data)
{
	// The PS1 is allowed to access the CD-ROM directly, uses the 0x1f7e0000-0x1f7e000f registers
	// truckk does not use this feature so it can't be tested, but the code exists in truckk to use it so its usage can at least be confirmed
	m_ide_ps1_enabled = data != 0;
}

void namcos12_cdxa_device::ps1_int10_finished_w(uint16_t data)
{
	m_psx_int10_busy = false;
}

void namcos12_cdxa_device::trigger_psx_int10_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_psx_int10_cb(1);
	m_psx_int10_busy = true;
}

uint16_t namcos12_cdxa_device::cdrom_status_flag_r(offs_t offset, uint16_t mem_mask)
{
	// Needs to return 0x800 for it to signal the interrupt for the PS1
	// Guessed
	return m_psx_int10_busy ? 0 : 0x800;
}

uint16_t namcos12_cdxa_device::sh2_cdrom_cs0_r(offs_t offset, uint16_t mem_mask)
{
	return m_ata->cs0_r(offset >> 13, mem_mask);
}

void namcos12_cdxa_device::sh2_cdrom_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata->cs0_w(offset >> 13, data, mem_mask);
}

void namcos12_cdxa_device::volume_w(offs_t offset, uint16_t data)
{
	// The data/pins to be held low/high is encoded into the memory address
	m_volume_write_counter ^= 1;
	m_mb87078->data_w(m_volume_write_counter, offset & 0xff);
}

uint16_t namcos12_cdxa_device::cdrom_cs0_r(offs_t offset, uint16_t mem_mask)
{
	return m_ata->cs0_r(offset, mem_mask);
}

void namcos12_cdxa_device::cdrom_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata->cs0_w(offset, data, mem_mask);
}

void namcos12_cdxa_device::mb87078_gain_changed(offs_t offset, uint8_t data)
{
	m_lc78836m->set_output_gain(offset, m_mb87078->gain_factor_r(offset));
}

void namcos12_cdxa_device::audio_dac_w(int state)
{
	m_lc78836m->bclk_w(0);
	m_lc78836m->lrck_w(m_audio_lrck);
	m_lc78836m->data_w(state);
	m_lc78836m->bclk_w(1);

	m_audio_cur_bit++;

	if (m_audio_cur_bit >= 16) {
		m_audio_lrck ^= 1;
		m_audio_cur_bit = 0;
	}
}

void namcos12_cdxa_device::portb_w(uint16_t data)
{
	m_lc78836m->mute_w(BIT(data, 6));
}

void namcos12_cdxa_device::clockgen_w(offs_t offset, uint16_t data)
{
	switch (offset) {
		case 0:
			m_icd2061a->data_w(0);
			break;
		case 1:
			m_icd2061a->data_w(1);
			break;
		case 2:
			m_icd2061a->clk_w(0);
			break;
		case 3:
			m_icd2061a->clk_w(1);
			break;
	}
}
