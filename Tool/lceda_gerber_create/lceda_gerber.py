# Warn = """\033[0;31;40m
# =======警告=======
# 此脚本仅包含有限的“自动重命名”功能，它：
# - 不会对Gerber文件进行解析
# - 不会验证文件格式是否正确
# - 不会处理钻孔文件
# - 极有可能损坏你的文件或引起生产错误
# - 需要你根据实际情况进行修改
# 如果你不知道自己在干什么，请手动修改文件而不是使用脚本
# 请编辑此脚本以解除警告
# \033[0m"""
# print(Warn)
# raise Exception("READ THE WARNING")

import os
import yaml
import datetime

def getLegalFileName(filPathList, rule):
    '''
    获取符合指定后缀名的文件 返回路劲列表
    '''
    # 定义一个后缀集合
    suffixSet = set()
    for name, value in rule.items():
        suffixs = value.split("|")
        for suffix in suffixs:
            suffixSet.add(suffix)
    
    legalList = []
    illegalList = []
    for filePath in filPathList:
        name, ext = os.path.splitext(filePath)
        if ext in suffixSet:
            legalList.append(filePath)
        else:
            illegalList.append(filePath)
    
    return legalList, illegalList

def addHeaderToFile(srcFilePath, dstFilePath, header):
    '''
    copy文件并添加文件头
    '''
    with open(srcFilePath, "r") as file:
        fileData = file.read()
        
    with open(dstFilePath, "w") as file:
        file.write(header)
        file.write(fileData)


if __name__ == "__main__":
    try:
        nowTime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        # 解析多个任务
        header = open("header", "r", encoding="utf-8").read().replace("%s", nowTime)
        taskFile = open("task.yaml", "r", encoding="utf-8")
        tastInfoList = list(yaml.load_all(taskFile, Loader=yaml.SafeLoader))
        taskFile.close()

        BaseOutPath = ".\\SnailHeater_Gerber_" + datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
        os.mkdir(BaseOutPath)
        for taskInfo in tastInfoList:
            GerberFileDir = taskInfo["GerberFileDir"]
            GerberOutDir = taskInfo["GerberOutDir"]
            print("\nTaskInfo ---> \n", GerberFileDir, "\n", taskInfo)

            # 创建新 Gerber 的目录
            dstDir = GerberOutDir
            if not dstDir:
                dirName = os.path.basename(GerberFileDir)
                dstDir = os.path.join(BaseOutPath, dirName)
            if not os.path.exists(dstDir):
                os.mkdir(dstDir)

            #创建PCB下单必读文档
            with open(os.path.join(dstDir, taskInfo["TextFileName"]), "w") as textFile:
                textFile.write(taskInfo["TextFileContent"])

            SrcDirFiles = os.listdir(GerberFileDir)
            print(SrcDirFiles)
            legalList, illegalList = getLegalFileName(SrcDirFiles, taskInfo["SUFFIX"])

            dstPathList = []
            for filePath in legalList:
                dstPathList.append(os.path.join(dstDir, os.path.basename(filePath)))

            print("\n\n", dstPathList)
            for srcPath, dstPath in zip(legalList, dstPathList):
                addHeaderToFile(os.path.join(GerberFileDir, srcPath), dstPath, header)

            # 原封不动拷贝没有命中规则的文件
            dstPathList = []
            for filePath in illegalList:
                dstPathList.append(os.path.join(dstDir, os.path.basename(filePath)))
            for srcPath, dstPath in zip(illegalList, dstPathList):
                pass
                os.system('copy "%s" "%s"'%(os.path.join(GerberFileDir, srcPath), dstPath))
    except Exception as err:
        print(err)

    print("\n\n生成的文件在本目录下的 【%s】 下\n" % BaseOutPath)
    input("按回车以关闭本软件。")
    exit()

    