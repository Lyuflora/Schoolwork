# -*- coding:utf-8 -*-
from flask import Flask, render_template
import requests
import bs4
from bs4 import BeautifulSoup
import chardet
import re
import xml.dom.minidom as minidom
import urllib.request
from bs4 import BeautifulSoup
import shutil, os
import xmltodict
import sys

# import app.xmldict

app = Flask(__name__,template_folder='../templates',static_folder='../static')


def readfile1(music_dir, image_dir, txt_dir, music, image, txt_title, txt_lyrics):
    i = 1
    num = len(os.listdir(music_dir))
    print(music_dir)

    music_dir = "." + music_dir
    image_dir = "." + image_dir
    while i<num+1:
        music_dir = music_dir+('%d'% (i + 200)).zfill(3) + ".mp3"
        music.append(music_dir)
        # print(music_dir)
        music_dir = music_dir[:-7]

        image_dir = image_dir + ('%d' % (i + 200)).zfill(3) + ".png"
        image.append(image_dir)
        image_dir = image_dir[:-7]

        txt_dir = txt_dir + ('%d' % (i + 200)).zfill(3) + ".txt"
        f = open(txt_dir, encoding='gb18030', errors='ignore')
        txt_title.append(f.readline())
        txt_lyrics.append(f.readline()+f.readline())
        txt_dir = txt_dir[:-7]

        i = i+1

    return num


def readfile2(music_dir, image_dir, txt_dir, music, image, txt_title, txt_lyrics):
    i = 1
    num = len(os.listdir(music_dir))
    #    print('music_dir:%d' % num)

    music_dir = "." + music_dir
    image_dir = "." + image_dir
    while i<num+1:

        music_dir = music_dir + ('%d' % i).zfill(3) + ".mp3"
        music.append(music_dir)
        print(music_dir)
        music_dir = music_dir[:-7]

        image_dir = image_dir + ('%d' % i).zfill(3) + ".png"
        image.append(image_dir)
        image_dir = image_dir[:-7]

        txt_dir = txt_dir + ('%d' % i).zfill(3) + ".txt"
        f = open(txt_dir, encoding='gb18030', errors='ignore')
        txt_title.append(f.readline())
        txt_lyrics.append(f.readline() + f.readline())
        txt_dir = txt_dir[:-7]

        i = i+1

    return num


def get_soundtrack():
    music1_dir = "./static/resource/songs/soundtrack/"
    image1_dir = "./static/resource/images/soundtrack/"
    txt1_dir = "./static/resource/txt/soundtrack/"

    music1 = []
    image1 = []
    txt1_title = []
    txt1_lyrics = []

    num_soundtrack = readfile1(music1_dir, image1_dir, txt1_dir, music1, image1, txt1_title, txt1_lyrics)
    dictionary = {
        "num1": num_soundtrack,
        "music1": music1,
        "image1": image1,
        "txt1_title": txt1_title,
        "txt1_lyrics": txt1_lyrics
    }

    return dictionary


def get_others():
    music1_dir = "./static/resource/songs/others/"
    image1_dir = "./static/resource/images/others/"
    txt1_dir = "./static/resource/txt/others/"

    music1 = []
    image1 = []
    txt1_title = []
    txt1_lyrics = []

    num_soundtrack = readfile2(music1_dir, image1_dir, txt1_dir, music1, image1, txt1_title, txt1_lyrics)
    dictionary = {
        "num1": num_soundtrack,
        "music1": music1,
        "image1": image1,
        "txt1_title": txt1_title,
        "txt1_lyrics": txt1_lyrics
    }

    return dictionary


def return_dictionary(d):
    #    d = get_soundtrack()

    musiclist = d['music1']
    imagelist = d['image1']
    txtitlelist = d['txt1_title']
    txlyricslist = d['txt1_lyrics']
    num = len(txtitlelist)
    print('num:%d' % num)

    cd = []
    i = 0
    onesong = {}
    while i<num:

        onesong = {'id': i, 'musicscr': musiclist[i], 'image': imagelist[i], 'title': txtitlelist[i], 'lyrics': txlyricslist[i]}
        cd.append(onesong)
        print(onesong['image'])
        i = i + 1

    print('CD size:%d' % len(cd))
    return cd

