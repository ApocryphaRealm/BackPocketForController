#!/usr/bin/env python3
r"""Assemble "Back Pocket For Controller <ver>" in 7. current test builds from this repo's build tree and documents.

One build line (SE 1.5.97 / AE 1.6.1170 / GOG 1.6.1179): Back Pocket's hooks carry no 1.7 addresses, so there is no
FOMOD - the package installs as it is.

    <Display Name> <ver>\SKSE\Plugins\BackPocketForController.{dll,pdb,ini}
                        \Interface\BackPocket\category_icon.swf
                        \README.txt, LICENSE-BackPocketForController-GPL-3.0.txt, LICENSE-BackPocket-MIT.txt,
                         NOTICE.md, THIRD_PARTY_NOTICES.md, CHANGELOG.md

    python tools/build-package.py            (version read from CMakeLists.txt, which the version gate stamps)
"""
import os, re, shutil

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.normpath(os.path.join(REPO, "..", ".."))
NAME, TARGET = "Back Pocket For Controller", "BackPocketForController"


def fail(msg):
    raise SystemExit("build-package: FAIL  " + msg)


def main():
    cm = open(os.path.join(REPO, "CMakeLists.txt"), encoding="utf-8-sig").read()
    m = re.search(r"(?m)^\s*VERSION\s+(\d+\.\d+\.\d+)", cm)
    if not m:
        fail("no project VERSION in CMakeLists.txt")
    ver = m.group(1)
    out = os.path.join(ROOT, "7. current test builds", f"{NAME} {ver}")
    if os.path.exists(out):
        shutil.rmtree(out)
    plugins = os.path.join(out, "SKSE", "Plugins")
    os.makedirs(plugins)
    for ext in ("dll", "pdb"):
        src = os.path.join(REPO, "build", "relwithdebinfo-se-only", f"{TARGET}.{ext}")
        if not os.path.exists(src):
            fail(f"no {src} - run build.bat first")
        shutil.copy2(src, plugins)
    shutil.copy2(os.path.join(REPO, "dist", f"{TARGET}.ini"), plugins)
    icon = os.path.join(out, "Interface", "BackPocket")
    os.makedirs(icon)
    shutil.copy2(os.path.join(REPO, "dist", "Interface", "BackPocket", "category_icon.swf"), icon)

    readme = open(os.path.join(REPO, "dist", "README.txt"), encoding="utf-8").read()
    if f"Version {ver}" not in readme[:240]:
        fail(f"dist\\README.txt does not name version {ver} at the top")
    shutil.copy2(os.path.join(REPO, "dist", "README.txt"), os.path.join(out, "README.txt"))
    shutil.copy2(os.path.join(REPO, "LICENSE"), os.path.join(out, f"LICENSE-{TARGET}-GPL-3.0.txt"))
    for f in ("LICENSE-BackPocket-MIT.txt", "NOTICE.md", "THIRD_PARTY_NOTICES.md", "CHANGELOG.md"):
        shutil.copy2(os.path.join(REPO, f), os.path.join(out, f))
    n = sum(len(fs) for _, _, fs in os.walk(out))
    print(f"build-package: pass  {out}  ({n} files, DLL {os.path.getsize(os.path.join(plugins, TARGET + '.dll'))} bytes)")


if __name__ == "__main__":
    main()
