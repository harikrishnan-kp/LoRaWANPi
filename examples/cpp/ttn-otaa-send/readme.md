## Build
```
# Access the C++ OTAA example folder
$ cd examples/cpp/ttn-otaa-send

# Make the project
$ make

# Running the program
$ ./ttn-otaa-send
```

## How to run
```bash
./ttn-otaa-send <DevEUI> <AppEUI> <AppKey> <Rain_mm> <LED_FLAG>

# Example
./ttn-otaa-send 1122334455667788 0011223344556677 8F2C1D4E5F6A7B8C9D0E1F2A3B4C5D6E 3.12 1
```

This example demonstrates TTN OTAA join followed by a single uplink on port 1 with a fixed rain payload.
