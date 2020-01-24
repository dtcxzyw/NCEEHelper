#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sqlite3


class Dict (object):

    def __init__(self, filename):
        self.__dbname = filename
        self.__conn = None
        self.__open()

    # 初始化并创建必要的表格和索引
    def __open(self):
        sql = '''
        CREATE TABLE IF NOT EXISTS "stardict" (
            "id" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,
            "word" VARCHAR(64) COLLATE NOCASE NOT NULL UNIQUE,
            "sw" VARCHAR(64) COLLATE NOCASE NOT NULL,
            "phonetic" VARCHAR(64),
            "definition" TEXT,
            "translation" TEXT,
            "pos" VARCHAR(16),
            "collins" INTEGER DEFAULT(0),
            "oxford" INTEGER DEFAULT(0),
            "tag" VARCHAR(64),
            "bnc" INTEGER DEFAULT(NULL),
            "frq" INTEGER DEFAULT(NULL),
            "exchange" TEXT,
            "detail" TEXT,
            "audio" TEXT
        );
        CREATE UNIQUE INDEX IF NOT EXISTS "stardict_1" ON stardict (id);
        CREATE UNIQUE INDEX IF NOT EXISTS "stardict_2" ON stardict (word);
        CREATE INDEX IF NOT EXISTS "stardict_3" ON stardict (sw, word collate nocase);
        CREATE INDEX IF NOT EXISTS "sd_1" ON stardict (word collate nocase);
        '''

        self.__conn = sqlite3.connect(
            self.__dbname, isolation_level="IMMEDIATE")
        self.__conn.isolation_level = "IMMEDIATE"

        sql = '\n'.join([n.strip('\t') for n in sql.split('\n')])
        sql = sql.strip('\n')

        self.__conn.executescript(sql)
        self.__conn.commit()
        return True

    # 关闭数据库
    def close(self):
        if self.__conn:
            self.__conn.close()
        self.__conn = None

    def __del__(self):
        self.close()

    # 浏览词典
    def __iter__(self):
        c = self.__conn.cursor()
        sql = 'select "id", "word" from "stardict"'
        sql += ' order by "word" collate nocase;'
        c.execute(sql)
        return c.__iter__()

    # 取得长度
    def __len__(self):
        return self.count()

    # 检测存在
    def __contains__(self, key):
        return self.query(key) is not None

    # 查询单词
    def __getitem__(self, key):
        return self.query(key)

    # 取得所有单词
    def dumps(self):
        return [n.lower() for _, n in self.__iter__()]
