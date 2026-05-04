import type { SimulationSnapshot } from '../engine/types';

const phases = ['producing', 'processing', 'collecting', 'complete'] as const;

function QueueBox({ label, count, active }: { label: string; count: number; active: boolean }) {
  return (
    <div className={`border px-4 py-3 min-w-[120px] text-center transition-colors duration-200 ${
      active ? 'border-[#1a1a1a] bg-[#1a1a1a] text-[#f5f0eb]' : 'border-[#d4d0cb] bg-[#eae5df]'
    }`}>
      <div className="text-[11px] uppercase tracking-[0.08em] opacity-60 mb-1">{label}</div>
      <div className="font-sans font-medium text-xl">{count.toLocaleString()}</div>
    </div>
  );
}

function Arrow() {
  return <div className="text-[#c4bfb8] text-lg self-center px-2 select-none">&rarr;</div>;
}

export default function PipelineView({ snapshot }: { snapshot: SimulationSnapshot | null }) {
  if (!snapshot) {
    return (
      <div className="border border-[#d4d0cb] bg-[#eae5df] px-6 py-8 text-center text-[#6b6b6b] text-sm">
        Configure and run a simulation to see the pipeline.
      </div>
    );
  }

  const p = snapshot.phase;

  return (
    <div>
      <div className="flex items-center justify-center gap-0 flex-wrap">
        <QueueBox label="Input Queue" count={snapshot.inputQueueSize} active={p === 'producing'} />
        <Arrow />
        <QueueBox label="Work Queue" count={snapshot.workQueueSize} active={p === 'processing'} />
        <Arrow />
        <QueueBox label="Output Queue" count={snapshot.outputQueueSize} active={p === 'collecting'} />
        <Arrow />
        <QueueBox label="Collected" count={snapshot.stats.eventsCollected} active={p === 'complete'} />
      </div>

      <div className="mt-4 flex items-center justify-center gap-6 text-[12px] text-[#6b6b6b]">
        {phases.map(ph => (
          <span key={ph} className={`uppercase tracking-[0.08em] transition-colors ${
            p === ph ? 'text-[#1a1a1a] font-medium' : ''
          }`}>
            {ph === 'complete' ? 'done' : ph}
            {p === ph && ph !== 'complete' && (
              <span className="inline-block ml-1 animate-pulse">&bull;</span>
            )}
          </span>
        ))}
      </div>

      {snapshot.currentEvent && p !== 'complete' && (
        <div className="mt-3 text-center text-[12px] text-[#6b6b6b] font-mono">
          seq #{snapshot.currentEvent.sequenceId} &mdash; {snapshot.currentEvent.event.type}
          {' '}&mdash; ${snapshot.currentEvent.event.price.toFixed(2)}
          {' '}&times; {snapshot.currentEvent.event.quantity}
        </div>
      )}
    </div>
  );
}
