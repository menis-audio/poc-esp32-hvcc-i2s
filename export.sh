#!/usr/bin/env bash
set -euo pipefail

rm -rf main/hvcc
cd main
hvcc test.pd -o hvcc
cd ..
idf.py build
idf.py flash
