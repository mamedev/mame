// license:BSD-3-Clause
// copyright-holders:windyfairy
#include "emu.h"
#include "speaker.h"

#include "k573fpga.h"
#include "k573enc.h"

// TODO: Check if this is optimal
#define MINIMUM_BUFFER 1

// TODO: Is this really needed? Confirm
#define MAX_FRAME_SYNC_MATCHES 1

#define MINIMP3_ONLY_MP3
#define MINIMP3_NO_STDIO
#define MINIMP3_IMPLEMENTATION
#include "minimp3/minimp3.h"
#include "minimp3/minimp3_ex.h"

k573fpga_device::k573fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    device_t(mconfig, KONAMI_573_DIGITAL_FPGA, tag, owner, clock),
    mas3507d(*this, "mpeg"),
    m_samples(*this, "samples"),
    use_fake_fpga(false)
{
}

void k573fpga_device::device_start()
{
    ram_swap = std::make_unique<uint16_t[]>(32 * 1024 * 1024);
    save_pointer(NAME(ram_swap), 32 * 1024 * 1024 );

    device_reset();
}

void k573fpga_device::device_reset()
{
    mp3_start_adr = 0;
    mp3_end_adr = 0;
    crypto_key1 = 0;
    crypto_key2 = 0;
    crypto_key3 = 0;
    mp3_last_frame = 0;
    mp3_last_adr = 0;
    mp3_last_decrypt_adr = 0;
    mp3_next_sync = 0;
    last_copied_samples = 0;
    decrypt_finished = false;

    if (!channel_l_pcm) {
        free(channel_l_pcm);
    }

    channel_l_pcm = nullptr;
    last_buffer_size_channel_l = 0;

    if (!channel_r_pcm) {
        free(channel_r_pcm);
    }

    channel_r_pcm = nullptr;
    last_buffer_size_channel_r = 0;

    memset((void*)ram_swap.get(), 0, 12 * 1024 * 1024 * 2);
    memset(&mp3_info, 0, sizeof(mp3_info));

    mas3507d->set_samples(m_samples);

    last_position_update = 0;
    position_diff = 0;
}

void k573fpga_device::device_add_mconfig(machine_config &config)
{
    MAS3507D(config, "mpeg");

    /* sound hardware */
    SPEAKER(config, "lspeaker").front_left();
    SPEAKER(config, "rspeaker").front_right();
    SAMPLES(config, m_samples);
    m_samples->set_channels(2);
    m_samples->set_samples_update_callback(FUNC(k573fpga_device::k573fpga_stream_update));
    m_samples->add_route(0, "lspeaker", 1.0);
    m_samples->add_route(1, "rspeaker", 1.0);
}

uint32_t k573fpga_device::get_mp3_playback() {
    if (mp3_start_adr == 0) {
        if (m_samples->get_position(0) == 0 && decrypt_finished) {
            // The game is still requesting position updates after the song has ended.
            // This happens in some games like DDR 4th Mix Plus where it uses
            // the position in song for actual game engine timings.
            // The counter will properly end when the game signals to the FPGA to stop playback.
            last_position_update += position_diff;
        } else {
            position_diff = m_samples->get_position(0) - last_position_update;
            last_position_update = m_samples->get_position(0);
        }

        last_position_update = m_samples->get_position(0);

        return last_position_update;
    }

    return 0;
}

uint16_t k573fpga_device::i2c_read()
{
    int scl = mas3507d->i2c_scl_r() << 13;
    int sda = mas3507d->i2c_sda_r() << 12;

    return scl | sda;
}

void k573fpga_device::i2c_write(uint16_t data)
{
    mas3507d->i2c_scl_w(data & 0x2000);
    mas3507d->i2c_sda_w(data & 0x1000);
}

uint16_t k573fpga_device::get_mpeg_ctrl()
{
    if (mpeg_ctrl_flag == 0xe000 || (m_samples->get_position(0) > 0 && mp3_decrypt_mode)) {
        // This has been tested with real hardware, but this flag is always held 0x1000 when the audio is being played
        return mpeg_ctrl_flag | 0x1000;
    }

    return mpeg_ctrl_flag;
}

