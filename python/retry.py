"""simple retry decorator with exponential backoff"""

import time
import functools

def retry(max_attempts=3, delay=1.0, backoff=2.0, exceptions=(Exception,)):
    def decorator(fn):
        @functools.wraps(fn)
        def wrapper(*args, **kwargs):
            d = delay
            for attempt in range(max_attempts):
                try:
                    return fn(*args, **kwargs)
                except exceptions as e:
                    if attempt == max_attempts - 1:
                        raise
                    # print(f"attempt {attempt+1} failed: {e}, retrying in {d}s")
                    time.sleep(d)
                    d *= backoff
        return wrapper
    return decorator


# usage:
# @retry(max_attempts=5, delay=0.5, exceptions=(ConnectionError, TimeoutError))
# def flaky_request():
#     ...
