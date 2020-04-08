#!/usr/bin/python3
# -*- coding: utf-8 -*-

import time
import re
from bs4 import BeautifulSoup
from faker import Faker
import requests
import os

output = './Output/ReaderDigest/'
count = 0
cache = 0
vaild = 0
new = 0

blacklist = {}

fake = Faker("zh_CN")


def getHtmlImpl(address):
    while True:
        time.sleep(0.05)
        header = {"User-Agent": fake.user_agent()}
        return requests.get(address, headers=header, allow_redirects=True, timeout=5).content.decode("UTF-8")


def getHtml(address):
    res = None
    for i in range(10):
        try:
            res = getHtmlImpl(address)
        except BaseException as e:
            print(address, e)
            time.sleep(1.0)
            continue
        else:
            return res
    print("Failed to get {}".format(address))
    return u""


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
    count += 1
    filename = output+name
    if os.path.exists(filename):
        print("use cache {}".format(name))
        global cache
        cache += 1
        vaild += 1
        return
    text = getHtml(url)
    soup = BeautifulSoup(text, "lxml")
    blks = soup.select_one('section[class="content-wrapper pure-g"]')
    if blks == None:
        print("invalid article {}".format(url))
        return
    out = open(filename, "w", encoding='utf-8')

    dfsWrite(out, blks)

    print("{} -> {}".format(url, name))
    vaild += 1
    global new
    new += 1
    print("New count {}".format(new))


def getContentUrl(url, baseLen):
    name = url[baseLen:-1]
    if name.find("page/") == 0:
        return
    getContent(name.replace("/", "-")+".txt", url)
    print('------')


def searchIndex(address, base):
    print('-------')
    text = getHtml(address)

    soup = BeautifulSoup(text, "lxml")

    for blks in soup.select('section[class="archive-content"]'):
        for blk in blks.select('a'):
            url = blk['href']
            if url.find(base) != -1 and (base.find("culture") != -1 or url.count("/") >= 6) and url != base:
                getContentUrl(url, len(base))

    nxt = soup.select_one('a[class="next page-numbers"]')
    if nxt != None:
        nxt = nxt['href']
    return nxt


def searchIter(url):
    base = url
    page = 0
    while url != None:
        page += 1
        old = new
        print("Page {}".format(page))
        url = searchIndex(url, base)
        if old == new:
            break


def searchMain():
    global count
    count = 0
    global cache
    cache = 0
    global vaild
    vaild = 0
    global new
    new = 0

    searchIter("https://www.rd.com/true-stories/")
    searchIter("https://www.thehealthy.com/food/")
    searchIter("https://www.thehealthy.com/exercise/")
    searchIter("https://www.thehealthy.com/aging/mind-memory/")
    searchIter("https://www.rd.com/advice/")
    searchIter("https://www.rd.com/culture/")

    print("count {} cache {} vaild {} new {}".format(count, cache, vaild, new))
    return new


if __name__ == '__main__':
    searchMain()
