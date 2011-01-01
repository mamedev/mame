/***************************************************************************

Per-game specific JVS settings / idle loop skips for the MAME Naomi driver.

suchie3: check bp c0a6458 (might be protection related)

tetkiwam: check bp c09613a

vtennis: check wpset dee3ec8,8,w,wpdata==0xa8804000

vtennis2: check bp c020130 / wpset c013ff0,f,w,wpdata==0x3f800000 -> 0xc020434 (test mode)

smarinef: put cabinet in STD mode, bp c027968, wpset c0e66a6,4,w


hotd2: bp 0xc0ba235, modify work RAM 0xc9c35e8 to be zero, bpclear

hotd2o: bp 0xc0ba1f6, modify work RAM 0xc9c35a8 to be zero, bpclear
***************************************************************************/

#include "emu.h"
#include "includes/naomi.h"

UINT64 *naomi_ram64;
int jvsboard_type;
UINT16 actel_id;

static READ64_HANDLER( naomi_bios_idle_skip_r )
{
	if (cpu_get_pc(space->cpu)==0xc04173c)
		cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(500));
		//cpu_spinuntil_int(space->cpu);
//  else
//      printf("%08x\n", cpu_get_pc(space->cpu));

	return naomi_ram64[0x2ad238/8];
}

static UINT8 asciihex_to_dec(UINT8 in)
{
	if (in>=0x30 && in<=0x39)
	{
		return in - 0x30;
	}
	else
	if (in>=0x41 && in<=0x46)
	{
		return in - 0x37;
	}
	/*
    else
    if (in>=0x61 && in<=0x66)
    {
        return in - 0x57;
    }
    */
	else
	{
		fatalerror("unexpected value in asciihex_to_dec");
	}


}

// development helper function
static void create_pic_from_retdat(running_machine* machine)
{
	{
		UINT8* hexregion = machine->region("pichex")->base();
		UINT8* retregion = machine->region("picreturn")->base();
		UINT8* newregion = machine->region("pic")->base();
		int outcount = 0;

		if (hexregion && retregion && newregion)
		{
			int hexoffs = 0;
			int line;

			hexoffs += 0x11; // skip first line  // :020000040000FA

			for (line=0;line<0x200;line++)
			{
				int offs2;

				hexoffs+= 0x1; // skip :
				hexoffs+= 0x8; // skip line #  (:20xxxxxx incrementing in 0x2000)

				for (offs2=0;offs2<0x20;offs2++)
				{
					UINT8 ascii1 = hexregion[hexoffs+0];
					UINT8 ascii2 = hexregion[hexoffs+1];
					UINT8 dec1 = asciihex_to_dec(ascii1);
					UINT8 dec2 = asciihex_to_dec(ascii2);
					UINT8 val = dec1 << 4 | dec2;

					//printf("%02x%02x", ascii1, ascii2);

					printf("%02x", val);

					newregion[outcount] = val;

					hexoffs+=2;
					outcount++;
				}

				hexoffs+=0x4; // skip running checksum + newline

				printf("\n");


			}

			{
				int i;
				printf("string 1 (key1)\n");
				for (i=0;i<7;i++)
				{
					printf("%02x %02x\n", newregion[0x780+i*2], retregion[0x31+i]);

					newregion[0x780+i*2] = retregion[0x31+i]; // patch with extracted data
				}

				printf("string 2 (key2)\n");
				for (i=0;i<7;i++)
				{
					printf("%02x %02x\n", newregion[0x7a0+i*2], retregion[0x29+i]);

					newregion[0x7a0+i*2] = retregion[0x29+i]; // patch with extracted data
				}

				printf("string 3 (filename)\n");
				for (i=0;i<7;i++)
				{
					printf("%02x %02x\n", newregion[0x7c0+i*2], retregion[0x21+i]);

					newregion[0x7c0+i*2] = retregion[0x21+i]; // patch with extracted data
				}

				printf("string 4 (filename?)\n");
				for (i=0;i<7;i++)
				{
					printf("%02x %02x\n", newregion[0x7e0+i*2], retregion[0x19+i]);

					newregion[0x7e0+i*2] = retregion[0x19+i]; // patch with extracted data
				}
			}


			{
				FILE *fp;
				char filename[256];
				sprintf(filename,"picbin_%s", machine->gamedrv->name);
				fp=fopen(filename, "w+b");
				if (fp)
				{
					fwrite(newregion, outcount, 1, fp);
					fclose(fp);
				}

				printf("wrote %04x bytes\n", outcount);
			}

			// hex dumps end with
			//:10400000000000000000000000000000000082002E
			//:00000001FF


		}
	}
}

