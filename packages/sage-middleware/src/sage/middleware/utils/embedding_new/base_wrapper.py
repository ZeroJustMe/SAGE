"""
Embedding Wrapper 基类和接口定义
"""
import asyncio
from abc import ABC, abstractmethod
from typing import List, Union, Optional, Any
from .types import EmbeddingModelInfo


class BaseEmbeddingWrapper(ABC):
    """Embedding Wrapper 基类"""
    
    def __init__(self, model_info: EmbeddingModelInfo, **kwargs):
        self.model_info = model_info
        self.config = {**model_info.config, **kwargs}
        self._initialized = False
    
    @abstractmethod
    def initialize(self):
        """初始化模型"""
        pass
    
    @abstractmethod
    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        pass
    
    @abstractmethod
    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量文本 embedding"""
        pass
    
    async def async_embed(self, text: str) -> List[float]:
        """异步单个文本 embedding"""
        return await asyncio.get_event_loop().run_in_executor(None, self.embed, text)
    
    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量文本 embedding"""
        return await asyncio.get_event_loop().run_in_executor(None, self.batch_embed, texts)
    
    def get_dimension(self) -> int:
        """获取向量维度"""
        return self.model_info.dimension
    
    def get_max_tokens(self) -> int:
        """获取最大 token 数"""
        return self.model_info.max_tokens
    
    def __enter__(self):
        if not self._initialized:
            self.initialize()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        pass