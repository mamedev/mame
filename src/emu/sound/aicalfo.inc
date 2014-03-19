/*
    AICA LFO handling

    Part of the AICA emulator package.
    (not compiled directly, #included from aica.c)

    By ElSemi, kingshriek, Deunan Knute, and R. Belmont
*/

#define LFO_SHIFT   8

struct LFO_t
{
	unsigned short phase;
	UINT32 phase_step;
	int *table;
	int *scale;
};

#define LFIX(v) ((unsigned int) ((float) (1<<LFO_SHIFT)*(v)))

//Convert DB to multiply amplitude
#define DB(v)   LFIX(pow(10.0,v/20.0))

//Convert cents to step increment
#define CENTS(v) LFIX(pow(2.0,v/1200.0))

static int PLFO_TRI[256],PLFO_SQR[256],PLFO_SAW[256],PLFO_NOI[256];
static int ALFO_TRI[256],ALFO_SQR[256],ALFO_SAW[256],ALFO_NOI[256];
static const float LFOFreq[32]={0.17f,0.19f,0.23f,0.27f,0.34f,0.39f,0.45f,0.55f,0.68f,0.78f,0.92f,1.10f,1.39f,1.60f,1.87f,2.27f,
				2.87f,3.31f,3.92f,4.79f,6.15f,7.18f,8.60f,10.8f,14.4f,17.2f,21.5f,28.7f,43.1f,57.4f,86.1f,172.3f};
static const float ASCALE[8]={0.0f,0.4f,0.8f,1.5f,3.0f,6.0f,12.0f,24.0f};
static const float PSCALE[8]={0.0f,7.0f,13.5f,27.0f,55.0f,112.0f,230.0f,494.0f};
static int PSCALES[8][256];
static int ASCALES[8][256];

static void AICALFO_Init(running_machine &machine)
{
	int i,s;
	for(i=0;i<256;++i)
	{
		int a,p;
//      float TL;
		//Saw
		a=255-i;
		if(i<128)
			p=i;
		else
			p=i-256;
		ALFO_SAW[i]=a;
		PLFO_SAW[i]=p;

		//Square
		if(i<128)
		{
			a=255;
			p=127;
		}
		else
		{
			a=0;
			p=-128;
		}
		ALFO_SQR[i]=a;
		PLFO_SQR[i]=p;

		//Tri
		if(i<128)
			a=255-(i*2);
		else
			a=(i*2)-256;
		if(i<64)
			p=i*2;
		else if(i<128)
			p=255-i*2;
		else if(i<192)
			p=256-i*2;
		else
			p=i*2-511;
		ALFO_TRI[i]=a;
		PLFO_TRI[i]=p;

		//noise
		//a=lfo_noise[i];
		a=machine.rand()&0xff;
		p=128-a;
		ALFO_NOI[i]=a;
		PLFO_NOI[i]=p;
	}

	for(s=0;s<8;++s)
	{
		float limit=PSCALE[s];
		for(i=-128;i<128;++i)
		{
			PSCALES[s][i+128]=CENTS(((limit*(float) i)/128.0));
		}
		limit=-ASCALE[s];
		for(i=0;i<256;++i)
		{
			ASCALES[s][i]=DB(((limit*(float) i)/256.0));
		}
	}
}

INLINE signed int AICAPLFO_Step(LFO_t *LFO)
{
	int p;

	LFO->phase+=LFO->phase_step;
#if LFO_SHIFT!=8
	LFO->phase&=(1<<(LFO_SHIFT+8))-1;
#endif
	p=LFO->table[LFO->phase>>LFO_SHIFT];
	p=LFO->scale[p+128];
	return p<<(SHIFT-LFO_SHIFT);
}

INLINE signed int AICAALFO_Step(LFO_t *LFO)
{
	int p;
	LFO->phase+=LFO->phase_step;
#if LFO_SHIFT!=8
	LFO->phase&=(1<<(LFO_SHIFT+8))-1;
#endif
	p=LFO->table[LFO->phase>>LFO_SHIFT];
	p=LFO->scale[p];
	return p<<(SHIFT-LFO_SHIFT);
}

static void AICALFO_ComputeStep(LFO_t *LFO,UINT32 LFOF,UINT32 LFOWS,UINT32 LFOS,int ALFO)
{
	float step=(float) LFOFreq[LFOF]*256.0/(float)44100.0;
	LFO->phase_step=(unsigned int) ((float) (1<<LFO_SHIFT)*step);
	if(ALFO)
	{
		switch(LFOWS)
		{
			case 0: LFO->table=ALFO_SAW; break;
			case 1: LFO->table=ALFO_SQR; break;
			case 2: LFO->table=ALFO_TRI; break;
			case 3: LFO->table=ALFO_NOI; break;
			default: printf("Unknown ALFO %d\n", LFOWS);
		}
		LFO->scale=ASCALES[LFOS];
	}
	else
	{
		switch(LFOWS)
		{
			case 0: LFO->table=PLFO_SAW; break;
			case 1: LFO->table=PLFO_SQR; break;
			case 2: LFO->table=PLFO_TRI; break;
			case 3: LFO->table=PLFO_NOI; break;
			default: printf("Unknown PLFO %d\n", LFOWS);
		}
		LFO->scale=PSCALES[LFOS];
	}
}
