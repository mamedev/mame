// license:BSD-3-Clause
// copyright-holders:Mike Coates
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

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/astrocde.h"


/****************************************************************************
 * 64 Phonemes - currently 1 sample per phoneme, will be combined sometime!
 ****************************************************************************/

static const char *const PhonemeTable[65] =
{
	"EH3","EH2","EH1","PA0","DT" ,"A1" ,"A2" ,"ZH",
	"AH2","I3" ,"I2" ,"I1" ,"M"  ,"N"  ,"B"  ,"V",
	"CH" ,"SH" ,"Z"  ,"AW1","NG" ,"AH1","OO1","OO",
	"L"  ,"K"  ,"J"  ,"H"  ,"G"  ,"F"  ,"D"  ,"S",
	"A"  ,"AY" ,"Y1" ,"UH3","AH" ,"P"  ,"O"  ,"I",
	"U"  ,"Y"  ,"T"  ,"R"  ,"E"  ,"W"  ,"AE" ,"AE1",
	"AW2","UH2","UH1","UH" ,"O2" ,"O1" ,"IU" ,"U1",
	"THV","TH" ,"ER" ,"EH" ,"E1" ,"AW" ,"PA1","STOP",
	nullptr
};

/* Missing samples : ready,  from,  one,  bite,  youl,  explode,  if,  myself,  back,
   cant,  do,  wait,  worlings,  very,  babies,  breath,  fire,  beat,  rest,
   then,  never,  worlock,  escape,  door,  try,  any,  harder,  only,  meet,  with,
   doom,  pop,
   Problems with YOU and YOU'LL and YOU'DD */

static const char *const wowWordTable[] =
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
"YI3U", "Y1IUU", "YIUUI", "Y1IUU1U1", "YI3U1", "Y1IUUL", "YIUU1L", "Y1IUUD", "YO2O2R",nullptr
};

#define num_samples (sizeof(wowWordTable)/sizeof(char *))


const char *const wow_sample_names[] =
{
	"*wow",
	"a", "again", "ahh", "am", "and",
	"anew", "another", "any", "anyone", "appear", "are", "are", "babies", "back",
	"beat", "become", "best", "better", "bite", "bones", "breath", "but", "can", "cant",
	"chance", "chest", "coin", "dance", "destroy",
	"develop", "do", "dont", "doom", "door", "draw", "dungeon", "dungeons",
	"each", "eaten", "escape", "explode", "fear", "find", "find", "fire", "for", "from",
	"garwor", "get", "get", "get", "getting", "good", "hahahaha", "harder",
	"hasnt", "have", "heavyw", "hey", "hope",
	"hungry", "hungry", "hurry", "i", "i", "if", "if", "im", "i1", "ill", "in",
	"insert", "invisibl", "it", "lie", "magic",
	"magical", "me", "meet", "months",
	"my", "my", "my", "my", "my", "myself", "near", "never",
	"now", "of", "off", "one", "only", "oven", "pause", "pets", "powerful", "pop",
	"radar", "ready",
	"rest", "say", "science", "see", "spause", "start", "the", "the", "the", "the", "then",
	"through", "thurwor", "time", "to", "to", "to", "treasure", "try", "very", "wait",
	"war", "warrior", "watch", "we", "welcome",
	"were", "while", "will", "with", "wizard", "wont",
	"wor", "world", "worlings", "worlock",
	"you", "you", "you", "you", "you", "youl", "youl", "youd", "your",nullptr
};


READ8_MEMBER( astrocde_state::wow_speech_r )
{
	UINT8 data = offset >> 8;
#if USE_FAKE_VOTRAX
	int Phoneme/*, Intonation*/;
	int i = 0;
	offset &= 0xff;

	m_totalword_ptr = m_totalword;

	Phoneme = data & 0x3F;
	//Intonation = data >> 6;

	//logerror("Data : %d Speech : %s at intonation %d\n",Phoneme, PhonemeTable[Phoneme],Intonation);

	if(Phoneme==63) {
		m_samples->stop(0);
		//logerror("Clearing sample %s\n",m_totalword);
		m_totalword[0] = 0;                 /* Clear the total word stack */
		return data;
	}
	if (Phoneme==3)                        /* We know PA0 is never part of a word */
		m_totalword[0] = 0;                 /* Clear the total word stack */

	/* Phoneme to word translation */

	if (*(m_totalword) == 0) {
		strcpy(m_totalword,PhonemeTable[Phoneme]);                      /* Copy over the first phoneme */
		if (m_plural != 0) {
			//logerror("found a possible plural at %d\n",m_plural-1);
			if (!strcmp("S",m_totalword)) {         /* Plural check */
				m_samples->start(0, num_samples-2);      /* play the sample at position of word */
				m_samples->set_frequency(0, 11025);    /* play at correct rate */
				m_totalword[0] = 0;                 /* Clear the total word stack */
				m_oldword[0] = 0;                   /* Clear the total word stack */
				return data;
			} else {
					m_plural=0;
			}
		}
	} else
		strcat(m_totalword,PhonemeTable[Phoneme]);                      /* Copy over the first phoneme */

	//logerror("Total word = %s\n",m_totalword);

	for (i=0; wowWordTable[i]; i++) {
		if (!strcmp(wowWordTable[i],m_totalword)) {         /* Scan the word (sample) table for the complete word */
			/* WOW has Dungeon */
			if ((!strcmp("GDTO1RFYA2N",m_totalword)) || (!strcmp("RO1U1BAH1T",m_totalword)) || (!strcmp("KO1UH3I3E1N",m_totalword))) {        /* May be plural */
				m_plural=i+1;
				strcpy(m_oldword,m_totalword);
				//logerror("Storing sample position %d and copying string %s\n",m_plural,m_oldword);
			} else {
				m_plural=0;
			}
			m_samples->start(0, i);                      /* play the sample at position of word */
			m_samples->set_frequency(0, 11025);         /* play at correct rate */
			//logerror("Playing sample %d\n",i);
			m_totalword[0] = 0;                 /* Clear the total word stack */
			return data;
		}
	}
#else
	m_votrax->inflection_w(space, 0, data >> 6);
	m_votrax->write(space, 0, data);
#endif

	/* Note : We should really also use volume in this as well as frequency */
	return data;                                   /* Return nicely */
}


CUSTOM_INPUT_MEMBER( astrocde_state::wow_speech_status_r )
{
#if USE_FAKE_VOTRAX
	return !m_samples->playing(0);
#else
	return m_votrax->request();
#endif
}
