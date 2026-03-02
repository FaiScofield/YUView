function JsonDB-Init()
{
  [string] $outputPath = (EnsureTrailingSlash( Get-SourceDirectory ))
  $outputPath += "compile_commands.json"

  # Clean up existing global variables to support running the script multiple times
  if (Get-Variable -name "kJsonCompilationDbPath" -Scope Global -ErrorAction SilentlyContinue)
  {
    Remove-Variable -name "kJsonCompilationDbPath" -Scope Global -Force
  }

  # Set-Variable -name "kJsonCompilationDbPath" -value $outputPath -option Constant -scope Global
  Set-Variable -name "kJsonCompilationDbPath" -value $outputPath -scope Global
  Set-Variable -name "kJsonCompilationDbCount" -value 0 -scope Global

  # Write-Output "JSON Compilation Database update to: $kJsonCompilationDbPath"

  "[" | Out-File $kJsonCompilationDbPath -Encoding "UTF8"
}

function JsonDB-Append($text)
{
  $text | Out-File $kJsonCompilationDbPath -append -Encoding "UTF8"
}

function JsonDB-Finalize()
{
  JsonDB-Append "]"
  Write-Output "Exported JSON Compilation Database to $kJsonCompilationDbPath"
}

function JsonDB-Push($directory, $file, $command)
{
  if ($kJsonCompilationDbCount -ge 1)
  {
    JsonDB-Append "  ,"
  }

  # use only slashes
  $command = $command.Replace('\', '/')
  $file = $file.Replace('\', '/')
  $directory = $directory.Replace('\', '/')

  # escape double quotes
  $command = $command.Replace('"', '\"')

  # make paths relative to directory
  $command = $command.Replace("$directory/", "")
  $file = $file.Replace("$directory/", "")

  JsonDB-Append "  {`r`n    ""directory"": ""$directory"",`r`n    ""command"": ""$command"",`r`n    ""file"": ""$file""`r`n  }"

  Set-Variable -name "kJsonCompilationDbCount" -value ($kJsonCompilationDbCount + 1) -scope Global
}
