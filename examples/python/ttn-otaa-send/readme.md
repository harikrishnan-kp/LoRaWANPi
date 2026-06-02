## Build
```
# Build the Python wrapper library
$ cd python
$ make
```

## How to run
```bash
python3 examples/python/ttn-otaa-send/ttn-otaa-send.py <DevEUI> <AppEUI> <AppKey> <Rain_mm> <UseLeds>

# Example
python3 examples/python/ttn-otaa-send/ttn-otaa-send.py 1122334455667788 0011223344556677 8F2C1D4E5F6A7B8C9D0E1F2A3B4C5D6E 3.12 1
```

This example performs a TTN OTAA join, then sends a rain payload on port 1.
