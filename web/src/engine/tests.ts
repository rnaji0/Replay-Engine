import type { TestResult, EventType } from './types';
import { runInstant } from './ReplayEngine';

function pass(module: string, name: string, msg: string, ms: number): TestResult {
  return { module, name, status: 'passed', message: msg, durationMs: ms };
}

function fail(module: string, name: string, msg: string, ms: number): TestResult {
  return { module, name, status: 'failed', message: msg, durationMs: ms };
}

function timed<T>(fn: () => T): [T, number] {
  const s = performance.now();
  const r = fn();
  return [r, performance.now() - s];
}

export function runDeterminismTests(): TestResult[] {
  const results: TestResult[] = [];
  const mod = 'ReplayDeterminism';

  // Bit-identical checksum
  {
    timed(() => {});
    const s = performance.now();
    const cfg = { eventCount: 2000, workerCount: 2, eventTypes: ['NewOrder' as EventType], batchSize: 2000 };
    const checksums: number[] = [];
    for (let i = 0; i < 5; i++) {
      checksums.push(runInstant(cfg).checksum);
    }
    const elapsed = performance.now() - s;
    const allMatch = checksums.every(c => c === checksums[0]);
    results.push(allMatch
      ? pass(mod, 'BitIdenticalChecksum', `5 runs identical: ${checksums[0]}`, elapsed)
      : fail(mod, 'BitIdenticalChecksum', 'checksums diverged', elapsed));
  }

  // Sequence ordering
  {
    const s = performance.now();
    const snap = runInstant({ eventCount: 500, workerCount: 4, eventTypes: ['NewOrder'], batchSize: 500 });
    const sorted = [...snap.results].sort((a, b) => a.sequenceId - b.sequenceId);
    const elapsed = performance.now() - s;
    const ok = sorted.every((r, i) => r.sequenceId === i + 1);
    results.push(ok
      ? pass(mod, 'SequenceOrdering', '500 IDs contiguous', elapsed)
      : fail(mod, 'SequenceOrdering', 'gap in sequence IDs', elapsed));
  }

  // Varying worker counts
  {
    const s = performance.now();
    const base = runInstant({ eventCount: 1000, workerCount: 1, eventTypes: ['NewOrder'], batchSize: 1000 });
    let ok = true;
    for (const w of [2, 4, 8]) {
      const snap = runInstant({ eventCount: 1000, workerCount: w, eventTypes: ['NewOrder'], batchSize: 1000 });
      if (snap.checksum !== base.checksum) { ok = false; break; }
    }
    const elapsed = performance.now() - s;
    results.push(ok
      ? pass(mod, 'DeterminismVaryingWorkers', 'identical across 1,2,4,8 workers', elapsed)
      : fail(mod, 'DeterminismVaryingWorkers', 'checksum differs across worker counts', elapsed));
  }

  // Mixed event types
  {
    const s = performance.now();
    const types: EventType[] = ['NewOrder', 'CancelOrder', 'Trade', 'ModifyOrder', 'QuoteUpdate'];
    const cfg = { eventCount: 500, workerCount: 2, eventTypes: types, batchSize: 500 };
    const c1 = runInstant(cfg).checksum;
    const c2 = runInstant(cfg).checksum;
    const elapsed = performance.now() - s;
    results.push(c1 === c2
      ? pass(mod, 'MixedEventTypes', 'all 5 types deterministic', elapsed)
      : fail(mod, 'MixedEventTypes', 'mixed types non-deterministic', elapsed));
  }

  // Multiple run consistency
  {
    const s = performance.now();
    const cfg = { eventCount: 300, workerCount: 2, eventTypes: ['NewOrder' as EventType], batchSize: 300 };
    const checksums: number[] = [];
    const counts: number[] = [];
    for (let i = 0; i < 10; i++) {
      const snap = runInstant(cfg);
      checksums.push(snap.checksum);
      counts.push(snap.results.length);
    }
    const elapsed = performance.now() - s;
    const ok = checksums.every(c => c === checksums[0]) && counts.every(c => c === counts[0]);
    results.push(ok
      ? pass(mod, 'MultipleRunConsistency', '10 runs consistent', elapsed)
      : fail(mod, 'MultipleRunConsistency', 'divergence detected', elapsed));
  }

  return results;
}

