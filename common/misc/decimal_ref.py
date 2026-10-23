#!/usr/bin/env python3
"""Independent reference for common/misc/decimal.h (exact rationals).

    python3 common/misc/decimal_ref.py --check /tmp/decimal_test [N] [SEED]

generates N deterministic random expressions (default 50000), evaluates them
with fractions.Fraction under the precision contract documented in
decimal.h, runs `decimal_test --eval` on the same lines and compares.
Exit status 0 when every result matches.
"""
import random
import subprocess
import sys
from fractions import Fraction

SIG, SCALE, LIMIT = 15, 15, 10 ** 15


class CalcError(Exception):
    pass


def round_contract(x):
    """Round exact x once: 15 significant digits, quantum >= 10^-15, ties away from zero."""
    if x == 0:
        return Fraction(0)
    a = abs(x)
    e = 0                                  # 10^e <= a < 10^(e+1)
    while a >= Fraction(10) ** (e + 1):
        e += 1
    while a < Fraction(10) ** e:
        e -= 1
    q = max(e - (SIG - 1), -SCALE)
    unit = Fraction(10) ** q
    n, rem = divmod(a, unit)
    if rem * 2 >= unit:
        n += 1
    r = n * unit
    if r >= LIMIT:
        raise CalcError('ERR_OVERFLOW')
    return r if x > 0 else -r


def parse_literal(s):
    if not s or s == '.' or s.count('.') > 1 or any(c not in '0123456789.' for c in s):
        raise CalcError('ERR_SYNTAX')
    ip, _, fp = s.partition('.')
    digits = (ip + fp).lstrip('0')
    if len(digits) > SIG or len(fp) > SCALE:
        raise CalcError('ERR_INPUT')
    return Fraction(int(ip or '0')) + (Fraction(int(fp), 10 ** len(fp)) if fp else 0)


def evaluate(expr):
    neg = expr.startswith('-')
    body = expr[1:] if neg else expr
    tokens, cur = [], ''
    for c in body:
        if c in '+-*/':
            tokens += [cur, c]
            cur = ''
        else:
            cur += c
    tokens.append(cur)
    vals = []
    for i, t in enumerate(tokens):
        if i % 2 == 0:
            try:
                vals.append(parse_literal(t))
            except CalcError:
                raise
    if neg:
        vals[0] = -vals[0]
    ops = tokens[1::2]
    # left to right: * and / bind tighter; every operation rounded once
    total, term, pending = None, vals[0], None
    for op, v in zip(ops, vals[1:]):
        if op in '*/':
            if op == '/' and v == 0:
                raise CalcError('ERR_DIV0')
            term = round_contract(term * v if op == '*' else term / v)
        else:
            total = term if pending is None else round_contract(total + term if pending == '+' else total - term)
            pending, term = op, v
    total = term if pending is None else round_contract(total + term if pending == '+' else total - term)
    return total


def fmt(x):
    neg = x < 0
    m = abs(x) * 10 ** SCALE
    assert m.denominator == 1
    m = m.numerator
    ip, fp = divmod(m, 10 ** SCALE)
    s = str(ip)
    if fp:
        s += '.' + str(fp).zfill(SCALE).rstrip('0')
    return ('-' if neg and m else '') + s


def reference(expr):
    try:
        return fmt(evaluate(expr))
    except CalcError as e:
        return str(e)


def literal(rng):
    kind = rng.random()
    if kind < 0.08:
        return rng.choice(['0', '1', '2', '3', '5', '9', '10', '0.5', '99999', '0.1', '0.000000000000001',
                           '999999999999999', '0.999999999999999', '123456789012345', '7'])
    sig = rng.randint(1, SIG)
    digits = str(rng.randint(10 ** (sig - 1), 10 ** sig - 1))
    if kind < 0.3:                                    # integer
        return digits
    frac = rng.randint(1, min(SCALE, sig + rng.randint(0, 6)))
    if frac >= len(digits):
        if frac - len(digits) + len(digits) > SCALE:
            frac = SCALE
        return '0.' + digits.rjust(frac, '0')[-SCALE:]
    return digits[:-frac] + '.' + digits[-frac:]


def boundary(rng):
    """Expressions aimed at rounding ties, carries and the range limits."""
    a15 = str(rng.randint(10 ** 14, 10 ** 15 - 1))
    k = rng.randint(0, 14)
    a = (a15[:15 - k] + '.' + a15[15 - k:]) if k else a15          # 15 significant digits
    tie = '0.' + '0' * (k - 1) + '5' if k else '0.5'                # half a unit of a's last place
    return rng.choice([
        a + '+' + tie, a + '-' + tie, '-' + a + '-' + tie, a + '/2', a + '*5', a + '*0.5',
        a + '/' + rng.choice(['3', '7', '9', '11', '0.3', '1.5', '8']),
        a + '*' + a, a + '*1' + '0' * rng.randint(0, 3), a + '/0.' + '0' * rng.randint(0, 13) + '1',
        '0.' + '0' * rng.randint(0, 13) + str(rng.randint(1, 9)) + rng.choice('*/') + str(rng.randint(1, 99)),
        '999999999999999' + rng.choice(['+', '-']) + rng.choice(['0.4', '0.5', '0.6', '1']),
        a + '-' + a, a + '+' + a,
    ])


def expression(rng):
    if rng.random() < 0.3:
        return boundary(rng)
    n = rng.choices([1, 2, 3, 4, 6], weights=[1, 6, 4, 2, 1])[0]
    parts = [literal(rng)]
    for _ in range(n - 1):
        parts.append(rng.choices('+-*/', weights=[3, 3, 3, 3])[0])
        parts.append(literal(rng))
    e = ''.join(parts)
    if rng.random() < 0.1:
        e = '-' + e
    return e


def check(binary, n, seed):
    rng = random.Random(seed)
    exprs = [expression(rng) for _ in range(n)]
    out = subprocess.run([binary, '--eval'], input='\n'.join(exprs) + '\n', capture_output=True,
                         text=True, check=True).stdout.splitlines()
    bad = 0
    stats = {}
    for e, line in zip(exprs, out):
        got = line.split('\t', 1)[1]
        want = reference(e)
        k = want if want.startswith('ERR') else 'value'
        stats[k] = stats.get(k, 0) + 1
        if got != want:
            bad += 1
            if bad <= 10:
                print('MISMATCH %-50s C=%s ref=%s' % (e, got, want))
    print('%d expressions (seed %d) %s: %d mismatches' % (len(exprs), seed, dict(sorted(stats.items())), bad))
    return 1 if bad or len(out) != len(exprs) else 0


if __name__ == '__main__':
    if len(sys.argv) >= 3 and sys.argv[1] == '--check':
        sys.exit(check(sys.argv[2], int(sys.argv[3]) if len(sys.argv) > 3 else 50000,
                       int(sys.argv[4]) if len(sys.argv) > 4 else 1))
    for line in sys.stdin:
        print(reference(line.strip()))
