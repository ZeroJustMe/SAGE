"""
NVIDIA OpenAI API Wrapper
"""
import asyncio
import sys
import os
import logging
from typing import List, Optional, Dict, Any

from ..base_wrapper import BaseEmbeddingWrapper

try:
    from openai import OpenAI, AsyncOpenAI
    from openai.types import CreateEmbeddingResponse
except ImportError:
    raise ImportError(
        "openai package is required for NVIDIA OpenAI embedding functionality. "
        "Please install it via: pip install openai"
    )


class NvidiaOpenAIWrapper(BaseEmbeddingWrapper):
    """NVIDIA OpenAI API 包装器（使用 NIM 接口）"""
    
    def initialize(self):
        """初始化 NVIDIA OpenAI 客户端"""
        if self._initialized:
            return
        
        # 获取 API key（NVIDIA 使用 NVIDIA_API_KEY 环境变量）
        api_key = self.config.get("api_key") or os.getenv("NVIDIA_API_KEY")
        if not api_key:
            raise ValueError("NVIDIA API key is required (set NVIDIA_API_KEY environment variable)")
        
        # 设置环境变量
        os.environ["NVIDIA_API_KEY"] = api_key
        
        # 获取配置参数
        base_url = self.config.get("base_url", "https://integrate.api.nvidia.com/v1")
        timeout = self.config.get("timeout", 60)
        max_retries = self.config.get("max_retries", 3)
        
        # 设置默认请求头
        default_headers = {
            "User-Agent": "SAGE/0.0 NVIDIA-NIM",
            "Content-Type": "application/json",
        }
        
        # 合并自定义请求头
        custom_headers = self.config.get("headers", {})
        headers = {**default_headers, **custom_headers}
        
        try:
            # 创建同步客户端
            sync_client_config = {
                "api_key": api_key,
                "base_url": base_url,
                "default_headers": headers,
                "timeout": timeout,
                "max_retries": max_retries,
            }
            
            self.client = OpenAI(**sync_client_config)
            
            # 创建异步客户端
            async_client_config = sync_client_config.copy()
            self.async_client = AsyncOpenAI(**async_client_config)
            
            self._initialized = True
            
        except Exception as e:
            raise RuntimeError(f"Failed to initialize NVIDIA OpenAI client: {e}")
    
    def _prepare_extra_body(self) -> Dict[str, Any]:
        """准备 NVIDIA 特有的 extra_body 参数"""
        input_type = self.config.get("input_type", "passage")
        truncate = self.config.get("truncate", "NONE")
        
        return {
            "input_type": input_type,
            "truncate": truncate
        }
    
    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        return self.batch_embed([text])[0]
    
    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        try:
            encoding_format = self.config.get("encoding_format", "float")
            extra_body = self._prepare_extra_body()
            
            response: CreateEmbeddingResponse = self.client.embeddings.create(
                model=self.model_info.model_path,
                input=texts,
                encoding_format=encoding_format,
                extra_body=extra_body
            )
            
            # 提取 embeddings
            embeddings = [item.embedding for item in response.data]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"NVIDIA OpenAI embedding failed: {e}")
    
    async def async_embed(self, text: str) -> List[float]:
        """异步单个文本 embedding"""
        embeddings = await self.async_batch_embed([text])
        return embeddings[0]
    
    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        try:
            encoding_format = self.config.get("encoding_format", "float")
            extra_body = self._prepare_extra_body()
            
            response = await self.async_client.embeddings.create(
                model=self.model_info.model_path,
                input=texts,
                encoding_format=encoding_format,
                extra_body=extra_body
            )
            
            # 提取 embeddings
            embeddings = [item.embedding for item in response.data]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"NVIDIA OpenAI async embedding failed: {e}")
    
    def embed_with_input_type(self, texts: List[str], input_type: str) -> List[List[float]]:
        """使用特定输入类型生成 embeddings"""
        if not self._initialized:
            self.initialize()
        
        try:
            encoding_format = self.config.get("encoding_format", "float")
            truncate = self.config.get("truncate", "NONE")
            
            extra_body = {
                "input_type": input_type,
                "truncate": truncate
            }
            
            response: CreateEmbeddingResponse = self.client.embeddings.create(
                model=self.model_info.model_path,
                input=texts,
                encoding_format=encoding_format,
                extra_body=extra_body
            )
            
            embeddings = [item.embedding for item in response.data]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"NVIDIA OpenAI embedding with input_type failed: {e}")
    
    async def async_embed_with_input_type(self, texts: List[str], input_type: str) -> List[List[float]]:
        """异步使用特定输入类型生成 embeddings"""
        if not self._initialized:
            self.initialize()
        
        try:
            encoding_format = self.config.get("encoding_format", "float")
            truncate = self.config.get("truncate", "NONE")
            
            extra_body = {
                "input_type": input_type,
                "truncate": truncate
            }
            
            response = await self.async_client.embeddings.create(
                model=self.model_info.model_path,
                input=texts,
                encoding_format=encoding_format,
                extra_body=extra_body
            )
            
            embeddings = [item.embedding for item in response.data]
            return embeddings
            
        except Exception as e:
            raise RuntimeError(f"NVIDIA OpenAI async embedding with input_type failed: {e}")
    
    def embed_query(self, query: str) -> List[float]:
        """为查询生成 embedding（input_type="query"）"""
        return self.embed_with_input_type([query], "query")[0]
    
    def embed_passage(self, passage: str) -> List[float]:
        """为文档段落生成 embedding（input_type="passage"）"""
        return self.embed_with_input_type([passage], "passage")[0]
    
    async def async_embed_query(self, query: str) -> List[float]:
        """异步为查询生成 embedding"""
        embeddings = await self.async_embed_with_input_type([query], "query")
        return embeddings[0]
    
    async def async_embed_passage(self, passage: str) -> List[float]:
        """异步为文档段落生成 embedding"""
        embeddings = await self.async_embed_with_input_type([passage], "passage")
        return embeddings[0]
    