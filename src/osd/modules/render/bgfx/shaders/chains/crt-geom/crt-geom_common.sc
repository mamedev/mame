// Comment the next line to disable interpolation in linear gamma (and gain speed).
//#define LINEAR_PROCESSING

// Enable 3x oversampling of the beam profile
#define OVERSAMPLE

// Use the older, purely gaussian beam profile
#define USEGAUSSIAN

// Macros.
#define FIX(c) max(abs(c), 1e-5)
#define PI 3.141592653589

vec4 TEX2D(vec2 c)
{
  vec2 underscan = step(0.0,c) * step(0.0,vec2_splat(1.0)-c);
  vec4 col = texture2D(mpass_texture, c) * vec4_splat(underscan.x*underscan.y);
#ifdef LINEAR_PROCESSING
  col = pow(col, vec4_splat(CRTgamma.x));
#endif
  return col;
}

// Enable screen curvature.
uniform vec4 curvature;

uniform vec4 spot_size;
uniform vec4 spot_growth;
uniform vec4 spot_growth_power;

uniform vec4 u_interp;

uniform vec4 aperture_strength;
uniform vec4 aperture_brightboost;

uniform vec4 CRTgamma;
uniform vec4 monitorsRGB;
uniform vec4 monitorgamma;

uniform vec4 overscan;
uniform vec4 aspect;

uniform vec4 d;
uniform vec4 R;

uniform vec4 cornersize;
uniform vec4 cornersmooth;

float intersect(vec2 xy , vec2 sinangle, vec2 cosangle)
{
  float A = dot(xy,xy)+d.x*d.x;
  float B = 2.0*(R.x*(dot(xy,sinangle)-d.x*cosangle.x*cosangle.y)-d.x*d.x);
  float C = d.x*d.x + 2.0*R.x*d.x*cosangle.x*cosangle.y;
  return (-B-sqrt(B*B-4.0*A*C))/(2.0*A);
}

vec2 bkwtrans(vec2 xy, vec2 sinangle, vec2 cosangle)
{
  float c = intersect(xy, sinangle, cosangle);
  vec2 pt = vec2_splat(c)*xy;
  pt -= vec2_splat(-R.x)*sinangle;
  pt /= vec2_splat(R.x);
  vec2 tang = sinangle/cosangle;
  vec2 poc = pt/cosangle;
  float A = dot(tang,tang)+1.0;
  float B = -2.0*dot(poc,tang);
  float C = dot(poc,poc)-1.0;
  float a = (-B+sqrt(B*B-4.0*A*C))/(2.0*A);
  vec2 uv = (pt-a*sinangle)/cosangle;
  float r = FIX(R.x*acos(a));
  return uv*r/sin(r/R.x);
}

vec2 transform(vec2 coord, vec3 stretch, vec2 sinangle, vec2 cosangle)
{
  coord = (coord-vec2_splat(0.5))*aspect.xy*stretch.z+stretch.xy;
  return (bkwtrans(coord, sinangle, cosangle)/overscan.xy/aspect.xy+vec2_splat(0.5));
}

float corner(vec2 coord)
{
  coord = (coord - vec2_splat(0.5)) * overscan.xy + vec2_splat(0.5);
  coord = min(coord, vec2_splat(1.0)-coord) * aspect.xy;
  vec2 cdist = vec2_splat(cornersize.x);
  coord = (cdist - min(coord,cdist));
  float dist = sqrt(dot(coord,coord));
  return clamp((max(cdist.x,1e-3)-dist)*cornersmooth.x,0.0, 1.0);
}

// Calculate the influence of a scanline on the current pixel.
//
// 'distance' is the distance in texture coordinates from the current
// pixel to the scanline in question.
// 'color' is the colour of the scanline at the horizontal location of
// the current pixel.
vec4 scanlineWeights(float distance, vec4 color)
{
  // "wid" controls the width of the scanline beam, for each RGB channel
  // The "weights" lines basically specify the formula that gives
  // you the profile of the beam, i.e. the intensity as
  // a function of distance from the vertical center of the
  // scanline. In this case, it is gaussian if width=2, and
  // becomes nongaussian for larger widths. Ideally this should
  // be normalized so that the integral across the beam is
  // independent of its width. That is, for a narrower beam
  // "weights" should have a higher peak at the center of the
  // scanline than for a wider beam.
#ifdef USEGAUSSIAN
  vec4 wid = spot_size.x + spot_growth.x * pow(color, vec4_splat(spot_growth_power.x));
  vec4 weights = vec4(distance / wid);
  float maxwid = spot_size.x + spot_growth.x;
  float norm = maxwid / ( 1.0 + exp(-1.0/(maxwid*maxwid)) );
  return norm * exp(-weights * weights) / wid;
#else
  vec4 wid = 2.0 + 2.0 * pow(color, vec4_splat(4.0));
  vec4 weights = vec4_splat(distance / 0.3);
  return 1.4 * exp(-pow(weights * inversesqrt(0.5 * wid), wid)) / (0.6 + 0.2 * wid);
#endif
}

