#include "stdafx.h"
#include "LZWCodec.h"
#include <stdlib.h>
#include <string.h>

/*
 * Robust TIFF/Photoshop LZW implementation (MSB-first).
 *
 * Decoder notes:
 * - Uses prefix/first-char tables (no per-entry malloc) to reconstruct sequences.
 * - Handles the KwKwK special case.
 * - Tries two variants when a decode fails:
 *     - mode A: increase code width BEFORE reading the next code when next_code >= (1<<code_size)
 *     - mode B: increase code width AFTER adding a new code when next_code > (1<<code_size)
 *   This covers common encoder differences (Photoshop vs some other encoders).
 *
 * Encoder notes:
 * - Simple dictionary; linear search (no hash) to avoid STL.
 * - Emits CLEAR when dictionary fills; uses MSB-first bit packing.
 *
 * Limitations:
 * - No predictor handling (apply/remove externally).
 * - Encoder is not optimized for speed.
 */

#define MAX_DICT 4096
#define CLEAR_CODE 256
#define EOI_CODE 257
#define MIN_CODE_SIZE 9
#define MAX_CODE_SIZE 12

/*******************************
 * MSB-first bit reader
 *
 * bit_pos: number of bits already consumed in current byte (0..7),
 *          counting from MSB side (0 means next bit to read is bit 7).
 *
 * Returns 1 on success, 0 on failure (not enough bits).
 *******************************/
static int read_bits_msb(const uint8_t *in, int in_size, int *byte_pos, int *bit_pos, int nbits, int *out_code)
{
    if (nbits <= 0) return 0;
    unsigned int result = 0;
    int bits_read = 0;

    while (bits_read < nbits) {
        if (*byte_pos >= in_size) return 0;
        int avail = 8 - *bit_pos;                // bits available in current byte from current position
        int take = nbits - bits_read;
        if (take > avail) take = avail;

        uint8_t cur = in[*byte_pos];
        int shift_right = avail - take;
        uint8_t mask = (uint8_t)(((1u << take) - 1u) << shift_right);
        uint8_t part = (uint8_t)((cur & mask) >> shift_right);

        // append chunk to result (MSB-first stream: left-shift existing result)
        result = (result << take) | (unsigned int)part;

        bits_read += take;
        *bit_pos += take;
        if (*bit_pos >= 8) {
            *bit_pos = 0;
            (*byte_pos)++;
        }
    }

    *out_code = (int)result;
    return 1;
}

static int read_bits_lsb(const uint8_t* in, int in_size, int* byte_pos, int* bit_pos, int nbits, int* out_code)
{
    if (nbits <= 0) return 0;
    unsigned int result = 0;
    int bits_read = 0;
    int shift = 0;
    while (bits_read < nbits) {
        if (*byte_pos >= in_size) return 0;
        int avail = 8 - *bit_pos;
        int take = nbits - bits_read;
        if (take > avail) take = avail;

        uint8_t cur = in[*byte_pos];
        // LSB-first: extract low bits starting at bit_pos
        uint8_t part = (uint8_t)((cur >> *bit_pos) & ((1u << take) - 1u));
        result |= ((unsigned int)part) << shift;

        bits_read += take;
        shift += take;
        *bit_pos += take;
        if (*bit_pos >= 8) { *bit_pos = 0; (*byte_pos)++; }
    }
    *out_code = (int)result;
    return 1;
}

/*******************************
 * MSB-first bit writer helpers
 *
 * curr_bitpos: number of bits already filled in current byte (0..8),
 *              counting from MSB side. When curr_bitpos==8 the byte is full.
 *
 * Writes high-order bits first.
 *******************************/
