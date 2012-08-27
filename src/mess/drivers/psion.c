/***************************************************************************

        Psion Organiser II series

        Driver by Sandro Ronco

        TODO:
        - dump CGROM of the HD44780
        - emulate other devices in slot C(Comms Link, printer)

        Note:
        - 4 lines display has an custom LCD controller derived from an HD66780
        - NVRAM works only if the machine is turned off (with OFF menu) before closing MESS

        More info:
            http://archive.psion2.org/org2/techref/index.htm

****************************************************************************/


#include "emu.h"
#include "includes/psion.h"
#include "rendlay.h"

static TIMER_DEVICE_CALLBACK( nmi_timer )
{
	psion_state *state = timer.machine().driver_data<psion_state>();

	if (state->m_enable_nmi)
		cputag_set_input_line(timer.machine(), "maincpu", INPUT_LINE_NMI, PULSE_LINE);
}

UINT8 psion_state::kb_read(running_machine &machine)
{
	static const char *const bitnames[] = {"K1", "K2", "K3", "K4", "K5", "K6", "K7"};
	UINT8 line, data = 0x7c;

	if (m_kb_counter)
	{
		for (line = 0; line < 7; line++)
			if (m_kb_counter == (0x7f & ~(1 << line)))
				data = machine.root_device().ioport(bitnames[line])->read();
	}
	else
	{
		//Read all the input lines
		for (line = 0; line < 7; line++)
			data &= machine.root_device().ioport(bitnames[line])->read();
	}

	return data & 0x7c;
}

void psion_state::update_banks(running_machine &machine)
{
	psion_state *state = machine.driver_data<psion_state>();
	if (m_ram_bank < m_ram_bank_count && m_ram_bank_count)
		state->membank("rambank")->set_entry(m_ram_bank);

	if (m_rom_bank < m_rom_bank_count && m_rom_bank_count)
		state->membank("rombank")->set_entry(m_rom_bank);
}

WRITE8_MEMBER( psion_state::hd63701_int_reg_w )
{
	switch (offset)
	{
	case 0x01:
		m_port2_ddr = data;
		break;
	case 0x03:
		/* datapack i/o data bus */
		m_pack1->data_w(data & m_port2_ddr);
		m_pack2->data_w(data & m_port2_ddr);
		break;
	case 0x08:
		m_tcsr_value = data;
		break;
	case 0x15:
		/* read-only */
		break;
	case 0x16:
		m_port6_ddr = data;
		break;
	case 0x17:
		/*
        datapack control lines
        x--- ---- slot on/off
        -x-- ---- slot 3
        --x- ---- slot 2
        ---x ---- slot 1
        ---- x--- output enable
        ---- -x-- program line
        ---- --x- reset line
        ---- ---x clock line
        */
		m_port6 = (data & m_port6_ddr) | (m_port6 & ~m_port6_ddr);

		m_pack1->control_w((m_port6 & 0x8f) | (m_port6 & 0x10));
		m_pack2->control_w((m_port6 & 0x8f) | ((m_port6 & 0x20) >> 1));
		break;
	}

	m6801_io_w(&space, offset, data);
}

READ8_MEMBER( psion_state::hd63701_int_reg_r )
{
	switch (offset)
	{
	case 0x03:
		/* datapack i/o data bus */
		return (m_pack1->data_r() | m_pack2->data_r()) & (~m_port2_ddr);
	case 0x14:
		return (m6801_io_r(&space, offset)&0x7f) | (m_stby_pwr<<7);
	case 0x15:
		/*
        x--- ---- ON key active high
        -xxx xx-- keys matrix active low
        ---- --x- pulse
        ---- ---x battery status
        */
		return kb_read(machine()) | ioport("BATTERY")->read() | ioport("ON")->read() | (m_kb_counter == 0x7ff)<<1 | m_pulse<<1;
	case 0x17:
		/* datapack control lines */
		return (m_pack1->control_r() | (m_pack2->control_r() & 0x8f)) | ((m_pack2->control_r() & 0x10)<<1);
	case 0x08:
		m6801_io_w(&space, offset, m_tcsr_value);
	default:
		return m6801_io_r(&space, offset);
	}
}