void k573fpga_device::set_mpeg_ctrl(uint16_t data)
{
    logerror("FPGA MPEG control %c%c%c | %08x %08x\n",
                data & 0x8000 ? '#' : '.',
                data & 0x4000 ? '#' : '.',
                data & 0x2000 ? '#' : '.',
                mp3_start_adr, mp3_end_adr);

    mpeg_ctrl_flag = data;

    if (data == 0xa000) {
        // Stop playback
        m_samples->stop(0);
        m_samples->stop(1);
    }

    if ((data & 0xa000) == 0xa000) {
        // Reset playback state for start and stop events
        mp3_last_frame = 0;
        mp3_next_sync = 0;
        mp3_last_adr = mp3_start_adr;
        mp3_last_decrypt_adr = mp3_start_adr;
        last_copied_samples = 0;
        last_buffer_size_channel_l = 0;
        last_buffer_size_channel_r = 0;

        last_position_update = 0;
        position_diff = 0;
        decrypt_finished = false;

        if (channel_l_pcm) {
            free(channel_l_pcm);
            channel_l_pcm = nullptr;
        }

        if (channel_r_pcm) {
            free(channel_r_pcm);
            channel_r_pcm = nullptr;
        }
    }

    if ((data & 0xe000) == 0xe000) {
        mp3_decrypt_mode = true;
    } else {
        mp3_decrypt_mode = false;
    }
}

int32_t k573fpga_device::find_enc_key()
{
    if (k573enc_lookup.find(crypto_key1) == k573enc_lookup.end()) {
        return -1;
    }

    return k573enc_lookup[crypto_key1];
}

inline uint16_t k573fpga_device::fpga_decrypt_byte_real(uint16_t v)
{
    uint16_t m = crypto_key1 ^ crypto_key2;

    v = bitswap<16>(
        v,
        15 - BIT(m, 0xF),
        14 + BIT(m, 0xF),
        13 - BIT(m, 0xE),
        12 + BIT(m, 0xE),
        11 - BIT(m, 0xB),
        10 + BIT(m, 0xB),
        9 - BIT(m, 0x9),
        8 + BIT(m, 0x9),
        7 - BIT(m, 0x8),
        6 + BIT(m, 0x8),
        5 - BIT(m, 0x5),
        4 + BIT(m, 0x5),
        3 - BIT(m, 0x3),
        2 + BIT(m, 0x3),
        1 - BIT(m, 0x2),
        0 + BIT(m, 0x2)
    );

    v ^= (BIT(m, 0xD) << 14) ^
        (BIT(m, 0xC) << 12) ^
        (BIT(m, 0xA) << 10) ^
        (BIT(m, 0x7) << 8) ^
        (BIT(m, 0x6) << 6) ^
        (BIT(m, 0x4) << 4) ^
        (BIT(m, 0x1) << 2) ^
        (BIT(m, 0x0) << 0);

    v ^= bitswap<16>(
        (uint16_t)crypto_key3,
        7, 0, 6, 1,
        5, 2, 4, 3,
        3, 4, 2, 5,
        1, 6, 0, 7
    );

    return (v >> 8) | ((v & 0xff) << 8);
}

inline uint16_t k573fpga_device::fpga_decrypt_byte_fake(uint16_t data, uint32_t crypto_idx)
{
    // Not the real algorithm. Only used for DDR Solo Bass Mix (ddrsbm)
    int32_t crypto_enc_idx = find_enc_key();

    if (crypto_enc_idx == -1) {
        return data;
    }

    uint8_t *key = k573enc_keys[crypto_enc_idx];

    uint16_t output_word = 0;
    for (int cur_bit = 0; cur_bit < 8; cur_bit++) {
        int even_bit_shift = (cur_bit * 2) & 0xff;
        int odd_bit_shift = (cur_bit * 2 + 1) & 0xff;
        int is_even_bit_set = (data & (1 << even_bit_shift)) != 0;
        int is_odd_bit_set = (data & (1 << odd_bit_shift)) != 0;
        int is_key_bit_set = (key[crypto_idx % 16] & (1 << cur_bit)) != 0;
        int is_scramble_bit_set = (key[(crypto_idx - 1) % 16] & (1 << cur_bit)) != 0;

        if (is_scramble_bit_set == 1) {
            int t = is_even_bit_set;
            is_even_bit_set = is_odd_bit_set;
            is_odd_bit_set = t;
        }

        if (((is_even_bit_set ^ is_key_bit_set)) == 1) {
            output_word |= 1 << even_bit_shift;
        }

        if ((is_odd_bit_set) == 1) {
            output_word |= 1 << odd_bit_shift;
        }
    }

    return (output_word >> 8) | ((output_word & 0xff) << 8);
}

