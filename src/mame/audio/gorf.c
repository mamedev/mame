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

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/custom.h"
#include "sound/samples.h"
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

static const char *GorfWordTable[] =
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

const char *gorf_sample_names[] =
{
 "*gorf","a.wav","a.wav","again.wav","am.wav","am.wav","and.wav","anhilatn.wav",
 "another.wav","another.wav","are.wav","are.wav",
 "avenger.wav","bad.wav","bad.wav","be.wav",
 "been.wav","but.wav","button.wav","cadet.wav",
 "cannot.wav","captain.wav","chronicl.wav","coin.wav","coins.wav","colonel.wav",
 "consciou.wav","defender.wav","destroy.wav","destroyd.wav",
 "doom.wav","draws.wav","empire.wav","end.wav",
 "enemy.wav","escape.wav","flagship.wav","for.wav","galactic.wav",
 "general.wav","gorf.wav","gorphian.wav","gorphian.wav","gorphins.wav",
 "hahahahu.wav","hahaher.wav","harder.wav","have.wav",
 "hitting.wav","i.wav","i.wav","impossib.wav","in.wav","insert.wav",
 "is.wav","live.wav","long.wav","meet.wav","move.wav",
 "my.wav","my.wav",
 "near.wav","next.wav","nice.wav","no.wav",
 "now.wav","pause.wav","player.wav","prepare.wav","prisonrs.wav",
 "promoted.wav","push.wav","robot.wav","robots.wav","robots.wav",
 "seek.wav","ship.wav","shot.wav","some.wav","space.wav","spause.wav",
 "survival.wav","take.wav","the.wav","the.wav","the.wav","time.wav",
 "to.wav","to.wav","unbeatab.wav",
 "warrior.wav","warriors.wav","will.wav",
 "you.wav","you.wav","you.wav","you.wav","your.wav","your.wav","yourself.wav",
 "s.wav","for.wav","for.wav","will.wav","gorph.wav",
// Missing Samples
 "coin.wav", "attack.wav","bite.wav","conquer.wav","devour.wav","dust.wav",
 "galaxy.wav","got.wav","power.wav","try.wav","supreme.wav","all.wav",
 "hail.wav","emperor.wav",
 0
} ;


/* Total word to join the phonemes together - Global to make it easier to use */
static char totalword[256], *totalword_ptr;
static char oldword[256];
static int plural = 0;

READ8_HANDLER( gorf_speech_r )
{
    int Phoneme,Intonation;
    int i = 0;

	UINT8 data = offset >> 8;
	offset &= 0xff;

    totalword_ptr = totalword;

    Phoneme = data & 0x3F;
    Intonation = data >> 6;

    logerror("Date : %d Speech : %s at intonation %d\n",Phoneme, PhonemeTable[Phoneme],Intonation);

	 if(Phoneme==63) {
   		sample_stop(0);
                if (strlen(totalword)>2) logerror("Clearing sample %s\n",totalword);
                totalword[0] = 0;				   /* Clear the total word stack */
					 return data;
    }

/* Phoneme to word translation */

	 if (strlen(totalword) == 0) {
		 strcpy(totalword,PhonemeTable[Phoneme]);	                   /* Copy over the first phoneme */
		 if (plural != 0) {
			 logerror("found a possible plural at %d\n",plural-1);
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

	 logerror("Total word = %s\n",totalword);

	 for (i=0; GorfWordTable[i]; i++) {
		 if (!strcmp(GorfWordTable[i],totalword)) {		   /* Scan the word (sample) table for the complete word */
			 if ((!strcmp("GDTO1RFYA2N",totalword)) || (!strcmp("RO1U1BAH1T",totalword)) || (!strcmp("KO1UH3I3E1N",totalword)) || (!strcmp("WORAYY1EH3R",totalword)) || (!strcmp("IN",totalword)) ) {              /* May be plural */
				 plural=i+1;
				 strcpy(oldword,totalword);
		  logerror("Storing sample position %d and copying string %s\n",plural,oldword);
			 } else {
             plural=0;
          }
          sample_start(0, i, 0);	                   /* play the sample at position of word */
          sample_set_freq(0, 11025);       /* play at correct rate */
          logerror("Playing sample %d",i);
          totalword[0] = 0;				   /* Clear the total word stack */
          return data;
       }
    }

    /* Note : We should really also use volume in this as well as frequency */
	 return data;				                   /* Return nicely */
}


UINT32 gorf_speech_status_r(void *param)
{
	return !sample_playing(0);
}
