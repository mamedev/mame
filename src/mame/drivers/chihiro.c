/*

Chihiro is an Xbox based arcade motherboard from SEGA
A Chihiro system consists of network board, media board, base board & Xbox board

The whole system is divided into 2 parts and each part has two boards.
The upper part contains a media board with a TSOP48 where there is an xbox .xbe loader
(this is the dashboard you see when you power the Chihiro) and for a network board (100% the same
as the one in the Triforce v3, same firmware also)

The bottom section consists of an Xbox board with 128MB of RAM and with a different MCPX than
a retail one and a base board that handles JVS and Video.

Network Board Dump : Ver1305.bin
Media Board dump   : FPR21042_M29W160ET.bin
Base Board Dumps   : ic10_g24lc64.bin ic11_24lc024.bin pc20_g24lc64.bin
Xbox Board Dump    : Not dumped

FPR21042_M29W160ET.bin :
As in Triforce, it consists of two versions in the same flash, the first MB of the flash has
an older version as backup, and the second MB has the current version, versions included are:
SegaBoot Ver.2.00.0 Build:Feb  7 2003 12:28:30
SegaBoot Ver.2.13.0 Build:Mar  3 2005 17:03:15

ic10_g24lc64.bin: This dump contains the firmware of the Base Board, serial number and REGION of the whole system
Region is located at Offset 0x00001F10 , 01 means JAP, 02 Means USA, 03 Means EXPORT, if you
want to change the region of your Chihiro Board, just change this byte.
pc20_g24lc64.bin: it seems a backup of the base board without region or serial, older version maybe?
ic11_24lc024.bin: this is the mysterious one, as the previous 2, its on the Base Board, and just contains some
strings the interesting thing is that it contains the string SBJE and if you go to the system info menu
on the Chihiro and you press service button 16 times in a row, a message will be displayed: GAME ID SBJE

Thanks to Alex, Mr Mudkips, and Philip Burke for this info.

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "includes/naomibd.h"
#include "debug/debugcon.h"
#include "debug/debugcmd.h"

/* jamtable instructions for Chihiro
St.     Instr.       Comment
0x01    POKEPCI      PCICONF[OP2] := OP1
0x02    OUTB         PORT[OP2] := OP1
0x03    POKE         MEM[OP2] := OP1
0x04    BNE          IF ACC <> OP2 THEN PC := PC + OP1
0x05    PEEKPCI      ACC := PCICONF[OP2]
0x06    AND/OR       ACC := (ACC & OP2) | OP1
0x07    BRA          PC := PC + OP1
0x08    INB          ACC := PORT[OP2]
0x09    PEEK         ACC := MEM[OP2]
0xE1    (prefix)     execute the instruction code in OP2 with OP2 := OP1, OP1 := ACC
0xEE    END
*/

