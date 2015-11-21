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
//  if (space.device().safe_pc()==0xc04173c)
//      space.device().execute().spin_until_time(attotime::from_usec(500));
		//space.device().execute().spin_until_interrupt();
//  else
//      printf("%08x\n", space.device().safe_pc());

	return dc_ram[0x2ad238/8];
}

READ64_MEMBER(naomi_state::naomi_biosh_idle_skip_r )
{
//  if (space.device().safe_pc()==0xc045ffc)
//      space.device().execute().spin_until_time(attotime::from_usec(500));

//   printf("%08x\n", space.device().safe_pc());

	return dc_ram[0x2b0600/8];
}

READ64_MEMBER(naomi_state::naomi2_biose_idle_skip_r )
{
//  if (space.device().safe_pc()==0xc04637c)
//      space.device().execute().spin_until_time(attotime::from_usec(500));
		//space.device().execute().spin_until_interrupt();
//  else
//      printf("%08x\n", space.device().safe_pc());

	return dc_ram[0x2b0600/8];
}

UINT8 naomi_state::asciihex_to_dec(UINT8 in)
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
		UINT8* hexregion = memregion("pichex")->base();
		UINT8* retregion = memregion("picreturn")->base();
		UINT8* newregion = memregion("pic")->base();
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

DRIVER_INIT_MEMBER(naomi_state,naomi)
{
	//m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2ad238, 0xc2ad23f, read64_delegate(FUNC(naomi_state::naomi_biose_idle_skip_r),this); // rev e bios
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2b0600, 0xc2b0607, read64_delegate(FUNC(naomi_state::naomi_biosh_idle_skip_r),this)); // rev h bios
	actel_id = 0xffff;

	create_pic_from_retdat();
}

DRIVER_INIT_MEMBER(naomi_state,naomi2)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2b0600, 0xc2b0607, read64_delegate(FUNC(naomi_state::naomi2_biose_idle_skip_r),this)); // rev e bios
	actel_id = 0xffff;

	create_pic_from_retdat();
}

INPUT_CHANGED_MEMBER(naomi_state::naomi_mp_w)
{
	m_mp_mux = newval;
}
CUSTOM_INPUT_MEMBER(naomi_state::naomi_mp_r)
{
	const char *tagptr = (const char *)param;
	UINT8 retval = 0;

	for (int i = 0x80; i >= 0x08; i >>= 1)
	{
		if (m_mp_mux & i)
			retval |= read_safe(ioport(tagptr), 0);
		tagptr += strlen(tagptr) + 1;
	}
	return retval;
}

DRIVER_INIT_MEMBER(naomi_state,naomi_mp)
{
	//m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2ad238, 0xc2ad23f, read64_delegate(FUNC(naomi_state::naomi_biose_idle_skip_r),this); // rev e bios
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2b0600, 0xc2b0607, read64_delegate(FUNC(naomi_state::naomi_biosh_idle_skip_r),this)); // rev h bios
	actel_id = 0xffff;
	m_mp_mux = 0;

	create_pic_from_retdat();
}

DRIVER_INIT_MEMBER(naomi_state,naomigd)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2ad238, 0xc2ad23f, read64_delegate(FUNC(naomi_state::naomi_biose_idle_skip_r),this)); // rev e bios
	//m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2b0600, 0xc2b0607, read64_delegate(FUNC(naomi_state::naomi_biosh_idle_skip_r),this)); // rev h bios
	actel_id = 0xffff;

	create_pic_from_retdat();
}

DRIVER_INIT_MEMBER(naomi_state,naomigd_mp)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2ad238, 0xc2ad23f, read64_delegate(FUNC(naomi_state::naomi_biose_idle_skip_r),this)); // rev e bios
	//m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2b0600, 0xc2b0607, read64_delegate(FUNC(naomi_state::naomi_biosh_idle_skip_r),this)); // rev h bios
	actel_id = 0xffff;
	m_mp_mux = 0;

	create_pic_from_retdat();
}


