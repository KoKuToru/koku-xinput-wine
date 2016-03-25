`builtin`-Hack
=========

*adds Ordinal 100 function to wine builtindll*

Applications that use SDL2 need this, or it wont use XInput...

Build
---------
```
bash build.sh
```

Needs `libwine`..

Usage
---------
Copy the files from `bin` into your wine-prefix, replacing the original files

Alternative-Usage
---------
Remove the `.so`-part and copy the files into the folder next to the game executeable
