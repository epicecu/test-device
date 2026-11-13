# Repository Guidelines

Use Taskfile commands as the supported interface. Keep firmware heap-free and
bounded, use Corelib for Programmor transport/session behaviour, and use Table
for curve/map validation, mutation and interpolation. Do not create competing
protocol or lookup-table implementations.

`device-config/` is managed through the Programmor CLI. Pull before editing and
run the authoritative Cloud check only from a trusted authenticated workstation.
Never add Cloud credentials to CI.

Generated Nanopb files under `firmware/src/generated` are regenerated with
`task generate`. Use Australian/UK English in human-readable prose and comments.
Run `task all` and `git diff --check` before handoff.