/* Read/Write common */
void psion_state::io_rw(address_space &space, UINT16 offset)
{
	if (space.debugger_access())
		return;

	switch (offset & 0xffc0)
	{
	case 0xc0:
		/* switch off, CPU goes into standby mode */
		m_enable_nmi = 0;
		m_stby_pwr = 1;
		space.machine().device<cpu_device>("maincpu")->suspend(SUSPEND_REASON_HALT, 1);
		break;
	case 0x100:
		m_pulse = 1;
		break;
	case 0x140:
		m_pulse = 0;
		break;
	case 0x200:
		m_kb_counter = 0;
		break;
	case 0x180:
		beep_set_state(m_beep, 1);
		break;
	case 0x1c0:
		beep_set_state(m_beep, 0);
		break;
	case 0x240:
		if (offset == 0x260 && (m_rom_bank_count || m_ram_bank_count))
		{
			m_ram_bank=0;
			m_rom_bank=0;
			update_banks(machine());
		}
		else
			m_kb_counter++;
		break;
	case 0x280:
		if (offset == 0x2a0 && m_ram_bank_count)
		{
			m_ram_bank++;
			update_banks(machine());
		}
		else
			m_enable_nmi = 1;
		break;
	case 0x2c0:
		if (offset == 0x2e0 && m_rom_bank_count)
		{
			m_rom_bank++;
			update_banks(machine());
		}
		else
			m_enable_nmi = 0;
		break;
	}
}

WRITE8_MEMBER( psion_state::io_w )
{
	switch (offset & 0x0ffc0)
	{
	case 0x80:
		if (offset & 1)
			m_lcdc->data_write(space, offset, data);
		else
			m_lcdc->control_write(space, offset, data);
		break;
	default:
		io_rw(space, offset);
	}
}

READ8_MEMBER( psion_state::io_r )
{
	switch (offset & 0xffc0)
	{
	case 0x80:
		if (offset & 1)
			return m_lcdc->data_read(space, offset);
		else
			return m_lcdc->control_read(space, offset);
	default:
		io_rw(space, offset);
	}

	return 0;
}

static INPUT_CHANGED( psion_on )
{
	cpu_device *cpu = field.machine().device<cpu_device>("maincpu");

	/* reset the CPU for resume from standby */
	if (cpu->suspended(SUSPEND_REASON_HALT))
		cpu->reset();
}

static ADDRESS_MAP_START(psioncm_mem, AS_PROGRAM, 8, psion_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(hd63701_int_reg_r, hd63701_int_reg_w)
	AM_RANGE(0x0040, 0x00ff) AM_RAM AM_SHARE("sys_register")
	AM_RANGE(0x0100, 0x03ff) AM_READWRITE(io_r, io_w)
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(psionla_mem, AS_PROGRAM, 8, psion_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(hd63701_int_reg_r, hd63701_int_reg_w)
	AM_RANGE(0x0040, 0x00ff) AM_RAM AM_SHARE("sys_register")
	AM_RANGE(0x0100, 0x03ff) AM_READWRITE(io_r, io_w)
	AM_RANGE(0x0400, 0x5fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(psionp350_mem, AS_PROGRAM, 8, psion_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(hd63701_int_reg_r, hd63701_int_reg_w)
	AM_RANGE(0x0040, 0x00ff) AM_RAM AM_SHARE("sys_register")
	AM_RANGE(0x0100, 0x03ff) AM_READWRITE(io_r, io_w)
	AM_RANGE(0x0400, 0x3fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("rambank")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(psionlam_mem, AS_PROGRAM, 8, psion_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(hd63701_int_reg_r, hd63701_int_reg_w)
	AM_RANGE(0x0040, 0x00ff) AM_RAM AM_SHARE("sys_register")
	AM_RANGE(0x0100, 0x03ff) AM_READWRITE(io_r, io_w)
	AM_RANGE(0x0400, 0x7fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("rombank")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(psionlz_mem, AS_PROGRAM, 8, psion_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(hd63701_int_reg_r, hd63701_int_reg_w)
	AM_RANGE(0x0040, 0x00ff) AM_RAM AM_SHARE("sys_register")
	AM_RANGE(0x0100, 0x03ff) AM_READWRITE(io_r, io_w)
	AM_RANGE(0x0400, 0x3fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("rambank")
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("rombank")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( psion )
	PORT_START("BATTERY")
		PORT_CONFNAME( 0x01, 0x00, "Battery Status" )
		PORT_CONFSETTING( 0x00, DEF_STR( Normal ) )
		PORT_CONFSETTING( 0x01, "Low Battery" )

	PORT_START("ON")
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ON/CLEAR") PORT_CODE(KEYCODE_MINUS)  PORT_CHANGED(psion_on, 0)

	PORT_START("K1")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up [CAP]") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down [NUM]") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)

	PORT_START("K2")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S [;]") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M [,]") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G [=]") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A [<]") PORT_CODE(KEYCODE_A)

	PORT_START("K3")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T [:]") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N [$]") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H [\"]") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B [>]") PORT_CODE(KEYCODE_B)

	PORT_START("K4")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y [0]") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U [1]") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O [4]") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I [7]") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C [(]") PORT_CODE(KEYCODE_C)

	PORT_START("K5")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W [3]") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q [6]") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K [9]") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E [%]") PORT_CODE(KEYCODE_E)

	PORT_START("K6")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXE") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X [+]") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R [-]") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L [*]") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F [/]") PORT_CODE(KEYCODE_F)

	PORT_START("K7")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z [.]") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V [2]") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P [5]") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J [8]") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D [)]") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

