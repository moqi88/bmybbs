#!/bin/sh
# ��bmy�������grep
# author: interma@bmy

cd ./src
grep -n $1 *.c
grep -n $1 *.h
cd ../nju09
grep -n $1 *.c
grep -n $1 *.h
cd ../local_utl
grep -n $1 *.c
grep -n $1 *.h
cd ../ythtlib
grep -n $1 *.c
grep -n $1 *.h
cd ../libythtbbs
grep -n $1 *.c
grep -n $1 *.h
cd ../include
grep -n $1 *.h
