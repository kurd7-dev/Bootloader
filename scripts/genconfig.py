#!/usr/bin/env python3

import pathlib
import re
import sys


def parse_dot_config(path: pathlib.Path):
    out = {}
    for raw in path.read_text().splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if "=" not in line:
            continue
        k, v = line.split("=", 1)
        out[k.strip()] = v.strip()
    return out


def c_hex_or_int(v: str):
    if re.match(r"^0x[0-9a-fA-F]+$", v):
        return v + "ULL"
    return v


def main():
    if len(sys.argv) != 4:
        print("usage: genconfig.py <.config> <autoconf.h> <config.mk>")
        return 1

    cfg_path = pathlib.Path(sys.argv[1])
    out_h = pathlib.Path(sys.argv[2])
    out_mk = pathlib.Path(sys.argv[3])

    cfg = parse_dot_config(cfg_path)

    out_h.parent.mkdir(parents=True, exist_ok=True)
    out_mk.parent.mkdir(parents=True, exist_ok=True)

    lines_h = [
        "/* Auto-generated. Do not edit. */",
        "#ifndef __BL_AUTOCONF_H__",
        "#define __BL_AUTOCONF_H__",
        "",
    ]

    for k in sorted(cfg.keys()):
        v = cfg[k]
        if v == "y":
            lines_h.append(f"#define {k} 1")
        elif v == "n":
            lines_h.append(f"#define {k} 0")
        elif v.startswith('"') and v.endswith('"'):
            lines_h.append(f"#define {k} {v}")
        else:
            lines_h.append(f"#define {k} {c_hex_or_int(v)}")

    lines_h += ["", "#endif", ""]

    lines_mk = ["# Auto-generated. Do not edit."]
    for k in sorted(cfg.keys()):
        lines_mk.append(f"{k}:={cfg[k]}")
    lines_mk.append("")

    out_h.write_text("\n".join(lines_h))
    out_mk.write_text("\n".join(lines_mk))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())