# EmbedKit 文档自动化部署系统

## 系统概述

这是一个完整的GitHub Actions自动化工作流，用于收集项目中的README文件并部署到GitHub Pages的docsify文档站点。

## 核心特性

- ✅ **自动收集**：扫描项目中所有README文件
- ✅ **智能分类**：按目录结构自动组织文档
- ✅ **中文支持**：完整支持中文文档和排序
- ✅ **链接修复**：自动处理相对路径链接
- ✅ **零配置**：推送到master分支即自动触发
- ✅ **快速部署**：直接部署到GitHub Pages
- ✅ **故障恢复**：完善的错误处理和调试支持

## 文件结构

```
.github/
├── workflows/
│   └── deploy-docs.yml          # 主工作流文件
├── scripts/                     # 脚本目录
│   ├── package.json            # 依赖配置
│   ├── collect-readme.js       # README收集脚本
│   ├── generate-sidebar.js     # 侧边栏生成脚本
│   └── test-local.js           # 本地测试脚本
├── DEPLOYMENT_SETUP.md         # 部署配置指南
├── TROUBLESHOOTING.md          # 故障排除指南
└── README.md                   # 本文档
```

## 快速开始

### 1. 启用GitHub Pages

1. 进入仓库设置页面
2. 找到"Pages"部分
3. 选择"GitHub Actions"作为源
4. 保存设置

### 2. 推送代码触发部署

```bash
git add .
git commit -m "启用文档自动部署"
git push origin master
```

### 3. 查看部署结果

- 访问Actions页面查看工作流执行状态
- 部署完成后访问`https://username.github.io/repositoryname`

## 本地测试

在推送前可以本地测试：

```bash
cd .github/scripts
npm install
node test-local.js
```

## 工作流程

1. **触发条件**：推送到master分支
2. **收集README**：扫描所有README文件并处理
3. **生成索引**：创建docsify侧边栏结构  
4. **构建站点**：准备docsify配置和资源
5. **部署Pages**：发布到GitHub Pages

## 自定义配置

### 修改收集规则

编辑`scripts/collect-readme.js`：

```javascript
const patterns = [
  'README.md',
  'README.*.md',
  '**/README.md',
  'docs/*.md'        // 添加新的扫描路径
];
```

### 自定义侧边栏

编辑`scripts/generate-sidebar.js`中的分类逻辑：

```javascript
getCategoryFromPath(relativePath) {
  if (relativePath.includes('tutorial')) {
    return 'tutorials';  // 新分类
  }
  // ... 其他逻辑
}
```

### docsify主题

修改`.docsify/index.html`中的配置：

```javascript
window.$docsify = {
  name: '您的项目名称',
  repo: '您的仓库地址',
  // ... 其他配置
};
```

## 技术特点

### 高性能
- 并行处理多个文件
- 智能缓存机制
- 增量更新支持

### 安全性
- 最小权限原则
- 依赖安全扫描
- 官方Actions使用

### 可维护性
- 模块化代码结构
- 详细的错误日志
- 完善的文档支持

## 支持的文件类型

| 文件模式 | 说明 | 示例 |
|---------|------|------|
| `README.md` | 根目录主文档 | `/README.md` |
| `README.*.md` | 多语言README | `/README.zh.md` |
| `**/README.md` | 子目录README | `/src/core/README.md` |
| `*.md` | 其他Markdown文档 | `/CONTRIBUTING.md` |

## 生成的文档结构

```
docs/
├── README.md                    # 项目首页
├── _sidebar.md                  # 自动生成的导航
├── modules/                     # 模块文档
│   ├── core.md                 # 来自src/core/README.md
│   └── utils.md                # 来自src/utils/README.md
├── CONTRIBUTING.md              # 贡献指南
├── SETUP_GUIDE.md              # 安装指南
└── index.html                  # docsify配置
```

## 问题排查

遇到问题时：

1. 查看Actions执行日志
2. 参考`TROUBLESHOOTING.md`文档
3. 运行本地测试验证
4. 检查文件权限和路径

## 维护建议

- **定期更新**：每月检查依赖更新
- **监控性能**：关注构建时间和资源使用
- **备份配置**：重要配置文件纳入版本控制
- **文档质量**：定期检查链接和内容有效性

## 版本历史

- **v1.0.0** - 基础功能实现
  - README自动收集
  - 侧边栏生成
  - GitHub Pages部署

## 贡献指南

欢迎提交Issue和Pull Request来改进这个自动化系统。

## 许可证

本自动化系统遵循项目主许可证。