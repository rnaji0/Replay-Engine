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

    // cross-config: all should be identical
    const allSame = newRows.every(r => r.checksums[0] === newRows[0].checksums[0]);
    setRows(newRows);
    setRunning(false);
  }

  return (
    <div>
      <div className="flex items-center justify-between mb-4">
        <SectionLabel>Determinism Verification</SectionLabel>
        <button
          onClick={run}
          disabled={running}
          className="text-[12px] px-4 py-1.5 border border-[#d4d0cb] hover:border-[#1a1a1a] transition-colors cursor-pointer bg-transparent disabled:opacity-50"
        >
          {running ? 'Running...' : 'Verify'}
        </button>
      </div>

      {rows.length > 0 && (
        <div className="overflow-x-auto">
          <table className="w-full text-[13px]">
            <thead>
              <tr className="text-[11px] uppercase tracking-[0.08em] text-[#6b6b6b]">
                <th className="text-left py-2 pr-4 font-medium">Workers</th>
                <th className="text-left py-2 pr-4 font-medium">Run 1</th>
                <th className="text-left py-2 pr-4 font-medium">Run 2</th>
                <th className="text-left py-2 pr-4 font-medium">Run 3</th>
                <th className="text-left py-2 font-medium">Match</th>
              </tr>
            </thead>
            <tbody>
              {rows.map(r => (
                <tr key={r.workers} className="border-t border-[#eae5df]">
                  <td className="py-2 pr-4 font-sans font-medium">{r.workers}</td>
                  {r.checksums.map((c, i) => (
                    <td key={i} className="py-2 pr-4 font-mono text-[12px] text-[#6b6b6b]">
                      {c.toString(16).toUpperCase()}
                    </td>
                  ))}
                  <td className={`py-2 font-medium ${r.allMatch ? 'text-[#2d6a3f]' : 'text-[#8b2525]'}`}>
                    {r.allMatch ? 'YES' : 'NO'}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
          {rows.length > 0 && rows.every(r => r.allMatch) && rows.every(r => r.checksums[0] === rows[0].checksums[0]) && (
            <p className="text-[12px] text-[#2d6a3f] mt-3">
              All configurations produce identical checksums &mdash; deterministic replay verified.
            </p>
          )}
        </div>
      )}

      {rows.length === 0 && (
        <p className="text-[13px] text-[#6b6b6b]">
          Runs the same 5,000 events across 1, 2, 4, and 8 worker configurations, 3 times each. Verifies bit-identical checksums.
        </p>
      )}
    </div>
  );
}
