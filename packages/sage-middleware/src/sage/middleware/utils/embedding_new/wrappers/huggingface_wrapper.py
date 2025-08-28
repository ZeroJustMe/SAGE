"""
HuggingFace 模型 Wrapper - 基于原有代码重构
"""
import os
import asyncio
from functools import lru_cache
from typing import List
from ..base_wrapper import BaseEmbeddingWrapper

# 设置环境变量防止并行化警告
os.environ["TOKENIZERS_PARALLELISM"] = "false"

# 动态依赖安装支持
try:
    import pipmaster as pm
except ImportError:
    pm = None

# 检查必要的依赖
try:
    from transformers import AutoTokenizer, AutoModel
except ImportError:
    if pm:
        pm.install("transformers")
        from transformers import AutoTokenizer, AutoModel
    else:
        raise ImportError(
            "transformers package is required for HuggingFace embedding functionality. "
            "Please install it via: pip install transformers"
        )

try:
    import torch
except ImportError:
    if pm:
        pm.install("torch")
        import torch
    else:
        raise ImportError(
            "torch package is required for HuggingFace embedding functionality. "
            "Please install it via: pip install torch"
        )

try:
    import tenacity
except ImportError:
    if pm:
        pm.install("tenacity")
        import tenacity
    else:
        raise ImportError(
            "tenacity package is required for HuggingFace embedding functionality. "
            "Please install it via: pip install tenacity"
        )

try:
    import numpy
except ImportError:
    if pm:
        pm.install("numpy")
        import numpy
    else:
        raise ImportError(
            "numpy package is required for HuggingFace embedding functionality. "
            "Please install it via: pip install numpy"
        )


class HuggingFaceWrapper(BaseEmbeddingWrapper):
    """HuggingFace 模型包装器 - 基于原有实现重构"""
    
    def __init__(self, model_info, **kwargs):
        super().__init__(model_info, **kwargs)
        self.tokenizer = None
        self.model = None
        self._model_cache_key = None
    
    def initialize(self):
        """初始化 HuggingFace 模型"""
        if self._initialized:
            return
        
        model_path = self.model_info.model_path
        
        # 使用缓存的模型初始化函数
        self.model, self.tokenizer = self._initialize_hf_model_cached(
            model_path,
            device_map=self.config.get("device_map", "auto"),
            trust_remote_code=self.config.get("trust_remote_code", True)
        )
        
        self._initialized = True
    
    @lru_cache(maxsize=3)  # 缓存最多3个模型
    def _initialize_hf_model_cached(self, model_name, device_map="auto", trust_remote_code=True):
        """缓存版本的模型初始化函数"""
        print(f"Loading HuggingFace model: {model_name}")
        
        # 加载tokenizer
        tokenizer = AutoTokenizer.from_pretrained(
            model_name, 
            device_map=device_map, 
            trust_remote_code=trust_remote_code
        )
        
        # 加载embedding模型（使用AutoModel而不是AutoModelForCausalLM）
        model = AutoModel.from_pretrained(
            model_name, 
            device_map=device_map, 
            trust_remote_code=trust_remote_code
        )
        
        # 设置pad_token
        if tokenizer.pad_token is None:
            tokenizer.pad_token = tokenizer.eos_token
            
        print(f"Model loaded successfully: {model_name}")
        return model, tokenizer
    
    def embed(self, text: str) -> List[float]:
        """单个文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        return self._hf_embed_sync(text, self.tokenizer, self.model)
    
    def batch_embed(self, texts: List[str]) -> List[List[float]]:
        """批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        return [self._hf_embed_sync(text, self.tokenizer, self.model) for text in texts]
    
    def _hf_embed_sync(self, text: str, tokenizer, embed_model) -> List[float]:
        """
        使用 HuggingFace 模型同步生成文本 embedding - 基于原有实现
        
        Args:
            text (str): 输入文本
            tokenizer: 已加载的 tokenizer
            embed_model: 已加载的 PyTorch embedding 模型
            
        Returns:
            List[float]: embedding 向量
        """
        device = next(embed_model.parameters()).device
        
        # 编码文本，支持最大长度限制
        encoded_texts = tokenizer(
            text, 
            return_tensors="pt", 
            padding=True, 
            truncation=True,
            max_length=self.model_info.max_tokens
        ).to(device)
        
        with torch.no_grad():
            outputs = embed_model(
                input_ids=encoded_texts["input_ids"],
                attention_mask=encoded_texts["attention_mask"],
            )
            # 使用平均池化获取句子级别的embedding
            embeddings = outputs.last_hidden_state.mean(dim=1)
        
        # 处理不同的数据类型
        if embeddings.dtype == torch.bfloat16:
            return embeddings.detach().to(torch.float32).cpu()[0].tolist()
        else:
            return embeddings.detach().cpu()[0].tolist()
    
    @tenacity.retry(
        wait=tenacity.wait_exponential(multiplier=1, min=4, max=10),
        stop=tenacity.stop_after_attempt(3),
        reraise=True
    )
    def _hf_embed_with_retry(self, text: str) -> List[float]:
        """带重试机制的embedding函数"""
        return self.embed(text)
    
    async def async_embed(self, text: str) -> List[float]:
        """异步单个文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        # 在线程池中执行同步操作
        loop = asyncio.get_event_loop()
        return await loop.run_in_executor(
            None, 
            self._hf_embed_sync, 
            text, 
            self.tokenizer, 
            self.model
        )
    
    async def async_batch_embed(self, texts: List[str]) -> List[List[float]]:
        """异步批量文本 embedding"""
        if not self._initialized:
            self.initialize()
        
        # 并发处理多个文本
        tasks = [self.async_embed(text) for text in texts]
        return await asyncio.gather(*tasks)
    
    def get_model_info(self) -> dict:
        """获取模型详细信息"""
        if not self._initialized:
            return {"status": "not_initialized"}
        
        device = next(self.model.parameters()).device
        return {
            "model_name": self.model_info.model_path,
            "device": str(device),
            "vocab_size": len(self.tokenizer),
            "max_tokens": self.model_info.max_tokens,
            "dimension": self.model_info.dimension,
            "pad_token": self.tokenizer.pad_token,
        }
    
    def __del__(self):
        """清理资源"""
        if hasattr(self, 'model') and self.model is not None:
            # 清理GPU内存
            if torch.cuda.is_available():
                torch.cuda.empty_cache()