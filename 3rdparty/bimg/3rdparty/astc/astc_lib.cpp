/*----------------------------------------------------------------------------*/
/**
 *  @author Andrew Willmott
 *
 *  @brief  Library api for astc codec, to be used as an alternative to astc_toplevel.cpp
 */
/*----------------------------------------------------------------------------*/


#include "astc_lib.h"

#include "astc_codec_internals.h"

#include <math.h>
#include <stdio.h>

// Globals declared in astc_codec_internals.h
int perform_srgb_transform = 0;
int alpha_force_use_of_hdr = 0;
int rgb_force_use_of_hdr = 0;
int print_tile_errors = 0;

#ifdef DEBUG_PRINT_DIAGNOSTICS
    int print_diagnostics = 0;
    int diagnostics_tile = -1;
#endif

// ASTC code expects this to be defined
void astc_codec_internal_error(const char* filename, int line)
{
    fprintf(stderr, "ASTC encode error @ %s:%d\n", filename, line);
}

// @todo add HDR variants

namespace
{
    static bool s_tables_initialised = false;

    inline void init_tables()
    {
        if (!s_tables_initialised)
        {
            prepare_angular_tables();
            build_quantization_mode_table();

            s_tables_initialised = true;
        }
    }

    const swizzlepattern k_swizzles[] =
    {
        { 0, 1, 2, 3 }, // ASTC_RGBA
        { 2, 1, 0, 3 }, // ASTC_BGRA
        { 0, 0, 0, 1 }, // ASTC_ENC_NORMAL_RA
        { 0, 0, 0, 1 }, // ASTC_ENC_NORMAL_RA_PCPT
        { 0, 3, 6, 5 }, // ASTC_DEC_RA_NORMAL
    };

    void alloc_temp_buffers(compress_symbolic_block_buffers* temp_buffers)
    {
        temp_buffers->ewb                                                = new error_weight_block;
        temp_buffers->ewbo                                               = new error_weight_block_orig;
        temp_buffers->tempblocks                                         = new symbolic_compressed_block[4];
        temp_buffers->temp                                               = new imageblock;

        temp_buffers->planes2                                            = new compress_fixed_partition_buffers;
        temp_buffers->planes2->ei1                                       = new endpoints_and_weights;
        temp_buffers->planes2->ei2                                       = new endpoints_and_weights;
        temp_buffers->planes2->eix1                                      = new endpoints_and_weights[MAX_DECIMATION_MODES];
        temp_buffers->planes2->eix2                                      = new endpoints_and_weights[MAX_DECIMATION_MODES];
        temp_buffers->planes2->decimated_quantized_weights               = new float[2 * MAX_DECIMATION_MODES * MAX_WEIGHTS_PER_BLOCK];
        temp_buffers->planes2->decimated_weights                         = new float[2 * MAX_DECIMATION_MODES * MAX_WEIGHTS_PER_BLOCK];
        temp_buffers->planes2->flt_quantized_decimated_quantized_weights = new float[2 * MAX_WEIGHT_MODES * MAX_WEIGHTS_PER_BLOCK];
        temp_buffers->planes2->u8_quantized_decimated_quantized_weights  = new uint8_t[2 * MAX_WEIGHT_MODES * MAX_WEIGHTS_PER_BLOCK];
        temp_buffers->plane1                                             = temp_buffers->planes2;
    }

    void free_temp_buffers(compress_symbolic_block_buffers* temp_buffers)
    {
        delete[] temp_buffers->planes2->decimated_quantized_weights;
        delete[] temp_buffers->planes2->decimated_weights;
        delete[] temp_buffers->planes2->flt_quantized_decimated_quantized_weights;
        delete[] temp_buffers->planes2->u8_quantized_decimated_quantized_weights;
        delete[] temp_buffers->planes2->eix1;
        delete[] temp_buffers->planes2->eix2;
        delete   temp_buffers->planes2->ei1;
        delete   temp_buffers->planes2->ei2;
        delete   temp_buffers->planes2;

        delete[] temp_buffers->tempblocks;
        delete   temp_buffers->temp;
        delete   temp_buffers->ewbo;
        delete   temp_buffers->ewb;
    }


