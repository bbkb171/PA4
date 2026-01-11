# PA4


COOL语言语义分析器实现文档

项目概述

本项目实现了COOL（Classroom Object-Oriented Language）语言的语义分析器，负责对COOL程序进行静态语义检查，包括类型检查、继承关系验证、方法重写检查等。

文件结构


.
├── semant.h              # 语义分析器头文件
├── semant.cc             # 语义分析器主要实现
├── cool-tree.h           # AST节点定义
├── cool-tree.handcode.h  # AST节点手写代码
├── symtab.h             # 符号表实现
└── README.md            # 项目说明文档


核心设计

1. 类表（ClassTable）设计

ClassTable 是语义分析的核心数据结构，负责：

• 存储所有类定义信息

• 维护类继承关系

• 提供类型检查功能

• 管理符号表环境

2. 符号表管理

使用两层符号表结构：
• 类表：存储类名到类定义的映射

• 对象环境表：存储变量名到类型的映射

3. 类型系统实现

支持COOL完整的类型系统，包括：
• 基本类型：Int、Bool、String

• 对象类型：用户定义的类

• 特殊类型：SELF_TYPE

关键特性实现

SELF_TYPE处理

SELF_TYPE是COOL语言的核心特性，表示"当前类的类型"。实现中特别注意：
// SELF_TYPE的类型检查规则
bool is_subtype(Symbol child, Symbol parent) {
    if (child == SELF_TYPE && parent == SELF_TYPE) return true;
    if (child == SELF_TYPE) return true;  // SELF_TYPE可赋值给任何类型
    if (parent == SELF_TYPE) return false; // 任何类型不能赋值给SELF_TYPE
    // ... 其他检查
}


方法查找和重写检查

实现了完整的方法继承链查找：
method_class* find_method(Symbol class_name, Symbol method_name) {
    // 在类及其所有祖先中查找方法
    while (current != No_class) {
        // 在当前类中查找
        // 如果未找到，继续在父类中查找
        current = get_parent(current);
    }
    return NULL;
}


类型推断和LUB计算

实现了最小上界（Least Upper Bound）算法：
Symbol lub(Symbol type1, Symbol type2) {
    if (type1 == SELF_TYPE && type2 == SELF_TYPE) return SELF_TYPE;
    if (type1 == SELF_TYPE || type2 == SELF_TYPE) return Object;
    // 寻找最近的共同祖先
    // ...
}


主要功能模块

1. 基本类安装

安装COOL语言的内置基本类：Object、IO、Int、Bool、String。

2. 继承图构建

构建类的继承关系图，检查继承合法性。

3. 语义检查

• 类定义检查

• 继承关系验证

• 方法重写规则检查

• Main类存在性验证

4. 类型检查

对各类表达式进行类型推断和验证：
• 算术表达式

• 方法调用

• 条件表达式

• 赋值表达式

• 块表达式等

实现难点与解决方案

难点1：内存管理

问题：符号表中存储类定义指针时容易产生悬空指针。

解决方案：
// 使用new分配内存，避免局部变量地址失效
Class_ *class_ptr = new Class_(c);
class_table->addid(name, class_ptr);


难点2：SELF_TYPE语义

问题：SELF_TYPE在运行时解析，静态检查需要特殊处理。

解决方案：
• 在方法体中，self的类型设为SELF_TYPE

• 方法调用时，根据上下文解析SELF_TYPE

• 类型比较时，SELF_TYPE有特殊规则

难点3：方法查找

问题：需要沿继承链查找方法，处理重写规则。

解决方案：
• 递归遍历父类链

• 检查参数数量和类型一致性

• 验证返回类型兼容性

编译和测试

编译命令

make clean
make semant


测试方法

# 测试单个文件
./mysemant good.cl

# 对比官方输出
diff <(./lexer good.cl | ./parser | ./semant) <(./lexer good.cl | ./parser | /usr/class/bin/semant)


代码质量保证

1. 错误处理

• 使用统一的错误报告机制

• 提供详细的错误信息定位

• 支持多错误收集和报告

2. 调试支持

• 内置调试输出控制

• 详细的类型检查日志

• 内存泄漏检测支持

3. 代码规范

• 遵循C++最佳实践

• 充分的代码注释

• 模块化的函数设计

性能考虑

1. 符号表优化：使用哈希表实现快速查找
2. 类型缓存：缓存类型检查结果避免重复计算
3. 继承链缓存：优化方法查找性能

扩展性设计

代码结构支持未来扩展：
• 新的表达式类型可以轻松添加

• 类型系统支持新类型添加

• 错误报告机制可扩展

已知限制

1. 当前实现假设输入程序语法正确
2. 某些边界情况的错误消息可能不够详细
3. 性能在极端大型程序上可能需优化

总结

本语义分析器完整实现了COOL语言的静态语义检查功能，正确处理了包括SELF_TYPE在内的复杂类型特性，提供了健壮的错误检查和详细的诊断信息。代码结构清晰，易于维护和扩展。
