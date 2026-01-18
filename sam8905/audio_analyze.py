#!/usr/bin/env python3
"""
Audio analysis tool for SAM8905 output verification.

Usage:
    python audio_analyze.py <wav_file> [--expected-freq <hz>]

Example:
    python audio_analyze.py /tmp/test_sam_output.wav --expected-freq 440
"""

import argparse
import wave
import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
from scipy.fft import fft, fftfreq


def load_wav(filename: str) -> tuple:
    """Load WAV file and return (samples, sample_rate)."""
    with wave.open(filename, 'rb') as wav:
        sample_rate = wav.getframerate()
        n_channels = wav.getnchannels()
        n_frames = wav.getnframes()
        frames = wav.readframes(n_frames)

        # Determine sample width
        sample_width = wav.getsampwidth()
        if sample_width == 2:
            dtype = np.int16
        elif sample_width == 4:
            dtype = np.int32
        else:
            dtype = np.int8

        samples = np.frombuffer(frames, dtype=dtype)

        # Reshape to channels
        if n_channels > 1:
            samples = samples.reshape(-1, n_channels)

    return samples, sample_rate


def analyze_frequency(samples: np.ndarray, sample_rate: int, channel: int = 0) -> dict:
    """Analyze frequency content using FFT."""
    # Get single channel
    if samples.ndim > 1:
        data = samples[:, channel].astype(float)
    else:
        data = samples.astype(float)

    # Skip initial silence/transient
    nonzero_start = np.argmax(np.abs(data) > np.max(np.abs(data)) * 0.1)
    data = data[nonzero_start:]

    if len(data) < 1024:
        return {'error': 'Not enough samples'}

    # Use a window to reduce spectral leakage
    n = min(len(data), 8192)
    data = data[:n]
    window = signal.windows.hann(n)
    windowed = data * window

    # FFT
    yf = fft(windowed)
    xf = fftfreq(n, 1/sample_rate)

    # Only positive frequencies
    pos_mask = xf >= 0
    xf = xf[pos_mask]
    yf = np.abs(yf[pos_mask])

    # Find peak frequency
    peak_idx = np.argmax(yf[1:]) + 1  # Skip DC
    peak_freq = xf[peak_idx]
    peak_magnitude = yf[peak_idx]

    # Calculate THD (Total Harmonic Distortion)
    fundamental_power = peak_magnitude ** 2
    harmonic_power = 0
    for h in range(2, 6):  # 2nd through 5th harmonic
        harmonic_freq = peak_freq * h
        if harmonic_freq < sample_rate / 2:
            h_idx = np.argmin(np.abs(xf - harmonic_freq))
            harmonic_power += yf[h_idx] ** 2

    thd = np.sqrt(harmonic_power / fundamental_power) * 100 if fundamental_power > 0 else 0

    return {
        'peak_freq': peak_freq,
        'peak_magnitude': peak_magnitude,
        'thd_percent': thd,
        'frequencies': xf,
        'magnitudes': yf,
        'n_samples': n
    }


def analyze_waveform(samples: np.ndarray, sample_rate: int, expected_freq: float = None) -> dict:
    """Analyze waveform characteristics."""
    if samples.ndim > 1:
        data = samples[:, 0].astype(float)
    else:
        data = samples.astype(float)

    # Skip silence
    nonzero_start = np.argmax(np.abs(data) > 10)
    data = data[nonzero_start:]

    if len(data) < 100:
        return {'error': 'Not enough non-zero samples'}

    # Basic stats
    stats = {
        'min': int(np.min(data)),
        'max': int(np.max(data)),
        'mean': float(np.mean(data)),
        'rms': float(np.sqrt(np.mean(data ** 2))),
        'peak_to_peak': int(np.max(data) - np.min(data)),
    }

    # Zero crossing rate (estimate frequency)
    zero_crossings = np.where(np.diff(np.signbit(data)))[0]
    if len(zero_crossings) > 2:
        avg_period_samples = np.mean(np.diff(zero_crossings)) * 2
        estimated_freq = sample_rate / avg_period_samples
        stats['zero_crossing_freq'] = float(estimated_freq)

    # Check if it looks like a sine wave
    # A pure sine should have consistent peak-to-peak in each cycle
    if expected_freq:
        period_samples = int(sample_rate / expected_freq)
        if len(data) > period_samples * 3:
            # Measure amplitude of several cycles
            amplitudes = []
            for i in range(0, len(data) - period_samples, period_samples):
                cycle = data[i:i+period_samples]
                amplitudes.append(np.max(cycle) - np.min(cycle))
            if amplitudes:
                stats['amplitude_std'] = float(np.std(amplitudes))
                stats['amplitude_mean'] = float(np.mean(amplitudes))
                stats['amplitude_variation'] = float(np.std(amplitudes) / np.mean(amplitudes) * 100) if np.mean(amplitudes) > 0 else 0

    return stats


