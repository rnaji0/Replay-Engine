import type { SimulationSnapshot } from '../engine/types';

const phases = ['producing', 'processing', 'collecting', 'complete'] as const;

function QueueBox({ label, count, active }: { label: string; count: number; active: boolean }) {
  return (
    <div className={`rounded-2xl px-5 py-4 min-w-[130px] text-center transition-all duration-200 ${
      active
        ? 'bg-[#0d0d0d] text-white shadow-lg'
        : 'bg-[#f7f7f7] text-[#0d0d0d] border border-[#ebebeb]'
    }`}>
      <div className={`text-[11px] tracking-[0.02em] mb-1 ${active ? 'text-white/50' : 'text-[#737373]'}`}>
        {label}
      </div>
      <div className="text-xl font-medium tabular-nums">{count.toLocaleString()}</div>
    </div>
  );
}

function Arrow() {
  return (
    <svg width="20" height="20" viewBox="0 0 20 20" className="text-[#d4d4d4] shrink-0 mx-1">
      <path d="M7 4l6 6-6 6" stroke="currentColor" strokeWidth="1.5" fill="none" strokeLinecap="round" strokeLinejoin="round" />
    </svg>
  );
}

export default function PipelineView({ snapshot }: { snapshot: SimulationSnapshot | null }) {
  if (!snapshot) {
    return (
      <div className="rounded-2xl bg-[#f7f7f7] border border-[#ebebeb] px-6 py-10 text-center text-[#737373] text-[14px]">
        Configure and run a simulation to see the pipeline.
      </div>
    );
  }

  const p = snapshot.phase;

  return (
    <div>
      <div className="flex items-center justify-center gap-1 flex-wrap">
        <QueueBox label="Input Queue" count={snapshot.inputQueueSize} active={p === 'producing'} />
        <Arrow />
        <QueueBox label="Work Queue" count={snapshot.workQueueSize} active={p === 'processing'} />
        <Arrow />
        <QueueBox label="Output Queue" count={snapshot.outputQueueSize} active={p === 'collecting'} />
        <Arrow />
        <QueueBox label="Collected" count={snapshot.stats.eventsCollected} active={p === 'complete'} />
      </div>

      <div className="mt-5 flex items-center justify-center gap-6 text-[12px] text-[#a3a3a3]">
        {phases.map(ph => (
          <span key={ph} className={`tracking-[0.01em] transition-colors ${
            p === ph ? 'text-[#0d0d0d] font-medium' : ''
          }`}>
            {ph === 'complete' ? 'Done' : ph.charAt(0).toUpperCase() + ph.slice(1)}
            {p === ph && ph !== 'complete' && (
              <span className="inline-block ml-1 animate-pulse">&bull;</span>
            )}
          </span>
        ))}
      </div>

      {snapshot.currentEvent && p !== 'complete' && (
        <div className="mt-3 text-center text-[12px] text-[#a3a3a3] font-mono">
          seq #{snapshot.currentEvent.sequenceId} &mdash; {snapshot.currentEvent.event.type}
          {' '}&mdash; ${snapshot.currentEvent.event.price.toFixed(2)}
          {' '}&times; {snapshot.currentEvent.event.quantity}
        </div>
      )}
    </div>
  );
}
