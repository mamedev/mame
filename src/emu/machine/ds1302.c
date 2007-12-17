#include "driver.h"
#include "ds1302.h"

/********************
    DALLAS
    DS1302

   RTC+BACKUP RAM

   Emulation by ElSemi
********************/

/********************
Missing:
Burst Mode
Clock programming   (useless)
********************/

static UINT32 ShiftIn=0;
static UINT8 ShiftOut=0;
static UINT8 ICount=0;
static UINT8 LastClk;
static UINT8 LastCmd=0;
static UINT8 SRAM[0x20];


void DS1302_RST(UINT8 val)
{
	if(!val)
	{
		ICount=0;
		LastClk=0;
		ShiftIn=0;
		LastCmd=0;
	}
}

void DS1302_DAT(UINT8 val)
{
	if(val)
		ShiftIn|=(1<<ICount);
	else
		ShiftIn&=~(1<<ICount);
}

static UINT8 bcd(UINT8 v)
{
	return ((v/10)<<4)|(v%10);
}

void DS1302_CLK(UINT8 val)
{
	if(val!=LastClk)
	{
		if(val)	//Rising, shift in command
		{
			ICount++;
			if(ICount==8)	//Command start
			{
				mame_system_time systime;
				mame_get_base_datetime(Machine, &systime);

				switch(ShiftIn)
				{
					case 0x81:	//Sec
						ShiftOut=bcd(systime.local_time.second);
						break;
					case 0x83:	//Min
						ShiftOut=bcd(systime.local_time.minute);
						break;
					case 0x85:	//Hour
						ShiftOut=bcd(systime.local_time.hour);
						break;
					case 0x87:	//Day
						ShiftOut=bcd(systime.local_time.mday);
						break;
					case 0x89:	//Month
						ShiftOut=bcd(systime.local_time.month+1);
						break;
					case 0x8b:	//weekday
						ShiftOut=bcd(systime.local_time.weekday);
						break;
					case 0x8d:	//Year
						ShiftOut=bcd(systime.local_time.year%100);
						break;
					default:
						ShiftOut=0x0;
				}
				if(ShiftIn>0xc0)
					ShiftOut=SRAM[(ShiftIn>>1)&0x1f];
				LastCmd=ShiftIn&0xff;
				ICount++;
			}
			if(ICount==17 && !(LastCmd&1))
			{
				UINT8 val=(ShiftIn>>9)&0xff;

				switch(LastCmd)
				{
					case 0x80:	//Sec

						break;
					case 0x82:	//Min

						break;
					case 0x84:	//Hour

						break;
					case 0x86:	//Day

						break;
					case 0x88:	//Month

						break;
					case 0x8a:	//weekday

						break;
					case 0x8c:	//Year

						break;
					default:
						ShiftOut=0x0;
				}
				if(LastCmd>0xc0)
				{
					SRAM[(LastCmd>>1)&0x1f]=val;
				}



			}
		}
	}
	LastClk=val;
}

UINT8 DS1302_RD(void)
{
	return (ShiftOut&(1<<(ICount-9)))?1:0;
}
