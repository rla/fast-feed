all: check test

test:
	mocha tests

check:
	jshint index.js tests

.PHONY: test check