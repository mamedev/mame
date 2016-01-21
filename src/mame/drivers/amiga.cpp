// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Commodore Amiga

***************************************************************************/

#include "emu.h"
#include "includes/amiga.h"
#include "bus/amiga/zorro/zorro.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "machine/bankdev.h"
#include "machine/6525tpi.h"
#include "machine/mos6526.h"
#include "machine/gayle.h"
#include "machine/ataintf.h"
#include "machine/dmac.h"
#include "machine/nvram.h"
#include "machine/i2cmem.h"
#include "machine/amigafdc.h"
#include "machine/amigakbd.h"
#include "machine/cr511b.h"
#include "machine/rp5c01.h"
#include "softlist.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a1000_state : public amiga_state
{
public:
	a1000_state(const machine_config &mconfig, device_type type, const char *tag) :
	amiga_state(mconfig, type, tag),
	m_bootrom(*this, "bootrom"),
	m_wom(*this, "wom")
	{ }

	DECLARE_DRIVER_INIT( pal );
	DECLARE_DRIVER_INIT( ntsc );

	DECLARE_WRITE16_MEMBER( write_protect_w );

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<address_map_bank_device> m_bootrom;
	required_memory_bank m_wom;
	std::vector<UINT16> m_wom_ram;
};

class a2000_state : public amiga_state
{
public:
	a2000_state(const machine_config &mconfig, device_type type, const char *tag) :
	amiga_state(mconfig, type, tag),
	m_rtc(*this, "u65"),
	m_zorro(*this, ZORROBUS_TAG),
	m_zorro2_int2(0),
	m_zorro2_int6(0)
	{ }

	DECLARE_DRIVER_INIT( pal );
	DECLARE_DRIVER_INIT( ntsc );

	DECLARE_WRITE_LINE_MEMBER( zorro2_int2_w );
	DECLARE_WRITE_LINE_MEMBER( zorro2_int6_w );

	DECLARE_READ16_MEMBER( clock_r );
	DECLARE_WRITE16_MEMBER( clock_w );

protected:
	virtual void machine_reset() override;

	// amiga_state overrides
	virtual bool int2_pending() override;
	virtual bool int6_pending() override;

private:
	// devices
	required_device<msm6242_device> m_rtc;
	required_device<zorro2_device> m_zorro;

	// internal state
	int m_zorro2_int2;
	int m_zorro2_int6;
};

class a500_state : public amiga_state
{
public:
	a500_state(const machine_config &mconfig, device_type type, const char *tag) :
	amiga_state(mconfig, type, tag),
	m_side(*this, EXP_SLOT_TAG),
	m_side_int2(0),
	m_side_int6(0)
	{ }

	DECLARE_DRIVER_INIT( pal );
	DECLARE_DRIVER_INIT( ntsc );

protected:
	virtual void machine_reset() override;

	// amiga_state overrides
	virtual bool int2_pending() override;
	virtual bool int6_pending() override;

private:
	// devices
	required_device<exp_slot_device> m_side;

	// internal state
	int m_side_int2;
	int m_side_int6;
};

class cdtv_state : public amiga_state
{
public:
	cdtv_state(const machine_config &mconfig, device_type type, const char *tag) :
	amiga_state(mconfig, type, tag),
	m_rtc(*this, "u61"),
	m_dmac(*this, "u36"),
	m_tpi(*this, "u32"),
	m_cdrom(*this, "cdrom"),
	m_dmac_irq(0),
	m_tpi_irq(0)
	{ }

	DECLARE_DRIVER_INIT( pal );
	DECLARE_DRIVER_INIT( ntsc );

	DECLARE_READ16_MEMBER( clock_r );
	DECLARE_WRITE16_MEMBER( clock_w );

	DECLARE_READ8_MEMBER( dmac_scsi_data_read );
	DECLARE_WRITE8_MEMBER( dmac_scsi_data_write );
	DECLARE_READ8_MEMBER( dmac_io_read );
	DECLARE_WRITE8_MEMBER( dmac_io_write );
	DECLARE_WRITE_LINE_MEMBER( dmac_int_w );

	DECLARE_WRITE8_MEMBER( tpi_port_b_write );
	DECLARE_WRITE_LINE_MEMBER( tpi_int_w );

protected:
	// driver_device overrides
	virtual void machine_start() override;

	// amiga_state overrides
	virtual bool int2_pending() override;
	virtual bool int6_pending() override;

private:
	// devices
	required_device<msm6242_device> m_rtc;
	required_device<dmac_device> m_dmac;
	required_device<tpi6525_device> m_tpi;
	required_device<cr511b_device> m_cdrom;

	// internal state
	int m_dmac_irq;
	int m_tpi_irq;
};

class a3000_state : public amiga_state
{
public:
	a3000_state(const machine_config &mconfig, device_type type, const char *tag) :
	amiga_state(mconfig, type, tag)
	{ }

	DECLARE_READ32_MEMBER( scsi_r );
	DECLARE_WRITE32_MEMBER( scsi_w );
	DECLARE_READ32_MEMBER( motherboard_r );
	DECLARE_WRITE32_MEMBER( motherboard_w );

	DECLARE_DRIVER_INIT( pal );
	DECLARE_DRIVER_INIT( ntsc );

protected:

private:
};

class a500p_state : public amiga_state
{
public:
	a500p_state(const machine_config &mconfig, device_type type, const char *tag) :
	amiga_state(mconfig, type, tag),
	m_rtc(*this, "u9"),
	m_side(*this, EXP_SLOT_TAG),
	m_side_int2(0),
	m_side_int6(0)
	{ }

	DECLARE_READ16_MEMBER( clock_r );
	DECLARE_WRITE16_MEMBER( clock_w );

	DECLARE_DRIVER_INIT( pal );
	DECLARE_DRIVER_INIT( ntsc );

protected:
	virtual void machine_reset() override;

	// amiga_state overrides
	virtual bool int2_pending() override;
	virtual bool int6_pending() override;

private:
	// devices
	required_device<msm6242_device> m_rtc;
	required_device<exp_slot_device> m_side;

	// internal state
	int m_side_int2;
	int m_side_int6;
};

class a600_state : public amiga_state
{
public:
	a600_state(const machine_config &mconfig, device_type type, const char *tag) :
	amiga_state(mconfig, type, tag),
	m_gayle_int2(0)
	{ }

	DECLARE_WRITE_LINE_MEMBER( gayle_int2_w );

	DECLARE_DRIVER_INIT( pal );
	DECLARE_DRIVER_INIT( ntsc );

	static const UINT8 GAYLE_ID = 0xd0;

protected:
	virtual bool int2_pending() override;

private:
	int m_gayle_int2;
};

class a1200_state : public amiga_state
{
public:
	a1200_state(const machine_config &mconfig, device_type type, const char *tag) :
	amiga_state(mconfig, type, tag),
	m_gayle_int2(0)
	{ }

	DECLARE_WRITE_LINE_MEMBER( gayle_int2_w );

	DECLARE_DRIVER_INIT( pal );
	DECLARE_DRIVER_INIT( ntsc );

	static const UINT8 GAYLE_ID = 0xd1;

protected:
	virtual bool int2_pending() override;

private:
	int m_gayle_int2;
};

class a4000_state : public amiga_state
{
public:
	a4000_state(const machine_config &mconfig, device_type type, const char *tag) :
	amiga_state(mconfig, type, tag),
	m_ata(*this, "ata"),
	m_ramsey_config(0),
	m_gary_coldboot(1),
	m_gary_timeout(0),
	m_gary_toenb(0),
	m_ide_interrupt(0)
	{ }

	DECLARE_READ32_MEMBER( scsi_r );
	DECLARE_WRITE32_MEMBER( scsi_w );
	DECLARE_READ16_MEMBER( ide_r );
	DECLARE_WRITE16_MEMBER( ide_w );
	DECLARE_WRITE_LINE_MEMBER( ide_interrupt_w );
	DECLARE_READ32_MEMBER( motherboard_r );
	DECLARE_WRITE32_MEMBER( motherboard_w );

	DECLARE_DRIVER_INIT( pal );
	DECLARE_DRIVER_INIT( ntsc );

protected:

private:
	required_device<ata_interface_device> m_ata;

	int m_ramsey_config;
	int m_gary_coldboot;
	int m_gary_timeout;
	int m_gary_toenb;
	int m_ide_interrupt;
};

class cd32_state : public amiga_state
{
public:
	cd32_state(const machine_config &mconfig, device_type type, const char *tag) :
	amiga_state(mconfig, type, tag),
	m_p1_port(*this, "p1_cd32_buttons"),
	m_p2_port(*this, "p2_cd32_buttons"),
	m_cdda(*this, "cdda")
	{ }

	DECLARE_WRITE8_MEMBER( akiko_cia_0_port_a_write );

	DECLARE_CUSTOM_INPUT_MEMBER( cd32_input );
	DECLARE_CUSTOM_INPUT_MEMBER( cd32_sel_mirror_input );

	DECLARE_DRIVER_INIT( pal );
	DECLARE_DRIVER_INIT( ntsc );

	required_ioport m_p1_port;
	required_ioport m_p2_port;

	int m_oldstate[2];
	int m_cd32_shifter[2];
	UINT16 m_potgo_value;

protected:
	// amiga_state overrides
	virtual void potgo_w(UINT16 data) override;

private:
	required_device<cdda_device> m_cdda;
};


//**************************************************************************
//  REAL TIME CLOCK
//**************************************************************************

READ16_MEMBER( cdtv_state::clock_r )
{
	return m_rtc->read(space, offset / 2);
}

WRITE16_MEMBER( cdtv_state::clock_w )
{
	m_rtc->write(space, offset / 2, data);
}

READ16_MEMBER( a2000_state::clock_r )
{
	return m_rtc->read(space, offset / 2);
}

WRITE16_MEMBER( a2000_state::clock_w )
{
	m_rtc->write(space, offset / 2, data);
}

READ16_MEMBER( a500p_state::clock_r )
{
	return m_rtc->read(space, offset / 2);
}

WRITE16_MEMBER( a500p_state::clock_w )
{
	m_rtc->write(space, offset / 2, data);
}


//**************************************************************************
//  CD-ROM CONTROLLER
//**************************************************************************

READ8_MEMBER( cdtv_state::dmac_scsi_data_read )
{
	if (offset >= 0xb0 && offset <= 0xbf)
		return m_tpi->read(space, offset);

	return 0xff;
}

WRITE8_MEMBER( cdtv_state::dmac_scsi_data_write )
{
	if (offset >= 0xb0 && offset <= 0xbf)
		m_tpi->write(space, offset, data);
}