static NVRAM_HANDLER( psion )
{
	psion_state *state = machine.driver_data<psion_state>();

	if (read_or_write)
	{
		file->write(state->m_sys_register, 0xc0);
		file->write(state->m_ram, state->m_ram.bytes());
		if (state->m_ram_bank_count)
			file->write(state->m_paged_ram, state->m_ram_bank_count * 0x4000);
	}
	else
	{
		if (file)
		{
			file->read(state->m_sys_register, 0xc0);
			file->read(state->m_ram, state->m_ram.bytes());
			if (state->m_ram_bank_count)
				file->read(state->m_paged_ram, state->m_ram_bank_count * 0x4000);

			//warm start
			state->m_stby_pwr = 1;
		}
		else
			//cold start
			state->m_stby_pwr = 0;
	}
}

void psion_state::machine_start()
{
	if (!strcmp(machine().system().name, "psionlam"))
	{
		m_rom_bank_count = 3;
		m_ram_bank_count = 0;
	}
	else if (!strcmp(machine().system().name, "psionp350"))
	{
		m_rom_bank_count = 0;
		m_ram_bank_count = 5;
	}
	else if (!strncmp(machine().system().name, "psionlz", 7))
	{
		m_rom_bank_count = 3;
		m_ram_bank_count = 3;
	}
	else if (!strcmp(machine().system().name, "psionp464"))
	{
		m_rom_bank_count = 3;
		m_ram_bank_count = 9;
	}
	else
	{
		m_rom_bank_count = 0;
		m_ram_bank_count = 0;
	}

	if (m_rom_bank_count)
	{
		UINT8* rom_base = (UINT8 *)machine().root_device().memregion("maincpu")->base();

		membank("rombank")->configure_entry(0, rom_base + 0x8000);
		membank("rombank")->configure_entries(1, m_rom_bank_count-1, rom_base + 0x10000, 0x4000);
		membank("rombank")->set_entry(0);
	}

	if (m_ram_bank_count)
	{
		m_paged_ram = auto_alloc_array(machine(), UINT8, m_ram_bank_count * 0x4000);
		membank("rambank")->configure_entries(0, m_ram_bank_count, m_paged_ram, 0x4000);
		membank("rambank")->set_entry(0);
	}

	save_item(NAME(m_kb_counter));
	save_item(NAME(m_enable_nmi));
	save_item(NAME(m_tcsr_value));
	save_item(NAME(m_stby_pwr));
	save_item(NAME(m_pulse));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_port2_ddr));
	save_item(NAME(m_port2));
	save_item(NAME(m_port6_ddr));
	save_item(NAME(m_port6));
	save_pointer(NAME(m_paged_ram), m_ram_bank_count * 0x4000);
}

