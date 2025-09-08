# 贡献指南

感谢您对 EmbedKit 项目的关注！我们欢迎并感谢所有形式的贡献。

## 目录

- [行为准则](#行为准则)
- [如何贡献](#如何贡献)
- [开发流程](#开发流程)
- [代码规范](#代码规范)
- [提交规范](#提交规范)
- [测试要求](#测试要求)
- [文档要求](#文档要求)

## 行为准则

### 我们的承诺

为了营造一个开放和友好的环境，我们承诺：

- 使用友好和包容的语言
- 尊重不同的观点和经验
- 优雅地接受建设性批评
- 关注什么对社区最有利
- 对其他社区成员表示同理心

### 不可接受的行为

- 使用性化语言或图像
- 恶意攻击、侮辱性评论
- 公开或私下骚扰
- 未经许可发布他人隐私信息
- 其他不道德或不专业的行为

## 如何贡献

### 报告问题

1. 检查[现有 Issues](https://github.com/zuoliangyu/EmbedKit/issues)避免重复
2. 使用[问题模板](.github/ISSUE_TEMPLATE)创建新 Issue
3. 提供详细的问题描述：
   - 环境信息（MCU型号、编译器版本等）
   - 重现步骤
   - 期望行为
   - 实际行为
   - 相关代码片段

### 提出新功能

1. 先在 [Discussions](https://github.com/zuoliangyu/EmbedKit/discussions) 中讨论
2. 说明功能的使用场景和价值
3. 考虑对现有代码的影响
4. 评估内存和性能影响

### 提交代码

1. Fork 本仓库
2. 基于 `develop` 分支创建特性分支
3. 编写代码并添加测试
4. 确保通过所有测试
5. 提交 Pull Request

## 开发流程

### 1. 环境准备

```bash
# 克隆仓库
git clone https://github.com/zuoliangyu/EmbedKit.git
cd EmbedKit

# 创建开发分支
git checkout -b feature/your-feature-name

# 安装开发工具（可选）
make setup
```

### 2. 分支策略

```
master
  └── develop
       ├── feature/xxx  (新功能)
       ├── bugfix/xxx   (错误修复)
       └── hotfix/xxx   (紧急修复)
```

- `master`: 稳定发布版本
- `develop`: 开发主分支
- `feature/*`: 新功能开发
- `bugfix/*`: 错误修复
- `hotfix/*`: 紧急修复

### 3. 开发流程

```bash
# 1. 同步上游代码
git fetch upstream
git rebase upstream/develop

# 2. 开发功能
# ... 编写代码 ...

# 3. 运行测试
make test

# 4. 代码格式化
make format

# 5. 静态分析
make lint

# 6. 提交代码
git add .
git commit -m "feat: add new feature"

# 7. 推送到 fork
git push origin feature/your-feature-name
```

## 代码规范

### C 语言编码规范

#### 命名约定

```c
// 宏定义：全大写，下划线分隔
#define MAX_BUFFER_SIZE 256
#define EK_SUCCESS 0

// 类型定义：小写，下划线分隔，_t 结尾
typedef struct {
    uint32_t id;
    char name[32];
} user_info_t;

// 函数：模块前缀_动作_对象
ek_pool_t* ek_pool_create(size_t size);
void ek_pool_destroy(ek_pool_t* pool);

// 全局变量：g_ 前缀
extern uint32_t g_system_tick;

// 静态变量：s_ 前缀
static uint32_t s_instance_count = 0;

// 枚举：大写，下划线分隔
typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_ERROR
} system_state_t;
```

#### 代码格式

```c
// 缩进：4 个空格
// 大括号：K&R 风格
if (condition) {
    do_something();
} else {
    do_other();
}

// 函数定义
/**
 * @brief 函数简要说明
 * @param param1 参数1说明
 * @param param2 参数2说明
 * @return 返回值说明
 */
int function_name(int param1, char* param2)
{
    // 函数体
    return 0;
}

// 指针声明：* 靠近变量名
char* str;
int* numbers;

// 条件语句：明确比较
if (ptr != NULL) {  // 好
if (ptr) {          // 避免

// 常量比较：常量在左
if (NULL == ptr) {  // 防止误写为赋值
if (5 == value) {
```

#### 注释规范

```c
/**
 * @file module_name.h
 * @brief 模块简要说明
 * @author 作者
 * @date 2024-01-01
 */

/**
 * @brief 函数详细说明
 * 
 * 更详细的功能描述，包括：
 * - 功能点1
 * - 功能点2
 * 
 * @param[in]  input  输入参数
 * @param[out] output 输出参数
 * @param[in,out] data 输入输出参数
 * 
 * @return 返回值说明
 * @retval 0 成功
 * @retval -1 失败
 * 
 * @note 注意事项
 * @warning 警告信息
 * 
 * @code
 * // 使用示例
 * int result = function_name(input, &output, data);
 * @endcode
 */

/* 多行注释
 * 第二行
 * 第三行
 */

// 单行注释
```

### 文件组织

```c
/* module_name.h */
#ifndef EK_MODULE_NAME_H
#define EK_MODULE_NAME_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "embedkit/common.h"

/* Exported types ------------------------------------------------------------*/
typedef struct {
    uint32_t field1;
    uint32_t field2;
} module_data_t;

/* Exported constants --------------------------------------------------------*/
#define MODULE_MAX_SIZE 100

/* Exported macros -----------------------------------------------------------*/
#define MODULE_CALC(x) ((x) * 2)

/* Exported functions --------------------------------------------------------*/
void module_init(void);
int module_process(module_data_t* data);

#ifdef __cplusplus
}
#endif

#endif /* EK_MODULE_NAME_H */
```

## 提交规范

### Commit Message 格式

```
<type>(<scope>): <subject>

<body>

<footer>
```

#### Type 类型

- `feat`: 新功能
- `fix`: 错误修复
- `docs`: 文档更新
- `style`: 代码格式（不影响功能）
- `refactor`: 重构
- `perf`: 性能优化
- `test`: 测试相关
- `chore`: 构建过程或辅助工具
- `revert`: 回滚提交

#### 示例

```bash
feat(scheduler): add priority-based task scheduling

- Implement priority queue for tasks
- Add preemption support
- Update documentation

Closes #123
```

```bash
fix(memory): fix memory leak in pool allocator

The pool allocator was not properly freeing memory blocks
when the pool was destroyed. This commit adds proper cleanup
logic to prevent memory leaks.

Fixes #456
```

## 测试要求

### 单元测试

```c
// test_module_name.c
#include "unity.h"
#include "embedkit/module_name.h"

void setUp(void) {
    // 测试前设置
}

void tearDown(void) {
    // 测试后清理
}

void test_module_function_should_return_zero(void) {
    // Arrange
    int input = 42;
    
    // Act
    int result = module_function(input);
    
    // Assert
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_module_function_should_handle_null(void) {
    // Act & Assert
    TEST_ASSERT_EQUAL_INT(-1, module_function(NULL));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_module_function_should_return_zero);
    RUN_TEST(test_module_function_should_handle_null);
    return UNITY_END();
}
```

### 集成测试

```c
// 测试多个模块的交互
void test_scheduler_with_memory_pool(void) {
    // 初始化内存池
    ek_pool_t* pool = ek_pool_create(1024, 64);
    
    // 创建任务
    task_t* task = ek_pool_alloc(pool);
    TEST_ASSERT_NOT_NULL(task);
    
    // 添加到调度器
    ek_scheduler_add_task(task);
    
    // 运行调度器
    ek_scheduler_run_once();
    
    // 验证结果
    TEST_ASSERT_TRUE(task->executed);
    
    // 清理
    ek_pool_free(pool, task);
    ek_pool_destroy(pool);
}
```

### 性能测试

```c
void test_performance_memory_allocation(void) {
    uint32_t start_time = get_tick_count();
    
    for (int i = 0; i < 1000; i++) {
        void* ptr = ek_malloc(128);
        TEST_ASSERT_NOT_NULL(ptr);
        ek_free(ptr);
    }
    
    uint32_t elapsed = get_tick_count() - start_time;
    TEST_ASSERT_LESS_THAN(100, elapsed);  // 应在100ms内完成
}
```

## 文档要求

### API 文档

每个公共 API 必须有完整的文档：

```c
/**
 * @brief 创建内存池
 * 
 * 创建一个固定大小块的内存池，用于高效的内存分配。
 * 
 * @param buffer 内存缓冲区指针
 * @param buffer_size 缓冲区总大小（字节）
 * @param block_size 每个块的大小（字节）
 * 
 * @return 成功返回内存池句柄，失败返回 NULL
 * 
 * @note block_size 必须至少为 sizeof(void*)
 * @warning buffer 必须在内存池生命周期内保持有效
 * 
 * @code
 * uint8_t buffer[1024];
 * ek_pool_t* pool = ek_pool_create(buffer, 1024, 64);
 * if (pool != NULL) {
 *     // 使用内存池
 * }
 * @endcode
 * 
 * @see ek_pool_destroy()
 * @since v1.0.0
 */
ek_pool_t* ek_pool_create(void* buffer, size_t buffer_size, size_t block_size);
```

### 模块文档

在 `docs/modules/` 目录下为每个模块创建 Markdown 文档：

```markdown
# 模块名称

## 概述
模块功能的简要描述。

## 特性
- 特性1
- 特性2

## 使用方法
### 基础用法
\```c
// 代码示例
\```

### 高级用法
\```c
// 代码示例
\```

## API 参考
### 函数列表
- `function1()` - 功能描述
- `function2()` - 功能描述

## 配置选项
| 宏定义 | 默认值 | 说明 |
|--------|--------|------|
| CONFIG_1 | 100 | 配置说明 |

## 性能指标
| 操作 | 时间复杂度 | 空间复杂度 |
|------|------------|------------|
| 分配 | O(1) | O(1) |

## 常见问题
Q: 问题1？
A: 答案1。

## 相关链接
- [API 文档](../api/module.md)
- [示例代码](../examples/module.md)
```

## Pull Request 流程

### PR 检查清单

提交 PR 前，请确保：

- [ ] 代码符合编码规范
- [ ] 添加了必要的测试
- [ ] 所有测试通过
- [ ] 更新了相关文档
- [ ] 提交信息符合规范
- [ ] 没有引入新的警告
- [ ] 考虑了向后兼容性
- [ ] 在目标平台上测试过

### PR 模板

```markdown
## 描述
简要描述这个 PR 的目的和改动。

## 改动类型
- [ ] Bug 修复
- [ ] 新功能
- [ ] 破坏性变更
- [ ] 文档更新

## 测试
- [ ] 单元测试
- [ ] 集成测试
- [ ] 手动测试

## 检查清单
- [ ] 代码自审
- [ ] 文档更新
- [ ] 测试通过

## 相关 Issue
Closes #xxx

## 截图（如适用）
```

## 发布流程

### 版本号规范

遵循[语义化版本](https://semver.org/lang/zh-CN/)：

- `MAJOR.MINOR.PATCH`
- MAJOR: 不兼容的 API 变更
- MINOR: 向后兼容的功能新增
- PATCH: 向后兼容的问题修复

### 发布检查清单

- [ ] 更新版本号
- [ ] 更新 CHANGELOG.md
- [ ] 运行完整测试套件
- [ ] 构建文档
- [ ] 创建 Git tag
- [ ] 发布 Release Notes

## 获取帮助

- 📖 查看[文档](docs/)
- 💬 加入[讨论](https://github.com/zuoliangyu/EmbedKit/discussions)
- 🐛 报告[问题](https://github.com/zuoliangyu/EmbedKit/issues)
- 📧 联系维护者：embedkit@example.com

## 致谢

感谢所有贡献者的付出！

特别感谢：
- 代码贡献者
- 文档编写者
- 测试人员
- 提出建议的用户

## 许可证

通过贡献代码，您同意您的贡献将按照项目的 [MIT 许可证](LICENSE) 进行许可。