export default function SectionLabel({ children }: { children: React.ReactNode }) {
  return (
    <h2 className="text-[11px] font-sans font-medium tracking-[0.12em] uppercase text-[#6b6b6b] mb-4">
      {children}
    </h2>
  );
}
