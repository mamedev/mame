#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:MAMEdev Team

import decimal
import pathlib
import sys
import tempfile
import unittest


sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1]))
import ci_shards


class GeneratedProjectInventoryTest(unittest.TestCase):

    def setUp(self):
        self.temporary_directory = tempfile.TemporaryDirectory()
        self.source_root = pathlib.Path(self.temporary_directory.name)
        self.solution_dir = self.source_root / 'build' / 'projects' / 'test'
        self.solution_dir.mkdir(parents=True)
        self.solution_makefile = self.solution_dir / 'Makefile'
        self.profile = {
                'complete_map_hash': 'checked-map',
                'driver_map': {'shards': []},
                'library_shards': [{
                        'name': 'core',
                        'outputs': ['libknown.a', 'libremoved.a'],
                        'targets': ['known', 'removed']}],
                'profile': {
                        'archive': {'prefix': 'lib', 'suffix': '.a'},
                        'build': {'configuration': 'release64'},
                        'deferred_projects': ['app'],
                        'executable_project': 'app',
                        'executable_suffix': '',
                        'id': 'test-profile'}}

    def tearDown(self):
        self.temporary_directory.cleanup()

    def write_solution(self, projects):
        self.solution_makefile.write_text(
                'PROJECTS := %s\n' % ' '.join(projects), encoding='utf-8')

    def write_project(self, name, output, dependencies=(), archive=True):
        link_command = (
                '$(AR) -qc $(TARGET)'
                if archive
                else '$(CXX) -o $(TARGET) $(LINKOBJS) $(LIBS)')
        makefile = self.solution_dir / ('%s.make' % name)
        makefile.write_text(
                '\n'.join((
                        'ifeq ($(config),release64)',
                        '  TARGETDIR = ../../../out',
                        '  override TARGET = $(TARGETDIR)/%s' % output,
                        '  LIBDEPS +=%s' % (
                                (' ' + ' '.join(dependencies)) if dependencies else ''),
                        '  LINKCMD = %s' % link_command,
                        'endif',
                        '')),
                encoding='utf-8')

    def reconciliation_profile(self):
        archive = {'prefix': 'lib', 'suffix': '.a'}
        library_definition = ci_shards.shard_definition(
                'libraries', ['known', 'removed'],
                ['libknown.a', 'libremoved.a'])
        driver_map = ci_shards.build_driver_map(
                {'driver-1': ['driver']},
                {'driver': decimal.Decimal('2.000')},
                {'driver': {'sample_count': 1, 'weight': 2.0}},
                {'compiler': 'test'},
                [{'file': 'timings.csv', 'run_id': '1', 'sha256': '0' * 64}],
                decimal.Decimal('2.000'),
                decimal.Decimal('2.000'),
                archive)
        profile = {
                '$schema': '../ci-shard-map.schema.json',
                'complete_map_hash': None,
                'driver_map': driver_map,
                'library_shards': [{
                        'definition_hash': ci_shards.canonical_hash(
                                library_definition),
                        'name': 'core',
                        'outputs': library_definition['outputs'],
                        'targets': library_definition['targets']}],
                'profile': {
                        'archive': archive,
                        'build': {'configuration': 'release64'},
                        'build_root': 'out',
                        'deferred_projects': ['app'],
                        'executable_project': 'app',
                        'executable_suffix': '',
                        'final_projects': ['app'],
                        'id': 'test-profile'},
                'schema_version': ci_shards.PROFILE_SCHEMA_VERSION}
        profile['complete_map_hash'] = ci_shards.canonical_hash(
                ci_shards.profile_map_identity(profile))
        return profile

    def test_inventory_records_outputs_kinds_and_project_dependencies(self):
        self.write_solution(['app', 'known', 'newlib'])
        self.write_project('known', 'libknown.a')
        self.write_project(
                'newlib', 'libnewlib.a', ('../../../out/libknown.a',))
        self.write_project(
                'app', 'app',
                ('../../../out/libknown.a', '../../../out/libnewlib.a'),
                archive=False)

        inventory = ci_shards.generated_project_inventory(
                self.solution_makefile, self.profile, self.source_root)
        projects = dict(
                (project['name'], project) for project in inventory['projects'])

        self.assertEqual('static-library', projects['newlib']['kind'])
        self.assertEqual(['out/libnewlib.a'], projects['newlib']['outputs'])
        self.assertEqual(['known'], projects['newlib']['dependencies'])
        self.assertEqual(['known', 'newlib'], projects['app']['dependencies'])
        self.assertEqual([], projects['app']['metadata_errors'])

    def test_drift_report_is_nonfatal_and_renders_html(self):
        self.write_solution(['app', 'known', 'newlib'])
        self.write_project('known', 'libknown.a')
        self.write_project('newlib', 'libnewlib.a')
        self.write_project('app', 'app', archive=False)

        inventory = ci_shards.generated_project_inventory(
                self.solution_makefile, self.profile, self.source_root)
        report = ci_shards.generated_target_drift(inventory, self.profile)
        summary = ci_shards.render_target_drift_summary(report)

        self.assertTrue(report['has_drift'])
        self.assertEqual(
                ['newlib'],
                [project['name'] for project in report['added_projects']])
        self.assertEqual(
                ['removed'],
                [project['name'] for project in report['removed_projects']])
        self.assertIn('<h1>CI shard target manifest drift detected</h1>', summary)
        self.assertIn('<code>newlib</code>', summary)
        self.assertIn('<code>removed</code>', summary)

    def test_exact_inventory_has_no_summary(self):
        self.write_solution(['app', 'known', 'removed'])
        self.write_project('known', 'libknown.a')
        self.write_project('removed', 'libremoved.a')
        self.write_project('app', 'app', archive=False)

        inventory = ci_shards.generated_project_inventory(
                self.solution_makefile, self.profile, self.source_root)
        report = ci_shards.generated_target_drift(inventory, self.profile)

        self.assertFalse(report['has_drift'])
        self.assertEqual('exact', report['recovery_mode'])
        self.assertEqual('', ci_shards.render_target_drift_summary(report))

    def test_failed_reconciliation_is_visible_in_summary(self):
        self.write_solution(['app', 'known', 'newlib'])
        self.write_project('known', 'libknown.a')
        self.write_project('newlib', 'newlib.exe', archive=False)
        self.write_project('app', 'app', archive=False)

        inventory = ci_shards.generated_project_inventory(
                self.solution_makefile, self.profile, self.source_root)
        report = ci_shards.generated_target_drift(
                inventory,
                self.profile,
                recovery_mode='reconciliation-failed',
                reconciliation_errors=['newlib is not an independent archive'])
        summary = ci_shards.render_target_drift_summary(report)

        self.assertIn('<code>reconciliation-failed</code>', summary)
        self.assertIn('newlib is not an independent archive', summary)

    def test_missing_primary_executable_is_fatal(self):
        self.write_solution(['known'])
        self.write_project('known', 'libknown.a')

        with self.assertRaisesRegex(ValueError, 'primary executable project'):
            ci_shards.generated_project_inventory(
                    self.solution_makefile, self.profile, self.source_root)

    def test_reconcile_assigns_new_archive_and_removes_missing_archive(self):
        profile = self.reconciliation_profile()
        self.write_solution(['app', 'driver', 'known', 'newlib'])
        self.write_project('known', 'libknown.a')
        self.write_project('driver', 'libdriver.a')
        self.write_project('newlib', 'libnewlib.a')
        self.write_project('app', 'app', archive=False)

        inventory = ci_shards.generated_project_inventory(
                self.solution_makefile, profile, self.source_root)
        runtime, assignments = ci_shards.reconcile_profile_map(
                inventory, profile, self.source_root / 'test-profile.json')
        report = ci_shards.generated_target_drift(
                inventory,
                profile,
                recovery_mode='runtime-reconciled',
                provisional_assignments=assignments,
                runtime_manifest_hash=runtime['complete_map_hash'])

        self.assertEqual(['known'], runtime['library_shards'][0]['targets'])
        self.assertEqual(
                ['driver', 'newlib'],
                runtime['driver_map']['shards'][0]['targets'])
        self.assertEqual('driver-1', assignments['newlib'])
        self.assertEqual(
                {'sample_count': 0, 'weight': 2.0},
                runtime['driver_map']['timing']['targets']['newlib'])
        self.assertEqual('runtime-reconciled', report['recovery_mode'])
        self.assertEqual(
                [{'group': 'driver-1', 'project': 'newlib'}],
                report['provisional_assignments'])
        self.assertIn('<code>driver-1</code>',
                      ci_shards.render_target_drift_summary(report))
        self.assertEqual(
                runtime,
                ci_shards.reconcile_profile_map(
                        inventory, profile,
                        self.source_root / 'test-profile.json')[0])

    def test_reconcile_preserves_exact_profile(self):
        profile = self.reconciliation_profile()
        self.write_solution(['app', 'driver', 'known', 'removed'])
        self.write_project('known', 'libknown.a')
        self.write_project('removed', 'libremoved.a')
        self.write_project('driver', 'libdriver.a')
        self.write_project('app', 'app', archive=False)

        inventory = ci_shards.generated_project_inventory(
                self.solution_makefile, profile, self.source_root)
        runtime, assignments = ci_shards.reconcile_profile_map(
                inventory, profile, self.source_root / 'test-profile.json')

        self.assertEqual(profile, runtime)
        self.assertEqual({}, assignments)

    def test_reconcile_rejects_archive_with_project_dependencies(self):
        profile = self.reconciliation_profile()
        self.write_solution(['app', 'driver', 'known', 'newlib', 'removed'])
        self.write_project('known', 'libknown.a')
        self.write_project('removed', 'libremoved.a')
        self.write_project('driver', 'libdriver.a')
        self.write_project('newlib', 'libnewlib.a', ('../../../out/libknown.a',))
        self.write_project('app', 'app', archive=False)

        inventory = ci_shards.generated_project_inventory(
                self.solution_makefile, profile, self.source_root)
        with self.assertRaisesRegex(ValueError, 'project dependencies are not supported'):
            ci_shards.reconcile_profile_map(
                    inventory, profile, self.source_root / 'test-profile.json')

    def test_reconcile_rejects_removed_deferred_project(self):
        profile = self.reconciliation_profile()
        profile['profile']['deferred_projects'].append('tool')
        profile['profile']['deferred_projects'].sort()
        profile['complete_map_hash'] = ci_shards.canonical_hash(
                ci_shards.profile_map_identity(profile))
        self.write_solution(['app', 'driver', 'known', 'removed'])
        self.write_project('known', 'libknown.a')
        self.write_project('removed', 'libremoved.a')
        self.write_project('driver', 'libdriver.a')
        self.write_project('app', 'app', archive=False)

        inventory = ci_shards.generated_project_inventory(
                self.solution_makefile, profile, self.source_root)
        with self.assertRaisesRegex(ValueError, 'removed deferred projects: tool'):
            ci_shards.reconcile_profile_map(
                    inventory, profile, self.source_root / 'test-profile.json')


if __name__ == '__main__':
    unittest.main()