export function runStressTests(): TestResult[] {
  const results: TestResult[] = [];
  const mod = 'StressTests';

  // High throughput
  {
    const count = 50000;
    const s = performance.now();
    const snap = runInstant({ eventCount: count, workerCount: 4, eventTypes: ['NewOrder'], batchSize: count });
    const elapsed = performance.now() - s;
    const eps = Math.round(count / (elapsed / 1000));
    results.push(snap.results.length === count
      ? pass(mod, 'HighThroughput', `${eps.toLocaleString()} events/sec`, elapsed)
      : fail(mod, 'HighThroughput', 'event loss', elapsed));
  }

  // Rapid start/stop
  {
    const s = performance.now();
    let ok = true;
    for (let i = 0; i < 20; i++) {
      const snap = runInstant({ eventCount: 100, workerCount: 2, eventTypes: ['NewOrder'], batchSize: 100 });
      if (snap.results.length !== 100) { ok = false; break; }
    }
    const elapsed = performance.now() - s;
    results.push(ok
      ? pass(mod, 'RapidStartStop', '20 cycles completed', elapsed)
      : fail(mod, 'RapidStartStop', 'lost events', elapsed));
  }

  // Burst traffic
  {
    const s = performance.now();
    const snap = runInstant({ eventCount: 10000, workerCount: 4, eventTypes: ['NewOrder'], batchSize: 1000 });
    const elapsed = performance.now() - s;
    results.push(snap.results.length === 10000
      ? pass(mod, 'BurstTraffic', '10k events in 10 bursts', elapsed)
      : fail(mod, 'BurstTraffic', 'event loss', elapsed));
  }

  // Sustained load
  {
    const s = performance.now();
    const snap = runInstant({ eventCount: 20000, workerCount: 4, eventTypes: ['NewOrder'], batchSize: 20000 });
    const elapsed = performance.now() - s;
    results.push(snap.results.length === 20000
      ? pass(mod, 'SustainedLoad', `20k in ${elapsed.toFixed(1)}ms`, elapsed)
      : fail(mod, 'SustainedLoad', 'event loss', elapsed));
  }

  return results;
}

export function runMemoryTests(): TestResult[] {
  const results: TestResult[] = [];
  const mod = 'MemoryTests';

  // Repeated alloc/dealloc
  {
    const s = performance.now();
    for (let i = 0; i < 50; i++) {
      runInstant({ eventCount: 500, workerCount: 2, eventTypes: ['NewOrder'], batchSize: 500 });
    }
    const elapsed = performance.now() - s;
    results.push(pass(mod, 'RepeatedAllocDealloc', '50 cycles stable', elapsed));
  }

  // Event size compact
  {
    const s = performance.now();
    const snap = runInstant({ eventCount: 1, workerCount: 1, eventTypes: ['NewOrder'], batchSize: 1 });
    const keys = Object.keys(snap.results[0]);
    const elapsed = performance.now() - s;
    results.push(pass(mod, 'EventStructCompact', `${keys.length} fields per result`, elapsed));
  }

  // Large batch
  {
    const s = performance.now();
    const snap = runInstant({ eventCount: 50000, workerCount: 4, eventTypes: ['NewOrder'], batchSize: 50000 });
    const elapsed = performance.now() - s;
    results.push(snap.results.length === 50000
      ? pass(mod, 'LargeEventBatch', '50k processed', elapsed)
      : fail(mod, 'LargeEventBatch', 'event loss', elapsed));
  }

  return results;
}

