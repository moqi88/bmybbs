#!/bin/sh
# 对bmy代码进行grep
# author: interma@bmy

cd ./src
grep -n $1 *.c
grep -n $1 *.h
cd ../ythtlib
grep -n $1 *.c
grep -n $1 *.h
cd ../libythtbbs
grep -n $1 *.c
grep -n $1 *.h
cd ../include
#grep $1 *.c
grep -n $1 *.h
