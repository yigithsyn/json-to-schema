# Building and publishing the documentation

The guide uses MkDocs with its built-in Read the Docs theme, following the
libamcore documentation layout. Markdown sources live in `docs/public`;
`mkdocs.yml` defines site metadata and navigation.

## Local setup

From the repository root, use Python 3.13 or newer and optionally create an
isolated environment:

```powershell
python -m venv .venv-docs
.\.venv-docs\Scripts\Activate.ps1
python -m pip install -r docs/requirements.txt
```

In a Unix-like shell, activate the environment with
`source .venv-docs/bin/activate` instead.

## Build and preview

```powershell
python -m mkdocs build --strict
python -m mkdocs serve
```

The strict build rejects documentation warnings, including broken internal
links. Generated HTML goes to `site`, which is not committed. The preview
normally runs at <http://127.0.0.1:8000/>; use the URL printed by MkDocs and stop
the server with Ctrl+C.

The Makefile also provides `make docs` and `make docs-serve`. Both use the
currently selected Python interpreter; override it with `PYTHON=python3` if
needed. Install dependencies before running these targets.

When adding a page, create its Markdown source in `docs/public`, add it to
`nav` in `mkdocs.yml`, and run a strict build. Keep examples consistent with the
actual C API and CLI.

## GitHub Pages

The Documentation workflow:

1. Builds the site strictly for documentation pull requests and each `main` push
   through the CI workflow.
2. Uploads the generated site as a Pages artifact.
3. Deploys only on pushes to `main` or manual runs on `main`.

The workflow also supports `workflow_call` for reuse by another workflow.
Pull requests build the documentation but do not deploy it. The CI workflow
reuses this check and deploys documentation independently of release builds.

To enable hosting, a repository administrator must select **GitHub Actions**
under **Settings > Pages > Build and deployment > Source**. Commit and push the
documentation changes to `main`, or run **Actions > Documentation > Run workflow**
on `main`.

The configured site address is
<https://yigithsyn.github.io/json-to-schema/>. It becomes available only after
Pages has been enabled and a deployment succeeds. Change `site_url` in
`mkdocs.yml` if the hosting address changes.

## Releases

The CI workflow checks the version in `CMakeLists.txt` on pushes to `main`.
When it differs from the latest release tag, it moves notes from the
`[Unreleased]` section of `CHANGELOG.md` into a dated release entry, creates an
annotated `v<version>` tag, and starts cross-platform build and test jobs. The
same jobs also run for manually pushed `v*` tags, which must match the CMake
project version.

The Release workflow packages the library, public headers, and CLI as ZIP files
on Linux, Windows, and macOS, then attaches them to a GitHub Release using that
tag's changelog entry. Bump `project(VERSION ...)` in `CMakeLists.txt` and add
release notes under `[Unreleased]` to publish a new version. Release
preparation behavior is covered by `node --test
tests/test_release_workflow.cjs`.

## Read the Docs

The repository also includes `.readthedocs.yaml`, using the same MkDocs
configuration and requirements file. Import the repository into Read the Docs
and enable builds there to use that hosting service. The configuration alone
does not create a hosted project.