READ8_MEMBER( cdtv_state::dmac_io_read )
{
	return m_cdrom->read(space, 0);
}

WRITE8_MEMBER( cdtv_state::dmac_io_write )
{
	m_cdrom->write(space, 0, data);
}

WRITE_LINE_MEMBER( cdtv_state::dmac_int_w )
{
	m_dmac_irq = state;
	update_int2();
}

WRITE8_MEMBER( cdtv_state::tpi_port_b_write )
{
	m_cdrom->cmd_w(BIT(data, 0));
	m_cdrom->enable_w(BIT(data, 1));
}

WRITE_LINE_MEMBER( cdtv_state::tpi_int_w )
{
	m_tpi_irq = state;
	update_int2();
}


//**************************************************************************
//  DRIVER INIT
//**************************************************************************

// ocs chipset (agnus with support for 512k or 1mb chip ram, denise)
DRIVER_INIT_MEMBER( a1000_state, pal )
{
	m_agnus_id = AGNUS_PAL;     // 8367
	m_denise_id = DENISE;       // 8362
}

DRIVER_INIT_MEMBER( a1000_state, ntsc )
{
	m_agnus_id = AGNUS_NTSC;    // 8361
	m_denise_id = DENISE;       // 8362
}

DRIVER_INIT_MEMBER( a2000_state, pal )
{
	m_agnus_id = AGNUS_PAL;     // 8371 (later versions 8372A)
	m_denise_id = DENISE;       // 8362
}

DRIVER_INIT_MEMBER( a2000_state, ntsc )
{
	m_agnus_id = AGNUS_NTSC;    // 8370 (later versions 8372A)
	m_denise_id = DENISE;       // 8362
}

DRIVER_INIT_MEMBER( a500_state, pal )
{
	m_agnus_id = AGNUS_PAL;     // 8371 (later versions 8372A)
	m_denise_id = DENISE;       // 8362
}

DRIVER_INIT_MEMBER( a500_state, ntsc )
{
	m_agnus_id = AGNUS_NTSC;    // 8370 (later versions 8372A)
	m_denise_id = DENISE;       // 8362
}

DRIVER_INIT_MEMBER( cdtv_state, pal )
{
	m_agnus_id = AGNUS_HR_PAL;  // 8372A
	m_denise_id = DENISE;       // 8362
}

DRIVER_INIT_MEMBER( cdtv_state, ntsc )
{
	m_agnus_id = AGNUS_HR_NTSC; // 8372A
	m_denise_id = DENISE;       // 8362
}

// ecs chipset (agnus with support for 2mb chip ram, super denise)
DRIVER_INIT_MEMBER( a3000_state, pal )
{
	m_agnus_id = AGNUS_HR_PAL_NEW;  // 8372B (early versions: 8372AB)
	m_denise_id = DENISE_HR;        // 8373
}

DRIVER_INIT_MEMBER( a3000_state, ntsc )
{
	m_agnus_id = AGNUS_HR_NTSC_NEW; // 8372B (early versions: 8372AB)
	m_denise_id = DENISE_HR;        // 8373
}

DRIVER_INIT_MEMBER( a500p_state, pal )
{
	m_agnus_id = AGNUS_HR_PAL;  // 8375 (390544-01)
	m_denise_id = DENISE_HR;    // 8373
}

DRIVER_INIT_MEMBER( a500p_state, ntsc )
{
	m_agnus_id = AGNUS_HR_NTSC; // 8375 (390544-02)
	m_denise_id = DENISE_HR;    // 8373
}

DRIVER_INIT_MEMBER( a600_state, pal )
{
	m_agnus_id = AGNUS_HR_PAL;  // 8375 (390544-01)
	m_denise_id = DENISE_HR;    // 8373
}

DRIVER_INIT_MEMBER( a600_state, ntsc )
{
	m_agnus_id = AGNUS_HR_NTSC; // 8375 (390544-02)
	m_denise_id = DENISE_HR;    // 8373
}

// aga chipset (alice and lisa)
DRIVER_INIT_MEMBER( a1200_state, pal )
{
	m_agnus_id = ALICE_PAL_NEW;
	m_denise_id = LISA;
}

DRIVER_INIT_MEMBER( a1200_state, ntsc )
{
	m_agnus_id = ALICE_NTSC_NEW;
	m_denise_id = LISA;
}

DRIVER_INIT_MEMBER( a4000_state, pal )
{
	m_agnus_id = ALICE_PAL_NEW;
	m_denise_id = LISA;
}

DRIVER_INIT_MEMBER( a4000_state, ntsc )
{
	m_agnus_id = ALICE_NTSC_NEW;
	m_denise_id = LISA;
}

DRIVER_INIT_MEMBER( cd32_state, pal )
{
	m_agnus_id = ALICE_PAL_NEW;
	m_denise_id = LISA;
}

DRIVER_INIT_MEMBER( cd32_state, ntsc )
{
	m_agnus_id = ALICE_NTSC_NEW;
	m_denise_id = LISA;
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void a1000_state::machine_start()
{
	// start base machine
	amiga_state::machine_start();

	// allocate 256kb for wom
	m_wom_ram.resize(256 * 1024 / 2);
	m_wom->set_base(&m_wom_ram[0]);
}

void a1000_state::machine_reset()
{
	// base reset
	amiga_state::machine_reset();

	// bootrom visible, wom writable
	m_bootrom->set_bank(0);
	m_maincpu->space(AS_PROGRAM).install_write_bank(0xfc0000, 0xffffff, "wom");
}

// any write to this area will write protect the wom and disable the bootrom
WRITE16_MEMBER( a1000_state::write_protect_w )
{
	m_bootrom->set_bank(1);
	m_maincpu->space(AS_PROGRAM).nop_write(0xfc0000, 0xffffff);
}

void a2000_state::machine_reset()
{
	// base reset
	amiga_state::machine_reset();

	// reset zorro devices
	m_zorro->reset();
}

WRITE_LINE_MEMBER( a2000_state::zorro2_int2_w )
{
	m_zorro2_int2 = state;
	update_int2();
}

WRITE_LINE_MEMBER( a2000_state::zorro2_int6_w )
{
	m_zorro2_int6 = state;
	update_int6();
}

bool a2000_state::int2_pending()
{
	return m_cia_0_irq || m_zorro2_int2;
}

bool a2000_state::int6_pending()
{
	return m_cia_1_irq || m_zorro2_int6;
}

void a500_state::machine_reset()
{
	// base reset
	amiga_state::machine_reset();

	// reset side expansion slot device
	m_side->reset();
}

bool a500_state::int2_pending()
{
	return m_cia_0_irq || m_side_int2;
}

bool a500_state::int6_pending()
{
	return m_cia_1_irq || m_side_int6;
}

void cdtv_state::machine_start()
{
	// start base machine
	amiga_state::machine_start();

	// setup dmac
	m_dmac->set_address_space(&m_maincpu->space(AS_PROGRAM));
	m_dmac->ramsz_w(0);
}

bool cdtv_state::int2_pending()
{
	return m_cia_0_irq || m_dmac_irq || m_tpi_irq;
}

bool cdtv_state::int6_pending()
{
	return m_cia_1_irq;
}

READ32_MEMBER( a3000_state::scsi_r )
{
	UINT32 data = 0xffffffff;
	logerror("scsi_r(%06x): %08x & %08x\n", offset, data, mem_mask);
	return data;
}

WRITE32_MEMBER( a3000_state::scsi_w )
{
	logerror("scsi_w(%06x): %08x & %08x\n", offset, data, mem_mask);
}

READ32_MEMBER( a3000_state::motherboard_r )
{
	UINT32 data = 0xffffffff;
	logerror("motherboard_r(%06x): %08x & %08x\n", offset, data, mem_mask);
	return data;
}

WRITE32_MEMBER( a3000_state::motherboard_w )
{
	logerror("motherboard_w(%06x): %08x & %08x\n", offset, data, mem_mask);
}

void a500p_state::machine_reset()
{
	// base reset
	amiga_state::machine_reset();

	// reset side expansion slot device
	m_side->reset();
}

bool a500p_state::int2_pending()
{
	return m_cia_0_irq || m_side_int2;
}

bool a500p_state::int6_pending()
{
	return m_cia_1_irq || m_side_int6;
}

bool a600_state::int2_pending()
{
	return m_cia_0_irq || m_gayle_int2;
}

WRITE_LINE_MEMBER( a600_state::gayle_int2_w )
{
	m_gayle_int2 = state;
	update_int2();
}

bool a1200_state::int2_pending()
{
	return m_cia_0_irq || m_gayle_int2;
}

WRITE_LINE_MEMBER( a1200_state::gayle_int2_w )
{
	m_gayle_int2 = state;
	update_int2();
}

READ32_MEMBER( a4000_state::scsi_r )
{
	UINT16 data = 0xffff;
	logerror("scsi_r(%06x): %08x & %08x\n", offset, data, mem_mask);
	return data;
}

WRITE32_MEMBER( a4000_state::scsi_w )
{
	logerror("scsi_w(%06x): %08x & %08x\n", offset, data, mem_mask);
}

READ16_MEMBER( a4000_state::ide_r )
{
	UINT16 data = 0xffff;

	// ide interrupt register
	if (offset == 0x1010)
		return m_ide_interrupt << 15;

	// swap
	mem_mask = (mem_mask << 8) | (mem_mask >> 8);

	// this very likely doesn't respond to all the addresses, figure out which ones
	if (BIT(offset, 12))
		data = m_ata->read_cs1(space, (offset >> 1) & 0x07, mem_mask);
	else
		data = m_ata->read_cs0(space, (offset >> 1) & 0x07, mem_mask);

	// swap
	data = (data << 8) | (data >> 8);

	return data;
}

WRITE16_MEMBER( a4000_state::ide_w )
{
	// ide interrupt register, read only
	if (offset == 0x1010)
		return;

	// swap
	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = (data << 8) | (data >> 8);

	// this very likely doesn't respond to all the addresses, figure out which ones
	if (BIT(offset, 12))
		m_ata->write_cs1(space, (offset >> 1) & 0x07, data, mem_mask);
	else
		m_ata->write_cs0(space, (offset >> 1) & 0x07, data, mem_mask);
}

WRITE_LINE_MEMBER( a4000_state::ide_interrupt_w )
{
	m_ide_interrupt = state;
}

READ32_MEMBER( a4000_state::motherboard_r )
{
	UINT32 data = 0;

	if (offset == 0)
	{
		if (ACCESSING_BITS_0_7)
			data |= m_ramsey_config & 0xff;
		if (ACCESSING_BITS_8_15)
			data |= (m_gary_coldboot << 7 | 0x7f) << 8;
		if (ACCESSING_BITS_16_23)
			data |= (m_gary_toenb << 7 | 0x7f) << 16;
		if (ACCESSING_BITS_24_31)
			data |= (m_gary_timeout << 7 | 0x7f) << 24;
	}
	else
		data = 0xffffffff;

	logerror("motherboard_r(%06x): %08x & %08x\n", offset, data, mem_mask);

	return data;
}

WRITE32_MEMBER( a4000_state::motherboard_w )
{
	if (offset == 0)
	{
		if (ACCESSING_BITS_0_7)
			m_ramsey_config = data & 0xff;
		if (ACCESSING_BITS_8_15)
			m_gary_coldboot = BIT(data, 7);
		if (ACCESSING_BITS_16_23)
			m_gary_toenb = BIT(data, 7);
		if (ACCESSING_BITS_24_31)
			m_gary_timeout = BIT(data, 7);
	}

	logerror("motherboard_w(%06x): %08x & %08x\n", offset, data, mem_mask);
}

void cd32_state::potgo_w(UINT16 data)
{
	int i;

	m_potgo_value = m_potgo_value & 0x5500;
	m_potgo_value |= data & 0xaa00;

	for (i = 0; i < 8; i += 2)
	{
		UINT16 dir = 0x0200 << i;
		if (data & dir)
		{
			UINT16 d = 0x0100 << i;
			m_potgo_value &= ~d;
			m_potgo_value |= data & d;
		}
	}
	for (i = 0; i < 2; i++)
	{
		UINT16 p5dir = 0x0200 << (i * 4); /* output enable P5 */
		UINT16 p5dat = 0x0100 << (i * 4); /* data P5 */
		if ((m_potgo_value & p5dir) && (m_potgo_value & p5dat))
			m_cd32_shifter[i] = 8;
	}
}

static void handle_cd32_joystick_cia(running_machine &machine, UINT8 pra, UINT8 dra)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	int i;

	for (i = 0; i < 2; i++)
	{
		UINT8 but = 0x40 << i;
		UINT16 p5dir = 0x0200 << (i * 4); /* output enable P5 */
		UINT16 p5dat = 0x0100 << (i * 4); /* data P5 */

		if (!(state->m_potgo_value & p5dir) || !(state->m_potgo_value & p5dat))
		{
			if ((dra & but) && (pra & but) != state->m_oldstate[i])
			{
				if (!(pra & but))
				{
					state->m_cd32_shifter[i]--;
					if (state->m_cd32_shifter[i] < 0)
						state->m_cd32_shifter[i] = 0;
				}
			}
		}
		state->m_oldstate[i] = pra & but;
	}
}

