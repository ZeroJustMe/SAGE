#!/usr/bin/env python3
"""
SAGE Flow Package Setup
专门用于sage_flow模块的打包脚本
"""

from setuptools import setup, find_packages
import shutil
from pathlib import Path

def copy_built_extensions():
    """复制已构建的C++扩展"""
    current_dir = Path(__file__).parent
    build_dir = current_dir / "build"
    
    if not build_dir.exists():
        print("⚠️ build目录不存在，请先运行CMake构建")
        return False
    
    # 查找.so文件
    so_files = list(build_dir.glob("*.so"))
    if not so_files:
        print("⚠️ 未找到.so文件，请先构建C++扩展")
        return False
    
    # 复制到当前目录
    for so_file in so_files:
        dest = current_dir / so_file.name
        if dest.exists():
            dest.unlink()
        shutil.copy2(so_file, dest)
        print(f"✅ 复制扩展: {so_file.name}")
    
    return True

# 在打包前复制扩展文件
print("📦 准备 SAGE Flow 包...")
if not copy_built_extensions():
    print("❌ 无法复制C++扩展，构建可能失败")

setup(
    name="sage-flow",
    version="0.2.0",
    author="IntelliStream",
    author_email="intellistream@outlook.com",
    description="SAGE Flow - High-Performance Stream Processing Engine",
    long_description="""
SAGE Flow是一个高性能的C++数据流处理引擎，提供Python绑定。

特性：
- 高性能C++核心引擎
- 简洁的Python API
- 多模态消息处理
- 灵活的操作符系统
- 流式数据处理

安装后可以通过以下方式使用：
```python
import sage_flow as sf
env = sf.Environment("my_job")
stream = env.create_datastream()
```
    """.strip(),
    long_description_content_type="text/plain",
    
    # 包配置
    packages=["sage_flow"],
    package_dir={"sage_flow": "."},
    
    # 包含C++扩展文件
    package_data={
        "sage_flow": ["*.so"],
    },
    include_package_data=True,
    
    # 依赖
    install_requires=[
        "pybind11>=2.10.0",
    ],
    
    python_requires=">=3.11",
    
    # 分类
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: Apache Software License",
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
        "Topic :: Scientific/Engineering",
    ],
    
    # 不要压缩zip，因为包含二进制文件
    zip_safe=False,
    
    # 入口点
    entry_points={
        "console_scripts": [
            "sage-flow=sage_flow:main",
        ],
    },
)
