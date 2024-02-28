#!/bin/bash
# 指定要输出的文件夹
DIR="/mnt/e/workspace/vscode_workspace/RoodOs/src/kernel/fs"
# 遍历目录中的文件
for file in "$DIR"/*; do
  if [ -f "$file" ]; then
    echo "//-- "$(basename "$file")""     
    python3 interface_extra.py "$file"
    echo "//--"
    echo " "
  fi
done