static UINT16 handle_joystick_potgor(running_machine &machine, UINT16 potgor)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	ioport_port * player_portname[] = { state->m_p1_port, state->m_p2_port };
	int i;

	for (i = 0; i < 2; i++)
	{
		UINT16 p9dir = 0x0800 << (i * 4); /* output enable P9 */
		UINT16 p9dat = 0x0400 << (i * 4); /* data P9 */
		UINT16 p5dir = 0x0200 << (i * 4); /* output enable P5 */
		UINT16 p5dat = 0x0100 << (i * 4); /* data P5 */

		/* p5 is floating in input-mode */
		potgor &= ~p5dat;
		potgor |= state->m_potgo_value & p5dat;
		if (!(state->m_potgo_value & p9dir))
			potgor |= p9dat;
		/* P5 output and 1 -> shift register is kept reset (Blue button) */
		if ((state->m_potgo_value & p5dir) && (state->m_potgo_value & p5dat))
			state->m_cd32_shifter[i] = 8;
		/* shift at 1 == return one, >1 = return button states */
		if (state->m_cd32_shifter[i] == 0)
			potgor &= ~p9dat; /* shift at zero == return zero */
		if (state->m_cd32_shifter[i] >= 2 && ((player_portname[i])->read() & (1 << (state->m_cd32_shifter[i] - 2))))
			potgor &= ~p9dat;
	}
	return potgor;
}

CUSTOM_INPUT_MEMBER( cd32_state::cd32_input )
{
	return handle_joystick_potgor(machine(), m_potgo_value) >> 8;
}

CUSTOM_INPUT_MEMBER( cd32_state::cd32_sel_mirror_input )
{
	ioport_port* ports[2]= { m_p1_port, m_p2_port };
	UINT8 bits = ports[(int)(FPTR)param]->read();
	return (bits & 0x20)>>5;
}

WRITE8_MEMBER( cd32_state::akiko_cia_0_port_a_write )
{
	// bit 0, cd audio mute
	m_cdda->set_output_gain(0, BIT(data, 0) ? 0.0 : 1.0);

	// bit 1, power led
	output().set_led_value(0, BIT(data, 1) ? 0 : 1);

	handle_cd32_joystick_cia(machine(), data, m_cia_0->read(space, 2));
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

// The first Amiga systems used a PAL to decode chip selects, later systems
// switched to the "Gary" chip, the A3000 and A4000 used the "Super Gary"
// chip. The A600 and A1200 use the Gayle chip, while the CD32 uses its
// Akiko custom chip.

#if 0
static ADDRESS_MAP_START( a1000_overlay_map, AS_PROGRAM, 16, a1000_state )
	AM_RANGE(0x000000, 0x03ffff) AM_MIRROR(0x1c0000) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x200000, 0x201fff) AM_MIRROR(0x03e000) AM_ROM AM_REGION("bootrom", 0)
	AM_RANGE(0x280000, 0x2bffff) AM_MIRROR(0x040000) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x300000, 0x33ffff) AM_MIRROR(0x040000) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x380000, 0x381fff) AM_MIRROR(0x03e000) AM_ROM AM_REGION("bootrom", 0)
ADDRESS_MAP_END
#endif

