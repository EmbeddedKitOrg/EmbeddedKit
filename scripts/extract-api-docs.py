#!/usr/bin/env python3
"""
API文档提取脚本
从C语言头文件中提取API文档并生成Markdown格式
"""

import os
import re
import json
from pathlib import Path
from typing import List, Dict, Any

class APIDocExtractor:
    def __init__(self, src_dir='src', include_dir='include', output_dir='docs/api'):
        self.src_dir = Path(src_dir)
        self.include_dir = Path(include_dir)
        self.output_dir = Path(output_dir)
        self.apis = {}
        
        # 确保输出目录存在
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
    def extract_from_file(self, file_path: Path) -> List[Dict[str, Any]]:
        """从单个文件提取API文档"""
        apis = []
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # 提取所有的文档注释块和对应的函数声明
            pattern = r'/\*\*(.*?)\*/\s*([^;{]+(?:;|\{))'
            matches = re.findall(pattern, content, re.DOTALL)
            
            for comment, declaration in matches:
                api_info = self.parse_api_block(comment, declaration)
                if api_info:
                    api_info['file'] = str(file_path)
                    apis.append(api_info)
                    
        except Exception as e:
            print(f"处理文件 {file_path} 时出错: {e}")
            
        return apis
    
    def parse_api_block(self, comment: str, declaration: str) -> Dict[str, Any]:
        """解析API文档块"""
        api_info = {
            'name': '',
            'brief': '',
            'description': '',
            'params': [],
            'return': '',
            'signature': '',
            'example': '',
            'notes': [],
            'since': '',
            'deprecated': False,
            'tags': []
        }
        
        # 清理声明
        declaration = declaration.strip()
        if declaration.endswith('{'):
            declaration = declaration[:-1].strip()
        if declaration.endswith(';'):
            declaration = declaration[:-1].strip()
        
        # 提取函数名
        func_match = re.search(r'(\w+)\s*\(', declaration)
        if func_match:
            api_info['name'] = func_match.group(1)
            api_info['signature'] = declaration
        else:
            return None
        
        # 解析注释内容
        lines = comment.strip().split('\n')
        current_section = 'description'
        current_content = []
        
        for line in lines:
            line = line.strip()
            # 移除注释标记
            line = re.sub(r'^\*\s*', '', line)
            
            # 检查是否是特殊标记
            if line.startswith('@'):
                # 保存之前的内容
                if current_section == 'description' and current_content:
                    if not api_info['brief']:
                        api_info['brief'] = current_content[0]
                    api_info['description'] = '\n'.join(current_content)
                    current_content = []
                
                # 解析特殊标记
                if line.startswith('@brief'):
                    api_info['brief'] = line[6:].strip()
                elif line.startswith('@param'):
                    param_match = re.match(r'@param\s+(\[?\w+\]?)\s+(.*)', line)
                    if param_match:
                        param_name = param_match.group(1)
                        param_desc = param_match.group(2)
                        # 检查是否是输入/输出参数
                        if param_name.startswith('[') and param_name.endswith(']'):
                            param_type = 'inout'
                            param_name = param_name[1:-1]
                        else:
                            param_type = 'in'
                        api_info['params'].append({
                            'name': param_name,
                            'type': param_type,
                            'description': param_desc
                        })
                elif line.startswith('@return'):
                    api_info['return'] = line[7:].strip()
                elif line.startswith('@example'):
                    current_section = 'example'
                elif line.startswith('@note'):
                    api_info['notes'].append(line[5:].strip())
                elif line.startswith('@since'):
                    api_info['since'] = line[6:].strip()
                elif line.startswith('@deprecated'):
                    api_info['deprecated'] = True
                elif line.startswith('@tag'):
                    api_info['tags'].append(line[4:].strip())
            else:
                if current_section == 'example':
                    if not line.startswith('@'):
                        current_content.append(line)
                else:
                    current_content.append(line)
        
        # 处理剩余内容
        if current_section == 'description' and current_content:
            if not api_info['brief']:
                api_info['brief'] = current_content[0] if current_content else ''
            api_info['description'] = '\n'.join(current_content)
        elif current_section == 'example' and current_content:
            api_info['example'] = '\n'.join(current_content)
        
        return api_info
    
    def scan_headers(self):
        """扫描所有头文件"""
        print("🔍 扫描头文件...")
        
        # 扫描include目录
        if self.include_dir.exists():
            for header in self.include_dir.rglob('*.h'):
                module_name = self.get_module_name(header)
                if module_name not in self.apis:
                    self.apis[module_name] = []
                
                apis = self.extract_from_file(header)
                self.apis[module_name].extend(apis)
        
        # 扫描src目录中的头文件
        if self.src_dir.exists():
            for header in self.src_dir.rglob('*.h'):
                module_name = self.get_module_name(header)
                if module_name not in self.apis:
                    self.apis[module_name] = []
                
                apis = self.extract_from_file(header)
                self.apis[module_name].extend(apis)
    
    def get_module_name(self, file_path: Path) -> str:
        """获取模块名称"""
        # 尝试从路径中提取模块名
        parts = file_path.parts
        
        if 'include' in parts:
            idx = parts.index('include')
            if idx + 1 < len(parts):
                return parts[idx + 1].replace('.h', '')
        
        if 'src' in parts:
            idx = parts.index('src')
            if idx + 1 < len(parts):
                return parts[idx + 1]
        
        # 默认使用文件名
        return file_path.stem
    
    def generate_markdown(self, module_name: str, apis: List[Dict[str, Any]]) -> str:
        """生成Markdown格式的API文档"""
        lines = []
        
        # 标题
        lines.append(f"# {module_name} API 参考")
        lines.append("")
        lines.append(f"本文档自动从源代码生成，最后更新时间：{self.get_timestamp()}")
        lines.append("")
        
        # 模块概述
        lines.append("## 模块概述")
        lines.append("")
        lines.append(f"{module_name} 模块提供了以下 {len(apis)} 个API函数：")
        lines.append("")
        
        # API列表
        lines.append("## API 列表")
        lines.append("")
        lines.append("| 函数名 | 简介 | 状态 |")
        lines.append("|--------|------|------|")
        
        for api in apis:
            status = "⚠️ 已废弃" if api['deprecated'] else "✅ 正常"
            brief = api['brief'] or api['description'][:50] + '...' if api['description'] else '无描述'
            lines.append(f"| [{api['name']}](#{api['name'].lower()}) | {brief} | {status} |")
        
        lines.append("")
        
        # 详细API文档
        lines.append("## API 详细文档")
        lines.append("")
        
        for api in apis:
            lines.append(f"### {api['name']}")
            lines.append("")
            
            # 废弃警告
            if api['deprecated']:
                lines.append("> ⚠️ **注意**: 此API已被废弃，请使用替代方案。")
                lines.append("")
            
            # 简介
            if api['brief']:
                lines.append(f"**简介**: {api['brief']}")
                lines.append("")
            
            # 函数签名
            lines.append("**函数签名**:")
            lines.append("```c")
            lines.append(api['signature'])
            lines.append("```")
            lines.append("")
            
            # 详细描述
            if api['description']:
                lines.append("**详细描述**:")
                lines.append("")
                lines.append(api['description'])
                lines.append("")
            
            # 参数说明
            if api['params']:
                lines.append("**参数**:")
                lines.append("")
                lines.append("| 参数名 | 类型 | 说明 |")
                lines.append("|--------|------|------|")
                
                for param in api['params']:
                    param_type_icon = "📥" if param['type'] == 'in' else "📤" if param['type'] == 'out' else "🔄"
                    lines.append(f"| {param['name']} | {param_type_icon} | {param['description']} |")
                
                lines.append("")
            
            # 返回值
            if api['return']:
                lines.append("**返回值**:")
                lines.append("")
                lines.append(api['return'])
                lines.append("")
            
            # 示例代码
            if api['example']:
                lines.append("**示例**:")
                lines.append("```c")
                lines.append(api['example'])
                lines.append("```")
                lines.append("")
            
            # 注意事项
            if api['notes']:
                lines.append("**注意事项**:")
                lines.append("")
                for note in api['notes']:
                    lines.append(f"- {note}")
                lines.append("")
            
            # 版本信息
            if api['since']:
                lines.append(f"**可用版本**: {api['since']}")
                lines.append("")
            
            # 标签
            if api['tags']:
                lines.append(f"**标签**: {', '.join(['`' + tag + '`' for tag in api['tags']])}")
                lines.append("")
            
            # 源文件
            lines.append(f"**源文件**: `{api['file']}`")
            lines.append("")
            lines.append("---")
            lines.append("")
        
        return '\n'.join(lines)
    
    def get_timestamp(self) -> str:
        """获取当前时间戳"""
        from datetime import datetime
        return datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    
    def save_api_docs(self):
        """保存API文档"""
        print("📝 生成API文档...")
        
        # 生成总览文档
        overview_lines = []
        overview_lines.append("# API 参考文档")
        overview_lines.append("")
        overview_lines.append("本文档包含 EmbedKit 的完整 API 参考。")
        overview_lines.append("")
        overview_lines.append("## 模块列表")
        overview_lines.append("")
        
        total_apis = 0
        for module_name, apis in sorted(self.apis.items()):
            if apis:
                api_count = len(apis)
                total_apis += api_count
                overview_lines.append(f"- [{module_name}]({module_name}.md) - {api_count} 个API")
                
                # 生成模块API文档
                module_doc = self.generate_markdown(module_name, apis)
                module_file = self.output_dir / f"{module_name}.md"
                
                with open(module_file, 'w', encoding='utf-8') as f:
                    f.write(module_doc)
                
                print(f"  ✅ 生成 {module_name} 模块文档: {api_count} 个API")
        
        overview_lines.append("")
        overview_lines.append(f"**总计**: {total_apis} 个API函数")
        overview_lines.append("")
        overview_lines.append("## 使用说明")
        overview_lines.append("")
        overview_lines.append("1. 点击模块名称查看该模块的详细API文档")
        overview_lines.append("2. 使用浏览器的搜索功能（Ctrl+F）快速查找特定API")
        overview_lines.append("3. 每个API都包含详细的参数说明和示例代码")
        overview_lines.append("")
        overview_lines.append("## API 文档规范")
        overview_lines.append("")
        overview_lines.append("### 参数类型标记")
        overview_lines.append("")
        overview_lines.append("- 📥 输入参数（in）")
        overview_lines.append("- 📤 输出参数（out）")
        overview_lines.append("- 🔄 输入输出参数（inout）")
        overview_lines.append("")
        overview_lines.append("### 状态标记")
        overview_lines.append("")
        overview_lines.append("- ✅ 正常 - API正常可用")
        overview_lines.append("- ⚠️ 已废弃 - API已废弃，建议使用替代方案")
        overview_lines.append("- 🔧 实验性 - API处于实验阶段，可能会变更")
        
        # 保存总览文档
        overview_file = self.output_dir / "README.md"
        with open(overview_file, 'w', encoding='utf-8') as f:
            f.write('\n'.join(overview_lines))
        
        print(f"✅ API文档生成完成: 共 {total_apis} 个API")
    
    def generate_api_index(self):
        """生成API索引（JSON格式）"""
        index = {
            'generated': self.get_timestamp(),
            'modules': {},
            'total_apis': 0
        }
        
        for module_name, apis in self.apis.items():
            if apis:
                index['modules'][module_name] = {
                    'count': len(apis),
                    'apis': [
                        {
                            'name': api['name'],
                            'brief': api['brief'],
                            'deprecated': api['deprecated'],
                            'tags': api['tags']
                        }
                        for api in apis
                    ]
                }
                index['total_apis'] += len(apis)
        
        # 保存索引文件
        index_file = self.output_dir / "api-index.json"
        with open(index_file, 'w', encoding='utf-8') as f:
            json.dump(index, f, ensure_ascii=False, indent=2)
        
        print(f"✅ API索引已生成: {index_file}")
    
    def run(self):
        """运行API文档提取器"""
        print("🚀 开始提取API文档...")
        self.scan_headers()
        self.save_api_docs()
        self.generate_api_index()
        print("✨ API文档提取完成！")

if __name__ == '__main__':
    extractor = APIDocExtractor()
    extractor.run()