package snippets

import "sync"

// generic worker pool. submit jobs, get results
type WorkerPool[T any, R any] struct {
	jobs    chan T
	results chan R
	wg      sync.WaitGroup
}

func NewWorkerPool[T any, R any](workers int, fn func(T) R) *WorkerPool[T, R] {
	wp := &WorkerPool[T, R]{
		jobs:    make(chan T, workers*2),
		results: make(chan R, workers*2),
	}
	for i := 0; i < workers; i++ {
		wp.wg.Add(1)
		go func() {
			defer wp.wg.Done()
			for job := range wp.jobs {
				wp.results <- fn(job)
			}
		}()
	}
	return wp
}

func (wp *WorkerPool[T, R]) Submit(job T) {
	wp.jobs <- job
}

func (wp *WorkerPool[T, R]) Close() {
	close(wp.jobs)
	wp.wg.Wait()
	close(wp.results)
}

func (wp *WorkerPool[T, R]) Results() <-chan R {
	return wp.results
}
