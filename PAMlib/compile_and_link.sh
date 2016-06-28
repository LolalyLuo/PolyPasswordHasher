#!/bin/bash

gcc -fPIC -fno-stack-protector -c  PPHpam.c
sudo ld -x --shared -o /lib/security/mypam.so PPHpam.o 
