/***************************************************************************

    IBM AT Compatibles

***************************************************************************/

/* mingw-gcc defines this */
#ifdef i386
#undef i386
#endif /* i386 */


#include "includes/at.h"
#include "machine/pc_keyboards.h"


static ADDRESS_MAP_START( at16_map, AS_PROGRAM, 16, at_state )
	AM_RANGE(0x000000, 0x09ffff) AM_MIRROR(0xff000000) AM_RAMBANK("bank10")
	AM_RANGE(0x0c0000, 0x0c7fff) AM_ROM
	AM_RANGE(0x0c8000, 0x0cffff) AM_ROM
	AM_RANGE(0x0d0000, 0x0effff) AM_RAM
	AM_RANGE(0x0f0000, 0x0fffff) AM_ROM
	AM_RANGE(0xff0000, 0xffffff) AM_ROM AM_REGION("maincpu", 0x0f0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( neat_map, AS_PROGRAM, 16, at_state )
	AM_RANGE(0x000000, 0x09ffff) AM_MIRROR(0xff000000) AM_RAMBANK("bank10")
	AM_RANGE(0x0c0000, 0x0c7fff) AM_ROM
	AM_RANGE(0x0c8000, 0x0cffff) AM_ROM
	AM_RANGE(0x0d0000, 0x0effff) AM_RAM
	AM_RANGE(0x0f0000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0xfeffff) AM_RAM
	AM_RANGE(0xff0000, 0xffffff) AM_ROM AM_REGION("maincpu", 0x0f0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at386_map, AS_PROGRAM, 32, at_state )
	ADDRESS_MAP_GLOBAL_MASK(0x00ffffff)
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x000a0000, 0x000bffff) AM_NOP
	AM_RANGE(0x000c0000, 0x000c7fff) AM_ROM
	AM_RANGE(0x000c8000, 0x000cffff) AM_ROM
	AM_RANGE(0x000d0000, 0x000effff) AM_ROM
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROM
	AM_RANGE(0x00800000, 0x00800bff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x00ff0000, 0x00ffffff) AM_ROM AM_REGION("maincpu", 0x0f0000)
ADDRESS_MAP_END

// memory is mostly handled by the chipset
static ADDRESS_MAP_START( ct486_map, AS_PROGRAM, 32, at_state )
	AM_RANGE(0x00800000, 0x00800bff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( at586_map, AS_PROGRAM, 32, at586_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x000a0000, 0x000bffff) AM_NOP
	AM_RANGE(0x00800000, 0x00800bff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("isa", 0x20000)
ADDRESS_MAP_END



READ8_MEMBER( at_state::at_dma8237_2_r )
{
	return m_dma8237_2->read(space, offset / 2);
}

WRITE8_MEMBER( at_state::at_dma8237_2_w )
{
	m_dma8237_2->write(space, offset / 2, data);
}

READ8_MEMBER( at_state::at_keybc_r )
{
	switch (offset)
	{
	case 0: return m_keybc->data_r(space, 0);
	case 1: return at_portb_r(space, 0);
	}

	return 0xff;
}

WRITE8_MEMBER( at_state::at_keybc_w )
{
	switch (offset)
	{
	case 0: m_keybc->data_w(space, 0, data); break;
	case 1: at_portb_w(space, 0, data); break;
	}
}


WRITE8_MEMBER( at_state::write_rtc )
{
	if (offset==0) {
		m_nmi_enabled = BIT(data,7);
		m_isabus->set_nmi_state((m_nmi_enabled==0) && (m_channel_check==0));
		m_mc146818->write(space,0,data);
	}
	else {
		m_mc146818->write(space,offset,data);
	}
}

static ADDRESS_MAP_START( at16_io, AS_IO, 16, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_master", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE8(at_keybc_r, at_keybc_w, 0xffff)
	AM_RANGE(0x0064, 0x0067) AM_DEVREADWRITE8("keybc", at_keyboard_controller_device, status_r, command_w, 0xffff)
	AM_RANGE(0x0070, 0x007f) AM_DEVREAD8("rtc", mc146818_device, read, 0xffff) AM_WRITE8(write_rtc , 0xffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(at_page8_r, at_page8_w, 0xffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_slave", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x00c0, 0x00df) AM_READWRITE8(at_dma8237_2_r, at_dma8237_2_w, 0xffff)
ADDRESS_MAP_END

READ16_MEMBER( at_state::neat_chipset_r )
{
	if (ACCESSING_BITS_0_7)
		return 0xff;
	if (ACCESSING_BITS_8_15)
		return m_cs8221->data_r(space, 0, 0) << 8;
	return 0xffff;
}

WRITE16_MEMBER( at_state::neat_chipset_w )
{
	if (ACCESSING_BITS_0_7)
		m_cs8221->address_w(space, 0, data, 0);

	if (ACCESSING_BITS_8_15)
		m_cs8221->data_w(space, 0, data >> 8, 0);
}

static ADDRESS_MAP_START( neat_io, AS_IO, 16, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8_LEGACY("pic8259_master", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x0022, 0x0023) AM_READWRITE(neat_chipset_r, neat_chipset_w)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE8(at_keybc_r, at_keybc_w, 0xffff)
	AM_RANGE(0x0064, 0x0067) AM_DEVREADWRITE8("keybc", at_keyboard_controller_device, status_r, command_w, 0xffff)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write , 0xffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(at_page8_r, at_page8_w, 0xffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_slave", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x00c0, 0x00df) AM_READWRITE8(at_dma8237_2_r, at_dma8237_2_w, 0xffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at386_io, AS_IO, 32, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", am9517a_device, read, write, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_master", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE8(at_keybc_r, at_keybc_w, 0xffff)
	AM_RANGE(0x0064, 0x0067) AM_DEVREADWRITE8("keybc", at_keyboard_controller_device, status_r, command_w, 0xffff)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write , 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(at_page8_r, at_page8_w, 0xffffffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_slave", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_READWRITE8(at_dma8237_2_r, at_dma8237_2_w, 0xffffffff)
ADDRESS_MAP_END


READ32_MEMBER( at_state::ct486_chipset_r )
{
	if (ACCESSING_BITS_0_7)
		return pic8259_r(m_pic8259_master, space, 0);

	if (ACCESSING_BITS_8_15)
		return pic8259_r(m_pic8259_master, space, 1) << 8;

	if (ACCESSING_BITS_24_31)
		return m_cs4031->data_r(space, 0, 0) << 24;

	return 0xffffffff;
}

WRITE32_MEMBER( at_state::ct486_chipset_w )
{
	if (ACCESSING_BITS_0_7)
		pic8259_w(m_pic8259_master, space, 0, data);

	if (ACCESSING_BITS_8_15)
		pic8259_w(m_pic8259_master, space, 1, data >> 8);

	if (ACCESSING_BITS_16_23)
		m_cs4031->address_w(space, 0, data >> 16, 0);

	if (ACCESSING_BITS_24_31)
		m_cs4031->data_w(space, 0, data >> 24, 0);
}

static ADDRESS_MAP_START( ct486_io, AS_IO, 32, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", am9517a_device, read, write, 0xffffffff)
	AM_RANGE(0x0020, 0x0023) AM_READWRITE(ct486_chipset_r, ct486_chipset_w)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE8(at_keybc_r, at_keybc_w, 0xffff)
	AM_RANGE(0x0064, 0x0067) AM_DEVREADWRITE8("keybc", at_keyboard_controller_device, status_r, command_w, 0xffff)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write , 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(at_page8_r, at_page8_w, 0xffffffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_slave", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_READWRITE8(at_dma8237_2_r, at_dma8237_2_w, 0xffffffff)
ADDRESS_MAP_END


static ADDRESS_MAP_START( at586_io, AS_IO, 32, at586_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( megapc_io, AS_IO, 32, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", am9517a_device, read, write, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_master", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE8(at_keybc_r, at_keybc_w, 0xffff) // TODO: is this the correct type?
	AM_RANGE(0x0064, 0x0067) AM_DEVREADWRITE8("keybc", at_keyboard_controller_device, status_r, command_w, 0xffff)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write , 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(at_page8_r, at_page8_w, 0xffffffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_slave", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_READWRITE8(at_dma8237_2_r, at_dma8237_2_w, 0xffffffff)
ADDRESS_MAP_END


static INPUT_PORTS_START( atcga )
    PORT_START("DSW0") /* IN1 */
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(	0x00, "1" )
	PORT_DIPSETTING(	0x40, "2" )
	PORT_DIPSETTING(	0x80, "3" )
	PORT_DIPSETTING(	0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x00, "Graphics adapter")
	PORT_DIPSETTING(	0x00, "EGA/VGA" )
	PORT_DIPSETTING(	0x10, "Color 40x25" )
	PORT_DIPSETTING(	0x20, "Color 80x25" )
	PORT_DIPSETTING(	0x30, "Monochrome" )
	PORT_DIPNAME( 0x0c, 0x0c, "RAM banks")
	PORT_DIPSETTING(	0x00, "1 - 16  64 256K" )
	PORT_DIPSETTING(	0x04, "2 - 32 128 512K" )
	PORT_DIPSETTING(	0x08, "3 - 48 192 576K" )
	PORT_DIPSETTING(	0x0c, "4 - 64 256 640K" )
	PORT_DIPNAME( 0x02, 0x00, "80387 installed")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x01, "Floppy installed")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Yes ) )

	PORT_INCLUDE( pcvideo_cga )
INPUT_PORTS_END

static INPUT_PORTS_START( atvga )
	PORT_START("IN0") /* IN0 */
	PORT_DIPNAME( 0x08, 0x00, "VGA 1")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "VGA 2")
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "VGA 3")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "VGA 4")
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

    PORT_START("DSW0") /* IN1 */
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(	0x00, "1" )
	PORT_DIPSETTING(	0x40, "2" )
	PORT_DIPSETTING(	0x80, "3" )
	PORT_DIPSETTING(	0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x00, "Graphics adapter")
	PORT_DIPSETTING(	0x00, "EGA/VGA" )
	PORT_DIPSETTING(	0x10, "Color 40x25" )
	PORT_DIPSETTING(	0x20, "Color 80x25" )
	PORT_DIPSETTING(	0x30, "Monochrome" )
	PORT_DIPNAME( 0x0c, 0x0c, "RAM banks")
	PORT_DIPSETTING(	0x00, "1 - 16  64 256K" )
	PORT_DIPSETTING(	0x04, "2 - 32 128 512K" )
	PORT_DIPSETTING(	0x08, "3 - 48 192 576K" )
	PORT_DIPSETTING(	0x0c, "4 - 64 256 640K" )
	PORT_DIPNAME( 0x02, 0x00, "80387 installed")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x01, "Floppy installed")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Yes ) )
INPUT_PORTS_END

static const unsigned i286_address_mask = 0x00ffffff;

static const at_keyboard_controller_interface keyboard_controller_intf =
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_RESET),
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_A20),
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir1_w),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("pc_kbdc", pc_kbdc_device, clock_write_from_mb),
	DEVCB_DEVICE_LINE_MEMBER("pc_kbdc", pc_kbdc_device, data_write_from_mb)
};

static const pc_kbdc_interface pc_kbdc_intf =
{
	DEVCB_DEVICE_LINE_MEMBER("keybc", at_keyboard_controller_device, keyboard_clock_w),
	DEVCB_DEVICE_LINE_MEMBER("keybc", at_keyboard_controller_device, keyboard_data_w)
};

WRITE_LINE_MEMBER( at_state::at_mc146818_irq )
{
	pic8259_ir0_w(m_pic8259_slave, (state) ? 0 : 1);
}

const struct mc146818_interface at_mc146818_config =
{
	DEVCB_DRIVER_LINE_MEMBER(at_state, at_mc146818_irq)
};

