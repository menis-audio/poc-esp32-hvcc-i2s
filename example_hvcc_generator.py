#!/usr/bin/env python3
import os
import sys
import argparse
import shutil
import subprocess

# Expose an alias HVCC external generator so `hvcc -G example_hvcc_generator` works.
try:
    from hvcc.types.compiler import CompilerResp, ExternInfo, Generator
    from hvcc.types.meta import Meta
    from c2espidf import c2espidf as _c2espidf

    class example_hvcc_generator(Generator):
        @classmethod
        def compile(
            cls,
            c_src_dir: str,
            out_dir: str,
            externs: ExternInfo,
            patch_name: str | None = None,
            patch_meta: Meta = Meta(),
            num_input_channels: int = 0,
            num_output_channels: int = 0,
            copyright: str | None = None,
            verbose: bool | None = False
        ) -> CompilerResp:
            return _c2espidf.compile(
                c_src_dir=c_src_dir,
                out_dir=out_dir,
                externs=externs,
                patch_name=patch_name,
                patch_meta=patch_meta,
                num_input_channels=num_input_channels,
                num_output_channels=num_output_channels,
                copyright=copyright,
                verbose=verbose,
            )
except Exception:
    # If hvcc isn't importable, keep CLI working; hvcc will import this
    # module in its own environment where hvcc is available.
    pass


def run(cmd, cwd=None, env=None):
    print("$", " ".join(cmd))
    try:
        subprocess.check_call(cmd, cwd=cwd, env=env)
    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}: {' '.join(cmd)}")
        sys.exit(e.returncode)


def main():
    parser = argparse.ArgumentParser(description="HVCC → ESP-IDF generator and optional build/flash")
    parser.add_argument("pd_patch", nargs="?", default="main/test.pd", help="Path to Pure Data patch")
    parser.add_argument("--out", "-o", default="generated/espidf_app", help="Output ESP-IDF project directory")
    parser.add_argument("--port", "-p", default=os.environ.get("ESPPORT", os.environ.get("PORT", "")), help="Serial port for flashing (e.g., /dev/ttyUSB0)")
    parser.add_argument("--target", default="esp32", help="ESP-IDF target (e.g., esp32)")
    args = parser.parse_args()

    # Ensure hvcc is available
    if shutil.which("hvcc") is None:
        print("Error: 'hvcc' not found on PATH. Install Heavy (HVCC) and ensure 'hvcc' is available.")
        sys.exit(1)

    out_dir = args.out
    os.makedirs(os.path.dirname(out_dir), exist_ok=True)
    if os.path.isdir(out_dir):
        shutil.rmtree(out_dir)

    # Let hvcc discover local generator module(s)
    env = os.environ.copy()
    env["PYTHONPATH"] = env.get("PYTHONPATH", "") + (":" if env.get("PYTHONPATH") else "") + os.getcwd()

    print(f"Generating ESP-IDF app via HVCC external generator -> {out_dir}")
    # You can use either alias name or the original module name:
    #   -G example_hvcc_generator  (this module)
    #   -G c2espidf               (original implementation)
    run(["hvcc", args.pd_patch, "-G", "example_hvcc_generator", "-o", out_dir], env=env)

    # Optional ESP-IDF build + flash if idf.py is available
    if shutil.which("idf.py") is None:
        print(f"idf.py not found. Project generated at {out_dir}.")
        print("To build:")
        print("  . \"$HOME/esp/esp-idf/export.sh\"")
        print(f"  cd {out_dir} && idf.py set-target {args.target} && idf.py build && idf.py -p {args.port or '/dev/ttyUSB0'} flash")
        sys.exit(0)

    # Build
    print("Setting ESP-IDF target:", args.target)
    run(["idf.py", "set-target", args.target], cwd=out_dir)
    print("Building firmware")
    run(["idf.py", "build"], cwd=out_dir)

    # Flash
    flash_cmd = ["idf.py", "flash"]
    if args.port:
        flash_cmd = ["idf.py", "-p", args.port, "flash"]
        print(f"Flashing to port {args.port}")
    else:
        print("Flashing (auto port)")
    run(flash_cmd, cwd=out_dir)
    print("Done. Optionally run: idf.py monitor")


if __name__ == "__main__":
    main()
