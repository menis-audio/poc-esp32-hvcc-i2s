import os
import shutil
import time
from typing import Optional

import jinja2
from hvcc.types.compiler import CompilerResp, ExternInfo, Generator
from hvcc.types.meta import Meta

def render_templates(project_name: str, out_dir: str, heavy_header: str, hv_new_fn: str,
                     ws_pin: int = 26, bclk_pin: int = 27, dout_pin: int = 25, sample_rate: int = 48000) -> None:
    base_dir = os.path.dirname(os.path.abspath(__file__))
    templates_dir = os.path.join(base_dir, 'c2espidf', 'templates')
    if not os.path.isdir(templates_dir):
        # fallback to sibling templates if running as a package module
        templates_dir = os.path.join(os.path.dirname(base_dir), 'c2espidf', 'templates')

    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(templates_dir),
        trim_blocks=True,
        lstrip_blocks=True,
    )

    # Root CMakeLists.txt
    root_cmake = env.get_template('root_CMakeLists.txt.j2').render(project_name=project_name)
    with open(os.path.join(out_dir, 'CMakeLists.txt'), 'w') as f:
        f.write(root_cmake)

    # main/CMakeLists.txt
    main_dir = os.path.join(out_dir, 'main')
    os.makedirs(main_dir, exist_ok=True)
    main_cmake = env.get_template('main_CMakeLists.txt.j2').render()
    with open(os.path.join(main_dir, 'CMakeLists.txt'), 'w') as f:
        f.write(main_cmake)

    # Wrapper C file
    wrapper = env.get_template('poc_esp32_hvcc_i2s.c.j2').render(
        heavy_header=heavy_header,
        hv_new_fn=hv_new_fn,
        ws_pin=ws_pin,
        bclk_pin=bclk_pin,
        dout_pin=dout_pin,
        sample_rate=sample_rate,
    )
    with open(os.path.join(main_dir, 'poc_esp32_hvcc_i2s.c'), 'w') as f:
        f.write(wrapper)

class c2espidf(Generator):
    @classmethod
    def compile(
        cls,
        c_src_dir: str,
        out_dir: str,
        externs: ExternInfo,
        patch_name: Optional[str] = None,
        patch_meta: Meta = Meta(),
        num_input_channels: int = 0,
        num_output_channels: int = 0,
        copyright: Optional[str] = None,
        verbose: Optional[bool] = False
    ) -> CompilerResp:
        t0 = time.time()
        project_name = (patch_name or "hvcc_esp32_audio").replace(" ", "_")

        main_dir = os.path.join(out_dir, "main")
        hvcc_c_dir = os.path.join(main_dir, "hvcc", "c")
        os.makedirs(hvcc_c_dir, exist_ok=True)

        # `c_src_dir` provided by HVCC already points to the C source folder.
        src_c_dir = c_src_dir
        if not os.path.isdir(src_c_dir):
            raise RuntimeError(f"HVCC c source directory not found: {src_c_dir}")

        for name in os.listdir(src_c_dir):
            src = os.path.join(src_c_dir, name)
            dst = os.path.join(hvcc_c_dir, name)
            if os.path.isfile(src):
                shutil.copy2(src, dst)

        # Determine Heavy header and init function
        heavy_header = "Heavy_heavy.h"
        hv_new_fn = "hv_heavy_new"
        for name in os.listdir(hvcc_c_dir):
            if name.startswith("Heavy_") and name.endswith(".h"):
                heavy_header = name
                base = name.replace("Heavy_", "").replace(".h", "")
                hv_new_fn = f"hv_{base}_new"
                break

        render_templates(project_name, out_dir, heavy_header, hv_new_fn)

        hv_msg = os.path.join(hvcc_c_dir, "HvMessage.c")
        if os.path.exists(hv_msg):
            with open(hv_msg, "r") as rf:
                s = rf.read()
            s = s.replace("\"0x%X\"", "\"0x%\" PRIX32")
            with open(hv_msg, "w") as wf:
                wf.write(s)

        hv_utils = os.path.join(hvcc_c_dir, "HvUtils.h")
        if os.path.exists(hv_utils):
            with open(hv_utils, "r") as rf:
                s = rf.read()
            if "#include <inttypes.h>" not in s and "#include <stdint.h>" in s:
                s = s.replace("#include <stdint.h>", "#include <stdint.h>\n#include <inttypes.h>")
            with open(hv_utils, "w") as wf:
                wf.write(s)

        t1 = time.time()
        return CompilerResp(
            stage='c2espidf',
            compile_time=t1 - t0,
            in_dir=c_src_dir,
            out_dir=out_dir
        )
