#  希尔伯特曲线绘制代码来源 https://github.com/tujinhao/GS_Hbed_Generator

# 版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。
# 本文链接：https://blog.csdn.net/ranjuese/article/details/115552972
# 铜导线电阻计算 https://murata.eetrend.com/article/2019-04/1002732.html


import matplotlib.pyplot as plt
import json

# 立创EDA有关于坐标都是 x*10mil
# 画布原点(4020,3343)  
# pcb坐标系原点在左上角，X轴从左到右递增，y轴从上到下递增
pcb_x0 = 4020
pcb_y0 = 3343

# 曲线
strokeWidthReal = 5 # 线宽 mil
strokethickness = 1 # 铜箔厚度 1oz
clearanceReal = 20 # 线中心间距 mil

# lceda格式的线宽为strokeWidthReal / 10 mil
lcedaStrokeWidth = strokeWidthReal / 10  
# lceda格式的线中心间距为 clearanceReal / 10 mil  
# 线边缘的间距为（clearance - strokeWidth）*10mil
lcedaClearance = clearanceReal / 10    
order = 3   #曲线阶数

size = ((2**order-1) * clearanceReal + strokeWidthReal)  # 边长 /mil
print( str(size) + " mil")
print( str(size * 0.0254) + " mm")

if(size > 394 ):
    print( "曲线边长大于10cm")
# 曲线起始坐标
# 曲线从PCB的左下角开始绘制
# 100mm = 3940 mil



#曲线居中
offsetx = pcb_x0 + 197 - size/2  
offsety = pcb_y0 - size/2 - 197


def _hilbert(direction, rotation, order):
    if order == 0:
        return

    direction += rotation
    _hilbert(direction, -rotation, order - 1)
    step1(direction)

    direction -= rotation
    _hilbert(direction, rotation, order - 1)
    step1(direction)
    _hilbert(direction, rotation, order - 1)

    direction -= rotation
    step1(direction)
    _hilbert(direction, -rotation, order - 1)


def step1(direction):
    next = {0: (1, 0), 1: (0, 1), 2: (-1, 0),
            3: (0, -1)}[direction & 0x3]  # 取后两位？

    global x, y
    x.append(x[-1] + next[0])
    y.append(y[-1] + next[1])


def hilbert(order):
    global x, y
    x = [0, ]
    y = [0, ]
    _hilbert(0, 1, order)
    return (x, y)

# 把生成的希尔伯特曲线坐标写成立创EDA布线的形式
def PCB_hilbert():
    f = open('./TRACK.txt', 'w', encoding='utf-8')

    print("TRACK~" + str(lcedaStrokeWidth) + "~1~BED~", end='', file=f)

    for i in range(0, len(x) - 1):
        # print >> f, str(i)+'\n'
        print(str(x[i]*lcedaClearance + offsetx) + ' ' +
              str(y[i]*lcedaClearance + offsety) + ' ', end='', file=f)
    print(str(x[len(x) - 1]*lcedaClearance + offsetx) + ' ' + str(y[len(x) - 1]
          * lcedaClearance + offsety) + "~gee123~0", end='', file=f)

    f.close()

