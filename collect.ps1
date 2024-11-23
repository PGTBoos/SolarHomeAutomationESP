# Create output file path
$outputFile = "combined_files.md"

# Define excluded extensions
$excludedExtensions = @(
    ".json",
    ".jpg", ".jpeg", ".png", ".gif", ".bmp", 
    ".tiff", ".ico", ".svg", ".webp"
)

# Initialize empty string for content
$markdownContent = ""

# Get all files from specified directories
$files = Get-ChildItem -Path ".\src", ".\data", ".\include" -File -Recurse | 
    Where-Object { $excludedExtensions -notcontains $_.Extension }

foreach ($file in $files) {
    # Get file extension without the dot
    $fileType = $file.Extension.TrimStart(".")
    
    # Add filename as header
    $markdownContent += "# $($file.Name)`n`n"
    
    # Add file content in code block with type
    $markdownContent += "```$fileType`n"
    $markdownContent += (Get-Content -Path $file.FullName -Raw)
    $markdownContent += "`n````n`n"
}

# Write content to file
$markdownContent | Out-File -FilePath $outputFile -Encoding UTF8

Write-Host "Files have been combined into $outputFile"
Write-Host "Excluded file types: $($excludedExtensions -join ', ')"