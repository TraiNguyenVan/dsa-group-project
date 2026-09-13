package main

// Peak-memory test for the Go phonebook (report D.5, pprof).
// Env-gated so normal `go test` is unaffected:
//
//	MEMPROFILE_CSV=<csv> MEMPROFILE_OUT=<pprof-out> go test -v -run TestMemProfile
//
// GC is disabled during load so runtime.MemStats HeapInuse after load is
// the peak live heap (no collection can shrink it mid-load).

import (
	"fmt"
	"os"
	"runtime"
	"runtime/debug"
	"runtime/pprof"
	"testing"
)

func TestMemProfile(t *testing.T) {
	csvPath := os.Getenv("MEMPROFILE_CSV")
	outPath := os.Getenv("MEMPROFILE_OUT")
	if csvPath == "" || outPath == "" {
		t.Skip("MEMPROFILE_CSV/MEMPROFILE_OUT not set; skipping memory profile")
	}

	// Disable GC so HeapInuse after load == peak live heap.
	old := debug.SetGCPercent(-1)
	defer debug.SetGCPercent(old)

	pb := NewPhoneBook()
	loaded := pb.LoadFromCSV(csvPath)
	if loaded == -1 {
		t.Fatalf("Cannot open file: %s", csvPath)
	}

	var ms runtime.MemStats
	runtime.ReadMemStats(&ms)

	f, err := os.Create(outPath)
	if err != nil {
		t.Fatalf("cannot write pprof: %v", err)
	}
	if err := pprof.WriteHeapProfile(f); err != nil {
		f.Close()
		t.Fatalf("pprof write failed: %v", err)
	}
	f.Close()

	fmt.Printf("loaded %d contacts\n", loaded)
	fmt.Printf("peak_go_heap_bytes %d\n", ms.HeapInuse)
	fmt.Printf("go HeapAlloc=%d HeapSys=%d Sys=%d NumGC=%d\n",
		ms.HeapAlloc, ms.HeapSys, ms.Sys, ms.NumGC)
}
