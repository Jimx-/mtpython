import errno

with open('errno_def.h', 'w') as f:
	f.write('/* Generated by generate_errno.py, do not edit */\n')
	f.write('#define W(x) (space->wrap_int(context, (x)))\n')
	for e in dir(errno):
		if e.startswith('E'):
			f.write('add_def("' + e + '", W(' + str(getattr(errno, e)) + '));\n')
	f.write('#undef W')