export default function SectionLabel({ children }: { children: React.ReactNode }) {
  return (
    <h2 className="text-[13px] font-medium tracking-[-0.01em] text-[#0d0d0d] mb-5">
      {children}
    </h2>
  );
}