static ADDRESS_MAP_START( a1000_overlay_map, AS_PROGRAM, 16, a1000_state )
	AM_RANGE(0x000000, 0x07ffff) AM_MIRROR(0x180000) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x200000, 0x201fff) AM_MIRROR(0x03e000) AM_ROM AM_REGION("bootrom", 0)
	AM_RANGE(0x280000, 0x2fffff) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x300000, 0x37ffff) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x380000, 0x381fff) AM_MIRROR(0x03e000) AM_ROM AM_REGION("bootrom", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( a1000_bootrom_map, AS_PROGRAM, 16, a1000_state )
	AM_RANGE(0x000000, 0x001fff) AM_MIRROR(0x3e000) AM_ROM AM_REGION("bootrom", 0) AM_WRITE(write_protect_w)
	AM_RANGE(0x040000, 0x07ffff) AM_ROMBANK("wom")
ADDRESS_MAP_END

static ADDRESS_MAP_START( a1000_mem, AS_PROGRAM, 16, a1000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_DEVICE("overlay", address_map_bank_device, amap16)
	AM_RANGE(0xa00000, 0xbfffff) AM_READWRITE(cia_r, cia_w)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xe00000, 0xe7ffff) AM_WRITENOP AM_READ(rom_mirror_r)
	AM_RANGE(0xe80000, 0xefffff) AM_NOP // autoconfig space (installed by devices)
	AM_RANGE(0xf80000, 0xfbffff) AM_DEVICE("bootrom", address_map_bank_device, amap16)
	AM_RANGE(0xfc0000, 0xffffff) AM_READWRITE_BANK("wom")
ADDRESS_MAP_END

// Gary/Super Gary/Gayle with 512KB chip RAM
static ADDRESS_MAP_START( overlay_512kb_map, AS_PROGRAM, 16, amiga_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_MIRROR(0x180000) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

// Gary/Super Gary/Gayle with 1MB chip RAM
static ADDRESS_MAP_START( overlay_1mb_map, AS_PROGRAM, 16, amiga_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x0fffff) AM_MIRROR(0x100000) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

// Gary/Super Gary/Gayle with 1MB chip RAM (32 bit system)
static ADDRESS_MAP_START( overlay_1mb_map32, AS_PROGRAM, 32, amiga_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x0fffff) AM_MIRROR(0x100000) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

// Gary/Super Gary/Gayle with 2MB chip RAM (32 bit system)
static ADDRESS_MAP_START( overlay_2mb_map16, AS_PROGRAM, 16, amiga_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

// Gary/Super Gary/Gayle with 2MB chip RAM (32 bit system)
static ADDRESS_MAP_START( overlay_2mb_map32, AS_PROGRAM, 32, amiga_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

// 512KB chip RAM, 512KB slow RAM, RTC
static ADDRESS_MAP_START( a2000_mem, AS_PROGRAM, 16, a2000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_DEVICE("overlay", address_map_bank_device, amap16)
	AM_RANGE(0xa00000, 0xbfffff) AM_READWRITE(cia_r, cia_w)
	AM_RANGE(0xc00000, 0xc7ffff) AM_RAM
	AM_RANGE(0xc80000, 0xd7ffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xd80000, 0xdbffff) AM_NOP
	AM_RANGE(0xdc0000, 0xdc7fff) AM_READWRITE(clock_r, clock_w)
	AM_RANGE(0xd80000, 0xddffff) AM_NOP
	AM_RANGE(0xde0000, 0xdeffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xdf0000, 0xdfffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xe00000, 0xe7ffff) AM_WRITENOP AM_READ(rom_mirror_r)
	AM_RANGE(0xe80000, 0xefffff) AM_NOP // autoconfig space (installed by devices)
	AM_RANGE(0xf00000, 0xf7ffff) AM_NOP // cartridge space
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

// 512KB chip RAM and no clock
static ADDRESS_MAP_START( a500_mem, AS_PROGRAM, 16, a500_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_DEVICE("overlay", address_map_bank_device, amap16)
	AM_RANGE(0xa00000, 0xbfffff) AM_READWRITE(cia_r, cia_w)
	AM_RANGE(0xc00000, 0xd7ffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xd80000, 0xddffff) AM_NOP
	AM_RANGE(0xde0000, 0xdeffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xdf0000, 0xdfffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xe00000, 0xe7ffff) AM_WRITENOP AM_READ(rom_mirror_r)
	AM_RANGE(0xe80000, 0xefffff) AM_NOP // autoconfig space (installed by devices)
	AM_RANGE(0xf00000, 0xf7ffff) AM_NOP // cartridge space
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

// 1MB chip RAM, RTC and CDTV specific hardware
static ADDRESS_MAP_START( cdtv_mem, AS_PROGRAM, 16, cdtv_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_DEVICE("overlay", address_map_bank_device, amap16)
	AM_RANGE(0xa00000, 0xbfffff) AM_READWRITE(cia_r, cia_w)
	AM_RANGE(0xc00000, 0xd7ffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xd80000, 0xdbffff) AM_NOP
	AM_RANGE(0xdc0000, 0xdc7fff) AM_READWRITE(clock_r, clock_w)
	AM_RANGE(0xdc8000, 0xdc87ff) AM_MIRROR(0x7800) AM_RAM AM_SHARE("sram")
	AM_RANGE(0xdd0000, 0xddffff) AM_NOP
	AM_RANGE(0xde0000, 0xdeffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xdf0000, 0xdfffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xe00000, 0xe3ffff) AM_MIRROR(0x40000) AM_RAM AM_SHARE("memcard")
	AM_RANGE(0xe80000, 0xefffff) AM_NOP // autoconfig space (installed by devices)
	AM_RANGE(0xf00000, 0xf3ffff) AM_MIRROR(0x40000) AM_ROM AM_REGION("cdrom", 0)
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cdtv_rc_mem, AS_PROGRAM, 8, cdtv_state )
	AM_RANGE(0x0800, 0x0fff) AM_ROM AM_REGION("rcmcu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( a3000_mem, AS_PROGRAM, 32, a3000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x001fffff) AM_DEVICE("overlay", address_map_bank_device, amap32)
	AM_RANGE(0x00b80000, 0x00bfffff) AM_READWRITE16(cia_r, cia_w, 0xffffffff)
	AM_RANGE(0x00c00000, 0x00cfffff) AM_READWRITE16(custom_chip_r, custom_chip_w, 0xffffffff)
	AM_RANGE(0x00d00000, 0x00dbffff) AM_NOP
	AM_RANGE(0x00dc0000, 0x00dcffff) AM_DEVREADWRITE8("rtc", rp5c01_device, read, write, 0x000000ff)
	AM_RANGE(0x00dd0000, 0x00ddffff) AM_READWRITE(scsi_r, scsi_w)
	AM_RANGE(0x00de0000, 0x00deffff) AM_READWRITE(motherboard_r, motherboard_w)
	AM_RANGE(0x00df0000, 0x00dfffff) AM_READWRITE16(custom_chip_r, custom_chip_w, 0xffffffff)
	AM_RANGE(0x00e80000, 0x00efffff) AM_NOP // autoconfig space (installed by devices)
	AM_RANGE(0x00f00000, 0x00f7ffff) AM_NOP // cartridge space
	AM_RANGE(0x00f80000, 0x00ffffff) AM_ROM AM_REGION("kickstart", 0)
	AM_RANGE(0x07f00000, 0x07ffffff) AM_RAM // motherboard ram (up to 16mb), grows downward
ADDRESS_MAP_END

// 1MB chip RAM and RTC
static ADDRESS_MAP_START( a500p_mem, AS_PROGRAM, 16, a500p_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_DEVICE("overlay", address_map_bank_device, amap16)
	AM_RANGE(0xa00000, 0xbfffff) AM_READWRITE(cia_r, cia_w)
	AM_RANGE(0xc00000, 0xc7ffff) AM_RAM
	AM_RANGE(0xc80000, 0xd7ffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xd80000, 0xdbffff) AM_NOP
	AM_RANGE(0xdc0000, 0xdc7fff) AM_READWRITE(clock_r, clock_w)
	AM_RANGE(0xd80000, 0xddffff) AM_NOP
	AM_RANGE(0xde0000, 0xdeffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xdf0000, 0xdfffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xe00000, 0xe7ffff) AM_WRITENOP AM_READ(rom_mirror_r)
	AM_RANGE(0xe80000, 0xefffff) AM_NOP // autoconfig space (installed by devices)
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

// 1MB chip RAM, IDE and PCMCIA
static ADDRESS_MAP_START( a600_mem, AS_PROGRAM, 16, a600_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_DEVICE("overlay", address_map_bank_device, amap16)
	AM_RANGE(0x200000, 0xa7ffff) AM_NOP
	AM_RANGE(0xa80000, 0xafffff) AM_WRITENOP AM_READ(rom_mirror_r)
	AM_RANGE(0xb00000, 0xb7ffff) AM_WRITENOP AM_READ(rom_mirror_r)
	AM_RANGE(0xb80000, 0xbeffff) AM_NOP // reserved (cdtv)
	AM_RANGE(0xbf0000, 0xbfffff) AM_READWRITE(cia_r, gayle_cia_w)
	AM_RANGE(0xc00000, 0xd7ffff) AM_NOP // slow mem
	AM_RANGE(0xd80000, 0xd8ffff) AM_NOP // spare chip select
	AM_RANGE(0xd90000, 0xd9ffff) AM_NOP // arcnet chip select
	AM_RANGE(0xda0000, 0xdaffff) AM_DEVREADWRITE("gayle", gayle_device, gayle_r, gayle_w)
	AM_RANGE(0xdb0000, 0xdbffff) AM_NOP // reserved (external ide)
	AM_RANGE(0xdc0000, 0xdcffff) AM_NOP // rtc
	AM_RANGE(0xdd0000, 0xddffff) AM_NOP // reserved (dma controller)
	AM_RANGE(0xde0000, 0xdeffff) AM_DEVREADWRITE("gayle", gayle_device, gayle_id_r, gayle_id_w)
	AM_RANGE(0xdf0000, 0xdfffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xe00000, 0xe7ffff) AM_WRITENOP AM_READ(rom_mirror_r)
	AM_RANGE(0xe80000, 0xefffff) AM_NOP // autoconfig space (installed by devices)
	AM_RANGE(0xf00000, 0xf7ffff) AM_NOP // cartridge space
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

// 2MB chip RAM, IDE and PCMCIA
static ADDRESS_MAP_START( a1200_mem, AS_PROGRAM, 32, a1200_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_DEVICE("overlay", address_map_bank_device, amap32)
	AM_RANGE(0x200000, 0xa7ffff) AM_NOP
	AM_RANGE(0xa80000, 0xafffff) AM_WRITENOP AM_READ(rom_mirror32_r)
	AM_RANGE(0xb00000, 0xb7ffff) AM_WRITENOP AM_READ(rom_mirror32_r)
	AM_RANGE(0xb80000, 0xbeffff) AM_NOP // reserved (cdtv)
	AM_RANGE(0xbf0000, 0xbfffff) AM_READWRITE16(cia_r, gayle_cia_w, 0xffffffff)
	AM_RANGE(0xc00000, 0xd7ffff) AM_NOP // slow mem
	AM_RANGE(0xd80000, 0xd8ffff) AM_NOP // spare chip select
	AM_RANGE(0xd90000, 0xd9ffff) AM_NOP // arcnet chip select
	AM_RANGE(0xda0000, 0xdaffff) AM_DEVREADWRITE16("gayle", gayle_device, gayle_r, gayle_w, 0xffffffff)
	AM_RANGE(0xdb0000, 0xdbffff) AM_NOP // reserved (external ide)
	AM_RANGE(0xdc0000, 0xdcffff) AM_NOP // rtc
	AM_RANGE(0xdd0000, 0xddffff) AM_NOP // reserved (dma controller)
	AM_RANGE(0xde0000, 0xdeffff) AM_DEVREADWRITE16("gayle", gayle_device, gayle_id_r, gayle_id_w, 0xffffffff)
	AM_RANGE(0xdf0000, 0xdfffff) AM_READWRITE16(custom_chip_r, custom_chip_w, 0xffffffff)
	AM_RANGE(0xe00000, 0xe7ffff) AM_WRITENOP AM_READ(rom_mirror32_r)
	AM_RANGE(0xe80000, 0xefffff) AM_NOP // autoconfig space (installed by devices)
	AM_RANGE(0xf00000, 0xf7ffff) AM_NOP // cartridge space
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

// 2MB chip RAM, 4 MB fast RAM, RTC and IDE
static ADDRESS_MAP_START( a4000_mem, AS_PROGRAM, 32, a4000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x001fffff) AM_DEVICE("overlay", address_map_bank_device, amap32)
	AM_RANGE(0x00200000, 0x009fffff) AM_NOP // zorro2 expansion
	AM_RANGE(0x00a00000, 0x00b7ffff) AM_NOP
	AM_RANGE(0x00b80000, 0x00beffff) AM_NOP
	AM_RANGE(0x00bf0000, 0x00bfffff) AM_READWRITE16(cia_r, cia_w, 0xffffffff)
	AM_RANGE(0x00c00000, 0x00cfffff) AM_READWRITE16(custom_chip_r, custom_chip_w, 0xffffffff)
	AM_RANGE(0x00d00000, 0x00d9ffff) AM_NOP
	AM_RANGE(0x00da0000, 0x00dbffff) AM_NOP
	AM_RANGE(0x00dc0000, 0x00dcffff) AM_DEVREADWRITE8("rtc", rp5c01_device, read, write, 0x000000ff)
	AM_RANGE(0x00dd0000, 0x00dd0fff) AM_NOP
	AM_RANGE(0x00dd1000, 0x00dd3fff) AM_READWRITE16(ide_r, ide_w, 0xffffffff)
	AM_RANGE(0x00dd4000, 0x00ddffff) AM_NOP
	AM_RANGE(0x00de0000, 0x00deffff) AM_READWRITE(motherboard_r, motherboard_w)
	AM_RANGE(0x00df0000, 0x00dfffff) AM_READWRITE16(custom_chip_r, custom_chip_w, 0xffffffff)
	AM_RANGE(0x00e00000, 0x00e7ffff) AM_WRITENOP AM_READ(rom_mirror32_r)
	AM_RANGE(0x00e80000, 0x00efffff) AM_NOP // zorro2 autoconfig space (installed by devices)
	AM_RANGE(0x00f00000, 0x00f7ffff) AM_NOP // cartridge space
	AM_RANGE(0x00f80000, 0x00ffffff) AM_ROM AM_REGION("kickstart", 0)
	AM_RANGE(0x01000000, 0x017fffff) AM_NOP // reserved (8 mb chip ram)
	AM_RANGE(0x01800000, 0x06ffffff) AM_NOP // reserved (motherboard fast ram expansion)
	AM_RANGE(0x07000000, 0x07bfffff) AM_NOP // motherboard ram
	AM_RANGE(0x07c00000, 0x07ffffff) AM_RAM // motherboard ram (up to 16mb), grows downward
ADDRESS_MAP_END

// 2MB chip RAM, 2 MB fast RAM, RTC and IDE
static ADDRESS_MAP_START( a400030_mem, AS_PROGRAM, 32, a4000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_IMPORT_FROM(a4000_mem)
	AM_RANGE(0x07000000, 0x07dfffff) AM_NOP // motherboard ram
	AM_RANGE(0x07e00000, 0x07ffffff) AM_RAM // motherboard ram (up to 16mb), grows downward
ADDRESS_MAP_END

// 2MB chip RAM and CD-ROM
static ADDRESS_MAP_START( cd32_mem, AS_PROGRAM, 32, cd32_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_DEVICE("overlay", address_map_bank_device, amap32)
	AM_RANGE(0xb80000, 0xb8003f) AM_DEVREADWRITE("akiko", akiko_device, read, write)
	AM_RANGE(0xbf0000, 0xbfffff) AM_READWRITE16(cia_r, gayle_cia_w, 0xffffffff)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE16(custom_chip_r, custom_chip_w, 0xffffffff)
	AM_RANGE(0xe00000, 0xe7ffff) AM_ROM AM_REGION("kickstart", 0x80000)
	AM_RANGE(0xa00000, 0xf7ffff) AM_NOP
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

// 2 MB chip RAM, IDE, RTC and SCSI
static ADDRESS_MAP_START( a4000t_mem, AS_PROGRAM, 32, a4000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_IMPORT_FROM(a4000_mem)
	AM_RANGE(0x00dd0000, 0x00dd0fff) AM_READWRITE(scsi_r, scsi_w)
ADDRESS_MAP_END


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( amiga )
	PORT_START("input")
	PORT_CONFNAME(0x10, 0x00, "Game Port 0 Device")
	PORT_CONFSETTING(0x00, "Mouse")
	PORT_CONFSETTING(0x10, DEF_STR(Joystick))
	PORT_CONFNAME(0x20, 0x20, "Game Port 1 Device")
	PORT_CONFSETTING(0x00, "Mouse")
	PORT_CONFSETTING(0x20, DEF_STR(Joystick) )

	PORT_START("cia_0_port_a")
	PORT_BIT(0x3f, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, amiga_state, floppy_drive_status, 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_BUTTON1) PORT_PLAYER(2)

	PORT_START("joy_0_dat")
	PORT_BIT(0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, amiga_state, amiga_joystick_convert, 0)
	PORT_BIT(0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("joy_1_dat")
	PORT_BIT(0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, amiga_state, amiga_joystick_convert, 1)
	PORT_BIT(0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("potgo")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0xaaff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("p1_joy")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)

	PORT_START("p2_joy")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)

	PORT_START("p1_mouse_x")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START("p1_mouse_y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START("p2_mouse_x")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(2)

	PORT_START("p2_mouse_y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(2)
INPUT_PORTS_END

INPUT_PORTS_START( cd32 )
	PORT_INCLUDE(amiga)

	PORT_MODIFY("cia_0_port_a")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_SPECIAL )
	// this is the regular port for reading a single button joystick on the Amiga, many CD32 games require this to mirror the pad start button!
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, cd32_state, cd32_sel_mirror_input, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, cd32_state, cd32_sel_mirror_input, 1)

	PORT_MODIFY("joy_0_dat")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, amiga_state, amiga_joystick_convert, 0)
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("joy_1_dat")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, amiga_state, amiga_joystick_convert, 1)
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("potgo")
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, cd32_state, cd32_input, 0)
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )

	// CD32 '11' button pad (4 dpad directions + 7 buttons), not read directly
	PORT_START("p1_cd32_buttons")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Play/Pause")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Left Trigger/Rewind")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Right Trigger/Forward")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Green/Stop")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Yellow/Shuffle")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Red/Select")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Blue/Loop")

	// CD32 '11' button pad (4 dpad directions + 7 buttons), not read directly
	PORT_START("p2_cd32_buttons")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Play/Pause")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Left Trigger/Rewind")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Right Trigger/Forward")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Green/Stop")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Yellow/Shuffle")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Red/Select")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Blue/Loop")
INPUT_PORTS_END


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static SLOT_INTERFACE_START( amiga_floppies )
	SLOT_INTERFACE("35dd", FLOPPY_35_DD)
SLOT_INTERFACE_END

// basic elements common to all amigas
static MACHINE_CONFIG_START( amiga_base, amiga_state )
	// video
	MCFG_FRAGMENT_ADD(pal_video)

	MCFG_PALETTE_ADD("palette", 4096)
	MCFG_PALETTE_INIT_OWNER(amiga_state, amiga)

	MCFG_VIDEO_START_OVERRIDE(amiga_state, amiga)

	// cia
	MCFG_DEVICE_ADD("cia_0", MOS8520, amiga_state::CLK_E_PAL)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(amiga_state, cia_0_irq))
	MCFG_MOS6526_PA_INPUT_CALLBACK(IOPORT("cia_0_port_a"))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(amiga_state, cia_0_port_a_write))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_MOS6526_PC_CALLBACK(DEVWRITELINE("centronics", centronics_device, write_strobe))
	MCFG_MOS6526_SP_CALLBACK(DEVWRITELINE("kbd", amigakbd_device, kdat_w))

	MCFG_DEVICE_ADD("cia_1", MOS8520, amiga_state::CLK_E_PAL)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(amiga_state, cia_1_irq))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(amiga_state, cia_1_port_a_read))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(amiga_state, cia_1_port_a_write))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(DEVWRITE8("fdc", amiga_fdc, ciaaprb_w))

	// audio
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("amiga", AMIGA, amiga_state::CLK_C1_PAL)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.50)
	MCFG_SOUND_ROUTE(3, "lspeaker", 0.50)

	// floppy drives
	MCFG_DEVICE_ADD("fdc", AMIGA_FDC, amiga_state::CLK_7M_PAL)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", amiga_floppies, "35dd", amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", amiga_floppies, nullptr, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", amiga_floppies, nullptr, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", amiga_floppies, nullptr, amiga_fdc::floppy_formats)

	// rs232
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(amiga_state, rs232_rx_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE(amiga_state, rs232_dcd_w))
	MCFG_RS232_DSR_HANDLER(WRITELINE(amiga_state, rs232_dsr_w))
	MCFG_RS232_RI_HANDLER(WRITELINE(amiga_state, rs232_ri_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE(amiga_state, rs232_cts_w))

	// centronics
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(amiga_state, centronics_ack_w))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(amiga_state, centronics_busy_w))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(amiga_state, centronics_perror_w))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(amiga_state, centronics_select_w))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	// keyboard
	MCFG_DEVICE_ADD("kbd", AMIGAKBD, 0)
	MCFG_AMIGA_KEYBOARD_KCLK_CALLBACK(DEVWRITELINE("cia_0", mos8520_device, cnt_w))
	MCFG_AMIGA_KEYBOARD_KDAT_CALLBACK(DEVWRITELINE("cia_0", mos8520_device, sp_w))
	MCFG_AMIGA_KEYBOARD_KRST_CALLBACK(WRITELINE(amiga_state, kbreset_w))

	// software
	MCFG_SOFTWARE_LIST_ADD("wb_list", "amiga_workbench")
	MCFG_SOFTWARE_LIST_ADD("hardware_list", "amiga_hardware")
	MCFG_SOFTWARE_LIST_ADD("apps_list", "amiga_apps")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a1000, amiga_base, a1000_state )
	// main cpu
	MCFG_CPU_ADD("maincpu", M68000, amiga_state::CLK_7M_PAL)
	MCFG_CPU_PROGRAM_MAP(a1000_mem)

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(a1000_overlay_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	MCFG_DEVICE_ADD("bootrom", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(a1000_bootrom_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(19)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x40000)

	MCFG_SOFTWARE_LIST_ADD("a1000_list", "amiga_a1000")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a1000n, a1000, a1000_state )
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_NTSC)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a2000, amiga_base, a2000_state )
	// main cpu
	MCFG_CPU_ADD("maincpu", M68000, amiga_state::CLK_7M_PAL)
	MCFG_CPU_PROGRAM_MAP(a2000_mem)

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(overlay_512kb_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	// real-time clock
	MCFG_DEVICE_ADD("u65", MSM6242, XTAL_32_768kHz)

	// cpu slot
	MCFG_EXPANSION_SLOT_ADD("maincpu", a2000_expansion_cards, nullptr)

	// zorro slots
	MCFG_ZORRO2_ADD("maincpu")
	MCFG_ZORRO2_INT2_HANDLER(WRITELINE(a2000_state, zorro2_int2_w))
	MCFG_ZORRO2_INT6_HANDLER(WRITELINE(a2000_state, zorro2_int6_w))
	MCFG_ZORRO2_SLOT_ADD("zorro1", zorro2_cards, nullptr)
	MCFG_ZORRO2_SLOT_ADD("zorro2", zorro2_cards, nullptr)
	MCFG_ZORRO2_SLOT_ADD("zorro3", zorro2_cards, nullptr)
	MCFG_ZORRO2_SLOT_ADD("zorro4", zorro2_cards, nullptr)
	MCFG_ZORRO2_SLOT_ADD("zorro5", zorro2_cards, nullptr)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a2000n, a2000, a2000_state )
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_NTSC)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a500, amiga_base, a500_state )
	// main cpu
	MCFG_CPU_ADD("maincpu", M68000, amiga_state::CLK_7M_PAL)
	MCFG_CPU_PROGRAM_MAP(a500_mem)

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	//MCFG_DEVICE_PROGRAM_MAP(overlay_512kb_map)
	MCFG_DEVICE_PROGRAM_MAP(overlay_1mb_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	// cpu slot
	MCFG_EXPANSION_SLOT_ADD("maincpu", a500_expansion_cards, nullptr)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a500n, a500, a500_state )
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_NTSC)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( cdtv, amiga_base, cdtv_state )
	// main cpu
	MCFG_CPU_ADD("maincpu", M68000, amiga_state::CLK_7M_PAL)
	MCFG_CPU_PROGRAM_MAP(cdtv_mem)

	// remote control input converter
	MCFG_CPU_ADD("u75", M6502, XTAL_3MHz)
	MCFG_CPU_PROGRAM_MAP(cdtv_rc_mem)
	MCFG_DEVICE_DISABLE()

	// lcd controller
