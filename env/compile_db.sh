#!/bin/bash

ROOT_FOLDER="$(readlink -f $(dirname $0)/..)"

pushd $ROOT_FOLDER

tup generate --config bin/tup.config build.sh

bear ./build.sh

find . -exec file {} \; | grep ELF | cut -d ':' -f 1 | while read file; do rm $file; done

rm build.sh

popd
