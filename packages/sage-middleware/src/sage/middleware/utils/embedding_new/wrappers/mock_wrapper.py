"""
Mock Embedding Wrapper for testing
"""
import random
from typing import List
from ..base_wrapper import BaseEmbeddingWrapper


class MockWrapper(BaseEmbeddingWrapper):
    """Mock 测试用包装器"""
    
    def initialize(self):
        """初始化 Mock 模型"""
        if self._initialized:
            return
        
        self.dimension = self.model_info.dimension
        self._initialized = True
    
    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        # 生成固定的随机向量（基于文本内容确保一致性）
        random.seed(hash(text) % (2**32))
        return [random.uniform(-1, 1) for _ in range(self.dimension)]
    
    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量文本 embedding"""
        return [self.embed(text) for text in texts]