"""
SAGE Flow - C++ Stream Processing Engine with Python Bindings
高性能数据流处理模块

This module provides Python bindings for the SAGE Flow C++ stream processing engine.
"""

import sys
import os
from pathlib import Path

# 版本信息
__version__ = "0.2.0"
__author__ = "IntelliStream"

def _find_sage_flow_module():
    """查找sage_flow_datastream模块"""
    # 当前目录下的build目录
    current_dir = Path(__file__).parent
    build_dir = current_dir / "build"
    
    if build_dir.exists():
        # 将build目录添加到Python路径
        sys.path.insert(0, str(build_dir))
        try:
            import sage_flow_datastream
            return sage_flow_datastream
        except ImportError:
            pass
    
    # 尝试直接导入（已安装的情况）
    try:
        import sage_flow_datastream
        return sage_flow_datastream
    except ImportError as e:
        raise ImportError(
            f"Cannot import sage_flow_datastream module. "
            f"Please ensure it's built and installed. Error: {e}"
        ) from e

# 导入C++模块
try:
    _datastream_module = _find_sage_flow_module()
    
    # 导出主要类和函数
    Environment = _datastream_module.Environment
    DataStream = _datastream_module.DataStream
    MultiModalMessage = _datastream_module.MultiModalMessage
    create_text_message = _datastream_module.create_text_message
    create_binary_message = _datastream_module.create_binary_message
    
    # 导出操作符相关
    Operator = _datastream_module.Operator
    SinkFunction = _datastream_module.SinkFunction
    TerminalSinkOperator = _datastream_module.TerminalSinkOperator
    FileSinkOperator = _datastream_module.FileSinkOperator
    VectorStoreSinkOperator = _datastream_module.VectorStoreSinkOperator
    
    # 导出配置相关
    EnvironmentConfig = _datastream_module.EnvironmentConfig
    FileSinkConfig = _datastream_module.FileSinkConfig
    VectorStoreConfig = _datastream_module.VectorStoreConfig
    
    # 导出工厂函数
    CreateTerminalSink = _datastream_module.CreateTerminalSink
    CreateFileSink = _datastream_module.CreateFileSink
    CreateVectorStoreSink = _datastream_module.CreateVectorStoreSink
    
    # 导出枚举
    ContentType = _datastream_module.ContentType
    OperatorType = _datastream_module.OperatorType
    FileFormat = _datastream_module.FileFormat
    VectorDataType = _datastream_module.VectorDataType
    VectorData = _datastream_module.VectorData
    
    print(f"✅ SAGE Flow v{__version__} loaded successfully")
    
except ImportError as e:
    print(f"❌ Failed to load SAGE Flow: {e}")
    raise

def main():
    """SAGE Flow命令行入口"""
    print(f"SAGE Flow v{__version__}")
    print("高性能数据流处理引擎")
    print("\n可用组件:")
    
    components = [
        "Environment - 执行环境",
        "DataStream - 数据流API", 
        "MultiModalMessage - 多模态消息",
        "Operators - 数据处理操作符",
        "Sinks - 数据输出组件"
    ]
    
    for comp in components:
        print(f"  • {comp}")
    
    print(f"\n使用方法:")
    print(f"  import sage_flow as sf")
    print(f"  env = sf.Environment('my_job')")
    print(f"  stream = env.create_datastream()")

if __name__ == "__main__":
    main()