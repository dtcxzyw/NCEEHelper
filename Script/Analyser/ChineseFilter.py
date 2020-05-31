#!/usr/bin/python3
# -*- coding: utf-8 -*-

import jieba
import jieba.posseg as pseg
import os
import re
from progressbar import *
from multiprocessing import Pool

nchn = re.compile(r"[^\u4e00-\u9fff]")


def countLine(line):
    counts = set()
    line = re.sub(nchn, "", line)
    words = pseg.cut(line)
    for word, flag in words:
        counts.add(word)
    return counts


def countBigFile(filename, counts, num):
    with open(filename, encoding="utf-8") as file:
        widgets = ['Progress: ', Percentage(), ' ', Bar('#'), ' ',
                   Timer(), ' ', ETA()]
        lines = file.readlines()

        siz = len(lines)

        with Pool(processes=num) as pool:
            pbar = ProgressBar(widgets=widgets, maxval=siz).start()
            cnt = 0
            for res in pool.imap_unordered(countLine, lines):
                cnt = cnt+1
                pbar.update(cnt)
                for word in res:
                    counts[word] = counts.get(word, 0) + 1
            pbar.finish()


def countFile(filename, counts, num):
    text = open(filename, encoding="utf-8").read()
    text = re.sub(nchn, "", text)
    words = pseg.cut(text)
    for word, flag in words:
        counts[word] = counts.get(word, 0) + 1


def outputFilter(counts):
    out = open("./Output/chineseFilter.txt", "w", encoding="utf-8")
    items = list(counts.items())
    items.sort(key=lambda x: x[1], reverse=True)
    for word, freq in counts.items():
        if word.strip() != "":
            out.write("{} {}\n".format(word, freq))


def loadStopWords():
    threshold = int(sys.argv[2])
    res = set()
    with open("Output/chineseFilter.txt", encoding="utf-8") as f:
        for line in f:
            data = line.split()
            if int(data[1]) >= threshold:
                res.add(data[0])
    return res


def outputCount(counts):
    stopwords = loadStopWords()
    out = open("./Output/chineseCount.txt", "w", encoding="utf-8")
    items = list(counts.items())
    items.sort(key=lambda x: x[1], reverse=True)
    for word, count in items:
        if word not in stopwords:
            out.write("{:<10}{:>7}\n".format(word, count))


def count(dirs, num, fileCountFunc, outputFunc):
    counts = dict()
    cnt = 0
    for dir in dirs:
        for r, sd, files in os.walk(dir):
            for f in files:
                cnt = cnt+1
                path = r+"/"+f
                print("{} {}".format(cnt, path))
                fileCountFunc(path, counts, num)
    outputFunc(counts)


def usage():
    print("Filter ThreadNum/Count StopWordThreshold\n")


if __name__ == '__main__':
    usage()
    mode = sys.argv[1]
    if mode == "Filter":
        num = int(sys.argv[2])
        count({"./Input/raw_chat_corpus"}, num, countBigFile, outputFilter)
    elif mode == "Count":
        count({'../Spider/Output/XinHua/', '../Spider/Output/People/',
               "../Spider/Output/GuangMing/", "../Spider/Output/HuanQiu/"}, 0, countFile, outputCount)
    else:
        raise "BadMode"