    // More direct version of the astc_codec_image routine, which operates on a
    // more conventional 2D image layout. Doesn't support padding, so
    // mean_stdev_radius and alpha_radius etc. must be zero.
    void to_imageblock
    (
        imageblock*    pb,
        const uint8_t* src_data,
        int            src_stride,
        int            xpos,
        int            ypos,
        int            xsize,
        int            ysize,
        int            xdim,
        int            ydim,
        swizzlepattern swz,
        bool           srgb
    )
    {
        float* fptr = pb->orig_data;

        pb->xpos = xpos;
        pb->ypos = ypos;
        pb->zpos = 0;

        float data[6];
        data[4] = 0;
        data[5] = 1;

        for (int y = 0; y < ydim; y++)
        {
            for (int x = 0; x < xdim; x++)
            {
                int xi = xpos + x;
                int yi = ypos + y;

                if (xi >= xsize)
                    xi = xsize - 1;
                if (yi >= ysize)
                    yi = ysize - 1;

                int offset = src_stride * yi + 4 * xi;

                int r = src_data[offset + 0];
                int g = src_data[offset + 1];
                int b = src_data[offset + 2];
                int a = src_data[offset + 3];

                data[0] = r / 255.0f;
                data[1] = g / 255.0f;
                data[2] = b / 255.0f;
                data[3] = a / 255.0f;

                fptr[0] = data[swz.r];
                fptr[1] = data[swz.g];
                fptr[2] = data[swz.b];
                fptr[3] = data[swz.a];

                fptr += 4;
            }
        }

        // perform sRGB-to-linear transform on input data, if requested.
        int pixelcount = xdim * ydim;

        if (srgb)
        {
            fptr = pb->orig_data;

            for (int i = 0; i < pixelcount; i++)
            {
                float r = fptr[0];
                float g = fptr[1];
                float b = fptr[2];

                if (r <= 0.04045f)
                    r = r * (1.0f / 12.92f);
                else if (r <= 1)
                    r = pow((r + 0.055f) * (1.0f / 1.055f), 2.4f);

                if (g <= 0.04045f)
                    g = g * (1.0f / 12.92f);
                else if (g <= 1)
                    g = pow((g + 0.055f) * (1.0f / 1.055f), 2.4f);

                if (b <= 0.04045f)
                    b = b * (1.0f / 12.92f);
                else if (b <= 1)
                    b = pow((b + 0.055f) * (1.0f / 1.055f), 2.4f);

                fptr[0] = r;
                fptr[1] = g;
                fptr[2] = b;

                fptr += 4;
            }
        }

        for (int i = 0; i < pixelcount; i++)
        {
            pb->rgb_lns  [i] = 0;
            pb->alpha_lns[i] = 0;
            pb->nan_texel[i] = 0;
        }

        imageblock_initialize_work_from_orig(pb, pixelcount);

        update_imageblock_flags(pb, xdim, ydim, 1);
    }

