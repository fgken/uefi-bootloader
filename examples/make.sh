#!/bin/sh
gcc -nostdlib -e foo -o out-serial-A.elf out-serial-A.c
cp out-serial-A.elf ../image/
