# Programmor Test Device

The Programmor Test Device is the official open-source firmware for exercising
the Programmor App with a real microcontroller. It runs on a Teensy 4.1,
connects through USB RawHID, identifies as Configuration 1001, and implements
all nine demonstration Shares.

The repository is also the Git source for the Test Device project in Programmor
Cloud. `programmor.yaml` links the `device-config/` directory to that project;
the CLI synchronises only the Device Config source and never touches firmware,
tests, Git history, or build output.

## Supported hardware

- PJRC Teensy 4.1
- USB RawHID `16c0:0486`
- Programmor Config 1001, released version 1

Pico, USB Serial and Ethernet are not supported by firmware `v0.1.0`.

## Flash a release

1. Download the `.hex` and matching `.sha256` from the GitHub Release.
2. Verify it with `sha256sum --check <file>.sha256`.
3. Open the official PJRC Teensy Loader, select the `.hex`, connect a Teensy
   4.1 and program it. Press the board's PROGRAM button if the loader asks.

The `.hex` is data and does not need executable permissions. Linux users must
install PJRC's Teensy udev rules before the loader can access the board.
Reflashing a newer release is the update process; firmware `v0.1.0` does not
self-update.

After boot, three short LED flashes indicate successful initialisation. A
repeating seven- or eight-flash code indicates model or Corelib setup failure.

## Build and upload

The supported entry point is [Task](https://taskfile.dev/). Tooling and
microcontroller dependencies are pinned locally.

```sh
task setup
task all
task upload
```

`task upload` uses PlatformIO's headless Teensy loader. The Teensy may require
one physical PROGRAM-button press when first entering loader mode.

The firmware integrates:

- [EpicECU Corelib](https://github.com/epicecu/corelib) for PFP, Transaction
  Protocol v2, sessions, subscriptions and bounded transport state.
- [EpicECU Table](https://github.com/epicecu/table) for validated curve/map
  storage, interpolation and atomic Nanopb snapshots.

Neither protocol handling nor lookup-table validation is reimplemented here.

## Test

```sh
task test
task build
```

Native tests exercise every Share and invalid-update preservation. The physical
adapter acceptance additionally requires the private adapter development
workspace and a connected, freshly programmed Teensy:

```sh
task e2e PROGRAMMOR_ADAPTERS_DIR=../programmor-adapters-private
```

## Synchronise Config 1001

Production Cloud is the shared workspace; Git remains the review and history
boundary. Sign in with the Programmor CLI, then use this sequence:

```sh
programmor-cli project status
programmor-cli pull
# Review and edit device-config/ and firmware together.
git diff
programmor-cli check
```

Always pull before editing when Cloud may have changed. The CLI preserves
conflicts instead of merging them. `programmor-cli check` uploads the committed
source snapshot, creates an immutable revision and runs authoritative Cloud
validation. It does not publish Config 1001.

Public CI performs repository identity, Protobuf and schema-provenance checks.
It intentionally has no user OAuth credentials and cannot push to Cloud.

## Version identity

Firmware SemVer is stored in `VERSION`. Common1 encodes it as
`major * 1,000,000 + minor * 1,000 + patch`; release `v0.1.0` therefore reports
`1000`. Common1 continues to report `shares_version = 1` until Config 1001
version 2 is separately reviewed and published.

Device state is held in RAM and returns to documented defaults after reset.
This firmware is for Programmor testing and demonstration, not production ECU
control.
