"""
Instructor API Wrapper
"""
import os
import asyncio
import numpy as np
from typing import List
from ..base_wrapper import BaseEmbeddingWrapper

try:
    # from InstructorEmbedding import INSTRUCTOR
    from sentence_transformers import SentenceTransformer
except ImportError:
    raise ImportError("InstructorEmbedding and sentence_transformers packages are required for Instructor functionality")


class InstructorWrapper(BaseEmbeddingWrapper):
    """Instructor 模型包装器"""
    
    def initialize(self):
        """初始化 Instructor 模型"""
        if self._initialized:
            return
        
        try:
            # 使用 SentenceTransformer 加载模型
            self.model = SentenceTransformer(self.model_info.model_path)
            self._initialized = True
        except Exception as e:
            raise RuntimeError(f"Failed to initialize Instructor model: {e}")
    
    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        return self.batch_embed([text])[0]
    
    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        try:
            # 使用模型编码文本
            embeddings = self.model.encode(texts)
            
            # 如果返回的是 numpy array，转换为 Python list
            if isinstance(embeddings, np.ndarray):
                if embeddings.ndim == 1:
                    # 单个向量
                    return [embeddings.tolist()]
                else:
                    # 多个向量
                    return embeddings.tolist()
            else:
                # 如果已经是列表格式
                return embeddings
                
        except Exception as e:
            raise RuntimeError(f"Instructor embedding failed: {e}")
    
    async def async_embed(self, text: str) -> List[float]:
        """异步单个文本 embedding"""
        embeddings = await self.async_batch_embed([text])
        return embeddings[0]
    
    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        try:
            # 在线程池中运行同步的编码操作
            loop = asyncio.get_event_loop()
            embeddings = await loop.run_in_executor(None, self.model.encode, texts)
            
            # 处理返回的 embeddings
            if isinstance(embeddings, np.ndarray):
                if embeddings.ndim == 1:
                    return [embeddings.tolist()]
                else:
                    return embeddings.tolist()
            else:
                return embeddings
                
        except Exception as e:
            raise RuntimeError(f"Instructor async embedding failed: {e}")

    def get_embedding_dimension(self) -> int:
        """获取 embedding 维度"""
        if not self._initialized:
            self.initialize()
        
        # 检查模型是否有维度信息
        if hasattr(self.model, 'get_sentence_embedding_dimension'):
            return self.model.get_sentence_embedding_dimension()
        else:
            # 使用测试文本来获取维度
            test_embedding = self.embed("test")
            return len(test_embedding)
    
    def validate_model(self) -> bool:
        """验证模型是否可用"""
        try:
            # 尝试获取一个简单的 embedding
            self.embed("test")
            return True
        except Exception:
            return False
    
    def get_max_sequence_length(self) -> int:
        """获取模型支持的最大序列长度"""
        if not self._initialized:
            self.initialize()
        
        if hasattr(self.model, 'max_seq_length'):
            return self.model.max_seq_length
        else:
            # 默认值，可以根据具体模型调整
            return 512