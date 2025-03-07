.PHONY: configure build check clean

configure:
	_scripts\configure.bat

build:
	_scripts\build.bat

check:
	_scripts\test.bat

clean:
	_scripts\clean.bat
