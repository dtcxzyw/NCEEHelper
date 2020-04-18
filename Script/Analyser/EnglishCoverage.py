#!/usr/bin/python3
# -*- coding: utf-8 -*-

# https://github.com/rocketk/wordcounter
# https://blog.csdn.net/zhuzuwei/article/details/80484501

import sys
import nltk
from nltk.tokenize import word_tokenize
from nltk.stem.wordnet import WordNetLemmatizer
import re
import os
import ReadWordDict
import Lemmatizer
import time
from multiprocessing import Pool

# patterns that used to find or/and replace particular chars or words
# to find chars that are not a letter, a blank or a quotation
pat_letter = re.compile(r'[^a-zA-Z \']+')
# to find the 's following the pronouns. re.I is refers to ignore case
pat_is = re.compile("(it|he|she|that|this|there|here)(\'s)", re.I)
# to find the 's following the letters
pat_s = re.compile("(?<=[a-zA-Z])\'s")
# to find the ' following the words ending by s
pat_s2 = re.compile("(?<=s)\'s?")
# to find the abbreviation of not
pat_not = re.compile("(?<=[a-zA-Z])n\'t")
# to find the abbreviation of would
pat_would = re.compile("(?<=[a-zA-Z])\'d")
# to find the abbreviation of will
pat_will = re.compile("(?<=[a-zA-Z])\'ll")
# to find the abbreviation of am
pat_am = re.compile("(?<=[I|i])\'m")
# to find the abbreviation of are
pat_are = re.compile("(?<=[a-zA-Z])\'re")
# to find the abbreviation of have
pat_ve = re.compile("(?<=[a-zA-Z])\'ve")


def filter(text):
    new_text = text
    new_text = pat_letter.sub(' ', text).strip().lower()
    new_text = pat_is.sub(r"\1 is", new_text)
    new_text = pat_s.sub("", new_text)
    new_text = pat_s2.sub("", new_text)
    new_text = pat_not.sub(" not", new_text)
    new_text = pat_would.sub(" would", new_text)
    new_text = pat_will.sub(" will", new_text)
    new_text = pat_am.sub(" am", new_text)
    new_text = pat_are.sub(" are", new_text)
    new_text = pat_ve.sub(" have", new_text)
    new_text = new_text.replace('\'', ' ')
    return new_text


sen_tokenizer = nltk.data.load('tokenizers/punkt/english.pickle')
lmzer = WordNetLemmatizer()
lmdb = Lemmatizer.LemmaDB()
lmdb.load("../../DataBase/English/ECDICTData/lemma.en.txt")


def lemmatize(word, pos):
    res = lmdb.word_stem(word)
    if res is None:
        return lmzer.lemmatize(word, pos)
    return res[0]


def getWordnetPos(treebank_tag):
    if treebank_tag.startswith('J'):
        return nltk.corpus.wordnet.ADJ
    elif treebank_tag.startswith('V'):
        return nltk.corpus.wordnet.VERB
    elif treebank_tag.startswith('N'):
        return nltk.corpus.wordnet.NOUN
    elif treebank_tag.startswith('R'):
        return nltk.corpus.wordnet.ADV
    else:
        return nltk.corpus.wordnet.NOUN


wordDictName = sys.argv[1]

if wordDictName != "readword" and wordDictName != "zgk":
    raise "BadDict"


def toSet(dic):
    resDict = set()
    for word in dic:
        if word.isalpha():
            resDict.add(word)
    return resDict


wordDict = toSet(ReadWordDict.Dict(
    "../../DataBase/English/ECDICTData/"+wordDictName+".db").dumps())
stdDict = toSet(ReadWordDict.Dict(
    "../../DataBase/English/ECDICTData/stardict.db").dumps())


def tryAdd(word):
    global wordDict
    global stdDict
    if word in stdDict and word not in wordDict:
        wordDict.add(word)


def fixTail(word):
    if word.endswith("y"):
        return word[:-1]+"i"
    if word.endswith("e"):
        return word[:-1]
    return None


