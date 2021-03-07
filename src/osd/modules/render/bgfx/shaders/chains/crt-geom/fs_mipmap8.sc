$input v_texCoord

#include "common.sh"

SAMPLER2D(s_screen, 0);
SAMPLER2D(s_mipmap, 1);

uniform vec4 u_tex_size0;
uniform vec4 u_tex_size1;
uniform vec4 u_smooth;

/*
 * Based loosely on the idea outlined at
 *   https://nyorain.github.io/mipmap-compute-shader.html
 * Each higher mipmap level is delayed by one frame.
 *
 * A rectangle containing (1,1) has the last mipmap level
 * (normally a single pixel containing the average of the frame),
 * passed through a two-tap IIR temporal filter.
 */

const vec2 v05 = vec2(0.5,0.5);

// pix has integer coordinates -> average four pixels using bilinear filtering
vec3 sample_screen(vec2 pix)
{
  vec2 border = step(-0.5, u_tex_size0.xy - pix);
  float weight = border.x * border.y;

  // avoid boundary! if dimensions are odd, average 1 or 2 pixels
  vec2 x = clamp(pix, vec2_splat(0.0), u_tex_size0.xy - v05);
  // if we're at the boundary, include weight factor
  // -> effectively always average 4 pixels, with some of them possibly zero
  border = clamp(u_tex_size0.xy-x,0.0,1.0);
  weight *= border.x * border.y;

  vec3 tex = texture2D(s_screen, x / u_tex_size0.xy).rgb;

  return tex * vec3_splat(weight);
}

vec3 sample_mipmap(vec2 pix, vec2 offset, vec2 size)
{
  vec2 border = step(-0.5, size - pix) * step(-0.5, pix);
  float weight = border.x * border.y;

  // avoid boundary! if dimensions are odd, average 1 or 2 pixels
  vec2 x = clamp(pix, vec2_splat(0.0), size - v05);
  // if we're at the boundary, include weight factor
  // -> effectively always average 4 pixels, with some of them possibly zero
  border = clamp(size-x,0.0,1.0);
  weight *= border.x * border.y;

  vec3 tex = texture2D(s_mipmap, (x + offset) / u_tex_size0.xy).rgb;

  return tex * vec3_splat(weight);
}

void main()
{
  vec2 pix = v_texCoord.xy * u_tex_size0.xy;
  vec2 x = pix*vec2_splat(8.0);
  if (pix.x < u_tex_size0.x * 0.5) {
    // first level: sample from screen texture

    // sample 4x4 grid
    // each sample uses bilinear filtering to average a 2x2 region
    // overall reduction by 8x8
    vec3 tex = ( sample_screen(x + vec2(-3.0,-3.0))
               + sample_screen(x + vec2(-3.0,-1.0))
               + sample_screen(x + vec2(-3.0, 1.0))
               + sample_screen(x + vec2(-3.0, 3.0))
               + sample_screen(x + vec2(-1.0,-3.0))
               + sample_screen(x + vec2(-1.0,-1.0))
               + sample_screen(x + vec2(-1.0, 1.0))
               + sample_screen(x + vec2(-1.0, 3.0))
               + sample_screen(x + vec2( 1.0,-3.0))
               + sample_screen(x + vec2( 1.0,-1.0))
               + sample_screen(x + vec2( 1.0, 1.0))
               + sample_screen(x + vec2( 1.0, 3.0))
               + sample_screen(x + vec2( 3.0,-3.0))
               + sample_screen(x + vec2( 3.0,-1.0))
               + sample_screen(x + vec2( 3.0, 1.0))
               + sample_screen(x + vec2( 3.0, 3.0)) ) * 0.0625;

    gl_FragColor = vec4(tex.rgb,1.0);
  } else {
    // higher levels: sample from mipmap texture
    vec2 newsize = u_tex_size0.xy; // size of texture being sampled
    float fac = 8.0; // keep track of the linear reduction factor
    float dx = u_tex_size0.x;
    vec2 offset = vec2_splat(0.0);
    vec2 offset0 = offset;
    bool fc = false;

    for (int i = 0; i < 3; i++) { // 3 is sufficient for dimensions up to 4k
      // place subsequent levels to the right
      if (pix.x > dx * 0.5) {
        dx = dx * 0.5;
        newsize = ceil(newsize*v05*v05*v05); // size of sampled texture is eighthed
        fac *= 8.0;
        offset0 = offset;
        offset.x = offset.x + dx;
        pix.x -= dx;
        x = pix * vec2_splat(8.0);
        if (newsize.x < 8.5 && newsize.y < 8.5) { // reducing to a single pixel
          fc = true;
          break;
        }
      } else break;
    }

    if (fc)
      x = vec2(4.0,4.0);

    vec3 tex = ( sample_mipmap(x + vec2(-3.0,-3.0),offset0,newsize)
               + sample_mipmap(x + vec2(-3.0,-1.0),offset0,newsize)
               + sample_mipmap(x + vec2(-3.0, 1.0),offset0,newsize)
               + sample_mipmap(x + vec2(-3.0, 3.0),offset0,newsize)
               + sample_mipmap(x + vec2(-1.0,-3.0),offset0,newsize)
               + sample_mipmap(x + vec2(-1.0,-1.0),offset0,newsize)
               + sample_mipmap(x + vec2(-1.0, 1.0),offset0,newsize)
               + sample_mipmap(x + vec2(-1.0, 3.0),offset0,newsize)
               + sample_mipmap(x + vec2( 1.0,-3.0),offset0,newsize)
               + sample_mipmap(x + vec2( 1.0,-1.0),offset0,newsize)
               + sample_mipmap(x + vec2( 1.0, 1.0),offset0,newsize)
               + sample_mipmap(x + vec2( 1.0, 3.0),offset0,newsize)
               + sample_mipmap(x + vec2( 3.0,-3.0),offset0,newsize)
               + sample_mipmap(x + vec2( 3.0,-1.0),offset0,newsize)
               + sample_mipmap(x + vec2( 3.0, 1.0),offset0,newsize)
               + sample_mipmap(x + vec2( 3.0, 3.0),offset0,newsize) ) * 0.0625;

    if (fc) { // this sample yields average over the whole screen
      // apply a compensation for the zeros included in the 8^n average
      float f = fac*fac / (u_tex_size0.x * u_tex_size0.y);
      vec3 col = tex * vec3_splat(f);
      if (v_texCoord.y * u_tex_size1.y < ( offset0.y + u_tex_size1.y ) * 0.5) {
        gl_FragColor = vec4(col,1.0);
      } else {
        // we are near (1,1): in this region include the temporal filter
        vec4 old = texture2D(s_mipmap, v_texCoord.xy);
        gl_FragColor = vec4(mix(col,old.rgb,u_smooth.x),1.0);
      }
    } else {
      gl_FragColor = vec4(tex.rgb,1.0);
    }
  }
}