/* jamtable disassembler */
static void jamtable_disasm(running_machine *machine, address_space *space,UINT32 address,UINT32 size) // 0xff000080 == fff00080
{
	UINT32 base,addr;
	UINT32 opcode,op1,op2;
	char sop1[16];
	char sop2[16];
	char pcrel[16];
	int prefix;

	addr=address;
	while (1)
	{
		base=addr;
		opcode=space->read_byte(addr);
		addr++;
		op1=space->read_dword(addr);
		addr+=4;
		op2=space->read_dword(addr);
		addr+=4;
		if (opcode == 0xe1)
		{
			opcode=op2 & 255;
			op2=op1;
			//op1=edi;
			sprintf(sop2,"%08X",op2);
			sprintf(sop1,"ACC");
			sprintf(pcrel,"PC+ACC");
			prefix=1;
		}
		else
		{
			sprintf(sop2,"%08X",op2);
			sprintf(sop1,"%08X",op1);
			sprintf(pcrel,"%08X",base+9+op1);
			prefix=0;
		}
		debug_console_printf(machine,"%08X ",base);
		// dl=instr ebx=par1 eax=par2
		switch (opcode)
		{
			case 0x01:
				// if ((op2 & 0xff) == 0x880) op1=op1 & 0xfffffffd
				// out cf8,op2
				// out cfc,op1
				// out cf8,0
				// cf8 (CONFIG_ADDRESS) format:
				// 31 30      24 23        16 15           11 10              8 7               2 1 0
				// +-+----------+------------+---------------+-----------------+-----------------+-+-+
				// | | Reserved | Bus Number | Device Number | Function Number | Register Number |0|0|
				// +-+----------+------------+---------------+-----------------+-----------------+-+-+
				// 31 - Enable bit
				debug_console_printf(machine,"POKEPCI PCICONF[%s]=%s\n",sop2,sop1);
				break;
			case 0x02:
				debug_console_printf(machine,"OUTB    PORT[%s]=%s\n",sop2,sop1);
				break;
			case 0x03:
				debug_console_printf(machine,"POKE    MEM[%s]=%s\n",sop2,sop1);
				break;
			case 0x04:
				debug_console_printf(machine,"BNE     IF ACC != %s THEN PC=%s\n",sop2,pcrel);
				break;
			case 0x05:
				// out cf8,op2
				// in acc,cfc
				debug_console_printf(machine,"PEEKPCI ACC=PCICONF[%s]\n",sop2);
				break;
			case 0x06:
				debug_console_printf(machine,"AND/OR  ACC=(ACC & %s) | %s\n",sop2,sop1);
				break;
			case 0x07:
				debug_console_printf(machine,"BRA     PC=%s\n",pcrel);
				break;
			case 0x08:
				debug_console_printf(machine,"INB     ACC=PORT[%s]\n",sop2);
				break;
			case 0x09:
				debug_console_printf(machine,"PEEK    ACC=MEM[%s]\n",sop2);
				break;
			case 0xee:
				debug_console_printf(machine,"END\n");
				break;
			default:
				debug_console_printf(machine,"NOP     ????\n");
				break;
		}
		if (opcode == 0xee)
			break;
		if (size <= 9)
			break;
		size-=9;
	}
}

void jamtable_disasm_command(running_machine *machine, int ref, int params, const char **param)
{
	address_space *space=machine->firstcpu->space();
	UINT64	addr,size;

	if (params < 2)
		return;
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;
	if (!debug_command_parameter_number(machine, param[1], &size))
		return;
	jamtable_disasm(machine, space, (UINT32)addr, (UINT32)size);
}

/*
St.     Instr.       Comment
0x02    PEEK         ACC := MEM[OP1]
0x03    POKE         MEM[OP1] := OP2
0x04    POKEPCI      PCICONF[OP1] := OP2
0x05    PEEKPCI      ACC := PCICONF[OP1]
0x06    AND/OR       ACC := (ACC & OP1) | OP2
0x07    (prefix)     execute the instruction code in OP1 with OP1 := OP2, OP2 := ACC
0x08    BNE          IF ACC = OP1 THEN PC := PC + OP2
0x09    BRA          PC := PC + OP2
0x10    AND/OR ACC2  (unused/defunct) ACC2 := (ACC2 & OP1) | OP2
0x11    OUTB         PORT[OP1] := OP2
0x12    INB          ACC := PORT(OP1)
0xEE    END
*/
#ifdef UNUSED_FUNCTION
static READ32_HANDLER( chihiro_jamtable )
{
	return 0xEEEEEEEE;
}
#endif

static UINT32 dummy_pci_r(running_device *busdevice, running_device *device, int function, int reg, UINT32 mem_mask)
{
	logerror("  bus:%d function:%d register:%d mask:%08X\n",((pci_bus_config *)downcast<const legacy_device_config_base &>(busdevice->baseconfig()).inline_config())->busnum,function,reg,mem_mask);
	return 0;
}

static void dummy_pci_w(running_device *busdevice, running_device *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	logerror("  bus:%d function:%d register:%d data:%08X mask:%08X\n",((pci_bus_config *)downcast<const legacy_device_config_base &>(busdevice->baseconfig()).inline_config())->busnum,function,reg,data,mem_mask);
}