def getHtml(index=0):
    print('正在抓取第{}页信息'.format(index+1))
    url = 'https://movie.douban.com/top250?start='+str(index*25)+'&filter='
    url = 'https://book.douban.com/tag/%E6%8E%A8%E7%90%86?start='+str(index*20)+'&filter='

    r = requests.get(url, headers={'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.120 Safari/537.36'})
    # code = chardet.detect(r.content)['encoding']
    # print(code)   #utf-8
    # return r.content.decode(code)
    return r


# 返回datalist列表，每部电影是一个字典


# 从xml中读取书本的信息
def xml2html():
    with open("wxml.xml", encoding='UTF-8') as fd:
        data = xmltodict.parse(fd.read())
        print(data)
        box = data['booklist']  # 获取最外层的标签
        print(box)
        page = box.get('page', [])  # 获取次外层的标签
        print(page)

        spider_bookdict = []

        for page_index in range(0, len(page)):
            books = page[page_index].get('li', [])  # 多个标签名相同时，获取到的是标签列表
            spider_book = {}
            for onebook in books:
                print(onebook.get('@bookid'))
                print(onebook.get('@link'))   # 获取属性，使用'@'前缀
                # print(onebook.get('@score'))
                # print(onebook.get('@year'))
                # print(onebook.get('@reviewers'))
                print("%s:%s"%(onebook.get('@title'), onebook.get('@link')))
                review = onebook['review']
                # print(review)  # print(rev.get('#text'))    # 获取标签文本，使用'#text'

                spider_book = {
                    "bookid": onebook.get('@bookid'),
                    "title": onebook.get('@title'),
                    "year": onebook.get('@year'),
                    "score": onebook.get('@score'),
                    "reviewers": onebook.get('@reviewers'),
                    "cover": "../static/base/cover/"+"%s"%(onebook.get('@bookid'))+".jpg",
                    "cover_": onebook.get('@relink'),
                    "review": review,
                }
                spider_bookdict.append(spider_book)
                print(spider_book["cover"])
    return spider_bookdict


def putlcmusic():
    # 当前文件的路径
    pwd = os.getcwd()
    # 当前文件的父路径
    father_path = os.path.abspath(os.path.dirname(pwd) + os.path.sep + ".")
    # 当前文件的前两级目录
    grader_father = os.path.abspath(os.path.dirname(pwd) + os.path.sep + "..")
    print(father_path)
    local = father_path + "/music/static/base/lcmusic"
    print(local)
    # file_dir = r"D:/pycharm workspace/music/static/base/lcmusic"

    file_dir = local
    i = 0
    lccd = []
    b = None
    for root, dirs, files in os.walk(file_dir):
        i += 1
        print(files)  # 当前路径下所有非目录子文件
        print(i)


    print(files) # 歌名列表
    files = [str(i) for i in files]
    for i in range(len(files)):
        print ("序号：%s   值：%s" % (i + 1, files[i]))
        files[i] = files[i].replace('\xa0', ' ')
        src = local+"/"+files[i]
        s = src
        # files[i] = "".join(files[i].split())
        lc = {
            "lctitle": files[i][:-4],
            "lcsrc": "../static/base/lcmusic/"+files[i],
        }
        print("lc",lc)
        lccd.append(lc)

    # shutil.copytree(os.getcwd()+"spider_music/data/", "../static/base/lcmusic/")
    # 复制data文件夹下所有的内容到../static/base/lcmusic/中

    return lccd







@app.route('/')
def mainpage():
    putlcmusic()
    return render_template('entrance.html')


@app.route('/Music')
def index():
    general_data = {'title': 'Music Player'}
    stream_entries1 = return_dictionary(get_soundtrack())
    stream_entries2 = return_dictionary(get_others())
#    for e in stream_entries1:
#       print(e)

    return render_template('index.html', entries1=stream_entries1, entries2=stream_entries2, **general_data)
    # stream_entries3 = return_moviedict(1)

    # return render_template('play.html', entries3=stream_entries3)


@app.route('/Movie')
def douban():
    dbbook = xml2html()
    print(dbbook)
    return render_template('book2.html', entries3=dbbook)


@app.route('/Radio')
def kanlix():
    lclist = putlcmusic()
    print(lclist)
    return render_template('radio.html', entries4=lclist)



if __name__ == '__main__':
    app.run()
