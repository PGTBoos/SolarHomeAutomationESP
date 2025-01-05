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
    #
    
    # Add file content in code block with type
    $markdownContent += [char]10+"-------------------" 
    $markdownContent += [char]10+(Get-Content  $file.FullName -Raw)
   # $markdownContent += [char]10+"// $($file.Name)"
 
}

# Write content to file
$markdownContent | Out-File -FilePath $outputFile -Encoding UTF8

Write-Host "Files have been combined into $outputFile"
Write-Host "Excluded file types: $($excludedExtensions -join ', ')"