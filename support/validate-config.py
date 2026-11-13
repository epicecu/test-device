#!/usr/bin/env python3
"""Check the public, repository-owned invariants before authoritative Cloud validation."""

from pathlib import Path

import yaml


ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "device-config" / "device-config.yaml"
TEST_PROTO = ROOT / "device-config" / "protobuf" / "test.proto"
TABLE_PROTO = ROOT / "device-config" / "protobuf" / "table" / "v1" / "table.proto"


def main() -> None:
    value = yaml.safe_load(CONFIG.read_text(encoding="utf-8"))
    assert value["formatVersion"] == 1
    assert value["interfaceVersion"] == 1
    assert value["configId"] == 1001
    assert value["configVersion"] == 2
    assert value["metadata"]["repository"] == "https://github.com/epicecu/test-device"
    profiles = {
        (profile["adapterId"], profile["connectionMethod"]): profile
        for profile in value["adapterProfiles"]
    }
    assert ("com.epicecu.programmor.adapter.test", "test") in profiles
    hid = profiles[("com.epicecu.programmor.adapter.hid", "usb-hid")]
    assert hid["matches"] == [{"vendorId": 5824, "productId": 1158}]
    shares = {model["shareId"] for model in value["dataModels"]}
    assert shares == set(range(1, 10))
    proto = TEST_PROTO.read_text(encoding="utf-8")
    assert 'import "table/v1/table.proto";' in proto
    assert TABLE_PROTO.is_file()
    print("Config 1001 repository contract passed.")


if __name__ == "__main__":
    main()
