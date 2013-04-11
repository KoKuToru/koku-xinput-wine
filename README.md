koku-xinput-wine
================

Adds xinput support to wine, without changing the source of wine.

Install
---------------------
If you are on 64Bit you will need 32Bit tool-chain (multilib)

Because this will generate 32Bit code, no 64Bit support !

It depens on SDL1.2-Librarys.
     
     [user@host code]$git clone https://github.com/KoKuToru/koku-xinput-wine
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
File at ~/.config/koku-xinput-wine.ini, gets auto generated at first usage.

     XINPUT_GAMEPAD_LEFT_THUMB_X    = A01
     XINPUT_GAMEPAD_LEFT_THUMB_Y    = A02*-1
     ...
     XINPUT_GAMEPAD_DPAD_UP         = H01&0x0001
     ...
     XINPUT_GAMEPAD_Y               = B04
    
### First config-parameter
     XINPUT_GAMEPAD_DPAD_UP
    
i thinks this parameters pretty much explain itself.. if you don't understand, feel free to ask me.

### Second config-parameter
     
The first letter stands for:
   
    A = Axis
    B = Buttons
    H = Hats
    
Follwing with a number, which explains the id.

     A10
     
 Axi with id 10.

### Third config-parameter (optional)
    
Starts with a '&', followed by a 16-Bit hex number.
Bit mask are useful for hats, hats return 1 value for 4 different directions.

    0x0001 = SDL_HAT_UP
    0x0002 = SDL_HAT_RIGHT
    0x0004 = SDL_HAT_DOWN
    0x0008 = SLD_HAT_LEFT
    
### Forth config-parameter (optional)

Starts with a '+', followed by a integer number.
Multiplys the result with the value.

    A02*-1
    
Flips the Axi Nr. 2.
