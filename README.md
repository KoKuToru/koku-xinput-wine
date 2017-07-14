koku-xinput-wine
================

Adds xinput support to wine, without changing the source of wine.
Modified to use SDL2 gamepad mappings

Install
---------------------
If you are on 64Bit you will need 32Bit tool-chain (multilib)

Because this will generate 32Bit code, for 64Bit support see below.

It depens on SDL2-Librarys.
     
     [user@host code]$git clone https://github.com/KoKuToru/koku-xinput-wine.git
     [user@host code]$cd koku-xinput-wine
     [user&host code]$cmake .
     [user@host code]$make
     
After this there will be a 'koku-xinput-wine.so' in the folder.

Usage
---------------------

     [user@host game]$export LD_PRELOAD=/lib-path/koku-xinput-wine.so
     [user@host game]$wine game.exe
     
Simple Elegance, without patching wine.
Of course it would be wiser to put this code into wine..

Config
---------------------

You can add sdl2 gamepad mappings by putting them in file named "gamecontrollerdb.txt" and place it next to your game executable, or via the *SDL_GAMECONTROLLERCONFIG* environment variable.

You can find a premade gamecontrollerdb.txt with a lot of mappings [here](https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt)

Building for 64 bit
---------------------
You can build a 64 bit version by using the following cmake instead of the above:

     [user&host code]$cmake -DBUILD_M32=OFF .

Note: the CoSetProxyBlanket hook is not compiled into this build so might not work in some cases where the 32 bit version would.

This is required for 64Bit games, if you're on 64Bit platform but running a 32Bit game you still need to compile as noted before.
