# lvgl_image_converter

![GitHub top language](https://img.shields.io/github/languages/top/W-Mai/lvgl_image_converter?style=for-the-badge)

## Install requirements

```
pip install -r requirements.txt
```

## Usage

```shell
usage: lv_img_conv.py [-h] [-f {true_color,true_color_alpha,true_color_chroma,indexed_1,indexed_2,indexed_4,indexed_8,alpha_1,alpha_2,alpha_4,alpha_8,raw,raw_alpha,raw_chroma}]
                      [-cf {RGB332,RGB565,RGB565SWAP,RGB888}] [-ff {C,BIN}] [-o O] [-r] [-d]
                      filepath [filepath ...]

positional arguments:
  filepath              images dir paths (or file paths) you wanna convert

options:
  -h, --help            show this help message and exit
  -f {true_color,true_color_alpha,true_color_chroma,indexed_1,indexed_2,indexed_4,indexed_8,alpha_1,alpha_2,alpha_4,alpha_8,raw,raw_alpha,raw_chroma}, -format {true_color,true_color_alpha,true_color_chroma,indexed_1,indexed_2,indexed_4,indexed_8,alpha_1,alpha_2,alpha_4,alpha_8,raw,raw_alpha,raw_chroma}
                        converted file format: true_color, true_color_alpha, true_color_chroma, indexed_1, indexed_2, indexed_4, indexed_8, alpha_1, alpha_2, alpha_4, alpha_8, raw, raw_alpha,
                        raw_chroma. The default is: true_color
  -cf {RGB332,RGB565,RGB565SWAP,RGB888}, -color-format {RGB332,RGB565,RGB565SWAP,RGB888}
                        converted color format: RGB332, RGB565, RGB565SWAP, RGB888
  -ff {C,BIN}, -file-format {C,BIN}
                        converted file format: C(*.c), BIN(*.bin)
  -o O, -output-filepath O
                        output file path. if not set, it will saved in the input dir
  -r                    convert files recursively
  -d                    need to dith
```

### Simple usage

Recursively convert pictures in the `images` directory with flags

- true_color_alpha
- RGB888
- BIN
- output to `test`

```shell
python3 lv_img_conv.py -f true_color_alpha -cf RGB888 -ff BIN -o test -r ./images
```

## Windows special version

> 内容参考了peng-zhihui在[HoloCubic](https://github.com/peng-zhihui/HoloCubic)中对原作者工程的修改

For more details, please see the [`./lv_img_conv_win_spec`](./lv_img_conv_win_spec) directory. 
