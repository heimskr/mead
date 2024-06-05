mead_sources = run_command('grabber.sh', check: true).stdout().strip().split('\n')

mead_deps = [
	dependency('threads'),
	dependency('re2'),
]

inc_dirs = [
	include_directories('..' / 'include'),
]

exe = executable('mead', mead_sources,
	dependencies: mead_deps,
	install: true,
	include_directories: [inc_dirs])

test('basic', exe)
