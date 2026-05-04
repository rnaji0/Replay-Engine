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
    <div className="max-w-[760px] mx-auto px-6 pt-24 pb-16">
      <Header />

      <section className="mb-20">
        <SectionLabel>Simulation</SectionLabel>
        <Controls onRun={handleRun} onStop={handleStop} running={running} />
      </section>

      <section className="mb-20">
        <SectionLabel>Pipeline</SectionLabel>
        <PipelineView snapshot={snapshot} />
      </section>

      {snapshot && (
        <section className="mb-20">
          <SectionLabel>Metrics</SectionLabel>
          <MetricsPanel snapshot={snapshot} />
        </section>
      )}

      {snapshot && snapshot.results.length > 0 && (
        <section className="mb-20">
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-16">
            <div>
              <SectionLabel>Worker Distribution</SectionLabel>
              <ThroughputChart results={snapshot.results} workerCount={workerCount} />
            </div>
            <div>
              <SectionLabel>Latency</SectionLabel>
              <LatencyHistogram results={snapshot.results} />
            </div>
          </div>
        </section>
      )}

      <hr className="border-[#f0f0f0] my-20" />

      <section className="mb-20">
        <DeterminismMatrix />
      </section>

      <hr className="border-[#f0f0f0] my-20" />

      <section className="mb-24">
        <TestRunner />
      </section>

      <footer className="text-[13px] text-[#d4d4d4] text-center pb-12">
        Replay Engine
      </footer>
    </div>
  );
}
