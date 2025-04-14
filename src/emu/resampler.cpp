// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Audio resampler

#include "emu.h"
#include "resampler.h"

// How an accurate resampler works ?

// Resampling uses a number of well-known theorems we are not trying
// to prove here.

// Samping theorem.  A digital signal sampled at frequency fs is
// equivalent to an analog signal where all frequencies are between 0
// and fs/2.  Equivalent here means that the samples are unique given
// the analog signal, the analog sugnal is unique given the samples,
// and going analog -> digital -> analog is perfect.

// That gives us point one: resampling from fs to ft is, semantically,
// reconstructing the analog signal from the fs sampling, removing all
// frequencies over ft/2, then sampling at ft.


// Up-sampling theorem.  Take a digital signal at frequency fs, and k
// an integer > 1.  Create a new digital signal at frequency fs*k by
// alternatively taking one sample from the original signal and adding
// k-1 zeroes.  If one recreates the corresponding analog signal and
// removes all frequencies over fs/2, then it will be identical to the
// original analog signal, up to a constant multiplier on the
// amplitude. For the curious the frequencies over fs/2 get copies of
// the original spectrum with inversions, e.g. the frequency fs/2-a is
// copied at fs/2+a, then it's not inverted at fs..fs*1.5, inverted
// again between fs*1.5 and fs*2, etc.

// A corollary is that if one starts for an analog signal with no
// frequencies over fs/2, samples it at fs, then up-samples to fs*k by
// adding zeroes, remove (filter) from the upsampled signal all
// frequencies over fs/2 then reconstruct the analog signal you get a
// result identical to the original signal.  It's a perfect
// upsampling, assuming the filtering is perfect.


// Down-sampling theorem.  Take a digital signal at frequency ft*k,
// with k and integer > 1.  Create a new digital signal at frequency
// ft by alternatively taking one sample from the original signal and
// dropping k-1 samples.  If the original signal had no frequency over
// ft/2, then the reconstructed analog signal is identical to the
// original one, up to a constant multiplier on the amplitude.  So it
// is a perfect downsampling assuming the original signal has nothing
// over ft/2.  For the curious if there are frequencies over ft/2,
// they end up added to the lower frequencies with inversions.  The
// frequency ft/2+a is added to ft/2-a, etc (signal to upsampling,
// only the other way around).

// The corollary there is that if one starts with a ft*k digital
// signal, filters out everything over ft/2, then keeps only one
// sample every k, then reconstruct the analog signal, you get the
// original analog signal with frequencies over ft/2 removed, which is
// reasonable given they are not representable at sampling frequency
// ft anyway.  As such it is called perfect because it's the best
// possible result in any case.

// Incidentally, the parasite audible frequencies added with the
// wrapping when the original is insufficiently filtered before
// dropping the samples are called aliasing, as in the high barely
// audible frequencies that was there but not noticed gets aliased to
// a very audible and annoying lower frequency.


// As a result, the recipe to go from frequency fs to ft for a digital
// signal is:

// - find a frequency fm = ks*fs = kt*ft with ks and kt integers.
//   When fs and ft are integers (our case), the easy solution is
//   fm = fs * ft / gcd(fs, ft)

// - up-sample the original signal x(t) into xm(t) with:
//      xm(ks*t) = x(t)
//      xm(other) = 0

// - filter the resulting fm Hz signal to remove all frequencies above
//   fs/2.  This is also called "lowpass at fs/2"

// - lowpass at ft/2

// - down-sample the fm signal into the resulting y(t) signal by:
//      y(t) = xm(kt*t)

// And, assuming the filtering is perfect (it isn't, of course), the
// result is a perfect resampling.

// Now to optimize all that.  The first point is that an ideal lowpass
// at fs/2 followed by an ideal lowpass at ft/2 is strictly equivalent
// to an ideal lowpass at min(fs/2, ft/2).  So only one filter is
// needed.

// The second point depends on the type of filter used.  In our case
// the filter type known as FIR has a big advantage.  A FIR filter
// computes the output signal as a finite ponderated sum on the values
// of the input signal only (also called a convolution).  E.g.
//    y(t) = sum(k=0, n-1) a[k] * x[t-k]
// where a[0..n-1] are constants called the coefficients of the filter.

