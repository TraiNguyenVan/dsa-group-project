"""Faithful port of include/timer.hpp."""
import time
from typing import Callable

Repeats = 5


def time_it(work: Callable[[], None]) -> float:
    t0 = time.perf_counter()
    work()
    t1 = time.perf_counter()
    return (t1 - t0) * 1000.0


def benchmark(work: Callable[[], None], repeats: int = 5) -> float:
    if repeats <= 0:
        return 0.0
    best = float("inf")
    for _ in range(repeats):
        t = time_it(work)
        if t < best:
            best = t
    return best


def print_task_duration(work: Callable[[], None]) -> None:
    print(f"\nTook: {time_it(work)}ms.")
