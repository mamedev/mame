#include "emu.h"
#include "sound/okim6295.h"
#include "includes/toaplan2.h"

/****************************************************************************
  The Toaplan 2 hardware with V25 secondary CPU controls the sound through
  to a YM2151 and OKI M6295 on some boards. Here we just interpret some of
  commands sent to the V25, directly onto the OKI M6295

  These tables convert commands sent from the main CPU, into sample numbers
  played back by the sound processor.
  The ADPCM ROMs contain intrument samples which are sequenced by the
  sound processor to create some of the backing tracks. This is beyond the
  scope of this playback file. Time would be better spent elsewhere.
****************************************************************************/

static const UINT8 fixeight_cmd_snd[128] =
{
/* Some sound commands are mixed with tones produced by the FM chip */
/* Probably 96(60H), 82(52H), 80(50H) and 70(46H) and maybe others */
/*00*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*08*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*10*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*18*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*20*/  0x18, 0x3e, 0x37, 0x48, 0x38, 0x49, 0x4a, 0x4b,
/*28*/  0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x53, 0x54, 0x51,
/*30*/  0x52, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
/*38*/  0x07, 0x08, 0x00, 0x1b, 0x00, 0x43, 0x00, 0x00,
/*40*/  0x00, 0x47, 0x00, 0x3a, 0x44, 0x0a, 0x0a, 0x06,
/*48*/  0x3c, 0x46, 0x3f, 0x45, 0x00, 0x02, 0x04, 0x10,
/*50*/  0x0f, 0x11, 0x09, 0x0d, 0x0c, 0x0b, 0x00, 0x00,
/*58*/  0x00, 0x15, 0x3d, 0x3f, 0x1e, 0x1c, 0x19, 0x13,
/*60*/  0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*68*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*70*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*78*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void play_oki_sound(okim6295_device *oki, int game_sound, int data)
{
	int status = oki->read_status();

	logerror("Playing sample %02x from command %02x\n",game_sound,data);

	if (game_sound != 0)
	{
		if ((status & 0x01) == 0) {
			oki->write_command(0x80 | game_sound);
			oki->write_command(0x11);
		}
		else if ((status & 0x02) == 0) {
			oki->write_command(0x80 | game_sound);
			oki->write_command(0x21);
		}
		else if ((status & 0x04) == 0) {
			oki->write_command(0x80 | game_sound);
			oki->write_command(0x41);
		}
		else if ((status & 0x08) == 0) {
			oki->write_command(0x80 | game_sound);
			oki->write_command(0x81);
		}
	}
}

void fixeight_okisnd_w(device_t *device, int data)
{
//  popmessage("Writing %04x to Sound CPU",data);

	okim6295_device *oki = downcast<okim6295_device *>(device);
	if (data == 0)
	{
		oki->write_command(0x78);		/* Stop playing effects */
	}
	else if ((data > 0) && (data < 128))
	{
		play_oki_sound(oki, fixeight_cmd_snd[data], data);
	}
}
