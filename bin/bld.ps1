$v = (Get-ChildItem "../src/" -File -Recurse | ForEach-Object { $_.FullName })
gcc $v -o ../bin/vtxg.exe