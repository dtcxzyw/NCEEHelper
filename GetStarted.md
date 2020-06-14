# NCEEHelper使用指南

## 前置条件
+ 请确保足够的磁盘空间（应该是上界）
    + 本体 3.59G（程序+数据+代码+缓存）
    + vcpkg 7.24G（这里水分比较多，有一些不相关的库被计入）
    + CMake 91MB
    + Git 251MB
    + GitHub Desktop 181MB
    + Google Chrome 470MB
    + Microsoft Speech SDK 5.1 83MB
    + MSVC可再发行组件包 120MB
    + Visual Studio Code 738MB（本体+扩展）
    + Visual Studio 约11.5GB(本体+缓存+Win10SDK)
    + Python3 822MB
    + SQLiteStudio 63MB
    + NLTK数据 163MB

    合计约25.5GB
+ 请确保有足够的耐心和一定的应对突发事件的能力（这是你唯一的文档了）
+ 请确保有C++/Python/SQL/HTML/CSS/MarkDown/CMake/JSON/JavaScript/Git相关经验或有学习能力
+ 请确保有足够的网速和时间来安装依赖/学习知识/解决问题
## 依赖安装
### C++部分
+ 安装[Visual Studio](https://visualstudio.microsoft.com/zh-hans/) (也可以是其它任何C++编译器)
+ 安装[CMake](https://cmake.org/)
+ 安装[vcpkg](https://github.com/microsoft/vcpkg) （也可以自行下载编译依赖）
+ 从vcpkg安装crossguid,cxxopts,icu,nlohmann-json,pybind11,rang
+ 可能需要Microsoft Speech SDK 5.1
### Python部分
+ 安装[CPython](https://www.python.org/)
+ 从pip安装requirements.txt内的包
+ 下载nltk数据，名称根据报错内容得到
### VSCode部分
+ 安装[Visual Studio Code](https://code.visualstudio.com/)
+ 安装扩展guid,Markdown All in One,Markdown PDF,Python
+ 安装Google Chrome（任意Chromium或Chrome浏览器，用于打印PDF，当然也可以选用pypandoc+latex方案）
### Misc
+ 安装SQLiteStudio或其它SQL数据库编辑器
+ 安装Git和GitHub Desktop（会用命令行可省去）
+ 下载语料库到Script/Analyser/Input中
## 安装及使用
1. 克隆仓库并注意克隆submodule(Bus)
2. 用CMake编译生成NCEEHelper（注意在Bin/Shared文件夹中放icu，在English插件目录下放pythonxx.dll）
3. 可以开始边修边用了
### 目录结构
+ Bin：可执行程序目录
+ Build：CMake项目生成目录
+ DataBase：数据库（英语+生物）
+ History：历史记录（可清除或自己写个脚本把它拍扁到一个文件以保留题目难度信息，减少磨合时间）
+ Licenses：开源协议，可从这里找依赖项
+ Note：手打笔记
+ Script：脚本（包括爬虫、分析器、工具）
+ Src：源代码
+ Urls：参考资料
### 使用方法
+ 从Urls中看资料
+ 编译Note中的.md文件为pdf并手动修复排版，看笔记
+ 运行Script/Spider/UpdateAll.py获取最新新闻/时评
+ 运行Script/Analyser下的各种分析器
+ 运行Bin/NCEEHelper进行自我测试（目前有英语阅读词汇、英语词形变换、生物填空）
+ 运行Script/Tools/analyser.py得到复习可视化反馈
+ 运行Bin/NCEEHelper将生物题目打印出来用于纸质版复习
### 未来扩展
+ 可以研究源代码和Bus的代码，为NCEEHelper写更多的插件
+ 研究如何从时评中得到自己需要的东西（我实验了几个月得到的还是噪声大到没法用的结果）
+ 等NLP发展成熟了，试试直接通过OCR从书本中提取关键知识并挖空
+ 吐槽我的代码和随性的gitcommit记录（和OI-Source一个风格）
