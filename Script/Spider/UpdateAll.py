#!/usr/bin/python3
# -*- coding: utf-8 -*-

import ReaderDigest
import GuangMing
import People
import XinHua
import Science
import HuanQiu

rdu = ReaderDigest.searchMain()
gmu = GuangMing.searchMain()
pu = People.searchMain()
xhu = XinHua.searchMain()
scu = Science.searchMain()
hqu = HuanQiu.searchMain()

print("\nReaderDigest {} GuangMing {} People {} XinHua {} Science {} HuanQiu {}".format(
    rdu, gmu, pu, xhu, scu, hqu))