vec4 cubic(vec4 x, float B, float C)
{
  // https://en.wikipedia.org/wiki/Mitchell%E2%80%93Netravali_filters
  vec2 a = x.yz; // components in [0,1]
  vec2 b = x.xw; // components in [1,2]
  vec2 a2 = a*a;
  vec2 b2 = b*b;
  a = (2.0-1.5*B-1.0*C)*a*a2 + (-3.0+2.0*B+C)*a2 + (1.0-(1.0/3.0)*B);
  b = ((-1.0/6.0)*B-C)*b*b2 + (B+5.0*C)*b2 + (-2.0*B-8.0*C)*b + ((4.0/3.0)*B+4.0*C);
  return vec4(b.x,a.x,a.y,b.y);
}

vec4 x_coeffs(vec4 x, float pos_x)
{
  if (u_interp.x < 0.5) { // box
    float wid = length(vec2(dFdx(pos_x),dFdy(pos_x)));
    float dx = clamp((0.5 + 0.5*wid - x.y)/wid, 0.0, 1.0);
    return vec4(0.0,dx,1.0-dx,0.0);
  } else if (u_interp.x < 1.5) { // linear
    return vec4(0.0, 1.0-x.y, 1.0-x.z, 0.0);
  } else if (u_interp.x < 2.5) { // Lanczos
    // Prevent division by zero.
    vec4 coeffs = FIX(PI * x);
    // Lanczos2 kernel.
    coeffs = 2.0 * sin(coeffs) * sin(coeffs / 2.0) / (coeffs * coeffs);
    // Normalize.
    coeffs /= dot(coeffs, vec4_splat(1.0));
    return coeffs;
  } else if (u_interp.x < 3.5) { // Catmull-Rom
    return cubic(x,0.0,0.5);
  } else if (u_interp.x < 4.5) { // Mitchell-Netravali
    return cubic(x,1.0/3.0,1.0/3.0);
  } else /*if (u_interp.x < 5.5)*/ { // B-spline
    return cubic(x,1.0,0.0);
  }
}

vec4 sample_scanline(vec2 xy, vec4 coeffs, float onex)
{
  // Calculate the effective colour of the given
  // scanline at the horizontal location of the current pixel,
  // using the Lanczos coefficients.
  vec4 col = clamp(TEX2D(xy + vec2(-onex, 0.0))*coeffs.x +
                   TEX2D(xy)*coeffs.y +
                   TEX2D(xy +vec2(onex, 0.0))*coeffs.z +
                   TEX2D(xy + vec2(2.0 * onex, 0.0))*coeffs.w , 0.0, 1.0);
  return col;
}

vec3 apply_shadow_mask(vec2 coord, vec3 col)
{
  vec2 xy = coord * u_quad_dims.xy / u_tex_size1.xy;
  vec4 mask = texture2D(mask_texture, xy);
  // count of total bright pixels is encoded in the mask's alpha channel
  float nbright = 255.0 - 255.0*mask.a;
  // fraction of bright pixels in the mask
  float fbright = nbright / ( u_tex_size1.x * u_tex_size1.y );
  // average darkening factor of the mask
  float aperture_average = mix(1.0-aperture_strength.x*(1.0-aperture_brightboost.x), 1.0, fbright);
  // colour of dark mask pixels
  vec3 clow = vec3_splat(1.0-aperture_strength.x) * col + vec3_splat(aperture_strength.x*(aperture_brightboost.x)) * col * col;
  float ifbright = 1.0 / fbright;
  // colour of bright mask pixels
  vec3 chi = vec3_splat(ifbright*aperture_average) * col - vec3_splat(ifbright - 1.0) * clow;
  return mix(clow,chi,mask.rgb); // mask texture selects dark vs bright
}

vec3 linear_to_sRGB(vec3 col)
{
  // only applies the gamma ramp; does not adjust the primaries
  vec3 linear_ramp = vec3(lessThan(col, vec3_splat(0.0031308)));
  vec3 clin = col * vec3_splat(12.92);
  vec3 cpow = pow(col, vec3_splat(1.0/2.4)) * vec3_splat(1.055) - vec3_splat(0.055);
  return mix(cpow, clin, linear_ramp);
}

vec3 linear_to_output(vec3 col)
{
  if (monitorsRGB.x > 0.5)
    return linear_to_sRGB(col);
  else
    return pow(col, vec3_splat(1.0 / monitorgamma.x));
}
