/**************************************************************************

    WOW/Votrax SC-01 Emulator

    Mike@Dissfulfils.co.uk

        Modified to match phonemes to words

        Ajudd@quantime.co.uk

**************************************************************************

wow_sh_start  - Start emulation, load samples from Votrax subdirectory
wow_sh_w      - Write data to votrax port
wow_sh_status - Return busy status (-1 = busy)
wow_port_2_r  - Returns status of voice port
wow_sh_ update- Null

**************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/samples.h"
#include "sound/custom.h"
#include "includes/astrocde.h"


/****************************************************************************
 * 64 Phonemes - currently 1 sample per phoneme, will be combined sometime!
 ****************************************************************************/

static const char *PhonemeTable[65] =
{
 "EH3","EH2","EH1","PA0","DT" ,"A1" ,"A2" ,"ZH",
 "AH2","I3" ,"I2" ,"I1" ,"M"  ,"N"  ,"B"  ,"V",
 "CH" ,"SH" ,"Z"  ,"AW1","NG" ,"AH1","OO1","OO",
 "L"  ,"K"  ,"J"  ,"H"  ,"G"  ,"F"  ,"D"  ,"S",
 "A"  ,"AY" ,"Y1" ,"UH3","AH" ,"P"  ,"O"  ,"I",
 "U"  ,"Y"  ,"T"  ,"R"  ,"E"  ,"W"  ,"AE" ,"AE1",
 "AW2","UH2","UH1","UH" ,"O2" ,"O1" ,"IU" ,"U1",
 "THV","TH" ,"ER" ,"EH" ,"E1" ,"AW" ,"PA1","STOP",
 0
};

/* Missing samples : ready.wav from.wav one.wav bite.wav youl.wav explode.wav if.wav myself.wav back.wav
   cant.wav do.wav wait.wav worlings.wav very.wav babies.wav breath.wav fire.wav beat.wav rest.wav
   then.wav never.wav worlock.wav escape.wav door.wav try.wav any.wav harder.wav only.wav meet.wav with.wav
   doom.wav pop.wav
   Problems with YOU and YOU'LL and YOU'DD */

static const char *wowWordTable[] =
{
"AH1I3Y1", "UH1GA1EH1N", "AHAH2", "AE1EH3M", "AE1EH3ND",
"anew.wav", "AH1NUHTHER", "AE1NY", "anyone.wav", "appear.wav", "AH1UH3R", "UHR", "BABYY1S", "BAE1EH3K",
"BE1T", "become.wav", "BEHST", "BEH1TER", "BUH3AH2YT", "bones.wav", "BRE1YTH", "but.wav", "can.wav", "KAE1EH3NT",
"chance.wav", "CHEHST", "KO1O2I3Y1N", "dance.wav", "DE1STRO1UH3I3AY",
"DE1VEH1LUH3PT", "DIUU", "DONT", "DUUM", "DOO1R", "draw.wav", "DUHNJEH1N", "DUHNJEH1NZ",
"each.wav", "eaten.wav", "EHSPA0KA2I3Y1P", "EHKPA0SPLOU1D", "fear.wav", "FAH1I3YND", "FAH1I3Y1ND", "FAH1EH3AYR", "FOR", "FRUHMM",
"garwor.wav", "GEHT", "GEH1T", "GEHEH3T", "GEHTING", "good.wav", "HAH1HAH1HAH1HAH1", "HAH1RDER",
"hasnt.wav", "have.wav", "HEH1I3VE1WA1I3Y1TS", "HAI1Y1", "HOP",
"HUHNGRY", "HUHNGGRY", "HERRY", "AH1EH3I3Y", "AH1UH3I3Y", "IF", "I1F", "AH1I3YM", "AH1EH3I3YL", "AH1I3Y1L", "IN1",
"INSERT", "invisibl.wav", "IT", "lie.wav", "MAE1EH3DJI1KUH1L",
"MAE1EH3DJI1KUH1L", "MEE1", "MEE1T", "months.wav",
"MAH1EH3I3Y", "MAH2AH2EH3I3Y", "MAH1I1Y", "MAH1I3Y1", "MAH1I3Y", "MAH1I3YSEHLF", "near.wav", "NEH1VER",
"NAH1UH3U1", "UHV", "AWF", "WUHN", "O1NLY", "UHVEHN", "PA1", "PEHTS", "PAH1WERFUH1L", "PAH1P",
"radar.wav", "REHDY",
"REHST", "say.wav", "SAH1I3AYEHNS", "SE1Y", "PA0", "start.wav", "THVAYAY", "THVUH", "THVUH1", "THUH1", "THVEH1N",
"THVRU", "thurwor.wav", "time.wav", "TU1", "TUU1", "TIUU1", "TREH1ZHERT", "TRAH1EH3I3Y", "VEHEH3RY", "WA2AYYT",
"WOO1R", "WORYER", "watch.wav", "WE1Y", "WEHLKUHM",
"WERR", "WAH1EH3I3L", "WIL", "WITH", "WIZERD", "wont.wav",
"WO1O2R", "WO1ERLD", "WORLINGS", "WORLUHK",
"YI3U", "Y1IUU", "YIUUI", "Y1IUU1U1", "YI3U1", "Y1IUUL", "YIUU1L", "Y1IUUD", "YO2O2R",0
};

#define num_samples (sizeof(wowWordTable)/sizeof(char *))


