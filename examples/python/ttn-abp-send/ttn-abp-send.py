from __future__ import annotations

import argparse
import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from lorawanpi import LoRaWAN, RadioConfig


def parse_led_flag(value: str) -> bool:
    try:
        parsed = int(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("UseLeds must be 0 or 1") from exc

    if parsed not in (0, 1):
        raise argparse.ArgumentTypeError("UseLeds must be 0 or 1")
    return bool(parsed)


def print_downlink(downlink_payload: bytes, radio_rssi: int | None, radio_snr: float | None) -> None:
    print(f"Downlink payload: {downlink_payload.hex()}")
    if radio_rssi is not None:
        print(f"RSSI: {radio_rssi} dBm")
    if radio_snr is not None:
        print(f"SNR: {radio_snr:g} dB")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Send an ABP uplink using LoRaWANPi")
    parser.add_argument("devaddr", help="Device address (hex)")
    parser.add_argument("nwkskey", help="Network session key (hex)")
    parser.add_argument("appskey", help="Application session key (hex)")
    parser.add_argument("data", help="Data payload (text)")
    parser.add_argument("use_leds", type=parse_led_flag, help="0 or 1 to enable LEDs")
    return parser.parse_args()


def main(args: argparse.Namespace) -> int:
    payload = args.data.encode("utf-8")
    use_leds = args.use_leds

    now = int(time.time())
    print(f"({now}) {time.ctime(now)}")
    print(f"Payload: {payload.hex()}")

    lora = LoRaWAN(radio=RadioConfig(use_leds=use_leds))
    lora.configure_abp(devaddr=args.devaddr, nwkskey=args.nwkskey, appskey=args.appskey)

    try:
        result = lora.send(payload, port=1, confirmed=False, timeout=30.0)
    except Exception as exc:
        print(exc, file=sys.stderr)
        return 1

    if result.ack:
        print("Received ACK!")
    else:
        print("No ACK received.")

    if result.downlink:
        print_downlink(result.downlink.payload, result.downlink.radio.rssi_dbm, result.downlink.radio.snr_db)

    return 0


if __name__ == "__main__":
    raise SystemExit(main(parse_args()))
