import type { SimulationSnapshot } from '../engine/types';

function Stat({ label, value }: { label: string; value: string }) {
  return (
    <div>
      <div className="text-[12px] text-[#a3a3a3] mb-1">{label}</div>
      <div className="text-[18px] font-medium text-[#0d0d0d] tabular-nums">{value}</div>
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
    <div className="grid grid-cols-2 sm:grid-cols-3 lg:grid-cols-6 gap-6">
      <Stat label="Produced" value={s.eventsProduced.toLocaleString()} />
      <Stat label="Processed" value={s.eventsProcessed.toLocaleString()} />
      <Stat label="Collected" value={s.eventsCollected.toLocaleString()} />
      <Stat label="Throughput" value={`${throughput.toLocaleString()}/s`} />
      <Stat label="Avg Latency" value={`${avgLatency.toFixed(0)}ns`} />
      <Stat label="Checksum" value={snapshot.checksum.toString(16).toUpperCase()} />
    </div>
  );
}
