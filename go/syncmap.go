package snippets

import "sync"

// ShardedMap is a concurrent map using sharding to reduce lock contention.
// way faster than sync.Map for mixed read/write workloads
type ShardedMap[K comparable, V any] struct {
	shards    []*shard[K, V]
	numShards int
	hashFn    func(K) uint64
}

type shard[K comparable, V any] struct {
	mu   sync.RWMutex
	data map[K]V
}

func NewShardedMap[K comparable, V any](numShards int, hashFn func(K) uint64) *ShardedMap[K, V] {
	sm := &ShardedMap[K, V]{
		shards:    make([]*shard[K, V], numShards),
		numShards: numShards,
		hashFn:    hashFn,
	}
	for i := range sm.shards {
		sm.shards[i] = &shard[K, V]{data: make(map[K]V)}
	}
	return sm
}

func (sm *ShardedMap[K, V]) getShard(key K) *shard[K, V] {
	h := sm.hashFn(key)
	return sm.shards[h%uint64(sm.numShards)]
}

func (sm *ShardedMap[K, V]) Get(key K) (V, bool) {
	s := sm.getShard(key)
	s.mu.RLock()
	v, ok := s.data[key]
	s.mu.RUnlock()
	return v, ok
}

func (sm *ShardedMap[K, V]) Set(key K, val V) {
	s := sm.getShard(key)
	s.mu.Lock()
	s.data[key] = val
	s.mu.Unlock()
}

func (sm *ShardedMap[K, V]) Delete(key K) {
	s := sm.getShard(key)
	s.mu.Lock()
	delete(s.data, key)
	s.mu.Unlock()
}
