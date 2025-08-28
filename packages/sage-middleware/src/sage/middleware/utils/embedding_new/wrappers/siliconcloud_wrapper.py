"""
SiliconCloud API Wrapper - Updated Version
"""
import os
import asyncio
import base64
import struct
from typing import List
from ..base_wrapper import BaseEmbeddingWrapper

try:
    import aiohttp
    import requests
except ImportError:
    raise ImportError("aiohttp and requests packages are required for SiliconCloud API functionality")


class SiliconCloudWrapper(BaseEmbeddingWrapper):
    """SiliconCloud API 包装器"""
    
    def initialize(self):
        """初始化 SiliconCloud 客户端配置"""
        if self._initialized:
            return
        
        api_key = self.config.get("api_key") or os.getenv("SILICONCLOUD_API_KEY")
        if not api_key:
            raise ValueError("SiliconCloud API key is required")
        
        # 确保 API key 格式正确
        if not api_key.startswith("Bearer "):
            api_key = "Bearer " + api_key
        
        self.api_key = api_key
        self.base_url = self.config.get("base_url", "https://api.siliconflow.cn/v1/embeddings")
        self.max_token_size = self.config.get("max_token_size", 512)
        self._initialized = True
    
    def _decode_base64_embedding(self, base64_string: str) -> List[float]:
        """解码 base64 格式的 embedding"""
        try:
            decode_bytes = base64.b64decode(base64_string)
            n = len(decode_bytes) // 4
            float_array = struct.unpack("<" + "f" * n, decode_bytes)
            return list(float_array)
        except Exception as e:
            raise RuntimeError(f"Failed to decode base64 embedding: {e}")
    
    def _extract_embeddings(self, response_data) -> List[List[float]]:
        """从 SiliconCloud 响应中提取 embeddings"""
        try:
            if "code" in response_data:
                raise ValueError(f"SiliconCloud API error: {response_data}")
            
            if "data" not in response_data or not response_data["data"]:
                raise ValueError("Response does not contain embedding data")
            
            embeddings = []
            for item in response_data["data"]:
                if "embedding" not in item:
                    raise ValueError("Response item does not contain embedding")
                
                base64_string = item["embedding"]
                embedding = self._decode_base64_embedding(base64_string)
                embeddings.append(embedding)
            
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"Failed to extract embeddings from response: {e}")
    
    def _prepare_texts(self, texts: List[str]) -> List[str]:
        """预处理文本，截断过长的文本"""
        return [text[:self.max_token_size] for text in texts]
    
    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        return self.batch_embed([text])[0]
    
    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        processed_texts = self._prepare_texts(texts)
        
        headers = {
            "Authorization": self.api_key,
            "Content-Type": "application/json",
        }
        
        payload = {
            "model": self.model_info.model_path,
            "input": processed_texts,
            "encoding_format": "base64",
        }
        
        try:
            response = requests.post(self.base_url, headers=headers, json=payload)
            response.raise_for_status()
            content = response.json()
            
            return self._extract_embeddings(content)
            
        except requests.RequestException as e:
            raise RuntimeError(f"SiliconCloud API request failed: {e}")
        except Exception as e:
            raise RuntimeError(f"SiliconCloud embedding failed: {e}")
    
    async def async_embed(self, text: str) -> List[float]:
        """异步单个文本 embedding"""
        embeddings = await self.async_batch_embed([text])
        return embeddings[0]
    
    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        processed_texts = self._prepare_texts(texts)
        
        headers = {
            "Authorization": self.api_key,
            "Content-Type": "application/json",
        }
        
        payload = {
            "model": self.model_info.model_path,
            "input": processed_texts,
            "encoding_format": "base64",
        }
        
        try:
            async with aiohttp.ClientSession() as session:
                async with session.post(self.base_url, headers=headers, json=payload) as response:
                    if not response.ok:
                        error_text = await response.text()
                        raise aiohttp.ClientResponseError(
                            request_info=response.request_info,
                            history=response.history,
                            status=response.status,
                            message=error_text
                        )
                    
                    content = await response.json()
                    return self._extract_embeddings(content)
                    
        except aiohttp.ClientError as e:
            raise RuntimeError(f"SiliconCloud async API request failed: {e}")
        except Exception as e:
            raise RuntimeError(f"SiliconCloud async embedding failed: {e}")