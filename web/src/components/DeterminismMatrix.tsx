import { useState } from 'react';
import { runInstant } from '../engine/ReplayEngine';
import type { EventType } from '../engine/types';
import SectionLabel from './SectionLabel';

interface Row {
  workers: number;
  checksums: number[];
  allMatch: boolean;
}

export default function DeterminismMatrix() {
  const [rows, setRows] = useState<Row[]>([]);
  const [running, setRunning] = useState(false);

  function run() {
    setRunning(true);
    const newRows: Row[] = [];
    const runsPerConfig = 3;

    for (const workers of [1, 2, 4, 8]) {
      const checksums: number[] = [];
      for (let r = 0; r < runsPerConfig; r++) {
        const snap = runInstant({
          eventCount: 5000,
          workerCount: workers,
          eventTypes: ['NewOrder', 'Trade', 'CancelOrder'] as EventType[],
          batchSize: 5000,
        });
        checksums.push(snap.checksum);
      }
      newRows.push({
        workers,
        checksums,
        allMatch: checksums.every(c => c === checksums[0]),
      });
    }

    setRows(newRows);
    setRunning(false);
  }

  return (
    <div>
      <div className="flex items-center justify-between mb-5">
        <SectionLabel>Determinism Verification</SectionLabel>
        <button
          onClick={run}
          disabled={running}
          className="text-[13px] px-5 py-2 rounded-full border border-[#e5e5e5] hover:border-[#0d0d0d] transition-colors cursor-pointer bg-white disabled:opacity-40 font-medium"
        >
          {running ? 'Running...' : 'Verify'}
        </button>
      </div>

      {rows.length > 0 && (
        <div className="overflow-x-auto">
          <table className="w-full text-[13px]">
            <thead>
              <tr className="text-[12px] text-[#a3a3a3]">
                <th className="text-left py-3 pr-4 font-normal">Workers</th>
                <th className="text-left py-3 pr-4 font-normal">Run 1</th>
                <th className="text-left py-3 pr-4 font-normal">Run 2</th>
                <th className="text-left py-3 pr-4 font-normal">Run 3</th>
                <th className="text-left py-3 font-normal">Match</th>
              </tr>
            </thead>
            <tbody>
              {rows.map(r => (
                <tr key={r.workers} className="border-t border-[#f0f0f0]">
                  <td className="py-3 pr-4 font-medium">{r.workers}</td>
                  {r.checksums.map((c, i) => (
                    <td key={i} className="py-3 pr-4 font-mono text-[12px] text-[#737373] tabular-nums">
                      {c.toString(16).toUpperCase()}
                    </td>
                  ))}
                  <td className="py-3">
                    <span className={`inline-flex px-3 py-0.5 rounded-full text-[12px] font-medium ${
                      r.allMatch
                        ? 'bg-[#ecfdf5] text-[#16a34a]'
                        : 'bg-[#fef2f2] text-[#dc2626]'
                    }`}>
                      {r.allMatch ? 'Yes' : 'No'}
                    </span>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
          {rows.length > 0 && rows.every(r => r.allMatch) && rows.every(r => r.checksums[0] === rows[0].checksums[0]) && (
            <p className="text-[13px] text-[#16a34a] mt-4">
              All configurations produce identical checksums &mdash; deterministic replay verified.
            </p>
          )}
        </div>
      )}

      {rows.length === 0 && (
        <p className="text-[14px] text-[#737373]">
          Runs the same 5,000 events across 1, 2, 4, and 8 worker configurations, 3 times each. Verifies bit-identical checksums.
        </p>
      )}
    </div>
  );
}