#if 0
	MCFG_CPU_ADD("u62", LC6554, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(lcd_mem)
#endif

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(overlay_1mb_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	// standard sram
	MCFG_NVRAM_ADD_0FILL("sram")

	// 256kb memory card
	MCFG_NVRAM_ADD_0FILL("memcard")

	// real-time clock
	MCFG_DEVICE_ADD("u61", MSM6242, XTAL_32_768kHz)

	// cd-rom controller
	MCFG_DMAC_ADD("u36", amiga_state::CLK_7M_PAL)
	MCFG_DMAC_SCSI_READ_HANDLER(READ8(cdtv_state, dmac_scsi_data_read))
	MCFG_DMAC_SCSI_WRITE_HANDLER(WRITE8(cdtv_state, dmac_scsi_data_write))
	MCFG_DMAC_IO_READ_HANDLER(READ8(cdtv_state, dmac_io_read))
	MCFG_DMAC_IO_WRITE_HANDLER(WRITE8(cdtv_state, dmac_io_write))
	MCFG_DMAC_INT_HANDLER(WRITELINE(cdtv_state, dmac_int_w))

	MCFG_DEVICE_ADD("u32", TPI6525, 0)
	MCFG_TPI6525_OUT_IRQ_CB(WRITELINE(cdtv_state, tpi_int_w))
	MCFG_TPI6525_OUT_PB_CB(WRITE8(cdtv_state, tpi_port_b_write))

	// cd-rom
	MCFG_CR511B_ADD("cdrom")
	MCFG_CR511B_SCOR_HANDLER(DEVWRITELINE("u32", tpi6525_device, i1_w)) MCFG_DEVCB_INVERT
	MCFG_CR511B_STCH_HANDLER(DEVWRITELINE("u32", tpi6525_device, i2_w)) MCFG_DEVCB_INVERT
	MCFG_CR511B_STEN_HANDLER(DEVWRITELINE("u32", tpi6525_device, i3_w))
	MCFG_CR511B_XAEN_HANDLER(DEVWRITELINE("u32", tpi6525_device, pb2_w))
	MCFG_CR511B_DRQ_HANDLER(DEVWRITELINE("u36", dmac_device, xdreq_w))
	MCFG_CR511B_DTEN_HANDLER(DEVWRITELINE("u36", dmac_device, xdreq_w))

	MCFG_SOFTWARE_LIST_ADD("cd_list", "cdtv")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( cdtvn, cdtv, cdtv_state )
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_NTSC)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("u36")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a3000, amiga_base, a3000_state )
	// main cpu
	MCFG_CPU_ADD("maincpu", M68030, XTAL_32MHz / 2)
	MCFG_CPU_PROGRAM_MAP(a3000_mem)

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(overlay_1mb_map32)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(32)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	// real-time clock
	MCFG_DEVICE_ADD("rtc", RP5C01, XTAL_32_768kHz)

	// todo: zorro3 slots, super dmac, scsi

	MCFG_SOFTWARE_LIST_ADD("a3000_list", "amiga_a3000")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a3000n, a3000, a3000_state )
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a500p, amiga_base, a500p_state )
	// main cpu
	MCFG_CPU_ADD("maincpu", M68000, amiga_state::CLK_7M_PAL)
	MCFG_CPU_PROGRAM_MAP(a500p_mem)

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(overlay_1mb_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	// real-time clock
	MCFG_DEVICE_ADD("u9", MSM6242, XTAL_32_768kHz)

	// cpu slot
	MCFG_EXPANSION_SLOT_ADD("maincpu", a500_expansion_cards, nullptr)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a500pn, a500p, a500p_state )
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_NTSC)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a600, amiga_base, a600_state )
	// main cpu
	MCFG_CPU_ADD("maincpu", M68000, amiga_state::CLK_7M_PAL)
	MCFG_CPU_PROGRAM_MAP(a600_mem)

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(overlay_2mb_map16)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	MCFG_GAYLE_ADD("gayle", amiga_state::CLK_28M_PAL / 2, a600_state::GAYLE_ID)
	MCFG_GAYLE_INT2_HANDLER(WRITELINE(a600_state, gayle_int2_w))
	MCFG_GAYLE_CS0_READ_HANDLER(DEVREAD16("ata", ata_interface_device, read_cs0))
	MCFG_GAYLE_CS0_WRITE_HANDLER(DEVWRITE16("ata", ata_interface_device, write_cs0))
	MCFG_GAYLE_CS1_READ_HANDLER(DEVREAD16("ata", ata_interface_device, read_cs1))
	MCFG_GAYLE_CS1_WRITE_HANDLER(DEVWRITE16("ata", ata_interface_device, write_cs1))

	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", nullptr, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("gayle", gayle_device, ide_interrupt_w))

	// todo: pcmcia
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a600n, a600, a600_state )
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
	MCFG_DEVICE_MODIFY("gayle")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_28M_NTSC / 2)
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_NTSC)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a1200, amiga_base, a1200_state )
	// main cpu
	MCFG_CPU_ADD("maincpu", M68EC020, amiga_state::CLK_28M_PAL / 2)
	MCFG_CPU_PROGRAM_MAP(a1200_mem)

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(overlay_2mb_map32)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(32)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	MCFG_DEVICE_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga_aga)
	MCFG_SCREEN_NO_PALETTE

	MCFG_DEVICE_REMOVE("palette")

	MCFG_VIDEO_START_OVERRIDE(amiga_state, amiga_aga)

	MCFG_GAYLE_ADD("gayle", amiga_state::CLK_28M_PAL / 2, a1200_state::GAYLE_ID)
	MCFG_GAYLE_INT2_HANDLER(WRITELINE(a1200_state, gayle_int2_w))
	MCFG_GAYLE_CS0_READ_HANDLER(DEVREAD16("ata", ata_interface_device, read_cs0))
	MCFG_GAYLE_CS0_WRITE_HANDLER(DEVWRITE16("ata", ata_interface_device, write_cs0))
	MCFG_GAYLE_CS1_READ_HANDLER(DEVREAD16("ata", ata_interface_device, read_cs1))
	MCFG_GAYLE_CS1_WRITE_HANDLER(DEVWRITE16("ata", ata_interface_device, write_cs1))

	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", nullptr, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("gayle", gayle_device, ide_interrupt_w))

	// todo: pcmcia
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a1200n, a1200, a1200_state )
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_28M_NTSC / 2)
	MCFG_DEVICE_MODIFY("gayle")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_28M_NTSC / 2)
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga_aga)
	MCFG_SCREEN_NO_PALETTE
	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_NTSC)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a4000, amiga_base, a4000_state )
	// main cpu
	MCFG_CPU_ADD("maincpu", M68040, XTAL_50MHz / 2)
	MCFG_CPU_PROGRAM_MAP(a4000_mem)

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(overlay_2mb_map32)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(32)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	MCFG_DEVICE_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga_aga)
	MCFG_SCREEN_NO_PALETTE

	MCFG_DEVICE_REMOVE("palette")

	MCFG_VIDEO_START_OVERRIDE(amiga_state, amiga_aga)

	// real-time clock
	MCFG_DEVICE_ADD("rtc", RP5C01, XTAL_32_768kHz)

	// ide
	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", nullptr, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(a4000_state, ide_interrupt_w))

	// todo: zorro3
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a4000n, a4000, a4000_state )
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga_aga)
	MCFG_SCREEN_NO_PALETTE
	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_NTSC)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a400030, a4000, a4000_state )
	// main cpu
	MCFG_DEVICE_REMOVE("maincpu")
	MCFG_CPU_ADD("maincpu", M68EC030, XTAL_50MHz / 2)
	MCFG_CPU_PROGRAM_MAP(a400030_mem)

	// todo: ide
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a400030n, a400030, a4000_state )
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga_aga)
	MCFG_SCREEN_NO_PALETTE
	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_NTSC)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( cd32, amiga_base, cd32_state )
	// main cpu
	MCFG_CPU_ADD("maincpu", M68EC020, amiga_state::CLK_28M_PAL / 2)
	MCFG_CPU_PROGRAM_MAP(cd32_mem)

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(overlay_2mb_map32)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(32)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	MCFG_I2CMEM_ADD("i2cmem")
	MCFG_I2CMEM_PAGE_SIZE(16)
	MCFG_I2CMEM_DATA_SIZE(1024)

	MCFG_AKIKO_ADD("akiko", "maincpu")
	MCFG_AKIKO_SCL_HANDLER(DEVWRITELINE("i2cmem", i2cmem_device, write_scl))
	MCFG_AKIKO_SDA_READ_HANDLER(DEVREADLINE("i2cmem", i2cmem_device, read_sda))
	MCFG_AKIKO_SDA_WRITE_HANDLER(DEVWRITELINE("i2cmem", i2cmem_device, write_sda))

	MCFG_DEVICE_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga_aga)
	MCFG_SCREEN_NO_PALETTE

	MCFG_DEVICE_REMOVE("palette")

	MCFG_VIDEO_START_OVERRIDE(amiga_state, amiga_aga)

	MCFG_SOUND_ADD("cdda", CDDA, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(cd32_state, akiko_cia_0_port_a_write))
	MCFG_MOS6526_SP_CALLBACK(NULL)

	MCFG_CDROM_ADD("cdrom")
	MCFG_CDROM_INTERFACE("cd32_cdrom")
	MCFG_SOFTWARE_LIST_ADD("cd_list", "cd32")
	MCFG_DEVICE_REMOVE("kbd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( cd32n, cd32, cd32_state )
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_28M_NTSC / 2)
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga_aga)
	MCFG_SCREEN_NO_PALETTE
	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_NTSC)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a4000t, a4000, a4000_state )
	// main cpu
	MCFG_DEVICE_REMOVE("maincpu")
	MCFG_CPU_ADD("maincpu", M68040, XTAL_50MHz / 2)
	MCFG_CPU_PROGRAM_MAP(a4000t_mem)

	// todo: ide, zorro3, scsi, super dmac
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( a4000tn, a4000, a4000_state )
	MCFG_DEVICE_REMOVE("screen")
	MCFG_FRAGMENT_ADD(ntsc_video)
	MCFG_DEVICE_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga_aga)
	MCFG_SCREEN_NO_PALETTE
	MCFG_DEVICE_MODIFY("amiga")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_C1_NTSC)
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_E_NTSC)
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(amiga_state::CLK_7M_NTSC)
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// Amiga 1000
//
// Shipped with a small bootrom to load kickstart from disk because the
// Kickstart wasn't finished in time.

