"""debounce decorator. calls the function only after it hasn't been
   called for `wait` seconds. useful for search-as-you-type stuff"""

import threading

def debounce(wait):
    def decorator(fn):
        timer = [None]
        def debounced(*args, **kwargs):
            if timer[0]:
                timer[0].cancel()
            timer[0] = threading.Timer(wait, fn, args, kwargs)
            timer[0].start()
        debounced.cancel = lambda: timer[0] and timer[0].cancel()
        return debounced
    return decorator