SAMPLES_UPDATE_CB_MEMBER(k573fpga_device::k573fpga_stream_update)
{
    if ((mpeg_ctrl_flag & 0xe000) != 0xe000 || mp3_last_adr >= mp3_end_adr || (m_samples->get_position(0) < mp3_next_sync && mp3_next_sync != 0)) {
        return;
    }

    int new_samples = 0;

    if (!use_fake_fpga && mp3_start_adr != mp3_dynamic_base) {
        // Base addresses are kind of a "hack" and should be dealt with eventually.
        // Files that are loaded at he base address are dynamic, meaning the data constantly changes.
        // Data will not get loaded outside of the base address region past the boot sequence, however.
        // As such, it's possible to decrypt and play the full file if it's not at the designated base address.
        // Only 4 games seem to use a base address that is non-0: GF11DM10 (0x00540000) and GF10DM9 (0x00500000).
        // This way of doing things seems kind of arbitrary, so it most likely doesn't need to be handled in a
        // specific case like this, but this leads to much better performance overall since the data is usually
        // small (<1mb).
        // Another interesting thing about base addresses is that non-base address files are almost always looped
        // in-game, with the exception of the Konami logo sound. Without looping non-base address sounds, things
        // like system BGMs and song wheel previews will stop looping.

        crypto_key1 = orig_crypto_key1;
        crypto_key2 = orig_crypto_key2;
        crypto_key3 = orig_crypto_key3;

        for (int32_t cur_idx = mp3_start_adr; cur_idx < mp3_end_adr; cur_idx += 2) {
            if (use_fake_fpga) {
                ram_swap[cur_idx >> 1] = fpga_decrypt_byte_fake(ram[cur_idx >> 1], (cur_idx - mp3_start_adr) / 2);
            } else {
                ram_swap[cur_idx >> 1] = fpga_decrypt_byte_real(ram[cur_idx >> 1]);
            }

            if (!use_fake_fpga) {
                crypto_key1 = (crypto_key1 & 0x8000) | ((crypto_key1 << 1) & 0x7FFE) | ((crypto_key1 >> 14) & 1);

                if ((((crypto_key1 >> 15) ^ crypto_key1) & 1) != 0) {
                    crypto_key2 = (crypto_key2 << 1) | (crypto_key2 >> 15);
                }
            }

            crypto_key3++;
        }

        // Decrypt entire block of memory
        if (mp3_info.buffer != NULL) {
            free(mp3_info.buffer);
        }

        mp3dec_load_buf(&mp3_dec, ((uint8_t*)ram_swap.get()) + mp3_start_adr, mp3_end_adr - mp3_start_adr, &mp3_info, NULL, NULL);
        new_samples = mp3_info.samples;
        last_copied_samples = 0;
        mp3_last_adr += mp3_end_adr - mp3_start_adr;
    } else {
        // For base address files, decrypt in a streaming manner.
        // By the time the play flag is set, there's usually enough data to decode the first frame or two.
        // Some files (specifcially GFDM's audio) has a large 0x600 byte ID3 header, so in order to guarantee that
        // the first frame is found, I've found it best to decrypt 0x2000 bytes in the first go and then a smaller
        // amount after that for every subsequent update.

        new_samples = mp3_info.samples;


        int decrypt_buffer = MINIMUM_BUFFER;
        int decrypt_buffer_speed = buffer_speed;

        if (mp3_next_sync == 0) {
            crypto_key1 = orig_crypto_key1;
            crypto_key2 = orig_crypto_key2;
            crypto_key3 = orig_crypto_key3;

            mp3_last_decrypt_adr = mp3_start_adr;

            // Cover a large chunk of ID3 or junk at the beginning of a file when decrypting first frame
            decrypt_buffer = 1;
            decrypt_buffer_speed = 0x2000;

            last_position_update = 0;
            position_diff = 0;
            decrypt_finished = false;
        }

        // Decrypt chunk of data
        if (mp3_last_decrypt_adr < mp3_end_adr && mp3_last_adr + decrypt_buffer_speed * decrypt_buffer >= mp3_last_decrypt_adr) {
            int32_t cur_idx;

            for (cur_idx = mp3_last_decrypt_adr; cur_idx - mp3_last_decrypt_adr < decrypt_buffer_speed * decrypt_buffer && cur_idx < mp3_end_adr; cur_idx += 2) {
                if (use_fake_fpga) {
                    ram_swap[cur_idx >> 1] = fpga_decrypt_byte_fake(ram[cur_idx >> 1], (cur_idx - mp3_start_adr) / 2);
                } else {
                    ram_swap[cur_idx >> 1] = fpga_decrypt_byte_real(ram[cur_idx >> 1]);
                }

                if (!use_fake_fpga) {
                    crypto_key1 = (crypto_key1 & 0x8000) | ((crypto_key1 << 1) & 0x7FFE) | ((crypto_key1 >> 14) & 1);

                    if ((((crypto_key1 >> 15) ^ crypto_key1) & 1) != 0) {
                        crypto_key2 = (crypto_key2 << 1) | (crypto_key2 >> 15);
                    }
                }

                crypto_key3++;
            }

            mp3_last_decrypt_adr = cur_idx;
        }

        int samples;
        int free_format_bytes = 0, frame_size = 0;
        int buf_size = decrypt_buffer_speed * decrypt_buffer;

        uint8_t *buf = (uint8_t*)ram_swap.get() + mp3_last_adr;

        // Detect first frame in MP3 data
        if (mp3_next_sync == 0) {
            buf_size = 0x2000;

            // Everything on the System 573 has a header 0x600 or less from what I've seen, but just ot be sure, check the first 0x2000 bytes
            uint32_t first_frame = mp3d_find_frame(buf, buf_size, &free_format_bytes, &frame_size);
            buf += first_frame;
            mp3_last_adr += first_frame;
            buf_size -= first_frame;
        }

        // Skip x amount of samples previously read (this fixes some weird glitches)
        for (int frame_skip = 0; frame_skip < mp3_last_frame && buf < (uint8_t*)ram_swap.get() + mp3_end_adr; frame_skip++) {
            mp3d_find_frame(buf, buf_size, &free_format_bytes, &frame_size);
            buf += frame_size;
            mp3_last_adr += frame_size;
        }

        mp3_last_frame = 0;

        if (mp3_next_sync == 0) {
            // For the first iteration, we need to set up the MP3 state and such
            if (mp3_info.buffer != NULL) {
                free(mp3_info.buffer);
            }

            memset(&mp3_info, 0, sizeof(mp3_info));
            memset(&mp3_frame_info, 0, sizeof(mp3_frame_info));
            new_samples = 0;

            /* try to make allocation size assumption by first frame */
            mp3dec_init(&mp3_dec);

            samples = mp3dec_decode_frame(&mp3_dec, buf, buf_size, mp3_pcm, &mp3_frame_info);

            if (!samples) {
                return;
            }

            samples *= mp3_frame_info.channels;

            mp3_allocated = (buf_size / mp3_frame_info.frame_bytes) * samples * sizeof(mp3d_sample_t) + MINIMP3_MAX_SAMPLES_PER_FRAME * sizeof(mp3d_sample_t);
            mp3_info.buffer = (mp3d_sample_t*)malloc(mp3_allocated);

            if (!mp3_info.buffer) {
                return;
            }

            mp3_info.samples = samples;
            memcpy(mp3_info.buffer, mp3_pcm, mp3_info.samples * sizeof(mp3d_sample_t));

            /* save info */
            mp3_info.channels = mp3_frame_info.channels;
            mp3_info.hz       = mp3_frame_info.hz;
            mp3_info.layer    = mp3_frame_info.layer;
            mp3_last_frame = 0;

            decrypt_buffer = 8;
        }

        /* decode rest frames */
        buf_size = decrypt_buffer_speed * 2;

        for (int frame_skip = 0; frame_skip < decrypt_buffer && buf < (uint8_t*)ram_swap.get() + mp3_end_adr; frame_skip++) {
            if (buf + buf_size > (uint8_t*)ram_swap.get() + mp3_end_adr) {
                buf_size = ((uint8_t*)ram_swap.get() + mp3_end_adr) - buf;
            }
            if ((mp3_allocated - mp3_info.samples * sizeof(mp3d_sample_t)) < MINIMP3_MAX_SAMPLES_PER_FRAME * sizeof(mp3d_sample_t)) {
                mp3_allocated *= 2;
                mp3_info.buffer = (mp3d_sample_t*)realloc(mp3_info.buffer, mp3_allocated);
            }

            samples = mp3dec_decode_frame(&mp3_dec, buf, buf_size, mp3_info.buffer + mp3_info.samples, &mp3_frame_info);

            if (buf + buf_size >= (uint8_t*)ram_swap.get() + mp3_end_adr) {
                // Some songs have a weird frame at the very end of the song which can be skipped usually.
                mp3_last_adr += buf_size;
                break;
            }

            if (!samples) {
                mp3_last_decrypt_adr = mp3_start_adr; // Force it to redecrypt everything in case the previous decrypt was too fast to find a full frame
                crypto_key1 = orig_crypto_key1;
                crypto_key2 = orig_crypto_key2;
                crypto_key3 = orig_crypto_key3;
                break;
            }

            if (mp3_info.hz != mp3_frame_info.hz || mp3_info.layer != mp3_frame_info.layer) {
                break;
            }

            if (mp3_info.channels && mp3_info.channels != mp3_frame_info.channels) {
                break;
            }

            buf += mp3_frame_info.frame_bytes;
            mp3_info.samples += samples * mp3_frame_info.channels;
            mp3_last_frame++;
        }

        new_samples = mp3_info.samples - new_samples;
    }

    if (new_samples > 0) {
        size_t buffer_size = 0;

        if (mp3_info.channels == 1) {
            buffer_size = mp3_info.samples;
        } else {
            buffer_size = mp3_info.samples / 2;
        }

        if (channel_l_pcm == nullptr) {
            last_buffer_size_channel_l = buffer_size * 2;

            if (last_buffer_size_channel_l == 0) {
                last_buffer_size_channel_l = 0x1000;
            }

            channel_l_pcm = (mp3d_sample_t*)calloc(last_buffer_size_channel_l, sizeof(mp3d_sample_t));
        } else if (buffer_size > last_buffer_size_channel_l) {
            // realloc
            last_buffer_size_channel_l = buffer_size * 2;

            if (last_buffer_size_channel_l == 0) {
                last_buffer_size_channel_l = 0x1000;
            }

            channel_l_pcm = (mp3d_sample_t*)realloc(channel_l_pcm, last_buffer_size_channel_l * sizeof(mp3d_sample_t));
        }

        if (channel_r_pcm == nullptr) {
            last_buffer_size_channel_r = buffer_size * 2;

            if (last_buffer_size_channel_r == 0) {
                last_buffer_size_channel_r = 0x1000;
            }

            channel_r_pcm = (mp3d_sample_t*)calloc(last_buffer_size_channel_r, sizeof(mp3d_sample_t));
        } else if (buffer_size > last_buffer_size_channel_r) {
            // realloc
            last_buffer_size_channel_r = buffer_size * 2;

            if (last_buffer_size_channel_r == 0) {
                last_buffer_size_channel_r = 0x1000;
            }

            channel_r_pcm = (mp3d_sample_t*)realloc(channel_r_pcm, last_buffer_size_channel_r * sizeof(mp3d_sample_t));
        }

        if (mp3_info.channels == 1) {
            memcpy(((int8_t*)channel_l_pcm) + (last_copied_samples * sizeof(mp3d_sample_t)), ((int8_t*)mp3_info.buffer) + (last_copied_samples * sizeof(mp3d_sample_t)), (buffer_size - last_copied_samples) * sizeof(mp3d_sample_t));
            memcpy(((int8_t*)channel_r_pcm) + (last_copied_samples * sizeof(mp3d_sample_t)), ((int8_t*)mp3_info.buffer) + (last_copied_samples * sizeof(mp3d_sample_t)), (buffer_size - last_copied_samples) * sizeof(mp3d_sample_t));
        } else {
            for (size_t i = last_copied_samples; i < buffer_size; i++) {
                channel_l_pcm[i] = mp3_info.buffer[i * sizeof(mp3d_sample_t)];
                channel_r_pcm[i] = mp3_info.buffer[i * sizeof(mp3d_sample_t) + 1];
            }
        }

        last_copied_samples = buffer_size;

        m_samples->update_raw(0, channel_l_pcm, buffer_size, mp3_info.hz, mp3_start_adr != mp3_dynamic_base);
        m_samples->update_raw(1, channel_r_pcm, buffer_size, mp3_info.hz, mp3_start_adr != mp3_dynamic_base);

        mp3_next_sync = buffer_size - (buffer_size / 3); // Grab more data sometime before the current buffer ends. This is arbitrary and kinda hacky, but it worked best between various games in my testing.
    }

    if (mp3_last_adr >= mp3_end_adr) {
        decrypt_finished = true;
    }
}

DEFINE_DEVICE_TYPE(KONAMI_573_DIGITAL_FPGA, k573fpga_device, "k573fpga", "Konami 573 Digital I/O FPGA")
