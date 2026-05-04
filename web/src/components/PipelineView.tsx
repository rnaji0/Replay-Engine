import type { SimulationSnapshot } from '../engine/types';

function QueueBox({ label, count, active }: { label: string; count: number; active: boolean }) {
  return (
    <div className={`flex-1 rounded-xl px-4 py-4 text-center transition-all duration-300 ${
      active
        ? 'bg-[#0d0d0d] text-white'
        : 'bg-[#fafafa] text-[#0d0d0d] border border-[#f0f0f0]'
    }`}>
      <div className={`text-[11px] tracking-wide uppercase mb-1 ${
        active ? 'text-white/50' : 'text-[#a3a3a3]'
      }`}>
        {label}
      </div>
      <div className="text-[20px] font-medium tabular-nums tracking-[-0.02em]">
        {count.toLocaleString()}
      </div>
    </div>
  );
}

export default function PipelineView({ snapshot }: { snapshot: SimulationSnapshot | null }) {
  if (!snapshot) {
    return (
      <div className="rounded-xl bg-[#fafafa] border border-[#f0f0f0] px-6 py-10 text-center text-[#a3a3a3] text-[13px]">
        Run a simulation to see the pipeline.
      </div>
    );
  }

  const p = snapshot.phase;

  return (
    <div>
      <div className="grid grid-cols-4 gap-2">
        <QueueBox label="Input" count={snapshot.inputQueueSize} active={p === 'producing'} />
        <QueueBox label="Processing" count={snapshot.workQueueSize} active={p === 'processing'} />
        <QueueBox label="Output" count={snapshot.outputQueueSize} active={p === 'collecting'} />
        <QueueBox label="Collected" count={snapshot.stats.eventsCollected} active={p === 'complete'} />
      </div>

      {snapshot.currentEvent && p !== 'complete' && (
        <div className="mt-3 text-center text-[11px] text-[#a3a3a3] font-mono tabular-nums">
          #{snapshot.currentEvent.sequenceId} &middot; {snapshot.currentEvent.event.type}
        </div>
      )}
    </div>
  );
}
