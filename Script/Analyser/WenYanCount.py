#!/usr/bin/python3
# -*- coding: utf-8 -*-

from hanziconv import HanziConv
import nltk
import os
import re
import json

nchn = re.compile(r"[^\u4e00-\u9fff]")


def countText(text, words, step):
    text = HanziConv.toSimplified(text)
    text = re.sub(nchn, "", text)
    for i in range(0, len(text)):
        sub = text[i:i+step]
        if len(sub) == step:
            words[sub] = words.get(sub, 0)+1


def countPar(par, words, step):
    # title = par.get("title", par.get(
    #    "rhythmic", par.get("chapter", "Unknown")))
    #author = par.get("author", "Unknown")
    #print("count {} by {}".format(title, author))
    npar = par.get("paragraphs", par.get("content", ""))
    if npar != None:
        for sent in npar:
            countText(sent, words, step)


def countFile(file, enc, words, step):
    with open(file, encoding=enc) as f:
        text = ""
        if file.find(".json") != -1:
            pars = json.load(f)
            if isinstance(pars, list):
                for par in pars:
                    countPar(par, words, step)
            else:
                countPar(pars, words, step)

        else:
            countText(f.read(), words, step)


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
                countFile(r+"/"+f, encoding, words, step)
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
    #count('Input/24his/', 'utf-8', 'wenyan')
    count('Input/poem/', 'utf-8', 'shici')