static int bw_write_bits_msb(uint8_t **outbuf, int *out_cap, int *out_pos, uint32_t val, int nbits, uint8_t *curr_byte, int *curr_bitpos)
{
    if (nbits <= 0) return 1;
    int bits_to_write = nbits;

    while (bits_to_write > 0) {
        int free_bits = 8 - *curr_bitpos;
        int take = bits_to_write < free_bits ? bits_to_write : free_bits;

        int shift = bits_to_write - take; // drop low bits
        uint32_t part = (val >> shift) & ((1u << take) - 1u);

        int shift_into_byte = free_bits - take;
        *curr_byte |= (uint8_t)(part << shift_into_byte);

        *curr_bitpos += take;
        bits_to_write -= take;

        if (*curr_bitpos == 8) {
            if (*out_pos + 1 > *out_cap) {
                int newcap = (*out_cap == 0) ? 1024 : (*out_cap * 2);
                uint8_t *tmp = (uint8_t*)realloc(*outbuf, newcap);
                if (tmp == NULL) return 0;
                *outbuf = tmp;
                *out_cap = newcap;
            }
            (*outbuf)[(*out_pos)++] = *curr_byte;
            *curr_byte = 0;
            *curr_bitpos = 0;
        }
    }

    return 1;
}

static int bw_flush_msb(uint8_t **outbuf, int *out_cap, int *out_pos, uint8_t *curr_byte, int *curr_bitpos)
{
    if (*curr_bitpos > 0) {
        if (*out_pos + 1 > *out_cap) {
            int newcap = (*out_cap == 0) ? 1024 : (*out_cap * 2);
            uint8_t *tmp = (uint8_t*)realloc(*outbuf, newcap);
            if (tmp == NULL) return 0;
            *outbuf = tmp;
            *out_cap = newcap;
        }
        (*outbuf)[(*out_pos)++] = *curr_byte;
        *curr_byte = 0;
        *curr_bitpos = 0;
    }
    return 1;
}

typedef int (*bit_reader_fn)(const uint8_t*, int , int* , int* , int , int*);
/************************************************************
 * Core decompress routine with configurable code-size timing.
 *
 * increase_before_read:
 *   - if non-zero, the decoder will check (and increase) code_size before reading each next code
 *     when next_code >= (1<<code_size). This matches some encoder variants.
 *   - if zero, code_size growth is handled after adding new dictionary entries (alternate timing).
 *
 * Returns:
 *   1 on success
 *   0 on failure (or insufficient output buffer). If insufficient buffer, *OutSize is set to required size.
 ************************************************************/
