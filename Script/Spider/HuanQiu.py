#!/usr/bin/python3
# -*- coding: utf-8 -*-

import random
import time
import re
from bs4 import BeautifulSoup
import requests
import os
import json

base = "https://opinion.huanqiu.com/"

output = './Output/HuanQiu/'
count = 0
cache = 0
vaild = 0
new = 0

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
    blk = soup.select_one('section[data-type="rtext"]')
    if blk == None:
        return
    out = open(filename, "w", encoding='utf-8')
    for p in blk.select("p"):
        out.write(p.text+"\n")
    print("{} -> {}".format(url, name))
    vaild = vaild+1
    global new
    new = new+1


def searchRecommend(address, offset):
    windowName = '"/e3pmub6h5/e3pmub75a","/e3pmub6h5/e3pn00if8","/e3pmub6h5/e3pn03vit","/e3pmub6h5/e3pn4bi4t","/e3pmub6h5/e3pr9baf6","/e3pmub6h5/e3prafm0g","/e3pmub6h5/e3prcgifj","/e3pmub6h5/e81curi71","/e3pmub6h5/e81cv14rf","/e3pmub6h5/e81cv14rf/e81cv52ha","/e3pmub6h5/e81cv14rf/e81cv7hp0","/e3pmub6h5/e81cv14rf/e81cvaa3q","/e3pmub6h5/e81cv14rf/e81cvcd7e"'
    off = str(offset*20)
    url = address+'api/list?node=' + windowName+"&offset="+off+"&limit=20"
    res = requests.get(url, headers={
        'X-Requested-With': 'XMLHttpRequest', "contentType": "application/json"})
    li = json.loads(res.text).get("list", list())
    for art in li:
        aid = art.get("aid", None)
        if aid is None:
            continue
        print(art.get("title", ""))
        getContent(str(aid)+".txt", base+"article/"+str(aid))


def searchMain():
    global count
    count = 0
    global cache
    cache = 0
    global vaild
    vaild = 0
    global new
    new = 0
    for off in range(0, 50):
        last = new
        searchRecommend(base, off)
        if new == last:
            break

    print("count {} cache {} vaild {} new {}".format(count, cache, vaild, new))
    return new


if __name__ == '__main__':
    searchMain()
