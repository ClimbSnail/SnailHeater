# Copyright (c) 2021 W-Mai
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
# lvgl_image_converter/lv_img_converter/lv_img_converter.py
# Created by W-Mai on 2021/2/17.
# repo: https://github.com/W-Mai/lvgl_image_converter
#
##############################################################


import struct
from typing import *
from decimal import Decimal, ROUND_HALF_DOWN

from PIL import Image


def get_color_from_palette(palette, index):
    return [palette[3 * index + i] for i in range(3)]


def check_exist(li: List, index: int):
    if index >= len(li):
        return 0
    if index < len(li) and li[index] is None:
        return -1
    if index < len(li) and li[index] is not None:
        return 1


def force_update(li: List, index: int, elem: Any):
    check_res = check_exist(li, index)
    if check_res:
        li[index] = elem
    elif check_res == 0:
        li.extend([None] * (index - len(li)))
        li.append(elem)


class _CONST:
    class ConstError(TypeError):
        pass

    CF_TRUE_COLOR_332 = 0  # Helper formats. Used internally
    CF_TRUE_COLOR_565 = 1
    CF_TRUE_COLOR_565_SWAP = 2
    CF_TRUE_COLOR_888 = 3
    CF_ALPHA_1_BIT = 4
    CF_ALPHA_2_BIT = 5
    CF_ALPHA_4_BIT = 6
    CF_ALPHA_8_BIT = 7
    CF_INDEXED_1_BIT = 8
    CF_INDEXED_2_BIT = 9
    CF_INDEXED_4_BIT = 10
    CF_INDEXED_8_BIT = 11
    CF_RAW = 12
    CF_RAW_ALPHA = 13
    CF_RAW_CHROMA = 12

    CF_TRUE_COLOR = 100  # Helper formats is C arrays contains all true color formats (using in "download")
    CF_TRUE_COLOR_ALPHA = 101
    CF_TRUE_COLOR_CHROMA = 102

    def __setattr__(self, name, value):
        raise self.ConstError(f"Can't rebind const {name}")