static int decompress_core_with_reader(uint8_t *in, int InSize, uint8_t *out, int *OutSize, int increase_before_read, bit_reader_fn reader)
{
    if (in == NULL || InSize <= 0 || out == NULL || OutSize == NULL) return 0;
    int out_capacity = *OutSize;
    int out_pos = 0;

    // prefix table: prefix[code] = previous code index, append_char[code] = last char
    int *prefix = (int*)malloc(MAX_DICT * sizeof(int));
    uint8_t *append_char = (uint8_t*)malloc(MAX_DICT * sizeof(uint8_t));
    if (!prefix || !append_char) { if (prefix) free(prefix); if (append_char) free(append_char); return 0; }

    // initialize table entries 0..255
    for (int i = 0; i < 256; ++i) {
        prefix[i] = -1;
        append_char[i] = (uint8_t)i;
    }
    for (int i = 256; i < MAX_DICT; ++i) {
        prefix[i] = 0;
        append_char[i] = 0;
    }

    int next_code = 258; // next available code index
    int code_size = MIN_CODE_SIZE;
    int code_mask = (1 << code_size) - 1;

    int byte_pos = 0;
    int bit_pos = 0; // MSB-first consumed bits in current byte
    int code = 0;

    // function to output a code's sequence into out (by reconstructing from prefix table)
    // uses a stack buffer on stack of size MAX_DICT (worst-case length)
    uint8_t *stack = (uint8_t*)malloc(MAX_DICT);
    if (!stack) { free(prefix); free(append_char); return 0; }

    // Read first code (after possibly skipping initial CLEAR)
    if (!read_bits_msb(in, InSize, &byte_pos, &bit_pos, code_size, &code)) {
        free(prefix); free(append_char); free(stack); *OutSize = 0; return 0;
    }
    if (code == CLEAR_CODE) {
        // reset
        next_code = 258;
        code_size = MIN_CODE_SIZE;
        code_mask = (1 << code_size) - 1;
        if (!read_bits_msb(in, InSize, &byte_pos, &bit_pos, code_size, &code)) {
            free(prefix); free(append_char); free(stack); *OutSize = 0; return 0;
        }
    }
    if (code == EOI_CODE) {
        free(prefix); free(append_char); free(stack); *OutSize = 0; return 1;
    }

    // write first code
    if (code < 256) {
        if (out_pos + 1 > out_capacity) { *OutSize = out_pos + 1; free(prefix); free(append_char); free(stack); return 0; }
        out[out_pos++] = (uint8_t)code;
    } else {
        // invalid first code
        free(prefix); free(append_char); free(stack); *OutSize = 0; return 0;
    }

    int prev_code = code;

    while (1) {
        // adjust code size BEFORE reading next code if mode requests it
        if (increase_before_read && next_code >= (1 << code_size) && code_size < MAX_CODE_SIZE) {
            ++code_size;
            code_mask = (1 << code_size) - 1;
        }

        if (!read_bits_msb(in, InSize, &byte_pos, &bit_pos, code_size, &code)) break;

        if (code == CLEAR_CODE) {
            // reset table
            next_code = 258;
            code_size = MIN_CODE_SIZE;
            code_mask = (1 << code_size) - 1;
            // read next code after clear
            if (!read_bits_msb(in, InSize, &byte_pos, &bit_pos, code_size, &code)) break;
            if (code == EOI_CODE) break;
            // output code (must be in 0..255)
            if (code >= 0 && code < 256) {
                if (out_pos + 1 > out_capacity) { *OutSize = out_pos + 1; free(prefix); free(append_char); free(stack); return 0; }
                out[out_pos++] = (uint8_t)code;
                prev_code = code;
                continue;
            } else {
                // invalid
                free(prefix); free(append_char); free(stack); *OutSize = 0; return 0;
            }
        } else if (code == EOI_CODE) {
            break;
        }

        // decode code -> sequence (handle KwKwK)
        int cur_code = code;
        int stack_pos = 0;

        if (cur_code < next_code) {
            // reconstruct sequence by walking prefixes
            int cc = cur_code;
            while (cc >= 0) {
                stack[stack_pos++] = append_char[cc];
                cc = prefix[cc];
            }
        } else if (cur_code == next_code) {
            // special case: KwKwK => sequence = prev_sequence + first_char(prev_sequence)
            // find first char of prev sequence
            int cc = prev_code;
            uint8_t first_char = 0;
            // walk to first element (prefix -1 gives single byte)
            int path_len = 0;
            while (cc >= 0) {
                first_char = append_char[cc];
                int p = prefix[cc];
                if (p < 0) break;
                cc = p;
                ++path_len;
                if (path_len > MAX_DICT) { free(prefix); free(append_char); free(stack); *OutSize = 0; return 0; }
            }
            // now reconstruct prev sequence into stack
            cc = prev_code;
            while (cc >= 0) {
                stack[stack_pos++] = append_char[cc];
                cc = prefix[cc];
            }
            // append first_char
            stack[stack_pos++] = first_char;
        } else {
            // invalid code
            free(prefix); free(append_char); free(stack); *OutSize = 0; return 0;
        }

        // stack currently holds sequence in reverse order; compute length
        int seq_len = stack_pos;
        // output sequence in correct order
        if (out_pos + seq_len > out_capacity) { *OutSize = out_pos + seq_len; free(prefix); free(append_char); free(stack); return 0; }
        for (int i = seq_len - 1; i >= 0; --i) {
            out[out_pos++] = stack[i];
        }

        // add new dictionary entry: prev_sequence + first_char_of_current_sequence
        if (next_code < MAX_DICT) {
            // find first char of current sequence (stack[stack_pos-1] is first when reversed)
            uint8_t first_char_of_current = stack[stack_pos - 1];
            prefix[next_code] = prev_code;
            append_char[next_code] = first_char_of_current;
            ++next_code;
            // adjust code size AFTER adding if mode specifies (increase_before_read==0 => adjust here)
            if (!increase_before_read && next_code > (1 << code_size) && code_size < MAX_CODE_SIZE) {
                ++code_size;
                code_mask = (1 << code_size) - 1;
            }
        }

        prev_code = code;
    }

    free(stack);
    free(prefix);
    free(append_char);

    *OutSize = out_pos;
    return 1;
}