ROM_START( a1000 )
	ROM_REGION16_BE(0x2000, "bootrom", 0)
	ROM_LOAD16_BYTE("252179-01.u5n", 0x0000, 0x1000, CRC(42553bc4) SHA1(8855a97f7a44e3f62d1c88d938fee1f4c606af5b))
	ROM_LOAD16_BYTE("252180-01.u5p", 0x0001, 0x1000, CRC(8e5b9a37) SHA1(d10f1564b99f5ffe108fa042362e877f569de2c3))

	// PALs, all of type PAL16L8
	ROM_REGION(0x104, "dpalen", 0)
	ROM_LOAD("252128-01.u4t", 0, 0x104, CRC(28209ff2) SHA1(20c03b6b8e7254231f4b3014dc2c4d9274d469d2))
	ROM_REGION(0x104, "dpalcas", 0)
	ROM_LOAD("252128-02.u6p", 0, 0x104, CRC(b928efd2) SHA1(430794a544d9160e1b786e97e0dec5f25502a00a))
	ROM_REGION(0x104, "daugen", 0)
	ROM_LOAD("252128-03.u4s", 0, 0x104, CRC(87747964) SHA1(00d72ec707c582363525fde56176973c7327b1d7))
	ROM_REGION(0x104, "daugcas", 0)
	ROM_LOAD("252128-04.u6n", 0, 0x104, CRC(f903adb4) SHA1(4c8fb696fd1aaf9bb8c9efddeac24bb36f119c5f))
