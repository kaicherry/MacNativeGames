"""The dev-tree shader-libraries builder (games/nfs/scripts/build_shader_libs.py).

It compiles build/shader-cache/rt-<chain>/*.metal into build/shader-libs/<SHA-256 of the MSL>.metallib, the exact
name nvn_mtl.m shader_load looks for before compiling a variant on demand. The tests run it with a compiler that
copies its input, so no Metal toolchain is needed.
"""
import contextlib
import hashlib
import importlib.util
import io
from pathlib import Path
import stat
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
_spec = importlib.util.spec_from_file_location('build_shader_libs', ROOT / 'games/nfs/scripts/build_shader_libs.py')
builder = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(builder)

CHAIN = 'totk-native-v4-unverified'


def run(argv):
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        code = builder.main(argv)
    return code, out.getvalue(), err.getvalue()


class BuilderTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.work = self.root / 'work'
        self.cache = self.work / f'build/shader-cache/rt-{CHAIN}'
        self.cache.mkdir(parents=True)
        compiler = self.root / 'metal'  # copies the source to the output, as a stand-in for the Metal compiler
        compiler.write_text('#!/bin/sh\nwhile [ "$1" != "-o" ]; do shift; done\ncp "$3" "$2"\n')
        compiler.chmod(compiler.stat().st_mode | stat.S_IEXEC)
        self.compiler = str(compiler)

    def put(self, name, text):
        (self.cache / name).write_text(text)
        return hashlib.sha256(text.encode()).hexdigest()

    def build(self):
        return run(['--work', str(self.work), '--chain', CHAIN, '--metal', self.compiler])

    def libs(self):
        return self.work / 'build/shader-libs'

    def test_compiles_the_cache_named_by_source_hash(self):
        digest = self.put('0017_b.metal', '// translated variant')
        self.put('0017_b.json', '{}')  # beside the source, not a source: ignored
        code, out, _ = self.build()
        self.assertEqual(code, 0)
        lib = self.libs() / f'{digest}.metallib'
        self.assertTrue(lib.is_file())
        self.assertEqual(lib.read_text(), '// translated variant')
        self.assertEqual(sorted(p.name for p in self.libs().iterdir() if p.suffix == '.metallib'), [f'{digest}.metallib'])
        self.assertIn('1 compiled, 0 reused, 0 failed of 1', out)

    def test_reuses_what_is_already_there(self):
        self.put('a_b.metal', '// one')
        self.build()
        code, out, _ = self.build()
        self.assertEqual(code, 0)
        self.assertIn('0 compiled, 1 reused, 0 failed of 1', out)

    def test_a_new_compiler_rebuilds_from_scratch(self):
        digest = self.put('a_b.metal', '// one')
        self.build()
        self.assertTrue((self.libs() / f'{digest}.metallib').is_file())
        other = self.root / 'metal2'
        other.write_text('#!/bin/sh\nwhile [ "$1" != "-o" ]; do shift; done\ncp "$3" "$2"\n')
        other.chmod(other.stat().st_mode | stat.S_IEXEC)
        code, out, _ = run(['--work', str(self.work), '--chain', CHAIN, '--metal', str(other)])
        self.assertEqual(code, 0)
        self.assertIn('1 compiled, 0 reused', out, 'a changed compiler identity must not reuse the old libraries')

    def test_an_empty_cache_is_not_an_error(self):
        code, out, _ = self.build()
        self.assertEqual(code, 0)
        self.assertIn('nothing to compile', out)
        self.assertFalse(self.libs().exists())

    def test_a_failing_source_is_reported_and_left_out(self):
        (self.cache / 'bad_b.metal').write_text('not metal')
        bad = self.root / 'metal-fail'
        bad.write_text('#!/bin/sh\nexit 1\n')
        bad.chmod(bad.stat().st_mode | stat.S_IEXEC)
        code, out, err = run(['--work', str(self.work), '--chain', CHAIN, '--metal', str(bad)])
        self.assertEqual(code, 0)
        self.assertIn('0 compiled, 0 reused, 1 failed of 1', out)
        self.assertIn('bad_b.metal', err)
        self.assertEqual([p for p in self.libs().glob('*.metallib')], [])

    def test_the_chain_default_comes_from_the_header(self):
        self.assertEqual(builder.chain(ROOT / 'games/nfs'), CHAIN)


if __name__ == '__main__':
    unittest.main()
