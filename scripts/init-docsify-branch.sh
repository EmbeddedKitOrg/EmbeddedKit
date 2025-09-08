#!/bin/bash

# docsify分支初始化脚本
# 用于手动设置docsify分支的初始化结构

set -e

echo "🚀 开始初始化docsify分支..."

# 检查是否在正确的仓库中
if [ ! -d ".git" ]; then
    echo "❌ 错误: 当前目录不是git仓库"
    exit 1
fi

# 确保当前在master分支
current_branch=$(git branch --show-current)
if [ "$current_branch" != "master" ]; then
    echo "⚠️  当前分支: $current_branch, 切换到master分支"
    git checkout master
fi

# 检查docsify分支是否存在
if git show-ref --verify --quiet refs/heads/docsify; then
    echo "📦 docsify分支已存在，切换到该分支"
    git checkout docsify
else
    echo "📦 创建新的docsify分支"
    git checkout --orphan docsify
    
    # 清空工作区
    git rm -rf . || true
    
    echo "🧹 清空工作区完成"
fi

# 创建基本的docsify目录结构
mkdir -p docs/{modules,api,examples,assets}

echo "📁 创建目录结构..."

# 创建主页
cat > docs/README.md << 'EOF'
# EmbedKit 文档

欢迎使用 EmbedKit - 轻量级嵌入式开发工具包！

## 快速导航

