UEFI-OSLoader
=============

## Overview
UEFI環境で動くシンプルなOS/UEFIアプリ・ローダーです。
[EDKII](https://github.com/tianocore/edk2)を使って作成。
UEFIの勉強を兼ねた最低限の実装です。
コードの理解しやすさ重視で作っています。

## Feature
* Simple
* Easy to understand

## Build
Environment: Ubuntu 14.10 x86_64, gcc 4.9.1
Target: x64

1. apt-get install git uuid-dev nasm gcc g++ python ruby rake qemu-system-x86-64
1. git submodule update --init
1. rake setup
1. rake build

gccのバージョンが4.9.x以外の場合は、実行前にRakefileの"GCC49"の部分をバージョンに合わせて書き換えてください。

## How to Run
1. rake run

### Run Example
1. rake example
1. (起動したコンソールに入力) UefiOSloader
1. ('A'が出力されれば成功)
1. Ctrl-a, xでqemuを終了