ROM_END

#define rom_a1000n  rom_a1000

// Amiga 2000 and Amiga 500
//
// Early models shipped with Kickstart 1.2, later versions with Kickstart 1.3.
// Kickstart 2.04 and 3.1 upgrade available. The Kickstart 2.04 upgrade was also
// available as a special version that included a jumper wire, which was needed
// for some early motherboard revisions (P/N: 363968-01).

ROM_START( a2000 )
	ROM_REGION16_BE(0x80000, "kickstart", 0)
	ROM_DEFAULT_BIOS("kick13")
	ROM_SYSTEM_BIOS(0, "kick12",  "Kickstart 1.2 (33.180)")
	ROMX_LOAD("315093-01.u2", 0x00000, 0x40000, CRC(a6ce1636) SHA1(11f9e62cf299f72184835b7b2a70a16333fc0d88), ROM_GROUPWORD | ROM_BIOS(1))
	ROM_COPY("kickstart", 0x00000, 0x40000, 0x40000)
	ROM_SYSTEM_BIOS(1, "kick13",  "Kickstart 1.3 (34.5)")
	ROMX_LOAD("315093-02.u2", 0x00000, 0x40000, CRC(c4f0f55f) SHA1(891e9a547772fe0c6c19b610baf8bc4ea7fcb785), ROM_GROUPWORD | ROM_BIOS(2))
	ROM_COPY("kickstart", 0x00000, 0x40000, 0x40000)
	ROM_SYSTEM_BIOS(2, "kick204", "Kickstart 2.04 (37.175)")
	ROMX_LOAD("390979-01.u2", 0x00000, 0x80000, CRC(c3bdb240) SHA1(c5839f5cb98a7a8947065c3ed2f14f5f42e334a1), ROM_GROUPWORD | ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "kick31",  "Kickstart 3.1 (40.63)")
	ROMX_LOAD("kick40063.u2", 0x00000, 0x80000, CRC(fc24ae0d) SHA1(3b7f1493b27e212830f989f26ca76c02049f09ca), ROM_GROUPWORD | ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "logica2", "Logica Diagnostic 2.0")
	ROMX_LOAD("logica2.u2",   0x00000, 0x80000, CRC(8484f426) SHA1(ba10d16166b2e2d6177c979c99edf8462b21651e), ROM_GROUPWORD | ROM_BIOS(5))
ROM_END

// Amiga 2000CR chip location: U500
#define rom_a2000n  rom_a2000

// Amiga 500 chip location: U6
#define rom_a500   rom_a2000
#define rom_a500n  rom_a2000

// Amiga 500+
//
// Shipped with Kickstart 2.04. Kickstart 3.1 upgrade available.

ROM_START( a500p )
	ROM_REGION16_BE(0x80000, "kickstart", 0)
	ROM_DEFAULT_BIOS("kick204")
	ROM_SYSTEM_BIOS(0, "kick204", "Kickstart 2.04 (37.175)")
	ROMX_LOAD("390979-01.u6", 0x00000, 0x80000, CRC(c3bdb240) SHA1(c5839f5cb98a7a8947065c3ed2f14f5f42e334a1), ROM_GROUPWORD | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "kick31",  "Kickstart 3.1 (40.63)")
	ROMX_LOAD("kick40063.u6", 0x00000, 0x80000, CRC(fc24ae0d) SHA1(3b7f1493b27e212830f989f26ca76c02049f09ca), ROM_GROUPWORD | ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "logica2", "Logica Diagnostic 2.0")
	ROMX_LOAD("logica2.u6",   0x00000, 0x80000, CRC(8484f426) SHA1(ba10d16166b2e2d6177c979c99edf8462b21651e), ROM_GROUPWORD | ROM_BIOS(3))
ROM_END

#define rom_a500pn  rom_a500p

// Commodore CDTV
//
// Shipped with a standard Kickstart 1.3 and the needed additional drivers
// in two extra chips.

ROM_START( cdtv )
	// cd-rom driver
	ROM_REGION16_BE(0x40000, "cdrom", 0)
	ROM_LOAD16_BYTE("391008-01.u34", 0x00000, 0x20000, CRC(791cb14b) SHA1(277a1778924496353ffe56be68063d2a334360e4))
	ROM_LOAD16_BYTE("391009-01.u35", 0x00001, 0x20000, CRC(accbbc2e) SHA1(41b06d1679c6e6933c3378b7626025f7641ebc5c))

	// standard amiga kickstart 1.3
	ROM_REGION16_BE(0x80000, "kickstart", 0)
	ROMX_LOAD("315093-02.u13", 0x00000, 0x40000, CRC(c4f0f55f) SHA1(891e9a547772fe0c6c19b610baf8bc4ea7fcb785), ROM_GROUPWORD)
	ROM_COPY("kickstart", 0x00000, 0x40000, 0x40000)

	// remote control input converter, mos 6500/1 mcu
	ROM_REGION(0x800, "rcmcu", 0)
	ROM_LOAD("252609-02.u75", 0x000, 0x800, NO_DUMP)

	// lcd controller, sanyo lc6554h
	ROM_REGION(0x1000, "lcd", 0)
	ROM_LOAD("252608-01.u62", 0x0000, 0x1000, NO_DUMP)
ROM_END

#define rom_cdtvn  rom_cdtv

// Amiga 3000
//
// Early models have a special version of Kickstart 1.4/2.0 that boots
// Kickstart 1.3 or 2.0 from hard disk or floppy. Later versions have
// Kickstart 2.04 installed as ROM. Upgrade available for
// Kickstart 3.1.

