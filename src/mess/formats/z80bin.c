#include "emu.h"
#include "z80bin.h"

/*-------------------------------------------------
    z80bin_load_file - load a z80bin file into
    memory
-------------------------------------------------*/

static int z80bin_load_file(device_image_interface *image, const char *file_type, UINT16 *exec_addr, UINT16 *start_addr, UINT16 *end_addr )
{
	int ch;
	UINT16 args[3];
	UINT16 i=0, j, size;
	UINT8 data;
	char pgmname[256];
	char message[256];

	image->fseek(7, SEEK_SET);

	while((ch = image->fgetc()) != 0x1A)
	{
		if (ch == EOF)
		{
			image->seterror(IMAGE_ERROR_INVALIDIMAGE, "Unexpected EOF while getting file name");
			image->message(" Unexpected EOF while getting file name");
			return IMAGE_INIT_FAIL;
		}

		if (ch != '\0')
		{
			if (i >= (ARRAY_LENGTH(pgmname) - 1))
			{
				image->seterror(IMAGE_ERROR_INVALIDIMAGE, "File name too long");
				image->message(" File name too long");
				return IMAGE_INIT_FAIL;
			}

			pgmname[i] = ch;	/* build program name */
			i++;
		}
	}

	pgmname[i] = '\0';	/* terminate string with a null */

	if (image->fread(args, sizeof(args)) != sizeof(args))
	{
		image->seterror(IMAGE_ERROR_INVALIDIMAGE, "Unexpected EOF while getting file size");
		image->message(" Unexpected EOF while getting file size");
		return IMAGE_INIT_FAIL;
	}

	exec_addr[0] = LITTLE_ENDIANIZE_INT16(args[0]);
	start_addr[0] = LITTLE_ENDIANIZE_INT16(args[1]);
	end_addr[0]	= LITTLE_ENDIANIZE_INT16(args[2]);

	size = (end_addr[0] - start_addr[0] + 1) & 0xffff;

	/* display a message about the loaded quickload */
	image->message(" %s\nsize=%04X : start=%04X : end=%04X : exec=%04X",pgmname,size,start_addr[0],end_addr[0],exec_addr[0]);

	for (i = 0; i < size; i++)
	{
		j = (start_addr[0] + i) & 0xffff;
		if (image->fread(&data, 1) != 1)
		{
			snprintf(message, ARRAY_LENGTH(message), "%s: Unexpected EOF while writing byte to %04X", pgmname, (unsigned) j);
			image->seterror(IMAGE_ERROR_INVALIDIMAGE, message);
			image->message("%s: Unexpected EOF while writing byte to %04X", pgmname, (unsigned) j);
			return IMAGE_INIT_FAIL;
		}
		image->device().machine().device("maincpu")->memory().space(AS_PROGRAM).write_byte(j, data);
	}

	return IMAGE_INIT_PASS;
}



/*-------------------------------------------------
    QUICKLOAD_LOAD( super80 )
-------------------------------------------------*/

QUICKLOAD_LOAD( super80 )
{
	UINT16 exec_addr, start_addr, end_addr;
	int autorun;

	/* load the binary into memory */
	if (z80bin_load_file(&image, file_type, &exec_addr, &start_addr, &end_addr) == IMAGE_INIT_FAIL)
		return IMAGE_INIT_FAIL;

	/* is this file executable? */
	if (exec_addr != 0xffff)
	{
		/* check to see if autorun is on (I hate how this works) */
		autorun = image.device().machine().root_device().ioport("CONFIG")->read_safe(0xFF) & 1;

		if (autorun)
			image.device().machine().device("maincpu")->state().set_pc(exec_addr);
	}

	return IMAGE_INIT_PASS;
}

/*-------------------------------------------------
    QUICKLOAD_LOAD( mbee_z80bin )
-------------------------------------------------*/

QUICKLOAD_LOAD( mbee_z80bin )
{
	UINT16 execute_address, start_addr, end_addr;
	int autorun;

	/* load the binary into memory */
	if (z80bin_load_file(&image, file_type, &execute_address, &start_addr, &end_addr) == IMAGE_INIT_FAIL)
		return IMAGE_INIT_FAIL;

	/* is this file executable? */
	if (execute_address != 0xffff)
	{
		/* check to see if autorun is on (I hate how this works) */
		autorun = image.device().machine().root_device().ioport("CONFIG")->read_safe(0xFF) & 1;

		device_t *cpu = image.device().machine().device("maincpu");
		address_space &space = image.device().machine().device("maincpu")->memory().space(AS_PROGRAM);

		space.write_word(0xa6, execute_address);			/* fix the EXEC command */

		if (autorun)
		{
			space.write_word(0xa2, execute_address);		/* fix warm-start vector to get around some copy-protections */
			cpu->state().set_pc(execute_address);
		}
		else
		{
			space.write_word(0xa2, 0x8517);
		}
	}

	return IMAGE_INIT_PASS;
}

/*-------------------------------------------------
    QUICKLOAD_LOAD( sorcerer )
-------------------------------------------------*/

QUICKLOAD_LOAD( sorcerer )
{
	UINT16 execute_address, start_address, end_address;
	int autorun;
	/* load the binary into memory */
	if (z80bin_load_file(&image, file_type, &execute_address, &start_address, &end_address) == IMAGE_INIT_FAIL)
		return IMAGE_INIT_FAIL;

	/* is this file executable? */
	if (execute_address != 0xffff)
	{
		/* check to see if autorun is on (I hate how this works) */
		autorun = image.device().machine().root_device().ioport("CONFIG")->read_safe(0xFF) & 1;

		address_space &space = image.device().machine().device("maincpu")->memory().space(AS_PROGRAM);

		if ((execute_address >= 0xc000) && (execute_address <= 0xdfff) && (space.read_byte(0xdffa) != 0xc3))
			return IMAGE_INIT_FAIL;		/* can't run a program if the cartridge isn't in */

		/* Since Exidy Basic is by Microsoft, it needs some preprocessing before it can be run.
        1. A start address of 01D5 indicates a basic program which needs its pointers fixed up.
        2. If autorunning, jump to C689 (command processor), else jump to C3DD (READY prompt).
        Important addresses:
            01D5 = start (load) address of a conventional basic program
            C858 = an autorun basic program will have this exec address on the tape
            C3DD = part of basic that displays READY and lets user enter input */

		if ((start_address == 0x1d5) || (execute_address == 0xc858))
		{
			UINT8 i;
			static const UINT8 data[]={
				0xcd, 0x26, 0xc4,	// CALL C426    ;set up other pointers
				0x21, 0xd4, 1,		// LD HL,01D4   ;start of program address (used by C689)
				0x36, 0,		// LD (HL),00   ;make sure dummy end-of-line is there
				0xc3, 0x89, 0xc6	// JP C689  ;run program
			};

			for (i = 0; i < ARRAY_LENGTH(data); i++)
				space.write_byte(0xf01f + i, data[i]);

			if (!autorun)
				space.write_word(0xf028,0xc3dd);

			/* tell BASIC where program ends */
			space.write_byte(0x1b7, end_address & 0xff);
			space.write_byte(0x1b8, (end_address >> 8) & 0xff);

			if ((execute_address != 0xc858) && autorun)
				space.write_word(0xf028, execute_address);

			image.device().machine().device("maincpu")->state().set_pc(0xf01f);
		}
		else
		{
			if (autorun)
				image.device().machine().device("maincpu")->state().set_pc(execute_address);
		}

	}

	return IMAGE_INIT_PASS;
}