static READ32_HANDLER( dummy_r )
{
	return 0;
}

static WRITE32_HANDLER( dummy_w )
{
}

int smbus_cx25871(int command,int rw,int data)
{
	logerror("cx25871: %d %d %d\n",command,rw,data);
	return 0;
}

typedef struct _smbus_state {
	int status;
	int control;
	int address;
	int data;
	int command;
	int rw;
	int (*devices[128])(int command,int rw,int data);
	UINT32 words[256/4];
} smbus_state;
smbus_state smbusst;

void smbus_register_device(int address,int (*handler)(int command,int rw,int data))
{
	if (address < 128)
		smbusst.devices[address]=handler;
}

static READ32_HANDLER( smbus_r )
{
	if ((offset == 0) && (mem_mask == 0xff)) // 0 smbus status
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | (smbusst.status << 0);
	if ((offset == 1) && (mem_mask == 0xff0000)) // 6 smbus data
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | (smbusst.data << 16);
	return smbusst.words[offset];
}

static WRITE32_HANDLER( smbus_w )
{
	COMBINE_DATA(smbusst.words);
	if ((offset == 0) && (mem_mask == 0xff)) // 0 smbus status
		smbusst.status &= ~data;
	if ((offset == 0) && (mem_mask == 0xff0000)) // 2 smbus control
	{
		data=data>>16;
		smbusst.control = data;
		if ((smbusst.control & 6) == 2)
		{
			if (smbusst.devices[smbusst.address & 127]) {
				if (smbusst.rw == 0) {
					smbusst.devices[smbusst.address & 127](smbusst.command,smbusst.rw,smbusst.data);
				}
				else {
					smbusst.data=smbusst.devices[smbusst.address & 127](smbusst.command,smbusst.rw,smbusst.data);
				}
			}
			smbusst.status |= 0x10;
		}
	}
	if ((offset == 1) && (mem_mask == 0xff)) // 4 smbus address
	{
		smbusst.address = data >> 1;
		smbusst.rw = data & 1;
	}
	if ((offset == 1) && (mem_mask == 0xff0000)) // 6 smbus data
	{
		data=data>>16;
		smbusst.data = data;
	}
	if ((offset == 2) && (mem_mask == 0xff)) // 8 smbus command
		smbusst.command = data;
}


static ADDRESS_MAP_START( xbox_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x07ffffff) AM_RAM
	AM_RANGE(0xff000000, 0xffffffff) AM_ROM AM_REGION("bios", 0) AM_MIRROR(0x00f80000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(xbox_map_io, ADDRESS_SPACE_IO, 32)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_32le_r, pci_32le_w)
	AM_RANGE(0x8000, 0x80ff) AM_READWRITE(dummy_r, dummy_w)
	AM_RANGE(0xc000, 0xc0ff) AM_READWRITE(smbus_r, smbus_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( chihiro )
INPUT_PORTS_END

static MACHINE_START( chihiro )
{
	smbus_register_device(0x45,smbus_cx25871);
	if (machine->debug_flags & DEBUG_FLAG_ENABLED)
		debug_console_register_command(machine,"jamdis",CMDFLAG_NONE,0,2,3,jamtable_disasm_command);
}

static MACHINE_CONFIG_START( chihiro_base, driver_device )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", PENTIUM, 733333333) /* Wrong! */
	MDRV_CPU_PROGRAM_MAP(xbox_map)
	MDRV_CPU_IO_MAP(xbox_map_io)

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_PCI_BUS_ADD("pcibus", 0)
	MDRV_PCI_BUS_DEVICE(0, "PCI Bridge Device - Host Bridge", dummy_pci_r, dummy_pci_w)
	MDRV_PCI_BUS_DEVICE(1, "HUB Interface - ISA Bridge", dummy_pci_r, dummy_pci_w)
	MDRV_PCI_BUS_DEVICE(2, "OHCI USB Controller 1", dummy_pci_r, dummy_pci_w)
	MDRV_PCI_BUS_DEVICE(3, "OHCI USB Controller 2", dummy_pci_r, dummy_pci_w)
	MDRV_PCI_BUS_DEVICE(30, "AGP Host to PCI Bridge", dummy_pci_r, dummy_pci_w)
	MDRV_PCI_BUS_ADD("agpbus", 1)
	MDRV_PCI_BUS_SIBLING("pcibus")
	MDRV_PCI_BUS_DEVICE(0, "NV2A GeForce 3MX Integrated GPU/Northbridge", dummy_pci_r, dummy_pci_w)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MDRV_MACHINE_START(chihiro)

	MDRV_PALETTE_LENGTH(65536)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( chihirogd, chihiro_base )
	MDRV_NAOMI_DIMM_BOARD_ADD("rom_board", "gdrom", "user1", "picreturn")
MACHINE_CONFIG_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1)) /* Note '+1' */

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1)) /* Note '+1' */

