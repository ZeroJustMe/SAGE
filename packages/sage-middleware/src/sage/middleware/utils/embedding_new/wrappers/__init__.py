"""
Embedding Wrappers Module
"""
from .huggingface_wrapper import HuggingFaceWrapper
from .openai_wrapper import OpenAIWrapper
from .cohere_wrapper import CohereWrapper
from .mock_wrapper import MockWrapper
from .nvidia_openai_wrapper import NvidiaOpenAIWrapper
from .bedrock_wrapper import BedrockWrapper
from .ollama_wrapper import OllamaWrapper
from .jina_wrapper import JinaWrapper
from .zhipu_wrapper import ZhipuAIWrapper
from .siliconcloud_wrapper import SiliconCloudWrapper
from .lollms_wrapper import LollmsWrapper

# from .instructor_wrapper import InstructorWrapper

__all__ = [
    'HuggingFaceWrapper',
    'OpenAIWrapper', 
    'CohereWrapper',
    'MockWrapper',
    'OllamaWrapper',
    'NvidiaOpenAIWrapper',
    'BedrockWrapper',
    'JinaWrapper',
    'ZhipuAIWrapper',
    'SiliconCloudWrapper',
    'LollmsWrapperr',
    # 'InstructorWrapper',
]
