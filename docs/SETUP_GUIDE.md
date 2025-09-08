# EmbedKit 配置指导

## 📋 快速开始

### 1. 项目结构设置
```
EmbedKit/
├── .github/
│   ├── ISSUE_TEMPLATE/
│   │   ├── bug_report.md
│   │   ├── feature_request.md
│   │   └── config.yml
│   └── pull_request_template.md
├── docs/
│   ├── DEVELOPMENT_WORKFLOW.md
│   ├── CODE_REVIEW_CHECKLIST.md
│   └── SETUP_GUIDE.md
├── src/                    # 源代码目录
├── include/               # 头文件目录
├── tests/                 # 测试代码目录
├── examples/              # 示例代码目录
└── README.md
```

### 2. 分支设置
```bash
# 创建主要分支
git checkout -b preview
git push -u origin preview

git checkout -b develop/zuolan
git push -u origin develop/zuolan

git checkout -b develop/nn  
git push -u origin develop/nn

# 回到master分支
git checkout master
```

### 3. 分支保护规则配置
在GitHub仓库的Settings → Branches中配置：

#### Master分支保护
- ✅ Require pull request reviews before merging
- ✅ Require review from code owners
- ✅ Dismiss stale PR reviews when new commits are pushed
- ✅ Require status checks to pass before merging
- ✅ Require branches to be up to date before merging
- ✅ Include administrators

#### Preview分支保护  
- ✅ Require pull request reviews before merging
- ✅ Allow force pushes (for develop branch merging)

## 🔧 开发环境配置

### Git配置
```bash
# 设置用户信息
git config user.name "Your Name"
git config user.email "your.email@example.com"

# 设置默认编辑器
git config core.editor "code --wait"

# 设置行结束符处理
git config core.autocrlf input  # Linux/Mac
git config core.autocrlf true   # Windows

# 设置默认分支名称
git config init.defaultBranch master
```

### 推荐的.gitignore配置
```gitignore
# 编译输出
*.o
*.obj
*.elf
*.bin
*.hex
*.map
*.lst
*.dis

# 调试文件
*.pdb
*.ilk

# IDE和编辑器
.vscode/
*.suo
*.user
*.sln.docstates
.vs/

# Keil MDK-ARM
*.uvguix.*
*.uvoptx
*.bak
*.dep
*.axf

# IAR
Debug/
Release/
*.eww
*.ewd
*.ewl
*.ewp
*.ewt

# STM32CubeIDE
.metadata/
.settings/

# 临时文件
*~
.DS_Store
Thumbs.db

# 构建目录
build/
dist/
out/
```

## 👥 团队协作配置

### 代码所有者配置 (CODEOWNERS)
```bash
# 创建CODEOWNERS文件
cat > .github/CODEOWNERS << 'EOF'
# 全局代码所有者
* @zuolan

# 核心模块需要特别审查
/src/core/ @zuolan
/include/core/ @zuolan

# 文档更新
/docs/ @zuolan
README.md @zuolan
EOF
```

### GitHub标签配置
在Issues → Labels中配置标签：

#### 类型标签
- `bug` - 🐛 Bug报告 (红色: #d73a4a)
- `enhancement` - ✨ 新功能 (绿色: #a2eeef)  
- `documentation` - 📚 文档 (蓝色: #0075ca)
- `refactor` - 🔧 重构 (黄色: #fbca04)
- `performance` - ⚡ 性能 (橙色: #ff9500)
- `test` - 🧪 测试 (紫色: #7057ff)

#### 优先级标签
- `priority:high` - 🔴 高优先级 (红色: #b60205)
- `priority:medium` - 🟡 中优先级 (黄色: #fbca04)
- `priority:low` - 🟢 低优先级 (绿色: #0e8a16)

#### 硬件平台标签
- `platform:arm-m0` - ARM Cortex-M0
- `platform:arm-m4` - ARM Cortex-M4  
- `platform:riscv` - RISC-V
- `platform:8bit` - 8位MCU

## 🔍 代码质量工具配置

### clang-format配置
创建`.clang-format`文件：
```yaml
BasedOnStyle: Google
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
AlwaysBreakAfterReturnType: None
BreakBeforeBraces: Linux
IndentCaseLabels: true
SpaceAfterCStyleCast: true
AlignTrailingComments: true
```

### Doxygen配置
```bash
# 生成Doxygen配置文件
doxygen -g
```

主要配置修改：
```
PROJECT_NAME = "EmbedKit"
PROJECT_BRIEF = "Embedded C Library"
OUTPUT_LANGUAGE = Chinese
EXTRACT_ALL = YES
EXTRACT_PRIVATE = NO
GENERATE_LATEX = NO
HAVE_DOT = YES
UML_LOOK = YES
```

### 编译配置模板
创建`CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.12)
project(EmbedKit C)

# 编译选项
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstack-usage=512")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")

# 包含目录
include_directories(include)

# 源文件
file(GLOB_RECURSE SOURCES "src/*.c")

# 生成库文件
add_library(embedkit STATIC ${SOURCES})

# 测试配置
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
```

## 📊 持续集成配置（可选）

如果需要自动化测试，可以配置GitHub Actions：

创建`.github/workflows/ci.yml`:
```yaml
name: CI

on:
  push:
    branches: [ master, preview ]
  pull_request:
    branches: [ master ]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y gcc-arm-none-eabi
    
    - name: Build
      run: |
        mkdir build
        cd build
        cmake ..
        make
    
    - name: Run static analysis
      run: |
        # 运行静态分析工具
        cppcheck --error-exitcode=1 src/
    
    - name: Generate documentation
      run: |
        doxygen Doxyfile
```

## 🎯 使用建议

### 日常开发流程
1. **开始工作**: 从个人develop分支开始
2. **功能完成**: 合并到preview分支进行集成测试  
3. **创建PR**: 从preview到master
4. **代码审查**: 使用提供的检查清单
5. **合并发布**: 审查通过后合并到master

### 最佳实践
- 🔄 定期同步master分支到个人分支
- 📝 提交信息遵循约定格式
- 🧪 每个PR都要经过充分测试  
- 📚 重要变更更新文档
- 🏷️ 合理使用GitHub标签分类

### 常见问题解决
```bash
# 解决合并冲突
git checkout preview
git pull origin preview
git merge develop/zuolan
# 解决冲突后
git commit
git push origin preview

# 重置错误的提交
git reset --hard HEAD~1
git push --force-with-lease origin develop/zuolan

# 同步fork的仓库
git remote add upstream https://github.com/original/repo.git
git fetch upstream
git checkout master
git merge upstream/master
```

## 📞 支持和帮助

- 📖 查看项目文档：`docs/`目录
- 💬 讨论问题：GitHub Discussions
- 🐛 报告Bug：使用Bug报告模板
- ✨ 功能请求：使用功能请求模板

配置完成后，您的EmbedKit项目将拥有完整的开发工作流程和代码审查机制！