def plot_analysis(samples: np.ndarray, sample_rate: int, freq_analysis: dict,
                  expected_freq: float = None, output_file: str = None):
    """Create analysis plots."""
    fig, axes = plt.subplots(3, 1, figsize=(14, 10))

    # Get mono data
    if samples.ndim > 1:
        data = samples[:, 0]
    else:
        data = samples

    # Skip initial silence
    nonzero_start = np.argmax(np.abs(data) > 10)
    data = data[nonzero_start:]

    # Plot 1: Time domain (first ~10 cycles or 1000 samples)
    if expected_freq and expected_freq > 0:
        n_show = min(len(data), int(sample_rate / expected_freq * 10))
    else:
        n_show = min(len(data), 2000)

    t = np.arange(n_show) / sample_rate * 1000  # ms
    axes[0].plot(t, data[:n_show], 'b-', linewidth=0.5)
    axes[0].set_xlabel('Time (ms)')
    axes[0].set_ylabel('Amplitude')
    axes[0].set_title('Waveform (Time Domain)')
    axes[0].grid(True, alpha=0.3)

    # Add ideal sine overlay if frequency known
    if expected_freq and 'peak_freq' in freq_analysis:
        detected_freq = freq_analysis['peak_freq']
        # Estimate phase and amplitude
        amp = (np.max(data[:n_show]) - np.min(data[:n_show])) / 2
        offset = (np.max(data[:n_show]) + np.min(data[:n_show])) / 2
        ideal_sine = amp * np.sin(2 * np.pi * detected_freq * np.arange(n_show) / sample_rate) + offset
        axes[0].plot(t, ideal_sine, 'r--', linewidth=0.5, alpha=0.7, label=f'Ideal {detected_freq:.1f}Hz sine')
        axes[0].legend()

    # Plot 2: FFT (frequency domain)
    if 'frequencies' in freq_analysis:
        freqs = freq_analysis['frequencies']
        mags = freq_analysis['magnitudes']

        # Show up to 2kHz or Nyquist
        max_freq = min(2000, sample_rate / 2)
        mask = freqs <= max_freq

        axes[1].plot(freqs[mask], 20 * np.log10(mags[mask] + 1e-10), 'b-')
        axes[1].set_xlabel('Frequency (Hz)')
        axes[1].set_ylabel('Magnitude (dB)')
        axes[1].set_title(f'Frequency Spectrum (Peak: {freq_analysis["peak_freq"]:.1f} Hz)')
        axes[1].grid(True, alpha=0.3)

        # Mark expected and detected frequencies
        if expected_freq:
            axes[1].axvline(expected_freq, color='g', linestyle='--', alpha=0.7, label=f'Expected: {expected_freq}Hz')
        axes[1].axvline(freq_analysis['peak_freq'], color='r', linestyle=':', alpha=0.7,
                       label=f'Detected: {freq_analysis["peak_freq"]:.1f}Hz')
        axes[1].legend()

    # Plot 3: Spectrogram
    if len(data) > 256:
        f, t_spec, Sxx = signal.spectrogram(data.astype(float), sample_rate, nperseg=256)
        axes[2].pcolormesh(t_spec, f, 10 * np.log10(Sxx + 1e-10), shading='gouraud', cmap='viridis')
        axes[2].set_ylabel('Frequency (Hz)')
        axes[2].set_xlabel('Time (s)')
        axes[2].set_title('Spectrogram')
        axes[2].set_ylim(0, min(2000, sample_rate / 2))

    plt.tight_layout()

    if output_file:
        plt.savefig(output_file, dpi=150)
        print(f"Saved plot to {output_file}")
    else:
        plt.show()