void psion_state::machine_reset()
{
	m_enable_nmi=0;
	m_kb_counter=0;
	m_ram_bank=0;
	m_rom_bank=0;
	m_pulse=0;

	if (m_rom_bank_count || m_ram_bank_count)
		update_banks(machine());
}

static PALETTE_INIT( psion )
{
	palette_set_color(machine, 0, MAKE_RGB(138, 146, 148));
	palette_set_color(machine, 1, MAKE_RGB(92, 83, 88));
}

static const gfx_layout psion_charlayout =
{
	5, 8,					/* 5 x 8 characters */
	256,					/* 256 characters */
	1,						/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8						/* 8 bytes */
};

static GFXDECODE_START( psion )
	GFXDECODE_ENTRY( "hd44780", 0x0000, psion_charlayout, 0, 1 )
GFXDECODE_END

static HD44780_INTERFACE( psion_2line_display )
{
	2,					// number of lines
	16,					// chars for line
	NULL				// pixel update callback
};

/* basic configuration for 2 lines display */
static MACHINE_CONFIG_START( psion_2lines, psion_state )
	/* basic machine hardware */
    MCFG_CPU_ADD("maincpu", HD63701, 980000) // should be HD6303 at 0.98MHz

    /* video hardware */
    MCFG_SCREEN_ADD("screen", RASTER)
    MCFG_SCREEN_REFRESH_RATE(50)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(6*16, 9*2)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*16-1, 0, 9*2-1)
	MCFG_DEFAULT_LAYOUT(layout_lcd)
    MCFG_PALETTE_LENGTH(2)
    MCFG_PALETTE_INIT(psion)
	MCFG_GFXDECODE(psion)

	MCFG_HD44780_ADD("hd44780", psion_2line_display)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( BEEPER_TAG, BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

	MCFG_NVRAM_HANDLER(psion)

	MCFG_TIMER_ADD_PERIODIC("nmi_timer", nmi_timer, attotime::from_seconds(1))

	/* Datapack */
	MCFG_PSION_DATAPACK_ADD("pack1")
	MCFG_PSION_DATAPACK_ADD("pack2")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("pack_list", "psion")
MACHINE_CONFIG_END


static HD44780_INTERFACE( psion_4line_display )
{
	4,					// number of lines
	20,					// chars for line
	NULL				// pixel update callback
};

/* basic configuration for 4 lines display */
static MACHINE_CONFIG_START( psion_4lines, psion_state )
	/* basic machine hardware */
    MCFG_CPU_ADD("maincpu",HD63701, 980000) // should be HD6303 at 0.98MHz

    /* video hardware */
    MCFG_SCREEN_ADD("screen", RASTER)
    MCFG_SCREEN_REFRESH_RATE(50)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(6*20, 9*4)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*20-1, 0, 9*4-1)
	MCFG_DEFAULT_LAYOUT(layout_lcd)
    MCFG_PALETTE_LENGTH(2)
    MCFG_PALETTE_INIT(psion)
	MCFG_GFXDECODE(psion)

	MCFG_PSION_CUSTOM_LCDC_ADD("hd44780", psion_4line_display)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( BEEPER_TAG, BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

	MCFG_NVRAM_HANDLER(psion)

	MCFG_TIMER_ADD_PERIODIC("nmi_timer", nmi_timer, attotime::from_seconds(1))

	/* Datapack */
	MCFG_PSION_DATAPACK_ADD("pack1")
	MCFG_PSION_DATAPACK_ADD("pack2")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("pack_list", "psion")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( psioncm, psion_2lines )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(psioncm_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( psionla, psion_2lines )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(psionla_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( psionlam, psion_2lines )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(psionlam_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( psionp350, psion_2lines )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(psionp350_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( psionlz, psion_4lines )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(psionlz_mem)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( psioncm )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v24", "CM v2.4")
	ROMX_LOAD( "24-cm.dat",    0x8000, 0x8000,  CRC(f6798394) SHA1(736997f0db9a9ee50d6785636bdc3f8ff1c33c66), ROM_BIOS(1))

	ROM_REGION( 0x0860, "hd44780", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END

ROM_START( psionla )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v33", "LA v3.3")
	ROMX_LOAD( "33-la.dat",    0x8000, 0x8000,  CRC(02668ed4) SHA1(e5d4ee6b1cde310a2970ffcc6f29a0ce09b08c46), ROM_BIOS(1))

	ROM_REGION( 0x0860, "hd44780", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END

ROM_START( psionp350 )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v36", "POS350 v3.6")
	ROMX_LOAD( "36-p350.dat",  0x8000, 0x8000,  CRC(3a371a74) SHA1(9167210b2c0c3bd196afc08ca44ab23e4e62635e), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v38", "POS350 v3.8")
	ROMX_LOAD( "38-p350.dat",  0x8000, 0x8000,  CRC(1b8b082f) SHA1(a3e875a59860e344f304a831148a7980f28eaa4a), ROM_BIOS(2))

	ROM_REGION( 0x0860, "hd44780", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END

ROM_START( psionlam )
    ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v37", "LA v3.7")
	ROMX_LOAD( "37-lam.dat",   0x8000, 0x10000, CRC(7ee3a1bc) SHA1(c7fbd6c8e47c9b7d5f636e9f56e911b363d6796b), ROM_BIOS(1))

	ROM_REGION( 0x0860, "hd44780", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END

ROM_START( psionlz64 )
    ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v44", "LZ64 v4.4")
	ROMX_LOAD( "44-lz64.dat",  0x8000, 0x10000, CRC(aa487913) SHA1(5a44390f63fc8c1bc94299ab2eb291bc3a5b989a), ROM_BIOS(1))

	ROM_REGION( 0x0860, "hd44780", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END

ROM_START( psionlz64s )
    ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v46", "LZ64 v4.6")
	ROMX_LOAD( "46-lz64s.dat", 0x8000, 0x10000, CRC(328d9772) SHA1(7f9e2d591d59ecfb0822d7067c2fe59542ea16dd), ROM_BIOS(1))

	ROM_REGION( 0x0860, "hd44780", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END

ROM_START( psionlz )
    ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v46", "LZ v4.6")
	ROMX_LOAD( "46-lz.dat",    0x8000, 0x10000, CRC(22715f48) SHA1(cf460c81cadb53eddb7afd8dadecbe8c38ea3fc2), ROM_BIOS(1))

	ROM_REGION( 0x0860, "hd44780", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END

ROM_START( psionp464 )
    ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v46", "POS464 v4.6")
	ROMX_LOAD( "46-p464.dat",  0x8000, 0x10000, CRC(672a0945) SHA1(d2a6e3fe1019d1bd7ae4725e33a0b9973f8cd7d8), ROM_BIOS(1))

	ROM_REGION( 0x0860, "hd44780", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT COMPANY   FULLNAME       FLAGS */
COMP( 1986, psioncm,	0,       0, 	psioncm,		psion, driver_device,	 0,   "Psion",   "Organiser II CM",		GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS)
COMP( 1986, psionla,	psioncm, 0, 	psionla,	    psion, driver_device,	 0,   "Psion",   "Organiser II LA",		GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS)
COMP( 1986, psionp350,	psioncm, 0, 	psionp350,	    psion, driver_device,	 0,   "Psion",   "Organiser II P350",	GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS)
COMP( 1986, psionlam,	psioncm, 0, 	psionlam,	    psion, driver_device,	 0,   "Psion",   "Organiser II LAM",	GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS)
COMP( 1989, psionlz,	0,		 0, 	psionlz,	    psion, driver_device,	 0,   "Psion",   "Organiser II LZ",		GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS)
COMP( 1989, psionlz64,  psionlz, 0, 	psionlz,	    psion, driver_device,	 0,   "Psion",   "Organiser II LZ64",	GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS)
COMP( 1989, psionlz64s,	psionlz, 0, 	psionlz,	    psion, driver_device,	 0,   "Psion",   "Organiser II LZ64S",	GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS)
COMP( 1989, psionp464,	psionlz, 0, 	psionlz,	    psion, driver_device,	 0,   "Psion",   "Organiser II P464",	GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS)
