$input a_position, i_data0, i_data1, i_data2
$output v_beam, v_beam_color, v_beam_timing

// license:BSD-3-Clause
// copyright-holders:Hans Andersson
// Expands each instanced vector segment into a padded screen-space quad and
// passes beam-local distance, color, width, and timing data to the beam shader.

#include "common.sh"

void main()
{
	// Per-instance data is packed by beam_instance in vectorrenderer.cpp:
	//   i_data0 = segment endpoints in target-pixel coordinates
	//   i_data1 = linear RGB and Gaussian sigma in pixels
	//   i_data2 = normalized scan start/duration, intensity, unused
	// Scan time is proportional to accumulated segment length, approximating a
	// beam that traverses the display list at constant deflection speed.
	vec2 p0 = i_data0.xy;
	vec2 p1 = i_data0.zw;
	vec2 delta = p1 - p0;
	float beamLength = length(delta);
	vec2 direction = beamLength > 0.0001 ? delta / beamLength : vec2(1.0, 0.0);
	vec2 normal = vec2(-direction.y, direction.x);
	float sigma = max(i_data1.w, 0.01);

	// a_position describes a unit segment quad: x selects an endpoint and y is
	// the signed side. Extend it by six sigma (at least one pixel) so the quad
	// contains the Gaussian tails and round endpoint caps evaluated per pixel.
	float padding = max(sigma * 6.0, 1.0);
	float along = mix(-padding, beamLength + padding, a_position.x);
	float across = a_position.y * padding;
	vec2 world = p0 + direction * along + normal * across;

	gl_Position = mul(u_viewProj, vec4(world, 0.0, 1.0));
	// Beam-local coordinates let the fragment shader evaluate distance without
	// depending on target orientation. Timing is constant for the whole instance.
	v_beam = vec4(along, across, beamLength, sigma);
	v_beam_color = vec4(i_data1.rgb, i_data2.z);
	v_beam_timing = i_data2;
}
