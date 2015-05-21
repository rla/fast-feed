all: check test

test:
	mocha tests

test-leak:
	node --gc_global tests-leak/cdata.js

check:
	jshint index.js tests

.PHONY: test test-leak check
