# GitHub Actions 文档部署故障排除指南

## 常见问题与解决方案

### 1. 工作流执行失败

#### 问题：工作流无法启动
**症状**：推送代码后，Actions标签页没有显示工作流运行

**可能原因**：
- 工作流文件路径错误
- YAML语法错误
- 分支名称不匹配

**解决方案**：
```bash
# 检查文件路径
ls -la .github/workflows/

# 验证YAML语法
npx js-yaml .github/workflows/deploy-docs.yml

# 检查分支配置
git branch --show-current
```

#### 问题：权限不足错误
**症状**：
```
Error: Resource not accessible by integration
```

**解决方案**：
1. 确认仓库设置中已启用GitHub Pages
2. 检查工作流权限配置：
   ```yaml
   permissions:
     contents: read
     pages: write
     id-token: write
   ```

### 2. 依赖安装问题

#### 问题：npm install失败
**症状**：
```
npm ERR! code ENOTFOUND
npm ERR! syscall getaddrinfo
```

**解决方案**：
1. 检查package.json文件路径
2. 验证依赖版本兼容性：
   ```bash
   cd .github/scripts
   npm audit
   npm update
   ```

#### 问题：Node.js版本不兼容
**症状**：
```
Error: The engine "node" is incompatible with this module
```

**解决方案**：
修改`.github/workflows/deploy-docs.yml`：
```yaml
- name: 设置Node.js环境
  uses: actions/setup-node@v4
  with:
    node-version: '18'  # 确保版本兼容
```

### 3. 文档收集问题

#### 问题：README文件未被收集
**症状**：生成的文档中缺少某些README文件

**调试步骤**：
1. 检查文件扫描模式：
   ```javascript
   // 在collect-readme.js中添加调试日志
   console.log('扫描模式:', patterns);
   console.log('忽略模式:', ignorePatterns);
   ```

2. 手动测试文件匹配：
   ```bash
   find . -name "README*" -type f | grep -v node_modules
   ```

**解决方案**：
修改`.github/scripts/collect-readme.js`中的扫描模式：
```javascript
const patterns = [
  'README.md',
  'README.*.md', 
  '**/README.md',
  '**/README.*.md',
  'your-custom-pattern/**/*.md'  // 添加自定义模式
];
```

#### 问题：文档链接损坏
**症状**：生成的文档中图片或链接无法访问

**解决方案**：
1. 检查相对路径处理逻辑
2. 确保资源文件正确复制到docs目录
3. 修复链接处理函数：
   ```javascript
   fixRelativeLinks(content, sourceDir) {
     // 添加更完善的链接处理逻辑
     content = content.replace(/!\[([^\]]*)\]\((?!http)([^)]+)\)/g, (match, alt, src) => {
       // 确保路径正确性
       const absolutePath = path.posix.join('..', sourceDir, src);
       return `![${alt}](${absolutePath})`;
     });
     return content;
   }
   ```

### 4. 侧边栏生成问题

#### 问题：侧边栏结构混乱
**症状**：文档分类不正确或排序异常

**调试方法**：
1. 添加调试输出：
   ```javascript
   // 在generate-sidebar.js中添加
   console.log('文档分类结果:', this.documentTree);
   ```

2. 检查分类逻辑：
   ```javascript
   getCategoryFromPath(relativePath) {
     console.log('处理路径:', relativePath);
     const category = /* 分类逻辑 */;
     console.log('分类结果:', category);
     return category;
   }
   ```

**解决方案**：
调整分类和排序逻辑以符合项目结构。

#### 问题：中文标题显示异常
**症状**：侧边栏中文字符乱码或显示不正确

**解决方案**：
1. 确认文件编码为UTF-8
2. 检查文件读取时的编码设置：
   ```javascript
   const content = await fs.readFile(filePath, 'utf8');
   await fs.writeFile(targetPath, finalContent, 'utf8');
   ```

### 5. GitHub Pages部署问题

#### 问题：页面404错误
**症状**：文档部署成功但访问显示404

**可能原因**：
- 缺少.nojekyll文件
- 首页文件路径错误
- docsify配置问题

**解决方案**：
1. 确认.nojekyll文件存在：
   ```yaml
   # 在工作流中添加
   touch docs/.nojekyll
   ```

2. 检查docsify配置：
   ```javascript
   window.$docsify = {
     basePath: '../',
     homepage: 'README.md'  // 确保首页路径正确
   };
   ```

#### 问题：自定义域名不工作
**症状**：CNAME配置后域名无法访问

**解决方案**：
1. 检查CNAME文件格式：
   ```
   # 文件内容应该只有域名
   your-domain.com
   ```

2. 验证DNS配置：
   ```bash
   nslookup your-domain.com
   dig your-domain.com CNAME
   ```

### 6. 性能和资源问题

#### 问题：构建时间过长
**症状**：工作流执行超过时间限制

**优化方案**：
1. 启用并行处理：
   ```javascript
   // 使用Promise.all并行处理
   await Promise.all(files.map(file => this.processFile(file)));
   ```

2. 减少不必要的文件处理
3. 优化文件IO操作

#### 问题：内存使用过高
**症状**：工作流因内存不足失败

**解决方案**：
1. 分批处理大量文件
2. 及时释放不需要的变量
3. 使用流式处理大文件

### 7. 调试技巧

#### 启用详细日志
在脚本中添加详细的调试信息：
```javascript
console.log('🔍 调试信息:', {
  projectRoot: this.projectRoot,
  docsDir: this.docsDir,
  collectedFiles: this.collectedFiles.length
});
```

#### 本地测试脚本
在推送前本地测试脚本：
```bash
cd .github/scripts
npm install
node collect-readme.js
node generate-sidebar.js
```

#### 分步执行工作流
可以注释掉部分步骤来定位问题：
```yaml
# - name: 生成文档索引
#   run: node .github/scripts/generate-sidebar.js
```

### 8. 回滚和恢复

#### 快速回滚
如果新部署出现问题，可以快速回滚：
1. 在Actions页面重新运行之前的成功版本
2. 或者临时禁用自动部署，手动修复问题

#### 备份重要配置
定期备份关键配置文件：
- `.docsify/index.html`
- `docs/_sidebar.md`（如果手动维护）
- 自定义的文档文件

### 9. 预防措施

#### 代码质量检查
添加基本的质量检查：
```javascript
// 验证生成的文件
if (!fs.existsSync(this.sidebarPath)) {
  throw new Error('侧边栏文件生成失败');
}
```

#### 监控和告警
设置GitHub通知以在工作流失败时及时收到警报。

#### 定期维护
- 每月检查依赖更新
- 定期测试文档链接有效性
- 监控Pages服务状态

## 联系支持

如果遇到无法解决的问题：
1. 查看GitHub Pages状态页
2. 检查GitHub Actions服务状态
3. 在仓库Issues中寻找类似问题
4. 提交新的Issue并提供详细的错误日志