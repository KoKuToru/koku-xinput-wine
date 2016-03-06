`CoSetProxyBlanket`-Hack
=========

*Return a fake XInput-Controller.*

Pretty ugly hack.   
This hack only works when the application uses the example [code from msdn](https://msdn.microsoft.com/en-us/library/windows/desktop/ee417014(v=vs.85).aspx).

Usage
---------
Add it to your `LD_PRELOAD`
```
export LD_PRELOAD="$LD_REPLOAD;/lib-path/koku-xinput-cosetproxyblanket.so"
```

Why is it called `CoSetProxyBlanket`-Hack ?
---------
Because this hack uses `CoSetProxyBlanket` to manipulate the results..
