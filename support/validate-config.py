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
    "operating-mode-choice",
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
    assert value["configVersion"] == 6
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
    serial = profiles[("com.epicecu.programmor.adapter.serial", "serial")]
    assert serial == {
        "adapterId": "com.epicecu.programmor.adapter.serial",
        "connectionMethod": "serial",
        "settings": {
            "baudRate": 115200,
            "dataBits": 8,
            "parity": "none",
            "stopBits": 1,
            "flowControl": "none",
        },
        "openDelayMs": 0,
        "discovery": {"usb": [{"vendorId": 5824, "productId": 1155}]},
    }
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
    operating_mode = components_by_id["operating-mode-choice"]
    assert operating_mode["binding"] == "general.operating-mode"
    assert operating_mode["data"] == {
        "commit": "immediate",
        "options": [
            {"value": "OPERATING_MODE_OFF", "label": "Off"},
            {"value": "OPERATING_MODE_MANUAL", "label": "Manual"},
            {"value": "OPERATING_MODE_AUTOMATIC", "label": "Automatic"},
        ],
    }
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
    functions_by_id = {function["id"]: function for function in value["functions"]}

    def section_contract(function_id: str) -> tuple[list[dict], dict]:
        function_sections = functions_by_id[function_id]["views"][0]["sections"]
        function_placements = {
            section["id"]: {
                placement["component"]: placement["layout"]
                for placement in section["placements"]
            }
            for section in function_sections
        }
        return function_sections, function_placements

    sections, placements = section_contract("general")
    assert [(section["id"], section["name"]) for section in sections] == [
        ("items", "Values"),
        ("float-controls", "Float controls"),
        ("boolean-controls", "Boolean controls"),
        ("select-combobox", "Selection controls"),
    ]
    assert placements["items"] == {
        "double-number": {"column": 1, "row": 1, "columns": 3, "rows": 1},
        "ip-address": {"column": 4, "row": 1, "columns": 3, "rows": 1},
        "port-number": {"column": 7, "row": 1, "columns": 3, "rows": 1},
        "date-time": {"column": 10, "row": 1, "columns": 3, "rows": 1},
    }
    assert placements["float-controls"] == {
        "float-number": {"column": 1, "row": 1, "columns": 3, "rows": 1},
        "float-number-slider": {
            "column": 4,
            "row": 1,
            "columns": 5,
            "rows": 1,
        },
        "float-linear-gauge": {
            "column": 9,
            "row": 1,
            "columns": 4,
            "rows": 1,
        },
    }
    assert placements["boolean-controls"] == {
        "boolean-value": {"column": 1, "row": 1, "columns": 3, "rows": 1},
        "boolean-checkbox": {"column": 4, "row": 1, "columns": 3, "rows": 1},
        "set-boolean-true": {"column": 7, "row": 1, "columns": 3, "rows": 1},
        "boolean-status": {"column": 10, "row": 1, "columns": 3, "rows": 1},
    }
    assert placements["select-combobox"] == {
        "day-number": {"column": 1, "row": 1, "columns": 4, "rows": 1},
        "day-name": {"column": 5, "row": 1, "columns": 4, "rows": 1},
        "operating-mode-choice": {
            "column": 9,
            "row": 1,
            "columns": 4,
            "rows": 1,
        },
    }
    counter_sections, counter_placements = section_contract("counter")
    assert [(section["id"], section["name"]) for section in counter_sections] == [
        ("counter", "Counter"),
    ]
    assert counter_sections[0]["editMode"] == "form"
    assert counter_placements["counter"] == {
        "starting-number": {"column": 1, "row": 1, "columns": 3, "rows": 1},
        "ending-number": {"column": 4, "row": 1, "columns": 3, "rows": 1},
        "counter-value": {"column": 7, "row": 1, "columns": 3, "rows": 1},
        "counter-range": {"column": 1, "row": 2, "columns": 6, "rows": 2},
        "counter-status": {"column": 7, "row": 2, "columns": 3, "rows": 1},
    }
    string_sections, string_placements = section_contract("string")
    assert [(section["id"], section["name"]) for section in string_sections] == [
        ("string", "Message comparison")
    ]
    assert string_placements["string"] == {
        "welcome-message": {"column": 1, "row": 1, "columns": 4, "rows": 1},
        "welcome-message-label": {
            "column": 5,
            "row": 1,
            "columns": 4,
            "rows": 1,
        },
        "welcome-message-multiline": {
            "column": 1,
            "row": 2,
            "columns": 4,
            "rows": 2,
        },
        "developer-label": {"column": 5, "row": 2, "columns": 4, "rows": 1},
    }
    graph_sections, graph_placements = section_contract("graph")
    assert [(section["id"], section["name"]) for section in graph_sections] == [
        ("counter-history", "History"),
        ("counter-scope", "Oscilloscope"),
        ("input-scatter", "XY comparison"),
    ]
    assert graph_placements == {
        "counter-history": {
            "counter-graph": {"column": 1, "row": 1, "columns": 8, "rows": 5}
        },
        "counter-scope": {
            "counter-scope": {"column": 1, "row": 1, "columns": 8, "rows": 6}
        },
        "input-scatter": {
            "input-scatter": {"column": 1, "row": 1, "columns": 8, "rows": 5}
        },
    }
    lookup_sections, lookup_placements = section_contract("lookup-tables")
    assert [(section["id"], section["name"]) for section in lookup_sections] == [
        ("calibration", "1D table"),
        ("lookup-map", "2D table"),
    ]
    assert lookup_placements == {
        "calibration": {
            "example-curve": {"column": 1, "row": 1, "columns": 8, "rows": 3}
        },
        "lookup-map": {
            "example-map": {"column": 1, "row": 1, "columns": 8, "rows": 5}
        },
    }
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
    proto = TEST_PROTO.read_text(encoding="utf-8")
    assert 'import "table/v1/table.proto";' in proto
    assert TABLE_PROTO.is_file()
    print("Config 1001 repository contract passed.")


if __name__ == "__main__":
    main()
