"""
Embedding 模块使用示例
"""
import asyncio
from embedding_new import get_embedding_model, list_embedding_models


def basic_usage_example():
    """基本使用示例"""
    print("=== 基本使用示例 ===")
    
    # 列出可用模型
    models = list_embedding_models()
    print(f"可用模型数量: {len(models)}")
    for model in models:  
        status = "✓" if model["is_available"] else "✗"
        cached = "(已缓存)" if model.get("is_cached") else ""
        print(f"{status} {model['name']}: {model['dimension']}d {cached}")
    
    # 使用本地模型（如果可用）
    try:
        model = get_embedding_model("nomic-embed-text")
        with model:
            # 单个文本embedding
            text = "这是一个测试文本"
            embedding = model.embed(text)
            print(f"文本: '{text}'")
            print(f"向量维度: {len(embedding)}")
            print(f"前5个值: {embedding[:5]}")
            
            # 批量embedding
            texts = ["hello world", "测试文本", "another example"]
            embeddings = model.batch_embed(texts)
            print(f"\n批量处理 {len(texts)} 个文本")
            for i, (text, emb) in enumerate(zip(texts, embeddings)):
                print(f"文本{i+1}: '{text}' -> 向量维度: {len(emb)}")

    except Exception as e:
        print(f"本地模型不可用: {e}")

    # 使用Mock模型（总是可用）
    print("\n=== 使用Mock模型 ===")
    model = get_embedding_model("mock-small")
    with model:
        embedding = model.embed("test text")
        print(f"Mock模型向量维度: {len(embedding)}")


def api_model_example():
    """API模型使用示例"""
    print("\n=== API模型示例 ===")
    
    # 使用Cohere API（需要设置COHERE_API_KEY环境变量）
    # try:
    #     model = get_embedding_model("embed-multilingual-v3.0")
    #     with model:
    #         texts = ["Hello world", "This is a test"]
    #         embeddings = model.batch_embed(texts)
    #         print(f"Cohere模型成功处理 {len(texts)} 个文本")
    #         print(f"向量维度: {len(embeddings[0])}")
    # except Exception as e:
    #     print(f"Cohere API不可用: {e}")
    
    # 使用OpenAI API（需要设置OPENAI_API_KEY环境变量）
    try:
        model = get_embedding_model("amazon-titan-embed-text-v1")
        with model:
            embedding = model.embed("Test with OpenAI")
            print(f"OpenAI模型向量维度: {len(embedding)}")
    except Exception as e:
        print(f"OpenAI API不可用: {e}")


async def async_example():
    """异步使用示例"""
    print("\n=== 异步操作示例 ===")
    
    model = get_embedding_model("mock-large")
    with model:
        # 异步单个文本
        embedding = await model.async_embed("async test")
        print(f"异步单个文本 - 向量维度: {len(embedding)}")
        
        # 异步批量文本
        texts = ["async text 1", "async text 2", "async text 3"]
        embeddings = await model.async_batch_embed(texts)
        print(f"异步批量处理 {len(texts)} 个文本")


def custom_config_example():
    """自定义配置示例"""
    print("\n=== 自定义配置示例 ===")
    
    # 为Cohere模型指定不同的input_type
    try:
        model = get_embedding_model(
            "embed-multilingual-v3.0", 
            input_type="search_document"  # 自定义参数
        )
        with model:
            embedding = model.embed("Custom config test")
            print(f"自定义配置成功，向量维度: {len(embedding)}")
    except Exception as e:
        print(f"自定义配置失败: {e}")
    
    # 为OpenAI模型指定自定义base_url
    try:
        model = get_embedding_model(
            "text-embedding-ada-002",
            base_url="https://your-custom-openai-endpoint/v1"
        )
        # 注意：这里不会真正调用，只是展示如何传递自定义配置
        print("自定义OpenAI端点配置成功")
    except Exception as e:
        print(f"自定义OpenAI配置: {e}")


def model_comparison_example():
    """模型对比示例"""
    print("\n=== 模型对比示例 ===")
    
    test_text = "This is a test sentence for comparison."
    available_models = ["mock-small", "mock-large"]
    
    results = {}
    for model_name in available_models:
        try:
            model = get_embedding_model(model_name)
            with model:
                embedding = model.embed(test_text)
                results[model_name] = {
                    "dimension": len(embedding),
                    "sample_values": embedding[:3]
                }
        except Exception as e:
            results[model_name] = {"error": str(e)}
    
    print("模型对比结果:")
    for model_name, result in results.items():
        if "error" in result:
            print(f"  {model_name}: 错误 - {result['error']}")
        else:
            print(f"  {model_name}: {result['dimension']}d, 示例值: {result['sample_values']}")


if __name__ == "__main__":
    # 运行所有示例
    basic_usage_example()
    api_model_example()
    custom_config_example()
    model_comparison_example()
    
    # 运行异步示例
    asyncio.run(async_example())