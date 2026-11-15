# Make Config 1001 a Clear, Customisable Showcase

## Summary

Rewrite Config 1001 for visitors without automotive knowledge and expand its dashboard picker into a representative showcase of every supported component family. Preserve all stable IDs, bindings, protocol fields, function order, and firmware behaviour.

## Sidebar Copy

| Function ID | New name | New description |
| --- | --- | --- |
| `settings` | **Device overview** | Check that the test device is running and review its example wiring. |
| `counter` | **Counter demo** | Choose the start and end values, then watch the counter update live. |
| `string` | **Text controls** | Try single-line, multiline, and read-only text with one shared message. |
| `general` | **Input controls** | Try numbers, addresses, dates, switches, buttons, sliders, and dropdowns. |
| `graph` | **Counter history** | Watch counter values over time using graph and oscilloscope views. |
| `table` | **Editable records** | View, sort, and edit example records stored on the test device. |
| `hex-editor` | **Device memory** | Inspect and edit the device’s 64-byte memory in hex, binary, or ASCII. |
| `lookup-tables` | **Lookup tables** | Edit 1D or 2D tables. |

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

The picker remains an allowlist only; no components are automatically placed, and each selected component uses its existing preferred dashboard dimensions.

## Validation

- Preserve all existing uncommitted Config v3, firmware, support, and test changes.
- Validate that all 17 dashboard entries resolve uniquely and remain compatible with their layout capabilities.
- Stage Config 1001 through the App development-config workflow.
- Smoke-test adding compact, chart, table, memory, lookup, scope, and wiring components to a dashboard.
- Verify sidebar and picker copy at normal and narrow widths.
- Run the complete Test Device and App gates, followed by `programmor-cli check` without publishing Config 1001.

## Assumptions

- Config 1001 targets first-time users and developers without ECU knowledge.
- Existing saved dashboards remain valid because component and function IDs do not change.
- The unpublished version 3 draft does not require another version increment.
