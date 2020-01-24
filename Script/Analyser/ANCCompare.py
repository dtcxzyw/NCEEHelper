#!/usr/bin/python3
# -*- coding: utf-8 -*-
import ReadWordDict
import Lemmatizer
from nltk.stem.wordnet import WordNetLemmatizer

lmzer = WordNetLemmatizer()
lmdb = Lemmatizer.LemmaDB()
lmdb.load("../../DataBase/English/ECDICTData/lemma.en.txt")


def lemmatize(word):
    res = lmdb.word_stem(word)
    if res is None:
        return lmzer.lemmatize(word)
    return res[0]


rd = ReadWordDict.Dict("../../DataBase/English/ECDICTData/readword.db").dumps()
rd.extend({"uh-huh", "um-hum"})
out = open("Output/ANCCount.txt", "w", encoding="utf-8")
with open("Input/ANC-token-count.txt", encoding="utf-8") as f:
    count = 0
    for line in f.readlines():
        pos = line.find(" ")
        word = line[:pos]
        lem = lemmatize(word)
        if len(word) > 2 and lem not in rd and word not in rd:
            rd.append(lem)
            out.write(line)
            count = count+1
            if count > 100:
                break
