import { useState } from 'react';
import type { SimulationConfig, EventType } from '../engine/types';

const allEventTypes: EventType[] = ['NewOrder', 'CancelOrder', 'Trade', 'ModifyOrder', 'QuoteUpdate'];

interface ControlsProps {
  onRun: (config: SimulationConfig) => void;
  onStop: () => void;
  running: boolean;
}

export default function Controls({ onRun, onStop, running }: ControlsProps) {
  const [eventCount, setEventCount] = useState(5000);
  const [workerCount, setWorkerCount] = useState(4);
  const [speed, setSpeed] = useState(200);
  const [selectedTypes, setSelectedTypes] = useState<EventType[]>(['NewOrder']);

  function toggleType(t: EventType) {
    setSelectedTypes(prev =>
      prev.includes(t) ? prev.filter(x => x !== t) : [...prev, t]
    );
  }

  function handleRun() {
    const types = selectedTypes.length > 0 ? selectedTypes : ['NewOrder' as EventType];
    onRun({ eventCount, workerCount, eventTypes: types, batchSize: speed });
  }

  return (
    <div className="space-y-5">
      <div className="flex items-center gap-8 flex-wrap">
        <div className="flex-1 min-w-[200px]">
          <label className="text-[12px] text-[#6b6b6b] uppercase tracking-[0.08em] block mb-1.5">
            Events &mdash; {eventCount.toLocaleString()}
          </label>
          <input
            type="range"
            min={100}
            max={50000}
            step={100}
            value={eventCount}
            onChange={e => setEventCount(+e.target.value)}
            className="w-full"
            disabled={running}
          />
        </div>
        <div className="flex-1 min-w-[200px]">
          <label className="text-[12px] text-[#6b6b6b] uppercase tracking-[0.08em] block mb-1.5">
            Workers &mdash; {workerCount}
          </label>
          <input
            type="range"
            min={1}
            max={16}
            step={1}
            value={workerCount}
            onChange={e => setWorkerCount(+e.target.value)}
            className="w-full"
            disabled={running}
          />
        </div>
        <div className="flex-1 min-w-[200px]">
          <label className="text-[12px] text-[#6b6b6b] uppercase tracking-[0.08em] block mb-1.5">
            Batch Size &mdash; {speed}
          </label>
          <input
            type="range"
            min={10}
            max={5000}
            step={10}
            value={speed}
            onChange={e => setSpeed(+e.target.value)}
            className="w-full"
            disabled={running}
          />
        </div>
      </div>

      <div>
        <label className="text-[12px] text-[#6b6b6b] uppercase tracking-[0.08em] block mb-2">
          Event Types
        </label>
        <div className="flex gap-2 flex-wrap">
          {allEventTypes.map(t => (
            <button
              key={t}
              onClick={() => toggleType(t)}
              disabled={running}
              className={`px-3 py-1.5 text-[13px] border transition-colors cursor-pointer ${
                selectedTypes.includes(t)
                  ? 'bg-[#1a1a1a] text-[#f5f0eb] border-[#1a1a1a]'
                  : 'bg-transparent text-[#1a1a1a] border-[#d4d0cb] hover:border-[#1a1a1a]'
              } disabled:opacity-50`}
            >
              {t}
            </button>
          ))}
        </div>
      </div>

      <div className="flex gap-3">
        {!running ? (
          <button
            onClick={handleRun}
            className="bg-[#1a1a1a] text-[#f5f0eb] px-6 py-2 text-[13px] tracking-[0.04em] border border-[#1a1a1a] hover:bg-[#333] transition-colors cursor-pointer"
          >
            Run Simulation
          </button>
        ) : (
          <button
            onClick={onStop}
            className="bg-transparent text-[#1a1a1a] px-6 py-2 text-[13px] tracking-[0.04em] border border-[#d4d0cb] hover:border-[#1a1a1a] transition-colors cursor-pointer"
          >
            Stop
          </button>
        )}
      </div>
    </div>
  );
}
