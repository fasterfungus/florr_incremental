#!/bin/bash
cd Client/build
cmake ..
make -j8
cp florr_incremental-client.js florr_incremental-client.wasm ../public/
python3 -m http.server --directory ../public