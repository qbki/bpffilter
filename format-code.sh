#!/usr/bin/env sh
clang-format -i `git ls-files | egrep '\.(h|cpp|hxx|c)$$'`
