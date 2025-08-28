"""
Lollms API Wrapper - Updated Version
"""
import os
import asyncio
from typing import List
from ..base_wrapper import BaseEmbeddingWrapper

try:
    import aiohttp
    import requests
except ImportError:
    raise ImportError("aiohttp and requests packages are required for Lollms API functionality")


class LollmsWrapper(BaseEmbeddingWrapper):
    """Lollms API 包装器"""
    
    def initialize(self):
        """初始化 Lollms 客户端配置"""
        if self._initialized:
            return
        
        self.base_url = self.config.get("base_url", "http://localhost:9600")
        self.api_key = self.config.get("api_key") or os.getenv("LOLLMS_API_KEY")
        
        # 准备请求头
        self.headers = {"Content-Type": "application/json"}
        if self.api_key:
            self.headers["Authorization"] = self.api_key
        
        self.endpoint = f"{self.base_url}/lollms_embed"
        self._initialized = True
    
    def _extract_embedding(self, response_data) -> List[float]:
        """从 Lollms 响应中提取 embedding"""
        try:
            if "vector" not in response_data:
                raise ValueError("Response does not contain vector field")
            
            embedding = response_data["vector"]
            if not isinstance(embedding, list):
                raise ValueError("Vector field is not a list")
            
            return embedding
            
        except Exception as e:
            raise RuntimeError(f"Failed to extract embedding from response: {e}")
    
    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        request_data = {"text": text}
        
        try:
            response = requests.post(
                self.endpoint, 
                json=request_data, 
                headers=self.headers
            )
            response.raise_for_status()
            result = response.json()
            
            return self._extract_embedding(result)
            
        except requests.RequestException as e:
            raise RuntimeError(f"Lollms API request failed: {e}")
        except Exception as e:
            raise RuntimeError(f"Lollms embedding failed: {e}")
    
    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量文本 embedding - 逐个处理（Lollms API 不支持批量）"""
        if not self._initialized:
            self.initialize()
        
        embeddings = []
        for text in texts:
            embedding = self.embed(text)
            embeddings.append(embedding)
        
        return embeddings
    
    async def async_embed(self, text: str) -> List[float]:
        """异步单个文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        request_data = {"text": text}
        
        try:
            async with aiohttp.ClientSession(headers=self.headers) as session:
                async with session.post(self.endpoint, json=request_data) as response:
                    if not response.ok:
                        error_text = await response.text()
                        raise aiohttp.ClientResponseError(
                            request_info=response.request_info,
                            history=response.history,
                            status=response.status,
                            message=error_text
                        )
                    
                    result = await response.json()
                    return self._extract_embedding(result)
                    
        except aiohttp.ClientError as e:
            raise RuntimeError(f"Lollms async API request failed: {e}")
        except Exception as e:
            raise RuntimeError(f"Lollms async embedding failed: {e}")
    
    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量文本 embedding - 并发处理"""
        if not self._initialized:
            self.initialize()
        
        # 使用 asyncio.gather 并发处理多个请求
        tasks = [self.async_embed(text) for text in texts]
        embeddings = await asyncio.gather(*tasks)
        
        return embeddings