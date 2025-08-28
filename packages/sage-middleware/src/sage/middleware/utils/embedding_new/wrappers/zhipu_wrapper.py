"""
ZhipuAI API Wrapper - Updated Version
"""
import os
import asyncio
from typing import List
from ..base_wrapper import BaseEmbeddingWrapper

try:
    from zhipuai import ZhipuAI
except ImportError:
    raise ImportError("zhipuai package is required for ZhipuAI API functionality")


class ZhipuAIWrapper(BaseEmbeddingWrapper):
    """ZhipuAI API 包装器"""
    
    def initialize(self):
        """初始化 ZhipuAI 客户端"""
        if self._initialized:
            return
        
        api_key = self.config.get("api_key") or os.getenv("ZHIPUAI_API_KEY")
        if not api_key:
            raise ValueError("ZhipuAI API key is required")
        
        self.client = ZhipuAI(api_key=api_key)
        self._initialized = True
    
    def _extract_embeddings(self, response) -> List[List[float]]:
        """从 ZhipuAI 响应中提取 embeddings"""
        try:
            if hasattr(response, 'data') and response.data:
                embeddings = []
                for item in response.data:
                    if hasattr(item, 'embedding'):
                        embeddings.append(item.embedding)
                    else:
                        raise AttributeError("Response item does not contain embedding")
                return embeddings
            else:
                raise AttributeError("Response does not contain data")
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
            response = self.client.embeddings.create(
                model=self.model_info.model_path,
                input=texts,
                **{k: v for k, v in self.config.items() 
                   if k not in ['api_key', 'model_path']}
            )
            
            return self._extract_embeddings(response)
            
        except Exception as e:
            raise RuntimeError(f"ZhipuAI embedding failed: {e}")
    
    async def async_embed(self, text: str) -> List[float]:
        """异步单个文本 embedding"""
        embeddings = await self.async_batch_embed([text])
        return embeddings[0]
    
    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        try:
            # ZhipuAI SDK 目前没有原生异步支持，使用线程池执行
            loop = asyncio.get_event_loop()
            response = await loop.run_in_executor(
                None,
                lambda: self.client.embeddings.create(
                    model=self.model_info.model_path,
                    input=texts,
                    **{k: v for k, v in self.config.items() 
                       if k not in ['api_key', 'model_path']}
                )
            )
            
            return self._extract_embeddings(response)
            
        except Exception as e:
            raise RuntimeError(f"ZhipuAI async embedding failed: {e}")
