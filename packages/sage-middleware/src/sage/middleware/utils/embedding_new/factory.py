"""
Embedding Model Factory - 管理所有可用的 embedding 模型信息
"""
import os
import json
from typing import Dict, List, Optional, Any
from .types import ModelType, EmbeddingModelInfo


class EmbeddingFactory:
    """Embedding 模型工厂"""
    
    def __init__(self, config_path: Optional[str] = None):
        self.config_path = config_path or os.path.join(
            os.path.dirname(__file__), "model_registry.json"
        )
        self.models: Dict[str, EmbeddingModelInfo] = {}
        self._load_model_registry()
        self._update_availability()
    
    def _load_model_registry(self):
        """从配置文件加载模型注册表"""
        try:
            with open(self.config_path, 'r', encoding='utf-8') as f:
                registry = json.load(f)
            
            for model_data in registry['models']:
                # 转换字符串类型为枚举
                model_data['model_type'] = ModelType(model_data['model_type'])
                model_info = EmbeddingModelInfo(**model_data)
                self.models[model_info.name] = model_info
        except FileNotFoundError:
            # 如果配置文件不存在，使用默认配置
            self._create_default_registry()
    
    def _create_default_registry(self):
        """创建默认的模型注册表"""
        default_models = [
            # HuggingFace 本地模型
            {
                "name": "bge-small-zh-v1.5",
                "display_name": "BGE Small Chinese v1.5",
                "model_type": ModelType.LOCAL_HF,
                "dimension": 512,
                "max_tokens": 512,
                "provider": "BAAI",
                "model_path": "BAAI/bge-small-zh-v1.5",
                "wrapper_class": "HuggingFaceWrapper",
                "requires_api_key": False,
                "config": {"trust_remote_code": True}
            },
            {
                "name": "bge-m3",
                "display_name": "BGE M3",
                "model_type": ModelType.LOCAL_HF,
                "dimension": 1024,
                "max_tokens": 8192,
                "provider": "BAAI",
                "model_path": "BAAI/bge-m3",
                "wrapper_class": "HuggingFaceWrapper",
                "requires_api_key": False,
                "config": {"trust_remote_code": True}
            },
            {
                "name": "all-MiniLM-L6-v2",
                "display_name": "All MiniLM L6 v2",
                "model_type": ModelType.LOCAL_HF,
                "dimension": 384,
                "max_tokens": 256,
                "provider": "sentence-transformers",
                "model_path": "sentence-transformers/all-MiniLM-L6-v2",
                "wrapper_class": "HuggingFaceWrapper",
                "requires_api_key": False,
                "config": {"trust_remote_code": True}
            },
            
            # API 模型
            {
                "name": "text-embedding-ada-002",
                "display_name": "OpenAI Ada 002",
                "model_type": ModelType.API_OPENAI,
                "dimension": 1536,
                "max_tokens": 8191,
                "provider": "OpenAI",
                "model_path": "text-embedding-ada-002",
                "wrapper_class": "OpenAIWrapper",
                "requires_api_key": True,
                "config": {"base_url": "https://api.openai.com/v1"}
            },
            {
                "name": "text-embedding-3-small",
                "display_name": "OpenAI Embedding 3 Small",
                "model_type": ModelType.API_OPENAI,
                "dimension": 1536,
                "max_tokens": 8191,
                "provider": "OpenAI",
                "model_path": "text-embedding-3-small",
                "wrapper_class": "OpenAIWrapper",
                "requires_api_key": True,
                "config": {"base_url": "https://api.openai.com/v1"}
            },
            {
                "name": "embed-multilingual-v3.0",
                "display_name": "Cohere Multilingual v3.0",
                "model_type": ModelType.API_COHERE,
                "dimension": 1024,
                "max_tokens": 512,
                "provider": "Cohere",
                "model_path": "embed-multilingual-v3.0",
                "wrapper_class": "CohereWrapper",
                "requires_api_key": True,
                "config": {"input_type": "classification", "embedding_types": ["float"]}
            },
            {
                "name": "embed-english-v3.0",
                "display_name": "Cohere English v3.0",
                "model_type": ModelType.API_COHERE,
                "dimension": 1024,
                "max_tokens": 512,
                "provider": "Cohere",
                "model_path": "embed-english-v3.0",
                "wrapper_class": "CohereWrapper",
                "requires_api_key": True,
                "config": {"input_type": "classification", "embedding_types": ["float"]}
            },
            
            # Mock 测试模型
            {
                "name": "mock-small",
                "display_name": "Mock Small (384d)",
                "model_type": ModelType.MOCK,
                "dimension": 384,
                "max_tokens": 512,
                "provider": "Mock",
                "model_path": "mock",
                "wrapper_class": "MockWrapper",
                "requires_api_key": False,
                "config": {}
            },
            {
                "name": "mock-large",
                "display_name": "Mock Large (1536d)",
                "model_type": ModelType.MOCK,
                "dimension": 1536,
                "max_tokens": 8191,
                "provider": "Mock",
                "model_path": "mock",
                "wrapper_class": "MockWrapper",
                "requires_api_key": False,
                "config": {}
            }
        ]
        
        for model_data in default_models:
            model_info = EmbeddingModelInfo(**model_data)
            self.models[model_info.name] = model_info
    
    def _update_availability(self):
        """更新模型可用状态"""
        for model_name, model_info in self.models.items():
            model_info.is_available = self._check_model_availability(model_info)
            if model_info.model_type == ModelType.LOCAL_HF:
                model_info.is_cached = self._check_local_cache(model_info.model_path)
    
    def _check_model_availability(self, model_info: EmbeddingModelInfo) -> bool:
        """检查模型是否可用"""
        if model_info.requires_api_key:
            # 检查相应的环境变量是否设置
            api_key_env = self._get_api_key_env(model_info.model_type)
            return os.getenv(api_key_env) is not None
        else:
            # 对于本地模型和 Mock 模型，总是可用
            return True
    
    def _get_api_key_env(self, model_type: ModelType) -> str:
        """获取对应模型类型的 API key 环境变量名"""
        mapping = {
            ModelType.API_OPENAI: "OPENAI_API_KEY",
            ModelType.API_NVIDIA: "NVIDIA_API_KEY",
            ModelType.API_COHERE: "COHERE_API_KEY",
            ModelType.API_JINA: "JINA_API_KEY",
            ModelType.API_ZHIPUAI: "ZHIPU_API_KEY",
            ModelType.API_BEDROCK: "AWS_ACCESS_KEY_ID",
            ModelType.API_SILICONCLOUD: "SILICONCLOUD_API_KEY",
            ModelType.API_LOLLMS: "LOLLMS_API_KEY",
        }
        return mapping.get(model_type, "")
    
    def _check_local_cache(self, model_path: str) -> bool:
        """检查本地模型是否已缓存"""
        try:
            from transformers import AutoModel
            # 尝试从缓存加载，不下载
            AutoModel.from_pretrained(model_path, local_files_only=True)
            return True
        except:
            return False
    
    def list_models(self, available_only: bool = False) -> List[EmbeddingModelInfo]:
        """列出所有模型"""
        models = list(self.models.values())
        if available_only:
            models = [m for m in models if m.is_available]
        return models
    
    def get_model_info(self, name: str) -> Optional[EmbeddingModelInfo]:
        """获取模型信息"""
        return self.models.get(name)
    
    def register_model(self, model_info: EmbeddingModelInfo):
        """注册新模型"""
        self.models[model_info.name] = model_info
        self._save_registry()
    
    def _save_registry(self):
        """保存模型注册表到文件"""
        registry = {
            "models": [
                {
                    "name": info.name,
                    "display_name": info.display_name,
                    "model_type": info.model_type.value,
                    "dimension": info.dimension,
                    "max_tokens": info.max_tokens,
                    "provider": info.provider,
                    "model_path": info.model_path,
                    "wrapper_class": info.wrapper_class,
                    "requires_api_key": info.requires_api_key,
                    "config": info.config
                }
                for info in self.models.values()
            ]
        }
        
        os.makedirs(os.path.dirname(self.config_path), exist_ok=True)
        with open(self.config_path, 'w', encoding='utf-8') as f:
            json.dump(registry, f, indent=2, ensure_ascii=False)