static const isa16bus_interface isabus_intf =
{
	// interrupts
	DEVCB_DEVICE_LINE("pic8259_slave",  pic8259_ir2_w), // in place of irq 2 on at irq 9 is used
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir3_w),
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir4_w),
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir5_w),
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir6_w),
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir7_w),

	DEVCB_DEVICE_LINE("pic8259_slave", pic8259_ir3_w),
	DEVCB_DEVICE_LINE("pic8259_slave", pic8259_ir4_w),
	DEVCB_DEVICE_LINE("pic8259_slave", pic8259_ir5_w),
	DEVCB_DEVICE_LINE("pic8259_slave", pic8259_ir6_w),
	DEVCB_DEVICE_LINE("pic8259_slave", pic8259_ir7_w),

	// dma request
	DEVCB_DEVICE_LINE_MEMBER("dma8237_1", am9517a_device, dreq0_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237_1", am9517a_device, dreq1_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237_1", am9517a_device, dreq2_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237_1", am9517a_device, dreq3_w),

	DEVCB_DEVICE_LINE_MEMBER("dma8237_2", am9517a_device, dreq1_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237_2", am9517a_device, dreq2_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237_2", am9517a_device, dreq3_w),
};

static SLOT_INTERFACE_START(pc_isa16_cards)
	// ISA 8 bit
	SLOT_INTERFACE("mda", ISA8_MDA)
	SLOT_INTERFACE("cga", ISA8_CGA)
	SLOT_INTERFACE("ega", ISA8_EGA)
	SLOT_INTERFACE("vga", ISA8_VGA)
	SLOT_INTERFACE("svga_et4k", ISA8_SVGA_ET4K)
	SLOT_INTERFACE("svga_dm",ISA8_SVGA_CIRRUS)
	SLOT_INTERFACE("com", ISA8_COM)
	SLOT_INTERFACE("comat", ISA8_COM_AT)
	SLOT_INTERFACE("fdc", ISA8_FDC)
	SLOT_INTERFACE("hdc", ISA8_HDC)
	SLOT_INTERFACE("adlib", ISA8_ADLIB)
	SLOT_INTERFACE("hercules", ISA8_HERCULES)
	SLOT_INTERFACE("gblaster", ISA8_GAME_BLASTER)
	SLOT_INTERFACE("sblaster1_0", ISA8_SOUND_BLASTER_1_0)
	SLOT_INTERFACE("sblaster1_5", ISA8_SOUND_BLASTER_1_5)
	SLOT_INTERFACE("stereo_fx", ISA8_STEREO_FX)
	SLOT_INTERFACE("ne1000", NE1000)
	SLOT_INTERFACE("3c503", EL2_3C503)
	SLOT_INTERFACE("mpu401", ISA8_MPU401)
	SLOT_INTERFACE("lpt", ISA8_LPT)
	SLOT_INTERFACE("ibm_mfc", ISA8_IBM_MFC)
	SLOT_INTERFACE("fdcsmc", ISA8_FDC_SMC)
	// ISA 16 bit
	SLOT_INTERFACE("ide", ISA16_IDE)
	SLOT_INTERFACE("ide_cd", ISA16_IDE_CD)
	SLOT_INTERFACE("ne2000", NE2000)
	SLOT_INTERFACE("aha1542", AHA1542)
	SLOT_INTERFACE("gus",ISA16_GUS)
	SLOT_INTERFACE("sblaster_16", ISA16_SOUND_BLASTER_16)
	SLOT_INTERFACE("svga_s3",ISA16_SVGA_S3)
	SLOT_INTERFACE("gfxultra",ISA16_VGA_GFXULTRA)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( at_motherboard )
	MCFG_MACHINE_START_OVERRIDE(at_state, at )
	MCFG_MACHINE_RESET_OVERRIDE(at_state, at )

	MCFG_PIT8254_ADD( "pit8254", at_pit8254_config )

	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, at_dma8237_1_config )
	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, at_dma8237_2_config )

	MCFG_PIC8259_ADD( "pic8259_master", at_pic8259_master_config )
	MCFG_PIC8259_ADD( "pic8259_slave", at_pic8259_slave_config )

	MCFG_AT_KEYBOARD_CONTROLLER_ADD("keybc", XTAL_12MHz, keyboard_controller_intf)
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL, NULL)

	MCFG_MC146818_IRQ_ADD( "rtc", MC146818_STANDARD, at_mc146818_config )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* video hardware */
	MCFG_PALETTE_LENGTH( 256 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ibm5170, at_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, XTAL_12MHz/2 /*6000000*/)
	MCFG_CPU_PROGRAM_MAP(at16_map)
	MCFG_CPU_IO_MAP(at16_io)
	MCFG_CPU_CONFIG(i286_address_mask)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_FRAGMENT_ADD( at_motherboard )
	MCFG_ISA16_BUS_ADD("isabus", "maincpu", isabus_intf)
	MCFG_ISA16_SLOT_ADD("isabus","isa1", pc_isa16_cards, "ega", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa2", pc_isa16_cards, "fdc", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa3", pc_isa16_cards, "comat", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa4", pc_isa16_cards, "ide", NULL, false)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5170")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1664K")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,16M")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ibm5170a, ibm5170 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(XTAL_16MHz/2)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ec1849, ibm5170 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(12000000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ibm5162, at_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, 6000000 /*6000000*/)
	MCFG_CPU_PROGRAM_MAP(at16_map)
	MCFG_CPU_IO_MAP(at16_io)
	MCFG_CPU_CONFIG(i286_address_mask)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_FRAGMENT_ADD( at_motherboard )

	MCFG_ISA16_BUS_ADD("isabus", "maincpu", isabus_intf)
	MCFG_ISA16_SLOT_ADD("isabus","isa1", pc_isa16_cards, "fdc", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa2", pc_isa16_cards, "ide", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa3", pc_isa16_cards, "comat", NULL, false)

	MCFG_FRAGMENT_ADD( pcvideo_cga )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1664K")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,16M")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( ps2m30286, at_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, 12000000)
	MCFG_CPU_PROGRAM_MAP(at16_map)
	MCFG_CPU_IO_MAP(at16_io)
	MCFG_CPU_CONFIG(i286_address_mask)

	MCFG_FRAGMENT_ADD( at_motherboard )

	MCFG_ISA16_BUS_ADD("isabus", "maincpu", isabus_intf)
	MCFG_ISA16_SLOT_ADD("isabus","isa1", pc_isa16_cards, "fdc", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa2", pc_isa16_cards, "ide", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa3", pc_isa16_cards, "comat", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa4", pc_isa16_cards, "svga_et4k", NULL, false)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1664K")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,16M")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( neat, at_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, 12000000)
	MCFG_CPU_PROGRAM_MAP(neat_map)
	MCFG_CPU_IO_MAP(neat_io)
	MCFG_CPU_CONFIG(i286_address_mask)

	MCFG_FRAGMENT_ADD( at_motherboard )

	MCFG_ISA16_BUS_ADD("isabus", "maincpu", isabus_intf)
	MCFG_ISA16_SLOT_ADD("isabus", "isa1", pc_isa16_cards, "fdc", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus", "isa2", pc_isa16_cards, "ide", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus", "isa3", pc_isa16_cards, "comat", NULL, false)

	MCFG_FRAGMENT_ADD( pcvideo_cga )

	MCFG_CS8221_ADD("cs8221", "maincpu", "isa", "bios")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1664K")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,16M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( atvga, at_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, 12000000)
	MCFG_CPU_PROGRAM_MAP(at16_map)
	MCFG_CPU_IO_MAP(at16_io)
	MCFG_CPU_CONFIG(i286_address_mask)

	MCFG_FRAGMENT_ADD( at_motherboard )

	MCFG_ISA16_BUS_ADD("isabus", "maincpu", isabus_intf)
	MCFG_ISA16_SLOT_ADD("isabus","isa1", pc_isa16_cards, "fdcsmc", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa2", pc_isa16_cards, "ide", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa3", pc_isa16_cards, "comat", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa4", pc_isa16_cards, "ne2000", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa5", pc_isa16_cards, "svga_et4k", NULL, false)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1664K")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,16M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( xb42639, at_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, 12500000)
	MCFG_CPU_PROGRAM_MAP(at16_map)
	MCFG_CPU_IO_MAP(at16_io)
	MCFG_CPU_CONFIG(i286_address_mask)

	MCFG_FRAGMENT_ADD( at_motherboard )

	MCFG_ISA16_BUS_ADD("isabus", "maincpu", isabus_intf)
	MCFG_ISA16_SLOT_ADD("isabus","isa1", pc_isa16_cards, "fdc", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa2", pc_isa16_cards, "ide", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa3", pc_isa16_cards, "comat", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa4", pc_isa16_cards, "svga_et4k", NULL, false)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1664K")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,16M")
MACHINE_CONFIG_END

//-------------------------------------------------
//  DEVICE_INPUT_DEFAULTS( ide_2nd)
//-------------------------------------------------

static DEVICE_INPUT_DEFAULTS_START( ide_2nd )
	DEVICE_INPUT_DEFAULTS("DSW", 0x01, 0x01)
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_START( at386, at_state )
	MCFG_CPU_ADD("maincpu", I386, 12000000)
	MCFG_CPU_PROGRAM_MAP(at386_map)
	MCFG_CPU_IO_MAP(at386_io)

	MCFG_FRAGMENT_ADD( at_motherboard )
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_ISA16_BUS_ADD("isabus", "maincpu", isabus_intf)
	// on board devices
	MCFG_ISA16_SLOT_ADD("isabus","board1", pc_isa16_cards, "fdcsmc", NULL, true)
	MCFG_ISA16_SLOT_ADD("isabus","board2", pc_isa16_cards, "comat", NULL, true)
	MCFG_ISA16_SLOT_ADD("isabus","board3", pc_isa16_cards, "ide", NULL, true)
	MCFG_ISA16_SLOT_ADD("isabus","board4", pc_isa16_cards, "lpt", NULL, true)
	// ISA cards
	MCFG_ISA16_SLOT_ADD("isabus","isa1", pc_isa16_cards, "svga_et4k", NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa2", pc_isa16_cards, NULL, NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa3", pc_isa16_cards, NULL, NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa4", pc_isa16_cards, NULL, NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus","isa5", pc_isa16_cards, "ide_cd", ide_2nd, false) //2nd-ary IDE

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1664K")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,15M,16M,32M,64M")

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( at486, at386 )
	MCFG_CPU_REPLACE("maincpu", I486, 25000000)
	MCFG_CPU_PROGRAM_MAP(at386_map)
	MCFG_CPU_IO_MAP(at386_io)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ct486, at386 )
	MCFG_CPU_REPLACE("maincpu", I486, 25000000)
	MCFG_CPU_PROGRAM_MAP(ct486_map)
	MCFG_CPU_IO_MAP(ct486_io)

	MCFG_CS4031_ADD("cs4031", "maincpu", "isa", "bios")

	MCFG_DEVICE_REMOVE(RAM_TAG)
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("1M,2M,8M,16M,32M,64M")
MACHINE_CONFIG_END


const struct i82439tx_interface tx_config =
{
	"maincpu",
	"isa"
};

