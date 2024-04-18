lvgl_image_converter

![GitHub top language](https://img.shields.io/github/languages/top/W-Mai/lvgl_image_converter?style=for-the-badge)

## 安装依赖

```
pip install -r requirements.txt
```
执行前需要关闭VPN。

## 更新

内容参考了peng-zhihui在[HoloCubic](https://github.com/peng-zhihui/HoloCubic)中对原作者工程的修改。修改后实现功能：

* 输出格式可配置
* 可选输出.c文件和.bin文件

## 使用方法

### 参数修改

config文件中进行必要配置，相关参数值如下：

```python
color_format_dir = {
    "CF_TRUE_COLOR_332"             :Converter.FLAG.CF_TRUE_COLOR_332,  # Helper formats. Used internally
    "CF_TRUE_COLOR_565"             :Converter.FLAG.CF_TRUE_COLOR_565,
    "CF_TRUE_COLOR_565_SWAP"        :Converter.FLAG.CF_TRUE_COLOR_565_SWAP,
    "CF_TRUE_COLOR_888"             :Converter.FLAG.CF_TRUE_COLOR_888,
    "CF_ALPHA_1_BIT"                :Converter.FLAG.CF_ALPHA_1_BIT,
    "CF_ALPHA_2_BIT"                :Converter.FLAG.CF_ALPHA_2_BIT,
    "CF_ALPHA_4_BIT"                :Converter.FLAG.CF_ALPHA_4_BIT,
    "CF_ALPHA_8_BIT"                :Converter.FLAG.CF_ALPHA_8_BIT,
    "CF_INDEXED_1_BIT"              :Converter.FLAG.CF_INDEXED_1_BIT,
    "CF_INDEXED_2_BIT"              :Converter.FLAG.CF_INDEXED_2_BIT,
    "CF_INDEXED_4_BIT"              :Converter.FLAG.CF_INDEXED_4_BIT,
    "CF_INDEXED_8_BIT"              :Converter.FLAG.CF_INDEXED_8_BIT,
    "CF_RAW"                        :Converter.FLAG.CF_RAW,
    "CF_RAW_ALPHA"                  :Converter.FLAG.CF_RAW_ALPHA,
    "CF_RAW_CHROMA"                 :Converter.FLAG.CF_RAW_CHROMA,
    "CF_TRUE_COLOR"                 :Converter.FLAG.CF_TRUE_COLOR,  # Helper formats is C arrays contains all true color formats (using in "download")
    "CF_TRUE_COLOR_ALPHA"           :Converter.FLAG.CF_TRUE_COLOR_ALPHA,
    "CF_TRUE_COLOR_CHROMA"          :Converter.FLAG.CF_TRUE_COLOR_CHROMA,
}

cfg_en = {
    "yes"   : 1,
    "no"    : 0,
}
```

### 文件输出

* 根据需要修改配置文件
* 将需要转换的文件放入文件夹`images`中，文件夹可配置
* 执行main.py或`lvgl_image_converter.exe`

## 打包可执行文件

```
pip instsall pyinstaller
```

```
pyinstaller -F -n lvgl_image_converter.exe main.py
```
