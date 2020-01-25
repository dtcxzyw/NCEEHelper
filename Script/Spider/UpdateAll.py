#!/usr/bin/python3
# -*- coding: utf-8 -*-

import ReaderDigest
import GuangMing
import People
import XinHua
import Science

rdu = ReaderDigest.searchMain()
gmu = GuangMing.searchMain()
pu = People.searchMain()
xhu = XinHua.searchMain()
scu = Science.searchMain()

print("\nReaderDigest {} GuangMing {} People {} XinHua {} Science {}".format(
    rdu, gmu, pu, xhu, scu))
