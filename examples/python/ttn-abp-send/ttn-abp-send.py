from __future__ import annotations

import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
PYTHON_PACKAGE_DIR = REPO_ROOT / "python"
if str(PYTHON_PACKAGE_DIR) not in sys.path:
    sys.path.insert(0, str(PYTHON_PACKAGE_DIR))

from lorawanpi import LoraWanPiError, encode_rain_payload, send_rain_abp


TXRX_ACK = 0x80
TXRX_NACK = 0x40
EV_TXCOMPLETE = 10


def usage(program: str) -> str:
    return f"Usage: {program} <DevAddr> <Nwkskey> <Appskey> <Data> <UseLeds>"


def parse_led_flag(value: str) -> bool:
    try:
        parsed = int(value)
    except ValueError as exc:
        raise ValueError("UseLeds must be 0 or 1") from exc

    if parsed not in (0, 1):
        raise ValueError("UseLeds must be 0 or 1")
    return bool(parsed)


def print_downlink(downlink: bytes, rssi_dbm: int, snr_db: float) -> None:
    print(f"RSSI: {rssi_dbm} dBm")
    print(f"SNR: {snr_db:g} dB")
    print("Data Received!")
    print(" ".join(f"0x{byte:02x}" for byte in downlink))


def main(argv: list[str]) -> int:
    if len(argv) != 6:
        print(usage(argv[0]), file=sys.stderr)
        return 1

    _, devaddr, nwkskey, appskey, data_str, leds_str = argv

    try:
        data = float(data_str)
        use_leds = parse_led_flag(leds_str)
    except ValueError as exc:
        print(exc, file=sys.stderr)
        print(usage(argv[0]), file=sys.stderr)
        return 1

    payload = encode_rain_payload(data)
    now = int(time.time())
    print(f"({now}) {time.ctime(now)}")
    print(f"Payload: {' '.join(f'0x{byte:02x}' for byte in payload)}")

    try:
        result = send_rain_abp(devaddr, nwkskey, appskey, data, use_leds=use_leds)
    except (ValueError, LoraWanPiError) as exc:
        print(exc, file=sys.stderr)
        return 1

    if result.event == EV_TXCOMPLETE:
        print(f"Event EV_TXCOMPLETE, time: {int(time.monotonic())}")
    else:
        print(f"Event {result.event}")

    if result.txrx_flags & TXRX_ACK or result.ack:
        print("Received ACK!")
    elif result.txrx_flags & TXRX_NACK or result.nack:
        print("No ACK received!")

    if result.downlink:
        print_downlink(result.downlink, result.rssi_dbm, result.snr_db)

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))


# TTN decode payload
#
# function Decode(fPort, bytes, variables) {
#   var decoded = {};
#   decoded.data = ((bytes[0] << 8) | bytes[1]) / 100.0;
#   return decoded;
# }
