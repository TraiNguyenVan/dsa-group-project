package main

// Peak-memory test for the Go phonebook (report D.5, pprof).
// Env-gated so normal `go test` is unaffected:
//
//	MEMPROFILE_CSV=<csv> MEMPROFILE_OUT=<pprof-out> go test -v -run TestMemProfile
//
// GC is left enabled (default GOGC) so the number matches the guide's
// "peak footprint of the data structure" under normal runtime — same rule
// as JS (GC + heapUsed) and Java (G1 polled). Two explicit GCs settle
// floating garbage, then HeapAlloc is the live footprint.

import (
	"fmt"
	"os"
	"runtime"
	"runtime/pprof"
	"testing"
)

func TestMemProfile(t *testing.T) {
	csvPath := os.Getenv("MEMPROFILE_CSV")
	outPath := os.Getenv("MEMPROFILE_OUT")
	if csvPath == "" || outPath == "" {
		t.Skip("MEMPROFILE_CSV/MEMPROFILE_OUT not set; skipping memory profile")
	}

	pb := NewPhoneBook()
	loaded := pb.LoadFromCSV(csvPath)
	if loaded == -1 {
		t.Fatalf("Cannot open file: %s", csvPath)
	}

	// Settle floating garbage so HeapAlloc == live footprint.
	// KeepAlive: pb is unused after this point, without it the
	// explicit GCs below could collect the phonebook itself.
	runtime.GC()
	runtime.GC()
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
	runtime.KeepAlive(pb)

	fmt.Printf("loaded %d contacts\n", loaded)
	fmt.Printf("peak_go_heap_bytes %d\n", ms.HeapAlloc)
	fmt.Printf("go HeapAlloc=%d HeapInuse=%d HeapSys=%d Sys=%d NumGC=%d\n",
		ms.HeapAlloc, ms.HeapInuse, ms.HeapSys, ms.Sys, ms.NumGC)
}