// Why this type of filter is pertinent shows up when building the
// complete computation:

// y(t) = filter(xm)[kt*t]
//      = sum(k=0, n-1) a[k] * xm[kt*t - k]
//      = sum(k=0, n-1) a[k] * | x[(kt*t-k)/ks] when kt*t-k is divisible by ks
//                             | 0              otherwise
//      = sum(k=(kt*t) mod ks, n-1, step=ks) a[k] * x[(kt*t-k)/ks]

//        (noting p = (kt*t) mode ks, and a // b integer divide of a by b)
//      = sum(k=0, (n-1 - p))//ks) a[k*ks + p] x[(kt*t) // ks) - k]

// Splitting the filter coefficients in ks phases ap[0..ks-1] where
// ap[p][k] = a[p + ks*k], and noting t0 = (k*kt) // ks:

// y(t) = sum(k=0, len(ap[p])-1) ap[p][k] * x[t0-k]

// So we can take a big FIR filter and split it into ks interpolation
// filters and just apply the correct one at each sample.  We can make
// things even easier by ensuring that the size of every interpolation
// filter is the same.

// The art of creating the big FIR filter so that it doesn't change
// the signal too much is complicated enough that entire books have
// been written on the topic.  We use here a simple solution which is
// to use a so-called zero-phase filter, which is a symmetrical filter
// which looks into the future to filter out the frequencies without
// changing the phases, and shift it in the past by half its length,
// making it causal (e.g. not looking into the future anymore).  It is
// then called linear-phase, and has a latency of exactly half its
// length.  The filter itself is made very traditionally, by
// multiplying a sinc by a Hann window.

// The filter size is selected by maximizing the latency to 5ms and
// capping the length at 400, which experimentally seems to ensure a
// sharp rejection of more than 100dB in every case.

// Finally, remember that up and downsampling steps multiply the
// amplitude by a constant (upsampling divides by k, downsamply
// multiply by k in fact).  To compensate for that and numerical
// errors the easiest way to to normalize each phase-filter
// independently to ensure the sum of their coefficients is 1.  It is
// easy to see why it works: a constant input signal must be
// transformed into a constant output signal at the exact same level.
// Having the sum of coefficients being 1 ensures that.


audio_resampler::audio_resampler(u32 fs, u32 ft)
{
	m_ft = ft;
	m_fs = fs;

	// Compute the multiplier for fs and ft to reach the common frequency
	u32 gcd = compute_gcd(fs, ft);
	m_ftm = fs / gcd;
	m_fsm = ft / gcd;

	// Compute the per-phase filter length to limit the latency to 5ms and capping it
	m_order_per_lane = u32(fs * 0.005 * 2);
	if(m_order_per_lane > 400)
		m_order_per_lane = 400;

	// Reduce the number of phases to be less than 200
	m_phase_shift = 0;
	while(((m_fsm - 1) >> m_phase_shift) >= 200)
		m_phase_shift ++;

	m_phases = ((m_fsm - 1) >> m_phase_shift) + 1;

	// Compute the global filter length
	u32 filter_length = m_order_per_lane * m_phases;
	if((filter_length & 1) == 0)
		filter_length --;
	u32 hlen = filter_length / 2;

	// Prepare the per-phase filters
	m_coefficients.resize(m_phases);
	for(u32 i = 0; i != m_phases; i++)
		m_coefficients[i].resize(m_order_per_lane, 0.0);

	// Select the filter cutoff.  Keep it in audible range.
	double cutoff = std::min(fs/2.0, ft/2.0);
	if(cutoff > 20000)
		cutoff = 20000;

	// Compute the filter and send the coefficients to the appropriate phase
	auto set_filter = [this](u32 i, float v) { m_coefficients[i % m_phases][i / m_phases] = v; };

	double wc = 2 * M_PI * cutoff / (double(fs) * m_fsm / (1 << m_phase_shift));
	double a = wc / M_PI;
	for(u32 i = 1; i != hlen; i++) {
		double win = cos(i*M_PI/hlen/2);
		win = win*win;
		double s = a * sin(i*wc)/(i*wc) * win;

		set_filter(hlen-1+i, s);
		set_filter(hlen-1-i, s);
	}
	set_filter(hlen-1, a);

	// Normalize the per-phase filters
	for(u32 i = 0; i != m_phases; i++) {
		float s = 0;
		for(u32 j = 0; j != m_order_per_lane; j++)
			s += m_coefficients[i][j];
		s = 1/s;
		for(u32 j = 0; j != m_order_per_lane; j++)
			m_coefficients[i][j] *= s;
	}

	// Compute the phase shift from one sample to the next
	m_delta = m_ftm % m_fsm;
	m_skip  = m_ftm / m_fsm;
}

