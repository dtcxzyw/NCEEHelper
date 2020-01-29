#!/usr/bin/python3
# -*- coding: utf-8 -*-

import pypandoc
import os

print("Pandoc", pypandoc.get_pandoc_version())

base = "../../Note/"

for r, ds, fs in os.walk(base):
    for f in fs:
        if f.endswith(".md"):
            src = r+"/"+f
            dst = src.replace(".md", ".pdf")
            print(src, "->", dst)
            print(pypandoc.convert_file(
                src, "pdf", outputfile=dst, format="gfm", encoding="utf-8",
                extra_args=["-V", "CJKmainfont=Microsoft YaHei", "--pdf-engine=xelatex"]))
