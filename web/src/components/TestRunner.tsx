import { useState } from 'react';
import type { TestResult } from '../engine/types';
import { runAllTests } from '../engine/tests';
import SectionLabel from './SectionLabel';

export default function TestRunner() {
  const [results, setResults] = useState<TestResult[]>([]);
  const [running, setRunning] = useState(false);

  function run() {
    setRunning(true);
    setResults([]);
    setTimeout(() => {
      const r = runAllTests();
      setResults(r);
      setRunning(false);
    }, 50);
  }

  const passed = results.filter(r => r.status === 'passed').length;
  const failed = results.filter(r => r.status === 'failed').length;
  const total = results.length;
  const totalMs = results.reduce((s, r) => s + r.durationMs, 0);

  const modules = [...new Set(results.map(r => r.module))];

  return (
    <div>
      <div className="flex items-center justify-between mb-5">
        <SectionLabel>Test Suite</SectionLabel>
        <button
          onClick={run}
          disabled={running}
          className="text-[13px] px-5 py-2 rounded-full border border-[#e5e5e5] hover:border-[#0d0d0d] transition-colors cursor-pointer bg-white disabled:opacity-40 font-medium"
        >
          {running ? 'Running...' : 'Run All Tests'}
        </button>
      </div>

      {results.length === 0 && !running && (
        <p className="text-[14px] text-[#737373]">
          Runs the full 20-test suite: determinism, stress, memory, metrics, and fault injection.
        </p>
      )}

      {running && (
        <p className="text-[14px] text-[#737373] animate-pulse">Running tests...</p>
      )}

      {results.length > 0 && (
        <div>
          <div className="flex gap-5 mb-6 text-[14px]">
            <div className="flex items-baseline gap-1.5">
              <span className="text-[22px] font-medium tabular-nums">{total}</span>
              <span className="text-[#a3a3a3]">total</span>
            </div>
            <div className="flex items-baseline gap-1.5">
              <span className="text-[22px] font-medium tabular-nums text-[#16a34a]">{passed}</span>
              <span className="text-[#a3a3a3]">passed</span>
            </div>
            {failed > 0 && (
              <div className="flex items-baseline gap-1.5">
                <span className="text-[22px] font-medium tabular-nums text-[#dc2626]">{failed}</span>
                <span className="text-[#a3a3a3]">failed</span>
              </div>
            )}
            <div className="flex items-baseline gap-1.5">
              <span className="text-[22px] font-medium tabular-nums">{totalMs.toFixed(1)}</span>
              <span className="text-[#a3a3a3]">ms</span>
            </div>
          </div>

          {modules.map(mod => (
            <div key={mod} className="mb-5">
              <div className="text-[13px] font-medium text-[#0d0d0d] mb-2.5">{mod}</div>
              <div className="space-y-1.5">
                {results.filter(r => r.module === mod).map((r, i) => (
                  <div key={i} className="flex items-baseline gap-3 text-[13px]">
                    <span className={`font-mono text-[11px] shrink-0 inline-flex px-2 py-0.5 rounded-full ${
                      r.status === 'passed'
                        ? 'bg-[#ecfdf5] text-[#16a34a]'
                        : 'bg-[#fef2f2] text-[#dc2626]'
                    }`}>
                      {r.status === 'passed' ? 'Pass' : 'Fail'}
                    </span>
                    <span className="text-[#0d0d0d]">{r.name}</span>
                    <span className="text-[#d4d4d4]">&middot;</span>
                    <span className="text-[#737373] text-[12px] truncate">{r.message}</span>
                    <span className="text-[#d4d4d4] text-[11px] ml-auto shrink-0 tabular-nums">{r.durationMs.toFixed(1)}ms</span>
                  </div>
                ))}
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}