ROM_START( a3000 )
	ROM_REGION32_BE(0x80000, "kickstart", 0)
	ROM_DEFAULT_BIOS("kick20")
	ROM_SYSTEM_BIOS(0, "kick14", "Kickstart 1.4 (3312.20085?)")
	ROMX_LOAD("390629-01.u182", 0x00000, 0x40000, NO_DUMP, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROMX_LOAD("390630-01.u183", 0x00002, 0x40000, NO_DUMP, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "kick20", "Kickstart 2.0 (36.16)")
	// COPYRIGHT 1990 CAI // ALL RIGHTS RESERVED // ALPHA 5 ROM 0 CS=9713
	ROMX_LOAD("390629-02.u182", 0x00000, 0x40000, CRC(58327536) SHA1(d1713d7f31474a5948e6d488e33686061cf3d1e2), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	// COPYRIGHT 1990 CAI // ALL RIGHTS RESERVED // ALPHA 5 ROM 1 CS=9B21
	ROMX_LOAD("390630-02.u183", 0x00002, 0x40000, CRC(fe2f7fb9) SHA1(c05c9c52d014c66f9019152b3f2a2adc2c678794), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "kick204", "Kickstart 2.04 (37.175)")
	ROMX_LOAD("390629-03.u182", 0x00000, 0x40000, CRC(a245dbdf) SHA1(83bab8e95d378b55b0c6ae6561385a96f638598f), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(3))
	ROMX_LOAD("390630-03.u183", 0x00002, 0x40000, CRC(7db1332b) SHA1(48f14b31279da6757848df6feb5318818f8f576c), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "kick31", "Kickstart 3.1 (40.68)")
	ROMX_LOAD("kick31.u182",    0x00000, 0x40000, CRC(286b9a0d) SHA1(6763a2258ec493f7408cf663110dae9a17803ad1), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(4))
	ROMX_LOAD("kick31.u183",    0x00002, 0x40000, CRC(0b8cde6a) SHA1(5f02e97b48ebbba87d516a56b0400c6fc3434d8d), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "logica2", "Logica Diagnostic 2.0")
	ROMX_LOAD("logica2.u182",    0x00000, 0x40000, CRC(566bc3f9) SHA1(891d3b7892843517d800d24593168b1d8f1646ca), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(5))
	ROMX_LOAD("logica2.u183",    0x00002, 0x40000, CRC(aac94759) SHA1(da8a4f9ae1aa84f5e2a5dcc5c9d7e4378a9698b7), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(5))
ROM_END

#define rom_a3000n  rom_a3000


// Amiga 600
//
// According to Greg Donner's Workbench page, very early models shipped with
// Kickstart 2.04.
//
// Kickstart 2.05 differences:
// - 2.05 37.299: No HDD support
// - 2.05 37.300: HDD support
// - 2.05 37.350: HDD size limits removed
//
// Kickstart 3.1 upgrade available.
//
// The keyboard controller is included on the motherboard, still based on the
// 6500/1.

ROM_START( a600 )
	ROM_REGION16_BE(0x80000, "kickstart", 0)
	ROM_DEFAULT_BIOS("kick205-350")
	ROM_SYSTEM_BIOS(0, "kick204", "Kickstart 2.04 (37.175)")
	ROMX_LOAD("390979-01.u6", 0x00000, 0x80000, CRC(c3bdb240) SHA1(c5839f5cb98a7a8947065c3ed2f14f5f42e334a1), ROM_GROUPWORD | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "kick205-299", "Kickstart 2.05 (37.299)")
	ROMX_LOAD("391388-01.u6", 0x00000, 0x80000, CRC(83028fb5) SHA1(87508de834dc7eb47359cede72d2e3c8a2e5d8db), ROM_GROUPWORD | ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "kick205-300", "Kickstart 2.05 (37.300)")
	ROMX_LOAD("391304-01.u6", 0x00000, 0x80000, CRC(64466c2a) SHA1(f72d89148dac39c696e30b10859ebc859226637b), ROM_GROUPWORD | ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "kick205-350", "Kickstart 2.05 (37.300)")
	ROMX_LOAD("391304-02.u6", 0x00000, 0x80000, CRC(43b0df7b) SHA1(02843c4253bbd29aba535b0aa3bd9a85034ecde4), ROM_GROUPWORD | ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "kick31",  "Kickstart 3.1 (40.63)")
	ROMX_LOAD("kick40063.u6", 0x00000, 0x80000, CRC(fc24ae0d) SHA1(3b7f1493b27e212830f989f26ca76c02049f09ca), ROM_GROUPWORD | ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "logica2", "Logica Diagnostic 2.0")
	ROMX_LOAD("logica2.u6",   0x00000, 0x80000, CRC(8484f426) SHA1(ba10d16166b2e2d6177c979c99edf8462b21651e), ROM_GROUPWORD | ROM_BIOS(6))

	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD("391079-01.u13", 0x000, 0x800, NO_DUMP)
ROM_END

#define rom_a600n  rom_a600

// Amiga 1200
//
// Early models shipped with Kickstart 3.0, later versions with
// Kickstart 3.1. Keyboard controller is included on the motherboard,
// but was changed to a 68HC05 core.

ROM_START( a1200 )
	ROM_REGION32_BE(0x80000, "kickstart", 0)
	ROM_DEFAULT_BIOS("kick31")
	ROM_SYSTEM_BIOS(0, "kick30", "Kickstart 3.0 (39.106)")
	ROMX_LOAD("391523-01.u6a", 0x00000, 0x40000, CRC(c742a412) SHA1(999eb81c65dfd07a71ee19315d99c7eb858ab186), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROMX_LOAD("391524-01.u6b", 0x00002, 0x40000, CRC(d55c6ec6) SHA1(3341108d3a402882b5ef9d3b242cbf3c8ab1a3e9), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "kick31", "Kickstart 3.1 (40.68)")
	ROMX_LOAD("391773-01.u6a", 0x00000, 0x40000, CRC(08dbf275) SHA1(b8800f5f909298109ea69690b1b8523fa22ddb37), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	ROMX_LOAD("391774-01.u6b", 0x00002, 0x40000, CRC(16c07bf8) SHA1(90e331be1970b0e53f53a9b0390b51b59b3869c2), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "logica2", "Logica Diagnostic 2.0")
	ROMX_LOAD("logica2.u6a", 0x00000, 0x40000, CRC(566bc3f9) SHA1(891d3b7892843517d800d24593168b1d8f1646ca), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(3))
	ROMX_LOAD("logica2.u6b", 0x00002, 0x40000, CRC(aac94759) SHA1(da8a4f9ae1aa84f5e2a5dcc5c9d7e4378a9698b7), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(3))

	ROM_REGION(0x4000, "keyboard", 0)
	ROM_LOAD("391508-01.u13", 0x0000, 0x1040, NO_DUMP) // COMMODORE | 391508-01 REV0 | KEYBOARD MPU (MC68HC05C4AFN)
	ROM_LOAD("391508-02.u13", 0x0000, 0x2f40, NO_DUMP) // Amiga Tech REV1 Keyboard MPU (MC68HC05C12FN)
ROM_END

#define rom_a1200n  rom_a1200

// Amiga 4000
//
// Shipped with Kickstart 3.0, upgradable to Kickstart 3.1.

ROM_START( a4000 )
	ROM_REGION32_BE(0x80000, "kickstart", 0)
	ROM_DEFAULT_BIOS("kick30")
	ROM_SYSTEM_BIOS(0, "kick30", "Kickstart 3.0 (39.106)")
	ROMX_LOAD("391513-02.u175", 0x00000, 0x40000, CRC(36f64dd0) SHA1(196e9f3f9cad934e181c07da33083b1f0a3c702f), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROMX_LOAD("391514-02.u176", 0x00002, 0x40000, CRC(17266a55) SHA1(42fbed3453d1f11ccbde89a9826f2d1175cca5cc), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "kick31-68", "Kickstart 3.1 (40.68)")
	ROMX_LOAD("kick40068.u175", 0x00000, 0x40000, CRC(b2af34f8) SHA1(24e52b5efc02049517387ab7b1a1475fc540350e), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	ROMX_LOAD("kick40068.u176", 0x00002, 0x40000, CRC(e65636a3) SHA1(313c7cbda5779e56f19a41d34e760f517626d882), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "kick31-70", "Kickstart 3.1 (40.70)")
	ROMX_LOAD("kick40070.u175", 0x00000, 0x40000, CRC(f9cbecc9) SHA1(138d8cb43b8312fe16d69070de607469b3d4078e), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(3))
	ROMX_LOAD("kick40070.u176", 0x00002, 0x40000, CRC(f8248355) SHA1(c23795479fae3910c185512ca268b82f1ae4fe05), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "logica2", "Logica Diagnostic 2.0")
	ROMX_LOAD("logica2.u6a",    0x00000, 0x40000, CRC(566bc3f9) SHA1(891d3b7892843517d800d24593168b1d8f1646ca), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(4))
	ROMX_LOAD("logica2.u6b",    0x00002, 0x40000, CRC(aac94759) SHA1(da8a4f9ae1aa84f5e2a5dcc5c9d7e4378a9698b7), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(4))
ROM_END

#define rom_a4000n    rom_a4000
#define rom_a400030   rom_a4000
#define rom_a400030n  rom_a4000

// Amiga 4000T
//
// Shipped with Kickstart 3.1 (40.70).

ROM_START( a4000t )
	ROM_REGION32_BE(0x80000, "kickstart", 0)
	ROM_DEFAULT_BIOS("kick31")
	ROM_SYSTEM_BIOS(0, "kick31", "Kickstart 3.1 (40.70)")
	ROMX_LOAD("391657-01.u175", 0x00000, 0x40000, CRC(0ca94f70) SHA1(b3806edacb3362fc16a154ce1eeec5bf5bc24789), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROMX_LOAD("391658-01.u176", 0x00002, 0x40000, CRC(dfe03120) SHA1(cd7a706c431b04d87814d3a2d8b397100cf44c0c), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "logica2", "Logica Diagnostic 2.0")
	ROMX_LOAD("logica2.u6a",    0x00000, 0x40000, CRC(566bc3f9) SHA1(891d3b7892843517d800d24593168b1d8f1646ca), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	ROMX_LOAD("logica2.u6b",    0x00002, 0x40000, CRC(aac94759) SHA1(da8a4f9ae1aa84f5e2a5dcc5c9d7e4378a9698b7), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
ROM_END

#define rom_a4000tn  rom_a4000t

// Amiga CD32
//
// Shipped with Kickstart 3.1 and additional software interleaved in a 1MB rom chip.

ROM_START( cd32 )
	ROM_REGION32_BE(0x100000, "kickstart", 0)
//  ROM_LOAD16_WORD("391640-03.u6a", 0x000000, 0x100000, CRC(a4fbc94a) SHA1(816ce6c5077875850c7d43452230a9ba3a2902db)) // todo: this is the real dump
	ROM_LOAD16_WORD("391640-03.u6a", 0x000000, 0x100000, CRC(d3837ae4) SHA1(06807db3181637455f4d46582d9972afec8956d9))
ROM_END

#define rom_cd32n  rom_cd32


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

// OCS Chipset
COMP( 1985, a1000,    0,      0, a1000,    amiga, a1000_state, pal,  "Commodore", "Amiga 1000 (PAL)",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1985, a1000n,   a1000,  0, a1000n,   amiga, a1000_state, ntsc, "Commodore", "Amiga 1000 (NTSC)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, a2000,    0,      0, a2000,    amiga, a2000_state, pal,  "Commodore", "Amiga 2000 (PAL)",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, a2000n,   a2000,  0, a2000n,   amiga, a2000_state, ntsc, "Commodore", "Amiga 2000 (NTSC)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, a500,     0,      0, a500,     amiga, a500_state,  pal,  "Commodore", "Amiga 500 (PAL)",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, a500n,    a500,   0, a500n,    amiga, a500_state,  ntsc, "Commodore", "Amiga 500 (NTSC)",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1990, cdtv,     0,      0, cdtv,     amiga, cdtv_state,  pal,  "Commodore", "CDTV (PAL)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1990, cdtvn,    cdtv,   0, cdtvn,    amiga, cdtv_state,  ntsc, "Commodore", "CDTV (NTSC)",           MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// ECS Chipset
COMP( 1990, a3000,    0,      0, a3000,    amiga, a3000_state, pal,  "Commodore", "Amiga 3000 (PAL)",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1990, a3000n,   a3000,  0, a3000n,   amiga, a3000_state, ntsc, "Commodore", "Amiga 3000 (NTSC)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1992, a500p,    0,      0, a500p,    amiga, a500p_state, pal,  "Commodore", "Amiga 500 Plus (PAL)",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1992, a500pn,   a500p,  0, a500pn,   amiga, a500p_state, ntsc, "Commodore", "Amiga 500 Plus (NTSC)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1992, a600,     0,      0, a600,     amiga, a600_state,  pal,  "Commodore", "Amiga 600 (PAL)",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1992, a600n,    a600,   0, a600n,    amiga, a600_state,  ntsc, "Commodore", "Amiga 600 (NTSC)",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// AGA Chipset
COMP( 1992, a1200,    0,      0, a1200,    amiga, a1200_state, pal,  "Commodore", "Amiga 1200 (PAL)",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1992, a1200n,   a1200,  0, a1200n,   amiga, a1200_state, ntsc, "Commodore", "Amiga 1200 (NTSC)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1992, a4000,    0,      0, a4000,    amiga, a4000_state, pal,  "Commodore", "Amiga 4000/040 (PAL)",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1992, a4000n,   a4000,  0, a4000n,   amiga, a4000_state, ntsc, "Commodore", "Amiga 4000/040 (NTSC)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1993, a400030,  a4000,  0, a400030,  amiga, a4000_state, pal,  "Commodore", "Amiga 4000/030 (PAL)",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1993, a400030n, a4000,  0, a400030n, amiga, a4000_state, ntsc, "Commodore", "Amiga 4000/030 (NTSC)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1993, cd32,     0,      0, cd32,     cd32,  cd32_state,  pal,  "Commodore", "Amiga CD32 (PAL)",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1993, cd32n,    cd32,   0, cd32n,    cd32,  cd32_state,  ntsc, "Commodore", "Amiga CD32 (NTSC)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1994, a4000t,   0,      0, a4000t,   amiga, a4000_state, pal,  "Commodore", "Amiga 4000T (PAL)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1994, a4000tn,  a4000t, 0, a4000tn,  amiga, a4000_state, ntsc, "Commodore", "Amiga 4000T (NTSC)",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
