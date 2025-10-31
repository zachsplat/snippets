#!/usr/bin/env python3
"""quick and dirty csv querying from the command line.
   basically a worse version of csvkit but I didn't want to install anything"""

import csv
import sys
import operator

def load(path):
    with open(path) as f:
        reader = csv.DictReader(f)
        return list(reader), reader.fieldnames

def query(rows, where=None, select=None, sort=None, limit=None):
    if where:
        field, op, val = parse_where(where)
        ops = {'=': operator.eq, '>': operator.gt, '<': operator.lt,
               '>=': operator.ge, '<=': operator.le, '!=': operator.ne}
        cmp = ops.get(op, operator.eq)
        rows = [r for r in rows if cmp(r.get(field, ''), val)]
    if sort:
        reverse = sort.startswith('-')
        key = sort.lstrip('-')
        rows.sort(key=lambda r: r.get(key, ''), reverse=reverse)
    if limit:
        rows = rows[:limit]
    if select:
        fields = [f.strip() for f in select.split(',')]
        rows = [{k: r.get(k, '') for k in fields} for r in rows]
    return rows

def parse_where(expr):
    for op in ['>=', '<=', '!=', '>', '<', '=']:
        if op in expr:
            parts = expr.split(op, 1)
            return parts[0].strip(), op, parts[1].strip()
    return expr, '=', ''

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('usage: csv_query.py <file.csv> [--where "col=val"] [--select "col1,col2"] [--sort col] [--limit N]')
        sys.exit(1)

    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('file')
    ap.add_argument('--where')
    ap.add_argument('--select')
    ap.add_argument('--sort')
    ap.add_argument('--limit', type=int)
    args = ap.parse_args()

    rows, fields = load(args.file)
    result = query(rows, args.where, args.select, args.sort, args.limit)

    if not result:
        print('no results')
    else:
        w = csv.DictWriter(sys.stdout, fieldnames=result[0].keys())
        w.writeheader()
        w.writerows(result)
