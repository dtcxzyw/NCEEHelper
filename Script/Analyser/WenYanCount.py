#!/usr/bin/python3
# -*- coding: utf-8 -*-

from hanziconv import HanziConv
import nltk
import os
import re

nchn = re.compile(r"[^\u4e00-\u9fff]")


def countFile(file, enc, words, step):
    with open(file, encoding=enc) as f:
        text = HanziConv.toSimplified(f.read())
        text = re.sub(nchn, "", text)
        for i in range(0, len(text)):
            sub = text[i:i+step]
            words[sub] = words.get(sub, 0)+1


def count(dir, encoding, outPre):
    lw = {}
    for step in range(4, 0, -1):
        print("Round {}".format(step))
        words = {}
        cnt = 0
        for r, sd, files in os.walk(dir):
            for f in files:
                cnt = cnt+1
                print("{} {}".format(cnt, f))
                countFile(r+f, encoding, words, step)
        cnt = 0
        for (lword, lf) in lw:
            cnt = cnt+1
            print("filt {} {}".format(cnt, lword))
            for i in range(0, len(lword)):
                for j in range(i, len(lword)):
                    sub = lword[i:j+1]
                    if sub in words:
                        words[sub] -= lf
        print("sorting")
        lw = sorted(words.items(), key=lambda item: item[1], reverse=True)
        lw = lw[:min(1000, len(lw))]
        out = open("./Output/{}{}.txt".format(outPre, step),
                   "w", encoding="utf-8")
        for word, freq in lw:
            out.write("{} {}\n".format(word, freq))


if __name__ == '__main__':
    count('Input/24his/', 'utf-8', 'wenyan')
