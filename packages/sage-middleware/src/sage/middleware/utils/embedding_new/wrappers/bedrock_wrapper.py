"""
AWS Bedrock API Wrapper
"""
import copy
import os
import json
import asyncio
import logging
from typing import List, Optional, Dict, Any

from ..base_wrapper import BaseEmbeddingWrapper

try:
    import aioboto3
    import boto3
except ImportError:
    raise ImportError(
        "aioboto3 and boto3 packages are required for AWS Bedrock embedding functionality. "
        "Please install them via: pip install aioboto3 boto3"
    )

try:
    from tenacity import (
        retry,
        stop_after_attempt,
        wait_exponential,
        retry_if_exception_type,
    )
except ImportError:
    raise ImportError(
        "tenacity package is required for AWS Bedrock embedding functionality. "
        "Please install it via: pip install tenacity"
    )

import numpy as np


class BedrockError(Exception):
    """Generic error for issues related to Amazon Bedrock"""


class BedrockWrapper(BaseEmbeddingWrapper):
    """AWS Bedrock API 包装器"""
    
    def initialize(self):
        """初始化 AWS Bedrock 客户端"""
        if self._initialized:
            return
        
        try:
            # 获取 AWS 认证信息
            aws_access_key_id = self.config.get("aws_access_key_id") or os.getenv("AWS_ACCESS_KEY_ID")
            aws_secret_access_key = self.config.get("aws_secret_access_key") or os.getenv("AWS_SECRET_ACCESS_KEY")
            aws_session_token = self.config.get("aws_session_token") or os.getenv("AWS_SESSION_TOKEN")
            region_name = self.config.get("region_name", "us-east-1")
            
            # 设置环境变量（如果提供了参数）
            if aws_access_key_id:
                os.environ["AWS_ACCESS_KEY_ID"] = aws_access_key_id
            if aws_secret_access_key:
                os.environ["AWS_SECRET_ACCESS_KEY"] = aws_secret_access_key
            if aws_session_token:
                os.environ["AWS_SESSION_TOKEN"] = aws_session_token
            
            # 准备客户端配置
            client_config = {}
            if region_name:
                client_config["region_name"] = region_name
            if aws_access_key_id and aws_secret_access_key:
                client_config.update({
                    "aws_access_key_id": aws_access_key_id,
                    "aws_secret_access_key": aws_secret_access_key,
                })
                if aws_session_token:
                    client_config["aws_session_token"] = aws_session_token
            
            # 创建同步客户端
            self.client = boto3.client("bedrock-runtime", **client_config)
            
            # 创建异步会话
            self.session = aioboto3.Session()
            self.async_client_config = client_config
            
            self._initialized = True
            
        except Exception as e:
            raise RuntimeError(f"Failed to initialize AWS Bedrock client: {e}")
    
    def _get_model_provider(self) -> str:
        """获取模型提供商"""
        return self.model_info.model_path.split(".")[0]
    
    def _prepare_amazon_body(self, text: str) -> str:
        """准备 Amazon Titan 模型的请求体"""
        model_path = self.model_info.model_path
        
        if "v2" in model_path:
            body = {
                "inputText": text,
                "embeddingTypes": ["float"],
            }
        elif "v1" in model_path:
            body = {"inputText": text}
        else:
            raise ValueError(f"Amazon model {model_path} is not supported!")
        
        return json.dumps(body)
    
    def _prepare_cohere_body(self, texts: List[str], input_type: str = "search_document") -> str:
        """准备 Cohere 模型的请求体"""
        body = {
            "texts": texts,
            "input_type": input_type,
            "truncate": self.config.get("truncate", "NONE"),
        }
        return json.dumps(body)
    
    def _extract_amazon_embedding(self, response_body: Dict[str, Any]) -> List[float]:
        """从 Amazon 模型响应中提取 embedding"""
        return response_body["embedding"]
    
    def _extract_cohere_embeddings(self, response_body: Dict[str, Any]) -> List[List[float]]:
        """从 Cohere 模型响应中提取 embeddings"""
        return response_body["embeddings"]
    
    @retry(
        retry=retry_if_exception_type(Exception),
        wait=wait_exponential(multiplier=1, min=4, max=60),
        stop=stop_after_attempt(3)
    )
    def _invoke_model_sync(self, body: str) -> Dict[str, Any]:
        """同步调用模型（带重试）"""
        try:
            response = self.client.invoke_model(
                modelId=self.model_info.model_path,
                body=body,
                accept="application/json",
                contentType="application/json",
            )
            response_body = json.loads(response["body"].read())
            return response_body
        except Exception as e:
            raise BedrockError(f"Bedrock model invocation failed: {e}")
    
    @retry(
        retry=retry_if_exception_type(Exception),
        wait=wait_exponential(multiplier=1, min=4, max=60),
        stop=stop_after_attempt(3)
    )
    async def _invoke_model_async(self, body: str) -> Dict[str, Any]:
        """异步调用模型（带重试）"""
        try:
            async with self.session.client("bedrock-runtime", **self.async_client_config) as bedrock_client:
                response = await bedrock_client.invoke_model(
                    modelId=self.model_info.model_path,
                    body=body,
                    accept="application/json",
                    contentType="application/json",
                )
                response_body = await response.get("body").json()
                return response_body
        except Exception as e:
            raise BedrockError(f"Bedrock async model invocation failed: {e}")
    
    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        return self.batch_embed([text])[0]
    
    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        model_provider = self._get_model_provider()
        
        try:
            if model_provider == "amazon":
                # Amazon 模型不支持批量，需要逐个处理
                embeddings = []
                for text in texts:
                    body = self._prepare_amazon_body(text)
                    response_body = self._invoke_model_sync(body)
                    embedding = self._extract_amazon_embedding(response_body)
                    embeddings.append(embedding)
                return embeddings
                
            elif model_provider == "cohere":
                # Cohere 支持批量处理
                input_type = self.config.get("input_type", "search_document")
                body = self._prepare_cohere_body(texts, input_type)
                response_body = self._invoke_model_sync(body)
                return self._extract_cohere_embeddings(response_body)
                
            else:
                raise ValueError(f"Model provider '{model_provider}' is not supported!")
                
        except Exception as e:
            raise RuntimeError(f"Bedrock embedding failed: {e}")
    
    async def async_embed(self, text: str) -> List[float]:
        """异步单个文本 embedding"""
        embeddings = await self.async_batch_embed([text])
        return embeddings[0]
    
    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        model_provider = self._get_model_provider()
        
        try:
            if model_provider == "amazon":
                # Amazon 模型不支持批量，并发处理
                tasks = []
                for text in texts:
                    body = self._prepare_amazon_body(text)
                    task = self._invoke_model_async(body)
                    tasks.append(task)
                
                responses = await asyncio.gather(*tasks)
                embeddings = [self._extract_amazon_embedding(resp) for resp in responses]
                return embeddings
                
            elif model_provider == "cohere":
                # Cohere 支持批量处理
                input_type = self.config.get("input_type", "search_document")
                body = self._prepare_cohere_body(texts, input_type)
                response_body = await self._invoke_model_async(body)
                return self._extract_cohere_embeddings(response_body)
                
            else:
                raise ValueError(f"Model provider '{model_provider}' is not supported!")
                
        except Exception as e:
            raise RuntimeError(f"Bedrock async embedding failed: {e}")
    
    def embed_with_input_type(self, texts: List[str], input_type: str) -> List[List[float]]:
        """使用特定输入类型生成 embeddings（仅适用于 Cohere）"""
        if self._get_model_provider() != "cohere":
            raise ValueError("input_type parameter is only supported for Cohere models")
        
        if not self._initialized:
            self.initialize()
        
        try:
            body = self._prepare_cohere_body(texts, input_type)
            response_body = self._invoke_model_sync(body)
            return self._extract_cohere_embeddings(response_body)
        except Exception as e:
            raise RuntimeError(f"Bedrock embedding with input_type failed: {e}")
    
    async def async_embed_with_input_type(self, texts: List[str], input_type: str) -> List[List[float]]:
        """异步使用特定输入类型生成 embeddings（仅适用于 Cohere）"""
        if self._get_model_provider() != "cohere":
            raise ValueError("input_type parameter is only supported for Cohere models")
        
        if not self._initialized:
            self.initialize()
        
        try:
            body = self._prepare_cohere_body(texts, input_type)
            response_body = await self._invoke_model_async(body)
            return self._extract_cohere_embeddings(response_body)
        except Exception as e:
            raise RuntimeError(f"Bedrock async embedding with input_type failed: {e}")
    