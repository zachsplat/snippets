/* lock-free single-producer single-consumer ring buffer.
   useful for passing data between threads without mutexes */

#include <stdint.h>
#include <string.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>

struct ringbuf {
    uint8_t *data;
    size_t size;    /* must be power of 2 */
    size_t mask;
    atomic_size_t head;  /* write position */
    atomic_size_t tail;  /* read position */
};

struct ringbuf *ringbuf_create(size_t size)
{
    /* round up to power of 2 */
    size_t s = 1;
    while (s < size) s <<= 1;

    struct ringbuf *rb = malloc(sizeof(*rb));
    rb->data = malloc(s);
    rb->size = s;
    rb->mask = s - 1;
    atomic_store(&rb->head, 0);
    atomic_store(&rb->tail, 0);
    return rb;
}

size_t ringbuf_write(struct ringbuf *rb, const void *buf, size_t len)
{
    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    size_t avail = rb->size - (head - tail);
    if (len > avail) len = avail;
    if (len == 0) return 0;

    size_t off = head & rb->mask;
    size_t first = rb->size - off;
    if (first > len) first = len;
    memcpy(rb->data + off, buf, first);
    if (len > first)
        memcpy(rb->data, (uint8_t *)buf + first, len - first);

    atomic_store_explicit(&rb->head, head + len, memory_order_release);
    return len;
}

size_t ringbuf_read(struct ringbuf *rb, void *buf, size_t len)
{
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t avail = head - tail;
    if (len > avail) len = avail;
    if (len == 0) return 0;

    size_t off = tail & rb->mask;
    size_t first = rb->size - off;
    if (first > len) first = len;
    memcpy(buf, rb->data + off, first);
    if (len > first)
        memcpy((uint8_t *)buf + first, rb->data, len - first);

    atomic_store_explicit(&rb->tail, tail + len, memory_order_release);
    return len;
}

void ringbuf_free(struct ringbuf *rb) {
    free(rb->data);
    free(rb);
}

#ifdef TEST_RINGBUF
int main(void) {
    struct ringbuf *rb = ringbuf_create(16);
    char w[] = "hello world";
    ringbuf_write(rb, w, strlen(w));

    char r[32] = {0};
    size_t n = ringbuf_read(rb, r, sizeof(r));
    printf("read %zu: '%s'\n", n, r);
    ringbuf_free(rb);
}
#endif
