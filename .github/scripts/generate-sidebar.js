#!/usr/bin/env node

const fs = require('fs-extra');
const path = require('path');
const { glob } = require('glob');

/**
 * Docsify侧边栏生成脚本
 * 功能：
 * 1. 扫描docs目录中的所有文档
 * 2. 按目录结构生成层次化的侧边栏
 * 3. 生成_sidebar.md文件
 * 4. 支持中文文档
 */

class SidebarGenerator {
  constructor() {
    this.docsDir = path.resolve('docs');
    this.sidebarPath = path.join(this.docsDir, '_sidebar.md');
    this.documentTree = new Map();
  }

  /**
   * 主执行函数
   */
  async run() {
    try {
      console.log('📝 开始生成docsify侧边栏...');
      
      await this.scanDocuments();
      await this.generateSidebar();
      
      console.log('✅ 侧边栏生成完成');
      
    } catch (error) {
      console.error('❌ 侧边栏生成过程中出现错误:', error);
      process.exit(1);
    }
  }

  /**
   * 扫描文档目录
   */
  async scanDocuments() {
    console.log('🔍 扫描文档目录...');
    
    const patterns = [
      '*.md',
      '**/*.md'
    ];
    
    const ignorePatterns = [
      '_sidebar.md',
      '_navbar.md',
      '_coverpage.md'
    ];
    
    for (const pattern of patterns) {
      const files = await glob(pattern, {
        cwd: this.docsDir,
        ignore: ignorePatterns
      });
      
      for (const file of files) {
        await this.processDocument(file);
      }
    }
    
    console.log(`📄 扫描到 ${this.documentTree.size} 个文档分类`);
  }

  /**
   * 处理单个文档
   */
  async processDocument(relativePath) {
    const fullPath = path.join(this.docsDir, relativePath);
    
    if (!(await fs.pathExists(fullPath))) {
      return;
    }
    
    // 读取文件的第一行标题
    const title = await this.extractTitle(fullPath, relativePath);
    const category = this.getCategoryFromPath(relativePath);
    
    if (!this.documentTree.has(category)) {
      this.documentTree.set(category, []);
    }
    
    this.documentTree.get(category).push({
      path: relativePath,
      title: title,
      order: this.getDocumentOrder(relativePath)
    });
  }

