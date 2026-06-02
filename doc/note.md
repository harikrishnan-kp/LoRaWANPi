## Note
- in lmic lib, changes are made in 
    - lmic/oslmic.c @ os_runloop
    - lmic/oslmic.h
    - lmic/Makefile

## python wrapper
```
cd python
make
```
Makefile builds python/lorawanpi/liblorawanpi.so That shared library is required by the Python wrapper APIs

## TODO
- Test in real hardware 