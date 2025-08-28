"""
Jina AI API Wrapper
"""
import asyncio
import os
import logging
from typing import List, Optional, Dict, Any

from ..base_wrapper import BaseEmbeddingWrapper

try:
    import tenacity
    from tenacity import (
        retry,
        stop_after_attempt,
        wait_exponential,
        retry_if_exception_type,
    )
except ImportError:
    raise ImportError(
        "tenacity package is required for Jina embedding functionality. "
        "Please install it via: pip install tenacity"
    )

try:
    import aiohttp
except ImportError:
    raise ImportError(
        "aiohttp package is required for Jina embedding functionality. "
        "Please install it via: pip install aiohttp"
    )

try:
    import requests
except ImportError:
    raise ImportError(
        "requests package is required for Jina embedding functionality. "
        "Please install it via: pip install requests"
    )

import numpy as np


class JinaError(Exception):
    """Generic error for issues related to Jina AI API"""


class JinaWrapper(BaseEmbeddingWrapper):
    """Jina AI API 包装器"""
    
    def initialize(self):
        """初始化 Jina AI 客户端"""
        if self._initialized:
            return
        
        # 获取 API key
        api_key = self.config.get("api_key") or os.getenv("JINA_API_KEY")
        if not api_key:
            raise ValueError("Jina API key is required (set JINA_API_KEY environment variable)")
        
        # 设置环境变量
        os.environ["JINA_API_KEY"] = api_key
        
        # 获取配置参数
        self.base_url = self.config.get("base_url", "https://api.jina.ai/v1/embeddings")
        self.timeout = self.config.get("timeout", 60)
        self.max_retries = self.config.get("max_retries", 3)
        
        # 准备请求头
        self.headers = {
            "Content-Type": "application/json",
            "Authorization": f"Bearer {api_key}",
        }
        
        # 合并自定义请求头
        custom_headers = self.config.get("headers", {})
        self.headers.update(custom_headers)
        
        self._initialized = True
    
    def _prepare_payload(self, texts: List[str]) -> Dict[str, Any]:
        """准备 API 请求载荷"""
        dimensions = self.config.get("dimensions", self.model_info.dimension)
        late_chunking = self.config.get("late_chunking", False)
        normalized = self.config.get("normalized", True)
        embedding_type = self.config.get("embedding_type", "float")
        
        # 如果只有一个文本，直接传字符串；多个文本传列表
        input_data = texts[0] if len(texts) == 1 else texts
        
        payload = {
            "model": self.model_info.model_path,
            "normalized": normalized,
            "embedding_type": embedding_type,
            "dimensions": dimensions,
            "late_chunking": late_chunking,
            "input": input_data,
        }
        
        return payload
    
    @retry(
        retry=retry_if_exception_type(Exception),
        wait=wait_exponential(multiplier=1, min=4, max=60),
        stop=stop_after_attempt(3)
    )
    async def _fetch_data_async(self, payload: Dict[str, Any]) -> List[Dict[str, Any]]:
        """异步获取数据（带重试）"""
        try:
            timeout = aiohttp.ClientTimeout(total=self.timeout)
            async with aiohttp.ClientSession(timeout=timeout) as session:
                async with session.post(self.base_url, headers=self.headers, json=payload) as response:
                    if response.status != 200:
                        error_text = await response.text()
                        raise JinaError(f"Jina API returned status {response.status}: {error_text}")
                    
                    response_json = await response.json()
                    data_list = response_json.get("data", [])
                    
                    if not data_list:
                        raise JinaError("No embedding data returned from Jina API")
                    
                    return data_list
                    
        except Exception as e:
            raise JinaError(f"Jina async API call failed: {e}")
    
    @retry(
        retry=retry_if_exception_type(Exception),
        wait=wait_exponential(multiplier=1, min=4, max=60),
        stop=stop_after_attempt(3)
    )
    def _fetch_data_sync(self, payload: Dict[str, Any]) -> List[Dict[str, Any]]:
        """同步获取数据（带重试）"""
        try:
            response = requests.post(
                self.base_url, 
                headers=self.headers, 
                json=payload,
                timeout=self.timeout
            )
            response.raise_for_status()
            
            data = response.json()
            data_list = data.get("data", [])
            
            if not data_list:
                raise JinaError("No embedding data returned from Jina API")
            
            return data_list
            
        except requests.exceptions.RequestException as e:
            raise JinaError(f"Jina sync API call failed: {e}")
    
    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        return self.batch_embed([text])[0]
    
    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        try:
            payload = self._prepare_payload(texts)
            data_list = self._fetch_data_sync(payload)
            
            # 提取 embeddings
            embeddings = [item["embedding"] for item in data_list]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"Jina embedding failed: {e}")
    
    async def async_embed(self, text: str) -> List[float]:
        """异步单个文本 embedding"""
        embeddings = await self.async_batch_embed([text])
        return embeddings[0]
    
    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        try:
            payload = self._prepare_payload(texts)
            data_list = await self._fetch_data_async(payload)
            
            # 提取 embeddings
            embeddings = [item["embedding"] for item in data_list]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"Jina async embedding failed: {e}")
    
    def embed_with_dimensions(self, texts: List[str], dimensions: int) -> List[List[float]]:
        """使用特定维度生成 embeddings"""
        if not self._initialized:
            self.initialize()
        
        try:
            # 临时修改配置
            original_dimensions = self.config.get("dimensions")
            self.config["dimensions"] = dimensions
            
            payload = self._prepare_payload(texts)
            data_list = self._fetch_data_sync(payload)
            
            # 恢复原始配置
            if original_dimensions is not None:
                self.config["dimensions"] = original_dimensions
            else:
                self.config.pop("dimensions", None)
            
            embeddings = [item["embedding"] for item in data_list]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"Jina embedding with custom dimensions failed: {e}")
    
    async def async_embed_with_dimensions(self, texts: List[str], dimensions: int) -> List[List[float]]:
        """异步使用特定维度生成 embeddings"""
        if not self._initialized:
            self.initialize()
        
        try:
            # 临时修改配置
            original_dimensions = self.config.get("dimensions")
            self.config["dimensions"] = dimensions
            
            payload = self._prepare_payload(texts)
            data_list = await self._fetch_data_async(payload)
            
            # 恢复原始配置
            if original_dimensions is not None:
                self.config["dimensions"] = original_dimensions
            else:
                self.config.pop("dimensions", None)
            
            embeddings = [item["embedding"] for item in data_list]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"Jina async embedding with custom dimensions failed: {e}")
    
    def embed_with_late_chunking(self, texts: List[str], late_chunking: bool = True) -> List[List[float]]:
        """使用 late chunking 生成 embeddings"""
        if not self._initialized:
            self.initialize()
        
        try:
            # 临时修改配置
            original_late_chunking = self.config.get("late_chunking")
            self.config["late_chunking"] = late_chunking
            
            payload = self._prepare_payload(texts)
            data_list = self._fetch_data_sync(payload)
            
            # 恢复原始配置
            self.config["late_chunking"] = original_late_chunking
            
            embeddings = [item["embedding"] for item in data_list]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"Jina embedding with late chunking failed: {e}")
    
    async def async_embed_with_late_chunking(self, texts: List[str], late_chunking: bool = True) -> List[List[float]]:
        """异步使用 late chunking 生成 embeddings"""
        if not self._initialized:
            self.initialize()
        
        try:
            # 临时修改配置
            original_late_chunking = self.config.get("late_chunking")
            self.config["late_chunking"] = late_chunking
            
            payload = self._prepare_payload(texts)
            data_list = await self._fetch_data_async(payload)
            
            # 恢复原始配置
            self.config["late_chunking"] = original_late_chunking
            
            embeddings = [item["embedding"] for item in data_list]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"Jina async embedding with late chunking failed: {e}")
    