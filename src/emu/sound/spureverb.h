// license:BSD-3-Clause
// copyright-holders:pSXAuthor, R. Belmont
#ifndef _included_reverb_
	#define _included_reverb_

	struct reverb_params
	{
		float band_pole,
					band_gain,

					comb_delay[2][4],
					comb_gain,

					allpass_delay,
					allpass_gain;
	};

	class reverb
	{
		signed short *y[2][4],
									*x[2],
									*ax[2],
									*ay[2],
									bx1[2][2],by1[2];
		int yp,
				max_delay,
				sound_hz;
		typedef int comb_param[2][4];



		void comb_allpass(signed short *sp,
											signed short *dp,
											const reverb_params *rp,
											const int wetvol_l,
											const int wetvol_r,
											const unsigned int _sz);
		void comb_allpass4(signed short *sp,
												signed short *dp,
												const comb_param &comb_delay,
												const int comb_gain,
												const int allpass_delay,
												const int allpass_gain,
												const int *rvol,
												const unsigned int sz);
		void comb_allpass1(signed short *sp,
												signed short *dp,
												const comb_param &comb_delay,
												const int comb_gain,
												const int allpass_delay,
												const int allpass_gain,
												const int *rvol,
												const unsigned int sz);
		void bandpass(signed short *sp,
									const reverb_params *rp,
									const unsigned int sz);

	public:
		reverb(const int hz, const int maxdelay=65536);
		~reverb();

		void process(signed short *output,
									signed short *reverb_input,
									const reverb_params *rp,
									const int wetvol_l,
									const int wetvol_r,
									const unsigned int sz);

		void reset();
	};

#endif
