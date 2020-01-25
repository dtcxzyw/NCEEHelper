#!/usr/bin/python3
# -*- coding: utf-8 -*-

from hanziconv import HanziConv
import nltk
import os


def countFile(file, enc, words):
    with open(file, encoding=enc) as f:
        print(HanziConv.toSimplified(f.read()))


def count(dir, encoding, out):
    words = []
    cnt = 0
    for r, sd, files in os.walk(dir):
        for f in files:
            cnt = cnt+1
            print("{} {}".format(cnt, f))
            countFile(r+f, encoding, words)
    dist = nltk.FreqDist(words)
    out = open("./Output/"+out+".txt", "w", encoding="utf-8")
    for (word, freq) in dist.most_common():
        out.write("{} {}\n".format(word, freq))


if __name__ == '__main__':
    count('Input/24his/', 'utf-8', 'wenyan')
