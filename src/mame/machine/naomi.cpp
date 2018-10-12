// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Olivier Galibert, David Haywood, Samuele Zannoli, R. Belmont, ElSemi
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
#include "includes/dc.h"
#include "includes/naomi.h"
#include "sound/aica.h"

READ64_MEMBER(naomi_state::naomi_biose_idle_skip_r )
{
//  if (m_maincpu->pc()==0xc04173c)
//      m_maincpu->spin_until_time(attotime::from_usec(500));
//m_maincpu->spin_until_interrupt();
//  else
//      printf("%08x\n", m_maincpu->pc());

	return dc_ram[0x2ad238/8];
}

READ64_MEMBER(naomi_state::naomi_biosh_idle_skip_r )
{
//  if (m_maincpu->pc()==0xc045ffc)
//      m_maincpu->spin_until_time(attotime::from_usec(500));

//   printf("%08\n", m_maincpu->pc());

	return dc_ram[0x2b0600/8];
}

READ64_MEMBER(naomi_state::naomi2_biose_idle_skip_r )
{
//  if (m_maincpu->pc()==0xc04637c)
//      m_maincpu->spin_until_time(attotime::from_usec(500));
		//m_maincpu->spin_until_interrupt();
//  else
//      printf("%08x\n", m_maincpu->pc());

	return dc_ram[0x2b0600/8];
}

uint8_t naomi_state::asciihex_to_dec(uint8_t in)
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
		fatalerror("unexpected value in asciihex_to_dec\n");
	}


}

// development helper function
void naomi_state::create_pic_from_retdat()
{
	{
		memory_region * rgn_hexregion = memregion("pichex");
		memory_region * rgn_retregion = memregion("picreturn");
		memory_region * rgn_newregion = memregion("pic");
		int outcount = 0;

		if (rgn_hexregion && rgn_newregion)
		{
			uint8_t* hexregion = rgn_hexregion->base();
			uint8_t* newregion = rgn_newregion->base();


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
					uint8_t ascii1 = hexregion[hexoffs+0];
					uint8_t ascii2 = hexregion[hexoffs+1];
					uint8_t dec1 = asciihex_to_dec(ascii1);
					uint8_t dec2 = asciihex_to_dec(ascii2);
					uint8_t val = dec1 << 4 | dec2;

					//printf("%02x%02x", ascii1, ascii2);

					printf("%02x", val);

					newregion[outcount] = val;

					hexoffs+=2;
					outcount++;
				}

				hexoffs+=0x4; // skip running checksum + newline

				printf("\n");


			}

			if (rgn_retregion && rgn_newregion)
			{
				uint8_t* retregion = rgn_retregion->base();
				uint8_t* newregion = rgn_newregion->base();


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

			if (rgn_newregion)
			{
				uint8_t* newregion = rgn_newregion->base();

				FILE *fp;
				char filename[256];
				sprintf(filename,"picbin_%s", machine().system().name);
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

void naomi_state::set_drc_options()
{
	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY | SH2DRC_STRICT_PCREL);
	m_maincpu->sh2drc_add_fastram(0x00000000, 0x001fffff, true, m_rombase);
	m_maincpu->sh2drc_add_fastram(0x0c000000, 0x0dffffff, false, dc_ram);
}

void naomi_state::init_naomi()
{
	//m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2ad238, 0xc2ad23f, read64_delegate(FUNC(naomi_state::naomi_biose_idle_skip_r),this); // rev e bios
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2b0600, 0xc2b0607, read64_delegate(FUNC(naomi_state::naomi_biosh_idle_skip_r), this)); // rev h bios

	set_drc_options();
	create_pic_from_retdat();
}

void naomi2_state::init_naomi2()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2b0600, 0xc2b0607, read64_delegate(FUNC(naomi_state::naomi2_biose_idle_skip_r),this)); // rev e bios

	set_drc_options();
	create_pic_from_retdat();
}

INPUT_CHANGED_MEMBER(naomi_state::naomi_mp_w)
{
	m_mp_mux = newval;
}

CUSTOM_INPUT_MEMBER(naomi_state::naomi_mp_r)
{
	const char *tagptr = (const char *)param;
	uint8_t retval = 0;

	for (int i = 0x80; i >= 0x08; i >>= 1)
	{
		if (m_mp_mux & i)
		{
			ioport_port *port = ioport(tagptr);
			if (port != nullptr)
				retval |= port->read();
		}
		tagptr += strlen(tagptr) + 1;
	}
	return retval;
}

