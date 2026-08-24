$input v_beam, v_beam_color, v_beam_timing

// license:BSD-3-Clause
// copyright-holders:Hans Andersson
// Rasterizes each vector as an HDR Gaussian beam, including intensity-dependent
// core/halo width and scan-order timing, into the phosphor accumulation buffer.

#include "common.sh"

// x = frame interval (seconds), y = phosphor persistence (seconds),
// z = beam-energy gain, w = halo strength.
uniform vec4 u_vector_params;
uniform vec4 u_target_dims;

// Integrate the core and halo over the approximate pixel footprint. Adding
// the box-filter variance to the Gaussian variance prevents subpixel beams
// from aliasing; scaling by sigma/filteredSigma preserves integrated energy.
float beam_response_filtered(float along, float across, float beamLength, float coreSigma, float haloSigma, float haloStrength)
{
	// Outside either endpoint, include longitudinal distance to produce round
	// caps. Between the endpoints only perpendicular distance contributes.
	float pastEndpoint = max(max(-along, along - beamLength), 0.0);
	float pixelAcross = length(vec2(dFdx(across), dFdy(across)));
	float pixelVariance = pixelAcross * pixelAcross / 12.0;
	float filteredCoreSigma = sqrt(coreSigma * coreSigma + pixelVariance);
	float filteredHaloSigma = sqrt(haloSigma * haloSigma + pixelVariance);
	float coreScale = coreSigma / filteredCoreSigma;
	float haloScale = haloSigma / filteredHaloSigma;
	float distanceSquared = across * across + pastEndpoint * pastEndpoint;

	return
		coreScale * exp(-0.5 * distanceSquared / (filteredCoreSigma * filteredCoreSigma)) +
		haloStrength * haloScale * exp(-0.5 * distanceSquared / (filteredHaloSigma * filteredHaloSigma));
}

void main()
{
	float along = v_beam.x;
	float across = v_beam.y;
	float beamLength = v_beam.z;

	float intensity = max(v_beam_color.a, 0.0);
	float intensityResponse = sqrt(clamp(intensity, 0.0, 1.0));

	// Brighter vectors get a slightly wider core and a somewhat wider halo.
	// Keep the response conservative so bright text does not become swollen.
	float baseSigma = max(v_beam.w, 0.01);
	float coreSigma = baseSigma * mix(1.0, 1.12, intensityResponse);
	float haloSigma = baseSigma * 3.5 * mix(1.0, 1.25, intensityResponse);

	float haloStrength =
		u_vector_params.w * mix(0.6, 1.0, intensityResponse);

	// Widen each Gaussian to account for the pixel's width across the beam.
	// Reduce its peak by the same ratio so the total beam energy stays constant.
	float radial = beam_response_filtered(
		along, across, beamLength, coreSigma, haloSigma, haloStrength);

	// The quad includes padded caps, so clamp the beam-local coordinate to the
	// physical segment. Put a degenerate segment at its temporal midpoint.
	float segmentPosition =
		beamLength > 0.0001
			? clamp(along / beamLength, 0.0, 1.0)
			: 0.5;

	// v_beam_timing.xy is the segment's start and duration as fractions of the
	// complete display-list scan. Convert the fragment's arrival time into its
	// age, in seconds, at the end of the current frame interval.
	float arrival = clamp(
		v_beam_timing.x + v_beam_timing.y * segmentPosition,
		0.0,
		1.0);

	float age = max(u_vector_params.x * (1.0 - arrival), 0.0);

	// Use the physical phosphor persistence when it is long enough. The ten-frame
	// floor prevents short persistence settings from erasing vectors drawn early
	// in this frame; inter-frame decay still uses the unmodified persistence.
	float scanPersistence = max(u_vector_params.y, u_vector_params.x * 10.0);
	float temporal = exp(-age / max(scanPersistence, 0.001));

	vec3 energy =
		v_beam_color.rgb *
		intensity *
		radial *
		temporal *
		u_vector_params.z;

	gl_FragColor = vec4(energy, 1.0);
}
