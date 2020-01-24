#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys


class LemmaDB (object):

    def __init__(self):
        self._stems = {}
        self._words = {}
        self._frqs = {}

    # 读取数据
    def load(self, filename, encoding=None):
        content = open(filename, 'rb').read()
        if content[:3] == b'\xef\xbb\xbf':
            content = content[3:].decode('utf-8', 'ignore')
        elif encoding is not None:
            text = content.decode(encoding, 'ignore')
        else:
            text = None
            match = ['utf-8', sys.getdefaultencoding(), 'ascii']
            for encoding in match + ['gbk', 'latin1']:
                try:
                    text = content.decode(encoding)
                    break
                except:
                    pass
            if text is None:
                text = content.decode('utf-8', 'ignore')
        number = 0
        for line in text.split('\n'):
            number += 1
            line = line.strip('\r\n ')
            if (not line) or (line[:1] == ';'):
                continue
            pos = line.find('->')
            if not pos:
                continue
            stem = line[:pos].strip()
            p1 = stem.find('/')
            frq = 0
            if p1 >= 0:
                frq = int(stem[p1 + 1:].strip())
                stem = stem[:p1].strip()
            if not stem:
                continue
            if frq > 0:
                self._frqs[stem] = frq
            for word in line[pos + 2:].strip().split(','):
                p1 = word.find('/')
                if p1 >= 0:
                    word = word[:p1].strip()
                if not word:
                    continue
                self.add(stem, word.strip())
        return True

    # 根据词根找衍生，或者根据衍生反向找词根
    def get(self, word, reverse=False):
        if not reverse:
            if word not in self._stems:
                if word in self._words:
                    return [word]
                return None
            words = [(v, k) for (k, v) in self._stems[word].items()]
        else:
            if word not in self._words:
                if word in self._stems:
                    return [word]
                return None
            words = [(v, k) for (k, v) in self._words[word].items()]
        words.sort()
        return [k for (v, k) in words]

    # 知道一个单词求它的词根
    def word_stem(self, word):
        return self.get(word, reverse=True)

    def add(self, stem, word):
        if stem not in self._stems:
            self._stems[stem] = {}
        if word not in self._stems[stem]:
            self._stems[stem][word] = len(self._stems[stem])
        if word not in self._words:
            self._words[word] = {}
        if stem not in self._words[word]:
            self._words[word][stem] = len(self._words[word])
        return True

    def __len__(self):
        return len(self._stems)

    def __getitem__(self, stem):
        return self.get(stem)

    def __contains__(self, stem):
        return (stem in self._stems)

    def __iter__(self):
        return self._stems.__iter__()
