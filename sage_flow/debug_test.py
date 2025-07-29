#!/usr/bin/env python3
import sys
sys.path.insert(0, 'build')

try:
    import sage_flow_datastream as sfd
    print("✅ 导入成功")
    
    # 创建环境
    env = sfd.Environment("debug_test")
    print(f"✅ 环境创建: {env.get_job_name()}")
    
    # 简单数据源
    counter = [0]
    def simple_source():
        counter[0] += 1
        if counter[0] <= 1:
            return sfd.create_text_message(1000, "测试消息")
        return None
    
    def simple_sink(msg):
        print(f"✅ 接收到消息: {msg.get_content_as_string()}")
    
    # 创建流水线
    stream = env.create_datastream()
    stream.from_source(simple_source).sink(simple_sink)
    
    print(f"✅ 流水线创建，操作符数量: {stream.get_operator_count()}")
    
    # 执行
    print("🚀 开始执行...")
    stream.execute()
    print("✅ 执行完成")
    
    env.close()
    
except Exception as e:
    print(f"❌ 错误: {e}")
    import traceback
    traceback.print_exc()
