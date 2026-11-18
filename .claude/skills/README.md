# Vendored Zephyr agent skills

Claude Code loads the skills in this directory automatically for anyone working on zperf.

They are copied unmodified from
[beriberikix/zephyr-agent-skills](https://github.com/beriberikix/zephyr-agent-skills)
at commit `ed63cdfb8cdfbeb5946ea39c33f4aa6bcf3a5cce`, under the Apache License 2.0
(see `LICENSE` in this directory).

| Skill | Why it is here |
|-------|----------------|
| `zephyr-module` | zperf is an out-of-tree Zephyr module (`zephyr/module.yml`, Kconfig, CMake) |
| `kernel-basics` | threads, logging and shell, which zperf hooks into and prints through |
| `kernel-services` | work queues and other kernel services used by applications zperf profiles |
| `testing-debugging` | ztest, twister, tracing and Thread Analyzer |
| `power-performance` | power states and performance tuning, related to the CPU load zperf reports |
| `native-sim` | host-side builds and tests without hardware |

## Why the skills are copied

The upstream repository can also be installed as a Claude Code plugin marketplace, but at the
pinned commit its `.claude-plugin/marketplace.json` fails validation in Claude Code 2.1.327
(`owner` must be an object and `plugins` an array). Once that is fixed upstream, these copies
can be replaced with an `extraKnownMarketplaces` entry in `.claude/settings.json`.

## Updating

```sh
git clone https://github.com/beriberikix/zephyr-agent-skills /tmp/zephyr-agent-skills
for s in zephyr-module kernel-basics kernel-services testing-debugging power-performance native-sim; do
    rm -rf ".claude/skills/$s"
    cp -r "/tmp/zephyr-agent-skills/skills/$s" ".claude/skills/$s"
done
cp /tmp/zephyr-agent-skills/LICENSE .claude/skills/LICENSE
```

Then update the commit hash above.