  /**
   * 从文件中提取标题
   */
  async extractTitle(filePath, relativePath) {
    try {
      const content = await fs.readFile(filePath, 'utf8');
      
      // 匹配第一个# 标题
      const titleMatch = content.match(/^#\s+(.+)$/m);
      if (titleMatch) {
        return titleMatch[1].trim();
      }
      
      // 如果没有找到标题，使用文件名
      const basename = path.basename(relativePath, '.md');
      return this.formatFilename(basename);
      
    } catch (error) {
      console.warn(`⚠️ 无法读取文件标题 ${relativePath}:`, error.message);
      return this.formatFilename(path.basename(relativePath, '.md'));
    }
  }

  /**
   * 从文件路径获取分类
   */
  getCategoryFromPath(relativePath) {
    const dir = path.dirname(relativePath);
    
    if (dir === '.' || relativePath === 'README.md') {
      return 'main';
    }
    
    if (dir.startsWith('api')) {
      return 'api';
    }
    
    if (dir.startsWith('examples')) {
      return 'examples';
    }
    
    if (dir.startsWith('modules')) {
      return 'modules';
    }
    
    return 'others';
  }

  /**
   * 获取文档排序权重
   */
  getDocumentOrder(relativePath) {
    const filename = path.basename(relativePath);
    
    // 主要文档优先
    if (filename === 'README.md') return 0;
    if (filename === 'SETUP_GUIDE.md') return 1;
    if (filename === 'DEVELOPMENT_WORKFLOW.md') return 2;
    if (filename === 'CODE_REVIEW_CHECKLIST.md') return 3;
    if (filename === 'CONTRIBUTING.md') return 4;
    if (filename === 'BRANCH_PROTECTION_GUIDE.md') return 5;
    
    // API文档
    if (relativePath.startsWith('api/')) return 100;
    
    // 示例文档
    if (relativePath.startsWith('examples/')) return 200;
    
    // 模块文档
    if (relativePath.startsWith('modules/')) return 300;
    
    // 其他文档
    return 999;
  }

  /**
   * 格式化文件名为可读标题
   */
  formatFilename(filename) {
    return filename
      .replace(/[-_]/g, ' ')
      .replace(/\b\w/g, l => l.toUpperCase())
      .replace(/README/gi, '说明文档')
      .replace(/API/gi, 'API接口')
      .replace(/SETUP/gi, '安装配置')
      .replace(/DEVELOPMENT/gi, '开发')
      .replace(/WORKFLOW/gi, '流程')
      .replace(/CODE REVIEW/gi, '代码审查')
      .replace(/CHECKLIST/gi, '清单')
      .replace(/CONTRIBUTING/gi, '贡献指南')
      .replace(/BRANCH PROTECTION/gi, '分支保护')
      .replace(/GUIDE/gi, '指南');
  }

  /**
   * 生成侧边栏内容
   */
  async generateSidebar() {
    console.log('📋 生成侧边栏内容...');
    
    const sidebarContent = [];
    
    // 添加头部注释
    sidebarContent.push('<!-- 此文件由自动化脚本生成，请勿手动修改 -->');
    sidebarContent.push('<!-- 要修改侧边栏，请编辑 .github/scripts/generate-sidebar.js -->');
    sidebarContent.push('');
    
    // 主要文档
    if (this.documentTree.has('main')) {
      const docs = this.documentTree.get('main')
        .sort((a, b) => a.order - b.order);
      
      for (const doc of docs) {
        sidebarContent.push(`- [${doc.title}](${doc.path})`);
      }
      sidebarContent.push('');
    }
    
    // 开发指南
    const devDocs = [];
    for (const [category, docs] of this.documentTree) {
      if (category === 'main') continue;
      
      docs.forEach(doc => {
        if (doc.path.includes('SETUP') || doc.path.includes('DEVELOPMENT') || 
            doc.path.includes('CODE_REVIEW') || doc.path.includes('CONTRIBUTING') ||
            doc.path.includes('BRANCH_PROTECTION')) {
          devDocs.push(doc);
        }
      });
    }
    
    if (devDocs.length > 0) {
      sidebarContent.push('- **开发指南**');
      devDocs
        .sort((a, b) => a.order - b.order)
        .forEach(doc => {
          sidebarContent.push(`  - [${doc.title}](${doc.path})`);
        });
      sidebarContent.push('');
    }
    
    // API文档
    if (this.documentTree.has('api')) {
      sidebarContent.push('- **API文档**');
      const docs = this.documentTree.get('api')
        .sort((a, b) => a.title.localeCompare(b.title, 'zh-CN'));
        
      for (const doc of docs) {
        sidebarContent.push(`  - [${doc.title}](${doc.path})`);
      }
      sidebarContent.push('');
    }
    
    // 模块文档
    if (this.documentTree.has('modules')) {
      sidebarContent.push('- **模块文档**');
      const docs = this.documentTree.get('modules')
        .sort((a, b) => a.title.localeCompare(b.title, 'zh-CN'));
        
      for (const doc of docs) {
        sidebarContent.push(`  - [${doc.title}](${doc.path})`);
      }
      sidebarContent.push('');
    }
    
    // 示例文档
    if (this.documentTree.has('examples')) {
      sidebarContent.push('- **示例代码**');
      const docs = this.documentTree.get('examples')
        .sort((a, b) => a.title.localeCompare(b.title, 'zh-CN'));
        
      for (const doc of docs) {
        sidebarContent.push(`  - [${doc.title}](${doc.path})`);
      }
      sidebarContent.push('');
    }
    
    // 其他文档
    if (this.documentTree.has('others')) {
      sidebarContent.push('- **其他文档**');
      const docs = this.documentTree.get('others')
        .sort((a, b) => a.title.localeCompare(b.title, 'zh-CN'));
        
      for (const doc of docs) {
        sidebarContent.push(`  - [${doc.title}](${doc.path})`);
      }
      sidebarContent.push('');
    }
    
    // 外部链接
    sidebarContent.push('- **相关链接**');
    sidebarContent.push('  - [GitHub仓库](https://github.com/zuoliangyu/EmbedKit)');
    sidebarContent.push('  - [问题反馈](https://github.com/zuoliangyu/EmbedKit/issues)');
    sidebarContent.push('  - [参与贡献](https://github.com/zuoliangyu/EmbedKit/pulls)');
    
    // 写入文件
    const finalContent = sidebarContent.join('\n');
    await fs.writeFile(this.sidebarPath, finalContent, 'utf8');
    
    console.log(`✅ 侧边栏文件已生成: ${this.sidebarPath}`);
    console.log(`📊 包含 ${this.getTotalDocCount()} 个文档链接`);
  }

  /**
   * 获取文档总数
   */
  getTotalDocCount() {
    let total = 0;
    for (const docs of this.documentTree.values()) {
      total += docs.length;
    }
    return total;
  }
}

// 执行脚本
if (require.main === module) {
  const generator = new SidebarGenerator();
  generator.run();
}

module.exports = SidebarGenerator;