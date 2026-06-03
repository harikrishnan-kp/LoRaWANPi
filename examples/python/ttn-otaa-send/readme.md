## Build
```
# Build the Python wrapper library
$ make
```

## How to run
```bash
python3 examples/python/ttn-otaa-send/ttn-otaa-send.py <DevEUI> <AppEUI> <AppKey> <Data> <UseLeds>

# Example
python3 examples/python/ttn-otaa-send/ttn-otaa-send.py 1122334455667788 0011223344556677 8F2C1D4E5F6A7B8C9D0E1F2A3B4C5D6E hello 1
```

Arguments are positional: `<DevEUI> <AppEUI> <AppKey> <Data> <UseLeds>`.

This example performs a TTN OTAA join, then sends an application payload on port 1.

## TTN Payload Decoder

Add this JavaScript decoder to The Things Network to display the uplink as text and hex.

```javascript
// Decode UTF-8 text and provide hex representation
function Decode(fPort, bytes) {
  var decoded = {};
  try {
    var chars = [];
    for (var i = 0; i < bytes.length; i++) chars.push(String.fromCharCode(bytes[i]));
    decoded.text = chars.join('');
  } catch (e) {
    decoded.text = null;
  }
  decoded.hex = bytes.map(function(b){ return (b < 16 ? '0' : '') + b.toString(16); }).join('');
  return decoded;
}
```
