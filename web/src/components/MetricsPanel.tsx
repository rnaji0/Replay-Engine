import type { SimulationSnapshot } from '../engine/types';

function Stat({ label, value, sub }: { label: string; value: string; sub?: string }) {
  return (
    <div className="py-5 border-b border-[#f0f0f0] last:border-b-0 flex items-baseline justify-between">
      <span className="text-[14px] text-[#737373]">{label}</span>
      <div className="text-right">
        <span className="text-[15px] font-medium text-[#0d0d0d] tabular-nums">{value}</span>
        {sub && <span className="text-[13px] text-[#a3a3a3] ml-1">{sub}</span>}
      </div>
    </div>
  );
}

export default function MetricsPanel({ snapshot }: { snapshot: SimulationSnapshot | null }) {
  if (!snapshot) return null;

  const s = snapshot.stats;
  const elapsed = s.endTime > 0 ? s.endTime - s.startTime : performance.now() - s.startTime;
  const throughput = elapsed > 0 ? Math.round(s.eventsProcessed / (elapsed / 1000)) : 0;
  const avgLatency = s.eventsProcessed > 0 ? (s.totalLatencyNs / s.eventsProcessed) : 0;

  return (
    <div className="grid grid-cols-1 sm:grid-cols-2 gap-x-16">
      <div>
        <Stat label="Events Produced" value={s.eventsProduced.toLocaleString()} />
        <Stat label="Events Processed" value={s.eventsProcessed.toLocaleString()} />
        <Stat label="Events Collected" value={s.eventsCollected.toLocaleString()} />
      </div>
      <div>
        <Stat label="Throughput" value={throughput.toLocaleString()} sub="events/s" />
        <Stat label="Avg Latency" value={avgLatency.toFixed(0)} sub="ns" />
        <Stat label="State Checksum" value={snapshot.checksum.toString(16).toUpperCase()} />
      </div>
    </div>
  );
}
