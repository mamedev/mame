/**************************************************************************

    Gorf/Votrax SC-01 Emulator

    Mike@Dissfulfils.co.uk

        Modified to match phonemes to words

        Ajudd@quantime.co.uk

**************************************************************************

gorf_sh_w      - Write data to votrax port
gorf_sh_status - Return busy status (-1 = busy)
gorf_port_2_r  - Returns status of voice port
gorf_sh_ update- Null

**************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/samples.h"
#include "sound/votrax.h"
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
 0
};

static const char *const GorfWordTable[] =
{
 "A2AYY1","A2E1","UH1GEH1I3N","AE1EH2M","AEM",
 "AE1EH3ND","UH1NAH2I1YLA2SHUH2N","AH2NUHTHER","AH1NUHTHVRR",
 "AH1R","UHR","UH1VEH1EH3NNDJER","BAEEH3D","BAEEH1D","BE",
 "BEH3EH1N","buht","BUH1DTTEH2NN","KUHDEH2T",
 "KAE1NUH1T","KAE1EH3PTI3N",
 "KRAH2UH3NI3KUH3O2LZ","KO1UH3I3E1N","KO1UH3I3E1NS",
 "KERNAH2L","KAH1NCHEHSNEHS","DE1FEH1NDER",
 "DE1STRO1I1Y","DE1STRO1I1Y1D",
 "DU1UM","DRAW1S","EHMPAH2I3YR","EHND",
 "EH1NEH1MY","EH1SKA1E1P","FLEHGSHIP",
 "FOR","GUH1LAEKTI1K",
 "DJEH2NERUH3L","GDTO1O1RRFF","GDTO1RFYA2N","GDTO1RFE1EH2N","GDTO1RFYA2NS",
 "HAH1HAH1HAH1HAH1","hahaher.wav","HUHRDER",
 "HAE1EH3V","HI1TI1NG","AH1I1Y", "AH1I1Y1","I1MPAH1SI1BL",
 "IN*","INSERT","I1S","LI1V","LAWNG","MEE1T","MUU1V",
 "MAH2I1Y","MAH2I3Y","NIR","NEHKST","NUH3AH2YS","NO",
 "NAH1O1U1W","PA1","PLA1AYER","PRE1PAE1ER","PRI1SI3NEH3RS",
 "PRUH2MOTEH3D","POO1IUSH","RO1U1BAH1T","RO1U1BAH1TS",
 "RO1U1BAH1UH3TS","SEK", "SHIP","SHAH1UH3T","SUHM","SPA2I3YS","PA0",
 "SERVAH2I1Y1VUH3L","TAK","THVUH","THVUH1",
 "THUH","TAH1EH3YM","TU","TIUU1",
 "UH2NBE1AYTUH3BUH3L",
 "WORAYY1EH3R","WORAYY1EH3RS","WI1L",
 "Y1I3U1","YIUU1U1","YI1U1U1","Y1IUU1U1","Y1I1U1U1","YOR","YU1O1RSEH1LF","s.wav",
 "FO1R","FO2R","WIL","GDTO1RVYA2N",

 "KO1UH3I3AYNN",
 "UH1TAEEH3K","BAH2I3Y1T","KAH1NKER","DYVAH1U1ER","DUHST","GAE1LUH1KSY","GAH1EH3T",
 "PAH1I1R","TRAH2I1Y","SU1PRE1N","AWL","HA2AYL",
 "EH1MPAH1I1R",
0
};

#define num_samples (sizeof(GorfWordTable)/sizeof(char *))

const char *const gorf_sample_names[] =
{
 "*gorf","a","a","again","am","am","and","anhilatn",
 "another","another","are","are",
 "avenger","bad","bad","be",
 "been","but","button","cadet",
 "cannot","captain","chronicl","coin","coins","colonel",
 "consciou","defender","destroy","destroyd",
 "doom","draws","empire","end",
 "enemy","escape","flagship","for","galactic",
 "general","gorf","gorphian","gorphian","gorphins",
 "hahahahu","hahaher","harder","have",
 "hitting","i","i","impossib","in","insert",
 "is","live","long","meet","move",
 "my","my",
 "near","next","nice","no",
 "now","pause","player","prepare","prisonrs",
 "promoted","push","robot","robots","robots",
 "seek","ship","shot","some","space","spause",
 "survival","take","the","the","the","time",
 "to","to","unbeatab",
 "warrior","warriors","will",
 "you","you","you","you","your","your","yourself",
 "s","for","for","will","gorph",
// Missing Samples
 "coin", "attack","bite","conquer","devour","dust",
 "galaxy","got","power","try","supreme","all",
 "hail","emperor",
 0
};


READ8_HANDLER( gorf_speech_r )
{
	UINT8 data = offset >> 8;
#if USE_FAKE_VOTRAX
	astrocde_state *state = space->machine().driver_data<astrocde_state>();
	samples_device *samples = space->machine().device<samples_device>("samples");
	int Phoneme, Intonation;
	int i = 0;
	offset &= 0xff;

	state->m_totalword_ptr = state->m_totalword;

	Phoneme = data & 0x3F;
	Intonation = data >> 6;

	logerror("Date : %d Speech : %s at intonation %d\n",Phoneme, PhonemeTable[Phoneme],Intonation);

	if(Phoneme==63) {
		samples->stop(0);
		if (strlen(state->m_totalword)>2) logerror("Clearing sample %s\n",state->m_totalword);
		state->m_totalword[0] = 0;				   /* Clear the total word stack */
		return data;
	}

	/* Phoneme to word translation */

	if (*(state->m_totalword) == 0) {
		strcpy(state->m_totalword,PhonemeTable[Phoneme]);	                   /* Copy over the first phoneme */
		if (state->m_plural != 0) {
			logerror("found a possible plural at %d\n",state->m_plural-1);
			if (!strcmp("S",state->m_totalword)) {		   /* Plural check */
				samples->start(0, num_samples-2);	   /* play the sample at position of word */
				samples->set_frequency(0, 11025);    /* play at correct rate */
				state->m_totalword[0] = 0;				   /* Clear the total word stack */
				state->m_oldword[0] = 0;				   /* Clear the total word stack */
				return data;
			} else {
				state->m_plural=0;
			}
		}
	} else
		strcat(state->m_totalword,PhonemeTable[Phoneme]);	                   /* Copy over the first phoneme */

	logerror("Total word = %s\n",state->m_totalword);

	for (i=0; GorfWordTable[i]; i++) {
		if (!strcmp(GorfWordTable[i],state->m_totalword)) {		   /* Scan the word (sample) table for the complete word */
			if ((!strcmp("GDTO1RFYA2N",state->m_totalword)) || (!strcmp("RO1U1BAH1T",state->m_totalword)) || (!strcmp("KO1UH3I3E1N",state->m_totalword)) || (!strcmp("WORAYY1EH3R",state->m_totalword)) || (!strcmp("IN",state->m_totalword)) ) {              /* May be state->m_plural */
				state->m_plural=i+1;
				strcpy(state->m_oldword,state->m_totalword);
				logerror("Storing sample position %d and copying string %s\n",state->m_plural,state->m_oldword);
			} else {
				state->m_plural=0;
			}
			samples->start(0, i);	                   /* play the sample at position of word */
			samples->set_frequency(0, 11025);       /* play at correct rate */
			logerror("Playing sample %d",i);
			state->m_totalword[0] = 0;				   /* Clear the total word stack */
			return data;
		}
	}
#else
	votrax_sc01_device *votrax = space->machine().device<votrax_sc01_device>("votrax");
	votrax->inflection_w(*space, 0, data >> 6);
	votrax->write(*space, 0, data);
#endif

	/* Note : We should really also use volume in this as well as frequency */
	return data;				                   /* Return nicely */
}


CUSTOM_INPUT( gorf_speech_status_r )
{
#if USE_FAKE_VOTRAX
	samples_device *samples = field.machine().device<samples_device>("samples");
	return !samples->playing(0);
#else
	votrax_sc01_device *votrax = field.machine().device<votrax_sc01_device>("votrax");
	return votrax->request();
#endif
}
