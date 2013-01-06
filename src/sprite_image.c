#include "shadowdive/sprite_image.h"
#include "shadowdive/rgba_image.h"
#include "shadowdive/palette.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

sd_sprite_image* sd_sprite_image_create(unsigned int w, unsigned int h, unsigned int len) {
    sd_sprite_image *img = (sd_sprite_image*)malloc(sizeof(sd_sprite_image));
    img->w = w;
    img->h = h;
    img->len = len;
    img->data = (char*)malloc(len);
    memset(img->data, 0, len);
    return img;
}

void sd_sprite_image_delete(sd_sprite_image *img, int missing) {
    if (!missing) {
        free(img->data);
    }
    free(img);
}

char color_to_palette_index(uint8_t r, uint8_t g, uint8_t b, sd_palette *pal) {
    /*for(int i = 255; i >= 0; i--) {*/
    for(int i = 0; i < 256; i++) {
        uint8_t red = pal->data[i][0] & 0xff;
        uint8_t blue = pal->data[i][1] & 0xff;
        uint8_t green = pal->data[i][2] & 0xff;
        if (red == r && blue == b && green == g) {
            /*if (i != 63) {*/
                /*printf("Red %u == %u; blue %u == %u; green %u == %u -- %u\n", red, r, blue, b, green, g, i);*/
            /*}*/
            return i;
        }
    }
    return 0;
}

sd_sprite_image* sd_sprite_image_encode(sd_rgba_image *img, sd_palette *pal, int remapping) {
    int lastx = -1;
    int lasty = 0;
    unsigned int rgb_size = (img->w * img->h * 4);
    // allocate a buffer plenty big enough, we will trim it later
    char *buf = (char*)malloc(img->w * img->h * 4);
    int i = 0;
    int rowlen = 0;
    uint16_t c;
    printf("writing Y as %d at %d\n", 0, i);
    // always initialize Y to 0
    buf[i++] = 2;
    buf[i++] = 0;
    int rowstart = i;

    for(int pos = 0; pos <= rgb_size; pos+= 4) {
        uint8_t r = img->data[pos];
        uint8_t g = img->data[pos+1];
        uint8_t b = img->data[pos+2];
        uint8_t a = img->data[pos+3];

        // ignore anytjhing but fully opaque pixels
        if (a == 255) {
            int16_t x = (pos/4) % img->w;
            int16_t y = (pos/4) / img->w;
            if (y != lasty) {
                // new row
                printf("writing Y as %d at %d\n", y, i);
                c = (y*4)+2;
                // write little endian unsigned word
                buf[i++] = c & 0x00ff;
                buf[i++] = (c & 0xff00) >> 8;
                if (!rowlen) {
                    printf("rowstart0 is %d\n", i);
                    rowstart = i;
                    i+=2;
                }
                lasty=y;
            }
            if (x != lastx+1) {
                // dont write X coordinate if we just wrote a row and the nex X coordinate is 0
                // because the decoder resets X coordinate to 0 after each row
                if (x != 0) {
                    // we skipped some columns
                    printf("writing X as %d at %d\n", x, i);
                    c = (x*4);
                    // write little endian unsigned word
                    buf[i++] = c & 0x00ff;
                    buf[i++] = (c & 0xff00) >> 8;
                }
                if (!rowlen) {
                    printf("rowstart1 is %d\n", i);
                    rowstart = i;
                    i+=2;
                }
            }
            // write out the length of the previous row, if there was one
            if (y != lasty || x != lastx+1) {
                if (rowlen) {
                    // go back and write in the width of the row of pixels
                    c = (rowlen*4)+1;
                    // place to write is at i - rowlen
                    printf("i is %d, rowlen is %d, writing R at %d\n", i, rowlen, rowstart);
                    // write little endian unsigned word
                    buf[rowstart] = c & 0x00ff;
                    buf[rowstart+1] = (c & 0xff00) >> 8;
                    rowlen = 0;
                    lastx = -1;
                    printf("rowstart2 is %d\n", i);
                    rowstart = i;
                    i+=2;
                }
            } else if (lasty == 0 && x == 0) {
                printf("rowstart3 is %d\n", i);
                rowstart = i;
                i+=2;
            }


            lastx=x;
            // write byte
            buf[i++] = color_to_palette_index(r, g, b, pal);
            rowlen++;
            /*printf("rgba at pos %u X: %u Y: %u %u %u %u %u\n", pos, x, y, r, g, b, a);*/
        }
    }
    // update the length of the last row
    if (rowlen) {
        // go back and write in the width of the row of pixels
        c = (rowlen*4)+1;
        // place to write is at i - rowlen
        printf("LAST i is %d, rowlen is %d, writing R at %d\n", i, rowlen, rowstart);
        // write little endian unsigned word
        buf[rowstart] = c & 0x00ff;
        buf[rowstart+1] = (c & 0xff00) >> 8;
    }

    printf("writing EOF at %d\n", i);
    // End of sprite marker, a WORD of value 7
    buf[i++] = 7; 
    buf[i++] = 0;
    sd_sprite_image *sprite = sd_sprite_image_create(img->w, img->h, i);
    memcpy(sprite->data, buf, i);
    free(buf);
    return sprite;
}

sd_rgba_image* sd_sprite_image_decode(sd_sprite_image *img, sd_palette *pal, int remapping) {
    sd_rgba_image *rgba = img->len>0 ? sd_rgba_image_create(img->w, img->h) : sd_rgba_image_create(1, 1);
    uint16_t x = 0;
    uint16_t y = 0;
    int i = 0;
    if (img->w == 0 || img->h == 0) {
        // XXX CREDITS.BK has a bunch of 0 width sprites, for some unknown reason
        return rgba;
    }
    while(i < img->len) {
        // read a word
        uint16_t c = (uint8_t)img->data[i] + ((uint8_t)img->data[i+1] << 8);
        char op = c % 4;
        uint16_t data = c / 4;
        i += 2; // we read 2 bytes
        switch(op) {
            case 0:
                x = data;
                printf("X is %u @ %d\n", x, i-2);
                break;
            case 2:
                y = data;
                printf("Y is %u @ %d\n", y, i-2);
                break;
            case 1:
                printf("R is %u @ %d\n", data, i-2);
                while(data > 0) {
                    uint8_t b = img->data[i];
                    int pos = ((y * img->w) + x) * 4;
                    if(remapping > -1) {
                        rgba->data[pos+0] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][0];
                        rgba->data[pos+1] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][1];
                        rgba->data[pos+2] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][2];
                    } else {
                        rgba->data[pos+0] = (uint8_t)pal->data[b][0];
                        rgba->data[pos+1] = (uint8_t)pal->data[b][1];
                        rgba->data[pos+2] = (uint8_t)pal->data[b][2];
                    }
                    rgba->data[pos+3] = 255; // fully opaque
                    i++; // we read 1 byte
                    x++;
                    data--;
                }
                x = 0;
                break;
            case 3:
                printf("EOF @ %d\n", i-2);
                assert(i == img->len);
                break;
        }
    }
    return rgba;
}

