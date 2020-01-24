#!/usr/bin/python2.6
# -*- coding: utf-8 -*-

import time
import re
from bs4 import BeautifulSoup
import random
import requests
import os

base = 'http://guancha.gmw.cn'
output = './Output/GuangMing'
count = 0
cache = 0
vaild = 0
new = 0

blacklist = {u"责编", u"（转载请注明来源“光明网”，作者“光明网评论员”）"}

agents = [
    "Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0",
    "Mozilla/5.0 (X11; Gentoo; rv:67.0) Gecko/20100101 Firefox/67.0",
    "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.13; rv:60.0) Gecko/20100101 Firefox/60.0",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/76.0.3809.80 Safari/537.36",
    "Mozilla/5.0 (Windows NT 6.2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/76.0.3809.98 Safari/537.36",
    "Mozilla/5.0 (Windows NT 6.3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3833.114 Safari/537.36",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/76.0.3809.110 Safari/537.36",
    "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.12; rv:68.0) Gecko/20100101 Firefox/68.0"
]


def getHtml(address):
    time.sleep(0.05)
    header = {"User-Agent": random.choice(agents)}
    return requests.get(address, headers=header).content.decode("UTF-8")


def dfsWrite(out, blk):
    if blk.string == None or blk.string.strip() == "":
        if hasattr(blk, "children"):
            for sub in blk.children:
                if sub == '\n':
                    out.write('\n')
                elif not dfsWrite(out, sub):
                    return False
        return True
    else:
        text = blk.string
        st = text.strip()
        if st in blacklist:
            return False
        else:
            if text.find(u'责编') == -1 and text.find(u'责任编辑') == -1:
                out.write(text)
            return True


def getContent(name, url):
    global count
    global vaild
    count = count + 1
    filename = output+name
    if os.path.exists(filename):
        print("use cache {}".format(name))
        global cache
        cache = cache + 1
        vaild = vaild+1
        return
    text = getHtml(url)
    soup = BeautifulSoup(text, "lxml")
    blks = soup.select_one('div[class="u-mainText"]')
    if blks == None:
        blks = soup.select_one('div[id="contentMain"]')
    if blks == None:
        print("invalid article {}".format(url))
        return
    out = open(filename, "w", encoding='utf-8')
    out.write(soup.find("h1").string)

    for blk in blks.select("p"):
        notice = blk.select_one('strong')
        if notice != None and notice.string != None and notice.string.find(u'阅读提示') != -1:
            continue
        if not dfsWrite(out, blk):
            break
    print("{} -> {}".format(url, name))
    vaild = vaild+1
    global new
    new = new+1


def getContentUrl(url):
    if url.find('http') == -1:
        url = base+"/"+url
    pos1 = url.rfind('/')
    pos2 = url.rfind(".")
    getContent(url[pos1:pos2]+".txt", url)
    print('------')


def searchIndex(address, id):
    print('-------')
    text = getHtml(address)

    soup = BeautifulSoup(text, "lxml")
    if id == 11273:
        blks = soup.select_one('div[class="content_left"]')

        for art in blks.select('a'):
            url = art['href']
            if url.find('content_') != -1:
                if url.find("guancha.gmw.cn") == -1:
                    # 防止跳出
                    continue
                print("Title:", art.string)
                getContentUrl(url)

    else:
        blks = soup.select_one('ul[class="channel-newsGroup"]')

        urls = blks.select('li')
        for url in urls:
            tmp = url.select_one('a')
            title = tmp.string
            t = url.select_one('span[class="channel-newsTime"]').string
            name = title + " "+t
            print(name)
            getContentUrl(tmp['href'])

    nxt = None
    for blk in soup.select("a"):
        if blk.string == u'下一页':
            pos = address.rfind("/")
            nxt = address[:pos+1] + blk['href']
            break
    return nxt


def searchIter(id):
    address = base+"/node_"+str(id)+".htm"
    print("begin with {}".format(address))
    while address != None:
        address = searchIndex(address, id)


def searchMain():
    global count
    count = 0
    global cache
    cache = 0
    global vaild
    vaild = 0
    global new
    new = 0
    searchIter(11273)
    searchIter(26275)
    searchIter(87838)

    print("count {} cache {} vaild {} new {}".format(count, cache, vaild, new))
    return new


if __name__ == '__main__':
    searchMain()
