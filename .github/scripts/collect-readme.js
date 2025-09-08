#!/usr/bin/env node

const fs = require('fs-extra');
const path = require('path');
const { glob } = require('glob');

/**
 * README文件收集和处理脚本
 * 功能：
 * 1. 扫描项目中所有README文件
 * 2. 按目录结构组织文件
 * 3. 生成适合docsify的目录结构
 * 4. 处理中文文档编码
 */

class ReadmeCollector {
  constructor() {
    this.projectRoot = path.resolve('.');
    this.docsDir = path.join(this.projectRoot, 'docs');
    this.collectedFiles = [];
  }

  /**
   * 主执行函数
   */
  async run() {
    try {
      console.log('🔍 开始扫描项目中的README文件...');
      
      await this.scanReadmeFiles();
      await this.processReadmeFiles();
      await this.copyAdditionalDocs();
      
      console.log('✅ README文件收集完成');
      console.log(`📁 共收集 ${this.collectedFiles.length} 个文件`);
      
    } catch (error) {
      console.error('❌ README收集过程中出现错误:', error);
      process.exit(1);
    }
  }

  /**
   * 扫描所有README文件
   */
  async scanReadmeFiles() {
    const patterns = [
      'README.md',
      'README.*.md',
      '**/README.md',
      '**/README.*.md'
    ];

    // 排除的目录
    const ignorePatterns = [
      'node_modules/**',
      '.git/**',
      'build/**',
      'dist/**',
      'docs/**', // 避免重复收集docs目录下的文件
      '.github/**'
    ];

    for (const pattern of patterns) {
      const files = await glob(pattern, {
        ignore: ignorePatterns,
        cwd: this.projectRoot
      });

      for (const file of files) {
        const fullPath = path.join(this.projectRoot, file);
        const relativePath = path.relative(this.projectRoot, fullPath);
        
        if (await fs.pathExists(fullPath)) {
          this.collectedFiles.push({
            originalPath: fullPath,
            relativePath: relativePath,
            targetPath: this.generateTargetPath(relativePath),
            directory: path.dirname(relativePath),
            filename: path.basename(relativePath)
          });
        }
      }
    }

    console.log(`📄 发现 ${this.collectedFiles.length} 个README文件:`);
    this.collectedFiles.forEach(file => {
      console.log(`   ${file.relativePath} -> ${file.targetPath}`);
    });
  }

  /**
   * 生成目标文件路径
   */
  generateTargetPath(relativePath) {
    const dir = path.dirname(relativePath);
    const basename = path.basename(relativePath, '.md');
    
    // 根目录的README.md直接作为主页
    if (relativePath === 'README.md') {
      return 'README.md';
    }
    
    // 其他目录的README文件
    if (dir === '.') {
      return `${basename}.md`;
    }
    
    // 子目录的README文件，使用目录名作为文件名
    const normalizedDir = dir.replace(/[\\\/]/g, '_');
    if (basename === 'README') {
      return `modules/${normalizedDir}.md`;
    }
    
    return `modules/${normalizedDir}_${basename}.md`;
  }

  /**
   * 处理README文件内容
   */
  async processReadmeFiles() {
    console.log('🔄 开始处理README文件内容...');
    
    // 确保目标目录存在
    await fs.ensureDir(path.join(this.docsDir, 'modules'));
    await fs.ensureDir(path.join(this.docsDir, 'api'));
    await fs.ensureDir(path.join(this.docsDir, 'examples'));

    for (const file of this.collectedFiles) {
      await this.processReadmeFile(file);
    }
  }

  /**
   * 处理单个README文件
   */
  async processReadmeFile(file) {
    try {
      let content = await fs.readFile(file.originalPath, 'utf8');
      
      // 添加文件来源信息
      const sourceInfo = `<!-- 此文件由自动化脚本生成，源文件: ${file.relativePath} -->\n\n`;
      
      // 处理相对路径链接
      content = this.fixRelativeLinks(content, file.directory);
      
      // 添加模块标记（如果不是主README）
      if (file.relativePath !== 'README.md') {
        const moduleTitle = this.generateModuleTitle(file.directory, file.filename);
        content = `# ${moduleTitle}\n\n${content}`;
      }
      
      const finalContent = sourceInfo + content;
      const targetPath = path.join(this.docsDir, file.targetPath);
      
      await fs.ensureDir(path.dirname(targetPath));
      await fs.writeFile(targetPath, finalContent, 'utf8');
      
      console.log(`✅ 处理完成: ${file.relativePath}`);
      
    } catch (error) {
      console.error(`❌ 处理文件失败 ${file.relativePath}:`, error.message);
    }
  }

  /**
   * 修复相对路径链接
   */
  fixRelativeLinks(content, sourceDir) {
    // 修复Markdown图片链接
    content = content.replace(/!\[([^\]]*)\]\((?!http)([^)]+)\)/g, (match, alt, src) => {
      const absolutePath = path.posix.join('..', sourceDir, src);
      return `![${alt}](${absolutePath})`;
    });
    
    // 修复Markdown文档链接
    content = content.replace(/\[([^\]]+)\]\((?!http)([^)]+\.md)\)/g, (match, text, src) => {
      const absolutePath = path.posix.join('..', sourceDir, src);
      return `[${text}](${absolutePath})`;
    });
    
    return content;
  }

  /**
   * 生成模块标题
   */
  generateModuleTitle(directory, filename) {
    if (directory === '.') {
      return filename.replace(/README\.?/, '').replace(/\.md$/, '') || '根目录文档';
    }
    
    const dirParts = directory.split(/[\\\/]/);
    const moduleName = dirParts[dirParts.length - 1];
    
    return `${moduleName} 模块`;
  }

  /**
   * 复制其他重要文档文件
   */
  async copyAdditionalDocs() {
    console.log('📋 复制其他重要文档文件...');
    
    const additionalFiles = [
      'CONTRIBUTING.md',
      'CHANGELOG.md',
      'LICENSE',
      'BRANCH_PROTECTION_GUIDE.md'
    ];

    for (const filename of additionalFiles) {
      const sourcePath = path.join(this.projectRoot, filename);
      const targetPath = path.join(this.docsDir, filename);
      
      if (await fs.pathExists(sourcePath)) {
        try {
          let content = await fs.readFile(sourcePath, 'utf8');
          
          // 添加来源信息
          content = `<!-- 此文件由自动化脚本生成，源文件: ${filename} -->\n\n${content}`;
          
          await fs.writeFile(targetPath, content, 'utf8');
          console.log(`✅ 复制完成: ${filename}`);
          
        } catch (error) {
          console.warn(`⚠️ 复制文件失败 ${filename}:`, error.message);
        }
      }
    }
  }
}

// 执行脚本
if (require.main === module) {
  const collector = new ReadmeCollector();
  collector.run();
}

module.exports = ReadmeCollector;