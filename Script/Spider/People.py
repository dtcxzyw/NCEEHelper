#!/usr/bin/python2.6
# -*- coding: utf-8 -*-

import time
import re
from bs4 import BeautifulSoup
import requests
import os

base = 'http://opinion.people.com.cn'
output = './Output/People'
count = 0
cache = 0
vaild = 0

blacklist = {u"小蒋的话：", u"相关评论"}


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
    time.sleep(0.05)
    text = requests.get(
        url).content.decode("gbk")
    soup = BeautifulSoup(text, "lxml")
    blk = soup.select_one('div[class="box_con"]')
    if blk == None:
        return
    out = open(filename, "w", encoding='utf-8')
    out.write(soup.find("h1").string)

    dfsWrite(out, blk)
    print("{} -> {}".format(url, name))
    vaild = vaild+1


def searchIndex(address):
    print('-------')
    time.sleep(0.1)
    text = requests.get(address).content.decode("gbk")

    soup = BeautifulSoup(text, "lxml")
    blks = soup.select('ul[class="list_14 clearfix"]')

    for blk in blks:
        urls = blk.select('li')
        for url in urls:
            tmp = url.select_one('a')
            title = tmp.string
            t = url.select_one('i').string
            name = title + " "+t
            print(name)
            url = base+tmp['href']
            pos1 = url.rfind('/')
            pos2 = url.rfind(".")
            getContent(url[pos1:pos2]+".txt", url)
            print('------')

    nxt = None
    for blk in soup.select("a"):
        if blk.string == u'下一页':
            pos = address.rfind("/")
            nxt = address[:pos+1] + blk['href']
            break
    return nxt


def searchIter(id):
    address = base+"/GB/"+str(id)+"/index.html"
    print("begin with {}".format(address))
    while address != None:
        address = searchIndex(address)


def searchMain():
    global count
    count = 0
    global cache
    cache = 0
    global vaild
    vaild = 0
    searchIter(159301)
    searchIter(1034)
    searchIter(51863)
    searchIter(364183)
    searchIter(51854)

    print("count {} cache {} vaild {}".format(count, cache, vaild))


if __name__ == '__main__':
    searchMain()
