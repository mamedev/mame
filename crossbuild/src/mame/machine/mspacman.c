#include "driver.h"
#include "includes/pacman.h"


/*
  It turns out the bootleg is the decrypted version with the checksum check
  removed and interrupt mode changed to 1.

  u7= boot 4($3000-$3fff) other than 4 bytes(checksum check and interupt mode)
  u6= boot 6($9000-$9fff). The second half of u6 gets mirrored to the second
      half of boot5($8800-$8fff) where it is used.
  u5= first half of boot5($8000-$87ff)

  $8000-$81ef contain 8 byte patches that are overlayed on locations in $0000-$2fff

  The Ms Pacman daughter board is not activated with the mainboard.  As near as I
  can tell it requires a sequence of bytes starting at around 3176 and ending with
  3196. The location of the bytes doesn't seem to matter, just that those bytes
  are executed. That sequence of bytes includes a write to 5006 so I'm using that
  to bankswitch, but that is not accurate. The actual change is I believe at $317D.
  The daughterboard can also be deactivated. A read to any of the several 8 byte
  chunks listed will cause the Ms Pac roms to disappear and Pacman to show up.
  As a result I couldn't verify what they contained. They should be the same as the
  pacman roms, but I don't see how it could matter. These areas can be accessed by
  the random number generator at $2a23 and the board is deactivated but is
  immediately reactivated. So the net result is no change. The exact trigger for
  this is not yet known.

  deactivation, 8 bytes starting at:
  $38,$3b0,$1600,$2120,$3ff0,$8000

  David Widel
  d_widel@hotmail.com
*/




static UINT8 decryptd(UINT8 e)
{
	UINT8 d;

	d  = (e & 0x80) >> 3;
	d |= (e & 0x40) >> 3;
	d |= (e & 0x20)     ;
	d |= (e & 0x10) << 2;
	d |= (e & 0x08) >> 1;
	d |= (e & 0x04) >> 1;
	d |= (e & 0x02) >> 1;
	d |= (e & 0x01) << 7;

	return d;
}

static UINT32 decrypta1(UINT32 e)
{
	UINT32 d;

	d  = (e & 0x800)     ;
	d |= (e & 0x400) >> 7;
	d |= (e & 0x200) >> 2;
	d |= (e & 0x100) << 1;
	d |= (e & 0x80) << 3;
	d |= (e & 0x40) << 2;
	d |= (e & 0x20) << 1;
	d |= (e & 0x10) << 1;
	d |= (e & 0x08) << 1;
	d |= (e & 0x04)     ;
	d |= (e & 0x02)     ;
	d |= (e & 0x01)     ;

	return d;
}

static UINT32 decrypta2(UINT32 e)
{
	UINT32 d;
	d  = (e & 0x800)     ;
	d |= (e & 0x400) >> 2;
	d |= (e & 0x200) >> 2;
	d |= (e & 0x100) >> 3;
	d |= (e & 0x80) << 2;
	d |= (e & 0x40) << 4;
	d |= (e & 0x20) << 1;
	d |= (e & 0x10) >> 1;
	d |= (e & 0x08) << 1;
	d |= (e & 0x04)     ;
	d |= (e & 0x02)     ;
	d |= (e & 0x01)     ;

	return d;
}




static void mspacman_decode(void)
{
	int i;
	UINT8 *RAM;



	/* CPU ROMs */

	RAM = memory_region(REGION_CPU1);
	for (i = 0; i < 0x1000; i++)
	{
	RAM[0x10000+i] = RAM[0x0000+i];
	RAM[0x11000+i] = RAM[0x1000+i];
	RAM[0x12000+i] = RAM[0x2000+i];
	RAM[0x1a000+i] = RAM[0x2000+i];  /*not needed but it's there*/
	RAM[0x1b000+i] = RAM[0x3000+i];  /*not needed but it's there*/

	}


	for (i = 0; i < 0x1000; i++)
	{
		RAM[decrypta1(i)+0x13000] = decryptd(RAM[0xb000+i]);	/*u7*/
		RAM[decrypta1(i)+0x19000] = decryptd(RAM[0x9000+i]);	/*u6*/
	}

	for (i = 0; i < 0x800; i++)
	{
		RAM[decrypta2(i)+0x18000] = decryptd(RAM[0x8000+i]);  	/*u5*/
		RAM[0x18800+i] = RAM[0x19800+i];
	}



	for (i = 0; i < 8; i++)
	{
		RAM[0x10410+i] = RAM[0x18008+i];
		RAM[0x108E0+i] = RAM[0x181D8+i];
		RAM[0x10A30+i] = RAM[0x18118+i];
		RAM[0x10BD0+i] = RAM[0x180D8+i];
		RAM[0x10C20+i] = RAM[0x18120+i];
		RAM[0x10E58+i] = RAM[0x18168+i];
		RAM[0x10EA8+i] = RAM[0x18198+i];

		RAM[0x11000+i] = RAM[0x18020+i];
		RAM[0x11008+i] = RAM[0x18010+i];
		RAM[0x11288+i] = RAM[0x18098+i];
		RAM[0x11348+i] = RAM[0x18048+i];
		RAM[0x11688+i] = RAM[0x18088+i];
		RAM[0x116B0+i] = RAM[0x18188+i];
		RAM[0x116D8+i] = RAM[0x180C8+i];
		RAM[0x116F8+i] = RAM[0x181C8+i];
		RAM[0x119A8+i] = RAM[0x180A8+i];
		RAM[0x119B8+i] = RAM[0x181A8+i];

		RAM[0x12060+i] = RAM[0x18148+i];
		RAM[0x12108+i] = RAM[0x18018+i];
		RAM[0x121A0+i] = RAM[0x181A0+i];
		RAM[0x12298+i] = RAM[0x180A0+i];
		RAM[0x123E0+i] = RAM[0x180E8+i];
		RAM[0x12418+i] = RAM[0x18000+i];
		RAM[0x12448+i] = RAM[0x18058+i];
		RAM[0x12470+i] = RAM[0x18140+i];
		RAM[0x12488+i] = RAM[0x18080+i];
		RAM[0x124B0+i] = RAM[0x18180+i];
		RAM[0x124D8+i] = RAM[0x180C0+i];
		RAM[0x124F8+i] = RAM[0x181C0+i];
		RAM[0x12748+i] = RAM[0x18050+i];
		RAM[0x12780+i] = RAM[0x18090+i];
		RAM[0x127B8+i] = RAM[0x18190+i];
		RAM[0x12800+i] = RAM[0x18028+i];
		RAM[0x12B20+i] = RAM[0x18100+i];
		RAM[0x12B30+i] = RAM[0x18110+i];
		RAM[0x12BF0+i] = RAM[0x181D0+i];
		RAM[0x12CC0+i] = RAM[0x180D0+i];
		RAM[0x12CD8+i] = RAM[0x180E0+i];
		RAM[0x12CF0+i] = RAM[0x181E0+i];
		RAM[0x12D60+i] = RAM[0x18160+i];
	}
}


MACHINE_RESET( mspacman )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	mspacman_decode();

	memory_configure_bank(1, 0, 2, &RAM[0x00000], 0x10000);
	memory_set_bank(1, 0);
}


WRITE8_HANDLER( mspacman_activate_rom )
{
	if (data==1) memory_set_bank(1, 1);
}


