// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Electron 68000 Expansion

    This is a homebrew 68000 second processor for the Electron from around 1990.

    Commands:
    *FX29 - sends a short stub to the 68000 which tells it to:
            (a) start the 68000 OS in ROM now.
            (b) start the 68000 OS on every reset from now on, until the next
                CTRL+BREAK or power cycle.
    *FX28 - tells sideways bank 12 to make itself the current language ROM on the
            6502, and the language entry point tells the 68000 to start the BASIC
            interpreter in ROM, but the 68000 OS has to be currently running.

    BREAK will restart the current language ROM, so will restart 68000 BASIC.
    CTRL+BREAK will go back to 6502 BASIC as the language ROM.

    Usage notes:
    - to enter 68000 BASIC do *FX29, then *FX28, then BREAK.
    - to start the word processor, do CALL&407000 from the 68000 BASIC prompt.
    - to use the assembler, type your assembly code in as a BASIC program, with
      instruction mnemonics in lower case. Set D% to where in RAM you want the
      object code stored and P% to 2, then CALL&401C8C.

**********************************************************************/


#include "emu.h"
#include "mc68k.h"

#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/input_merger.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_MC68K, electron_mc68k_device, "electron_mc68k", "Electron 68000 Expansion")



//-------------------------------------------------
//  ADDRESS_MAP( mem_map )
//-------------------------------------------------

void electron_mc68k_device::mem_map(address_map &map)
{
	map(0x000000, 0x00ffff).ram();
	map(0x400000, 0x40ffff).rom().region("main_rom", 0);
	map(0x800000, 0x800fff).rom().region("boot_rom", 0);
	map(0xff8000, 0xff801f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask16(0xff00);
	map(0xff8020, 0xff803f).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write)).umask16(0xff00);
	map(0xff8040, 0xff805f).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0xff00);
}

//-------------------------------------------------
//  ROM( mc68k )
//-------------------------------------------------

ROM_START( mc68k )
	ROM_REGION(0x8000, "exp_rom", 0)
	ROM_LOAD("elk68k_lower.bin", 0x0000, 0x4000, CRC(6e18f9fb) SHA1(31d228e4b6be18c51dd6a0c42c9e6e1e507decc6))
	ROM_LOAD("elk68k_upper.bin", 0x4000, 0x4000, CRC(0c960571) SHA1(cf3a060fee7ac23128d9e4657790d218681e6e61))

	ROM_REGION16_BE(0x1000, "boot_rom", ROMREGION_ERASE00)
	ROM_LOAD("68kboot.bin", 0x0000, 0x0100, CRC(5855ef60) SHA1(76165ee0f34a24fd2a908987fd5f05e23ea75677))

	ROM_REGION16_BE(0x10000, "main_rom", ROMREGION_ERASE00)
	ROM_LOAD("68kmain.bin", 0x0000, 0x10000, CRC(4d7a9e4f) SHA1(90afb5910995511ae943ad62ea2967eff85e8ef7))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_mc68k_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &electron_mc68k_device::mem_map);
	m_maincpu->disable_interrupt_mixer();

	INPUT_MERGER_ANY_HIGH(config, "irq_ipl0").output_handler().set_inputline(m_maincpu, M68K_IRQ_IPL0);
	INPUT_MERGER_ANY_HIGH(config, "irq_ipl1").output_handler().set_inputline(m_maincpu, M68K_IRQ_IPL1);
	INPUT_MERGER_ANY_HIGH(config, "irq_exp").output_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));

	RAM(config, m_ram).set_default_size("256K").set_extra_options("64K,128K,192K");

	PIA6821(config, m_pia[0], 0);
	m_pia[0]->writepb_handler().set(m_pia[1], FUNC(pia6821_device::set_a_input));
	m_pia[0]->ca2_handler().set(m_pia[1], FUNC(pia6821_device::cb1_w));
	m_pia[0]->cb2_handler().set(m_pia[1], FUNC(pia6821_device::ca1_w));
	m_pia[0]->irqa_handler().set("irq_ipl0", FUNC(input_merger_device::in_w<0>));
	m_pia[0]->irqb_handler().set("irq_ipl0", FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia[1], 0);
	m_pia[1]->writepb_handler().set(m_pia[0], FUNC(pia6821_device::set_a_input));
	m_pia[1]->ca2_handler().set(m_pia[0], FUNC(pia6821_device::cb1_w));
	m_pia[1]->cb2_handler().set(m_pia[0], FUNC(pia6821_device::ca1_w));
	m_pia[1]->irqa_handler().set("irq_exp", FUNC(input_merger_device::in_w<0>));
	m_pia[1]->irqb_handler().set("irq_exp", FUNC(input_merger_device::in_w<1>));

	via6522_device &via(MOS6522(config, "via", 10_MHz_XTAL / 10));
	via.irq_handler().set("irq_ipl1", FUNC(input_merger_device::in_w<0>));

	acia6850_device &acia(ACIA6850(config, "acia", 0));
	acia.irq_handler().set("irq_ipl1", FUNC(input_merger_device::in_w<1>));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 3.6864_MHz_XTAL / 12));
	acia_clock.signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia", FUNC(acia6850_device::write_rxc));
}

const tiny_rom_entry *electron_mc68k_device::device_rom_region() const
{
	return ROM_NAME( mc68k );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_mc68k_device - constructor
//-------------------------------------------------

electron_mc68k_device::electron_mc68k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ELECTRON_MC68K, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_ram(*this, RAM_TAG)
	, m_pia(*this, "pia%u", 0U)
	, m_boot_rom(*this, "boot_rom")
	, m_exp_rom(*this, "exp_rom")
	, m_romsel(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_mc68k_device::device_start()
{
	save_item(NAME(m_romsel));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_mc68k_device::device_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	// address map during booting
	program.install_rom(0x0000, 0x0fff, m_boot_rom->base());

	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0x800000, 0xffffff,
			"rom_shadow_r",
			[this] (offs_t offset, u16 &data, u16 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// address map after booting
					m_maincpu->space(AS_PROGRAM).install_ram(0x000000, m_ram->mask(), m_ram->pointer());
				}
			},
			&m_rom_shadow_tap);
}

//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

u8 electron_mc68k_device::expbus_r(offs_t offset)
{
	u8 data = 0xff;

	switch (offset >> 12)
	{
	case 0x8: case 0x9: case 0xa: case 0xb:
		switch (m_romsel)
		{
		 case 12:
			data = m_exp_rom->base()[0x0000 | (offset & 0x3fff)];
			break;
		case 13:
			data = m_exp_rom->base()[0x4000 | (offset & 0x3fff)];
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfd:
			switch (offset & 0xf8)
			{
			case 0x80:
				data = m_pia[1]->read_alt(offset);
				break;
			}
			break;
		}
	}

	return data;
}

//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_mc68k_device::expbus_w(offs_t offset, u8 data)
{
	switch (offset >> 8)
	{
	case 0xfd:
		switch (offset & 0xf8)
		{
		case 0x80:
			m_pia[1]->write_alt(offset, data);
			break;
		}
		break;

	case 0xfe:
		if (offset == 0xfe05)
		{
			m_romsel = data & 0x0f;
		}
		break;
	}
}