class Converter(object):
    FLAG = _CONST()

    def __init__(
        self,
        path: str,
        out_name: str,
        dither: bool = True,
        cf=FLAG.CF_INDEXED_4_BIT,
        cf_palette_bgr_en=True,
    ):
        self.dither = None  # Dithering enable/disable
        self.w = None  # Image width
        self.h = None  # Image height
        self.cf = None  # Color format
        self.alpha = None  # Add alpha byte or not
        self.chroma = None  # Chroma keyed?
        self.d_out = None  # Output data (result)
        self.img = None  # Image resource

        # Helper variables
        self.r_act = 0
        self.b_act = 0
        self.g_act = 0

        # For dithering
        self.r_earr = None  # Classification error for next row of pixels
        self.g_earr = None
        self.b_earr = None

        self.r_nerr = None  # Classification error for next pixel
        self.g_err = None
        self.b_nerr = None

        self.path = path
        self.out_name = out_name  # Name of the output file
        self.cf = cf
        self.dither = dither
        self.cf_palette_bgr_en = cf_palette_bgr_en

        if cf in [self.FLAG.CF_RAW, self.FLAG.CF_RAW_ALPHA, self.FLAG.CF_RAW_CHROMA]:
            return
        self.img: Image.Image = Image.open(path)
        self.w, self.h = self.img.size

        if self.dither:
            self.r_earr = [0] * (self.w + 2)
            self.g_earr = [0] * (self.w + 2)
            self.b_earr = [0] * (self.w + 2)

        self.r_nerr = 0
        self.g_nerr = 0
        self.b_nerr = 0

    def convert(self, cf=None, alpha: int = 0) -> NoReturn:
        if cf is not None:
            self.cf = cf
        self.d_out = []
        self.alpha = alpha

        if self.cf in (
            self.FLAG.CF_RAW,
            self.FLAG.CF_RAW_ALPHA,
            self.FLAG.CF_RAW_CHROMA,
        ):
            with open(self.path, "rb") as f:
                self.d_out = f.read()
            return

        palette_size = {
            self.FLAG.CF_INDEXED_1_BIT: 2,
            self.FLAG.CF_INDEXED_2_BIT: 4,
            self.FLAG.CF_INDEXED_4_BIT: 16,
            self.FLAG.CF_INDEXED_8_BIT: 256,
        }.get(self.cf, 0)

        if palette_size:
            img_tmp = Image.new("RGB", (self.w, self.h))
            img_tmp.paste(img_tmp, self.img.size)
            self.img = self.img.convert(mode="P", colors=palette_size)
            real_palette_size = len(
                self.img.getcolors()
            )  # The real number of colors in the image's palette
            real_palette = self.img.getpalette()
            # self.img.show()
            for i in range(palette_size):
                if i < real_palette_size:
                    c = get_color_from_palette(real_palette, i)
                    if self.cf_palette_bgr_en:
                        c = [c[2 - i] for i in range(3)]
                    self.d_out.extend(c)
                    self.d_out.append(0xFF)
                else:
                    self.d_out.extend([0xFF, 0xFF, 0xFF, 0xFF])

        # Convert all the pixels
        for y in range(self.h):
            self._dither_reset()
            for x in range(self.w):
                self._conv_px(x, y)

        # Revert the original image if it was converted to indexed
        if palette_size:
            self.img.paste(img_tmp)

    def format_to_c_array(self) -> AnyStr:
        c_array = ""
        i = 0
        y_end, x_end = self.h, self.w

        if self.cf == self.FLAG.CF_TRUE_COLOR_332:
            c_array += "\n#if LV_COLOR_DEPTH == 1 || LV_COLOR_DEPTH == 8"
            c_array += (
                "\n  /*Pixel format: Blue: 2 bit, Green: 3 bit, Red: 3 bit, Alpha 8 bit */"
                if self.alpha
                else "\n  /*Pixel format: Blue: 2 bit, Green: 3 bit, Red: 3 bit*/"
            )
        elif self.cf == self.FLAG.CF_TRUE_COLOR_565:
            c_array += "\n#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0"
            c_array += (
                "\n  /*Pixel format: Blue: 5 bit, Green: 6 bit, Red: 5 bit, Alpha 8 bit*/"
                if self.alpha
                else "\n  /*Pixel format: Blue: 5 bit, Green: 6 bit, Red: 5 bit*/"
            )
        elif self.cf == self.FLAG.CF_TRUE_COLOR_565_SWAP:
            c_array += "\n#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP != 0"
            c_array += (
                "\n  /*Pixel format:  Blue: 5 bit Green: 6 bit, Red: 5 bit, Alpha 8 bit  BUT the 2  color "
                "bytes are swapped*/"
                if self.alpha
                else "\n  /*Pixel format: Blue: 5 bit, Green: 6 bit, Red: 5 bit BUT the 2 bytes are swapped*/"
            )
        elif self.cf == self.FLAG.CF_TRUE_COLOR_888:
            c_array += "\n#if LV_COLOR_DEPTH == 32"
            c_array += (
                "\n  /*Pixel format: Blue: 8 bit, Green: 8 bit, Red: 8 bit, Fix 0xFF: 8 bit, */"
                if self.alpha
                else "\n  /*Pixel format:  Blue: 8 bit, Green: 8 bit, Red: 8 bit, Alpha: 8 bit*/"
            )
        elif self.cf == self.FLAG.CF_INDEXED_1_BIT:
            c_array += "\n  "
            for p in range(2):
                tmp_str = ", ".join(
                    [f"0x{self.d_out[p * 4 + s]:02X}" for s in range(4)]
                )
                tmp_str = ", ".join([tmp_str, f"\t/*Color of index {p}*/\n  "])
                c_array += tmp_str
            i = 2 * 4
        elif self.cf == self.FLAG.CF_INDEXED_2_BIT:
            c_array += "\n  "
            for p in range(4):
                tmp_str = ", ".join(
                    [f"0x{self.d_out[p * 4 + s]:02X}" for s in range(4)]
                )
                tmp_str = ", ".join([tmp_str, f"\t/*Color of index {p}*/\n  "])
                c_array += tmp_str
            i = 4 * 4
        elif self.cf == self.FLAG.CF_INDEXED_4_BIT:
            c_array += "\n  "
            for p in range(16):
                tmp_str = ", ".join(
                    [f"0x{self.d_out[p * 4 + s]:02X}" for s in range(4)]
                )
                tmp_str = ", ".join([tmp_str, f"\t/*Color of index {p}*/\n  "])
                c_array += tmp_str
            i = 16 * 4
        elif self.cf == self.FLAG.CF_INDEXED_8_BIT:
            c_array += "\n  "
            for p in range(256):
                tmp_str = ", ".join(
                    [f"0x{self.d_out[p * 4 + s]:02X}" for s in range(4)]
                )
                tmp_str = ", ".join([tmp_str, f"\t/*Color of index {p}*/\n  "])
                c_array += tmp_str
            i = 256 * 4
        elif self.cf in (
            self.FLAG.CF_RAW,
            self.FLAG.CF_RAW_ALPHA,
            self.FLAG.CF_RAW_CHROMA,
        ):
            y_end, x_end = 1, len(self.d_out)
            i = 1

        tmp_arr = []

        def append_and_increase():
            nonlocal i, tmp_arr
            tmp_arr.append(f"0x{self.d_out[i]:02X}")
            i += 1

        for y in range(y_end):
            for x in range(x_end):
                if self.cf == self.FLAG.CF_TRUE_COLOR_332:
                    append_and_increase()
                    if self.alpha:
                        append_and_increase()
                elif self.cf in (
                    self.FLAG.CF_TRUE_COLOR_565,
                    self.FLAG.CF_TRUE_COLOR_565_SWAP,
                ):
                    append_and_increase()
                    append_and_increase()
                    if self.alpha:
                        append_and_increase()
                elif self.cf == self.FLAG.CF_TRUE_COLOR_888:
                    append_and_increase()
                    append_and_increase()
                    append_and_increase()
                    append_and_increase()
                elif self.cf in (self.FLAG.CF_ALPHA_1_BIT, self.FLAG.CF_INDEXED_1_BIT):
                    if x & 0x7 == 0:
                        append_and_increase()
                elif self.cf in (self.FLAG.CF_ALPHA_2_BIT, self.FLAG.CF_INDEXED_2_BIT):
                    if x & 0x3 == 0:
                        append_and_increase()
                elif self.cf in (self.FLAG.CF_ALPHA_4_BIT, self.FLAG.CF_INDEXED_4_BIT):
                    if x & 0x1 == 0:
                        append_and_increase()
                elif self.cf in (self.FLAG.CF_ALPHA_8_BIT, self.FLAG.CF_INDEXED_8_BIT):
                    append_and_increase()
                elif self.cf in (
                    self.FLAG.CF_RAW,
                    self.FLAG.CF_RAW_ALPHA,
                    self.FLAG.CF_RAW_CHROMA,
                ):
                    append_and_increase()

        x_end = x_end // {
            self.FLAG.CF_ALPHA_1_BIT: 8,
            self.FLAG.CF_INDEXED_1_BIT: 8,
            self.FLAG.CF_ALPHA_2_BIT: 4,
            self.FLAG.CF_INDEXED_2_BIT: 4,
            self.FLAG.CF_ALPHA_4_BIT: 2,
            self.FLAG.CF_INDEXED_4_BIT: 2,
        }.get(self.cf, 1)

        x_end = x_end * {
            self.FLAG.CF_TRUE_COLOR_565: 3 if self.alpha else 2,
            self.FLAG.CF_TRUE_COLOR_565_SWAP: 3 if self.alpha else 2,
            self.FLAG.CF_TRUE_COLOR_888: 4,
        }.get(self.cf, 1)

        if self.cf in (
            self.FLAG.CF_RAW,
            self.FLAG.CF_RAW_ALPHA,
            self.FLAG.CF_RAW_CHROMA,
        ):
            tmp_str = "\n  " + ", \n  ".join(
                ", ".join(tmp_arr[(x_end // 16) * x: (x_end // 16) * x + 16])
                for x in range(x_end // 16)
            )
        else:
            tmp_str = "\n  " + ", \n  ".join(
                ", ".join(tmp_arr[y * x_end: (y + 1) * x_end]) for y in range(y_end)
            )

        c_array += tmp_str
        if self.cf in (
            self.FLAG.CF_TRUE_COLOR_332,
            self.FLAG.CF_TRUE_COLOR_565,
            self.FLAG.CF_TRUE_COLOR_565_SWAP,
            self.FLAG.CF_TRUE_COLOR_888,
        ):
            c_array = c_array + "\n#endif"

        return c_array

    def _get_c_header(self) -> AnyStr:
        c_header = r"""#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "../lvgl/lvgl.h"
#endif

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif
"""
        attr_name = "LV_ATTRIBUTE_IMG_" + self.out_name.upper()
        c_header = rf"""{c_header}
#ifndef {attr_name}
#define {attr_name}
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST {attr_name} uint8_t {self.out_name}_map[] = {{"""
        return c_header

    def _get_c_footer(self, cf) -> AnyStr:
        c_footer = rf"""
}};

const lv_img_dsc_t {self.out_name} = {{
  .header.always_zero = 0,
  .header.w = {self.w},
  .header.h = {self.h},
  .data_size = """
        c_footer += {
            self.FLAG.CF_TRUE_COLOR: f"{self.w * self.h} * LV_COLOR_SIZE / 8,\n  "
            f".header.cf = LV_IMG_CF_TRUE_COLOR,",
            self.FLAG.CF_TRUE_COLOR_ALPHA: f"{self.w * self.h} * LV_IMG_PX_SIZE_ALPHA_BYTE,\n  .header.cf = "
            f"LV_IMG_CF_TRUE_COLOR_ALPHA,",
            self.FLAG.CF_TRUE_COLOR_CHROMA: f"{self.w * self.h} * LV_COLOR_SIZE / 8,\n  .header.cf = "
            f"LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED,",
            self.FLAG.CF_ALPHA_1_BIT: f"{len(self.d_out)},\n  .header.cf = LV_IMG_CF_ALPHA_1BIT,",
            self.FLAG.CF_ALPHA_2_BIT: f"{len(self.d_out)},\n  .header.cf = LV_IMG_CF_ALPHA_2BIT,",
            self.FLAG.CF_ALPHA_4_BIT: f"{len(self.d_out)},\n  .header.cf = LV_IMG_CF_ALPHA_4BIT,",
            self.FLAG.CF_ALPHA_8_BIT: f"{len(self.d_out)},\n  .header.cf = LV_IMG_CF_ALPHA_8BIT,",
            self.FLAG.CF_INDEXED_1_BIT: f"{len(self.d_out)},\n  .header.cf = LV_IMG_CF_INDEXED_1BIT,",
            self.FLAG.CF_INDEXED_2_BIT: f"{len(self.d_out)},\n  .header.cf = LV_IMG_CF_INDEXED_2BIT,",
            self.FLAG.CF_INDEXED_4_BIT: f"{len(self.d_out)},\n  .header.cf = LV_IMG_CF_INDEXED_4BIT,",
            self.FLAG.CF_INDEXED_8_BIT: f"{len(self.d_out)},\n  .header.cf = LV_IMG_CF_INDEXED_8BIT,",
            self.FLAG.CF_RAW: f"{len(self.d_out)},\n  .header.cf = LV_IMG_CF_RAW,",
            self.FLAG.CF_RAW_ALPHA: f"{len(self.d_out)},\n  .header.cf = LV_IMG_CF_RAW_ALPHA,",
            self.FLAG.CF_RAW_CHROMA: f"{len(self.d_out)},\n  .header.cf = LV_IMG_CF_RAW_CHROMA_KEYED,",
        }.get(cf, "") + f"\n  .data = {self.out_name}_map,\n}};\n"
        return c_footer

    def get_c_code_file(self, cf=-1, content="") -> AnyStr:
        if len(content) < 1:
            content = self.format_to_c_array()
        if cf < 0:
            cf = self.cf
        out = self._get_c_header() + content + self._get_c_footer(cf)
        return out

    def get_bin_file(self, cf=-1, content=None) -> bytes:
        if not content:
            content = self.d_out
        if cf < 0:
            cf = self.cf

        lv_cf = {  # Color format in lvgl
            self.FLAG.CF_TRUE_COLOR: 4,
            self.FLAG.CF_TRUE_COLOR_ALPHA: 5,
            self.FLAG.CF_TRUE_COLOR_CHROMA: 6,
            self.FLAG.CF_INDEXED_1_BIT: 7,
            self.FLAG.CF_INDEXED_2_BIT: 8,
            self.FLAG.CF_INDEXED_4_BIT: 9,
            self.FLAG.CF_INDEXED_8_BIT: 10,
            self.FLAG.CF_ALPHA_1_BIT: 11,
            self.FLAG.CF_ALPHA_2_BIT: 12,
            self.FLAG.CF_ALPHA_4_BIT: 13,
            self.FLAG.CF_ALPHA_8_BIT: 14,
        }.get(cf, 4)

        header = lv_cf + (self.w << 10) + (self.h << 21)
        header_bin = struct.pack("<L", header)
        content = struct.pack(f"<{len(content)}B", *content)
        return header_bin + content

    def _conv_px(self, x, y):
        c = self.img.getpixel((x, y))

        if self.img.mode == "P":
            c = get_color_from_palette(self.img.getpalette(), c)

        a = 0xFF
        if self.alpha and len(c) == 4:
            a = c[3]

        r, g, b = c[:3]
        cx = self.img.getpixel((x, y))
        self._dither_next(r, g, b, x)

        if self.cf == self.FLAG.CF_TRUE_COLOR_332:
            c8 = self.r_act | (self.g_act >> 3) | (self.b_act >> 6)  # RGB332
            self.d_out.append(c8)
            if self.alpha:
                self.d_out.append(a)
        elif self.cf == self.FLAG.CF_TRUE_COLOR_565:
            c16 = (self.r_act << 8) | (self.g_act << 3) | (self.b_act >> 3)  # RGB565
            self.d_out.append(c16 & 0xFF)
            self.d_out.append((c16 >> 8) & 0xFF)
            if self.alpha:
                self.d_out.append(a)
        elif self.cf == self.FLAG.CF_TRUE_COLOR_565_SWAP:
            c16 = (
                (self.r_act << 8) | (self.g_act << 3) | (self.b_act >> 3)
            )  # RGB565 SWAP
            self.d_out.append((c16 >> 8) & 0xFF)
            self.d_out.append(c16 & 0xFF)
            if self.alpha:
                self.d_out.append(a)
        elif self.cf == self.FLAG.CF_TRUE_COLOR_888:
            self.d_out.append(self.b_act)
            self.d_out.append(self.g_act)
            self.d_out.append(self.r_act)
            self.d_out.append(a)
        elif self.cf == self.FLAG.CF_ALPHA_1_BIT:
            w = self.w >> 3
            if self.w & 0x07:
                w += 1
            p = w * y + (x >> 3)
            if not check_exist(self.d_out, p):
                force_update(self.d_out, p, 0)  # Clear the bits first
            if a > 0x80:
                self.d_out[p] |= 1 << (7 - (x & 0x7))
        elif self.cf == self.FLAG.CF_ALPHA_2_BIT:
            w = self.w >> 2
            if self.w & 0x03:
                w += 1
            p = w * y + (x >> 2)
            if not check_exist(self.d_out, p):
                force_update(self.d_out, p, 0)  # Clear the bits first
            self.d_out[p] |= (a >> 6) << (6 - ((x & 0x3) * 2))

        elif self.cf == self.FLAG.CF_ALPHA_4_BIT:
            w = self.w >> 1
            if self.w & 0x01:
                w += 1

            p = w * y + (x >> 1)
            if not check_exist(self.d_out, p):
                force_update(self.d_out, p, 0)  # Clear the bits first
            self.d_out[p] |= (a >> 4) << (4 - ((x & 0x1) * 4))

        elif self.cf == self.FLAG.CF_ALPHA_8_BIT:
            p = self.w * y + x
            force_update(self.d_out, p, a)

        elif self.cf == self.FLAG.CF_INDEXED_1_BIT:
            w = self.w >> 3
            if self.w & 0x07:
                w += 1

            p = w * y + (x >> 3) + 8  # +8 for the palette
            if not check_exist(self.d_out, p):
                force_update(self.d_out, p, 0)  # Clear the bits first
            self.d_out[p] |= (cx & 0x1) << (7 - (x & 0x7))

        elif self.cf == self.FLAG.CF_INDEXED_2_BIT:
            w = self.w >> 2
            if self.w & 0x03:
                w += 1

            p = w * y + (x >> 2) + 16  # +16 for the palette
            if not check_exist(self.d_out, p):
                force_update(self.d_out, p, 0)  # Clear the bits first
            self.d_out[p] |= (cx & 0x3) << (6 - ((x & 0x3) * 2))

        elif self.cf == self.FLAG.CF_INDEXED_4_BIT:
            w = self.w >> 1
            if self.w & 0x01:
                w += 1

            p = w * y + (x >> 1) + 64  # +64 for the palette
            if not check_exist(self.d_out, p):
                force_update(self.d_out, p, 0)  # Clear the bits first
            self.d_out[p] |= (cx & 0xF) << (4 - ((x & 0x1) * 4))

        elif self.cf == self.FLAG.CF_INDEXED_8_BIT:
            p = self.w * y + x + 1024  # +1024 for the palette
            force_update(self.d_out, p, cx & 0xFF)

    def _dither_reset(self):
        if self.dither:
            self.r_nerr = 0
            self.g_nerr = 0
            self.b_nerr = 0

    def _dither_next(self, r, g, b, x):
        if self.dither:
            self.r_act = r + self.r_nerr + self.r_earr[x + 1]
            self.r_earr[x + 1] = 0

            self.g_act = g + self.g_nerr + self.g_earr[x + 1]
            self.g_earr[x + 1] = 0

            self.b_act = b + self.b_nerr + self.b_earr[x + 1]
            self.b_earr[x + 1] = 0

            if self.cf == self.FLAG.CF_TRUE_COLOR_332:
                self.r_act = self._classify_pixel(self.r_act, 3)
                self.g_act = self._classify_pixel(self.g_act, 3)
                self.b_act = self._classify_pixel(self.b_act, 2)

                if self.r_act > 0xE0:
                    self.r_act = 0xE0
                if self.g_act > 0xE0:
                    self.g_act = 0xE0
                if self.b_act > 0xC0:
                    self.b_act = 0xC0

            elif (
                self.cf == self.FLAG.CF_TRUE_COLOR_565
                or self.cf == self.FLAG.CF_TRUE_COLOR_565_SWAP
            ):
                self.r_act = self._classify_pixel(self.r_act, 5)
                self.g_act = self._classify_pixel(self.g_act, 6)
                self.b_act = self._classify_pixel(self.b_act, 5)

                if self.r_act > 0xF8:
                    self.r_act = 0xF8
                if self.g_act > 0xFC:
                    self.g_act = 0xFC
                if self.b_act > 0xF8:
                    self.b_act = 0xF8

            elif self.cf == self.FLAG.CF_TRUE_COLOR_888:
                self.r_act = self._classify_pixel(self.r_act, 8)
                self.g_act = self._classify_pixel(self.g_act, 8)
                self.b_act = self._classify_pixel(self.b_act, 8)

                if self.r_act > 0xFF:
                    self.r_act = 0xFF
                if self.g_act > 0xFF:
                    self.g_act = 0xFF
                if self.b_act > 0xFF:
                    self.b_act = 0xFF

            self.r_err = r - self.r_act
            self.g_err = g - self.g_act
            self.b_err = b - self.b_act

            self.r_nerr = round((7 * self.r_err) / 16)
            self.g_nerr = round((7 * self.g_err) / 16)
            self.b_nerr = round((7 * self.b_err) / 16)

            self.r_earr[x] += round((3 * self.r_err) / 16)
            self.g_earr[x] += round((3 * self.g_err) / 16)
            self.b_earr[x] += round((3 * self.b_err) / 16)

            self.r_earr[x + 1] += round((5 * self.r_err) / 16)
            self.g_earr[x + 1] += round((5 * self.g_err) / 16)
            self.b_earr[x + 1] += round((5 * self.b_err) / 16)

            self.r_earr[x + 2] += round(self.r_err / 16)
            self.g_earr[x + 2] += round(self.g_err / 16)
            self.b_earr[x + 2] += round(self.b_err / 16)

        elif self.cf == self.FLAG.CF_TRUE_COLOR_332:
            self.r_act = self._classify_pixel(r, 3)
            self.g_act = self._classify_pixel(g, 3)
            self.b_act = self._classify_pixel(b, 2)

            if self.r_act > 0xE0:
                self.r_act = 0xE0
            if self.g_act > 0xE0:
                self.g_act = 0xE0
            if self.b_act > 0xC0:
                self.b_act = 0xC0

        elif (
            self.cf == self.FLAG.CF_TRUE_COLOR_565
            or self.cf == self.FLAG.CF_TRUE_COLOR_565_SWAP
        ):
            self.r_act = self._classify_pixel(r, 5)
            self.g_act = self._classify_pixel(g, 6)
            self.b_act = self._classify_pixel(b, 5)

            if self.r_act > 0xF8:
                self.r_act = 0xF8
            if self.g_act > 0xFC:
                self.g_act = 0xFC
            if self.b_act > 0xF8:
                self.b_act = 0xF8

        elif self.cf == self.FLAG.CF_TRUE_COLOR_888:
            self.r_act = self._classify_pixel(r, 8)
            self.g_act = self._classify_pixel(g, 8)
            self.b_act = self._classify_pixel(b, 8)

            if self.r_act > 0xFF:
                self.r_act = 0xFF
            if self.g_act > 0xFF:
                self.g_act = 0xFF
            if self.b_act > 0xFF:
                self.b_act = 0xFF

    @staticmethod
    def _classify_pixel(value, bits):
        tmp = 1 << (8 - bits)
        val = int(Decimal(str(value / tmp)).quantize(Decimal('0'), rounding=ROUND_HALF_DOWN)) * tmp
        return val if val >= 0 else 0
