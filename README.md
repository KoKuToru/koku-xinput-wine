koku-xinput-wine
================

Adds xinput support to wine, without changing the source of wine.  
Modified to use SDL2 gamepad mappings.

Install
---------------------
If you are on 64Bit you will need a 32Bit tool-chain (multilib).

You will also need SDL2 libraries and development headers installed.  On 64Bit systems you will need both 32Bit and 64Bit SDL2 libraries.

     [user@host code]$ git clone https://github.com/KoKuToru/koku-xinput-wine.git
     [user@host code]$ cd koku-xinput-wine
     [user&host code]$ cmake .
     [user@host code]$ make

After this there will be a 'koku-xinput-wine.so' in the folder, and on 64Bit systems there will also be 'koku-xinput-wine64.so'.

Usage
---------------------

To hook a 32Bit application:

     [user@host game]$ export LD_PRELOAD=/lib-path/koku-xinput-wine.so
     [user@host game]$ wine game.exe

To hook a 64Bit application:

     [user@host game]$ export LD_PRELOAD=/lib-path/koku-xinput-wine64.so
     [user@host game]$ wine game64.exe

To hook both 32Bit and 64Bit applications (the correct hook will be automatically selected):

     [user@host game]$ export LD_PRELOAD="/lib-path/koku-xinput-wine.so /lib-path/koku-xinput-wine64.so"
     [user@host game]$ wine game.exe
     [user@host game]$ wine game64.exe

Simple Elegance, without patching wine.
Of course it would be wiser to put this code into wine..

Config
---------------------

You can add SDL2 gamepad mappings by putting them in file named "gamecontrollerdb.txt" and place it next to your game executable, or via the *SDL_GAMECONTROLLERCONFIG* environment variable.

You can find a premade gamecontrollerdb.txt with a lot of mappings [here](https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt).

These mappings are not needed if you're already using a standard Xbox controller.

Troubleshooting
---------------------

When the library is properly loaded, you should notice two quick rumbles sent to the controller.

When running on a 64bit system, you may see errors like this:

     ERROR: ld.so: object '/lib-path/koku-xinput-wine.so' from LD_PRELOAD cannot be preloaded (wrong ELF class: ELFCLASS32): ignored.
     ERROR: ld.so: object '/lib-path/koku-xinput-wine64.so' from LD_PRELOAD cannot be preloaded (wrong ELF class: ELFCLASS64): ignored.

These messages don't necessarily mean there is a problem.  They're just saying that the 64Bit hooks aren't installed into 32Bit applications and vica-versa.  If you're seeing these errors and not getting the "welcome" rumbles, double-check that you're preloading the correct library.