u32 audio_resampler::compute_gcd(u32 fs, u32 ft)
{
	u32 v1 = fs > ft ? fs : ft;
	u32 v2 = fs > ft ? ft : fs;
	while(v2) {
		u32 v3 = v1 % v2;
		v1 = v2;
		v2 = v3;
	}
	return v1;
}

void audio_resampler::apply(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<sample_t> &dest, u64 dest_sample, u32 srcc, float gain, u32 samples) const
{
	u32 seconds = dest_sample / m_ft;
	u32 dsamp = dest_sample % m_ft;
	u32 ssamp = (u64(dsamp) * m_fs) / m_ft;
	u64 ssample = ssamp + u64(m_fs) * seconds;
	u32 phase = (dsamp * m_ftm) % m_fsm;

	const sample_t *s = src.ptrs(srcc, ssample - src.sync_sample());
	sample_t *d = dest.data();
	for(u32 sample = 0; sample != samples; sample++) {
		sample_t acc = 0;
		const sample_t *s1 = s;
		const float *filter = m_coefficients[phase >> m_phase_shift].data();
		for(u32 k = 0; k != m_order_per_lane; k++)
			acc += *filter++ * *s1--;
		*d++ += acc * gain;
		phase += m_delta;
		s += m_skip;
		while(phase >= m_fsm) {
			phase -= m_fsm;
			s ++;
		}
	}
}

void audio_resampler::apply(const emu::detail::output_buffer_interleaved<s16> &src, std::vector<sample_t> &dest, u64 dest_sample, u32 srcc, float gain, u32 samples) const
{
	u32 seconds = dest_sample / m_ft;
	u32 dsamp = dest_sample % m_ft;
	u32 ssamp = (u64(dsamp) * m_fs) / m_ft;
	u64 ssample = ssamp + u64(m_fs) * seconds;
	u32 phase = (dsamp * m_ftm) % m_fsm;

	gain /= 32768;

	const s16 *s = src.ptrs(srcc, ssample - src.sync_sample());
	sample_t *d = dest.data();
	int step = src.channels();
	for(u32 sample = 0; sample != samples; sample++) {
		sample_t acc = 0;
		const s16 *s1 = s;
		const float *filter = m_coefficients[phase >> m_phase_shift].data();
		for(u32 k = 0; k != m_order_per_lane; k++) {
			acc += *filter++ * *s1;
			s1 -= step;
		}
		*d++ += acc * gain;
		phase += m_delta;
		s += m_skip * step;
		while(phase >= m_fsm) {
			phase -= m_fsm;
			s += step;
		}
	}
}


void audio_resampler::apply(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<s16> &dest, u32 destc, int dchannels, u64 dest_sample, u32 srcc, float gain, u32 samples) const
{
	u32 seconds = dest_sample / m_ft;
	u32 dsamp = dest_sample % m_ft;
	u32 ssamp = (u64(dsamp) * m_fs) / m_ft;
	u64 ssample = ssamp + u64(m_fs) * seconds;
	u32 phase = (dsamp * m_ftm) % m_fsm;

	gain *= 32768;

	const sample_t *s = src.ptrs(srcc, ssample - src.sync_sample());
	s16 *d = dest.data() + destc;
	for(u32 sample = 0; sample != samples; sample++) {
		sample_t acc = 0;
		const sample_t *s1 = s;
		const float *filter = m_coefficients[phase >> m_phase_shift].data();
		for(u32 k = 0; k != m_order_per_lane; k++)
			acc += *filter++ * *s1--;
		*d += acc * gain;
		d += dchannels;
		phase += m_delta;
		s += m_skip;
		while(phase >= m_fsm) {
			phase -= m_fsm;
			s ++;
		}
	}
}
