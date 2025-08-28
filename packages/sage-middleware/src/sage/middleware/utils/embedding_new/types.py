"""
Embedding 相关类型和枚举定义
"""
from enum import Enum
from dataclasses import dataclass
from typing import Dict, Any


class ModelType(Enum):
    """模型类型枚举"""
    LOCAL_HF = "local_hf"                  # 需要下载到本地的 HuggingFace 模型
    LOCAL_INSTRUCTOR = "local_instructor"  # Instructor 模型
    LOCAL_OLLAMA = "local_ollama"          # 本地 Ollama 模型
    API_OPENAI = "api_openai"              # OpenAI 兼容 API
    API_NVIDIA = "api_nvidia"              # NVIDIA OpenAI 兼容 API
    API_COHERE = "api_cohere"              # Cohere API
    API_JINA = "api_jina"                  # Jina API
    API_ZHIPUAI = "api_zhipuai"            # 智谱 AI API
    API_BEDROCK = "api_bedrock"            # AWS Bedrock API
    API_SILICONCLOUD = "api_siliconcloud"  # 硅基流动 API
    API_LOLLMS = "api_lollms"              # LOLLMS API
    MOCK = "mock"                          # 测试用 Mock 模型


@dataclass
class EmbeddingModelInfo:
    """Embedding 模型信息"""
    name: str                              # 模型名称/标识符
    display_name: str                      # 显示名称
    model_type: ModelType                  # 模型类型
    dimension: int                         # 向量维度
    max_tokens: int                        # 最大 token 数
    provider: str                          # 提供商
    model_path: str                        # 模型路径或 API 模型名
    wrapper_class: str                     # 对应的 wrapper 类名
    requires_api_key: bool                 # 是否需要 API key
    config: Dict[str, Any]                 # 额外配置参数
    is_cached: bool = False                # 是否已缓存到本地
    is_available: bool = False             # 是否可用