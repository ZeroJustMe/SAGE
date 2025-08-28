"""
OpenAI API Wrapper
"""
import asyncio
import sys
import os
import logging
from typing import List, Optional, Dict, Any

if sys.version_info < (3, 9):
    from typing import AsyncIterator
else:
    from collections.abc import AsyncIterator

from ..base_wrapper import BaseEmbeddingWrapper

try:
    from openai import OpenAI, AsyncOpenAI
    from openai.types import CreateEmbeddingResponse
except ImportError:
    raise ImportError(
        "openai package is required for OpenAI embedding functionality. "
        "Please install it via: pip install openai"
    )


class OpenAIWrapper(BaseEmbeddingWrapper):
    """OpenAI API 包装器"""
    
    def initialize(self):
        """初始化 OpenAI 客户端"""
        if self._initialized:
            return
        
        # 获取 API key
        api_key = self.config.get("api_key") or os.getenv("OPENAI_API_KEY")
        if not api_key:
            raise ValueError("OpenAI API key is required")
        
        # 获取配置参数
        base_url = self.config.get("base_url")
        timeout = self.config.get("timeout", 60)
        max_retries = self.config.get("max_retries", 3)
        
        # 设置默认请求头
        default_headers = {
            "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_8) SAGE/0.0",
            "Content-Type": "application/json",
        }
        
        # 合并自定义请求头
        custom_headers = self.config.get("headers", {})
        headers = {**default_headers, **custom_headers}
        
        try:
            # 创建同步客户端
            sync_client_config = {
                "api_key": api_key,
                "default_headers": headers,
                "timeout": timeout,
                "max_retries": max_retries,
            }
            if base_url:
                sync_client_config["base_url"] = base_url
            
            self.client = OpenAI(**sync_client_config)
            
            # 创建异步客户端
            async_client_config = sync_client_config.copy()
            self.async_client = AsyncOpenAI(**async_client_config)
            
            self._initialized = True
            
        except Exception as e:
            raise RuntimeError(f"Failed to initialize OpenAI client: {e}")
    
    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        return self.batch_embed([text])[0]
    
    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        try:
            # OpenAI API 支持批量请求
            response: CreateEmbeddingResponse = self.client.embeddings.create(
                model=self.model_info.model_path,
                input=texts,
                encoding_format="float"
            )
            
            # 提取 embeddings
            embeddings = [item.embedding for item in response.data]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"OpenAI embedding failed: {e}")
    
    async def async_embed(self, text: str) -> List[float]:
        """异步单个文本 embedding"""
        embeddings = await self.async_batch_embed([text])
        return embeddings[0]
    
    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        try:
            response = await self.async_client.embeddings.create(
                model=self.model_info.model_path,
                input=texts,
                encoding_format="float"
            )
            
            # 提取 embeddings
            embeddings = [item.embedding for item in response.data]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"OpenAI async embedding failed: {e}")