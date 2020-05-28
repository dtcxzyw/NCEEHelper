#!/usr/bin/python3
# -*- coding: utf-8 -*-

import random
import time
import re
from bs4 import BeautifulSoup
import requests
import os

base = "http://www.news.cn"

output = './Output/XinHua'
count = 0
cache = 0
vaild = 0
new = 0

blacklist = {"+1"}

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
    time.sleep(1.0)
    header = {"User-Agent": random.choice(agents)}
    return requests.get(address, headers=header)


def dfsWrite(out, blk):
    if blk == None:
        return False
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
        vaild = vaild + 1
        return
    text = getHtml(url).content.decode("utf-8")
    soup = BeautifulSoup(text, "lxml")
    blk = soup.select_one('div[id="p-detail"]')
    if blk == None:
        blk = soup.select_one('div[id="content"]')
    if blk == None:
        print("Bad article")
        return
    out = open(filename, "w", encoding='utf-8')
    dfsWrite(out, soup.select_one('div[class="h-title"]'))
    dfsWrite(out, soup.select_one('h1'))
    dfsWrite(out, blk)
    print("{} -> {}".format(url, name))
    vaild = vaild+1
    global new
    new = new+1


def searchIndex(address, sel):
    text = getHtml(address).content.decode("utf-8")
    soup = BeautifulSoup(text, "lxml")
    blks = soup.select(sel)

    for blk in blks:
        urls = blk.select('li')
        for url in urls:
            tmp = url.select_one('a')
            title = tmp.string
            t = url.select_one('span').string
            name = title + " "+t
            print(name)
            url = tmp['href']
            pos1 = url.rfind('/')
            pos2 = url.rfind(".")
            getContent(url[pos1:pos2]+".txt", url)
            print('------')
        # 不要今日关注
        return


def searchMain():
    global count
    count = 0
    global cache
    cache = 0
    global vaild
    vaild = 0
    global new
    new = 0
    searchIndex(base+"/xhsd/index.htm", 'div[id="hideData3"]')
    searchIndex(base+"/comments/ycpl.htm", 'div[id="hideData"]')
    searchIndex(base+"/comments/zxpl.htm", 'div[id="hideData"]')

    print("count {} cache {} vaild {} new {}".format(count, cache, vaild, new))
    return new


if __name__ == '__main__':
    searchMain()
