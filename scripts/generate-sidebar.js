#!/usr/bin/env node

/**
 * 自动生成侧边栏脚本
 * 扫描项目结构并生成动态侧边栏配置
 */

const fs = require('fs');
const path = require('path');
const glob = require('glob');

// 配置
const config = {
  docsRoot: path.join(__dirname, '..', 'docs'),
  sidebarFile: path.join(__dirname, '..', 'docs', '_sidebar.md'),
  excludeDirs: ['_media', 'assets', '.git', 'node_modules'],
  excludeFiles: ['_sidebar.md', '_navbar.md', '_coverpage.md', '404.md'],
  moduleMetadata: path.join(__dirname, '..', 'module-metadata.json'),
  languages: ['zh', 'en'] // 支持的语言
};

// 模块信息映射
const moduleInfo = {
  'scheduler': { name: '任务调度器', status: 'stable', icon: '⚙️' },
  'memory': { name: '内存管理', status: 'stable', icon: '💾' },
  'data_structures': { name: '数据结构', status: 'stable', icon: '📊' },
  'utils': { name: '工具函数', status: 'beta', icon: '🔧' },
  'hal': { name: '硬件抽象层', status: 'experimental', icon: '🔌' },
  'network': { name: '网络协议栈', status: 'experimental', icon: '🌐' },
  'filesystem': { name: '文件系统', status: 'beta', icon: '📁' },
  'crypto': { name: '加密算法', status: 'beta', icon: '🔐' }
};

// 状态标签
const statusBadges = {
  'stable': '<span class="module-badge badge-stable">稳定</span>',
  'beta': '<span class="module-badge badge-beta">测试</span>',
  'experimental': '<span class="module-badge badge-experimental">实验</span>',
  'deprecated': '<span class="module-badge badge-deprecated">废弃</span>'
};

/**
 * 扫描目录结构
 */