#define CHIHIRO_BIOS \
	ROM_REGION( 0x1000000, "bios", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "Chihiro Bios" ) \
	ROM_LOAD_BIOS( 0,  "chihiro_xbox_bios.bin", 0x000000, 0x80000, CRC(66232714) SHA1(b700b0041af8f84835e45d1d1250247bf7077188) ) \
	ROM_REGION( 0x200000, "others", 0) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "fpr21042_m29w160et.bin", 0x000000, 0x200000, CRC(a4fcab0b) SHA1(a13cf9c5cdfe8605d82150b7573652f419b30197) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ic10_g24lc64.bin", 0x000000, 0x2000, CRC(cfc5e06f) SHA1(3ababd4334d8d57abb22dd98bd2d347df39648d9) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ic11_24lc024.bin", 0x000000, 0x80, CRC(8dc8374e) SHA1(cc03a0650bfac4bf6cb66e414bbef121cba53efe) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "pc20_g24lc64.bin", 0x000000, 0x2000, CRC(7742ab62) SHA1(82dad6e2a75bab4a4840dc6939462f1fb9b95101) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ver1305.bin", 0x000000, 0x200000, CRC(a738ea1c) SHA1(45d94d0c39be1cb3db9fab6610a88a550adda4e9) ) \

ROM_START( chihiro )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
ROM_END



ROM_START( hotd3 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0001", 0,  SHA1(174c72f851d0c97e8993227467f16b0781ed2f5c) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0348-com.data", 0x00, 0x50, CRC(d28219ef) SHA1(40dbbc092bc9f99b8d2ae67fbefacd62184f90ec) )
ROM_END

ROM_START( outr2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0004a", 0, SHA1(27acd2d0680e6bafa0d052f60b4372adc37224fd) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0372-com.data", 0x00, 0x50, CRC(a15c9666) SHA1(fd36c524744acb33e579ccb257c71375a5d3af67) )
ROM_END

/*

Title   GHOST SQUAD
Media ID    004F
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDX-0012A
Version V2.000
Release Date    20041209
Manufacturer ID

PIC
253-5508-0398
317-0398-COM

*/

ROM_START( ghostsqu )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0012a", 0,  SHA1(d7d78ce4992cb16ee5b4ac6ca7a37c46b07e8c14) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0398-com.data", 0x00, 0x50, CRC(8c5391a2) SHA1(e64cadeb30c94c3cd4002630cd79cc76c7bde2ed) )
ROM_END


ROM_START( gundamos )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0013", 0, SHA1(96b3dafcc2d2d6803fe3bf43a245d43ee5e0e5a6) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdx-0013.data", 0x00, 0x50, CRC(0479c383) SHA1(7e86a037d2f9d09cec61a38cb19de510bf9482b3) )
ROM_END

/*

Title   VIRTUA COP 3
Media ID    C4AD
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDX-0003A
Version V2.004
Release Date    20030226
Manufacturer ID
TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 599 1058400
track02.raw 750 2101    3179904
track03.bin 45150   549299  1185760800


PIC
255-5508-354
317-054-COM

*/

