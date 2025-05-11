package snippets

import "container/list"

// LRU cache, not thread safe
type LRU[K comparable, V any] struct {
	cap   int
	items map[K]*list.Element
	order *list.List
}

type entry[K comparable, V any] struct {
	key K
	val V
}

func NewLRU[K comparable, V any](cap int) *LRU[K, V] {
	return &LRU[K, V]{
		cap:   cap,
		items: make(map[K]*list.Element),
		order: list.New(),
	}
}

func (c *LRU[K, V]) Get(key K) (V, bool) {
	if el, ok := c.items[key]; ok {
		c.order.MoveToFront(el)
		return el.Value.(*entry[K, V]).val, true
	}
	var zero V
	return zero, false
}

func (c *LRU[K, V]) Put(key K, val V) {
	if el, ok := c.items[key]; ok {
		c.order.MoveToFront(el)
		el.Value.(*entry[K, V]).val = val
		return
	}
	if c.order.Len() >= c.cap {
		oldest := c.order.Back()
		if oldest != nil {
			c.order.Remove(oldest)
			delete(c.items, oldest.Value.(*entry[K, V]).key)
		}
	}
	el := c.order.PushFront(&entry[K, V]{key, val})
	c.items[key] = el
}

func (c *LRU[K, V]) Len() int { return c.order.Len() }
