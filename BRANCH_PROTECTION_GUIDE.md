# EmbedKit 分支保护策略配置指导

## 项目概述

- **仓库**: zuoliangyu/EmbedKit
- **主分支**: master
- **开发分支**: develop/zuolan, develop/nn  
- **预览分支**: preview
- **测试分支**: test (临时)
- **文档分支**: docsify (自动部署目标)

## 工作流程架构

```
develop/zuolan ──┐
                 ├─→ preview ─→ PR ─→ master ─→ docsify (自动部署)
develop/nn ──────┘                ↑
                                人工审查
                                (仅所有者)
```

## 1. 分支保护规则配置

### 1.1 保护 master 分支

**导航路径**: `Settings` > `Branches` > `Add rule`

**分支名称模式**: `master`

**保护设置**:
- ✅ **Require a pull request before merging**
  - ✅ Require approvals: `1`
  - ✅ Dismiss stale PR approvals when new commits are pushed
  - ✅ Require review from code owners (需配置 CODEOWNERS 文件)
  - ❌ Allow specified actors to bypass required pull requests (确保仅所有者可审查)

- ✅ **Restrict pushes**
  - 添加例外: 仅添加仓库所有者账户
  - 这确保只有所有者可以直接推送到 master

- ✅ **Restrict who can push to matching branches**
  - 仅允许仓库所有者和指定的协作者

- ❌ **Require status checks to pass before merging** (不需要自动化测试)

- ✅ **Require branches to be up to date before merging**

- ✅ **Require linear history** (推荐，保持清洁的提交历史)

- ✅ **Include administrators** (管理员也需要遵守规则)

### 1.2 保护 preview 分支

**分支名称模式**: `preview`

**保护设置**:
- ✅ **Require a pull request before merging**
  - ✅ Require approvals: `1`
  - ❌ Dismiss stale PR approvals (允许快速迭代)
  
- ❌ **Restrict pushes** (允许开发者直接推送到 preview)

- ❌ **Require status checks to pass before merging**

- ✅ **Require branches to be up to date before merging**

### 1.3 保护 develop/* 分支

**分支名称模式**: `develop/*`

**保护设置**:
- ❌ **Require a pull request before merging** (允许开发者自由开发)
- ❌ **Restrict pushes**
- ❌ **Require status checks**

## 2. 仓库权限设置

### 2.1 协作者权限分配

**导航路径**: `Settings` > `Manage access`

| 用户角色 | 权限级别 | 说明 |
|---------|---------|------|
| zuoliangyu (所有者) | Admin | 完全控制权限 |
| nn (协作者) | Write | 可推送到 develop/nn, preview |
| 其他协作者 | Write | 可推送到指定的 develop 分支 |

### 2.2 分支权限矩阵

| 分支 | zuoliangyu | nn | 其他协作者 |
|-----|-----------|----|-----------| 
| master | 直接推送 + 审查 | 仅PR | 仅PR |
| preview | 推送 + 合并 | 推送 + 合并 | 推送 + 合并 |
| develop/zuolan | 推送 + 合并 | 读取 | 读取 |
| develop/nn | 推送 + 合并 | 推送 + 合并 | 读取 |
| test | 推送 + 删除 | 推送 + 删除 | 推送 + 删除 |

## 3. CODEOWNERS 文件配置

创建 `.github/CODEOWNERS` 文件：

```
# 全局代码所有者
* @zuoliangyu

# 特定目录的代码所有者（如果需要）
# /src/ @zuoliangyu
# /docs/ @zuoliangyu

# 分支特定规则
# master分支的所有变更必须由所有者审查
```

## 4. 自动化部署配置

### 4.1 GitHub Actions 工作流

创建 `.github/workflows/deploy-docs.yml`：

