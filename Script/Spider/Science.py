#!/usr/bin/python3
# -*- coding: utf-8 -*-

import time
import re
from bs4 import BeautifulSoup
from faker import Faker
import requests
import os

base = "https://science.sciencemag.org"
output = './Output/Science/'
count = 0
cache = 0
vaild = 0
new = 0

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
    blk = soup.select_one('meta[name="DC.Description"]')
    if blk == None:
        blk = soup.select_one('div[class="article__body"]')
        if blk == None:
            print("invalid article {}".format(url))
            return
        out = open(filename, "w", encoding="utf-8")
        dfsWrite(out, blk)
    else:
        out = open(filename, "w", encoding='utf-8')
        out.write(blk['content'])

    print("{} -> {}".format(url, name))
    vaild += 1
    global new
    new += 1
    print("New count {}".format(new))


def getContentUrl(name):
    url = base+name
    name = name[9:].replace("/", "-")
    pos = name.find('.')
    getContent(name[:pos]+".txt", url)
    print('------')


def searchIndex(address):
    print('-------')
    text = getHtml(address)

    soup = BeautifulSoup(text, "lxml")

    blks = soup.select_one(
        'li[class="issue-toc-section issue-toc-section-contents"]')

    for blk in blks.select('a'):
        url = blk['href']
        if url.endswith(".full"):
            getContentUrl(url)


def searchYear(year):
    url = base+"/content/by/year/"+str(year)
    text = getHtml(url)

    soup = BeautifulSoup(text, "lxml")
    mths = soup.select_one('ul[class="issue-month-detail"]')

    for mth in mths.select('a[class="highwire-cite-linked-title"]'):
        name = mth.select_one('h3').string+" "+str(year) + \
            " "+mth.select_one('p').string
        print(name)
        searchIndex(base+mth['href'])


newsBase = "https://sciencemag.org"


def getNewsUrl(name):
    url = newsBase+name
    pos = name.rfind('/')
    name = name[pos+1:]
    getContent(name+".txt", url)
    print("------")


def searchLatestNews(url):
    print("-------")
    text = getHtml(url)
    soup = BeautifulSoup(text, "lxml")
    hls = soup.select_one('ul[class="headline-list"]')
    for headline in hls.select('h2[class="media__headline"]'):
        ref = headline.select_one('a')
        print(ref.string)
        getNewsUrl(ref['href'])
    nxt = soup.select_one('a[title="Go to next page"]')
    if nxt != None:
        return newsBase+nxt['href']
    return None


def searchLatestNewsIter():
    url = newsBase+"/news/latest-news"
    cnt = 1
    while url != None and cnt <= 60:
        print("Page {}".format(cnt))
        old = new
        url = searchLatestNews(url)
        cnt += 1
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

    # searchYear(2019)
    searchYear(2020)
    searchLatestNewsIter()

    print("count {} cache {} vaild {} new {}".format(count, cache, vaild, new))
    return new


if __name__ == '__main__':
    searchMain()