ROM_START( vcop3 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0003a", 0,  SHA1(cdfec1d2ef02ae9e29cb1462f08904177bc4c9ea) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0354-com.data", 0x00, 0x50,  CRC(df7e3217) SHA1(9f0f4bf6b15f3b6eeea81eaa27b3d25bd94110da) )
ROM_END


ROM_START( mj2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0006c", 0, SHA1(505653117a73ed8b256ccf19450e7573a4dc57e9) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE) // key was missing
	ROM_LOAD("gdx-0006c.pic_data", 0x00, 0x50, NO_DUMP )
ROM_END

ROM_START( ollie )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0007", 0, SHA1(8898a571a427936bffcecd3ef27cb79087d22798) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdx-0007.data", 0x00, 0x50, CRC(d2a8b31f) SHA1(e9ee2df30031826db6bc4bd91969e6680255dcf9) )
ROM_END



ROM_START( wangmid )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0009b", 0, SHA1(e032b9fd8d5d09255592f02f7531a608e8179c9c) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdx-0009b.data", 0x00, 0x50, CRC(3af801f3) SHA1(e9a2558930f3f1f55d5b3c2cadad69329d931f26) )
ROM_END


ROM_START( wangmid2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0015", 0, SHA1(259483fd211a70c23205ffd852316d616c5a2740) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-5106-com.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END


ROM_START( mj3 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0017d", 0, SHA1(cfbbd452c8f4efe0e99f398f5521fc3574b913bb) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE) // key was missing
	ROM_LOAD("gdx-0017d.pic_data", 0x00, 0x50, NO_DUMP )
ROM_END

ROM_START( scg06nt )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0018a", 0, SHA1(e6f3dc8066392854ad7d83f81d3cbc81a5e340b3) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdx-0018.data", 0x00, 0x50, CRC(1a210abd) SHA1(43a54d028315d2dfa9f8ea6fb59265e0b980b02f) )
ROM_END

ROM_START( outr2st )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0014a", 0, SHA1(4f9656634c47631f63eab554a13d19b15558217e) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)	// number was not readable on pic, please fix if known
	ROM_LOAD( "317-0xxx-com.pic", 0x000000, 0x004000, CRC(f94cf26f) SHA1(dd4af2b52935c7b2d8cd196ec1a30c0ef0993322) )
ROM_END

ROM_START( crtaxihr )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0002b", 0, SHA1(4056ebd5587d6c897f475240bc5a4075a995aa8c) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0353-com.pic", 0x000000, 0x004000, CRC(1c6830b1) SHA1(75be47441783c18ee296209a34c432864deed70d) )
ROM_END



GAME( 200?, chihiro,  0,       chihiro_base, chihiro,    0, ROT0, "Sega",           "Chihiro Bios", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
GAME( 2002, hotd3,    chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "The House of the Dead III (GDX-0001)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, crtaxihr, chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Crazy Taxi High Roller (GDX-0002B)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, vcop3,    chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Virtua Cop 3 (GDX-0003A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, outr2,    chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Out Run 2 (Rev. A) (GDX-0004A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, mj2,      chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Sega Network Taisen Mahjong MJ 2 (Rev C) (GDX-0006C)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, ollie,    chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Ollie King (GDX-0007)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, wangmid,  chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Wangan Midnight Maximum Tune (Rev. B) (Export) (GDX-0009B)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, wangmid2, chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Wangan Midnight Maximum Tune 2 (Export) (GDX-0015)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, ghostsqu, chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Ghost Squad (Ver. A?) (GDX-0012A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, gundamos, chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Gundam Battle Operating Simulator (GDX-0013)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, outr2st,  chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Out Run 2 Special Tours (GDX-0014A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, mj3,      chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Sega Network Taisen Mahjong MJ 3 (Rev D) (GDX-0017D)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2006, scg06nt,  chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Sega Club Golf 2006 Next Tours (Rev A) (GDX-0018A)", GAME_NO_SOUND|GAME_NOT_WORKING )
