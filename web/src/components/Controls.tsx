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
      <div className="flex items-center gap-10 flex-wrap">
        <div className="flex-1 min-w-[180px]">
          <label className="text-[13px] text-[#a3a3a3] block mb-2">
            Events &middot; {eventCount.toLocaleString()}
          </label>
          <input
            type="range" min={100} max={50000} step={100}
            value={eventCount} onChange={e => setEventCount(+e.target.value)}
            className="w-full" disabled={running}
          />
        </div>
        <div className="flex-1 min-w-[180px]">
          <label className="text-[13px] text-[#a3a3a3] block mb-2">
            Workers &middot; {workerCount}
          </label>
          <input
            type="range" min={1} max={16} step={1}
            value={workerCount} onChange={e => setWorkerCount(+e.target.value)}
            className="w-full" disabled={running}
          />
        </div>
        <div className="flex-1 min-w-[180px]">
          <label className="text-[13px] text-[#a3a3a3] block mb-2">
            Batch &middot; {speed}
          </label>
          <input
            type="range" min={10} max={5000} step={10}
            value={speed} onChange={e => setSpeed(+e.target.value)}
            className="w-full" disabled={running}
          />
        </div>
      </div>

      <div className="flex items-center gap-4 flex-wrap">
        <div className="flex gap-1.5 flex-wrap">
          {allEventTypes.map(t => (
            <button
              key={t}
              onClick={() => toggleType(t)}
              disabled={running}
              className={`px-3 py-1 text-[12px] rounded-full border transition-all cursor-pointer ${
                selectedTypes.includes(t)
                  ? 'bg-[#0d0d0d] text-white border-[#0d0d0d]'
                  : 'bg-white text-[#737373] border-[#e5e5e5] hover:text-[#0d0d0d] hover:border-[#b5b5b5]'
              } disabled:opacity-40 disabled:cursor-default`}
            >
              {t}
            </button>
          ))}
        </div>

        <div className="ml-auto">
          {!running ? (
            <button
              onClick={handleRun}
              className="bg-[#0d0d0d] text-white px-5 py-1.5 text-[13px] font-medium rounded-full hover:bg-[#333] transition-colors cursor-pointer"
            >
              Run
            </button>
          ) : (
            <button
              onClick={onStop}
              className="bg-white text-[#0d0d0d] px-5 py-1.5 text-[13px] font-medium rounded-full border border-[#e5e5e5] hover:border-[#0d0d0d] transition-colors cursor-pointer"
            >
              Stop
            </button>
          )}
        </div>
      </div>
    </div>
  );
}
