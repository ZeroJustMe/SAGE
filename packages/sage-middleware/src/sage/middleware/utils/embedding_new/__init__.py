"""
Embedding methods and models for SAGE middleware.

重构后的 embedding 模块提供统一的接口管理各种 embedding 模型。

主要特性：
- 统一的模型管理和注册
- 支持批量 embedding 操作  
- 本地模型缓存状态检测
- 多种 embedding 服务提供商支持
- 简化的 API 接口
- 异步操作支持
"""

from .embedding_api import (
    get_embedding_model,
    list_embedding_models, 
    refresh_model_availability,
)
from .base_wrapper import BaseEmbeddingWrapper
from .types import ModelType, EmbeddingModelInfo
from .manager import EmbeddingManager
from .factory import EmbeddingFactory

__all__ = [
    # 主要 API
    'get_embedding_model',
    'list_embedding_models',
    'refresh_model_availability',
    
    # 核心类
    'BaseEmbeddingWrapper',
    'EmbeddingModelInfo',
    'ModelType',
    'EmbeddingManager',
    'EmbeddingFactory',
]

__version__ = "2.0.0"