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

SHOWCASE_DESCRIPTION = "Explore Programmor components and workflows."

SHOWCASE_FUNCTIONS = [
    (
        "settings",
        "Device overview",
        "Runtime status and wiring.",
    ),
    (
        "counter",
        "Counter demo",
        "Set bounds and watch the count.",
    ),
    (
        "string",
        "Text controls",
        "Edit and compare shared text.",
    ),
    (
        "general",
        "Input controls",
        "Try common values and controls.",
    ),
    (
        "graph",
        "Data plots",
        "Compare graph, scope, and scatter views.",
    ),
    (
        "table",
        "Editable records",
        "Sort and edit device records.",
    ),
    (
        "hex-editor",
        "Device memory",
        "Inspect and edit device bytes.",
    ),
    (
        "lookup-tables",
        "Lookup tables",
        "Edit 1D and 2D tables.",
    ),
]

DASHBOARD_COMPONENTS = [
    "counter-gauge",
    "loops-gauge",
    "counter-value",
    "welcome-message-label",
    "day-name",
    "boolean-checkbox",
    "set-boolean-true",
    "float-number-slider",
    "counter-status",
    "welcome-message-multiline",
    "memory-editor",
    "device-connections",
    "example-table",
    "counter-graph",
    "counter-scope",
    "example-curve",
    "example-map",
    "boolean-choice",
    "float-linear-gauge",
    "counter-range",
    "input-scatter",
]

SHOWCASE_COMPONENT_LABELS = {
    "welcome-message-label": "Welcome message (read-only)",
    "day-name": "Day selector",
    "example-table": "Editable records",
    "example-curve": "1D lookup table",
    "example-map": "2D lookup table",
}


def main() -> None:
    value = yaml.safe_load(CONFIG.read_text(encoding="utf-8"))
    assert value["formatVersion"] == 1
    assert value["interfaceVersion"] == 1
    assert value["configId"] == 1001
    assert value["configVersion"] == 4
    header = DEVICE_HEADER.read_text(encoding="utf-8")
    version_match = re.search(
        r"PROGRAMMOR_TEST_DEVICE_CONFIG_VERSION\s*=\s*(\d+)", header
    )
    assert version_match is not None
    assert int(version_match.group(1)) == value["configVersion"]
    assert value["metadata"]["repository"] == "https://github.com/epicecu/test-device"
    assert value["metadata"]["description"] == SHOWCASE_DESCRIPTION
    profiles = {
        (profile["adapterId"], profile["connectionMethod"]): profile
        for profile in value["adapterProfiles"]
    }
    assert ("com.epicecu.programmor.adapter.test", "test") in profiles
    hid = profiles[("com.epicecu.programmor.adapter.hid", "usb-hid")]
    assert hid["matches"] == [{"vendorId": 5824, "productId": 1158}]
    shares = {model["shareId"] for model in value["dataModels"]}
    assert shares == set(range(1, 10))
    component_list = value["components"]
    component_ids = [component["id"] for component in component_list]
    assert len(component_ids) == len(set(component_ids))
    components = {component["type"] for component in component_list}
    assert {
        "hex-editor",
        "table",
        "lookup-table",
        "segmented-choice",
        "linear-gauge",
        "range-slider",
        "scatter-plot",
    }.issubset(components)
    components_by_id = {component["id"]: component for component in component_list}
    assert value["dashboard"] == {
        "enabled": True,
        "allowedComponents": DASHBOARD_COMPONENTS,
    }
    assert len(DASHBOARD_COMPONENTS) == len(set(DASHBOARD_COMPONENTS)) == 21
    assert all(component_id in components_by_id for component_id in DASHBOARD_COMPONENTS)
    assert {
        component_id: components_by_id[component_id]["label"]
        for component_id in SHOWCASE_COMPONENT_LABELS
    } == SHOWCASE_COMPONENT_LABELS
    assert [
        (function["id"], function["name"], function["description"])
        for function in value["functions"]
    ] == SHOWCASE_FUNCTIONS
    lookup_tables = next(model for model in value["dataModels"] if model["shareId"] == 9)
    lookup_dimensions = {
        2 if "y_axis" in item["sample"] else 1
        for item in lookup_tables["values"]
        if item["type"] == "lookup-table"
    }
    assert lookup_dimensions == {1, 2}
    curve = components_by_id["example-curve"]
    assert curve["help"] == (
        "Edit a one-dimensional lookup table while the live marker shows the current input."
    )
    assert curve["accessibility"]["name"] == "1D lookup table"
    assert curve["data"]["xAxis"] == {
        "label": "Input",
        "formatter": "integer-plain",
        "editable": True,
        "displayPosition": "top",
    }
    assert curve["data"]["cells"] == {
        "label": "Output",
        "formatter": "integer-plain",
        "editable": True,
    }
    assert curve["data"]["xTracer"] == "lookup-tables.engine-speed"
    lookup_map = components_by_id["example-map"]
    assert lookup_map["help"] == (
        "Edit a two-dimensional lookup table while the live marker shows the current "
        "X and Y inputs."
    )
    assert lookup_map["accessibility"]["name"] == "2D lookup table"
    assert lookup_map["data"]["xAxis"] == {
        "label": "Input X",
        "formatter": "integer-plain",
        "editable": True,
    }
    assert lookup_map["data"]["yAxis"] == {
        "label": "Input Y",
        "formatter": "integer-plain",
        "editable": True,
        "displayOrder": "high-to-low",
    }
    assert lookup_map["data"]["cells"] == {
        "label": "Output",
        "formatter": "lookup-decimal",
        "editable": True,
    }
    assert lookup_map["data"]["xTracer"] == "lookup-tables.engine-speed"
    assert lookup_map["data"]["yTracer"] == "lookup-tables.load"
    lookup_function = next(
        function for function in value["functions"] if function["id"] == "lookup-tables"
    )
    assert lookup_function["views"][0]["sections"][0]["name"] == "Lookup examples"
    proto = TEST_PROTO.read_text(encoding="utf-8")
    assert 'import "table/v1/table.proto";' in proto
    assert TABLE_PROTO.is_file()
    print("Config 1001 repository contract passed.")


if __name__ == "__main__":
    main()
