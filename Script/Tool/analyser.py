#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import numpy as np
from matplotlib import pyplot as plt
import matplotlib
import time


def generateTest(klib):
    path = "../../Bin/Results/{}.log".format(klib)
    X = []
    A = []
    C = []
    M = []
    E = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            data = line.split()
            X.append(float(data[0]))
            A.append(float(data[1]))
            C.append(float(data[2]))
            M.append(float(data[3]))
            E.append(float(data[4]))
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


def generateTime(klib):
    path = "../../Bin/Results/{}.res".format(klib)
    X = []
    A = []
    cnt = 0
    ticks = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            data = line.split()
            px = data[0]
            if px == "10000":
                px = "never"
            ticks.append(px)
            cnt = cnt+1
            X.append(cnt)
            A.append(int(data[1]))
    ct = time.localtime(time.time())
    plt.title("{} {}.{}.{}".format(klib, ct.tm_year, ct.tm_mon, ct.tm_mday))
    plt.xlabel("Date")
    plt.ylabel("Count")
    rects = plt.bar(X, A)
    for rect in rects:
        height = rect.get_height()
        plt.annotate('{}'.format(height),
                     xy=(rect.get_x() + rect.get_width() / 2, height),
                     xytext=(0, 1),
                     textcoords="offset points",
                     ha='center', va='bottom')
    plt.xticks(X, ticks)
    plt.savefig("ARes/{}-Time.png".format(klib))
    plt.close('all')


def generateAcc(klib):
    import seaborn as sns
    path = "../../Bin/Results/{}.dist".format(klib)
    X = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            data = line.split()
            acc = float(data[0])
            for i in range(int(data[1])):
                X.append(acc)
    ct = time.localtime(time.time())
    plt.title("{} {}.{}.{}".format(klib, ct.tm_year, ct.tm_mon, ct.tm_mday))
    plt.xlabel("Acc")
    plt.ylabel("Density")
    sns.distplot(X, bins=10, norm_hist=True)
    plt.savefig("ARes/{}-Acc.png".format(klib))
    plt.close('all')


def generate(klib):
    cmd = "cd ../../Bin && NCEEHelper.exe Analyser {} ImportanceSampler".format(
        klib)
    res = os.system(cmd)
    if res != 0:
        print("Failed to generate data for ", klib)
        return
    generateTest(klib)
    generateTime(klib)
    generateAcc(klib)


if __name__ == "__main__":
    generate("Biology")
    generate("WordCast")
    generate("ReadWord")