- [📖 开始使用](#开始使用)
- [🏗️ 模块介绍](#模块介绍) 
- [💡 示例代码](#示例代码)
- [📚 API文档](#api文档)

## 开始使用

EmbedKit 是一个专为资源受限的嵌入式系统设计的轻量级C语言库。

### 主要特性

- 🚀 **极致轻量** - 最小ROM占用，可配置的RAM使用
- 🔧 **模块化设计** - 按需引入，零依赖架构  
- ⚡ **高性能** - 针对嵌入式处理器优化
- 🛡️ **安全可靠** - 静态内存分配，可预测执行时间

### 快速开始

```c
#include "embedkit.h"

int main() {
    // 初始化EmbedKit
    embedkit_init();
    
    // 你的代码...
    
    return 0;
}
```

## 模块介绍

### 核心模块

- [内存管理](modules/memory.md) - 高效的内存池和堆管理
- [数据结构](modules/data_structures.md) - 优化的链表、队列等
- [任务调度](modules/scheduler.md) - 轻量级协作式调度器

查看左侧导航了解更多模块详情。

## 示例代码

- [基础示例](examples/basic.md) - 快速上手示例
- [高级示例](examples/advanced.md) - 复杂应用场景

## API文档  

完整的API参考文档请查看[API章节](api/README.md)。

---

> 📝 **文档说明**: 本文档从master分支自动收集生成，如有问题请在[GitHub](https://github.com/zuoliangyu/EmbedKit)上反馈。
EOF

# 创建侧边栏
cat > docs/_sidebar.md << 'EOF'
* [首页](/)

* **快速开始**
  * [安装指南](getting-started/installation.md)
  * [第一个程序](getting-started/first-program.md)
  * [配置选项](getting-started/configuration.md)

* **核心模块**
  * [内存管理](modules/memory.md)
  * [数据结构](modules/data_structures.md) 
  * [任务调度](modules/scheduler.md)

* **示例代码**
  * [基础示例](examples/basic.md)
  * [高级示例](examples/advanced.md)

* **API文档**  
  * [API概览](api/README.md)
  * [核心API](api/core.md)
  * [工具API](api/utils.md)

* **更多**
  * [常见问题](faq.md)
  * [更新日志](changelog.md)
  * [贡献指南](contributing.md)
EOF

# 创建导航栏
cat > docs/_navbar.md << 'EOF'
* [🏠 首页](/)
* [🚀 快速开始](getting-started/installation.md)
* [📖 GitHub](https://github.com/zuoliangyu/EmbedKit)
* [🐛 问题反馈](https://github.com/zuoliangyu/EmbedKit/issues)
* [💬 讨论](https://github.com/zuoliangyu/EmbedKit/discussions)
EOF

# 创建封面页
cat > docs/_coverpage.md << 'EOF'
![logo](assets/logo.png)

# EmbedKit

> 轻量级、模块化的嵌入式开发工具包

- 极致轻量，适合资源受限环境
- 模块化设计，按需引入
- 高性能，针对嵌入式优化
- 详细文档和丰富示例

[快速开始](getting-started/installation.md)
[查看GitHub](https://github.com/zuoliangyu/EmbedKit)

<!-- 背景图片 -->
<!-- ![](assets/bg.png) -->

<!-- 背景色 -->
![color](#2c3e50)
EOF

# 创建404页面
cat > docs/_404.md << 'EOF'
# 页面未找到

很抱歉，您访问的页面不存在。

## 可能的原因

- 页面地址输入错误
- 页面已被删除或移动
- 链接已过期

## 建议

- 检查URL拼写是否正确
- 返回[首页](/)重新导航
- 使用搜索功能查找相关内容
- 在[GitHub Issues](https://github.com/zuoliangyu/EmbedKit/issues)反馈问题

---

[🏠 返回首页](/) | [📚 查看文档](/) | [🐛 报告问题](https://github.com/zuoliangyu/EmbedKit/issues)
EOF

# 创建docsify配置文件
cat > docs/index.html << 'EOF'
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <title>EmbedKit - 轻量级嵌入式开发工具包</title>
  <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" />
  <meta name="description" content="EmbedKit - 轻量级、模块化的嵌入式C语言开发库，专为资源受限的嵌入式系统设计">
  <meta name="keywords" content="嵌入式,C语言,开发库,内存管理,任务调度,数据结构">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, minimum-scale=1.0">
  
  <!-- 主题样式 -->
  <link rel="stylesheet" href="//cdn.jsdelivr.net/npm/docsify@4/lib/themes/vue.css">
  
  <!-- 自定义样式 -->
  <style>
    :root {
      --base-color: #2c3e50;
      --sidebar-width: 300px;
    }
    
    .app-name-link img {
      height: 40px;
    }
    
    .sidebar-toggle {
      background: transparent;
    }
    
    .markdown-section {
      max-width: none;
    }
    
    .markdown-section pre {
      background-color: #f8f9fa;
    }
    
    .markdown-section code {
      color: #e74c3c;
      background-color: #f8f9fa;
      padding: 2px 4px;
      border-radius: 3px;
    }
    
    /* 表格样式优化 */
    .markdown-section table {
      border-collapse: collapse;
      width: 100%;
      margin: 1em 0;
    }
    
    .markdown-section table th,
    .markdown-section table td {
      border: 1px solid #ddd;
      padding: 8px 12px;
      text-align: left;
    }
    
    .markdown-section table th {
      background-color: #f8f9fa;
      font-weight: 600;
    }
    
    /* 徽章样式 */
    .badge {
      display: inline-block;
      padding: 3px 7px;
      font-size: 12px;
      font-weight: 700;
      line-height: 1;
      color: #fff;
      text-align: center;
      white-space: nowrap;
      vertical-align: baseline;
      border-radius: 10px;
      margin-right: 5px;
    }
    
    .badge-success { background-color: #28a745; }
    .badge-info { background-color: #17a2b8; }
    .badge-warning { background-color: #ffc107; color: #212529; }
    .badge-danger { background-color: #dc3545; }
  </style>
  
  <!-- Favicon -->
  <link rel="icon" href="data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>🛠️</text></svg>">
</head>
<body>
  <div id="app">📖 加载中...</div>
  
  <script>
    window.$docsify = {
      // 基本配置
      name: 'EmbedKit',
      repo: 'https://github.com/zuoliangyu/EmbedKit',
      loadSidebar: true,
      loadNavbar: true,
      coverpage: true,
      onlyCover: false,
      
      // 导航配置
      auto2top: true,
      homepage: 'README.md',
      maxLevel: 4,
      subMaxLevel: 3,
      
      // 搜索配置
      search: {
        maxAge: 86400000,
        paths: 'auto',
        placeholder: '🔍 搜索文档...',
        noData: '😔 没有找到结果',
        depth: 6,
        hideOtherSidebarContent: false
      },
      
      // 代码复制
      copyCode: {
        buttonText: '📋 复制代码',
        errorText: '❌ 复制失败',
        successText: '✅ 复制成功'
      },
      
      // 分页导航
      pagination: {
        previousText: '⬅️ 上一页',
        nextText: '下一页 ➡️',
        crossChapter: true,
        crossChapterText: true
      },
      
      // 字数统计
      count: {
        countable: true,
        fontsize: '0.9em',
        color: 'rgb(90,90,90)',
        language: 'chinese'
      },
      
      // 代码主题
      themeable: {
        readyTransition: true,
        responsiveTables: true
      },
      
      // 页脚
      plugins: [
        function(hook, vm) {
          hook.beforeEach(function (html) {
            var url = 'https://github.com/zuoliangyu/EmbedKit/blob/master/' + vm.route.file;
            var editHtml = '📝 [编辑此页面](' + url + ')\n';
            return editHtml + html;
          });
          
          hook.afterEach(function (html) {
            var footer = [
              '<footer style="text-align: center; margin-top: 50px; padding: 20px; border-top: 1px solid #eee;">',
              '<p>📖 文档更新时间: ' + new Date().toLocaleString('zh-CN') + '</p>',
              '<p>🛠️ Made with ❤️ for embedded developers</p>',
              '<p><a href="https://github.com/zuoliangyu/EmbedKit" target="_blank">⭐ 给个Star支持一下</a></p>',
              '</footer>'
            ].join('');
            return html + footer;
          });
        }
      ]
    }
  </script>
  
  <!-- Docsify核心 -->
  <script src="//cdn.jsdelivr.net/npm/docsify@4"></script>
  
  <!-- 插件 -->
  <script src="//cdn.jsdelivr.net/npm/docsify/lib/plugins/search.min.js"></script>
  <script src="//cdn.jsdelivr.net/npm/docsify-copy-code@2"></script>
  <script src="//cdn.jsdelivr.net/npm/docsify-pagination@2/dist/docsify-pagination.min.js"></script>
  <script src="//cdn.jsdelivr.net/npm/docsify/lib/plugins/zoom-image.min.js"></script>
  <script src="//cdn.jsdelivr.net/npm/docsify-count/dist/countable.min.js"></script>
  
  <!-- 语法高亮 -->
  <script src="//cdn.jsdelivr.net/npm/prismjs@1/components/prism-c.min.js"></script>
  <script src="//cdn.jsdelivr.net/npm/prismjs@1/components/prism-cpp.min.js"></script>
  <script src="//cdn.jsdelivr.net/npm/prismjs@1/components/prism-bash.min.js"></script>
  <script src="//cdn.jsdelivr.net/npm/prismjs@1/components/prism-json.min.js"></script>
  <script src="//cdn.jsdelivr.net/npm/prismjs@1/components/prism-makefile.min.js"></script>
  
  <!-- PWA支持 -->
  <script>
    if (typeof navigator.serviceWorker !== 'undefined') {
      navigator.serviceWorker.register('sw.js')
    }
  </script>
</body>
</html>
EOF

# 创建 .nojekyll 文件
touch docs/.nojekyll

# 创建 .gitignore
cat > docs/.gitignore << 'EOF'
# 临时文件
*.tmp
*.temp
*~

# 系统文件
.DS_Store
Thumbs.db

# 编辑器文件
.vscode/
.idea/
*.swp
*.swo

# 日志文件
*.log
EOF

# 添加基础文件到git
git add .
git status

echo ""
echo "✅ docsify分支初始化完成!"
echo ""
echo "📁 创建的目录结构:"
echo "docs/"
echo "├── README.md          # 主页"
echo "├── _sidebar.md        # 侧边栏导航"
echo "├── _navbar.md         # 顶部导航"
echo "├── _coverpage.md      # 封面页"
echo "├── _404.md            # 404页面"
echo "├── index.html         # docsify配置"
echo "├── .nojekyll          # GitHub Pages配置"
echo "├── modules/           # 模块文档目录"
echo "├── api/               # API文档目录"
echo "├── examples/          # 示例目录"
echo "└── assets/            # 静态资源目录"
echo ""
echo "📝 下一步操作:"
echo "1. git commit -m \"初始化docsify分支\""
echo "2. git push -u origin docsify"
echo "3. 在GitHub仓库设置中启用Pages，选择docsify分支的docs目录"
echo ""
echo "🌐 完成后可通过 https://<username>.github.io/<repository>/ 访问文档"
EOF