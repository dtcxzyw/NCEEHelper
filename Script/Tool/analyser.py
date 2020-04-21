#!/usr/bin/python3
# -*- coding: utf-8 -*-

import matplotlib.gridspec as gridspec
import os
import numpy as np
from matplotlib import pyplot as plt
import matplotlib
import time


def generateTest(klib, ax):
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
    ax.set_xlabel("Count")
    ax.set_ylabel("Ratio")
    ax.set_ylim(0.0, 1.0)
    ax.plot(X, A, label="Accuracy", marker=".")
    ax.plot(X, C, label="Coverage", marker=".")
    ax.plot(X, M, label="Master", marker='.')
    ax.plot(X, E, label="E[Master]", marker='.')
    ax.legend()


def generateTime(klib, ax):
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
    ax.set_xlabel("Date")
    ax.set_ylabel("Count")
    ax.set_xticks(X)
    ax.set_xticklabels(ticks)
    rects = ax.bar(X, A)
    for rect in rects:
        height = rect.get_height()
        ax.annotate('{}'.format(height),
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 1),
                    textcoords="offset points",
                    ha='center', va='bottom')


def generateAcc(klib, ax):
    import seaborn as sns
    path = "../../Bin/Results/{}.dist".format(klib)
    X = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            data = line.split()
            acc = float(data[0])
            for i in range(int(data[1])):
                X.append(acc)
    ax.set_xlabel("Acc")
    ax.set_ylabel("Density")
    sns.distplot(X, bins=10, norm_hist=True, ax=ax)


def generate(klib):
    cmd = "cd ../../Bin && NCEEHelper.exe Analyser {} ImportanceSampler".format(
        klib)
    res = os.system(cmd)
    if res != 0:
        print("Failed to generate data for ", klib)
        return
    ct = time.localtime(time.time())
    fig = plt.figure(constrained_layout=False, figsize=[12, 9])
    fig.suptitle("{} {}.{}.{}".format(
        klib, ct.tm_year, ct.tm_mon, ct.tm_mday))

    import matplotlib.gridspec as gridspec
    gs = gridspec.GridSpec(2, 1, figure=fig)

    generateTime(klib, fig.add_subplot(gs[0]))
    gs2 = gridspec.GridSpecFromSubplotSpec(1, 2, subplot_spec=gs[1])
    generateTest(klib, fig.add_subplot(gs2[0]))
    generateAcc(klib, fig.add_subplot(gs2[1]))

    # fig.align_labels()
    plt.savefig("ARes/{}.png".format(klib), bbox_inches='tight')
    plt.close('all')


if __name__ == "__main__":
    generate("Biology")
    generate("WordCast")
    generate("ReadWord")