    void encode_astc
    (
        const uint8_t*                  src,
        int                             src_stride,
        swizzlepattern                  src_swz,
        int                             xsize,
        int                             ysize,
        int                             xdim,
        int                             ydim,
        const error_weighting_params*   ewp,
        astc_decode_mode                decode_mode,
        uint8_t*                        dst
    )
    {
        int xblocks = (xsize + xdim - 1) / xdim;
        int yblocks = (ysize + ydim - 1) / ydim;

        get_block_size_descriptor(xdim, ydim, 1);
        get_partition_table(xdim, ydim, 1, 0);

        imageblock pb;

        compress_symbolic_block_buffers temp_buffers;
        alloc_temp_buffers(&temp_buffers);

        astc_codec_image image_info = { nullptr, nullptr, xsize, ysize, 1, 0 };

        for (int y = 0; y < yblocks; y++)
            for (int x = 0; x < xblocks; x++)
            {
                to_imageblock(&pb, src, src_stride, x * xdim, y * ydim, xsize, ysize, xdim, ydim, src_swz, decode_mode == DECODE_LDR_SRGB);

                symbolic_compressed_block scb;
                compress_symbolic_block(&image_info, decode_mode, xdim, ydim, 1, ewp, &pb, &scb, &temp_buffers);

                physical_compressed_block pcb = symbolic_to_physical(xdim, ydim, 1, &scb);

                uint8_t* dst_block = dst + (y * xblocks + x) * 16;

                *(physical_compressed_block*) dst_block = pcb;
            }

        free_temp_buffers(&temp_buffers);
    }

    void init_ewp(error_weighting_params& ewp)
    {
        ewp.rgb_power                   = 1.0f;
        ewp.alpha_power                 = 1.0f;
        ewp.rgb_base_weight             = 1.0f;
        ewp.alpha_base_weight           = 1.0f;
        ewp.rgb_mean_weight             = 0.0f;
        ewp.rgb_stdev_weight            = 0.0f;
        ewp.alpha_mean_weight           = 0.0f;
        ewp.alpha_stdev_weight          = 0.0f;

        ewp.rgb_mean_and_stdev_mixing   = 0.0f;
        ewp.mean_stdev_radius           = 0;
        ewp.enable_rgb_scale_with_alpha = 0;
        ewp.alpha_radius                = 0;

        ewp.block_artifact_suppression  = 0.0f;
        ewp.rgba_weights[0]             = 1.0f;
        ewp.rgba_weights[1]             = 1.0f;
        ewp.rgba_weights[2]             = 1.0f;
        ewp.rgba_weights[3]             = 1.0f;
        ewp.ra_normal_angular_scale     = 0;
    }

    void setup_ewp(ASTC_COMPRESS_MODE mode, int ydim, int xdim, error_weighting_params& ewp)
    {
        float oplimit    = 0.0;
        float dblimit_2d = 0.0;
        float bmc        = 0.0;
        float mincorrel  = 0.0;

        int plimit       = -1;
        int maxiters     = 0;

        float log10_texels_2d = log((float)(xdim * ydim)) / log(10.0f);

        if (mode == ASTC_COMPRESS_VERY_FAST)
        {
            plimit = 2;
            oplimit = 1.0;
            dblimit_2d = MAX(70 - 35 * log10_texels_2d, 53 - 19 * log10_texels_2d);
            bmc = 25;
            mincorrel = 0.5;
            maxiters = 1;
        }
        else if (mode == ASTC_COMPRESS_FAST)
        {
            plimit = 4;
            oplimit = 1.0;
            mincorrel = 0.5;
            dblimit_2d = MAX(85 - 35 * log10_texels_2d, 63 - 19 * log10_texels_2d);
            bmc = 50;
            maxiters = 1;
        }
        else if (mode == ASTC_COMPRESS_MEDIUM)
        {
            plimit = 25;
            oplimit = 1.2f;
            mincorrel = 0.75f;
            dblimit_2d = MAX(95 - 35 * log10_texels_2d, 70 - 19 * log10_texels_2d);
            bmc = 75;
            maxiters = 2;
        }
        else if (mode == ASTC_COMPRESS_THOROUGH)
        {
            plimit = 100;
            oplimit = 2.5f;
            mincorrel = 0.95f;
            dblimit_2d = MAX(105 - 35 * log10_texels_2d, 77 - 19 * log10_texels_2d);
            bmc = 95;
            maxiters = 4;
        }
        else if (mode == ASTC_COMPRESS_EXHAUSTIVE)
        {
            plimit = PARTITION_COUNT;
            oplimit = 1000.0f;
            mincorrel = 0.99f;
            dblimit_2d = 999.0f;
            bmc = 100;
            maxiters = 4;
        }

        ewp.max_refinement_iters = maxiters;
        ewp.block_mode_cutoff = bmc / 100.0f;

        float texel_avg_error_limit_2d;

        if (rgb_force_use_of_hdr == 0)
        {
            texel_avg_error_limit_2d = pow(0.1f, dblimit_2d * 0.1f) * 65535.0f * 65535.0f;
        }
        else
        {
            texel_avg_error_limit_2d = 0.0f;
        }
        ewp.partition_1_to_2_limit = oplimit;
        ewp.lowest_correlation_cutoff = mincorrel;

        int partitions_to_test = plimit;
        if (partitions_to_test < 1)
            partitions_to_test = 1;
        else if (partitions_to_test > PARTITION_COUNT)
            partitions_to_test = PARTITION_COUNT;
        ewp.partition_search_limit = partitions_to_test;

        ewp.texel_avg_error_limit = texel_avg_error_limit_2d;

        expand_block_artifact_suppression(xdim, ydim, 1, &ewp);
    }

