"""
Cohere API Wrapper - Fixed Version
"""
import os
import asyncio
from typing import List
from ..base_wrapper import BaseEmbeddingWrapper

try:
    import cohere
except ImportError:
    raise ImportError("cohere package is required for Cohere API functionality")


class CohereWrapper(BaseEmbeddingWrapper):
    """Cohere API 包装器"""
    
    def initialize(self):
        """初始化 Cohere 客户端"""
        if self._initialized:
            return
        
        api_key = self.config.get("api_key") or os.getenv("COHERE_API_KEY")
        if not api_key:
            raise ValueError("Cohere API key is required")
        
        self.client = cohere.Client(api_key=api_key)
        self.async_client = cohere.AsyncClient(api_key=api_key)
        self._initialized = True
    
    def _extract_embeddings(self, response) -> List[List[float]]:
        """从 Cohere 响应中提取 embeddings"""
        try:
            # 新版本 Cohere API 的响应结构
            if hasattr(response, 'embeddings'):
                embeddings = response.embeddings
                
                # 检查 embeddings 的类型和结构
                if hasattr(embeddings, 'float'):
                    # 如果是 EmbedByTypeResponseEmbeddings 对象
                    return embeddings.float
                elif isinstance(embeddings, list):
                    # 如果直接是列表
                    return embeddings
                else:
                    # 尝试转换为列表
                    return list(embeddings)
            else:
                raise AttributeError("Response does not contain embeddings")
        except Exception as e:
            raise RuntimeError(f"Failed to extract embeddings from response: {e}")
    
    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        return self.batch_embed([text])[0]
    
    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        try:
            response = self.client.embed(
                texts=texts,
                model=self.model_info.model_path,
                input_type=self.config.get("input_type", "classification"),
                # 移除 embedding_types 参数，使用默认值
            )
            
            return self._extract_embeddings(response)
            
        except Exception as e:
            # 如果上面的方法失败，尝试旧版本的 API 调用
            try:
                response = self.client.embed(
                    texts=texts,
                    model=self.model_info.model_path,
                    input_type=self.config.get("input_type", "classification"),
                    embedding_types=["float"]  # 明确指定 float 类型
                )
                return self._extract_embeddings(response)
            except Exception as e2:
                raise RuntimeError(f"Cohere embedding failed: {e2}")
    
    async def async_embed(self, text: str) -> List[float]:
        """异步单个文本 embedding"""
        embeddings = await self.async_batch_embed([text])
        return embeddings[0]
    
    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        try:
            response = await self.async_client.embed(
                texts=texts,
                model=self.model_info.model_path,
                input_type=self.config.get("input_type", "classification"),
                # 移除 embedding_types 参数，使用默认值
            )
            
            return self._extract_embeddings(response)
            
        except Exception as e:
            # 如果上面的方法失败，尝试旧版本的 API 调用
            try:
                response = await self.async_client.embed(
                    texts=texts,
                    model=self.model_info.model_path,
                    input_type=self.config.get("input_type", "classification"),
                    embedding_types=["float"]  # 明确指定 float 类型
                )
                return self._extract_embeddings(response)
            except Exception as e2:
                raise RuntimeError(f"Cohere async embedding failed: {e2}")

    def get_embedding_dimension(self) -> int:
        """获取 embedding 维度"""
        if not self._initialized:
            self.initialize()
        
        # 使用一个简单的测试文本来获取维度
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