const char *wow_sample_names[] =
{
	"*wow",
	"a.wav", "again.wav", "ahh.wav", "am.wav", "and.wav",
	"anew.wav", "another.wav", "any.wav", "anyone.wav", "appear.wav", "are.wav", "are.wav", "babies.wav", "back.wav",
	"beat.wav", "become.wav", "best.wav", "better.wav", "bite.wav", "bones.wav", "breath.wav", "but.wav", "can.wav", "cant.wav",
	"chance.wav", "chest.wav", "coin.wav", "dance.wav", "destroy.wav",
	"develop.wav", "do.wav", "dont.wav", "doom.wav", "door.wav", "draw.wav", "dungeon.wav", "dungeons.wav",
	"each.wav", "eaten.wav", "escape.wav", "explode.wav", "fear.wav", "find.wav", "find.wav", "fire.wav", "for.wav", "from.wav",
	"garwor.wav", "get.wav", "get.wav", "get.wav", "getting.wav", "good.wav", "hahahaha.wav", "harder.wav",
	"hasnt.wav", "have.wav", "heavyw.wav", "hey.wav", "hope.wav",
	"hungry.wav", "hungry.wav", "hurry.wav", "i.wav", "i.wav", "if.wav", "if.wav", "im.wav", "i1.wav", "ill.wav", "in.wav",
	"insert.wav", "invisibl.wav", "it.wav", "lie.wav", "magic.wav",
	"magical.wav", "me.wav", "meet.wav", "months.wav",
	"my.wav", "my.wav", "my.wav", "my.wav", "my.wav", "myself.wav", "near.wav", "never.wav",
	"now.wav", "of.wav", "off.wav", "one.wav", "only.wav", "oven.wav", "pause.wav", "pets.wav", "powerful.wav", "pop.wav",
	"radar.wav", "ready.wav",
	"rest.wav", "say.wav", "science.wav", "see.wav", "spause.wav", "start.wav", "the.wav", "the.wav", "the.wav", "the.wav", "then.wav",
	"through.wav", "thurwor.wav", "time.wav", "to.wav", "to.wav", "to.wav", "treasure.wav", "try.wav", "very.wav", "wait.wav",
	"war.wav", "warrior.wav", "watch.wav", "we.wav", "welcome.wav",
	"were.wav", "while.wav", "will.wav", "with.wav", "wizard.wav", "wont.wav",
	"wor.wav", "world.wav", "worlings.wav", "worlock.wav",
	"you.wav", "you.wav", "you.wav", "you.wav", "you.wav", "youl.wav", "youl.wav", "youd.wav", "your.wav",0
};


/* Total word to join the phonemes together - Global to make it easier to use */

static char totalword[256], *totalword_ptr;
static char oldword[256];
static int plural;

READ8_HANDLER( wow_speech_r )
{
	int Phoneme,Intonation;
	int i = 0;

	UINT8 data = offset >> 8;
	offset &= 0xff;

	totalword_ptr = totalword;

	Phoneme = data & 0x3F;
	Intonation = data >> 6;

//  logerror("Data : %d Speech : %s at intonation %d\n",Phoneme, PhonemeTable[Phoneme],Intonation);

	if(Phoneme==63) {
   		sample_stop(0);
//              logerror("Clearing sample %s\n",totalword);
				totalword[0] = 0;				   /* Clear the total word stack */
				return data;
	}
	if (Phoneme==3)						   /* We know PA0 is never part of a word */
				totalword[0] = 0;				   /* Clear the total word stack */

/* Phoneme to word translation */

	if (strlen(totalword) == 0) {
	   strcpy(totalword,PhonemeTable[Phoneme]);	                   /* Copy over the first phoneme */
	   if (plural != 0) {
//        logerror("found a possible plural at %d\n",plural-1);
		  if (!strcmp("S",totalword)) {		   /* Plural check */
			 sample_start(0, num_samples-2, 0);	   /* play the sample at position of word */
			 sample_set_freq(0, 11025);    /* play at correct rate */
			 totalword[0] = 0;				   /* Clear the total word stack */
			 oldword[0] = 0;				   /* Clear the total word stack */
			 return data;
		  } else {
			 plural=0;
		  }
	   }
	} else
	   strcat(totalword,PhonemeTable[Phoneme]);	                   /* Copy over the first phoneme */

//  logerror("Total word = %s\n",totalword);

	for (i=0; wowWordTable[i]; i++) {
	   if (!strcmp(wowWordTable[i],totalword)) {		   /* Scan the word (sample) table for the complete word */
	  /* WOW has Dungeon */
		  if ((!strcmp("GDTO1RFYA2N",totalword)) || (!strcmp("RO1U1BAH1T",totalword)) || (!strcmp("KO1UH3I3E1N",totalword))) {		   /* May be plural */
			 plural=i+1;
			 strcpy(oldword,totalword);
//       logerror("Storing sample position %d and copying string %s\n",plural,oldword);
		  } else {
			 plural=0;
		  }
		  sample_start(0, i, 0);	                   /* play the sample at position of word */
		  sample_set_freq(0, 11025);         /* play at correct rate */
//        logerror("Playing sample %d\n",i);
		  totalword[0] = 0;				   /* Clear the total word stack */
		  return data;
	   }
	}

	/* Note : We should really also use volume in this as well as frequency */
	return data;				                   /* Return nicely */
}


UINT32 wow_speech_status_r(void *param)
{
	return !sample_playing(0);
}