    void setup_ewp(ASTC_CHANNELS channels, error_weighting_params& ewp)
    {
        if (channels == ASTC_ENC_NORMAL_RA)
        {
            ewp.rgba_weights[0] = 1.0f;
            ewp.rgba_weights[1] = 0.01f;
            ewp.rgba_weights[2] = 0.01f;
            ewp.rgba_weights[3] = 1.0f;
            ewp.ra_normal_angular_scale = 1;
        }
    }
}

size_t astc_compressed_size(int w, int h, int bw, int bh)
{
    int nx = (w + bw - 1) / bw;
    int ny = (h + bh - 1) / bh;

    return nx * ny * 16;
}

void astc_compress
(
    int                src_width,
    int                src_height,
    const uint8_t*     src_data,
    ASTC_CHANNELS      src_channels,
    int                src_stride,

    int                block_width,
    int                block_height,
    ASTC_COMPRESS_MODE compress_mode,
    ASTC_DECODE_MODE   decode_mode,
    uint8_t*           dst_data
)
{
    init_tables();

    error_weighting_params ewp;
    init_ewp(ewp);
    setup_ewp(compress_mode, block_width, block_height, ewp);
    setup_ewp(src_channels, ewp);   // For special-case encoding of some channel types such as normals

    if (src_stride == 0)
        src_stride = src_width * 4;

    encode_astc
    (
        src_data,
        src_stride,
        k_swizzles[src_channels],
        src_width, src_height,
        block_width, block_height,
        &ewp,
        (astc_decode_mode) decode_mode,
        dst_data
    );
}

