#!/usr/bin/env python3
"""
文档索引更新脚本
自动扫描文档结构并生成索引文件
"""

import os
import json
import re
from datetime import datetime
from pathlib import Path
import hashlib

class DocsIndexer:
    def __init__(self, docs_root='docs'):
        self.docs_root = Path(docs_root)
        self.index = {
            'version': '1.0.0',
            'generated': datetime.now().isoformat(),
            'documents': {},
            'modules': {},
            'api': {},
            'examples': {},
            'tags': {},
            'search_index': []
        }
        
    def scan_markdown_files(self):
        """扫描所有Markdown文件"""
        for md_file in self.docs_root.rglob('*.md'):
            if md_file.name.startswith('_'):
                continue
                
            relative_path = md_file.relative_to(self.docs_root)
            self.process_file(md_file, relative_path)
    
    def process_file(self, file_path, relative_path):
        """处理单个Markdown文件"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
                
            # 提取元数据
            metadata = self.extract_metadata(content)
            
            # 提取标题
            title = self.extract_title(content)
            
            # 提取标签
            tags = self.extract_tags(content)
            
            # 提取摘要
            summary = self.extract_summary(content)
            
            # 计算内容哈希
            content_hash = hashlib.md5(content.encode()).hexdigest()
            
            # 获取文件统计信息
            stats = file_path.stat()
            
            # 构建文档信息
            doc_info = {
                'path': str(relative_path).replace('\\', '/'),
                'title': title,
                'summary': summary,
                'tags': tags,
                'metadata': metadata,
                'size': stats.st_size,
                'modified': datetime.fromtimestamp(stats.st_mtime).isoformat(),
                'hash': content_hash,
                'word_count': len(content.split()),
                'reading_time': self.calculate_reading_time(content)
            }
            
            # 分类存储
            self.categorize_document(doc_info, relative_path)
            
            # 添加到搜索索引
            self.add_to_search_index(doc_info, content)
            
        except Exception as e:
            print(f"处理文件 {file_path} 时出错: {e}")
    
    def extract_metadata(self, content):
        """提取YAML前置元数据"""
        metadata = {}
        yaml_match = re.match(r'^---\n(.*?)\n---', content, re.DOTALL)
        
        if yaml_match:
            yaml_content = yaml_match.group(1)
            for line in yaml_content.split('\n'):
                if ':' in line:
                    key, value = line.split(':', 1)
                    metadata[key.strip()] = value.strip()
        
        return metadata
    
    def extract_title(self, content):
        """提取文档标题"""
        # 优先从元数据中获取
        metadata = self.extract_metadata(content)
        if 'title' in metadata:
            return metadata['title']
        
        # 否则从第一个H1标题获取
        h1_match = re.search(r'^#\s+(.+)$', content, re.MULTILINE)
        if h1_match:
            return h1_match.group(1).strip()
        
        return "未命名文档"
    
    def extract_tags(self, content):
        """提取标签"""
        tags = []
        
        # 从元数据中提取
        metadata = self.extract_metadata(content)
        if 'tags' in metadata:
            tag_str = metadata['tags']
            tags.extend([t.strip() for t in tag_str.split(',')])
        
        # 从内容中提取标记的标签
        tag_matches = re.findall(r'\[标签:(.+?)\]', content)
        tags.extend(tag_matches)
        
        # 自动检测模块标签
        module_matches = re.findall(r'\[模块:(stable|beta|experimental|deprecated)\]', content)
        tags.extend(module_matches)
        
        return list(set(tags))  # 去重
    
    def extract_summary(self, content, max_length=200):
        """提取文档摘要"""
        # 移除元数据
        content = re.sub(r'^---\n.*?\n---\n', '', content, flags=re.DOTALL)
        
        # 移除标题
        content = re.sub(r'^#+\s+.+$', '', content, flags=re.MULTILINE)
        
        # 移除代码块
        content = re.sub(r'```[\s\S]*?```', '', content)
        content = re.sub(r'`[^`]+`', '', content)
        
        # 移除链接和图片
        content = re.sub(r'!\[.*?\]\(.*?\)', '', content)
        content = re.sub(r'\[.*?\]\(.*?\)', '', content)
        
        # 移除特殊标记
        content = re.sub(r'[*_~>#\-+]', '', content)
        
        # 清理空白
        content = ' '.join(content.split())
        
        # 截取摘要
        if len(content) > max_length:
            content = content[:max_length] + '...'
        
        return content
    
    def calculate_reading_time(self, content):
        """计算阅读时间（分钟）"""
        # 中文约300字/分钟，英文约200词/分钟
        chinese_chars = len(re.findall(r'[\u4e00-\u9fa5]', content))
        english_words = len(re.findall(r'\b[a-zA-Z]+\b', content))
        
        reading_time = (chinese_chars / 300) + (english_words / 200)
        return max(1, round(reading_time))
    
    def categorize_document(self, doc_info, relative_path):
        """对文档进行分类"""
        path_parts = relative_path.parts
        
        # 主索引
        self.index['documents'][str(relative_path)] = doc_info
        
        # 模块文档
        if len(path_parts) > 1 and path_parts[0] == 'modules':
            module_name = path_parts[1].replace('.md', '')
            if module_name not in self.index['modules']:
                self.index['modules'][module_name] = []
            self.index['modules'][module_name].append(doc_info)
        
        # API文档
        elif len(path_parts) > 1 and path_parts[0] == 'api':
            api_name = path_parts[1].replace('.md', '')
            self.index['api'][api_name] = doc_info
        
        # 示例文档
        elif len(path_parts) > 1 and path_parts[0] == 'examples':
            if 'examples' not in self.index:
                self.index['examples'] = []
            self.index['examples'].append(doc_info)
        
        # 标签索引
        for tag in doc_info['tags']:
            if tag not in self.index['tags']:
                self.index['tags'][tag] = []
            self.index['tags'][tag].append({
                'path': doc_info['path'],
                'title': doc_info['title']
            })
    
    def add_to_search_index(self, doc_info, content):
        """添加到搜索索引"""
        # 提取所有标题
        headers = re.findall(r'^#+\s+(.+)$', content, re.MULTILINE)
        
        # 构建搜索条目
        search_entry = {
            'path': doc_info['path'],
            'title': doc_info['title'],
            'headers': headers,
            'tags': doc_info['tags'],
            'summary': doc_info['summary']
        }
        
        self.index['search_index'].append(search_entry)
    
    def generate_statistics(self):
        """生成统计信息"""
        self.index['statistics'] = {
            'total_documents': len(self.index['documents']),
            'total_modules': len(self.index['modules']),
            'total_apis': len(self.index['api']),
            'total_examples': len(self.index.get('examples', [])),
            'total_tags': len(self.index['tags']),
            'total_size': sum(doc['size'] for doc in self.index['documents'].values()),
            'average_reading_time': sum(doc['reading_time'] for doc in self.index['documents'].values()) // max(1, len(self.index['documents']))
        }
    
    def save_index(self, output_file='docs/docs-index.json'):
        """保存索引文件"""
        self.generate_statistics()
        
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(self.index, f, ensure_ascii=False, indent=2)
        
        print(f"✅ 文档索引已保存到: {output_file}")
        print(f"📊 统计信息:")
        for key, value in self.index['statistics'].items():
            print(f"  - {key}: {value}")
    
    def generate_sitemap(self, base_url='https://zuoliangyu.github.io/EmbedKit'):
        """生成站点地图"""
        sitemap_content = ['# 站点地图\n']
        
        # 按分类组织
        sitemap_content.append('\n## 核心文档\n')
        for path, doc in self.index['documents'].items():
            if not path.startswith(('modules/', 'api/', 'examples/')):
                sitemap_content.append(f"- [{doc['title']}](/{path})")
        
        if self.index['modules']:
            sitemap_content.append('\n## 模块文档\n')
            for module, docs in self.index['modules'].items():
                sitemap_content.append(f"\n### {module}\n")
                for doc in docs:
                    sitemap_content.append(f"- [{doc['title']}](/{doc['path']})")
        
        if self.index['api']:
            sitemap_content.append('\n## API 文档\n')
            for api, doc in self.index['api'].items():
                sitemap_content.append(f"- [{doc['title']}](/{doc['path']})")
        
        if self.index.get('examples'):
            sitemap_content.append('\n## 示例代码\n')
            for doc in self.index['examples']:
                sitemap_content.append(f"- [{doc['title']}](/{doc['path']})")
        
        # 标签云
        if self.index['tags']:
            sitemap_content.append('\n## 标签索引\n')
            for tag, docs in sorted(self.index['tags'].items()):
                sitemap_content.append(f"\n### {tag} ({len(docs)})\n")
                for doc in docs[:5]:  # 只显示前5个
                    sitemap_content.append(f"- [{doc['title']}](/{doc['path']})")
                if len(docs) > 5:
                    sitemap_content.append(f"- ...还有 {len(docs) - 5} 个文档")
        
        # 保存站点地图
        with open(self.docs_root / 'sitemap.md', 'w', encoding='utf-8') as f:
            f.write('\n'.join(sitemap_content))
        
        print(f"✅ 站点地图已生成: {self.docs_root / 'sitemap.md'}")
        
        # 生成XML sitemap（用于搜索引擎）
        self.generate_xml_sitemap(base_url)
    
    def generate_xml_sitemap(self, base_url):
        """生成XML格式的站点地图"""
        xml_content = ['<?xml version="1.0" encoding="UTF-8"?>']
        xml_content.append('<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">')
        
        for path, doc in self.index['documents'].items():
            url = f"{base_url}/#{path.replace('.md', '')}"
            xml_content.append('  <url>')
            xml_content.append(f'    <loc>{url}</loc>')
            xml_content.append(f'    <lastmod>{doc["modified"][:10]}</lastmod>')
            xml_content.append('    <changefreq>weekly</changefreq>')
            xml_content.append('    <priority>0.8</priority>')
            xml_content.append('  </url>')
        
        xml_content.append('</urlset>')
        
        with open(self.docs_root / 'sitemap.xml', 'w', encoding='utf-8') as f:
            f.write('\n'.join(xml_content))
        
        print(f"✅ XML站点地图已生成: {self.docs_root / 'sitemap.xml'}")
    
    def run(self):
        """运行索引器"""
        print("🔄 开始扫描文档...")
        self.scan_markdown_files()
        self.save_index()
        self.generate_sitemap()
        print("✨ 文档索引更新完成！")

if __name__ == '__main__':
    indexer = DocsIndexer()
    indexer.run()