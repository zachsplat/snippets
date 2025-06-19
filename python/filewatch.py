"""watch a file for changes and run a callback. polling-based,
   no inotify because I wanted it to work on mac too"""

import os
import time

def watch(path, callback, interval=1.0):
    last_mtime = 0
    while True:
        try:
            mtime = os.path.getmtime(path)
            if mtime > last_mtime:
                last_mtime = mtime
                callback(path)
        except FileNotFoundError:
            pass
        time.sleep(interval)

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        print(f'usage: {sys.argv[0]} <file>')
        sys.exit(1)
    def on_change(path):
        print(f'{path} changed')
    watch(sys.argv[1], on_change)
