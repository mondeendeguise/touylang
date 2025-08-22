#!/usr/bin/env bash

set -xe

cc main.c lexer.c -o touy -Wall -Wextra -pedantic -ggdb -O2