static SLOT_INTERFACE_START( pci_devices )
	SLOT_INTERFACE_INTERNAL("i82439tx", I82439TX)
	SLOT_INTERFACE_INTERNAL("i82371ab", I82371AB)
	SLOT_INTERFACE_INTERNAL("i82371sb", I82371SB)
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( at586, at586_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 60000000)
	MCFG_CPU_PROGRAM_MAP(at586_map)
	MCFG_CPU_IO_MAP(at586_io)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("1M,2M,8M,16M,32M,64M")

	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE("pcibus:0", pci_devices, "i82439tx", NULL, &tx_config, 0, true)
	MCFG_PCI_BUS_DEVICE("pcibus:1", pci_devices, "i82371ab", NULL, NULL,	   0, true)

	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371ab:isabus","isa1", pc_isa16_cards, "svga_et4k", NULL, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371ab:isabus","isa2", pc_isa16_cards, NULL, NULL, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371ab:isabus","isa3", pc_isa16_cards, NULL, NULL, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371ab:isabus","isa4", pc_isa16_cards, NULL, NULL, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371ab:isabus","isa5", pc_isa16_cards, "ide_cd", ide_2nd, false) //2nd-ary IDE
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( at586x3, at586_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 60000000)
	MCFG_CPU_PROGRAM_MAP(at586_map)
	MCFG_CPU_IO_MAP(at586_io)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("1M,2M,8M,16M,32M,64M")

	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE("pcibus:0", pci_devices, "i82439tx", NULL, &tx_config, 0, true)
	MCFG_PCI_BUS_DEVICE("pcibus:1", pci_devices, "i82371sb", NULL, NULL,	   0, true)

	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371sb:isabus","isa1", pc_isa16_cards, "svga_et4k", NULL, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371sb:isabus","isa2", pc_isa16_cards, NULL, NULL, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371sb:isabus","isa3", pc_isa16_cards, NULL, NULL, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371sb:isabus","isa4", pc_isa16_cards, NULL, NULL, false)
	MCFG_ISA16_SLOT_ADD(":pcibus:1:i82371sb:isabus","isa5", pc_isa16_cards, "ide_cd", ide_2nd, false) //2nd-ary IDE
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( c386sx16, at386 )
	MCFG_CPU_REPLACE("maincpu", I386, 16000000)		/* 386SX */
	MCFG_CPU_PROGRAM_MAP(at386_map)
	MCFG_CPU_IO_MAP(at386_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( megapc, at386 )
	MCFG_CPU_REPLACE("maincpu", I386, XTAL_50MHz / 2)
	MCFG_CPU_PROGRAM_MAP(at386_map)
	MCFG_CPU_IO_MAP(megapc_io)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","megapc")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( megapcpl, megapc )
	MCFG_CPU_REPLACE("maincpu", I486, 66000000 / 2)
	MCFG_CPU_PROGRAM_MAP(at386_map)
	MCFG_CPU_IO_MAP(megapc_io)
MACHINE_CONFIG_END



#if 0
	// ibm at
	// most likely 2 32 kbyte chips for 16 bit access
	ROM_LOAD("atbios.bin", 0xf0000, 0x10000, CRC(674426be)) // BASIC C1.1, beeps
	// split into 2 chips for 16 bit access
	ROM_LOAD_EVEN("ibmat.0", 0xf0000, 0x8000, CRC(4995be7a))
	ROM_LOAD_ODD("ibmat.1", 0xf0000, 0x8000, CRC(c32713e4))

	/* I know about a 1984 version in 2 32kb roms */

	/* at, ami bios and diagnostics */
	ROM_LOAD_EVEN("rom01.bin", 0xf0000, 0x8000, CRC(679296a7))
	ROM_LOAD_ODD("rom02.bin", 0xf0000, 0x8000, CRC(65ae1f97))

	/* */
	ROM_LOAD("neat286.bin", 0xf0000, 0x10000, CRC(07985d9b))
	// split into 2 chips for 16 bit access
	ROM_LOAD_EVEN("neat.0", 0xf0000, 0x8000, CRC(4c36e61d))
	ROM_LOAD_ODD("neat.1", 0xf0000, 0x8000, CRC(4e90f294))

	/* most likely 1 chip!, for lower costs */
	ROM_LOAD("at386.bin", 0xf0000, 0x10000, CRC(3df9732a))

	/* at486 */
	ROM_LOAD("at486.bin", 0xf0000, 0x10000, CRC(31214616))

	ROM_LOAD("", 0x??000, 0x2000, CRC())
#endif

ROM_START( ibm5170 )
	ROM_REGION(0x100000,"maincpu", 0)

	ROM_SYSTEM_BIOS( 0, "rev1", "IBM PC/AT 5170 01/10/84")
	ROMX_LOAD("6181028.u27", 0xf0000, 0x8000, CRC(f6573f2a) SHA1(3e52cfa6a6a62b4e8576f4fe076c858c220e6c1a), ROM_SKIP(1) | ROM_BIOS(1)) /* T 6181028 8506AAA // TMM23256P-5878 // (C)IBM CORP 1981,-1984 */
	ROMX_LOAD("6181029.u47", 0xf0001, 0x8000, CRC(7075fbb2) SHA1(a7b885cfd38710c9bc509da1e3ba9b543a2760be), ROM_SKIP(1) | ROM_BIOS(1)) /* T 6181029 8506AAA // TMM23256P-5879 // (C)IBM CORP 1981,-1984 */

	ROM_SYSTEM_BIOS( 1, "rev2", "IBM PC/AT 5170 06/10/85")	/* Another verifaction of these crcs would be nice */
	ROMX_LOAD("6480090.u27", 0xf0000, 0x8000, CRC(99703aa9) SHA1(18022e93a0412c8477e58f8c61a87718a0b9ab0e), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("6480091.u47", 0xf0001, 0x8000, CRC(013ef44b) SHA1(bfa15d2180a1902cb6d38c6eed3740f5617afd16), ROM_SKIP(1) | ROM_BIOS(2))

//  ROM_SYSTEM_BIOS( 2, "atdiag", "IBM PC/AT 5170 w/Super Diagnostics")
//  ROMX_LOAD("atdiage.bin", 0xf8000, 0x4000, CRC(e8855d0c) SHA1(c9d53e61c08da0a64f43d691bf6cadae5393843a), ROM_SKIP(1) | ROM_BIOS(3))
//  ROMX_LOAD("atdiago.bin", 0xf8001, 0x4000, CRC(606fa71d) SHA1(165e45bae7ae2da274f1e645c763c5bfcbde027b), ROM_SKIP(1) | ROM_BIOS(3))

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD("1501824_717750.mmipal14l4.u87.jed", 0x0000, 0x02E7, CRC(3C819A27) SHA1(D2F4889E628DBBEF50B7F48CB1D1A313232BACC8)) /* MMI 1501824 717750 // (C)1983 IBM(M) */
	ROM_LOAD("1503135_705075.mmipal14l4.u130.jed", 0x02E7, 0x02E7, CRC(AAC77198) SHA1(B318DA3A1FBE5402836C1B548E231E0794D0C032)) /* MMI 1503135 705075 // (C) IBM CORP 83 */
	/* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD("1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD("1501814.82s123an.u115", 0x0000, 0x0020, CRC(849C9217) SHA1(2955AE1705C3B59170F1373F99B3EA5C174C4544)) /* N82S123AN 8713 // SK-D 1501814 */
	ROM_LOAD("55x8041.82s147an.u72", 0x0020, 0x0200, CRC(F2CC4FE6) SHA1(E285468516BD05083155A8A272583DEEF655315A)) /* S N82S147AN 8709 // V-C55X8041 */
ROM_END

ROM_START( ec1849 )
	ROM_REGION16_LE(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "cpu-card_27c256_015.rom", 0xf0000, 0x8000, CRC(68eadf0a) SHA1(903a7f1c3ebc6b27c31b512b2908c483608b5c13))
	ROM_LOAD16_BYTE( "cpu-card_27c256_016.rom", 0xf0001, 0x8000, CRC(bc3924d6) SHA1(596be415e6c2bc4ff30a187f146664531565712c))
	ROM_LOAD16_BYTE( "video-card_573rf6( 2764)_040.rom", 0xc0001, 0x2000, CRC(a3ece315) SHA1(e800e11c3b1b6fcaf41bfb7d4058a9d34fdd2b3f))
	ROM_LOAD16_BYTE( "video-card_573rf6( 2764)_041.rom", 0xc0000, 0x2000, CRC(b0a2ba7f) SHA1(c8160e8bc97cd391558f1dddd3fd3ec4a19d030c))

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD("cga.chr",     0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))

	ROM_REGION(0x50000, "gfx2", ROMREGION_ERASE00)
ROM_END

ROM_START( ibm5170a )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "rev3", "IBM PC/AT 5170 11/15/85")
	ROMX_LOAD("61x9266.u27", 0xf0000, 0x8000, CRC(4995be7a) SHA1(8e8e5c863ae3b8c55fd394e345d8cca48b6e575c), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("61x9265.u47", 0xf0001, 0x8000, CRC(c32713e4) SHA1(22ed4e2be9f948682891e2fd056a97dbea01203c), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 1, "3270at", "IBM 3270 PC/AT 5281 11/15/85") /* pretty much just a part string and checksum change from the 5170 rev3 */
	ROMX_LOAD("62x0820.u27", 0xf0000, 0x8000, CRC(e9cc3761) SHA1(ff9373c1a1f34a32fb6acdabc189c61b01acf9aa), ROM_SKIP(1) | ROM_BIOS(2)) /* T 62X0820-U27 8714HAK // TMM23256P-6746 // (C)IBM CORP 1981,-1985 */
	ROMX_LOAD("62x0821.u47", 0xf0001, 0x8000, CRC(b5978ccb) SHA1(2a1aeb9ae3cd7e60fc4c383ca026208b82156810), ROM_SKIP(1) | ROM_BIOS(2)) /* T 62X0821-U47 8715HAK // TMM23256P-6747 // (C)IBM CORP 1981,-1985 */

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD("1501824_717750.mmipal14l4.u87.jed", 0x0000, 0x02E7, CRC(3C819A27) SHA1(D2F4889E628DBBEF50B7F48CB1D1A313232BACC8)) /* MMI 1501824 717750 // (C)1983 IBM(M) */
	ROM_LOAD("1503135_705075.mmipal14l4.u130.jed", 0x02E7, 0x02E7, CRC(AAC77198) SHA1(B318DA3A1FBE5402836C1B548E231E0794D0C032)) /* MMI 1503135 705075 // (C) IBM CORP 83 */	/* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD("1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD("1501814.82s123an.u115", 0x0000, 0x0020, CRC(849C9217) SHA1(2955AE1705C3B59170F1373F99B3EA5C174C4544)) /* N82S123AN 8713 // SK-D 1501814 */
	ROM_LOAD("55x8041.82s147an.u72", 0x0020, 0x0200, CRC(F2CC4FE6) SHA1(E285468516BD05083155A8A272583DEEF655315A)) /* S N82S147AN 8709 // V-C55X8041 */
ROM_END


ROM_START( ibm5162 ) //MB p/n 62x1168
	ROM_REGION16_LE(0x1000000,"maincpu", 0)

	ROM_LOAD16_BYTE("78x7460.u34", 0xf0000, 0x8000, CRC(1db4bd8f) SHA1(7be669fbb998d8b4626fefa7cd1208d3b2a88c31)) /* 78X7460 U34 // (C) IBM CORP // 1981-1986 */
	ROM_LOAD16_BYTE("78x7461.u35", 0xf0001, 0x8000, CRC(be14b453) SHA1(ec7c10087dbd53f9c6d1174e8f14212e2aec1818)) /* 78X7461 U35 // (C) IBM CORP // 1981-1986 */

	/* Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f))

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD("59x7599.mmipal20l8.u27.jed", 0x0000, 0x02E7, NO_DUMP) /* MMI PAL20L8ACN5 8631 // N59X7599 IBM (C)85 K3 */
	ROM_LOAD("1503135.mmipal14l4.u81.jed", 0x02E7, 0x02E7, CRC(AAC77198) SHA1(B318DA3A1FBE5402836C1B548E231E0794D0C032)) /* MMI 1503135 8625 // (C) IBM CORP 83 */
	/* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD("1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD("1501814.82s123an.u72", 0x0000, 0x0020, CRC(849C9217) SHA1(2955AE1705C3B59170F1373F99B3EA5C174C4544)) /* N82S123AN 8623 // SK-U 1501814 */
	ROM_LOAD("59x7594.82s147an.u90", 0x0020, 0x0200, NO_DUMP) /* S N82S147AN 8629 // VCT 59X7594 */
ROM_END


ROM_START( i8530286 )
	ROM_REGION(0x1000000,"maincpu", 0)
	// saved from running machine
	ROM_LOAD16_BYTE("ps2m30.0", 0xe0000, 0x10000, CRC(9965a634) SHA1(c237b1760f8a4561ec47dc70fe2e9df664e56596))
	ROM_RELOAD(0xfe0000,0x10000)
	ROM_LOAD16_BYTE("ps2m30.1", 0xe0001, 0x10000, CRC(1448d3cb) SHA1(13fa26d895ce084278cd5ab1208fc16c80115ebe))
	ROM_RELOAD(0xfe0001,0x10000)
ROM_END

/*

8530-H31 (Model 30/286)
======================
  P/N          Date
33F5381A EC C01446 1990

*/
ROM_START( i8530h31 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD( "33f5381a.bin", 0xe0000, 0x20000, CRC(ff57057d) SHA1(d7f1777077a8df43c3c14d175b9709bd3969c4b1))
ROM_END

/*
8535-043 (Model 35)
===================
  P/N    Checksum     Date
04G2021    C26C       1991    ODD
04G2022    9B94       1991    EVEN
*/
ROM_START( i8535043 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "04g2021.bin", 0xe0001, 0x10000, CRC(4069b2eb) SHA1(9855c84c81d1f07e1da66b1ca45c1c10c0717a90))
	ROM_RELOAD(0xfe0001, 0x10000)
	ROM_LOAD16_BYTE( "04g2022.bin", 0xe0000, 0x10000, CRC(35c1af65) SHA1(7d2445cc463969c808fdd78e0a27a03db5dfc698))
	ROM_RELOAD(0xfe0000, 0x10000)
ROM_END

/*
8550-021 (Model 50)
===================
 Code     Date       Internal
90X7420  4/12/87 --> 90X6815
90X7423  8/12/87 --> 90X6816
90X7426  8/12/87 --> 90X6817
90X7429 18/12/87 --> 90X6818

Same ROMs used by : (According to http://www.ibmmuseum.com/ohlandl/8565/8560.html)

IBM Personal System/2 Model 60 (8560-041 and 8560-071)
IBM Personal System/2 Model 65 SX (8565-061 and 8565-121)

*/
ROM_START( i8550021 )
    ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "90x7423.zm14", 0xe0000, 0x8000, CRC(2c1633e0) SHA1(1af7faa526585a7cfb69e71d90a75e1f1c541586))
	ROM_RELOAD(0xfe0000, 0x8000)
	ROM_LOAD16_BYTE( "90x7426.zm16", 0xe0001, 0x8000, CRC(e7c762ce) SHA1(228f67dc915d84519da7fc1a59b7f9254278f3a0))
	ROM_RELOAD(0xfe0001, 0x8000)
	ROM_LOAD16_BYTE( "90x7420.zm13", 0xf0001, 0x8000, CRC(19a57cc1) SHA1(5b31ba66cd3690e651a450619a32b7210769945d))
	ROM_RELOAD(0xff0001, 0x8000)
	ROM_LOAD16_BYTE( "90x7429.zm18", 0xf0000, 0x8000, CRC(6f0120f6) SHA1(e112c291ac3d9f6507c93ac49ad26f9fd2245fd2))
	ROM_RELOAD(0xff0000, 0x8000)

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "72x8455.zm82", 0x000, 0x800, CRC(7da223d3) SHA1(54c52ff6c6a2310f79b2c7e6d1259be9de868f0e) )
ROM_END

