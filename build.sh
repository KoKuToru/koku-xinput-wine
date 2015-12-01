#!/bin/bash
(echo && \
pushd xinput1_3 && \
winemaker --nosource-fix --dll --wine32 --nomfc -ldl -lSDL2 -lpthread . -I../include && \
make
popd) && \
(echo && \
pushd xinput1_4 && \
winemaker --nosource-fix --dll --wine32 --nomfc -ldl . -I../include && \
make
popd) && \
cp xinput1_3/xinput1_3.dll.so bin/xinput1_3.dll && \
cp xinput1_4/xinput1_4.dll.so bin/xinput1_4.dll && \
echo && echo "all done, files are in bin-folder"

