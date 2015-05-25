// license:BSD-3-Clause
// copyright-holders:pSXAuthor, R. Belmont
#include "emu.h"
#include "spureverb.h"

//
//
//

//#define use_intrinsics

#ifdef use_intrinsics
	#include <mmintrin.h>
#endif

//
//
//

static inline int clamp(int v)
{
	if (v<-32768) return -32768;
	if (v>32767) return 32767;
	return v;
}

//
//
//

reverb::reverb(const int hz, const int maxdelay)
	:  yp(0),
		max_delay(maxdelay),
		sound_hz(hz)
{
	for (int c=0; c<2; c++)
	{
		for (int f=0; f<4; f++) {
			y[c][f]=new signed short [maxdelay];
			memset(y[c][f], 0, sizeof(signed short) * maxdelay);
		}
		x[c]=new signed short [maxdelay];
		memset(x[c], 0, sizeof(signed short) * maxdelay);
		ax[c]=new signed short [maxdelay];
		memset(ax[c], 0, sizeof(signed short) * maxdelay);
		ay[c]=new signed short [maxdelay];
		memset(ay[c], 0, sizeof(signed short) * maxdelay);
	}
	memset(bx1,0,sizeof(bx1));
	memset(by1,0,sizeof(by1));
}

//
//
//

reverb::~reverb()
{
	for (int c=0; c<2; c++)
	{
		for (int f=0; f<4; f++)
			global_free_array(y[c][f]);
		global_free_array(x[c]);
		global_free_array(ax[c]);
		global_free_array(ay[c]);
	}
}

//
//
//

void reverb::bandpass(signed short *sp,
											const reverb_params *rp,
											const unsigned int sz)
{
	int band_pole=(int)(rp->band_pole*32767),
			band_gain=(int)(rp->band_gain*32767);

	// Bandpass

	int xp=yp;
	for (unsigned int i=0; i<(sz>>2); i++, sp+=2)
	{
		for (int c=0; c<2; c++)
		{
			int x1=(xp-1)&(max_delay-1),
					bv;

			bv=sp[c]+bx1[c][1]+((band_pole*x[c][x1])>>15);
			bv=(bv*band_gain)>>15;
			x[c][xp]=clamp(bv);
			bx1[c][1]=bx1[c][0];
			bx1[c][0]=sp[c];
		}

		xp=(xp+1)&(max_delay-1);
	}
}

void reverb::comb_allpass1(signed short *sp,
														signed short *dp,
														const comb_param &comb_delay,
														const int comb_gain,
														const int allpass_delay,
														const int allpass_gain,
														const int *rvol,
														const unsigned int sz)
{
	for (unsigned int i=0; i<(sz>>2); i++, sp+=2, dp+=2)
	{
		for (int c=0; c<2; c++)
		{
			// Comb

			int v=0;

			for (int f=0; f<4; f++)
			{
				int yck=(yp-comb_delay[c][f])&(max_delay-1);
				y[c][f][yp]=clamp(x[c][yck]+((comb_gain*y[c][f][yck])>>15));
				v+=y[c][f][yp];
			}

			v>>=2;

			// Allpass

			if (allpass_delay)
			{
				ax[c][yp]=v;

				int ypa=(yp-allpass_delay)&(max_delay-1);
				v=((allpass_gain*(ay[c][ypa]-x[c][yp]))>>15)+ax[c][ypa];
				v=clamp(v);
				ay[c][yp]=v;
			}

			// Output

			dp[c]=clamp(((v*rvol[c])>>15)+dp[c]+sp[c]);
		}
		yp=(yp+1)&(max_delay-1);
	}
}

//
//
//

