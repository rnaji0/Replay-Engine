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
    <div className="space-y-6">
      <div className="flex items-center gap-10 flex-wrap">
        <div className="flex-1 min-w-[200px]">
          <label className="text-[13px] text-[#737373] block mb-2">
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
          <label className="text-[13px] text-[#737373] block mb-2">
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
          <label className="text-[13px] text-[#737373] block mb-2">
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
        <label className="text-[13px] text-[#737373] block mb-3">
          Event Types
        </label>
        <div className="flex gap-2 flex-wrap">
          {allEventTypes.map(t => (
            <button
              key={t}
              onClick={() => toggleType(t)}
              disabled={running}
              className={`px-4 py-1.5 text-[13px] rounded-full border transition-all cursor-pointer ${
                selectedTypes.includes(t)
                  ? 'bg-[#0d0d0d] text-white border-[#0d0d0d]'
                  : 'bg-white text-[#0d0d0d] border-[#e5e5e5] hover:border-[#0d0d0d]'
              } disabled:opacity-40 disabled:cursor-default`}
            >
              {t}
            </button>
          ))}
        </div>
      </div>

      <div className="flex gap-3 pt-1">
        {!running ? (
          <button
            onClick={handleRun}
            className="bg-[#0d0d0d] text-white px-7 py-2.5 text-[14px] font-medium rounded-full border border-[#0d0d0d] hover:bg-[#2a2a2a] transition-colors cursor-pointer"
          >
            Run Simulation
          </button>
        ) : (
          <button
            onClick={onStop}
            className="bg-white text-[#0d0d0d] px-7 py-2.5 text-[14px] font-medium rounded-full border border-[#e5e5e5] hover:border-[#0d0d0d] transition-colors cursor-pointer"
          >
            Stop
          </button>
        )}
      </div>
    </div>
  );
}
