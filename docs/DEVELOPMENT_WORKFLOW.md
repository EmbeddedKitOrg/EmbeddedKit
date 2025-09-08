# EmbedKit 开发工作流程

## 🚀 工作流程概述

```
develop/zuolan ──┐
                 ├─→ preview ──→ PR审查 ──→ master
develop/nn ──────┘
```

### 分支策略
- `master`: 主分支，稳定版本
- `preview`: 预览分支，集成测试
- `develop/zuolan`: zuolan开发分支
- `develop/nn`: nn开发分支

## 👥 开发流程

### 1. 开始新功能开发
```bash
# 从master创建个人开发分支
git checkout master
git pull origin master
git checkout -b develop/zuolan/feature-name
# 或
git checkout -b develop/nn/feature-name
```

### 2. 日常开发
```bash
# 提交代码
git add .
git commit -m "feat: 添加新功能描述"
git push origin develop/zuolan/feature-name
```

### 3. 合并到preview分支
```bash
# 切换到preview分支
git checkout preview
git pull origin preview

# 合并开发分支
git merge develop/zuolan/feature-name
git push origin preview
```

### 4. 创建Pull Request
- 从 `preview` 分支创建到 `master` 分支的PR
- 使用PR模板填写详细信息
- 指定审查者（项目所有者）

### 5. 代码审查和合并
- 审查者检查代码质量
- 进行手动测试验证
- 审查通过后合并到master

## 📏 代码规范

### 文件结构限制
- **代码文件行数**: 每个C文件不超过250行
- **文件夹文件数**: 每个文件夹不超过8个文件
- **模块化设计**: 功能独立，接口清晰

### 提交消息规范
```
<type>(<scope>): <description>

<body>

<footer>
```

#### 类型 (type)
- `feat`: 新功能
- `fix`: Bug修复
- `docs`: 文档更新
- `refactor`: 代码重构
- `perf`: 性能优化
- `test`: 测试相关
- `chore`: 构建或辅助工具变更

#### 示例
```
feat(uart): 添加DMA传输支持

- 实现UART DMA发送功能
- 添加DMA接收缓冲区管理
- 优化内存使用效率

Closes #123
```

## 🧪 测试流程

### 手动测试检查清单
#### 功能测试
- [ ] API接口正确性
- [ ] 边界条件处理
- [ ] 错误处理机制
- [ ] 硬件兼容性

#### 性能测试
- [ ] 内存使用量测试
- [ ] 执行时间测试
- [ ] 栈使用深度测试
- [ ] 实时性验证

#### 集成测试
- [ ] 多模块协作测试
- [ ] 中断处理测试
- [ ] 并发安全性测试
- [ ] 长时间运行稳定性

### 测试环境搭建
```c
// 测试配置示例
#define TEST_PLATFORM_STM32F4
#define TEST_CLOCK_84MHZ
#define TEST_RAM_128KB
#define TEST_FLASH_512KB
```

## 🔍 代码审查检查清单

### 架构设计
- [ ] 模块划分合理
- [ ] 接口设计清晰
- [ ] 依赖关系简单
- [ ] 可扩展性良好

### 代码质量
- [ ] 命名规范一致
- [ ] 注释充分恰当
- [ ] 错误处理完善
- [ ] 代码格式统一

### 嵌入式特性
- [ ] 内存管理安全
- [ ] 栈使用合理
- [ ] 中断安全考虑
- [ ] 硬件抽象恰当

### 性能考虑
- [ ] 关键路径优化
- [ ] 内存访问效率
- [ ] 算法复杂度合理
- [ ] 缓存友好设计

## 🚨 发布流程

### 版本号规范
- `MAJOR.MINOR.PATCH`
- `1.0.0` - 主要版本
- `1.1.0` - 次要版本
- `1.1.1` - 补丁版本

### 发布步骤
1. 确保所有测试通过
2. 更新版本号和变更日志
3. 创建release标签
4. 生成发布包
5. 更新文档

## 🛠️ 工具和环境

### 推荐开发工具
- **IDE**: Keil MDK-ARM、IAR、STM32CubeIDE
- **代码格式化**: clang-format
- **版本控制**: Git
- **文档**: Doxygen

### 编译配置
```makefile
# 推荐编译选项
CFLAGS = -Wall -Wextra -std=c99 -O2
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wstack-usage=512
```

## 📚 相关文档
- [代码规范](CODE_STYLE.md)
- [API文档](API_REFERENCE.md)
- [测试指南](TESTING_GUIDE.md)
- [部署指南](DEPLOYMENT.md)