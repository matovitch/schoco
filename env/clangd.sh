#!/bin/bash

ROOT_FOLDER="$(readlink -f $(dirname $0)/..)"

clangd -compile-commands-dir=$ROOT_FOLDER
