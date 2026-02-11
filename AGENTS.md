## Instruction-Hierarchie

- Primaere Regelquelle: `AGENTS.md` im Repo-Root. Diese Datei gilt immer zuerst.
- Sekundaere Regelquelle: `.github/instructions/Anweisungen.instructions.md` fuer projekt-/dateispezifische Workflows.
- Lade-Regel: Bei Aenderungen an `arduino/**`, `nodered/**`, `components.yaml`, `README.md`, `deploy_flows.sh` oder `DEPLOYMENT_GUIDE.md` wird `.github/instructions/Anweisungen.instructions.md` zusaetzlich angewendet.
- Konflikt-Regel: Bei Widerspruch gewinnt `AGENTS.md`. `Anweisungen.instructions.md` darf nur praezisieren/verschaerfen, nicht lockern.
- Fallback: Ist `.github/instructions/Anweisungen.instructions.md` nicht lesbar oder fehlt, wird mit `AGENTS.md` weitergearbeitet und der fehlende Kontext kurz gemeldet.
- Benennung: Kein Umbenennen noetig; `Anweisungen.instructions.md` bleibt wie sie ist.