/*
8550-061 (Model 50Z)
===================
                  P/N              Date
AMI 8935MKN     15F8365    S63512  1988
AMI 8948MML     15F8366    S63512  1988

http://ps-2.kev009.com:8081/ohlandl/8550/8550z_Planar.html


*/
ROM_START( i8550061 )
    ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "15f8365.zm5", 0xe0001, 0x10000, CRC(35aa3ecf) SHA1(a122531092a9cb08600b276da9c9c3ce385aab7b))
	ROM_RELOAD(0xfe0001, 0x10000)
	ROM_LOAD16_BYTE( "15f8366.zm6", 0xe0000, 0x10000, CRC(11bf564d) SHA1(0dda6a7ca9294cfaab5bdf4c05973be13b2766fc))
	ROM_RELOAD(0xfe0000, 0x10000)
ROM_END

/*
8555-X61 (Model 55SX)
===================
         Code     Date       Internal
ODD     33F8145  13/03/90 --> 33F8153
EVEN    33F8146  31/01/90 --> 33F8152

8555-081 (Model 55SX)
===================
                         Code          Date    Internal
ODD     AMI 9205MEN     92F0627 EC32680 88 --> 33F8153
EVEN    AMI 9203MGS     92F0626 EC32680 88 --> 33F8152

*/
ROM_START( i8555081 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE("33f8145.zm40", 0xe0001, 0x10000, CRC(0895894c) SHA1(7cee77828867ad1bdbe0ac223bc25d23c65b28a0))
	ROM_RELOAD(0xfe0001, 0x10000)
	ROM_LOAD16_BYTE("33f8146.zm41", 0xe0000, 0x10000, CRC(c6020680) SHA1(b25a64e4b2dca07c567648401100e04e89bbcddb))
	ROM_RELOAD(0xfe0000, 0x10000)
ROM_END

/*
8580-071 (Model 80)
===================
                  Code    Date      Internal
AMI 8924MBW     90X8548   1987  --> 72X7551
AMI 8924MBL     90X8549   1987  --> 72X7554
AMI 8924MBG     90X8550   1987  --> 72X7557
AMI 8921MBK     90X8551   1987  --> 72X7560
*/
ROM_START( i8580071 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD( "90x8548.bin", 0xe0000, 0x8000, CRC(1f13eea5) SHA1(0bf53ad86f47db3825a713ea2e4ef23715cc4f79))
	ROM_LOAD( "90x8549.bin", 0xe0001, 0x8000, CRC(9e0f4a99) SHA1(b8600f04159ed281a57416274390ba9302be541b))
	ROM_LOAD( "90x8550.bin", 0xf0000, 0x8000, CRC(cb21df96) SHA1(0c2765f6becfa3f9171c4f13f7b74d19c4c9acc2))
	ROM_LOAD( "90x8551.bin", 0xf0001, 0x8000, CRC(3d7e9868) SHA1(2928fe0e48a573cc2c0c41bd7f7188a54a908229))
ROM_END

/*
8580-111 (Model 80)
===================
                 Code    Date    Internal
AMI 8934MDL     15F6637  1987 --> 15F6597
AMI 8944MDI     15F6639  1987 --> 15F6600
*/
ROM_START( i8580111 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "15f6637.bin", 0xe0000, 0x10000, CRC(76c36d1a) SHA1(c68d52a2e5fbd303225ebb006f91869b29ef700a))
	ROM_LOAD16_BYTE( "15f6639.bin", 0xe0001, 0x10000, CRC(82cf0f7d) SHA1(13bb39225757b89749af70e881af0228673dbe0c))
ROM_END

ROM_START( at )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "ami211", "AMI 21.1") /*(Motherboard Manufacturer: Dataexpert Corp. Motherboard) (Neat 286 Bios, 82c21x Chipset ) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "ami211.bin",	 0xf0000, 0x10000,CRC(a0b5d269) SHA1(44db8227d35a09e39b93ed944f85dcddb0dd0d39), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "at", "PC 286") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 03-11-1987)*/
	ROMX_LOAD("at110387.1", 0xf0001, 0x8000, CRC(679296a7) SHA1(ae891314cac614dfece686d8e1d74f4763cf40e3),ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD("at110387.0", 0xf0000, 0x8000, CRC(65ae1f97) SHA1(91a29c7deecf7a9afbba330e64e0eee9aafee4d1),ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "ami206", "AMI C 206.1")  /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 15-10-1990)*/
	ROMX_LOAD( "amic206.bin",	 0xf0000, 0x10000,CRC(25a67c34) SHA1(91e9d8cdc2f1b40a601a23ceaff2189fd1245f3b), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "amic21", "AMI C 21.1") /* might be bad dump, doesn't start */
	ROMX_LOAD( "amic21-1.bin",  0xf0001, 0x8000, CRC(5644ed38) SHA1(963555ec77845defc3b42b433280908e1797076e),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "amic21-2.bin",  0xf0000, 0x8000, CRC(8ffe7752) SHA1(68215f07a170ee7bdcb3e52b370d470af1741f7e),ROM_SKIP(1) | ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(4, "ami101", "AMI HT 101.1") /* Quadtel Enhanced 286 Bios Version 3.04.02 */
	ROMX_LOAD( "amiht-h.bin",   0xf0001, 0x8000, CRC(8022545f) SHA1(42541d4392ad00b0e064b3a8ccf2786d875c7c19),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "amiht-l.bin",   0xf0000, 0x8000, CRC(285f6b8f) SHA1(2fce4ec53b68c9a7580858e16c926dc907820872),ROM_SKIP(1) | ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(5, "ami121", "AMI HT 12.1") /* might be bad dump, doesn't start */
	ROMX_LOAD( "ami2ev86.bin",  0xf0001, 0x8000, CRC(55deb5c2) SHA1(19ce1a7cc985b5895c585e39211475de2e3b0dd1),ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "ami2od86.bin",  0xf0000, 0x8000, CRC(04a2cec4) SHA1(564d37a8b2c0f4d0e23cd1e280a09d47c9945da8),ROM_SKIP(1) | ROM_BIOS(6) )
	ROM_SYSTEM_BIOS(6, "ami122", "AMI HT 12.2") /* might be bad dump, doesn't start */
	ROMX_LOAD( "ami2ev89.bin",  0xf0001, 0x8000, CRC(705d36e0) SHA1(0c9cfb71ced4587f109b9b6dfc2a9c92302fdb99),ROM_SKIP(1) | ROM_BIOS(7) )
	ROMX_LOAD( "ami2od89.bin",  0xf0000, 0x8000, CRC(7c81bbe8) SHA1(a2c7eca586f6e2e76b9101191e080a1f1cb8b833),ROM_SKIP(1) | ROM_BIOS(7) )
	ROM_SYSTEM_BIOS(7, "ami123", "AMI HT 12.3") /*(Motherboard Manufacturer: Aquarius Systems USA Inc.) (BIOS release date:: 13-06-1990)*/
	ROMX_LOAD( "ht12h.bin",     0xf0001, 0x8000, CRC(db8b471e) SHA1(7b5fa1c131061fa7719247db3e282f6d30226778),ROM_SKIP(1) | ROM_BIOS(8) )
	ROMX_LOAD( "ht12l.bin",     0xf0000, 0x8000, CRC(74fd178a) SHA1(97c8283e574abbed962b701f3e8091fb82823b80),ROM_SKIP(1) | ROM_BIOS(8) )
	ROM_SYSTEM_BIOS(8, "ami181", "AMI HT 18.1") /* might be bad dump, doesn't start */
	ROMX_LOAD( "ht18.bin",     0xf0000, 0x10000, CRC(f65a6f9a) SHA1(7dfdf7d243f9f645165dc009c5097dd515f86fbb), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS(9, "amiht21", "AMI HT 21.1") /* might be bad dump, doesn't start */
	ROMX_LOAD( "ht21e.bin",    0xf0000, 0x10000, CRC(e80f7fed) SHA1(62d958d98c95e9e4d1b290a6c1054ae98770f276), ROM_BIOS(10) )
	ROM_SYSTEM_BIOS(10, "amip1", "AMI P.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "poisk-h.bin",   0xf0001, 0x8000, CRC(83fd3f8c) SHA1(ca94850bbd949b97b11710629886b0ee69489a81),ROM_SKIP(1) | ROM_BIOS(11) )
	ROMX_LOAD( "poisk-l.bin",   0xf0000, 0x8000, CRC(0b2ed291) SHA1(bb51a3f317cf4d429a6cfb44a46ca0ac39d9aaa7),ROM_SKIP(1) | ROM_BIOS(11) )
	ROM_SYSTEM_BIOS(11, "aw201", "Award 201")
	ROMX_LOAD( "83201-5h.bin",  0xf0001, 0x8000, CRC(968d1fc0) SHA1(dc4122a6c696f0b43e7894dc1b669346eed755d5),ROM_SKIP(1) | ROM_BIOS(12) )
	ROMX_LOAD( "83201-5l.bin",  0xf0000, 0x8000, CRC(bf50a89a) SHA1(2349a1db6017a7fb0673e99d3680c8753407be8d),ROM_SKIP(1) | ROM_BIOS(12) )
	ROM_SYSTEM_BIOS(12, "aw303", "Award 303")
	ROMX_LOAD( "aw303-hi.bin",  0xf8001, 0x4000, CRC(78f32d7e) SHA1(1c88398fb171b33b7e6191bad63704ae85bfed8b), ROM_SKIP(1) | ROM_BIOS(13) )
	ROMX_LOAD( "aw303-lo.bin",  0xf8000, 0x4000, CRC(3d2a70c0) SHA1(1329113bec514ed2a6d803067b1132744ef534dd), ROM_SKIP(1) | ROM_BIOS(13) )
	/* Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f))

	ROM_REGION(0x50000, "gfx2", ROMREGION_ERASE00)
ROM_END

ROM_START( cmdpc30 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROMX_LOAD( "commodore pc 30 iii even.bin", 0xf8000, 0x4000, CRC(36307aa9) SHA1(50237ffea703b867de426ab9ebc2af46bac1d0e1),ROM_SKIP(1))
	ROMX_LOAD( "commodore pc 30 iii odd.bin",  0xf8001, 0x4000, CRC(41bae42d) SHA1(27d6ad9554be86359d44331f25591e3122a31519),ROM_SKIP(1))
	/* Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f))

	ROM_REGION(0x50000, "gfx2", ROMREGION_ERASE00)
ROM_END

ROM_START( atvga )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "vl82c", "VL82C311L-FC4")/*(Motherboard Manufacturer: Biostar Microtech Corp.) (BIOS release date: 05-05-1991)*/
	ROMX_LOAD( "2vlm001.bin",     0xf0000, 0x10000, CRC(f34d800a) SHA1(638aca592a0e525f957beb525e95ca666a994ee8), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "ami211", "AMI 21.1") /*(Motherboard Manufacturer: Dataexpert Corp. Motherboard) (Neat 286 Bios, 82c21x Chipset ) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "ami211.bin",	 0xf0000, 0x10000,CRC(a0b5d269) SHA1(44db8227d35a09e39b93ed944f85dcddb0dd0d39), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "ami206", "AMI C 206.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 15-10-1990)*/
	ROMX_LOAD( "amic206.bin",	 0xf0000, 0x10000,CRC(25a67c34) SHA1(91e9d8cdc2f1b40a601a23ceaff2189fd1245f3b), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "amic21", "AMI C 21.1") /* might be bad dump, doesn't start */
	ROMX_LOAD( "amic21-1.bin",  0xf0001, 0x8000, CRC(5644ed38) SHA1(963555ec77845defc3b42b433280908e1797076e),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "amic21-2.bin",  0xf0000, 0x8000, CRC(8ffe7752) SHA1(68215f07a170ee7bdcb3e52b370d470af1741f7e),ROM_SKIP(1) | ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(4, "ami101", "AMI HT 101.1") /* Quadtel Enhanced 286 Bios Version 3.04.02 */
	ROMX_LOAD( "amiht-h.bin",   0xf0001, 0x8000, CRC(8022545f) SHA1(42541d4392ad00b0e064b3a8ccf2786d875c7c19),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "amiht-l.bin",   0xf0000, 0x8000, CRC(285f6b8f) SHA1(2fce4ec53b68c9a7580858e16c926dc907820872),ROM_SKIP(1) | ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(5, "ami121", "AMI HT 12.1") /* might be bad dump, doesn't start */
	ROMX_LOAD( "ami2ev86.bin",  0xf0001, 0x8000, CRC(55deb5c2) SHA1(19ce1a7cc985b5895c585e39211475de2e3b0dd1),ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "ami2od86.bin",  0xf0000, 0x8000, CRC(04a2cec4) SHA1(564d37a8b2c0f4d0e23cd1e280a09d47c9945da8),ROM_SKIP(1) | ROM_BIOS(6) )
	ROM_SYSTEM_BIOS(6, "ami122", "AMI HT 12.2") /* might be bad dump, doesn't start */
	ROMX_LOAD( "ami2ev89.bin",  0xf0001, 0x8000, CRC(705d36e0) SHA1(0c9cfb71ced4587f109b9b6dfc2a9c92302fdb99),ROM_SKIP(1) | ROM_BIOS(7) )
	ROMX_LOAD( "ami2od89.bin",  0xf0000, 0x8000, CRC(7c81bbe8) SHA1(a2c7eca586f6e2e76b9101191e080a1f1cb8b833),ROM_SKIP(1) | ROM_BIOS(7) )
	ROM_SYSTEM_BIOS(7, "ami123", "AMI HT 12.3") /*(Motherboard Manufacturer: Aquarius Systems USA Inc.) (BIOS release date:: 13-06-1990)*/
	ROMX_LOAD( "ht12h.bin",     0xf0001, 0x8000, CRC(db8b471e) SHA1(7b5fa1c131061fa7719247db3e282f6d30226778),ROM_SKIP(1) | ROM_BIOS(8) )
	ROMX_LOAD( "ht12l.bin",     0xf0000, 0x8000, CRC(74fd178a) SHA1(97c8283e574abbed962b701f3e8091fb82823b80),ROM_SKIP(1) | ROM_BIOS(8) )
	ROM_SYSTEM_BIOS(8, "ami181", "AMI HT 18.1") /* might be bad dump, doesn't start */
	ROMX_LOAD( "ht18.bin",     0xf0000, 0x10000, CRC(f65a6f9a) SHA1(7dfdf7d243f9f645165dc009c5097dd515f86fbb), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS(9, "amiht21", "AMI HT 21.1") /* might be bad dump, doesn't start */
	ROMX_LOAD( "ht21e.bin",    0xf0000, 0x10000, CRC(e80f7fed) SHA1(62d958d98c95e9e4d1b290a6c1054ae98770f276), ROM_BIOS(10) )
	ROM_SYSTEM_BIOS(10, "amip1", "AMI P.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "poisk-h.bin",   0xf0001, 0x8000, CRC(83fd3f8c) SHA1(ca94850bbd949b97b11710629886b0ee69489a81),ROM_SKIP(1) | ROM_BIOS(11) )
	ROMX_LOAD( "poisk-l.bin",   0xf0000, 0x8000, CRC(0b2ed291) SHA1(bb51a3f317cf4d429a6cfb44a46ca0ac39d9aaa7),ROM_SKIP(1) | ROM_BIOS(11) )
	ROM_SYSTEM_BIOS(11, "ami1131", "AMI-1131") /*(Motherboard Manufacturer: Elitegroup Computer Co., Ltd.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "2hlm003h.bin",   0xf0001, 0x8000, CRC(2babb42b) SHA1(3da6538f44b434cdec0cbdddd392ccfd34666f06),ROM_SKIP(1) | ROM_BIOS(12) )
	ROMX_LOAD( "2hlm003l.bin",   0xf0000, 0x8000, CRC(317cbcbf) SHA1(1adad6280d8b07c2921fc5fc13ecaa10e6bfebdc),ROM_SKIP(1) | ROM_BIOS(12) )
	ROM_SYSTEM_BIOS(12, "at", "PC 286") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 03-11-1987)*/
	ROMX_LOAD("at110387.1", 0xf0001, 0x8000, CRC(679296a7) SHA1(ae891314cac614dfece686d8e1d74f4763cf40e3),ROM_SKIP(1) | ROM_BIOS(13) )
	ROMX_LOAD("at110387.0", 0xf0000, 0x8000, CRC(65ae1f97) SHA1(91a29c7deecf7a9afbba330e64e0eee9aafee4d1),ROM_SKIP(1) | ROM_BIOS(13) )
ROM_END

ROM_START( xb42639 )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x1000000, "maincpu", 0)
	// XEN-S (Venus I Motherboard)
	ROM_LOAD16_BYTE("3-10-17i.lo", 0xf0000, 0x8000, CRC(3786ca1e) SHA1(c682d7c76f234559d03bcf21010c13c4dbeafb69))
	ROM_RELOAD(0xff0000,0x8000)
	ROM_LOAD16_BYTE("3-10-17i.hi", 0xf0001, 0x8000, CRC(d66710eb) SHA1(e8c1cd5f9ecfbd8825655e416d7ddf2ae362e69b))
	ROM_RELOAD(0xff0001,0x8000)
ROM_END

ROM_START( xb42639a )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x1000000, "maincpu", 0)
	// XEN-S (Venus II Motherboard)
	ROM_LOAD16_BYTE("10217.lo", 0xf0000, 0x8000, CRC(ea53406f) SHA1(2958dfdbda14de4e6b9d6a8c3781131ab1e32bef))
	ROM_RELOAD(0xff0000,0x8000)
	ROM_LOAD16_BYTE("10217.hi", 0xf0001, 0x8000, CRC(111725cf) SHA1(f6018a45bda4476d40c5881fb0a506ff75ec1688))
	ROM_RELOAD(0xff0001,0x8000)
ROM_END

ROM_START( xb42664 )
	/* actual VGA BIOS not dumped */
	ROM_REGION(0x1000000, "maincpu", 0)
	// XEN-S (Venus I Motherboard)
	ROM_LOAD16_BYTE("3-10-17i.lo", 0xf0000, 0x8000, CRC(3786ca1e) SHA1(c682d7c76f234559d03bcf21010c13c4dbeafb69))
	ROM_RELOAD(0xff0000,0x8000)
	ROM_LOAD16_BYTE("3-10-17i.hi", 0xf0001, 0x8000, CRC(d66710eb) SHA1(e8c1cd5f9ecfbd8825655e416d7ddf2ae362e69b))
	ROM_RELOAD(0xff0001,0x8000)
ROM_END

ROM_START( xb42664a )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x1000000, "maincpu", 0)
	// XEN-S (Venus II Motherboard)
	ROM_LOAD16_BYTE("10217.lo", 0xf0000, 0x8000, CRC(ea53406f) SHA1(2958dfdbda14de4e6b9d6a8c3781131ab1e32bef))
	ROM_RELOAD(0xff0000,0x8000)
	ROM_LOAD16_BYTE("10217.hi", 0xf0001, 0x8000, CRC(111725cf) SHA1(f6018a45bda4476d40c5881fb0a506ff75ec1688))
	ROM_RELOAD(0xff0001,0x8000)
ROM_END


ROM_START( neat )
	ROM_REGION(0x1000000,"maincpu", 0)
	//ROM_SYSTEM_BIOS(0, "neat286", "NEAT 286")
	ROM_LOAD16_BYTE("at030389.0", 0xf0000, 0x8000, CRC(4c36e61d) SHA1(094e8d5e6819889163cb22a2cf559186de782582))
	//ROM_RELOAD(0xff0000,0x8000)
	ROM_LOAD16_BYTE("at030389.1", 0xf0001, 0x8000, CRC(4e90f294) SHA1(18c21fd8d7e959e2292a9afbbaf78310f9cad12f))
	//ROM_RELOAD(0xff0001,0x8000)

	/* Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f))

	ROM_REGION(0x50000, "gfx2", ROMREGION_ERASE00)
ROM_END


ROM_START( at386 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "ami386", "AMI 386")
	ROMX_LOAD("ami386.bin",  0xf0000, 0x10000, CRC(3a807d7f) SHA1(8289ba36a3dfc3324333b1a834bc6b0402b546f0), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "at386", "unknown 386")	// This dump possibly comes from a MITAC INC 386 board, given that the original driver had it as manufacturer
	ROMX_LOAD("at386.bin",  0xf0000, 0x10000, CRC(3df9732a) SHA1(def71567dee373dc67063f204ef44ffab9453ead), ROM_BIOS(2))
	//ROM_RELOAD(0xff0000,0x10000)
	ROM_SYSTEM_BIOS(2, "neatsx", "NEATsx 386sx")
	ROMX_LOAD("012l-u25.bin", 0xf0000, 0x8000, CRC(4AB1862D) SHA1(D4E8D0FF43731270478CA7671A129080FF350A4F),ROM_SKIP(1) | ROM_BIOS(3) )
	//ROM_RELOAD(0xff0000,0x8000)
	ROMX_LOAD("012h-u24.bin", 0xf0001, 0x8000, CRC(17472521) SHA1(7588C148FE53D9DC4CB2D0AB6E0FD51A39BB5D1A),ROM_SKIP(1) | ROM_BIOS(3) )
	//ROM_RELOAD(0xff0000,0x8000)
	ROM_SYSTEM_BIOS(3, "amicg", "AMI CG")
	ROMX_LOAD( "amicg.1",		 0xf0000, 0x10000,CRC(8408965a) SHA1(9893d3ac851e01b06a68a67d3721df36ca2c96f5), ROM_BIOS(4) )
ROM_END


ROM_START( at486 )
	ROM_REGION(0x1000000, "maincpu", 0)

	ROM_SYSTEM_BIOS(0, "at486", "PC/AT 486")
	ROMX_LOAD("at486.bin",   0x0f0000, 0x10000, CRC(31214616) SHA1(51b41fa44d92151025fc9ad06e518e906935e689), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "mg48602", "UMC MG-48602")
	ROMX_LOAD("mg48602.bin", 0x0f0000, 0x10000, CRC(45797823) SHA1(a5fab258aecabde615e1e97af5911d6cf9938c11), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "ft01232", "Free Tech 01-232")
	ROMX_LOAD("ft01232.bin", 0x0f0000, 0x10000, CRC(30efaf92) SHA1(665c8ef05ca052dcc06bb473c9539546bfef1e86), ROM_BIOS(3))

	/* 486 boards from FIC

    naming convention
    xxxxx101 --> for EPROM
    xxxxx701 --> for EEPROM using a Flash Utility v5.02
    xxxxBxxx --> NS 311/312 IO Core Logic
    xxxxCxxx --> NS 332 IO Core Logic
    xxxxGxxx --> Winbond W83787F IO Core Logic
    xxxxJxxx --> Winbond W83877F IO Core Logic

    */

	/* this is the year 2000 beta bios from FIC, supports GIO-VT, GAC-V, GAC-2, VIP-IO, VIO-VP and GVT-2 */
	ROM_SYSTEM_BIOS(3, "ficy2k", "FIC 486 3.276GN1") /* 1997-06-16, includes CL-GD5429 VGA BIOS 1.00a */
	ROMX_LOAD("3276gn1.bin",  0x0e0000, 0x20000, CRC(d4ff0cc4) SHA1(567b6bdbc9bff306c8c955f275e01ae4c45fd5f2), ROM_BIOS(4))

	ROM_SYSTEM_BIOS(4, "ficgac2", "FIC 486-GAC-2") /* 1994-04-29, includes CL-GD542X VGA BIOS 1.50 */
	ROMX_LOAD("att409be.bin", 0x0e0000, 0x20000, CRC(c58e017b) SHA1(14c19e720ce62eb2afe28a70f4e4ebafab0f9e77), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "ficgacv", "FIC 486-GAC-V 3.27GN1") /* 1996-04-08, includes CL-GD542X VGA BIOS 1.41 */
	ROMX_LOAD("327gn1.awd",   0x0e0000, 0x20000, CRC(017614d4) SHA1(2228c28f21a7e78033d24319449297936465b164), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(6, "ficgiovp", "FIC 486-GIO-VP 3.15GN") /* 1994-05-06 */
	ROMX_LOAD("giovp315.rom", 0x0f0000, 0x10000, CRC(e102c3f5) SHA1(f15a7e9311cc17afe86da0b369607768b030ddec), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(7, "ficgiovt", "FIC 486-GIO-VT 3.06G") /* 1994-11-20 */
	ROMX_LOAD("306gcd00.awd", 0x0f0000, 0x10000, CRC(75f3ded4) SHA1(999d4b58204e0b0f33262d0613c855b528bf9597), ROM_BIOS(8))

	ROM_SYSTEM_BIOS(8, "ficgiovt2_326", "FIC 486-GIO-VT2 3.26G")  /* 1994-07-06 */
	ROMX_LOAD("326g1c00.awd", 0x0f0000, 0x10000, CRC(2e729ab5) SHA1(b713f97fa0e0b62856dab917f417f5b21020b354), ROM_BIOS(9))
	ROM_SYSTEM_BIOS(9, "ficgiovt2_3276", "FIC 486-GIO-VT2 3.276") /* 1997-07-17 */
	ROMX_LOAD("32760000.bin", 0x0f0000, 0x10000, CRC(ad179128) SHA1(595f67ba4a1c8eb5e118d75bf657fff3803dcf4f), ROM_BIOS(10))

	ROM_SYSTEM_BIOS(10, "ficgvt2", "FIC 486-GVT-2 3.07G") /* 1994-11-02 */
	ROMX_LOAD("3073.bin",     0x0f0000, 0x10000, CRC(a6723863) SHA1(ee93a2f1ec84a3d67e267d0a490029f9165f1533), ROM_BIOS(11))
	ROM_SYSTEM_BIOS(11, "ficgpak2", "FIC 486-PAK-2 5.15S") /* 1995-06-27, includes Phoenix S3 TRIO64 Enhanced VGA BIOS 1.4-01 */
	ROMX_LOAD("515sbd8a.awd", 0x0e0000, 0x20000, CRC(778247e1) SHA1(07d8f0f2464abf507be1e8dfa06cd88737782411), ROM_BIOS(12))

	ROM_SYSTEM_BIOS(12, "ficpio3g7", "FIC 486-PIO-3 1.15G705") /* pnp */
	ROMX_LOAD("115g705.awd",  0x0e0000, 0x20000, CRC(ddb1544a) SHA1(d165c9ecdc9397789abddfe0fef69fdf954fa41b), ROM_BIOS(13))
	ROM_SYSTEM_BIOS(13, "ficpio3g1", "FIC 486-PIO-3 1.15G105") /* non-pnp */
	ROMX_LOAD("115g105.awd",  0x0e0000, 0x20000, CRC(b327eb83) SHA1(9e1ff53e07ca035d8d43951bac345fec7131678d), ROM_BIOS(14))

	ROM_SYSTEM_BIOS(14, "ficpos", "FIC 486-POS")
	ROMX_LOAD("116di6b7.bin", 0x0e0000, 0x20000, CRC(d1d84616) SHA1(2f2b27ce100cf784260d8e155b48db8cfbc63285), ROM_BIOS(15))
	ROM_SYSTEM_BIOS(15, "ficpvt", "FIC 486-PVT 5.15")          /* 1995-06-27 */
	ROMX_LOAD("5150eef3.awd", 0x0e0000, 0x20000, CRC(eb35785d) SHA1(1e601bc8da73f22f11effe9cdf5a84d52576142b), ROM_BIOS(16))
	ROM_SYSTEM_BIOS(16, "ficpvtio", "FIC 486-PVT-IO 5.162W2")  /* 1995-10-05 */
	ROMX_LOAD("5162cf37.awd", 0x0e0000, 0x20000, CRC(378d813d) SHA1(aa674eff5b972b31924941534c3c988f6f78dc93), ROM_BIOS(17))
	ROM_SYSTEM_BIOS(17, "ficvipio426", "FIC 486-VIP-IO 4.26GN2") /* 1994-12-07 */
	ROMX_LOAD("426gn2.awd",   0x0e0000, 0x20000, CRC(5f472aa9) SHA1(9160abefae32b450e973651c052657b4becc72ba), ROM_BIOS(18))
	ROM_SYSTEM_BIOS(18, "ficvipio427", "FIC 486-VIP-IO 4.27GN2A") /* 1996-02-14 */
	ROMX_LOAD("427gn2a.awd",  0x0e0000, 0x20000, CRC(035ad56d) SHA1(0086db3eff711fc710b30e7f422fc5b4ab8d47aa), ROM_BIOS(19))
	ROM_SYSTEM_BIOS(19, "ficvipio2", "FIC 486-VIP-IO2")
	ROMX_LOAD("1164g701.awd", 0x0e0000, 0x20000, CRC(7b762683) SHA1(84debce7239c8b1978246688ae538f7c4f519d13), ROM_BIOS(20))

	ROM_SYSTEM_BIOS(20, "qdi", "QDI PX486DX33/50P3")
	ROMX_LOAD("qdi_px486.u23", 0x0f0000, 0x10000, CRC(c80ecfb6) SHA1(34cc9ef68ff719cd0771297bf184efa83a805f3e), ROM_BIOS(21))
ROM_END


// Unknown 486 board with Chips & Technologies CS4031 chipset
ROM_START( ct486 )
	ROM_REGION(0x40000, "isa", ROMREGION_ERASEFF)
	ROM_REGION(0x100000, "bios", 0)
	ROM_LOAD("chips_1.ami", 0xf0000, 0x10000, CRC(a14a7511) SHA1(b88d09be66905ed2deddc26a6f8522e7d2d6f9a8))
ROM_END


// FIC 486-PIO-2 (4 ISA, 4 PCI)
// VIA VT82C505 + VT82C496G + VT82C406MV, NS311/312 or NS332 I/O
ROM_START( ficpio2 )
	ROM_REGION(0x1000000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "ficpio2c7", "FIC 486-PIO-2 1.15C701") /* pnp, i/o core: NS 332 */
	ROMX_LOAD("115c701.awd",  0x0e0000, 0x20000, CRC(b0dd7975) SHA1(bfde13b0fbd141bc945d37d92faca9f4f59b716d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "ficpio2b7", "FIC 486-PIO-2 1.15B701") /* pnp, i/o core: NS 311/312 */
	ROMX_LOAD("115b701.awd",  0x0e0000, 0x20000, CRC(ac24abad) SHA1(01174d84ed32fb1d95cd632d09f773acb8666c83), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "ficpio2c1", "FIC 486-PIO-2 1.15C101") /* non-pnp, i/o core: NS 332  */
	ROMX_LOAD("115c101.awd",  0x0e0000, 0x20000, CRC(5fadde88) SHA1(eff79692c1ecf34b6ea3f02409d14ce1f5c51bf9), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "ficpio2b1", "FIC 486-PIO-2 1.15B101") /* non-pnp, i/o core: NS 311/312  */
	ROMX_LOAD("115b101.awd",  0x0e0000, 0x20000, CRC(ff69617d) SHA1(ecbfc7315dcf6bd3e5b59e3ae9258759f64fe7a0), ROM_BIOS(4))
ROM_END


ROM_START( at586 )
	ROM_REGION32_LE(0x40000, "isa", 0)
	ROM_SYSTEM_BIOS(0, "sptx", "SP-586TX")
	ROMX_LOAD("sp586tx.bin",   0x20000, 0x20000, CRC(1003d72c) SHA1(ec9224ff9b0fdfd6e462cb7bbf419875414739d6), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "unisys", "Unisys 586") // probably bad dump due to need of hack in i82439tx to work
	ROMX_LOAD("at586.bin",     0x20000, 0x20000, CRC(717037f5) SHA1(1d49d1b7a4a40d07d1a897b7f8c827754d76f824), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(2, "ga586t2", "Gigabyte GA-586T2") // ITE 8679 I/O
	ROMX_LOAD("gb_ga586t2.bin",  0x20000, 0x20000, CRC(3a50a6e1) SHA1(dea859b4f1492d0d08aacd260ed1e83e00ebac08), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "5tx52", "Acorp 5TX52") // W83877TF I/O
	ROMX_LOAD("acorp_5tx52.bin", 0x20000, 0x20000, CRC(04d69419) SHA1(983377674fef05e710c8665c14cc348c99166fb6), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "txp4", "ASUS TXP4") // W83977TF-A I/O
	ROMX_LOAD("asus_txp4.bin",   0x20000, 0x20000, CRC(a1321bb1) SHA1(92e5f14d8505119f85b148a63510617ac12bcdf3), ROM_BIOS(5))
ROM_END

ROM_START( at586x3 )
	ROM_REGION32_LE(0x40000, "isa", 0)
	ROM_LOAD("5hx29.bin",   0x20000, 0x20000, CRC(07719a55) SHA1(b63993fd5186cdb4f28c117428a507cd069e1f68))
ROM_END

ROM_START( c386sx16 )
	ROM_REGION(0x1000000,"maincpu", 0)
	/* actual VGA BIOS not dumped - uses a WD Paradise according to http://www.cbmhardware.de/pc/pc.php */

	/* Commodore 80386SX BIOS Rev. 1.03 */
	/* Copyright (C) 1985-1990 Commodore Electronics Ltd. */
	/* Copyright (C) 1985-1990 Phoenix Technologies Ltd. */
	ROM_LOAD16_BYTE( "390914-01.u39", 0xf0000, 0x8000, CRC(8f849198) SHA1(550b04bac0d0807d6e95ec25391a81272779b41b)) /* 390914-01 V1.03 CS-2100 U39 Copyright (C) 1990 CBM */
	ROM_LOAD16_BYTE( "390915-01.u38", 0xf0001, 0x8000, CRC(ee4bad92) SHA1(6e02ef97a7ce336485814c06a1693bc099ce5cfb)) /* 390915-01 V1.03 CS-2100 U38 Copyright (C) 1990 CBM */
ROM_END

ROM_START( xb42663 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "qi310223.lo", 0xe0000, 0x10000, CRC(53047f49) SHA1(7b38e533f7f27295269549c63e5477d950239167))
	ROM_LOAD16_BYTE( "qi310223.hi", 0xe0001, 0x10000, CRC(4852869f) SHA1(98599d4691d40b3fac2936034c70b386ce4caf77))
ROM_END

ROM_START( qi600 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "qi610223.lo", 0xe0000, 0x10000, CRC(563114a9) SHA1(62932b3bf0b5502ff708f604c21773f00afda58e))
	ROM_LOAD16_BYTE( "qi610223.hi", 0xe0001, 0x10000, CRC(0ae133f6) SHA1(6039c366f7fe0ebf60b34c1a7d6b2d781b664001))
ROM_END

ROM_START( qi900 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "qi910224.lo", 0xe0000, 0x10000, CRC(b012ad3c) SHA1(807e788a6bd03f5e983fe503af3d0b202c754b8a))
	ROM_LOAD16_BYTE( "qi910224.hi", 0xe0001, 0x10000, CRC(36e66d56) SHA1(0900c5272ec3ced550f18fb08db59ab7f67a621e))
ROM_END

ROM_START( ftsserv )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "fts10226.lo", 0xe0000, 0x10000, CRC(efbd738f) SHA1(d5258760bafdaf1bf13c4a49da76d4b5e7b4ccbd))
	ROM_LOAD16_BYTE( "fts10226.hi", 0xe0001, 0x10000, CRC(2460853f) SHA1(a6bba8d2f800140afd129c4d5278f7ae8fe7e63a))
	/* FT Server series Front Panel */
	ROM_REGION(0x10000,"front", 0)
	ROM_LOAD( "fp10009.bin",	 0x0000, 0x8000, CRC(8aa7f718) SHA1(9ee6c6a5bb92622ea8d3805196d42ff68887d820))
ROM_END

ROM_START( apxenls3 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "31020.lo", 0xf0000, 0x8000, CRC(a19678d2) SHA1(d13c12fa7e94333555eabf58b81bad421e21cd91))
	ROM_LOAD16_BYTE( "31020.hi", 0xf0001, 0x8000, CRC(4922e020) SHA1(64e6448323dad2209e004cd93fa181582e768ed5))
ROM_END

ROM_START( aplanst )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "31024", "Bios 3-10-24")
	ROMX_LOAD("31024.lo", 0xf0000, 0x8000, CRC(e52b59e1) SHA1(cfcaa4d8d658df8df463108ef30695bd4ee7a617), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD("31024.hi", 0xf0001, 0x8000, CRC(7286aefa) SHA1(dfc0e3f4936780fa62ae9ec392ce17aa65e717cd), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "31025", "Bios 3-10-25")
	ROMX_LOAD("31025.lo", 0xf0000, 0x8000, CRC(1aec09bc) SHA1(51d56c97c7c1674554aa89b68945329ea967a8bc), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD("31025.hi", 0xf0001, 0x8000, CRC(0763caa5) SHA1(48510a933dcd6efea3b14d04444f584c3e6fefeb), ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "31026", "Bios 3-10-26i")
	ROMX_LOAD("31026i.lo", 0xf0000, 0x8000, CRC(670b6ab4) SHA1(8d61a0edf187f99b67eb58f5e11276deee801d17), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD("31026i.hi", 0xf0001, 0x8000, CRC(ef01c54f) SHA1(911f95d65ab96878e5e7ebccfc4b329db47a1351), ROM_SKIP(1) | ROM_BIOS(3) )
ROM_END

ROM_START( aplannb )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "lsl31025.lo", 0xe0000, 0x10000, CRC(8bb7229b) SHA1(31449d12884ec4e7752e6c1ce7ce9e0d044eadf2))
	ROM_LOAD16_BYTE( "lsh31025.hi", 0xe0001, 0x10000, CRC(09e5c1b9) SHA1(d42be83b4181d3733268c29df04a4d2918370f4e))
ROM_END

ROM_START( apvxft )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "ft10221.lo", 0xe0000, 0x10000, CRC(8f339de0) SHA1(a6542406746eaf1ff7f9e3678c5cbe5522fb314a))
	ROM_LOAD16_BYTE( "ft10221.hi", 0xe0001, 0x10000, CRC(3b16bc31) SHA1(0592d1d81e7fd4715b0612083482db122d78c7f2))
ROM_END

ROM_START( aplscar )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "car306", "Caracal 3.06")
	ROMX_LOAD("car306.bin",   0xc0000, 0x40000, CRC(fc271dea) SHA1(6207cfd312c9957243b8157c90a952404e43b237), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "car307", "Caracal 3.07")
	ROMX_LOAD("car307.bin",   0xc0000, 0x40000, CRC(66a01852) SHA1(b0a68c9d67921d27ba483a1c50463406c08d3085), ROM_BIOS(2))
ROM_END

ROM_START( apxena1 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD("a1-r26.bin",   0xe0000, 0x20000, CRC(d29e983e) SHA1(5977df7f8d7ac2a154aa043bb6f539d96d51fcad))
ROM_END

ROM_START( apxenp2 )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "p2r02g2", "p2r02g2")
	ROMX_LOAD("p2r02g2.bin",   0xe0000, 0x20000, CRC(311bcc5a) SHA1(be6fa144322077dcf66b065e7f4e61aab8c278b4), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD("p2r01f0.bin",   0xe0000, 0x20000, CRC(bbc68f2e) SHA1(6954a52a7dda5521794151aff7a04225e9c7df77), ROM_BIOS(2))
ROM_END

ROM_START( apxeni )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "lep121", "Rom Bios 1.2.1")
	ROMX_LOAD( "lep121.bin", 0xf8000, 0x8000, CRC(948c1927) SHA1(d06bdbd6292db73c815ad1060daf055293dfddf5), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD( "lep121s.bin", 0xf8000, 0x8000, CRC(296118e4) SHA1(d1feaa9704e6ce3bc10c900bdd310d9494b02304), ROM_BIOS(2))
ROM_END

ROM_START( aplsbon )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "bon106", "Boinsai 1-06")
	ROMX_LOAD("bon106.bin",   0xe0000, 0x20000, CRC(98a4eb76) SHA1(e0587afa78aeb9a8803f9b9f9e457e9847b0a2b2), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "bon203", "Boinsai 2-03")
	ROMX_LOAD("bon203.bin",   0xe0000, 0x20000, CRC(32a0e125) SHA1(a4fcbd76952599993fa8b76aa36a96386648abb2), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "bon10703", "Boinsai 1-07-03")
	ROMX_LOAD("bon10703.bin",   0xe0000, 0x20000, CRC(0275b3c2) SHA1(55ef4cbb7f3166f678aaa478234a42049deaba5f), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "bon20402", "Boinsai 2.03")
	ROMX_LOAD("bon20402.bin",   0xe0000, 0x20000, CRC(ac5803fb) SHA1(b8fe92711c6a38a5d9e6497e76a0929c1685c631), ROM_BIOS(4))
ROM_END

ROM_START( apxlsam )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "sam107", "ROM BIOS Version 1-07")
	ROMX_LOAD("sam1-07.bin",   0xe0000, 0x20000, CRC(65e05a8e) SHA1(c3cd198a129122cb05a28798e54331b06cfdd310), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "sam206", "ROM BIOS Version 2-06")
	ROMX_LOAD("sam2-06.bin",   0xe0000, 0x20000, CRC(9768bb0f) SHA1(8166b77b133072f72f23debf85984eb19578ffc1), ROM_BIOS(2))
ROM_END

/* FIC VT-503 (Intel TX chipset, ITE 8679 Super I/O) */
ROM_START( ficvt503 )
	ROM_REGION32_LE(0x40000, "isa", 0)
	ROM_SYSTEM_BIOS(0, "109gi13", "1.09GI13") /* 1997-10-02 */
	ROMX_LOAD("109gi13.bin", 0x20000, 0x20000, CRC(0c32af48) SHA1(2cce40a98598f1ed1f398975f7a90c8be4200667), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "109gi14", "1.09GI14") /* 1997-11-07 */
	ROMX_LOAD("109gi14.awd", 0x20000, 0x20000, CRC(588c5cc8) SHA1(710e5405850fd975b362a422bfe9bc6d6c9a36cd), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "109gi15", "1.09GI15") /* 1997-11-07 */
	ROMX_LOAD("109gi15.awd", 0x20000, 0x20000, CRC(649a3481) SHA1(e681c6ab55a67cec5978dfffa75fcddc2aa0de4d), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "109gi16", "1.09GI16") /* 2000-03-23 */
	ROMX_LOAD("109gi16.bin", 0x20000, 0x20000, CRC(a928f271) SHA1(127a83a60752cc33b3ca49774488e511ec7bac55), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "115gk140", "1.15GK140") /* 1999-03-03 */
	ROMX_LOAD("115gk140.awd", 0x20000, 0x20000, CRC(65e88956) SHA1(f94bb0732e00b5b0f18f4e349db24a289f8379c5), ROM_BIOS(5))
ROM_END

ROM_START( aprpand )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD( "pf10226.std", 0xe0000, 0x20000, CRC(7396fb87) SHA1(a109cbad2179eec55f86c0297a59bb015461da21))
ROM_END

ROM_START( aprfte )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD( "1-2r2-4.486", 0xe0000, 0x20000, CRC(bccc236d) SHA1(0765299363e68cf65710a688c360a087856ece8f))
ROM_END

ROM_START( megapc )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "41651-bios lo.u18",  0xe0000, 0x10000, CRC(1e9bd3b7) SHA1(14fd39ec12df7fae99ccdb0484ee097d93bf8d95))
	ROM_LOAD16_BYTE( "211253-bios hi.u19", 0xe0001, 0x10000, CRC(6acb573f) SHA1(376d483db2bd1c775d46424e1176b24779591525))
ROM_END

ROM_START( megapcpl )
	ROM_REGION(0x1000000,"maincpu", 0)
	ROM_LOAD16_BYTE( "41652.u18",  0xe0000, 0x10000, CRC(6f5b9a1c) SHA1(cae981a35a01234fcec99a96cb38075d7bf23474))
	ROM_LOAD16_BYTE( "486slc.u19", 0xe0001, 0x10000, CRC(6fb7e3e9) SHA1(c439cb5a0d83176ceb2a3555e295dc1f84d85103))
ROM_END

ROM_START( t2000sx )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD( "014d.ic9", 0xe0000, 0x20000, CRC(e9010b02) SHA1(75688fc8e222640fa22bcc90343c6966fe0da87f))
ROM_END

ROM_START( pc2386 )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD( "c000.bin", 0xc0000, 0x4000, CRC(33145bbf) SHA1(c49eaec19f656482e12c8bf282cd4ee5986d227d) )
	ROM_LOAD( "f000.bin", 0xf0000, 0x10000, CRC(f54a063c) SHA1(ce70ec493053afab662f51199ef9c9304a209b8e) )

	ROM_REGION( 0x2000, "gfx1", ROMREGION_ERASE00 )

	ROM_REGION( 0x1000, "keyboard", 0 ) // PC2286 / PC2386 102-key keyboard
	ROM_LOAD( "40211.ic801", 0x000, 0x1000, CRC(4440d981) SHA1(a76006a929f26c178e09908c66f28abc92e7744c) )
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR  NAME      PARENT   COMPAT   MACHINE    INPUT       INIT    COMPANY     FULLNAME */
COMP ( 1984, ibm5170,  0,       ibm5150, ibm5170,   atcga, at_state,      atcga,  "International Business Machines",  "IBM PC/AT 5170", GAME_NOT_WORKING )
COMP ( 1985, ibm5170a, ibm5170, 0,       ibm5170a,  atcga, at_state,      atcga,  "International Business Machines",  "IBM PC/AT 5170 8MHz", GAME_NOT_WORKING )
COMP ( 1985, ibm5162,  ibm5170, 0,       ibm5162,   atcga, at_state,      atcga,  "International Business Machines",  "IBM PC/XT-286 5162", GAME_NOT_WORKING )
COMP ( 1990, i8530h31, ibm5170, 0,       ps2m30286, atvga, at_state,      atvga,  "International Business Machines",  "IBM PS/2 8530-H31 (Model 30/286)", GAME_NOT_WORKING )
COMP ( 1988, i8530286, ibm5170, 0,       ps2m30286, atvga, at_state,      atvga,  "International Business Machines",  "IBM PS/2 Model 30-286", GAME_NOT_WORKING )
COMP ( 198?, i8535043, ibm5170, 0,       at386, 	atvga, at_state,      atvga,  "International Business Machines",  "IBM PS/2 8535-043 (Model 35)", GAME_NOT_WORKING )
COMP ( 198?, i8550021, ibm5170, 0,       at386, 	atvga, at_state,      atvga,  "International Business Machines",  "IBM PS/2 8550-021 (Model 50)", GAME_NOT_WORKING )
COMP ( 198?, i8550061, ibm5170, 0,       at386, 	atvga, at_state,      atvga,  "International Business Machines",  "IBM PS/2 8550-061 (Model 50Z)", GAME_NOT_WORKING )
COMP ( 1989, i8555081, ibm5170, 0,       at386,     atvga, at_state,      atvga,  "International Business Machines",  "IBM PS/2 8550-081 (Model 55SX)", GAME_NOT_WORKING )
COMP ( 198?, i8580071, ibm5170, 0,       at386,     atvga, at_state,      atvga,  "International Business Machines",  "IBM PS/2 8580-071 (Model 80)", GAME_NOT_WORKING )
COMP ( 198?, i8580111, ibm5170, 0,       at386,     atvga, at_state,      atvga,  "International Business Machines",  "IBM PS/2 8580-111 (Model 80)", GAME_NOT_WORKING )
COMP ( 1987, at,       ibm5170, 0,       ibm5162,   atcga, at_state,      atcga,  "<generic>",  "PC/AT (CGA, MF2 Keyboard)", GAME_NOT_WORKING )
COMP ( 1987, atvga,    ibm5170, 0,       atvga,     atvga, at_state,      atvga,  "<generic>",  "PC/AT (VGA, MF2 Keyboard)" , GAME_NOT_WORKING )
COMP ( 1988, at386,    ibm5170, 0,       at386,     atvga, at_state,      atvga,  "<generic>",  "PC/AT 386 (VGA, MF2 Keyboard)", GAME_NOT_WORKING )
COMP ( 1990, at486,    ibm5170, 0,       at486,     atvga, at_state,      atvga,  "<generic>",  "PC/AT 486 (VGA, MF2 Keyboard)", GAME_NOT_WORKING )
COMP ( 1990, at586,    ibm5170, 0,       at586,     atvga, driver_device,      0,	"<generic>",  "PC/AT 586 (PIIX4)", GAME_NOT_WORKING )
COMP ( 1990, at586x3,  ibm5170, 0,       at586x3,   atvga, driver_device,      0,	    "<generic>",  "PC/AT 586 (PIIX3)", GAME_NOT_WORKING )
COMP ( 1989, neat,     ibm5170, 0,       neat,      atcga, at_state,      atcga,  "<generic>",  "NEAT (CGA, MF2 Keyboard)", GAME_NOT_WORKING )
COMP ( 1993, ct486,    ibm5170, 0,       ct486,     atvga, at_state,      atvga,  "<unknown>",  "PC/AT 486 with C&T chipset", GAME_NOT_WORKING )
COMP ( 1993, ec1849,   ibm5170, 0,       ec1849,    atcga, at_state,      atcga,  "<unknown>",  "EC-1849", GAME_NOT_WORKING )
COMP ( 1993, megapc,   ibm5170, 0,       megapc,    atvga, at_state,      atvga,  "Amstrad plc", "MegaPC", GAME_NOT_WORKING )
COMP ( 199?, megapcpl, ibm5170, 0,       megapcpl,  atvga, at_state,      atvga,  "Amstrad plc", "MegaPC Plus", GAME_NOT_WORKING )
COMP ( 1989, pc2386,   ibm5170, 0,       at386,     atvga, at_state,      atvga,  "Amstrad plc", "Amstrad PC2386", GAME_NOT_WORKING )
COMP ( 1991, aprfte,   ibm5170, 0,       at486,     atvga, at_state,      atvga,  "Apricot",  "Apricot FT//ex 486 (J3 Motherboard)", GAME_NOT_WORKING )
COMP ( 1991, ftsserv,  ibm5170, 0,       at486,     atvga, at_state,      atvga,  "Apricot",  "Apricot FTs (Scorpion)", GAME_NOT_WORKING )
COMP ( 1992, aprpand,  ibm5170, 0,       at486,     atvga, at_state,      atvga,  "Apricot",  "Apricot FTs (Panther Rev F 1.02.26)", GAME_NOT_WORKING )
COMP ( 1990, aplanst,  ibm5170, 0,       at386,     atvga, at_state,      atvga,  "Apricot",  "Apricot LANstation (Krypton Motherboard)", GAME_NOT_WORKING )
COMP ( 1990, aplannb,  ibm5170, 0,       at386,     atvga, at_state,      atvga,  "Apricot",  "Apricot LANstation (Novell Remote Boot)", GAME_NOT_WORKING )
COMP ( 1992, aplscar,  ibm5170, 0,       at386,     atvga, at_state,      atvga,  "Apricot",  "Apricot LS Pro (Caracal Motherboard)", GAME_NOT_WORKING )
COMP ( 1992, aplsbon,  ibm5170, 0,       at486,     atvga, at_state,      atvga,  "Apricot",  "Apricot LS Pro (Bonsai Motherboard)", GAME_NOT_WORKING )
COMP ( 1988, xb42663,  ibm5170, 0,       at386,     atvga, at_state,      atvga,  "Apricot",  "Apricot Qi 300 (Rev D,E & F Motherboard)", GAME_NOT_WORKING )
COMP ( 1988, qi600,    ibm5170, 0,       at386,     atvga, at_state,      atvga,  "Apricot",  "Apricot Qi 600 (Neptune Motherboard)", GAME_NOT_WORKING )
COMP ( 1990, qi900,    ibm5170, 0,       at486,     atvga, at_state,      atvga,  "Apricot",  "Apricot Qi 900 (Scorpion Motherboard)", GAME_NOT_WORKING )
COMP ( 1989, apvxft,   ibm5170, 0,       at486,     atvga, at_state,      atvga,  "Apricot",  "Apricot VX FT server", GAME_NOT_WORKING )
COMP ( 1991, apxenls3, ibm5170, 0,       at486,     atvga, at_state,      atvga,  "Apricot",  "Apricot XEN-LS (Venus IV Motherboard)", GAME_NOT_WORKING )
COMP ( 1993, apxlsam,  ibm5170, 0,       at486,     atvga, at_state,      atvga,  "Apricot",  "Apricot XEN-LS II (Samurai Motherboard)", GAME_NOT_WORKING )
COMP ( 1987, apxeni,   ibm5170, 0,       at386,     atvga, at_state,      atvga,  "Apricot",  "Apricot XEN-i 386 (Leopard Motherboard)" , GAME_NOT_WORKING )
COMP ( 1989, xb42639,  ibm5170, 0,       xb42639,   atvga, at_state,      atvga,  "Apricot",  "Apricot XEN-S (Venus I Motherboard 286)" , GAME_NOT_WORKING )
COMP ( 1990, xb42639a, ibm5170, 0,       xb42639,   atvga, at_state,      atvga,  "Apricot",  "Apricot XEN-S (Venus II Motherboard 286)" , GAME_NOT_WORKING )
COMP ( 1989, xb42664,  ibm5170, 0,       at386,     atvga, at_state,      atvga,  "Apricot",  "Apricot XEN-S (Venus I Motherboard 386)" , GAME_NOT_WORKING )
COMP ( 1990, xb42664a, ibm5170, 0,       at386,     atvga, at_state,      atvga,  "Apricot",  "Apricot XEN-S (Venus II Motherboard 386)" , GAME_NOT_WORKING )
COMP ( 1993, apxena1,  ibm5170, 0,       at486,     atvga, at_state,      atvga,  "Apricot",  "Apricot XEN PC (A1 Motherboard)", GAME_NOT_WORKING )
COMP ( 1993, apxenp2,  ibm5170, 0,       at486,     atvga, at_state,      atvga,  "Apricot",  "Apricot XEN PC (P2 Motherboard)", GAME_NOT_WORKING )
COMP ( 1990, c386sx16, ibm5170, 0,       c386sx16,  atvga, at_state,      atvga,  "Commodore Business Machines", "Commodore 386SX-16", GAME_NOT_WORKING )
COMP ( 1988, cmdpc30,  ibm5170, 0,       ibm5162,   atcga, at_state,      atcga,  "Commodore Business Machines",  "PC 30 III", GAME_NOT_WORKING )
COMP ( 1995, ficpio2,  ibm5170, 0,       at486,     atvga, at_state,      atvga,  "FIC", "486-PIO-2", GAME_NOT_WORKING )
COMP ( 1997, ficvt503, ibm5170, 0,       at586,     atvga, driver_device,      0,      "FIC", "VT-503", GAME_NOT_WORKING )
COMP ( 1991, t2000sx,  ibm5170, 0,       c386sx16,  atvga, at_state,      atvga,  "Toshiba",  "T2000SX", GAME_NOT_WORKING )
