#ifndef cem3394_h
#define cem3394_h


#define CEM3394_SAMPLE_RATE		(44100*4)


/* interface */
struct cem3394_interface
{
	double vco_zero_freq;				/* frequency at 0V for VCO */
	double filter_zero_freq;			/* frequency at 0V for filter */
	void (*external)(int, int, short *);/* external input source */
};

/* inputs */
enum
{
	CEM3394_VCO_FREQUENCY = 0,
	CEM3394_MODULATION_AMOUNT,
	CEM3394_WAVE_SELECT,
	CEM3394_PULSE_WIDTH,
	CEM3394_MIXER_BALANCE,
	CEM3394_FILTER_RESONANCE,
	CEM3394_FILTER_FREQENCY,
	CEM3394_FINAL_GAIN
};

/* set the voltage going to a particular parameter */
void cem3394_set_voltage(int chip, int input, double voltage);

/* get the translated parameter associated with the given input as follows:
    CEM3394_VCO_FREQUENCY:      frequency in Hz
    CEM3394_MODULATION_AMOUNT:  scale factor, 0.0 to 2.0
    CEM3394_WAVE_SELECT:        voltage from this line
    CEM3394_PULSE_WIDTH:        width fraction, from 0.0 to 1.0
    CEM3394_MIXER_BALANCE:      balance, from -1.0 to 1.0
    CEM3394_FILTER_RESONANCE:   resonance, from 0.0 to 1.0
    CEM3394_FILTER_FREQENCY:    frequency, in Hz
    CEM3394_FINAL_GAIN:         gain, in dB */
double cem3394_get_parameter(int chip, int input);


#endif

