"""
更新后的 Embedding API
"""
from typing import List, Dict
from .manager import EmbeddingManager
from .base_wrapper import BaseEmbeddingWrapper

# 全局实例
_embedding_manager = EmbeddingManager()


def get_embedding_model(name: str = "bge-small-zh-v1.5", **kwargs) -> BaseEmbeddingWrapper:
    """
    获取 embedding 模型实例
    
    Args:
        name: 模型名称，如 "bge-small-zh-v1.5", "text-embedding-ada-002"
        **kwargs: 额外的配置参数
    
    Returns:
        BaseEmbeddingWrapper: embedding 模型实例
    
    Example:
        # 使用本地 HuggingFace 模型
        model = get_embedding_model("bge-small-zh-v1.5")
        with model:
            embedding = model.embed("hello world")
            embeddings = model.batch_embed(["hello", "world"])
        
        # 使用 OpenAI API
        model = get_embedding_model("text-embedding-ada-002", api_key="sk-xxx")
        with model:
            embeddings = model.batch_embed(["hello", "world"])
    """
    return _embedding_manager.get_embedding_model(name, **kwargs)


def list_embedding_models(available_only: bool = True) -> List[Dict]:
    """
    列出可用的 embedding 模型
    
    Args:
        available_only: 是否只返回可用的模型
    
    Returns:
        List[Dict]: 模型信息列表，包含 name, display_name, provider, dimension, 
                   type, is_available, is_cached, requires_api_key 等字段
    
    Example:
        models = list_embedding_models()
        for model in models:
            status = "✓" if model["is_available"] else "✗"
            cached = "(cached)" if model.get("is_cached") else ""
            print(f"{status} {model['name']}: {model['dimension']}d {cached}")
    """
    return _embedding_manager.list_embedding_models(available_only)


def refresh_model_availability():
    """刷新所有模型的可用状态"""
    _embedding_manager.refresh_availability()

