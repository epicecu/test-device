# Config 1001 Version 4 Showcase

## Summary

Keep Config 1001 concise for visitors without automotive knowledge and demonstrate the supported component families. Preserve stable IDs, bindings, protocol fields, function order, and firmware behaviour.

## Sidebar Copy

| Function ID | New name | New description |
| --- | --- | --- |
| `settings` | **Device overview** | Runtime status and wiring. |
| `counter` | **Counter demo** | Set bounds and watch the count. |
| `string` | **Text controls** | Edit and compare shared text. |
| `general` | **Input controls** | Try common values and controls. |
| `graph` | **Data plots** | Compare graph, scope, and scatter views. |
| `table` | **Editable records** | Sort and edit device records. |
| `hex-editor` | **Device memory** | Inspect and edit device bytes. |
| `lookup-tables` | **Lookup tables** | Edit 1D and 2D tables. |

## Neutral Lookup Presentation

- Rename the section to **Lookup examples**.
- Rename the components to **1D lookup table** and **2D lookup table**.
- Use **Input**, **Input X**, **Input Y**, and **Output** for visible axes and values.
- Explain the live markers without RPM, load, fuel, calibration, or operating-point terminology.
- Update help and accessibility names consistently.
- Preserve internal `engine-speed` and `load` bindings, Protobuf fields, data, and tracer behaviour.

## Dashboard Components

Keep the existing three entries and expand `dashboard.allowedComponents` to this representative set:

- Existing: `counter-gauge`, `loops-gauge`, `counter-value`.
- Compact controls: `welcome-message-label`, `day-name`, `boolean-checkbox`, `set-boolean-true`, `float-number-slider`, `counter-status`, `welcome-message-multiline`.
- Complex controls: `memory-editor`, `device-connections`, `example-table`, `counter-graph`, `counter-scope`, `example-curve`, `example-map`.

Use clearer picker labels where needed:

- `welcome-message-label`: **Welcome message (read-only)**
- `day-name`: **Day selector**
- `example-table`: **Editable records**
- Lookup components use their new neutral names.

Add `boolean-choice`, `float-linear-gauge`, `counter-range`, and `input-scatter` to the allowlist. The picker remains an allowlist only; no components are automatically placed.

## New Components

- Place Segmented Choice and Linear Gauge on Input controls.
- Place Range Slider on Counter demo.
- Place Scatter Plot after the graph and oscilloscope on Data plots.
- Reuse existing bindings so the controls demonstrate shared canonical values without Protobuf changes.

## Validation

- Retain immutable Config 1001 versions 2 and 3 as history fixtures.
- Validate that all 21 dashboard entries resolve uniquely and remain compatible with their layout capabilities.
- Stage and publish Config 1001 version 4 through the Cloud release workflow.
- Smoke-test the four new component types and existing representative components.
- Verify sidebar and picker copy at normal and narrow widths.
- Run the complete Test Device, Device Config, adapter, App, documentation, Cloud, CLI and WWW gates.

## Assumptions

- Config 1001 targets first-time users and developers without ECU knowledge.
- Existing saved dashboards remain valid because component and function IDs do not change.
- Published version 3 remains immutable; these changes use version 4.
