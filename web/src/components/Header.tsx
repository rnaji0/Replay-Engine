export default function Header() {
  return (
    <header className="mb-16 text-center">
      <h1 className="text-[40px] font-medium tracking-[-0.03em] text-[#0d0d0d] mb-3 leading-[1.1]">
        Replay Engine
      </h1>
      <p className="text-[#737373] text-[15px] max-w-[480px] mx-auto">
        Lock-free multi-threaded deterministic replay. 3.5M events/sec, bit-identical across runs.
      </p>
      <p className="text-[#a3a3a3] text-[12px] mt-3">
        TypeScript-ported interactive demo of the C++ engine
      </p>
    </header>
  );
}
