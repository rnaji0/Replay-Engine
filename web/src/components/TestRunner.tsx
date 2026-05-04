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
      <div className="flex items-center justify-between mb-4">
        <SectionLabel>Test Suite</SectionLabel>
        <button
          onClick={run}
          disabled={running}
          className="text-[12px] px-4 py-1.5 border border-[#d4d0cb] hover:border-[#1a1a1a] transition-colors cursor-pointer bg-transparent disabled:opacity-50"
        >
          {running ? 'Running...' : 'Run All Tests'}
        </button>
      </div>

      {results.length === 0 && !running && (
        <p className="text-[13px] text-[#6b6b6b]">
          Runs the full 20-test suite: determinism, stress, memory, metrics, and fault injection.
        </p>
      )}

      {running && (
        <p className="text-[13px] text-[#6b6b6b] animate-pulse">Running tests...</p>
      )}

      {results.length > 0 && (
        <div>
          <div className="flex gap-6 mb-5 text-[13px]">
            <span>
              <span className="font-sans font-medium text-lg mr-1">{total}</span>
              <span className="text-[#6b6b6b]">total</span>
            </span>
            <span>
              <span className="font-sans font-medium text-lg mr-1 text-[#2d6a3f]">{passed}</span>
              <span className="text-[#6b6b6b]">passed</span>
            </span>
            {failed > 0 && (
              <span>
                <span className="font-sans font-medium text-lg mr-1 text-[#8b2525]">{failed}</span>
                <span className="text-[#6b6b6b]">failed</span>
              </span>
            )}
            <span>
              <span className="font-sans font-medium text-lg mr-1">{totalMs.toFixed(1)}</span>
              <span className="text-[#6b6b6b]">ms</span>
            </span>
          </div>

          {modules.map(mod => (
            <div key={mod} className="mb-4">
              <div className="text-[12px] font-medium text-[#1a1a1a] mb-2">{mod}</div>
              <div className="space-y-1">
                {results.filter(r => r.module === mod).map((r, i) => (
                  <div key={i} className="flex items-baseline gap-3 text-[13px]">
                    <span className={`font-mono text-[11px] w-[42px] shrink-0 ${
                      r.status === 'passed' ? 'text-[#2d6a3f]' : 'text-[#8b2525]'
                    }`}>
                      {r.status === 'passed' ? 'PASS' : 'FAIL'}
                    </span>
                    <span className="text-[#1a1a1a]">{r.name}</span>
                    <span className="text-[#c4bfb8]">&mdash;</span>
                    <span className="text-[#6b6b6b] text-[12px] truncate">{r.message}</span>
                    <span className="text-[#c4bfb8] text-[11px] ml-auto shrink-0">{r.durationMs.toFixed(1)}ms</span>
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
