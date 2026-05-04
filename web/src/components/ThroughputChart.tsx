import { useMemo } from 'react';
import type { ProcessedResult } from '../engine/types';

function Bar({ label, value, maxValue }: { label: string; value: number; maxValue: number }) {
  const pct = maxValue > 0 ? (value / maxValue) * 100 : 0;
  return (
    <div className="flex items-center gap-3">
      <div className="w-20 text-right text-[13px] text-[#737373] shrink-0">{label}</div>
      <div className="flex-1 h-6 bg-[#f7f7f7] rounded-full relative overflow-hidden">
        <div
          className="h-full bg-[#0d0d0d] rounded-full transition-all duration-500"
          style={{ width: `${pct}%` }}
        />
      </div>
      <div className="w-20 text-[13px] text-[#737373] font-mono shrink-0 tabular-nums">{value.toLocaleString()}</div>
    </div>
  );
}

export default function ThroughputChart({ results, workerCount }: { results: ProcessedResult[]; workerCount: number }) {
  const workerDistribution = useMemo(() => {
    const dist = new Map<number, number>();
    for (let i = 0; i < workerCount; i++) dist.set(i, 0);
    for (const r of results) {
      dist.set(r.workerId, (dist.get(r.workerId) || 0) + 1);
    }
    return dist;
  }, [results, workerCount]);

  const maxCount = Math.max(...workerDistribution.values(), 1);

  if (results.length === 0) return null;

  return (
    <div className="space-y-2.5">
      {Array.from(workerDistribution.entries())
        .sort((a, b) => a[0] - b[0])
        .map(([wid, count]) => (
          <Bar key={wid} label={`Worker ${wid}`} value={count} maxValue={maxCount} />
        ))}
    </div>
  );
}
