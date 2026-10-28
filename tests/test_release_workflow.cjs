const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const test = require('node:test');
const vm = require('node:vm');

function workflowScript(name) {
  const workflow = fs.readFileSync(
    path.join(__dirname, '..', '.github', 'workflows', name), 'utf8');
  const lines = workflow.replace(/\r\n/g, '\n').split('\n');
  const start = lines.indexOf('          script: |');
  assert.notEqual(start, -1);
  const script = [];
  for (const line of lines.slice(start + 1)) {
    if (line.trim() && !line.startsWith('            ')) break;
    script.push(line.slice(12));
  }
  return `(async () => {\n${script.join('\n')}\n})()`;
}

const prepareScript = workflowScript('ci.yml');
const publishScript = workflowScript('release.yml');
const history = '## [v0.1.0] - 2026-10-01\n\n- Historical change.\n';

async function prepare({
  changelog = `# Changelog\n\n## [Unreleased]\n\n- Pending change.\n\n${history}`,
  tags = 'v0.1.0',
  ref = 'refs/heads/main',
  overrides = {},
} = {}) {
  const files = {
    'CMakeLists.txt': 'project(json_to_schema\n VERSION 0.1.1\n LANGUAGES C)\n',
    'CHANGELOG.md': changelog,
    ...overrides,
  };
  const original = { ...files };
  const calls = [];
  const outputs = {};
  let committed;
  const fileSystem = {
    readFileSync: name => {
      assert.ok(Object.hasOwn(files, name), `Unexpected file: ${name}`);
      return files[name];
    },
    writeFileSync: (name, content) => { files[name] = content; },
  };

  await vm.runInNewContext(prepareScript, {
    require: name => {
      assert.equal(name, 'fs');
      return fileSystem;
    },
    core: { setOutput: (name, value) => { outputs[name] = value; } },
    context: {
      eventName: 'push',
      ref,
      ref_name: ref.replace(/^refs\/tags\//, ''),
      sha: 'source-sha',
    },
    exec: {
      getExecOutput: async (command, args) => {
        assert.equal(command, 'git');
        calls.push(Array.from(args));
        let stdout = '';
        if (args[0] === 'tag' && args[1] === '--list') stdout = tags;
        if (args[0] === 'log') stdout = 'Commit subject\nAnother subject\n';
        if (args[0] === 'diff') {
          stdout = Object.keys(original)
            .filter(name => files[name] !== original[name])
            .join('\n');
        }
        if (args[0] === 'commit') committed = { ...files };
        return { stdout };
      },
    },
  });
  return { files, original, calls, outputs, committed, fileSystem };
}

test('prepares changelog notes and tags the release commit', async () => {
  const result = await prepare();
  const { files, calls, committed, outputs } = result;
  assert.match(files['CHANGELOG.md'], /## \[Unreleased\]\n\n## \[v0\.1\.1\] - \d{4}-\d{2}-\d{2}/);
  assert.equal(files['CHANGELOG.md'].split('- Pending change.').length, 2);
  assert.ok(files['CHANGELOG.md'].endsWith(history));
  assert.deepEqual(committed, files);
  assert.deepEqual(outputs, { tag: 'v0.1.1', changed: 'true' });
  assert.equal(calls.some(args => args[0] === 'log'), false);
  const commitIndex = calls.findIndex(args => args[0] === 'commit');
  const tagIndex = calls.findIndex(args => args[0] === 'tag' && args[1] === '-a');
  assert.ok(commitIndex < tagIndex);
  assert.match(calls[tagIndex][4], /Pending change/);
  assert.doesNotMatch(calls[tagIndex][4], /Unreleased/);
  assert.equal(calls[tagIndex].at(-1), 'HEAD');
  assert.ok(calls.findIndex(args => args[0] === 'push') > tagIndex);

  await vm.runInNewContext(publishScript, {
    require: () => result.fileSystem,
    process: { env: { RELEASE_TAG: 'v0.1.1' } },
  });
  assert.match(files['release-notes.md'], /Pending change/);
  assert.doesNotMatch(files['release-notes.md'], /Unreleased|Historical change/);
  await assert.rejects(
    vm.runInNewContext(publishScript, {
      require: () => result.fileSystem,
      process: { env: { RELEASE_TAG: 'v0.2.0' } },
    }),
    /does not match/,
  );
});

test('uses commit subjects when no unreleased notes are present', async () => {
  const { files, calls } = await prepare({
    changelog: `# Changelog\n\n## [Unreleased]\n\n${history}`,
  });
  assert.match(files['CHANGELOG.md'], /### Changes\n\n- Commit subject\n- Another subject/);
  assert.equal(calls.filter(args => args[0] === 'log').length, 1);
});

test('preserves CRLF changelog formatting for the first release', async () => {
  const { files } = await prepare({
    tags: '',
    changelog: '# Changelog\r\n\r\n## [Unreleased]\r\n\r\n- First change.\r\n',
  });
  assert.match(files['CHANGELOG.md'], /## \[v0\.1\.1\]/);
  assert.match(files['CHANGELOG.md'], /- First change\./);
  assert.doesNotMatch(files['CHANGELOG.md'], /(?<!\r)\n/);
});

test('does not prepare a release on tag pushes or when the version already exists', async () => {
  for (const options of [
    { tags: 'v0.1.1' },
    { ref: 'refs/tags/v0.1.1' },
  ]) {
    const { files, original, outputs, calls } = await prepare(options);
    assert.deepEqual(files, original);
    assert.equal(outputs.changed, 'false');
    assert.equal(calls.some(args => args[0] === 'push'), false);
  }
});

test('rejects reused tags, mismatched tag versions, and missing CMake versions', async () => {
  await assert.rejects(prepare({ tags: 'v0.1.2\nv0.1.1' }), /already exists/);
  await assert.rejects(prepare({ tags: 'v0.1.2' }), /older than latest release/);
  await assert.rejects(prepare({ ref: 'refs/tags/v0.1.0' }), /does not match/);
  await assert.rejects(
    prepare({ overrides: { 'CMakeLists.txt': 'project(json_to_schema)' } }),
    /Cannot read json_to_schema VERSION/,
  );
});
