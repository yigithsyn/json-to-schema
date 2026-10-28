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

## Read the Docs

The repository also includes `.readthedocs.yaml`, using the same MkDocs
configuration and requirements file. Import the repository into Read the Docs
and enable builds there to use that hosting service. The configuration alone
does not create a hosted project.
