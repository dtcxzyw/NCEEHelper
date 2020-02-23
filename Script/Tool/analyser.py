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
    X = []
    A = []
    C = []
    M = []
    E = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            ratio = line.split()
            X.append(float(ratio[0]))
            A.append(float(ratio[1]))
            C.append(float(ratio[2]))
            M.append(float(ratio[3]))
            E.append(float(ratio[4]))
    ct = time.localtime(time.time())
    plt.title("{} {}.{}.{}".format(klib, ct.tm_year, ct.tm_mon, ct.tm_mday))
    plt.xlabel("Count")
    plt.ylabel("Ratio")
    plt.ylim(0.0, 1.0)
    plt.plot(X, A, label="Accuracy", marker=".")
    plt.plot(X, C, label="Coverage", marker=".")
    plt.plot(X, M, label="Master", marker='.')
    plt.plot(X, E, label="E[Master]", marker='.')
    plt.legend()
    plt.savefig("ARes/{}.png".format(klib))
    plt.close('all')


if __name__ == "__main__":
    generate("Biology")
    generate("WordCast")
    generate("ReadWord")