CUSTOM_INPUT_MEMBER(naomi_state::naomi_kb_r)
{
	// TODO: player 2 input reading
//  const int *tagptr = (const int *)param;
	uint8_t retval = 0;
	static const char *const keynames[] =
	{
		"P1.ROW0", "P1.ROW1", "P1.ROW2", "P1.ROW3", "P1.ROW4"
	};

	for(int i=0;i<5;i++)
	{
		uint32_t row;

		// read the current row
		row = ioport(keynames[i])->read();

		// if anything is pressed, convert the 32-bit raw value to keycode
		if(row != 0)
		{
			// base value x20
			retval = i * 0x20;
			for(int j=0;j<32;j++)
			{
				if(row & 1 << j)
					return retval + j;
			}
		}
	}

	return retval;
}

void naomi_state::init_naomi_mp()
{
	//m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2ad238, 0xc2ad23f, read64_delegate(FUNC(naomi_state::naomi_biose_idle_skip_r),this); // rev e bios
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2b0600, 0xc2b0607, read64_delegate(FUNC(naomi_state::naomi_biosh_idle_skip_r),this)); // rev h bios
	m_mp_mux = 0;

	set_drc_options();
	create_pic_from_retdat();
}

void naomi_state::init_naomigd()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2ad238, 0xc2ad23f, read64_delegate(FUNC(naomi_state::naomi_biose_idle_skip_r),this)); // rev e bios
	//m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2b0600, 0xc2b0607, read64_delegate(FUNC(naomi_state::naomi_biosh_idle_skip_r),this)); // rev h bios

	set_drc_options();
	create_pic_from_retdat();
}

void naomi_state::init_naomigd_mp()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2ad238, 0xc2ad23f, read64_delegate(FUNC(naomi_state::naomi_biose_idle_skip_r),this)); // rev e bios
	//m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2b0600, 0xc2b0607, read64_delegate(FUNC(naomi_state::naomi_biosh_idle_skip_r),this)); // rev h bios
	m_mp_mux = 0;

	set_drc_options();
	create_pic_from_retdat();
}


READ64_MEMBER(naomi_state::naomigd_ggxxsla_idle_skip_r )
{
//  if (m_maincpu->pc()==0x0c0c9adc)
//      m_maincpu->spin_until_time(attotime::from_usec(500));

	return dc_ram[0x1aae18/8];
}

void naomi_state::init_ggxxsla()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc1aae18, 0xc1aae1f, read64_delegate(FUNC(naomi_state::naomigd_ggxxsla_idle_skip_r),this));
	init_naomigd();
}

READ64_MEMBER(naomi_state::naomigd_ggxx_idle_skip_r )
{
//  if (m_maincpu->pc()==0xc0b5c3c) // or 0xc0bab0c
//      m_maincpu->spin_until_time(attotime::from_usec(500));

	return dc_ram[0x1837b8/8];
}


void naomi_state::init_ggxx()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc1837b8, 0xc1837bf, read64_delegate(FUNC(naomi_state::naomigd_ggxx_idle_skip_r),this));
	init_naomigd();
}

READ64_MEMBER(naomi_state::naomigd_ggxxrl_idle_skip_r )
{
//  if (m_maincpu->pc()==0xc0b84bc) // or 0xc0bab0c
//      m_maincpu->spin_until_time(attotime::from_usec(500));

	//printf("%08x\n", m_maincpu->pc());

	return dc_ram[0x18d6c8/8];
}

void naomi_state::init_ggxxrl()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc18d6c8, 0xc18d6cf, read64_delegate(FUNC(naomi_state::naomigd_ggxxrl_idle_skip_r),this));
	init_naomigd();
}

/* at least speeds up the annoying copyright screens ;-) */
READ64_MEMBER(naomi_state::naomigd_sfz3ugd_idle_skip_r )
{
//  if (m_maincpu->pc()==0xc36a2dc)
//      m_maincpu->spin_until_time(attotime::from_usec(500));

	return dc_ram[0x5dc900/8];
}

void naomi_state::init_sfz3ugd()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc5dc900, 0xc5dc907, read64_delegate(FUNC(naomi_state::naomigd_sfz3ugd_idle_skip_r),this));
	init_naomigd();
}


READ64_MEMBER(naomi_state::hotd2_idle_skip_r )
{
//  if (m_maincpu->pc()==0xc0cfcbc)
//      m_maincpu->spin_until_time(attotime::from_usec(500));
		//m_maincpu->spin_until_interrupt();
//  else
//  printf("%08x\n", m_maincpu->pc());

	return dc_ram[0xa25fb8/8];
}

void naomi_state::init_hotd2()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xca25fb8, 0xca25fbf, read64_delegate(FUNC(naomi_state::hotd2_idle_skip_r),this));
	set_drc_options();
}

// f355 PC=0xc065f7c RAM=0xc26dafc
