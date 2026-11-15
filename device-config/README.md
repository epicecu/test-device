# Config 1001 example source

This folder is the public, editable Device Config used by the Test Device
firmware and by `task dev` in a sibling Programmor App checkout. It is kept in
Git as a practical example for firmware developers, code generation, review and
release evidence.

Programmor Cloud remains the authoritative online source for Config 1001.
[View Config 1001 on Programmor](https://www.programmor.com/configurations/1001).
The published page may describe an earlier released version while this folder
contains the next in-development version.

Editing or merging this folder does not update Cloud automatically. To work on
the Cloud version, use a trusted authenticated workstation:

```sh
programmor-cli pull
programmor-cli check
programmor-cli push
```

`check` validates the local snapshot without changing Cloud. `push` is the
explicit durable action: it saves the managed source, creates and validates an
immutable revision, builds the unsigned development package and makes that
package available to authorised App Developer mode after every step succeeds.
The CLI manages only `device-config.yaml` and `protobuf/**/*.proto`.

Before a firmware release, reconcile this example with the intended Cloud
revision and verify byte-for-byte identity of every managed file. A Git tag is
release evidence; it does not replace Cloud authority or publish a config.
