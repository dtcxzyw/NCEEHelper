#!/usr/bin/python2.6
# -*- coding: utf-8 -*-

import time
import re
from bs4 import BeautifulSoup
import random
import requests
import os

base = 'http://rd.com'
output = './Output/ReaderDigest/'
count = 0
cache = 0
vaild = 0
new = 0

blacklist = {}

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
    if hasattr(blk, "name"):
        if blk.name == 'script':
            return True
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
            if text.strip() != "View as slideshow":
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
    blks = soup.select_one('div[class="entry-content"]')
    if blks == None:
        print("invalid article {}".format(url))
        return
    out = open(filename, "w", encoding='utf-8')

    dfsWrite(out, blks)

    print("{} -> {}".format(url, name))
    vaild = vaild+1
    global new
    new = new+1


def getContentUrl(url):
    pos = len("https://www.rd.com/true-stories/")
    name = url[pos:-1]
    if name.find("page/") == 0:
        return
    getContent(name.replace("/", "-")+".txt", url)
    print('------')


def searchIndex(address):
    print('-------')
    text = getHtml(address)

    soup = BeautifulSoup(text, "lxml")

    blks = soup.select_one('div[class="site-inner"]')

    for blk in blks.select('a'):
        url = blk['href']
        if url.find("www.rd.com/true-stories/") != -1 and url.count("/") >= 6:
            getContentUrl(url)

    nxt = soup.select_one('a[class="next page-numbers"]')
    if nxt != None:
        nxt = nxt['href']
    return nxt


def searchMain():
    global count
    count = 0
    global cache
    cache = 0
    global vaild
    vaild = 0
    global new
    new = 0

    url = base+"/true-stories/"
    while url != None:
        url = searchIndex(url)

    print("count {} cache {} vaild {} new {}".format(count, cache, vaild, new))
    return new


if __name__ == '__main__':
    searchMain()
