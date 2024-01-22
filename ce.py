# 提取某个c文件中的函数声明
import re
import sys

def process_function_declaration(declaration):
    # 将花括号及内部内容替换为分号，并将分号放在同一行
    processed_declaration = re.sub(r'{[^}]*}', ';', declaration).replace('\n', '')
    return processed_declaration

def extract_and_process_function_declarations(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
        declarations = re.findall(r'\b[A-Za-z_][A-Za-z0-9_]*\s+\*?[A-Za-z_][A-Za-z0-9_]*\s*\([^)]*\)\s*{[^}]*}', content)
        for declaration in declarations:
            a=declaration.split()
            if a[1]!="if":
                processed_declaration = process_function_declaration(declaration)
                print(processed_declaration)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: process_declarations.py <input_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    extract_and_process_function_declarations(input_file)