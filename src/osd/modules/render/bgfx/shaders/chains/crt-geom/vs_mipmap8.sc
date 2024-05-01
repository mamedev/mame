$input a_position, a_texcoord0, a_color0
$output v_texCoord

#include "common.sh"

uniform vec4 u_inv_view_dims;

void main()
{
	// Do the standard vertex processing.
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
#if BGFX_SHADER_LANGUAGE_HLSL && BGFX_SHADER_LANGUAGE_HLSL <= 300
	gl_Position.xy += u_inv_view_dims.xy * gl_Position.w;
#endif
	v_texCoord = a_texcoord0;
}
