package com.phonebook;

// Peak-memory probe for the Java phonebook (report D.5, VisualVM/JFR).
// Loads one CSV with the real PhoneBook port and reports:
//   - peak used heap, sampled by a daemon thread every ~5 ms during load
//   - live used heap after a full GC (settled footprint)
//
// Run wrapped in a flight recording for headless VisualVM evidence:
//   java -XX:StartFlightRecording=filename=<out>.jfr,dumponexit=true \
//        -cp out com.phonebook.MemProfile <csv>
//
// stdout (last line machine-parsed by benchmark/mem_profile.py):
//   peak_java_heap_bytes <N>

public class MemProfile {

    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("usage: MemProfile <csv>");
            System.exit(2);
        }
        String csvPath = args[0];

        PhoneBook phonebook = new PhoneBook();

        // Sampler thread: track peak used heap while loadfromCSV runs.
        final long[] peak = {0};
        final boolean[] running = {true};
        Runtime rt = Runtime.getRuntime();
        Thread sampler = new Thread(() -> {
            while (running[0]) {
                long used = rt.totalMemory() - rt.freeMemory();
                if (used > peak[0]) {
                    peak[0] = used;
                }
                try {
                    Thread.sleep(5);
                } catch (InterruptedException e) {
                    break;
                }
            }
        });
        sampler.setDaemon(true);
        sampler.start();

        int loaded = phonebook.loadfromCSV(csvPath);
        running[0] = false;
        if (loaded == -1) {
            System.err.println("Cannot open file: " + csvPath);
            System.exit(1);
        }
        try {
            sampler.join(100);
        } catch (InterruptedException ignored) {
            // fall through
        }

        // Settled footprint: full GC, then measure live heap.
        System.gc();
        try {
            Thread.sleep(100);
        } catch (InterruptedException ignored) {
            // fall through
        }
        System.gc();
        long liveAfterGc = rt.totalMemory() - rt.freeMemory();

        System.out.println("loaded " + loaded + " contacts");
        System.out.println("java_live_after_gc_bytes " + liveAfterGc);
        System.out.println("peak_java_heap_bytes " + peak[0]);
    }
}
