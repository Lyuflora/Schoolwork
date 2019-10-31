import os
import sys
#当前文件的路径
pwd = os.getcwd()
#当前文件的父路径
father_path=os.path.abspath(os.path.dirname(pwd)+os.path.sep+".")
#当前文件的前两级目录
grader_father=os.path.abspath(os.path.dirname(pwd)+os.path.sep+"..")
local = grader_father + "/music/static/base/lcmusic"
print(local)
file_dir = r"D:/pycharm workspace/music/static/base/lcmusic"

file_dir = local
i=0
lccd = []
b = None
for root, dirs, files in os.walk(file_dir):
    i+=1
    print(files) #当前路径下所有非目录子文件
    print(i)
    lccd = files
print(lccd)