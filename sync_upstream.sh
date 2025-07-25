#!/bin/bash

# 小智 ESP32 上游同步脚本
# 用于同步上游更新并应用到旋转分支

set -e

echo "🔄 开始同步上游更新..."

# 获取上游更新
echo "📥 获取上游更新..."
git fetch upstream

# 切换到主分支
echo "📋 切换到主分支..."
git checkout main

# 合并上游更新
echo "🔄 合并上游更新..."
git merge upstream/main

# 切换到旋转分支
echo "📋 切换到旋转分支..."
git checkout waveshare-c6-rotated

# 变基到主分支
echo "🔄 变基到主分支..."
git rebase main

# 推送更新
echo "📤 推送更新到远程仓库..."
git push origin waveshare-c6-rotated --force-with-lease

echo "✅ 同步完成！"
echo ""
echo "📝 当前状态："
echo "   - 主分支已同步到上游最新版本"
echo "   - 旋转分支已更新并推送"
echo ""
echo "🔧 下一步："
echo "   1. 测试旋转配置是否正常工作"
echo "   2. 如有问题，调整 config.h 中的参数"
echo "   3. 提交并推送修改" 