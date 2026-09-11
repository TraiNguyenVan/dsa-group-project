package main

import (
	"fmt"
	"math"
	"time"
)

// Faithful port of include/timer.hpp.

// TimeIt runs work and returns elapsed milliseconds.
func TimeIt(work func()) float64 {
	t0 := time.Now()
	work()
	return float64(time.Since(t0).Nanoseconds()) / 1e6
}

// Benchmark returns the best of `repeats` runs.
func Benchmark(work func(), repeats ...int) float64 {
	r := 5
	if len(repeats) > 0 {
		r = repeats[0]
	}
	if r <= 0 {
		return 0.0
	}
	best := math.MaxFloat64
	for i := 0; i < r; i++ {
		t := TimeIt(work)
		if t < best {
			best = t
		}
	}
	return best
}

// PrintTaskDuration prints "\nTook: Xms." like C++ printTaskDuration.
func PrintTaskDuration(work func()) {
	fmt.Printf("\nTook: %vms.\n", TimeIt(work))
}