void reverb::comb_allpass4(signed short *sp,
														signed short *dp,
														const comb_param &comb_delay,
														const int comb_gain,
														const int allpass_delay,
														const int allpass_gain,
														const int *rvol,
														const unsigned int sz)
{
#ifdef use_intrinsics
	__m64   cg=_mm_set1_pi16(comb_gain),
				ag=_mm_set1_pi16(allpass_gain),
				rv[2];
	rv[0]=_mm_set1_pi16(rvol[0]);
	rv[1]=_mm_set1_pi16(rvol[1]);

	for (unsigned int i=0; i<(sz>>4); i++, sp+=2<<2, dp+=2<<2)
	{
		__m64 dv[2];

		for (int c=0; c<2; c++)
		{
			// Comb

			__m64 v=_mm_setzero_si64();

			for (int f=0; f<4; f++)
			{
				int yck=(yp-comb_delay[c][f])&(max_delay-1);
				__m64 xv=*(__m64 *)(&x[c][yck]),
							yv=*(__m64 *)(&y[c][f][yck]);
				yv=_mm_mulhi_pi16(yv,cg);
				yv=_mm_adds_pi16(yv,yv);
				yv=_mm_adds_pi16(xv,yv);
				*((__m64 *)&y[c][f][yp])=yv;
				yv=_mm_srai_pi16(yv,2);
				v=_mm_adds_pi16(v,yv);
			}

			// Allpass

			if (allpass_delay)
			{
				*((__m64 *)&ax[c][yp])=v;

				int ypa=(yp-allpass_delay)&(max_delay-1);
				__m64 ayv=*(__m64 *)&ay[c][ypa],
								xv=*(__m64 *)&x[c][yp],
								axv=*(__m64 *)&ax[c][ypa];

				ayv=_mm_subs_pi16(ayv,xv);
				ayv=_mm_mulhi_pi16(ayv,ag);
				ayv=_mm_adds_pi16(ayv,ayv);
				v=_mm_adds_pi16(ayv,axv);
				*((__m64 *)&ay[c][yp])=v;
			}

			// Output

			dv[c]=_mm_mulhi_pi16(v,rv[c]);
			dv[c]=_mm_adds_pi16(dv[c],dv[c]);
		}

		__m64 dv1=_mm_unpacklo_pi16(dv[0],dv[1]),
					dv2=_mm_unpackhi_pi16(dv[0],dv[1]),
					d1=*(__m64 *)&dp[0],
					d2=*(__m64 *)&dp[4],
					s1=*(__m64 *)&sp[0],
					s2=*(__m64 *)&sp[4];
		d1=_mm_adds_pi16(d1,s1);
		d2=_mm_adds_pi16(d2,s2);
		d1=_mm_adds_pi16(d1,dv1);
		d2=_mm_adds_pi16(d2,dv2);
		*(__m64 *)&dp[0]=d1;
		*(__m64 *)&dp[4]=d2;

		yp=(yp+4)&(max_delay-1);
	}

	_mm_empty();
#endif
}

//
//
//

void reverb::comb_allpass(signed short *sp,
													signed short *dp,
													const reverb_params *rp,
													const int wetvol_l,
													const int wetvol_r,
													const unsigned int _sz)
{
	unsigned int sz=_sz;
	comb_param comb_delay;
	int comb_gain=(int)(rp->comb_gain*32767),
			allpass_delay=(int)(((rp->allpass_delay/1000.0f)*sound_hz))&~3,
			allpass_gain=(int)(rp->allpass_gain*32767),
			rvol[2]={ (signed short)wetvol_l,
								(signed short)wetvol_r };

	for (int i=0; i<4; i++)
		for (int c=0; c<2; c++)
			comb_delay[c][i]=(int)(((rp->comb_delay[c][i]/1000.0f)*sound_hz))&~3;

	#ifdef use_intrinsics

	if (yp&3)
	{
		unsigned int n=min(sz,(unsigned int)(4-(yp&3))<<2);
		comb_allpass1(sp,dp,
									(const comb_param &)comb_delay,
									comb_gain,
									allpass_delay,
									allpass_gain,
									rvol,
									n);
		sp+=(n>>1);
		dp+=(n>>1);
		sz-=n;
	}

	if (sz>=16)
	{
		unsigned int n=sz&~15;
		comb_allpass4(sp,dp,
									(const comb_param &)comb_delay,
									comb_gain,
									allpass_delay,
									allpass_gain,
									rvol,
									n);
		sp+=n>>1;
		dp+=n>>1;
		sz-=n;
	}

	if (sz)
	{
		comb_allpass1(sp,dp,
									(const comb_param &)comb_delay,
									comb_gain,
									allpass_delay,
									allpass_gain,
									rvol,
									sz);
	}

	#else
		comb_allpass1(sp,dp,
									(const comb_param &)comb_delay,
									comb_gain,
									allpass_delay,
									allpass_gain,
									rvol,
									sz);
	#endif
}

//
//
//

void reverb::process(signed short *output,
										signed short *reverb_input,
										const reverb_params *rp,
										const int wetvol_l,
										const int wetvol_r,
											const unsigned int sz)
{
	signed short *sp=(signed short *)reverb_input,
								*dp=(signed short *)output;

	if (rp->band_gain>0.0f)
	{
		// Do reverb processing

		bandpass(sp,rp,sz);
		comb_allpass(sp,dp,rp,wetvol_l,wetvol_r,sz);
	} else
	{
		// Reverb disabled - just mix the input to the output

		for (unsigned int i=0; i<(sz>>2); i++)
		{
			output[0]=clamp(output[0]+reverb_input[0]);
			output[1]=clamp(output[1]+reverb_input[1]);
			output+=2;
			reverb_input+=2;
		}
	}
}