/************************************************************
 * Public Decompress wrapper:
 *   tries two timing modes to maximize compatibility with Photoshop TIFFs.
 ************************************************************/
int LZW_Decompress(uint8_t *in, int InSize, uint8_t *out, int *OutSize)
{
    if (!in || InSize <= 0 || !out || !OutSize) return 0;

    int outcap = *OutSize;
    int outsize_try = outcap;

    // 1) MSB, earlyChange = 1 (Photoshop commonly)
    outsize_try = outcap;
    if (decompress_core_with_reader(in, InSize, out, &outsize_try, 1, read_bits_msb)) {
        *OutSize = outsize_try;
        return 1;
    }
    // 2) MSB, earlyChange = 0
    outsize_try = outcap;
    if (decompress_core_with_reader(in, InSize, out, &outsize_try, 0, read_bits_msb)) {
        *OutSize = outsize_try;
        return 1;
    }
    // 3) LSB, earlyChange = 1
    outsize_try = outcap;
    if (decompress_core_with_reader(in, InSize, out, &outsize_try, 1, read_bits_lsb)) {
        *OutSize = outsize_try;
        return 1;
    }
    // 4) LSB, earlyChange = 0
    outsize_try = outcap;
    if (decompress_core_with_reader(in, InSize, out, &outsize_try, 0, read_bits_lsb)) {
        *OutSize = outsize_try;
        return 1;
    }

    // All failed; choose largest required size reported (or leave OutSize unchanged)
    // Optionally log debug info to help diagnosis.
    return 0;
}

/************************************************************
 * Simple (unoptimized) compressor producing MSB-first codes.
 * Linear dictionary search used to avoid STL.
 *
 * Returns 1 on success, 0 on failure or if out buffer too small (OutSize set to required size).
 ************************************************************/
static int dict_find(uint8_t **table, int *table_len, int next_code, const uint8_t *seq, int seq_len)
{
    for (int i = 0; i < next_code; ++i) {
        if (!table[i]) continue;
        if (table_len[i] != seq_len) continue;
        if (memcmp(table[i], seq, seq_len) == 0) return i;
    }
    return -1;
}