```yaml
name: Deploy to Docsify

on:
  push:
    branches: [ master ]
  workflow_dispatch:

jobs:
  deploy:
    runs-on: ubuntu-latest
    
    steps:
    - name: Checkout
      uses: actions/checkout@v4
      with:
        fetch-depth: 0
        
    - name: Setup Node.js
      uses: actions/setup-node@v4
      with:
        node-version: '18'
        
    - name: Build documentation
      run: |
        # 这里添加文档构建逻辑
        echo "Building documentation..."
        
    - name: Deploy to docsify branch
      uses: peaceiris/actions-gh-pages@v3
      with:
        github_token: ${{ secrets.GITHUB_TOKEN }}
        publish_dir: ./docs
        publish_branch: docsify
        user_name: 'github-actions[bot]'
        user_email: 'github-actions[bot]@users.noreply.github.com'
        commit_message: 'Deploy docs from master: ${{ github.sha }}'
```

## 5. 分支合并策略推荐

### 5.1 合并策略配置

**导航路径**: `Settings` > `General` > `Pull Requests`

**推荐设置**:
- ✅ **Allow squash merging** (推荐用于 develop → preview)
- ✅ **Allow merge commits** (推荐用于 preview → master)
- ❌ **Allow rebase merging** (避免复杂的历史)

### 5.2 合并流程标准

1. **develop → preview**:
   - 使用 Squash merge
   - 提交信息格式: `feat: [简短描述] (#PR号)`

2. **preview → master**:
   - 使用 Merge commit  
   - 保留完整的合并记录
   - 创建详细的 PR 描述

## 6. 安全性考虑

### 6.1 访问控制

1. **双因素认证**: 强制所有协作者启用 2FA
2. **SSH密钥管理**: 定期轮换 SSH 密钥
3. **Personal Access Token**: 限制权限范围，设置过期时间

### 6.2 敏感信息保护

1. **Secrets管理**: 
   - 导航至 `Settings` > `Secrets and variables` > `Actions`
   - 添加必要的环境变量和API密钥

2. **文件忽略**:
   ```gitignore
   # 敏感配置文件
   .env
   .env.local
   *.key
   *.pem
   config/secret.h
   ```

### 6.3 审计日志

- **启用路径**: `Settings` > `Security & analysis`
- ✅ **Dependency graph**
- ✅ **Dependabot alerts**  
- ✅ **Dependabot security updates**

## 7. 最佳实践

### 7.1 提交规范

```
类型(作用域): 简短描述

详细描述（可选）

类型：
- feat: 新功能
- fix: 修复bug
- docs: 文档更新
- style: 代码格式调整
- refactor: 重构
- test: 测试相关
- chore: 构建过程或辅助工具的变动
```

### 7.2 PR模板

创建 `.github/pull_request_template.md`：

```markdown
## 变更描述
简要描述本次PR的主要变更

## 变更类型
- [ ] 新功能 (feat)
- [ ] Bug修复 (fix)
- [ ] 文档更新 (docs)
- [ ] 重构 (refactor)
- [ ] 其他

## 测试清单
- [ ] 本地编译通过
- [ ] 功能测试完成
- [ ] 文档已更新

## 相关Issue
关联的Issue编号: #

## 额外说明
其他需要说明的内容
```

### 7.3 分支命名规范

```
develop/[username]     # 个人开发分支
feature/[feature-name] # 功能分支
hotfix/[issue-name]    # 紧急修复分支
preview                # 预览分支
test                   # 临时测试分支
```

## 8. 实施检查清单

### 8.1 初始配置
- [ ] 设置 master 分支保护规则
- [ ] 设置 preview 分支保护规则  
- [ ] 配置协作者权限
- [ ] 创建 CODEOWNERS 文件
- [ ] 设置自动化部署工作流

### 8.2 团队培训
- [ ] 向团队成员说明新的工作流程
- [ ] 演示PR创建和审查流程
- [ ] 确认所有成员理解分支策略

### 8.3 监控和维护
- [ ] 定期检查分支保护规则是否生效
- [ ] 监控自动化部署状态
- [ ] 定期审查协作者权限

## 故障排除

### 常见问题

1. **无法推送到master分支**
   - 检查是否通过PR提交
   - 确认是否有足够权限

2. **自动部署失败**
   - 检查GitHub Actions日志
   - 确认docsify分支权限设置

3. **PR无法合并**
   - 检查是否有必要的审查
   - 确认分支是否为最新状态

---

**配置完成后，建议先用测试分支验证整个流程，确保所有设置都按预期工作。**