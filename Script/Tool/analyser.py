#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import numpy as np
from matplotlib import pyplot as plt
import matplotlib
import time


def generate(klib):
    cmd = "cd ../../Bin && NCEEHelper.exe Analyser {} ImportanceSampler".format(
        klib)
    res = os.system(cmd)
    if res != 0:
        print("Failed to generate data for ", klib)
        return
    path = "../../Bin/Results/{}.log".format(klib)
    cnt = 0
    X = []
    A = []
    C = []
    M = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            cnt += 1
            X.append(cnt)
            ratio = line.split()
            A.append(float(ratio[0]))
            C.append(float(ratio[1]))
            M.append(float(ratio[2]))
    ct = time.localtime(time.time())
    plt.title("{} {}.{}.{}".format(klib, ct.tm_year, ct.tm_mon, ct.tm_mday))
    plt.xlabel("Time")
    plt.ylabel("Ratio")
    plt.ylim(0.0, 1.0)
    plt.plot(X, A, label="Accuracy")
    plt.plot(X, C, label="Coverage")
    plt.plot(X, M, label="Master")
    plt.legend()
    plt.savefig("ARes/{}.png".format(klib))
    plt.close('all')


if __name__ == "__main__":
    generate("Biology")
    generate("WordCast")
    generate("ReadWord")
