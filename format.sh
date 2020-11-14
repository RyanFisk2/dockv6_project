#!/bin/bash
find . -type f | egrep '\.[ch]$' |  xargs clang-format -i
