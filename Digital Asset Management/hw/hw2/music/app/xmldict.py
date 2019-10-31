# 从xml中读出字典
import xmltodict


# xml = '''<?xml version="1.0" encoding="utf-8"?>
# <booklist>
#     <page page_num="1">
#         <li bookid="0" link="D:\pycharm workspace\music\app\cover/1.jpg" reviewers="396980" score="9.1" title="白夜行" year="2008">
# 			<review>“只希望能手牵手在太阳下散步”，这个象征故事内核的绝望念想，有如一个美丽的幌子，随着无数凌乱、压抑、悲凉的故事片段像纪录片一样一一还原：没有痴痴相思，没有海... </review>
# 		</li>
# 		<li bookid="1" link="D:\pycharm workspace\music\app\cover/2.jpg" reviewers="330200" score="8.9" title="嫌疑人X的献身" year="2008">
# 			<review>百年一遇的数学天才石神，每天唯一的乐趣，便是去固定的便当店买午餐，只为看一眼在便当店做事的邻居靖子。# 靖子与女儿相依为命，失手杀了前来纠缠的前夫。石神提出由... </review>
# 		</li>
#         <li bookid="2" title="嫌疑人X的献身" year="2008" reviewers="396491" img="/images/2.jpg">
#
#             <review>“只希望能手牵手在太阳下散步”，这个象征故事内核的绝望念想，有如一个美丽的幌子，随着无数凌乱、压抑、悲凉的故事片段像纪录片一样一一还原：没有痴痴相思，没有海... </review>
#
#         </li>
#     </page>
# </booklist>
# '''
#
# data = parse(xml)  # 解析xml为有序字典
def xml2html():
    with open('wxml.xml', 'r', encoding='UTF-8') as fd:
        data = xmltodict.parse(fd.read())
        print(data)
        box = data['booklist'] # 获取最外层的标签
        print(box)
        page = box.get('page', [])  # 获取次外层的标签
        print(page)

        spider_bookdict = []

        for page_index in range(0,len(page)):
            books = page[page_index].get('li', [])  # 多个标签名相同时，获取到的是标签列表
            spider_book = {}
            for onebook in books:
                print(onebook.get('@bookid'))
                print(onebook.get('@title'))   # 获取属性，使用'@'前缀
                print(onebook.get('@score'))
                print(onebook.get('@year'))
                print(onebook.get('@reviewers'))

                review = onebook['review']
                print(review)  # print(rev.get('#text'))    # 获取标签文本，使用'#text'

                spider_book={
                    "bookid": onebook.get('@bookid'),
                    "title": onebook.get('@title'),
                    "year": onebook.get('@year'),
                    "score": onebook.get('@score'),
                    "reviewers": onebook.get('@reviewers'),
                    "cover": onebook.get('@link'),
                    "review": review,
                }
                spider_bookdict.append(spider_book)
                print(spider_bookdict)
    return spider_bookdict


xml2html()