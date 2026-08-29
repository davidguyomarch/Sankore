# download-latest.ps1
# Downloads the latest Open-Sankore build artifact from GitHub Actions
# Run this in PowerShell on the Windows VM

$repo = "davidguyomarch/Sankore"
$artifactName = "Open-Sankore-Windows-x64"
$destDir = "$env:USERPROFILE\Desktop\Open-Sankore"

Write-Host "=== Open-Sankore Artifact Downloader ===" -ForegroundColor Cyan
Write-Host "Fetching latest successful build from GitHub Actions..."

# Get latest successful run
$runsUrl = "https://api.github.com/repos/$repo/actions/runs?status=success&per_page=1"
try {
    $runs = Invoke-RestMethod -Uri $runsUrl -Headers @{"Accept"="application/vnd.github.v3+json"}
} catch {
    Write-Host "ERROR: Cannot reach GitHub API. Check internet connection." -ForegroundColor Red
    exit 1
}

if ($runs.workflow_runs.Count -eq 0) {
    Write-Host "ERROR: No successful builds found." -ForegroundColor Red
    exit 1
}

$runId = $runs.workflow_runs[0].id
$runDate = $runs.workflow_runs[0].created_at
Write-Host "Latest build: Run #$runId ($runDate)"

# Get artifacts for this run
$artifactsUrl = "https://api.github.com/repos/$repo/actions/runs/$runId/artifacts"
$artifacts = Invoke-RestMethod -Uri $artifactsUrl -Headers @{"Accept"="application/vnd.github.v3+json"}

$artifact = $artifacts.artifacts | Where-Object { $_.name -eq $artifactName }
if (-not $artifact) {
    Write-Host "ERROR: Artifact '$artifactName' not found in run #$runId" -ForegroundColor Red
    exit 1
}

$downloadUrl = $artifact.archive_download_url
Write-Host "Artifact: $($artifact.name) ($([math]::Round($artifact.size_in_bytes / 1MB, 1)) MB)"

# Need a GitHub token for artifact download (API requires auth)
$token = $env:GITHUB_TOKEN
if (-not $token) {
    Write-Host ""
    Write-Host "GitHub requires authentication to download artifacts." -ForegroundColor Yellow
    Write-Host "Option 1: Set GITHUB_TOKEN environment variable"
    Write-Host "Option 2: Download manually from:" -ForegroundColor Yellow
    Write-Host "  https://github.com/$repo/actions/runs/$runId" -ForegroundColor Green
    Write-Host ""
    Write-Host "To set token: `$env:GITHUB_TOKEN = 'ghp_your_token_here'" -ForegroundColor Gray
    
    # Try to open the browser instead
    $browserUrl = "https://github.com/$repo/actions/runs/$runId"
    Write-Host "Opening browser to download page..." -ForegroundColor Cyan
    Start-Process $browserUrl
    exit 0
}

# Download with token
Write-Host "Downloading artifact..."
$zipPath = "$env:TEMP\$artifactName.zip"
$headers = @{
    "Accept" = "application/vnd.github.v3+json"
    "Authorization" = "Bearer $token"
}
Invoke-WebRequest -Uri $downloadUrl -Headers $headers -OutFile $zipPath

# Extract
Write-Host "Extracting to $destDir..."
if (Test-Path $destDir) { Remove-Item -Recurse -Force $destDir }
Expand-Archive -Path $zipPath -DestinationPath $destDir -Force
Remove-Item $zipPath

Write-Host ""
Write-Host "=== Done! ===" -ForegroundColor Green
Write-Host "Open-Sankore installed at: $destDir"
Write-Host "Run: $destDir\Open-Sankore.exe"
Write-Host ""

# List files
Get-ChildItem $destDir | Format-Table Name, Length -AutoSize
