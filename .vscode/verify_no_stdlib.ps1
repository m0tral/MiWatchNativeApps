$forbidden = @(
    '<stdlib.h>','<stdio.h>','<string.h>','<wchar.h>','<unistd.h>',
    '<time.h>','<math.h>','<pthread.h>','<semaphore.h>','<signal.h>',
    '<errno.h>','<locale.h>','<setjmp.h>','<sys/types.h>','<sys/stat.h>',
    '<sys/wait.h>','<dirent.h>','<fcntl.h>','<malloc.h>','<assert.h>'
)

# Filenames allowed to mention stdlib. The replacement headers (mem.h,
# print.h) reference the standard names they're shadowing in comments;
# reminder.h is included as a documentation exception.
$allow = @(
    'mem.h','print.h',
    'reminder.h'
)

$files = Get-ChildItem -LiteralPath 'mb10pro' -Recurse -File -Include *.c,*.h |
    Where-Object { $allow -notcontains $_.Name }

$violations = @()
foreach ($f in $files) {
    $lines = Get-Content $f.FullName
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        foreach ($h in $forbidden) {
            if ($line -match [regex]::Escape($h)) {
                $violations += ('{0}:{1}  imports {2}' -f $f.FullName, ($i + 1), $h)
                break
            }
        }
    }
}

if ($violations.Count -gt 0) {
    Write-Host ''
    Write-Host 'BUILD BLOCKED -- forbidden standard-header imports:' -ForegroundColor Red
    foreach ($v in $violations) {
        Write-Host ('  ' + $v) -ForegroundColor Red
    }
    Write-Host ''
    Write-Host ('Allowed only freestanding headers (<stdint.h>, <stddef.h>, <stdarg.h>). ' +
               'Replace standard-library calls with the local helpers in misc/mem.h / ' +
               'misc/print.h or with firmware bindings in platform/mb10p_platform.h.') -ForegroundColor Yellow
    exit 1
} else {
    Write-Host 'OK: no stdlib imports found in mb10pro/.' -ForegroundColor Green
}