int LZW_Compress(uint8_t *in, int InSize, uint8_t *out, int *OutSize)
{
    if (in == NULL || InSize < 0 || out == NULL || OutSize == NULL) return 0;

    // Build initial dictionary
    uint8_t **table = (uint8_t**)malloc(MAX_DICT * sizeof(uint8_t*));
    int *table_len = (int*)malloc(MAX_DICT * sizeof(int));
    if (!table || !table_len) { if (table) free(table); if (table_len) free(table_len); return 0; }
    for (int i = 0; i < 256; ++i) {
        table[i] = (uint8_t*)malloc(1);
        if (!table[i]) { // cleanup
            for (int j = 0; j < i; ++j) free(table[j]);
            free(table); free(table_len); return 0;
        }
        table[i][0] = (uint8_t)i;
        table_len[i] = 1;
    }
    for (int i = 256; i < MAX_DICT; ++i) { table[i] = NULL; table_len[i] = 0; }

    int next_code = 258;
    int code_size = MIN_CODE_SIZE;
    int max_code_for_size = (1 << code_size);

    // dynamic output buffer (growable)
    uint8_t *dyn_out = NULL;
    int dyn_cap = 0;
    int dyn_pos = 0;
    uint8_t curr_byte = 0;
    int curr_bitpos = 0;

    // write CLEAR_CODE
    if (!bw_write_bits_msb(&dyn_out, &dyn_cap, &dyn_pos, CLEAR_CODE, code_size, &curr_byte, &curr_bitpos)) goto compress_fail;

    if (InSize == 0) {
        // write EOI and flush
        if (!bw_write_bits_msb(&dyn_out, &dyn_cap, &dyn_pos, EOI_CODE, code_size, &curr_byte, &curr_bitpos)) goto compress_fail;
        if (!bw_flush_msb(&dyn_out, &dyn_cap, &dyn_pos, &curr_byte, &curr_bitpos)) goto compress_fail;
        if (dyn_pos <= *OutSize) {
            memcpy(out, dyn_out, dyn_pos);
            *OutSize = dyn_pos;
            // cleanup
            for (int i = 0; i < MAX_DICT; ++i) if (table[i]) free(table[i]);
            free(table); free(table_len); free(dyn_out);
            return 1;
        } else {
            *OutSize = dyn_pos;
            for (int i = 0; i < MAX_DICT; ++i) if (table[i]) free(table[i]);
            free(table); free(table_len); free(dyn_out);
            return 0;
        }
    }

    // w = first byte
    uint8_t *w = (uint8_t*)malloc(1);
    if (!w) goto compress_fail;
    w[0] = in[0];
    int w_len = 1;
    int w_code = (int)w[0];

    for (int i = 1; i < InSize; ++i) {
        uint8_t k = in[i];
        int wc_len = w_len + 1;
        uint8_t *wc = (uint8_t*)malloc(wc_len);
        if (!wc) { free(w); goto compress_fail; }
        memcpy(wc, w, w_len);
        wc[wc_len - 1] = k;

        int found = dict_find(table, table_len, next_code, wc, wc_len);
        if (found != -1) {
            // w = wc
            free(w);
            w = wc;
            w_len = wc_len;
            w_code = found;
        } else {
            // output code for w
            if (!bw_write_bits_msb(&dyn_out, &dyn_cap, &dyn_pos, (uint32_t)w_code, code_size, &curr_byte, &curr_bitpos)) {
                free(wc); free(w); goto compress_fail;
            }

            // add wc to dictionary
            if (next_code < MAX_DICT) {
                uint8_t *newseq = (uint8_t*)malloc(wc_len);
                if (!newseq) { free(wc); free(w); goto compress_fail; }
                memcpy(newseq, wc, wc_len);
                table[next_code] = newseq;
                table_len[next_code] = wc_len;
                ++next_code;

                if (next_code >= max_code_for_size && code_size < MAX_CODE_SIZE) {
                    ++code_size;
                    max_code_for_size = (1 << code_size);
                } else if (next_code >= MAX_DICT) {
                    // emit CLEAR and reset
                    if (!bw_write_bits_msb(&dyn_out, &dyn_cap, &dyn_pos, CLEAR_CODE, code_size, &curr_byte, &curr_bitpos)) {
                        free(wc); free(w); goto compress_fail;
                    }
                    for (int j = 258; j < MAX_DICT; ++j) { if (table[j]) { free(table[j]); table[j] = NULL; table_len[j] = 0; } }
                    next_code = 258;
                    code_size = MIN_CODE_SIZE;
                    max_code_for_size = (1 << code_size);
                }
            }

            // w = [k]
            free(w);
            w = (uint8_t*)malloc(1);
            if (!w) { free(wc); goto compress_fail; }
            w[0] = k;
            w_len = 1;
            w_code = (int)k;

            free(wc);
        }
    }

    // output w
    if (!bw_write_bits_msb(&dyn_out, &dyn_cap, &dyn_pos, (uint32_t)w_code, code_size, &curr_byte, &curr_bitpos)) { free(w); goto compress_fail; }
    free(w);

    // write EOI
    if (!bw_write_bits_msb(&dyn_out, &dyn_cap, &dyn_pos, EOI_CODE, code_size, &curr_byte, &curr_bitpos)) goto compress_fail;
    if (!bw_flush_msb(&dyn_out, &dyn_cap, &dyn_pos, &curr_byte, &curr_bitpos)) goto compress_fail;

    // copy to caller buffer if fits
    if (dyn_pos <= *OutSize) {
        memcpy(out, dyn_out, dyn_pos);
        *OutSize = dyn_pos;
        for (int i = 0; i < MAX_DICT; ++i) if (table[i]) free(table[i]);
        free(table); free(table_len); free(dyn_out);
        return 1;
    } else {
        *OutSize = dyn_pos;
        for (int i = 0; i < MAX_DICT; ++i) if (table[i]) free(table[i]);
        free(table); free(table_len); free(dyn_out);
        return 0;
    }

compress_fail:
    if (dyn_out) free(dyn_out);
    for (int i = 0; i < MAX_DICT; ++i) if (table && table[i]) free(table[i]);
    if (table) free(table);
    if (table_len) free(table_len);
    return 0;
}