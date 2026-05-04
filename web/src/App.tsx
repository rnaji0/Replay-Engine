import { useState, useRef, useCallback } from 'react';
import type { SimulationConfig, SimulationSnapshot } from './engine/types';
import { runSimulation } from './engine/ReplayEngine';
import Header from './components/Header';
import SectionLabel from './components/SectionLabel';
import Controls from './components/Controls';
import PipelineView from './components/PipelineView';
import MetricsPanel from './components/MetricsPanel';
import ThroughputChart from './components/ThroughputChart';
import LatencyHistogram from './components/LatencyHistogram';
import DeterminismMatrix from './components/DeterminismMatrix';
import TestRunner from './components/TestRunner';

export default function App() {
  const [snapshot, setSnapshot] = useState<SimulationSnapshot | null>(null);
  const [running, setRunning] = useState(false);
  const [workerCount, setWorkerCount] = useState(4);
  const cancelRef = useRef<(() => void) | null>(null);

  const handleRun = useCallback((config: SimulationConfig) => {
    if (cancelRef.current) cancelRef.current();
    setRunning(true);
    setWorkerCount(config.workerCount);
    setSnapshot(null);

    const sim = runSimulation(config, {
      onSnapshot: (snap) => setSnapshot(snap),
      onComplete: (snap) => {
        setSnapshot(snap);
        setRunning(false);
      },
    });

    cancelRef.current = sim.cancel;
  }, []);

  const handleStop = useCallback(() => {
    if (cancelRef.current) {
      cancelRef.current();
      cancelRef.current = null;
    }
    setRunning(false);
  }, []);

  return (
    <div className="max-w-[900px] mx-auto px-6 py-16">
      <Header />

      {/* Simulation Controls */}
      <section className="mb-12">
        <SectionLabel>Simulation</SectionLabel>
        <Controls onRun={handleRun} onStop={handleStop} running={running} />
      </section>

      {/* Pipeline Visualization */}
      <section className="mb-12">
        <SectionLabel>Pipeline</SectionLabel>
        <PipelineView snapshot={snapshot} />
      </section>

      {/* Live Metrics */}
      {snapshot && (
        <section className="mb-12">
          <SectionLabel>Metrics</SectionLabel>
          <MetricsPanel snapshot={snapshot} />
        </section>
      )}

      {/* Worker Distribution & Latency */}
      {snapshot && snapshot.results.length > 0 && (
        <section className="mb-12">
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-8">
            <div>
              <SectionLabel>Worker Distribution</SectionLabel>
              <ThroughputChart results={snapshot.results} workerCount={workerCount} />
            </div>
            <div>
              <SectionLabel>Latency Histogram</SectionLabel>
              <LatencyHistogram results={snapshot.results} />
            </div>
          </div>
        </section>
      )}

      {/* Divider */}
      <div className="border-t border-[#d4d0cb] my-12" />

      {/* Determinism Verification */}
      <section className="mb-12">
        <DeterminismMatrix />
      </section>

      {/* Divider */}
      <div className="border-t border-[#d4d0cb] my-12" />

      {/* Test Suite */}
      <section className="mb-16">
        <TestRunner />
      </section>

      {/* Footer */}
      <footer className="text-[12px] text-[#c4bfb8] pb-8">
        Replay Engine &mdash; C++20, lock-free MPMC queues, deterministic multi-threaded replay.
      </footer>
    </div>
  );
}
