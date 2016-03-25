#!/bin/bash
set -e

for dll in 'xinput1_1' 'xinput1_2' 'xinput9_1_0' 'xinput1_3' 'xinput1_4'
do
    pushd $dll
    winemaker --nosource-fix --dll --wine32 --nomfc -ldl . -I../include       
    make
    popd
    cp $dll/$dll.dll.so bin/$dll.dll.so   
done
echo ""
echo ""
echo "all done, files are in bin-folder"
