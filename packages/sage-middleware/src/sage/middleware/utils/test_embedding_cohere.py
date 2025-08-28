#!/usr/bin/env python3
"""
简单的 Cohere 嵌入模型测试脚本
"""

import os
import sys


def test_cohere_simple():
    """简单的 Cohere 测试"""
    print("=== Cohere 测试 ===")
    
    # 1. 检查 API Key
    api_key = os.getenv("COHERE_API_KEY")
    if not api_key:
        print("❌ 请先设置 COHERE_API_KEY 环境变量")
        print("   方法1: export COHERE_API_KEY='your-api-key'")
        print("   方法2: 在代码中设置 os.environ['COHERE_API_KEY'] = 'your-api-key'")
        return False
    
    print(f"✅ API Key 已设置: {api_key[:8]}...{api_key[-4:]}")
    
    try:
        # 2. 导入模块
        from embedding_new import get_embedding_model
        print("✅ 模块导入成功")
        
        # 3. 创建模型实例
        model = get_embedding_model("embed-multilingual-v3.0")
        print("✅ 模型实例创建成功")
        
        # 4. 测试单个文本
        with model:
            text = "Hello, this is a test sentence."
            embedding = model.embed(text)
            print(f"✅ 单个文本嵌入成功")
            print(f"   文本: '{text}'")
            print(f"   向量维度: {len(embedding)}")
            print(f"   前5个值: {embedding[:5]}")
            
            # 5. 测试批量文本
            texts = [
                "Hello world",
                "This is a test",
                "Cohere embedding works!"
            ]
            embeddings = model.batch_embed(texts)
            print(f"✅ 批量嵌入成功")
            print(f"   处理了 {len(texts)} 个文本")
            for i, (text, emb) in enumerate(zip(texts, embeddings)):
                print(f"   文本{i+1}: '{text}' -> 维度: {len(emb)}")
        
        print("\n🎉 所有测试通过!")
        return True
        
    except Exception as e:
        print(f"❌ 测试失败: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_cohere_with_custom_config():
    """测试自定义配置"""
    print("\n=== 自定义配置测试 ===")
    
    try:
        from embedding_new import get_embedding_model
        
        # 使用不同的 input_type
        model = get_embedding_model(
            "embed-multilingual-v3.0", 
            input_type="search_document"  # 搜索文档模式
        )
        
        with model:
            text = "This is a document for search."
            embedding = model.embed(text)
            print(f"✅ 搜索文档模式嵌入成功")
            print(f"   向量维度: {len(embedding)}")
            
        # 测试分类模式
        model2 = get_embedding_model(
            "embed-multilingual-v3.0", 
            input_type="classification"  # 分类模式
        )
        
        with model2:
            embedding2 = model2.embed("This is for classification.")
            print(f"✅ 分类模式嵌入成功")
            print(f"   向量维度: {len(embedding2)}")
            
    except Exception as e:
        print(f"❌ 自定义配置测试失败: {e}")


if __name__ == "__main__":
    # 如果没有设置环境变量，可以在这里直接设置（仅用于测试）
    # 取消注释下面一行并填入你的 API Key
    # os.environ["COHERE_API_KEY"] = "your-cohere-api-key-here"
    
    success = test_cohere_simple()
    
    if success:
        test_cohere_with_custom_config()