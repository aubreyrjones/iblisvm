all:
	./scons.py -j 4

one:
	./scons.py 

clean:
	./scons.py -c

release:
	./scons.py -j 4 mode=release

deps:
	./scons.py --dry-run --tree=prune