namespace
{
    // More direct version of the astc_codec_image routine, which operates on a
    // more conventional 2D image layout.
    void from_imageblock(int xdim, int ydim, const imageblock* pb, bool srgb, swizzlepattern swz, uint8_t* dst_data, int dst_stride)
    {
        const float*   fptr = pb->orig_data;
        const uint8_t* nptr = pb->nan_texel;

        for (int y = 0; y < ydim; y++)
        {
            for (int x = 0; x < xdim; x++)
            {
                if (*nptr)
                {
                    // NaN-pixel, but we can't display it. Display purple instead.
                    dst_data[4 * x + swz.r] = 0xFF;
                    dst_data[4 * x + swz.g] = 0x00;
                    dst_data[4 * x + swz.b] = 0xFF;
                    dst_data[4 * x + swz.a] = 0xFF;
                }
                else
                {
                    float r = fptr[0];
                    float g = fptr[1];
                    float b = fptr[2];
                    float a = fptr[3];

                    if (srgb)
                    {
                        if (r <= 0.0031308f)
                            r = r * 12.92f;
                        else if (r <= 1)
                            r = 1.055f * pow(r, (1.0f / 2.4f)) - 0.055f;

                        if (g <= 0.0031308f)
                            g = g * 12.92f;
                        else if (g <= 1)
                            g = 1.055f * pow(g, (1.0f / 2.4f)) - 0.055f;

                        if (b <= 0.0031308f)
                            b = b * 12.92f;
                        else if (b <= 1)
                            b = 1.055f * pow(b, (1.0f / 2.4f)) - 0.055f;
                    }

                    // clamp to [0,1]
                    if (r > 1.0f)
                        r = 1.0f;
                    if (g > 1.0f)
                        g = 1.0f;
                    if (b > 1.0f)
                        b = 1.0f;
                    if (a > 1.0f)
                        a = 1.0f;

                    // pack the data
                    dst_data[4 * x + swz.r] = uint8_t(floorf(r * 255.0f + 0.5f));
                    dst_data[4 * x + swz.g] = uint8_t(floorf(g * 255.0f + 0.5f));
                    dst_data[4 * x + swz.b] = uint8_t(floorf(b * 255.0f + 0.5f));
                    dst_data[4 * x + swz.a] = uint8_t(floorf(a * 255.0f + 0.5f));
                }

                fptr += 4;
                nptr++;
            }

            dst_data += dst_stride;
        }
    }
}

void astc_decompress
(
    const uint8_t*     src_data,
    int                xdim,
    int                ydim,
    ASTC_DECODE_MODE   decode_mode,

    int                xsize,
    int                ysize,
    uint8_t*           dst_data,
    ASTC_CHANNELS      dst_channels,
    int                dst_stride
)
{
    init_tables();

    int xblocks = (xsize + xdim - 1) / xdim;
    int yblocks = (ysize + ydim - 1) / ydim;

    if (dst_stride == 0)
        dst_stride = 4 * xsize;

    imageblock pb;

    for (int y = 0; y < yblocks; y++)
    {
        int ypos = y * ydim;
        int clamp_ydim = MIN(ysize - ypos, ydim);

        uint8_t* dst_row = dst_data + ypos * dst_stride;

        for (int x = 0; x < xblocks; x++)
        {
            int xpos = x * xdim;
            int clamp_xdim = MIN(xsize - xpos, xdim);

            physical_compressed_block pcb = *(const physical_compressed_block *) src_data;
            symbolic_compressed_block scb;

            physical_to_symbolic(xdim, ydim, 1, pcb, &scb);
            decompress_symbolic_block((astc_decode_mode) decode_mode, xdim, ydim, 1, xpos, ypos, 0, &scb, &pb);

            from_imageblock(clamp_xdim, clamp_ydim, &pb, decode_mode == ASTC_DECODE_LDR_SRGB, k_swizzles[dst_channels], dst_row + xpos * 4, dst_stride);

            src_data += 16;
        }
    }
}

// Relevant astc source files. These aren't set up for a bulk build yet though.
#ifdef DISABLED
    #include "astc_block_sizes2.cpp"
    #include "astc_color_quantize.cpp"
    #include "astc_color_unquantize.cpp"
    #include "astc_compress_symbolic.cpp"
    #include "astc_compute_variance.cpp"
    #include "astc_decompress_symbolic.cpp"
    #include "astc_encoding_choice_error.cpp"
    #include "astc_find_best_partitioning.cpp"
    #include "astc_ideal_endpoints_and_weights.cpp"
    #include "astc_imageblock.cpp"
    #include "astc_integer_sequence.cpp"
    #include "astc_kmeans_partitioning.cpp"
    #include "astc_partition_tables.cpp"
    #include "astc_percentile_tables.cpp"
    #include "astc_pick_best_endpoint_format.cpp"
    #include "astc_quantization.cpp"
    #include "astc_symbolic_physical.cpp"
    #include "astc_weight_align.cpp"
    #include "astc_weight_quant_xfer_tables.cpp"
    #include "mathlib.cpp"
    #include "softfloat.cpp"
#endif
