export type EventType = 'NewOrder' | 'CancelOrder' | 'Trade' | 'ModifyOrder' | 'QuoteUpdate';

export interface Event {
  type: EventType;
  timestamp: number;
  orderId: number;
  price: number;
  quantity: number;
  action: 'B' | 'S';
  open: boolean;
}

export interface SequencedEvent {
  event: Event;
  sequenceId: number;
}

export interface ProcessedResult {
  sequenceId: number;
  orderId: number;
  stateValue: number;
  processedTimestamp: number;
  workerId: number;
  latencyNs: number;
}

export interface EngineStats {
  eventsProduced: number;
  eventsProcessed: number;
  eventsCollected: number;
  totalLatencyNs: number;
  minLatencyNs: number;
  maxLatencyNs: number;
  startTime: number;
  endTime: number;
}

export interface SimulationConfig {
  eventCount: number;
  workerCount: number;
  eventTypes: EventType[];
  batchSize: number;
}

export interface SimulationSnapshot {
  inputQueueSize: number;
  workQueueSize: number;
  outputQueueSize: number;
  stats: EngineStats;
  results: ProcessedResult[];
  checksum: number;
  phase: 'idle' | 'producing' | 'processing' | 'collecting' | 'complete';
  currentEvent: SequencedEvent | null;
}

export type TestStatus = 'pending' | 'running' | 'passed' | 'failed';

export interface TestResult {
  name: string;
  module: string;
  status: TestStatus;
  message: string;
  durationMs: number;
}