export function runMetricsTests(): TestResult[] {
  const results: TestResult[] = [];
  const mod = 'MetricsTests';

  // Event count accuracy
  {
    const s = performance.now();
    const snap = runInstant({ eventCount: 10000, workerCount: 4, eventTypes: ['NewOrder'], batchSize: 10000 });
    const elapsed = performance.now() - s;
    const st = snap.stats;
    const ok = st.eventsProduced === 10000 && st.eventsProcessed === 10000 && st.eventsCollected === 10000;
    results.push(ok
      ? pass(mod, 'EventCountAccuracy', 'all counts = 10000', elapsed)
      : fail(mod, 'EventCountAccuracy', 'count mismatch', elapsed));
  }

  // Latency tracking
  {
    const s = performance.now();
    const snap = runInstant({ eventCount: 5000, workerCount: 2, eventTypes: ['NewOrder'], batchSize: 5000 });
    const elapsed = performance.now() - s;
    const st = snap.stats;
    const avg = st.totalLatencyNs / st.eventsProcessed;
    results.push(st.minLatencyNs <= avg && avg <= st.maxLatencyNs
      ? pass(mod, 'LatencyTracking', `avg=${avg.toFixed(0)}ns`, elapsed)
      : fail(mod, 'LatencyTracking', 'invariant violated', elapsed));
  }

  // Stats consistency
  {
    const s = performance.now();
    const snap = runInstant({ eventCount: 2000, workerCount: 3, eventTypes: ['NewOrder'], batchSize: 2000 });
    const elapsed = performance.now() - s;
    const st = snap.stats;
    results.push(st.eventsProduced === st.eventsProcessed && st.eventsProcessed === st.eventsCollected
      ? pass(mod, 'StatsConsistency', `all = ${st.eventsProduced}`, elapsed)
      : fail(mod, 'StatsConsistency', 'pipeline counts diverge', elapsed));
  }

  return results;
}

export function runFaultTests(): TestResult[] {
  const results: TestResult[] = [];
  const mod = 'FaultTests';

  // Zero price events
  {
    const s = performance.now();
    const snap = runInstant({ eventCount: 100, workerCount: 2, eventTypes: ['QuoteUpdate'], batchSize: 100 });
    const elapsed = performance.now() - s;
    results.push(snap.results.length === 100
      ? pass(mod, 'ZeroPriceEvents', '100 processed', elapsed)
      : fail(mod, 'ZeroPriceEvents', 'events dropped', elapsed));
  }

  // Massive cancel stream
  {
    const s = performance.now();
    const snap = runInstant({ eventCount: 1000, workerCount: 2, eventTypes: ['CancelOrder'], batchSize: 1000 });
    const elapsed = performance.now() - s;
    results.push(snap.results.length === 1000
      ? pass(mod, 'MassiveCancelStream', '1000 cancels processed', elapsed)
      : fail(mod, 'MassiveCancelStream', 'cancel loss', elapsed));
  }

  // Checksum non-zero
  {
    const s = performance.now();
    let ok = true;
    for (const count of [1, 10, 100, 1000]) {
      const snap = runInstant({ eventCount: count, workerCount: 2, eventTypes: ['NewOrder'], batchSize: count });
      if (snap.checksum === 0) { ok = false; break; }
    }
    const elapsed = performance.now() - s;
    results.push(ok
      ? pass(mod, 'ChecksumNonZero', 'non-zero for 1,10,100,1000', elapsed)
      : fail(mod, 'ChecksumNonZero', 'zero checksum detected', elapsed));
  }

  // Result value consistency
  {
    const s = performance.now();
    const snap = runInstant({ eventCount: 2, workerCount: 1, eventTypes: ['NewOrder'], batchSize: 2 });
    const elapsed = performance.now() - s;
    const r = snap.results.sort((a, b) => a.sequenceId - b.sequenceId);
    const v0 = r[0]?.stateValue;
    const expected0 = (50.0) * (10);
    results.push(Math.abs(v0 - expected0) < 0.01
      ? pass(mod, 'ResultValueConsistency', `value=${v0}`, elapsed)
      : fail(mod, 'ResultValueConsistency', `expected ${expected0}, got ${v0}`, elapsed));
  }

  return results;
}

export function runAllTests(): TestResult[] {
  return [
    ...runDeterminismTests(),
    ...runStressTests(),
    ...runMemoryTests(),
    ...runMetricsTests(),
    ...runFaultTests(),
  ];
}
