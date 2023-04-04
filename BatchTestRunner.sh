#!/bin/bash

tags=("gplTinyModel1" "gplTinyModel2" "carModel1" "carModel2" "ppuModel1" "ppuModel2" "aplModel1" "aplModel2" "gplModel1" "gplModel2" "eshopModel1" "eshopModel2" "violetModel1" "violetModel2" "gplAheadModel1" "gplAheadModel2" "waterlooModel1" "waterlooModel2" "axTLSModel1" "axTLSModel2" "uClibCModel1" "uClibCModel2" "berkeleyDBModel1" "berkeleyDBModel2")
output=""

for str in ${tags[@]}; do
  output=`./build/meson.debug.linux.x86_64/FMBuilderTest [$str] | grep "All tests passed" | wc -l`;
  echo $str: $output;
done