function scanDirectory(dir, baseDir = '') {
  const items = [];
  const files = fs.readdirSync(dir);
  
  files.forEach(file => {
    const filePath = path.join(dir, file);
    const relativePath = path.relative(config.docsRoot, filePath).replace(/\\/g, '/');
    const stat = fs.statSync(filePath);
    
    // 跳过排除的文件和目录
    if (config.excludeDirs.includes(file) || config.excludeFiles.includes(file)) {
      return;
    }
    
    if (stat.isDirectory()) {
      const subItems = scanDirectory(filePath, relativePath);
      if (subItems.length > 0) {
        items.push({
          type: 'directory',
          name: file,
          path: relativePath,
          children: subItems
        });
      }
    } else if (file.endsWith('.md')) {
      // 读取文件头部获取标题
      const content = fs.readFileSync(filePath, 'utf8');
      const titleMatch = content.match(/^#\s+(.+)$/m);
      const title = titleMatch ? titleMatch[1] : file.replace('.md', '');
      
      items.push({
        type: 'file',
        name: file,
        title: title,
        path: relativePath
      });
    }
  });
  
  return items;
}

/**
 * 生成侧边栏Markdown
 */
function generateSidebar(structure) {
  const lines = ['<!-- docs/_sidebar.md -->'];
  lines.push('<!-- 自动生成的侧边栏，请勿手动编辑 -->');
  lines.push('<!-- 生成时间: ' + new Date().toISOString() + ' -->');
  lines.push('');
  
  // 添加主页链接
  lines.push('* [🏠 **首页**](/)');
  lines.push('');
  
  // 快速开始部分
  lines.push('* **快速开始**');
  lines.push('  * [📖 简介](README.md)');
  lines.push('  * [🚀 安装指南](SETUP_GUIDE.md)');
  lines.push('  * [💻 开发工作流](DEVELOPMENT_WORKFLOW.md)');
  lines.push('  * [🤝 贡献指南](CONTRIBUTING.md)');
  lines.push('');
  
  // 扫描modules目录
  const modulesPath = path.join(config.docsRoot, 'modules');
  if (fs.existsSync(modulesPath)) {
    lines.push('* **核心模块**');
    
    const moduleFiles = fs.readdirSync(modulesPath);
    const modules = {};
    
    // 组织模块文件
    moduleFiles.forEach(file => {
      if (file.endsWith('.md') && file !== 'README.md' && file !== 'README_TEMPLATE.md') {
        const moduleName = file.replace('.md', '');
        const content = fs.readFileSync(path.join(modulesPath, file), 'utf8');
        const titleMatch = content.match(/^#\s+(.+)$/m);
        const title = titleMatch ? titleMatch[1] : moduleName;
        
        // 检查是否有对应的模块信息
        const info = moduleInfo[moduleName] || { 
          name: title, 
          status: 'experimental',
          icon: '📦'
        };
        
        // 提取模块的子章节
        const sections = [];
        const sectionMatches = content.matchAll(/^##\s+(.+)$/gm);
        for (const match of sectionMatches) {
          sections.push(match[1]);
        }
        
        modules[moduleName] = {
          file: file,
          title: info.name,
          icon: info.icon,
          status: info.status,
          sections: sections
        };
      }
    });
    
    // 按状态排序模块（stable > beta > experimental）
    const sortedModules = Object.entries(modules).sort((a, b) => {
      const statusOrder = { 'stable': 0, 'beta': 1, 'experimental': 2, 'deprecated': 3 };
      return (statusOrder[a[1].status] || 3) - (statusOrder[b[1].status] || 3);
    });
    
    // 生成模块列表
    sortedModules.forEach(([key, module]) => {
      const badge = statusBadges[module.status] || '';
      lines.push(`  * ${module.icon} [${module.title}](modules/${module.file}) ${badge}`);
      
      // 如果有子章节，可以展开显示
      if (module.sections.length > 0 && module.status === 'stable') {
        module.sections.slice(0, 3).forEach(section => {
          lines.push(`    * [${section}](modules/${module.file}#${encodeURIComponent(section.toLowerCase().replace(/\s+/g, '-'))})`);
        });
      }
    });
    lines.push('');
  }
  
  // API 文档部分
  const apiPath = path.join(config.docsRoot, 'api');
  if (fs.existsSync(apiPath)) {
    lines.push('* **API 参考**');
    lines.push('  * [📚 API 总览](api/README.md)');
    
    const apiFiles = fs.readdirSync(apiPath);
    apiFiles.forEach(file => {
      if (file.endsWith('.md') && file !== 'README.md') {
        const name = file.replace('.md', '');
        const formattedName = name.split('_').map(word => 
          word.charAt(0).toUpperCase() + word.slice(1)
        ).join(' ');
        lines.push(`  * [${formattedName} API](api/${file})`);
      }
    });
    lines.push('');
  }
  
  // 示例代码部分
  const examplesPath = path.join(config.docsRoot, 'examples');
  if (fs.existsSync(examplesPath)) {
    lines.push('* **示例代码**');
    lines.push('  * [💡 示例总览](examples/README.md)');
    
    // 扫描示例子目录
    const exampleDirs = fs.readdirSync(examplesPath).filter(item => {
      const itemPath = path.join(examplesPath, item);
      return fs.statSync(itemPath).isDirectory();
    });
    
    exampleDirs.forEach(dir => {
      const dirPath = path.join(examplesPath, dir);
      const readmePath = path.join(dirPath, 'README.md');
      
      if (fs.existsSync(readmePath)) {
        const content = fs.readFileSync(readmePath, 'utf8');
        const titleMatch = content.match(/^#\s+(.+)$/m);
        const title = titleMatch ? titleMatch[1] : dir;
        lines.push(`  * [${title}](examples/${dir}/README.md)`);
      }
    });
    lines.push('');
  }
  
  // 设计文档部分
  lines.push('* **架构设计**');
  lines.push('  * [🏗️ 系统架构](design/architecture.md)');
  lines.push('  * [📐 设计原则](design/principles.md)');
  lines.push('  * [⚡ 性能优化](design/optimization.md)');
  lines.push('  * [🗺️ 内存布局](design/memory_layout.md)');
  lines.push('');
  
  // 最佳实践部分
  lines.push('* **最佳实践**');
  lines.push('  * [📝 编码规范](best_practices/coding_standards.md)');
  lines.push('  * [🛡️ 内存安全](best_practices/memory_safety.md)');
  lines.push('  * [⏱️ 实时性保证](best_practices/real_time.md)');
  lines.push('  * [🐛 调试技巧](best_practices/debugging.md)');
  lines.push('');
  
  // 移植指南
  lines.push('* **移植指南**');
  lines.push('  * [🔄 移植概述](porting/overview.md)');
  lines.push('  * [🔌 硬件抽象层](porting/hal.md)');
  lines.push('  * [📱 平台适配](porting/platforms.md)');
  lines.push('  * [🔨 编译器支持](porting/compilers.md)');
  lines.push('');
  
  // 参考资料
  lines.push('* **参考资料**');
  lines.push('  * [❓ 常见问题](faq.md)');
  lines.push('  * [📖 术语表](glossary.md)');
  lines.push('  * [📝 更新日志](CHANGELOG.md)');
  lines.push('  * [🗺️ 路线图](roadmap.md)');
  lines.push('  * [📜 许可证](LICENSE.md)');
  lines.push('');
  
  // 社区资源
  lines.push('* **社区**');
  lines.push('  * [💬 讨论区](https://github.com/zuoliangyu/EmbedKit/discussions)');
  lines.push('  * [🐛 问题跟踪](https://github.com/zuoliangyu/EmbedKit/issues)');
  lines.push('  * [🔀 Pull Requests](https://github.com/zuoliangyu/EmbedKit/pulls)');
  lines.push('  * [⭐ Star History](https://star-history.com/#zuoliangyu/EmbedKit)');
  
  return lines.join('\n');
}

/**
 * 生成模块元数据
 */
function generateModuleMetadata() {
  const metadata = {
    generated: new Date().toISOString(),
    modules: {},
    statistics: {
      totalModules: 0,
      stableModules: 0,
      betaModules: 0,
      experimentalModules: 0
    }
  };
  
  // 扫描src目录获取实际模块信息
  const srcPath = path.join(__dirname, '..', 'src');
  if (fs.existsSync(srcPath)) {
    const srcDirs = fs.readdirSync(srcPath).filter(item => {
      const itemPath = path.join(srcPath, item);
      return fs.statSync(itemPath).isDirectory();
    });
    
    srcDirs.forEach(dir => {
      const info = moduleInfo[dir] || {
        name: dir,
        status: 'experimental',
        icon: '📦'
      };
      
      // 统计文件
      const dirPath = path.join(srcPath, dir);
      const files = glob.sync(path.join(dirPath, '**/*.{c,h}'));
      
      metadata.modules[dir] = {
        name: info.name,
        status: info.status,
        icon: info.icon,
        path: `src/${dir}`,
        files: files.length,
        lastModified: new Date().toISOString()
      };
      
      // 更新统计
      metadata.statistics.totalModules++;
      metadata.statistics[`${info.status}Modules`]++;
    });
  }
  
  return metadata;
}

/**
 * 主函数
 */
function main() {
  console.log('🔄 开始生成侧边栏...');
  
  try {
    // 扫描文档结构
    const structure = scanDirectory(config.docsRoot);
    
    // 生成侧边栏内容
    const sidebarContent = generateSidebar(structure);
    
    // 写入侧边栏文件
    fs.writeFileSync(config.sidebarFile, sidebarContent, 'utf8');
    console.log('✅ 侧边栏生成成功:', config.sidebarFile);
    
    // 生成模块元数据
    const metadata = generateModuleMetadata();
    fs.writeFileSync(config.moduleMetadata, JSON.stringify(metadata, null, 2), 'utf8');
    console.log('✅ 模块元数据生成成功:', config.moduleMetadata);
    
    // 显示统计信息
    console.log('\n📊 统计信息:');
    console.log(`  - 总模块数: ${metadata.statistics.totalModules}`);
    console.log(`  - 稳定模块: ${metadata.statistics.stableModules}`);
    console.log(`  - 测试模块: ${metadata.statistics.betaModules}`);
    console.log(`  - 实验模块: ${metadata.statistics.experimentalModules}`);
    
  } catch (error) {
    console.error('❌ 生成失败:', error.message);
    process.exit(1);
  }
}

// 如果直接运行脚本
if (require.main === module) {
  main();
}

module.exports = { generateSidebar, generateModuleMetadata };