DRIVER_INIT(naomi)
{
	memory_install_read64_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc2ad238, 0xc2ad23f, 0, 0, naomi_bios_idle_skip_r); // rev e bios
	jvsboard_type = JVSBD_DEFAULT;
	actel_id = 0xffff;

	create_pic_from_retdat(machine);
}

DRIVER_INIT(naomi2)
{
	jvsboard_type = JVSBD_DEFAULT;
	actel_id = 0xffff;

	create_pic_from_retdat(machine);
}

DRIVER_INIT(naomi_mp)
{
	memory_install_read64_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc2ad238, 0xc2ad23f, 0, 0, naomi_bios_idle_skip_r); // rev e bios
	jvsboard_type = JVSBD_MAHJONG;
	actel_id = 0xffff;

	create_pic_from_retdat(machine);
}

static READ64_HANDLER( naomigd_ggxxsla_idle_skip_r )
{
	if (cpu_get_pc(space->cpu)==0x0c0c9adc)
		cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(500));

	return naomi_ram64[0x1aae18/8];
}

DRIVER_INIT( ggxxsla )
{
	memory_install_read64_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc1aae18, 0xc1aae1f, 0, 0, naomigd_ggxxsla_idle_skip_r);
	DRIVER_INIT_CALL(naomi);
}

static READ64_HANDLER( naomigd_ggxx_idle_skip_r )
{
	if (cpu_get_pc(space->cpu)==0xc0b5c3c) // or 0xc0bab0c
		cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(500));

	return naomi_ram64[0x1837b8/8];
}


DRIVER_INIT( ggxx )
{
	memory_install_read64_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc1837b8, 0xc1837bf, 0, 0, naomigd_ggxx_idle_skip_r);
	DRIVER_INIT_CALL(naomi);
}

static READ64_HANDLER( naomigd_ggxxrl_idle_skip_r )
{
	if (cpu_get_pc(space->cpu)==0xc0b84bc) // or 0xc0bab0c
		cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(500));

	//printf("%08x\n", cpu_get_pc(space->cpu));

	return naomi_ram64[0x18d6c8/8];
}

DRIVER_INIT( ggxxrl )
{
	memory_install_read64_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc18d6c8, 0xc18d6cf, 0, 0, naomigd_ggxxrl_idle_skip_r);
	DRIVER_INIT_CALL(naomi);
}

/* at least speeds up the annoying copyright screens ;-) */
static READ64_HANDLER( naomigd_sfz3ugd_idle_skip_r )
{
	if (cpu_get_pc(space->cpu)==0xc36a2dc)
		cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(500));

	return naomi_ram64[0x5dc900/8];
}

DRIVER_INIT( sfz3ugd )
{
	memory_install_read64_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc5dc900, 0xc5dc907, 0, 0, naomigd_sfz3ugd_idle_skip_r);
	DRIVER_INIT_CALL(naomi);
}


DRIVER_INIT( qmegamis )
{
	DRIVER_INIT_CALL(naomi);
	actel_id = 0; //FIXME: correct value
}

DRIVER_INIT( mvsc2 )
{
	DRIVER_INIT_CALL(naomi);
	actel_id = 0; //FIXME: correct value
}

DRIVER_INIT( gram2000 )
{
	DRIVER_INIT_CALL(naomi);
	actel_id = 0; //FIXME: correct value
}

DRIVER_INIT( vf4evoct )
{
	DRIVER_INIT_CALL(naomi2);
	actel_id = 0; //FIXME: correct value
}

DRIVER_INIT( kick4csh )
{
	DRIVER_INIT_CALL(naomi2);
	actel_id = 0; //FIXME: correct value
}

