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


def getContent(name, url):
    time.sleep(0.05)
    global count
    count = count + 1
    filename = output+name
    if os.path.exists(filename):
        print("use cache {}".format(name))
        global cache
        cache = cache + 1
        return
    text = requests.get(
        url).content.decode("gbk")
    soup = BeautifulSoup(text, "lxml")
    blk = soup.select_one('div[class="box_con"]')
    out = open(filename, "w", encoding='utf-8')
    print("{} -> {}".format(url, name))
    out.write(soup.find("h1").string)
    for par in blk.select("p"):
        text = par.string
        if text == None:
            text = ""
            for npar in par.children:
                if npar.string != None:
                    text = text+npar.string
        if text.strip() == u"相关评论":
            break
        out.write(text)


def searchIndex(address):
    print('-------')
    text = requests.get(address).content.decode("gbk")

    soup = BeautifulSoup(text, "lxml")
    blks = soup.select('ul[class="list_14 clearfix"]')

    for blk in blks:
        urls = blk.select('li')
        for url in urls:
            tmp = url.select_one('a')
            title = tmp.string
            time = url.select_one('i').string
            name = title + " "+time
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


searchIter(159301)
searchIter(1034)
searchIter(51863)
searchIter(364183)
searchIter(51854)

print("count {} cache {}".format(count, cache))
