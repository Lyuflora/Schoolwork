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
import os
from urllib.request import quote, unquote




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
def getmovietitle(page):
    dataList = []
    path = os.getcwd()

    new_path='../static/base/cover/'

    print("newpath:%s page:%s" % (new_path, page))


    for i in range(0, page):
        html = getHtml(i).text
        soup = BeautifulSoup(html, 'html.parser')
        # 先找到父元素
        parent = soup.find('div', attrs={'id': 'content'})  # 一定要先找到id的位置,通过id的位置找到父元素
        # print(type(parentDiv))    #<class 'bs4.element.Tag'>
        lis = parent.find_all('li')
        index = 1
        for each in lis:
            data = []

            # 获取书名title 封面链接link
            if isinstance(each, bs4.element.Tag):
                film_name = each.find('div', attrs={'class': 'info'}).find('a').get('title')  # 方法三
                data.append(film_name)
                print(film_name)
                cover_link = each.find('img',)
                # print(cover_link)

            # 从第一张开始计数
            image_counter = (i) * 20 + index
            file_name = '%s.jpg' % (image_counter-1)
            img_url = cover_link['src']
            img_local = new_path + file_name
            data.append(new_path+file_name)   # 源地址：img_url  爬取保存到本地：img_local
            if len(img_url) > 0:
                urllib.request.urlretrieve(img_url, img_local)
                index = index+1
            print('下载图片%s完成' % image_counter)

            img_rel = '/' + file_name   # 备用相对地址
            data.append(img_rel)

            # 获取出版时间year
            reg1 = re.compile('.*(\\d{4}).*')  # 通过正则只获取中间的4个数字
            film_time_str = each.find('div', attrs={'class': 'pub'}).get_text()

             # 获取第一个匹配的正则数字, 因为有个别没有年份，所以加判断
            if len(re.findall(reg1, film_time_str)): # 判断是否有年份
                film_time = re.findall(reg1, film_time_str)[0]
            else:
                film_time = ''
            data.append(film_time)

            # 获取评分score
            film_score = each.find('div', attrs={'class': 'star clearfix'}).find_all('span')[1].get_text()
            data.append(film_score)

            # 获取评分人数reviewers
            reg2 = re.compile('[0-9\*\/\-]+')  # 匹配任意个数字
            discuss_num_str = each.find('div', attrs={'class': 'star clearfix'}).find_all('span')[2].get_text()
            discuss_num = re.findall(reg2, discuss_num_str)[0]  # 获取第一个匹配的正则数字
            data.append(discuss_num)

            # 获取短评review, 因为有个别没有简评标签，所以加判断
            if each.find('p'):  # 判断是否有p对象
                filmfilm_revieweview = each.find('p').get_text()
                    # .find('span').get_text()  # 如果p标签里只有一个span就不需要指定span的attrs
            else:
                filmfilm_revieweview = ''
            data.append(filmfilm_revieweview)
            dataList.append(data)
            # print(data)

    return dataList


def return_moviedict(page):

    moviedict=[]
    onemovie = {}
    datalist = getmovietitle(page)

    for j, each in enumerate(datalist):
        onemovie = {
            "title": datalist[j][0],
            "link": datalist[j][1],
            "relink": datalist[j][2],
            "year": datalist[j][3],
            "score": datalist[j][4],
            "reviewers": datalist[j][5],
            "review": datalist[j][6]
        }
        moviedict.append(onemovie)
        print(datalist[j][1])
        # print(datalist[j])
        # print(onemovie['title'])
    return moviedict


print("done")
getpage = 2
stream_entries3 = return_moviedict(getpage)   # 参数是页数
print(stream_entries3)
bookdict = stream_entries3


dom = minidom.getDOMImplementation().createDocument(None, 'booklist', None)
root = dom.documentElement

# len()

for k in range(0, getpage):
    onepage = dom.createElement('page')
    onepage.setAttribute('pagenum', str(k))
    for i in range(19):
        element = dom.createElement('li')

        booktitle = bookdict[i]['title']
        coverlink = bookdict[i]['link']
        coverrelink = bookdict[i]['relink']
        bookyear = bookdict[i]['year']
        bookscore = bookdict[i]['score']
        bookreview = bookdict[i]['review']
        bookreviewers = bookdict[i]['reviewers']

        rtxt = dom.createElement('review')
        rtxt.appendChild(dom.createTextNode(bookreview))
        element.appendChild(rtxt)
        element.setAttribute('bookid', str(i+k*20))
        element.setAttribute('title', booktitle)
        element.setAttribute('link', coverlink)
        element.setAttribute('relink', coverrelink)
        element.setAttribute('year', str(bookyear))
        element.setAttribute('score', bookscore)
        element.setAttribute('reviewers', bookreviewers)

        onepage.appendChild(element)

    root.appendChild(onepage)


# 保存文件xml
with open('wxml.xml', 'w', encoding='utf-8') as f:
    dom.writexml(f, addindent='\t', newl='\n', encoding='utf-8')


print("done")


