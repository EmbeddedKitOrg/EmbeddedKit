#!/usr/bin/env node

const fs = require('fs-extra');
const path = require('path');

/**
 * 本地测试脚本
 * 用于在推送前测试文档生成流程
 */

async function testLocal() {
  console.log('🧪 开始本地测试文档生成流程...');
  
  try {
    // 1. 测试README收集
    console.log('\n📚 测试README文件收集...');
    const ReadmeCollector = require('./collect-readme');
    const collector = new ReadmeCollector();
    await collector.run();
    
    // 2. 测试侧边栏生成
    console.log('\n📋 测试侧边栏生成...');
    const SidebarGenerator = require('./generate-sidebar');
    const generator = new SidebarGenerator();
    await generator.run();
    
    // 3. 验证生成的文件
    console.log('\n✅ 验证生成的文件...');
    const docsDir = path.resolve('docs');
    
    const requiredFiles = [
      'README.md',
      '_sidebar.md',
      'index.html'
    ];
    
    for (const file of requiredFiles) {
      const filePath = path.join(docsDir, file);
      if (await fs.pathExists(filePath)) {
        const stats = await fs.stat(filePath);
        console.log(`✅ ${file}: ${stats.size} 字节`);
      } else {
        console.log(`❌ ${file}: 文件不存在`);
      }
    }
    
    // 4. 检查文档内容
    console.log('\n📖 检查文档内容...');
    const sidebarPath = path.join(docsDir, '_sidebar.md');
    if (await fs.pathExists(sidebarPath)) {
      const content = await fs.readFile(sidebarPath, 'utf8');
      const lines = content.split('\n').filter(line => line.trim());
      console.log(`✅ 侧边栏包含 ${lines.length} 行内容`);
    }
    
    console.log('\n🎉 本地测试完成！');
    console.log('💡 可以通过以下命令启动本地预览:');
    console.log('   npx docsify serve docs');
    
  } catch (error) {
    console.error('❌ 测试失败:', error);
    process.exit(1);
  }
}

// 执行测试
if (require.main === module) {
  testLocal();
}

module.exports = testLocal;