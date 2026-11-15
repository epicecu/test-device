#!/usr/bin/env python3
"""Check the public, repository-owned invariants before authoritative Cloud validation."""

import re
from pathlib import Path

import yaml


ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "device-config" / "device-config.yaml"
TEST_PROTO = ROOT / "device-config" / "protobuf" / "test.proto"
TABLE_PROTO = ROOT / "device-config" / "protobuf" / "table" / "v1" / "table.proto"
DEVICE_HEADER = ROOT / "firmware" / "include" / "test_device.h"


def main() -> None:
    value = yaml.safe_load(CONFIG.read_text(encoding="utf-8"))
    assert value["formatVersion"] == 1
    assert value["interfaceVersion"] == 1
    assert value["configId"] == 1001
    assert value["configVersion"] == 3
    header = DEVICE_HEADER.read_text(encoding="utf-8")
    version_match = re.search(
        r"PROGRAMMOR_TEST_DEVICE_CONFIG_VERSION\s*=\s*(\d+)", header
    )
    assert version_match is not None
    assert int(version_match.group(1)) == value["configVersion"]
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
    components = {component["type"] for component in value["components"]}
    assert {"hex-editor", "table", "lookup-table"}.issubset(components)
    lookup_tables = next(model for model in value["dataModels"] if model["shareId"] == 9)
    lookup_dimensions = {
        2 if "y_axis" in item["sample"] else 1
        for item in lookup_tables["values"]
        if item["type"] == "lookup-table"
    }
    assert lookup_dimensions == {1, 2}
    proto = TEST_PROTO.read_text(encoding="utf-8")
    assert 'import "table/v1/table.proto";' in proto
    assert TABLE_PROTO.is_file()
    print("Config 1001 repository contract passed.")


if __name__ == "__main__":
    main()
