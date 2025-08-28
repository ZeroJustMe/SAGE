"""
统一的 Embedding 管理器
"""
from typing import Dict, Type, List, Optional
from .base_wrapper import BaseEmbeddingWrapper
from .factory import EmbeddingFactory
from .wrappers import HuggingFaceWrapper
from .wrappers import CohereWrapper
from .wrappers import MockWrapper
from .wrappers import OllamaWrapper
from .wrappers import OpenAIWrapper
from .wrappers import NvidiaOpenAIWrapper
from .wrappers import BedrockWrapper
from .wrappers import JinaWrapper
from .wrappers import ZhipuAIWrapper
from .wrappers import SiliconCloudWrapper
from .wrappers import LollmsWrapper
# from .wrappers import InstructorWrapper


class EmbeddingManager:
    """Embedding 模型管理器"""
    
    def __init__(self):
        self.factory = EmbeddingFactory()
        self.wrapper_registry: Dict[str, Type[BaseEmbeddingWrapper]] = {
            "HuggingFaceWrapper": HuggingFaceWrapper,
            "OpenAIWrapper": OpenAIWrapper,
            "CohereWrapper": CohereWrapper,
            "MockWrapper": MockWrapper,
            "OllamaWrapper": OllamaWrapper,
            "NvidiaOpenAIWrapper": NvidiaOpenAIWrapper,
            "BedrockWrapper": BedrockWrapper,
            "JinaWrapper": JinaWrapper,
            "ZhipuAIWrapper": ZhipuAIWrapper,
            "SiliconCloudWrapper": SiliconCloudWrapper,
            "LollmsWrapper": LollmsWrapper,
            # "InstructorWrapper": InstructorWrapper,
        }
        self._model_cache: Dict[str, BaseEmbeddingWrapper] = {}
    
    def get_embedding_model(self, name: str, **kwargs) -> BaseEmbeddingWrapper:
        """获取 embedding 模型实例"""
        # 获取模型信息
        model_info = self.factory.get_model_info(name)
        if not model_info:
            raise ValueError(f"Unknown embedding model: {name}")
        
        if not model_info.is_available:
            if model_info.requires_api_key:
                api_key_env = self.factory._get_api_key_env(model_info.model_type)
                raise ValueError(f"Model {name} requires API key. Please set {api_key_env} environment variable.")
            else:
                raise ValueError(f"Model {name} is not available")
        
        # 创建 wrapper 实例
        wrapper_class = self.wrapper_registry.get(model_info.wrapper_class)
        if not wrapper_class:
            raise ValueError(f"Unknown wrapper class: {model_info.wrapper_class}")
        
        wrapper = wrapper_class(model_info, **kwargs)
        return wrapper
    
    def list_embedding_models(self, available_only: bool = True) -> List[Dict]:
        """列出可用的 embedding 模型"""
        models = self.factory.list_models(available_only=available_only)
        return [
            {
                "name": model.name,
                "display_name": model.display_name,
                "provider": model.provider,
                "dimension": model.dimension,
                "max_tokens": model.max_tokens,
                "type": model.model_type.value,
                "is_available": model.is_available,
                "is_cached": model.is_cached,
                "requires_api_key": model.requires_api_key
            }
            for model in models
        ]
    
    def register_wrapper(self, name: str, wrapper_class: Type[BaseEmbeddingWrapper]):
        """注册新的 wrapper 类"""
        self.wrapper_registry[name] = wrapper_class
    
    def refresh_availability(self):
        """刷新所有模型的可用状态"""
        self.factory._update_availability()