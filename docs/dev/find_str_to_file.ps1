# 定义需要搜索的文件夹路径$FolderPath = "C:\yourFolderPath"

# 定义要查找的字符串
$SearchString = "qos"

# 定义结果输出文件的路径
$OutputFile = "demo.txt"

# 通过 Get-ChildItem 获取文件夹内的所有文件
$Files = Get-ChildItem -Path $FolderPath -Recurse -File

# 清空或创建输出文件
if (Test-Path -Path $OutputFile) {
    Clear-Content -Path $OutputFile
} else {
    New-Item -Path $OutputFile -Type File
}

# 遍历文件，对每个文件进行搜索
foreach ($File in $Files) {
    # 读取文件内容
    $FileContent = Get-Content -Path $File.FullName

    # 使用 Select-String 在文件内容中搜索指定字符串
    $SearchResult = $FileContent | Select-String -Pattern $SearchString
    # 如果搜索结果不为空，输出文件路径和搜索到的内容至输出文件
    if ($SearchResult) {
        "文件路径: $($File.FullName)" | Out-File -FilePath $OutputFile -Encoding UTF8 -Append 
        "匹配内容: $($SearchResult.Line)" | Out-File -FilePath $OutputFile -Encoding UTF8 -Append 
        "------------------------------------" | Out-File -FilePath $OutputFile -Encoding UTF8 -Append 
    }
}