#创建一个json文件，可以直接用立创EDA打开
def creat_json():

    f = open('./TRACK.txt', 'r', encoding='utf-8')
    TRACK_Hilbertf = f.read()
    data = {
        "head": {
            "docType": "3",
            "editorVersion": "6.4.25",
            "newgId": True,
            "c_para": {},
            "x": 4020,
            "y": 3343,
            "hasIdFlag": True,
            "importFlag": 0,
            "transformList": ""
        },
        "canvas": "CA~1000~1000~#000000~yes~#FFFFFF~10~1000~1000~line~0.5~mm~1~45~~0.5~4020~3343~0~yes",
        "shape": [
            "TRACK~1~10~~4020 3333 4020 2959.2992~gge11~0",
            "TRACK~1~10~~4403.7008 3343 4030 3343~gge9~0",
            "TRACK~1~10~~4413.7008 2959.2992 4413.7008 3333~gge7~0",
            "TRACK~1~10~~4030 2949.2992 4403.7008 2949.2992~gge5~0",
            "ARC~1~10~~M4403.7008 2949.2992A10 10 0 0 1 4413.7008 2959.2992~~gge6~0",
            "ARC~1~10~~M4413.7008 3333A10 10 0 0 1 4403.7008 3343~~gge8~0",
            "ARC~1~10~~M4030 3343A10 10 0 0 1 4020 3333~~gge10~0",
            "ARC~1~10~~M4020 2959.2992A10 10 0 0 1 4030 2949.2992~~gge12~0",
            TRACK_Hilbertf
        ],

        "layers": [
            "1~TopLayer~#FF0000~true~true~true~",
            "2~BottomLayer~#0000FF~true~false~true~",
            "3~TopSilkLayer~#FFCC00~true~false~true~",
            "4~BottomSilkLayer~#66CC33~true~false~true~",
            "5~TopPasteMaskLayer~#808080~true~false~true~",
            "6~BottomPasteMaskLayer~#800000~true~false~true~",
            "7~TopSolderMaskLayer~#800080~true~false~true~0.3",
            "8~BottomSolderMaskLayer~#AA00FF~true~false~true~0.3",
            "9~Ratlines~#6464FF~false~false~true~",
            "10~BoardOutLine~#FF00FF~true~false~true~",
            "11~Multi-Layer~#C0C0C0~true~false~true~",
            "12~Document~#FFFFFF~true~false~true~",
            "13~TopAssembly~#33CC99~false~false~false~",
            "14~BottomAssembly~#5555FF~false~false~false~",
            "15~Mechanical~#F022F0~false~false~false~",
            "19~3DModel~#66CCFF~false~false~false~",
            "21~Inner1~#999966~false~false~false~~",
            "22~Inner2~#008000~false~false~false~~",
            "23~Inner3~#00FF00~false~false~false~~",
            "24~Inner4~#BC8E00~false~false~false~~",
            "25~Inner5~#70DBFA~false~false~false~~",
            "26~Inner6~#00CC66~false~false~false~~",
            "27~Inner7~#9966FF~false~false~false~~",
            "28~Inner8~#800080~false~false~false~~",
            "29~Inner9~#008080~false~false~false~~",
            "30~Inner10~#15935F~false~false~false~~",
            "31~Inner11~#000080~false~false~false~~",
            "32~Inner12~#00B400~false~false~false~~",
            "33~Inner13~#2E4756~false~false~false~~",
            "34~Inner14~#99842F~false~false~false~~",
            "35~Inner15~#FFFFAA~false~false~false~~",
            "36~Inner16~#99842F~false~false~false~~",
            "37~Inner17~#2E4756~false~false~false~~",
            "38~Inner18~#3535FF~false~false~false~~",
            "39~Inner19~#8000BC~false~false~false~~",
            "40~Inner20~#43AE5F~false~false~false~~",
            "41~Inner21~#C3ECCE~false~false~false~~",
            "42~Inner22~#728978~false~false~false~~",
            "43~Inner23~#39503F~false~false~false~~",
            "44~Inner24~#0C715D~false~false~false~~",
            "45~Inner25~#5A8A80~false~false~false~~",
            "46~Inner26~#2B937E~false~false~false~~",
            "47~Inner27~#23999D~false~false~false~~",
            "48~Inner28~#45B4E3~false~false~false~~",
            "49~Inner29~#215DA1~false~false~false~~",
            "50~Inner30~#4564D7~false~false~false~~",
            "51~Inner31~#6969E9~false~false~false~~",
            "52~Inner32~#9069E9~false~false~false~~",
            "99~ComponentShapeLayer~#00CCCC~false~false~false~0.4",
            "100~LeadShapeLayer~#CC9999~false~false~false~",
            "101~ComponentMarkingLayer~#66FFCC~false~false~false~",
            "Hole~Hole~#222222~false~false~true~",
            "DRCError~DRCError~#FAD609~false~false~true~"
        ],
        "objects": [
            "All~true~false",
            "Component~true~true",
            "Prefix~true~true",
            "Name~true~false",
            "Track~true~true",
            "Pad~true~true",
            "Via~true~true",
            "Hole~true~true",
            "Copper_Area~true~true",
            "Circle~true~true",
            "Arc~true~true",
            "Solid_Region~true~true",
            "Text~true~true",
            "Image~true~true",
            "Rect~true~true",
            "Dimension~true~true",
            "Protractor~true~true"
        ],
        "BBox": {
            "x": 4020,
            "y": 2883.3,
            "width": 393.7,
            "height": 393.7
        },
        "preference": {
            "hideFootprints": "",
            "hideNets": ""
        },
        "DRCRULE": {
            "Default": {
                "trackWidth": 1,
                "lcedaClearance": 0.6,
                "viaHoleDiameter": 2.4,
                "viaHoleD": 1.2
            },
            "isRealtime": False,
            "isDrcOnRoutingOrPlaceVia": False,
            "checkObjectToCopperarea": True,
            "showDRCRangeLine": True
        },
        "netColors": []

    }
    json_data = json.dumps(data)
    f = open('./Bed_lceda.json', 'w')
    f.write(json_data)
    f.close()
    #print(json_data)


def main():
    x, y = hilbert(order)
    PCB_hilbert()
    creat_json()

    plt.plot(x, y)
    plt.show()




if __name__ == '__main__':

    main()
    # print(x)
    # print(y)
