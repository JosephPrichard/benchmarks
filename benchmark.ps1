Write-Output "Starting benchmark...`n"

Write-Output "Running C benchmark"
$startTime = Get-Date
Start-Process -FilePath ".\c\cmake-build-debug\PuzzleC.exe" -ArgumentList "./8puzzles.txt" -Wait -NoNewWindow
$duration = (Get-Date) - $startTime
Write-Output "`nProcess started and ran for $($duration.Milliseconds) miliseconds."

Write-Output "`n"