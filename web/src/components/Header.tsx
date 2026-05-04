export default function Header() {
  return (
    <header className="mb-12">
      <h1 className="text-[42px] font-light tracking-[-0.02em] text-[#1a1a1a] mb-1 leading-tight">
        Replay Engine
      </h1>
      <p className="text-[#6b6b6b] text-[14px]">
        Lock-free multi-threaded deterministic replay. 3.5M events/sec, bit-identical across runs.
      </p>
    </header>
  );
}
