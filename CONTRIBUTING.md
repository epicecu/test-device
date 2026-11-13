# Contributing

Use a branch and keep firmware changes, Config 1001 changes and tests in the
same review when they describe one behaviour. Run `task all` and
`git diff --check` before opening a pull request.

Do not commit credentials, Cloud tokens, private source archives, hardware
identifiers, generated build output, or `.programmor-conflicts/`.

Generated Nanopb sources are committed for reproducible embedded builds. Edit
the `.proto` or `.options` source and run `task generate`; do not edit generated
files directly.

Cloud synchronisation is an operator action from a trusted workstation. Public
CI must remain credential-free.
