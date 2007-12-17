/***********************************
 updated: 1997-04-09 08:46 TT
 updated  20-3-1998 LT Added colour changes on base explosion
 updated  02-6-1998 HJB copied from 8080bw and removed unneeded code
 *
 * Author      : Tormod Tjaberg
 * Created     : 1997-04-09
 * Description : Sound routines for the 'astinvad' games
 *
 * The samples were taken from Michael Strutt's (mstrutt@pixie.co.za)
 * excellent space invader emulator and converted to signed samples so
 * they would work under SEAL. The port info was also gleaned from
 * his emulator. These sounds should also work on all the invader games.
 *
 */

#include "driver.h"
#include "sound/samples.h"
#include "includes/astinvad.h"

static const char *astinvad_sample_names[] =
{
	"*invaders",
	"0.wav",
	"1.wav",
	"2.wav",
	"3.wav",
	"4.wav",
	"5.wav",
	"6.wav",
	"7.wav",
	"8.wav",
	0
};

/* sample sound IDs - must match sample file name table above */
enum
{
	SND_UFO = 0,
	SND_SHOT,
	SND_BASEHIT,
	SND_INVADERHIT,
	SND_FLEET1,
	SND_FLEET2,
	SND_FLEET3,
	SND_FLEET4,
	SND_UFOHIT
};


struct Samplesinterface astinvad_samples_interface =
{
	6,   /* channels */
	astinvad_sample_names
};


WRITE8_HANDLER( astinvad_sound1_w )
{
	static int state;

	int bitsGoneHi = data & ~state;

	sound_global_enable(data & 0x20);

	if (!(data & 1))
	{
		sample_stop(0);
	}

	if (bitsGoneHi & 0x01) sample_start(0, SND_UFO, 1);
	if (bitsGoneHi & 0x02) sample_start(1, SND_SHOT, 0);
	if (bitsGoneHi & 0x04) sample_start(2, SND_BASEHIT, 0);
	if (bitsGoneHi & 0x08) sample_start(3, SND_INVADERHIT, 0);

	astinvad_set_screen_red(data & 0x04);

	state = data;
}


WRITE8_HANDLER( astinvad_sound2_w )
{
	static int state;

	int bitsGoneHi = data & ~state;

	if (bitsGoneHi & 0x01) sample_start(5, SND_FLEET1, 0);
	if (bitsGoneHi & 0x02) sample_start(5, SND_FLEET2, 0);
	if (bitsGoneHi & 0x04) sample_start(5, SND_FLEET3, 0);
	if (bitsGoneHi & 0x08) sample_start(5, SND_FLEET4, 0);
	if (bitsGoneHi & 0x10) sample_start(4, SND_UFOHIT, 0);

	flip_screen_set(readinputport(3) & data & 0x20);

	state = data;
}


WRITE8_HANDLER( spaceint_sound1_w )
{
	static int state;

	int bitsGoneHi = data & ~state;

	if (!(data & 0x08))
	{
		sample_stop(0);
	}

	if (bitsGoneHi & 0x01) sample_start(1, SND_SHOT, 0);
	if (bitsGoneHi & 0x02) sample_start(2, SND_BASEHIT, 0);
	if (bitsGoneHi & 0x04) sample_start(4, SND_UFOHIT, 0);
	if (bitsGoneHi & 0x08) sample_start(0, SND_UFO, 1);

	if (bitsGoneHi & 0x10) sample_start(5, SND_FLEET1, 0);
	if (bitsGoneHi & 0x20) sample_start(5, SND_FLEET2, 0);
	if (bitsGoneHi & 0x40) sample_start(5, SND_FLEET3, 0);
	if (bitsGoneHi & 0x80) sample_start(5, SND_FLEET4, 0);

	state = data;
}


WRITE8_HANDLER( spaceint_sound2_w )
{
	static int state;

	int bitsGoneHi = data & ~state;

	sound_global_enable(data & 0x02);

	if (bitsGoneHi & 0x04) sample_start(3, SND_INVADERHIT, 0);

	flip_screen_set(readinputport(3) & data & 0x80);

	state = data;
}
