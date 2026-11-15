# Cloud-Owned Device Config with Environment-Safe Development

## Summary

Programmor Cloud is the authoritative source for a project's Device Config.
Developers check that source out into a temporary local working folder, edit it
with normal tools or a coding agent, validate the exact snapshot in Cloud, and
explicitly push accepted work back to Cloud. Git is optional local review and
release evidence; it is not a second writable authority.

Config 1001 is the deliberate public exception: `test-device/device-config/`
remains a reviewed example, firmware input and local App fixture. It must be
reconciled with the intended Cloud revision before a firmware release. Its Git
tag records what the firmware was built against, but does not publish or
override the Cloud configuration.

## Developer Workflow

1. Select a named CLI environment: `local`, `testing`, or `production`.
   Production is the zero-configuration default. Local and Testing require a
   complete process-only endpoint, issuer and client profile.
2. Run `programmor-cli workspace checkout <project-id> [directory]` to create a
   missing or empty working folder safely. Existing files are never silently
   replaced.
3. Run `programmor-cli status` to see the environment, source digests,
   synchronisation state, conflicts and recommended next action.
4. Edit only the managed Device Config files. `programmor-cli check` performs
   bounded transient Cloud validation and does not save source, create a
   revision or change the active development build.
5. Run `programmor-cli push` only after review. Push saves one generation,
   creates and validates an immutable revision, builds an unsigned development
   package and atomically advances the active development build only after all
   stages succeed.
6. Use `programmor-cli project start-next-version <project-id>` when development
   moves beyond the latest signed version. Cloud selects the smallest version
   greater than every signed or published version. It does not rewrite local
   source; the developer updates and checks the source explicitly.

Pulls remain transactional and preserve divergent local work as conflict
copies. Credentials, state and caches are bound to the named environment.
Repository content cannot select the Cloud or OAuth issuer receiving a bearer
token.

## Cloud and App Behaviour

- A project has at most one selected development version and one active
  development package.
- A signed version remains consumed even if its publication is abandoned.
- Only a ready, unsigned package for the exact validated revision can become
  active. A failure leaves the previous active package unchanged.
- The App reauthorises project access before returning an active development
  package. Development packages are excluded from the ordinary catalogue and
  released-configuration resolution.
- A future positive Config version is valid only on the explicit development
  path. Normal public acquisition continues to use signed published versions.
- App environment labels remain visible. Source development tasks are:
  - `task dev`: Local Cloud plus the sibling Config 1001 example.
  - `task dev:test-device`: Local Cloud with the physical Test Device.
  - `task dev:testing`: Testing Cloud development packages.
  - `task dev:production`: Production Cloud development packages with analytics
    and crash reporting disabled for the source-built App.

## Repository Responsibilities

- `programmor-documentation-private` owns the requirements, authority model,
  security boundaries and environment workflow.
- `programmor-cloud` owns shared source, revision validation, version selection,
  unsigned development packages and the active-build pointer.
- `programmor-cli` owns safe checkout, pull, status, transient check and durable
  push UX, including stable redacted JSON and JSONL contracts.
- `programmor` owns environment-labelled App development modes and local Config
  1001 fixture loading.
- `programmor-www` exposes the next-version action and documents the Cloud-owned
  CLI workflow.
- `programmor-deploy` owns Local `/cli/v1` routing and generation of a matched,
  ignored CLI environment profile. It never writes credentials into a repo.
- `test-device` owns the public Config 1001 example and firmware reconciliation
  evidence. The online authority remains
  [Config 1001 on Programmor](https://www.programmor.com/configurations/1001).
- `programmor-adapters-private` keeps the Test adapter aligned with Config 1001
  version 3 during development.
- `programmor-device-config` continues to own language validation and package
  construction, not Cloud workflow policy.

## Acceptance

- Complete local repository gates pass, including CLI security and output
  regression tests.
- Clean worktrees on `dev1.h` run secret-dependent Cloud, WWW and Deploy gates;
  dirty primary Cloud and Deploy worktrees are never used.
- Testing acceptance covers environment isolation, checkout, no-op status and
  pull, invalid transient check, durable push, App acquisition, next-version
  selection, stale-version denial, idempotent retries, conflicts and redaction.
- Production receives a separate reviewed activation and controlled smoke test.
  No CLI release tag or Config publication is created as part of this plan.

