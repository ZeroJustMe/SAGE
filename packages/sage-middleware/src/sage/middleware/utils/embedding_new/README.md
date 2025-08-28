# Embedding Module

一个统一的文本嵌入（Embedding）模块，支持多种本地和 API 模型，提供高效的批量处理和本地模型缓存功能。

## 主要特性

- **统一接口**：支持本地模型（HuggingFace、Ollama）和 API 服务（OpenAI、Cohere、ZhipuAI 等）
- **高效批量处理**：`batch_embed` 接口支持批量文本处理，显著提升性能
- **本地模型缓存**：避免重复下载，Pipeline 运行时直接加载本地缓存
- **异步支持**：完整的异步 API，支持并发处理
- **自动错误处理**：统一的错误处理和重试机制
- **灵活配置**：支持自定义模型参数和配置

## 安装依赖

```bash
pip install torch transformers sentence-transformers
pip install openai cohere zhipuai aiohttp requests
pip install ollama boto3  # 可选，根据需要安装
```

## 快速开始

### 基本使用

```python
from embedding_new import get_embedding_model, list_embedding_models

# 查看所有可用模型
models = list_embedding_models()
for model in models:
    status = "✓" if model["is_available"] else "✗"
    cached = "(已缓存)" if model.get("is_cached") else ""
    print(f"{status} {model['name']}: {model['dimension']}d {cached}")

# 使用模型
model = get_embedding_model("nomic-embed-text")
with model:
    # 单个文本
    embedding = model.embed("Hello world")
    print(f"向量维度: {len(embedding)}")
    
    # 批量处理（推荐）
    texts = ["Hello world", "测试文本", "Another example"]
    embeddings = model.batch_embed(texts)
    print(f"批量处理了 {len(embeddings)} 个文本")
```

### 异步处理

```python
import asyncio

async def async_example():
    model = get_embedding_model("nomic-embed-text")
    with model:
        # 异步批量处理
        texts = ["text1", "text2", "text3"]
        embeddings = await model.async_batch_embed(texts)
        return embeddings

# 运行异步示例
embeddings = asyncio.run(async_example())
```

## 核心功能详解

### 1. 批量处理接口 (batch_embed)

批量处理是提升 Embedding 性能的关键功能：

#### 为什么使用批量处理？

- **性能提升**：一次处理多个文本，减少网络请求和模型加载开销
- **内存优化**：批量处理时更高效地利用 GPU 内存
- **成本节省**：API 调用次数减少，降低费用

#### 批量处理示例

```python
# ❌ 低效：逐个处理
texts = ["text1", "text2", "text3", "text4", "text5"]
embeddings = []
for text in texts:
    embedding = model.embed(text)  # 每次都要调用模型
    embeddings.append(embedding)

# ✅ 高效：批量处理
texts = ["text1", "text2", "text3", "text4", "text5"]
embeddings = model.batch_embed(texts)  # 一次调用处理所有文本
```

#### 批量大小建议

```python
# 根据模型类型调整批量大小
def process_large_dataset(texts, model_name):
    model = get_embedding_model(model_name)
    
    # 不同模型的推荐批量大小
    batch_sizes = {
        "local": 32,      # 本地模型
        "openai": 100,    # OpenAI API
        "cohere": 96,     # Cohere API
        "mock": 1000      # Mock模型测试
    }
    
    batch_size = batch_sizes.get(model.model_info.provider.lower(), 32)
    
    all_embeddings = []
    for i in range(0, len(texts), batch_size):
        batch = texts[i:i + batch_size]
        batch_embeddings = model.batch_embed(batch)
        all_embeddings.extend(batch_embeddings)
    
    return all_embeddings
```

### 2. 本地模型缓存 (Local Model Cache)

#### 缓存机制

本地模型下载后会自动缓存，避免 Pipeline 运行时重复下载：

```python
# 检查模型缓存状态
models = list_embedding_models()
for model in models:
    if model.get("is_cached"):
        print(f"✓ {model['name']} 已缓存")
    else:
        print(f"⬇ {model['name']} 需要下载")
```



## 支持的模型列表

### 本地模型

| 服务商 | 模型名称 | 维度 | 最大Token | 语言支持 | 特点 |
|--------|----------|------|----------|----------|------|
| **BAAI** | `bge-small-zh-v1.5` | 512 | 512 | 中文 | 轻量级中文模型 |
| BAAI | `bge-m3` | 1024 | 8192 | 多语言 | 多语言大模型 |
| **sentence-transformers** | `all-MiniLM-L6-v2` | 384 | 256 | 英文 | 轻量级、快速 |
| **hkunlp** | `instructor-large` | 768 | 512 | 英文 | 指令式嵌入 |

### 本地服务模型

| 服务商 | 模型名称 | 维度 | 最大Token | 端口 | 特点 |
|--------|----------|------|----------|------|------|
| **Ollama** | `nomic-embed-text` | 768 | 2048 | 11434 | 通用文本嵌入 |
| **Lollms** | `lollms-embedding` | 768 | 2048 | 9600 | 本地推理服务 |

### API 模型

