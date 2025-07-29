# SAGE Flow 文档中心

本目录包含 SAGE Flow 框架的全部技术文档，按类别组织如下：

## 架构与实现文档

### 核心架构
- **[架构重构总结](./architecture_refactor_summary.md)** - SAGE Flow 重大架构重构的完整记录
- **[操作符架构重构](./operator_architecture_restructuring.md)** - 从继承模式到组合模式的详细转换
- **[类结构分析](./class_structure_analysis.md)** - CandyFlow 与 SAGE Flow 的类结构对比分析
- **[CandyFlow 类分析](./candyflow_class_analysis.md)** - CandyFlow 框架核心类的详细分析

### 实现总结
- **[实现总结](./IMPLEMENTATION_SUMMARY.md)** - SAGE Flow 框架实现的总体概览
- **[Python 绑定总结](./PYTHON_BINDINGS_SUMMARY.md)** - Python C++ 绑定的实现细节

### 代码质量
- **[代码清理报告](./code_cleanup_final_report.md)** - 全面代码清理的详细记录
- **[编码标准](./coding-standards.md)** - SAGE Flow 项目的代码规范和最佳实践

## 文档目的

这些文档记录了 SAGE Flow 框架的发展历程，包括：

1. **重大架构决策** - 为什么从继承模式转向组合模式
2. **实现细节** - 核心组件的技术实现方式  
3. **代码质量提升** - 如何保持代码库的整洁和一致性
4. **开发指南** - 新开发者如何理解和贡献代码

## 使用指南

- **新开发者** - 建议从 `IMPLEMENTATION_SUMMARY.md` 开始了解框架概览
- **架构理解** - 阅读 `architecture_refactor_summary.md` 了解设计决策
- **代码贡献** - 参考 `coding-standards.md` 了解代码规范
- **问题排查** - 查看 `code_cleanup_final_report.md` 了解已知问题和解决方案

## 文档维护

本文档集合反映了 SAGE Flow 框架的当前状态。如有更新需求，请保持文档的准确性和一致性。
