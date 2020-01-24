#!/usr/bin/python3
# -*- coding: utf-8 -*-

import ReaderDigest
import GuangMing
import People
import XinHua

# ReaderDigest.searchMain()
rdu = 0
gmu = GuangMing.searchMain()
pu = People.searchMain()
xhu = XinHua.searchMain()

print("\nReaderDigest {} GuangMing {} People {} XinHua {}".format(rdu, gmu, pu, xhu))
