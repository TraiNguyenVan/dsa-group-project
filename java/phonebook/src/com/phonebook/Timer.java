package com.phonebook;

// Faithful port of include/timer.hpp.
public final class Timer {
    private Timer() {}

    public static double timeIt(Runnable work) {
        long t0 = System.nanoTime();
        work.run();
        long t1 = System.nanoTime();
        return (t1 - t0) / 1e6;
    }

    public static double benchmark(Runnable work, int repeats) {
        if (repeats <= 0) {
            return 0.0;
        }
        double best = Double.MAX_VALUE;
        for (int r = 0; r < repeats; r++) {
            double t = timeIt(work);
            if (t < best) {
                best = t;
            }
        }
        return best;
    }

    public static double benchmark(Runnable work) {
        return benchmark(work, 5);
    }

    public static void printTaskDuration(Runnable work) {
        System.out.println("\nTook: " + timeIt(work) + "ms.");
    }
}
