#!/usr/bin/env python3
"""
文档链接检查脚本
检查文档中的死链接和无效引用
"""

import os
import re
from pathlib import Path
from typing import List, Tuple

class LinkChecker:
    def __init__(self, docs_dir='docs'):
        self.docs_dir = Path(docs_dir)
        self.errors = []
        self.warnings = []
        self.checked_links = set()
        
    def check_file(self, file_path: Path) -> List[Tuple[int, str, str]]:
        """检查单个文件中的链接"""
        issues = []
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()
            
            for line_num, line in enumerate(lines, 1):
                # 查找Markdown链接
                md_links = re.findall(r'\[([^\]]+)\]\(([^)]+)\)', line)
                for link_text, link_url in md_links:
                    issue = self.validate_link(link_url, file_path)
                    if issue:
                        issues.append((line_num, link_url, issue))
                
                # 查找HTML链接
                html_links = re.findall(r'href="([^"]+)"', line)
                for link_url in html_links:
                    issue = self.validate_link(link_url, file_path)
                    if issue:
                        issues.append((line_num, link_url, issue))
                        
        except Exception as e:
            self.errors.append(f"无法读取文件 {file_path}: {e}")
            
        return issues
    
    def validate_link(self, link: str, source_file: Path) -> str:
        """验证链接有效性"""
        # 跳过外部链接
        if link.startswith(('http://', 'https://', 'mailto:', 'tel:')):
            return None
        
        # 跳过锚点链接
        if link.startswith('#'):
            return None
        
        # 处理相对路径
        if link.startswith('/'):
            # 绝对路径（相对于docs目录）
            target_path = self.docs_dir / link[1:]
        else:
            # 相对路径（相对于当前文件）
            target_path = source_file.parent / link
            
        # 移除锚点
        if '#' in str(target_path):
            target_path = Path(str(target_path).split('#')[0])
        
        # 检查文件是否存在
        if not target_path.exists():
            # 尝试添加.md扩展名
            md_path = Path(str(target_path) + '.md')
            if not md_path.exists():
                return f"文件不存在: {target_path}"
        
        return None
    
    def check_all_files(self):
        """检查所有Markdown文件"""
        print("🔍 开始检查文档链接...")
        
        md_files = list(self.docs_dir.rglob('*.md'))
        html_files = list(self.docs_dir.rglob('*.html'))
        all_files = md_files + html_files
        
        total_issues = 0
        
        for file_path in all_files:
            issues = self.check_file(file_path)
            if issues:
                relative_path = file_path.relative_to(self.docs_dir)
                print(f"\n❌ {relative_path}:")
                for line_num, link, issue in issues:
                    print(f"  行 {line_num}: {link}")
                    print(f"    → {issue}")
                    total_issues += 1
        
        if total_issues == 0:
            print("\n✅ 所有链接检查通过！")
        else:
            print(f"\n⚠️ 发现 {total_issues} 个链接问题")
        
        return total_issues == 0
    
    def generate_report(self):
        """生成检查报告"""
        report_lines = []
        report_lines.append("# 链接检查报告")
        report_lines.append("")
        report_lines.append(f"检查时间: {self.get_timestamp()}")
        report_lines.append("")
        
        if self.errors:
            report_lines.append("## 错误")
            report_lines.append("")
            for error in self.errors:
                report_lines.append(f"- {error}")
            report_lines.append("")
        
        if self.warnings:
            report_lines.append("## 警告")
            report_lines.append("")
            for warning in self.warnings:
                report_lines.append(f"- {warning}")
            report_lines.append("")
        
        return '\n'.join(report_lines)
    
    def get_timestamp(self):
        """获取时间戳"""
        from datetime import datetime
        return datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    
    def run(self):
        """运行链接检查器"""
        success = self.check_all_files()
        
        if self.errors or self.warnings:
            report = self.generate_report()
            report_file = self.docs_dir / 'link-check-report.md'
            with open(report_file, 'w', encoding='utf-8') as f:
                f.write(report)
            print(f"\n📄 详细报告已保存到: {report_file}")
        
        return success

if __name__ == '__main__':
    checker = LinkChecker()
    success = checker.run()
    exit(0 if success else 1)