def main():
    parser = argparse.ArgumentParser(description='Analyze audio output from SAM8905')
    parser.add_argument('wav_file', help='WAV file to analyze')
    parser.add_argument('--expected-freq', '-f', type=float, default=None,
                       help='Expected fundamental frequency (Hz)')
    parser.add_argument('--output', '-o', type=str, default=None,
                       help='Output plot file (PNG)')
    parser.add_argument('--no-plot', action='store_true',
                       help='Skip plotting')
    args = parser.parse_args()

    print(f"Loading {args.wav_file}...")
    samples, sample_rate = load_wav(args.wav_file)

    print(f"Sample rate: {sample_rate} Hz")
    print(f"Channels: {samples.shape[1] if samples.ndim > 1 else 1}")
    print(f"Duration: {len(samples) / sample_rate:.2f} s")
    print(f"Samples: {len(samples)}")
    print()

    # Waveform analysis
    print("=== Waveform Analysis ===")
    waveform = analyze_waveform(samples, sample_rate, args.expected_freq)
    if 'error' in waveform:
        print(f"Error: {waveform['error']}")
    else:
        print(f"Min: {waveform['min']}")
        print(f"Max: {waveform['max']}")
        print(f"Peak-to-Peak: {waveform['peak_to_peak']}")
        print(f"RMS: {waveform['rms']:.1f}")
        print(f"DC Offset: {waveform['mean']:.1f}")
        if 'zero_crossing_freq' in waveform:
            print(f"Zero-crossing frequency: {waveform['zero_crossing_freq']:.1f} Hz")
        if 'amplitude_variation' in waveform:
            print(f"Amplitude variation: {waveform['amplitude_variation']:.1f}%")
    print()

    # Frequency analysis
    print("=== Frequency Analysis ===")
    freq = analyze_frequency(samples, sample_rate)
    if 'error' in freq:
        print(f"Error: {freq['error']}")
    else:
        print(f"Peak frequency: {freq['peak_freq']:.2f} Hz")
        print(f"THD: {freq['thd_percent']:.2f}%")

        if args.expected_freq:
            error = abs(freq['peak_freq'] - args.expected_freq)
            error_percent = error / args.expected_freq * 100
            print(f"Expected: {args.expected_freq} Hz")
            print(f"Error: {error:.2f} Hz ({error_percent:.2f}%)")

            if error_percent < 1:
                print("✓ Frequency matches expected value")
            elif error_percent < 5:
                print("~ Frequency close to expected (within 5%)")
            else:
                print("✗ Frequency does NOT match expected value")
    print()

    # Verdict
    print("=== Verdict ===")
    is_sine = True
    reasons = []

    if 'peak_freq' in freq:
        if freq['thd_percent'] > 10:
            is_sine = False
            reasons.append(f"High THD ({freq['thd_percent']:.1f}%) - not a clean sine")

        if args.expected_freq:
            error_percent = abs(freq['peak_freq'] - args.expected_freq) / args.expected_freq * 100
            if error_percent > 5:
                is_sine = False
                reasons.append(f"Frequency mismatch ({error_percent:.1f}%)")

    if 'amplitude_variation' in waveform and waveform['amplitude_variation'] > 20:
        is_sine = False
        reasons.append(f"Amplitude unstable ({waveform['amplitude_variation']:.1f}% variation)")

    if waveform.get('peak_to_peak', 0) < 100:
        is_sine = False
        reasons.append(f"Very low amplitude (peak-to-peak: {waveform.get('peak_to_peak', 0)})")

    if is_sine:
        print("✓ Output appears to be a valid sine wave")
    else:
        print("✗ Output does NOT appear to be a clean sine wave:")
        for r in reasons:
            print(f"  - {r}")

    # Plot
    if not args.no_plot:
        output_file = args.output or args.wav_file.replace('.wav', '_analysis.png')
        plot_analysis(samples, sample_rate, freq, args.expected_freq, output_file)


if __name__ == '__main__':
    main()
