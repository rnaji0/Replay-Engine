import type {
  Event, EventType, SequencedEvent, ProcessedResult,
  EngineStats, SimulationConfig, SimulationSnapshot,
} from './types';

function createEvent(type: EventType, i: number): Event {
  const base: Event = {
    type,
    timestamp: 1000 + i * 10,
    orderId: 100 + i,
    price: 50.0 + (i % 100) * 0.5,
    quantity: 10 + (i % 50),
    action: i % 2 === 0 ? 'B' : 'S',
    open: true,
  };

  switch (type) {
    case 'CancelOrder':
      return { ...base, open: false, price: 0, quantity: 0 };
    case 'Trade':
      return base;
    case 'ModifyOrder':
      return { ...base, price: base.price * 1.05, quantity: base.quantity + 5 };
    case 'QuoteUpdate':
      return { ...base, quantity: 0 };
    default:
      return base;
  }
}

function processEvent(se: SequencedEvent, workerId: number): ProcessedResult {
  const e = se.event;
  let stateValue = 0;

  switch (e.type) {
    case 'NewOrder':
      stateValue = e.price * e.quantity;
      break;
    case 'CancelOrder':
      stateValue = -1;
      break;
    case 'Trade':
      stateValue = e.price * e.quantity * (e.action === 'B' ? 1 : -1);
      break;
    case 'ModifyOrder':
      stateValue = e.price * e.quantity;
      break;
    case 'QuoteUpdate':
      stateValue = e.price;
      break;
  }

  return {
    sequenceId: se.sequenceId,
    orderId: e.orderId,
    stateValue,
    processedTimestamp: performance.now(),
    workerId,
    latencyNs: 0,
  };
}

function computeChecksum(results: ProcessedResult[]): number {
  let checksum = 0;
  for (const r of results) {
    let hash = (r.sequenceId * 2654435761) >>> 0;
    hash ^= Math.round(r.stateValue * 1000);
    hash ^= (r.orderId * 40503) >>> 0;
    checksum ^= hash;
  }
  return checksum >>> 0;
}

export function generateEvents(config: SimulationConfig): Event[] {
  const events: Event[] = [];
  const types = config.eventTypes.length > 0 ? config.eventTypes : ['NewOrder' as EventType];
  for (let i = 0; i < config.eventCount; i++) {
    const type = types[i % types.length];
    events.push(createEvent(type, i));
  }
  return events;
}

export interface SimulationCallbacks {
  onSnapshot: (snapshot: SimulationSnapshot) => void;
  onComplete: (snapshot: SimulationSnapshot) => void;
}

export function runSimulation(
  config: SimulationConfig,
  callbacks: SimulationCallbacks,
): { cancel: () => void } {
  let cancelled = false;
  const events = generateEvents(config);

  const inputQueue: Event[] = [...events];
  const workQueue: SequencedEvent[] = [];
  const outputQueue: ProcessedResult[] = [];
  const collectedResults: ProcessedResult[] = [];

  const stats: EngineStats = {
    eventsProduced: 0,
    eventsProcessed: 0,
    eventsCollected: 0,
    totalLatencyNs: 0,
    minLatencyNs: Infinity,
    maxLatencyNs: 0,
    startTime: performance.now(),
    endTime: 0,
  };

  let sequenceId = 1;
  let phase: SimulationSnapshot['phase'] = 'producing';
  let currentEvent: SequencedEvent | null = null;

  const batchSize = config.batchSize;

  function snapshot(): SimulationSnapshot {
    return {
      inputQueueSize: inputQueue.length,
      workQueueSize: workQueue.length,
      outputQueueSize: outputQueue.length,
      stats: { ...stats },
      results: [...collectedResults],
      checksum: computeChecksum(collectedResults),
      phase,
      currentEvent,
    };
  }

  function tick() {
    if (cancelled) return;

    // produce batch
    if (inputQueue.length > 0) {
      phase = 'producing';
      const count = Math.min(batchSize, inputQueue.length);
      for (let i = 0; i < count; i++) {
        const event = inputQueue.shift()!;
        const se: SequencedEvent = { event, sequenceId: sequenceId++ };
        workQueue.push(se);
        stats.eventsProduced++;
      }
    }

    // process batch
    if (workQueue.length > 0) {
      phase = 'processing';
      const count = Math.min(batchSize, workQueue.length);
      for (let i = 0; i < count; i++) {
        const se = workQueue.shift()!;
        currentEvent = se;
        const workerId = (se.sequenceId - 1) % config.workerCount;
        const startTs = performance.now();
        const result = processEvent(se, workerId);
        const endTs = performance.now();
        const latency = (endTs - startTs) * 1_000_000;
        result.latencyNs = latency;

        stats.totalLatencyNs += latency;
        stats.minLatencyNs = Math.min(stats.minLatencyNs, latency);
        stats.maxLatencyNs = Math.max(stats.maxLatencyNs, latency);
        stats.eventsProcessed++;

        outputQueue.push(result);
      }
    }

    // collect batch
    if (outputQueue.length > 0) {
      phase = 'collecting';
      const count = Math.min(batchSize, outputQueue.length);
      for (let i = 0; i < count; i++) {
        collectedResults.push(outputQueue.shift()!);
        stats.eventsCollected++;
      }
    }

    callbacks.onSnapshot(snapshot());

    if (inputQueue.length === 0 && workQueue.length === 0 && outputQueue.length === 0) {
      phase = 'complete';
      stats.endTime = performance.now();
      currentEvent = null;
      callbacks.onComplete(snapshot());
      return;
    }

    requestAnimationFrame(tick);
  }

  requestAnimationFrame(tick);

  return {
    cancel: () => { cancelled = true; },
  };
}

export function runInstant(config: SimulationConfig): SimulationSnapshot {
  const events = generateEvents(config);
  const results: ProcessedResult[] = [];
  let sequenceId = 1;

  const stats: EngineStats = {
    eventsProduced: events.length,
    eventsProcessed: 0,
    eventsCollected: 0,
    totalLatencyNs: 0,
    minLatencyNs: Infinity,
    maxLatencyNs: 0,
    startTime: performance.now(),
    endTime: 0,
  };

  for (const event of events) {
    const se: SequencedEvent = { event, sequenceId: sequenceId++ };
    const workerId = (se.sequenceId - 1) % config.workerCount;
    const startTs = performance.now();
    const result = processEvent(se, workerId);
    const endTs = performance.now();
    const latency = (endTs - startTs) * 1_000_000;
    result.latencyNs = latency;

    stats.totalLatencyNs += latency;
    stats.minLatencyNs = Math.min(stats.minLatencyNs, latency);
    stats.maxLatencyNs = Math.max(stats.maxLatencyNs, latency);
    stats.eventsProcessed++;
    stats.eventsCollected++;
    results.push(result);
  }

  stats.endTime = performance.now();

  return {
    inputQueueSize: 0,
    workQueueSize: 0,
    outputQueueSize: 0,
    stats,
    results,
    checksum: computeChecksum(results),
    phase: 'complete',
    currentEvent: null,
  };
}

export { computeChecksum };