| 服务商 | 模型名称 | 维度 | 最大Token | 特点 |
|--------|----------|------|----------|------|
| **OpenAI** | `text-embedding-ada-002` | 1536 | 8191 | 经典稳定模型 |
| OpenAI | `text-embedding-3-small` | 1536 | 8191 | 新一代小模型 |
| OpenAI | `text-embedding-3-large` | 3072 | 8191 | 最新大模型，支持自定义维度 |
| **NVIDIA** | `nvidia-llama-3.2-nv-embedqa-1b-v1` | 4096 | 2048 | 高维度，支持查询/文档区分 |
| **Jina AI** | `jina-embeddings-v3` | 1024 | 8192 | 支持自定义维度、Late Chunking |
| Jina AI | `jina-embeddings-v2-base-en` | 768 | 8192 | 英文专用 |
| Jina AI | `jina-embeddings-v2-small-en` | 512 | 512 | 轻量级英文模型 |
| **Cohere** | `embed-english-v3.0` | 1024 | 512 | 英文专用 |
| Cohere | `embed-multilingual-v3.0` | 1024 | 512 | 多语言支持 |
| **ZhipuAI** | `embedding-3` | 2048 | 8192 | 中文优化 |
| **SiliconCloud** | `bce-embedding-base_v1` | 768 | 512 | 高性价比，Base64编码 |

### Amazon Bedrock 模型

| 服务商 | 模型名称 | 维度 | 最大Token | 特点 |
|--------|----------|------|----------|------|
| **Amazon** | `amazon-titan-embed-text-v1` | 1536 | 8000 | Titan v1 |
| Amazon | `amazon-titan-embed-text-v2` | 1024 | 8000 | Titan v2 |
| **Cohere (Bedrock)** | `cohere-embed-english-v3` | 1024 | 512 | Bedrock上的英文模型 |
| Cohere (Bedrock) | `cohere-embed-multilingual-v3` | 1024 | 512 | Bedrock上的多语言模型 |

### 测试模型

| 服务商 | 模型名称 | 维度 | 最大Token | 特点 |
|--------|----------|------|----------|------|
| **Mock** | `mock-small` | 384 | 512 | 测试用小模型 |
| Mock | `mock-large` | 1536 | 8191 | 测试用大模型 |

## 使用建议

### 性能优化

1. **优先使用批量接口**
```python
# 推荐
embeddings = model.batch_embed(texts)

# 避免
embeddings = [model.embed(text) for text in texts]
```

2. **合理设置批量大小**
```python
# 根据内存和模型大小调整
batch_size = 32 if "large" in model_name else 64
```

3. **使用异步处理大量数据**
```python
async def process_large_dataset(texts):
    model = get_embedding_model("nomic-embed-text")
    with model:
        return await model.async_batch_embed(texts)
```

### 模型选择指南

#### 生产环境推荐

1. **中文场景**：
   - 轻量级：`bge-small-zh-v1.5` (512d)
   - 高性能：`bge-m3` (1024d, 多语言)
   - API选择：`embedding-3` (ZhipuAI, 2048d)

2. **英文场景**：
   - 轻量级：`all-MiniLM-L6-v2` (384d)
   - 本地服务：`nomic-embed-text` (768d, Ollama)
   - 指令式：`instructor-large` (768d)

3. **多语言场景**：
   - 本地：`bge-m3` (1024d, 8192 tokens)
   - API：`embed-multilingual-v3.0` (Cohere, 1024d)

4. **轻量级部署**：
   - 最小：`all-MiniLM-L6-v2` (384d)
   - 中等：`bge-small-zh-v1.5` (512d, 中文)
   - 测试：`mock-small` (384d)

#### API 服务推荐

1. **高质量需求**：
   - OpenAI `text-embedding-3-large` (3072d, 支持自定义维度)
   - NVIDIA `nvidia-llama-3.2-nv-embedqa-1b-v1` (4096d)

2. **成本敏感**：
   - `bce-embedding-base_v1` (SiliconCloud, 768d)
   - `embedding-3` (ZhipuAI, 2048d)

3. **多语言支持**：
   - `embed-multilingual-v3.0` (Cohere, 1024d)
   - `bge-m3` (本地, 1024d)

4. **特殊功能**：
   - 自定义维度：Jina `jina-embeddings-v3`, OpenAI `text-embedding-3-large`
   - Late Chunking：Jina `jina-embeddings-v3`
   - 查询/文档区分：NVIDIA 模型

### 错误处理

```python
def robust_embedding(texts, model_name="nomic-embed-text", fallback="mock-large"):
    try:
        model = get_embedding_model(model_name)
        with model:
            return model.batch_embed(texts)
    except Exception as e:
        print(f"主模型失败: {e}, 使用备用模型: {fallback}")
        model = get_embedding_model(fallback)
        with model:
            return model.batch_embed(texts)
```

## 高级配置

### 自定义模型参数

```python
# OpenAI 自定义配置
model = get_embedding_model(
    "text-embedding-ada-002",
    base_url="https://your-custom-endpoint/v1",
    timeout=30
)

# Cohere 自定义配置
model = get_embedding_model(
    "embed-multilingual-v3.0",
    input_type="search_document",  # 或 "classification"
    truncate="END"
)

# 本地模型设备配置
model = get_embedding_model(
    "nomic-embed-text",
    device="cuda:0",  # 指定 GPU
    trust_remote_code=True
)
```

### 环境变量配置

```bash
# API 密钥
export OPENAI_API_KEY="your-openai-key"
export COHERE_API_KEY="your-cohere-key"
export ZHIPUAI_API_KEY="your-zhipuai-key"

# 自定义缓存目录
export HF_HOME="/custom/cache/huggingface"
export TRANSFORMERS_CACHE="/custom/cache/transformers"
```