import { useMemo } from 'react';
import type { ProcessedResult } from '../engine/types';

export default function LatencyHistogram({ results }: { results: ProcessedResult[] }) {
  const bins = useMemo(() => {
    if (results.length === 0) return [];

    const latencies = results.map(r => r.latencyNs);
    const min = Math.min(...latencies);
    const max = Math.max(...latencies);
    const range = max - min || 1;
    const binCount = 12;
    const binWidth = range / binCount;

    const counts = new Array(binCount).fill(0);
    for (const l of latencies) {
      let bin = Math.floor((l - min) / binWidth);
      if (bin >= binCount) bin = binCount - 1;
      counts[bin]++;
    }

    const maxCount = Math.max(...counts, 1);
    return counts.map((c, i) => ({
      lo: min + i * binWidth,
      hi: min + (i + 1) * binWidth,
      count: c,
      pct: (c / maxCount) * 100,
    }));
  }, [results]);

  if (results.length === 0) return null;

  return (
    <div className="space-y-1">
      {bins.map((b, i) => (
        <div key={i} className="flex items-center gap-2">
          <div className="w-20 text-right text-[11px] text-[#6b6b6b] font-mono shrink-0">
            {b.lo.toFixed(0)}
          </div>
          <div className="flex-1 h-4 bg-[#eae5df] relative">
            <div
              className="h-full bg-[#1a1a1a] opacity-70"
              style={{ width: `${b.pct}%` }}
            />
          </div>
          <div className="w-12 text-[11px] text-[#6b6b6b] font-mono shrink-0">{b.count}</div>
        </div>
      ))}
      <div className="text-[11px] text-[#6b6b6b] mt-1">Latency distribution (ns)</div>
    </div>
  );
}
