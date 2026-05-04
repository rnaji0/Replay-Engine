import { useMemo } from 'react';
import type { ProcessedResult } from '../engine/types';

export default function LatencyHistogram({ results }: { results: ProcessedResult[] }) {
  const bins = useMemo(() => {
    if (results.length === 0) return [];

    const latencies = results.map(r => r.latencyNs).filter(l => l > 0);
    if (latencies.length === 0) {
      return [{ lo: 0, hi: 1, count: results.length, pct: 100 }];
    }

    const min = Math.min(...latencies);
    const max = Math.max(...latencies);
    const range = max - min || 1;
    const binCount = 8;
    const binWidth = range / binCount;

    const counts = new Array(binCount).fill(0);
    for (const l of latencies) {
      let bin = Math.floor((l - min) / binWidth);
      if (bin >= binCount) bin = binCount - 1;
      counts[bin]++;
    }

    const maxCount = Math.max(...counts, 1);
    return counts
      .map((c, i) => ({
        lo: min + i * binWidth,
        hi: min + (i + 1) * binWidth,
        count: c,
        pct: (c / maxCount) * 100,
      }))
      .filter(b => b.count > 0);
  }, [results]);

  if (results.length === 0) return null;

  return (
    <div>
      {bins.map((b, i) => (
        <div key={i} className="flex items-center gap-4 py-2">
          <div className="w-16 text-right text-[12px] text-[#a3a3a3] tabular-nums shrink-0">
            {b.lo < 1000 ? `${b.lo.toFixed(0)}` : `${(b.lo / 1000).toFixed(1)}k`}
          </div>
          <div className="flex-1 h-1.5 bg-[#f5f5f5] rounded-full relative overflow-hidden">
            <div
              className="h-full bg-[#0d0d0d] opacity-50 rounded-full"
              style={{ width: `${b.pct}%` }}
            />
          </div>
          <div className="w-12 text-right text-[12px] text-[#a3a3a3] tabular-nums shrink-0">{b.count.toLocaleString()}</div>
        </div>
      ))}
      <p className="text-[12px] text-[#d4d4d4] mt-3">nanoseconds</p>
    </div>
  );
}
