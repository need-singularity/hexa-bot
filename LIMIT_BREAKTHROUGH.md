<!-- @created: 2026-05-12 -->
<!-- @scope: real-limits audit — robot / bot substrate (actuator, planning, perception, agent coordination, honesty) -->
<!-- @authority: per LATTICE_POLICY.md §1.2 -->
<!-- @wave: M (limit-breakthrough audit, application repos) -->

# LIMIT_BREAKTHROUGH.md — hexa-bot real-limits audit

> **Honest scope**: hexa-bot is a **4-verb robot substrate** (robotics
> / transport / automation / dog-robot). Per the brief, the Turing
> test is a moving target (LLMs largely pass it for short
> conversation); the real limits are **actuator power density**,
> **perception long-tail**, **agent-coordination complexity**, and
> **honesty / hallucination rate** of any embedded LLM-style decision
> layer. **No AGI claims.** This audit names verifiable performance
> bounds.

---

## §1 Domain

`hexa-bot` is an **embodied-robot substrate** covering quadruped
(dog-robot), wheeled / aerial transport, factory automation, and
general robotics. Distinct from `hexa-mobility` (which is L5
automotive) — `hexa-bot` is robot-axis: actuators, sim-to-real,
gait control, manipulation, automation control loops.

The dominant operating envelopes are:
- **Actuator energy density** — torque/mass for joints; bounded by
  motor + gearbox physics
- **Battery / tether power** — same chemistry walls as `hexa-mobility`
- **Perception reliability** — long-tail edge-case failure
- **Decision honesty** — if LLM in control loop, hallucination rate
  becomes a safety variable

---

## §2 Limits

### §2.1 HARD walls (physics / computation / information)

| # | Limit | Bound | Origin |
|---|-------|-------|--------|
| H1 | **Motor torque density** | ~10–50 Nm/kg BLDC; ~100 Nm/kg with reduction | Magnetics + thermal; HARD |
| H2 | **Battery energy density** | ~300 Wh/kg Li-ion 2026; ~3000 Wh/kg Li-air theoretical | Electrochemistry; HARD |
| H3 | **Servo bandwidth** | ~100–1000 Hz typical industrial | Mechanical resonance + control-loop stability; HARD |
| H4 | **Control-loop closure delay** | ≥ sensor-to-actuator round-trip | Light-cone + sample-rate; HARD |
| H5 | **PAC-learning sample complexity** | poly(1/ε, 1/δ, VC-dim) | Valiant 1984; HARD lower bound |
| H6 | **Adversarial-example robustness** | Provable robustness ≪ empirical | Madry et al.; HARD on smooth NN classes |
| H7 | **Halting / planning decidability** | STRIPS planning PSPACE-complete, general motion planning undecidable | Bylander 1994; HARD |
| H8 | **Multi-agent coordination** | NEXP-complete general DEC-POMDP | Bernstein et al. 2002; HARD |
| H9 | **Sensor noise floor** | kT noise + shot noise on photon detection | Thermodynamics + quantum; HARD |
| H10 | **Hallucination floor (auto-regressive LLM)** | ≥ ε > 0 without grounding | Inherent in unconditional generation; bounded SOFT-ly |

### §2.2 SOFT envelopes (engineering / measurable)

| # | Envelope | Current | Breakthrough margin |
|---|----------|---------|---------------------|
| S1 | **Quadruped peak power-to-weight** | 30–60 W/kg (Spot, Unitree B2) | SOFT, target 100+ W/kg with QDD actuators |
| S2 | **Walking endurance** | 60–120 min (Spot); 240 min (B2) | SOFT, bounded by H2 |
| S3 | **Object-grasp success on novel item** | ~50–80% (Google RT-2 benchmark) | SOFT, dataset-bounded |
| S4 | **LLM hallucination rate (Q&A no retrieval)** | 5–20% per claim (HELM 2024) | SOFT, target <2% with RAG + citation |
| S5 | **Sim-to-real transfer gap** | 10–40% performance loss | SOFT, declining with domain randomization |
| S6 | **Industrial-robot uptime** | 95–99% (mature lines) | SOFT, asymptote at 99.9% |
| S7 | **Manipulator settling time** | 100–500 ms typical | SOFT, bounded by H3 |
| S8 | **MTBF (collaborative robot)** | 30,000–80,000 h | SOFT |

### §2.3 Negotiated / standard (NOT physical)

- **ISO 10218 industrial-robot safety** — standard
- **ISO 13482 personal-care-robot safety** — standard
- **EU Machinery Regulation 2023/1230** — regulatory
- **FAA Part 107 (small UAS)** — regulatory
- **CE / FCC EMC compliance** — regulatory
- **"Turing test"** — historical benchmark; *the brief explicitly notes
  this is decreasingly meaningful*. Treated as cultural, not technical.

---

## §3 Assessment

The 3 most binding HARD walls:

1. **H2 (battery)** — same wall as `hexa-mobility`. A 30 kg quadruped
   at 60 W average draws ~60 Wh/h; a 1 kWh pack runs ~16 h. Doubling
   endurance means doubling pack mass *or* doubling chemistry density.
2. **H6 + H10 (adversarial / hallucination)** — if any LLM-style
   decision layer is in the control loop, adversarial perception and
   hallucinated plans become safety-critical. The HARD wall is that
   no smooth NN is provably robust against bounded perturbation
   beyond modest ε; the SOFT bound is grounding + retrieval + citation.
3. **H8 (multi-agent coordination)** — NEXP-complete for DEC-POMDP
   means *general* coordination of N robots is intractable; only
   structured (factored, weakly-coupled) subclasses are tractable.

