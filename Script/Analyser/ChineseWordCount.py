#!/usr/bin/python3
# -*- coding: utf-8 -*-

# https://blog.csdn.net/htgt_tuntuntun/article/details/80499427

import jieba
import os

stopwords = [line.strip() for line in open(
    "Input/stopwords.txt", encoding="utf-8").readlines()]


def countFile(filename, counts):
    text = open(filename, encoding="utf-8").read()
    words = jieba.cut_for_search(text, HMM=True)
    for word in words:
        if word.isspace():
            continue
        if word not in stopwords:
            counts[word] = counts.get(word, 0) + 1


def output(counts):
    out = open("./Output/chinese.txt", "w", encoding="utf-8")
    items = list(counts.items())
    items.sort(key=lambda x: x[1], reverse=True)
    for word, count in items:
        if len(word) >= 4:
            out.write("{:<10}{:>7}\n".format(word, count))


def count(dirs):
    counts = {}
    cnt = 0
    for d in dirs:
        for r, sd, files in os.walk(d):
            for f in files:
                cnt = cnt+1
                print("{} {}".format(cnt, f))
                countFile(r+f, counts)
    output(counts)


if __name__ == '__main__':
    count({'../Spider/Output/XinHua/', '../Spider/Output/People/',
           "../Spider/Output/GuangMing/"})
