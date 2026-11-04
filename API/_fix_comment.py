# -*- coding: utf-8 -*-
p = r'E:/Desktop/Project/bzk.EXTI/API/USART.c'
b = open(p, 'rb').read()
for enc in ('utf-8', 'gbk'):
    try:
        t = b.decode(enc)
        print('decoded as', enc)
        break
    except UnicodeDecodeError:
        continue
else:
    raise SystemExit('decode failed')

old = None
for line in t.splitlines():
    if 'AA' in line and '|' in line:
        print('FOUND:', line)
        old = line

# Replace pipe-separated frame format with space-separated
t2 = t.replace('AA | ', 'AA ')
t2 = t2.replace(' | ', ' ')

# Also update Port layout comment style if pipes remain - already handled

if t2 == t:
    # fallback: direct line replace for garbled or utf8
    import re
    t2, n = re.subn(
        r'\* .*AA \|.*\| BB',
        '* 帧格式: AA 命令 长度N 数据[N] 校验 BB',
        t,
        count=1,
    )
    print('regex replacements', n)

open(p, 'w', encoding=enc, newline='\r\n').write(t2)
print('done, enc=', enc)

# verify
for line in t2.splitlines():
    if 'AA' in line and ('帧' in line or '01 02' in line or 'CMD' in line or '命令' in line or '格式' in line):
        print('OUT:', line)
