# GitHub Actions 文档部署配置指南

## 概述

这个GitHub Actions工作流实现了以下功能：
1. 自动收集项目中的所有README文件
2. 生成适合docsify的文档结构
3. 自动部署到GitHub Pages

## 文件结构

```
.github/
├── workflows/
│   └── deploy-docs.yml          # 主工作流文件
├── scripts/
│   ├── package.json             # Node.js依赖配置
│   ├── collect-readme.js        # README文件收集脚本
│   └── generate-sidebar.js      # 侧边栏生成脚本
└── DEPLOYMENT_SETUP.md          # 本配置说明
```

## 配置步骤

### 1. 启用GitHub Pages

1. 进入GitHub仓库设置页面
2. 滚动到"Pages"部分
3. 在"Source"选项中选择"GitHub Actions"
4. 保存设置

### 2. 配置分支保护（可选但推荐）

为了确保文档质量，建议配置分支保护规则：

1. 进入仓库设置 → Branches
2. 添加对`master`分支的保护规则：
   - 要求状态检查通过
   - 要求分支为最新
   - 包含管理员

### 3. 自定义域名（可选）

如果需要使用自定义域名：

1. 在`deploy-docs.yml`中取消注释CNAME相关行：
   ```yaml
   # 创建CNAME文件（如果需要自定义域名）
   echo "your-domain.com" > docs/CNAME
   ```
2. 将`your-domain.com`替换为您的实际域名
3. 在域名DNS设置中添加CNAME记录指向`username.github.io`

## 工作流程说明

### 触发条件
- 推送到`master`分支时自动触发
- 支持手动触发（workflow_dispatch）

### 执行步骤

1. **检出代码**：获取完整的Git历史记录
2. **设置Node.js环境**：安装Node.js和npm依赖
3. **收集README文件**：
   - 扫描项目中所有README文件
   - 按目录结构组织文件
   - 处理相对路径链接
   - 生成模块文档
4. **生成文档索引**：
   - 创建docsify侧边栏
   - 按分类组织文档
   - 支持中文排序
5. **构建文档站点**：
   - 复制docsify配置文件
   - 创建.nojekyll文件
   - 准备部署制品
6. **部署到GitHub Pages**：
   - 上传文档到Pages服务
   - 生成访问URL

## 文档组织结构

生成的文档按以下结构组织：

```
docs/
├── README.md                    # 项目主页（来自根目录）
├── _sidebar.md                  # 自动生成的侧边栏
├── _navbar.md                   # 导航栏（手动维护）
├── _coverpage.md               # 封面页（手动维护）
├── index.html                  # docsify配置（来自.docsify/）
├── modules/                    # 模块文档
│   ├── component_a.md          # 来自src/component_a/README.md
│   └── component_b.md          # 来自src/component_b/README.md
├── api/                       # API文档（手动维护）
├── examples/                  # 示例文档（手动维护）
├── CONTRIBUTING.md            # 贡献指南（来自根目录）
├── SETUP_GUIDE.md            # 安装指南（手动维护）
└── DEVELOPMENT_WORKFLOW.md   # 开发流程（手动维护）
```

## 自定义配置

### 修改文档收集规则

编辑`.github/scripts/collect-readme.js`：

```javascript
// 修改扫描模式
const patterns = [
  'README.md',
  'README.*.md',
  '**/README.md',
  'docs/*.md',        // 添加新的扫描路径
  'wiki/*.md'         // 添加wiki目录
];

// 修改忽略规则
const ignorePatterns = [
  'node_modules/**',
  '.git/**',
  'build/**',
  'dist/**',
  'temp/**'           // 添加新的忽略路径
];
```

### 自定义侧边栏结构

编辑`.github/scripts/generate-sidebar.js`：

```javascript
// 修改分类逻辑
getCategoryFromPath(relativePath) {
  const dir = path.dirname(relativePath);
  
  if (dir.startsWith('tutorials')) {
    return 'tutorials';  // 添加新分类
  }
  
  // ... 其他分类逻辑
}

// 修改文档排序
getDocumentOrder(relativePath) {
  const filename = path.basename(relativePath);
  
  if (filename === 'QUICK_START.md') return 1;  // 添加新的排序规则
  
  // ... 其他排序逻辑
}
```

### 修改docsify配置

主要配置在`.docsify/index.html`中：

```javascript
window.$docsify = {
  name: 'EmbedKit',
  repo: 'https://github.com/zuoliangyu/EmbedKit',
  
  // 修改主题
  themeable: {
    readyTransition: true,
    responsiveTables: true
  },
  
  // 添加插件
  plugins: [
    // 自定义插件
  ]
};
```

## 性能优化

### 缓存优化
工作流使用了以下缓存机制：
- Node.js模块缓存
- 自动清理15分钟缓存

### 并行处理
脚本支持并行处理多个文件，提高执行效率。

### 增量更新
仅在文件发生变化时更新文档，避免不必要的重建。

## 安全考虑

### 权限设置
工作流使用最小权限原则：
- `contents: read` - 读取仓库内容
- `pages: write` - 写入Pages服务
- `id-token: write` - 用于认证

### 依赖安全
- 仅安装生产依赖（`--only=production`）
- 固定版本号避免供应链攻击
- 使用GitHub官方Actions

## 监控和调试

### 查看执行日志
1. 进入仓库的"Actions"标签页
2. 选择具体的工作流运行
3. 查看详细的执行日志

### 常见问题排查
- 检查Node.js版本兼容性
- 验证文件权限设置
- 确认依赖安装成功

## 维护指南

### 定期更新
- 定期更新Node.js版本
- 更新npm依赖包
- 检查GitHub Actions版本

### 备份重要配置
建议将以下文件纳入版本控制：
- `.docsify/index.html`
- `docs/_navbar.md`
- `docs/_coverpage.md`
- 手动维护的文档文件

### 文档质量检查
建议定期检查：
- 链接有效性
- 图片加载
- 中文编码
- 移动端适配