def extendDict():
    import copy
    global wordDict
    baseDict = copy.deepcopy(wordDict)
    prefix = [
        "a",     "ab",   "ap",    "ante",  "anti",  "auto",  "be",
        "bi",    "co",   "de",    "demi",  "dis",   "en",    "ex",
        "extra", "fore", "mid",   "post",  "hyper", "hypo",  "in",
        "im",    "ir",   "il",    "infra", "inter", "intra", "mis",
        "mono",  "non",  "off",   "oft",   "out",   "over",  "para",
        "post",  "pre",  "pro",   "poly",  "re",    "retro", "semi",
        "sub",   "sur",  "trans", "ultra", "un",    "under", "up"
    ]
    suffix = [
        "able", "age",   "al",   "ance",   "ant",  "ate",  "arian",
        "ary",  "ation", "cy",   "cation", "dom",  "ee",   "er",
        "ese",  "esque", "ess",  "en",     "ence", "ety",  "fold",
        "ful",  "hood",  "ics",  "ial",    "ical", "ify",  "ion",
        "ish",  "ism",   "ist",  "ista",   "ite",  "itis", "ize",
        "ive",  "ity",   "less", "let",    "ling", "logy", "ment",
        "most", "ness",  "oid",  "or",     "ory",  "ous",  "ship",
        "t",    "ty",    "th",   "ward",   "wise", "y",    "ure",
        "ze"
    ]
    rep = [("al", "ety"), ("ous", "ety"), ("ble", "bility"), ("t", "cy"),
           ("te", "cy"), ("t", "ssion"), ("ve", "f"), ("d", "se"), ("d", "sion"),
           ("fy", "fication"), ("e", "ption"), ("ve", "ption"), ("be", "ption")]
    for word in baseDict:
        for pre in prefix:
            tryAdd(pre+word)
        for suf in suffix:
            tryAdd(word+suf)
        fw = fixTail(word)
        if fw != None:
            for suf in suffix:
                tryAdd(fw+suf)
        for pair in rep:
            if word.endswith(pair[0]):
                tryAdd(word[:-len(pair[0])]+pair[1])


oldSize = len(wordDict)
extendDict()
print("Size {}->{}".format(oldSize, len(wordDict)))


def countFile(file):
    global wordDict
    global stdDict
    hit = 0
    miss = 0
    stra = set()
    fima = set()
    with open(file, encoding="utf-8") as f:
        sens = sen_tokenizer.tokenize(f.read())
        for sen in sens:
            ws = nltk.pos_tag(word_tokenize(filter(sen).lower()))
            for word in ws:
                wt = word[0]
                if len(wt) <= 3:
                    continue
                if wt in wordDict:
                    if wt not in fima:
                        fima.add(wt)
                        hit += 1
                else:
                    pos = getWordnetPos(word[1])
                    lwt = lemmatize(wt, pos)
                    if lwt in wordDict:
                        if lwt not in fima:
                            fima.add(lwt)
                            hit += 1
                    elif lwt in stdDict and lwt not in stra:
                        miss += 1
                        stra.add(lwt)

    return (hit, miss, stra, fima)


def count(dirs):
    hit = 0
    miss = 0
    tasks = []
    for d in dirs:
        for r, sd, files in os.walk(d):
            for f in files:
                tasks.append(r+f)

    tot = len(tasks)
    cnt = 0

    stra = []
    fima = []

    with Pool(processes=8) as pool:
        for nh, nm, nstra, nfima in pool.imap_unordered(countFile, tasks):
            hit += nh
            miss += nm
            cnt += 1
            for word in nstra:
                stra.append(word)
            for word in nfima:
                fima.append(word)
            print("count {} hit {} miss {}".format(cnt, hit, miss))
            print("coverage {:.2f}%".format(hit*100.0/(hit+miss+1)))
            print("process {:.2f}%".format(cnt*100.0/tot))

    print("Done")
    print("count {} hit {} miss {}".format(tot, hit, miss))
    print("coverage {}%".format(hit*100.0/(hit+miss+1)))

    stopwords = [line.strip() for line in open(
        "Input/ensw.txt", encoding="utf-8").readlines()]

    with open("./Output/english-"+wordDictName+".txt", "w", encoding="utf-8") as out:
        for (word, freq) in nltk.FreqDist(stra).most_common(1000):
            if word in stopwords:
                out.write("#S# {} {}\n".format(word, freq))
            else:
                out.write("{} {}\n".format(word, freq))

    for word in wordDict:
        fima.append(word)

    with open("./Output/english-count-"+wordDictName+".txt", "w", encoding="utf-8") as out:
        for (word, freq) in nltk.FreqDist(fima).most_common():
            out.write("{} {} {}\n".format(word, freq-1, freq-1 < 100))


if __name__ == '__main__':
    count({'../Spider/Output/ReaderDigest/', '../Spider/Output/Science/'})
    pass
