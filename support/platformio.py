Import("env")

from pathlib import Path


libdeps = Path(env.subst("$PROJECT_LIBDEPS_DIR")) / env.subst("$PIOENV")
table = libdeps / "Table"
corelib = libdeps / "Corelib"
table_nanopb = table / "extras" / "nanopb"
corelib_nanopb = corelib / "src" / "vendor" / "nanopb"

required = (
    table / "src" / "Table.h",
    table_nanopb / "table" / "nanopb.c",
    table_nanopb / "table" / "v1" / "table.pb.c",
    corelib / "src" / "Corelib.h",
    corelib_nanopb / "pb.h",
)
missing = [str(path) for path in required if not path.is_file()]
if missing:
    raise RuntimeError("Pinned firmware dependencies are incomplete: " + ", ".join(missing))

env.Append(
    CPPPATH=[
        str(table / "src"),
        str(table_nanopb),
        str(corelib / "src"),
        str(corelib_nanopb),
    ]
)
env.BuildSources(
    str(Path(env.subst("$BUILD_DIR")) / "table-nanopb"),
    str(table_nanopb / "table"),
)
