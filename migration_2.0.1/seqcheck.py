import re, sys
t = open(sys.argv[1], 'rb').read().decode(errors='replace').replace('\r', '')
t = t[t.index('Welcome to STM32 world !'):]
head = t.split('Send:')[0]
vals = [(int(a), int(b)) for a, b in re.findall(r'^Send: (\d+) Received: (\d+)$', t, re.M)]
bad = [i for i, (a, b) in enumerate(vals) if a != b or a != i]
print(f'header {head!r}; {len(vals)} complete lines, values 0..{vals[-1][0]}, out-of-sequence: {len(bad)}')
