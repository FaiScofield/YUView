#!/usr/bin/env python3
import os
import sys
import xml.etree.ElementTree as ET
import json
import re
from pathlib import Path

# 预定义 MSBuild 宏（可根据实际环境扩展）
DEFAULT_MACROS = {
    "Platform": "x64",
    "Configuration": "Debug",
    "WindowsSdkDir": os.environ.get("WindowsSdkDir", "C:\\Program Files (x86)\\Windows Kits\\10\\"),
    "VCInstallDir": os.environ.get("VCInstallDir", ""),
}

def expand_macros(text, macros):
    if not text:
        return ""
    for k, v in macros.items():
        text = text.replace(f" $ ({k})", v)
    # 移除未识别的  $ (...)（保守处理）
    text = re.sub(r'\ $   $ [^)]* $  ', '', text)
    return text

def parse_cl_compile(item_group, proj_dir, macros):
    commands = []
    for cl_compile in item_group.findall(".//{http://schemas.microsoft.com/developer/msbuild/2003}ClCompile"):
        file_path = cl_compile.get("Include")
        if not file_path:
            continue
        abs_file = (proj_dir / file_path).resolve()
        if not abs_file.exists():
            continue

        # 获取编译选项（简单合并）
        options = []
        for elem in cl_compile:
            tag = elem.tag.split("}")[-1]
            if tag in ("PreprocessorDefinitions", "AdditionalIncludeDirectories"):
                val = expand_macros(elem.text, macros)
                if val:
                    prefix = "/D" if tag == "PreprocessorDefinitions" else "/I"
                    for part in val.split(";"):
                        if part.strip():
                            options.append(prefix + part.strip())

        # 基础命令（可根据需要添加更多标志）
        command = ["cl.exe", "/c"] + options + [str(abs_file)]
        commands.append({
            "directory": str(proj_dir),
            "command": " ".join(command),
            "file": str(abs_file)
        })
    return commands

def main():
    if len(sys.argv) < 2:
        print("Usage: python vcxproj_to_compile_commands.py <project.vcxproj> [output.json]")
        sys.exit(1)

    proj_path = Path(sys.argv[1]).resolve()
    if not proj_path.exists():
        print(f"Error: {proj_path} not found.")
        sys.exit(1)

    output_path = sys.argv[2] if len(sys.argv) > 2 else "compile_commands.json"
    proj_dir = proj_path.parent

    # 注册命名空间（避免 {ns} 前缀）
    ET.register_namespace("", "http://schemas.microsoft.com/developer/msbuild/2003")

    tree = ET.parse(proj_path)
    root = tree.getroot()

    # 使用默认宏（可从环境或命令行扩展）
    macros = DEFAULT_MACROS.copy()

    all_commands = []
    for item_group in root.findall(".//{http://schemas.microsoft.com/developer/msbuild/2003}ItemGroup"):
        all_commands.extend(parse_cl_compile(item_group, proj_dir, macros))

    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(all_commands, f, indent=2)

    print(f"Generated {len(all_commands)} entries to {output_path}")

if __name__ == "__main__":
    main()