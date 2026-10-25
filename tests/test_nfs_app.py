"""The packaged NFS app: what build_app.py puts in Contents/Resources/nfs.

The launcher chdirs into that folder and the loader, the shader translator and the shared-font code read their files by
path from it, so the list of files is a contract: a missing load-set image or main.npdm stops the boot, and a path that
names a predecessor tree would make the bundle need this repository.
"""
import importlib.util
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
_spec = importlib.util.spec_from_file_location('build_app_nfs', ROOT / 'build_app.py')
builder = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(builder)


class NfsAppTests(unittest.TestCase):
    def test_the_bundle_carries_the_whole_load_set(self):
        files = builder.nfs_app_files()
        for name in ('rtld', 'main', 'sdk', 'subsdk0', 'subsdk1'):
            self.assertIn(f'rewritten/nfs100/{name}.patched.bin', files)
        self.assertIn('extracted/nfs100/exefs/main.npdm', files)
        self.assertIn('extracted/nfs100/romfs/runtime-index.bin', files)

    def test_every_bundled_path_is_inside_the_work_tree(self):
        for rel in builder.nfs_app_files():
            self.assertFalse(rel.startswith('/'), rel)
            self.assertNotIn('..', Path(rel).parts, rel)
            self.assertFalse(rel.startswith(('games/', 'totk/', 'by3/')), rel)

    def test_the_app_identity_names_this_title(self):
        name, bundle_id, executable = builder.APPS['nfs']
        self.assertEqual(bundle_id, 'org.macnativegames.nfs')
        self.assertEqual(executable, 'NFS')
        self.assertEqual(builder.NFS100['title_id'], '010029B0118E8000')
        self.assertEqual(builder.TITLES[builder.NFS100['title_id']], 'nfs')


if __name__ == '__main__':
    unittest.main()
