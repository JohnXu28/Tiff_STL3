/**
 * @brief Halftone – ordered dithering with arbitrary M×N table, Grayscale → 1-bit packed output
 *
 * @param src        Input grayscale image (1 byte/pixel)
 * @param dst        Output 1-bit image (pre-allocated, (width+7)/8 * height bytes)
 * @param width      Image width in pixels
 * @param height     Image height in lines
 * @param table      Threshold table [table_rows * table_cols], values 0..255, row-major
 * @param table_rows Number of rows in the table (vertical period)
 * @param table_cols Number of columns in the table (horizontal period)
 * @return 0 on success, -1 on invalid parameters or allocation failure
 */

/* Standard 4×4 Bayer ordered dither table, pre-scaled to [0..255].
 * Pass as 'table' with table_rows=4, table_cols=4. */
#include <stdint.h>
#include < stdio.h >
#include < cstdlib >
#include < string.h >

typedef uint8_t byte;
const uint8_t imgproc_halftone_bayer4[4][4] = {
    {   0, 136,  34, 170 },
    { 204,  68, 238, 102 },
    {  51, 187,  17, 153 },
    { 255, 119, 221,  85 }
};

int imgproc_halftone_1bit(const uint8_t *src, uint8_t *dst,
                           int width, int height,
                           const uint8_t *table, int table_rows, int table_cols)
{
    if (!src || !dst || !table || width <= 0 || height <= 0 ||
        table_rows <= 0 || table_cols <= 0)
        return -1;

    /* Log input parameters for debugging and traceability */
    printf("imgproc_halftone_1bit: src=%p dst=%p width=%d height=%d table=%p rows=%d cols=%d\n",
           (const void*)src, (void*)dst, width, height, (const void*)table, table_rows, table_cols);

    int dst_stride = (width + 7) / 8;

    /* Pre-tile ALL table rows into thr_table[table_rows × width] once.
     * The pattern repeats every table_rows lines; by pre-computing every
     * tiled row up-front, the main loop only needs a pointer lookup per line
     * instead of rebuilding the threshold row each time (96×5100 = ~478 KB
     * stays resident in L2 cache across the full page). */
    uint8_t *thr_table = (uint8_t *)malloc((size_t)table_rows * (size_t)width);
    if (!thr_table) return -1;
    {
        int tr, tx, x;
        for (tr = 0; tr < table_rows; tr++) {
            const uint8_t *tbl_row = table + tr * table_cols;
            uint8_t       *thr_row = thr_table + tr * width;
            tx = 0;
            for (x = 0; x < width; x++) {
                thr_row[x] = tbl_row[tx];
                if (++tx >= table_cols) tx = 0;
            }
        }
    }

    /* Zero the entire output buffer in one shot – faster than per-row memset. */
    memset(dst, 0, (size_t)dst_stride * (size_t)height);

    int     simd_w = 0; /* unused in #else but keeps compiler happy */
    (void)simd_w;

    int ty = 0;
    int y;
    for (y = 0; y < height; y++) {
        const uint8_t *src_row = src + y * width;
        uint8_t       *dst_row = dst + y * dst_stride;
        const uint8_t *thr_row = thr_table + (size_t)ty * width;
        int x;

        if (++ty >= table_rows) ty = 0;

        /* Scalar path: process 8 pixels per iteration, branchless bit-pack */
        int x8 = (width >> 3) << 3; /* largest multiple of 8 */
        for (x = 0; x < x8; x += 8) {
            dst_row[x >> 3] = (uint8_t)(
                ((src_row[x+0] > thr_row[x+0]) << 7) |
                ((src_row[x+1] > thr_row[x+1]) << 6) |
                ((src_row[x+2] > thr_row[x+2]) << 5) |
                ((src_row[x+3] > thr_row[x+3]) << 4) |
                ((src_row[x+4] > thr_row[x+4]) << 3) |
                ((src_row[x+5] > thr_row[x+5]) << 2) |
                ((src_row[x+6] > thr_row[x+6]) << 1) |
                 (src_row[x+7] > thr_row[x+7]));
        }
        for (; x < width; x++) {
            if (src_row[x] > thr_row[x])
                dst_row[x >> 3] |= (uint8_t)(1 << (7 - (x & 7)));
        }
    }

    free(thr_table);
    return 0;
}