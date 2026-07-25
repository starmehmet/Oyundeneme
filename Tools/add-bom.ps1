# Source/ altindaki tum .h/.cpp/.cs dosyalarina UTF-8 BOM ekler (varsa dokunmaz).
# NEDEN: TEXT() literallerinde Turkce karakter var; BOM'suz UTF-8'i MSVC Turkce
# Windows'ta cp1254 olarak okur ve metinler bozulur (mojibake). UE kodlama
# standardi ASCII disi icerik icin UTF-8 with BOM ister.
# KULLANIM: Her kod duzenleme seansindan sonra calistir:
#   powershell -ExecutionPolicy Bypass -File Tools\add-bom.ps1

$root = Join-Path $PSScriptRoot "..\Source"
$files = Get-ChildItem $root -Recurse -Include *.h, *.cpp, *.cs -File
$bom = [byte[]](0xEF, 0xBB, 0xBF)
$fixed = 0

foreach ($f in $files) {
    $bytes = [System.IO.File]::ReadAllBytes($f.FullName)
    if ($bytes.Length -lt 3 -or $bytes[0] -ne 0xEF -or $bytes[1] -ne 0xBB -or $bytes[2] -ne 0xBF) {
        [System.IO.File]::WriteAllBytes($f.FullName, $bom + $bytes)
        $fixed++
        Write-Output "BOM eklendi: $($f.FullName)"
    }
}

Write-Output "Bitti: $fixed dosya duzeltildi / $($files.Count) tarandi"
