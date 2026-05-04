import type { SimulationSnapshot } from '../engine/types';

function QueueBox({ label, count, active }: { label: string; count: number; active: boolean }) {
  return (
    <div className={`rounded-2xl px-6 py-5 min-w-[140px] text-center transition-all duration-300 ${
      active
        ? 'bg-[#0d0d0d] text-white'
        : 'bg-[#fafafa] text-[#0d0d0d]'
    }`}>
      <div className={`text-[12px] mb-1.5 ${active ? 'text-white/40' : 'text-[#a3a3a3]'}`}>
        {label}
      </div>
      <div className="text-[22px] font-medium tabular-nums tracking-[-0.02em]">{count.toLocaleString()}</div>
    </div>
  );
}

function Arrow() {
  return (
    <svg width="16" height="16" viewBox="0 0 16 16" className="text-[#d4d4d4] shrink-0 mx-2">
      <path d="M6 3l5 5-5 5" stroke="currentColor" strokeWidth="1.2" fill="none" strokeLinecap="round" strokeLinejoin="round" />
    </svg>
  );
}

export default function PipelineView({ snapshot }: { snapshot: SimulationSnapshot | null }) {
  if (!snapshot) {
    return (
      <div className="rounded-2xl bg-[#fafafa] px-6 py-12 text-center text-[#a3a3a3] text-[14px]">
        Run a simulation to see the pipeline.
      </div>
    );
  }

  const p = snapshot.phase;

  return (
    <div>
      <div className="flex items-center justify-center gap-0 flex-wrap py-2">
        <QueueBox label="Input" count={snapshot.inputQueueSize} active={p === 'producing'} />
        <Arrow />
        <QueueBox label="Processing" count={snapshot.workQueueSize} active={p === 'processing'} />
        <Arrow />
        <QueueBox label="Output" count={snapshot.outputQueueSize} active={p === 'collecting'} />
        <Arrow />
        <QueueBox label="Collected" count={snapshot.stats.eventsCollected} active={p === 'complete'} />
      </div>

      {snapshot.currentEvent && p !== 'complete' && (
        <div className="mt-4 text-center text-[12px] text-[#a3a3a3] font-mono tabular-nums">
          #{snapshot.currentEvent.sequenceId} &middot; {snapshot.currentEvent.event.type}
        </div>
      )}
    </div>
  );
}
