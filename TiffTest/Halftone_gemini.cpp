#include <stdint.h>
#include <stddef.h>

/**
 * 將 8-bit 灰階影像轉換為 1-bit 半色調影像並打包 (支援任意大小的半色調矩陣)
 *
 * @param in_gray      輸入的 8-bit 灰階影像陣列
 * @param out_1bit     輸出的 1-bit 打包陣列 (需配置 ((width + 7) / 8) * height 的大小)
 * @param width        影像寬度
 * @param height       影像高度
 * @param dither_table 任意大小的半色調矩陣 (以 1D 陣列形式傳入)
 * @param table_w      半色調矩陣的寬度
 * @param table_h      半色調矩陣的高度
 */
void convert_gray8_to_halftone1bit_dynamic(
    const uint8_t *in_gray,
    uint8_t *out_1bit,
    int width,
    int height,
    const uint8_t *dither_table,
    int table_w,
    int table_h)
{
    int out_index = 0;

    for (int y = 0; y < height; y++) {
        // 預先計算目前 Y 座標對應到 dither table 的哪一列 (Row)
        int table_y = y % table_h;

        for (int x = 0; x < width; x += 8) { // 每次打包 8 個像素
            uint8_t packed_byte = 0;

            for (int bit = 0; bit < 8; bit++) {
                int current_x = x + bit;
                
                // 處理邊界：如果影像寬度不是 8 的倍數，直接中斷內層迴圈，剩下的 bit 保持為 0
                if (current_x >= width) break;

                // 取得目前像素的灰階值
                uint8_t pixel_val = in_gray[y * width + current_x];

                // 計算 X 座標對應到 dither table 的哪一行 (Column)
                int table_x = current_x % table_w;

                // 從 1D 陣列中取出對應的閥值 (Threshold)
                uint8_t threshold = dither_table[table_y * table_w + table_x];

                // 如果像素值大於閥值，設為 1 (白)
                if (pixel_val > threshold) {
                    packed_byte |= (1 << (7 - bit)); 
                }
            }

            // 將打包好的 1 個 Byte 寫入輸出陣列
            out_1bit[out_index++] = packed_byte;
        }
    }
}