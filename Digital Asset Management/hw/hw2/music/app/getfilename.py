import os
import glob

path = "D:/pycharm workspace/music/static/resource/songs/soundtrack/"
for filename in os.listdir(path):
    print(os.path.join(path, filename))

print(glob.glob(r'../static/resource/songs/sountrack/*.mp3'))