**Honest framing on "AI" claims** (per brief): hexa-bot does NOT
claim AGI. The auditable performance metrics are:
- Grasp success rate on benchmark distribution (S3)
- Sim-to-real performance gap (S5)
- Hallucination rate per claim with citation-source check (S4)
- MTBF / uptime (S6, S8)

These are *measurable*. "Intelligence" is not.

**Turing test framing**: per brief, LLMs pass short-conversation
Turing tests routinely (since ~2023). This makes the test
*decreasingly meaningful*. The real auditable axis is *verifiable
honesty* — citation accuracy + retrieval grounding + refusal-on-unknown.

---

## §4 Top-3 breakthroughs (most plausible 12–24 month)

### B1 — QDD (quasi-direct-drive) actuators + carbon-fiber chassis → 100+ W/kg quadruped peak (SOFT envelope move on S1)

MIT Cheetah lineage QDD actuators (low-gear-ratio, high-torque BLDC +
planetary) have demonstrated 5–10× peak torque vs traditional
servo-gear-train designs at comparable mass. The HARD wall (H1 motor
density) is approached but not broken. Existence proofs: Unitree B2,
MIT Mini Cheetah, ANYbotics ANYmal. Honest caveat: peak ≠ sustained;
thermal limits keep sustained power-to-weight closer to 30 W/kg.

### B2 — Retrieval-augmented planning + structured citation → hallucination rate <2% per claim (SOFT envelope move on S4)

For any decision-layer LLM in the bot's planning loop, retrieval
grounding (RAG) + structured citation + refusal-on-unknown training
has demonstrably reduced hallucination rate from ~10–20% to <2% in
constrained Q&A domains (HELM 2024, RAGAS benchmarks). The HARD wall
(H10) is not eliminated — there is no zero-hallucination
auto-regressive LLM — but the rate can be reduced 5–10× with
engineering. Honest caveat: this is a narrow improvement in
constrained domains; open-ended generation hallucination is harder.

### B3 — Structured multi-agent coordination via decentralized POMDP factoring → tractable N=10–100 robot swarms (SOFT envelope move on S6 + sidesteps H8)

By factoring multi-agent coordination into *weakly-coupled subgroups*
with explicit communication topology (FactorNet / VDN / QMIX-class
methods), N=10–100 robot swarms can coordinate in tractable time. The
HARD wall (H8 — NEXP-complete general DEC-POMDP) is not broken; it is
*sidestepped* by restricting to the structured-coupling subclass.
Existence proofs: Amazon-Kiva warehouse fleet (~10⁴ AGVs); academic
swarm-robotics demos. Honest caveat: structured subclass excludes
adversarial or strongly-coupled coordination tasks.

---

## §5 Caveats

1. **NO AGI claim**: per brief, hexa-bot does NOT claim AGI feasibility.
   The metrics above are narrow, measurable, and constrained to
   published benchmarks.
2. **NO Turing test as success metric**: per brief, the Turing test is
   decreasingly meaningful (LLMs pass short-conversation Turing routinely
   since 2023). Honesty-and-citation accuracy is the real auditable
   axis.
3. **Hallucination floor is non-zero**: even with RAG, auto-regressive
   LLMs have residual hallucination. Safety-critical loops MUST have
   verification layers (formal contract, human-in-loop, sanity bounds).
4. **Multi-agent intractability** (H8) means swarms scale only in
   structured subclasses; "emergent intelligence" claims for
   unstructured swarms are unsupported.
5. **Battery wall** (H2) applies same as mobility; endurance numbers
   (S2) scale linearly with pack mass at HARD chemistry ceiling.
6. **Safety standards** (ISO 10218, 13482) are regulatory contracts;
   compliance is engineering work, not physics.
7. **No n=6 magic**: per `LATTICE_POLICY.md §1.2`, motor torque
   density / hallucination rate / coordination complexity are NOT
   dictated by σ(6)=12. The lattice is organizing vocabulary.
8. **Public benchmarks only**: HELM, RAGAS, RoboCup, NIST OPS — no
   proprietary data; no GDPR concern.

---

## §6 References

- Valiant, L. *A theory of the learnable* (1984) — H5 PAC-learning
- Madry, A. et al. *Towards Deep Learning Models Resistant to Adversarial Attacks* (2018) — H6
- Bylander, T. *The computational complexity of propositional STRIPS planning* (1994) — H7
- Bernstein, D. S. et al. *The Complexity of Decentralized Control of Markov Decision Processes* (2002) — H8
- Turing, A. M. *Computing Machinery and Intelligence* (1950) — Turing-test reference (now treated as cultural artifact)
- Wen, B. et al. *MIT Cheetah series QDD actuator papers* (2014–2019) — B1
- Lewis, P. et al. *Retrieval-Augmented Generation for Knowledge-Intensive NLP Tasks* (NeurIPS 2020) — B2
- HELM benchmark (Stanford CRFM) 2023–2024 — S4
- Rashid, T. et al. *QMIX: Monotonic Value Function Factorisation for Deep Multi-Agent Reinforcement Learning* (ICML 2018) — B3
- ISO 10218-1:2011 / ISO 13482:2014 — robot safety standards
- Boston Dynamics Spot / Unitree B2 spec sheets (public) — S1, S2
- Google DeepMind RT-2 paper (2023) — S3 grasp benchmarks

---

> *"Turing is a moving target. Hallucination is a measurable rate.
> Energy density is chemistry. NEXP-complete is NEXP-complete."*

— hexa-bot Wave M audit (2026-05-12)
