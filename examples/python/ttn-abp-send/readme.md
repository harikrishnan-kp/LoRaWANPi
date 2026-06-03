## Build
```
# Build the Python wrapper library
$ make
```

## How to run
```bash
python3 examples/python/ttn-abp-send/ttn-abp-send.py AB0096CD 1A2B80150C4ED6DADA2B2CFD822C6378 12345678972908DA7A6C09771181A21C "hello" 1
```

Arguments are positional: `<DevAddr> <Nwkskey> <Appskey> <Data> <UseLeds>`.

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