READ64_MEMBER(naomi_state::naomigd_ggxxsla_idle_skip_r )
{
//  if (space.device().safe_pc()==0x0c0c9adc)
//      space.device().execute().spin_until_time(attotime::from_usec(500));

	return dc_ram[0x1aae18/8];
}

DRIVER_INIT_MEMBER(naomi_state,ggxxsla)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc1aae18, 0xc1aae1f, read64_delegate(FUNC(naomi_state::naomigd_ggxxsla_idle_skip_r),this));
	DRIVER_INIT_CALL(naomigd);
}

READ64_MEMBER(naomi_state::naomigd_ggxx_idle_skip_r )
{
//  if (space.device().safe_pc()==0xc0b5c3c) // or 0xc0bab0c
//      space.device().execute().spin_until_time(attotime::from_usec(500));

	return dc_ram[0x1837b8/8];
}


DRIVER_INIT_MEMBER(naomi_state,ggxx)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc1837b8, 0xc1837bf, read64_delegate(FUNC(naomi_state::naomigd_ggxx_idle_skip_r),this));
	DRIVER_INIT_CALL(naomigd);
}

READ64_MEMBER(naomi_state::naomigd_ggxxrl_idle_skip_r )
{
//  if (space.device().safe_pc()==0xc0b84bc) // or 0xc0bab0c
//      space.device().execute().spin_until_time(attotime::from_usec(500));

	//printf("%08x\n", space.device().safe_pc());

	return dc_ram[0x18d6c8/8];
}

DRIVER_INIT_MEMBER(naomi_state,ggxxrl)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc18d6c8, 0xc18d6cf, read64_delegate(FUNC(naomi_state::naomigd_ggxxrl_idle_skip_r),this));
	DRIVER_INIT_CALL(naomigd);
}

/* at least speeds up the annoying copyright screens ;-) */
READ64_MEMBER(naomi_state::naomigd_sfz3ugd_idle_skip_r )
{
//  if (space.device().safe_pc()==0xc36a2dc)
//      space.device().execute().spin_until_time(attotime::from_usec(500));

	return dc_ram[0x5dc900/8];
}

DRIVER_INIT_MEMBER(naomi_state,sfz3ugd)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc5dc900, 0xc5dc907, read64_delegate(FUNC(naomi_state::naomigd_sfz3ugd_idle_skip_r),this));
	DRIVER_INIT_CALL(naomigd);
}


DRIVER_INIT_MEMBER(naomi_state,qmegamis)
{
	DRIVER_INIT_CALL(naomi);
	actel_id = 0; //FIXME: correct value
}

DRIVER_INIT_MEMBER(naomi_state,mvsc2)
{
	DRIVER_INIT_CALL(naomi);
	actel_id = 0; //FIXME: correct value
}

DRIVER_INIT_MEMBER(naomi_state,gram2000)
{
	DRIVER_INIT_CALL(naomi);
	actel_id = 0; //FIXME: correct value
}

DRIVER_INIT_MEMBER(naomi_state,vf4evoct)
{
	DRIVER_INIT_CALL(naomi2);
	actel_id = 0; //FIXME: correct value
}

DRIVER_INIT_MEMBER(naomi_state,kick4csh)
{
	DRIVER_INIT_CALL(naomi);
	actel_id = 0; //FIXME: correct value
}

READ64_MEMBER(naomi_state::hotd2_idle_skip_r )
{
//  if (space.device().safe_pc()==0xc0cfcbc)
//      space.device().execute().spin_until_time(attotime::from_usec(500));
		//space.device().execute().spin_until_interrupt();
//  else
//  printf("%08x\n", space.device().safe_pc());

	return dc_ram[0xa25fb8/8];
}

DRIVER_INIT_MEMBER(naomi_state,hotd2)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xca25fb8, 0xca25fbf, read64_delegate(FUNC(naomi_state::hotd2_idle_skip_r),this));
}

// f355 PC=0xc065f7c RAM=0xc26dafc
