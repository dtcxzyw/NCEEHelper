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
    words = pseg.cut(line)
    for word, flag in words:
        word = re.sub(nchn, "", word)
        counts.add(word)
    return counts


def countFile(filename, counts, num):
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
                    counts.add(word)
            pbar.finish()


def output(counts):
    out = open("./Output/chineseFilter.txt", "w", encoding="utf-8")
    for word in counts:
        out.write(word+"\n")


def count(dir, num):
    counts = set()
    cnt = 0
    for r, sd, files in os.walk(dir):
        for f in files:
            cnt = cnt+1
            path = r+"/"+f
            print("{} {}".format(cnt, path))
            countFile(path, counts, num)
    output(counts)


if __name__ == '__main__':
    # jieba.enable_paddle()
    num = sys.argv[1]
    count("./Input/raw_chat_corpus", int(num))
