#!/usr/bin/python3
# -*- coding: utf-8 -*-

# https://blog.csdn.net/htgt_tuntuntun/article/details/80499427

import jieba
import jieba.posseg as pseg
import os
import re

nchn = re.compile(r"[^\u4e00-\u9fff]")


def countFile(filename, counts):
    text = open(filename, encoding="utf-8").read()
    words = pseg.cut(text)
    for word, flag in words:
        word = re.sub(nchn, "", word)
        counts[word] = counts.get(word, 0) + 1


def output(counts):
    stopwords = set(line.strip() for line in open(
        "Output/chineseFilter.txt", encoding="utf-8").readlines())
    out = open("./Output/chineseCount.txt", "w", encoding="utf-8")
    items = list(counts.items())
    items.sort(key=lambda x: x[1], reverse=True)
    for word, count in items:
        if word not in stopwords:
            out.write("{:<10}{:>7}\n".format(word, count))


def count(dirs):
    counts = dict()
    cnt = 0
    for d in dirs:
        for r, sd, files in os.walk(d):
            for f in files:
                cnt = cnt+1
                print("{} {}".format(cnt, f))
                countFile(r+f, counts)
    output(counts)


if __name__ == '__main__':
    # jieba.enable_paddle()
    count({'../Spider/Output/XinHua/', '../Spider/Output/People/',
           "../Spider/Output/GuangMing/", "../Spider/Output/HuanQiu/"})
