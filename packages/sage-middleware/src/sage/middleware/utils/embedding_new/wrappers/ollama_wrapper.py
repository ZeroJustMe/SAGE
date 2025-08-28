"""
Ollama API Wrapper - Embedding 版本 (基于 /api/embeddings, 更鲁棒)
"""

import sys
import os
import asyncio
from typing import List, Optional, Dict, Any

if sys.version_info < (3, 9):
    from typing import AsyncIterator
else:
    from collections.abc import AsyncIterator

from ..base_wrapper import BaseEmbeddingWrapper

try:
    import httpx
except ImportError:
    raise ImportError(
        "httpx package is required for Ollama embedding functionality. "
        "Please install it via: pip install httpx"
    )

try:
    import tenacity
except ImportError:
    raise ImportError(
        "tenacity package is required for Ollama embedding functionality. "
        "Please install it via: pip install tenacity"
    )


class OllamaWrapper(BaseEmbeddingWrapper):
    """Ollama API 包装器 (Embedding 版)"""

    def initialize(self):
        """初始化 HTTP 客户端"""
        if self._initialized:
            return

        base_url = self.config.get("base_url", "http://localhost:11434")
        api_key = self.config.get("api_key") or os.getenv("OLLAMA_API_KEY")
        timeout = self.config.get("timeout", 60)

        self.base_url = base_url.rstrip("/")
        self.timeout = timeout

        # 请求头
        self.headers = {
            "Content-Type": "application/json",
            "User-Agent": "SAGE/0.0",
        }
        if api_key:
            self.headers["Authorization"] = f"Bearer {api_key}"

        # 同步 & 异步客户端
        self.client = httpx.Client(base_url=self.base_url, headers=self.headers, timeout=self.timeout)
        self.async_client = httpx.AsyncClient(base_url=self.base_url, headers=self.headers, timeout=self.timeout)

        self._initialized = True

    def _is_embedding_model(self, model: str) -> bool:
        """检查是否是 embedding 模型"""
        return "embed" in model.lower()

    def _ensure_model(self, model: str) -> str:
        """如果不是 embedding 模型，强制换成 nomic-embed-text"""
        if not self._is_embedding_model(model):
            print(f"[OllamaWrapper] Warning: model '{model}' is not an embedding model. "
                  f"Falling back to 'nomic-embed-text'.")
            return "nomic-embed-text"
        return model

    @tenacity.retry(
        retry=tenacity.retry_if_exception_type(Exception),
        wait=tenacity.wait_exponential(multiplier=1, min=4, max=60),
        stop=tenacity.stop_after_attempt(3)
    )
    def _embed_with_retry(self, text: str) -> List[float]:
        """带重试的同步 embedding"""
        model = self._ensure_model(self.model_info.model_path)

        try:
            resp = self.client.post(
                "/api/embeddings",
                json={"model": model, "prompt": text}
            )
            resp.raise_for_status()
            data = resp.json()
            return data["embedding"]
        except Exception as e:
            raise RuntimeError(f"Ollama embedding failed: {e}")

    @tenacity.retry(
        retry=tenacity.retry_if_exception_type(Exception),
        wait=tenacity.wait_exponential(multiplier=1, min=4, max=60),
        stop=tenacity.stop_after_attempt(3)
    )
    async def _async_embed_with_retry(self, text: str) -> List[float]:
        """带重试的异步 embedding"""
        model = self._ensure_model(self.model_info.model_path)

        try:
            resp = await self.async_client.post(
                "/api/embeddings",
                json={"model": model, "prompt": text}
            )
            resp.raise_for_status()
            data = resp.json()
            return data["embedding"]
        except Exception as e:
            raise RuntimeError(f"Ollama async embedding failed: {e}")

    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        if not self._initialized:
            self.initialize()
        return self._embed_with_retry(text)

    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量 embedding"""
        if not self._initialized:
            self.initialize()
        return [self._embed_with_retry(t) for t in texts]

    async def async_embed(self, text: str) -> List[float]:
        """异步单个 embedding"""
        if not self._initialized:
            self.initialize()
        return await self._async_embed_with_retry(text)

    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量 embedding"""
        if not self._initialized:
            self.initialize()
        tasks = [self._async_embed_with_retry(t) for t in texts]
        return await asyncio.gather(*tasks)

    # def __del__(self):
    #     """释放 httpx 客户端"""
    #     if hasattr(self, "client") and self.client:
    #         self.client.close()
    #     if hasattr(self, "async_client") and self.async_client:
    #         asyncio.get_event_loop().create_task(self.async_client.aclose())

    def close(self):
        pass

    def __enter__(self):
        if not self._initialized:
            self.initialize()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    async def __aenter__(self):
        if not self._initialized:
            self.initialize()
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        await self.aclose()